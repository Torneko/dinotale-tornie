#pragma once
#include "boss_route_checkpoint_test0260.hpp"
#include "boss_driver_bridge0260.hpp"

namespace workshop::boss_route_checkpoint_bridge0260 {
namespace c=boss_campaign0250;namespace q=boss_route_checkpoint_test0260;namespace n=boss_native;
inline bool enabled=false;inline q::Controller controller;
struct EggCallScope {
 bool valid=false;uint32_t player=0,private_data=0;unsigned vi=0,room=177,logical_room=177,encounter=10,epoch=0;uint64_t seed=0;std::string token;bool fish_loadout=false;
};
inline bool same_egg_scope(const EggCallScope& a,const EggCallScope& b){return a.valid&&b.valid&&a.player==b.player&&a.private_data==b.private_data&&a.room==b.room&&a.logical_room==b.logical_room&&a.encounter==b.encounter&&a.epoch==b.epoch&&a.seed==b.seed&&a.token==b.token&&a.fish_loadout==b.fish_loadout;}
inline bool live_egg_scope(uint8_t* ram,const EggCallScope& s){
 if(!ram||!s.valid||!s.vi||s.token.empty()||s.room>=177||s.room==83||!n::actor(ram,s.player,0x4001)||n::word(ram,0x800F8828)!=s.player||!n::range(s.private_data,0x12CA)||n::word(ram,s.player+0x1F4)!=s.private_data)return false;
 if(s.fish_loadout)return ((s.seed==7&&s.encounter==3)||(s.seed==17&&s.encounter==2))&&s.logical_room==97&&s.room==167+s.encounter&&s.epoch==0;
 return s.seed==1&&(s.encounter==3||s.encounter==6||s.encounter==9)&&s.logical_room==(s.encounter==9?162u:151u);
}
class NativeEggWitness {
 EggCallScope wrapper_{},request_{};uint32_t request_ra_=0;
public:
 void reset(){wrapper_={};request_={};request_ra_=0;}
 bool observe(uint8_t* ram,const recomp_context* ctx,std::string_view function,const EggCallScope& s){
  if(function!="func_8003BEC8"&&function!="func_8003BE44")return false;
  if(!ctx||!live_egg_scope(ram,s)||uint32_t(ctx->r4)!=s.player)return false;
  if(function=="func_8003BEC8"){wrapper_=s;return false;}
  // Actual inner entry, same dispatch/VI and P1/private as the observed wrapper.
  // RA is reported, not used as an inferred C/MIPS call guard.
  const auto gates=boss_driver_bridge0260::observe_egg_request(ram,s.player);
  if(!same_egg_scope(wrapper_,s)||wrapper_.vi!=s.vi||uint32_t(ctx->r5)!=s.private_data||!gates.observed||!gates.ready)return false;
  const bool first=!same_egg_scope(request_,s);request_=s;request_ra_=uint32_t(ctx->r31);return first;
 }
 bool recent(uint8_t* ram,const EggCallScope& s)const{return live_egg_scope(ram,s)&&same_egg_scope(request_,s)&&s.vi>=request_.vi&&s.vi-request_.vi<=2;}
 unsigned vi()const{return request_.vi;}uint32_t ra()const{return request_ra_;}
};
inline NativeEggWitness egg_witness;
inline void configure(bool on=false){enabled=on;controller.configure(on);egg_witness.reset();}
inline bool complete(){return enabled&&controller.complete();}
inline EggCallScope current_egg_scope(uint8_t* ram,unsigned vi){
 EggCallScope out;if(!ram||!enabled||!c::active()||!c::driver)return out;const auto& s=c::session.snapshot();const auto& qa=controller.snapshot();
 if(!s.diagnostic||s.phase!=boss_battle0250::Phase::Fighting||!qa.enabled||(qa.phase!=q::Phase::AwaitFight&&qa.phase!=q::Phase::ShotHold&&qa.phase!=q::Phase::ShotRelease)||qa.player!=s.player||qa.checkpoint!=s.index||s.completed!=s.index||s.retry_epoch!=qa.activation_epoch)return out;
 out={true,s.player,n::word(ram,s.player+0x1F4),vi,c::physical_room,c::logical_room,s.index,s.retry_epoch,c::session.plan().seed,s.token};if(!live_egg_scope(ram,out))out.valid=false;return out;
}
inline void native_egg_request(uint8_t* ram,const recomp_context* ctx,std::string_view function,unsigned vi){
 if(function!="func_8003BEC8"&&function!="func_8003BE44")return;const auto scope=current_egg_scope(ram,vi);
 if(egg_witness.observe(ram,ctx,function,scope))std::fprintf(stderr,"[BossRouteEggAdmission] player=%08X private=%08X vi=%u room=%u source=%u index=%u epoch=%u seed=%llu token=%s wrapper=8003BEC8 request=8003BE44 ra=%08X nativeCall=1\n",scope.player,scope.private_data,scope.vi,scope.room,scope.logical_room,scope.encounter,scope.epoch,static_cast<unsigned long long>(scope.seed),scope.token.c_str(),egg_witness.ra());
}
struct ShotAdmission {bool observed=false,ready=false,pad_observed=false,released=false;};
inline ShotAdmission observe_shot_admission(uint8_t* ram,uint32_t player,const EggCallScope& scope,const NativeEggWitness& witness){
 ShotAdmission out;const auto native=boss_driver_bridge0260::observe_egg_request(ram,player);out.observed=native.observed;if(!native.observed)return out;
 // A real recent native wrapper->request dispatch replaces guessed pose,
 // queue, parent and callback signatures. The unchanged native gates remain.
 out.ready=native.ready&&scope.player==player&&witness.recent(ram,scope);
 const auto pad=n::word(ram,0x800F7EA8);if(!(pad&3)&&n::range(pad,16)){out.pad_observed=true;out.released=(boss_checkpoint_inventory0260::half(ram,pad)&0x200F)==0&&(boss_checkpoint_inventory0260::half(ram,pad+0xC)&0x200F)==0;}
 return out;
}
inline ShotAdmission observe_shot_admission(uint8_t* ram,uint32_t player,unsigned vi=0){return observe_shot_admission(ram,player,current_egg_scope(ram,vi),egg_witness);}
inline void apply_output(const q::Output& out,uint16_t* buttons,float* x,float* y){*buttons=out.intro_ack_only?uint16_t(*buttons&0x8000):out.buttons;*x=out.x;*y=out.y;}
inline std::optional<q::Target> observe_candy_contact(uint8_t* ram,uint32_t player,uint32_t actor,uint64_t birth,uint32_t observer_actor,const boss_family_native0250::Snapshot& proof,unsigned source,std::string_view token,bool diagnostic){
 const auto& candy=proof.actors[0];
 if(!proof.enabled||proof.invalidated||proof.family!=boss_family_native0250::Family::candy||proof.source_room!=source||proof.run_token!=token||proof.diagnostic!=diagnostic||proof.expected_actors!=1||proof.registered_actors!=1||proof.expected_hits!=12||!candy.initialized||candy.disposed||candy.completion_returned||candy.terminal_seen||candy.actor!=actor||observer_actor!=actor)return {};
 const q::Target contact{candy.head,birth,actor,candy.private_data,candy.head_private,player};return q::candy_contact(ram,contact,player)?std::optional<q::Target>(contact):std::nullopt;
}
inline q::Frame make_frame(uint8_t* ram,unsigned vi,std::array<q::Target,4>& targets){
 q::Frame f;if(!ram||!c::enabled)return f;const auto& s=c::session.snapshot();
 f.diagnostic=s.diagnostic;f.driver=c::driver;f.room=c::physical_room;f.logical_room=c::logical_room;f.vi=vi;f.seed=c::session.plan().seed;
 f.session=&s;f.characters=&c::checkpoints::characters.snapshot();f.saved=&c::checkpoints::selected_inventory();f.retry_restored_epoch=c::checkpoints::retry_restored_epoch();
 f.controls=s.player&&!n::byte(ram,0x800F7EA5)&&boss_driver_bridge0260::observe_controls(ram,s.player);
 const auto shot=observe_shot_admission(ram,s.player,vi);f.egg_admission_observed=shot.observed;f.egg_admitted=shot.ready;f.pad_observed=shot.pad_observed;f.z_released=shot.released;
 f.ledger=q::observe_ledger(ram,boss_tree0240::snapshot(),boss_tree_hybrid0260::snapshot(),c::rest_heart.snapshot());
 unsigned count=0;if(s.phase==boss_battle0250::Phase::Fighting)for(unsigned member=0;member<c::session.current().count&&member<4;++member){
  const auto& b=c::variants::snapshot().members[member];const auto actor=c::bound[member];
  if(!(s.defeated&(1u<<member))&&actor&&b.actor==actor&&b.birth&&c::variants::live(ram,member)&&n::actor(ram,actor,c::session.current().members[member].kind)){
   if(c::session.current().members[member].kind==0x455A){
    // Sampled Candy identity includes intro head updates; active_seen remains
    // a completion proof, not a guessed navigation gate.
    if(const auto contact=observe_candy_contact(ram,s.player,actor,b.birth,c::candies[member].bound_actor(),c::candies[member].snapshot(),f.logical_room,s.token,s.diagnostic))targets[count++]=*contact;
   }else targets[count++]={actor,b.birth};
  }
 }
 f.targets={targets.data(),count};return f;
}
inline void propagate_failure(){if(enabled&&controller.failed()&&c::session.snapshot().phase!=boss_battle0250::Phase::Failed)c::fail(controller.snapshot().failure);}
inline bool drive(uint8_t* ram,unsigned vi,uint16_t* buttons,float* x,float* y){
 if(!enabled||!ram||!c::enabled||!c::session.snapshot().player||!buttons||!x||!y)return false;
 std::array<q::Target,4> targets{};const auto f=make_frame(ram,vi,targets);const auto out=controller.frame(ram,f);propagate_failure();
 if(!out.handled&&!controller.failed())return false;
 apply_output(out,buttons,x,y);return true;
}
inline void observe(uint8_t* ram,unsigned vi){
 if(!enabled||!ram||!c::enabled||!c::session.snapshot().player)return;std::array<q::Target,4> targets{};controller.frame(ram,make_frame(ram,vi,targets));propagate_failure();
}
inline void native_death(uint8_t* ram,const recomp_context* context,unsigned vi){
 if(!enabled||!ram||!context||!c::enabled||!c::session.snapshot().player)return;std::array<q::Target,4> targets{};controller.native_death(ram,context,make_frame(ram,vi,targets));propagate_failure();
}
} // namespace workshop::boss_route_checkpoint_bridge0260
