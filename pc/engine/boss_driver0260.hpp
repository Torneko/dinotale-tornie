#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <span>
#include "boss_stick_aim0260.hpp"
#include "boss_tongue_geometry0260.hpp"
#include <limits>

namespace workshop::boss_driver0260 {
// Diagnostic input generator only. Callers supply read-only native observations.
// No RDRAM pointer, game-function dispatcher, health setter or victory API exists.
inline constexpr uint16_t jump_button=0x8000,tongue_button=0x4000,egg_button=0x2000;
enum class ThrowMode : uint8_t { HoldAndRelease,SecondPress };
enum class Phase : uint8_t { Idle,Supply,Approach,Face,AcquireAim,TrackAim,Release,SecondPress,Cooldown,SpringRelease,SeekClear,CancelAim,SupplyTravel,ReturnCombat };
struct Target {
 uint32_t actor=0;uint16_t kind=0;float x=0,y=0,vx=0,vy=0;
 bool active=false,vulnerable=false,defeated=false;
 // An explicitly owned red455A may supply its real head/segment coordinates.
 // Ordinary Candy remains with the tongue pilot; never disguise its native ID.
 bool red_candy=false;
 // Optional precise native41C6 observation; defaults preserve other families.
 bool clear_path=true;float native_speed=0,aim_radius=0;
 uint32_t observation_actor=0; // Diagnostic coordinate identity only; never used for control.
 uint64_t birth=0; // Explicit owned generation for the optional QA4526 target.
 bool monster_burst_observed=false,monster_burst_ready=false,monster_burst_passed=false;
};
struct Supply {
 uint32_t actor=0;uint16_t kind=0;float x=0,y=0;
 bool active=false,collectible=false;
 // Read-only native definition/private fields. A generator is never edible.
 uint16_t food_flags=0,feeder_kind=0,feeder_flags=0,feeder_axis=0;
 unsigned healing_gain=0;bool healing_clear=false; // Optional current native food/path witness.
};
struct Input {
 unsigned vi=0,room=177,encounter=0;uint32_t player=0;
 bool combat=false,controls=false,alive=false,grounded=false,mouth_full=false,spring_attached=false;
 unsigned primary_state=0,reserves=0;float x=0,y=0,min_x=0,max_x=768;
 bool has_preferred_x=false;float preferred_x=0;
 uint32_t aiming_actor=0,reticle_actor=0,projectile_actor=0;
 float reticle_x=0,reticle_y=0;
 ThrowMode throw_mode=ThrowMode::HoldAndRelease;
 std::span<const Target> targets;std::span<const Supply> supplies;
 // Optional safe standing X coordinates for arenas with separated islands.
 std::span<const float> stable_stands;
 bool native_stick_observed=false;boss_stick_aim0260::Axes native_stick{};boss_stick_aim0260::Mouth native_mouth;
 bool has_supply_stand=false;float supply_stand_x=0,supply_stand_y=0;
 // Explicit read-only151 navigation observations; defaults preserve all other families.
 bool monster_navigation=false,combat_supported=false,horizontal_speed_observed=false;float horizontal_speed=0;
 bool lips_jump_clearance_observed=false,lips_jump_column_clear=false;float lips_jump_x=0;
 bool native_egg_request_observed=false,native_egg_request_ready=false;
 bool native_tongue_observed=false,native_tongue_ready=false;
 unsigned native_tongue_phase=0;bool native_tongue_callback_pending=false;
 bool native_tongue_known=false,native_tongue_live=false,native_tongue_cancelled_idle=false;
 float native_tongue_max_reach=0,native_tongue_heading_degrees=std::numeric_limits<float>::quiet_NaN();
 bool native_tongue_heading_active=false;
 bool qa_bird=false;
 bool lips_escape_observed=false;uint32_t lips_escape_root=0;uint64_t lips_escape_birth=0;float lips_escape_x=0;int lips_escape_direction=0;
 bool genie_navigation=false,genie_support_observed=false;float genie_anchor_x=0;
 bool monster_upper_navigation=false,monster_upper_supported=false;
 bool monster_side_navigation=false; // CP6 QA only; configured+bitmap-proved retained464F.
 bool fish_navigation=false,fish_island_observed=false,fish_on_island=false;float fish_anchor_x=0;
 bool fish_evade_observed=false,fish_evade_imminent=false,fish_evade_head_clear=false,fish_evade_generation_valid=true;
 uint32_t fish_threat_actor=0;uint64_t fish_threat_birth=0;unsigned fish_threat_step=0;
 bool fish_counter_model_observed=false,fish_reticle_cleared=false;float fish_world_scale=0,fish_air_brake=0;
 // Native4632 has no campaign Variant birth. Keep its revalidated resource/
 // private identity separate from the owned Fish generation witness.
 bool fish_threat_water=false;uint32_t fish_threat_resource=0,fish_threat_private=0;
 unsigned fish_retry_epoch=0;
 bool lips_healing_observed=false,health_observed=false;unsigned health=0;
 // Caller proves the native164 Invisible family and its published standing anchor.
 bool invisible_navigation=false;
};
struct Output {bool handled=false;uint16_t buttons=0;float x=0,y=0;};
struct Config {
 unsigned face_vi=4,aim_timeout_vi=120,acquire_timeout_vi=90,cooldown_vi=65;
 float stand_off=150,projectile_speed=14,aim_cosine=.996f;
 bool valid()const{return face_vi>0&&face_vi<=30&&aim_timeout_vi>=10&&aim_timeout_vi<=300&&acquire_timeout_vi>=10&&acquire_timeout_vi<=300&&cooldown_vi>=10&&cooldown_vi<=300&&std::isfinite(stand_off)&&stand_off>=40&&stand_off<=300&&std::isfinite(projectile_speed)&&projectile_speed>0&&std::isfinite(aim_cosine)&&aim_cosine>.9f&&aim_cosine<1;}
};
struct Snapshot {
 bool enabled=false,diagnostic=false;Phase phase=Phase::Idle;
 bool monster_refilling=false,monster_returning=false;unsigned monster_settle_count=0,monster_settle_vi=0;
 unsigned phase_vi=0,last_vi=0,room=177,encounter=0,attempts=0,launches=0,aim_matches=0;
 uint32_t player=0,target=0,last_projectile=0,supply_actor=0;
 unsigned jump_vi=0,spring_detach_vi=0,supply_recovery_vi=0;bool have_vi=false,aim_seen=false,jump_active=false,spring_detached=false,supply_ready=false,supply_lower_ready=false,supply_recovery=false;
 // Supply navigation observes actual displacement; requested input alone is
 // never a landing, a consumed bird, or a successful recovery.
 bool supply_jump_grounded=false,supply_position_observed=false,supply_relocating=false;
 float supply_last_x=0,supply_last_y=0,supply_destination=0;
 unsigned supply_position_vi=0,supply_relocation_vi=0;
 bool lips_escaping=false;uint32_t lips_escape_root=0;uint64_t lips_escape_birth=0;float lips_escape_x=0;int lips_escape_direction=0;unsigned lips_escape_vi=0;
 bool monster_upper_combat=false;unsigned monster_arrival_count=0,monster_arrival_vi=0;
 bool fish_anchor_attested=false,fish_recovering=false;float fish_anchor_x=0;unsigned fish_recovery_vi=0,fish_settle_count=0,fish_settle_vi=0;
 bool fish_face_preparing=false;
 bool fish_evading=false,fish_evade_attempted=false,fish_evade_pending_z=false,fish_evade_sending_z=false;
 uint32_t fish_evade_actor=0,fish_evade_aim=0;uint64_t fish_evade_birth=0;unsigned fish_evade_vi=0,fish_evade_z_vi=0;
 bool fish_throw_counter=false;
 bool fish_evade_water=false;uint32_t fish_evade_resource=0,fish_evade_private=0;
 bool fish_evade_acquire_z=false;unsigned fish_evade_acquire_vi=0;
 float fish_anchor_y=0;bool fish_air_preparing=false;
 unsigned fish_cooldown_vi=0,fish_release_not_before=0,fish_authorized_not_before=0;uint32_t fish_authorized_aim=0;bool fish_shot_authorized=false;
 unsigned fish_retry_epoch=0;
 bool lips_healing=false;unsigned lips_heal_health=9;
 bool invisible_travelling=false,invisible_travel_expired=false;unsigned invisible_travel_vi=0;
};
inline bool point(float x,float y){return std::isfinite(x)&&std::isfinite(y)&&std::abs(x)<100000&&std::abs(y)<100000;}
inline bool supported(uint16_t kind){return kind==0x4599||kind==0x4072||kind==0x40A3||kind==0x41C6||kind==0x4511||kind==0x4631;}
inline bool target_valid(const Target& t,bool qa_bird=false){return t.actor&&(supported(t.kind)||(t.kind==0x455A&&t.red_candy)||(qa_bird&&t.kind==0x4526&&t.birth))&&t.active&&!t.defeated&&point(t.x,t.y)&&point(t.vx,t.vy);}
inline bool bird_supply(const Supply& s){return (s.kind==0x4526||s.kind==0x46DA)&&(s.food_flags&1);}
inline bool pot_supply(const Supply& s){return (s.kind==0x4539||s.kind==0x453A)&&(s.food_flags&1);}
inline bool edible_supply(const Supply& s){return bird_supply(s)||pot_supply(s);}
inline bool feeder_supply(const Supply& s){return s.kind==0x456E&&s.feeder_kind==0x4528&&s.feeder_axis==0;}
inline bool supply_valid(const Supply& s){return s.actor&&s.active&&point(s.x,s.y)&&((s.kind==0x4002&&s.collectible)||s.kind==0x4086||edible_supply(s)||feeder_supply(s));}
inline void predicted_target(const Input& in,const Target& t,float& tx,float& ty){
  const float target_y=t.y;
  const float distance=std::hypot(t.x-in.x,target_y-in.y);
  float lead=std::clamp(distance/14.f,0.f,24.f);
  if((t.kind==0x41C6||t.kind==0x40A3||t.kind==0x4072||t.kind==0x4511||t.kind==0x4631)&&std::isfinite(t.native_speed)&&t.native_speed>0){
   const float dx=t.x-in.x,dy=target_y-in.y,a=t.vx*t.vx+t.vy*t.vy-t.native_speed*t.native_speed,b=2*(dx*t.vx+dy*t.vy),c=dx*dx+dy*dy;
   const float disc=b*b-4*a*c;
   lead=distance/t.native_speed;
   if(std::abs(a)>.001f&&disc>=0){const float one=(-b+std::sqrt(disc))/(2*a),two=(-b-std::sqrt(disc))/(2*a);if(one>0&&two>0)lead=std::min(one,two);else if(one>0)lead=one;else if(two>0)lead=two;}
   lead=std::clamp(lead,0.f,60.f);
  }
  tx=t.x+t.vx*lead;ty=target_y+t.vy*lead;
 }
struct ReleaseTrace {
 unsigned serial=0,vi=0,room=177,encounter=0;uint32_t player=0,root=0,eye=0,aim=0,reticle=0;uint16_t kind=0;
 float x=0,y=0,target_x=0,target_y=0,vx=0,vy=0,predicted_x=0,predicted_y=0,reticle_x=0,reticle_y=0;
 boss_stick_aim0260::Axes native_axes{};
};
class Controller {
 Snapshot state_;Config config_;bool log_=true;Output cached_{};ReleaseTrace release_trace_{};unsigned release_logs_=0,heal_logs_=0;
 void trace_release(const Input& in,const char* reason){
  if(!log_||!state_.enabled||!state_.diagnostic||release_logs_>=128)return;
  const Target* target=nullptr;for(const auto& candidate:in.targets)if(candidate.actor==state_.target&&target_valid(candidate,in.qa_bird)){target=&candidate;break;}if(!target)return;
  float tx=0,ty=0;aim_point(in,*target,tx,ty);
  release_trace_={++release_logs_,in.vi,in.room,in.encounter,in.player,target->actor,target->observation_actor,in.aiming_actor,in.reticle_actor,target->kind,in.x,in.y,target->x,target->y,target->vx,target->vy,tx,ty,in.reticle_x,in.reticle_y,in.native_stick};
  const auto& r=release_trace_;
  std::fprintf(stderr,"[BossEggRelease] serial=%u vi=%u room=%u encounter=%u player=%08X root=%08X eye=%08X kind=%04X targetXY=%.6f,%.6f velocity=%.6f,%.6f intercept=%.6f,%.6f playerXY=%.6f,%.6f reticle=%08X reticleXY=%.6f,%.6f held4005=%08X nativeAxes=%d,%d throwMode=%u reason=%s nativeObservation=1 controlsOnly=1\n",r.serial,r.vi,r.room,r.encounter,r.player,r.root,r.eye,unsigned(r.kind),r.target_x,r.target_y,r.vx,r.vy,r.predicted_x,r.predicted_y,r.x,r.y,r.reticle,r.reticle_x,r.reticle_y,r.aim,r.native_axes.x,r.native_axes.y,unsigned(in.throw_mode),reason);
 }

 unsigned cooldown_duration(const Input& in)const{return in.fish_navigation&&!in.qa_bird?60u:config_.cooldown_vi;}
 void phase(const Input& in,Phase next,const char* reason){
  if(state_.phase==next)return;state_.phase=next;state_.phase_vi=in.vi;state_.aim_matches=0;
  if(in.fish_navigation&&!in.qa_bird){
   if(next==Phase::Cooldown){state_.fish_cooldown_vi=in.vi;state_.fish_release_not_before=in.vi+cooldown_duration(in);}
   if(next==Phase::CancelAim||next==Phase::Idle)state_.fish_shot_authorized=false;
   if(next==Phase::Release&&in.aiming_actor){state_.fish_shot_authorized=true;state_.fish_authorized_aim=in.aiming_actor;state_.fish_authorized_not_before=state_.fish_release_not_before;}
   if(log_&&(next==Phase::Cooldown||next==Phase::Release))std::fprintf(stderr,"[BossFishTiming] vi=%u phase=%u cooldownVI=%u nextReleaseNotBefore=%u authorized=%u authorizedAim=%08X authorizedNotBefore=%u controlsOnly=1\n",in.vi,unsigned(next),state_.fish_cooldown_vi,state_.fish_release_not_before,unsigned(state_.fish_shot_authorized),state_.fish_authorized_aim,state_.fish_authorized_not_before);
  }
  if(next!=Phase::Supply){state_.supply_actor=0;state_.jump_active=false;state_.jump_vi=0;state_.supply_jump_grounded=state_.supply_position_observed=state_.supply_relocating=false;}
  state_.spring_detached=false;state_.spring_detach_vi=0;
  if(log_)std::fprintf(stderr,"[BossDriver] vi=%u room=%u encounter=%u phase=%u target=%08X reason=%s controlsOnly=1\n",in.vi,in.room,in.encounter,unsigned(next),state_.target,reason);
  if(next==Phase::Release)trace_release(in,reason);
 }
 void fresh(const Input& in){const bool e=state_.enabled,d=state_.diagnostic;state_={};state_.enabled=e;state_.diagnostic=d;state_.player=in.player;state_.room=in.room;state_.encounter=in.encounter;state_.fish_retry_epoch=in.fish_retry_epoch;state_.phase_vi=in.vi;cached_={};}
 bool fish_shot_allowed(const Input& in)const{return !in.fish_navigation||in.qa_bird||(state_.fish_shot_authorized&&state_.fish_authorized_aim==in.aiming_actor&&in.vi>=state_.fish_authorized_not_before);}
 void aim_point(const Input& in,const Target& t,float& tx,float& ty)const{
  if((t.kind==0x41C6||t.kind==0x40A3||t.kind==0x4072||t.kind==0x4511||t.kind==0x4631)&&t.native_speed>0)predicted_target(in,t,tx,ty);
  else{const float lead=std::clamp(std::hypot(t.x-in.x,t.y-in.y)/config_.projectile_speed,0.f,24.f);tx=t.x+t.vx*lead;ty=t.y+t.vy*lead;}
 }
 bool reticle_reaches(const Input& in,const Target& t)const{
  if(t.kind==0x41C6&&t.monster_burst_observed)return t.monster_burst_ready;
  if(!in.reticle_actor||!point(in.reticle_x,in.reticle_y))return false;float tx=0,ty=0;aim_point(in,t,tx,ty);
  // Native404C8 stores this actual4003 point in4002 private8/C. NativeAAB8
  // expires the projectile there: an aligned but shorter ray cannot hit a
  // more distant target. No assumed infinite ray or forced range is used.
  return std::hypot(in.reticle_x-in.x,in.reticle_y-in.y)>=std::hypot(tx-in.x,ty-in.y);
 }
 const Target* select(const Input& in)const{
  if(in.fish_navigation&&!in.qa_bird&&!state_.fish_shot_authorized&&state_.phase!=Phase::Release&&state_.phase!=Phase::SecondPress){
   const Target* closest=nullptr;const Target* current=nullptr;float nearest=std::numeric_limits<float>::infinity(),current_distance=nearest;
   for(const auto& t:in.targets)if((t.kind==0x4511||t.kind==0x4631)&&target_valid(t)&&t.vulnerable&&t.clear_path&&std::isfinite(t.native_speed)&&t.native_speed>0){
    float tx=0,ty=0;aim_point(in,t,tx,ty);const float distance=std::hypot(tx-in.x,ty-in.y);if(!std::isfinite(distance))continue;
    if(distance<nearest){closest=&t;nearest=distance;}if(t.actor==state_.target){current=&t;current_distance=distance;}
   }
   if(closest)return current&&current_distance<=nearest+12.f?current:closest;
  }
  const Target* best=nullptr;float cost=1e30f;
  for(const auto& t:in.targets)if(target_valid(t,in.qa_bird)&&t.vulnerable){if(t.actor==state_.target&&t.clear_path)return &t;const float dx=t.x-in.x,dy=t.y-in.y,c=dx*dx+.35f*dy*dy+(t.clear_path?0.f:1e8f);if(c<cost){best=&t;cost=c;}}
  return best;
 }
 void aim_axes(const Input& in,const Target& t,Output& out)const{
  float tx=0,ty=0;
  aim_point(in,t,tx,ty);
  if(in.fish_navigation&&!in.qa_bird&&(t.kind==0x4511||t.kind==0x4631)&&in.native_stick_observed&&in.aiming_actor){
   // Primary3 constructs4005 while39338 is still inactive. Request native
   // zero through the observed input filter instead of moving the player.
   float ex=0,ey=0;
   if(in.primary_state!=3){
    if(!in.reticle_actor||!point(in.reticle_x,in.reticle_y))return;
    const float distance=std::hypot(tx-in.x,ty-in.y);if(!std::isfinite(distance)||distance<.001f)return;
    // Native39338/390F0 moves the actual4003 POSITION by nativeAxes/9.
    // Aim four pixels beyond the unchanged target ray for the range gate.
    ex=tx+4.f*(tx-in.x)/distance-in.reticle_x;ey=ty+4.f*(ty-in.y)/distance-in.reticle_y;
   }
   if(std::hypot(ex,ey)<=.5f)ex=ey=0;
   const boss_stick_aim0260::Candidate* best=nullptr;float best_error=std::numeric_limits<float>::infinity(),best_size=best_error;
   for(const auto& candidate:boss_stick_aim0260::legal_candidates()){
    const auto axes=boss_stick_aim0260::native_axes(candidate.raw,in.native_stick);
    // Air340F8 interprets nativeY<-48 as ground-pound before reticle
    // control (2E0B0 clears4005). Do not request that action while aiming.
    if(!in.grounded&&axes.y< -48)continue;
    const float dx=ex-float(axes.x)/9.f,dy=ey+float(axes.y)/9.f,error=dx*dx+dy*dy,size=float(axes.x*axes.x+axes.y*axes.y);
    if(error<best_error-.00001f||(std::abs(error-best_error)<=.00001f&&size<best_size)){best=&candidate;best_error=error;best_size=size;}
   }
   if(best){out.x=best->host.x;out.y=best->host.y;}return;
  }
  const float dx=tx-in.x,dy=ty-in.y,length=std::hypot(dx,dy);if(in.native_stick_observed){if(auto axes=boss_stick_aim0260::toward(dx,-dy,in.native_stick)){out.x=axes->host.x;out.y=axes->host.y;}return;}if(length>1){out.x=.85f*dx/length;out.y=-.85f*dy/length;}
 }
 bool aligned(const Input& in,const Target& t)const{
  if(!in.reticle_actor||!point(in.reticle_x,in.reticle_y))return false;
  // Monsters knock direct4002 contacts into collectible eggs. Only the
  // currently observed native airburst window licenses this scoped release.
  if(t.kind==0x41C6&&t.monster_burst_observed)return t.monster_burst_ready;
  float tx=0,ty=0;
  aim_point(in,t,tx,ty);
  const float ax=in.reticle_x-in.x,ay=in.reticle_y-in.y;tx-=in.x;ty-=in.y;
  const float aim_length=std::hypot(ax,ay),divisor=aim_length*std::hypot(tx,ty);
  if(divisor<=16||ax*tx+ay*ty<=0||aim_length<std::hypot(tx,ty))return false;
  if((t.kind==0x41C6||t.kind==0x40A3)&&t.aim_radius>0&&std::isfinite(t.aim_radius))return std::abs(ax*ty-ay*tx)/aim_length<=t.aim_radius;
  return ((ax*tx+ay*ty)/divisor)>=config_.aim_cosine;
 }
 Output cancel_aim(const Input& in,const char* reason){
  phase(in,Phase::CancelAim,reason);Output out{true};
  // Native3BD60 tests throwing before B. A hold-mode cancel must retain Z;
  // second-press mode must not create another Z edge.3BD18 then calls40C48.
  if(in.throw_mode==ThrowMode::HoldAndRelease)out.buttons=egg_button;
  if((in.vi-state_.phase_vi)%30<3)out.buttons|=tongue_button;
  return out;
 }
 bool island_center(const Input& in,float& center)const{
  bool found=false;float distance=1e30f;
  for(float at:in.stable_stands)if(std::isfinite(at)&&at>=in.min_x&&at<=in.max_x){const float delta=std::abs(at-in.x);if(delta<distance){distance=delta;center=at;found=true;}}
  return found;
 }
 static float walk(float delta,float tolerance=3.f){return std::abs(delta)>tolerance?(delta>0?.55f:-.55f):0;}
 bool fish_air_origin(const Input& in,bool preparing=false)const{
  if(!in.fish_navigation||in.qa_bird||!in.combat||!in.controls||!in.alive||in.encounter>=9||in.room!=167+in.encounter||!state_.fish_anchor_attested||!in.fish_island_observed||in.fish_anchor_x!=state_.fish_anchor_x||in.grounded||in.primary_state!=1||!in.horizontal_speed_observed||!std::isfinite(in.horizontal_speed)||!in.native_stick_observed||!in.native_egg_request_observed||!in.native_egg_request_ready||!in.fish_evade_observed||!in.fish_evade_head_clear)return false;
  if(in.mouth_full||!in.native_tongue_known||!in.native_tongue_observed||in.native_tongue_live||in.native_tongue_callback_pending||!point(in.x,in.y)||!std::isfinite(state_.fish_anchor_y)||in.y<0||in.y>state_.fish_anchor_y-16)return false;
  return std::abs(in.x-state_.fish_anchor_x)<=(preparing?12.f:8.f)&&std::abs(in.horizontal_speed)<=(preparing?2.f:.25f);
 }
 // Before a real4005 exists, ordinary walking must stay on the current island.
 bool island_guard(const Input& in,Output& out)const{
  float center=0;if(!island_center(in,center))return false;
  if(state_.fish_air_preparing&&fish_air_origin(in,true))return false;
  if(std::abs(center-in.x)>12||!in.grounded){out.x=walk(center-in.x);return true;}
  return false;
 }
 Output supply_route(const Input& in){
  phase(in,Phase::SupplyTravel,"native_upper_supply_platform");Output out{true};
  const float lower=in.has_preferred_x?in.preferred_x:in.x;
  if(in.grounded&&in.y>in.supply_stand_y+84)state_.supply_lower_ready=false;
  if(in.grounded&&std::abs(in.x-lower)<=8&&in.y>=in.supply_stand_y+24&&in.y<=in.supply_stand_y+84){
   if(!state_.supply_lower_ready){state_.supply_lower_ready=true;state_.supply_recovery=false;state_.phase_vi=in.vi;}
  }
  if(std::abs(in.x-in.supply_stand_x)<=8&&in.y<in.supply_stand_y+12&&!in.grounded)return out; // await native upper landing
  if(!state_.supply_lower_ready&&(in.y>in.supply_stand_y+84||state_.supply_recovery)){
   const float launch=std::clamp(lower+(lower<in.supply_stand_x?-40.f:40.f),in.min_x,in.max_x);
   if(!state_.supply_recovery){out.x=walk(launch-in.x,4);if(in.grounded&&std::abs(launch-in.x)<=4){state_.supply_recovery=true;state_.supply_recovery_vi=in.vi;}return out;}
   const unsigned recovery=in.vi-state_.supply_recovery_vi;if(recovery>=120){state_.supply_recovery=false;return out;}
   if(recovery>=2&&recovery<47)out.buttons=jump_button;
   if(recovery>=14)out.x=walk(lower-in.x);return out;
  }
  const unsigned elapsed=(in.vi-state_.phase_vi)%120;if(elapsed<2)return out;
  if(elapsed<47)out.buttons=jump_button;
  out.x=walk((state_.supply_lower_ready?in.supply_stand_x:lower)-in.x);return out;
 }
 // Relative mouth axes belong to a new B request. Waiting or rejected
 // requests preserve the observed global facing with a small real stick.
 void preserve_facing(const Input& in,Output& out)const{
  out.x=out.y=0;
  if(in.native_mouth.valid())if(auto face=boss_stick_aim0260::toward(in.native_mouth.right?1.f:-1.f,0,in.native_stick,4)){out.x=face->host.x;out.y=face->host.y;}
 }
 // Native25EA4 keeps updating174 while the tongue is live. Its input is
 // world-facing, unlike the relative first-B capture in2ED94. Keep the same
 // admitted bird during the flight; a missing/replaced bird uses the actual
 // native heading instead of selecting another source mid-tongue.
 Output hold_tongue(const Input& in){
  Output out{true,in.native_tongue_phase==2?tongue_button:uint16_t(0)};
  if(!in.native_tongue_heading_active||!in.native_stick_observed||!in.native_mouth.valid())return out;
  float heading=in.native_tongue_heading_degrees;
  for(const auto& source:in.supplies)if(source.actor==state_.supply_actor&&supply_valid(source)&&edible_supply(source)){
   if(const auto ray=boss_stick_aim0260::from_mouth(in.native_mouth,in.x,in.y,source.x,source.y))heading=std::clamp(std::atan2(ray->up,ray->forward)*180.f/boss_stick_aim0260::pi,-90.f,90.f);
   break;
  }
  if(const auto axes=boss_tongue_geometry0260::hold_heading_world(heading,in.native_mouth.right,in.native_stick)){out.x=axes->host.x;out.y=axes->host.y;}
  return out;
 }
 // Health is an observation, not a setter. Only an owned Lips encounter
 // can forage its real, reachable healing pots while eggs remain in stock.
 std::optional<Output> heal_lips(const Input& in){
  const bool owned=std::any_of(in.targets.begin(),in.targets.end(),[](const Target& t){return t.kind==0x40A3&&target_valid(t);});
  if(!in.lips_healing_observed||!in.health_observed||in.health>8||!owned){state_.lips_healing=false;return {};}
  const bool busy=monster_busy(in);
  if(in.health==8&&!busy){state_.lips_healing=false;return {};}
  if(in.aiming_actor||in.reticle_actor||state_.phase==Phase::Release||state_.phase==Phase::SecondPress)return {};
  if(!state_.lips_healing&&(in.health>5||busy||in.primary_state!=1))return {};
  if(in.primary_state!=1&&!busy)return {};
  std::array<Supply,150> pots{};unsigned count=0;
  for(const auto& food:in.supplies)if(count<pots.size()&&supply_valid(food)&&pot_supply(food)&&food.healing_gain>0&&food.healing_gain<=8&&food.healing_clear&&food.x>=in.min_x&&food.x<=in.max_x&&std::abs(food.x-in.x)<=160&&food.y-in.y>=-200&&food.y-in.y<=80)pots[count++]=food;
  // A missing pot is not permission to stand idle or chase a generator.
  // Keep an already admitted tongue/held-food cycle until native swallowing.
  if(!count&&!(state_.lips_healing&&busy))return {};
  const bool began=!state_.lips_healing;state_.lips_healing=true;
  Input forage=in;forage.supplies={pots.data(),count};
  auto out=supply(forage);
  if(log_&&heal_logs_<48&&(began||state_.lips_heal_health!=in.health)){
   ++heal_logs_;std::fprintf(stderr,"[BossLipsHeal] vi=%u room=%u encounter=%u player=%08X hp=%u eggs=%u source=%08X candidates=%u busy=%u nativeObservation=1 controlsOnly=1\n",in.vi,in.room,in.encounter,in.player,in.health,in.reserves,state_.supply_actor,count,unsigned(busy));
  }
  state_.lips_heal_health=in.health;return out;
 }
 Output supply(const Input& in){
  // Native159 can remain stale after native3D76C cancels both actors.
  // Only a proven live graph licenses extension hold. Unknown graphs stay
  // neutral; proven idle cancellation releases any old B before a retry.
  // Actual held food remains prioritary and raw native159 is never changed.
  if(in.native_tongue_observed&&!in.mouth_full){
   if(!in.native_tongue_known){phase(in,Phase::Supply,"unknown_native_tongue");return Output{true};}
   if(in.native_tongue_live&&(in.native_tongue_phase==1||in.native_tongue_phase==2)){phase(in,Phase::Supply,"native_tongue_motion");return hold_tongue(in);}
   if(in.native_tongue_cancelled_idle&&!in.native_tongue_live&&!in.native_tongue_callback_pending){
    if(cached_.buttons&tongue_button){phase(in,Phase::Supply,"native_tongue_cancel_release");return Output{true};}
   }else if(in.native_tongue_phase||in.native_tongue_callback_pending||in.native_tongue_live){phase(in,Phase::Supply,"native_tongue_lifecycle");return Output{true};}
  }
  if(in.has_supply_stand){
   if(state_.supply_ready&&(std::abs(in.x-in.supply_stand_x)>20||in.y>in.supply_stand_y+24))state_.supply_ready=false;
   if(!state_.supply_ready){
    if(in.monster_upper_navigation){
     if(!in.monster_upper_supported){state_.monster_arrival_count=0;return supply_route(in);}
     if(!monster_stopped(in)){state_.monster_arrival_count=0;return Output{true};}
     state_.monster_arrival_count=state_.monster_arrival_count&&in.vi==state_.monster_arrival_vi+1?state_.monster_arrival_count+1:1;state_.monster_arrival_vi=in.vi;
     if(state_.monster_arrival_count<2)return Output{true};
    }
    if(in.grounded&&std::abs(in.x-in.supply_stand_x)<=8&&in.y>=in.supply_stand_y-24&&in.y<=in.supply_stand_y+12)state_.supply_ready=true;else return supply_route(in);
   }
  }
  phase(in,Phase::Supply,"native_egg_supply");Output out{true};float center=0;const bool island=island_center(in,center),preferred=!island&&(in.has_supply_stand||in.has_preferred_x);
  if(preferred)center=in.has_supply_stand?in.supply_stand_x:in.preferred_x;const bool anchored=island||preferred;const float band=island?12.f:8.f;
  if(anchored)out.x=walk(center-in.x);
  if(in.mouth_full){state_.supply_position_observed=state_.supply_relocating=false;if((!anchored||std::abs(center-in.x)<=band)&&in.grounded)out.y=-.85f;return out;}
  const Supply* best=nullptr;const Supply* feeder=nullptr;const Supply* travel=nullptr;float cost=1e30f,feeder_cost=1e30f,travel_cost=1e30f;
  const bool monster=std::any_of(in.targets.begin(),in.targets.end(),[](const Target& t){return t.kind==0x41C6&&target_valid(t);});
  const float upper_bird=monster?-240.f:-200.f;
  for(const auto& s:in.supplies)if(supply_valid(s)){
   // The proved Genie plateau is continuous, but it does not cover remote
   // tilting platforms. Its empty-stock forage may use only sources over it.
   if(in.genie_navigation&&in.genie_support_observed&&(s.x<in.min_x||s.x>in.max_x))continue;
   const float dx=s.x-in.x,dy=s.y-in.y;
   if(feeder_supply(s)){
    if(std::abs(dy)<128){const float wait=std::clamp(s.x+((s.feeder_flags&1)?32.f:-32.f),in.min_x,in.max_x),c=std::abs(wait-in.x);if(c<feeder_cost){feeder=&s;feeder_cost=c;}}
    continue;
   }
   // A remote real bird is a walking destination only in a continuous arena.
   // It cannot pull Yoshi off a fish island, away from a mapped support, past
   // a native feeder, outside the arena, or toward an unreachable height.
   if(edible_supply(s)){
    if(dy<upper_bird||dy>80||s.x<in.min_x-20||s.x>in.max_x+20)continue;
    if(std::abs(dx)>160){if(!anchored&&s.x>=in.min_x&&s.x<=in.max_x){const float c=dx*dx+.4f*dy*dy;if(c<travel_cost){travel=&s;travel_cost=c;}}continue;}
   }
   const float c=dx*dx+.4f*dy*dy+(s.kind==0x4002?0.f:s.kind==0x4086?2000.f:5000.f);if(c<cost){cost=c;best=&s;}
  }
  if(!anchored&&!feeder){
   if(!state_.supply_position_observed||std::hypot(in.x-state_.supply_last_x,in.y-state_.supply_last_y)>8){
    state_.supply_position_observed=true;state_.supply_last_x=in.x;state_.supply_last_y=in.y;state_.supply_position_vi=in.vi;
   }
   const float inset=std::min(48.f,(in.max_x-in.min_x)*.25f);
   const bool edge=in.x<in.min_x+inset||in.x>in.max_x-inset;
   const bool stalled=in.vi-state_.supply_position_vi>=180;
   bool finished_relocation=false;
   if(state_.supply_relocating&&(best||travel||in.vi-state_.supply_relocation_vi>=120||std::abs(in.x-state_.supply_destination)<=8)){
    state_.supply_relocating=false;state_.supply_position_vi=in.vi;finished_relocation=true;
   }
   if(!best&&!travel&&!state_.supply_relocating&&!finished_relocation&&(edge||stalled)&&std::abs(in.x-(in.min_x+in.max_x)*.5f)>8){
    state_.supply_relocating=true;state_.supply_destination=(in.min_x+in.max_x)*.5f;state_.supply_relocation_vi=in.vi;
    if(log_)std::fprintf(stderr,"[BossSupplyRecovery] vi=%u room=%u encounter=%u player=%08X xy=%.3f,%.3f destination=%.3f reason=no_reachable_source controlsOnly=1\n",in.vi,in.room,in.encounter,in.player,in.x,in.y,state_.supply_destination);
   }
   if(state_.supply_relocating){state_.supply_actor=0;state_.jump_active=state_.supply_jump_grounded=false;out.x=walk(state_.supply_destination-in.x,8);return out;}
  }else{state_.supply_position_observed=state_.supply_relocating=false;}
  if(!best){
   state_.supply_actor=feeder?feeder->actor:travel?travel->actor:0;state_.jump_active=state_.supply_jump_grounded=false;
   if(feeder&&!island){const bool preferred_side=preferred&&((feeder->feeder_flags&1)?center>feeder->x+10:center<feeder->x-10);const float wait=preferred_side?center:std::clamp(feeder->x+((feeder->feeder_flags&1)?32.f:-32.f),in.min_x,in.max_x);out.x=walk(wait-in.x,preferred_side?3.f:4.f);}
   else if(travel&&!anchored){const float inset=std::min(32.f,(in.max_x-in.min_x)*.125f);out.x=walk(std::clamp(travel->x,in.min_x+inset,in.max_x-inset)-in.x,40);}
   return out; // Never A or B toward an invisible generator.
  }
  state_.supply_actor=best->actor;const float dx=best->x-in.x,dy=best->y-in.y;
  if(best->kind==0x4002){const float wanted=anchored?std::clamp(best->x,center-band,center+band):best->x;out.x=walk(wanted-in.x);state_.jump_active=false;return out;}
  if(best->kind==0x4086){
   const float wanted=anchored?std::clamp(best->x,center-band,center+band):best->x;out.x=walk(wanted-in.x,8);
   if((std::abs(dx)<=20||dy< -45)&&in.vi%120<65)out.buttons=jump_button;return out;
  }
  if(!anchored)out.x=walk(dx,40);
  if(pot_supply(*best)){
   state_.jump_active=state_.supply_jump_grounded=false;
   // Proven native4539/453A are ordinary edible food, including while lying
   // on the ground. Keep the real mouth/readiness gates and use B directly;
   // the flying-bird jump sequence must not be imposed on these objects.
   if(!in.native_stick_observed||!in.native_tongue_observed||!in.native_mouth.valid()){out.x=out.y=0;return out;}
   if(std::abs(dx)>4&&in.native_mouth.right!=(dx>=0)){if(const auto face=boss_stick_aim0260::toward(dx>=0?1.f:-1.f,0,in.native_stick,4)){out.x=face->host.x;out.y=face->host.y;}return out;}
   const auto ray=boss_stick_aim0260::from_mouth(in.native_mouth,in.x,in.y,best->x,best->y);
   if(!ray||!std::isfinite(in.native_tongue_max_reach)||in.native_tongue_max_reach<=0||ray->distance>in.native_tongue_max_reach)return out;
   if(!in.native_tongue_ready){preserve_facing(in,out);return out;}
   if(cached_.buttons&tongue_button){out.x=out.y=0;return out;}
   if(const auto axes=boss_stick_aim0260::toward(ray->forward,ray->up,in.native_stick);axes&&axes->cosine>=.9995f){out.x=axes->host.x;out.y=axes->host.y;out.buttons=tongue_button;}
   return out;
  }
  // Prepare the vertical tongue direction before B, rather than changing it
  // on the same frame as the tongue edge. Precise Monster B input below
  // separately inverts the real native deadzone from observed old axes.
  if(dy< -65)out.y=.85f;
  const bool close=std::abs(dx)<=110;
  if(island&&std::abs(center-in.x)>12)return out;
  if(!close)return out;
  if(std::abs(dx)>4){const float face=dx>0?.35f:-.35f;if(!anchored||std::abs((in.x-center)+face*4)<=band)out.x=face;}
  if(!state_.jump_active||(in.grounded&&!state_.supply_jump_grounded)){state_.jump_active=true;state_.supply_jump_grounded=in.grounded;state_.jump_vi=in.vi;}
  unsigned elapsed=in.vi-state_.jump_vi;
  if(in.grounded){
   // B wins over A in native415FC. Keep the jump edge alone, and if an
   // earlier native action blocked it, release A before the next attempt.
   if(elapsed>=64){state_.jump_vi=in.vi;elapsed=0;}
   if(elapsed<45)out.buttons=jump_button;
   return out;
  }
  // A new airborne observation is not a new ground jump. Holding A there
  // starts/restarts native flutter and used to keep an empty-stock Yoshi at
  // the wall whenever passing birds vanished and the supply epoch restarted.
  if(!state_.supply_jump_grounded)elapsed+=12;
  if(elapsed<12){out.buttons=jump_button;return out;}
  const bool reachable=std::abs(dx)<=100&&dy>=-110&&dy<=55;
  if(reachable){
   if(std::abs(dx)>4){const float face=dx>0?.35f:-.35f;if(!anchored||std::abs((in.x-center)+face*4)<=band)out.x=face;}
   if((elapsed-12)%24<4){
    if(in.native_stick_observed&&in.native_tongue_observed){
     // Holding B does not produce another native PRESSED-B edge. Preserve
     // the existing four-VI pulse but never resend relative motion alone.
     if(cached_.buttons&tongue_button){out.buttons=tongue_button;out.x=out.y=0;}
     else if(!in.native_tongue_ready)preserve_facing(in,out);
     else{
      const auto ray=boss_stick_aim0260::from_mouth(in.native_mouth,in.x,in.y,best->x,best->y);
      if(!ray){preserve_facing(in,out);if(in.native_mouth.valid())if(auto face=boss_stick_aim0260::toward(dx>=0?1.f:-1.f,0,in.native_stick,4)){out.x=face->host.x;out.y=face->host.y;}}
      else if(std::isfinite(in.native_tongue_max_reach)&&in.native_tongue_max_reach>0&&ray->distance<=in.native_tongue_max_reach){
       const auto axes=boss_stick_aim0260::toward(ray->forward,ray->up,in.native_stick);
       if(axes&&axes->cosine>=.9995f){out.x=axes->host.x;out.y=axes->host.y;out.buttons=tongue_button;}
       else preserve_facing(in,out);
      }else preserve_facing(in,out);
     }
    }else if(in.native_stick_observed){
     // Native axes alone do not establish an ordinary tongue admission or
     // a physical reach. Only fully synthetic inputs retain legacy pulses.
     out.x=out.y=0;
    }else out.buttons=tongue_button;
   }
  }
  if(!out.buttons&&dy< -45&&elapsed<65&&state_.supply_jump_grounded)out.buttons=jump_button;
  return out;
 }
 Output genie_supply(const Input& in){
  Input forage=in;forage.stable_stands={};forage.has_preferred_x=false;forage.has_supply_stand=false;
  auto out=supply(forage);
  if((in.x<=in.min_x&&out.x<0)||(in.x>=in.max_x&&out.x>0))out.x=0;
  return out;
 }
 // A stock-up session is tied to the player/room/encounter, not to the
 // moving eye or first available egg. All six reserves are native observations.
 bool monster_busy(const Input& in)const{
  if(in.mouth_full)return true;
  if(!in.native_tongue_observed)return false;
  return !in.native_tongue_known||in.native_tongue_live||in.native_tongue_callback_pending||
   (in.native_tongue_phase&&!in.native_tongue_cancelled_idle)||
   (in.native_tongue_cancelled_idle&&(cached_.buttons&tongue_button));
 }
 // Native23EA0/23DD0/23E18 reaches desiredDC0 exactly under neutral input.
 bool monster_stopped(const Input& in)const{return in.horizontal_speed_observed&&std::isfinite(in.horizontal_speed)&&in.horizontal_speed==0.f;}
 Output return_monster_combat(const Input& in){
  Output out{true};phase(in,Phase::ReturnCombat,"native_stocked_return_to_left_support");
  const float dx=in.preferred_x-in.x;
  if(in.combat_supported&&in.grounded&&std::abs(dx)<=8){
   out.x=walk(dx,3);if(std::abs(dx)<=3&&monster_stopped(in)){state_.monster_returning=false;phase(in,Phase::Approach,"native_left_support_arrived");}return out;
  }
  // No position, velocity or parent is manufactured. Release A first, then
  // ordinary A+direction crosses the real gap; actual support ends the route.
  // The two observed successful native returns needed repeated A epochs above
  // the combat platform, so do not turn the modulo-120 cadence into a single
  // edge. A failed third return fell to Y=465, well below native support at
  // Y=400, and repeating A there only bounced Yoshi on the unsafe lower floor.
  // Suppress those lower-floor pulses without faking support/landing/physics.
  const unsigned elapsed=(in.vi-state_.phase_vi)%120;if(elapsed<2)return out;
  out.x=walk(dx,3);
  if(in.y>420.f&&!in.combat_supported)return out;
  if(elapsed<47)out.buttons=jump_button;return out;
 }
 Output finish_monster_stock(const Input& in){
  if(monster_busy(in)){state_.monster_settle_count=0;return supply(in);}
  if(in.monster_upper_navigation&&(!in.native_tongue_observed||!in.native_tongue_known)){state_.monster_settle_count=0;return Output{true};}
  Output out{true};const float dx=in.supply_stand_x-in.x;
  const bool at_upper=in.grounded&&std::abs(dx)<=3&&in.y>=in.supply_stand_y-24&&in.y<=in.supply_stand_y+12&&(!in.monster_upper_navigation||in.monster_upper_supported);
  if(!at_upper){state_.monster_settle_count=0;
   if(std::abs(dx)<=8&&in.y<=in.supply_stand_y+12){out.x=walk(dx,3);return out;}
   return supply_route(in);
  }
  if(!monster_stopped(in)){state_.monster_settle_count=0;return out;}
  state_.monster_settle_count=state_.monster_settle_count&&in.vi==state_.monster_settle_vi+1?state_.monster_settle_count+1:1;state_.monster_settle_vi=in.vi;
  if(state_.monster_settle_count<2)return out;
  if(in.monster_upper_navigation){state_.monster_refilling=state_.monster_returning=false;state_.monster_upper_combat=true;state_.monster_settle_count=0;phase(in,Phase::Approach,"native_upper_support_stocked_combat");return out;}
  state_.monster_refilling=false;state_.monster_returning=true;state_.monster_settle_count=0;
  phase(in,Phase::ReturnCombat,"native_six_reserves_upper_support_settled");return out;
 }
 std::optional<Output> recover_fish_island(const Input& in){
  if(!in.fish_navigation||in.qa_bird)return {};
  // Native actions keep precedence even when the map witness is unavailable.
  if(in.aiming_actor||in.primary_state!=1||in.mouth_full||!in.native_tongue_known||!in.native_tongue_observed||in.native_tongue_live||in.native_tongue_callback_pending){state_.fish_settle_count=0;state_.fish_air_preparing=false;if(in.aiming_actor||in.primary_state==4)state_.fish_face_preparing=false;return {};}
  if(!in.fish_island_observed||!std::isfinite(in.fish_anchor_x)||in.fish_anchor_x<in.min_x||in.fish_anchor_x>in.max_x||!in.horizontal_speed_observed||!std::isfinite(in.horizontal_speed)||std::abs(in.horizontal_speed)>32||!in.native_stick_observed){state_.fish_settle_count=0;state_.fish_face_preparing=state_.fish_air_preparing=false;return Output{true};}
  if(!state_.fish_anchor_attested){
   if(!in.fish_on_island||!in.grounded||std::abs(in.x-in.fish_anchor_x)>12)return Output{true};
   state_.fish_anchor_attested=true;state_.fish_anchor_x=in.fish_anchor_x;state_.fish_anchor_y=in.y;
  }
  if(state_.fish_anchor_x!=in.fish_anchor_x){state_.fish_settle_count=0;state_.fish_face_preparing=state_.fish_air_preparing=false;return Output{true};}
  const float delta=state_.fish_anchor_x-in.x,vx=in.horizontal_speed;
  // Two stopped ground observations authorize this preparation cycle.
  // Its ordinary native4 facing input can create a small real velocity;
  // do not continually restart Face while that bounded motion remains on
  // the proved island. A new airborne/strong/outside displacement revokes it.
  if(state_.fish_face_preparing&&(state_.phase==Phase::Face||state_.phase==Phase::AcquireAim)){
   if(in.fish_on_island&&in.grounded&&std::abs(delta)<=12&&std::abs(vx)<=2){state_.fish_air_preparing=false;return {};}
   if(state_.fish_air_preparing&&fish_air_origin(in,true))return {};
   state_.fish_face_preparing=state_.fish_air_preparing=false;
  }
  const bool air_settled=fish_air_origin(in);
  const bool settled=(in.fish_on_island&&in.grounded&&std::abs(delta)<=8&&std::abs(vx)<=.25f)||air_settled;
  if(settled){
   state_.fish_settle_count=state_.fish_settle_count&&in.vi==state_.fish_settle_vi+1?state_.fish_settle_count+1:1;state_.fish_settle_vi=in.vi;
   if(state_.fish_settle_count<2){
    Output out{true};
    if(!in.native_stick.x&&!in.native_stick.y)return out;
    // Neutral host input can retain old filtered axes. Keep the first
    // quasi-stop observation at native0 instead of restarting counter54.
    for(const auto& candidate:boss_stick_aim0260::legal_candidates()){
     const auto axes=boss_stick_aim0260::native_axes(candidate.raw,in.native_stick);
     if(!axes.x&&!axes.y){out.x=candidate.host.x;out.y=candidate.host.y;break;}
    }
    return out;
   }
   const bool recovered=state_.fish_recovering;state_.fish_recovering=false;state_.fish_face_preparing=true;state_.fish_air_preparing=air_settled;
   if(recovered&&(state_.phase==Phase::Face||state_.phase==Phase::AcquireAim))phase(in,Phase::Approach,"fish_recovery_facing_refresh");
   return {};
  }
  state_.fish_settle_count=0;
  state_.fish_air_preparing=false;
  if(!state_.fish_recovering){state_.fish_recovering=true;state_.fish_recovery_vi=in.vi;}
  Output out{true};float direction=0;
  if(std::abs(delta)>8)direction=delta>0?1.f:-1.f;
  else if(vx!=0)direction=vx>0?-1.f:1.f;
  if(direction){if(const auto axes=boss_stick_aim0260::toward(direction,0,in.native_stick,54)){out.x=axes->host.x;out.y=axes->host.y;}}
  if(!in.grounded&&in.vi-state_.fish_recovery_vi<45)out.buttons=jump_button;
  // Keep the native shot's existing Cooldown phase/epoch. Neutral airborne
  // input sets acceleration0 and can retain C4; this countersteer is ordinary
  // input, not a velocity, pose, support or landing mutation.
  return out;
 }
 std::optional<float> fish_stopping_distance(const Input& in)const{
  if(!in.fish_navigation||in.qa_bird||!in.fish_island_observed||!state_.fish_anchor_attested||in.fish_anchor_x!=state_.fish_anchor_x||!in.horizontal_speed_observed||!in.fish_counter_model_observed||!std::isfinite(in.horizontal_speed)||std::abs(in.horizontal_speed)>32||!std::isfinite(in.fish_world_scale)||in.fish_world_scale<=0||in.fish_world_scale>4||!std::isfinite(in.fish_air_brake)||in.fish_air_brake<.1f||in.fish_air_brake>1)return {};
  float speed=std::abs(in.horizontal_speed),distance=speed*in.fish_world_scale;
  // One current-velocity update budgets ordinary B cancellation. Then use
  // the observed native full-counter acceleration, with an extra current
  // term before each decrement. This is a conservative stopping estimate,
  // not proof against another native push or a successful native landing.
  for(unsigned step=0;speed>0&&step<321;++step){distance+=speed*in.fish_world_scale;speed=std::max(0.f,speed-in.fish_air_brake);}
  return std::isfinite(distance)?std::optional<float>(distance):std::nullopt;
 }
 bool fish_drift_risk(const Input& in)const{
  const auto distance=fish_stopping_distance(in);if(!distance)return false;
  const float end=in.x+(in.horizontal_speed>0?*distance:in.horizontal_speed<0?-*distance:0);
  // Keep reserve inside the same native12px island band used by Approach.
  // Waiting until the physical last edge leaves no braking room for a push.
  return end<state_.fish_anchor_x-12||end>state_.fish_anchor_x+12;
 }
 void counter_fish_throw(const Input& in,Output& out){
  state_.fish_throw_counter=false;
  if(!in.controls||!in.alive||!in.combat||in.primary_state!=4||in.aiming_actor||in.reticle_actor||!in.fish_reticle_cleared||!in.native_stick_observed||in.mouth_full||!in.native_tongue_known||!in.native_tongue_observed||in.native_tongue_live||in.native_tongue_callback_pending||!fish_drift_risk(in))return;
  const float delta=state_.fish_anchor_x-in.x;float direction=delta>0?1.f:delta<0?-1.f:0;
  if(std::abs(delta)<=8&&in.horizontal_speed!=0)direction=in.horizontal_speed>0?-1.f:1.f;
  if(direction)if(const auto axes=boss_stick_aim0260::toward(direction,0,in.native_stick,54)){out.x=axes->host.x;out.y=axes->host.y;state_.fish_throw_counter=true;}
 }
 // Keep native164 travel independent of boss visibility. The old generic
 // Approach deadline could start aiming after180VI while still at X140,
 // before reaching the spring and the observed central standing anchor.
 std::optional<Output> invisible_travel(const Input& in){
  if(!in.invisible_navigation||in.aiming_actor)return {};
  const bool owned=std::any_of(in.targets.begin(),in.targets.end(),[](const Target& t){return t.kind==0x4599&&target_valid(t);});
  if(!owned||!in.has_preferred_x){phase(in,Phase::Idle,"invisible_anchor_unproved");return Output{true};}
  const float dx=in.preferred_x-in.x;
  if(std::abs(dx)<=3.f){state_.invisible_travelling=state_.invisible_travel_expired=false;return {};}
  if(!state_.invisible_travelling){state_.invisible_travelling=true;state_.invisible_travel_vi=in.vi;}
  if(state_.invisible_travel_expired||in.vi-state_.invisible_travel_vi>=1800){state_.invisible_travel_expired=true;phase(in,Phase::Idle,"invisible_anchor_travel_timeout");return Output{true};}
  // An attached rider and the existing bounded native detachment retain A
  // priority. This timer observes them but never forges a native release.
  if(in.spring_attached||state_.phase==Phase::SpringRelease)return {};
  phase(in,Phase::Approach,"invisible_anchor_travel");Output out{true};out.x=dx>0?.55f:-.55f;
  const bool upper=std::any_of(in.targets.begin(),in.targets.end(),[&](const Target& t){return t.kind==0x4599&&target_valid(t)&&t.y<in.y-100;});
  if(upper&&in.vi%120<65)out.buttons=jump_button;
  return out;
 }
 Output compute(const Input& in){
  Output out{true};
  if(!in.player||!in.combat||!in.controls||!in.alive||!point(in.x,in.y)||!point(in.min_x,in.max_x)||in.min_x>=in.max_x||unsigned(in.throw_mode)>1||(in.has_preferred_x&&(!std::isfinite(in.preferred_x)||in.preferred_x<in.min_x||in.preferred_x>in.max_x))){phase(in,Phase::Idle,"inactive_native_controls");state_.target=0;return out;}
  if((in.native_stick_observed&&!boss_stick_aim0260::valid(in.native_stick))||(in.has_supply_stand&&(!point(in.supply_stand_x,in.supply_stand_y)||in.supply_stand_x<in.min_x||in.supply_stand_x>in.max_x))){phase(in,Phase::Idle,"invalid_native_supply_observation");return out;}
  if(in.monster_navigation&&!in.aiming_actor&&(!in.has_supply_stand||!in.has_preferred_x||std::abs(in.preferred_x-312.f)>.01f||!in.horizontal_speed_observed||!std::isfinite(in.horizontal_speed)||std::abs(in.horizontal_speed)>64||in.reserves>6)){phase(in,Phase::Idle,"unproved_monster_navigation");return out;}
  if(in.monster_upper_navigation&&!in.monster_navigation){phase(in,Phase::Idle,"foreign_upper_navigation");return out;}
  if(const auto travel=invisible_travel(in))return *travel;
  // A native410A/469E keeps a rider attached until a fresh A edge. Release
  // earlier buttons first; never interrupt a real native4005 aim/throw.
  float escape_x=in.has_preferred_x?in.preferred_x:(in.min_x+in.max_x)*.5f;if(!in.has_preferred_x)island_center(in,escape_x);
  if(in.spring_attached&&!in.aiming_actor){
   phase(in,Phase::SpringRelease,"native_spring_rider");if(state_.spring_detached){state_.spring_detached=false;state_.spring_detach_vi=0;state_.phase_vi=in.vi;}out.x=walk(escape_x-in.x);
   const unsigned elapsed=(in.vi-state_.phase_vi)%30;if(elapsed>=2&&elapsed<6)out.buttons=jump_button;return out;
  }
  if(state_.phase==Phase::SpringRelease&&!in.aiming_actor){
   if(!state_.spring_detached){state_.spring_detached=true;state_.spring_detach_vi=in.vi;}
   if(in.vi-state_.spring_detach_vi<60&&(std::abs(escape_x-in.x)>8||!in.grounded)){
    out.x=walk(escape_x-in.x,8);const unsigned elapsed=in.vi-state_.phase_vi;if(elapsed>=2&&elapsed<30)out.buttons=jump_button;return out;
   }
  }
  if(state_.phase==Phase::SpringRelease)phase(in,Phase::Approach,"native_spring_release_observed");
  if(in.projectile_actor&&in.projectile_actor!=state_.last_projectile){
   state_.last_projectile=in.projectile_actor;
   if(state_.aim_seen){state_.launches++;state_.aim_seen=false;if(log_)std::fprintf(stderr,"[BossDriver] vi=%u launch-observed projectile=%08X attempts=%u launches=%u nativeObservation=1\n",in.vi,in.projectile_actor,state_.attempts,state_.launches);}
  }
  if(state_.phase==Phase::CancelAim){if(in.aiming_actor)return cancel_aim(in,"wait_native_aim_cancel");phase(in,Phase::Cooldown,"native_aim_cancel_observed");return out;}
  if(const auto recovery=recover_fish_island(in))return *recovery;
  if(const auto healing=heal_lips(in))return *healing;
  // Native4072 moves over gaps and tilting407D/F platforms. Its position
  // must not choose where Yoshi walks. Keep the bitmap-proved intro plateau
  // independently of temporary boss vulnerability or the shot cooldown.
  if(in.genie_navigation&&!in.aiming_actor){
   const bool owned=std::any_of(in.targets.begin(),in.targets.end(),[](const Target& t){return t.kind==0x4072&&target_valid(t);});
   if(!owned||!in.genie_support_observed||!std::isfinite(in.genie_anchor_x)||in.genie_anchor_x<in.min_x||in.genie_anchor_x>in.max_x){phase(in,Phase::Idle,"genie_static_support_unproved");return out;}
   // Empty stock needs the whole currently proved plateau, rather than the
   // shooting anchor's narrow island band. Native food/tongue handling stays
   // in supply(); the first observed reserve restores ordinary anchor priority.
   if(!in.reserves&&in.x>=in.min_x&&in.x<=in.max_x)return genie_supply(in);
   if(std::abs(in.x-in.genie_anchor_x)>8){phase(in,Phase::Approach,"genie_return_to_static_support");out.x=walk(in.genie_anchor_x-in.x,8);return out;}
  }
  // Replenishment belongs to this player/room/encounter, not the currently
  // vulnerable boss. Alternating native eyes must not restart its jump or
  // route epoch before a fresh A edge and the twelve-VI tongue delay. Real
  // 4005 and cancel transitions still take precedence; a live owned target
  // remains required, without requiring it to be vulnerable on every VI.
  if(in.monster_navigation&&!in.aiming_actor){
   const bool owned_monster=std::any_of(in.targets.begin(),in.targets.end(),[](const Target& t){return t.kind==0x41C6&&target_valid(t);});
   if(!owned_monster){phase(in,Phase::Idle,"no_live_owned_monster_for_navigation");return out;}
   // The CP6 terrain1 start is a real moving415F, not the removed middle
   // brick. A stocked player still reaches the retained proved static stand
   // by ordinary A+stick before acquiring aim; no landing is inferred here.
   if(in.monster_side_navigation&&!state_.monster_upper_combat&&!state_.monster_refilling&&!state_.monster_returning){state_.monster_returning=true;state_.monster_settle_count=0;}
   if(!in.reserves){state_.monster_refilling=true;state_.monster_returning=false;state_.monster_upper_combat=false;state_.monster_settle_count=0;}
   if(state_.monster_upper_combat&&!in.monster_upper_supported){
    state_.monster_upper_combat=false;state_.monster_settle_count=0;
    // In the scoped side-perch QA, a transient physical jump/foot loss is
    // navigation recovery. Remaining native eggs do not become a refill debt.
    if(in.monster_side_navigation&&in.reserves)state_.monster_returning=true;
    else state_.monster_refilling=true;
   }
   if(in.monster_side_navigation&&state_.monster_returning&&!state_.monster_refilling&&in.reserves)return finish_monster_stock(in);
   if(state_.monster_refilling){
    if(!state_.target)for(const auto& t:in.targets)if(t.kind==0x41C6&&target_valid(t)){state_.target=t.actor;break;}
    if(in.reserves<6)return supply(in);
    return finish_monster_stock(in);
   }
   if(state_.monster_returning)return return_monster_combat(in);
  }
  if(!in.reserves&&!in.aiming_actor&&(state_.phase==Phase::Supply||state_.phase==Phase::SupplyTravel)&&
     std::any_of(in.targets.begin(),in.targets.end(),[&](const Target& t){return target_valid(t,in.qa_bird);}))return supply(in);
  const Target* target=select(in);
  if(!target&&in.aiming_actor&&in.fish_navigation&&!in.qa_bird)return cancel_aim(in,"fish_no_proven_vulnerable_target");
  if(!target&&state_.lips_escaping&&!in.aiming_actor)for(const auto& t:in.targets)if(t.kind==0x40A3&&target_valid(t)&&t.actor==state_.lips_escape_root&&t.birth==state_.lips_escape_birth){target=&t;break;}
  if(!target&&in.aiming_actor){for(const auto& t:in.targets)if(target_valid(t,in.qa_bird)&&(t.kind==0x41C6||(t.kind==0x40A3&&in.native_stick_observed&&t.aim_radius>0)))return cancel_aim(in,t.kind==0x41C6?"monster_no_longer_vulnerable":"lips_no_longer_vulnerable");}
  if(!target){phase(in,Phase::Idle,"no_vulnerable_owned_target");state_.target=0;return out;}
  if(state_.target!=target->actor){
   const bool fish_tracking=in.fish_navigation&&!in.qa_bird&&in.aiming_actor&&state_.phase==Phase::TrackAim&&(target->kind==0x4511||target->kind==0x4631);
   state_.target=target->actor;if(fish_tracking)state_.aim_matches=0;else{phase(in,Phase::Approach,"select_native_target");state_.phase_vi=in.vi;}
  }
  if(in.fish_navigation&&!in.qa_bird&&in.aiming_actor&&(in.primary_state==1||in.primary_state==3)&&state_.phase==Phase::TrackAim&&in.horizontal_speed!=0&&(target->kind==0x4511||target->kind==0x4631)&&fish_drift_risk(in))return cancel_aim(in,"fish_aim_drift_braking_reserve");
  if(in.aiming_actor&&(target->kind==0x41C6||(target->kind==0x40A3&&in.native_stick_observed&&target->aim_radius>0))&&!target->clear_path)return cancel_aim(in,target->kind==0x41C6?"monster_shot_occluded":"lips_shot_occluded");
  if(in.aiming_actor){
   state_.aim_seen=true;
   if(state_.phase!=Phase::TrackAim&&state_.phase!=Phase::Release&&state_.phase!=Phase::SecondPress&&state_.phase!=Phase::Cooldown)phase(in,Phase::TrackAim,"native_4005_observed");
  }
  if(state_.phase==Phase::Release){
   aim_axes(in,*target,out);
   if(in.vi-state_.phase_vi>=2){if(in.throw_mode==ThrowMode::SecondPress&&in.aiming_actor&&fish_shot_allowed(in)){phase(in,Phase::SecondPress,"second_native_Z_edge");out.buttons=egg_button;}else phase(in,Phase::Cooldown,"native_release_sent");}return out;
  }
  if(state_.phase==Phase::SecondPress){aim_axes(in,*target,out);if(in.vi-state_.phase_vi<2){if(fish_shot_allowed(in))out.buttons=egg_button;}else phase(in,Phase::Cooldown,"second_native_Z_sent");return out;}
  if(state_.phase==Phase::Cooldown){const bool fish_prepare=in.fish_navigation&&!in.qa_bird&&!in.aiming_actor&&!in.reticle_actor&&state_.fish_face_preparing&&in.native_egg_request_observed&&in.native_egg_request_ready&&in.fish_evade_observed&&in.fish_evade_head_clear;if(in.vi-state_.phase_vi>=cooldown_duration(in)||fish_prepare)phase(in,Phase::Approach,fish_prepare?"fish_prepare_before_release_deadline":"observe_then_retry");return out;}
  if(state_.phase==Phase::TrackAim){
   if(!in.aiming_actor){phase(in,Phase::Cooldown,"aim_ended_naturally");return out;}
   aim_axes(in,*target,out);if(in.throw_mode==ThrowMode::HoldAndRelease)out.buttons=egg_button;
   if(target->kind==0x41C6&&target->monster_burst_observed&&target->monster_burst_passed)return cancel_aim(in,"monster_native_burst_range_passed");
   // Lips air releases can take eight VI before405C8: Yoshi and4003 move
   // during that native animation. Prefer a resolved grounded launch origin.
   const bool stable_origin=target->kind!=0x40A3||!in.native_stick_observed||in.grounded;
   state_.aim_matches=stable_origin&&aligned(in,*target)?state_.aim_matches+1:0;
   const unsigned required=target->kind==0x41C6&&target->monster_burst_observed?1u:3u;
   if(state_.aim_matches>=required||in.vi-state_.phase_vi>=config_.aim_timeout_vi){if(!reticle_reaches(in,*target))return cancel_aim(in,"native_reticle_range_too_short");if(state_.aim_matches<required)return cancel_aim(in,target->kind==0x41C6?"monster_precision_retry":target->kind==0x40A3?"lips_precision_retry":"native_aim_not_aligned");if(in.fish_navigation&&!in.qa_bird&&in.vi<state_.fish_release_not_before)return out;phase(in,Phase::Release,"native_reticle_aligned_and_reaching");out.buttons=0;}return out;
  }
  if(!in.reserves)return supply(in);
  if(target->kind==0x40A3&&target->observation_actor&&target->birth&&in.native_stick_observed&&!in.aiming_actor){
   const bool identity=in.lips_escape_root==target->actor&&in.lips_escape_birth==target->birth;
   if(state_.lips_escaping&&(state_.lips_escape_root!=target->actor||state_.lips_escape_birth!=target->birth||!identity||in.vi-state_.lips_escape_vi>=240))state_.lips_escaping=false;
   const bool observed=identity&&in.lips_escape_observed&&std::isfinite(in.lips_escape_x)&&in.lips_escape_x>=in.min_x&&in.lips_escape_x<=in.max_x&&(in.lips_escape_direction==1||in.lips_escape_direction==-1);
   if(!state_.lips_escaping&&observed&&!target->clear_path){state_.lips_escaping=true;state_.lips_escape_root=target->actor;state_.lips_escape_birth=target->birth;state_.lips_escape_x=in.lips_escape_x;state_.lips_escape_direction=in.lips_escape_direction;state_.lips_escape_vi=in.vi;phase(in,Phase::SeekClear,"lips_cross_above_body");}
   if(state_.lips_escaping){
    if(observed&&in.lips_escape_direction==state_.lips_escape_direction&&float(state_.lips_escape_direction)*(in.lips_escape_x-state_.lips_escape_x)>0)state_.lips_escape_x=in.lips_escape_x;
    if(float(state_.lips_escape_direction)*(in.x-state_.lips_escape_x)>=0){state_.lips_escaping=false;phase(in,Phase::Approach,"lips_other_side_observed");return out;}
    // Keep the destination even when crossing the boss centre or when a shot
    // briefly becomes clear. Current geometry must license EACH new X input.
    if(observed&&in.lips_escape_direction==state_.lips_escape_direction)out.x=float(state_.lips_escape_direction);
    if(in.lips_jump_clearance_observed&&in.lips_jump_column_clear)out.buttons=jump_button;
    return out;
   }
  }
  if(state_.phase==Phase::Supply||state_.phase==Phase::SupplyTravel||state_.phase==Phase::Idle)phase(in,Phase::Approach,"native_reserve_available");
  if(state_.phase==Phase::SeekClear&&target->clear_path)phase(in,Phase::Face,target->kind==0x40A3?"lips_clear_ray_observed":"monster_clear_ray_observed");
  const float dx=target->x-in.x;
  if((state_.phase==Phase::Approach||state_.phase==Phase::Face||state_.phase==Phase::AcquireAim)&&island_guard(in,out)){state_.phase_vi=in.vi;return out;}
  if(state_.phase==Phase::Approach){
   // This exact native Lips observation already proved a usable current
   // origin, or needs clearance here. Generic stand_off150 can walk it into
   // Bongo and trap it against the wall instead of taking the available shot.
   if(target->kind==0x40A3&&target->observation_actor&&in.native_stick_observed&&target->aim_radius>0)phase(in,Phase::Face,target->clear_path?"lips_current_origin_safe":"lips_clearance_at_current_origin");
   else{
   const float side=dx>=0?1.f:-1.f;float wanted=state_.monster_upper_combat?in.supply_stand_x:in.has_preferred_x?in.preferred_x:std::clamp(target->x-side*config_.stand_off,in.min_x,in.max_x);
   float safe_distance=1e30f;for(float at:in.stable_stands)if(std::isfinite(at)&&at>=in.min_x&&at<=in.max_x){const float distance=std::abs(at-in.x);if(distance<safe_distance){safe_distance=distance;wanted=at;}}
   const float move=wanted-in.x;
   // Native facing can drift from248 to251.010086 after a real egg shot.
   // The actual side support/feet witness already proves its8px safe band;
   // a3.01px correction must not start an unrelated above-target A jump.
   const bool side_standing=in.monster_side_navigation&&state_.monster_upper_combat&&in.monster_upper_supported&&in.grounded;
   const float tolerance=side_standing?8.f:in.has_preferred_x?3.f:16.f;
   if(std::abs(move)>tolerance&&in.vi-state_.phase_vi<180){out.x=move>0?.55f:-.55f;if(target->y<in.y-100&&in.vi%120<65)out.buttons=jump_button;return out;}
   phase(in,Phase::Face,"native_shooting_position");
   }
  }
  if((target->kind==0x41C6||(target->kind==0x40A3&&in.native_stick_observed&&target->aim_radius>0))&&!target->clear_path){
   if(target->kind==0x41C6&&state_.monster_upper_combat){phase(in,Phase::SeekClear,"native_upper_combat_wait_for_eye");return out;}
   const bool lips=target->kind==0x40A3;phase(in,Phase::SeekClear,lips?"lips_jump_for_clear_ray":"monster_jump_for_clear_ray");
   if(lips){
    if(!in.lips_jump_clearance_observed||!std::isfinite(in.lips_jump_x)||in.lips_jump_x<in.min_x||in.lips_jump_x>in.max_x)return out;
    if(!in.lips_jump_column_clear){out.x=walk(in.lips_jump_x-in.x,0);return out;}
    // Native57BA0->2F96C repeatedly pushes a touching player by +/-3.
    // A proven free head-probe column must not wait forever for exact C4=0.
    if(in.grounded&&(!in.horizontal_speed_observed||!std::isfinite(in.horizontal_speed)))return out;
   }else if(in.has_preferred_x)out.x=walk(in.preferred_x-in.x);
   if((in.vi-state_.phase_vi)%120<45)out.buttons=jump_button;
   return out;
  }
  if(state_.phase==Phase::Face){
   if(in.primary_state!=1||(in.native_egg_request_observed&&!in.native_egg_request_ready))return out;
   if(in.vi-state_.phase_vi<config_.face_vi){if(in.native_stick_observed){if(auto face=boss_stick_aim0260::toward(dx>=0?1.f:-1.f,0,in.native_stick,4)){out.x=face->host.x;out.y=face->host.y;}}else out.x=(dx>=0?1.f:-1.f)*.35f;return out;}
   phase(in,Phase::AcquireAim,"first_native_Z_edge");state_.attempts++;
  }
  if(state_.phase==Phase::AcquireAim){
   // A rejected Z edge is not retried by holding Z after native3BE44 opens.
   // Release while the observed native gate is closed, then return through
   // Face so a later admitted request is a fresh edge. Existing4005 wins above.
   if(in.native_egg_request_observed&&!in.native_egg_request_ready){phase(in,Phase::Face,"native_egg_request_gate_closed");return out;}
  out.buttons=egg_button;
  // Creating native4005 needs only the Z edge. Directional reticle control
  // begins after that actor is actually observed; it must not turn/move P1
  // during the initial creation request on a small Fish island.
  if(!in.fish_navigation||in.qa_bird)aim_axes(in,*target,out);
   if(in.vi-state_.phase_vi>=config_.acquire_timeout_vi){phase(in,in.native_egg_request_observed?Phase::Face:Phase::Release,in.native_egg_request_observed?"native_aim_fresh_edge_retry":"native_aim_not_observed");out.buttons=0;}
  }
  return out;
 }
 void evade_fish_jump(const Input& in,Output& out,uint16_t previous_buttons){
  const bool water_hold=state_.fish_evading&&state_.fish_evade_water&&in.vi>=state_.fish_evade_vi&&in.vi-state_.fish_evade_vi<45;
  const bool owned=in.fish_navigation&&!in.qa_bird&&in.combat&&in.controls&&in.alive&&out.handled&&in.fish_island_observed&&state_.fish_anchor_attested&&in.fish_anchor_x==state_.fish_anchor_x&&in.fish_evade_observed&&in.fish_evade_head_clear&&(in.fish_evade_generation_valid||water_hold);
  const bool mouth=in.mouth_full||!in.native_tongue_known||!in.native_tongue_observed||in.native_tongue_live||in.native_tongue_callback_pending;
  if(!owned||mouth||(out.buttons&tongue_button)||(in.primary_state!=1&&in.primary_state!=3&&in.primary_state!=4)){
   state_.fish_evading=state_.fish_evade_pending_z=state_.fish_evade_sending_z=state_.fish_evade_acquire_z=false;return;
  }
  if((state_.fish_evading||state_.fish_evade_pending_z||state_.fish_evade_sending_z)&&in.aiming_actor&&state_.fish_evade_aim&&in.aiming_actor!=state_.fish_evade_aim){state_.fish_evading=state_.fish_evade_pending_z=state_.fish_evade_sending_z=state_.fish_evade_acquire_z=false;return;}
  if(water_hold&&!state_.fish_evade_aim&&in.aiming_actor)state_.fish_evade_aim=in.aiming_actor;
  const bool threat_identity=in.fish_threat_water?(in.fish_threat_resource&&in.fish_threat_private):bool(in.fish_threat_birth);
  const bool water_without_aim=in.fish_threat_water&&in.primary_state==1&&!in.aiming_actor&&!in.reticle_actor&&in.fish_reticle_cleared;
  const bool new_water=in.fish_threat_water&&state_.fish_evade_water&&(in.fish_threat_actor!=state_.fish_evade_actor||in.fish_threat_resource!=state_.fish_evade_resource||in.fish_threat_private!=state_.fish_evade_private);
  // A second independently revalidated4632 may arrive before the old60VI
  // retry timer. Preserve the complete18VI gesture interval; the same water
  // identity and every Fish-body gesture retain the original60VI minimum.
  const unsigned rearm=new_water?18u:60u;
  if(!state_.fish_evading&&in.fish_evade_generation_valid&&(in.aiming_actor||water_without_aim)&&in.fish_evade_imminent&&in.fish_threat_actor&&threat_identity&&(!state_.fish_evade_attempted||in.vi-state_.fish_evade_vi>=rearm)){
   if(previous_buttons&jump_button){out.buttons&=uint16_t(~jump_button);return;}
   state_.fish_evading=state_.fish_evade_attempted=true;state_.fish_evade_vi=in.vi;
   state_.fish_evade_actor=in.fish_threat_actor;state_.fish_evade_birth=in.fish_threat_birth;state_.fish_evade_aim=in.aiming_actor;
   state_.fish_evade_water=in.fish_threat_water;state_.fish_evade_resource=in.fish_threat_resource;state_.fish_evade_private=in.fish_threat_private;
   if(log_)std::fprintf(stderr,"[BossFishEvade] vi=%u room=%u encounter=%u root=%08X birth=%llu aim=%08X threatStep=%u holdVI=%u mode=%u water4632=%u resource=%08X private=%08X ordinaryA=1 controlsOnly=1\n",in.vi,in.room,in.encounter,in.fish_threat_actor,static_cast<unsigned long long>(in.fish_threat_birth),in.aiming_actor,in.fish_threat_step,state_.fish_evade_water?45u:18u,unsigned(in.throw_mode),unsigned(in.fish_threat_water),in.fish_threat_resource,in.fish_threat_private);
  }
  // Complete only the already-admitted bounded water gesture after a4632
  // passes/retires/latches. Current player/epoch/source/map/head/mouth/aim
  // remain required; no vanished projectile is authority for a new start.
  const bool hold=state_.fish_evading&&in.vi-state_.fish_evade_vi<(state_.fish_evade_water?45u:18u);
  if(state_.fish_evading&&!hold)state_.fish_evading=false;
  const bool front=hold&&in.vi==state_.fish_evade_vi;
  if(hold)out.buttons|=jump_button;
  // With no4005, a fresh A also precedes the INITIAL BE44 Z request in
  // BOTH modes. Re-issue that ordinary request only on a later fresh edge
  // with its real native admission observed; never invent an acquired aim.
  if(front&&!in.aiming_actor&&(out.buttons&egg_button)){
   out.buttons&=uint16_t(~egg_button);state_.fish_evade_acquire_z=true;state_.fish_evade_acquire_vi=in.vi;
  }
  if(state_.fish_evade_acquire_z){
   if(in.aiming_actor)state_.fish_evade_acquire_z=false;
   else {out.buttons&=uint16_t(~egg_button);
    if(in.vi-state_.fish_evade_acquire_vi>=60)state_.fish_evade_acquire_z=false;
    else if(!front&&!(previous_buttons&egg_button)&&in.native_egg_request_observed&&in.native_egg_request_ready){out.buttons|=egg_button;state_.fish_evade_acquire_z=false;}
   }
  }
  // Native415FC handles a fresh A before3BEC8. In second-press mode a Z
  // edge on that same update would be lost. Deliver it later as a fresh
  // ordinary edge only while the actual4005/reticle window is still valid.
  if(front&&in.aiming_actor&&in.throw_mode==ThrowMode::SecondPress&&(out.buttons&egg_button)){
   out.buttons&=uint16_t(~egg_button);state_.fish_evade_pending_z=true;state_.fish_evade_z_vi=in.vi;
  }
  if(!in.aiming_actor){state_.fish_evade_pending_z=state_.fish_evade_sending_z=false;return;}
  if(state_.fish_evade_pending_z){
   out.buttons&=uint16_t(~egg_button);
   if(in.vi-state_.fish_evade_z_vi>=60){state_.fish_evade_pending_z=false;state_.fish_evading=false;out=cancel_aim(in,"fish_jump_second_Z_window_expired");return;}
   const Target* target=nullptr;for(const auto& t:in.targets)if(t.actor==state_.target&&target_valid(t)&&t.vulnerable){target=&t;break;}
   if(!front&&!(previous_buttons&egg_button)&&fish_shot_allowed(in)&&target&&aligned(in,*target)&&reticle_reaches(in,*target)){
    state_.fish_evade_pending_z=false;state_.fish_evade_sending_z=true;state_.fish_evade_z_vi=in.vi;
   }
  }
  if(state_.fish_evade_sending_z){if(!fish_shot_allowed(in)){out.buttons&=uint16_t(~egg_button);state_.fish_evade_sending_z=false;}else if(in.vi-state_.fish_evade_z_vi<2)out.buttons|=egg_button;else {out.buttons&=uint16_t(~egg_button);state_.fish_evade_sending_z=false;}}
 }
public:
 bool configure(bool enabled,bool diagnostic,const Config& config={},bool log=true){state_={};cached_={};release_trace_={};release_logs_=heal_logs_=0;config_=config;log_=log;if(!config.valid())return false;state_.enabled=enabled;state_.diagnostic=diagnostic;return !enabled||diagnostic;}
 const Snapshot& snapshot()const{return state_;}
 const ReleaseTrace& release_trace()const{return release_trace_;}
 // A separate ordinary-input swim pilot may own several hundred VI. Refresh
 // navigation only on its observed landing; preserve every shot counter,
 // cooldown, release serial and native provenance witness.
 bool resume_fish_navigation(const Input& in){
  if(!state_.enabled||!state_.diagnostic||!in.fish_navigation||in.qa_bird||!in.combat||!in.controls||!in.alive||!in.fish_island_observed||!in.fish_on_island||!in.grounded||!in.horizontal_speed_observed||std::abs(in.horizontal_speed)>.25f||!in.native_tongue_known||in.native_tongue_live||in.native_tongue_callback_pending||in.mouth_full||in.aiming_actor||in.reticle_actor||state_.player!=in.player||state_.room!=in.room||state_.encounter!=in.encounter||state_.fish_retry_epoch!=in.fish_retry_epoch||!state_.fish_anchor_attested||state_.fish_anchor_x!=in.fish_anchor_x)return false;
  state_.phase=Phase::Approach;state_.phase_vi=in.vi;state_.target=0;state_.aim_seen=false;state_.aim_matches=0;
  state_.fish_recovering=state_.fish_face_preparing=state_.fish_air_preparing=false;state_.fish_settle_count=0;
  state_.fish_evading=state_.fish_evade_pending_z=state_.fish_evade_sending_z=state_.fish_evade_acquire_z=false;
  state_.fish_shot_authorized=false;state_.have_vi=false;cached_={};return true;
 }
 // The scoped bird-flight helper only hands back after actual home feet,
 // idle mouth and two stopped observations. Forget stale supply gestures;
 // retain the genuine empty-stock refill debt and every shot/receipt counter.
 bool resume_monster_side_refill(const Input& in){
  if(!state_.enabled||!state_.diagnostic||!in.monster_side_navigation||!in.monster_upper_navigation||!in.monster_upper_supported||!in.grounded||!in.horizontal_speed_observed||in.horizontal_speed!=0||!in.native_tongue_observed||!in.native_tongue_known||in.native_tongue_live||in.native_tongue_callback_pending||in.mouth_full||in.aiming_actor||in.reticle_actor||state_.player!=in.player||state_.room!=in.room||state_.encounter!=in.encounter||!state_.monster_refilling||in.reserves>6)return false;
  state_.phase=Phase::Supply;state_.phase_vi=in.vi;state_.monster_returning=state_.monster_upper_combat=false;state_.monster_settle_count=state_.monster_arrival_count=0;
  state_.supply_ready=state_.supply_lower_ready=state_.supply_recovery=false;state_.jump_active=state_.supply_jump_grounded=false;state_.supply_actor=0;state_.have_vi=false;cached_={};return true;
 }
 Output step(const Input& in){
  if(!state_.enabled||!state_.diagnostic)return {};
  if(state_.player!=in.player||state_.room!=in.room||state_.encounter!=in.encounter||(in.fish_navigation&&state_.fish_retry_epoch!=in.fish_retry_epoch)||(state_.have_vi&&in.vi<state_.last_vi))fresh(in);
  if(state_.fish_shot_authorized&&state_.fish_authorized_aim!=in.aiming_actor){state_.fish_shot_authorized=false;if(state_.have_vi&&state_.last_vi==in.vi)cached_.buttons&=uint16_t(~egg_button);}
  if(state_.have_vi&&state_.last_vi==in.vi)return cached_;
  state_.have_vi=true;state_.last_vi=in.vi;auto out=compute(in);counter_fish_throw(in,out);evade_fish_jump(in,out,cached_.buttons);cached_=out;return cached_;
 }
 bool drive(const Input& in,uint16_t* buttons,float* x,float* y){if(!buttons||!x||!y)return false;const auto out=step(in);if(!out.handled)return false;*buttons=out.buttons;*x=out.x;*y=out.y;return true;}
};
}
