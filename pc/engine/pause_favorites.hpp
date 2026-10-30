#pragma once
#include "recomp.h"
#include "special_content_rules.hpp"
#include <array>
#include <bit>

namespace workshop::favorites {
struct List {std::array<uint16_t,4> frames{};unsigned count=0;};
inline constexpr std::array<uint16_t,6> classic{5,3,4,6,6,3};
constexpr List list(unsigned color){
    List result;
    if(color<6){
        result.frames[result.count++]=classic[color];
        for(unsigned f=0;f<6;f++)if(special::favorite(f,color))result.frames[result.count++]=9+f;
    }else if(color==6 || color==7){result.count=1;} // Native all-fruits indicator.
    else if(color<14){
        for(auto base:special::components[color-8])result.frames[result.count++]=classic[base];
        result.frames[result.count++]=9+color-8;
    }
    return result;
}
constexpr int pointerOffset(unsigned slot){return slot==0?4:int(0x24+slot*4);}
inline bool pointer(uint32_t p){return p>=0x80000400 && p<0x807FFFF0;}
inline float real(uint8_t* rdram,gpr at,unsigned offset){return std::bit_cast<float>(uint32_t(MEM_W(offset,at)));}
inline void real(uint8_t* rdram,gpr at,unsigned offset,float value){MEM_W(offset,at)=std::bit_cast<uint32_t>(value);}
inline void layout(uint8_t* rdram,gpr parent){
    const auto data=MEM_W(0x1F4,parent);if(!pointer(uint32_t(data)))return;
    const auto primary=MEM_W(4,data);if(!pointer(uint32_t(primary)))return;
    const auto color=uint32_t(MEM_W(0,int32_t(0x807F0008)));const auto favorites=list(color);
    if(favorites.count<2)return;
    const float center=real(rdram,primary,0x3C),parentScale=real(rdram,parent,0x6C);
    const float scale=favorites.count==4?0.75f:0.80f;
    const float sx=real(rdram,primary,0x6C)*scale,sy=real(rdram,primary,0x70)*scale;
    for(unsigned slot=0;slot<favorites.count;slot++){
        const auto actor=MEM_W(pointerOffset(slot),data);if(!pointer(uint32_t(actor)))continue;
        real(rdram,actor,0x3C,center+(float(slot)-float(favorites.count-1)*0.5f)*12.0f*parentScale);
        MEM_W(0x40,actor)=MEM_W(0x40,primary);MEM_W(0x44,actor)=MEM_W(0x44,primary);
        real(rdram,actor,0x6C,sx);real(rdram,actor,0x70,sy);
        MEM_B(0x165,actor)=MEM_BU(0x165,primary);
    }
}
inline bool frame(uint8_t* rdram,gpr actor){
    const unsigned parameter=MEM_HU(14,actor)&0x3FF;
    if(parameter<0x201 || parameter>0x204)return false;
    const auto favorites=list(uint32_t(MEM_W(0,int32_t(0x807F0008))));
    const unsigned slot=parameter-0x201;if(slot>=favorites.count)return false;
    MEM_H(0x144,actor)=favorites.frames[slot];return true;
}
inline void hide(uint8_t* rdram,gpr parent){
    const auto data=MEM_W(0x1F4,parent);if(!pointer(uint32_t(data)))return;
    for(unsigned slot=1;slot<4;slot++){const auto actor=MEM_W(pointerOffset(slot),data);if(pointer(uint32_t(actor)))MEM_B(0x165,actor)=1;}
}
}
