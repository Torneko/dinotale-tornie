#pragma once
#include "boss_checkpoint_inventory0260.hpp"

// Solo inventory adapter. This does not enlarge the deferred Boss Battle
// checkpoint format or change its validation and restoration paths.
namespace workshop::solo_inventory0300 {
namespace legacy=boss_checkpoint_inventory0260;
namespace n=boss_native;
using Scale=legacy::Scale;
using Egg=legacy::Egg;
using Payload=legacy::Payload;
using Inventory=legacy::Inventory;
using legacy::half;
using legacy::pointer;
using legacy::valid_scale;
using legacy::read_scale;
using legacy::normal_scale;
using legacy::stage_scale;
constexpr unsigned native_alias(unsigned color){return yoshi_colors0300::native_alias(color);}
inline bool same_player(uint8_t* ram,const Payload& p){
 return ram&&n::actor(ram,p.player,0x4001)&&n::word(ram,0x800F8828)==p.player&&
        p.pc_color<18&&p.native_color==native_alias(p.pc_color)&&
        n::word(ram,0x800F8DB0)==p.native_color&&n::word(ram,0x807F0008)==p.pc_color;
}
inline bool read(uint8_t* ram,uint32_t player,Inventory& out){
 if(!ram||!n::actor(ram,player,0x4001))return false;
 const auto data=n::word(ram,player+0x1F4);if(!pointer(data,0x12D0))return false;
 unsigned occupied=0;std::array<uint32_t,20> children{};
 for(unsigned i=0;i<children.size();i++){
  const auto a=n::word(ram,player+0x188+i*4);children[i]=a;if(!a)continue;
  if(!pointer(a,0x1FC)||n::word(ram,a)!=0x19971110||n::word(ram,a+0x180)!=player)return false;
  for(unsigned j=0;j<i;j++)if(children[j]==a)return false;
  ++occupied;
 }
 if(occupied!=half(ram,player+0x184))return false;
 Inventory result;result.count=n::byte(ram,data+0x12C8);const unsigned head=half(ram,data+0x12BE);
 if(result.count>6||head>=8)return false;
 for(unsigned i=0;i<result.count;i++){
  const auto a=n::word(ram,data+0x11FC+((head+i)&7)*24);
  if(!pointer(a,0x1FC)||n::word(ram,a)!=0x19971110||n::byte(ram,a+6)==3||n::word(ram,a+0x180)!=player)return false;
  const auto kind=half(ram,a+12);if(kind!=0x4004&&kind!=0x406C&&kind!=0x4144)return false;
  for(unsigned j=0;j<i;j++)if(result.actors[j]==a)return false;
  unsigned references=0;
  for(unsigned j=0;j<20;j++)if(n::word(ram,player+0x188+j*4)==a)++references;
  if(references!=1)return false;
  const auto palette=kind==0x4004?n::word(ram,a+0x1EC):0;
  if(kind==0x4004&&palette>=18)return false;
  Scale saved{};if(kind==0x4004&&!read_scale(ram,a,saved))return false;
  result.actors[i]=a;result.eggs[i]={uint16_t(kind),palette,saved};
 }
 out=result;return true;
}
inline bool valid(const Payload& p){
 if(!p.activation_vi||!pointer(p.player,0x1FC)||p.pc_color>=18||p.native_color!=native_alias(p.pc_color)||p.hp>8||p.count>6)return false;
 for(unsigned i=0;i<p.count;i++)if(p.eggs[i].kind!=0x4004||p.eggs[i].palette>=18||!valid_scale(p.eggs[i].saved_scale))return false;
 for(unsigned i=p.count;i<p.eggs.size();i++)if(p.eggs[i]!=Egg{})return false;
 return true;
}
inline bool capture(uint8_t* ram,uint32_t player,unsigned vi,Payload& out){
 if(!ram||!vi)return false;
 Payload result;result.activation_vi=vi;result.player=player;
 result.native_color=n::word(ram,0x800F8DB0);result.pc_color=n::word(ram,0x807F0008);
 // In0.26 primary1/sub11 is a living hungry action. The frozen0.24 solo
 // predicate rejects that substate globally, so keep its use out of captures.
 if(!solo_inventory0300::same_player(ram,result)||n::byte(ram,player+0x18)==0xA)return false;
 result.hp=n::word(ram,0x800F8DB4+result.native_color*4);
 Inventory inventory;if(!solo_inventory0300::read(ram,player,inventory))return false;
 result.count=inventory.count;result.eggs=inventory.eggs;
 if(!solo_inventory0300::valid(result))return false;
 out=result;return true;
}

inline bool matches(uint8_t* ram,const Payload& p){
 if(!solo_inventory0300::valid(p)||!solo_inventory0300::same_player(ram,p)||n::word(ram,0x800F8DB4+p.native_color*4)!=p.hp)return false;
 Inventory inventory;return solo_inventory0300::read(ram,p.player,inventory)&&inventory.count==p.count&&inventory.eggs==p.eggs;
}
} // namespace workshop::solo_inventory0300
