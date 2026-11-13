#pragma once
#include "hybrid_maskass_data.hpp"
#include "recomp.h"
#include <optional>
namespace workshop::enemy_egg_provenance0366 {
using Variant=std::optional<unsigned>(*)(uint32_t);
inline bool range(uint32_t p,unsigned size){return p>=0x80000400u&&p<0x80800000u&&size<=0x80800000u-p;}
constexpr bool excluded(unsigned color){return color==6||color==7||color==16||color==17||color>=18;}
inline std::optional<unsigned> resolve(uint8_t* rdram,uint32_t source,unsigned color,Variant variant){
 if(!rdram||!variant||excluded(color)||!range(source,0x180)||uint32_t(MEM_W(0,int32_t(source)))!=0x19971110u)return {};
 const auto kind=MEM_HU(12,int32_t(source));bool maskass=false;for(auto id:hybrid_mode::maskass_ids)maskass|=kind==id;if(!maskass)return {};
 // Resolve at the consumption fence, including after a ground-pound recolor.
 // No spawn colour, actor address or receipt survives this call.
 const auto selected=variant(source);if(!selected||*selected>=6)return {};return 8+*selected;
}
inline bool enemy_stack(uint8_t* rdram,recomp_context* ctx,Variant variant){
 if(!rdram||!ctx||!range(uint32_t(ctx->r29),0x14))return false;
 const auto color=uint32_t(MEM_W(0,int32_t(0x807F0008)));const auto selected=resolve(rdram,uint32_t(ctx->r5),color,variant);if(!selected)return false;
 MEM_W(0x10,ctx->r29)=*selected;return true;
}
inline bool default_color(uint8_t* rdram,recomp_context* ctx,Variant variant){
 if(!rdram||!ctx||!ctx->r2||!range(uint32_t(ctx->r29),0x28))return false;
 const auto color=uint32_t(MEM_W(0,int32_t(0x807F0008)));const auto source=uint32_t(MEM_W(0x24,ctx->r29));const auto selected=resolve(rdram,source,color,variant);if(!selected)return false;
 ctx->r7=*selected;return true;
}
}
