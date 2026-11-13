#pragma once
#include "recomp.h"
#include <cstdint>

namespace workshop::trial_special_native0350 {
constexpr bool scope(bool canonical,int pc_mode,int native_mode){
    return canonical&&native_mode==1&&(pc_mode==1||pc_mode==3);
}
constexpr unsigned metal_extras(unsigned pc_mask){return (pc_mask>>8)&0xC0u;}
constexpr unsigned carried_bw(unsigned native_flags){return (native_flags>>4)&0xC0u;}
inline void project(uint8_t* rdram,unsigned blocked_pc_mask){
    if(!rdram)return;
    // B/W use native bits6/7; Gold/Silver use extra bits6/7. Never cross aliases.
    MEM_W(0,int32_t(0x800F8DF4))=uint32_t(MEM_W(0,int32_t(0x800F8DF4)))&~(blocked_pc_mask&0xC0u);
    MEM_W(0,int32_t(0x807F1000))=uint32_t(MEM_W(0,int32_t(0x807F1000)))&~metal_extras(blocked_pc_mask);
}
} // namespace workshop::trial_special_native0350
