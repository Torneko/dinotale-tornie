#pragma once
#include "fruit_mix.hpp"
#include <map>

namespace workshop::fruit_mix {
// Room/placement identities belong to the level, never to a recycled RAM
// address. Keep unchanged outcomes too: otherwise every visit adds 25% again.
struct Visits {
    struct Choice {uint16_t original=0,selected=0;};
    using Source=std::array<uint32_t,5>; // room, initial type and initial XYZ
    std::map<uint32_t,std::map<unsigned,Choice>> placements;
    std::map<Source,std::array<uint16_t,4>> sources;
    std::unordered_map<uint32_t,Source> actors;
    void enter(){actors.clear();}
    void created(uint32_t actor,const Source& source){actors[actor]=source;}
    template<class Tickets> Stats apply(uint8_t* rdram,uint32_t metadata,uint32_t room,Tickets&& tickets,unsigned threshold=15){
        Stats stats;if(!playable(rdram,metadata))return stats;
        const auto meta=gpr(int32_t(metadata)),objects=gpr(int32_t(MEM_W(4,meta)));
        auto& choices=placements[room];
        for(unsigned i=0;i<MEM_HU(0,meta);i++){
            const uint16_t id=MEM_HU(i*16,objects);
            auto found=choices.find(i);
            if(found==choices.end()){
                if(!eligible(id))continue;
                found=choices.emplace(i,Choice{id,replacement(id,tickets(),threshold)}).first;
            }
            const auto& c=found->second;
            // A collected/disabled object must never be brought back.
            if(id!=c.original && id!=c.selected)continue;
            stats.eligible++;stats.changed+=c.original!=c.selected;
            MEM_H(i*16,objects)=c.selected;
        }
        return stats;
    }
    template<class Tickets> uint16_t pick(uint16_t id,uint32_t parent,Tickets&& tickets,unsigned threshold=15){
        if(!eligible(id))return id;
        auto found=actors.find(parent);
        if(found==actors.end())return replacement(id,tickets(),threshold);
        auto& chosen=sources[found->second][id-0x4030];
        if(!chosen)chosen=replacement(id,tickets(),threshold);
        return chosen;
    }
};
// DA0C receives the destination's stable room index before loading its data.
inline uint32_t visit_room(uint8_t* rdram,unsigned destination){
    return (uint32_t(MEM_W(0,gpr(int32_t(0x800F8B88))))&3)<<24
        | unsigned(MEM_BU(0,gpr(int32_t(0x800F8DFC))))<<16
        | unsigned(MEM_BU(0,gpr(int32_t(0x800F8DFD))))<<8 | (destination&255);
}
}
