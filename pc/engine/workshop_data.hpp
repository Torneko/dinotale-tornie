#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <map>
#include <set>
#include <span>
#include <stdexcept>
#include <vector>
#include "hybrid_maskass_data.hpp"
#include "vanilla_sources.hpp"
namespace workshop {
constexpr size_t rom_size=0x1000000,level_entry=0xA84A0+43*8,allocation_start=0xF00000;
constexpr uint32_t level_entry_ram=0x80000400+level_entry-0x1000;
constexpr std::array<uint8_t,8> magic{'Y','S','L','E','V','E','L','1'};
inline void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
inline uint32_t u32(std::span<const uint8_t> b,size_t p){require(p<=b.size() && b.size()-p>=4,"Truncated data");return uint32_t(b[p])<<24|uint32_t(b[p+1])<<16|uint32_t(b[p+2])<<8|b[p+3];}
inline uint16_t u16(std::span<const uint8_t> b,size_t p){require(p<=b.size() && b.size()-p>=2,"Truncated data");return uint16_t(b[p])<<8|b[p+1];}
inline void put32(std::vector<uint8_t>& b,size_t p,uint32_t v){require(p<=b.size() && b.size()-p>=4,"Write outside data");for(int n=0;n<4;n++)b[p+n]=uint8_t(v>>(24-8*n));}
struct RoomPayload {unsigned source,count,stride;size_t offset;};
inline std::vector<RoomPayload> payload_rooms(std::span<const uint8_t> file){
    require(file.size()>=16 && std::equal(magic.begin(),magic.end()-1,file.begin()) && file[7]>='1' && file[7]<='3',"Expected YSLEVEL data");
    std::vector<RoomPayload> result;
    if(file[7]=='3'){
        unsigned rooms=u32(file,12);require(rooms>=1 && rooms<=10,"Invalid room count");size_t p=16;
        for(unsigned i=0;i<rooms;i++){unsigned source=u32(file,p),count=u32(file,p+4);p+=8;require(count>=1 && count<=149 && size_t(count)*16<=file.size()-std::min(file.size(),p),"Invalid room placements");result.push_back({source,count,16,p});p+=size_t(count)*16;}
        require(p==file.size(),"Unexpected trailing room data");
    }else{unsigned stride=file[7]=='2'?16:12,count=u32(file,12);require(count>=1 && count<=149 && file.size()==16+size_t(count)*stride,"Invalid placement count or file length");result.push_back({u32(file,8),count,stride,16});}
    return result;
}
inline unsigned payload_count(std::span<const uint8_t> file){unsigned n=0;for(const auto& r:payload_rooms(file))n+=r.count;return n;}
inline void validate_payload(std::span<const uint8_t> file){
    auto rooms=payload_rooms(file);const unsigned root=u32(file,8);require(vanilla_stage(root)>=0 && rooms.front().source==root,"Invalid vanilla level source");
    std::set<unsigned> seen;unsigned total=0;
    for(const auto& room:rooms){require(vanilla_room(root,room.source) && seen.insert(room.source).second,"Room is duplicated or belongs to another level");total+=room.count;unsigned spawns=0;std::map<std::array<uint32_t,3>,uint8_t> colors;
        for(unsigned i=0;i<room.count;i++){size_t p=room.offset+size_t(i)*room.stride;spawns+=u16(file,p)==0x4001;
            if(room.stride==16){require(file[p+13]==0 && file[p+14]==0 && file[p+15]==0,"Unknown placement extensions");require(file[p+12]==255 || (file[p+12]<6 && std::binary_search(hybrid_mode::maskass_ids.begin(),hybrid_mode::maskass_ids.end(),u16(file,p))),"Invalid hybrid Maskass variant");if(file[p+12]<6){auto [it,added]=colors.emplace(std::array<uint32_t,3>{u32(file,p),u32(file,p+4),u32(file,p+8)},file[p+12]);require(added || it->second==file[p+12],"Conflicting Maskass colors");}}
            for(size_t axis:{size_t(4),size_t(8)}){float c=std::bit_cast<float>(u32(file,p+axis));require(std::isfinite(c) && std::abs(c)<=100000,"Invalid placement coordinates");}
        }require(spawns==1,"Preserve exactly one Yoshi spawn in each room");
    }require(total<=6000,"Too many placements in custom level");
}
inline std::vector<uint8_t> apply(std::span<const uint8_t> original,std::span<const uint8_t> file,size_t allocation=allocation_start,bool special_content=false){
    validate_payload(file);require(original.size()==rom_size,"Unexpected original ROM size");
    auto bank=[&](uint32_t pointer)->size_t{require((pointer>>28)==0,"Invalid bank pointer");size_t p=size_t(u32(original,0xA79D4+((pointer>>24)&15)*4))+(pointer&0xFFFFFF);require(p<original.size(),"Bank pointer outside ROM");return p;};
    std::vector<uint8_t> output(original.begin(),original.end());size_t cursor=allocation,bank4=u32(original,0xA79D4+16);
    auto ptr=[&](size_t p){return 0x04000000u|uint32_t(p-bank4);};
    auto append=[&](std::span<const uint8_t> bytes){require(cursor<=output.size() && bytes.size()<=output.size()-cursor,"Custom placement space exhausted");require(std::all_of(output.begin()+cursor,output.begin()+cursor+bytes.size(),[](uint8_t b){return b==255;}),"Custom space is not empty");std::copy(bytes.begin(),bytes.end(),output.begin()+cursor);cursor+=bytes.size();};
    for(const auto& room:payload_rooms(file)){
        const size_t entry=0xA84A0+size_t(room.source)*8,header=bank(u32(original,entry+4)),meta=bank(u32(original,bank(u32(original,header+0x5C)))),objects=bank(u32(original,meta+4));
        std::set<uint16_t> allowed;for(size_t i=0;i<u16(original,meta);i++)allowed.insert(u16(original,objects+i*12));if(special_content)for(uint16_t id=0x4F00;id<0x4F06;id++)allowed.insert(id);
        for(unsigned i=0;i<room.count;i++){size_t p=room.offset+size_t(i)*room.stride;require(allowed.contains(u16(file,p)),"Object resources are unavailable in its source room");require(special_content || room.stride==12 || file[p+12]==255,"Hybrid Maskass requires special runtime");}
        // The native loader (func_8006D85C) reads a fixed 0x118-byte LevelInfo.
        const size_t next=header+0x118;require(next<=original.size(),"Invalid source descriptor");
        size_t original_spawn=objects,custom_spawn=room.offset;
        for(unsigned i=0;i<u16(original,meta);i++)if(u16(original,objects+i*12)==0x4001)original_spawn=objects+i*12;
        for(unsigned i=0;i<room.count;i++)if(u16(file,room.offset+i*room.stride)==0x4001)custom_spawn=room.offset+i*room.stride;
        require(u32(original,original_spawn)==u32(file,custom_spawn) && u32(original,original_spawn+4)==u32(file,custom_spawn+4) && u32(original,original_spawn+8)==u32(file,custom_spawn+8),"Preserve the native entry point; camera-aware spawn editing is not supported yet");
        size_t new_objects=cursor;for(unsigned i=0;i<room.count;i++)append(file.subspan(room.offset+size_t(i)*room.stride,12));cursor=(cursor+15)&~size_t(15);size_t new_meta=cursor;
        std::vector<uint8_t> m(12);m[0]=uint8_t(room.count>>8);m[1]=uint8_t(room.count);put32(m,4,ptr(new_objects));put32(m,8,ptr(new_meta));append(m);cursor=(cursor+15)&~size_t(15);size_t new_header=cursor;
        std::vector<uint8_t> descriptor(original.begin()+header,original.begin()+next);put32(descriptor,0x5C,ptr(new_meta+8));append(descriptor);put32(output,entry+4,ptr(new_header));
    }return output;
}
}
