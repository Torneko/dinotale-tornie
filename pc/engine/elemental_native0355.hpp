#pragma once
#include "elemental_abilities0355.hpp"
#include "recomp.h"
#include <bit>
#include <cmath>
namespace workshop::elemental_native0355 {
inline bool range(uint32_t p,uint32_t size){return p>=0x80000400&&p<0x80800000&&size<=0x80800000-p;}
// Called synchronously only at native24260 ->24188, after its own target-speed
// computation. No actor address or multiplier is retained across births/frames.
// Baseline asm92170.data:80099D88={1,2}; B07B0.rodata:800AFD78=2/27.
// Native stick clamp15..54 therefore yields max4 (ordinary),8 (stroke).
inline bool swim_target(uint8_t* rdram,recomp_context* ctx,bool solo){
 if(!solo||!rdram||!ctx||uint32_t(ctx->r31)!=0x80024308)return false;
 const auto color=unsigned(MEM_W(0,int32_t(0x807F0008))),alias=unsigned(MEM_W(0,int32_t(0x800F8DB0)));
 if(color!=elemental_abilities0355::water||alias!=6)return false;
 const auto player=uint32_t(ctx->r4);
 if(!range(player,0x200)||uint32_t(MEM_W(0,int32_t(0x800F8828)))!=player||MEM_HU(12,int32_t(player))!=0x4001||MEM_BU(0x18,int32_t(player))!=1)return false;
 const auto sub=MEM_BU(0x30,int32_t(player));const auto callback=uint32_t(MEM_W(0x20,int32_t(player)));
 if(!((sub==77&&callback==0x80043450)||(sub==78&&callback==0x800434EC)))return false;
 const auto data=uint32_t(MEM_W(0x1F4,int32_t(player)));
 if(!range(data,0x12D0)||MEM_BU(0x113,int32_t(data))>1)return false;
 if(uint32_t(MEM_W(0,int32_t(0x80099D88)))!=0x3F800000||uint32_t(MEM_W(0,int32_t(0x80099D8C)))!=0x40000000||uint32_t(MEM_W(0,int32_t(0x800AFD78)))!=0x3D97B426)return false;
 const float target=std::bit_cast<float>(uint32_t(ctx->r5));
 if(!std::isfinite(target)||target<0||target>8.000001f)return false;
 ctx->r5=std::bit_cast<uint32_t>(target*1.25f);return true;
}
} // namespace workshop::elemental_native0355
