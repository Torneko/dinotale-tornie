#pragma once
#include "coop_rooms_native0280.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>

namespace workshop::coop_room_route_driver0280 {
// QA controller only. All guest access below is read-only. The Loader must
// explicitly enable this under its bounded, exact-fixture CLI guard. It emits
// normal normalized pad input, never poses, inventory, damage or room events.
namespace n=coop_native;namespace ids=coop_collision;namespace rn=coop_rooms_native0280;namespace life=coop_lifecycle0280;
enum class Mode {inactive,approach,jump,swallow,on_pot,finished};
inline constexpr const char* names[]={"inactive","approach","jump","swallow","on-pot","finished"};
struct Frame {
 unsigned vi=0,room=0,leader=2,player=2;
 bool enabled=false,bound=false,complete=false,busy=false,owner_valid=false,finished=false;
 bool pot_valid=false,parent_is_pot=false,grounded=false,mouth_full=false;
 float x=0,y=0,pot_x=6264,pot_y=184;
};
struct Output {bool handled=false;Mode mode=Mode::inactive;uint16_t buttons=0;float x=0,y=0;};
inline Output decide(const Frame& f){
 if(!f.enabled||!f.bound||!f.complete||f.busy||!f.owner_valid||f.player>1||f.leader>1)return {};
 if(f.finished&&f.room==43)return {true,Mode::finished};
 if(f.room!=81||!std::isfinite(f.x)||!std::isfinite(f.y)||f.x<6000||!f.pot_valid||!std::isfinite(f.pot_x)||!std::isfinite(f.pot_y))return {};
 if(f.mouth_full)return {true,Mode::swallow,0,0,-1};
 if(f.parent_is_pot){
  // Only the chosen leader may enter. The partner waits without horizontal
  // stick; a shared pot can carry both through its ordinary native callback.
  if(f.player==f.leader)return {true,Mode::on_pot,0x0400,0,-1};
  return {true,Mode::on_pot};
 }
 const float dx=f.pot_x-f.x;
 if(std::abs(dx)<=8){
  // QA candidate: a complete64-frame held-A pulse every240VI, without
  // cutting the jump at a guessed Y. Release to fall naturally afterwards.
  // AF8C alone still decides whether a real child link admits entry.
  if(f.player==f.leader){
   if((f.vi%240)<64)return {true,Mode::jump,0x8000,0,0};
   if(f.grounded)return {true,Mode::on_pot,0x0400,0,-1};
  }
  return {true,Mode::on_pot};
 }
 Output out{true,Mode::approach,0,dx>0?1.f:-1.f,0};
 // Near the pot use an ordinary held-A jump/flutter:32 frames in each96.
 // No guessed support is used to enter; only the real reciprocal parent above.
 if(std::abs(dx)<=96&&f.y>f.pot_y-40&&(f.vi%96)<32){out.buttons=0x8000;out.mode=Mode::jump;}
 else if((f.vi%48)<8)out.buttons=0x4000; // Ordinary B tongue near the two native Maskass.
 return out;
}
inline bool enabled=false,after_room_death=false;
inline std::array<Mode,2> last_mode{};
inline void configure(bool on,bool afterRoomDeath=false){enabled=on;after_room_death=on&&afterRoomDeath;last_mode={};}
inline bool root_observed(uint8_t* r,unsigned player){
 const auto& state=life::lifecycle.snapshot();if(player>1||!state.bound)return false;
 const auto& id=state.players[player].root;const auto now=ids::observed_identity(r,id.address);
 return now&&*now==id&&id.kind==0x4001&&n::range(id.private_data,0x1550)&&life::lifecycle.alive(r,player)&&n::byte(r,id.address+0x165)==0&&n::word(r,id.address+0x130)==0x800233B4;
}
// Separate QA pad phase after a genuinely completed second room transaction.
struct DeathFrame {unsigned vi=0,player=2;bool active=false,survivor_valid=false,victim_pending=false,victim_reborn=false,enemy_valid=false;float survivor_x=0,victim_x=0,enemy_x=0;};
inline Output decide_death(const DeathFrame& f){
 if(!f.active||f.player>1)return {};
 if(f.victim_reborn)return {true,Mode::finished};
 if(f.player==0){
  if(f.victim_pending||!f.enemy_valid||!std::isfinite(f.victim_x)||!std::isfinite(f.enemy_x))return {true,Mode::finished};
  const float dx=f.enemy_x-f.victim_x;return {true,Mode::approach,0,std::abs(dx)>24?(dx>0?1.f:-1.f):0.f,0};
 }
 if(!f.survivor_valid||!std::isfinite(f.survivor_x))return {};
 return {true,Mode::approach,uint16_t((f.vi%96)<64?0x8000:0),f.victim_pending?1.f:(f.survivor_x>15?-1.f:0.f),0};
}
inline bool after_death_scope(unsigned room){
 const auto& c=rn::coordinator;return after_room_death&&room==43&&c.epoch()==2&&c.phase()==coop_rooms0280::Phase::complete&&c.departure().room==81&&c.departure().route.destination()==43;
}
inline std::optional<float> first_death_enemy(uint8_t* r,uint32_t victim){
 const float victim_y=std::bit_cast<float>(n::word(r,victim+0x88));if(!std::isfinite(victim_y))return {};
 const auto manager=n::word(r,0x800F8810);if(!n::range(manager,0x14))return {};
 const auto capacity=ids::half(r,manager+8);const auto table=n::word(r,manager+0x10);if(!capacity||capacity>150||!n::range(table,size_t(capacity)*4))return {};
 for(unsigned i=0;i<capacity;i++){
  const auto a=n::word(r,table+i*4);if(!n::actor(r,a,0x403A)||n::byte(r,a+0x17D)!=2||n::byte(r,a+0x165)||n::word(r,a+0x180)||n::byte(r,a+0x18)==0xFF)continue;
  const auto id=ids::observed_identity(r,a);const auto resource=n::word(r,a+0x168);
  if(!id||id->kind!=0x403A||!n::range(resource,0x48)||ids::half(r,resource+0x3C)!=0x403A||!n::range(n::word(r,resource+0x40),4)||!n::range(n::word(r,resource+0x44),4))continue;
  const float x=std::bit_cast<float>(n::word(r,a+0x84)),y=std::bit_cast<float>(n::word(r,a+0x88));if(std::isfinite(x)&&std::isfinite(y)&&std::abs(y-victim_y)<=32)return x;
 }
 return {};
}
inline Output death_observation(uint8_t* r,unsigned vi,unsigned room,unsigned player){
 if(!after_death_scope(room)||player>1)return {};
 const auto& state=life::lifecycle.snapshot();const auto& victim=state.players[0];const auto& previous=rn::coordinator.departure().payload.players[0].actor;
 const auto now=ids::observed_identity(r,victim.root.address);
 const bool reborn=victim.phase==life::Phase::alive&&victim.root.generation!=previous.birth&&now&&*now==victim.root&&victim.root.address==coop_bridge::bound_primary;
 const bool pending=victim.phase==life::Phase::death_pending||victim.phase==life::Phase::retiring||victim.phase==life::Phase::waiting||(now&&*now==victim.root&&life::fatal(r,victim.root.address));
 if(!reborn&&!pending&&victim.phase!=life::Phase::alive)return {};
 DeathFrame f;f.vi=vi;f.player=player;f.active=true;f.victim_pending=pending;f.victim_reborn=reborn;
 if(player==0&&!reborn&&!pending&&root_observed(r,0)&&victim.root.address==coop_bridge::bound_primary){
  f.victim_x=std::bit_cast<float>(n::word(r,victim.root.address+0x84));const auto enemy=first_death_enemy(r,victim.root.address);f.enemy_valid=bool(enemy);if(enemy)f.enemy_x=*enemy;
 }
 if(player==1&&!reborn){
  const auto a=state.players[1].root.address;
  f.survivor_valid=root_observed(r,1)&&a==coop_bridge::bound_secondary;
  if(!f.survivor_valid)return {};f.survivor_x=std::bit_cast<float>(n::word(r,a+0x84));
 }
 return decide_death(f);
}
inline std::optional<uint32_t> return_pot(uint8_t* r){
 if(rn::current_room!=81)return {};
 const auto manager=n::word(r,0x800F8810);if(!n::range(manager,0x14))return {};
 const auto capacity=ids::half(r,manager+8);const auto table=n::word(r,manager+0x10);
 if(!capacity||capacity>150||!n::range(table,size_t(capacity)*4))return {};
 std::optional<uint32_t> result;
 for(unsigned i=0;i<capacity;i++){
  const auto a=n::word(r,table+i*4);if(!n::actor(r,a,0x4209))continue;
  const auto id=ids::observed_identity(r,a);coop_rooms0280::Route route;
  if(!id||id->kind!=0x4209||!rn::source(r,a,route)||route.destination()!=43)return {};
  const float x=std::bit_cast<float>(n::word(r,a+0x3C)),y=std::bit_cast<float>(n::word(r,a+0x40));
  const float world_y=std::bit_cast<float>(n::word(r,a+0x88));
  // AB27C/AAF8C apply ordinary native physics. The original184 is a
  // placement, not an immutable pose: GPU16 settled at raw185.2/world181.
  // Keep the unique X/resource/route identity and a bounded live Y band.
  if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(world_y)||std::abs(x-6264.f)>.5f||std::abs(y-184.f)>16.f||std::abs(world_y-184.f)>16.f)return {};
  if(result)return {};result=a;
 }
 return result;
}
inline bool input(uint8_t* r,unsigned vi,unsigned room,unsigned leader,unsigned player,uint16_t* buttons,float* x,float* y){
 if(!r||!buttons||!x||!y||player>1||leader>1)return false;
 last_mode[player]=Mode::inactive;
 const auto& state=life::lifecycle.snapshot();const auto& coord=rn::coordinator;
 if(!enabled||!coop_bridge::context||coop_bridge::invalidated||!state.enabled||!state.bound||state.failed||state.ending||state.room!=room||rn::busy()||coord.phase()!=coop_rooms0280::Phase::complete)return false;
 if(room!=81&&room!=43)return false;
 if(after_death_scope(room)){
  const auto out=death_observation(r,vi,room,player);last_mode[player]=out.mode;
  if(!out.handled)return false;*buttons=out.buttons;*x=out.x;*y=out.y;return true;
 }
 if(!root_observed(r,player))return false;
 const auto actor=state.players[player].root.address;
 if(actor!=(player?coop_bridge::bound_secondary:coop_bridge::bound_primary))return false;
 Frame f;f.vi=vi;f.room=room;f.leader=leader;f.player=player;f.enabled=f.bound=f.complete=f.owner_valid=true;
 f.finished=room==43&&coord.epoch()>=2&&coord.departure().room==81&&coord.departure().route.destination()==43;
 if(room==81){
  f.x=std::bit_cast<float>(n::word(r,actor+0x84));f.y=std::bit_cast<float>(n::word(r,actor+0x88));
  if(!std::isfinite(f.x)||f.x<6000)return false;
  const auto pot=return_pot(r);if(!pot)return false;f.pot_valid=true;
  f.pot_x=std::bit_cast<float>(n::word(r,*pot+0x84));f.pot_y=std::bit_cast<float>(n::word(r,*pot+0x88));
  const auto data=state.players[player].root.private_data;
  f.grounded=n::byte(r,data+0xF4)==0;f.mouth_full=n::word(r,data+0x164)!=0;
  const auto parent=n::word(r,actor+0x180);
  f.parent_is_pot=parent==*pot&&n::byte(r,actor+0x17D)==0&&rn::reciprocal(r,actor,*pot);
 }
 const auto out=decide(f);last_mode[player]=out.mode;
 if(!out.handled)return false;*buttons=out.buttons;*x=out.x;*y=out.y;return true;
}
// Read-only explanation for the Loader's bounded QA witness. The actual
// input caller supplies valid output pointers; this observation has no outputs
// and never invokes input(), changes last_mode or mutates guest/host state.
inline const char* rejection_reason(uint8_t* r,unsigned vi,unsigned room,unsigned leader,unsigned player){
 if(!r)return "null_ram";
 if(player>1)return "invalid_player";
 if(leader>1)return "invalid_leader";
 const auto& state=life::lifecycle.snapshot();const auto& coord=rn::coordinator;
 if(!enabled)return "disabled";
 if(!coop_bridge::context)return "no_context";
 if(coop_bridge::invalidated)return "bridge_invalidated";
 if(!state.enabled)return "lifecycle_disabled";
 if(!state.bound)return "lifecycle_unbound";
 if(state.failed)return "lifecycle_failed";
 if(state.ending)return "lifecycle_ending";
 if(state.room!=room)return "lifecycle_room";
 if(rn::busy())return "native_room_busy";
 if(coord.phase()!=coop_rooms0280::Phase::complete)return "coordinator_not_complete";
 if(room!=81&&room!=43)return "unsupported_room";
 if(after_death_scope(room))return death_observation(r,vi,room,player).handled?"handled":"after_room_death_observation";
 const auto& id=state.players[player].root;const auto now=ids::observed_identity(r,id.address);
 if(!now)return "root_identity_unknown";
 if(*now!=id)return "root_identity_changed";
 if(id.kind!=0x4001)return "root_kind";
 if(!n::range(id.private_data,0x1550))return "root_private_range";
 if(!life::lifecycle.alive(r,player))return "root_liveness";
 if(n::byte(r,id.address+0x165)!=0)return "root_hidden";
 if(n::word(r,id.address+0x130)!=0x800233B4)return "root_callback";
 if(id.address!=(player?coop_bridge::bound_secondary:coop_bridge::bound_primary))return "root_owner_binding";
 Frame f;f.vi=vi;f.room=room;f.leader=leader;f.player=player;f.enabled=f.bound=f.complete=f.owner_valid=true;
 f.finished=room==43&&coord.epoch()>=2&&coord.departure().room==81&&coord.departure().route.destination()==43;
 if(room==81){
  f.x=std::bit_cast<float>(n::word(r,id.address+0x84));f.y=std::bit_cast<float>(n::word(r,id.address+0x88));
  if(!std::isfinite(f.x))return "root_x_nonfinite";
  if(f.x<6000)return "before_approach";
  const auto pot=return_pot(r);if(!pot)return "return_pot_unavailable";f.pot_valid=true;
  f.pot_x=std::bit_cast<float>(n::word(r,*pot+0x84));f.pot_y=std::bit_cast<float>(n::word(r,*pot+0x88));
  f.grounded=n::byte(r,id.private_data+0xF4)==0;f.mouth_full=n::word(r,id.private_data+0x164)!=0;
  f.parent_is_pot=n::word(r,id.address+0x180)==*pot&&n::byte(r,id.address+0x17D)==0&&rn::reciprocal(r,id.address,*pot);
 }
 if(decide(f).handled)return "handled";
 if(room!=81)return "outside_approach_room";
 if(!std::isfinite(f.y))return "root_y_nonfinite";
 if(!std::isfinite(f.pot_x)||!std::isfinite(f.pot_y))return "pot_world_nonfinite";
 return "decision_declined";
}
} // namespace workshop::coop_room_route_driver0280
