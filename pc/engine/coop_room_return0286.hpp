#pragma once
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <optional>
#ifndef COOP_ROOM_RETURN0286_PURE
#include "coop_room_route_driver0280.hpp"
#endif

namespace workshop::coop_room_return0286 {
// New opt-in QA pad controller. Native room/lifecycle state is observed only.
// No position, damage, health, inventory, actor or room state is written here.
enum class Phase {inactive,seek,await_return,settle,p1_probe,gap,p2_probe,final_settle,complete,rejected};
inline const char* phase_name(Phase p){
 switch(p){case Phase::inactive:return "inactive";case Phase::seek:return "seek";case Phase::await_return:return "await-return";case Phase::settle:return "settle";case Phase::p1_probe:return "p1-probe";case Phase::gap:return "gap";case Phase::p2_probe:return "p2-probe";case Phase::final_settle:return "final-settle";case Phase::complete:return "complete";case Phase::rejected:return "rejected";}return "rejected";
}
struct Root {uint32_t address=0,private_data=0;uint64_t birth=0;unsigned color=14,kind=0;bool operator==(const Root&)const=default;};
struct Player {Root root{};bool observed=false,alive=false,controllable=false,native_fatal=false,pending=false;unsigned death_vi=0,respawn_vi=0;float x=0,y=0;};
struct Frame {
 unsigned vi=0,room=0,from=0,to=0,leader=2;uint64_t room_epoch=0,lifecycle_epoch=0;
 bool enabled=false,bound=false,bridge_valid=false,busy=false,room_complete=false;
 std::array<Root,2> original{};std::array<Player,2> players{};std::optional<float> enemy_x;
};
struct Output {uint16_t buttons=0;float x=0,y=0;bool operator==(const Output&)const=default;};
struct Witness {
 bool handled=false,complete=false,rejected=false,inputs_current=false;
 Phase phase=Phase::inactive;unsigned vi=0,scope_vi=0,death_vi=0,return_vi=0,elapsed=0,input_vi=0;
 uint64_t lifecycle_epoch=0,original_birth=0;std::array<Root,2> roots{};std::array<Output,2> inputs{};
};
inline bool finite(float value){return std::isfinite(value);}
inline bool valid_root(const Root& r){return r.kind==0x4001&&r.birth&&r.color<14&&r.address>=0x80000400u&&r.address<=0x807FFE04u&&!(r.address&3)&&r.private_data>=0x80000400u&&r.private_data<=0x807FEAB0u&&!(r.private_data&3);}
inline bool scope(const Frame& f){return f.enabled&&f.bound&&f.bridge_valid&&!f.busy&&f.room_complete&&f.room==43&&f.from==81&&f.to==43&&f.leader==0&&f.room_epoch==2&&f.lifecycle_epoch&&valid_root(f.original[0])&&valid_root(f.original[1])&&f.original[0].address!=f.original[1].address;}
inline bool live(const Player& p){return p.observed&&p.alive&&p.controllable&&!p.native_fatal&&valid_root(p.root)&&finite(p.x)&&finite(p.y);}
class Controller final {
 bool started_=false,rejected_=false;unsigned scope_vi_=0,death_vi_=0,return_vi_=0,last_vi_=0,dwell_until_=0;uint64_t epoch_=0;
 std::array<Root,2> original_{},returned_{};std::array<float,2> direction_{1,1};Witness last_{};
 bool same_scope(const Frame& f)const{return scope(f)&&f.lifecycle_epoch==epoch_&&f.original==original_;}
 bool survivor(const Frame& f)const{return live(f.players[1])&&f.players[1].root==original_[1];}
 bool returned_pair(const Frame& f)const{return return_vi_&&survivor(f)&&live(f.players[0])&&f.players[0].root==returned_[0]&&f.players[0].death_vi==death_vi_&&f.players[0].respawn_vi==return_vi_;}
 static Phase phase(unsigned elapsed){return elapsed<180?Phase::settle:elapsed<420?Phase::p1_probe:elapsed<660?Phase::gap:elapsed<900?Phase::p2_probe:elapsed<1020?Phase::final_settle:Phase::complete;}
 float patrol(unsigned p,float x,float low,float high){if(x<=low)direction_[p]=1;else if(x>=high)direction_[p]=-1;return direction_[p];}
 Output pending_patrol(unsigned vi,float x){
  if(vi<dwell_until_)return {};
  if((x<=15&&direction_[1]<0)||(x>=80&&direction_[1]>0)){direction_[1]=x<=15?1.f:-1.f;dwell_until_=vi>UINT32_MAX-48?UINT32_MAX:vi+48;return {};}
  return {uint16_t((vi%96)<64?0x8000:0),direction_[1],0};
 }
 Witness base(const Frame& f)const{Witness w;w.vi=f.vi;w.scope_vi=scope_vi_;w.death_vi=death_vi_;w.return_vi=return_vi_;w.lifecycle_epoch=epoch_;w.original_birth=original_[0].birth;for(unsigned p=0;p<2;p++)w.roots[p]=f.players[p].root;return w;}
public:
 void reset(){*this=Controller{};}
 Witness inspect(const Frame& f)const{
  Witness w=base(f);if(!started_)return w;
  w.rejected=rejected_||!same_scope(f)||!survivor(f)||f.vi<scope_vi_;
  if(return_vi_&&(f.vi<return_vi_||!returned_pair(f)))w.rejected=true;
  if(w.rejected){w.phase=Phase::rejected;return w;}
  w.handled=true;
  if(return_vi_){w.elapsed=f.vi-return_vi_;w.phase=phase(w.elapsed);w.complete=w.phase==Phase::complete;}
  else w.phase=death_vi_||f.players[0].native_fatal?Phase::await_return:Phase::seek;
  if(last_vi_==f.vi){w.inputs=last_.inputs;w.input_vi=last_vi_;w.inputs_current=true;}
  return w;
 }
 Witness step(const Frame& f){
  if(started_&&f.vi==last_vi_)return inspect(f);
  if(!started_){
   if(!scope(f)||!live(f.players[0])||!live(f.players[1])||f.players[0].root!=f.original[0]||f.players[1].root!=f.original[1])return {};
   started_=true;scope_vi_=f.vi;epoch_=f.lifecycle_epoch;original_=f.original;
  }
  if(f.vi<last_vi_||!same_scope(f)||!survivor(f)){rejected_=true;return inspect(f);}
  const auto& victim=f.players[0];
  if(!return_vi_){
   if(victim.pending&&victim.death_vi){
    if(victim.root!=original_[0]||victim.death_vi<scope_vi_||victim.death_vi>f.vi||(death_vi_&&victim.death_vi!=death_vi_))rejected_=true;
    else death_vi_=victim.death_vi;
   }
   if(live(victim)&&victim.root!=original_[0]){
    if(!death_vi_||victim.root.birth==original_[0].birth||victim.root.color!=original_[0].color||victim.death_vi!=death_vi_||victim.respawn_vi>f.vi||victim.respawn_vi<death_vi_||victim.respawn_vi-death_vi_<300)rejected_=true;
    else {return_vi_=victim.respawn_vi;returned_={victim.root,original_[1]};}
   }else if(victim.root!=original_[0]||(!victim.pending&&!victim.native_fatal&&!live(victim)))rejected_=true;
  }
  Witness w=inspect(f);last_vi_=f.vi;w.input_vi=f.vi;w.inputs_current=w.handled;
  if(w.handled&&!w.complete){
   if(!return_vi_){
    if(!death_vi_&&!victim.native_fatal&&live(victim)&&f.enemy_x&&finite(*f.enemy_x)){
     const float dx=*f.enemy_x-victim.x;w.inputs[0].x=std::abs(dx)>2?(dx>0?1.f:-1.f):0.f;
    }
    if(death_vi_)w.inputs[1]=pending_patrol(f.vi,f.players[1].x);
    else w.inputs[1]={0,f.players[1].x>15?-1.f:0.f,0};
   }else if(w.phase==Phase::p1_probe||w.phase==Phase::p2_probe){
    const unsigned p=w.phase==Phase::p1_probe?0:1;w.inputs[p].buttons=(f.vi%96)<64?0x8000:0;w.inputs[p].x=patrol(p,f.players[p].x,15,90);
   }
  }
  last_=w;return w;
 }
};

#ifndef COOP_ROOM_RETURN0286_PURE
namespace n=coop_native;namespace ids=coop_collision;namespace rn=coop_rooms_native0280;namespace life=coop_lifecycle0280;namespace bridge=coop_bridge;namespace old=coop_room_route_driver0280;
inline bool enabled=false;inline Controller controller;
inline void configure(bool on){enabled=on;controller.reset();}
inline Root root(const coop_rooms0280::Identity& id,unsigned color){return {id.address,id.private_data,id.birth,color,id.kind};}
inline Frame observe(uint8_t* r,unsigned vi,unsigned room){
 Frame f;f.vi=vi;f.room=room;f.enabled=enabled;if(!r||!enabled)return f;
 const auto& state=life::lifecycle.snapshot();const auto& c=rn::coordinator;const auto& d=c.departure();
 f.from=d.room;f.to=d.route.destination();f.leader=d.leader;f.room_epoch=c.epoch();f.lifecycle_epoch=state.epoch;f.room_complete=c.phase()==coop_rooms0280::Phase::complete;
 f.bound=state.enabled&&state.bound&&!state.failed&&!state.ending&&state.room==room;
 f.bridge_valid=bool(bridge::context)&&!bridge::invalidated&&!n::constructing&&!n::cleaning;f.busy=rn::busy();
 for(unsigned p=0;p<2;p++){
  f.original[p]=root(d.payload.players[p].actor,d.payload.players[p].color);
  const auto& source=state.players[p];auto& target=f.players[p];const auto& id=source.root;
  target.root={id.address,id.private_data,id.generation,source.color,id.kind};target.death_vi=source.death_vi;target.respawn_vi=source.respawn_vi;
  target.pending=source.phase==life::Phase::death_pending||source.phase==life::Phase::retiring||source.phase==life::Phase::waiting;
  const auto now=ids::observed_identity(r,id.address);target.observed=now&&*now==id;
  if(target.observed){target.native_fatal=life::fatal(r,id.address);target.alive=life::lifecycle.alive(r,p);target.controllable=old::root_observed(r,p)&&id.address==(p?bridge::bound_secondary:bridge::bound_primary);target.x=std::bit_cast<float>(n::word(r,id.address+0x84));target.y=std::bit_cast<float>(n::word(r,id.address+0x88));}
 }
 if(scope(f)&&live(f.players[0]))f.enemy_x=old::first_death_enemy(r,f.players[0].root.address);
 return f;
}
inline bool input(uint8_t* r,unsigned vi,unsigned room,unsigned player,uint16_t* buttons,float* x,float* y){
 if(player>1||!buttons||!x||!y)return false;const auto w=controller.step(observe(r,vi,room));if(!w.handled)return false;
 const auto& out=w.inputs[player];*buttons=out.buttons;*x=out.x;*y=out.y;return true;
}
inline Witness witness(uint8_t* r,unsigned vi,unsigned room){return controller.inspect(observe(r,vi,room));}
#endif
} // namespace
