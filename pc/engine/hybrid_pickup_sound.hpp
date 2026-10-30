#pragma once
#include "hybrid_finds.hpp"

extern "C" void func_80067E68(uint8_t*,recomp_context*);
extern "C" void func_80067818(uint8_t*,recomp_context*);

namespace workshop::finds {
inline bool pickup_sound(uint8_t* rdram,recomp_context* ctx,uint32_t actor,unsigned state){
    if(!take_pickup_sound(actor,state))return false;
    // Same bank and sound as the native rare-item discovery fanfare.
    // Keep the caller's native registers intact across both audio calls.
    const auto saved=*ctx;
    ctx->r4=6;func_80067E68(rdram,ctx);
    ctx->r5=ctx->r2;ctx->r4=0x251;func_80067818(rdram,ctx);
    *ctx=saved;return true;
}
}
