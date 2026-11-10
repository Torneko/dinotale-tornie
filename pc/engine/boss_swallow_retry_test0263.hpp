#pragma once
#include "boss_battle_runtime0250.hpp"
#include <cstdio>
#include <cmath>
namespace workshop::boss_swallow_retry_test0263 {
namespace c=boss_campaign0250;namespace b=boss_battle0250;namespace n=boss_native;namespace inv=boss_checkpoint_inventory0260;
inline bool enabled=false,failed=false,armed=false;inline unsigned first_vi=0,first_jump_vi=0,fish_vi=0,death_vi=0;inline inv::Payload saved{};
inline void configure(bool on){enabled=on;failed=armed=false;first_vi=first_jump_vi=fish_vi=death_vi=0;saved={};}
inline void reject(const char* reason,unsigned vi){if(!failed)std::fprintf(stderr,"[BossSwallowRetry] diagnostic-stop reason=%s vi=%u inputOnly=1 noGameplayFailure=1\n",reason,vi);failed=true;}
inline bool scope(){const auto& s=c::session.snapshot();return enabled&&c::enabled&&c::driver&&s.diagnostic&&c::session.plan().seed==163184534930300ull;}
inline bool fish_scope(){const auto& s=c::session.snapshot();const auto& e=c::session.current();return scope()&&s.index==1&&s.completed==1&&!s.retry_epoch&&c::physical_room==168&&c::logical_room==97&&e.family==b::Family::Fish&&e.variant==2&&e.count==2&&e.members[0].kind==0x4511&&e.members[1].kind==0x4072;}
inline bool drive(uint8_t* ram,unsigned vi,uint16_t* buttons,float* x,float* y){
 if(!ram||!buttons||!x||!y||!scope())return false;const auto& s=c::session.snapshot();
 if(s.retry_epoch){*buttons=0;*x=*y=0;if(!death_vi)reject("non_swallow_retry",vi);return true;}
 if(s.phase!=b::Phase::Fighting)return false;const auto player=s.player,priv=n::word(ram,player+0x1F4);
 if(!n::actor(ram,player,0x4001)||!n::range(priv,0x12D0)||c::fatal_player(ram,player))return false;
 if(s.index==0&&!(s.checkpoint_awake_mask&1)){
  if(!c::fight_ready.vi)return false;if(!first_vi)first_vi=vi;
  const float px=std::bit_cast<float>(n::word(ram,player+0x84));*buttons=0;*x=std::abs(px-200.f)>1.f?(px<200.f?.65f:-.65f):0;*y=0;
  if(first_jump_vi&&vi-first_jump_vi>=100)first_jump_vi=0;
  if(!first_jump_vi&&std::abs(px-200.f)<12.f&&!n::byte(ram,priv+0xF4)){first_jump_vi=vi;std::fprintf(stderr,"[BossSwallowRetry] no1-jump vi=%u x=%.3f inputOnly=1\n",vi,px);}
  if(first_jump_vi&&vi-first_jump_vi<18)*buttons=0x8000;
  if(vi-first_vi>1800)reject("no_native_no1_wake",vi);return true;
 }
 if(!armed&&(s.checkpoint_awake_mask&1)){
  const auto& p=c::checkpoints::selected_inventory();if(!inv::valid(p)||p.player!=player){reject("unproved_activation_inventory",vi);return false;}
  saved=p;armed=true;std::fprintf(stderr,"[BossSwallowRetry] armed player=%08X activationVI=%u hp=%u eggs=%u plane=%08X vi=%u inputOnly=1\n",saved.player,saved.activation_vi,saved.hp,saved.count,c::checkpoints::saved_pose[0].z,vi);
 }
 if(!fish_scope())return false;
 *buttons=0;*x=*y=0;if(!armed){reject("no_initial_checkpoint",vi);return true;}
 if(!fish_vi){fish_vi=vi;const auto& m=c::session.current().members[0];std::fprintf(stderr,"[BossSwallowRetry] seek fish=%08X color=%s player=%08X room=168 vi=%u noForcedContact=1 inputOnly=1\n",c::bound[0],m.green?"green":m.purple?"purple":"red",player,vi);}
 const float px=std::bit_cast<float>(n::word(ram,player+0x84));if(!std::isfinite(px)){reject("unknown_player_position",vi);return true;}
 const float target=((vi-fish_vi)/40)%2?388.f:380.f;*x=std::abs(px-target)>1.25f?(px<target?.55f:-.55f):0;
 if(vi-fish_vi>4200)reject("fish_contact_timeout",vi);return true;
}
inline void death_entry(uint8_t* ram,const recomp_context* ctx,unsigned vi){
 if(!ram||!ctx||!fish_scope()||!armed||death_vi||uint32_t(ctx->r31)!=0x8004217C)return;
 const auto p=c::session.snapshot().player,priv=n::word(ram,p+0x1F4);
 if(uint32_t(ctx->r4)!=p||p!=saved.player||!n::actor(ram,p,0x4001)||n::word(ram,0x800F8828)!=p||!n::range(priv,0x12D0)||n::byte(ram,p+0x18)!=0xA||n::byte(ram,p+0x30)!=0xC||n::word(ram,p+0x20)!=0x8003085C||n::word(ram,priv+0xC0)!=0x8002F5F4)return;
 death_vi=vi;std::fprintf(stderr,"[BossSwallowRetry] native-swallow-death player=%08X private=%08X ra=%08X state=10/12 callback=8003085C pending=8002F5F4 vi=%u nativeObservation=1\n",p,priv,uint32_t(ctx->r31),vi);
}
inline bool complete(uint8_t* ram){
 const auto& s=c::session.snapshot();return ram&&scope()&&!failed&&armed&&death_vi&&s.phase==b::Phase::Fighting&&s.index==0&&s.completed==0&&s.retry_epoch==1&&s.player==saved.player&&c::checkpoints::retry_restored_epoch()==1&&inv::matches(ram,saved)&&boss_checkpoint_pose0260::matches(ram,c::checkpoints::saved_pose[0],saved);
}
}
