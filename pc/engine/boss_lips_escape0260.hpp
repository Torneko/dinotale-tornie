#pragma once
#include "boss_lips_body0260.hpp"
#include "boss_monster_aim0260.hpp"
namespace workshop::boss_lips_escape0260 {
struct Escape {float x=0;int direction=0;float player_bottom=0,body_top=0;unsigned probes=0;};
// Ordinary airborne navigation. The object volumes prove that the player is
// currently above Bongo; the actual forward-side and head terrain probes prove
// a horizontal corridor at this height. Every observation is renewed each VI.
inline std::optional<Escape> observe(uint8_t* rdram,uint32_t player,const boss_lips_body0260::Scene& scene,const boss_monster_aim0260::Map& map,float min_x,float max_x,int direction_hint=0){
 namespace n=boss_native;namespace b=boss_lips_body0260;
 if(!rdram||!n::actor(rdram,player,0x4001)||n::word(rdram,0x800F8828)!=player||!std::isfinite(min_x)||!std::isfinite(max_x)||min_x>=max_x||(direction_hint!=0&&direction_hint!=-1&&direction_hint!=1))return {};
 const auto meta=n::word(rdram,player+0x174);if(!n::range(meta,4))return {};const auto def=n::word(rdram,meta);const auto box=b::read_box(rdram,player,def);if(!box||!n::range(def,0x38))return {};
 const float bottom=box->y+box->hy,top=scene.body.y-scene.body.hy;
 if(!(bottom<top)||std::abs(box->z-scene.body.z)>box->hz+scene.body.hz)return {};
 const float px=std::bit_cast<float>(n::word(rdram,player+0x84)),py=std::bit_cast<float>(n::word(rdram,player+0x88));
 // Start only near the obstructing body. Once across it, a temporarily
 // unavailable shot must not immediately start the reverse traversal.
 if(!direction_hint&&std::abs(box->x-scene.body.x)>scene.body.hx+box->hx+2)return {};
 const int direction=direction_hint?direction_hint:(box->x<scene.body.x?1:-1);
 // Two world pixels beyond the complete body/player separation avoid merely
 // reaching a point where a later side contact immediately rebounds again.
 const float target=scene.body.x+float(direction)*(scene.body.hx+box->hx+2+scene.eye.hx+box->hx+2)-(box->x-px);
 if(!std::isfinite(target)||target<min_x||target>max_x||std::abs(target-px)>256||!b::walk_clear(rdram,scene,player,target))return {};
 const auto xs=n::word(rdram,def+0x30),ys=n::word(rdram,def+0x34);if((xs&3)||(ys&3)||!n::range(xs,48)||!n::range(ys,48))return {};
 bool mirror=false;if(n::byte(rdram,player+0x164)!=7){const auto drawing=n::word(rdram,player+0x16C);if(drawing){if(!n::range(drawing,4))return {};const auto table=n::word(rdram,drawing);if(table){const auto index=n::word(rdram,player+0x138);if(index>=0x200000||!n::range(table,size_t(index)*4+4))return {};mirror=bool(n::word(rdram,table+index*4)&1);}}}
 unsigned probes=0;for(unsigned index=1;index<12;++index){if(index>=3&&index<=5)continue;float ox=std::bit_cast<float>(n::word(rdram,xs+4*index)),oy=std::bit_cast<float>(n::word(rdram,ys+4*index));if(!std::isfinite(ox)||!std::isfinite(oy)||std::abs(ox)>64||std::abs(oy)>64)return {};if(mirror)ox=-ox;if(index>=6&&float(direction)*ox<=0)continue;
  ++probes;const float count=std::ceil(std::abs(target-px));for(unsigned k=0;k<=unsigned(count);++k){const float x=px+(target-px)*(count?float(k)/count:0)+ox,y=py+oy;const auto solid=map.occupied(rdram,x,y);if(!solid||*solid)return {};}}
 if(probes!=5)return {};return Escape{target,direction,bottom,top,probes};
}
}
