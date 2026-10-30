#include "hybrid_mode.hpp"
#include "hybrid_maskass_data.hpp"
#include "hybrid_rewards.hpp"
#include "fruit_visits.hpp"
#include "json/json.hpp"
#include <bit>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>
using namespace workshop::hybrid_mode;
static void check(bool b,const char* m){if(!b){std::cerr<<m<<'\n';std::exit(1);}}
extern "C" void func_800692D4(uint8_t*,recomp_context*);
extern "C" void workshop_hybrid_recolor(uint8_t* rdram,recomp_context* c){if(c->r2==1)recolored(rdram,uint32_t(c->r16));}
extern "C" void func_8004590C(uint8_t* rdram,recomp_context* c){MEM_W(8,MEM_W(0x16C,c->r4))=c->r5;}
extern "C" void func_80067BBC(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_80067818(uint8_t*,recomp_context*){}
int main(int argc,char** argv){
    check(argc==2,"Expected disposable folder");auto folder=std::filesystem::path(argv[1]);std::filesystem::create_directories(folder);
    check(entitled(0xFFFFFF,true) && !entitled(0x7FFFFF,true) && !entitled(0xFFFFFF,false),"Hybrid launch conditions changed");
    for(unsigned f=0;f<4;f++){
        unsigned changed=0;std::array<unsigned,3> choices{};
        for(unsigned ticket=0;ticket<60;ticket++){
            auto id=workshop::fruit_mix::replacement(0x4030+f,ticket,45);
            if(id!=0x4030+f){changed++;bool compatible=false;for(unsigned i=0;i<3;i++)if(id==workshop::fruit_mix::choices[f][i]){compatible=true;choices[i]++;}check(compatible,"75% produced an incompatible fruit");}
            check(workshop::fruit_mix::replacement(0x4034,ticket,45)==0x4034,"Hybrid mode changed melons");
        }
        check(changed==45 && choices==std::array<unsigned,3>{15,15,15},"Hybrid replacement is not 75%, equally divided among compatible fruit");
    }
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
    const auto meta=gpr(int32_t(0x80010000)),objects=gpr(int32_t(0x80011000)),actor=gpr(int32_t(0x80012000)),payload=gpr(int32_t(0x80020000)),buffer=gpr(int32_t(0x80021000));
    MEM_H(0,meta)=2;MEM_W(4,meta)=uint32_t(objects);MEM_H(0,objects)=0x4001;MEM_H(16,objects)=0x4030;
    workshop::fruit_mix::Visits visits;visits.apply(rdram,uint32_t(meta),43,[]{return 44u;},45);
    auto first=MEM_HU(16,objects);check(first>=0x4F00,"Hybrid placement was not converted");
    for(unsigned i=0;i<20;i++){MEM_H(16,objects)=0x4030;visits.apply(rdram,uint32_t(meta),43,[]{return 59u;},45);check(MEM_HU(16,objects)==first,"Room revisit rerolled hybrid fruit");}
    auto path=folder/"records.json";initialize(path);activate(true);initialize_ram(rdram);enter(rdram,uint32_t(meta),43);
    const auto resource=gpr(int32_t(0x80013000)),drawing=gpr(int32_t(0x80014000)),properties=gpr(int32_t(0x80015000)),wrapper=gpr(int32_t(0x80016000)),camera=gpr(int32_t(0x80017000));
    MEM_W(0x168,actor)=uint32_t(resource);MEM_W(0x88,resource)=0x80030000;MEM_W(0x16C,actor)=uint32_t(drawing);MEM_W(0x174,actor)=uint32_t(wrapper);MEM_W(0,wrapper)=uint32_t(properties);
    MEM_W(0,int32_t(0x800F8318))=uint32_t(camera);MEM_B(0xF8,camera)=1;MEM_W(0x34,actor)=100;MEM_W(0x38,actor)=100;
    unsigned cycles=0;
    for(auto id:maskass_ids){
        recomp_context c{};c.r4=actor;c.r5=uint32_t(id)<<16;c.r6=0x43200000;c.r7=0x43C80000;c.r29=gpr(int32_t(0x80040000));MEM_W(0x10,c.r29)=0;
        created(rdram,&c,43);MEM_H(12,actor)=id;MEM_W(0x1EC,actor)=0;const auto start=*enemy_variant(uint32_t(actor));
        for(unsigned i=0;i<12;i++){
            const auto sp=c.r29;c.r4=actor;c.r31=0x12345678;func_800692D4(rdram,&c);
            check(c.r2==1 && c.r29==sp && c.r31==0x12345678,"Ground-pound continuation damaged native return or stack");
            const auto expected=(start+i+1)%6;check(enemy_variant(uint32_t(actor))==expected,"Maskass did not cycle all six colors");
            check(uint32_t(MEM_W(8,drawing))==palette_base+expected*512,"Recolored Maskass uses the wrong palette");cycles++;
        }
        const auto before=*enemy_variant(uint32_t(actor));MEM_B(0xF8,camera)=0;c.r4=actor;func_800692D4(rdram,&c);
        check(c.r2==0 && enemy_variant(uint32_t(actor))==before,"Maskass recolored without a ground pound");MEM_B(0xF8,camera)=1;
    }
    recomp_context c{};c.r4=actor;c.r5=0x406F0000;created(rdram,&c,43);check(!enemy_variant(uint32_t(actor)),"Recycled ordinary actor inherited a custom enemy reward");
    MEM_H(12,actor)=0x402C;MEM_H(14,actor)=0x200;paint(rdram,uint32_t(actor));check(MEM_HU(0x144,actor)==lucky_type()+3 && lucky_type()>=6 && lucky_type()<=11,"Lucky icon does not match its hybrid fruit");
    // Score table only: preserve access/checkpoint bytes and ordinary records.
    std::array<uint8_t,1018> original{};for(unsigned i=0;i<1018;i++){original[i]=uint8_t(i%200);MEM_B(i,payload)=original[i];}
    records_loaded(rdram,uint32_t(payload));check(MEM_BU(0,payload)==original[0] && MEM_BU(1017,payload)==original[1017],"Hybrid records changed progression header/checkpoint");
    for(unsigned i=4;i<1004;i++)check(MEM_BU(i,payload)==0,"Hybrid records did not start separately from ordinary scores");
    MEM_B(4,payload)=12;MEM_B(5,payload)=34;
    for(unsigned i=0;i<1018;i++)MEM_B(i,buffer)=MEM_BU(i,payload);
    check(records_saving(rdram,uint32_t(payload),uint32_t(buffer)),"Hybrid records save failed");
    for(unsigned i=0;i<1018;i++)check(MEM_BU(i,buffer)==original[i],"Ordinary EEPROM records or progress would have been overwritten");
    check(MEM_BU(4,payload)==12 && MEM_BU(5,payload)==34,"Saving ordinary EEPROM destroyed live hybrid scores");
    initialize(path);activate(true);for(unsigned i=0;i<1018;i++)MEM_B(i,payload)=original[i];records_loaded(rdram,uint32_t(payload));
    check(MEM_BU(4,payload)==12 && MEM_BU(5,payload)==34,"Hybrid records did not survive relaunch");
    auto future=folder/"future.json";{std::ofstream f(future);f<<"{\"version\":99}";}
    initialize(future);check(!readable(),"Unsupported record file enabled play");
    nlohmann::json j;std::ifstream(future)>>j;check(j["version"]==99,"Unsupported hybrid records overwritten");
    activate(false);check(!workshop::rewards::custom_enemy_variant && !workshop::rewards::lucky_fruit_type,"Returning to ordinary mode retained hybrid callbacks");
    std::cout<<"Hybrid mode: exact 75% compatible fruit, stable revisits, 51 native Maskass identities, "<<cycles<<" gated six-color cycles, matching Lucky icon, purchase/24-clear gates and separate persistent scores without changes to ordinary records passed.\n";
}
