#pragma once
#include "boss_stick_aim0260.hpp"
#include "boss_tongue_geometry0260.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace workshop::boss_rest_aim0260 {
namespace stick=boss_stick_aim0260;
inline constexpr uint16_t A=0x8000,B=0x4000;
enum class Phase:uint8_t {Idle,Approach,Face,Prepare,Tongue,Swallow,Jump};
enum class Goal:uint8_t {HealToSeven,ConsumeOwnedCandy};
struct Input {
 using Goal=boss_rest_aim0260::Goal;Goal goal=Goal::HealToSeven;
 unsigned vi=0,room=177,visit=0;uint32_t player=0,fruit=0;
 bool active=false,controls=false,alive=false,grounded=false,fruit_available=false,held_valid=false,callback_pending=false,stick_observed=false,request_ready=true,tongue_observed=false,tongue_live=false,tongue_cancelled_idle=false,heading_active=false;
 unsigned health=8,mouth_phase=0;float x=0,y=0,fruit_x=0,fruit_y=0,min_x=0,max_x=768,max_reach=0,heading_degrees=std::numeric_limits<float>::quiet_NaN();
 stick::Axes old_stick{};stick::Mouth mouth;
};
struct Output {bool handled=false;uint16_t buttons=0;float x=0,y=0;};
struct Snapshot {
 Goal goal=Goal::HealToSeven;
 bool enabled=false,diagnostic=false,have_vi=false,tongue_seen=false,air_seen=false,cancelled_seen=false;
 unsigned last_vi=0,room=177,visit=0,since=0,request_vi=0;uint32_t player=0,fruit=0;Phase phase=Phase::Idle;
};
class Controller {
 Snapshot state_{};Output cached_{};
 void phase(Phase next,unsigned vi){if(state_.phase!=next){state_.phase=next;state_.since=vi;}}
 void fresh(const Input& in){const bool enabled=state_.enabled,diagnostic=state_.diagnostic;state_={};state_.enabled=enabled;state_.diagnostic=diagnostic;state_.room=in.room;state_.visit=in.visit;state_.player=in.player;state_.fruit=in.fruit;state_.goal=in.goal;state_.since=in.vi;cached_={};}
 static bool point(float x,float y){return std::isfinite(x)&&std::isfinite(y)&&std::abs(x)<100000&&std::abs(y)<100000;}
 static void axes(Output& out,const stick::Result& value){out.x=value.host.x;out.y=value.host.y;}
 static bool direction(Output& out,float x,float y,const Input& in,float magnitude){if(auto value=stick::toward(x,y,in.old_stick,magnitude)){axes(out,*value);return true;}return false;}
 Output compute(const Input& in){
  if(!in.active)return {};
  Output out{true};
  if(in.goal!=Goal::HealToSeven&&in.goal!=Goal::ConsumeOwnedCandy)return out;
  if(in.goal==Goal::HealToSeven&&in.room!=83)return {};
  // The diagnostic caller must prove the live4550 belongs to its Candy.
  // Explicit consumption never changes actual HP or weakens the Rest scope.
  if(in.goal==Goal::ConsumeOwnedCandy&&(in.room<162||in.room>176))return out;
  if(!in.player||!in.controls||!in.alive||in.health>8||!point(in.x,in.y)||!point(in.min_x,in.max_x)||in.min_x>=in.max_x||!in.stick_observed||!stick::valid(in.old_stick))return out;
  // A real held actor alone licenses swallowing. A tongue phase, animation,
  // callback pointer or anticipated future fruit is never treated as food.
  if(in.held_valid){phase(Phase::Swallow,in.vi);if(in.grounded)direction(out,0,-1,in,48);return out;}
  if(!std::isfinite(in.max_reach)||in.max_reach<=0||in.max_reach>100000)return out;
  if(!in.tongue_observed||(in.tongue_live&&in.tongue_cancelled_idle))return out;
  const bool cancelled=in.tongue_cancelled_idle&&(in.mouth_phase==1||in.mouth_phase==2)&&!in.callback_pending;
  if(in.tongue_cancelled_idle&&!cancelled)return out;
  // Native cancellation leaves raw159 intact but puts both proven actors
  // inC9B0 and clears154/156. Release once before a new prepared B edge;
  // never rewrite, replace or infer a different raw native phase.
  if(cancelled&&!state_.cancelled_seen){state_.cancelled_seen=true;state_.tongue_seen=false;state_.request_vi=0;phase(Phase::Prepare,in.vi);return out;}
  if(!cancelled)state_.cancelled_seen=false;
  // Native328C0 retracts on releasedB; native25EA4 keeps following world
  // input during the accepted tongue. InitialB below still uses relative axes.
  if(!cancelled&&(in.mouth_phase||in.callback_pending)){
   if(in.mouth_phase&&!in.tongue_live)return out;
   state_.tongue_seen=true;phase(Phase::Tongue,in.vi);
   if(in.mouth_phase==2||(in.mouth_phase==1&&in.heading_active)){
    float heading=in.heading_degrees;
    if(in.fruit&&in.fruit_available&&point(in.fruit_x,in.fruit_y)&&in.mouth.valid())if(const auto ray=stick::from_mouth(in.mouth,in.x,in.y,in.fruit_x,in.fruit_y))heading=std::clamp(std::atan2(ray->up,ray->forward)*180.f/stick::pi,-90.f,90.f);
    const auto hold=boss_tongue_geometry0260::hold_heading_world(heading,in.mouth.right,in.old_stick,32);if(!hold)return out;
    axes(out,*hold);if(in.mouth_phase==2)out.buttons=B;
   }return out;
  }
  if(state_.phase==Phase::Tongue){
   // Reaching here means the native query proved idle; a rejected B
   // needs a clean release/prepare cycle, not a guessed timeout.
   state_.tongue_seen=false;phase(Phase::Prepare,in.vi);return out;
  }
  if((in.goal==Goal::HealToSeven&&in.health>=7)||!in.fruit||!in.fruit_available){phase(Phase::Idle,in.vi);out.handled=false;return out;}
  if(!point(in.fruit_x,in.fruit_y)||!in.mouth.valid())return out;
  const float dx=in.fruit_x-in.x,dy=in.fruit_y-in.y;
  const bool want_right=dx>=0;
  if(std::abs(dx)>4&&in.mouth.right!=want_right){phase(Phase::Face,in.vi);direction(out,want_right?1.f:-1.f,0,in,4);return out;}
  const auto ray=stick::from_mouth(in.mouth,in.x,in.y,in.fruit_x,in.fruit_y);
  if(!ray){phase(Phase::Approach,in.vi);const float facing=in.mouth.right?1.f:-1.f,wanted=std::clamp(in.fruit_x-facing*24,in.min_x,in.max_x),delta=wanted-in.x;if(std::abs(delta)>2){if(in.goal==Goal::ConsumeOwnedCandy)out.x=delta>0?.65f:-.65f;else direction(out,delta>0?1.f:-1.f,0,in,12);}return out;}
  // Jumping never proves that a new native airborne posture has occurred.
  if(state_.phase==Phase::Jump){
   if(!in.grounded)state_.air_seen=true;
   const unsigned elapsed=in.vi-state_.request_vi;
   if(state_.air_seen&&elapsed>=12&&ray->distance<=in.max_reach){phase(Phase::Prepare,in.vi);return out;}
   if(state_.air_seen&&in.grounded){state_.air_seen=false;phase(Phase::Prepare,in.vi);return out;}
   if(elapsed>=64){state_.request_vi=in.vi;return out;}
   if(elapsed<45)out.buttons=A;
   return out;
  }
  if(ray->distance>in.max_reach){
   if(std::abs(dx)>32){phase(Phase::Approach,in.vi);const float wanted=std::clamp(in.fruit_x-(dx>=0?24.f:-24.f),in.min_x,in.max_x),delta=wanted-in.x;if(std::abs(delta)>3){if(in.goal==Goal::ConsumeOwnedCandy)out.x=delta>0?.65f:-.65f;else direction(out,delta>0?1.f:-1.f,0,in,16);}return out;}
   if(dy<0&&in.grounded){phase(Phase::Jump,in.vi);state_.request_vi=in.vi;state_.air_seen=false;return out;}
   return out;
  }
  if(state_.phase!=Phase::Prepare)phase(Phase::Prepare,in.vi);
  // Tongue axes are relative to facing. Sending positive forward axes
  // before B would turn a left-facing player right through ordinary movement.
  // Two neutral VI let the real native filter settle; only the accepted B
  // edge may carry the relative mouth vector on that same VI.
  if(in.vi-state_.since<2)return out;
  const auto aimed=stick::toward(ray->forward,ray->up,in.old_stick,32);
  if(!aimed||aimed->cosine<.9995f)return out;
  // Readiness licenses only a new B edge. It never blocks a real held food
  // or the native extending tongue handled above, and starts no retry timer.
  if(!in.request_ready)return out;
  axes(out,*aimed);out.buttons=B;phase(Phase::Tongue,in.vi);state_.request_vi=in.vi;state_.tongue_seen=false;
  return out;
 }
public:
 bool configure(bool enabled,bool diagnostic){state_={};cached_={};state_.enabled=enabled;state_.diagnostic=diagnostic;return !enabled||diagnostic;}
 const Snapshot& snapshot()const{return state_;}
 Output step(const Input& in){
  if(!state_.enabled||!state_.diagnostic)return {};
  if(state_.player!=in.player||state_.room!=in.room||state_.visit!=in.visit||state_.fruit!=in.fruit||state_.goal!=in.goal||(state_.have_vi&&in.vi<state_.last_vi))fresh(in);
  if(state_.have_vi&&in.vi==state_.last_vi)return cached_;
  state_.have_vi=true;state_.last_vi=in.vi;cached_=compute(in);return cached_;
 }
};
}
