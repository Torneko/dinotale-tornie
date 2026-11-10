#pragma once
#include "workshop_data.hpp"
#include "recomp.h"
#include <optional>
#include <unordered_map>
namespace workshop::special_bubbles {
using Key=std::array<uint32_t,4>;
inline std::map<Key,unsigned> placements;
inline std::map<unsigned,uint16_t> aliases;
inline std::unordered_map<uint32_t,unsigned> actors;
inline void configure(std::span<const uint8_t> original,std::span<const uint8_t> payload){
    placements.clear();aliases.clear();actors.clear();if(payload.empty())return;
    for(const auto& room:payload_rooms(payload)){
        const auto header=terrain::bank(original,u32(original,0xA84A0+room.source*8+4));
        const auto meta=terrain::bank(original,u32(original,terrain::bank(original,u32(original,header+0x5C))));
        const auto objects=terrain::bank(original,u32(original,meta+4));uint16_t alias=65535;
        for(unsigned i=0;i<u16(original,meta);i++){auto id=u16(original,objects+i*12);if(native(id))alias=std::min(alias,id);}
        if(alias!=65535)aliases.emplace(room.source,alias);
        for(unsigned i=0;i<room.count;i++){const auto p=room.offset+i*room.stride;auto id=u16(payload,p);if(!special(id))continue;
            require(alias!=65535,"No compatible fruit bubble in source room");
            for(unsigned j=0;j<room.count;j++){const auto q=room.offset+j*room.stride;require(u16(payload,q)!=alias || u16(payload,q+2)!=u16(payload,p+2) || u32(payload,q+4)!=u32(payload,p+4) || u32(payload,q+8)!=u32(payload,p+8),"A native and special fruit bubble overlap");}
            const Key key{room.source,uint32_t(alias)<<16|u16(payload,p+2),u32(payload,p+4),u32(payload,p+8)};
            auto [it,added]=placements.emplace(key,id-0x4F20);require(added || it->second==id-0x4F20,"Overlapping special fruit bubbles must have the same fruit");
        }
    }
}
inline uint32_t packed(unsigned room,uint32_t value){if(!special(value>>16))return value;auto a=aliases.find(room);require(a!=aliases.end(),"Missing native bubble alias");return uint32_t(a->second)<<16|(value&65535);}
inline void created(recomp_context* ctx,unsigned room){
    const auto actor=uint32_t(ctx->r4);actors.erase(actor);
    auto it=placements.find({room&255,uint32_t(ctx->r5),uint32_t(ctx->r6),uint32_t(ctx->r7)});
    if(it!=placements.end())actors.emplace(actor,it->second);
}
inline std::optional<uint16_t> fruit(uint32_t parent){auto it=actors.find(parent);if(it==actors.end())return {};return uint16_t(0x4F00+it->second);}
}
