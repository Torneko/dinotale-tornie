#pragma once
#include "boss_lips_aim0260.hpp"
#include "boss_lips_corridor0260.hpp"
#include "boss_egg_shape0260.hpp"
#include <array>
namespace workshop::boss_lips_target0260 {
namespace n=boss_native;
struct Point {float x=0,y=0,offset_x=0,offset_y=0,half_x=0,half_y=0,half_z=0;unsigned candidate=0;boss_lips_corridor0260::Result corridor{};};
// Native600CC/60D90 use definitionA/C/E times the scales referenced by70.
// Aim at a strict interior point of that observed40A7 collision box. This only
// chooses diagnostic input coordinates; the native hit callback stays sole judge.
template<class Predicate> inline std::optional<Point> choose_if(uint8_t* rdram,const boss_monster_aim0260::Map& map,const boss_lips_aim0260::WeakPoint& weak,const boss_egg_shape0260::Shape& egg,float x,float y,float z,Predicate&& admissible,bool exposed=false){
 if(!rdram||egg.count!=1||!n::actor(rdram,weak.actor,0x40A7)||!boss_lips_aim0260::pointer(weak.metadata,4)||n::word(rdram,weak.actor+0x174)!=weak.metadata||n::word(rdram,weak.metadata)!=weak.definition||!boss_lips_aim0260::pointer(weak.definition,0x74)||!boss_driver0260::point(x,y)||!std::isfinite(z))return {};
 const auto scales=n::word(rdram,weak.definition+0x70);if(!boss_lips_aim0260::pointer(scales,12))return {};
 auto fl=[&](uint32_t p){return std::bit_cast<float>(n::word(rdram,p));};std::array<float,3> half{};
 for(unsigned axis=0;axis<3;++axis){const float scale=fl(scales+axis*4);const int extent=MEM_H(0xA+2*axis,n::base(weak.definition));if(!std::isfinite(scale)||scale<=0||scale>4||extent<=0||extent>256)return {};half[axis]=float(extent)*scale;if(!std::isfinite(half[axis])||half[axis]<=3||half[axis]>128)return {};}
 const float weak_z=fl(weak.actor+0x8C);if(!std::isfinite(weak_z)||std::abs(weak_z-z)>=half[2]+egg.half_z-2)return {};
 // The2px reticle tolerance remains strictly inside the native box: offsets
 // leave at least3px, even when a native scale shrinks the collision dimensions.
 const float radius_x=std::min(.8f*half[0],half[0]-3),radius_y=std::min(.8f*half[1],half[1]-3);
 constexpr std::array<float,5> fraction{0,.625f,-.625f,1,-1};const float side=x<weak.x?-1.f:1.f;const std::array<float,5> xs=exposed?std::array<float,5>{side,0,side*.625f,-side*.625f,-side}:fraction,ys=exposed?std::array<float,5>{0,-.625f,-1,.625f,1}:fraction;unsigned candidate=0;
 for(float fy:ys)for(float fx:xs){const float ox=fx*radius_x,oy=fy*radius_y,tx=weak.x+ox,ty=weak.y+oy;++candidate;const auto path=boss_lips_corridor0260::observe(rdram,map,x,y,tx,ty,egg.half_x,egg.half_y);if(path&&path->clear){const Point point{tx,ty,ox,oy,half[0],half[1],half[2],candidate,*path};if(admissible(point))return point;}}
 return {};
}
inline std::optional<Point> choose(uint8_t* rdram,const boss_monster_aim0260::Map& map,const boss_lips_aim0260::WeakPoint& weak,const boss_egg_shape0260::Shape& egg,float x,float y,float z){return choose_if(rdram,map,weak,egg,x,y,z,[](const Point&){return true;});}
}
