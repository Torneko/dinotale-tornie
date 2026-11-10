#pragma once
#include "special_victory.hpp"
#include "fruit_mix.hpp"
namespace workshop::special_victory {
inline std::optional<unsigned> paint(uint8_t* rdram,uint32_t actor){
    if(!fruit_mix::range(actor,0x174))return {};const auto a=gpr(int32_t(actor));const auto id=MEM_HU(12,a);
    if(id!=0x471A && id!=0x46E6)return {};
    // These native victory constructors already gate completion. The live
    // counter may be reset during the transition; all thirty ring entries survive.
    const auto base=gpr(int32_t(0x800F89D4));std::array<unsigned,30> ring;
    for(unsigned i=0;i<30;i++)ring[i]=uint32_t(MEM_W(i*4,base));
    auto fruit=special_type(30,ring);if(!fruit)return {};
    const uint32_t render=MEM_W(0x16C,a);if(!fruit_mix::range(render,21))return {};const auto drawing=gpr(int32_t(render));
    MEM_H(0x144,a)=6+*fruit;
    for(unsigned shade=0;shade<2;shade++){for(unsigned c=0;c<3;c++)MEM_B(13+shade*4+c,drawing)=colors[*fruit][c];MEM_B(16+shade*4,drawing)=shade?0:255;}
    return fruit;
}
}
