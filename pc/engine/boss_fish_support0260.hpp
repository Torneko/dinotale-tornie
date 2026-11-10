#pragma once
#include "boss_monster_aim0260.hpp"
namespace workshop::boss_fish_support0260 {
namespace n=boss_native;namespace m=boss_monster_aim0260;
struct Island {float x=0;bool on_island=false;};
inline std::optional<Island> observe(uint8_t* ram,uint32_t player,unsigned room,unsigned encounter,std::span<const float> centers,std::optional<float> previous={}){
 if(!ram||encounter>=9||room!=167+encounter||centers.size()!=3||!n::actor(ram,player,0x4001)||n::word(ram,0x800F8828)!=player||n::byte(ram,player+0x165)||n::word(ram,player+0x180))return {};
 const auto p=n::word(ram,player+0x1F4),meta=n::word(ram,player+0x174);if(!m::pointer(p,0x12D0)||!m::pointer(meta,8)||n::word(ram,meta+4))return {};
 const auto def=n::word(ram,meta);if(!m::pointer(def,0x80)||n::word(ram,def+0x2C)!=12)return {};
 const auto xs=n::word(ram,def+0x30),ys=n::word(ram,def+0x34);if(!m::pointer(xs,48)||!m::pointer(ys,48))return {};
 for(unsigned i=3;i<=5;++i)if(m::scalar(ram,xs+4*i)!=float(int(i)-4)*4||m::scalar(ram,ys+4*i)!=16)return {};
 const auto map=m::Map::read(ram);if(!map||map->columns!=3||map->rows!=2)return {};
 const float x=m::scalar(ram,player+0x84),y=m::scalar(ram,player+0x88);if(!boss_driver0260::point(x,y))return {};
 float anchor=0,distance=1e30f;
 for(float center:centers){if(!std::isfinite(center)||center<24||center>744)return {};
  if(previous){if(center==*previous)anchor=center;}
  else if(std::abs(center-x)<distance){distance=std::abs(center-x);anchor=center;}
 }
 if(!anchor)return {};
 // Native97's464F source stamps a48px platform, not the surrounding water.
 // Verify the current bitmap/material instead of accepting a host X alone.
 for(int dx=-24;dx<24;++dx){const auto above=map->tile(ram,anchor+float(dx),287),top=map->tile(ram,anchor+float(dx),288),below=map->tile(ram,anchor+float(dx),304);if(!above||*above||!top||*top!=0x5F01||!below||*below!=0x5F01)return {};}
 bool supported=!n::byte(ram,p+0xF4)&&std::abs(y-273.f)<=1;
 for(float dx:{-4.f,0.f,4.f}){const auto foot=map->tile(ram,x+dx,y+16);supported=supported&&foot&&*foot==0x5F01;}
 if(!previous&&(!supported||std::abs(x-anchor)>12))return {};
 return Island{anchor,supported};
}
}
