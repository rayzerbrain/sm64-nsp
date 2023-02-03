// #ifdef TARGET_NSP

#include <libndls.h>

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

uint16_t *buffer;
static uint32_t frames = 0;

void nsp_init(UNUSED const char *game_name, UNUSED bool start_in_fullscreen) {
    //set_cpu_speed(CPU_SPEED_150MHZ);
    lcd_init(SCR_320x240_565);

    buffer = malloc(SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint16_t));
}

void nsp_main_loop(void (*run_one_game_iter)(void)) {
    while (!isKeyPressed(KEY_NSPIRE_ESC)) {
        run_one_game_iter();
    }
}

void nsp_get_dimensions(uint32_t *width, uint32_t *height) {
    *width = SCREEN_WIDTH;
    *height = SCREEN_HEIGHT;
}

bool nsp_start_frame(void) {
    frames++;
    
    printf("Frame %d\n", frames);
    
    return !(frames % (int)(30 / configFrameskip));
}

void nsp_swap_buffers_begin(void) {
    int screen_size = SCREEN_WIDTH * SCREEN_HEIGHT;

    int full_count = 0;
    int empty_count = 0;

    for (int i = 0; i < screen_size; i++) {
        
        uint32_t c32 = gfx_output[i]; //aaaaaaaabbbbbbbbggggggggrrrrrrrr

        uint8_t a = (c32 & 0xff000000) >> 24;
        if (a == 0)
            empty_count++;
        else if (a == 0xff)
            full_count++;

        //r + g + b
        buffer[i] = (((c32 >> 3) & 0b11111) << 11) + (((c32 >> 10) & 0b111111) << 5) + ((c32 >> 19) & 0b11111); //rrrrr gggggg bbbbb
        //buffer[i] = ((uint32_t) ((uint8_t) c32 / 16.f) << 11) + ((uint32_t) ((uint8_t) (c32 >> 8) / 8.f) << 5) + ((uint32_t) ((uint8_t) (c32 >> 16) / 16.f));
    }

    printf("frame %d: full: %d. empty: %d.\n", frames, full_count, empty_count);

    lcd_blit(buffer, SCR_320x240_565);
}

void nsp_swap_buffers_end(void) {
}

// unimplemented windowing features
void nsp_set_keyboard_callbacks(
    bool (*on_key_down)(int scancode), bool (*on_key_up)(int scancode),
    void (*on_all_keys_up)(void)) {
}
void nsp_set_fullscreen_changed_callback(void (*on_fullscreen_changed)(bool is_now_fullscreen)) {
}
void nsp_set_fullscreen(bool enable) {
}
void nsp_handle_events(void) {
}
double nsp_get_time(void) {
    return 0.0;
}

void nsp_shutdown(void) {
    lcd_init(SCR_TYPE_INVALID);

    free(buffer);
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