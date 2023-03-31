#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <limits.h>

#ifndef _LANGUAGE_C
# define _LANGUAGE_C
#endif
#include <PR/gbi.h>

#include "gfx_frontend.h"
#include "gfx_backend.h"
#include "gfx_cc.h"
#include "macros.h"

#include "pc/fixed_pt.h"

#define ALIGN(x, a) (((x) + (a - 1)) & ~(a - 1))

#define MAX_TEXTURES 3072
#define TEXCACHE_STEP 0x10000

enum WrapType {
    WRAP_REPEAT = 0,
    WRAP_CLAMP  = 1,
    WRAP_MIRROR = 2,
};

enum DrawFlags {
    DRAW_ZWRITE = 1,
    DRAW_BLEND = 2,
    DRAW_BLEND_EDGE = 4,
};

enum MixType {
    SH_MT_NONE            = 0,
    SH_MT_COLOR           = 1 << 0,
    SH_MT_COLOR_COLOR     = 1 << 1,
    SH_MT_TEXTURE         = 1 << 2,
    SH_MT_TEXTURE_COLOR   = 1 << 3,
    SH_MT_TEXTURE_TEXTURE = 1 << 4,
};

typedef union Vector2 { 
    struct { fix64 x, y; };
    struct { fix64 u, v; };
} Vector2;

typedef union Vector3 {
    struct { fix64 r, g, b; };
    struct { fix64 x, y, z; };
    Vector2 xy;
    fix64 v[3];
} Vector3;

typedef union Vector4 {
    struct { fix64 r, g, b, a; };
    struct { fix64 x, y, z, w; };
    Vector2 xy;
    Vector3 xyz;
    fix64 v[4];
} Vector4;

typedef union Color4 {
    struct { uint8_t r, g, b, a; };
    uint32_t c;
} Color4;

struct Tri {
    fix64 *v0;
    fix64 *v1;
    fix64 *v2;
};

struct Texture;

// texture sampling function: takes integer u,v and wraps/clamps it, samples texture, returns color
typedef Color4 (*sample_fn_t)(const struct Texture * const, const int, const int);
// pixel drawing function: does blending, zwriting, alpha edge checking or whatever else, then plots pixel
typedef void (*draw_fn_t)(const int idx, uint16_t uz, const Color4 src);
// color combiner: takes float vertex properties and obtains final fragment color from them
typedef Color4 (*combine_fn_t)(const fix64 z, const fix64 *props);
// rasterizer: walks the triangle and interpolates a fixed amount of vertex properties
typedef void (*rast_fn_t)(const struct Tri tri);

struct ShaderProgram {
    uint32_t shader_id;
    struct CCFeatures cc;
    enum MixType mix;
    uint32_t draw_flags;
    int num_props;
    combine_fn_t combine;
    rast_fn_t rast;
};

struct Texture {
    int w, h;           // size
    int wrap_w, wrap_h; // size - 1 for wrapping
    bool filter;        // linear filter
    uint32_t addr;      // offset into texcache
    sample_fn_t sample; // sampling function (does wrapping/clamping)
};

struct Viewport {
    int x, y, w, h; // rect
    fix64 cx, cy;   // center
    fix64 hw, hh;   // half size
    // float zn, zf, cz, hz; // FIXME: ztrick
};

struct ClipRect {
    int x0, y0; // top left
    int x1, y1; // bottom right
};

uint32_t *gfx_output;

// this is set in the drawing functions
static draw_fn_t draw_fn;

static struct ShaderProgram shader_program_pool[0];
static uint8_t shader_program_pool_size;
static struct ShaderProgram *cur_shader = NULL;

static struct Texture *cur_tex[2]; // currently selected textures for both tiles
static struct Texture tex_hdr[MAX_TEXTURES];
static int cur_tmu = 0; // select tile (used only for uploading)

// texture cache: linearly stores RGBA data of every cached texture
static uint8_t *texcache;
static uint32_t texcache_addr; // current offset into cache
static uint32_t texcache_size; // cache capacity

static bool do_blend; // fragment blending toggle
static bool do_clip;  // scissor toggle

static struct ClipRect r_clip;
static struct Viewport r_view;

static Color4 fog_color; // this is set by set_fog_color() calls from gfx_pc

static bool z_test;        // whether to perform depth testing
static bool z_write;       // whether to write into the Z buffer
static fix64 z_offset;     // offset for decal mode
static uint16_t *z_buffer;

static int scr_width;
static int scr_height;
static int scr_size; // scr_width * scr_height

static int num = 0;

// color component interpolation table:
// lerp(x, y, t) = x + (y - x) * t
// the first index is x, the second is (y - x) + 256
static uint8_t lerp_tab[256][256 * 2 + 1];
// color component multiplication table: [x][y] = (x * y) / 256;
static uint8_t mult_tab[256][256];
// dither kernel for unreal texture filtering
static const Vector2 dither_tab[2][2] = {
    //{ {{ 0.25f, 0.00f }}, {{ 0.50f, 0.75f }} },
    //{ {{ 0.75f, 0.50f }}, {{ 0.00f, 0.25f }} },
};

/* math shit */

static inline float fclamp01(const float v) {
    return (v < 0.f) ? 0.f : (v > 1.f) ? 1.f : v;
}

static inline uint16_t u16clamp(const int v) {
    return (v < 0) ? (uint16_t)0 : (v > 0xFFFF) ? (uint16_t)0xFFFF : (uint16_t)v;
}

static inline int iwrap0w(const int x, const int wrap) {
    return x & wrap;
}

static inline int iclamp0w(const int x, const int wrap) {
    return (x < 0) ? 0 : (x > wrap) ? wrap : x;
}

static inline int imirror0w(const int x, const int wrap) {
    return iclamp0w(abs(x), wrap); // NOTE: this is not a universal solution
}

static inline float flerp(const float v0, const float v1, const float t) {
    return v0 + t * (v1 - v0);
}

static inline bool vec2_cmp(const Vector2 v1, const Vector2 v2) {
    return (v1.y == v2.y) ? (v1.x > v2.x) : (v1.y > v2.y);
}

static inline Vector4 vec4_sub(const Vector4 *v1, const Vector4 *v2) {
    return (Vector4) {{ v1->x - v2->x, v1->y - v2->y, v1->z - v2->z, 1.f }};
}

static inline Vector4 vec4_lerp(const Vector4 *v1, const Vector4 *v2, const float t) {
    return (Vector4) {{
        flerp(v1->x, v2->x, t),
        flerp(v1->y, v2->y, t),
        flerp(v1->z, v2->z, t),
        flerp(v1->w, v2->w, t),
    }};
}

static inline Color4 rgba_modulate(const Color4 c1, const Color4 c2) {
    return (Color4) {{
        .r = mult_tab[c1.r][c2.r],
        .g = mult_tab[c1.g][c2.g],
        .b = mult_tab[c1.b][c2.b],
        .a = mult_tab[c1.a][c2.a],
    }};
}

static inline Color4 rgba_blend(const Color4 src, const Color4 dst, const uint8_t a) {
    const uint8_t ia = 0xFF - a;
    return (Color4) {{
        .r = mult_tab[src.r][a] + mult_tab[dst.r][ia],
        .g = mult_tab[src.g][a] + mult_tab[dst.g][ia],
        .b = mult_tab[src.b][a] + mult_tab[dst.b][ia],
        .a = dst.a,
    }};
}

static inline Color4 rgba_lerp(const Color4 c1, const Color4 c2, const uint8_t t) {
    return (Color4) {{
        .r = c1.r + lerp_tab[t][0xFF + c2.r - c1.r],
        .g = c1.g + lerp_tab[t][0xFF + c2.g - c1.g],
        .b = c1.b + lerp_tab[t][0xFF + c2.b - c1.b],
        .a = c1.a + lerp_tab[t][0xFF + c2.a - c1.a],
    }};
}

static inline int imin(const int a, const int b) {
    return (a < b) ? a : b;
}

static inline int imax(const int a, const int b) {
    return (a > b) ? a : b;
}

static inline void viewport_transform(Vector4 *v) {
    // gfx_pc.c with ENABLE_SOFTRAST defined will feed us with everything already pre-multiplied by inverse of w
    v->x = fix_mult(v->x, r_view.hw) + r_view.cx + FIX_ONE_HALF;
    v->y = fix_mult(v->y, r_view.hh) + r_view.cy + FIX_ONE_HALF;
    // v->w is also already 1.f / v->w
}

/* texture sampling functions */

static inline Color4 tex_get(const struct Texture * const tex, const int x, const int y) {
    return (Color4) { .c = ((const uint32_t *)(texcache + tex->addr))[y * tex->w + x] };
}

static Color4 tex_sample_nearest_rr(const struct Texture * const tex, const int x, const int y) {
    return tex_get(tex, iwrap0w(x, tex->wrap_w), iwrap0w(y, tex->wrap_h));
}

static Color4 tex_sample_nearest_rc(const struct Texture * const tex, const int x, const int y) {
    return tex_get(tex, iwrap0w(x, tex->wrap_w), iclamp0w(y, tex->wrap_h));
}

static Color4 tex_sample_nearest_rm(const struct Texture * const tex, const int x, const int y) {
    return tex_get(tex, iwrap0w(x, tex->wrap_w), imirror0w(y, tex->wrap_h));
}

static Color4 tex_sample_nearest_cc(const struct Texture * const tex, const int x, const int y) {
    return tex_get(tex, iclamp0w(x, tex->wrap_w), iclamp0w(y, tex->wrap_h));
}

static Color4 tex_sample_nearest_cr(const struct Texture * const tex, const int x, const int y) {
    return tex_get(tex, iclamp0w(x, tex->wrap_w), iwrap0w(y, tex->wrap_h));
}

static Color4 tex_sample_nearest_cm(const struct Texture * const tex, const int x, const int y) {
    return tex_get(tex, iclamp0w(x, tex->wrap_w), imirror0w(y, tex->wrap_h));
}

static Color4 tex_sample_nearest_mm(const struct Texture * const tex, const int x, const int y) {
    return tex_get(tex, imirror0w(x, tex->wrap_w), imirror0w(y, tex->wrap_h));
}

static Color4 tex_sample_nearest_mc(const struct Texture * const tex, const int x, const int y) {
    return tex_get(tex, imirror0w(x, tex->wrap_w), iclamp0w(y, tex->wrap_h));
}

static Color4 tex_sample_nearest_mr(const struct Texture * const tex, const int x, const int y) {
    return tex_get(tex, imirror0w(x, tex->wrap_w), iwrap0w(y, tex->wrap_h));
}

static inline Color4 tex_sample_linear(const struct Texture * const tex, const fix64 u, const fix64 v, const Vector2 d) {
    const int x = FIX_2_INT(d.u + u * tex->w);
    const int y = FIX_2_INT(d.v + v * tex->h);
    return tex->sample(tex, x, y);
}

static inline Color4 tex_sample_nearest(const struct Texture * const tex, const fix64 u, const fix64 v) {
    const int x = FIX_2_INT(u * tex->w);
    const int y = FIX_2_INT(v * tex->h);
    return tex->sample(tex, x, y);
}

/* color combiners */

#define tex_sample tex_sample_nearest

static Color4 combine_rgb(const fix64 z, const fix64 *props) { // 3
    return (Color4){ { .r = fix_mult_i(props[0], z),
                       .g = fix_mult_i(props[1], z),
                       .b = fix_mult_i(props[2], z),
                       .a = 0xFF } };
}

static Color4 combine_rgba(const fix64 z, const fix64 *props) { // 4
    return (Color4){ { .r = fix_mult_i(props[0], z),
                       .g = fix_mult_i(props[1], z),
                       .b = fix_mult_i(props[2], z),
                       .a = fix_mult_i(props[3], z) } };
}

static Color4 combine_fog_rgb(const fix64 z, const fix64 *props) { // 5
    const uint8_t fog = fix_mult_i(props[0], z);
    const Color4 c = (Color4){ { .r = fix_mult_i(props[1], z),
                                 .g = fix_mult_i(props[2], z),
                                 .b = fix_mult_i(props[3], z),
                                 .a = 0xFF } };
    return rgba_blend(fog_color, c, fog);
}

static Color4 combine_fog_rgba(const fix64 z, const fix64 *props) {
    const uint8_t fog = fix_mult_i(props[0], z);
    const Color4 c = (Color4){ { .r = fix_mult_i(props[1], z),
                                 .g = fix_mult_i(props[2], z),
                                 .b = fix_mult_i(props[3], z),
                                 .a = fix_mult_i(props[4], z) } };
    return rgba_blend(fog_color, c, fog);
}

static Color4 combine_rgba_rgba(const fix64 z, const fix64 *props) {
    const Color4 ca = (Color4){ { .r = fix_mult_i(props[0], z),
                                  .g = fix_mult_i(props[1], z),
                                  .b = fix_mult_i(props[2], z),
                                  .a = fix_mult_i(props[3], z) } };
    const Color4 cb = (Color4){ { .r = fix_mult_i(props[4], z),
                                  .g = fix_mult_i(props[5], z),
                                  .b = fix_mult_i(props[6], z),
                                  .a = fix_mult_i(props[7], z) } };
    return rgba_modulate(ca, cb);
}

static Color4 combine_tex(const fix64 z, const fix64 *props) {
    return tex_sample(cur_tex[0], fix_mult(props[0], z), fix_mult(props[1], z));
}

static Color4 combine_tex_fog(const fix64 z, const fix64 *props) {
    const Color4 tc = tex_sample(cur_tex[0], fix_mult(props[0], z), fix_mult(props[1], z));
    const uint8_t fog = fix_mult_i(props[2], z);
    return rgba_blend(fog_color, tc, fog);
}

static Color4 combine_tex_rgb(const fix64 z, const fix64 *props) {
    const Color4 tc = tex_sample(cur_tex[0], fix_mult(props[0], z), fix_mult(props[1], z));
    const Color4 cc = (Color4){ { .r = fix_mult_i(props[2], z),
                                  .g = fix_mult_i(props[3], z),
                                  .b = fix_mult_i(props[4], z),
                                  .a = 0xFF } };
    return rgba_modulate(tc, cc);
}

static Color4 combine_tex_fog_rgb(const fix64 z, const fix64 *props) {
    const Color4 tc = tex_sample(cur_tex[0], fix_mult(props[0], z), fix_mult(props[1], z));
    const uint8_t fog = fix_mult_i(props[2], z);
    const Color4 cc = (Color4){ { .r = fix_mult_i(props[3], z),
                                  .g = fix_mult_i(props[4], z),
                                  .b = fix_mult_i(props[5], z),
                                  .a = 0xFF } };
    return rgba_blend(fog_color, rgba_modulate(tc, cc), fog);
}

static Color4 combine_tex_rgb_decal(const fix64 z, const fix64 *props) {
    const Color4 tc = tex_sample(cur_tex[0], fix_mult(props[0], z), fix_mult(props[1], z));
    const Color4 cc = (Color4){ { .r = fix_mult_i(props[2], z),
                                  .g = fix_mult_i(props[3], z),
                                  .b = fix_mult_i(props[4], z),
                                  .a = 0xFF } };
    return rgba_blend(tc, cc, tc.a);
}

static Color4 combine_tex_rgba(const fix64 z, const fix64 *props) {
    const Color4 tc = tex_sample(cur_tex[0], fix_mult(props[0], z), fix_mult(props[1], z));
    const Color4 cc = (Color4){ { .r = fix_mult_i(props[2], z),
                                  .g = fix_mult_i(props[3], z),
                                  .b = fix_mult_i(props[4], z),
                                  .a = fix_mult_i(props[5], z) } };
    return rgba_modulate(tc, cc);
}

static Color4 combine_tex_rgba_texa(const fix64 z, const fix64 *props) {
    const Color4 tc = tex_sample(cur_tex[0], fix_mult(props[0], z), fix_mult(props[1], z));
    const Color4 cc = (Color4){ { .r = fix_mult_i(props[2], z),
                                  .g = fix_mult_i(props[3], z),
                                  .b = fix_mult_i(props[4], z),
                                  .a = 0xFF } };
    return rgba_modulate(tc, cc);
}

static Color4 combine_tex_fog_rgba(const fix64 z, const fix64 *props) {
    const Color4 tc = tex_sample(cur_tex[0], fix_mult(props[0], z), fix_mult(props[1], z));
    const uint8_t fog = fix_mult_i(props[2], z);
    const Color4 cc = (Color4){ { .r = fix_mult_i(props[3], z),
                                  .g = fix_mult_i(props[4], z),
                                  .b = fix_mult_i(props[5], z),
                                  .a = fix_mult_i(props[6], z) } };
    return rgba_blend(fog_color, rgba_modulate(tc, cc), fog);
}

static Color4 combine_tex_rgba_decal(const fix64 z, const fix64 *props) {
    const Color4 tc = tex_sample(cur_tex[0], fix_mult(props[0], z), fix_mult(props[1], z));
    const Color4 cc = (Color4){ { .r = fix_mult_i(props[2], z),
                                  .g = fix_mult_i(props[3], z),
                                  .b = fix_mult_i(props[4], z),
                                  .a = fix_mult_i(props[5], z) } };
    return rgba_blend(tc, cc, tc.a);
}

static Color4 combine_tex_rgb_rgb(const fix64 z, const fix64 *props) {
    const Color4 tc = tex_sample(cur_tex[0], fix_mult(props[0], z), fix_mult(props[1], z));
    const Color4 cc1 = (Color4){ { .r = fix_mult_i(props[2], z),
                                   .g = fix_mult_i(props[3], z),
                                   .b = fix_mult_i(props[4], z),
                                   .a = 0xFF } };
    const Color4 cc2 = (Color4){ { .r = fix_mult_i(props[5], z),
                                   .g = fix_mult_i(props[6], z),
                                   .b = fix_mult_i(props[7], z),
                                   .a = 0xFF } };
    return rgba_lerp(cc2, cc1, tc.r);
}

static Color4 combine_tex_tex_rgba(const fix64 z, const fix64 *props) {
    const fix64 u = fix_mult(props[0], z);
    const fix64 v = fix_mult(props[1], z);
    const Color4 tc1 = tex_sample(cur_tex[0], u, v);
    const Color4 tc2 = tex_sample(cur_tex[1], u, v);
    const uint8_t r = fix_mult_i(props[2], z);
    return rgba_lerp(tc1, tc2, r);
}

/* fragment plotters */
static void draw_pixel(const int idx, UNUSED const uint16_t z, Color4 src) {
    gfx_output[idx] = src.c;
}

static void draw_pixel_zwrite(const int idx, const uint16_t z, Color4 src) {
    gfx_output[idx] = src.c;
    z_buffer[idx] = z;
}

static void draw_pixel_blend(const int idx, UNUSED const uint16_t z, Color4 src) {
    const uint8_t a = src.a;
    const uint8_t ia = 255 - a;
    const Color4 dst = (Color4) { .c = gfx_output[idx] };
    src.r = mult_tab[src.r][a] + mult_tab[dst.r][ia];
    src.g = mult_tab[src.g][a] + mult_tab[dst.g][ia];
    src.b = mult_tab[src.b][a] + mult_tab[dst.b][ia];
    gfx_output[idx] = src.c;
}

static void draw_pixel_blend_zwrite(const int idx, const uint16_t z, Color4 src) {
    const uint8_t a = src.a;
    const uint8_t ia = 255 - a;
    const Color4 dst = (Color4) { .c = gfx_output[idx] };
    src.r = mult_tab[src.r][a] + mult_tab[dst.r][ia];
    src.g = mult_tab[src.g][a] + mult_tab[dst.g][ia];
    src.b = mult_tab[src.b][a] + mult_tab[dst.b][ia];
    gfx_output[idx] = src.c;
    z_buffer[idx] = z;
}

static void draw_pixel_blend_edge(const int idx, UNUSED const uint16_t z, Color4 src) {
    if (src.a > 0x80) {
        const uint8_t a = src.a;
        const uint8_t ia = 255 - a;
        const Color4 dst = (Color4) { .c = gfx_output[idx] };
        src.r = mult_tab[src.r][a] + mult_tab[dst.r][ia];
        src.g = mult_tab[src.g][a] + mult_tab[dst.g][ia];
        src.b = mult_tab[src.b][a] + mult_tab[dst.b][ia];
        gfx_output[idx] = src.c;
    }
}

static void draw_pixel_blend_edge_zwrite(const int idx, const uint16_t z, Color4 src) {
    if (src.a > 0x80) {
        const uint8_t a = src.a;
        const uint8_t ia = 255 - a;
        const Color4 dst = (Color4) { .c = gfx_output[idx] };
        src.r = mult_tab[src.r][a] + mult_tab[dst.r][ia];
        src.g = mult_tab[src.g][a] + mult_tab[dst.g][ia];
        src.b = mult_tab[src.b][a] + mult_tab[dst.b][ia];
        gfx_output[idx] = src.c;
        z_buffer[idx] = z;
    }
}

/* rasterizers */

#define R_RASTERIZE_TRI_SEG(y_a, y_b, nprops) \
    register int y = y_a; \
    register int y_end = y_b; \
    register int x, x_end; \
    register int idx; \
    fix64 dx, w; \
    uint16_t uz; \
    /* draw triangle segment from y_a to y_b */ \
    while (y < y_end) { \
        /* do scissor clipping */ \
        x = imax(r_clip.x0, FIX_2_INT(x_a)); \
        x_end = imin(r_clip.x1, FIX_2_INT(x_b)); \
        /* do X subpixel prestepping */ \
        dx = FIX_ONE - (x_a - INT_2_FIX(x)); \
        for (i = 2; i < nprops; ++i) p[i] = p_a[i] + fix_mult(dx, dp[i].x); \
        idx = scr_width * (scr_height - y - 1) + x; \
        /* draw scanline from current x_a to current x_b */ \
         while (x++ < x_end) { \
            uz = u16clamp(FIX_2_INT(p[2] * 65535 + z_offset)); \
            if (!z_test || uz <= z_buffer[idx]) { \
                /* Improve efficiency here? w is 1 very often */ \
                w = p[3] == FIX_ONE ? FIX_ONE : FIX_INV(p[3]); /* the combiner will multiply by w any props it needs to persp correct */ \
                draw_fn(idx, uz, cur_shader->combine(w, p + 4)); \
            } \
            for (i = 2; i < nprops; ++i) p[i] += dp[i].x; \
            ++idx; \
        } \
        /* advance scanline start and end and prop starts */ \
        x_a += dxdy_a; \
        x_b += dxdy_b; \
        for (i = 2; i < nprops; ++i) p_a[i] += dpdy_a[i]; \
        ++y; \
    }

#define R_RASTERIZE(tri, nprops) \
    const fix64 *v0 = (fix64 *) tri.v0; \
    const fix64 *v1 = (fix64 *) tri.v1; \
    const fix64 *v2 = (fix64 *) tri.v2; \
    const int y0i = imax(r_clip.y0, FIX_2_INT(v0[1])); \
    const int y1i = imax(y0i, FIX_2_INT(v1[1])); \
    const int y2i = imin(r_clip.y1, FIX_2_INT(v2[1])); \
    if ((y0i == y1i && y0i == y2i) || (FIX_2_INT(v0[0]) == FIX_2_INT(v1[0]) && FIX_2_INT(v0[0]) == FIX_2_INT(v2[0]))) \
        return; /* triangle has zero area */  \
    const Vector4 ab = (Vector4) {{ v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2], v1[3] - v0[3] }}; \
    const Vector4 ac = (Vector4) {{ v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2], v2[3] - v0[3] }}; \
    const Vector2 bc = (Vector2) {{ v2[0] - v1[0], v2[1] - v1[1] }}; \
    const fix64 denom = fix_div_s(FIX_ONE, fix_mult(ac.x, ab.y) - fix_mult(ab.x, ac.y)); \
    const fix64 dxdy_ab = ab.y != 0 ? fix_div_s(ab.x, ab.y) : ab.x > 0 ? LLONG_MAX : LLONG_MIN; /* x increment along ab */ \
    const fix64 dxdy_ac = ac.y != 0 ? fix_div_s(ac.x, ac.y) : ac.x > 0 ? LLONG_MAX : LLONG_MIN; /* x increment along ac */ \
    const fix64 dxdy_bc = bc.y != 0 ? fix_div_s(bc.x, bc.y) : bc.x > 0 ? LLONG_MAX : LLONG_MIN; /* PROTECT AGAINST DIV BY ZERO HERE */ \
    const bool side = dxdy_ac > dxdy_ab; /* which side the longer edge (AC) is on */ \
    const fix64 y_pre0 = FIX_ONE - (v0[1] - INT_2_FIX(y0i));  /* subpixel pre-step */ \
    fix64 dpdy_a[nprops]; /* vertex prop increments along left edge */ \
    fix64 p_a[nprops];    /* vertex leftmost points */ \
    fix64 p[nprops];      /* current vertex prop values */ \
    Vector2 dp[nprops]; /* X and Y increments for vertex props */ \
    register int i; \
    /* we'll interpolate z/w (p[2]), 1/w (p[3]) and the other properties (also divided by w) */ \
    for (i = 2; i < nprops; ++i) { \
        dp[i].x = fix_mult(fix_mult(v2[i] - v0[i], ab.y) - fix_mult(v1[i] - v0[i], ac.y), denom); \
        dp[i].y = fix_mult(fix_mult(v1[i] - v0[i], ac.x) - fix_mult(v2[i] - v0[i], ab.x), denom); \
    } \
num++; \
    if (!side) { \
        /* longer edge is on the left */ \
        const fix64 dxdy_a = dxdy_ac; \
        /* first column of this scanline is on AC */ \
        fix64 x_a = v0[0] + fix_mult(y_pre0, dxdy_a); \
        for (i = 2; i < nprops; ++i) { \
            dpdy_a[i] = fix_mult(dxdy_ac, dp[i].x) + dp[i].y; \
            p_a[i] = v0[i] + fix_mult(y_pre0, dpdy_a[i]); \
        } \
        if (y0i < y1i) { \
            /* left is AC, right is AB */ \
            const fix64 dxdy_b = dxdy_ab; \
            /* last column of this scanline */ \
            fix64 x_b = v0[0] + fix_mult(y_pre0, dxdy_ab); \
            R_RASTERIZE_TRI_SEG(y0i, y1i, nprops); \
        } \
        if (y1i < y2i) { \
            /* left is AC, right is BC */ \
            const fix64 dxdy_b = dxdy_bc; \
            /* calculate prestep for vertex B */ \
            const fix64 y_pre1 = FIX_ONE - (v1[1] - INT_2_FIX(y1i)); \
            fix64 x_b = v1[0] + fix_mult(y_pre1, dxdy_bc); \
            R_RASTERIZE_TRI_SEG(y1i, y2i, nprops); \
        } \
    } else { \
        /* longer edge is on the right */ \
        const fix64 dxdy_b = dxdy_ac; \
        /* last column of this scanline is on AC */ \
        fix64 x_b = v0[0] + fix_mult(y_pre0, dxdy_ac); \
        if (y0i < y1i) { \
            /* right is AC, left is AB */ \
            const fix64 dxdy_a = dxdy_ab; \
            fix64 x_a = v0[0] + fix_mult(y_pre0, dxdy_a); \
            for (i = 2; i < nprops; ++i) { \
                dpdy_a[i] = fix_mult(dxdy_ab, dp[i].x) + dp[i].y; \
                p_a[i] = v0[i] + fix_mult(y_pre0, dpdy_a[i]); \
            } \
            R_RASTERIZE_TRI_SEG(y0i, y1i, nprops); \
        } \
        if (y1i < y2i) { \
            /* right is AC, left is BC */ \
            const fix64 y_pre1 = FIX_ONE - (v1[1] - INT_2_FIX(y1i)); \
            const fix64 dxdy_a = dxdy_bc; \
            fix64 x_a = v1[0] + fix_mult(y_pre1, dxdy_a); \
            for (i = 2; i < nprops; ++i) { \
                dpdy_a[i] = fix_mult(dxdy_bc, dp[i].x) + dp[i].y; \
                p_a[i] = v1[i] + fix_mult(y_pre1, dpdy_a[i]); \
            } \
            R_RASTERIZE_TRI_SEG(y1i, y2i, nprops); \
        } \
    }

// define a bunch of rasterizers/interpolators for known property counts
// nprops includes XYZW

#define DEFINE_RAST_FUNC(nprops) \
    static void rast_fn_ ## nprops (const struct Tri tri) { R_RASTERIZE(tri, nprops); }

#define GET_RAST_FUNC(nprops) rast_fn_ ## nprops

DEFINE_RAST_FUNC(6)
DEFINE_RAST_FUNC(7)
DEFINE_RAST_FUNC(8)
DEFINE_RAST_FUNC(9)
DEFINE_RAST_FUNC(10)
DEFINE_RAST_FUNC(11)
DEFINE_RAST_FUNC(12)
DEFINE_RAST_FUNC(13)
DEFINE_RAST_FUNC(14)

static inline void pop_triangle(const fix64 *buf, const int stride) {
    Vector4 *v0 = (Vector4 *)buf;
    Vector4 *v1 = (Vector4 *)(buf + stride);
    Vector4 *v2 = (Vector4 *)(buf + (stride << 1));
    Vector4 *vt;

    // the vertices come to us in clip space, but already divided by w, still gotta transform
    viewport_transform(v0);
    viewport_transform(v1);
    viewport_transform(v2);

    // sort in Y order
    if (v0->y > v1->y) { vt = v0; v0 = v1; v1 = vt; }
    if (v0->y > v2->y) { vt = v0; v0 = v2; v2 = vt; }
    if (v1->y > v2->y) { vt = v1; v1 = v2; v2 = vt; }

    const struct Tri out = (struct Tri) { (fix64 *)v0, (fix64 *)v1, (fix64 *)v2 };
    cur_shader->rast(out);
}

static inline void depth_clear(void) {
    memset(z_buffer, 0xFF, scr_size << 1);
}

static inline void color_clear(void) {
    memset(gfx_output, 0x00, scr_size << 2);
}

/* FIXME: ztrick fucks with sky blending
static inline void depth_swap(void) {
    ++z_frame;
    if (z_frame & 1) {
        r_view.zn = 0.f;
        r_view.zf = 0.4999f;
        z_reverse = false;
    } else {
        r_view.zn = 1.f;
        r_view.zf = 0.5f;
        z_reverse = true;
    }
    r_view.hz = (r_view.zf - r_view.zn) * 0.5f;
    r_view.cz = (r_view.zn + r_view.zf) * 0.5f;
}
*/

/* interface */

static bool gfx_soft_z_is_from_0_to_1(void) {
    return true;
}

static void gfx_soft_unload_shader(struct ShaderProgram *old_prg) {
    if (cur_shader && (cur_shader == old_prg || !old_prg))
        cur_shader = NULL;
}

static void gfx_soft_load_shader(struct ShaderProgram *new_prg) {
    cur_shader = new_prg;
}

static struct ShaderProgram *gfx_soft_create_and_load_new_shader(uint32_t shader_id) {
    static const rast_fn_t rast_funcs[] = {
        NULL,
        NULL,
        GET_RAST_FUNC(6),
        GET_RAST_FUNC(7),
        GET_RAST_FUNC(8),
        GET_RAST_FUNC(9),
        GET_RAST_FUNC(10),
        GET_RAST_FUNC(11),
        GET_RAST_FUNC(12),
        GET_RAST_FUNC(13),
        GET_RAST_FUNC(14),
    };

    struct CCFeatures ccf;
    gfx_cc_get_features(shader_id, &ccf);

    struct ShaderProgram *prg = &shader_program_pool[shader_program_pool_size++];

    prg->shader_id = shader_id;
    prg->cc = ccf;

    int num_props = 0;

    if (ccf.opt_fog) num_props++; // software renderer only gets fog intensity

    num_props += ccf.num_inputs * (ccf.opt_alpha ? 4 : 3);
    num_props += ccf.used_textures[0] * 2;

    if (ccf.used_textures[0] && ccf.used_textures[1]) {
        prg->mix = SH_MT_TEXTURE_TEXTURE;
        prg->combine = combine_tex_tex_rgba; // only one such known shader
    } else if (ccf.used_textures[0] && ccf.num_inputs) {
        prg->mix = SH_MT_TEXTURE_COLOR;
        if (ccf.num_inputs > 1)
            prg->combine = combine_tex_rgb_rgb; // only one such known shader
        else if (shader_id == 0x0000038D || shader_id == 0x01200A00 || shader_id == 0x01045A00)
            prg->combine = ccf.opt_alpha ? combine_tex_rgba_decal : combine_tex_rgb_decal;
        else if (ccf.opt_fog)
            prg->combine = ccf.opt_alpha ? combine_tex_fog_rgba : combine_tex_fog_rgb;
        else if (ccf.opt_alpha)
            prg->combine = shader_id == 0x01A00045 ? combine_tex_rgba_texa : combine_tex_rgba;
        else
            prg->combine = combine_tex_rgb;
    } else if (ccf.used_textures[0]) {
        prg->mix = SH_MT_TEXTURE;
        prg->combine = ccf.opt_fog ? combine_tex_fog : combine_tex;
    } else if (ccf.num_inputs > 1) {
        prg->mix = SH_MT_COLOR_COLOR;
        prg->combine = combine_rgba_rgba; // only one such known shader
    } else if (ccf.num_inputs) {
        prg->mix = SH_MT_COLOR;
        if (ccf.opt_fog)
            prg->combine = ccf.opt_alpha ? combine_fog_rgba : combine_fog_rgb;
        else
            prg->combine = ccf.opt_alpha ? combine_rgba : combine_rgb;
    }

    if (ccf.opt_alpha) {
        if (ccf.opt_texture_edge)
            prg->draw_flags = DRAW_BLEND_EDGE;
        else
            prg->draw_flags = DRAW_BLEND;
    } else {
        prg->draw_flags = 0;
    }

    prg->num_props = num_props;
    // pick rasterizer that interps the amount of float properties this shader requires
    prg->rast = rast_funcs[num_props];

    gfx_soft_load_shader(prg);

    return prg;
}
#pragma GCC push_options // BANDAID FIX: compiling this function with any other optimization level crashes the emulator for some reason
#pragma GCC optimize("-Og")
static struct ShaderProgram *gfx_soft_lookup_shader(uint32_t shader_id) {
    for (size_t i = 0; i < shader_program_pool_size; i++)
        if (shader_program_pool[i].shader_id == shader_id)
            return &shader_program_pool[i];
    return NULL;
}
#pragma GCC pop_options

static void gfx_soft_shader_get_info(struct ShaderProgram *prg, uint8_t *num_inputs, bool used_textures[2]) {
    *num_inputs = prg->cc.num_inputs;
    used_textures[0] = prg->cc.used_textures[0];
    used_textures[1] = prg->cc.used_textures[1];
}

static uint32_t gfx_soft_new_texture(void) {
    static uint16_t tex_num = 0; // amount of textures in cache
    const uint32_t id = tex_num++;
    if (tex_num > MAX_TEXTURES) {
        printf("gfx_soft: ran out of texture slots\n");
        abort();
    }

    tex_hdr[id].sample = tex_sample_nearest_rr;

    return id;
}

static void gfx_soft_select_texture(int tile, uint32_t texture_id) {
    cur_tex[tile] = tex_hdr + texture_id;
    cur_tmu = tile;
}

static uint32_t tex_cache_alloc(const uint32_t w, const uint32_t h) {
    const uint32_t size = w * h * 4;

    if (texcache_addr + size > texcache_size) {
        texcache_size += TEXCACHE_STEP + size;
        texcache_size = ALIGN(texcache_size, TEXCACHE_STEP);
        texcache = realloc(texcache, texcache_size);
        if (!texcache) {
            printf("gfx_soft: could not alloc %u bytes for texture cache\n", texcache_size);
            abort();
        }
    }

    uint32_t ret = texcache_addr;
    texcache_addr += size;
    return ret;
}

static void gfx_soft_upload_texture(const uint8_t *rgba32_buf, int width, int height) {
    uint32_t addr = tex_cache_alloc(width, height);
    memcpy(texcache + addr, rgba32_buf, width * height * 4);
    struct Texture *tex = cur_tex[cur_tmu];
    tex->addr = addr;
    tex->w = width;
    tex->h = height;
    tex->wrap_w = width - 1;
    tex->wrap_h = height - 1;
}

static inline int gfx_cm_to_local(uint32_t val) {
    if (val & G_TX_CLAMP) return WRAP_CLAMP;
    return (val & G_TX_MIRROR) ? WRAP_MIRROR : WRAP_REPEAT;
}

static void gfx_soft_set_sampler_parameters(int tile, bool linear_filter, uint32_t cms, uint32_t cmt) {
    static const sample_fn_t samplers[] = {
        tex_sample_nearest_rr, // 0000
        tex_sample_nearest_rc, // 0001
        tex_sample_nearest_rm, // 0010
        NULL,
        tex_sample_nearest_cr, // 0100
        tex_sample_nearest_cc, // 0101
        tex_sample_nearest_cm, // 0110
        NULL,
        tex_sample_nearest_mr, // 1000
        tex_sample_nearest_mc, // 1001
        tex_sample_nearest_mm, // 1010
    };

    cms = gfx_cm_to_local(cms) << 2;
    cmt = gfx_cm_to_local(cmt);

    cur_tex[tile]->filter = linear_filter;
    cur_tex[tile]->sample = samplers[cms | cmt];
}

static void gfx_soft_set_depth_test(bool depth_test) {
    z_test = depth_test;
}

static void gfx_soft_set_depth_mask(bool z_upd) {
    z_write = z_upd;
}

static void gfx_soft_set_zmode_decal(bool zmode_decal) {
    z_offset = zmode_decal ? INT_2_FIX(-32) : 0;
}

static void gfx_soft_set_viewport(int x, int y, int width, int height) {
    r_view.x = x;
    r_view.y = y;
    r_view.w = width;
    r_view.h = height;
    r_view.hw = INT_2_FIX(width >> 1);
    r_view.hh = INT_2_FIX(height >> 1); // might as well convert now
    r_view.cx = INT_2_FIX(x) + r_view.hw;
    r_view.cy = INT_2_FIX(y) + r_view.hh;
}

static void gfx_soft_set_scissor(int x, int y, int width, int height) {
    r_clip.x0 = x;
    r_clip.y0 = y;
    r_clip.x1 = x + width;
    r_clip.y1 = y + height;
}

static void gfx_soft_set_use_alpha(bool use_alpha) {
    do_blend = use_alpha;
}

static void gfx_soft_set_fog_color(const uint8_t *rgb) {
    fog_color.r = rgb[0];
    fog_color.g = rgb[1];
    fog_color.b = rgb[2];
    fog_color.a = 0xFF;
}

static inline void gfx_soft_pick_draw_func(void) {
    static const draw_fn_t draw_funcs[] = {
        draw_pixel,
        draw_pixel_zwrite,
        draw_pixel_blend,
        draw_pixel_blend_zwrite,
        draw_pixel_blend_edge,
        draw_pixel_blend_edge_zwrite,
    };
    draw_fn = draw_funcs[cur_shader->draw_flags | z_write];
}

static void gfx_soft_draw_triangles(fix64 buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris) {
    gfx_soft_pick_draw_func();
    const size_t num_verts = 3 * buf_vbo_num_tris;
    const size_t stride = buf_vbo_len / num_verts; //how many props per vertex
    for (size_t i = 0; i < num_verts * stride; i += 3 * stride)
        pop_triangle(buf_vbo + i, stride);
}

static void gfx_soft_fill_rect(int x0, int y0, int x1, int y1, const uint8_t *rgba) {
    // HACK: these are mainly used just to clear the screen and draw simple rects, so we ignore drawmode stuff and Z
    x0 = imax(0, x0);
    y0 = imax(0, y0);
    x1 = imin(scr_width, x1);
    y1 = imin(scr_height, y1);
    register const uint32_t color = *(uint32_t *)rgba;
    register uint32_t *base = gfx_output + y0 * scr_width + x0;
    register uint32_t *p;
    register int x, y;
    for (y = y0; y < y1; ++y, base += scr_width) {
        p = base;
        for (x = x0; x < x1; ++x, ++p)
            *p = color;
    }
}

static inline void gfx_soft_tex_rect_replace(int x0, int y0, int x1, int y1, const float u0, const float v0, const float dudx, const float dvdy) {
    register int base = y0 * scr_width + x0;
    register int idx;
    register int x, y;
    float u;
    float v = v0;
    for (y = y0; y < y1; ++y, base += scr_width, v += dvdy) {
        idx = base;
        u = u0;
        for (x = x0; x < x1; ++x, ++idx, u += dudx)
            draw_fn(idx, 0, cur_tex[0]->sample(cur_tex[0], u, v));
    }
}

static inline void gfx_soft_tex_rect_modulate(int x0, int y0, int x1, int y1, const float u0, const float v0, const float dudx, const float dvdy, const Color4 rgba) {
    register int base = y0 * scr_width + x0;
    register int idx;
    register int x, y;
    float u;
    float v = v0;
    for (y = y0; y < y1; ++y, base += scr_width, v += dvdy) {
        idx = base;
        u = u0;
        for (x = x0; x < x1; ++x, ++idx, u += dudx)
            draw_fn(idx, 0, rgba_modulate(cur_tex[0]->sample(cur_tex[0], u, v), rgba));
    }
}

static void gfx_soft_tex_rect(int x0, int y0, int x1, int y1, const float u0, const float v0, const float dudx, const float dvdy, const uint8_t *rgba) {
    x0 = imax(0, x0);
    y0 = imax(0, y0);
    x1 = imin(scr_width, x1);
    y1 = imin(scr_height, y1);
    gfx_soft_pick_draw_func();
    if (cur_shader->cc.num_inputs)
        gfx_soft_tex_rect_modulate(x0, y0, x1, y1, u0, v0, dudx, dvdy, *(Color4 *)rgba);
    else
        gfx_soft_tex_rect_replace(x0, y0, x1, y1, u0, v0, dudx, dvdy);
}

static void gfx_soft_prepare_tables(void) {
    for (int t = 0; t < 0x100; ++t) {
        for (int i = 0, sum = 0; i < 0x100; ++i, sum += t) {
            lerp_tab[t][0xFF - i] = (uint8_t)(-sum >> 8);
            lerp_tab[t][0xFF + i] = (uint8_t)( sum >> 8);
        }
    }

    for (int x = 0; x < 0x100; ++x)
        for (int y = 0; y < 0x100; ++y)
            mult_tab[x][y] = (x * y) >> 8;
}

static void gfx_soft_set_resolution(const int width, const int height) {
    if (z_buffer) free(z_buffer);
    if (gfx_output) free(gfx_output);

    scr_width = width;
    scr_height = height;
    scr_size = scr_width * scr_height;

    z_buffer = calloc(scr_width * scr_height, sizeof(int16_t));
    if (!z_buffer) {
        printf("gfx_soft: could not alloc zbuffer for %dx%d\n", scr_width, scr_height);
        abort();
    }

    gfx_output = calloc(scr_width * scr_height, sizeof(uint32_t));
    if (!gfx_output) {
        printf("gfx_soft: could not alloc color buffer for %dx%d\n", scr_width, scr_height);
        abort();
    }

    depth_clear();
}

static void gfx_soft_init(void) {
    texcache = calloc(1, TEXCACHE_STEP); // this will be realloc'd as needed
    texcache_size = TEXCACHE_STEP;
    texcache_addr = 0;
    if (!texcache) {
        printf("gfx_soft: could not alloc %u bytes for texture cache\n", TEXCACHE_STEP);
        abort();
    }

    z_test = true;
    z_write = true;
    do_blend = false;
    do_clip = false;

    gfx_soft_prepare_tables();

    gfx_soft_set_resolution(gfx_current_dimensions.width, gfx_current_dimensions.height);
}

static void gfx_soft_start_frame(void) {
    // depth_swap(); // FIXME: ztrick
    depth_clear();
}

static void gfx_soft_shutdown(void) {
    free(z_buffer);
    free(texcache);
}

static void gfx_soft_on_resize(void) {
}

static void gfx_soft_end_frame(void) {
}

static void gfx_soft_finish_render(void) {
}

struct GfxRenderingAPI gfx_soft_api = {
    gfx_soft_z_is_from_0_to_1,
    gfx_soft_unload_shader,
    gfx_soft_load_shader,
    gfx_soft_create_and_load_new_shader,
    gfx_soft_lookup_shader,
    gfx_soft_shader_get_info,
    gfx_soft_new_texture,
    gfx_soft_select_texture,
    gfx_soft_upload_texture,
    gfx_soft_set_sampler_parameters,
    gfx_soft_set_depth_test,
    gfx_soft_set_depth_mask,
    gfx_soft_set_zmode_decal,
    gfx_soft_set_viewport,
    gfx_soft_set_scissor,
    gfx_soft_set_use_alpha,
    gfx_soft_draw_triangles,
    gfx_soft_init,
    gfx_soft_on_resize,
    gfx_soft_start_frame,
    gfx_soft_end_frame,
    gfx_soft_finish_render,
    gfx_soft_fill_rect,
    gfx_soft_tex_rect,
    gfx_soft_set_fog_color,
    gfx_soft_shutdown,
};

