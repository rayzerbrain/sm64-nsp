// #ifdef TARGET_NSP

#include <libndls.h>
#include <SDL/SDL.h>

#include "gfx_window_manager_api.h"
#include "gfx_soft.h"
#include "macros.h"

#include "../configfile.h"
#include "../timer.h"

/*
struct GfxWindowManagerAPI {
    bool (*start_frame)(void);
    void (*swap_buffers_begin)(void);
    void (*swap_buffers_end)(void);
};

*/
#define HALF_WIDTH SCREEN_WIDTH / 2
#define HALF_HEIGHT SCREEN_HEIGHT / 2

static uint32_t current_frame = 0;
static bool skip_frame = false;

void nsp_init(UNUSED const char *game_name, UNUSED bool start_in_fullscreen) {
    //set_cpu_speed(CPU_SPEED_150MHZ);
    lcd_init(SCR_320x240_565);
}

void nsp_main_loop(void (*run_one_game_iter)(void)) {
    timer_start();

    while (true) {
        uint32_t ideal_frame = timer_elapsed() / 1056; // 33 decrements per ms * 33 ms per frame = 2112 decrements per frame

        int new_frames = ideal_frame - current_frame;

        if (new_frames) {
            int to_skip = (new_frames > configFrameskip) ? configFrameskip : (new_frames - 1); // catch up by skipping up to configFrameskip frames
            
            printf("Ideal frame: %u\nCurrent frame: %u\nFrames to render: %u\nSkipping: %u\n\n", ideal_frame, current_frame, new_frames, to_skip);
            uint32_t t0 = timer_elapsed();

            for (int i = 0; i < new_frames; ++i, --to_skip) {
                skip_frame = to_skip > 0;
                run_one_game_iter();
                current_frame++;

                if (isKeyPressed(KEY_NSPIRE_ESC)) {
                    return;
                }
            }
            printf("tFRAME: %u\n", timer_elapsed() - t0);
        } // mostly borrowed from dos implementation
    }
}

void nsp_get_dimensions(uint32_t *width, uint32_t *height) {
    if (config120pMode) { // quarter the pixel resolution
        *width = HALF_WIDTH;
        *height = HALF_HEIGHT;
    } else {
        *width = SCREEN_WIDTH;
        *height = SCREEN_HEIGHT;
    }
}

bool nsp_start_frame(void) {
    return !skip_frame; // return if frame should be rendered
}

static inline uint16_t c4444_to_c565(uint32_t c) {
    return ((c & 0b11111000) << 8) | ((c & 0b1111110000000000) >> 5) | ((c >> 19) & 0b11111);
}

void nsp_swap_buffers_begin(void) {
    static uint16_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
     
    // populate buffer according to configuration 
    if (config120pMode) {
        //spread 160 * 120 img to fill whole screen, not just top left quarter
        int img_pix = 0;
        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i += 2 * SCREEN_WIDTH) { // skip a row
            for (int col = 0; col < SCREEN_WIDTH; col += 2, img_pix++) {
                uint16_t c16 = c4444_to_c565(gfx_output[img_pix]);
                int index = i + col;

                buffer[index] = c16;
                buffer[index + 1] = c16;
                buffer[index + SCREEN_WIDTH] = c16;
                buffer[index + SCREEN_WIDTH + 1] = c16; // expand single pixel to 2*2 square, towards bottom right
            }
        }
    } else {
        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {

            uint32_t c32 = gfx_output[i]; // aaaaaaaabbbbbbbbggggggggrrrrrrrr

            // r + g + b
            buffer[i] = c4444_to_c565(c32); // rrrrr gggggg bbbbb
        }
    }

    lcd_blit(buffer, SCR_320x240_565);
}

void nsp_swap_buffers_end(void) {
}

// unimplemented windowing features
void nsp_set_keyboard_callbacks(
    UNUSED bool (*on_key_down)(int scancode), 
    UNUSED bool (*on_key_up)(int scancode),
    UNUSED void (*on_all_keys_up)(void)) {
}
void nsp_set_fullscreen_changed_callback(UNUSED void (*on_fullscreen_changed)(bool is_now_fullscreen)) {
}
void nsp_set_fullscreen(UNUSED bool enable) {
}
void nsp_handle_events(void) {
}
double nsp_get_time(void) {
    return 0.0;
}

void nsp_shutdown(void) {
    lcd_init(SCR_TYPE_INVALID);
    //timer_shutdown();
}

struct GfxWindowManagerAPI gfx_nsp_api = { nsp_init,
                                           nsp_set_keyboard_callbacks,
                                           nsp_set_fullscreen_changed_callback,
                                           nsp_set_fullscreen,
                                           nsp_main_loop,
                                           nsp_get_dimensions,
                                           nsp_handle_events,
                                           nsp_start_frame,
                                           nsp_swap_buffers_begin,
                                           nsp_swap_buffers_end,
                                           nsp_get_time,
                                           nsp_shutdown};

// #endif