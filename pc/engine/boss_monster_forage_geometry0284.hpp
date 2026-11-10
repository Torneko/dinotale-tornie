#pragma once
#include "boss_fish_water_recovery0260.hpp"
namespace workshop::boss_monster_forage_geometry0284 {
namespace n=boss_native;namespace m=boss_monster_aim0260;namespace water=boss_fish_water_recovery0260;
struct Scene {m::Map map;water::Body body;bool on_elevator=false;};
struct Elevator {uint32_t actor=0;float x=0,y=0,half_x=0;};
// An observed native415F is a landing COLUMN only. No attachment or support
// follows from its X/Y; the ordinary native collision must create the rider.
inline std::optional<Elevator> elevator(uint8_t* ram){
 auto* rdram=ram;if(!ram)return {};const auto map=m::Map::read(ram);if(!map||map->columns!=3||map->rows!=2)return {};
 constexpr uint32_t queue=0x800F8814;const unsigned cap=MEM_HU(8,n::base(queue));const auto table=n::word(ram,queue+0x10);if(cap>150||!m::pointer(table,cap*4))return {};
 std::optional<Elevator> found;
 for(unsigned k=0;k<cap;++k){const auto a=n::word(ram,table+4*k);if(!n::actor(ram,a,0x415F)||n::byte(ram,a+0x17D)!=2||n::byte(ram,a+0x18)==0xFF||n::word(ram,a+0x130)!=0x800483B8)continue;
  const float x=m::scalar(ram,a+0x84),y=m::scalar(ram,a+0x88);if(!std::isfinite(x)||!std::isfinite(y)||std::abs(x-312)>.5f||y<320||y>432)continue;
  const auto meta=n::word(ram,a+0x174);if(!m::pointer(meta,4))return {};const auto def=n::word(ram,meta);if(!m::pointer(def,0x74)||MEM_H(4,n::base(def))!=0)return {};
  const auto scale=n::word(ram,def+0x70);if(!m::pointer(scale,12))return {};const float sx=m::scalar(ram,scale),half=float(MEM_HU(0xA,n::base(def)))*sx;if(!std::isfinite(sx)||sx<=0||sx>4||!std::isfinite(half)||half<24||half>48)return {};
  if(found)return {};found=Elevator{a,x,y,half};
 }
 return found;
}

// A genuine415F rider has queue0 and metadata.owner==its reciprocal parent.
// Read the same original collider/probe union without changing water::body's
// stricter parentless contract or declaring the moving platform to be home.
inline std::optional<water::Body> elevator_body(uint8_t* ram,uint32_t player,uint32_t parent){
 auto* rdram=ram;const auto meta=n::word(ram,player+0x174);if(!m::pointer(meta,8)||n::word(ram,meta+4)!=parent)return {};
 const auto def=n::word(ram,meta);if(!m::pointer(def,0x74)||n::word(ram,def+0x2C)!=12)return {};
 const auto xs=n::word(ram,def+0x30),ys=n::word(ram,def+0x34),scale=n::word(ram,def+0x70);if(!m::pointer(xs,48)||!m::pointer(ys,48)||!m::pointer(scale,12))return {};
 const float sx=m::scalar(ram,scale),sy=m::scalar(ram,scale+4);if(!std::isfinite(sx)||!std::isfinite(sy)||sx<=0||sy<=0||sx>4||sy>4||MEM_H(4,n::base(def))!=0)return {};
 const float cy=float(MEM_H(6,n::base(def)))*sy,hx=float(MEM_HU(0xA,n::base(def)))*sx,hy=float(MEM_HU(0xC,n::base(def)))*sy;water::Body b{-hx,hx,cy-hy,cy+hy};
 for(unsigned i=0;i<12;++i){const float x=m::scalar(ram,xs+4*i),y=m::scalar(ram,ys+4*i);if(!boss_driver0260::point(x,y)||std::abs(x)>64||std::abs(y)>64)return {};b.left=std::min(b.left,x);b.right=std::max(b.right,x);b.top=std::min(b.top,y);b.bottom=std::max(b.bottom,y);}
 if(b.left>=0||b.right<=0||b.top>=0||b.bottom<=2||b.left< -64||b.right>64||b.top< -64||b.bottom>64)return {};return b;
}
// Read-only QA input clearance. Native movement, landing, object contacts and
// food admission remain independent observations owned by the controller.
inline std::optional<Scene> observe(uint8_t* ram,uint32_t player){
 if(!ram||!n::actor(ram,player,0x4001)||n::word(ram,0x800F8828)!=player)return {};
 const auto parent=n::word(ram,player+0x180);const unsigned queue=n::byte(ram,player+0x17D);bool rider=false;
 if(parent){
  if((queue!=0&&queue!=2)||!n::actor(ram,parent,0x415F)||n::byte(ram,parent+0x17D)!=2||n::byte(ram,parent+0x18)==0xFF||n::word(ram,parent+0x130)!=0x800483B8)return {};
  const auto slot=unsigned(n::byte(ram,player+0x17E)<<8|n::byte(ram,player+0x17F)),count=unsigned(n::byte(ram,parent+0x184)<<8|n::byte(ram,parent+0x185));
  const float x=m::scalar(ram,parent+0x84),y=m::scalar(ram,parent+0x88);if(slot>=20||!count||count>20||n::word(ram,parent+0x188+4*slot)!=player||!std::isfinite(x)||!std::isfinite(y)||std::abs(x-312)>.5f||y<320||y>432)return {};rider=true;
 }else if(queue!=2)return {};
 const auto map=m::Map::read(ram);const auto shape=rider?elevator_body(ram,player,parent):water::body(ram,player);
 if(!map||map->columns!=3||map->rows!=2||!shape)return {};
 return Scene{*map,*shape,rider};
}
inline bool clear(uint8_t* ram,const Scene& scene,float x,float y,float tx,float ty,bool floorContact=false){
 const auto& b=scene.body;const auto& map=scene.map;
 if(!ram||map.columns!=3||map.rows!=2||!boss_driver0260::point(x,y)||!boss_driver0260::point(tx,ty)||std::hypot(tx-x,ty-y)>192)return false;
 if(!std::isfinite(b.left)||!std::isfinite(b.right)||!std::isfinite(b.top)||!std::isfinite(b.bottom)||b.left>=0||b.right<=0||b.top>=0||b.bottom<=2||b.left< -64||b.right>64||b.top< -64||b.bottom>64)return false;
 const auto current=m::Map::read(ram);
 if(!current||current->descriptor!=map.descriptor||current->pages!=map.pages||current->data!=map.data||current->columns!=map.columns||current->rows!=map.rows||current->stride!=map.stride)return false;
 // The delegated helper also retains its pre-existing conservative body
 // samples. Bound both complete native-derived body endpoints to this map.
 for(const auto point:std::array<std::array<float,2>,2>{{{x,y},{tx,ty}}}){
  if(point[0]+std::min(b.left,-12.f)<0||point[0]+std::max(b.right,12.f)>=768||point[1]+std::min(b.top,-24.f)<0||point[1]+std::max(b.bottom-(floorContact?2.f:0.f),14.f)>=512)return false;
 }
 return water::clear(ram,map,b,x,y,tx,ty,floorContact);
}
} // namespace workshop::boss_monster_forage_geometry0284
