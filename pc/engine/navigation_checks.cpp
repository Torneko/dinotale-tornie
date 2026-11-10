#include "custom_passages.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <deque>
#include <cstring>
using workshop::require;
namespace {
std::deque<uint32_t> carry;std::vector<unsigned> calls;
uint32_t queue_address=0x800F9000,expected_player=0x800CB000;
}
extern "C" void func_8005D29C(uint8_t* rdram,recomp_context* c){
    require(uint32_t(c->r4)==queue_address,"Wrong native carry queue");
    calls.push_back(1);c->r2=carry.empty()?0:gpr(int32_t(carry.front()));
    if(!carry.empty()){auto a=carry.front();carry.pop_front();MEM_B(0x17D,int32_t(a))=0;}
}
extern "C" void func_8002BEB0(uint8_t*,recomp_context* c){require(uint32_t(c->r4)==expected_player,"Collision restore touched a different actor");calls.push_back(2);}
extern "C" void func_800443EC(uint8_t* rdram,recomp_context* c){require(uint32_t(c->r4)==expected_player,"Activation touched a different actor");calls.push_back(3);MEM_B(0x17D,c->r4)=4;}
extern "C" void func_80040C0C(uint8_t* rdram,recomp_context* c){require(uint32_t(c->r4)==expected_player,"Control restore touched a different actor");calls.push_back(4);MEM_W(0x14,MEM_W(0x1F4,c->r4))=1;}
extern "C" void func_80047AB8(uint8_t* rdram,recomp_context* c){require(uint32_t(c->r4)==expected_player+0x84 && uint32_t(c->r5)==expected_player,"Wrong native matrix update");calls.push_back(5);for(unsigned n=0;n<12;n+=4)MEM_W(n,c->r4)=MEM_W(0x3C+n,c->r5);}
std::vector<uint8_t> read(const char* path){std::ifstream f(path,std::ios::binary);require(bool(f),"Cannot read navigation fixture");return {std::istreambuf_iterator<char>(f),{}};}
void memory_checks(){
    using namespace workshop;using namespace passages;
    std::vector<uint8_t> memory(0x800000,0xA5);auto* rdram=memory.data();
    auto word=[&](uint32_t p,uint32_t v){MEM_W(0,int32_t(p))=int32_t(v);};
    auto live=[&](uint32_t a,uint16_t kind,uint32_t data){word(a,0x19971110);MEM_H(12,int32_t(a))=kind;word(a+0x1F4,data);MEM_B(0x165,int32_t(a))=1;MEM_H(0x184,int32_t(a))=0;word(a+0x180,0);};
    const uint32_t pot=0x800CA000,sibling=0x800CE000,private_data=0x80100000,sibling_data=0x80101000,native=0x80110000,level_info=0x80120000,egg=0x800CC000,extra=0x800CD000;
    const uint32_t px=std::bit_cast<uint32_t>(1176.0f),py=std::bit_cast<uint32_t>(824.0f);
    std::array<uint8_t,24> original{0x44,0xCA,0x20,0x00,81,0,0,0,0,0,0,0,0x43,0x70,0,0,0x43,0xFA,0,0,7,0,1,0};
    configure({},{});routes.push_back({43,9,81,0x42082000,px,py,{std::bit_cast<uint32_t>(584.0f),std::bit_cast<uint32_t>(824.0f)},original});
    route_keys.emplace(Key{43,0x42082000,px,py},0);
    live(pot,0x4208,private_data);live(sibling,0x4208,sibling_data);live(expected_player,0x4001,0x80130000);live(egg,0x4002,0x80140000);live(extra,0x400A,0x80150000);
    MEM_H(0x184,int32_t(expected_player))=1;word(expected_player+0x188,egg);word(egg+0x180,expected_player);MEM_B(0x17C,int32_t(egg))=0;
    const gpr native_base=gpr(int32_t(native)),slab_base=gpr(int32_t(slab));
    for(unsigned n=0;n<24;n++)MEM_B(n,native_base)=original[n];
    word(private_data+0x260,native);word(sibling_data+0x260,native);
    word(0x800F8828,expected_player);word(0x800F8770,queue_address);word(0x800F89D0,12);MEM_H(0,int32_t(0x800F897E))=7;
    MEM_B(1,int32_t(level_info))=81;word(expected_player+0x44,0x43FA2B85);
    recomp_context c{};c.r4=gpr(int32_t(pot));c.r5=gpr(int32_t(0x42082000));c.r6=px;c.r7=py;
    enter_room(43);created(&c,43);auto before=memory;prepare(rdram,pot);
    require(read_word(rdram,private_data+0x260)==slab,"Edited pot did not get private route");
    require(read_word(rdram,sibling_data+0x260)==native,"Ordinary sibling route changed");
    require(read_word(rdram,slab)==UINT32_MAX && MEM_BU(4,int32_t(slab))==81,"Private route target/destination is invalid");
    auto origin=navigation::camera_origin(routes.front().arrival);require(read_word(rdram,slab+8)==origin[0] && read_word(rdram,slab+12)==origin[1],"Private route did not use camera origin");
    for(unsigned n:{size_t(5),size_t(6),size_t(7),size_t(16),size_t(17),size_t(18),size_t(19),size_t(20),size_t(21),size_t(22),size_t(23)})require(MEM_BU(n,slab_base)==original[n],"Native route Z/stage/flags changed");
    for(size_t n=0;n<memory.size();n++)if(!((n>=0x7E0000 && n<0x7E0018) || (n>=0x100260 && n<0x100264)))require(memory[n]==before[n],"Private route wrote outside its owned fields");
    auto prepared=memory;prepare(rdram,pot);require(memory==prepared,"Repeated prepare changed an installed route");
    c.r4=gpr(int32_t(slab));departing(rdram,&c);require(pending && pending->epoch==1,"Departure epoch missing");
    carry={expected_player,extra};calls.clear();frame(rdram,&c);require(calls.empty(),"Carry restored before destination initialization");
    enter_room(120);require(!pending,"Different-room load retained a stale epoch");
    enter_room(43);departing(rdram,&c);require(pending && pending->epoch==2,"Second departure missing");
    const auto child_before=child_signature(rdram,expected_player);
    enter_room(81);c.r4=gpr(int32_t(level_info));room_ready(rdram,&c);require(pending && pending->phase==Phase::ready,"Destination did not become ready");
    c.r4=0x1234;c.r5=0x5678;c.r6=0x9ABC;c.r7=0xDEF0;auto saved=c;frame(rdram,&c);
    require(std::memcmp(&saved,&c,sizeof(c))==0,"Native carry restoration leaked registers");
    require(!pending && completed_epoch==2 && carry.empty(),"Carry did not commit exactly one epoch");
    require(calls==std::vector<unsigned>{1,2,3,4,5,1,1},"Native activation/reset sequence is incomplete");
    require(read_word(rdram,expected_player+0x84)==routes.front().arrival[0] && read_word(rdram,expected_player+0x88)==routes.front().arrival[1],"Restored player world XY is incorrect");
    require(read_word(rdram,expected_player+0x8C)==0x43FA2B85 && MEM_BU(0x17D,int32_t(expected_player))==4 && MEM_BU(0x165,int32_t(expected_player))==0 && read_word(rdram,expected_player+0x130)==0x800233B4,"Restored player is not active and visible");
    require(MEM_HU(0x184,int32_t(expected_player))==1 && read_word(rdram,egg+0x180)==expected_player && child_signature(rdram,expected_player)==child_before,"Arrival changed attached egg identity or parent links");
    require(MEM_HU(0,int32_t(0x800F897E))==7 && read_word(rdram,0x800F89D0)==12,"Arrival changed health or eaten fruit count");
    auto committed=memory;frame(rdram,&c);require(memory==committed && calls.size()==7,"Committed epoch ran twice");
    enter_room(43);c.r4=gpr(int32_t(slab));departing(rdram,&c);require(pending && pending->epoch==3,"Revisited pot cannot depart again");
    c.r4=gpr(int32_t(native));departing(rdram,&c);require(!pending,"Native departure did not cancel a failed custom epoch");
    destroy(rdram,pot);require(bindings.empty() && read_word(rdram,private_data+0x260)==native,"Actor destruction did not restore its native private pointer");
    c.r4=gpr(int32_t(slab));departing(rdram,&c);require(!pending,"Released actor left a valid stale route");
    configure({},{});auto clean=memory;frame(rdram,&c);prepare(rdram,pot);require(memory==clean,"Legacy/empty payload modified memory");
    for(float value:{-511.9f,-255.9f,-0.0f,0.0f,192.0f,240.0f,255.99f,256.0f,512.0f}){
        auto bits=std::bit_cast<uint32_t>(value);auto point=navigation::camera_origin({bits,bits});
        int bucket=(int(value)/256)*256;
        require(std::bit_cast<float>(point[0])==float(bucket?bucket-64:0) && std::bit_cast<float>(point[1])==float(bucket?bucket+16:0),"Native camera bucket rounding mismatch");
    }
}
int main(int argc,char** argv){
    try{
        memory_checks();
        if(argc==3){
            auto rom=read(argv[1]),file=read(argv[2]);workshop::validate_payload(file);auto rooms=workshop::payload_rooms(file);
            workshop::navigation::validate(rom,file,rooms,true);
            auto plans=workshop::navigation::plans(rom,file,rooms);
            require(!plans.empty(),"Navigation fixture has no effective routes");
            require(std::any_of(rooms.begin(),rooms.end(),[](const auto& r){return r.entry_flags || r.route_count;}),"Navigation fixture has no edits");
            for(const auto& plan:plans)require(workshop::vanilla_room(workshop::u32(file,8),plan.destination),"Effective route escaped source root");
            std::cout<<"Native source routes: "<<plans.size()<<" isolated instance plans.\n";
        }else require(argc==1,"Usage: NavigationChecks [original.z64 level.yslevel]");
        std::cout<<"Navigation memory: private 24-byte copies, exact bindings, sibling isolation, native activation order, once-only epochs, revisit/cancel, cleanup, no health/fruit writes and camera buckets PASS.\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
