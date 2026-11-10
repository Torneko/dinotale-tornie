#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <stdexcept>
#include <vector>
#include "hybrid_maskass_data.hpp"
#include "vanilla_sources.hpp"
#include "terrain_data.hpp"
#include "special_bubble_data.hpp"
#include "editor_object_data.hpp"
namespace workshop {
constexpr size_t rom_size=0x1000000,level_entry=0xA84A0+43*8,allocation_start=0xF00000;
constexpr uint32_t level_entry_ram=0x80000400+level_entry-0x1000;
constexpr std::array<uint8_t,8> magic{'Y','S','L','E','V','E','L','1'};
inline void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
inline uint32_t u32(std::span<const uint8_t> b,size_t p){require(p<=b.size() && b.size()-p>=4,"Truncated data");return uint32_t(b[p])<<24|uint32_t(b[p+1])<<16|uint32_t(b[p+2])<<8|b[p+3];}
inline uint16_t u16(std::span<const uint8_t> b,size_t p){require(p<=b.size() && b.size()-p>=2,"Truncated data");return uint16_t(b[p])<<8|b[p+1];}
inline void put32(std::vector<uint8_t>& b,size_t p,uint32_t v){require(p<=b.size() && b.size()-p>=4,"Write outside data");for(int n=0;n<4;n++)b[p+n]=uint8_t(v>>(24-8*n));}
struct RoomPayload {
    unsigned source,count,stride;size_t offset;
    unsigned terrain_count=0;size_t terrain_offset=0;int music_id=-1;
    unsigned entry_flags=0;uint32_t entry_x=0,entry_y=0;
    unsigned route_count=0;size_t route_offset=0;
};
inline std::vector<RoomPayload> payload_rooms(std::span<const uint8_t> file){
    require(file.size()>=16 && std::equal(magic.begin(),magic.end()-1,file.begin()) && file[7]>='1' && file[7]<='6',"Expected YSLEVEL data");
    std::vector<RoomPayload> result;
    if(file[7]>='3'){
        unsigned rooms=u32(file,12);require(rooms>=1 && rooms<=10,"Invalid room count");size_t p=16;
        for(unsigned i=0;i<rooms;i++){
            unsigned source=u32(file,p),count=u32(file,p+4),terrain_count=file[7]>='4'?u32(file,p+8):0;
            int music=file[7]>='5'?std::bit_cast<int32_t>(u32(file,p+12)):-1;
            unsigned entry_flags=file[7]=='6'?u32(file,p+16):0,entry_x=file[7]=='6'?u32(file,p+20):0,entry_y=file[7]=='6'?u32(file,p+24):0,routes=file[7]=='6'?u32(file,p+28):0;
            p+=file[7]=='6'?32:file[7]=='5'?16:file[7]=='4'?12:8;
            require(count>=1 && count<=149 && size_t(count)*16<=file.size()-std::min(file.size(),p),"Invalid room placements");
            RoomPayload room{source,count,16,p,terrain_count,0,music,entry_flags,entry_x,entry_y,routes,0};
            p+=size_t(count)*16;
            require(terrain_count<=65536 && size_t(terrain_count)*8<=file.size()-std::min(file.size(),p),"Invalid terrain change count");
            room.terrain_offset=p;p+=size_t(terrain_count)*8;
            require(routes<=count && size_t(routes)*20<=file.size()-std::min(file.size(),p),"Invalid passage count");
            room.route_offset=p;p+=size_t(routes)*20;result.push_back(room);
        }
        require(p==file.size(),"Unexpected trailing room data");
    }else{unsigned stride=file[7]=='2'?16:12,count=u32(file,12);require(count>=1 && count<=149 && file.size()==16+size_t(count)*stride,"Invalid placement count or file length");result.push_back({u32(file,8),count,stride,16});}
    return result;
}
inline unsigned payload_count(std::span<const uint8_t> file){unsigned n=0;for(const auto& r:payload_rooms(file))n+=r.count;return n;}
inline void validate_payload(std::span<const uint8_t> file){
    auto rooms=payload_rooms(file);const unsigned root=u32(file,8);require(vanilla_stage(root)>=0 && rooms.front().source==root,"Invalid vanilla level source");
    std::set<unsigned> included;for(const auto& room:rooms)included.insert(room.source);
    std::set<unsigned> seen;unsigned total=0,total_terrain=0;
    for(const auto& room:rooms){
        require(room.entry_flags<=1,"Unknown entry flags");
        for(uint32_t bits:{room.entry_x,room.entry_y}){float value=std::bit_cast<float>(bits);require(std::isfinite(value) && std::abs(value)<=100000,"Invalid entry coordinates");}
        require(room.entry_flags || (!room.entry_x && !room.entry_y),"Unused entry coordinates");
        std::set<unsigned> routed;
        for(unsigned i=0;i<room.route_count;i++){size_t p=room.route_offset+size_t(i)*20;require(u32(file,p)<room.count && routed.insert(u32(file,p)).second,"Duplicate or invalid passage index");require(included.contains(u32(file,p+4)),"Passage destination must be included");require(u32(file,p+8)<=1,"Unknown passage flags");for(size_t axis:{size_t(12),size_t(16)}){float value=std::bit_cast<float>(u32(file,p+axis));require(std::isfinite(value) && std::abs(value)<=100000,"Invalid passage arrival");}require(u32(file,p+8) || (!u32(file,p+12) && !u32(file,p+16)),"Unused passage coordinates");}
        require(room.music_id==-1 || (room.music_id>=1 && room.music_id<=16) || room.music_id==54,"Invalid room music");require(vanilla_room(root,room.source) && seen.insert(room.source).second,"Room is duplicated or belongs to another level");total+=room.count;total_terrain+=room.terrain_count;std::set<std::pair<unsigned,unsigned>> cells;for(unsigned i=0;i<room.terrain_count;i++){size_t p=room.terrain_offset+size_t(i)*8;require(u16(file,p)>=0x8000 && cells.emplace(u16(file,p),u16(file,p+2)).second,"Duplicate or invalid terrain cell");}unsigned spawns=0;std::map<std::array<uint32_t,3>,uint8_t> colors;std::map<std::array<uint32_t,3>,bool> egg_places,black_places;
        for(unsigned i=0;i<room.count;i++){size_t p=room.offset+size_t(i)*room.stride;spawns+=u16(file,p)==0x4001;
            if(u16(file,p)==0x45E3 || u16(file,p)==0x45E5){bool standalone=u16(file,p)==0x45E3;auto [it,fresh]=black_places.emplace(std::array<uint32_t,3>{u16(file,p+2),u32(file,p+4),u32(file,p+8)},standalone);require(fresh || (!standalone && !it->second),"Standalone black carrier overlaps another controller");}
            if(editor_objects::special(u16(file,p)) || editor_objects::native(u16(file,p))){bool added=editor_objects::special(u16(file,p));auto [it,fresh]=egg_places.emplace(std::array<uint32_t,3>{u16(file,p+2),u32(file,p+4),u32(file,p+8)},added);require(fresh || (!added && !it->second),"Editor egg bubbles overlap");}
            if(room.stride==16){require(file[p+13]==0 && file[p+14]==0 && file[p+15]==0,"Unknown placement extensions");require(file[p+12]==255 || (file[p+12]<6 && std::binary_search(hybrid_mode::maskass_ids.begin(),hybrid_mode::maskass_ids.end(),u16(file,p))),"Invalid hybrid Maskass variant");if(file[p+12]<6){auto [it,added]=colors.emplace(std::array<uint32_t,3>{u32(file,p),u32(file,p+4),u32(file,p+8)},file[p+12]);require(added || it->second==file[p+12],"Conflicting Maskass colors");}}
            for(size_t axis:{size_t(4),size_t(8)}){float c=std::bit_cast<float>(u32(file,p+axis));require(std::isfinite(c) && std::abs(c)<=100000,"Invalid placement coordinates");}
        }require(spawns==1,"Preserve exactly one Yoshi spawn in each room");
    }require(total<=6000,"Too many placements in custom level");require(total_terrain<=196608,"Too many terrain changes");
}
}
#include "navigation_data.hpp"
namespace workshop {
inline std::vector<uint8_t> apply(std::span<const uint8_t> original,std::span<const uint8_t> file,size_t allocation=allocation_start,bool special_content=false){
    validate_payload(file);require(original.size()==rom_size,"Unexpected original ROM size");
    navigation::validate(original,file,payload_rooms(file),special_content);
    auto bank=[&](uint32_t pointer)->size_t{require((pointer>>28)==0,"Invalid bank pointer");size_t p=size_t(u32(original,0xA79D4+((pointer>>24)&15)*4))+(pointer&0xFFFFFF);require(p<original.size(),"Bank pointer outside ROM");return p;};
    std::vector<uint8_t> output(original.begin(),original.end());size_t cursor=allocation,bank4=u32(original,0xA79D4+16);
    auto ptr=[&](size_t p){return 0x04000000u|uint32_t(p-bank4);};
    auto append=[&](std::span<const uint8_t> bytes){require(cursor<=output.size() && bytes.size()<=output.size()-cursor,"Custom placement space exhausted");require(std::all_of(output.begin()+cursor,output.begin()+cursor+bytes.size(),[](uint8_t b){return b==255;}),"Custom space is not empty");std::copy(bytes.begin(),bytes.end(),output.begin()+cursor);cursor+=bytes.size();};
    for(const auto& room:payload_rooms(file)){
        const size_t entry=0xA84A0+size_t(room.source)*8,header=bank(u32(original,entry+4)),meta=bank(u32(original,bank(u32(original,header+0x5C)))),objects=bank(u32(original,meta+4));
        std::set<uint16_t> allowed;for(size_t i=0;i<u16(original,meta);i++)allowed.insert(u16(original,objects+i*12));if(special_content)for(uint16_t id=0x4F00;id<0x4F06;id++)allowed.insert(id);
        if(allowed.contains(0x45E5))for(size_t p=header+0x60;p<header+0x118;p+=2){auto id=u16(original,p);if(id==65535)break;if(id==0x45E3){allowed.insert(0x45E3);break;}}
        uint16_t egg_alias=0,egg_layer=0;for(auto id:allowed)if(editor_objects::native(id)){egg_alias=id;break;}
        if(egg_alias)for(unsigned i=0;i<u16(original,meta);i++)if(u16(original,objects+i*12)==egg_alias){egg_layer=u16(original,objects+i*12+2);break;}
        uint16_t bubble_alias=0;for(auto id:allowed)if(special_bubbles::native(id)){bubble_alias=id;break;}
        for(unsigned i=0;i<room.count;i++){size_t p=room.offset+size_t(i)*room.stride;require(allowed.contains(u16(file,p)) || (special_content && special_bubbles::special(u16(file,p)) && bubble_alias) || (special_content && editor_objects::special(u16(file,p)) && egg_alias),"Object resources are unavailable in its source room");require(special_content || u16(file,p)!=0x45E3,"Standalone black carrier requires custom runtime");require(special_content || room.stride==12 || file[p+12]==255,"Hybrid Maskass requires special runtime");require(!editor_objects::special(u16(file,p)) || u16(file,p+2)==egg_layer,"Preserve native editor egg parameters");}
        // The native loader (func_8006D85C) reads a fixed 0x118-byte LevelInfo.
        const size_t next=header+0x118;require(next<=original.size(),"Invalid source descriptor");
        size_t original_spawn=objects,custom_spawn=room.offset;
        for(unsigned i=0;i<u16(original,meta);i++)if(u16(original,objects+i*12)==0x4001)original_spawn=objects+i*12;
        for(unsigned i=0;i<room.count;i++)if(u16(file,room.offset+i*room.stride)==0x4001)custom_spawn=room.offset+i*room.stride;
        require(u32(original,original_spawn)==u32(file,custom_spawn),"Preserve native Yoshi entry parameters");
        if(room.entry_flags)require(u32(file,custom_spawn+4)==room.entry_x && u32(file,custom_spawn+8)==room.entry_y,"Yoshi placement must match the room entry");
        else require(u32(original,original_spawn+4)==u32(file,custom_spawn+4) && u32(original,original_spawn+8)==u32(file,custom_spawn+8),"Preserve the inherited native entry point");
        size_t new_objects=cursor;for(unsigned i=0;i<room.count;i++){
            const auto record=file.subspan(room.offset+size_t(i)*room.stride,12);std::vector<uint8_t> bytes(record.begin(),record.end());
            if(editor_objects::special(u16(record,0))){bytes[0]=uint8_t(egg_alias>>8);bytes[1]=uint8_t(egg_alias);}
            if(special_bubbles::special(u16(record,0))){bytes[0]=uint8_t(bubble_alias>>8);bytes[1]=uint8_t(bubble_alias);}
            if(u16(record,0)==0x45E3){bytes[0]=0x45;bytes[1]=0xE5;}
            append(bytes);
        }cursor=(cursor+15)&~size_t(15);size_t new_meta=cursor;
        std::vector<uint8_t> m(12);m[0]=uint8_t(room.count>>8);m[1]=uint8_t(room.count);put32(m,4,ptr(new_objects));put32(m,8,ptr(new_meta));append(m);cursor=(cursor+15)&~size_t(15);size_t new_header=cursor;
        std::vector<uint8_t> descriptor(original.begin()+header,original.begin()+next);put32(descriptor,0x5C,ptr(new_meta+8));if(room.music_id>=0)put32(descriptor,0x100,uint32_t(room.music_id));if(room.entry_flags){auto origin=navigation::camera_origin({room.entry_x,room.entry_y});put32(descriptor,0x28,origin[0]);put32(descriptor,0x2C,origin[1]);}append(descriptor);put32(output,entry+4,ptr(new_header));
    }return terrain::apply(std::move(output),original,file,payload_rooms(file));
}
}
