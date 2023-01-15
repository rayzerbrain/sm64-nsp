#include <stdlib.h>
#ifdef TARGET_WEB
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "sm64.h"

#include "game/memory.h"
#include "audio/external.h"

#include "gfx/gfx_pc.h"
#include "gfx/gfx_soft.h"
#include "gfx/gfx_nsp.h"

#include "configfile.h"

#include "compat.h"

#define CONFIG_FILE "sm64config.txt"

OSMesg D_80339BEC;
OSMesgQueue gSIEventMesgQueue;

s8 gResetTimer;
s8 D_8032C648;
s8 gDebugLevelSelect;
s8 gShowProfiler;
s8 gShowDebugText;

static struct GfxWindowManagerAPI *wm_api;
static struct GfxRenderingAPI *rendering_api;

extern void gfx_run(Gfx *commands);
extern void thread5_game_loop(void *arg);
extern void create_next_audio_buffer(s16 *samples, u32 num_samples);
void game_loop_one_iteration(void);

void dispatch_audio_sptask(UNUSED struct SPTask *spTask) {
}

void set_vblank_handler(UNUSED s32 index, UNUSED struct VblankHandler *handler, UNUSED OSMesgQueue *queue, UNUSED OSMesg *msg) {
}

static uint8_t inited = 0;

#include "game/game_init.h" // for gGlobalTimer
void send_display_list(struct SPTask *spTask) {
    if (!inited) {
        return;
    }
    gfx_run((Gfx *)spTask->task.t.data_ptr);
}

#define printf

#ifdef VERSION_EU
#define SAMPLES_HIGH 656
#define SAMPLES_LOW 640
#else
#define SAMPLES_HIGH 544
#define SAMPLES_LOW 528
#endif

void produce_one_frame(void) {
    gfx_start_frame();

    game_loop_one_iteration();

    gfx_end_frame();
}

static void save_config(void) {
    configfile_save(CONFIG_FILE);
}

static void on_fullscreen_changed(bool is_now_fullscreen) {
    configFullscreen = is_now_fullscreen;
}

void game_exit(void) {
    gfx_shutdown();
    exit(0);
}

void main_func(void) {
    static u64 pool[0x165000 / 8 / 4 * sizeof(void *)];
    main_pool_init(pool, pool + sizeof(pool) / sizeof(pool[0]));
    gEffectsMemoryPool = mem_pool_init(0x4000, MEMORY_POOL_LEFT);

    configfile_load(CONFIG_FILE);
    //atexit(save_config);

    wm_api = &gfx_nsp_api;

    gfx_init(wm_api, rendering_api, "Super Mario 64 PC-Port", configFullscreen);

    audio_init();
    sound_init();

    //thread5_game_loop(NULL);
    wm_api->main_loop(produce_one_frame);
}

int main(UNUSED int argc, UNUSED char *argv[]) {
    main_func();
    //malloc(5 * sizeof(uint16_t));
    //printf("%d", sizeof(void *));
    return 0;
}
