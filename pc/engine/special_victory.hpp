#pragma once
#include "terrain_data.hpp"
#include <optional>
namespace workshop::special_victory {
using namespace workshop::terrain;
inline Bytes extend(Bytes rom){
    need(rom.size()>=0x1000000 && rom.size()<=0x1400000,"Invalid victory content size");
    size_t entry=0;for(unsigned i=0;i<1582;i++)if(be16(rom,0xA1C90+i*12)==0x471A){entry=0xA1C90+i*12;break;}
    need(entry!=0,"Missing victory resource");const size_t a=bank(rom,be32(rom,entry+4)),d=bank(rom,be32(rom,a+0x20));
    Bytes pixels=block(rom,be32(rom,a+0x28)),mapping=block(rom,be32(rom,a+0x38));
    need(pixels.size()==6*64*64 && mapping.size()==12 && be16(rom,d+4)==64 && be16(rom,d+6)==64,"Unexpected native victory atlas");
    pixels.resize(12*64*64,0xF0);
    for(unsigned fruit=0;fruit<6;fruit++){
        const size_t source=bank(rom,be32(rom,0xFB0000+fruit*12+4));
        Bytes ci=block(rom,be32(rom,source+0x28)),palette=block(rom,be32(rom,source+0x30));
        need(ci.size()==9*24*21 && palette.size()==512,"Invalid approved fruit sprite");
        // Preserve aspect ratio, native margin and antialiased alpha edge.
        for(unsigned y=0;y<48;y++)for(unsigned x=0;x<54;x++){
            unsigned solid=0;for(unsigned sy=0;sy<4;sy++)for(unsigned sx=0;sx<4;sx++){
                unsigned fx=std::min(23u,(x*4+sx)*24/(54*4)),fy=std::min(20u,(y*4+sy)*21/(48*4));
                solid+=be16(palette,ci[fy*24+fx]*2)&1;
            }
            pixels[(6+fruit)*4096+(y+8)*64+x+5]=uint8_t(0xF0|((solid*15+8)/16));
        }
    }
    Bytes dims(rom.begin()+d,rom.begin()+d+12),descriptor(rom.begin()+a,rom.begin()+a+0x64);half(dims,10,12*64);
    mapping.resize(24);for(unsigned i=0;i<12;i++)half(mapping,i*2,i);
    need(be32(rom,0xA79D4+60)==0 || be32(rom,0xA79D4+60)==0x1000000,"Victory extension bank occupied");
    word(rom,0xA79D4+60,0x1000000);
    auto pointer=[](size_t at){need(at>=0x1000000 && at<0x1400000,"Victory bank overflow");return 0x0F000000u|uint32_t(at-0x1000000);};
    auto append=[&](std::span<const uint8_t> bytes){size_t at=(rom.size()+15)&~size_t(15);need(at<=0x1400000 && bytes.size()<=0x1400000-at,"Victory atlas exceeds capacity");rom.resize(at+bytes.size());std::copy(bytes.begin(),bytes.end(),rom.begin()+at);return pointer(at);};
    auto store=[&](const Bytes& b){uint32_t at=append(b);Bytes info(12);word(info,0,uint32_t(b.size()));word(info,4,uint32_t(b.size()));word(info,8,at);return append(info);};
    word(descriptor,0x20,append(dims));word(descriptor,0x28,store(pixels));word(descriptor,0x38,store(mapping));
    const auto resource=append(descriptor);const uint32_t original_pointer=be32(rom,entry+4);
    // 46E6 (heart transition) and 471A (fruit transition) alias the same atlas.
    for(unsigned i=0;i<1582;i++){size_t e=0xA1C90+i*12;if(be32(rom,e+4)==original_pointer)word(rom,e+4,resource);}return rom;
}
inline std::optional<unsigned> special_type(unsigned count,std::span<const unsigned> ring){
    if(count!=30 || ring.size()!=30 || ring[0]<6 || ring[0]>11 || !std::all_of(ring.begin(),ring.end(),[&](auto n){return n==ring[0];}))return {};
    return ring[0]-6;
}
inline constexpr unsigned colors[6][3]={{239,139,48},{146,66,192},{33,188,185},{174,205,87},{135,85,47},{37,117,67}};
}
