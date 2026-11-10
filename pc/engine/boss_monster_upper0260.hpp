#pragma once
#include "boss_monster_aim0260.hpp"
namespace workshop::boss_monster_upper0260 {
namespace n=boss_native;namespace m=boss_monster_aim0260;
// Native151's real static upper support, not supplyReady or an elapsed route.
// Native20B78→49C70 uses foot probes3/4/5. Their WORLD float offsets are
// already resolved; multiplying them by the object-render scale is incorrect.
inline bool observe_at(uint8_t* rdram,uint32_t player,unsigned room,unsigned encounter,const m::Stand& stand){
 const bool original=encounter<10&&room==(encounter?167+encounter:162)&&stand.x==392&&stand.y==326&&stand.surface_y==336;
 const bool side=room==168&&encounter==1&&stand.x==248&&stand.y==310&&stand.surface_y==320;
 if((!original&&!side)||!rdram||!n::actor(rdram,player,0x4001)||n::word(rdram,0x800F8828)!=player||n::word(rdram,player+0x180))return false;
 const auto queue=n::byte(rdram,player+0x17D);if(queue!=0&&queue!=2)return false;
 const auto data=n::word(rdram,player+0x1F4),meta=n::word(rdram,player+0x174);if(!n::range(data,0xF5)||n::byte(rdram,data+0xF4)||!n::range(meta,8)||n::word(rdram,meta+4))return false;
 const auto def=n::word(rdram,meta);if(!n::range(def,0x3C)||n::word(rdram,def+0x2C)!=12)return false;
 const auto xs=n::word(rdram,def+0x30),ys=n::word(rdram,def+0x34);if(!m::pointer(xs,48)||!m::pointer(ys,48))return false;
 const float x=m::scalar(rdram,player+0x84),y=m::scalar(rdram,player+0x88);if(!boss_driver0260::point(x,y)||std::abs(x-stand.x)>8||y<stand.surface_y-34||y>stand.surface_y+2)return false;
 const auto map=m::Map::read(rdram);if(!map||map->columns!=3||map->rows!=2)return false;
 for(unsigned k=3;k<=5;++k){const float dx=m::scalar(rdram,xs+4*k),dy=m::scalar(rdram,ys+4*k);if(dx!=float(int(k)-4)*4||dy!=16)return false;
  const auto above=map->occupied(rdram,x+dx,y+dy-2),top=map->occupied(rdram,x+dx,y+dy-1),foot=map->occupied(rdram,x+dx,y+dy);
  if(!above||*above||!top||!*top||!foot||!*foot)return false;
 }
 return true;
}
inline bool observe(uint8_t* rdram,uint32_t player,unsigned room,unsigned encounter){
 const auto stand=m::supply_stand(rdram,room,encounter);return stand&&observe_at(rdram,player,room,encounter,*stand);
}
}
