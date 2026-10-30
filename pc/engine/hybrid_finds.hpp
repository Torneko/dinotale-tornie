#pragma once
#include "story_progress.hpp"
#include <filesystem>
#include <optional>

namespace workshop::finds {
struct View {unsigned found=0;bool initialized=false,migrated=false,readable=true,saved=true;};
void initialize(const std::filesystem::path& path="pc-hybrid-finds.json");
void migrate(uint8_t* rdram,unsigned story);
View view();
unsigned available(unsigned eligible);
void reset_attempt();
void enter(unsigned level,unsigned room,bool allowed);
std::optional<unsigned> reserve(unsigned eligible,unsigned source,unsigned x,unsigned y,bool whitePresent);
void spawned(uint32_t actor,unsigned color,bool scene=true);
bool scene_actor(uint32_t actor);
void created(uint32_t actor);
bool egg_state(uint32_t actor,unsigned state);
bool take_pickup_sound(uint32_t actor,unsigned state);
uint32_t touching(uint8_t* rdram,uint32_t player);
unsigned pending();
bool finish(uint8_t* rdram);
void clear_save();
}
