#ifndef GFX_PC_H
#define GFX_PC_H

#include <stdbool.h>
#include <stdint.h>

struct GfxRenderingAPI;
struct GfxWindowManagerAPI;

struct GfxDimensions {
    uint32_t width, height;
#ifdef GBI_FLOATS
    float aspect_ratio;
#else
    int32_t aspect_ratio;
#endif
};

extern struct GfxDimensions gfx_current_dimensions;

#ifdef __cplusplus
extern "C" {
#endif

void gfx_init(struct GfxWindowManagerAPI *wapi, struct GfxRenderingAPI *rapi, const char *game_name, bool start_in_fullscreen);
void gfx_shutdown(void);
struct GfxRenderingAPI *gfx_get_current_rendering_api(void);
void gfx_start_frame(void);
void gfx_run(Gfx *commands);
void gfx_end_frame(void);

#ifdef __cplusplus
}
#endif

#endif
