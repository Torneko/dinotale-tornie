#pragma once
#include "fruit_mix.hpp"
#include <array>
#include <bit>
#include <cstdint>

namespace workshop::collections::frustration0283 {
inline constexpr unsigned room=14,level=11,original_count=115,extra_count=8,
    supplemented_count=original_count+extra_count,native_capacity=150;
static_assert(supplemented_count<native_capacity);
struct Point {float x,y;};
// Room14's native80BE terrain is clear above the continuous floor atY464
// throughoutX0..383. Spawn is72,440: these need only a small normal jump.
inline constexpr std::array<Point,extra_count> additions{{
    {40.f,400.f},{72.f,400.f},{104.f,400.f},{136.f,400.f},
    {168.f,400.f},{200.f,400.f},{232.f,400.f},{264.f,400.f}
}};
inline constexpr std::array<Point,12> originals{{
    {2888.f,152.f},{2920.f,152.f},{2952.f,152.f},{2968.f,184.f},
    {3560.f,200.f},{3592.f,200.f},{3624.f,232.f},{4760.f,408.f},
    {4984.f,392.f},{5160.f,376.f},{5336.f,376.f},{5496.f,360.f}
}};
inline gpr address(uint32_t a){return gpr(int32_t(a));}
inline bool point(uint8_t* rdram,gpr record,Point p){
    return uint32_t(MEM_W(4,record))==std::bit_cast<uint32_t>(p.x) &&
        uint32_t(MEM_W(8,record))==std::bit_cast<uint32_t>(p.y);
}
inline bool coordinates(uint8_t* rdram,uint32_t objects){
    if((objects&3) || !fruit_mix::range(objects,native_capacity*16))return false;
    for(unsigned i=0;i<originals.size();i++)if(!point(rdram,address(objects+(14+i)*16),originals[i]))return false;
    return point(rdram,address(objects+82*16),{72.f,440.f}) &&
        point(rdram,address(objects+112*16),{160.f,120.f}) &&
        point(rdram,address(objects+113*16),{160.f,120.f}) &&
        point(rdram,address(objects+114*16),{160.f,120.f});
}
inline bool source(uint8_t* rdram,uint32_t objects){
    if(!coordinates(rdram,objects))return false;
    const auto list=address(objects);unsigned coins=0;
    for(unsigned i=0;i<original_count;i++)coins+=MEM_HU(i*16,list)==0x406F;
    if(coins!=originals.size())return false;
    for(unsigned i=0;i<originals.size();i++)if(MEM_HU((14+i)*16,list)!=0x406F ||
        !point(rdram,address(objects+(14+i)*16),originals[i]))return false;
    return MEM_HU(82*16,list)==0x4001 && point(rdram,address(objects+82*16),{72.f,440.f}) &&
        MEM_HU(112*16,list)==0x80BE && MEM_HU(113*16,list)==0x80BF && MEM_HU(114*16,list)==0x80C0;
}
// Native48E9C/6D340 consumes record word0 asFFFFFFFF;48CA4 restores it.
// Attest the unconsumed source once, then require that exact table plus its
// stationary source coordinates on cached revisits. No actor/flag is repaired.
struct Binding {uint32_t metadata=0,objects=0;};
inline Binding binding{};
// Called by the existing collection entry hook before80051418 reads count.
// The actual8006D85C loader clears150*16 bytes at8006D970..8006D984 and
// expands ROM records to that16-byte buffer. Reuse its unused suffix; never
// replace its pointer/allocation, existing records, flags or actor backlinks.
inline bool prepare(uint8_t* rdram,uint32_t metadata,bool enabled){
    if(!rdram || (metadata&3) || !fruit_mix::range(metadata,8))return false;
    const auto meta=address(metadata);const unsigned count=MEM_HU(0,meta);
    if(count!=original_count && count!=supplemented_count)return false;
    const uint32_t objects=MEM_W(4,meta);
    if(count==supplemented_count){
        if(binding.metadata!=metadata || binding.objects!=objects || !coordinates(rdram,objects))return false;
        for(unsigned i=0;i<extra_count;i++){
            const auto record=address(objects+(original_count+i)*16);
            if((MEM_HU(0,record)!=0x406F && uint32_t(MEM_W(0,record))!=0xFFFFFFFFu) ||
                !point(rdram,record,additions[i]))return false;
        }
        if(!enabled)MEM_H(0,meta)=original_count;
        return enabled; // Native flags/backlinks survive repeated entry.
    }
    if(!enabled || !source(rdram,objects))return false;
    binding={metadata,objects};
    for(unsigned i=0;i<extra_count;i++){
        const auto record=address(objects+(original_count+i)*16);
        MEM_W(0,record)=0x406F0000u+(12+i);
        MEM_W(4,record)=std::bit_cast<uint32_t>(additions[i].x);
        MEM_W(8,record)=std::bit_cast<uint32_t>(additions[i].y);
        MEM_W(12,record)=0;
    }
    MEM_H(0,meta)=supplemented_count;
    return true;
}
}
