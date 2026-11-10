#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <set>
#include <span>
#include <stdexcept>
#include <vector>

namespace workshop::terrain {
using Bytes=std::vector<uint8_t>;
inline void need(bool value,const char* message){if(!value)throw std::runtime_error(message);}
inline uint16_t be16(std::span<const uint8_t> b,size_t p){need(p<=b.size() && b.size()-p>=2,"Truncated terrain word");return uint16_t(b[p])<<8|b[p+1];}
inline uint32_t be32(std::span<const uint8_t> b,size_t p){need(p<=b.size() && b.size()-p>=4,"Truncated terrain pointer");return uint32_t(b[p])<<24|uint32_t(b[p+1])<<16|uint32_t(b[p+2])<<8|b[p+3];}
inline void word(Bytes& b,size_t p,uint32_t value){need(p<=b.size() && b.size()-p>=4,"Terrain write out of range");for(unsigned n=0;n<4;n++)b[p+n]=uint8_t(value>>(24-n*8));}
inline void half(Bytes& b,size_t p,uint16_t value){need(p<=b.size() && b.size()-p>=2,"Terrain halfword out of range");b[p]=uint8_t(value>>8);b[p+1]=uint8_t(value);}
inline size_t bank(std::span<const uint8_t> rom,uint32_t pointer){need((pointer>>28)==0,"Invalid terrain bank");size_t p=size_t(be32(rom,0xA79D4+((pointer>>24)&15)*4))+(pointer&0xFFFFFF);need(p<rom.size(),"Terrain bank outside content");return p;}
inline Bytes block(std::span<const uint8_t> rom,uint32_t pointer){
    size_t info=bank(rom,pointer),length=be32(rom,info),encoded=be32(rom,info+4),at=bank(rom,be32(rom,info+8));
    need(length>0 && length<=8388608 && encoded>0 && encoded<=8388608 && at<=rom.size() && encoded<=rom.size()-at,"Invalid terrain block length");
    auto input=rom.subspan(at,encoded);
    if(input.size()<4 || be32(input,0)!=0x434D5052){need(length<=encoded,"Truncated raw terrain");return Bytes(input.begin(),input.begin()+length);}
    need(input.size()>=32 && be32(input,16)==0x534D5352 && be16(input,20)==0x3030 && be32(input,24)==length,"Invalid SMSR terrain");
    size_t control=32,literal=32+size_t(be32(input,28));Bytes out;out.reserve(length);unsigned flags=0,bits=0;
    while(out.size()<length){
        if(!bits){flags=be16(input,control);control+=2;bits=16;}
        if(flags&0x8000){need(literal<input.size(),"Truncated terrain literals");out.push_back(input[literal++]);}
        else{unsigned pair=be16(input,control);control+=2;unsigned count=(pair>>12)+3,distance=(pair&4095)+1;need(distance<=out.size() && count<=length-out.size(),"Invalid terrain back reference");for(unsigned i=0;i<count;i++)out.push_back(out[out.size()-distance]);}
        flags=(flags<<1)&65535;bits--;
    }return out;
}
// Data is appended in a previously unused ROM bank. Native room dimensions,
// page counts and allocations remain unchanged; only selected cell values vary.
// The original cartridge image on disk and all executable code remain untouched.
template<class Rooms> inline Bytes apply(Bytes output,std::span<const uint8_t> original,std::span<const uint8_t> file,const Rooms& rooms){
    if(std::none_of(rooms.begin(),rooms.end(),[](const auto& room){return room.terrain_count!=0;}))return output;
    need(original.size()==0x1000000 && output.size()==0x1000000 && be32(original,0xA79D4+60)==0,"Terrain bank is unavailable");
    auto pointer=[](size_t offset){need(offset>=0x1000000 && offset<0x1400000,"Custom terrain bank overflow");return 0x0F000000u|uint32_t(offset-0x1000000);};
    auto append=[&](std::span<const uint8_t> bytes){size_t at=(output.size()+15)&~size_t(15);need(at<=0x1400000 && bytes.size()<=0x1400000-at,"Custom terrain data exceeds capacity");output.resize(at+bytes.size());std::copy(bytes.begin(),bytes.end(),output.begin()+at);return at;};
    auto store_block=[&](const Bytes& bytes){size_t at=append(bytes);Bytes info(12);word(info,0,uint32_t(bytes.size()));word(info,4,uint32_t(bytes.size()));word(info,8,pointer(at));return append(info);};
    std::map<unsigned,uint32_t> resources;
    for(const auto& room:rooms){
        std::map<unsigned,std::vector<size_t>> changes;
        for(unsigned i=0;i<room.terrain_count;i++){size_t at=room.terrain_offset+size_t(i)*8;changes[be16(file,at)].push_back(at);}
        for(const auto& [id,entries]:changes){
            need(!resources.contains(id),"Terrain resource shared across edited rooms");
            unsigned present=0;for(unsigned i=0;i<room.count;i++)if(be16(file,room.offset+i*room.stride)==id && be16(file,room.offset+i*room.stride+2)==1)present++;
            need(present==1,"Keep the original terrain placement");
            size_t a=0;for(unsigned i=0;i<1582;i++){size_t e=0xA1C90+i*12;if(be16(original,e)==id){a=bank(original,be32(original,e+4));break;}}
            need(a && a+0x64<=original.size() && be16(original,a)==0 && be32(original,a+8)==0x8006E548 && be32(original,a+0x4C)!=0,"Unknown primary terrain resource");
            size_t d=bank(original,be32(original,a+0x20));unsigned page=be16(original,d+2),tw=be16(original,d+4),th=be16(original,d+6),width=be16(original,d+8),height=be16(original,d+10);
            need(page==256 && tw==16 && th==16 && width && height && width%256==0 && height%256==0 && uint64_t(width)*height<=16777216,"Unsupported terrain dimensions");
            Bytes tiles=block(original,be32(original,a+0x38)),collision=block(original,be32(original,a+0x50)),pages=block(original,be32(original,a+0x40));
            need(!tiles.empty() && tiles.size()<=131072 && tiles.size()%512==0 && tiles.size()==collision.size(),"Invalid terrain page storage");
            unsigned loaded=unsigned(tiles.size()/512),page_cells=(width/256)*(height/256);std::set<unsigned> referenced;
            for(unsigned i=0;i<std::min(size_t(page_cells),pages.size());i++)if(pages[i]!=255){need(pages[i]<loaded,"Terrain page out of range");referenced.insert(pages[i]);}
            size_t pixels=bank(original,be32(original,a+0x28));unsigned pixel_bytes=be32(original,pixels);need(pixel_bytes && pixel_bytes%256==0,"Invalid terrain pixel atlas");unsigned tile_count=pixel_bytes/256;
            Bytes animation;if(be32(original,a+0x44)!=0)animation=block(original,be32(original,a+0x48));need(animation.size()%10==0,"Invalid terrain animation table");
            std::set<unsigned> masks{0};for(size_t i=0;i<collision.size();i+=2)masks.insert(be16(collision,i));
            for(size_t edit:entries){unsigned cell=be16(file,edit+2),tile=be16(file,edit+4),mask=be16(file,edit+6);
                need(cell<tiles.size()/2 && referenced.contains((cell/16)%loaded) && masks.contains(mask),"Terrain cell or collision is unavailable");
                bool valid=tile==65535 || (tile<32768 && tile<tile_count);
                if(tile>=32768 && tile!=65535){unsigned index=(tile&32767)*10;if(index+2<=animation.size()){unsigned frame=be16(animation,index);valid=frame==32768 || frame<tile_count;}}
                need(valid,"Terrain tile is unavailable");half(tiles,cell*2,uint16_t(tile));half(collision,cell*2,uint16_t(mask));
            }
            size_t tile_info=store_block(tiles),collision_info=store_block(collision);Bytes descriptor(original.begin()+a,original.begin()+a+0x64);word(descriptor,0x38,pointer(tile_info));word(descriptor,0x50,pointer(collision_info));resources.emplace(id,pointer(append(descriptor)));
        }
    }
    word(output,0xA79D4+60,0x1000000);
    for(unsigned i=0;i<1582;i++){size_t at=0xA1C90+i*12;auto changed=resources.find(be16(original,at));if(changed!=resources.end())word(output,at+4,changed->second);}
    return output;
}
}
