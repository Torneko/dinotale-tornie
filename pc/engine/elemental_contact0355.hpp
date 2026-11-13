#pragma once
#include "elemental_native0355.hpp"
#include <unordered_map>
namespace workshop::elemental_contact0355 {
namespace n=elemental_native0355;
class Tracker {
 struct Identity {uint32_t actor=0,resource=0,data=0;uint64_t birth=0;};
 struct Attempt {Identity player{},source{},parent{};uint32_t overlay19=0,overlay20=0,overlay130=0,overlay88=0,overlay207=0;unsigned mode=0;unsigned source_kind=0x40C7;bool blob=false,fireball=false,snow=false,fire=false;};
 std::unordered_map<uint32_t,uint64_t> births_;uint64_t serial_=0;Attempt attempt_{},pending_{};bool entered_=false;
 static uint32_t word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,int32_t(p)));}
 static bool actor(uint8_t* rdram,uint32_t a,unsigned kind){return n::range(a,0x200)&&MEM_HU(12,int32_t(a))==kind;}
 Identity identity(uint8_t* rdram,uint32_t a){if(!n::range(a,0x200))return {};return {a,word(rdram,a+0x168),word(rdram,a+0x1F4),birth(a)};}
 bool live(uint8_t* rdram,const Identity& i,unsigned kind)const{return i.birth&&birth(i.actor)==i.birth&&actor(rdram,i.actor,kind)&&word(rdram,i.actor+0x168)==i.resource&&word(rdram,i.actor+0x1F4)==i.data;}
 bool chain(uint8_t* rdram,const Attempt& a)const{
  if(a.snow)return live(rdram,a.player,0x4001)&&n::range(a.player.data,0x12D0)&&live(rdram,a.source,0x45B8)&&n::range(a.source.data,8)
   &&word(rdram,0x800F8828)==a.player.actor&&n::range(a.source.resource,0x48)&&MEM_HU(0x3C,int32_t(a.source.resource))==0x45B8&&word(rdram,a.source.resource+0x44)==a.overlay207;
  if(a.fireball)return live(rdram,a.player,0x4001)&&n::range(a.player.data,0x12D0)&&live(rdram,a.source,0x413A)&&n::range(a.source.data,0x18)
   &&word(rdram,0x800F8828)==a.player.actor&&n::range(a.source.resource,0x48)&&MEM_HU(0x3C,int32_t(a.source.resource))==0x413A&&word(rdram,a.source.resource+0x44)==a.overlay88;
  if(a.blob)return live(rdram,a.player,0x4001)&&n::range(a.player.data,0x12D0)&&live(rdram,a.source,a.source_kind)&&n::range(a.source.data,0x208)
   &&word(rdram,0x800F8828)==a.player.actor&&n::range(a.source.resource,0x48)&&MEM_HU(0x3C,int32_t(a.source.resource))==0x41C7&&word(rdram,a.source.resource+0x44)==a.overlay130;
  return live(rdram,a.player,0x4001)&&n::range(a.player.data,0x12D0)&&live(rdram,a.source,0x40C7)&&n::range(a.source.data,4)&&live(rdram,a.parent,0x44D0)&&n::range(a.parent.data,4)
   &&word(rdram,0x800F8828)==a.player.actor&&word(rdram,a.source.actor+0x180)==a.parent.actor
   &&n::range(a.source.resource,0x48)&&word(rdram,a.source.resource+0x44)==a.overlay20
   &&n::range(a.parent.resource,0x48)&&word(rdram,a.parent.resource+0x44)==a.overlay19;
 }
public:
 uint64_t birth(uint32_t a)const{auto f=births_.find(a);return f==births_.end()?0:f->second;}
 void forget(uint32_t a){births_.erase(a);if(a==pending_.player.actor||a==pending_.source.actor||a==pending_.parent.actor)pending_={};if(a==attempt_.player.actor||a==attempt_.source.actor||a==attempt_.parent.actor)attempt_.fire=false;}
 void created(uint32_t a){forget(a);if(n::range(a,0x200)){if(++serial_==0)++serial_;births_[a]=serial_;}}
 void clear_pending(){attempt_=pending_={};entered_=false;}
 void reset(){births_.clear();clear_pending();}
 // Snapshot source BEFORE nativeF6E4 reuses a1 for collision priority2.
 void begin(uint8_t* rdram,recomp_context* ctx,bool solo,uint32_t overlay19,uint32_t overlay20,uint32_t overlay130=0,uint32_t overlay88=0,uint32_t overlay207=0){
  attempt_={};entered_=false;if(!solo||!rdram||!ctx){pending_={};return;}
  const bool water=word(rdram,0x807F0008)==17&&word(rdram,0x800F8DB0)==6;
  if(!water&&(word(rdram,0x807F0008)!=16||word(rdram,0x800F8DB0)!=7)){pending_={};return;}
  const auto player=uint32_t(ctx->r4),source=uint32_t(ctx->r5);
  if(!actor(rdram,player,0x4001)||word(rdram,0x800F8828)!=player)return;
  entered_=true;attempt_.player=identity(rdram,player);
  // Original45B8 is the white snowball emitted by45B7. Its ROM Def16=0101
  // selects nativeDDA0 (mode0) for both upper and side contact in8001E094.
  if(water){
   if(actor(rdram,source,0x45B8)&&n::range(overlay207,0x600)&&ctx->r7==0&&uint32_t(ctx->r31)==0x8001DDE0){
    attempt_.source=identity(rdram,source);attempt_.overlay207=overlay207;attempt_.snow=true;attempt_.fire=chain(rdram,attempt_);
   }
   return;
  }
  // The ROM-verified413A flaming creature uses the two native generic contact
  // routes. Keep their mode and accepted callback distinct, and retain its live
  // birth/resource identity until that precise deferred native damage executes.
  if(actor(rdram,source,0x413A)&&n::range(overlay88,0x15A0)&&((ctx->r7==0&&uint32_t(ctx->r31)==0x8001DDE0)||(ctx->r7==1&&uint32_t(ctx->r31)==0x8001DE30))){
   attempt_.source=identity(rdram,source);attempt_.overlay88=overlay88;attempt_.mode=unsigned(ctx->r7);attempt_.fireball=true;attempt_.fire=chain(rdram,attempt_);return;
  }
  //41D0 is an exact resource-A alias of the visually verified41C7 fire mesh.
  // Its actual8057E480 overlap routine is the sole allowed caller, not a generic
  // white ghost, metal flame emitter, or an arbitrary3-point collision.
  if((actor(rdram,source,0x41C7)||actor(rdram,source,0x41D0))&&n::range(overlay130,0x1200)&&uint32_t(ctx->r31)==overlay130+0xA94&&ctx->r7==0){
   attempt_.source=identity(rdram,source);attempt_.source_kind=MEM_HU(12,int32_t(source));attempt_.overlay130=overlay130;attempt_.blob=true;attempt_.fire=chain(rdram,attempt_);return;
  }
  if(!actor(rdram,source,0x40C7)||!n::range(overlay19,0x4C0)||!n::range(overlay20,0x640)||uint32_t(ctx->r31)!=overlay20+0x2B0||ctx->r7!=0)return;
  attempt_.source=identity(rdram,source);attempt_.parent=identity(rdram,word(rdram,source+0x180));attempt_.overlay19=overlay19;attempt_.overlay20=overlay20;attempt_.fire=chain(rdram,attempt_);
 }
 // Every accepted generic hit replaces the old witness, including ordinary hits.
 // A rejected hit leaves the still-pending native callback and its witness alone.
 void end(uint8_t* rdram,recomp_context* ctx){
  if(!entered_||!rdram||!ctx)return;entered_=false;
  if(uint32_t(ctx->r2)!=1)return;pending_={};
  // Both native E5E4 and E670 suppress B798 while SuperHappy is active.
  // B798 also avoids B698 when HP is already0; neither case needs a witness.
  const auto hp=word(rdram,0x800F8DB4+(attempt_.snow?6:7)*4);
  if(word(rdram,0x800F8944)!=0||hp==0||hp>8)return;
  if(attempt_.fire&&chain(rdram,attempt_)&&word(rdram,attempt_.player.data+0xC0)==(attempt_.mode?0x8002E670u:0x8002E5E4u))pending_=attempt_;
 }
 // NativeB698 called only through real E5E4 -> B798 is the allowed consumer.
 // Both MIPS links and unchanged birth/resource/private graph are required.
 // Returns a loss quantity; native subtraction, clamping, audio and death remain.
 int loss(uint8_t* rdram,recomp_context* ctx){
  const auto p=pending_;pending_={};
  if(!p.fire||!rdram||!ctx||!chain(rdram,p))return 3;
  if(word(rdram,0x807F0008)!=(p.snow?17u:16u)||word(rdram,0x800F8DB0)!=(p.snow?6u:7u))return 3;
  if(uint32_t(ctx->r7)!=p.player.actor||uint32_t(ctx->r31)!=0x8002B7D0||!n::range(uint32_t(ctx->r29),0x40)||word(rdram,uint32_t(ctx->r29)+0x3C)!=(p.mode?0x8002E6A4u:0x8002E618u)||word(rdram,p.player.data+0xC0)!=(p.mode?0x8002E670u:0x8002E5E4u))return 3;
  const auto hp=word(rdram,0x800F8DB4+(p.snow?6:7)*4);if(hp>8||uint32_t(ctx->r16)!=hp)return 3;
  return p.snow?0:1;
 }
 bool pending()const{return pending_.fire;}
};
} // namespace workshop::elemental_contact0355
