#pragma once
#include "boss_driver0260.hpp"
#include "boss_native_egg_aim0284.hpp"
#include "boss_monster_aim0260.hpp"
#include "boss_monster_burst0260.hpp"
#include "boss_monster_upper0260.hpp"
#include "boss_lips_aim0260.hpp"
#include "boss_lips_jump0260.hpp"
#include "boss_lips_corridor0260.hpp"
#include "boss_egg_shape0260.hpp"
#include "boss_lips_target0260.hpp"
#include "boss_native_mouth0260.hpp"
#include "boss_lips_supply0260.hpp"
#include "boss_bird_test0260.hpp"
#include "boss_lips_motion0260.hpp"
#include "boss_lips_body0260.hpp"
#include "boss_lips_escape0260.hpp"
#include "boss_genie_support0260.hpp"
#include "boss_genie_aim0260.hpp"
#include "boss_fish_support0260.hpp"
#include "boss_fish_evade0260.hpp"
#include "boss_fish_water_evade0260.hpp"
#include "boss_fish_aim0260.hpp"
#include "boss_fish_shot_trace0260.hpp"
#include "boss_fish_water_recovery0260.hpp"
#include "boss_monster_forage0284.hpp"
namespace workshop::boss_driver_bridge0260 {
// Read-only native source observations, also exercised by isolated8MiB fixtures.
inline uint16_t egg_food_flags(uint8_t* rdram,uint32_t actor){
 namespace n=boss_native;
 if(!rdram||(!n::actor(rdram,actor,0x4526)&&!n::actor(rdram,actor,0x46DA))||MEM_BU(0x17D,n::base(actor))!=2||MEM_BU(0x18,n::base(actor))==0xFF)return 0;
 const auto metadata=n::word(rdram,actor+0x174);if(!n::range(metadata,4))return 0;const auto definition=n::word(rdram,metadata);if(!n::range(definition,0x20))return 0;
 return uint16_t(MEM_HU(0x1C,n::base(definition)));
}
inline bool observe_feeder(uint8_t* rdram,uint32_t actor,float player_y,boss_driver0260::Supply& out){
 namespace n=boss_native;namespace d=boss_driver0260;
 if(!rdram||!n::actor(rdram,actor,0x456E)||MEM_BU(0x17D,n::base(actor))!=2||MEM_BU(0x18,n::base(actor))!=1)return false;
 const auto sub=MEM_BU(0x30,n::base(actor));if(sub!=0xA&&sub!=0xB)return false;
 const auto p=n::word(rdram,actor+0x1F4);if(!n::range(p,6)||MEM_HU(0,n::base(p))!=0x4528||MEM_HU(4,n::base(p))!=0)return false;
 const auto x=std::bit_cast<float>(n::word(rdram,actor+0x84)),y=std::bit_cast<float>(n::word(rdram,actor+0x88));
 if(!d::point(x,y)||!std::isfinite(player_y)||std::abs(y-player_y)>=128)return false;
 out={actor,0x456E,x,y,true,false,0,0x4528,uint16_t(MEM_HU(2,n::base(p))),0};return true;
}
inline bool observe_spring_attachment(uint8_t* rdram,uint32_t player){
 namespace n=boss_native;
 // Native48114 removes the rider from the active queue through444D0/
 //43C94 (phase0), then47F9C installs the reciprocal sparse child slot.
 if(!rdram||!n::actor(rdram,player,0x4001)||n::word(rdram,0x800F8828)!=player||MEM_BU(0x17D,n::base(player))!=0)return false;
 const auto parent=n::word(rdram,player+0x180);
 if((!n::actor(rdram,parent,0x410A)&&!n::actor(rdram,parent,0x469E))||MEM_BU(0x17D,n::base(parent))!=2)return false;
 const auto slot=MEM_HU(0x17E,n::base(player)),count=MEM_HU(0x184,n::base(parent));
 return slot<20&&count>0&&count<=20&&n::word(rdram,parent+0x188+4*slot)==player;
}
struct EggRequestObservation {bool observed=false,ready=false;};
// Read-only gates from native3BE44/3BEC8.12C9 is a flag, not a guessed
// cooldown; primary1 may be airborne. Sub30hex is not excluded by3BE44.
inline EggRequestObservation observe_egg_request(uint8_t* rdram,uint32_t player){
 namespace n=boss_native;
 if(!rdram||!n::actor(rdram,player,0x4001)||n::word(rdram,0x800F8828)!=player)return {};
 const auto p=n::word(rdram,player+0x1F4);if(!n::range(p,0x12CA))return {};
 return {true,n::byte(rdram,player+0x18)==1&&n::byte(rdram,player+0x30)!=0x14&&n::byte(rdram,p+0x12C9)==0&&n::byte(rdram,p+0x1F8)==0};
}
// Caller scopes this observation to the configured native151 Monster arena.
// A real red415F parent can move vertically; the static terrain2 support is
// independently checked at its actual400px surface, never at guessed P1Y385.
inline bool observe_monster_combat_support(uint8_t* rdram,uint32_t player){
 namespace n=boss_native;
 if(!rdram||!n::actor(rdram,player,0x4001)||n::word(rdram,0x800F8828)!=player)return false;
 const auto phase=MEM_BU(0x17D,n::base(player));if((phase!=0&&phase!=2)||MEM_BU(0x18,n::base(player))==0xFF)return false;
 const auto p=n::word(rdram,player+0x1F4);if(!n::range(p,0xF5)||MEM_BU(0xF4,n::base(p))!=0)return false;
 const float x=std::bit_cast<float>(n::word(rdram,player+0x84)),y=std::bit_cast<float>(n::word(rdram,player+0x88));if(!boss_driver0260::point(x,y)||std::abs(x-312.f)>8)return false;
 const auto parent=n::word(rdram,player+0x180);
 if(parent){
  if(!n::actor(rdram,parent,0x415F)||MEM_BU(0x17D,n::base(parent))!=2||MEM_BU(0x18,n::base(parent))==0xFF)return false;
  const float px=std::bit_cast<float>(n::word(rdram,parent+0x84)),py=std::bit_cast<float>(n::word(rdram,parent+0x88));
  const auto slot=MEM_HU(0x17E,n::base(player)),count=MEM_HU(0x184,n::base(parent));
  return std::isfinite(px)&&std::isfinite(py)&&std::abs(px-312.f)<=.5f&&std::abs(y-py)<=48&&slot<20&&count>0&&count<=20&&n::word(rdram,parent+0x188+4*slot)==player;
 }
 if(y<376||y>392)return false;const auto map=boss_monster_aim0260::Map::read(rdram);if(!map||map->columns!=3||map->rows!=2)return false;
 for(float probe:{x-2,x,x+2}){const auto above=map->occupied(rdram,probe,399),top=map->occupied(rdram,probe,400),below=map->occupied(rdram,probe,402);if(!above||*above||!top||!*top||!below||!*below)return false;}
 return true;
}
inline void observe_monster_upper_navigation(uint8_t* ram,uint32_t player,unsigned room,unsigned encounter,boss_driver0260::Input& in){
 in.monster_upper_navigation=true;in.monster_upper_supported=boss_monster_upper0260::observe(ram,player,room,encounter);
}
// variants::bind assigns actor/birth for ordinary Lips, but intentionally does
// not initialize Binding.private_data. The native family witness owns that
// private identity. Require both independent bindings before reading40A7.
inline std::optional<boss_lips_aim0260::WeakPoint> observe_lips_member(uint8_t* rdram,unsigned member){
 namespace c=boss_campaign0250;namespace core=boss_battle0250;namespace family=boss_family_native0250;
 const auto& state=c::session.snapshot();const auto& variant=c::variants::snapshot();
 if(!rdram||!c::active()||!c::driver||!state.diagnostic||state.phase!=core::Phase::Fighting||member>=c::session.current().count||member>=4||c::session.current().members[member].kind!=0x40A3||(state.defeated&(1u<<member))||!variant.enabled||variant.invalidated||variant.encounter!=state.index||variant.room!=c::logical_room)return {};
 const auto root=c::bound[member];const auto& binding=variant.members[member];const auto& observer=c::observers[member];const auto& proof=observer.snapshot();const auto& actor=proof.actors[0];
 if(!root||binding.actor!=root||!binding.birth||observer.bound_actor()!=root||!proof.enabled||proof.invalidated||proof.family!=family::Family::giant_lips||proof.source_room!=c::logical_room||proof.run_token!=state.token||proof.expected_actors!=1||proof.registered_actors!=1||!actor.initialized||actor.disposed||actor.completion_returned||actor.actor!=root||!actor.private_data)return {};
 return boss_lips_aim0260::observe(rdram,root,actor.private_data,binding.birth);
}
// F7EA6 is a scene context, not native egg-input admission. The three native
// observer families can
// leave a genuinely active encounter at0/0; accept only the Runtime's exact
// witnessed baseline with the same released singleton and bound native actor.
inline bool observe_controls(uint8_t* rdram,uint32_t player){
 namespace c=boss_campaign0250;namespace n=boss_native;namespace core=boss_battle0250;
 if(!rdram)return false;
 if(n::byte(rdram,0x800F7EA6)==1)return true;
 const auto& state=c::session.snapshot();
 if(!c::active()||!c::driver||!state.diagnostic||state.phase!=core::Phase::Fighting||!c::flags_ready||c::restore_pending||c::flags5||c::flags6||n::byte(rdram,0x800F7EA5)||n::byte(rdram,0x800F7EA6))return false;
 if(player!=state.player||!c::witnesses::active_player(rdram,player)||n::word(rdram,0x800F8828)!=player||n::byte(rdram,player+0x17D)!=2||n::byte(rdram,player+0x165)||n::byte(rdram,player+0x18)==0||n::byte(rdram,player+0x18)==0xFF)return false;
 const auto p=n::word(rdram,player+0x1F4);if(!n::range(p,0x12F0)||n::word(rdram,p+0x12EC))return false;
 return c::witnessed_zero_controls(rdram,player);
}
inline boss_native_egg_aim0284::Observation observe_egg_aim(uint8_t* rdram,uint32_t player){return boss_native_egg_aim0284::observe(rdram,player);}
using boss_native_mouth0260::observe_native_tongue_ready;
using boss_native_mouth0260::observe_native_mouth;
inline bool cp6_monster_side_qa=false;
inline void configure_cp6_monster_side_qa(bool enabled){cp6_monster_side_qa=enabled;}
inline bool cp6_monster_side_scope(const boss_battle0250::Snapshot& state,const boss_battle0250::Encounter& fight,const boss_rooms_data0250::Arena& arena,uint64_t seed,unsigned logical,unsigned physical,bool driver){
 return cp6_monster_side_qa&&state.diagnostic&&state.phase==boss_battle0250::Phase::Fighting&&driver&&!state.token.empty()&&seed==37784098137700ull&&state.index==1&&state.completed==1&&!state.retry_epoch&&logical==151&&physical==168&&arena.source==168&&arena.logical_source==151&&fight.family==boss_battle0250::Family::Monsters&&fight.variant==0&&fight.terrain==1&&fight.count==2&&fight.members[0].kind==0x41C6&&fight.members[1].kind==0x41C6;
}
inline boss_driver0260::Controller controller;
inline boss_monster_forage0284::Controller monster_side_forage;
inline boss_bird_test0260::Controller bird_test;
inline boss_fish_water_recovery0260::Controller fish_water_return;
inline unsigned fish_water_logs=0;inline boss_fish_water_recovery0260::Stage fish_water_last=boss_fish_water_recovery0260::Stage::Idle;
inline bool qa_bird_complete(){return bird_test.complete();}
inline bool qa_bird_failed(){return bird_test.snapshot().phase==boss_bird_test0260::Phase::Failed;}
inline boss_monster_aim0260::History monster_history;
inline boss_lips_motion0260::History lips_history;
// Diagnostic-only, bounded read-only samples between the old 120-VI snapshots.
inline unsigned monster_return_motion_logs=0;
inline unsigned lips_jump_logs=0,genie_supply_log_windows=0,monster_contact_logs=0;
struct FishPilotTrace {uint32_t player=0;unsigned room=177,encounter=10,epoch=0,first_vi=0,last_vi=0,samples=0;bool have=false;};
inline FishPilotTrace fish_pilot_trace;
// Bounded diagnostic journal only. Native404C8 entry is BEFORE velocity is
// calculated: a0 is the created4002 projectile and a1 the source4001. A later
// read-only sample records actual output fields; no native function is invoked.
struct NativeLaunchTrace {
 bool pending=false;unsigned serial=0,vi=0,room=177,encounter=0,release_serial=0;
 uint32_t player=0,egg=0,private_data=0,resource=0;
};
inline std::array<NativeLaunchTrace,8> native_launch_traces{};
inline unsigned native_launch_logs=0,native_launch_first_logs=0;
inline boss_fish_shot_trace0260::Trace fish_shots;
inline void reset_launch_trace(){native_launch_traces={};native_launch_logs=native_launch_first_logs=0;fish_shots.reset();}
inline bool native_trace_enabled(){namespace c=boss_campaign0250;return c::enabled&&c::driver&&c::session.snapshot().diagnostic;}
inline boss_fish_shot_trace0260::Frame fish_shot_frame(){
 namespace c=boss_campaign0250;const auto& s=c::session.snapshot();
 return {native_trace_enabled()&&c::active()&&s.phase==boss_battle0250::Phase::Fighting&&c::session.current().family==boss_battle0250::Family::Fish&&!c::restore_pending,s.player,c::logical_room,c::physical_room,s.index,s.retry_epoch,s.token};
}
// Shared by the actual drive path and capture-backed composition checks.
// Both the prospective short ray and the CURRENT reticle use the151 map.
inline void plan_monster_burst(uint8_t* ram,const boss_monster_burst0260::Scene& s,const boss_driver0260::Input& in,boss_driver0260::Target& target,boss_monster_burst0260::Result& result){
 namespace d=boss_driver0260;namespace burst=boss_monster_burst0260;
 target.monster_burst_observed=true;target.monster_burst_ready=target.monster_burst_passed=false;result={};
 float tx=0,ty=0;d::predicted_target(in,target,tx,ty);const float distance=std::hypot(tx-in.x,ty-in.y);
 const float gap=(std::max(s.egg.half_x,s.egg.half_y)+8.f+std::min(s.burst_x,s.burst_y)+8.f)*.5f;
 const float q=distance>gap?(distance-gap)/distance:0;const auto ideal=burst::corridor(ram,s.map,in.x,in.y,in.x+(tx-in.x)*q,in.y+(ty-in.y)*q,s.egg.half_x,s.egg.half_y);
 target.clear_path=ideal&&ideal->clear;
 if(in.aiming_actor&&in.reticle_actor){result=burst::evaluate(s,target.actor,in.reticle_x,in.reticle_y);
  const auto actual=burst::corridor(ram,s.map,in.x,in.y,result.x,result.y,s.egg.half_x,s.egg.half_y);
  if(!actual||!actual->clear)result.ready=false;
  target.monster_burst_ready=result.ready;target.monster_burst_passed=result.passed;
 }
}
inline void trace_native_launch(uint8_t* rdram,const recomp_context* ctx,std::string_view name,unsigned vi){
 namespace c=boss_campaign0250;namespace n=boss_native;
 if(rdram&&ctx&&name=="func_800447C8")if(const auto* shot=fish_shots.finish(rdram,fish_shot_frame(),vi,uint32_t(ctx->r4))){
  const auto a=shot->egg,p=shot->private_data,meta=n::word(rdram,a+0x174);auto fl=[&](uint32_t at){return std::bit_cast<float>(n::word(rdram,at));};
  std::fprintf(stderr,"[BossFishEggEnd] row=%u serial=%u releaseSerial=%u sourceVI=%u vi=%u age=%u player=%08X room=%u encounter=%u epoch=%u token=%s egg=%08X private=%08X resource=%08X state=%u/%u queue=%u xyz=%.5f,%.5f,%.5f velocity=%.5f,%.5f,%.5f goal=%.5f,%.5f marked=%u param=%u metadata=%08X meta8=%08X metaC=%08X timing=447C8_entry nativeObservation=1 readOnly=1\n",fish_shots.rows,shot->serial,shot->release_serial,shot->vi,vi,vi-shot->vi,shot->context.player,shot->context.room,shot->context.encounter,shot->context.epoch,shot->context.token.c_str(),a,p,shot->resource,unsigned(n::byte(rdram,a+0x18)),unsigned(n::byte(rdram,a+0x30)),unsigned(n::byte(rdram,a+0x17D)),fl(a+0x84),fl(a+0x88),fl(a+0x8C),fl(a+0xC4),fl(a+0xC8),fl(a+0xCC),fl(p+8),fl(p+12),unsigned(n::byte(rdram,p+3)),n::word(rdram,p+0x10),meta,n::range(meta,0x10)?n::word(rdram,meta+8):0,n::range(meta,0x10)?n::word(rdram,meta+12):0);
 }
 if(rdram&&ctx&&native_trace_enabled()&&c::active()&&c::session.snapshot().phase==boss_battle0250::Phase::Fighting&&c::session.current().family==boss_battle0250::Family::Monsters&&monster_contact_logs<192){
  const auto a=uint32_t(ctx->r4),other=uint32_t(ctx->r5);auto fl=[&](uint32_t p){return std::bit_cast<float>(n::word(rdram,p));};
  if(name=="func_80554928"&&n::actor(rdram,a,0x46F9)&&(n::actor(rdram,other,0x4002)||n::actor(rdram,other,0x4018))){
   const auto root=n::word(rdram,a+0x180);for(unsigned member=0;member<c::session.current().count;++member){const auto& b=c::variants::snapshot().members[member];
    if(c::bound[member]==root&&b.actor==root&&b.birth&&n::actor(rdram,root,0x41C6)&&n::word(rdram,root+0x1F4)==b.private_data&&n::range(b.private_data,0x1000)){
     ++monster_contact_logs;std::fprintf(stderr,"[BossMonsterContact] sample=%u vi=%u root=%08X eye=%08X other=%08X kind=%04X state=%u/%u eyeXYZ=%.5f,%.5f,%.5f otherXYZ=%.5f,%.5f,%.5f nativeHP=%u DE4=%u DF8=%u timing=native_entry nativeObservation=1 noForcedDamage=1\n",monster_contact_logs,vi,root,a,other,unsigned(MEM_HU(12,n::base(other))),unsigned(n::byte(rdram,other+0x18)),unsigned(n::byte(rdram,other+0x30)),fl(a+0x84),fl(a+0x88),fl(a+0x8C),fl(other+0x84),fl(other+0x88),fl(other+0x8C),n::word(rdram,b.private_data+0xDE8),n::word(rdram,b.private_data+0xDE4),n::word(rdram,b.private_data+0xDF8));break;
    }
   }
  }
  if(name=="func_800447C8"&&n::actor(rdram,a,0x4002))for(const auto& shot:native_launch_traces){
   const auto p=n::word(rdram,a+0x1F4);if(shot.serial&&shot.egg==a&&shot.private_data==p&&shot.resource==n::word(rdram,a+0x168)&&shot.room==c::physical_room&&shot.encounter==c::session.snapshot().index&&vi>=shot.vi&&vi-shot.vi<=180&&n::range(p,0x14)){
    ++monster_contact_logs;std::fprintf(stderr,"[BossMonsterEggEnd] sample=%u vi=%u releaseSerial=%u egg=%08X state=%u/%u xyz=%.5f,%.5f,%.5f velocity=%.5f,%.5f goal=%.5f,%.5f marked=%u nativeObservation=1 readOnly=1\n",monster_contact_logs,vi,shot.release_serial,a,unsigned(n::byte(rdram,a+0x18)),unsigned(n::byte(rdram,a+0x30)),fl(a+0x84),fl(a+0x88),fl(a+0x8C),fl(a+0xC4),fl(a+0xC8),fl(p+8),fl(p+12),unsigned(n::byte(rdram,p+3)));break;
   }
  }
 }
 if(rdram&&ctx&&name=="func_800483B8"&&native_trace_enabled()&&c::active()&&c::session.snapshot().phase==boss_battle0250::Phase::Fighting){
  const auto& state=c::session.snapshot();
  for(unsigned member=0;member<c::session.current().count;++member){
   if(c::bound[member]==uint32_t(ctx->r4)&&c::session.current().members[member].kind==0x41C6){
    const auto& b=c::variants::snapshot().members[member];if(b.actor==uint32_t(ctx->r4)&&b.birth)monster_history.native_frame(rdram,b.actor,b.private_data,b.birth,c::physical_room,state.index,vi);
   }
   if(c::bound[member]==uint32_t(ctx->r4)&&c::session.current().members[member].kind==0x40A3){
    if(n::word(rdram,uint32_t(ctx->r4)+0x130)==0x800483B8)if(const auto weak=observe_lips_member(rdram,member)){const auto& b=c::variants::snapshot().members[member];const auto p=c::observers[member].snapshot().actors[0].private_data;const auto root_y=std::bit_cast<float>(n::word(rdram,b.actor+0x88)),root_x=std::bit_cast<float>(n::word(rdram,b.actor+0x84));lips_history.native_frame({b.actor,p,b.birth,c::physical_room,state.index,state.retry_epoch},*weak,root_y,vi,root_x);}
   }
  }
  return;
 }
 if(!rdram||!ctx||name!="func_800404C8"||!native_trace_enabled()||!c::active()||c::session.snapshot().phase!=boss_battle0250::Phase::Fighting||native_launch_logs>=128)return;
 const auto player=uint32_t(ctx->r5),egg=uint32_t(ctx->r4);
 if(player!=c::session.snapshot().player||n::word(rdram,0x800F8828)!=player||!n::actor(rdram,player,0x4001)||!n::actor(rdram,egg,0x4002))return;
 const auto priv=n::word(rdram,player+0x1F4),egg_private=n::word(rdram,egg+0x1F4),resource=n::word(rdram,egg+0x168);
 if(!n::range(priv,0x1F8)||!n::range(egg_private,0x14)||!n::range(resource,0x48))return;
 for(const auto& old:native_launch_traces)if(old.serial&&old.vi==vi&&old.egg==egg&&old.private_data==egg_private&&old.resource==resource)return;
 const auto reticle=n::word(rdram,priv+0x1F4);const bool reticle_valid=n::actor(rdram,reticle,0x4003)&&n::word(rdram,reticle+0x180)==player;
 auto fl=[&](uint32_t a){return std::bit_cast<float>(n::word(rdram,a));};const float missing=std::numeric_limits<float>::quiet_NaN();
 const auto& release=controller.release_trace();const unsigned release_serial=release.serial&&release.player==player&&release.room==c::physical_room&&release.encounter==c::session.snapshot().index&&vi>=release.vi&&vi-release.vi<=180?release.serial:0;
 if(release.kind==0x4526)bird_test.native_launch(rdram,player,egg,release.root,release_serial,vi);
 const unsigned serial=++native_launch_logs;native_launch_traces[(serial-1)%native_launch_traces.size()]={true,serial,vi,c::physical_room,c::session.snapshot().index,release_serial,player,egg,egg_private,resource};
 fish_shots.begin(rdram,fish_shot_frame(),vi,serial,release_serial,egg,c::variants::serial);
 std::fprintf(stderr,"[BossEggNative] serial=%u releaseSerial=%u vi=%u room=%u encounter=%u function=func_800404C8 timing=entry_before_velocity a0=%08X a1=%08X ra=%08X sp=%08X eggPrivate=%08X eggResource=%08X playerXY=%.6f,%.6f eggLocal=%.6f,%.6f eggWorld=%.6f,%.6f reticle=%08X reticleValid=%u reticleXY=%.6f,%.6f held4005=%08X nativeAxes=%d,%d nativeObservation=1 controlsOnly=1\n",serial,release_serial,vi,c::physical_room,c::session.snapshot().index,egg,player,uint32_t(ctx->r31),uint32_t(ctx->r29),egg_private,resource,fl(player+0x84),fl(player+0x88),fl(egg+0x3C),fl(egg+0x40),fl(egg+0x84),fl(egg+0x88),reticle,unsigned(reticle_valid),reticle_valid?fl(reticle+0x84):missing,reticle_valid?fl(reticle+0x88):missing,n::word(rdram,priv+0x1F0),int(MEM_H(0,n::base(0x800F897E))),int(MEM_H(0,n::base(0x800F8980))));
}
// Each admitted launch gets at most60VI of read-only post-shot context.
// A logical row contains both bound Fish and the FIRST production-attested
// primary pulse born after this launch; no pulse is inferred from geometry.
inline void sample_fish_shots(uint8_t* rdram,unsigned vi){
 namespace c=boss_campaign0250;namespace n=boss_native;namespace ex=boss_fish_explosion0260;
 const auto frame=fish_shot_frame();if(!boss_fish_shot_trace0260::current(rdram,frame))return;
 const auto& variants=c::variants::snapshot();const float missing=std::numeric_limits<float>::quiet_NaN();auto fl=[&](uint32_t p){return std::bit_cast<float>(n::word(rdram,p));};
 for(auto& shot:fish_shots.shots)if(fish_shots.sample(rdram,frame,shot,vi)){
  const bool egg=boss_fish_shot_trace0260::egg_identity(rdram,shot,shot.egg);const auto a=shot.egg,p=shot.private_data;
  if(!shot.pulse_birth)for(const auto& [actor,pulse]:c::variants::pulses)if(pulse.source==a&&pulse.primary_egg&&pulse.birth>shot.pulse_floor&&(!shot.pulse_birth||pulse.birth<shot.pulse_birth)){shot.pulse_actor=actor;shot.pulse_birth=pulse.birth;shot.pulse_private=pulse.private_data;}
  const auto found=c::variants::pulses.find(shot.pulse_actor);const bool tracked=found!=c::variants::pulses.end()&&found->second.primary_egg&&found->second.source==a&&found->second.birth==shot.pulse_birth&&shot.pulse_birth>shot.pulse_floor;
  if(tracked&&!shot.pulse_private)shot.pulse_private=found->second.private_data;
  const bool private_match=tracked&&found->second.private_data==shot.pulse_private;
  const bool valid=private_match&&c::variants::pulse(rdram,shot.pulse_actor);const auto puff=private_match?ex::observe(rdram,shot.pulse_actor,0x4018):std::nullopt;
  const bool puff_actor=tracked&&n::actor(rdram,shot.pulse_actor,0x4018);const auto actual_private=puff_actor?n::word(rdram,shot.pulse_actor+0x1F4):0;
  std::fprintf(stderr,"[BossFishShot] row=%u serial=%u releaseSerial=%u sourceVI=%u vi=%u age=%u player=%08X room=%u encounter=%u epoch=%u token=%s activeTick=%u ended=%u egg=%08X private=%08X resource=%08X eggIdentity=%u state=%u/%u queue=%u xyz=%.5f,%.5f,%.5f velocity=%.5f,%.5f,%.5f goal=%.5f,%.5f marked=%u pulse=%08X pulseBirth=%llu pulseFloor=%llu pulsePrivate=%08X tracked=%u pulseValid=%u boxValid=%u hitMask=%u pulseXYZ=%.5f,%.5f,%.5f pulseCentre=%.5f,%.5f,%.5f pulseHalf=%.5f,%.5f,%.5f",
   fish_shots.rows,shot.serial,shot.release_serial,shot.vi,vi,vi-shot.vi,frame.player,frame.room,frame.encounter,frame.epoch,frame.token.c_str(),variants.active_ticks,unsigned(shot.ended),a,p,shot.resource,unsigned(egg),egg?unsigned(n::byte(rdram,a+0x18)):255,egg?unsigned(n::byte(rdram,a+0x30)):255,egg?unsigned(n::byte(rdram,a+0x17D)):255,egg?fl(a+0x84):missing,egg?fl(a+0x88):missing,egg?fl(a+0x8C):missing,egg?fl(a+0xC4):missing,egg?fl(a+0xC8):missing,egg?fl(a+0xCC):missing,egg?fl(p+8):missing,egg?fl(p+12):missing,egg?unsigned(n::byte(rdram,p+3)):255,shot.pulse_actor,static_cast<unsigned long long>(shot.pulse_birth),static_cast<unsigned long long>(shot.pulse_floor),shot.pulse_private,unsigned(tracked),unsigned(valid),unsigned(bool(puff)),tracked?unsigned(found->second.hit_mask):0,puff?fl(shot.pulse_actor+0x84):missing,puff?fl(shot.pulse_actor+0x88):missing,puff?fl(shot.pulse_actor+0x8C):missing,puff?puff->center[0]:missing,puff?puff->center[1]:missing,puff?puff->center[2]:missing,puff?puff->half[0]:missing,puff?puff->half[1]:missing,puff?puff->half[2]:missing);
  std::fprintf(stderr," pulseActualPrivate=%08X pulseState=%u/%u pulseQueue=%u pulseMode=%08X variantsEnabled=%u variantsInvalid=%u",actual_private,puff_actor?unsigned(n::byte(rdram,shot.pulse_actor+0x18)):255,puff_actor?unsigned(n::byte(rdram,shot.pulse_actor+0x30)):255,puff_actor?unsigned(n::byte(rdram,shot.pulse_actor+0x17D)):255,n::range(actual_private,4)?n::word(rdram,actual_private):0xFFFFFFFFu,unsigned(variants.enabled),unsigned(variants.invalidated));
  for(unsigned member=0;member<c::session.current().count&&member<4;++member){
   const auto root=c::bound[member];const auto kind=c::session.current().members[member].kind;const auto& b=variants.members[member];const bool typed=kind==0x4511&&n::actor(rdram,root,kind);
   const auto data=typed?n::word(rdram,root+0x1F4):0;const bool binding=typed&&variants.enabled&&!variants.invalidated&&variants.room==97&&variants.encounter==frame.encounter&&c::variant_epoch==frame.epoch&&b.actor==root&&b.birth&&b.private_data==data&&n::range(data,0x188);
   const auto box=binding?ex::observe(rdram,root,kind):std::nullopt;const auto mirror=box?ex::mirrored(rdram,root):std::nullopt;const auto overlap=valid&&box?ex::overlaps(rdram,shot.pulse_actor,root):std::nullopt;
   std::fprintf(stderr," contactReady%u=%u deltaActiveTick%u=%u",member,unsigned(binding&&c::variants::fish_contact_ready(rdram,member)),member,variants.active_ticks>=b.last_hit_tick?variants.active_ticks-b.last_hit_tick:0);
   std::fprintf(stderr," fish%u=%08X/%04X binding%u=%u birth%u=%llu state%u=%u/%u private40_%u=%08X remaining%u=%u hits%u=%u lastHitTick%u=%u active%u=%u retire%u=%u box%u=%u mirror%u=%d xyz%u=%.5f,%.5f,%.5f centre%u=%.5f,%.5f,%.5f half%u=%.5f,%.5f,%.5f scales%u=%.5f,%.5f,%.5f velocity%u=%.5f,%.5f overlap%u=%d",
    member,root,unsigned(kind),member,unsigned(binding),member,static_cast<unsigned long long>(b.birth),member,typed?unsigned(n::byte(rdram,root+0x18)):255,typed?unsigned(n::byte(rdram,root+0x30)):255,member,binding?n::word(rdram,data+0x40):0,member,b.remaining,member,b.hits,member,b.last_hit_tick,member,unsigned(b.active),member,unsigned(b.retire_queued),member,unsigned(bool(box)),member,mirror?int(*mirror):-1,member,typed?fl(root+0x84):missing,typed?fl(root+0x88):missing,typed?fl(root+0x8C):missing,member,box?box->center[0]:missing,box?box->center[1]:missing,box?box->center[2]:missing,member,box?box->half[0]:missing,box?box->half[1]:missing,box?box->half[2]:missing,member,box?fl(box->scales):missing,box?fl(box->scales+4):missing,box?fl(box->scales+8):missing,member,typed?fl(root+0xC4):missing,typed?fl(root+0xC8):missing,member,overlap?int(*overlap):-1);
  }
  std::fprintf(stderr," nativeObservation=1 readOnly=1\n");
 }
}
inline void sample_native_launches(uint8_t* rdram,unsigned vi){
 namespace c=boss_campaign0250;namespace n=boss_native;
 if(!rdram||!native_trace_enabled())return;
 for(auto& shot:native_launch_traces)if(shot.pending){
  if(vi<shot.vi||vi-shot.vi>8||shot.room!=c::physical_room||shot.encounter!=c::session.snapshot().index){shot.pending=false;continue;}
  if(vi==shot.vi)continue;
  if(!n::actor(rdram,shot.egg,0x4002)||n::word(rdram,shot.egg+0x1F4)!=shot.private_data||n::word(rdram,shot.egg+0x168)!=shot.resource){shot.pending=false;continue;}
  shot.pending=false;if(native_launch_first_logs>=128)continue;++native_launch_first_logs;
  auto fl=[&](uint32_t a){return std::bit_cast<float>(n::word(rdram,a));};const auto a=shot.egg;
  std::fprintf(stderr,"[BossEggFirst] serial=%u releaseSerial=%u sourceVI=%u vi=%u deltaVI=%u room=%u encounter=%u egg=%08X eggPrivate=%08X eggResource=%08X state=%u sub=%u queuePhase=%u parent=%08X update=%08X movement=%08X local=%.6f,%.6f world=%.6f,%.6f velocity=%.6f,%.6f nativeObservation=1 controlsOnly=1\n",shot.serial,shot.release_serial,shot.vi,vi,vi-shot.vi,shot.room,shot.encounter,a,shot.private_data,shot.resource,n::byte(rdram,a+0x18),n::byte(rdram,a+0x30),n::byte(rdram,a+0x17D),n::word(rdram,a+0x180),n::word(rdram,a+0x20),n::word(rdram,a+0x130),fl(a+0x3C),fl(a+0x40),fl(a+0x84),fl(a+0x88),fl(a+0xC4),fl(a+0xC8));
 }
}
// Ordinary Genie does not populate Variants.private_data: that slot belongs
// to mutable Fish/Monster/red-Candy variants. The family observer owns the
// genuine D33C initializer/private receipt, while Variants owns its birth.
inline std::optional<boss_genie_aim0260::Motion> observe_genie_motion(uint8_t* rdram,unsigned member){
 namespace c=boss_campaign0250;namespace n=boss_native;namespace f=boss_family_native0250;
 const auto& s=c::session.snapshot();const auto& v=c::variants::snapshot();
 if(!rdram||!c::active()||!c::driver||!s.diagnostic||s.phase!=boss_battle0250::Phase::Fighting||s.index>=10||member>=c::session.current().count||member>=4||c::restore_pending||c::physical_room!=c::rooms::source_for_encounter(s.index)||!v.enabled||v.invalidated||v.encounter!=s.index||v.room!=c::logical_room||c::variant_epoch!=s.retry_epoch||c::observer_epochs[member]!=s.retry_epoch||c::session.current().members[member].kind!=0x4072||(s.defeated&(1u<<member)))return {};
 const auto root=c::bound[member];const auto& binding=v.members[member];const auto& observer=c::observers[member];const auto& proof=observer.snapshot();const auto& a=proof.actors[0];
 if(!root||binding.actor!=root||!binding.birth||binding.receipt_sent||binding.retire_queued||observer.bound_actor()!=root||!proof.enabled||proof.invalidated||proof.family!=f::Family::genie||proof.source_room!=c::logical_room||proof.run_token!=s.token||proof.expected_actors!=1||proof.registered_actors!=1||proof.expected_hits!=c::session.current().members[member].hits||!a.initialized||a.disposed||a.terminal_seen||a.completion_returned||a.actor!=root)return {};
 if(!n::actor(rdram,root,0x4072))return {};const auto resource=n::word(rdram,root+0x168),overlay=c::overlays[16];
 if(!n::range(resource,0x48)||!n::range(overlay,0x2AA0)||n::word(rdram,resource+0x40)!=overlay)return {};
 return boss_genie_aim0260::observe(rdram,root,a.private_data,binding.birth);
}
inline std::optional<boss_fish_aim0260::Motion> observe_fish_motion(uint8_t* rdram,unsigned member){
 namespace c=boss_campaign0250;namespace n=boss_native;const auto& state=c::session.snapshot();const auto& variants=c::variants::snapshot();
 if(!rdram||!c::active()||!c::driver||!state.diagnostic||state.phase!=boss_battle0250::Phase::Fighting||c::restore_pending||c::session.current().family!=boss_battle0250::Family::Fish||c::logical_room!=97||state.index>=9||c::physical_room!=167+state.index||member>=c::session.current().count||member>=variants.members.size()||(state.defeated&(1u<<member))||!variants.enabled||variants.invalidated||variants.room!=97||variants.encounter!=state.index||c::variant_epoch!=state.retry_epoch)return {};
 const auto& binding=variants.members[member];const auto actor=c::bound[member];const auto kind=c::session.current().members[member].kind;
 if(!actor||binding.actor!=actor||!binding.birth||binding.receipt_sent||binding.retire_queued||!n::actor(rdram,actor,kind)||binding.private_data!=n::word(rdram,actor+0x1F4))return {};
 return boss_fish_aim0260::observe(rdram,actor,kind,binding.birth);
}
inline void observe_fish_navigation(uint8_t* rdram,uint32_t player,unsigned room,unsigned encounter,std::span<const float> centers,boss_driver0260::Input& in){
 namespace c=boss_campaign0250;namespace n=boss_native;
 if(c::session.current().family!=boss_battle0250::Family::Fish||c::logical_room!=97||room!=c::physical_room||encounter!=c::session.snapshot().index||bird_test.snapshot().enabled)return;
 in.fish_navigation=true;in.fish_retry_epoch=c::session.snapshot().retry_epoch;
 const auto& saved=controller.snapshot();std::optional<float> previous;
 if(saved.player==player&&saved.room==room&&saved.encounter==encounter&&saved.fish_anchor_attested)previous=saved.fish_anchor_x;
 if(const auto island=boss_fish_support0260::observe(rdram,player,room,encounter,centers,previous)){
  in.fish_island_observed=true;in.fish_on_island=island->on_island;in.fish_anchor_x=island->x;
 }
 if(n::actor(rdram,player,0x4001)){in.horizontal_speed=std::bit_cast<float>(n::word(rdram,player+0xC4));in.horizontal_speed_observed=std::isfinite(in.horizontal_speed)&&std::abs(in.horizontal_speed)<=32;}
 if(n::actor(rdram,player,0x4001)){
  in.fish_world_scale=std::bit_cast<float>(n::word(rdram,player+0x100));in.fish_air_brake=std::bit_cast<float>(n::word(rdram,0x800AFD40));
  in.fish_counter_model_observed=std::isfinite(in.fish_world_scale)&&in.fish_world_scale>0&&in.fish_world_scale<=4&&std::isfinite(in.fish_air_brake)&&in.fish_air_brake>=.1f&&in.fish_air_brake<=1;
  const auto data=n::word(rdram,player+0x1F4);in.fish_reticle_cleared=n::range(data,0x1F8)&&n::word(rdram,data+0x1F4)==0;
 }
 const auto& state=c::session.snapshot();const auto& variants=c::variants::snapshot();
 if(!state.diagnostic||!c::driver||state.phase!=boss_battle0250::Phase::Fighting||state.player!=player||!variants.enabled||variants.invalidated||variants.encounter!=encounter||variants.room!=97||c::variant_epoch!=state.retry_epoch)return;
 std::array<boss_fish_evade0260::Root,4> roots{};unsigned count=0;
 const bool keeping=saved.fish_evading||saved.fish_evade_pending_z||saved.fish_evade_sending_z||saved.fish_evade_acquire_z;in.fish_evade_generation_valid=!keeping;
 for(unsigned member=0;member<c::session.current().count&&member<roots.size();++member){const auto& binding=variants.members[member];const auto a=c::bound[member];if(c::session.current().members[member].kind!=0x4511||(state.defeated&(1u<<member))||binding.actor!=a||!binding.birth||binding.receipt_sent||binding.retire_queued||!n::actor(rdram,a,0x4511))continue;
  const auto sub=n::byte(rdram,a+0x30);if(n::byte(rdram,a+0x18)!=1||sub>=boss_fish_green0260::updates.size()||!boss_fish_green0260::updates[sub])continue;
  roots[count++]={a,binding.birth};if(a==saved.fish_evade_actor&&binding.birth==saved.fish_evade_birth)in.fish_evade_generation_valid=true;
 }
 const auto threat=boss_fish_evade0260::observe(rdram,player,{roots.data(),count},room,encounter);
 in.fish_evade_observed=threat.observed;in.fish_evade_imminent=threat.imminent;in.fish_evade_head_clear=threat.head_clear;in.fish_threat_actor=threat.actor;in.fish_threat_birth=threat.birth;in.fish_threat_step=threat.step;
 // A new4632 departure needs a current actor/resource/private/latch witness,
 // never an invented Variant birth. During an admitted45VI gesture only
 // current P1/map/head context is read; its departed source is not revalidated.
 const boss_fish_water_evade0260::Identity previous_water{saved.fish_evade_actor,saved.fish_evade_resource,saved.fish_evade_private};
 const bool water_hold=saved.fish_evading&&saved.fish_evade_water&&in.vi>=saved.fish_evade_vi&&in.vi-saved.fish_evade_vi<45;
 const auto water=boss_fish_water_evade0260::observe(rdram,player,room,encounter,keeping&&saved.fish_evade_water&&!water_hold?&previous_water:nullptr);
 if(keeping&&saved.fish_evade_water)in.fish_evade_generation_valid=water.previous_valid;
 if(water.observed&&((keeping&&saved.fish_evade_water)||(!keeping&&water.imminent&&(!threat.imminent||water.step<=threat.step)))){
  in.fish_evade_observed=water.observed;in.fish_evade_imminent=water.imminent;in.fish_evade_head_clear=water.head_clear;in.fish_threat_actor=water.actor;in.fish_threat_birth=0;in.fish_threat_step=water.step;
  in.fish_threat_water=true;in.fish_threat_resource=water.identity.resource;in.fish_threat_private=water.identity.private_data;
 }
}
inline bool fish_pilot_window(const boss_driver0260::Input& in,unsigned epoch){
 auto& s=fish_pilot_trace;
 if(!s.have||s.player!=in.player||s.room!=in.room||s.encounter!=in.encounter||s.epoch!=epoch||in.vi<s.first_vi)s={in.player,in.room,in.encounter,epoch,in.vi,0,0,true};
 if(in.vi-s.first_vi>=480||(in.vi-s.first_vi)%4||s.samples>=120||(s.samples&&s.last_vi==in.vi))return false;
 s.last_vi=in.vi;++s.samples;return true;
}
inline void trace_fish_pilot(uint8_t* rdram,const boss_driver0260::Input& in,uint16_t buttons,float x,float y){
 namespace c=boss_campaign0250;namespace n=boss_native;
 const auto& state=c::session.snapshot();
 if(!rdram||!c::active()||!c::driver||!state.diagnostic||state.phase!=boss_battle0250::Phase::Fighting||c::session.current().family!=boss_battle0250::Family::Fish||c::logical_room!=97||c::physical_room!=c::source_for(state.index)||in.player!=state.player||!n::actor(rdram,in.player,0x4001))return;
 const auto p=n::word(rdram,in.player+0x1F4);if(!n::range(p,0x12D0)||!fish_pilot_window(in,state.retry_epoch))return;
 auto fl=[&](uint32_t a){return std::bit_cast<float>(n::word(rdram,a));};
 const auto meta=n::word(rdram,in.player+0x174);const auto& snap=controller.snapshot();
 const float zero=std::numeric_limits<float>::quiet_NaN();std::array<float,3> centers{zero,zero,zero};for(unsigned i=0;i<centers.size()&&i<in.stable_stands.size();++i)centers[i]=in.stable_stands[i];
 fprintf(stderr,"[BossFishPilot] sample=%u vi=%u firstVI=%u epoch=%u player=%08X xyz=%.5f,%.5f,%.5f C4C8=%.5f,%.5f desiredX=%.5f accelerationX=%.5f F4=%u state=%u/%u callback=%08X parent=%08X contactOwner=%08X pendingC0=%08X request=%u/%u phase=%u phaseVI=%u aim=%08X reticle=%08X reticleXY=%.5f,%.5f centers=%.2f,%.2f,%.2f nativeAxes=%d,%d outputButtons=%04X outputStick=%.5f,%.5f fishSupport=%u/%u/%u anchorX=%.2f recovering=%u settle=%u evade=%u/%u/%u threat=%08X step=%u Aactive=%u pendingZ=%u brakeModel=%u/%.3f/%.3f reticleClear=%u throwCounter=%u waterThreat=%u/%08X/%08X waterHold=%u nativeObservation=1 readOnly=1 controlsOnly=1\n",fish_pilot_trace.samples,in.vi,fish_pilot_trace.first_vi,state.retry_epoch,in.player,fl(in.player+0x84),fl(in.player+0x88),fl(in.player+0x8C),fl(in.player+0xC4),fl(in.player+0xC8),fl(in.player+0xDC),fl(in.player+0xE8),unsigned(n::byte(rdram,p+0xF4)),unsigned(n::byte(rdram,in.player+0x18)),unsigned(n::byte(rdram,in.player+0x30)),n::word(rdram,in.player+0x20),n::word(rdram,in.player+0x180),n::range(meta,8)?n::word(rdram,meta+4):0,n::word(rdram,p+0xC0),unsigned(in.native_egg_request_observed),unsigned(in.native_egg_request_ready),unsigned(snap.phase),snap.phase_vi,in.aiming_actor,in.reticle_actor,in.reticle_x,in.reticle_y,centers[0],centers[1],centers[2],in.native_stick.x,in.native_stick.y,unsigned(buttons),x,y,unsigned(in.fish_navigation),unsigned(in.fish_island_observed),unsigned(in.fish_on_island),in.fish_anchor_x,unsigned(snap.fish_recovering),snap.fish_settle_count,unsigned(in.fish_evade_observed),unsigned(in.fish_evade_imminent),unsigned(in.fish_evade_head_clear),in.fish_threat_actor,in.fish_threat_step,unsigned(snap.fish_evading),unsigned(snap.fish_evade_pending_z),unsigned(in.fish_counter_model_observed),in.fish_world_scale,in.fish_air_brake,unsigned(in.fish_reticle_cleared),unsigned(snap.fish_throw_counter),unsigned(in.fish_threat_water),in.fish_threat_resource,in.fish_threat_private,unsigned(snap.fish_evade_water));
 for(unsigned member=0;member<c::session.current().count&&member<4;++member){const auto a=c::bound[member];if(!n::actor(rdram,a,0x4511))continue;const auto data=n::word(rdram,a+0x1F4);const boss_driver0260::Target* t=nullptr;for(const auto& candidate:in.targets)if(candidate.actor==a){t=&candidate;break;}
  fprintf(stderr,"[BossFishPilotRoot] sample=%u vi=%u member=%u actor=%08X kind=%04X state=%u/%u callback=%08X xyz=%.5f,%.5f,%.5f C4C8=%.5f,%.5f movement=%08X private40=%08X targetObserved=%u targetXY=%.5f,%.5f targetVelocity=%.5f,%.5f nativeSpeed=%.5f vulnerable=%u nativeObservation=1 readOnly=1\n",fish_pilot_trace.samples,in.vi,member,a,unsigned(MEM_HU(12,n::base(a))),unsigned(n::byte(rdram,a+0x18)),unsigned(n::byte(rdram,a+0x30)),n::word(rdram,a+0x20),fl(a+0x84),fl(a+0x88),fl(a+0x8C),fl(a+0xC4),fl(a+0xC8),n::word(rdram,a+0x130),n::range(data,0x44)?n::word(rdram,data+0x40):0,unsigned(bool(t)),t?t->x:zero,t?t->y:zero,t?t->vx:zero,t?t->vy:zero,t?t->native_speed:zero,unsigned(t&&t->vulnerable));
 }
}
inline void configure(bool qa_bird=false){monster_side_forage.reset();fish_water_return.reset();fish_water_logs=0;fish_water_last=boss_fish_water_recovery0260::Stage::Idle;reset_launch_trace();fish_pilot_trace={};monster_return_motion_logs=0;lips_jump_logs=genie_supply_log_windows=monster_contact_logs=0;monster_history.reset();lips_history.reset();bird_test.configure(qa_bird);controller.configure(boss_campaign0250::enabled&&boss_campaign0250::driver,boss_campaign0250::session.snapshot().diagnostic);}
inline void observe_lips_health(uint8_t* rdram,uint32_t player,boss_driver0260::Input& in){
 namespace c=boss_campaign0250;namespace n=boss_native;const auto& s=c::session.snapshot();
 if(!rdram||!c::active()||!c::driver||!s.diagnostic||s.phase!=boss_battle0250::Phase::Fighting||c::session.current().family!=boss_battle0250::Family::Lips||c::logical_room!=165||c::physical_room!=c::source_for(s.index)||c::variant_epoch!=s.retry_epoch||s.player!=player||in.player!=player||n::word(rdram,0x800F8828)!=player||!n::actor(rdram,player,0x4001)||c::fatal_player(rdram,player))return;
 const auto hp=boss_solo0240::health(rdram);if(!hp||*hp>8)return;
 in.lips_healing_observed=in.health_observed=true;in.health=*hp;
}
inline void drive(uint8_t* rdram,unsigned vi,uint16_t* buttons,float* x,float* y,std::size_t observation_bytes=0x800000){
 if(!rdram||observation_bytes<0x800000)return;
 namespace c=boss_campaign0250;namespace n=boss_native;namespace d=boss_driver0260;
 sample_native_launches(rdram,vi);
 sample_fish_shots(rdram,vi);
 if(!rdram||!c::active()||!c::driver||c::session.snapshot().phase!=boss_battle0250::Phase::Fighting)return;
 const auto& state=c::session.snapshot();const auto player=state.player,priv=n::word(rdram,player+0x1F4);if(!n::actor(rdram,player,0x4001)||!n::range(priv,0x12D0))return;
 // Ordinary candy, an open yellow window, or a proven empty native FIFO
 // use the same tongue pilot. Red segments remain edible without healing.
 // Known ammunition keeps egg control; an unknown stock never grants tongue.
 for(unsigned k=0;k<c::session.current().count;++k)if(c::candy_tongue_pilot(c::session.current().members[k].kind,c::session.current().members[k].red,c::variants::yellow(),(state.defeated&(1u<<k))!=0,c::candy_empty_stock(rdram,player)))return;
 d::Input in;in.vi=vi;in.room=c::physical_room;in.encounter=state.index;in.player=player;in.combat=true;in.controls=observe_controls(rdram,player);in.alive=!c::fatal_player(rdram,player);in.primary_state=MEM_BU(0x18,n::base(player));in.grounded=MEM_BU(0xF4,n::base(priv))==0;in.spring_attached=observe_spring_attachment(rdram,player);
 auto any_actor=[&](uint32_t a){return n::range(a,0x1FC)&&n::word(rdram,a)==0x19971110&&MEM_BU(6,n::base(a))!=3;};
 auto fl=[&](uint32_t a){return std::bit_cast<float>(n::word(rdram,a));};in.x=fl(player+0x84);in.y=fl(player+0x88);const auto& arena=c::rooms::arena(c::physical_room);in.min_x=arena.min_x;in.max_x=arena.max_x;
 std::array<float,80> mouth_x{},mouth_y{};
 in.native_stick={int(MEM_H(0,n::base(0x800F897E))),int(MEM_H(0,n::base(0x800F8980)))};in.native_stick_observed=boss_stick_aim0260::valid(in.native_stick);
 observe_native_mouth(rdram,player,mouth_x,mouth_y,in.native_mouth);
 const auto tongue=boss_native_mouth0260::observe_ordinary_tongue(rdram,player);
 in.native_tongue_observed=true;in.native_tongue_ready=tongue.observed&&tongue.ready;
 in.native_tongue_phase=tongue.phase;in.native_tongue_callback_pending=tongue.callback_pending;
 in.native_tongue_known=tongue.observed;in.native_tongue_live=tongue.live;in.native_tongue_cancelled_idle=tongue.cancelled_idle;
 in.native_tongue_max_reach=tongue.max_reach;in.native_tongue_heading_degrees=tongue.heading_degrees;in.native_tongue_heading_active=tongue.heading_active;
 if(c::session.current().family==boss_battle0250::Family::Monsters){
  if(arena.logical_source==151){in.monster_navigation=true;in.has_preferred_x=true;in.preferred_x=312;
   const auto& fight=c::session.current();
   const bool side_scope=cp6_monster_side_scope(state,fight,arena,c::session.plan().seed,c::logical_room,c::physical_room,c::driver);
   const auto stand=side_scope?boss_monster_aim0260::configured_side_stand(rdram,c::physical_room,state.index,arena.objects):boss_monster_aim0260::supply_stand(rdram,c::physical_room,state.index);
   if(side_scope){in.monster_upper_navigation=in.monster_side_navigation=true;in.monster_upper_supported=stand&&boss_monster_upper0260::observe_at(rdram,player,c::physical_room,state.index,*stand);}
   else observe_monster_upper_navigation(rdram,player,c::physical_room,state.index,in);
   in.horizontal_speed=fl(player+0xC4);in.horizontal_speed_observed=std::isfinite(in.horizontal_speed)&&std::abs(in.horizontal_speed)<=64;
   in.combat_supported=observe_monster_combat_support(rdram,player);
   if(stand){in.has_supply_stand=true;in.supply_stand_x=stand->x;in.supply_stand_y=stand->y;}
  }
 }
 const auto egg_request=observe_egg_request(rdram,player);in.native_egg_request_observed=egg_request.observed;in.native_egg_request_ready=egg_request.ready;
 in.throw_mode=n::word(rdram,priv+0x1E0)==1?d::ThrowMode::SecondPress:d::ThrowMode::HoldAndRelease;
 std::array<uint32_t,6> eggs{};if(boss_solo0240::read_reserves(rdram,player,eggs))in.reserves=unsigned(std::count_if(eggs.begin(),eggs.end(),[](auto e){return e!=0;}));
 in.mouth_full=any_actor(n::word(rdram,priv+0x164));
 observe_lips_health(rdram,player,in);
 const auto egg_aim=observe_egg_aim(rdram,player);in.aiming_actor=egg_aim.held;in.reticle_actor=egg_aim.reticle;in.reticle_x=egg_aim.reticle_x;in.reticle_y=egg_aim.reticle_y;
 std::array<d::Target,4> targets{};unsigned count=0;
 std::optional<boss_monster_burst0260::Scene> monster_burst_scene;
 std::array<boss_monster_burst0260::Result,4> monster_burst_results{};
 if(c::session.current().family==boss_battle0250::Family::Monsters){
  std::array<boss_monster_burst0260::Root,4> roots{};unsigned total=0;
  for(unsigned member=0;member<c::session.current().count;++member)if(!(state.defeated&(1u<<member))&&c::session.current().members[member].kind==0x41C6){const auto& b=c::variants::snapshot().members[member];if(b.actor==c::bound[member]&&b.birth)roots[total++]={b.actor,b.private_data,b.birth};}
  if(total)monster_burst_scene=boss_monster_burst0260::observe(std::span<const uint8_t>(rdram,observation_bytes),player,{roots.data(),total},monster_history,c::physical_room,state.index,vi);
 }
 boss_lips_jump0260::Debug lips_jump_debug{};uint32_t lips_sample_root=0;bool lips_sample_path_clear=false;
 std::optional<boss_egg_shape0260::Shape> lips_egg_shape;
 if(c::session.current().family==boss_battle0250::Family::Lips)lips_egg_shape=boss_egg_shape0260::observe_aimed(std::span<const uint8_t>(rdram,observation_bytes),player);
 boss_lips_corridor0260::Result lips_corridor_debug{};bool lips_corridor_observed=false;
 std::optional<boss_lips_target0260::Point> lips_aim_point;float lips_center_x=0,lips_center_y=0;
 boss_lips_motion0260::Motion lips_motion{};float lips_intercept_x=0,lips_intercept_y=0;
 boss_lips_body0260::Result lips_body_result{};std::optional<boss_lips_body0260::Scene> lips_body_scene;
 for(unsigned k=0;k<c::session.current().count;++k){const auto a=c::bound[k];const auto kind=c::session.current().members[k].kind;const bool red_candy=kind==0x455A&&c::session.current().members[k].red;
  if(!n::actor(rdram,a,kind)||(state.defeated&(1u<<k))||(!d::supported(kind)&&!red_candy))continue;auto coordinate=a;
  if(red_candy){const auto& candy=c::candies[k].snapshot().actors[0];if(!candy.active_seen||!n::actor(rdram,candy.head,0x454F)||!n::range(candy.head_private,0x88))continue;coordinate=candy.head;float lowest=-1e9f;for(unsigned segment=0;segment<12;++segment){const auto b=n::word(rdram,candy.head_private+0x54+segment*4);if(n::actor(rdram,b,0x4550)&&MEM_BU(0x18,n::base(b))==1&&fl(b+0x88)>lowest){lowest=fl(b+0x88);coordinate=b;}}}

  bool lips_coordinate_observed=kind!=0x40A3,lips_clear_path=false;
  if(kind==0x40A3){if(const auto weak=observe_lips_member(rdram,k)){coordinate=weak->actor;lips_coordinate_observed=true;lips_sample_root=a;
    const auto& b=c::variants::snapshot().members[k];lips_motion=lips_history.observe({a,c::observers[k].snapshot().actors[0].private_data,b.birth,c::physical_room,state.index,state.retry_epoch},*weak,vi);
    const auto map=boss_monster_aim0260::Map::read(rdram);
    lips_center_x=weak->x;lips_center_y=weak->y;
     if(map&&lips_egg_shape){if(const auto corridor=boss_lips_corridor0260::observe(rdram,*map,in.x,in.y,weak->x,weak->y,lips_egg_shape->half_x,lips_egg_shape->half_y)){lips_corridor_observed=true;lips_corridor_debug=*corridor;}
      const auto body=boss_lips_body0260::observe(rdram,a,c::observers[k].snapshot().actors[0].private_data,b.birth);lips_body_scene=body;
      lips_aim_point=boss_lips_target0260::choose_if(rdram,*map,*weak,*lips_egg_shape,in.x,in.y,fl(player+0x8C),[&](const boss_lips_target0260::Point& point){
       if(!body||!lips_motion.observed||!lips_motion.body_observed)return false;
       d::Target candidate{a,kind,point.x,point.y,lips_motion.vx,lips_motion.vy,true,true,false,false,true,6.6f,2.f,weak->actor};float tx=0,ty=0;d::predicted_target(in,candidate,tx,ty);
       const auto future=boss_lips_corridor0260::observe(rdram,*map,in.x,in.y,tx,ty,lips_egg_shape->half_x,lips_egg_shape->half_y);
       lips_body_result=boss_lips_body0260::toward(*body,*lips_egg_shape,in.x,in.y,fl(player+0x8C),tx,ty,lips_motion.vx,lips_motion.body_vx,lips_motion.vy);
       return future&&future->clear&&lips_body_result.clear;
      },true);if(lips_aim_point){lips_clear_path=true;lips_corridor_observed=true;lips_corridor_debug=lips_aim_point->corridor;}}
    lips_sample_path_clear=lips_clear_path;
    if(lips_body_scene){in.lips_escape_root=a;in.lips_escape_birth=b.birth;const auto& prior=controller.snapshot();const int direction=prior.lips_escaping&&prior.lips_escape_root==a&&prior.lips_escape_birth==b.birth?prior.lips_escape_direction:0;if(map){if(const auto escape=boss_lips_escape0260::observe(rdram,player,*lips_body_scene,*map,in.min_x,in.max_x,direction)){in.lips_escape_observed=true;in.lips_escape_x=escape->x;in.lips_escape_direction=escape->direction;}}}
    if(!lips_clear_path||controller.snapshot().lips_escaping||in.lips_escape_observed){if(const auto jump=boss_lips_jump0260::observe(rdram,player,in.min_x,in.max_x,weak->y,&lips_jump_debug)){if(lips_body_scene&&boss_lips_body0260::walk_clear(rdram,*lips_body_scene,player,jump->x)){in.lips_jump_clearance_observed=true;in.lips_jump_column_clear=jump->current_clear;in.lips_jump_x=jump->x;}}}
    in.horizontal_speed=fl(player+0xC4);in.horizontal_speed_observed=std::isfinite(in.horizontal_speed)&&std::abs(in.horizontal_speed)<=64;
   }
  }
  const unsigned st=MEM_BU(0x18,n::base(a)),sub=MEM_BU(0x30,n::base(a));bool vulnerable=st==1;
  if(kind==0x4599)vulnerable=vulnerable&&sub!=1&&sub!=6&&sub!=7;
  if(kind==0x4072)vulnerable=vulnerable&&sub!=4&&sub!=5;
  if(kind==0x40A3){const auto p=n::word(rdram,a+0x1F4);vulnerable=vulnerable&&lips_coordinate_observed&&n::range(p,0x34)&&!n::word(rdram,p+0x30);}
  if(kind==0x41C6){const auto p=n::word(rdram,a+0x1F4);vulnerable=vulnerable&&n::range(p,0x1000)&&!n::word(rdram,p+0xDE4)&&!n::word(rdram,p+0xDF8);}
  if(kind==0x41C6){const auto& binding=c::variants::snapshot().members[k];const auto target=boss_monster_aim0260::choose(rdram,monster_history,a,binding.private_data,binding.birth,c::physical_room,state.index,vi,in,vulnerable);
   if(target){targets[count]=*target;
    // Recover only the diagnostic coordinate identity that choose() selected.
    // This never changes target coordinates, ordering, vulnerability or input.
    for(unsigned child=0;child<20;++child){boss_monster_aim0260::Eye eye;const auto id=n::word(rdram,a+0x188+4*child);if(boss_monster_aim0260::observe_eye(rdram,a,binding.private_data,binding.birth,id,eye)&&eye.x==target->x&&eye.y==target->y){targets[count].observation_actor=id;break;}}
    targets[count].monster_burst_observed=true;
    if(monster_burst_scene)plan_monster_burst(rdram,*monster_burst_scene,in,targets[count],monster_burst_results[count]);else targets[count].clear_path=false;
    ++count;
   }
   else {targets[count]={a,kind,fl(a+0x84),fl(a+0x88),0,0,st!=0xFF,false,false,false,false,6.6f,2.f};targets[count++].monster_burst_observed=true;}
  }else if(kind==0x40A3){
   d::Target t{a,kind,lips_aim_point?lips_aim_point->x:fl(coordinate+0x84),lips_aim_point?lips_aim_point->y:fl(coordinate+0x88),lips_motion.vx,lips_motion.vy,st!=0xFF,vulnerable&&lips_motion.observed,false,false,lips_clear_path,6.6f,2.f,lips_coordinate_observed?coordinate:0};
   t.birth=c::variants::snapshot().members[k].birth;
   d::predicted_target(in,t,lips_intercept_x,lips_intercept_y);const auto map=boss_monster_aim0260::Map::read(rdram);
   const auto future=map&&lips_egg_shape?boss_lips_corridor0260::observe(rdram,*map,in.x,in.y,lips_intercept_x,lips_intercept_y,lips_egg_shape->half_x,lips_egg_shape->half_y):std::nullopt;
   t.clear_path=t.clear_path&&lips_motion.observed&&future&&future->clear;lips_sample_path_clear=t.clear_path;targets[count++]=t;
  }
  else if(kind==0x4072){
   const auto motion=observe_genie_motion(rdram,k);
   targets[count++]={a,kind,motion?motion->x:fl(a+0x84),motion?motion->y:fl(a+0x88),motion?motion->vx:0,motion?motion->vy:0,st!=0xFF,vulnerable&&bool(motion),false,false,true,motion?6.6f:0};
  }
  else if(kind==0x4511||kind==0x4631){const auto motion=observe_fish_motion(rdram,k);d::Target t{a,kind,motion?motion->x:fl(a+0x84),motion?motion->y:fl(a+0x88),motion?motion->vx:0,motion?motion->vy:0,st!=0xFF,vulnerable&&bool(motion),false,false,true,motion?motion->speed:0};if(motion)t.birth=motion->birth;targets[count++]=t;}
  else targets[count++]={a,kind,fl(coordinate+0x84),fl(coordinate+0x88),0,0,st!=0xFF,vulnerable,false,red_candy};
 }
 in.targets={targets.data(),count};std::array<float,1> genie_stand{};
 if(c::session.current().family==boss_battle0250::Family::Genie&&arena.logical_source==163){in.genie_navigation=true;for(unsigned member=0;member<c::session.current().count;++member){const auto& binding=c::variants::snapshot().members[member];const auto support=boss_genie_support0260::observe(rdram,{true,arena.logical_source,c::physical_room,state.index,player,c::bound[member],binding.birth,arena.entry_x});if(support){in.genie_support_observed=true;in.genie_anchor_x=support->anchor;in.min_x=support->min_x;in.max_x=support->max_x;in.has_preferred_x=true;in.preferred_x=support->anchor;genie_stand[0]=support->anchor;in.stable_stands=genie_stand;break;}}}
 std::array<d::Supply,150> supplies{};unsigned supplied=0;
 constexpr uint32_t queue=0x800F8814;const auto cap=MEM_HU(8,n::base(queue));const auto table=n::word(rdram,queue+0x10);
 if(cap<=150&&n::range(table,cap*4))for(unsigned k=0;k<cap;++k){const auto a=n::word(rdram,table+k*4);if(!any_actor(a)||MEM_BU(0x17D,n::base(a))!=2)continue;const auto kind=MEM_HU(12,n::base(a));const auto st=MEM_BU(0x18,n::base(a));
  if(kind==0x4002&&st==0x68&&!n::word(rdram,a+0x180))in.projectile_actor=a;
  if((kind==0x4002&&st!=0x68&&!n::word(rdram,a+0x180))||kind==0x4086)supplies[supplied++]={a,uint16_t(kind),fl(a+0x84),fl(a+0x88),st!=0xFF,kind==0x4002};
  if(kind==0x4526||kind==0x46DA){
   const auto flags=egg_food_flags(rdram,a);if(flags&1)supplies[supplied++]={a,uint16_t(kind),fl(a+0x84),fl(a+0x88),st!=0xFF,false,flags};
  }
  if(c::session.current().family==boss_battle0250::Family::Lips&&(kind==0x4539||kind==0x453A)){
   if(auto food=boss_lips_supply0260::observe(rdram,a)){food->healing_clear=food->healing_gain>0&&lips_body_scene&&food->x>=in.min_x&&food->x<=in.max_x&&boss_lips_body0260::walk_clear(rdram,*lips_body_scene,player,food->x);supplies[supplied++]=*food;}
  }
  if(kind==0x456E){
   d::Supply source;if(observe_feeder(rdram,a,in.y,source))supplies[supplied++]=source;
  }
 }
 in.supplies={supplies.data(),supplied};if(c::session.current().family==boss_battle0250::Family::Fish){in.stable_stands=arena.safe_pot_xs;observe_fish_navigation(rdram,player,c::physical_room,state.index,arena.safe_pot_xs,in);}
 // The native Inviso pillar is reached through ordinary Approach jumps. This
 // horizontal preference must not use the fish-only grounded island guard.
 if(c::session.current().family==boss_battle0250::Family::Invisible&&arena.logical_source==164){float distance=1e30f;for(float at:arena.safe_pot_xs)if(std::isfinite(at)&&at>=in.min_x&&at<=in.max_x&&std::abs(at-in.x)<distance){distance=std::abs(at-in.x);in.preferred_x=at;in.has_preferred_x=in.invisible_navigation=true;}}
 // Locked native intros still need normal controller acknowledgements.
 // Never let the neutral aiming output erase the scripted dialogue edges.
 if(!in.controls){fish_water_return.reset();controller.step(in);
  if(cap<=150&&n::range(table,cap*4))for(unsigned k=0;k<cap;++k){const auto a=n::word(rdram,table+4*k);if(!n::actor(rdram,a,0x4088)||MEM_BU(0x18,n::base(a))!=5)continue;const auto p=n::word(rdram,a+0x1F4);if(!n::range(p,8))continue;const auto text=n::word(rdram,p+4);if(n::actor(rdram,text,0x4024)&&!n::word(rdram,text+0x1E0)&&vi%120<5){*buttons=0x8000;*x=*y=0;}}
  if(buttons&&x&&y)trace_fish_pilot(rdram,in,*buttons,*x,*y);
  return;
 }
 std::optional<d::Target> bird_target;
 if(bird_test.snapshot().enabled&&!bird_test.complete()){
  std::array<boss_bird_test0260::Bird,8> owned{};for(unsigned slot=0;slot<owned.size();++slot){const auto& b=c::variants::birds[slot];owned[slot]={b.actor,b.private_data,b.resource,b.factory,b.birth,b.source_actor,b.source_private,b.source_birth};}
  const boss_bird_test0260::Frame f{state.diagnostic,c::driver,in.controls,in.alive,c::physical_room,c::logical_room,state.index,vi,player,in.reserves,in.aiming_actor,in.x,in.y,in.min_x,in.max_x};
  bird_target=bird_test.observe(rdram,f,owned);
  if(bird_test.snapshot().phase==boss_bird_test0260::Phase::Failed){c::fail("qa_bird_native_injury_not_proved");*buttons=0;*x=*y=0;return;}
  if(!bird_test.complete()){
   in.qa_bird=true;in.targets=bird_target?std::span<const d::Target>(&*bird_target,1):std::span<const d::Target>{};
   if(!bird_target){controller.step(in);*buttons=0;*x=*y=0;return;}
  }
 }
 const auto& saved_fish=controller.snapshot();
 const bool water_scope=state.diagnostic&&c::driver&&c::logical_room==97&&c::session.current().family==boss_battle0250::Family::Fish&&!c::restore_pending&&c::variant_epoch==state.retry_epoch&&state.player==player&&saved_fish.player==player&&saved_fish.room==in.room&&saved_fish.encounter==in.encounter&&saved_fish.fish_retry_epoch==state.retry_epoch&&saved_fish.fish_anchor_attested&&!in.qa_bird;
 const auto water=fish_water_return.step(rdram,{water_scope,c::logical_room,state.retry_epoch,state.token,saved_fish.fish_anchor_y},in);
 if(water.handled){
  if(water.stage!=fish_water_last&&fish_water_logs<48){++fish_water_logs;fish_water_last=water.stage;std::fprintf(stderr,"[BossFishWaterReturn] vi=%u player=%08X room=%u index=%u epoch=%u stage=%u completed=%u xy=%.5f,%.5f anchor=%.5f,%.5f lane=%.5f riseY=%.5f buttons=%04X stick=%.5f,%.5f nativeObservation=1 controlsOnly=1\n",vi,player,in.room,in.encounter,state.retry_epoch,unsigned(water.stage),unsigned(water.completed),in.x,in.y,in.fish_anchor_x,saved_fish.fish_anchor_y,water.lane,water.rise_y,unsigned(water.buttons),water.x,water.y);}
  if(water.completed)controller.resume_fish_navigation(in);
  else {*buttons=water.buttons;*x=water.x;*y=water.y;trace_fish_pilot(rdram,in,*buttons,*x,*y);return;}
 }
 if(in.monster_side_navigation){
  // Keep the ordinary controller's genuine R0/full-six debt and timing updated;
  // the scoped helper owns only outgoing controls during its aerial grab.
  controller.step(in);
  std::array<boss_monster_forage0284::Bird,8> owned{};for(unsigned slot=0;slot<owned.size();++slot){const auto& b=c::variants::birds[slot];owned[slot]={b.actor,b.private_data,b.resource,b.factory,b.birth,b.source_actor,b.source_private,b.source_birth};}
  const auto forage=monster_side_forage.step(rdram,cp6_monster_side_qa,controller.snapshot().monster_refilling,state.token,in,owned);
  if(forage.handled){
   if(forage.completed)controller.resume_monster_side_refill(in);
   if(buttons&&x&&y){*buttons=forage.buttons;*x=forage.x;*y=forage.y;}
   return;
  }
 }else monster_side_forage.reset();
 controller.drive(in,buttons,x,y);
 if(buttons&&x&&y)trace_fish_pilot(rdram,in,*buttons,*x,*y);
 if(c::session.current().family==boss_battle0250::Family::Monsters&&vi%60==0&&vi<12000){for(unsigned k=0;k<count;++k){const auto& r=monster_burst_results[k];std::fprintf(stderr,"[BossMonsterBurstAim] vi=%u root=%08X scene=%u param=%u eyes=%u ready=%u reason=%u passed=%u expiry=%u directStep=%u burstXY=%.5f,%.5f marginXY=%.5f,%.5f targetVelocity=%.5f,%.5f nativeObservation=1 controlsOnly=1\n",vi,targets[k].actor,unsigned(bool(monster_burst_scene)),monster_burst_scene?monster_burst_scene->param:0,monster_burst_scene?monster_burst_scene->count:0,unsigned(r.ready),unsigned(r.reason),unsigned(r.passed),r.expiry,r.contact,r.x,r.y,monster_burst_scene?monster_burst_scene->mx:0.f,monster_burst_scene?monster_burst_scene->my:0.f,targets[k].vx,targets[k].vy);}}
 if(lips_sample_root&&buttons&&x&&y&&vi%60==0&&lips_jump_logs<512){
  std::fprintf(stderr,"[BossLipsBody] vi=%u observed=%u reason=%u weakStep=%u bodyStep=%u side=%u bodyVelocity=%.6f,%.6f bodyXY=%.5f,%.5f conservativeInputPlanning=1\n",vi,unsigned(bool(lips_body_scene)),unsigned(lips_body_result.reason),lips_body_result.weak_step,lips_body_result.body_step,lips_body_result.side,lips_motion.body_vx,lips_motion.vy,lips_body_scene?lips_body_scene->body.x:0.f,lips_body_scene?lips_body_scene->body.y:0.f);
  std::fprintf(stderr,"[BossLipsEscape] vi=%u observed=%u active=%u direction=%d goalX=%.5f currentX=%.5f controlsOnly=1\n",vi,unsigned(in.lips_escape_observed),unsigned(controller.snapshot().lips_escaping),controller.snapshot().lips_escape_direction,controller.snapshot().lips_escape_x,in.x);
  fprintf(stderr,"[BossLipsMotion] vi=%u root=%08X observed=%u nativeSamples=%u sampleVI=%u velocity=%.6f,%.6f projectileSpeed=6.6 intercept=%.6f,%.6f futurePathClear=%u nativeUpdateClock=1 rootWorldY=%.6f eyeWorldY=%.6f verticalParentWorld=1 controlsOnly=1\n",vi,lips_sample_root,unsigned(lips_motion.observed),lips_motion.samples,lips_motion.vi,lips_motion.vx,lips_motion.vy,lips_intercept_x,lips_intercept_y,unsigned(lips_sample_path_clear),fl(lips_sample_root+0x88),lips_center_y);
  ++lips_jump_logs;fprintf(stderr,"[BossLipsJump] sample=%u vi=%u encounter=%u root=%08X player=%08X state=%u/%u grounded=%u xy=%.5f,%.5f C4=%.9f nativeAxes=%d,%d reason=%u boxObserved=%u halfXY=%.5f,%.5f columnObserved=%u columnClear=%u safeX=%.5f pathClear=%u eggShapeObserved=%u eggProfile=%08X eggHalfXY=%.5f,%.5f corridorObserved=%u corridorPixels=%u firstSolid=%.1f,%.1f weakCenter=%.5f,%.5f aimPointObserved=%u aimPoint=%.5f,%.5f aimOffset=%.5f,%.5f weakHalfXY=%.5f,%.5f aimCandidate=%u inputButtons=%04X inputXY=%.5f,%.5f phase=%u phaseVI=%u nativeObservation=1 controlsOnly=1\n",lips_jump_logs,vi,state.index,lips_sample_root,player,in.primary_state,MEM_BU(0x30,n::base(player)),unsigned(in.grounded),in.x,in.y,in.horizontal_speed,in.native_stick.x,in.native_stick.y,unsigned(lips_jump_debug.reason),unsigned(lips_jump_debug.body_observed),lips_jump_debug.half_width,lips_jump_debug.half_height,unsigned(in.lips_jump_clearance_observed),unsigned(in.lips_jump_column_clear),in.lips_jump_x,unsigned(lips_sample_path_clear),unsigned(bool(lips_egg_shape)),lips_egg_shape?lips_egg_shape->profile:0,lips_egg_shape?lips_egg_shape->half_x:0.f,lips_egg_shape?lips_egg_shape->half_y:0.f,unsigned(lips_corridor_observed),lips_corridor_debug.pixels_tested,lips_corridor_debug.first_solid_x,lips_corridor_debug.first_solid_y,lips_center_x,lips_center_y,unsigned(bool(lips_aim_point)),lips_aim_point?lips_aim_point->x:lips_center_x,lips_aim_point?lips_aim_point->y:lips_center_y,lips_aim_point?lips_aim_point->offset_x:0.f,lips_aim_point?lips_aim_point->offset_y:0.f,lips_aim_point?lips_aim_point->half_x:0.f,lips_aim_point?lips_aim_point->half_y:0.f,lips_aim_point?lips_aim_point->candidate:0,unsigned(*buttons),*x,*y,unsigned(controller.snapshot().phase),controller.snapshot().phase_vi);
 }

 // Logs native x/y, horizontal/vertical speed, desired speed and acceleration
 // every six VI only during an authorized Monster return. No RDRAM writes,
 // collisions, physics changes or input changes. Native support is not inferred
 // from coordinates: it comes from observe_monster_combat_support above.
 if(c::session.snapshot().diagnostic&&in.monster_navigation&&controller.snapshot().monster_returning&&
    buttons&&x&&y&&vi%6==0&&monster_return_motion_logs<384){
  ++monster_return_motion_logs;
  const float vx=fl(player+0xC4),vy=fl(player+0xC8);
  const float desired=fl(player+0xDC),accel=fl(player+0xE8);
  const uint32_t parent=n::word(rdram,player+0x180);
  fprintf(stderr,"[BossReturnMotion] sample=%u vi=%u encounter=%u player=%08X xy=%.5f,%.5f velocity=%.5f,%.5f desiredX=%.5f accelerationX=%.5f grounded=%u nativeCombatSupport=%u parent=%08X phase=%u age=%u inputButtons=%04X inputXY=%.4f,%.4f nativeObservation=1 controlsOnly=1\n",
    monster_return_motion_logs,vi,state.index,player,in.x,in.y,vx,vy,desired,accel,unsigned(in.grounded),unsigned(in.combat_supported),parent,
    unsigned(controller.snapshot().phase),vi-controller.snapshot().phase_vi,unsigned(*buttons),*x,*y);
 }
 if(vi%120==0){fprintf(stderr,"[BossDriverNative] vi=%u player=%08X state=%u reserves=%u aim=%08X reticle=%08X position=%.1f,%.1f targets=%u supplies=%u controls=%u nativeAxes=%d,%d inverse=%u supplyStand=%u supplyReady=%u\n",vi,player,in.primary_state,in.reserves,in.aiming_actor,in.reticle_actor,in.x,in.y,count,supplied,unsigned(in.controls),in.native_stick.x,in.native_stick.y,unsigned(in.native_stick_observed),unsigned(in.has_supply_stand),unsigned(controller.snapshot().supply_ready));
  if(in.genie_navigation&&in.genie_support_observed&&!in.reserves&&!in.aiming_actor&&genie_supply_log_windows<48){
   ++genie_supply_log_windows;
   fprintf(stderr,"[BossGenieSupply] sample=%u vi=%u encounter=%u player=%08X playerXY=%.3f,%.3f bounds=%.1f,%.1f anchor=%.1f supplies=%u selected=%08X phase=%u mouthFull=%u tongue=%u/%u nativeObservation=1 controlsOnly=1\n",genie_supply_log_windows,vi,state.index,player,in.x,in.y,in.min_x,in.max_x,in.genie_anchor_x,supplied,controller.snapshot().supply_actor,unsigned(controller.snapshot().phase),unsigned(in.mouth_full),in.native_tongue_phase,unsigned(in.native_tongue_live));
   for(unsigned slot=0;slot<std::min(supplied,8u);++slot){const auto& s=supplies[slot];fprintf(stderr,"[BossGenieSupplySource] sample=%u vi=%u slot=%u actor=%08X kind=%04X xy=%.3f,%.3f flags=%04X feederKind=%04X feederFlags=%04X feederAxis=%u active=%u collectible=%u valid=%u insideSupport=%u nativeObservation=1 controlsOnly=1\n",genie_supply_log_windows,vi,slot,s.actor,unsigned(s.kind),s.x,s.y,unsigned(s.food_flags),unsigned(s.feeder_kind),unsigned(s.feeder_flags),unsigned(s.feeder_axis),unsigned(s.active),unsigned(s.collectible),unsigned(d::supply_valid(s)),unsigned(s.x>=in.min_x&&s.x<=in.max_x));}
  }
  if(controller.snapshot().phase==d::Phase::Supply){const d::Supply* bird=nullptr;float near_distance=1e30f;for(const auto& supply:in.supplies)if(d::bird_supply(supply)){const float distance=std::hypot(supply.x-in.x,supply.y-in.y);if(distance<near_distance){bird=&supply;near_distance=distance;}}if(bird)fprintf(stderr,"[BossDriverSupply] vi=%u encounter=%u actor=%08X kind=%04X flags=%04X position=%.2f,%.2f delta=%.2f,%.2f grounded=%u nativeObservation=1 controlsOnly=1\n",vi,state.index,bird->actor,bird->kind,bird->food_flags,bird->x,bird->y,bird->x-in.x,bird->y-in.y,unsigned(in.grounded));}
 }
}
}
