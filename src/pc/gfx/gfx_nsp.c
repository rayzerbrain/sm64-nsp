// #ifdef TARGET_NSP

#include <libndls.h>
#include <SDL/SDL.h>

#include "gfx_window_manager_api.h"
#include "gfx_soft.h"
#include "macros.h"

#include "../configfile.h"

/*
struct GfxWindowManagerAPI {
    bool (*start_frame)(void);
    void (*swap_buffers_begin)(void);
    void (*swap_buffers_end)(void);
};

*/
#define HALF_WIDTH SCREEN_WIDTH / 2
#define HALF_HEIGHT SCREEN_HEIGHT / 2

static uint32_t actual_frame = 0;
static uint32_t ideal_frame = 1;
static bool skip_frame = false;

static _SDL_TimerID timer_id;

uint32_t timer_callback(uint32_t ms, void *ideal_frame_num) {
    *ideal_frame_num++;
}

void nsp_init(UNUSED const char *game_name, UNUSED bool start_in_fullscreen) {
    //set_cpu_speed(CPU_SPEED_150MHZ);
    lcd_init(SCR_320x240_565);

    if (SDL_Init(SDL_INIT_TIMER)) {
        printf("Error initializing sdl timer.\n");
        abort();
    }
    timer_id = SDL_AddTimer(33, timer_callback, (void *) &ideal_frame_num); // runs ~30 times every second, increments ideal frame.
}

void nsp_main_loop(void (*run_one_game_iter)(void)) {
    while (!isKeyPressed(KEY_NSPIRE_ESC)) {
        int new_frames = ideal_frame - actual_frame;

        if (new_frames) {
            int to_skip = (new_frames > configFrameskip) ? configFrameskip : (new_frames - 1); // catch up by skipping up to configFrameskip frames
            
            for (int i = 0; i < new_frames; ++f, --to_skip) {
                skip_frame = to_skip > 0;
                run_one_game_iter();
                actual_frame++;
            }
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
    printf("Frame %u\nIdeal frame: %u\nSkipping? %d\n", actual_frame, ideal_frame, skip_frame);
    
    return !skip_frame; // return if frame should be rendered
}

static inline c4444_to_c565(uint32_t c) {
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
                buffer[index + SCREEN_WIDTH + 1] = c16;
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
    UNUSED bool (*on_key_down)(int scancode), bool (*on_key_up)(int scancode),
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
    SDL_RemoveTimer(timer_id);
    SDL_Quit();
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