#pragma once
#include "boss_native.hpp"
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
namespace workshop::boss_rest_heart0260 {
namespace n=boss_native;
// One logical native BIG RED HEART40C5 per campaign, available after fight5.
// This is not Lucky Tree's4035 Super Happy Heart. No health/timer/save is written.
inline constexpr uint16_t kind=0x40C5;
inline constexpr unsigned rest_room=83,required_completed=5;
inline constexpr float spawn_x=168.f,spawn_y=376.f,spawn_z=500.34f;
inline constexpr unsigned cull_extent=1024;
inline constexpr uint32_t original_factory=0x806E7240,spawn_offset=0xC30;
inline constexpr uint32_t heart_count_address=0x800F8958;
struct Frame {unsigned room=177,completed=0,vi=0;uint32_t player=0;bool rest_ready=false;};
struct SpawnRequest {uint64_t birth=0;unsigned visit=0;uint32_t player=0;float x=spawn_x,y=spawn_y,z=spawn_z;};
struct Binding {uint32_t actor=0,private_data=0,resource=0,metadata=0,definition=0,factory=0;uint64_t birth=0;};
struct Snapshot {
 bool enabled=false,consumed=false,invalidated=false,spawn_pending=false;
 unsigned room=177,visit=0,spawned_visit=0,collected_vi=0;uint64_t next_birth=0,collected_birth=0;
 Binding binding{};std::string failure;
};
inline bool pointer(uint32_t a,unsigned bytes){return !(a&3)&&n::range(a,bytes);}
class Bonus {
 Snapshot state_;SpawnRequest request_{};
 struct Contact {uint64_t birth=0;unsigned vi=0;bool accepted=false;} contact_;
 struct Pending {
  Binding identity{};uint32_t player=0,native_color=0,pc_color=0,before_count=0;unsigned vi=0;
  bool active=false,retire_requested=false,counter_observed=false,heal_requested=false;
 } pending_;
 void fail(const char* why){state_.invalidated=true;state_.failure=why;}
 static bool same_player(uint8_t* ram,uint32_t player,uint32_t native_color,uint32_t pc_color){
  return n::actor(ram,player,0x4001)&&n::word(ram,0x800F8828)==player&&n::word(ram,0x800F8DB0)==native_color&&n::word(ram,0x807F0008)==pc_color;
 }
 static bool identity(uint8_t* ram,const Binding& b){
  return ram&&b.birth&&n::actor(ram,b.actor,kind)&&n::word(ram,b.actor+0x1F4)==b.private_data&&n::word(ram,b.actor+0x168)==b.resource&&n::word(ram,b.actor+0x174)==b.metadata&&pointer(b.metadata,4)&&n::word(ram,b.metadata)==b.definition&&pointer(b.definition,0x2C)&&n::word(ram,b.definition+0x28)==b.factory+0x334;
 }
 bool collect_context(uint8_t* ram,const Frame& f)const{
  return pending_.active&&f.vi==pending_.vi&&f.player==pending_.player&&same_player(ram,pending_.player,pending_.native_color,pending_.pc_color);
 }
public:
 void configure(bool enabled){state_={};state_.enabled=enabled;request_={};contact_={};pending_={};}
 const Snapshot& snapshot()const{return state_;}
 void enter_room(unsigned room){
  if(!state_.enabled||state_.invalidated)return;
  if(room>255){fail("heart_invalid_room");return;}
  if(room==rest_room&&state_.room!=rest_room){if(state_.visit==std::numeric_limits<unsigned>::max()){fail("heart_visit_overflow");return;}++state_.visit;}
  // Room teardown alone is not collection. A real carried/attached token stays
  // bound until the native forget hook or a proved collect callback releases it.
  state_.room=room;
 }
 std::optional<SpawnRequest> request_spawn(uint8_t* ram,const Frame& f){
  if(!state_.enabled||state_.invalidated||state_.consumed||state_.spawn_pending||pending_.active||!ram||f.room!=rest_room||state_.room!=rest_room||!state_.visit||!f.rest_ready||f.completed<required_completed||f.completed>=10||!f.vi||!n::actor(ram,f.player,0x4001)||n::word(ram,0x800F8828)!=f.player)return {};
  if(state_.binding.actor||state_.spawned_visit==state_.visit)return {};
  if(state_.next_birth==std::numeric_limits<uint64_t>::max()){fail("heart_birth_overflow");return {};}
  state_.spawn_pending=true;state_.spawned_visit=state_.visit;request_={++state_.next_birth,state_.visit,f.player};return request_;
 }
 bool bind_spawn(uint8_t* ram,const Frame& f,const SpawnRequest& request,uint32_t actor){
  if(!state_.enabled||state_.invalidated||state_.consumed||!state_.spawn_pending||!ram||request.birth!=request_.birth||request.visit!=request_.visit||request.player!=request_.player||f.room!=rest_room||state_.room!=rest_room||state_.visit!=request.visit||!f.rest_ready||f.completed<required_completed||f.completed>=10||f.player!=request.player||n::word(ram,0x800F8828)!=f.player||state_.binding.actor||!n::actor(ram,actor,kind)||n::word(ram,actor+0x180)||(n::byte(ram,actor+0x17D)!=1&&n::byte(ram,actor+0x17D)!=2)){fail("heart_spawn_identity_rejected");return false;}
  // Native44194 registers a new actor as queued1. The original7E70
  // returns it before the next native update promotes it to active2.
  // Bind that exact birth now; contact/collection below still require2.
  Binding b{actor,n::word(ram,actor+0x1F4),n::word(ram,actor+0x168),n::word(ram,actor+0x174),0,0,request.birth};
  if(!pointer(b.resource,0x44)||!pointer(b.metadata,4)){fail("heart_spawn_resource_rejected");return false;}b.factory=n::word(ram,b.resource+0x40);b.definition=n::word(ram,b.metadata);
  if(!pointer(b.factory,0xF10)||!pointer(b.definition,0x2C)||n::word(ram,b.definition+4)||n::word(ram,b.definition+8)!=0x0000000F||n::word(ram,b.definition+0xC)!=0x000F000A||n::word(ram,b.definition+0x28)!=b.factory+0x334){fail("heart_spawn_collider_rejected");return false;}
  const auto phase=n::byte(ram,actor+0x18);const auto expected=phase==1?0x398u:phase==2?0x440u:phase==5?0x878u:0u;
  if(!expected||n::word(ram,actor+0x20)!=b.factory+expected){fail("heart_spawn_callback_rejected");return false;}
  const float x=std::bit_cast<float>(n::word(ram,actor+0x3C)),y=std::bit_cast<float>(n::word(ram,actor+0x40)),z=std::bit_cast<float>(n::word(ram,actor+0x44));
  if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(z)||x!=spawn_x||y!=spawn_y||z!=spawn_z){fail("heart_spawn_position_rejected");return false;}
  state_.binding=b;state_.spawn_pending=false;request_={};return true;
 }
 void spawn_failed(uint64_t birth){if(state_.spawn_pending&&request_.birth==birth)fail("heart_native_spawn_failed");}
 void forget(uint32_t actor){
  if(!state_.enabled||!actor||state_.binding.actor!=actor)return;
  // Native7728 retires the heart BEFORE increasing the counter and healing.
  // Keep that pending collect proof until those exact native effects return.
  if(!(pending_.active&&pending_.identity.actor==actor&&pending_.retire_requested))pending_={};
  state_.binding={};contact_={};
 }
 void frame(uint8_t* ram,const Frame& f){
  if(!state_.enabled||state_.invalidated||!ram)return;
  if(pending_.active&&f.vi>pending_.vi){fail("heart_collect_effect_missing");return;}
  if(state_.binding.actor&&!identity(ram,state_.binding)){state_.binding={};contact_={};}
 }
 // Returns true only once, after real contact, native collection/retirement,
 // the real heart-counter increment and the real max-health continuation.
 // Every native read below is bounded; all writes are private host bookkeeping.
 bool trace(uint8_t* ram,const recomp_context* c,std::string_view name,const Frame& f){
  if(!state_.enabled||state_.invalidated||state_.consumed||!ram||!c)return false;
  const auto a=uint32_t(c->r4),other=uint32_t(c->r5),ra=uint32_t(c->r31);
  const auto& b=state_.binding;
  if(name=="func_806E7468"&&b.actor&&a==b.actor){fail("heart_actor_reinitialized");return false;}
  if(name=="func_806E7574"&&b.actor&&a==b.actor&&identity(ram,b)){
   const auto phase=n::byte(ram,a+0x18);const auto callback=phase==1?0x398u:phase==2?0x440u:phase==5?0x878u:0u;const bool active=n::byte(ram,a+0x17D)==2&&callback&&n::word(ram,a+0x20)==b.factory+callback;
   const bool player=other==f.player&&n::actor(ram,other,0x4001)&&n::word(ram,0x800F8828)==other;
   const bool egg=n::actor(ram,other,0x4002)&&n::byte(ram,other+0x18)==0x68;
   if(active&&(player||egg))contact_={b.birth,f.vi,false};
  }
  if(name=="func_806E7E4C"&&b.actor&&a==b.actor&&other==3&&identity(ram,b)&&contact_.birth==b.birth&&contact_.vi==f.vi&&ra==b.factory+0x388)contact_.accepted=true;
  if(name=="func_806E7728"&&b.actor&&a==b.actor){
   if(pending_.active||!identity(ram,b)||n::byte(ram,a+0x17D)!=2||n::byte(ram,a+0x18)!=3||n::word(ram,a+0x20)!=b.factory+0x4E8||contact_.birth!=b.birth||!contact_.accepted||f.vi<contact_.vi||!n::actor(ram,f.player,0x4001)||n::word(ram,0x800F8828)!=f.player){fail("heart_collection_without_contact_proof");return false;}
   const auto color=n::word(ram,0x800F8DB0),pc=n::word(ram,0x807F0008),count=n::word(ram,heart_count_address);
   if(color>=8||pc>=14||count==std::numeric_limits<uint32_t>::max()){fail("heart_collect_owner_rejected");return false;}
   pending_={b,f.player,color,pc,count,f.vi,true,false,false,false};
  }
  if(name=="func_800447C8"&&pending_.active&&a==pending_.identity.actor&&ra==pending_.identity.factory+0x560&&collect_context(ram,f)&&identity(ram,pending_.identity))pending_.retire_requested=true;
  if(name=="func_80049624"&&pending_.active&&pending_.retire_requested&&ra==pending_.identity.factory+0x578&&collect_context(ram,f)&&n::word(ram,heart_count_address)==pending_.before_count+1)pending_.counter_observed=true;
  if(name=="func_8002B428"&&pending_.active&&pending_.retire_requested&&pending_.counter_observed&&a==8&&ra==pending_.identity.factory+0x580&&collect_context(ram,f)&&n::word(ram,heart_count_address)==pending_.before_count+1)pending_.heal_requested=true;
  if(name=="func_8002B3A0"&&pending_.active&&pending_.retire_requested&&pending_.counter_observed&&pending_.heal_requested&&ra==0x8002B460&&collect_context(ram,f)&&n::word(ram,heart_count_address)==pending_.before_count+1){
   state_.consumed=true;state_.collected_birth=pending_.identity.birth;state_.collected_vi=f.vi;state_.binding={};contact_={};pending_={};return true;
  }
  return false;
 }
};
} // namespace workshop::boss_rest_heart0260
