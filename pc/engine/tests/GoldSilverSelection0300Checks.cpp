#include "recomp.h"
#include <array>
#include <bit>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>

static unsigned checks=0,calls=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
template<class T> static T& access(uint8_t* ram,uint64_t a,uint64_t b,unsigned flip){
    const uint32_t address=uint32_t(a+b);if(address<0x80000000u||address>0x80800000u-sizeof(T))throw std::runtime_error("Native RAM access out of bounds");
    return *reinterpret_cast<T*>(ram+((address-0x80000000u)^flip));
}
#undef MEM_W
#undef MEM_H
#undef MEM_HU
#undef MEM_B
#undef MEM_BU
#define MEM_W(a,b) access<int32_t>(rdram,uint64_t(a),uint64_t(b),0)
#define MEM_H(a,b) access<int16_t>(rdram,uint64_t(a),uint64_t(b),2)
#define MEM_HU(a,b) access<uint16_t>(rdram,uint64_t(a),uint64_t(b),2)
#define MEM_B(a,b) access<int8_t>(rdram,uint64_t(a),uint64_t(b),3)
#define MEM_BU(a,b) access<uint8_t>(rdram,uint64_t(a),uint64_t(b),3)
static int32_t storage[400]{};int32_t* section_addresses=storage;
static uint64_t reg(uint32_t value){return uint64_t(int64_t(int32_t(value)));}
static uint32_t bits(float value){return std::bit_cast<uint32_t>(value);}
static void put32(std::vector<uint8_t>& ram,uint32_t a,uint32_t v){std::memcpy(ram.data()+(a&0x7FFFFF),&v,4);}
static void put16(std::vector<uint8_t>& ram,uint32_t a,uint16_t v){std::memcpy(ram.data()+((a&0x7FFFFF)^2),&v,2);}
static uint32_t read32(const std::vector<uint8_t>& ram,uint32_t a){uint32_t v;std::memcpy(&v,ram.data()+(a&0x7FFFFF),4);return v;}
struct Spawn{uint32_t id,x,y,arg;};static Spawn spawn{};static unsigned spawns=0,parents=0,continuations=0;
// Explicit host boundaries: record native factory arguments and supply one
// synthetic actor; no renderer, allocator, child list or game loop is executed.
static void func_8004613C(uint8_t*,recomp_context* ctx){spawn={uint32_t(ctx->r4),uint32_t(ctx->r5),uint32_t(ctx->r6),uint32_t(ctx->r7)};++spawns;ctx->r2=reg(0x80006000);}
static void func_80047F9C(uint8_t*,recomp_context*){++parents;}
static void workshop_804A4874(uint8_t*,recomp_context*){++continuations;}
static bool selected_egg=false;static uint32_t egg_palette=0;static unsigned egg_palette_calls=0;
static void func_80047058(uint8_t*,recomp_context* ctx){ctx->r2=selected_egg?1:0;}
static void func_8004590C(uint8_t*,recomp_context* ctx){egg_palette=uint32_t(ctx->r5);++egg_palette_calls;}
static void func_8040DCF0(uint8_t*,recomp_context* ctx){ctx->r2=0;}
static void func_800445A4(uint8_t*,recomp_context*){}
static void func_800457A0(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected egg animation boundary");}
static void func_80056564(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected egg collider boundary");}
static void func_80056584(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected egg collider boundary");}
static void func_80058958(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected egg service boundary");}
static void func_8040E254(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected egg reward boundary");}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-label"
#include METAL_SELECTION_NATIVE
#pragma clang diagnostic pop

static recomp_context context(){recomp_context c{};c.r29=reg(0x80770000);c.r31=reg(0x80012340);c.r16=reg(0x80001100);return c;}
int main(int argc,char** argv){try{
    need(argc==2,"Expected verified packed ROM fixture");std::ifstream file(argv[1],std::ios::binary);std::vector<uint8_t> rom{std::istreambuf_iterator<char>(file),{}};need(rom.size()==0x1000000,"Missing packed ROM");
    const auto be16=[&](size_t p){return uint16_t(unsigned(rom[p])<<8|rom[p+1]);};
    const auto be32=[&](size_t p){return uint32_t(rom[p])<<24|uint32_t(rom[p+1])<<16|uint32_t(rom[p+2])<<8|rom[p+3];};
    std::vector<uint8_t> ram(0x800000,0);
    for(size_t i=0;i<0xF0;++i)ram[(0x7FD200+i)^3]=rom[0xFC8400+i];
    for(uint32_t base:{0x80300000u,0x80318000u,0x80460000u}){
        section_addresses[129]=int32_t(base);
        for(unsigned i=0;i<14;++i){put16(ram,base+0x48C+i*2,be16(0xFC8400+i*2));put32(ram,base+0x4A8+i*4,be32(0xFC8420+i*4));put32(ram,base+0x4E0+i*4,be32(0xFC8460+i*4));}
        for(unsigned color=0;color<16;++color){
            auto ctx=context();ctx.r4=reg(0x80005000);ctx.r5=color;ctx.r6=bits(0.75f);const auto initial=ctx;spawns=parents=0;
            after_func_8057D750(ram.data(),&ctx);++calls;const auto actual=spawn;
            need(spawns==1&&parents==1,"Native selection did not spawn and parent exactly once");
            need(actual.id==(uint32_t(be16(0xFC8400+color*2))<<16|(0x200u+color)),"Wrong native selection ID");
            need(actual.x==be32(0xFC8420+color*4)&&actual.y==be32(0xFC8460+color*4)&&actual.arg==bits(0.75f),"Native position/scale disagrees with packed metadata");
            need(ctx.r29==initial.r29&&ctx.r31==initial.r31&&ctx.r16==initial.r16,"Selection ABI changed");
            need(ram[(0x6000+0x17C)^3]==1,"Original child flag missing");
            if(color<14){ctx=initial;before_func_8057D750(ram.data(),&ctx);++calls;need(spawn.id==actual.id&&spawn.x==actual.x&&spawn.y==actual.y&&spawn.arg==actual.arg,"Legacy selection changed across relocation bases");}
        }
    }
    for(unsigned color=0;color<16;++color){
        put16(ram,0x8000500E,uint16_t(0xAA00|color));auto ctx=context();ctx.r2=reg(0x80005000);const auto initial=ctx;const unsigned old_continuations=continuations;
        after_workshop_804A4AE0(ram.data(),&ctx);++calls;
        need(read32(ram,0x807F0008)==color,"PC display color was not retained");
        need(read32(ram,0x800F8DB0)==(color<8?color:color==14?6u:color==15?7u:0u),"Native alias is not WHITE6/BLACK7");
        need(continuations==old_continuations+1&&ctx.r29==initial.r29&&ctx.r16==initial.r16,"Confirmation continuation/ABI changed");
        if(color<14){const auto native=read32(ram,0x800F8DB0);ctx=initial;before_workshop_804A4AE0(ram.data(),&ctx);++calls;need(read32(ram,0x800F8DB0)==native&&read32(ram,0x807F0008)==color,"Old confirmation alias changed");}
    }
    using Fn=void(*)(uint8_t*,recomp_context*);
    const std::array<Fn,4> old_nav{before_func_804A0610,before_func_804A0954,before_func_804A0C98,before_func_804A0FDC};
    const std::array<Fn,4> new_nav{after_func_804A0610,after_func_804A0954,after_func_804A0C98,after_func_804A0FDC};
    for(unsigned direction=0;direction<4;++direction)for(unsigned slot=0;slot<15;++slot){
        for(unsigned i=0;i<15;++i)put32(ram,0x80007000+i*4,0);put32(ram,0x80007000+slot*4,0x80008000);
        put32(ram,0x8000803C,bits(direction==2?115.f:direction==3?85.f:100.f));put32(ram,0x80008040,bits(direction==0?115.f:direction==1?85.f:100.f));
        auto ctx=context();ctx.r4=reg(0x80007000);ctx.r5=bits(10.f);ctx.r6=bits(10.f);ctx.r7=bits(100.f);put32(ram,0x80770010,bits(100.f));const auto initial=ctx;
        new_nav[direction](ram.data(),&ctx);++calls;need(ctx.f0.fl==5.f,"Native navigation does not include new last two neighbours");need(ctx.r29==initial.r29&&ctx.r16==initial.r16,"Native navigation ABI changed");
        ctx=initial;old_nav[direction](ram.data(),&ctx);++calls;need(ctx.f0.fl==(slot<13?5.f:0.f),"Legacy navigation fixture mismatch");
    }
    for(size_t i=0;i<0x2200;++i)ram[(0x250000+i)^3]=rom[0xFC4100+i];
    put32(ram,0x80005168,0x80009000);put32(ram,0x80009088,0x80250000);
    for(uint32_t base:{0x80300000u,0x80318000u,0x80460000u})for(bool selected:{false,true})for(unsigned color=0;color<16;++color){
        section_addresses[6]=int32_t(base);selected_egg=selected;egg_palette_calls=0;put32(ram,0x807F0008,color);
        auto ctx=context();ctx.r4=reg(0x80005000);ctx.r17=reg(0x80001200);const auto initial=ctx;actual_func_8040DEAC(ram.data(),&ctx);++calls;
        const unsigned slice=selected?color+1:0;
        need(egg_palette_calls==1&&egg_palette==0x80250000u+slice*512,"4524 selected display+1/fallback0 mapping changed");
        need(ctx.r29==initial.r29&&ctx.r31==initial.r31&&ctx.r16==initial.r16&&ctx.r17==initial.r17,"4524 constructor ABI changed");
        for(unsigned i=0;i<512;++i)need(ram[((egg_palette&0x7FFFFF)+i)^3]==rom[0xFC4100+slice*512+i],"4524 native pointer does not select packed palette");
    }
    std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"actualNativeCalls\":"<<calls<<",\"relocationBases\":3,\"displayColors\":16,\"directions\":4,\"egg4524Cases\":96,\"egg4524FallbackSlice\":0,\"egg4524MetalSlices\":[15,16],\"rendererExecuted\":false,\"gpuValidated\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
