#pragma once
#include "recomp.h"
#include <array>
#include <cstdint>
#include <unordered_map>

namespace workshop::fruit_mix {
// One of 60 equally likely tickets: 45 retain the original, and five tickets
// for each of its three component-compatible hybrids. Total change = 25%.
inline constexpr std::array<std::array<uint16_t,3>,4> choices{{
    {0x4F00,0x4F01,0x4F04}, // apple: peach, blackberry, plum
    {0x4F00,0x4F03,0x4F05}, // banana: peach, pear, kiwi
    {0x4F02,0x4F03,0x4F04}, // watermelon: blueberry, pear, plum
    {0x4F01,0x4F02,0x4F05}  // grapes: blackberry, blueberry, kiwi
}};
constexpr bool eligible(uint16_t id){return id>=0x4030 && id<=0x4033;}
constexpr uint16_t replacement(uint16_t id,unsigned ticket,unsigned threshold=15){
    return eligible(id) && ticket<threshold?choices[id-0x4030][ticket%3]:id;
}
struct Random {
    uint32_t state;
    explicit Random(uint32_t seed):state(seed?seed:1){}
    unsigned ticket(){
        uint32_t value;
        do {state^=state<<13;state^=state>>17;state^=state<<5;value=state;} while(value<16);
        return value%60;
    }
};
// A bubble/carrier may recreate its preview before releasing the same fruit.
// Its first choice must survive that recreation, including an unchanged fruit.
struct SpawnChoices {
    std::unordered_map<uint32_t,std::array<uint16_t,4>> parents;
    void clear(){parents.clear();}
    void created(uint32_t parent){parents.erase(parent);}
    template<class Tickets> uint16_t pick(uint16_t id,uint32_t parent,Tickets&& tickets,unsigned threshold=15){
        if(!eligible(id))return id;
        if(!parent)return replacement(id,tickets(),threshold);
        auto& choice=parents[parent][id-0x4030];
        if(!choice)choice=replacement(id,tickets(),threshold);
        return choice;
    }
};
constexpr bool preview_source(uint16_t id){
    return (id>=0x41D1 && id<=0x41D7) || (id>=0x452A && id<=0x4530)
        || (id>=0x468D && id<=0x4694) || (id>=0x46A8 && id<=0x46AE)
        || (id>=0x4722 && id<=0x4724) || id==0x4647
        || (id>=0x4198 && id<=0x419D) || id==0x45B5 || id==0x4698;
}
inline bool range(uint32_t a,uint32_t bytes){return a>=0x80000400 && a<0x80800000 && bytes<=0x80800000-a;}
inline bool playable(uint8_t* rdram,uint32_t metadata){
    if(!range(metadata,8))return false;
    const auto meta=static_cast<gpr>(static_cast<int32_t>(metadata));
    const unsigned count=MEM_HU(0,meta);
    const uint32_t address=MEM_W(4,meta);
    if(!count || count>2000 || !range(address,count*16))return false;
    const auto objects=static_cast<gpr>(static_cast<int32_t>(address));
    bool player=false;
    for(unsigned i=0;i<count;i++)player|=MEM_HU(i*16,objects)==0x4001;
    return player; // Menu objects never become gameplay fruit.
}
struct Stats {unsigned eligible=0,changed=0;};
template<class Tickets> Stats apply(uint8_t* rdram,uint32_t metadata,bool enabled,Tickets&& tickets){
    Stats stats;if(!enabled || !playable(rdram,metadata))return stats;
    const auto meta=static_cast<gpr>(static_cast<int32_t>(metadata));
    const unsigned count=MEM_HU(0,meta);
    const auto objects=static_cast<gpr>(static_cast<int32_t>(MEM_W(4,meta)));
    for(unsigned i=0;i<count;i++){
        const uint16_t before=MEM_HU(i*16,objects);if(!eligible(before))continue;
        stats.eligible++;const auto after=replacement(before,tickets());
        if(after!=before){MEM_H(i*16,objects)=after;stats.changed++;}
    }
    return stats;
}
}
