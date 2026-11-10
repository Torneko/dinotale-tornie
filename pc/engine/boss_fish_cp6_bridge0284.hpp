#pragma once

#include "boss_driver_bridge0260.hpp"

#include "boss_route_checkpoint_bridge0260.hpp"

#include "boss_fish_cp6_test0284.hpp"
#include "boss_fish_cp6_fixture0284.hpp"



namespace workshop::boss_fish_cp6_bridge0284 {

inline boss_driver0260::Controller companion_controller;

inline bool companion_configured=false;

inline std::string companion_token;

inline void reset_companion(){companion_controller.configure(false,false);companion_configured=false;companion_token.clear();}



// QA input ownership only: a filtered native4072 target never masks a Fish

// member in the campaign, changes its flags, or manufactures a defeat receipt.

inline bool drive_companion(uint8_t* ram,unsigned vi,uint16_t* buttons,float* x,float* y){

 namespace c=boss_campaign0250;namespace b=boss_battle0250;namespace n=boss_native;namespace d=boss_driver0260;namespace bridge=boss_driver_bridge0260;

 auto* rdram=ram;

 if(!ram||!buttons||!x||!y||!vi||!c::active()||!c::driver)return false;

 const auto& s=c::session.snapshot();const auto& e=c::session.current();

 if(!s.diagnostic||s.token.empty()||c::session.plan().seed!=37784098137700ull||s.phase!=b::Phase::Fighting||s.index!=6||s.completed!=6||s.retry_epoch>1||s.checkpoint_completed!=6||!(s.checkpoint_awake_mask&4)||c::logical_room!=97||c::physical_room!=173||e.family!=b::Family::Fish||e.variant!=2||e.count!=2||e.members[0].kind!=0x4511||e.members[1].kind!=0x4072||(s.defeated&2))return false;

 const auto player=s.player,priv=n::word(ram,player+0x1F4);

 if(!n::actor(ram,player,0x4001)||n::word(ram,0x800F8828)!=player||!n::range(priv,0x12D0)||c::fatal_player(ram,player))return false;

 // Let the ordinary bridge retain native dialogue acknowledgements until

 // controls are actually released. Once released this helper owns all input.

 if(!bridge::observe_controls(ram,player))return false;

 *buttons=0;*x=*y=0;

 if(!companion_configured||companion_token!=s.token){companion_controller.configure(true,true);companion_configured=true;companion_token=s.token;}

 std::array<uint32_t,6> eggs{};if(!boss_solo0240::read_reserves(ram,player,eggs))return true;

 const auto& v=c::variants::snapshot();const auto& binding=v.members[1];const auto actor=c::bound[1];

 if(c::restore_pending||c::variant_epoch!=s.retry_epoch||!v.enabled||v.invalidated||v.encounter!=s.index||v.room!=97||binding.actor!=actor||!binding.birth||binding.receipt_sent||binding.retire_queued||!n::actor(ram,actor,0x4072))return true;

 auto fl=[&](uint32_t at){return std::bit_cast<float>(n::word(ram,at));};

 auto live=[&](uint32_t a){return n::range(a,0x1FC)&&n::word(ram,a)==0x19971110&&n::byte(ram,a+6)!=3;};

 const auto& arena=c::rooms::arena(c::physical_room);

 d::Input in;in.vi=vi;in.room=c::physical_room;in.encounter=s.index;in.player=player;in.combat=true;in.controls=true;in.alive=true;

 in.primary_state=n::byte(ram,player+0x18);in.grounded=n::byte(ram,priv+0xF4)==0;in.spring_attached=bridge::observe_spring_attachment(ram,player);

 in.x=fl(player+0x84);in.y=fl(player+0x88);in.min_x=arena.min_x;in.max_x=arena.max_x;in.reserves=unsigned(std::count_if(eggs.begin(),eggs.end(),[](auto a){return a!=0;}));

 in.native_stick={int(MEM_H(0,n::base(0x800F897E))),int(MEM_H(0,n::base(0x800F8980)))};in.native_stick_observed=boss_stick_aim0260::valid(in.native_stick);

 std::array<float,80> mouth_x{},mouth_y{};bridge::observe_native_mouth(ram,player,mouth_x,mouth_y,in.native_mouth);

 const auto tongue=boss_native_mouth0260::observe_ordinary_tongue(ram,player);

 in.native_tongue_observed=true;in.native_tongue_ready=tongue.observed&&tongue.ready;in.native_tongue_known=tongue.observed;in.native_tongue_live=tongue.live;in.native_tongue_cancelled_idle=tongue.cancelled_idle;

 in.native_tongue_phase=tongue.phase;in.native_tongue_callback_pending=tongue.callback_pending;in.native_tongue_max_reach=tongue.max_reach;in.native_tongue_heading_degrees=tongue.heading_degrees;in.native_tongue_heading_active=tongue.heading_active;

 const auto request=bridge::observe_egg_request(ram,player);in.native_egg_request_observed=request.observed;in.native_egg_request_ready=request.ready;

 in.throw_mode=n::word(ram,priv+0x1E0)==1?d::ThrowMode::SecondPress:d::ThrowMode::HoldAndRelease;in.mouth_full=live(n::word(ram,priv+0x164));

 const auto egg_aim=bridge::observe_egg_aim(ram,player);in.aiming_actor=egg_aim.held;in.reticle_actor=egg_aim.reticle;in.reticle_x=egg_aim.reticle_x;in.reticle_y=egg_aim.reticle_y;

 std::array<d::Target,1> targets{};const auto motion=bridge::observe_genie_motion(ram,1);

 if(motion){const auto primary=n::byte(ram,actor+0x18),sub=n::byte(ram,actor+0x30);targets[0]={actor,0x4072,motion->x,motion->y,motion->vx,motion->vy,primary!=0xFF,primary==1&&sub!=4&&sub!=5,false,false,true,6.6f};targets[0].birth=binding.birth;in.targets=targets;}

 std::array<d::Supply,150> supplies{};unsigned supplied=0;constexpr uint32_t queue=0x800F8814;const auto cap=uint16_t(MEM_HU(8,n::base(queue)));const auto table=n::word(ram,queue+0x10);

 if(cap>150||!n::range(table,unsigned(cap)*4))return true;

 for(unsigned k=0;k<cap;++k){const auto a=n::word(ram,table+k*4);if(!live(a)||n::byte(ram,a+0x17D)!=2)continue;const auto kind=uint16_t(MEM_HU(12,n::base(a)));const auto state=n::byte(ram,a+0x18);


  if(kind==0x4002&&state==0x68&&!n::word(ram,a+0x180))in.projectile_actor=a;

  if((kind==0x4002&&state!=0x68&&!n::word(ram,a+0x180))||kind==0x4086)supplies[supplied++]={a,kind,fl(a+0x84),fl(a+0x88),state!=0xFF,kind==0x4002};

  if(kind==0x4526||kind==0x46DA){const auto flags=bridge::egg_food_flags(ram,a);if(flags&1)supplies[supplied++]={a,kind,fl(a+0x84),fl(a+0x88),state!=0xFF,false,flags};}

  if(kind==0x456E){d::Supply source;if(bridge::observe_feeder(ram,a,in.y,source))supplies[supplied++]=source;}

 }

 in.supplies={supplies.data(),supplied};in.stable_stands=arena.safe_pot_xs;

 bridge::observe_fish_navigation(ram,player,in.room,in.encounter,arena.safe_pot_xs,in);

 // The shared observer's global pilot is idle while this helper owns input.

 // Rebind its read-only continuity observations to THIS pilot's actual epoch.

 const auto& saved=companion_controller.snapshot();const bool same=saved.player==player&&saved.room==in.room&&saved.encounter==in.encounter&&saved.fish_retry_epoch==s.retry_epoch;

 std::optional<float> previous;if(same&&saved.fish_anchor_attested)previous=saved.fish_anchor_x;

 in.fish_island_observed=in.fish_on_island=false;

 if(const auto island=boss_fish_support0260::observe(ram,player,in.room,in.encounter,arena.safe_pot_xs,previous)){in.fish_island_observed=true;in.fish_on_island=island->on_island;in.fish_anchor_x=island->x;}

 const bool keeping=same&&(saved.fish_evading||saved.fish_evade_pending_z||saved.fish_evade_sending_z||saved.fish_evade_acquire_z);

 std::array<boss_fish_evade0260::Root,1> roots{};unsigned roots_count=0;const auto& fish=v.members[0];const auto fish_actor=c::bound[0];in.fish_evade_generation_valid=!keeping;

 if(!(s.defeated&1)&&fish.actor==fish_actor&&fish.birth&&!fish.receipt_sent&&!fish.retire_queued&&n::actor(ram,fish_actor,0x4511)){

  const auto sub=n::byte(ram,fish_actor+0x30);if(n::byte(ram,fish_actor+0x18)==1&&sub<boss_fish_green0260::updates.size()&&boss_fish_green0260::updates[sub]){roots[roots_count++]={fish_actor,fish.birth};if(fish_actor==saved.fish_evade_actor&&fish.birth==saved.fish_evade_birth)in.fish_evade_generation_valid=true;}

 }

 const auto threat=boss_fish_evade0260::observe(ram,player,{roots.data(),roots_count},in.room,in.encounter);

 in.fish_evade_observed=threat.observed;in.fish_evade_imminent=threat.imminent;in.fish_evade_head_clear=threat.head_clear;in.fish_threat_actor=threat.actor;in.fish_threat_birth=threat.birth;in.fish_threat_step=threat.step;in.fish_threat_water=false;in.fish_threat_resource=in.fish_threat_private=0;

 const boss_fish_water_evade0260::Identity old_water{saved.fish_evade_actor,saved.fish_evade_resource,saved.fish_evade_private};const bool water_hold=keeping&&saved.fish_evading&&saved.fish_evade_water&&vi>=saved.fish_evade_vi&&vi-saved.fish_evade_vi<45;

 const auto water=boss_fish_water_evade0260::observe(ram,player,in.room,in.encounter,keeping&&saved.fish_evade_water&&!water_hold?&old_water:nullptr);

 if(keeping&&saved.fish_evade_water)in.fish_evade_generation_valid=water.previous_valid;

 if(water.observed&&((keeping&&saved.fish_evade_water)||(!keeping&&water.imminent&&(!threat.imminent||water.step<=threat.step)))){in.fish_evade_observed=water.observed;in.fish_evade_imminent=water.imminent;in.fish_evade_head_clear=water.head_clear;in.fish_threat_actor=water.actor;in.fish_threat_birth=0;in.fish_threat_step=water.step;in.fish_threat_water=true;in.fish_threat_resource=water.identity.resource;in.fish_threat_private=water.identity.private_data;}

 companion_controller.drive(in,buttons,x,y);return true;

}

} // namespace workshop::boss_fish_cp6_bridge0284

namespace workshop::boss_fish_cp6_bridge0284 {

namespace c=boss_campaign0250;namespace q=boss_fish_cp6_test0284;namespace b=boss_battle0250;namespace n=boss_native;

// Boot pulses depend on real menu scenes and a later native pad release.
// They never advance the campaign or claim that Nintendo rendering recovered.
struct BootFrame {
 bool scope=false,pad_observed=false;unsigned vi=0,stage=0,scene0=0,scene6=0;
 uint16_t held=0,pressed=0;std::string_view token;
};
struct BootOutput {bool handled=false;uint16_t buttons=0;};
class BootPulseController {
 unsigned key_=0,last_vi_=0,scene_vi_=0,release_vi_=0,pulse_vi_=0,next_vi_=0,pulses_=0;
 bool pulsing_=false,failed_=false;std::string token_;const char* failure_="";
 void stop(const char* why,const BootFrame& f){failed_=true;failure_=why;std::fprintf(stderr,"[BossFishCP6BootQA] phase=failed vi=%u stage=%u scene0=%u scene6=%u pulses=%u reason=%s inputOnly=1\n",f.vi,f.stage,f.scene0,f.scene6,pulses_,why);}
 void leave(){key_=0;scene_vi_=release_vi_=pulse_vi_=next_vi_=pulses_=0;pulsing_=false;}
public:
 void reset(){*this=BootPulseController{};}
 bool failed()const{return failed_;}const char* failure()const{return failure_;}
 BootOutput step(const BootFrame& f){
  if(!f.scope||!f.vi||f.token.empty()){leave();return {};}
  const unsigned key=f.stage==0&&f.scene0==4?1:f.stage==0&&f.scene0==5?2:f.stage==6&&f.scene6==2?3:0;
  if(!key){leave();return {};}
  if(failed_)return {true};
  if(last_vi_&&f.vi<last_vi_){stop("boot_vi_reversed",f);return {true};}last_vi_=f.vi;
  if(token_.empty())token_=f.token;else if(token_!=f.token){stop("boot_token_changed",f);return {true};}
  if(key_!=key){leave();key_=key;scene_vi_=release_vi_=f.vi;return {true};}
  if(f.vi-scene_vi_>=3600){stop("boot_scene_deadline",f);return {true};}
  const uint16_t button=key==1?0x1000:0x8000;
  if(!f.pad_observed){pulsing_=false;release_vi_=f.vi;return {true};}
  if(pulsing_){if(f.vi-pulse_vi_<6)return {true,button};pulsing_=false;release_vi_=f.vi;return {true};}
  // A neutral host output is not a release receipt. Both native held and
  // pressed A/Start must be clear in a later VI before each fresh edge.
  if(f.vi<=release_vi_||f.vi<next_vi_||((f.held|f.pressed)&0x9000))return {true};
  if(pulses_>=8){stop("boot_pulse_limit",f);return {true};}
  ++pulses_;pulse_vi_=f.vi;next_vi_=f.vi+120;pulsing_=true;
  std::fprintf(stderr,"[BossFishCP6BootQA] phase=pulse vi=%u stage=%u scene0=%u scene6=%u buttons=%04X pulse=%u nativeReleased=1 inputOnly=1\n",f.vi,f.stage,f.scene0,f.scene6,unsigned(button),pulses_);
  return {true,button};
 }
};
inline bool boot_session_scope(const b::Snapshot& s,bool qa_enabled,bool campaign_enabled,bool driver,q::Phase qa_phase,uint64_t seed,uint32_t qa_player,uint32_t singleton,q::StartingScope start=q::StartingScope::Full){
 const bool entry=start==q::StartingScope::FixtureRest6?(s.index==5&&s.completed==6&&s.max_completed==6&&!s.checkpoint_completed&&!s.checkpoint_awake_mask):(!s.index&&!s.completed&&!s.max_completed);
 return qa_enabled&&campaign_enabled&&driver&&qa_phase==q::Phase::Prefix&&seed==q::required_seed&&s.diagnostic&&s.phase==b::Phase::Armed&&entry&&!s.retry_epoch&&!s.player&&!qa_player&&!singleton&&!s.token.empty();
}
inline q::Controller controller;inline bool enabled=false;inline BootPulseController boot;
inline BootFrame read_boot_frame(uint8_t* ram,unsigned vi){
 BootFrame f;if(!ram||!c::enabled)return f;const auto& s=c::session.snapshot();
 f.scope=boot_session_scope(s,enabled,c::enabled,c::driver,controller.snapshot().phase,c::session.plan().seed,controller.snapshot().player,n::word(ram,0x800F8828),controller.snapshot().starting_scope);f.vi=vi;f.token=s.token;
 // BookLink's manager800F7EA0: stage+2, scene0+57D. Yoshi uses
 // its actual stage6 scene+593, not BookLink's legacy scene0 snapshot.
 f.stage=n::byte(ram,0x800F7EA2);f.scene0=n::byte(ram,0x800F841D);f.scene6=n::byte(ram,0x800F8433);
 const auto pad=n::word(ram,0x800F7EA8);if(!(pad&3)&&n::range(pad,16)){f.pad_observed=true;f.held=uint16_t(boss_checkpoint_inventory0260::half(ram,pad));f.pressed=uint16_t(boss_checkpoint_inventory0260::half(ram,pad+0xC));}return f;
}


inline void configure(bool on=false,q::StartingScope start=q::StartingScope::Full,q::Goal goal=q::Goal::ConsumedReplay){enabled=on;controller.configure(on,start,goal);reset_companion();boot.reset();boss_fish_cp6_fixture0284::configure(on&&start==q::StartingScope::FixtureRest6);boss_driver_bridge0260::configure_cp6_monster_side_qa(on&&start==q::StartingScope::Full);}
inline bool start_focused_fixture(){return enabled&&controller.snapshot().starting_scope==q::StartingScope::FixtureRest6&&boss_fish_cp6_fixture0284::start();}
inline bool fixture_pre_fence(uint8_t* ram,recomp_context* ctx,unsigned vi){return boss_fish_cp6_fixture0284::pre_fence(ram,ctx,vi);}

inline bool complete(){return enabled&&controller.complete();}

inline bool failed(){return enabled&&(controller.failed()||boot.failed()||boss_fish_cp6_fixture0284::state.failed);}

inline const q::Snapshot& snapshot(){return controller.snapshot();}

inline bool genuine_receipt(const b::NativeReceipt& p,unsigned index,unsigned member,unsigned epoch){return p.encounter==index&&p.member==member&&p.actor&&p.vi&&p.initialized&&p.active&&p.terminal&&p.causal_hit&&p.retry_epoch==epoch;}

inline q::Frame make_frame(uint8_t* ram,unsigned vi,std::array<boss_route_checkpoint_test0260::Target,4>& targets){

 q::Frame f;static_cast<boss_route_checkpoint_test0260::Frame&>(f)=boss_route_checkpoint_bridge0260::make_frame(ram,vi,targets);

 if(!ram||!c::enabled||!f.session)return f;const auto& s=*f.session;const auto& p=c::session.plan();const auto& e=p.fights[6];

 f.target_plan=p.seed==q::required_seed&&b::valid(p)&&e.family==b::Family::Fish&&e.variant==2&&e.terrain==1&&e.count==2&&e.members[0].kind==0x4511&&e.members[0].hits==3&&e.members[1].kind==0x4072&&e.members[1].hits==2;

 f.baseline=c::checkpoints::baseline_ready;f.prefix_receipts=controller.snapshot().starting_scope==q::StartingScope::Full;f.focused_fixture_ready=boss_fish_cp6_fixture0284::state.ready;

 for(unsigned index=0;index<6;++index)for(unsigned member=0;member<p.fights[index].count;++member)if(!genuine_receipt(c::history[index].members[member],index,member,0))f.prefix_receipts=false;

 f.saved_pose=&c::checkpoints::selected_pose();f.cargo_epoch=c::cargo_epoch;

 const auto& ready=c::fight_ready;f.fight_ready_vi=ready.vi;f.fight_ready=ready.vi&&ready.index==6&&ready.room==173&&ready.source==97&&ready.epoch==s.retry_epoch&&ready.player==s.player&&n::word(ram,s.player+0x1F4)==ready.private_data;

 // Native4072 lifetime belongs to MemberObserver, not mutable Variants.
 // Reuse the production pair receipt and its typed resource/private getters;
 // the introductory1/0 actor is initialized/alive before active_seen.
 const auto pair=c::fish_companion_intro_frame(ram);
 const bool native_pair=pair.active&&pair.index==6&&pair.room==173&&pair.source==97&&pair.epoch==s.retry_epoch&&pair.player==s.player&&pair.token==s.token&&
  boss_fish_companion_intro0260::member(ram,pair.fish,s.retry_epoch,0x188,0x3FF0,0x35A8)&&boss_fish_companion_intro0260::member(ram,pair.companion,s.retry_epoch,0x28,0x3200,0x2A9C);
 const auto& variants=c::variants::snapshot();f.members_live=native_pair&&variants.enabled&&!variants.invalidated&&variants.encounter==6&&variants.room==97&&c::variant_epoch==s.retry_epoch;

 for(unsigned member=0;member<2;++member){const auto& m=variants.members[member];f.members[member]=c::bound[member];f.births[member]=m.birth;

  if(!f.members[member]||m.actor!=f.members[member]||!m.birth||!n::actor(ram,m.actor,e.members[member].kind)||(member==0&&!c::variants::live(ram,0))||m.receipt_sent||m.retire_queued)f.members_live=false;

 }

 const auto& companion=c::history[6].members[1];f.companion_receipt=genuine_receipt(companion,6,1,0)&&companion.actor==controller.snapshot().first_members[1];

 const auto& cache=c::retry_placements.snapshot()[6];const auto& detached=c::placement_guard.snapshot()[1];const auto& qa=controller.snapshot();const bool exact_link=detached.captured&&detached.bound&&detached.initialized&&detached.actor==qa.first_members[1]&&detached.birth==qa.first_births[1]&&detached.record==cache.placements[1].record&&detached.receipt_vi==companion.vi&&detached.player==s.player;

 f.companion_pending=exact_link&&detached.pending;f.companion_consumed=exact_link&&detached.applied&&!detached.pending&&cache.captured&&cache.source==173&&cache.index==6&&cache.count==2&&n::range(cache.placements[1].record,16)&&n::word(ram,cache.placements[1].record)==0xFFFFFFFF&&(!n::actor(ram,detached.actor,0x4072)||(!n::word(ram,detached.actor+0x1D8)&&!n::word(ram,detached.actor+0x1DC)));

 f.replay_receipts=true;for(unsigned member=0;member<2;++member){const auto& receipt=c::history[6].members[member];if(!genuine_receipt(receipt,6,member,1)||receipt.actor!=controller.snapshot().replayed_members[member]||receipt.vi<=controller.snapshot().restored_vi)f.replay_receipts=false;}

 if(s.retry_epoch==1&&cache.restored_epoch!=1)f.members_live=false;

 return f;

}

inline bool drive(uint8_t* ram,unsigned vi,uint16_t* buttons,float* x,float* y){

 if(!enabled||!ram||!buttons||!x||!y||!c::enabled)return false;const auto boot_out=boot.step(read_boot_frame(ram,vi));if(boot_out.handled){*buttons=boot_out.buttons;*x=*y=0;return true;}if(controller.snapshot().starting_scope==q::StartingScope::FixtureRest6&&!boss_fish_cp6_fixture0284::state.ready&&n::actor(ram,n::word(ram,0x800F8828),0x4001)){*buttons=uint16_t(*buttons&0x8000);*x=*y=0;return true;}std::array<boss_route_checkpoint_test0260::Target,4> targets{};const auto out=controller.frame(ram,make_frame(ram,vi,targets));

 if(!out.handled)return false;if(out.companion){if(!drive_companion(ram,vi,buttons,x,y)){*buttons=uint16_t(*buttons&0x8000);*x=*y=0;}return true;}

 *buttons=out.intro_ack_only?uint16_t(*buttons&0x8000):out.buttons;*x=out.x;*y=out.y;return true;

}

inline void observe(uint8_t* ram,unsigned vi){if(!enabled||!ram||!c::enabled)return;std::array<boss_route_checkpoint_test0260::Target,4> targets{};controller.frame(ram,make_frame(ram,vi,targets));}

inline void native_death(uint8_t* ram,const recomp_context* ctx,unsigned vi){if(!enabled||!ram||!ctx||!c::enabled)return;std::array<boss_route_checkpoint_test0260::Target,4> targets{};controller.native_death(ram,ctx,make_frame(ram,vi,targets));}

} // namespace workshop::boss_fish_cp6_bridge0284

