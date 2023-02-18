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
static uint32_t frame_num = 0;

void nsp_init(UNUSED const char *game_name, UNUSED bool start_in_fullscreen) {
    //set_cpu_speed(CPU_SPEED_150MHZ);
    lcd_init(SCR_320x240_565);
}

void nsp_main_loop(void (*run_one_game_iter)(void)) {
    while (!isKeyPressed(KEY_NSPIRE_ESC)) { // insert frame timing system here
        run_one_game_iter();
    }
}

void nsp_get_dimensions(uint32_t *width, uint32_t *height) {
    *width = SCREEN_WIDTH;
    *height = SCREEN_HEIGHT;
}

bool nsp_start_frame(void) {
    frame_num++;
    
    printf("Frame %d\n", frame_num);
    
    return !(frame_num % (int)(30 / configFrameskip));
}

void nsp_swap_buffers_begin(void) {
    static uint16_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        
        uint32_t c32 = gfx_output[i]; //aaaaaaaabbbbbbbbggggggggrrrrrrrr

        //r + g + b
        buffer[i] = ((c32 & 0b11111000) << 8) | ((c32 & 0b1111110000000000) >> 5) | ((c32 >> 19) & 0b11111); //rrrrr gggggg bbbbb
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