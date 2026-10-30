#pragma once
#include "recomp.h"
namespace workshop::cache {
inline constexpr uint32_t manager=0x800DA5B8,records=0x807F8000,slots=0x807F9800;
inline constexpr unsigned capacity=512;
// Only the ROM-resource cache grows. Other native lists retain their original
// 150 sentinel and layout. Keep the old manager address for all its callers.
inline int find_slot(uint8_t* rdram,recomp_context* ctx,bool match){
    if(uint32_t(ctx->r4)!=manager)return 0;
    const auto array=gpr(int32_t(MEM_W(0x10,ctx->r4)));
    ctx->r2=capacity;
    for(unsigned i=0;i<capacity;i++){
        auto value=uint32_t(MEM_W(i*4,array));
        if((match && value && value==uint32_t(ctx->r5)) || (!match && !value)){ctx->r2=i;break;}
    }
    return 1;
}
}
