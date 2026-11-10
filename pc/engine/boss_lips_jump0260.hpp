#pragma once
#include "boss_monster_aim0260.hpp"
#include <cmath>
#include <optional>
namespace workshop::boss_lips_jump0260 {
namespace n=boss_native;
struct Clearance {bool current_clear=false;float x=0,half_width=0,half_height=0;};
enum class Reason:unsigned { Unobserved,Player,Metadata,Scales,Shape,Map,Sweep,UnknownPixel,CurrentClear,NearbyColumn,NoColumn };
struct Debug {Reason reason=Reason::Unobserved;bool body_observed=false;float half_width=0,half_height=0;};
// Diagnostic clearance for an upward jump, not native movement or collision.
// Native20844 probes terrain indices1/2.49C70/49864 read their FLOAT offsets
// through definition30/34, mirror X with49018, then query534C0. They are already
// world offsets, independent of the object collider A/C and definition70 scale.
inline std::optional<Clearance> observe(uint8_t* rdram,uint32_t player,float min_x,float max_x,float target_y,Debug* debug=nullptr){
 if(debug)*debug={};auto mark=[&](Reason reason){if(debug)debug->reason=reason;};mark(Reason::Player);
 if(!rdram||!n::actor(rdram,player,0x4001)||n::word(rdram,0x800F8828)!=player||!std::isfinite(min_x)||!std::isfinite(max_x)||min_x>=max_x||!std::isfinite(target_y))return {};
 auto pointer=[](uint32_t p,size_t bytes){return !(p&3)&&n::range(p,bytes);};
 mark(Reason::Metadata);const auto metadata=n::word(rdram,player+0x174);if(!pointer(metadata,4))return {};const auto definition=n::word(rdram,metadata);if(!pointer(definition,0x38))return {};const auto xs=n::word(rdram,definition+0x30),ys=n::word(rdram,definition+0x34);if(!pointer(xs,12)||!pointer(ys,12))return {};
 auto fl=[&](uint32_t a){return std::bit_cast<float>(n::word(rdram,a));};const float px=fl(player+0x84),py=fl(player+0x88);if(!boss_driver0260::point(px,py)||px<min_x||px>max_x)return {};
 bool mirror=false;if(n::byte(rdram,player+0x164)!=7){const auto drawing=n::word(rdram,player+0x16C);if(drawing){if(!pointer(drawing,4))return {};const auto table=n::word(rdram,drawing);if(table){const auto index=n::word(rdram,player+0x138);if(index>=0x200000||!pointer(table,size_t(index)*4+4))return {};mirror=bool(n::word(rdram,table+index*4)&1);}}}
 mark(Reason::Shape);std::array<std::array<float,2>,2> probes{};float hw=0,hh=0;
 for(unsigned i=0;i<probes.size();++i){float x=fl(xs+4*(i+1)),y=fl(ys+4*(i+1));if(!std::isfinite(x)||!std::isfinite(y)||std::abs(x)>64||y>=0||y< -64)return {};if(mirror)x=-x;probes[i]={x,y};hw=std::max(hw,std::abs(x));hh=std::max(hh,-y);}
 if(hw<=0||probes[0][0]*probes[1][0]>=0)return {};
 if(debug){debug->body_observed=true;debug->half_width=hw;debug->half_height=hh;}
 mark(Reason::Map);const auto map=boss_monster_aim0260::Map::read(rdram);if(!map)return {};
 // Sweep the two head probes only. Native side probes6..11 have their own
 // horizontal resolution; an adjacent wall pixel is not a ceiling and must
 // not prohibit A merely because the larger object collider overlaps it.
 mark(Reason::Sweep);const float rise=py-std::min(py,target_y);if(!std::isfinite(rise)||rise>160)return {};
 auto clear=[&](float x)->std::optional<bool>{
  for(const auto& probe:probes){const float at_x=x+probe[0];const int top=int(std::floor(std::min(py,target_y)+probe[1])),bottom=int(std::floor(py+probe[1]));if(at_x<0||at_x>=map->columns*256.f||top<0||bottom>=int(map->rows*256)||bottom<top)return {};
   for(int at_y=top;at_y<=bottom;++at_y){const auto occupied=map->occupied(rdram,at_x,float(at_y));if(!occupied)return {};if(*occupied)return false;}}
  return true;
 };
 mark(Reason::UnknownPixel);const auto current=clear(px);if(!current)return {};if(*current){mark(Reason::CurrentClear);return Clearance{true,px,hw,hh};}
 // Search only32 ordinary world pixels around the actual player, with no
 // assumed platform coordinate. Unknown bimap/profile data never licenses A.
 for(unsigned distance=1;distance<=32;++distance)for(float direction:{-1.f,1.f}){const float x=px+direction*float(distance);if(x<min_x||x>max_x)continue;const auto candidate=clear(x);if(!candidate)return {};if(*candidate){mark(Reason::NearbyColumn);return Clearance{false,x,hw,hh};}}
 mark(Reason::NoColumn);return {};
}
}
