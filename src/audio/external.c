#include "external.h"

s32 gAudioErrorFlags = 0;
f32 gDefaultSoundArgs[3] = { 0.0f, 0.0f, 0.0f };

struct SPTask *create_next_audio_frame_task(void) {
    return NULL;
};
void play_sound(s32 soundBits, f32 *pos){};
void audio_signal_game_loop_tick(void){};
void sequence_player_fade_out(u8 player, u16 fadeTimer){};
void fade_volume_scale(u8 player, u8 targetScale, u16 fadeTimer){};
void func_8031FFB4(u8 player, u16 fadeTimer, u8 arg2){};
void sequence_player_unlower(u8 player, u16 fadeTimer){};
void set_sound_disabled(u8 disabled){};
void sound_init(void){};
void func_803205E8(u32 soundBits, f32 *vec){};
void func_803206F8(f32 *arg0){};
void func_80320890(void){};
void sound_banks_disable(u8 player, u16 bankMask){};
void sound_banks_enable(u8 player, u16 bankMask){};
void func_80320A4C(u8 bankIndex, u8 arg1){};
void play_dialog_sound(u8 dialogID){};
void play_music(u8 player, u16 seqArgs, u16 fadeTimer){};
void stop_background_music(u16 seqId){};
void fadeout_background_music(u16 arg0, u16 fadeOut){};
void drop_queued_background_music(void){};
u16 get_current_background_music(void) {
    return 0;
};
void play_secondary_music(u8 seqId, u8 bgMusicVolume, u8 volume, u16 fadeTimer){};
void func_80321080(u16 fadeTimer){};
void func_803210D4(u16 fadeOutTime){};
void play_course_clear(void){};
void play_peachs_jingle(void){};
void play_puzzle_jingle(void){};
void play_star_fanfare(void){};
void play_power_star_jingle(u8 arg0){};
void play_race_fanfare(void){};
void play_toads_jingle(void){};
void sound_reset(u8 presetId){};
void audio_set_sound_mode(u8 arg0){};

void audio_init(void){}; // in load.c