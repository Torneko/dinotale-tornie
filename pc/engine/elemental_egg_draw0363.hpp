#pragma once
#include "elemental_resources0355.hpp"
#include "recomp.h"
namespace workshop::elemental_egg_draw0363 {
inline bool range(uint32_t p,unsigned bytes){return p>=0x80000400u&&p<0x80800000u&&bytes<=0x80800000u-p;}
// Native8003B8F0 assigns the palette before an egg gets its player parent.
// Intercept its actual palette-set continuation, retaining a1/native power7/6.
inline bool palette_offset(uint8_t* rdram,recomp_context* ctx,bool solo){
 if(!solo||!rdram||!ctx)return false;const auto egg=uint32_t(ctx->r4);
 if((egg&3)||!range(egg,0x1F0))return false;
 const auto w=[&](uint32_t p){return uint32_t(MEM_W(0,int32_t(p)));};
 const auto color=w(0x807F0008),alias=elemental_resources0355::native_alias(color);
 if(alias>=8||uint32_t(ctx->r5)!=alias||w(0x800F8DB0)!=alias)return false;
 const auto player=w(0x800F8828);if((player&3)||!range(player,0x200)||w(player)!=0x19971110u||MEM_HU(12,int32_t(player))!=0x4001)return false;
 const auto kind=MEM_HU(12,int32_t(egg));const auto parent=w(egg+0x180);
 if(w(egg)!=0x19971110u||(kind!=0x4002&&kind!=0x4004&&kind!=0x4005)||(parent&&parent!=player)||MEM_BU(0x17D,int32_t(egg))==3)return false;
 const auto resource=w(egg+0x168);if((resource&3)||!range(resource,0x90))return false;
 const auto bound=MEM_HU(0x3C,int32_t(resource));if(bound!=0x4002&&bound!=0x401D&&!(bound==0x4004&&(kind==0x4004||kind==0x4005)))return false;
 const auto base=w(resource+0x88);if((base&1)||!range(base,18*512)||uint32_t(ctx->r2)!=base)return false;
 ctx->r24=color*512;return true;
}
inline bool paint(uint8_t* rdram,uint32_t egg,bool solo){
 if(!solo||!rdram||!range(egg,0x1F0))return false;
 const auto w=[&](uint32_t p){return uint32_t(MEM_W(0,int32_t(p)));};
 const unsigned color=w(0x807F0008),alias=elemental_resources0355::native_alias(color);
 if(alias>=8||w(0x800F8DB0)!=alias)return false;
 const auto player=w(0x800F8828);if(!range(player,0x200)||w(player)!=0x19971110u||MEM_HU(12,int32_t(player))!=0x4001)return false;
 const auto kind=MEM_HU(12,int32_t(egg));
 if(w(egg)!=0x19971110u||(kind!=0x4002&&kind!=0x4004&&kind!=0x4005)||w(egg+0x180)!=player||w(egg+0x1EC)!=alias||MEM_BU(0x17D,int32_t(egg))==3)return false;
 const auto resource=w(egg+0x168),drawing=w(egg+0x16C);if(!range(resource,0x90)||!range(drawing,0x44))return false;
 const auto bound=MEM_HU(0x3C,int32_t(resource));if(bound!=0x4002&&bound!=0x401D&&!(bound==0x4004&&(kind==0x4004||kind==0x4005)))return false;
 const auto base=w(resource+0x88);if(!range(base,18*512))return false;
 // Actor power/state and shared resource are never rewritten, only its own drawing.
 MEM_W(8,int32_t(drawing))=base+color*512;return true;
}
}
