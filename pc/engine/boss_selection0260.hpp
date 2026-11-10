#pragma once
#include "boss_native.hpp"
#include "boss_battle0250.hpp"
#include <cstdint>

extern "C" void func_8079F1C0(uint8_t*,recomp_context*);

namespace workshop::boss_selection0260 {
// The USA Trial grid in scene41 is473A, RELOC277 (symbolic ovl_279) (ROM E9B930).
// Its native8079EDB8 confirmation rejects courses absent from800F8E5C.
// Boss Battle reuses only the existing first-arena confirmation branch;
// it never writes that access mask, progression, EEPROM or Yoshi life flags.
inline constexpr unsigned overlay_section=277,first_stage=8;
inline constexpr uint16_t selector_kind=0x473A;
inline constexpr uint32_t overlay_canonical=0x8079DAF0,overlay_size=0x1720;
inline constexpr uint32_t input_callback=0x8079EDB8,private_destructor=0x8079DB00;
struct Frame {
    bool enabled=false;
    boss_battle0250::Phase phase=boss_battle0250::Phase::Idle;
    uint32_t selector_overlay=0;
    // Only a freshly armed Boss Battle may skip the redundant Trial grid.
    // The default still requires the native A/Start input.
    bool automatic_first_entry=false;
};
using Dispatch=void(*)(uint8_t*,recomp_context*);
inline bool handle(uint8_t* rdram,recomp_context* ctx,const Frame& f,Dispatch dispatch=func_8079F1C0){
    namespace n=boss_native;
    if(!rdram || !ctx || !dispatch || !f.enabled || f.phase!=boss_battle0250::Phase::Armed ||
       !n::range(f.selector_overlay,overlay_size) || (f.selector_overlay&3))return false;
    constexpr uint32_t manager=0x800F7EA0;
    if(n::byte(rdram,manager+2)!=0 || n::byte(rdram,manager+0x57D)!=41)return false;
    const auto a=uint32_t(ctx->r4);
    if(!n::actor(rdram,a,selector_kind) || n::byte(rdram,a+0x18)!=0 || n::byte(rdram,a+0x30)!=3 ||
       n::word(rdram,a+0x20)!=f.selector_overlay+(input_callback-overlay_canonical) ||
       n::word(rdram,a+0x1F8)!=f.selector_overlay+(private_destructor-overlay_canonical))return false;
    const auto p=n::word(rdram,a+0x1F4),pad=n::word(rdram,manager+8);
    if((p&3) || !n::range(p,0x118) || (pad&1) || !n::range(pad,2))return false;
    const auto held=MEM_HU(0,n::base(pad));
    if((held&0x4000) || (!(held&0x9000) && !f.automatic_first_entry))return false;
    // Both native selected-coordinate copies must still name3-1. Do not
    // retarget an arbitrary course or interfere with ordinary Trial input.
    if(n::byte(rdram,0x800F8DFC)!=2 || n::byte(rdram,0x800F8DFD)!=0 ||
       n::word(rdram,p)!=0 || n::word(rdram,p+4)!=2 ||
       n::word(rdram,p+8)!=0 || n::word(rdram,p+0xC)!=0x40000000 || n::word(rdram,p+0x10)!=0)return false;
    // The real native fade child is ready; the following state will consume it.
    const auto fade=n::word(rdram,p+0x88);
    if(!n::actor(rdram,fade,0x4014) || n::word(rdram,fade+0x180)!=a ||
       !n::range(n::word(rdram,fade+0x16C),0x14))return false;
    // Exactly8079EF78..AC, after its access check.0/4 starts the real native
    // fade8079EFE4→8079F020 and subsequent Yoshi-selection setup.
    MEM_B(0,n::base(0x800FAAF8))=0x32;
    MEM_B(0,n::base(0x800FAB1C))=0xA;
    MEM_B(0,n::base(0x800FAB20))=9;
    MEM_W(0x10,n::base(p))=1;
    const auto saved=*ctx;
    ctx->r29=ADD32(saved.r29,-0x80);
    ctx->r4=n::base(a);ctx->r5=0;ctx->r6=4;
    dispatch(rdram,ctx);
    *ctx=saved;
    return true;
}
} // namespace workshop::boss_selection0260
