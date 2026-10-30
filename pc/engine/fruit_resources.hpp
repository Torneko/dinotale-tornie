#pragma once
#include "fruit_mix.hpp"

namespace workshop::fruit_resources {
constexpr bool hybrid(uint16_t id){return id>=0x4F00 && id<=0x4F05;}
constexpr bool world(uint16_t id){return (id>=0x4030 && id<=0x4035)||hybrid(id);}

// Native fruit share an atlas, so their actor pool keeps its loaded resource.
// Hybrids have separate atlases: reusing an actor across that boundary must
// let the native initializer bind the requested resource again.
inline bool prepare(uint8_t* rdram,uint32_t actor,uint16_t requested){
    if(!world(requested) || !fruit_mix::range(actor,0x16C))return false;
    const auto a=gpr(int32_t(actor));
    const uint32_t resource=MEM_W(0x168,a);
    if(!fruit_mix::range(resource,0x40))return false;
    const uint16_t bound=MEM_HU(0x3C,int32_t(resource));
    if(bound==requested || (!hybrid(requested) && !hybrid(bound)))return false;
    MEM_W(0x168,a)=0;
    return true;
}

// The pooled path skips the fresh render-state constructor (800455DC).
// These static world sprites borrow their texture descriptor; they have no
// owned/animated texture buffers. Refresh the same borrowed references and
// default palette as that constructor, without reallocating the actor.
inline bool finish(uint8_t* rdram,uint32_t actor){
    if(!fruit_mix::range(actor,0x170))return false;
    const auto a=gpr(int32_t(actor));
    if(!world(MEM_HU(12,a)) || MEM_BU(0x164,a)>1)return false;
    const uint32_t resource=MEM_W(0x168,a),drawing=MEM_W(0x16C,a);
    if(!fruit_mix::range(resource,0x90) || !fruit_mix::range(drawing,0x44))return false;
    const auto r=gpr(int32_t(resource)),d=gpr(int32_t(drawing));
    if(MEM_W(0x30,r) || MEM_W(0x38,d) || MEM_W(0x3C,d))return false;
    const uint32_t palette=MEM_W(0x88,r),texture=MEM_W(0x84,r),geometry=resource+0x74;
    const bool changed=uint32_t(MEM_W(8,d))!=palette || uint32_t(MEM_W(0x30,d))!=texture
        || uint32_t(MEM_W(0x40,d))!=geometry || MEM_BU(4,d)!=0;
    MEM_W(8,d)=palette;MEM_B(4,d)=0;MEM_W(0x30,d)=texture;MEM_W(0x40,d)=geometry;
    return changed;
}
}
