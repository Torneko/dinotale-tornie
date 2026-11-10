#ifndef DT_RECOMP_HOOKS_H
#define DT_RECOMP_HOOKS_H

// Force-included into every file in RecompiledFuncs/ (see CMakeLists.txt):
// declarations the [[patches.hook]] text in yoshisstory.us.toml uses beyond
// what N64Recomp's recomp.h provides.

#include "recomp.h"
int workshop_menu_title(uint8_t* rdram,recomp_context* ctx);
void workshop_reward_popup_created(uint8_t* rdram,recomp_context* ctx);

// librecomp/src/overlays.cpp
void load_overlays(uint32_t rom, int32_t ram_addr, uint32_t size);
void unload_overlays(int32_t ram_addr, uint32_t size);
// Native implementations of the mod cartridge's two bootstrap DMA adapters.
void workshop_80000438(uint8_t* rdram, recomp_context* ctx);
void workshop_807F1600(uint8_t* rdram, recomp_context* ctx);
#ifdef WORKSHOP_SPECIAL_CONTENT
int workshop_special_victory(uint8_t* rdram,recomp_context* ctx);
int workshop_special_food(uint8_t* rdram,recomp_context* ctx);
void workshop_food_health(uint8_t* rdram,recomp_context* ctx);
void workshop_food_score(uint8_t* rdram,recomp_context* ctx);
void workshop_food_popup(uint8_t* rdram,recomp_context* ctx);
void workshop_enemy_kill_reward(uint8_t* rdram,recomp_context* ctx);
int workshop_split_reward_popup(uint8_t* rdram,recomp_context* ctx);
void workshop_special_lookup_note(uint32_t id);
void workshop_trace_entry(uint8_t* rdram,recomp_context* ctx,const char* name);
int workshop_use_special_hud(void);
int workshop_resource_slot(uint8_t* rdram,recomp_context* ctx,int match);
void workshop_test_level_start(uint8_t* rdram,recomp_context* ctx);
void workshop_palette_note(uint8_t* rdram,recomp_context* ctx,int kind);
void workshop_native_fault(uint8_t* rdram,recomp_context* ctx);
void workshop_mix_level(uint8_t* rdram,recomp_context* ctx);
uint32_t workshop_mix_spawn(uint8_t* rdram, recomp_context* ctx);
void workshop_mix_parent_created(uint32_t parent);
void workshop_prepare_fruit_resource(uint8_t* rdram, recomp_context* ctx);
void workshop_finish_fruit_resource(uint8_t* rdram, recomp_context* ctx);
void workshop_pause_score_lives(uint8_t* rdram, recomp_context* ctx);
void workshop_end_score_lives(uint8_t* rdram, recomp_context* ctx);
unsigned workshop_find_unlocked(uint8_t* rdram);
void workshop_find_reset(void);
int workshop_metal_trial_color(uint8_t* rdram,recomp_context* ctx);
void workshop_metal_saved_lives(uint8_t* rdram,recomp_context* ctx);
int workshop_metal_death(uint8_t* rdram,recomp_context* ctx);
void workshop_metal_remaining(uint8_t* rdram,recomp_context* ctx);
void workshop_metal_rescued(uint8_t* rdram,recomp_context* ctx);
void workshop_find_clear(void);
int workshop_story_event0320(void);
void workshop_hard_init0330(uint8_t*,recomp_context*);
void workshop_hard_new0330(uint8_t*,recomp_context*,int);
int workshop_hard_continue0330(uint8_t*,recomp_context*);
void workshop_hard_save0330(uint8_t*,recomp_context*);
void workshop_hard_death0330(uint8_t*,recomp_context*);
void workshop_hard_rescue0330(uint8_t*,recomp_context*);
int workshop_hard_capacity0330(uint8_t*,recomp_context*);
int workshop_hard_picker0330(uint8_t*,recomp_context*);
void workshop_story_world_arm0320(uint8_t* rdram,recomp_context* ctx);
void workshop_story_world_commit0320(uint8_t* rdram,recomp_context* ctx);
void workshop_story_bowser_arm0320(uint8_t* rdram,recomp_context* ctx);
void workshop_story_bowser_commit0320(uint8_t* rdram,recomp_context* ctx);
void workshop_find_egg_state(uint8_t* rdram,recomp_context* ctx);
int workshop_find_hidden(uint8_t* rdram,recomp_context* ctx,uint32_t actor);
int workshop_find_release(uint8_t* rdram,recomp_context* ctx);
void workshop_find_container_gate(uint8_t* rdram,recomp_context* ctx);
int workshop_progress_valid(uint8_t* rdram,recomp_context* ctx);
int workshop_progress_trial_access(uint8_t* rdram);
int workshop_progress_trial(uint8_t* rdram,recomp_context* ctx);
void workshop_progress_read_begin(uint8_t* rdram,recomp_context* ctx);
void workshop_progress_read_end(uint8_t* rdram,recomp_context* ctx);
void workshop_progress_boot(uint8_t* rdram,recomp_context* ctx);
void workshop_progress_reconcile(uint8_t* rdram,recomp_context* ctx);
void workshop_pause_create(uint8_t* rdram,recomp_context* ctx);
void workshop_pause_position(uint8_t* rdram,recomp_context* ctx);
int workshop_pause_frame(uint8_t* rdram,recomp_context* ctx);
void workshop_pause_hide(uint8_t* rdram,recomp_context* ctx);
#endif
#ifdef WORKSHOP_CONTENT_BUILD
recomp_func_t* workshop_lookup(int32_t address,uint8_t* rdram,recomp_context* ctx,const char* caller);
#undef LOOKUP_FUNC
#define LOOKUP_FUNC(value) workshop_lookup((int32_t)(value),rdram,ctx,__func__)
#endif

// librecomp/src/ultra_translation.cpp
void osAiSetNextBuffer_recomp(uint8_t* rdram, recomp_context* ctx);

// src/main/crash_handler.cpp
void dinotale_crash_note_rdram(uint8_t* rdram);

// lib/RecompFrontend/recompui/src/api/ui_api_events.cpp
void recomp_run_ui_callbacks(uint8_t* rdram, recomp_context* ctx);

// src/game/widescreen.cpp
int32_t dinotale_bg_view_width(uint8_t* rdram, int32_t width);
uint32_t dinotale_camera_half_width_bits(void);
void dinotale_bg_view_created(uint8_t* rdram, uint32_t view);
void dinotale_bg_cover_outside(uint8_t* rdram, uint32_t rt, uint32_t obj);
void dinotale_fruit_frame_begin(uint8_t* rdram, uint32_t frame);
void dinotale_fruit_frame_end(uint8_t* rdram);
void dinotale_hud_actor_screen_pos(uint8_t* rdram, recomp_context* ctx);
int32_t dinotale_bg_row_columns(int32_t columns);
int32_t dinotale_bg_fill_columns(uint8_t* rdram, uint32_t view, int32_t columns);
uint32_t dinotale_screen_margin_bits(uint8_t* rdram, uint32_t actor, int32_t side);
int32_t dinotale_screen_left_test(uint8_t* rdram, uint32_t actor, int32_t value);
int32_t dinotale_screen_right_test(int32_t value);
int32_t dinotale_actor_screen_x(int32_t x);
void dinotale_bg_view_freed(uint8_t* rdram, uint32_t layer);

int workshop_egg_allowed(uint8_t* rdram, unsigned color);
int workshop_hybrid_records_save(uint8_t* rdram,recomp_context* ctx);
int workshop_hybrid_active(void);
unsigned workshop_hybrid_lucky_type(void);
unsigned workshop_hybrid_lucky_object(void);
void workshop_hybrid_recolor(uint8_t* rdram,recomp_context* ctx);
void workshop_egg_enemy_color(uint8_t* rdram,recomp_context* ctx);
void workshop_egg_default_color(uint8_t* rdram,recomp_context* ctx);
int workshop_egg_bubble_gate(uint8_t* rdram,recomp_context* ctx);

#endif // DT_RECOMP_HOOKS_H
