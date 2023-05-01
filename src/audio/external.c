#include "external.h"

s32 gAudioErrorFlags = 0;
f32 gDefaultSoundArgs[3] = { 0.0f, 0.0f, 0.0f };

struct SPTask *create_next_audio_frame_task(void) {
    return NULL;
};
void play_sound(UNUSED s32 soundBits, UNUSED f32 *pos){};
void audio_signal_game_loop_tick(void){};
void sequence_player_fade_out(UNUSED u8 player, UNUSED u16 fadeTimer){};
void fade_volume_scale(UNUSED u8 player, UNUSED u8 targetScale, UNUSED u16 fadeTimer){};
void func_8031FFB4(UNUSED u8 player, UNUSED u16 fadeTimer, UNUSED u8 arg2){};
void sequence_player_unlower(UNUSED u8 player, UNUSED u16 fadeTimer){};
void set_sound_disabled(UNUSED u8 disabled){};
void sound_init(void){};
void func_803205E8(UNUSED u32 soundBits, UNUSED f32 *vec){};
void func_803206F8(UNUSED f32 *arg0){};
void func_80320890(void){};
void sound_banks_disable(UNUSED u8 player, UNUSED u16 bankMask){};
void sound_banks_enable(UNUSED u8 player, UNUSED u16 bankMask){};
void func_80320A4C(UNUSED u8 bankIndex, UNUSED u8 arg1){};
void play_dialog_sound(UNUSED u8 dialogID){};
void play_music(UNUSED u8 player, UNUSED u16 seqArgs, UNUSED u16 fadeTimer){};
void stop_background_music(UNUSED u16 seqId){};
void fadeout_background_music(UNUSED u16 arg0, UNUSED u16 fadeOut){};
void drop_queued_background_music(void){};
u16 get_current_background_music(void) {
    return 0;
};
void play_secondary_music(UNUSED u8 seqId, UNUSED u8 bgMusicVolume, UNUSED u8 volume, UNUSED u16 fadeTimer){};
void func_80321080(UNUSED u16 fadeTimer){};
void func_803210D4(UNUSED u16 fadeOutTime){};
void play_course_clear(void){};
void play_peachs_jingle(void){};
void play_puzzle_jingle(void){};
void play_star_fanfare(void){};
void play_power_star_jingle(UNUSED u8 arg0){};
void play_race_fanfare(void){};
void play_toads_jingle(void){};
void sound_reset(UNUSED u8 presetId){};
void audio_set_sound_mode(UNUSED u8 arg0){};

void audio_init(void){}; // in load.c