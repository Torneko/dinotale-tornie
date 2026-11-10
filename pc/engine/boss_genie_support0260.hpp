#pragma once
#include "boss_monster_aim0260.hpp"
namespace workshop::boss_genie_support0260 {
namespace n=boss_native;
struct Frame {bool enabled=false;unsigned source=177,room=177,encounter=10;uint32_t player=0,root=0;uint64_t birth=0;float entry_x=0;};
struct Support {float anchor=0,min_x=0,max_x=0,surface_y=0;unsigned samples=0;};
// Native163 adds four464F records at264/312/360/408,Y712. Their real
// 16px tile stamps join intoX240..431,Y688..735 (also present in captured
// f706 before retry). Observe the CURRENT bitmap; never fabricate a floor.
inline std::optional<Support> observe(uint8_t* rdram,const Frame& f){
 if(!rdram||!f.enabled||f.source!=163||f.encounter>=10||f.room!=(f.encounter?167+f.encounter:162)||!f.birth||!n::actor(rdram,f.player,0x4001)||n::word(rdram,0x800F8828)!=f.player||!n::actor(rdram,f.root,0x4072)||!std::isfinite(f.entry_x)||f.entry_x<264||f.entry_x>408)return {};
 const auto map=boss_monster_aim0260::Map::read(rdram);if(!map||map->columns!=3||map->rows!=3)return {};
 Support out{f.entry_x,256,415,688,0};
 for(unsigned x=240;x<=431;++x){const auto above=map->occupied(rdram,float(x),687),top=map->occupied(rdram,float(x),688),inside=map->occupied(rdram,float(x),689);out.samples+=3;if(!above||*above||!top||!*top||!inside||!*inside)return {};}
 return out;
}
}
