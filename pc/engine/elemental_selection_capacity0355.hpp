#pragma once
#include "recomp.h"
#include <algorithm>
#include <cstdint>
namespace workshop::elemental_selection_capacity0355 {
inline bool clamp_white_icons(uint8_t* rdram,recomp_context* ctx){
 if(!rdram||!ctx||uint32_t(MEM_W(0,int32_t(0x800F8B88)))!=0)return false;
 const auto parent=uint32_t(ctx->r19),requested=uint32_t(ctx->r17);
 if(parent<0x80000400||parent>0x80800000-0x200||(parent&3)||requested>7)return false;
 if(uint32_t(MEM_W(0,int32_t(parent)))!=0x19971110)return false;
 const int children=MEM_H(0x184,int32_t(parent));
 const unsigned stored=(uint32_t(MEM_W(0,int32_t(0x800F8DF4)))>>14)&7;
 if(children<0||children>20||requested!=stored)return false;
 // Only this builder's local icon count changes. Keep the saved WhiteShyGuy
 // stock intact, and preserve native20-child capacity and all Yoshi choices.
 ctx->r17=std::min(requested,unsigned(20-children));return true;
}
} // namespace workshop::elemental_selection_capacity0355
