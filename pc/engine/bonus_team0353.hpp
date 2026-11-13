#pragma once
#include "recomp.h"
#include <cstdint>
namespace workshop::bonus_team0353 {
// Native game-thread only. This is a local Bonus life pool, not an ownership
// journal. The caller supplies a read-only projection of already-found Yoshis.
struct Rights {bool canonical_valid=false,finds_valid=false,metals_valid=false;unsigned bw=0,hybrids=0,metals=0;bool elementals_valid=false;unsigned elementals=0;};
constexpr unsigned owned(const Rights& r){
 return 0x3Fu|((r.canonical_valid?r.bw&3u:0u)<<6)|((r.finds_valid?r.hybrids&63u:0u)<<8)|((r.metals_valid?r.metals&3u:0u)<<14)|((r.elementals_valid?r.elementals&3u:0u)<<16);
}
class Team {
 unsigned owned_=0,alive_=0;
public:
 bool start(unsigned mask){if(mask>0x3FFFFu||(mask&63u)!=63u)return false;owned_=alive_=mask;return true;}
 void reset(){owned_=alive_=0;}
 unsigned ownership()const{return owned_;}
 unsigned available()const{return alive_;}
 unsigned extra()const{return alive_>>8;}
 void project(uint8_t* rdram)const{
  if(!rdram||!owned_)return;
  MEM_W(0,int32_t(0x800F8DF4))=(uint32_t(MEM_W(0,int32_t(0x800F8DF4)))&~255u)|(alive_&255u);
  MEM_W(0,int32_t(0x807F1000))=(uint32_t(MEM_W(0,int32_t(0x807F1000)))&~1023u)|extra();
 }
 bool death(uint8_t* rdram,recomp_context* ctx){
  if(!rdram||!ctx||!owned_)return false;
  const unsigned color=uint32_t(ctx->r9);if(color>=18||!(owned_&(1u<<color)))return false;
  alive_&=~(1u<<color);project(rdram);
  ctx->r10=MEM_W(0,int32_t(0x807F1000));ctx->r12=MEM_W(0,int32_t(0x800F8DF4));return true;
 }
};
}
