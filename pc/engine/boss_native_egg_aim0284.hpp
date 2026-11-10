#pragma once
#include "boss_native.hpp"
#include <bit>
#include <cmath>
namespace workshop::boss_native_egg_aim0284 {
namespace n=boss_native;
struct Observation {bool player_observed=false;uint32_t held=0,reticle=0;float reticle_x=0,reticle_y=0;unsigned held_flag=0;};
inline bool connected(uint8_t* rdram,uint32_t player,uint32_t child,uint16_t kind){
 if(!rdram||player%4||child%4||!n::range(player,0x1FC)||!n::actor(rdram,child,kind)||n::byte(rdram,child+0x17D)>2||n::byte(rdram,child+0x18)==0xFF||n::word(rdram,child+0x180)!=player)return false;
 const auto slot=unsigned(MEM_HU(0x17E,n::base(child))),count=unsigned(MEM_HU(0x184,n::base(player)));
 // Native child count is a population, not a dense highest slot index.
 // The real returned egg occupies slot8 while the population is8.
 if(slot>=20||!count||count>20||n::word(rdram,player+0x188+4*slot)!=child)return false;
 unsigned matches=0;for(unsigned k=0;k<20;++k)matches+=n::word(rdram,player+0x188+4*k)==child;return matches==1;
}
// Read-only QA identity, matching the native403E4/401D8/3BF08/40C48 graph.
// A returned FIFO egg can retain kind4005, phase69 and its sparse P1 slot;
// ONLY current private1F0 identifies the held egg. private1F4 owns4003.
// Do not require1F8==1: native pending3/BF88 has already installed1F0 before
// BF08 restores the saved primary and sets that flag. Do not reject phase69.
inline Observation observe(uint8_t* rdram,uint32_t player){
 Observation out;
 if(!rdram||player%4||!n::actor(rdram,player,0x4001)||n::word(rdram,0x800F8828)!=player||n::byte(rdram,player+0x17D)>2||n::byte(rdram,player+0x18)==0xFF)return out;
 const auto data=n::word(rdram,player+0x1F4);if(data%4||!n::range(data,0x12CA))return out;
 out.player_observed=true;out.held_flag=n::byte(rdram,data+0x1F8);
 const auto held=n::word(rdram,data+0x1F0);if(connected(rdram,player,held,0x4005))out.held=held;
 const auto reticle=n::word(rdram,data+0x1F4);
 if(connected(rdram,player,reticle,0x4003)){
  const float x=std::bit_cast<float>(n::word(rdram,reticle+0x84)),y=std::bit_cast<float>(n::word(rdram,reticle+0x88));
  if(std::isfinite(x)&&std::isfinite(y)&&std::abs(x)<100000&&std::abs(y)<100000){out.reticle=reticle;out.reticle_x=x;out.reticle_y=y;}
 }
 return out;
}
}
