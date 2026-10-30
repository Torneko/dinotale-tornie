#pragma once
#include "fruit_mix.hpp"
#include <unordered_set>

namespace workshop::reward_popup {
inline std::unordered_set<uint32_t> fours;
inline void created(uint32_t actor){fours.erase(actor);}
inline void mark(uint8_t* rdram,uint32_t actor){
    if(fruit_mix::range(actor,0x170) && MEM_HU(12,int32_t(actor))==0x4016)fours.insert(actor);
}
inline void paint(uint8_t* rdram,uint32_t actor){
    if(fours.contains(actor) && fruit_mix::range(actor,0x170) && MEM_HU(12,int32_t(actor))==0x4016)
        MEM_H(0x144,int32_t(actor))=7;
}
}
