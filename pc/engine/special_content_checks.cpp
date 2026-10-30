#include "recomp.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
extern "C" void workshop_807F1500(uint8_t*,recomp_context*);
extern "C" void func_8002B088(uint8_t*,recomp_context*);
extern "C" void func_80573F94(uint8_t*,recomp_context*);
extern "C" void func_805741B8(uint8_t*,recomp_context*);
static unsigned reached,sounds;
static int32_t loaded_sections[308]{};
extern "C" { int32_t* section_addresses=loaded_sections; }
extern "C" void workshop_8002B520(uint8_t*,recomp_context*){reached++;}
extern "C" void func_80067818(uint8_t*,recomp_context*){sounds++;}
extern "C" void func_8005FCA8(uint8_t*,recomp_context*){}
extern "C" void switch_error(const char*,uint32_t,uint32_t){std::abort();}
struct Case {uint32_t color,oid,native,props,expected;};
const Case cases[]={
#include "cases.inc"
};
int main(){
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
    recomp_context ctx{};ctx.r29=gpr(int32_t(0x80020000));
    auto check=[&](const Case& c){
        ctx.r13=c.props;
        MEM_W(0,int32_t(0x807F0008))=c.color;MEM_W(0,int32_t(0x807F0020))=0;
        MEM_W(0,int32_t(0x8001000C))=c.oid<<16;MEM_W(0x3C,ctx.r29)=0x80010000;MEM_W(0x2C,ctx.r29)=c.native;
        ctx.r2=gpr(int32_t(0x800A0000));ctx.r31=gpr(int32_t(0x80000300));
        const uint32_t shifts[]={24,20,24,16,16,28,20,28,24,28,20,16};
        for(unsigned i=0;i<12;i++)MEM_W(i*4,gpr(int32_t(0x807F0200)))=shifts[i];
        reached=0;workshop_807F1500(rdram,&ctx);
        if(reached!=1 || uint32_t(MEM_W(0x2C,ctx.r29))!=c.expected || MEM_W(0,int32_t(0x807F0020))!=1 || uint32_t(MEM_W(0,int32_t(0x807F0400)))!=c.oid){
            fprintf(stderr,"Food regression color=%u id=%04X expected=%u\n",c.color,c.oid,c.expected);std::exit(1);
        }
    };
    for(const auto& c:cases)check(c);
    // Independent expected pairs from the confirmed design, not the policy
    // helper's own table. Also cover health classes irrespective of lucky fruit.
    constexpr unsigned expectedPairs[6][2]={{1,2},{1,3},{3,0},{2,0},{1,0},{3,2}};
    unsigned added=0;
    for(unsigned fruit=0;fruit<6;fruit++)for(unsigned color=0;color<14;color++)for(unsigned native=0;native<8;native++){
        const unsigned originalFruitFamily[6]={0,1,2,3,3,1};
        const unsigned family=color<6?originalFruitFamily[color]:color;
        bool favorite=color==6 || color==7 || color==8+fruit || family==expectedPairs[fruit][0] || family==expectedPairs[fruit][1];
        check({color,0x4F00+fruit,native,0x76543210,favorite?3u:2u});added++;
        ctx.r4=gpr(int32_t(0x80010000));func_8002B088(rdram,&ctx);
        if(ctx.r2!=6+fruit)return 2;
    }
    for(unsigned fruit=0;fruit<6;fruit++){
        MEM_W(0,int32_t(0x8001000C))=(0x4F00+fruit)<<16;MEM_W(0,int32_t(0x800F89D0))=0;
        ctx.r4=gpr(int32_t(0x80010000));auto stack=ctx.r29;func_80573F94(rdram,&ctx);
        if(ctx.r29!=stack || MEM_W(0,int32_t(0x800F89D0))!=1 || uint32_t(MEM_W(0,int32_t(0x800F89D4)))!=6+fruit || uint32_t(MEM_HU(0,int32_t(0x800F8AD4)))!=6+fruit)return 3;
        // The thirtieth fruit follows the original level-completion path.
        for(unsigned i=0;i<30;i++)MEM_W(i*4,gpr(int32_t(0x800F89D4)))=6+fruit;
        MEM_W(0,int32_t(0x800F89D0))=29;MEM_W(0,int32_t(0x800F8AC8))=0;
        ctx.r4=gpr(int32_t(0x80010000));func_80573F94(rdram,&ctx);
        if(MEM_W(0,int32_t(0x800F89D0))!=30 || MEM_W(0,int32_t(0x800F8AC8))!=1)return 4;
    }
    unsigned frame_cases=0;
    for(unsigned group=0;group<2;group++)for(unsigned fruit=0;fruit<6;fruit++)for(unsigned param:{0u,0x1FFu,0x200u,0x231u,0x232u,0x3FFu})for(unsigned parent=0;parent<2;parent++){
        const unsigned id=(group?0x4F00:0x4030)+fruit;
        ctx.r4=gpr(int32_t(0x80010000));ctx.r5=ctx.r4+0xC4;
        MEM_W(12,ctx.r4)=(id<<16)|param;
        MEM_W(0x180,ctx.r4)=parent?0x80013000:0;
        MEM_W(0x174,ctx.r4)=0x80011000;
        MEM_W(0,gpr(int32_t(0x80011000)))=0x80012000;
        auto stack=ctx.r29;func_805741B8(rdram,&ctx);
        unsigned frame=MEM_HU(0x144,gpr(int32_t(0x80010000)));
        if(frame!=(group?3:3+fruit) || ctx.r29!=stack || MEM_HU(12,gpr(int32_t(0x80010000)))!=id){
            fprintf(stderr,"World sprite regression id=%04X frame=%u expected=%u\n",id,frame,group?3:3+fruit);return 5;
        }
        frame_cases++;
    }
    printf("Special fruit checks passed: %zu native MIPS cases, %u new consumption cases, 6 distinct ring/tally types, 6 level-completion paths and %u world-sprite selections.\n",std::size(cases),added,frame_cases);
}
