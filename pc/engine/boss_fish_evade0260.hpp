#pragma once
#include "boss_fish_explosion0260.hpp"
#include "boss_fish_support0260.hpp"
#include "boss_fish_green0260.hpp"
namespace workshop::boss_fish_evade0260 {
namespace n=boss_native;namespace m=boss_monster_aim0260;namespace boxes=boss_fish_explosion0260;
struct Root {uint32_t actor=0;uint64_t birth=0;};
struct Threat {bool observed=false,imminent=false,head_clear=false;uint32_t actor=0;uint64_t birth=0;unsigned step=0;};
inline Threat observe(uint8_t* ram,uint32_t player,std::span<const Root> roots,unsigned room,unsigned encounter){
 auto* rdram=ram;
 Threat out;
 if(!ram||roots.empty()||roots.size()>2||encounter>=9||room!=167+encounter||!n::actor(ram,player,0x4001)||n::word(ram,0x800F8828)!=player||n::byte(ram,player+0x165)||n::word(ram,player+0x180))return out;
 const auto primary=n::byte(ram,player+0x18);if(primary!=1&&primary!=3&&primary!=4)return out;
 const auto p=n::word(ram,player+0x1F4),meta=n::word(ram,player+0x174);if(!m::pointer(p,0x12D0)||!m::pointer(meta,0x18))return out;
 const auto def=n::word(ram,meta);if(!m::pointer(def,0x80)||!n::byte(ram,def+0x74)||n::word(ram,def+0x70)!=p+8)return out;
 const auto xs=n::word(ram,def+0x30),ys=n::word(ram,def+0x34);if(!m::pointer(xs,48)||!m::pointer(ys,48))return out;
 for(unsigned i=0;i<3;++i)if(MEM_H(4+2*i,n::base(def))||MEM_H(10+2*i,n::base(def))!=(i==0?14:i==1?19:20))return out;
 const auto scales=n::word(ram,def+0x70);
 boxes::Box person;for(unsigned i=0;i<3;++i){const float s=m::scalar(ram,scales+4*i),v=m::scalar(ram,player+0x84+4*i);if(!std::isfinite(s)||s<=0||s>4||!std::isfinite(v))return out;person.center[i]=v;person.half[i]=float(MEM_H(10+2*i,n::base(def)))*s;}
 const auto map=m::Map::read(ram);if(!map||map->columns!=3||map->rows!=2)return out;
 out.head_clear=(n::word(ram,meta+8)&0xF000)==0;
 for(unsigned i:{1u,2u}){const float dx=m::scalar(ram,xs+4*i),dy=m::scalar(ram,ys+4*i);if(!std::isfinite(dx)||!std::isfinite(dy)||std::abs(dx)>16||dy>=0||dy< -32)return Threat{};
  for(unsigned step=0;step<=48&&out.head_clear;++step){const auto solid=map->occupied(ram,person.center[0]+dx,person.center[1]+dy-float(step));if(!solid||*solid)out.head_clear=false;}
 }
 unsigned earliest=17;
 for(const auto& root:roots){const auto a=root.actor;if(!root.birth||(!n::actor(ram,a,0x4511)&&!n::actor(ram,a,0x4631))||n::byte(ram,a+0x18)!=1||n::word(ram,a+0x180)||n::word(ram,a+0x130)!=0x800483B8)return Threat{};
  const auto data=n::word(ram,a+0x1F4),resource=n::word(ram,a+0x168);if(!m::pointer(data,0x188)||!m::pointer(resource,0x48)||n::word(ram,data+0x40))return Threat{};
  const auto sub=n::byte(ram,a+0x30);if(sub>=boss_fish_green0260::updates.size()||!boss_fish_green0260::updates[sub])return Threat{};
  const auto factory=n::word(ram,resource+0x40);if(!m::pointer(factory,0x3FF0)||MEM_HU(0x3C,n::base(resource))!=MEM_HU(12,n::base(a))||n::word(ram,resource+0x44)!=factory+0x35A8||n::word(ram,a+0x20)!=factory+(boss_fish_green0260::updates[sub]-0x805C64F0))return Threat{};
  const auto box=boxes::observe(ram,a,uint16_t(MEM_HU(12,n::base(a))));if(!box||n::word(ram,box->definition+0x28)!=factory+0x410)return Threat{};
  const float vx=m::scalar(ram,a+0xC4)*m::scalar(ram,a+0x100)+m::scalar(ram,a+0x10C),vy=m::scalar(ram,a+0xC8)*m::scalar(ram,a+0x104)+m::scalar(ram,a+0x110);
  if(!std::isfinite(vx)||!std::isfinite(vy)||std::abs(vx)>16||std::abs(vy)>16)return Threat{};
  if(vy>=0)continue;
  // Input-risk estimate only: current-sized native boxes and constant current
  // native movement over16 updates. It licenses an ordinary A gesture, never
  // a hit/release or a claim about future callback order or exact collision.
  for(unsigned step=0;step<=16;++step){bool overlap=true;for(unsigned axis=0;axis<3;++axis){const float velocity=axis==0?vx:axis==1?vy:0;overlap=overlap&&std::abs(box->center[axis]+velocity*float(step)-person.center[axis])<=box->half[axis]+person.half[axis];}
   if(overlap&&step<earliest){earliest=step;out.imminent=true;out.actor=a;out.birth=root.birth;out.step=step;break;}
  }
 }
 out.observed=true;return out;
}
}
