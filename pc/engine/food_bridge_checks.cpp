#include "recomp.h"
#include <cstdio>
#include <vector>

extern "C" void workshop_807F1500(uint8_t*,recomp_context*);
static unsigned reached;
extern "C" void workshop_8002B520(uint8_t*,recomp_context*){reached++;}

struct Case {uint32_t color,oid,native,props,expected;};
const Case cases[]={
#include "cases.inc"
};
int main(){
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
    unsigned tested=0;
    for(const auto& c:cases){
        recomp_context ctx{};
        ctx.r29=gpr(int32_t(0x80020000));ctx.r13=c.props;
        MEM_W(0,gpr(int32_t(0x807F0008)))=c.color;
        MEM_W(0,gpr(int32_t(0x807F0020)))=0;
        MEM_W(0,gpr(int32_t(0x8001000C)))=c.oid<<16;
        MEM_W(0x3C,ctx.r29)=0x80010000;
        MEM_W(0x2C,ctx.r29)=c.native;
        // Original register contents at the jump out of 8002B470.
        ctx.r2=gpr(int32_t(0x800A0000));ctx.r31=gpr(int32_t(0x80000300));
        const uint32_t shifts[]={24,20,24,16,16,28,20,28,24,28,20,16};
        for(unsigned i=0;i<12;i++)MEM_W(i*4,gpr(int32_t(0x807F0200)))=shifts[i];
        reached=0;workshop_807F1500(rdram,&ctx);
        auto foodClass=uint32_t(MEM_W(0x2C,ctx.r29));
        auto count=uint32_t(MEM_W(0,gpr(int32_t(0x807F0020))));
        auto loggedId=uint32_t(MEM_W(0,gpr(int32_t(0x807F0400))));
        if(reached!=1 || foodClass!=c.expected || count!=1 || loggedId!=c.oid){
            fprintf(stderr,"PC food bridge failed: color=%u fruit=%04X native=%u props=%08X class=%u expected=%u continuation=%u count=%u id=%04X\n",c.color,c.oid,c.native,c.props,foodClass,c.expected,reached,count,loggedId);return 1;
        }
        tested++;
    }
    printf("PC generated food bridge: %u MIPS oracle cases passed; every path reaches the native continuation exactly once.\n",tested);
    return 0;
}
