#pragma once
// Included after RoomPayload and the payload readers have been declared.
namespace workshop::navigation {
using Point=std::array<uint32_t,2>;
inline bool coordinate(uint32_t bits){float c=std::bit_cast<float>(bits);return std::isfinite(c) && std::abs(c)<=100000;}
inline Point camera_origin(Point point){
    Point result{};
    for(unsigned axis=0;axis<2;axis++){
        float bucket=std::trunc(std::bit_cast<float>(point[axis])/256.0f)*256.0f;
        result[axis]=std::bit_cast<uint32_t>(bucket==0?0.0f:bucket+(axis?16.0f:-64.0f));
    }
    return result;
}
struct Object{uint32_t packed,x,y;};
struct Passage{size_t resource_a,record;uint32_t target;unsigned destination;};
class Source {
    std::span<const uint8_t> rom;
    std::map<uint16_t,std::pair<size_t,size_t>> resources;
public:
    explicit Source(std::span<const uint8_t> bytes):rom(bytes){
        for(unsigned i=0;i<1582;i++){size_t p=0xA1C90+i*12;resources.emplace(u16(rom,p),std::pair{bank(u32(rom,p+4)),bank(u32(rom,p+8))});}
    }
    size_t bank(uint32_t pointer) const {return terrain::bank(rom,pointer);}
    size_t header(unsigned room) const {require(room<177,"Invalid navigation source room");return bank(u32(rom,0xA84A0+size_t(room)*8+4));}
    std::vector<Object> objects(unsigned room) const {
        size_t h=header(room),meta=bank(u32(rom,bank(u32(rom,h+0x5C)))),p=bank(u32(rom,meta+4));
        unsigned count=u16(rom,meta);require(count && count<=149,"Invalid original room placements");
        std::vector<Object> out;out.reserve(count);
        for(unsigned i=0;i<count;i++,p+=12)out.push_back({u32(rom,p),u32(rom,p+4),u32(rom,p+8)});
        return out;
    }
    std::optional<Passage> passage(uint16_t id,bool pots_only=false) const {
        const auto found=resources.find(id);if(found==resources.end())return {};
        const auto [a,b]=found->second;if(b+0x38>rom.size())return {};
        if(pots_only){auto pot=resources.find(0x4208);if(pot==resources.end() || pot->second.first!=a)return {};}
        uint32_t ptr=u32(rom,b+0x34);if(!ptr || ptr>>28)return {};
        size_t p=bank(ptr);if(p+32>rom.size() || u32(rom,p)!=32 || rom[p+28]!=7 || rom[p+12]>=177)return {};
        if(!coordinate(u32(rom,p+16)) || !coordinate(u32(rom,p+20)))return {};
        return Passage{a,p,u32(rom,p+8),rom[p+12]};
    }
    std::optional<Point> native_arrival(uint16_t id,unsigned destination) const {
        const auto route=passage(id);if(!route || route->target==UINT32_MAX)return {};
        std::optional<Point> result;
        for(const auto& item:objects(destination))if((item.packed&0xFFFF03FFu)==(route->target&0xFFFF03FFu)){
            if(result)return {};result=Point{item.x,item.y};
        }
        return result;
    }
    template<class Rooms> Point effective_entry(unsigned root,unsigned destination,const Rooms& rooms) const {
        for(const auto& room:rooms)if(room.source==destination && room.entry_flags)return {room.entry_x,room.entry_y};
        for(const auto& item:objects(destination))if(item.packed>>16==0x4001 && (std::bit_cast<float>(item.x)!=0 || std::bit_cast<float>(item.y)!=0))return {item.x,item.y};
        const int stage=vanilla_stage(root);require(stage>=0,"Invalid entry source");
        for(unsigned source:vanilla_rooms[stage])if(source<177)for(const auto& item:objects(source)){
            const auto p=passage(uint16_t(item.packed>>16));if(!p || p->destination!=destination)continue;
            auto point=native_arrival(uint16_t(item.packed>>16),destination);if(point && coordinate((*point)[0]) && coordinate((*point)[1]))return *point;
        }
        throw std::runtime_error("No reliable native arrival; define a destination entry first");
    }
    bool within_terrain(unsigned room,Point point) const {
        const float x=std::bit_cast<float>(point[0]),y=std::bit_cast<float>(point[1]);
        for(const auto& item:objects(room)){
            if(item.packed>>16<0x8000 || uint16_t(item.packed)!=1)continue;
            auto found=resources.find(uint16_t(item.packed>>16));if(found==resources.end())continue;
            const size_t a=found->second.first;
            if(a+0x64>rom.size() || u16(rom,a)!=0 || u32(rom,a+8)!=0x8006E548 || !u32(rom,a+0x4C))continue;
            const size_t d=bank(u32(rom,a+0x20));
            unsigned page=u16(rom,d+2),tw=u16(rom,d+4),th=u16(rom,d+6),width=u16(rom,d+8),height=u16(rom,d+10);
            require(page==256 && tw==16 && th==16 && width && height && width%256==0 && height%256==0,"Unsupported entry terrain dimensions");
            const int tx=int(std::floor((x-(std::bit_cast<float>(item.x)-160.0f))/tw)),ty=int(std::floor((y-(std::bit_cast<float>(item.y)-120.0f))/th));
            if(tx<0 || ty<0 || tx>=int(width/tw) || ty>=int(height/th))continue;
            auto pages=terrain::block(rom,u32(rom,a+0x40)),tiles=terrain::block(rom,u32(rom,a+0x38));
            const size_t pi=size_t(ty/16)*(width/256)+unsigned(tx/16);
            if(pi>=pages.size() || pages[pi]==255)continue;
            unsigned loaded=unsigned(tiles.size()/512),id=pages[pi];
            if(id<loaded && (size_t(ty%16)*loaded*16+id*16+tx%16)*2<tiles.size())return true;
        }
        return false;
    }
    std::array<uint8_t,24> native_payload(const Passage& route) const {
        std::array<uint8_t,24> out;std::copy_n(rom.begin()+route.record+8,24,out.begin());return out;
    }
};
struct Plan {
    unsigned room,index,destination;uint32_t packed,x,y;
    Point arrival;std::array<uint8_t,24> native;
};
template<class Rooms> inline std::vector<Plan> plans(std::span<const uint8_t> original,std::span<const uint8_t> file,const Rooms& rooms){
    if(std::none_of(rooms.begin(),rooms.end(),[](const auto& room){return room.entry_flags || room.route_count;}))return {};
    Source source(original);std::vector<Plan> out;const unsigned root=u32(file,8);
    for(const auto& room:rooms)for(unsigned i=0;i<room.count;i++){
        const size_t p=room.offset+size_t(i)*room.stride;
        auto route=source.passage(u16(file,p),true);if(!route)continue;
        unsigned destination=route->destination;std::optional<Point> arrival;bool overridden=false;
        for(unsigned n=0;n<room.route_count;n++){size_t r=room.route_offset+size_t(n)*20;if(u32(file,r)!=i)continue;
            overridden=true;destination=u32(file,r+4);
            arrival=u32(file,r+8)?Point{u32(file,r+12),u32(file,r+16)}:source.effective_entry(root,destination,rooms);break;
        }
        if(!overridden)for(const auto& dest:rooms)if(dest.source==destination && dest.entry_flags){arrival=Point{dest.entry_x,dest.entry_y};break;}
        if(arrival){
            require(vanilla_room(root,room.source),"Passage source room is unavailable");
            const auto originals=source.objects(room.source);
            require(std::any_of(originals.begin(),originals.end(),[&](const auto& item){return item.packed==u32(file,p);}),"Preserve a native source-room pot parameter template");
            out.push_back({room.source,i,destination,u32(file,p),u32(file,p+4),u32(file,p+8),*arrival,source.native_payload(*route)});
        }
    }
    return out;
}
template<class Rooms> inline void validate(std::span<const uint8_t> original,std::span<const uint8_t> file,const Rooms& rooms,bool special_content){
    if(std::none_of(rooms.begin(),rooms.end(),[](const auto& room){return room.entry_flags || room.route_count;}))return;
    require(special_content,"Edited navigation requires custom runtime");
    Source source(original);const unsigned root=u32(file,8);
    for(const auto& room:rooms){
        if(room.entry_flags)require(source.within_terrain(room.source,{room.entry_x,room.entry_y}),"Entry must be inside an existing terrain page");
        std::map<std::array<uint32_t,3>,std::vector<unsigned>> groups;
        for(unsigned i=0;i<room.count;i++){size_t p=room.offset+size_t(i)*room.stride;groups[{u32(file,p),u32(file,p+4),u32(file,p+8)}].push_back(i);}
        for(unsigned n=0;n<room.route_count;n++){
            size_t r=room.route_offset+size_t(n)*20,p=room.offset+size_t(u32(file,r))*room.stride;
            require(source.passage(u16(file,p),true).has_value(),"Only supported native transport pots are editable");
            require(groups.at({u32(file,p),u32(file,p+4),u32(file,p+8)}).size()==1,"Overlapping passages cannot have ambiguous arrival settings");
            Point point=u32(file,r+8)?Point{u32(file,r+12),u32(file,r+16)}:source.effective_entry(root,u32(file,r+4),rooms);
            require(source.within_terrain(u32(file,r+4),point),"Arrival must be inside an existing terrain page");
        }
    }
    // Also validate native pots which inherit an edited destination entry.
    // Their packed parameters must match an original source-room instance.
    (void)plans(original,file,rooms);
}
}
