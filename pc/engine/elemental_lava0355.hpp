#pragma once
#include "recomp.h"
#include <cstdint>
namespace workshop::elemental_lava0355 {
inline bool range(uint32_t p,uint32_t size){return p>=0x80000400u&&p<0x80800000u&&size<=0x80800000u-p;}
// Only the current player's synchronous native534C0 terrain query is projected.
// The original material word and its low11 geometry bits remain in the tilemap.
// Native49C70/4AB60 then build that player's contact cache and water admission;
// nativeF560/FBE8 retain responsibility for buoyancy and swimming transitions.
inline uint32_t terrain_word(uint8_t* rdram,uint32_t actor,uint32_t raw,bool solo){
 if(!solo||!rdram||raw>0xFFFFu||(raw&0xF800u)!=0x6000u)return raw;
 if(uint32_t(MEM_W(0,int32_t(0x807F0008)))!=16u||uint32_t(MEM_W(0,int32_t(0x800F8DB0)))!=7u)return raw;
 if(!range(actor,0x200)||uint32_t(MEM_W(0,int32_t(0x800F8828)))!=actor||MEM_HU(12,int32_t(actor))!=0x4001)return raw;
 const auto state=MEM_BU(0x18,int32_t(actor));if(state<1||state>=5)return raw;
 const auto data=uint32_t(MEM_W(0x1F4,int32_t(actor))),contact=uint32_t(MEM_W(0x174,int32_t(actor)));
 if(!range(data,0x12D0)||!range(contact,0x20))return raw;
 const auto definition=uint32_t(MEM_W(0,int32_t(contact)));if(!range(definition,0x74))return raw;
 const auto samples=uint32_t(MEM_W(0x40,int32_t(definition)));if(!range(samples,12u*0x18u))return raw;
 return (raw&0x07FFu)|0x1000u;
}
} // namespace workshop::elemental_lava0355
