#pragma once
#include "bonus_runtime0352.hpp"
#include "recomp.h"
#include <span>
namespace workshop::bonus_game0352 {
struct Launch {unsigned bonus=0;uint64_t seed=0;bool new_attempt=false;};
bool request_bonus(unsigned bonus,bool new_attempt);
std::optional<Launch> pending_launch();
bool launch_requested();
struct View {
 bonus_runtime0352::Access access;
 bonus_session0351::Snapshot session;
 bool full_route_available=false,private_preview=false,interrupted=false,completed=false,game_over=false;
 unsigned selected_bonus=0;
};
bool active();
void refresh(const std::filesystem::path& profile);
View view();
void configure_private_preview(const std::filesystem::path& profile,bool requested);
bool preview_requested();
void configure_private_route(const std::filesystem::path&,bool requested,unsigned bonus=1,uint64_t seed=24);
void configure_private_complete(const std::filesystem::path&,bool requested,unsigned bonus=1,uint64_t seed=24);
void configure_bonus_selection(const std::filesystem::path&,unsigned bonus,uint64_t seed,bool new_attempt);
// Must run from on_init, before original descriptors are consumed.
bool prepare_native(std::span<const uint8_t> original,uint8_t* rdram,unsigned owned_mask=63,std::vector<uint8_t>* prepared_content=nullptr);
unsigned team_available(uint8_t* rdram=nullptr);
unsigned team_owned();
void native_course_access(uint8_t*);
int team_death(uint8_t*,recomp_context*);
std::u8string save_stem();
void navigation(uint8_t*,unsigned frame,uint16_t*,float*,float*);
void frame_context(unsigned vi,const int32_t* overlays,size_t count);
void wrapped_entry(uint8_t*,const recomp_context*,const char*);
void wrapped_leave(uint8_t*,const recomp_context*,const char*);
bool shared_completion(uint8_t*,const recomp_context*);
void bowser_arm(uint8_t*,const recomp_context*);
void bowser_commit(uint8_t*,const recomp_context*);
bool paint_begin(uint8_t*,uint32_t actor,bool outer_fish);
void paint_finish(uint8_t*);
int candy_pool_enter(uint8_t*,recomp_context*);
void candy_pool_leave(uint8_t*,const recomp_context*,int ticket);
void candy_pool_cleanup(uint8_t*,const recomp_context*,const recomp_context*);
bool observes_trace(const char* name);
void trace(uint8_t*,recomp_context*,const char* name);
int departing(uint8_t*,recomp_context*);
}
extern "C" int workshop_bonus_active0352();
extern "C" void workshop_bonus_fruit_frame0352(uint8_t*);
extern "C" int workshop_bonus_depart0352(uint8_t*,recomp_context*);
