#pragma once
#include "boss_driver0260.hpp"
#include "boss_native.hpp"
#include <bit>
#include <optional>
#include <span>

namespace workshop::boss_bird_test0260 {
namespace d=boss_driver0260;namespace n=boss_native;
enum class Phase:uint8_t { Disabled,WaitBird,Aim,AwaitInjury,Done,Failed };
// Copies of the campaign's owned renewable bindings. This helper never edits
// the bindings or guest memory and never calls a native damage function.
struct Bird {
 uint32_t actor=0,private_data=0,resource=0,factory=0;uint64_t birth=0;
 uint32_t source_actor=0,source_private=0;uint64_t source_birth=0;
};
struct Frame {
 bool diagnostic=false,driver=false,controls=false,alive=false;
 unsigned room=177,source_room=177,index=10,vi=0;uint32_t player=0;
 unsigned reserves=0;uint32_t aiming_actor=0;
 float x=0,y=0,min_x=0,max_x=768;
};
struct Snapshot {
 bool enabled=false;Phase phase=Phase::Disabled;unsigned start_vi=0,last_vi=0,shot_vi=0,shots=0,slot=0;
 uint32_t player=0,actor=0,private_data=0,resource=0,factory=0,egg=0,wounded=0;
 uint64_t birth=0;const char* failure="";
};
class Controller {
 Snapshot state_{};
 static uint32_t word(uint8_t* r,uint32_t a){return n::word(r,a);}
 static unsigned byte(uint8_t* r,uint32_t a){return n::byte(r,a);}
 static unsigned half(uint8_t* r,uint32_t a){return (byte(r,a)<<8)|byte(r,a+1);}
 static bool range(uint32_t a,unsigned bytes){return a%4==0&&n::range(a,bytes);}
 static float scalar(uint8_t* r,uint32_t a){return std::bit_cast<float>(word(r,a));}
 static bool ready(uint8_t* r,const Bird& b){
  const auto a=b.actor;if(!b.birth||!range(a,0x1FC)||!n::actor(r,a,0x4526)||byte(r,a+0x17D)!=2||byte(r,a+0x18)!=1||word(r,a+0x180)||byte(r,a+0x165))return false;
  const auto sub=byte(r,a+0x30),p=word(r,a+0x1F4),res=word(r,a+0x168),meta=word(r,a+0x174);
  if((sub!=5&&sub!=6)||!range(p,12)||!range(res,0x48)||!range(meta,4))return false;
  const auto factory=word(r,res+0x40),def=word(r,meta);
  // Same native354C/collision3444/callback23EC-or22B4 identity as bird_ready.
  if(!range(factory,0x4000)||!range(def,0x75)||half(r,res+0x3C)!=0x4526||word(r,res+0x44)!=factory+0x1B1C||word(r,def+0x28)!=factory+0x1A14||!byte(r,def+0x74)||word(r,a+0x20)!=factory+(sub==5?0x9BC:0x884))return false;
  return (!b.private_data||b.private_data==p)&&(!b.resource||b.resource==res)&&(!b.factory||b.factory==factory);
 }
 bool successor(uint8_t* r,const Bird& b)const{
  const auto a=b.actor;
  if(!state_.shots||b.source_actor!=state_.actor||b.source_birth!=state_.birth||b.source_private!=state_.private_data||!b.birth||b.birth==state_.birth||!range(a,0x1FC)||!n::actor(r,a,0x46DA)||word(r,a+0x180)||!range(b.resource,0x48)||b.factory!=state_.factory||word(r,a+0x168)!=b.resource||half(r,b.resource+0x3C)!=0x46DA||word(r,b.resource+0x40)!=b.factory||word(r,b.resource+0x44)!=b.factory+0x1B84)return false;
  const auto old=state_.actor;if(!n::actor(r,old,0x4526)||word(r,old+0x1F4)!=state_.private_data||word(r,old+0x168)!=state_.resource||word(r,old+0x180)!=a||byte(r,old+0x17D)!=0||byte(r,old+0x165)!=1||byte(r,old+0x17C)||word(r,old+0x20)!=state_.factory+0x1A08)return false;
  const auto slot=half(r,old+0x17E),count=half(r,a+0x184);
  if(slot>=20||!count||count>20||word(r,a+0x188+4*slot)!=old)return false;
  // The real factory maps46DA to35B4 (+1B84), not4046's3650.
  // 3444 returns its queued initializer; the next update allocates12B and
  // selects1/7/2514. Either real stage proves the same native replacement.
  if(byte(r,a+0x17D)==1)return byte(r,a+0x18)==0&&word(r,a+0x20)==b.factory+0x1B84&&!word(r,a+0x1F4);
  const auto p=word(r,a+0x1F4);
  return byte(r,a+0x17D)==2&&byte(r,a+0x18)==1&&byte(r,a+0x30)==7&&word(r,a+0x20)==b.factory+0xAE4&&range(p,12)&&(!b.private_data||b.private_data==p);
 }
 void fail(const char* why){state_.phase=Phase::Failed;state_.failure=why;std::fprintf(stderr,"[BossBirdTest] failed reason=%s target=%08X birth=%llu shots=%u vi=%u controlsOnly=1\n",why,state_.actor,static_cast<unsigned long long>(state_.birth),state_.shots,state_.last_vi);}
public:
 void configure(bool enabled=false){state_={};state_.enabled=enabled;state_.phase=enabled?Phase::WaitBird:Phase::Disabled;}
 const Snapshot& snapshot()const{return state_;}
 bool complete()const{return !state_.enabled||state_.phase==Phase::Done;}
 // Called only from the existing read-only404C8 entry observer, after it has
 // proved the native4002 and source4001. The release must name this true4526.
 void native_launch(uint8_t* r,uint32_t player,uint32_t egg,uint32_t release_root,unsigned release_serial,unsigned vi){
  if(!state_.enabled||state_.phase!=Phase::Aim||player!=state_.player||release_root!=state_.actor||!release_serial||vi<state_.last_vi||!n::actor(r,egg,0x4002)||!n::actor(r,player,0x4001)||word(r,0x800F8828)!=player)return;
  state_.egg=egg;state_.shot_vi=vi;++state_.shots;state_.phase=Phase::AwaitInjury;
  std::fprintf(stderr,"[BossBirdTest] native-shot target=%08X birth=%llu egg=%08X shot=%u vi=%u controlsOnly=1\n",state_.actor,static_cast<unsigned long long>(state_.birth),egg,state_.shots,vi);
 }
 std::optional<d::Target> observe(uint8_t* r,const Frame& f,std::span<const Bird> birds){
  if(!state_.enabled||state_.phase==Phase::Done||state_.phase==Phase::Failed)return {};
  if(!r||!f.diagnostic||!f.driver||f.room!=171||f.source_room!=97||f.index!=4||!f.alive||!n::actor(r,f.player,0x4001)||word(r,0x800F8828)!=f.player||!d::point(f.x,f.y)||!d::point(f.min_x,f.max_x)||f.min_x>=f.max_x){fail("invalid_automatic_fish_scope");return {};}
  if(state_.player&&(state_.player!=f.player||f.vi<state_.last_vi)){fail("player_or_epoch_changed");return {};}
  state_.last_vi=f.vi;if(!f.controls)return {};
  if(!state_.player){state_.player=f.player;state_.start_vi=f.vi;}
  if(f.vi-state_.start_vi>1200){fail("native_injury_deadline");return {};}
  if(state_.actor){
   if(state_.slot>=birds.size()){fail("owned_slot_missing");return {};}
   const auto& b=birds[state_.slot];
   if(successor(r,b)){state_.wounded=b.actor;state_.phase=Phase::Done;std::fprintf(stderr,"[BossBirdTest] complete source=%08X birth=%llu wounded=%08X shots=%u vi=%u native3444=1 controlsOnly=1\n",state_.actor,static_cast<unsigned long long>(state_.birth),b.actor,state_.shots,f.vi);return {};}
   if(b.actor!=state_.actor||b.birth!=state_.birth){fail("owned_generation_changed_without_proof");return {};}
   if(state_.phase==Phase::AwaitInjury){
    if(f.vi-state_.shot_vi<180)return {};
    if(state_.shots>=3){fail("three_native_shots_without_injury");return {};}
    state_.phase=Phase::Aim;
   }
  }
  if(!f.reserves&&!f.aiming_actor){fail("no_native_reserve_for_bird");return {};}
  if(!state_.actor){
   float best=1e30f;for(unsigned k=0;k<birds.size();++k){const auto& b=birds[k];if(!ready(r,b))continue;const auto x=scalar(r,b.actor+0x84),y=scalar(r,b.actor+0x88);const float dx=x-f.x,dy=y-f.y;
    if(!d::point(x,y)||x<f.min_x||x>f.max_x||std::abs(dx)>160||dy<-200||dy>80)continue;
    const float cost=dx*dx+.35f*dy*dy;if(cost>=best)continue;best=cost;state_.slot=k;
   }
   if(best==1e30f)return {};
   const auto& b=birds[state_.slot];state_.actor=b.actor;state_.birth=b.birth;state_.private_data=word(r,b.actor+0x1F4);state_.resource=word(r,b.actor+0x168);state_.factory=word(r,state_.resource+0x40);state_.phase=Phase::Aim;
   std::fprintf(stderr,"[BossBirdTest] selected target=%08X kind=4526 birth=%llu slot=%u vi=%u controlsOnly=1\n",state_.actor,static_cast<unsigned long long>(state_.birth),state_.slot,f.vi);
  }
  Bird expected{state_.actor,state_.private_data,state_.resource,state_.factory,state_.birth};
  if(!ready(r,expected))return {};
  const float x=scalar(r,state_.actor+0x84),y=scalar(r,state_.actor+0x88),vx=scalar(r,state_.actor+0xC4),vy=scalar(r,state_.actor+0xC8);
  if(!d::point(x,y)||!d::point(vx,vy)||std::abs(vx)>64||std::abs(vy)>64){fail("invalid_native_bird_motion");return {};}
  d::Target target{state_.actor,0x4526,x,y,vx,vy,true,true,false};target.birth=state_.birth;return target;
 }
};
}
