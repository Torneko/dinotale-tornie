#pragma once
#include "story_hard_access0330.hpp"
#include "recomp.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
namespace workshop::story_hard_bridge0330 {
struct Config {bool preview=false,preview_verified=false,genuine_events=false;int test_egg_color=-1;};
void configure(Config);
bool active();bool preview();std::string error();story_hard_access0330::Evidence access();
bool prepare_entry();std::u8string save_name();std::vector<uint8_t> patch_rom(std::vector<uint8_t>);
bool init_projection(uint8_t*,recomp_context*);
bool new_game(uint8_t*,recomp_context*);
bool continue_game(uint8_t*,recomp_context*);
void save_finished(uint8_t*,recomp_context*);
void clear_local();void frame(uint8_t*,recomp_context*);
std::optional<unsigned> available();unsigned unlocked_extras();
bool saved_extras(uint8_t*,recomp_context*);
bool death_enter(uint8_t*,recomp_context*);void death_finish(uint8_t*,recomp_context*);
bool rescue_enter(uint8_t*,recomp_context*);void rescue_finish(uint8_t*,recomp_context*);
bool capacity(uint8_t*,recomp_context*);bool picker(uint8_t*,recomp_context*);
void arm_world(uint8_t*,recomp_context*);void commit_world(uint8_t*,recomp_context*);
void arm_bowser(uint8_t*,recomp_context*);void commit_bowser(uint8_t*,recomp_context*);
bool end_tally(uint8_t*,recomp_context*);
void enter(uint8_t*,unsigned room);
void begin_child(recomp_context*);void prepare_release(uint8_t*,uint32_t wrapper);
void created(uint8_t*,recomp_context*);void restore(uint8_t*,uint32_t actor);
void dispose(uint8_t*,uint32_t actor);void forget(uint32_t actor);bool paint(uint8_t*,uint32_t actor);
bool egg_bubble_gate(uint8_t*,recomp_context*);bool release(uint8_t*,recomp_context*);
bool egg_state(uint8_t*,recomp_context*);
} // namespace workshop::story_hard_bridge0330
