#pragma once
#include "boss_checkpoint_inventory0260.hpp"
#include <bit>
#include <cmath>

namespace workshop::boss_checkpoint_pose0260 {
namespace n=boss_native;namespace inventory=boss_checkpoint_inventory0260;
// Warm2C57C resets XY motion and scale, but leaves the death animation's
// depth44/8C and depth motionCC/D8/114 on the retained4001 actor. Store only
// the stable native gameplay plane; XY still comes from the destination44CA.
struct Payload {
 uint32_t player=0,z=0;unsigned activation_vi=0,native_color=8,pc_color=18;
 bool operator==(const Payload&)const=default;
};
inline bool valid(const Payload& p){
 return inventory::pointer(p.player,0x1FC)&&p.activation_vi&&p.pc_color<18&&
  p.native_color==(yoshi_colors0300::native_alias(p.pc_color))&&std::isfinite(std::bit_cast<float>(p.z));
}
inline bool owner(const Payload& p,const inventory::Payload& i){
 return valid(p)&&inventory::valid(i)&&p.player==i.player&&p.activation_vi==i.activation_vi&&p.native_color==i.native_color&&p.pc_color==i.pc_color;
}
inline bool depth_motion_stopped(uint8_t* ram,uint32_t player){
 return ram&&n::actor(ram,player,0x4001)&&
  std::isfinite(std::bit_cast<float>(n::word(ram,player+0x8C)))&&
  n::word(ram,player+0xCC)==0&&n::word(ram,player+0xD8)==0&&n::word(ram,player+0x114)==0;
}
inline bool stable(uint8_t* ram,uint32_t player){
 return depth_motion_stopped(ram,player)&&!n::word(ram,player+0x180)&&n::word(ram,player+0x44)==n::word(ram,player+0x8C);
}
// allowed_parent comes only from the successful character wake receipt and
// its still-current Controller binding. The native wake temporarily attaches
// P1 to that Miss Warp;47C38's17C=0 branch adds local44 to parent world8C.
inline bool wake_parent(uint8_t* ram,uint32_t player,uint32_t allowed_parent){
 if(!ram||!n::actor(ram,player,0x4001)||!allowed_parent||!inventory::pointer(allowed_parent,0x1FC)||n::word(ram,player+0x180)!=allowed_parent||n::byte(ram,player+0x17C)!=0)return false;
 const auto kind=inventory::half(ram,allowed_parent+12),slot=inventory::half(ram,player+0x17E);
 if(kind<0x4507||kind>0x450A||!n::actor(ram,allowed_parent,uint16_t(kind))||n::byte(ram,allowed_parent+0x17D)==3||n::word(ram,allowed_parent+0x180)||slot>=20||n::word(ram,allowed_parent+0x188+slot*4)!=player)return false;
 unsigned occupied=0,matches=0;for(unsigned k=0;k<20;++k){const auto a=n::word(ram,allowed_parent+0x188+k*4);occupied+=a!=0;matches+=a==player;}
 if(matches!=1||occupied!=inventory::half(ram,allowed_parent+0x184))return false;
 const float local=std::bit_cast<float>(n::word(ram,player+0x44)),parent=std::bit_cast<float>(n::word(ram,allowed_parent+0x8C));
 return std::isfinite(local)&&std::isfinite(parent)&&std::bit_cast<uint32_t>(parent+local)==n::word(ram,player+0x8C);
}
inline bool capture(uint8_t* ram,const inventory::Payload& i,Payload& out,uint32_t allowed_parent=0,const Payload* gameplay_plane=nullptr){
 if(!inventory::valid(i)||!inventory::same_player(ram,i)||n::byte(ram,i.player+0x18)==0xA||!depth_motion_stopped(ram,i.player))return false;
 if(n::word(ram,i.player+0x180)){if(!wake_parent(ram,i.player,allowed_parent))return false;}
 else if(!stable(ram,i.player))return false;
 if(gameplay_plane&&(!valid(*gameplay_plane)||gameplay_plane->player!=i.player||gameplay_plane->native_color!=i.native_color||gameplay_plane->pc_color!=i.pc_color||gameplay_plane->activation_vi>i.activation_vi||gameplay_plane->z!=n::word(ram,i.player+0x8C)))return false;
 Payload p{i.player,n::word(ram,i.player+0x8C),i.activation_vi,i.native_color,i.pc_color};
 if(!valid(p))return false;out=p;return true;
}
inline bool matches(uint8_t* ram,const Payload& p,const inventory::Payload& i){
 return owner(p,i)&&inventory::same_player(ram,i)&&stable(ram,p.player)&&n::word(ram,p.player+0x8C)==p.z;
}
class Restore {
 unsigned last_epoch_=0;
public:
 void reset(){last_epoch_=0;}
 unsigned last_epoch()const{return last_epoch_;}
 bool apply(uint8_t* ram,const Payload& p,const inventory::Payload& i,unsigned death_vi,unsigned retry_epoch){
  if(!owner(p,i)||!inventory::same_player(ram,i)||death_vi<=p.activation_vi||!retry_epoch||retry_epoch<=last_epoch_||!inventory::fatal_death_state(ram,p.player)||n::word(ram,p.player+0x180))return false;
  // One explicit retry fence, after cinematic-child cleanup and before30B7C.
  // D8 is the priorCC snapshot (48EDC/48320);114 is the temporary Z
  // translation cleared by232E0. Never reapply during live gameplay or touch
  // XY, scale, HP or children.
  auto* rdram=ram;
  MEM_W(0x44,n::base(p.player))=p.z;MEM_W(0x8C,n::base(p.player))=p.z;
  MEM_W(0xCC,n::base(p.player))=0;MEM_W(0xD8,n::base(p.player))=0;MEM_W(0x114,n::base(p.player))=0;
  last_epoch_=retry_epoch;return matches(ram,p,i);
 }
};
} // namespace workshop::boss_checkpoint_pose0260
