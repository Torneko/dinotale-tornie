#include "pause_favorites.hpp"
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>
using namespace workshop::favorites;
extern "C" void workshop_807F0800(uint8_t*,recomp_context*);
extern "C" void workshop_807F0A00(uint8_t*,recomp_context*);
extern "C" void workshop_807F0C00(uint8_t*,recomp_context*);
extern "C" void workshop_807F0D00(uint8_t*,recomp_context*);
static void check(bool b,const char* text){if(!b){fprintf(stderr,"%s\n",text);std::exit(1);}}
static int32_t bases[400]{};extern "C" {int32_t* section_addresses=bases;}
static unsigned spawned=0,linked=0,transition=0;
extern "C" void func_8004613C(uint8_t* rdram,recomp_context* ctx){
    const auto actor=gpr(int32_t(0x80030000+spawned++*0x400));
    MEM_W(0,actor)=0x19971110;MEM_W(12,actor)=ctx->r4;ctx->r2=actor;
}
extern "C" void func_80047F9C(uint8_t* rdram,recomp_context* ctx){MEM_W(0x180,ctx->r5)=ctx->r4;linked++;}
static void transitioned(uint8_t*,recomp_context*){transition++;}
extern "C" recomp_func_t* get_function(int32_t at){check(uint32_t(at)==0x80012040,"Unexpected native pause transition");return transitioned;}
int main(){
    const std::vector<std::vector<unsigned>> expected={{5,11,12,13},{3,9,10,13},{4,9,12,14},{6,10,11,14},{6,10,11,14},{3,9,10,13},{0},{0},{3,4,9},{3,6,10},{6,5,11},{4,5,12},{3,5,13},{6,4,14}};
    std::vector<uint8_t> bytes(0x800000);auto* rdram=bytes.data();bases[292]=int32_t(0x80500000);
    MEM_W(0,int32_t(0x80502EC0))=0x3F800000;
    const auto parent=gpr(int32_t(0x80010000)),data=gpr(int32_t(0x80011000)),sp=gpr(int32_t(0x80020000));
    for(unsigned color=0;color<14;color++){
        MEM_W(0,int32_t(0x807F0008))=color;MEM_W(0x1F4,parent)=uint32_t(data);
        MEM_W(-4,data)=0x12345678;MEM_W(0x34,data)=0x23456789;
        spawned=linked=0;recomp_context ctx{};ctx.r4=parent;ctx.r29=sp;ctx.r31=0x80012000;
        workshop_807F0800(rdram,&ctx);
        check(spawned==expected[color].size() && linked==spawned,"Favorite icon creation/ownership mismatch");
        check(ctx.r29==sp && uint32_t(ctx.r31)==0x80012000,"Pause creation damaged the MIPS stack");
        for(unsigned slot=0;slot<spawned;slot++){
            const auto icon=MEM_W(pointerOffset(slot),data);check(MEM_W(0x180,icon)==parent,"Unowned favorite icon");
            ctx.r4=icon;ctx.r12=0x807BA300;ctx.r3=1;workshop_807F0D00(rdram,&ctx);
            check(MEM_HU(0x144,icon)==expected[color][slot],"Menu does not match actual favorite fruit");
        }
        const auto primary=MEM_W(4,data);real(rdram,parent,0x6C,1);real(rdram,primary,0x3C,104);real(rdram,primary,0x40,-32);real(rdram,primary,0x6C,1);real(rdram,primary,0x70,1);MEM_B(0x165,primary)=0;
        ctx.r29=sp;MEM_W(0x20,sp)=uint32_t(parent);MEM_W(0x14,sp)=0x80012000;workshop_807F0A00(rdram,&ctx);
        check(ctx.r29==sp+0x20 && uint32_t(ctx.r31)==0x80012000,"Pause positioning damaged its caller epilogue");
        float previous=-1000;
        for(unsigned slot=0;slot<spawned;slot++){
            const auto icon=MEM_W(pointerOffset(slot),data);const auto x=real(rdram,icon,0x3C);
            check(x>previous && std::abs(real(rdram,icon,0x40)+32)<0.001f && MEM_BU(0x165,icon)==0,"Favorite layout or visibility mismatch");
            if(slot)check(x-previous>=11 && x-previous<=13,"Favorite icons are too far apart or overlapping");
            previous=x;
        }
        if(spawned>1){
            const auto first=real(rdram,MEM_W(4,data),0x3C),last=real(rdram,MEM_W(pointerOffset(spawned-1),data),0x3C);
            check(last-first<=40 && std::abs((first+last)*0.5f-104)<0.001f,"Favorite group exceeds its space or loses its center");
        }
        ctx.r4=parent;ctx.r31=0x80012000;transition=0;workshop_807F0C00(rdram,&ctx);
        check(transition==1,"Native pause-close transition missing");
        for(unsigned slot=1;slot<spawned;slot++)check(MEM_BU(0x165,MEM_W(pointerOffset(slot),data))==1,"Favorite icon survives closing pause");
        check(uint32_t(MEM_W(-4,data))==0x12345678 && uint32_t(MEM_W(0x34,data))==0x23456789,"Pause state exceeded its 52-byte allocation");
    }
    std::puts("Pause favorites verified for 14 colors: exact icons, native creation/linking, caller epilogues, spacing, visibility, cleanup and 52-byte guards.");
}
