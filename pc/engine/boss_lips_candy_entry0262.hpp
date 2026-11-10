#pragma once
#include "boss_native.hpp"
#include <bit>
#include <cmath>
#include <string>
#include <string_view>

namespace workshop::boss_lips_candy_entry0262 {
namespace n=boss_native;
// Native64E18 adds128X and replaces Y with376 for its original512px arena.
// A Lips companion uses the existing RAW248/152 record in a256px arena.
// Capture before the actual constructor, then restore only that entry XY once.
// Native child offsets, gravity, velocities, facing, scale and state continue.
struct Frame {
 bool enabled=false,entering=false;unsigned source=177,room=177,index=10,epoch=0,variant=0,count=0,member=4;
 uint32_t actor=0,overlay=0;uint64_t birth=0;std::string_view token;
};
struct Snapshot {bool pending=false,applied=false;uint32_t actor=0,record=0,resource=0,stack=0,caller=0;uint64_t birth=0;unsigned room=177,index=10,epoch=0;};
inline bool pointer(uint32_t p,unsigned size){return !(p&3)&&n::range(p,size);}
inline bool scope(const Frame& f){return f.enabled&&f.entering&&f.source==165&&f.index>=1&&f.index<=8&&f.room==167+f.index&&f.variant==1&&f.count==2&&f.member==1&&f.birth&&!f.token.empty()&&f.token.size()<=128;}
inline bool resource(uint8_t* rdram,const Frame& f,uint32_t r){return pointer(f.overlay,0x3000)&&pointer(r,0x48)&&MEM_HU(0x3C,n::base(r))==0x455A&&n::word(rdram,r+0x40)==f.overlay&&n::word(rdram,r+0x44)==f.overlay+0x10;}
class Guard {
 Snapshot state_{};std::string token_;uint32_t z_=0;
 static constexpr uint32_t raw_x=0x43780000,raw_y=0x43180000; //248/152, native room165 record.
 bool link(uint8_t* rdram,const Frame& f)const{return n::actor(rdram,f.actor,0x455A)&&n::byte(rdram,f.actor+0x17D)==2&&n::word(rdram,f.actor+0x168)==state_.resource&&resource(rdram,f,state_.resource)&&n::word(rdram,f.actor+0x1D8)==state_.record&&n::word(rdram,f.actor+0x1DC)==0x455A0001&&pointer(state_.record,16)&&n::word(rdram,state_.record)==0xFFFFFFFF&&n::word(rdram,state_.record+4)==raw_x&&n::word(rdram,state_.record+8)==raw_y;}
public:
 void reset(){state_={};token_.clear();z_=0;}
 const Snapshot& snapshot()const{return state_;}
 bool capture(uint8_t* rdram,const recomp_context* c,const Frame& f){
  if(!rdram||!c||!scope(f)||state_.pending||state_.applied||uint32_t(c->r4)!=f.actor||uint32_t(c->r31)!=f.overlay+0x40||!pointer(uint32_t(c->r29),0x20)||!n::actor(rdram,f.actor,0x455A))return false;
  const auto r=n::word(rdram,f.actor+0x168),record=n::word(rdram,f.actor+0x1D8);const auto z=n::word(rdram,f.actor+0x44);
  if(!resource(rdram,f,r)||!pointer(record,16)||n::word(rdram,record)!=0xFFFFFFFF||n::word(rdram,record+4)!=raw_x||n::word(rdram,record+8)!=raw_y||n::word(rdram,f.actor+0x1DC)!=0x455A0001||n::byte(rdram,f.actor+0x17D)!=2||n::byte(rdram,f.actor+0x18)||n::byte(rdram,f.actor+0x30)||n::word(rdram,f.actor+0x20)!=f.overlay+0x10||n::word(rdram,f.actor+0x1F4)||n::word(rdram,f.actor+0x180)||n::word(rdram,f.actor+0x3C)!=raw_x||n::word(rdram,f.actor+0x40)!=raw_y||!std::isfinite(std::bit_cast<float>(z)))return false;
  state_={true,false,f.actor,record,r,uint32_t(c->r29),uint32_t(c->r31),f.birth,f.room,f.index,f.epoch};token_=f.token;z_=z;return true;
 }
 bool finish(uint8_t* rdram,const recomp_context* entry,const Frame& f){
  if(!rdram||!entry||!scope(f)||!state_.pending||state_.applied||f.actor!=state_.actor||f.birth!=state_.birth||f.room!=state_.room||f.index!=state_.index||f.epoch!=state_.epoch||f.token!=token_||uint32_t(entry->r4)!=f.actor||uint32_t(entry->r29)!=state_.stack||uint32_t(entry->r31)!=state_.caller||!link(rdram,f))return false;
  // Original64E18 selects1/3 via6622C: table167+2CD8+36+12 returns
  // native65578 (+8A8), independently observed in fresh8457 at VI1944.
  const auto p=n::word(rdram,f.actor+0x1F4),head=pointer(p,0x24)?n::word(rdram,p):0,hp=n::actor(rdram,head,0x454F)?n::word(rdram,head+0x1F4):0;
  if(!pointer(p,0x24)||!pointer(hp,0x88)||n::word(rdram,head+0x180)!=f.actor||n::word(rdram,hp+0x84)!=12||n::word(rdram,p+0x20)||n::byte(rdram,f.actor+0x18)!=1||n::byte(rdram,f.actor+0x30)!=3||n::word(rdram,f.actor+0x20)!=f.overlay+0x8A8||n::word(rdram,f.actor+0x3C)!=std::bit_cast<uint32_t>(376.f)||n::word(rdram,f.actor+0x40)!=std::bit_cast<uint32_t>(376.f)||n::word(rdram,f.actor+0x44)!=z_)return false;
  for(unsigned i=0;i<12;++i){const auto a=n::word(rdram,hp+0x54+4*i);if(!n::actor(rdram,a,0x4550))return false;for(unsigned j=0;j<i;++j)if(n::word(rdram,hp+0x54+4*j)==a)return false;}
  MEM_W(0x3C,n::base(f.actor))=raw_x;MEM_W(0x40,n::base(f.actor))=raw_y;
  state_.pending=false;state_.applied=true;return true;
 }
};
}
