#pragma once
#include "elemental_native0355.hpp"
namespace workshop::elemental_flame_food0355 {
namespace n=elemental_native0355;
// The sole native3C660 tongue gate calls this synchronously. No shared flags or
// food definitions are changed; only this Fire player's current collision gains
// the normal C0 pickup branch. Resource413A is the ROM-verified flaming enemy.
inline bool gate(uint8_t* rdram,recomp_context* ctx,bool solo,uint32_t overlay88,uint64_t player_birth,uint64_t source_birth){
 if(!solo||!rdram||!ctx||!player_birth||!source_birth||ctx->r2!=0||!n::range(overlay88,0x15A0))return false;
 if(uint32_t(MEM_W(0,int32_t(0x807F0008)))!=16||uint32_t(MEM_W(0,int32_t(0x800F8DB0)))!=7)return false;
 const auto player=uint32_t(MEM_W(0,int32_t(0x800F8828))),source=uint32_t(ctx->r16),tongue=uint32_t(ctx->r4);
 if(!n::range(player,0x200)||!n::range(source,0x200)||!n::range(tongue,0x200)||source!=uint32_t(ctx->r5))return false;
 if(MEM_HU(12,int32_t(player))!=0x4001||MEM_HU(12,int32_t(source))!=0x413A||MEM_HU(12,int32_t(tongue))!=0x400C||MEM_BU(0x18,int32_t(source))!=1)return false;
 const auto pdata=uint32_t(MEM_W(0x1F4,int32_t(player))),res=uint32_t(MEM_W(0x168,int32_t(source))),desc=uint32_t(MEM_W(0x174,int32_t(source)));
 if(!n::range(pdata,0x12D0)||pdata!=uint32_t(ctx->r7)||uint32_t(MEM_W(0x180,int32_t(tongue)))!=player||!n::range(res,0x48)||!n::range(desc,4))return false;
 const auto def=uint32_t(MEM_W(0,int32_t(desc)));
 if(!n::range(def,0x78)||def!=uint32_t(ctx->r3)||MEM_HU(0x3C,int32_t(res))!=0x413A||uint32_t(MEM_W(0x44,int32_t(res)))!=overlay88)return false;
 // Native413A payload properties=0 and edibility flags=0 are audited. A changed
 // definition or an unrelated projectile must continue through native behavior.
 if(uint32_t(MEM_W(0x18,int32_t(def)))!=0||MEM_HU(0x1C,int32_t(def))!=0)return false;
 ctx->r2=0xC0;return true;
}
} // namespace workshop::elemental_flame_food0355
