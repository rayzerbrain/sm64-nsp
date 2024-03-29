#include <stdlib.h>
#include <stdio.h>
#include <libndls.h>

#include "sm64.h"

#include "game/memory.h"
#include "audio/external.h"

#include "gfx/gfx_frontend.h"
#include "gfx/gfx_backend.h"
#include "gfx/gfx_nsp.h"
#include "fixed_pt.h"

#include "configfile.h"
#include "timer.h"


#define CONFIG_FILE "sm64_config.txt.tns"

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


void produce_one_frame(void) {
    gfx_start_frame();
    game_loop_one_iteration();
    gfx_end_frame();
}

static void save_config(void) {
    configfile_save(CONFIG_FILE);
}

static void on_fullscreen_changed(UNUSED bool is_now_fullscreen) {
}

int main(UNUSED int argc, char *argv[]) {
    static u64 pool[0x165000 / 8 / 4 * sizeof(void *)];
    main_pool_init(pool, pool + sizeof(pool) / sizeof(pool[0]));
    gEffectsMemoryPool = mem_pool_init(0x4000, MEMORY_POOL_LEFT);

    enable_relative_paths(argv);

    configfile_load(CONFIG_FILE);
    atexit(save_config);

    wm_api = &gfx_nsp_api;
    rendering_api = &gfx_soft_api;

    gfx_init(wm_api, rendering_api, "Super Mario 64 PC-Port", true);
    atexit(gfx_shutdown);

    thread5_game_loop(NULL);
    inited = 1;

    wm_api->main_loop(produce_one_frame);
}
