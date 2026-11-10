#include "../boss_rest_data0240.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <stdexcept>
namespace b=workshop::boss_rest_data0240;
static unsigned checks=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
static size_t bank(const std::vector<uint8_t>& rom,uint32_t p){return workshop::u32(rom,0xA79D4+((p>>24)&15)*4)+(p&0xffffff);}
template<class F>void rejected(F f,const char* why){bool caught=false;try{f();}catch(const std::exception&){caught=true;}need(caught,why);}
int main(int argc,char**argv){try{
    need(argc==2,"Original ROM argument required");std::ifstream f(argv[1],std::ios::binary);need(bool(f),"ROM open");std::vector<uint8_t> original((std::istreambuf_iterator<char>(f)),{});need(original.size()==workshop::rom_size,"ROM size");
    const auto before=original;const auto result=b::apply(original);need(original==before,"Caller ROM unchanged");need(result.size()==before.size(),"ROM size preserved");
    const size_t source_entry=0xA84A0+b::source_room*8;need(workshop::u32(result,source_entry)==workshop::u32(before,source_entry),"Native room initializer preserved");
    const size_t source_header=bank(before,workshop::u32(before,source_entry+4));const size_t cloned_header=bank(result,workshop::u32(result,source_entry+4));need(cloned_header>=b::clone_allocation,"Relocated room descriptor");
    need(std::equal(b::native_descriptor.begin(),b::native_descriptor.end(),before.begin()+source_header),"Fingerprinted native descriptor is immutable and remains exact ROM");
    need(std::equal(before.begin()+source_header,before.begin()+source_header+0x118,result.begin()+source_header),"Original ROM descriptor bytes remain untouched");
    need(workshop::u32(result,cloned_header+0x28)==std::bit_cast<uint32_t>(0.f) && workshop::u32(result,cloned_header+0x2C)==std::bit_cast<uint32_t>(256.f),"Cloned camera frames floor entry at0/256");
    need(workshop::u32(b::rest_descriptor,0x28)==workshop::u32(result,cloned_header+0x28) && workshop::u32(b::rest_descriptor,0x2C)==workshop::u32(result,cloned_header+0x2C),"Runtime camera payload can use the same cloned descriptor");
    need(b::entry_y-256.f==176.f && b::checkpoint_y-256.f==192.f && b::entry_x<b::checkpoint_x,"Camera includes entry and next checkpoint together inside the native240pixel viewport");

    bool descriptor_match=true;for(size_t n=0;n<0x118;n++)if(!(n>=0x5C&&n<0x60) && !(n>=0xCA&&n<0xDC) && !(n>=0x28&&n<0x30))descriptor_match&=result[cloned_header+n]==before[source_header+n];need(descriptor_match,"Native descriptor clone changes only camera, placement pointer and resource preloads");
    need(workshop::u16(result,cloned_header+0xCA)==0x4708 && workshop::u16(result,cloned_header+0xCC)==0x4086 && workshop::u16(result,cloned_header+0xCE)==0x40C5 && workshop::u16(result,cloned_header+0xD0)==0x4507 && workshop::u16(result,cloned_header+0xD2)==0x4508 && workshop::u16(result,cloned_header+0xD4)==0x4509 && workshop::u16(result,cloned_header+0xD6)==0x450A && workshop::u16(result,cloned_header+0xD8)==0x46F4 && workshop::u16(result,cloned_header+0xDA)==0xFFFF,"Native tree/block/red-heart/checkpoint resources retain order, followed by resident46F4 effect and terminator");
    std::set<uint16_t> preloads;unsigned tree_count=0;for(size_t n=0x60;n<0x100;n+=2){auto id=workshop::u16(result,cloned_header+n);if(id==0xFFFF)break;preloads.insert(id);tree_count+=id==0x4708;}
    for(uint16_t k=0x4507;k<=0x450A;++k)need(preloads.contains(k),"All four genuine checkpoint resources preloaded");
    need(preloads.contains(0x46F4)&&0xDA+2<=0x100,"Native checkpoint effect fits inside the existing descriptor preload area");
    need(tree_count==1,"One tree preload");need(preloads.contains(0x4086),"Native egg block preloaded");need(preloads.contains(0x40C5),"Native red heart available without static placement");for(uint16_t id=0x4030;id<=0x4035;id++)need(preloads.contains(id),"Native fruit resources preloaded");
    const size_t indirect=bank(result,workshop::u32(result,cloned_header+0x5C));const size_t meta=bank(result,workshop::u32(result,indirect));need(meta+8==indirect,"Native metadata self-pointer");need(workshop::u16(result,meta)==b::object_count,"Native rest geometry plus complete floor");
    const size_t objects=bank(result,workshop::u32(result,meta+4));need(objects==b::clone_allocation,"Object clone base");need(std::equal(b::rest_objects.begin(),b::rest_objects.end(),result.begin()+objects),"Generated objects exact");
    std::set<uint16_t> kinds;for(unsigned n=0;n<b::object_count;n++){size_t p=objects+n*12;auto id=workshop::u16(result,p);kinds.insert(id);float x=std::bit_cast<float>(workshop::u32(result,p+4)),y=std::bit_cast<float>(workshop::u32(result,p+8));if(id==0x4001 || id==0x44CA)need(x==120 && y==432,"Cold player and warm route arrive at the same floor entry");if(id==0x4274)need(x==712 && y==408,"Original safe flower pot placement");if(id==0x4708)need(x==360 && y==408,"Verified tree repositioned to rest center");if(id==0x4086)need(x==568 && y==392,"Verified egg block at accessible height");}
    need(b::checkpoint_x==160.f && b::checkpoint_y==448.f,"Checkpoint source stays at160/448");
    need(b::checkpoint_index==30 && b::object_count==31 && workshop::u32(result,objects+b::checkpoint_index*12)==b::checkpoint_packed,"One genuine checkpoint source placement at its reserved index");
    need(std::bit_cast<float>(workshop::u32(result,objects+b::checkpoint_index*12+4))==b::checkpoint_x && std::bit_cast<float>(workshop::u32(result,objects+b::checkpoint_index*12+8))==b::checkpoint_y,"Checkpoint uses safe floor coordinates");
    need(kinds==std::set<uint16_t>{0x4001,0x44CA,0x4274,0x4708,0x4086,0x4507,0x4591,0x4592,0x4593,0x8033,0x806F},"Only verified terrain, collision writers, tree, entry and pot");
    //8033 alone is empty under the entry. Native4591..4593 actors patch the
    //collision tiles; preserve every original record and require a solid row.
    const size_t original_meta=bank(before,workshop::u32(before,bank(before,workshop::u32(before,source_header+0x5C))));
    const size_t original_objects=bank(before,workshop::u32(before,original_meta+4));unsigned preserved_geometry=0;
    for(unsigned i=0;i<workshop::u16(before,original_meta);i++){
        const size_t source=original_objects+i*12;const auto id=workshop::u16(before,source);if(id<0x4591 || id>0x4593)continue;
        bool found=false;for(unsigned n=0;n<b::object_count;n++)found|=std::equal(before.begin()+source,before.begin()+source+12,result.begin()+objects+n*12);
        need(found,"Original collision-writer placement preserved byte-exact");preserved_geometry++;
    }
    need(preserved_geometry==12,"All twelve native geometry actors retained");
    std::array<bool,48> supported{};
    for(unsigned n=0;n<b::object_count;n++){
        const size_t p=objects+n*12;if(workshop::u16(result,p)!=0x4592 || std::bit_cast<float>(workshop::u32(result,p+8))!=472.0f)continue;
        float x=std::bit_cast<float>(workshop::u32(result,p+4));int first=int(std::floor((x-32.0f)/16.0f));
        for(int tile=first;tile<first+5;tile++)if(tile>=0 && tile<int(supported.size()))supported[tile]=true;
    }
    need(std::all_of(supported.begin(),supported.end(),[](bool v){return v;}),"Solid4592 collision floor spans the complete768pixel room");
    for(float x:{b::entry_x,b::tree_x,b::pot_x,b::egg_block_x,b::checkpoint_x})need(supported[unsigned(x/16.0f)],"Entry/tree/pot has real native floor support");
    need(b::floor_tile_row==29 && b::floor_patch_width==5 && b::floor_patch_origin_x==32,"Collision metadata matches native4592 shape");
    const size_t arena_entry=0xA84A0+b::arena_source_room*8;
    const size_t original_arena_header=bank(before,workshop::u32(before,arena_entry+4));
    const size_t arena_header=bank(result,workshop::u32(result,arena_entry+4));
    need(arena_header==b::arena_clone_allocation,"Arena162 descriptor cloned to reserved FF1000");
    need(workshop::u32(before,arena_entry)==workshop::u32(result,arena_entry),"Arena native initializer unchanged");
    bool arena_same=true;for(size_t n=0;n<0x118;n++)if(!(n>=0xD6 && n<0xDA))arena_same&=result[arena_header+n]==before[original_arena_header+n];
    need(arena_same,"Arena descriptor preserved except additional pot preload");
    need(workshop::u16(result,arena_header+0xD6)==0x4273 && workshop::u16(result,arena_header+0xD8)==0xFFFF,"Arena4273 preload plus terminator");
    const size_t arena_indirect=bank(result,workshop::u32(result,arena_header+0x5C));
    const size_t arena_meta=bank(result,workshop::u32(result,arena_indirect));
    need(arena_meta==bank(before,workshop::u32(before,bank(before,workshop::u32(before,original_arena_header+0x5C)))),"Arena original metadata pointer retained");
    need(workshop::u16(result,arena_meta)==24,"All24 original arena placements retained");
    const size_t arena_objects=bank(result,workshop::u32(result,arena_meta+4));unsigned candy_count=0,pot_count=0;
    for(unsigned i=0;i<24;i++){const auto kind=workshop::u16(result,arena_objects+i*12);candy_count+=kind==0x455A;pot_count+=kind==0x4273;}
    need(std::equal(before.begin()+arena_meta,before.begin()+arena_meta+12,result.begin()+arena_meta),"Arena metadata bytes unchanged");
    need(std::equal(before.begin()+arena_objects,before.begin()+arena_objects+24*12,result.begin()+arena_objects),"Arena object bytes unchanged");
    need(candy_count==1 && pot_count==0,"One original yellow boss; no premature exit pot");
    std::array<bool,48> arena_support{};
    for(unsigned i=0;i<24;i++){
        const size_t p=arena_objects+i*12;if(workshop::u16(result,p)!=0x4592 || std::bit_cast<float>(workshop::u32(result,p+8))!=472.0f)continue;
        const float x=std::bit_cast<float>(workshop::u32(result,p+4));const int first=int(std::floor((x-32.0f)/16.0f));
        for(int tile=first;tile<first+5;tile++)if(tile>=0 && tile<int(arena_support.size()))arena_support[tile]=true;
    }
    need(std::all_of(arena_support.begin(),arena_support.end(),[](bool v){return v;}),"Native arena floor supports every candidate pot spawn");
    need(b::arena_pot_offset_x==80.0f && b::arena_pot_min_x==40.0f && b::arena_pot_max_x==724.0f,"Near-player arena pot policy");
    need(b::arena_pot_min_x-32>=0 && b::arena_pot_max_x+32<768,"Pot bounds remain inside arena floor");
    for(float player_x:{0.0f,56.0f,248.0f,633.0f,760.0f}){
        const float x=std::clamp(player_x+b::arena_pot_offset_x,b::arena_pot_min_x,b::arena_pot_max_x);
        need(arena_support[unsigned(x/16.0f)] && std::abs(x-player_x)<=80,"Candidate near-player pot has floor and remains within80pixels");
    }
    need(b::arena_pot_kind==0x4273 && b::arena_pot_packed==0x42730000 && b::arena_pot_z_bits==0x43FA3AE1,"Verified native arena pot creation constants");
    size_t route_resource=0;for(unsigned i=0;i<1582;i++){const size_t at=0xA1C90+i*12;if(workshop::u16(before,at)==0x4273){route_resource=bank(before,workshop::u32(before,at+8));break;}}
    need(route_resource!=0,"Original4273 resource found");const size_t route=bank(before,workshop::u32(before,route_resource+0x34));
    need(std::equal(b::native_arena_pot_route.begin(),b::native_arena_pot_route.end(),before.begin()+route+8),"Exact native4273 route payload retained");
    need(b::native_arena_pot_route[4]==83 && b::native_arena_pot_route[20]==7,"Postvictory pot naturally targets realrest83");
    bool isolated=true;for(size_t n=0;n<result.size();n++)if(result[n]!=before[n] && !(n>=source_entry+4&&n<source_entry+8) && !(n>=b::clone_allocation&&n<cloned_header+0x118) && !(n>=arena_entry+4&&n<arena_entry+8) && !(n>=b::arena_clone_allocation&&n<arena_header+0x118)){isolated=false;break;}need(isolated,"Changes restricted to room83/162 pointers and two reserved clone slabs");
    need(workshop::u32(result,0xA84A0+52*8+4)==workshop::u32(before,0xA84A0+52*8+4),"Boss root remap left to caller");
    rejected([&]{b::apply(std::vector<uint8_t>(32));},"Truncated ROM rejected");auto occupied=original;occupied[b::clone_allocation]=0;rejected([&]{b::apply(occupied);},"Occupied slab rejected");auto arena_occupied=original;arena_occupied[b::arena_clone_allocation]=0;rejected([&]{b::apply(arena_occupied);},"Occupied arena slab rejected");rejected([&]{b::apply(result);},"Repeat application rejects overwrite");
    std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"treeNative\":true,\"gameLaunched\":false,\"treeRulesValidated\":false}\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
