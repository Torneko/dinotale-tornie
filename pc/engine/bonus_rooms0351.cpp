#include "bonus_rooms0351.hpp"
#include "workshop_data.hpp"
#include <iomanip>
#include <sstream>
namespace workshop::bonus_rooms0351 {
namespace {
std::string sha256(std::span<const uint8_t> input) {
 const uint32_t k[64]={0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
 auto r=[](uint32_t x,unsigned n){return (x>>n)|(x<<(32-n));};
 uint32_t h[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
 std::vector<uint8_t> data(input.begin(),input.end());uint64_t bits=uint64_t(data.size())*8;data.push_back(0x80);while(data.size()%64!=56)data.push_back(0);for(int i=7;i>=0;i--)data.push_back(uint8_t(bits>>(8*i)));
 for(size_t p=0;p<data.size();p+=64){uint32_t w[64];for(unsigned i=0;i<16;i++)w[i]=(uint32_t(data[p+i*4])<<24)|(uint32_t(data[p+i*4+1])<<16)|(uint32_t(data[p+i*4+2])<<8)|data[p+i*4+3];for(unsigned i=16;i<64;i++){auto s0=r(w[i-15],7)^r(w[i-15],18)^(w[i-15]>>3);auto s1=r(w[i-2],17)^r(w[i-2],19)^(w[i-2]>>10);w[i]=w[i-16]+s0+w[i-7]+s1;}uint32_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],v=h[7];for(unsigned i=0;i<64;i++){auto s1=r(e,6)^r(e,11)^r(e,25),ch=(e&f)^(~e&g),t1=v+s1+ch+k[i]+w[i],s0=r(a,2)^r(a,13)^r(a,22),maj=(a&b)^(a&c)^(b&c),t2=s0+maj;v=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;}h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=v;}
 std::ostringstream out;out<<std::hex<<std::setfill('0');for(auto x:h)out<<std::setw(8)<<x;return out.str();
}
void range(std::span<const uint8_t> rom,size_t offset,size_t bytes,const char* why){require(offset<=rom.size()&&bytes<=rom.size()-offset,why);}
bool nonzero(Point point){return std::bit_cast<float>(point[0])!=0||std::bit_cast<float>(point[1])!=0;}
Placement placement(const navigation::Object& item,size_t start,unsigned n){return {n,start+size_t(n)*12,item.packed,{item.x,item.y}};}
Passage route(uint16_t kind,const navigation::Passage& source,const navigation::Source& native){return {kind,source.destination,source.target,source.resource_a,source.record,native.passage(kind,true).has_value(),native.native_payload(source)};}
std::optional<Placement> raw_target(const Passage& passage,const Catalog& catalog){
    if(passage.target_packed==UINT32_MAX)return {};
    const auto* room=catalog.find(passage.destination);if(!room)return {};
    std::optional<Placement> found;
    for(const auto& item:room->placements)if((item.packed&0xFFFF03FFu)==(passage.target_packed&0xFFFF03FFu)){
        if(found)return {};found=item;
    }
    return found;
}
std::optional<Anchor> target(const Passage& passage,const Catalog& catalog,const navigation::Source& native,AnchorKind kind){
    const auto found=raw_target(passage,catalog);
    if(!found||!nonzero(found->point)||!native.within_terrain(passage.destination,found->point))return {};
    const auto actual=native.native_arrival(passage.kind,passage.destination);
    if(!actual||*actual!=found->point)return {};
    return Anchor{kind,passage.destination,*found,{},passage.kind};
}
}
const Room* Catalog::find(unsigned room)const noexcept{auto found=std::find_if(rooms.begin(),rooms.end(),[room](const Room& item){return item.id==room;});return found==rooms.end()?nullptr:&*found;}
static Catalog parse_structure(std::span<const uint8_t> original){
    require(original.size()==0x1000000,"Bonus requires exact16MiB normalized original USA ROM");
    require(u32(original,0)==0x80371240,"Bonus ROM byte order/magic invalid");
    navigation::Source native(original);Catalog catalog;catalog.rooms.reserve(113);std::set<unsigned> included;
    for(unsigned course=0;course<24;++course){
        auto& c=catalog.courses[course];c.index=course;c.world=course/4+1;c.root=vanilla_sources[course];auto& world=catalog.worlds[course/4];world.number=c.world;world.courses.push_back(course);
        for(unsigned n=0;n<4;++n){c.warp_destinations[n]=original[0xA83D4+course*4+n];require(vanilla_room(c.root,c.warp_destinations[n]),"Bonus checkpoint destination outside its source course");}
        for(unsigned id:vanilla_rooms[course])if(id<177){
            require(included.insert(id).second,"Bonus original room identity duplicated");Room room;room.id=id;room.course=course;room.world=c.world;room.root=c.root;c.rooms.push_back(id);world.rooms.push_back(id);
            room.native_init=u32(original,0xA84A0+size_t(id)*8);room.header_rom=native.header(id);range(original,room.header_rom,0x118,"Bonus room descriptor truncated");
            const size_t indirect=native.bank(u32(original,room.header_rom+0x5C));room.meta_rom=native.bank(u32(original,indirect));range(original,room.meta_rom,8,"Bonus room metadata truncated");
            const unsigned count=u16(original,room.meta_rom);require(count>0&&count<=149,"Bonus room placement count invalid");room.placements_rom=native.bank(u32(original,room.meta_rom+4));range(original,room.placements_rom,size_t(count)*12,"Bonus room placements truncated");
            const auto objects=native.objects(id);require(objects.size()==count,"Bonus room metadata disagreement");unsigned spawns=0;
            for(unsigned n=0;n<count;++n){auto item=placement(objects[n],room.placements_rom,n);require(navigation::coordinate(item.point[0])&&navigation::coordinate(item.point[1]),"Bonus original placement coordinate invalid");room.placements.push_back(item);const unsigned kind=item.packed>>16;
                if(kind==0x4001){room.spawn=item;++spawns;}
                if(kind>=0x4507&&kind<=0x450A)room.checkpoints.push_back({kind-0x4507+1,c.warp_destinations[kind-0x4507],item});
            }
            require(spawns==1,"Bonus room must preserve one native Yoshi placement");catalog.rooms.push_back(std::move(room));
        }
    }
    require(catalog.rooms.size()==113,"Bonus original room coverage invalid");
    for(auto& room:catalog.rooms){
        if(nonzero(room.spawn.point)&&native.within_terrain(room.id,room.spawn.point))room.entry=Anchor{AnchorKind::PlayerSpawn,room.id,room.spawn,{},{}};
        for(const auto& item:room.placements){const auto kind=uint16_t(item.packed>>16);const auto source=native.passage(kind);if(!source)continue;
            auto passage=route(kind,*source,native);auto anchor=target(passage,catalog,native,AnchorKind::PlacedIncoming);if(anchor)anchor->via_room=room.id;
            room.passages.push_back({item,passage,raw_target(passage,catalog),anchor});
        }
    }
    // Preserve Source::effective_entry ordering inside each complete original
    // course, including rooms not present in an older editor project.
    for(auto& room:catalog.rooms)if(!room.entry){
        const auto& course=catalog.courses[room.course];
        for(unsigned origin:course.rooms){const auto* source=catalog.find(origin);for(const auto& edge:source->passages)if(edge.native.destination==room.id&&edge.target){room.entry=edge.target;break;}if(room.entry)break;}
    }
    // Global resource records are useful research candidates for dynamically
    // spawned pots. No such candidate is treated as a placed/usable entrance.
    std::set<uint16_t> kinds;
    for(unsigned i=0;i<1582;++i)kinds.insert(u16(original,0xA1C90+size_t(i)*12));
    for(auto& room:catalog.rooms)if(!room.entry)for(uint16_t kind:kinds){const auto source=native.passage(kind,true);if(!source||source->destination!=room.id)continue;
        auto passage=route(kind,*source,native);auto anchor=target(passage,catalog,native,AnchorKind::UnplacedNativeTarget);if(anchor)room.candidates.push_back({passage,*anchor});
    }
    return catalog;
}
Catalog read(std::span<const uint8_t> original){
    require(original.size()==0x1000000,"Bonus requires exact16MiB normalized original USA ROM");
    require(u32(original,0)==0x80371240,"Bonus ROM byte order/magic invalid");
    // Reject inexact external images before resource lookup or decompression.
    const auto identity=sha256(original);require(identity==original_sha256,"Bonus original USA ROM SHA256 mismatch");
    auto catalog=parse_structure(original);catalog.source_sha256=identity;return catalog;
}

}
