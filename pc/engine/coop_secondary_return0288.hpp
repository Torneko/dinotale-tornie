#pragma once
#ifdef COOP_SECONDARY_RETURN0288_PURE
#ifndef COOP_ROOM_REENTRY0287_PURE
#define COOP_ROOM_REENTRY0287_PURE 1
#define COOP_SECONDARY_RETURN0288_LOCAL_PURE 1
#endif
#endif
#include "coop_room_reentry0287.hpp"
#ifdef COOP_SECONDARY_RETURN0288_LOCAL_PURE
#undef COOP_ROOM_REENTRY0287_PURE
#undef COOP_SECONDARY_RETURN0288_LOCAL_PURE
#endif
namespace workshop::coop_secondary_return0288 {
namespace r=coop_room_reentry0287;namespace q=coop_room_return0286;
using Root=r::Root;using Player=r::Player;using Output=r::Output;using Phase=r::Phase;
inline const char* phase_name(Phase p){return r::phase_name(p);}
struct Frame:r::Frame {std::array<bool,2> grounded{};std::array<float,2> vx{};bool primary_a_held=false,secondary_a_held=false,secondary_pad_observed=false;};
struct Witness:r::Witness {const char* rejection_reason="none";unsigned escort_stage=0;bool survivor_parked=false,lower_route=false;Witness()=default;Witness(const r::Witness& other):r::Witness(other){}};
inline bool first_room(const Frame& f){return r::first_room(f);}
inline bool second_room(const Frame& f){return r::second_room(f);}
inline bool session(const Frame& f){return r::session(f);}
// New opt-in QA only. The lower route is EXPERIMENTAL until nativeY at4600
// confirms it: absence ofA never constitutes proof of walking under a ramp.
class Controller final {
 enum class Escort:unsigned {approach,release,jump,travel,parked};
 r::EntryController entry_;Escort escort_=Escort::approach;unsigned escort_vi_=0;
 bool started_=false,rejected_=false,lower_=false,parked_=false;const char* reason_="none";
 unsigned scope_vi_=0,death_vi_=0,return_vi_=0,arrival_vi_=0,last_vi_=0;uint64_t epoch_=0;
 std::array<Root,2> initial_{},returned_{};std::array<float,2> directions_{1,1};Witness last_{};
 bool same_session(const Frame& f)const{return session(f)&&f.lifecycle_epoch==epoch_&&(first_room(f)?f.original==initial_:return_vi_&&f.original==returned_);}
 bool transit_identity(const Frame& f,unsigned p,const Root& expected)const{
  if(!f.phase_alive[p]||!r::identity_present(f.players[p],expected))return false;
  if(f.players[p].alive)return true;
  return second_room(f)?f.original==returned_&&bool(f.carried_mask&(1u<<p)):first_room(f)&&f.admission_valid&&f.admission_roots==returned_;
 }
 bool survivor(const Frame& f)const{return f.busy?transit_identity(f,0,initial_[0]):q::live(f.players[0])&&f.players[0].root==initial_[0];}
 bool fresh_pair(const Frame& f)const{return return_vi_&&survivor(f)&&(f.busy?transit_identity(f,1,returned_[1]):q::live(f.players[1])&&f.players[1].root==returned_[1])&&f.players[1].death_vi==death_vi_&&f.players[1].respawn_vi==return_vi_;}
 static Phase arrival_phase(unsigned t){return t<180?Phase::settle:t<420?Phase::p1_probe:t<660?Phase::gap:t<900?Phase::p2_probe:t<1020?Phase::final_settle:Phase::complete;}
 float patrol(unsigned p,float x){if(x<=15)directions_[p]=1;else if(x>=90)directions_[p]=-1;return directions_[p];}
 void reject(const char* reason){rejected_=true;reason_=reason;}
 Witness base(const Frame& f)const{Witness w;w.vi=f.vi;w.scope_vi=scope_vi_;w.death_vi=death_vi_;w.return_vi=return_vi_;w.arrival_vi=arrival_vi_;w.lifecycle_epoch=epoch_;w.initial=initial_;w.returned=returned_;w.escort_stage=parked_&&escort_==Escort::travel?4u:unsigned(escort_);w.survivor_parked=parked_;w.lower_route=lower_;for(unsigned p=0;p<2;p++)w.roots[p]=f.players[p].root;return w;}
 // Genuine9a9 observation: walking climbs the ramp, while the earlier
 // grounded4220/A64 jump passed below it. Exchange these ordinary strategies;
 // the new physical-owner route remains experimental until realY proves it.
 Output escort(const Frame& f){
  const auto& p=f.players[0];const float vx=f.vx[0];if(!q::finite(vx)){reject("primary_velocity_invalid");return {};}
  if(parked_)return {};
  const float dx=4820-p.x;
  if(std::abs(dx)<=6&&p.y<=200&&f.grounded[0]&&std::abs(vx)<=.25f){parked_=true;return {};}
  if(p.x>=4800&&p.y>270){reject("primary_failed_upper_route");return {};}
  if(std::abs(dx)>24)return {0,dx>0?1.f:-1.f,0};
  if(std::abs(dx)>6)return {0,dx>0?.55f:-.55f,0};
  return {0,std::abs(vx)>.25f?(vx>0?-.55f:.55f):0,0};
 }
 Output victim_seek(const Frame& f){
  const auto& p=f.players[1];const float vx=f.vx[1];
  if(!f.secondary_pad_observed){reject("secondary_pad_not_observed");return {};}
  if(!q::finite(vx)){reject("secondary_velocity_invalid");return {};}
  if(escort_==Escort::approach){
   const float dx=4220-p.x;
   if(std::abs(dx)<=10&&std::abs(p.y-337)<=1&&f.grounded[1]&&std::abs(vx)<=.25f&&!f.secondary_a_held){escort_=Escort::release;escort_vi_=f.vi;return {};}
   if(std::abs(dx)>24)return {0,dx>0?1.f:-1.f,0};
   if(std::abs(dx)>10)return {0,dx>0?.55f:-.55f,0};
   return {0,std::abs(vx)>.25f?(vx>0?-.55f:.55f):0,0};
  }
  if(escort_==Escort::release){
   if(!f.grounded[1]||std::abs(p.y-337)>1||std::abs(p.x-4220)>10||std::abs(vx)>.25f||f.secondary_a_held){escort_=Escort::approach;return {};}
   if(f.vi-escort_vi_<4)return {};
   escort_=Escort::jump;escort_vi_=f.vi;return {0x8000,1,0};
  }
  if(escort_==Escort::jump){if(f.vi-escort_vi_<64)return {0x8000,1,0};escort_=Escort::travel;}
  if(!parked_&&p.x>=4576){if(std::abs(p.x-4592)>6)return {0,p.x<4592?.55f:-.55f,0};return {0,std::abs(vx)>.25f?(vx>0?-.55f:.55f):0,0};}
  if(p.x>=4600&&!lower_){if(p.y<300){reject("secondary_on_upper_route");return {};}lower_=true;}
  if(f.enemy_x&&q::finite(*f.enemy_x)){const float dx=*f.enemy_x-p.x;return {0,std::abs(dx)>2?(dx>0?1.f:-1.f):0,0};}
  return {0,p.x<4912?1.f:0,0};
 }
public:
 void reset(){*this=Controller{};}
 Witness inspect(const Frame& f)const{
  Witness w=base(f);if(!started_)return entry_.inspect(f);
  w.rejected=rejected_||!same_session(f)||!survivor(f)||f.vi<scope_vi_;
  if(return_vi_&&(f.vi<return_vi_||!fresh_pair(f)))w.rejected=true;
  if(arrival_vi_&&(f.vi<arrival_vi_||!second_room(f)||f.room!=43||!f.room_complete||f.busy||f.lifecycle_room!=43))w.rejected=true;
  if(w.rejected){w.phase=Phase::rejected;w.rejection_reason=rejected_?reason_:"identity_or_session";return w;}w.handled=true;
  if(arrival_vi_){w.elapsed=f.vi-arrival_vi_;w.phase=arrival_phase(w.elapsed);w.complete=w.phase==Phase::complete;}
  else if(return_vi_){w.elapsed=f.vi-return_vi_;w.phase=f.busy||second_room(f)?Phase::transport:w.elapsed<180?Phase::return_settle:Phase::reenter;}
  else w.phase=death_vi_||f.players[1].native_fatal?Phase::await_return:!parked_?Phase::approach_hazard:f.enemy_x?Phase::seek:Phase::approach_hazard;
  if(last_vi_==f.vi){w.inputs=last_.inputs;w.control=last_.control;w.input_vi=last_vi_;w.inputs_current=true;w.delegated=last_.delegated;}
  return w;
 }
 Witness step(const Frame& f){
  if(!started_&&f.room_epoch==0)return entry_.step(f);
  if(started_&&f.vi==last_vi_)return inspect(f);
  if(!started_){
   if(!session(f)||!first_room(f)||!f.room_complete||f.busy||f.players[1].x<3800||f.players[1].x>4096||!q::live(f.players[0])||!q::live(f.players[1])||f.players[0].root!=f.original[0]||f.players[1].root!=f.original[1])return {};
   started_=true;scope_vi_=f.vi;epoch_=f.lifecycle_epoch;initial_=f.original;
  }
  if(f.vi<last_vi_||!same_session(f)||!survivor(f)){reject("identity_or_session");return inspect(f);}
  const auto& victim=f.players[1];
  if(!return_vi_){
   if(!first_room(f)||f.busy)reject("victim_room_or_transport");
   if(victim.pending&&victim.death_vi){if(!parked_||!lower_)reject("secondary_death_before_safe_escort");else if(victim.root!=initial_[1]||victim.death_vi<scope_vi_||victim.death_vi>f.vi||(death_vi_&&victim.death_vi!=death_vi_))reject("secondary_death_identity");else death_vi_=victim.death_vi;}
   if(death_vi_&&q::live(victim)&&victim.root==initial_[1])reject("retired_secondary_live_again");
   if(q::live(victim)&&victim.root!=initial_[1]){
    if(!death_vi_||victim.root.birth==initial_[1].birth||victim.root.color!=initial_[1].color||victim.death_vi!=death_vi_||victim.respawn_vi>f.vi||victim.respawn_vi<death_vi_||victim.respawn_vi-death_vi_<300)reject("secondary_return_identity_or_delay");
    else {return_vi_=victim.respawn_vi;returned_={initial_[0],victim.root};}
   }else if(victim.root!=initial_[1]||(!victim.pending&&!victim.native_fatal&&!q::live(victim)))reject("secondary_unobserved");
  }
  if(return_vi_&&!arrival_vi_&&second_room(f)&&f.room==43&&f.lifecycle_room==43&&f.room_complete&&!f.busy&&fresh_pair(f))arrival_vi_=f.vi;
  Witness w=inspect(f);last_vi_=f.vi;w.input_vi=f.vi;w.inputs_current=w.handled;
  if(w.handled&&!w.complete){
   if(!return_vi_){
    w.control={true,true};
    if(!death_vi_&&!victim.native_fatal&&q::live(victim)){w.inputs[0]=escort(f);w.inputs[1]=victim_seek(f);}
   }else if(!arrival_vi_){
    if(w.phase==Phase::return_settle)w.control={true,true};
    else if(w.phase==Phase::reenter){for(unsigned p=0;p<2;p++){if(f.players[p].x<6000){w.control[p]=true;w.inputs[p]={uint16_t((f.vi%96)<64?0x8000:0),1,0};}else w.delegated=true;}}
    else w.delegated=true;
   }else {
    w.control={true,true};
    if(w.phase==Phase::p1_probe||w.phase==Phase::p2_probe){const unsigned p=w.phase==Phase::p1_probe?0:1;if(p==0&&w.elapsed>=330)w.inputs[p]={0,-1,0};else w.inputs[p]={uint16_t((f.vi%96)<64?0x8000:0),patrol(p,f.players[p].x),0};}
   }
  }else if(w.complete)w.control={true,true};
  if(rejected_){w=inspect(f);w.handled=false;w.inputs={};w.control={};}w.escort_stage=parked_&&escort_==Escort::travel?4u:unsigned(escort_);w.survivor_parked=parked_;w.lower_route=lower_;last_=w;return w;
 }
};
#ifndef COOP_SECONDARY_RETURN0288_PURE
namespace n=coop_native;namespace ids=coop_collision;namespace rn=coop_rooms_native0280;namespace life=coop_lifecycle0280;namespace bridge=coop_bridge;namespace old=coop_room_route_driver0280;
inline bool enabled=false;inline Controller controller;
inline void configure(bool on){enabled=on;controller.reset();}
inline Frame observe(uint8_t* r,unsigned vi,unsigned room){
 Frame f;f.vi=vi;f.room=room;f.enabled=enabled;if(!r||!enabled)return f;
 const auto& state=life::lifecycle.snapshot();const auto& c=rn::coordinator;const auto& d=c.departure();
 f.from=d.room;f.to=d.route.destination();f.leader=d.leader;f.room_epoch=c.epoch();f.lifecycle_epoch=state.epoch;f.lifecycle_room=state.room;f.room_complete=c.phase()==coop_rooms0280::Phase::complete;f.carried_mask=c.carried();
 f.bound=state.enabled&&state.bound&&!state.failed&&!state.ending;f.bridge_valid=bool(bridge::context)&&!bridge::invalidated&&!n::constructing&&!n::cleaning;f.busy=rn::busy();
 for(unsigned p=0;p<2;p++){
  f.original[p]=q::root(d.payload.players[p].actor,d.payload.players[p].color);const auto& source=state.players[p];auto& target=f.players[p];const auto& id=source.root;
  f.phase_alive[p]=source.phase==life::Phase::alive;target.root={id.address,id.private_data,id.generation,source.color,id.kind};target.death_vi=source.death_vi;target.respawn_vi=source.respawn_vi;target.pending=source.phase==life::Phase::death_pending||source.phase==life::Phase::retiring||source.phase==life::Phase::waiting;
  const auto now=ids::observed_identity(r,id.address);target.observed=now&&*now==id;
  if(target.observed){target.native_fatal=life::fatal(r,id.address);target.alive=life::lifecycle.alive(r,p);target.controllable=old::root_observed(r,p)&&id.address==(p?bridge::bound_secondary:bridge::bound_primary);target.x=std::bit_cast<float>(n::word(r,id.address+0x84));target.y=std::bit_cast<float>(n::word(r,id.address+0x88));}
 }
 if(rn::admission){
  const auto& a=*rn::admission;const auto pot=rn::observed(r,a.pot);coop_rooms0280::Route route;
  if(a.owner==0&&a.life_epoch==state.epoch&&a.room==81&&room==81&&a.source.kind==0x4209&&a.source.address==a.pot&&pot&&*pot==a.source&&rn::source(r,a.pot,route)&&route==a.route&&route.destination()==43){f.admission_valid=true;for(unsigned p=0;p<2;p++)f.admission_roots[p]=q::root(a.payload.players[p].actor,a.payload.players[p].color);}
 }
 if(f.room_epoch==0&&room==43&&state.room==43&&f.bound&&f.bridge_valid){
  const auto manager=n::word(r,0x800F8810);if(n::range(manager,0x14)){
   const auto capacity=ids::half(r,manager+8);const auto table=n::word(r,manager+0x10);
   if(capacity&&capacity<=150&&n::range(table,size_t(capacity)*4))for(unsigned slot=0;slot<capacity;slot++){
    const auto a=n::word(r,table+slot*4);if(!n::actor(r,a,0x4208))continue;const auto id=rn::observed(r,a);coop_rooms0280::Route route;
    if(!id||id->kind!=0x4208||!rn::source(r,a,route)||route.destination()!=81)continue;
    const float x=std::bit_cast<float>(n::word(r,a+0x84)),y=std::bit_cast<float>(n::word(r,a+0x88));if(!q::finite(x)||!q::finite(y)||std::abs(x-6536)>.5f||std::abs(y-296)>16)continue;
    if(f.entry_pot_valid){f.entry_pot_valid=false;break;}f.entry_pot_valid=true;f.entry_pot=q::root(*id,14);f.entry_pot_x=x;f.entry_pot_y=y;
   }
  }
  const auto& player=state.players[0];const auto& id=player.root;
  if(f.entry_pot_valid&&f.players[0].observed&&q::valid_root(f.players[0].root)){
   f.entry_parent=n::word(r,id.address+0x180)==f.entry_pot.address&&n::byte(r,id.address+0x17D)==0&&rn::reciprocal(r,id.address,f.entry_pot.address);
   f.entry_grounded=n::byte(r,id.private_data+0xF4)==0;f.entry_vx=std::bit_cast<float>(n::word(r,id.address+0xC4));f.entry_a_held=n::byte(r,0x800F8995)!=0;
  }
  if(rn::admission){const auto& a=*rn::admission;const auto pot=rn::observed(r,a.pot);coop_rooms0280::Route route;
   if(a.owner==0&&a.life_epoch==state.epoch&&a.room==43&&a.source.kind==0x4208&&a.source.address==a.pot&&pot&&*pot==a.source&&rn::source(r,a.pot,route)&&route==a.route&&route.destination()==81){f.entry_admission_valid=true;for(unsigned p=0;p<2;p++)f.entry_admission_roots[p]=q::root(a.payload.players[p].actor,a.payload.players[p].color);}
  }
 }
 if(first_room(f)&&q::live(f.players[0]))f.enemy_x=coop_room_reentry0287::plateau_enemy_x(r);
 for(unsigned p=0;p<2;p++)if(f.players[p].observed&&q::valid_root(f.players[p].root)){
  f.grounded[p]=n::byte(r,f.players[p].root.private_data+0xF4)==0;
  f.vx[p]=std::bit_cast<float>(n::word(r,f.players[p].root.address+0xC4));
 }
 f.primary_a_held=n::byte(r,0x800F8995)!=0;
 // The P2 processed-A cache is captured by the actual native input scope.
 // Outside that scope the global A flags belong to P1 and cannot gate P2.
 f.secondary_pad_observed=coop_native_input::secondary_initialized;
 if(f.secondary_pad_observed)f.secondary_a_held=coop_native_input::active_owner==1?n::byte(r,0x800F8995)!=0:coop_native_input::states[1].buttons[0]!=0;
 return f;
}
inline bool input(uint8_t* ram,unsigned vi,unsigned room,unsigned player,uint16_t* buttons,float* x,float* y){if(player>1||!buttons||!x||!y)return false;const auto w=controller.step(observe(ram,vi,room));if(!w.handled||!w.control[player])return false;*buttons=w.inputs[player].buttons;*x=w.inputs[player].x;*y=w.inputs[player].y;return true;}
inline Witness witness(uint8_t* ram,unsigned vi,unsigned room){return controller.inspect(observe(ram,vi,room));}
#endif
}
