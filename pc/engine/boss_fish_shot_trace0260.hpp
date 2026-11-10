#pragma once
#include "boss_native.hpp"
#include <array>
#include <cstdint>
#include <string>
namespace workshop::boss_fish_shot_trace0260 {
namespace n=boss_native;
struct Frame {bool enabled=false;uint32_t player=0;unsigned source=177,room=177,encounter=10,epoch=0;std::string token{};};
struct Shot {
 Frame context{};unsigned serial=0,release_serial=0,vi=0,last_sample_vi=0;bool sampled=false,ended=false;
 uint32_t egg=0,private_data=0,resource=0;uint64_t pulse_floor=0,pulse_birth=0;uint32_t pulse_actor=0,pulse_private=0;
};
inline bool same(const Frame& a,const Frame& b){return a.player==b.player&&a.source==b.source&&a.room==b.room&&a.encounter==b.encounter&&a.epoch==b.epoch&&a.token==b.token;}
inline bool current(uint8_t* ram,const Frame& f){return ram&&f.enabled&&!f.token.empty()&&f.source==97&&f.encounter<9&&f.room==167+f.encounter&&n::word(ram,0x800F8828)==f.player&&n::actor(ram,f.player,0x4001);}
inline bool egg_identity(uint8_t* ram,const Shot& s,uint32_t egg){return ram&&s.serial&&egg==s.egg&&n::actor(ram,egg,0x4002)&&n::word(ram,egg+0x1F4)==s.private_data&&n::range(s.private_data,0x14)&&n::word(ram,egg+0x168)==s.resource&&n::range(s.resource,0x48);}
struct Trace {
 static constexpr unsigned max_shots=8,max_rows=256,window_vi=60,cadence_vi=2;
 Frame context{};std::array<Shot,max_shots> shots{};unsigned count=0,rows=0;
 void reset(){*this={};}
 Shot* begin(uint8_t* ram,const Frame& f,unsigned vi,unsigned serial,unsigned release_serial,uint32_t egg,uint64_t pulse_floor){
  if(!current(ram,f)||!serial||!n::actor(ram,egg,0x4002))return nullptr;
  const auto data=n::word(ram,egg+0x1F4),resource=n::word(ram,egg+0x168);if(!n::range(data,0x14)||!n::range(resource,0x48))return nullptr;
  if(count&&!same(context,f))reset();if(!count)context=f;
  for(const auto& old:shots)if(old.serial&&(old.serial==serial||(old.vi==vi&&old.egg==egg&&old.private_data==data&&old.resource==resource)))return nullptr;
  if(count>=max_shots)return nullptr;auto& s=shots[count++];s.context=f;s.serial=serial;s.release_serial=release_serial;s.vi=vi;s.egg=egg;s.private_data=data;s.resource=resource;s.pulse_floor=pulse_floor;return &s;
 }
 bool sample(uint8_t* ram,const Frame& f,Shot& s,unsigned vi){
  if(!current(ram,f)||!same(s.context,f)||!s.serial||rows>=max_rows||vi<=s.vi||vi-s.vi>window_vi||(s.sampled&&(vi<=s.last_sample_vi||vi-s.last_sample_vi<cadence_vi)))return false;
  s.sampled=true;s.last_sample_vi=vi;++rows;return true;
 }
 Shot* finish(uint8_t* ram,const Frame& f,unsigned vi,uint32_t egg){
  if(!current(ram,f)||rows>=max_rows)return nullptr;
  for(auto& s:shots)if(!s.ended&&same(s.context,f)&&vi>=s.vi&&vi-s.vi<=window_vi&&egg_identity(ram,s,egg)){s.ended=true;++rows;return &s;}return nullptr;
 }
};
}
