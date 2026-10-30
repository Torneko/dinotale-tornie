#include "hybrid_rewards.hpp"
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace workshop;
constexpr auto yoshi=int32_t(0x80010000),enemy=int32_t(0x80012000),egg=int32_t(0x80014000);
constexpr auto resource=int32_t(0x80016000),drawing=int32_t(0x80018000),data=int32_t(0x8001A000),palette=int32_t(0x80060000);
static unsigned variant=0;static bool custom=false;
static std::optional<unsigned> variant_for(uint32_t actor){return custom && actor==uint32_t(enemy)?std::optional<unsigned>(variant):std::nullopt;}
static void check(bool b,const char* m){if(!b){fprintf(stderr,"Egg regression: %s\n",m);std::exit(1);}}
extern "C" void func_8003B8B0(uint8_t*,recomp_context*);
extern "C" void func_800329B8(uint8_t*,recomp_context*);
extern "C" void func_8003BA24(uint8_t*,recomp_context*);
extern "C" void func_8004590C(uint8_t* rdram,recomp_context* c){MEM_W(8,drawing)=c->r5;}
extern "C" void func_8003B7D4(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_8004700C(uint8_t*,recomp_context* c){c->r2=yoshi;}
extern "C" void func_80047090(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_80046804(uint8_t*,recomp_context* c){c->r2=egg;}
extern "C" void func_80067818(uint8_t*,recomp_context*){}
extern "C" void func_80067E78(uint8_t*,recomp_context*){}
extern "C" void func_8004A73C(uint8_t*,recomp_context*){}
extern "C" void func_800657CC(uint8_t*,recomp_context*){}
extern "C" void func_80044194(uint8_t*,recomp_context*){}
int main(){
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
    MEM_H(12,egg)=0x4002;MEM_W(0x168,egg)=resource;MEM_W(0x88,resource)=palette;MEM_W(0x16C,egg)=drawing;
    MEM_W(0x1F4,yoshi)=data;MEM_W(0x64,data)=0x3F800000;
    for(int offset:{0x6C,0x70,0x74})MEM_W(offset,yoshi)=0x3F800000;
    MEM_W(0,int32_t(0x800B004C))=0x3F800000;MEM_W(0,int32_t(0x800B0050))=0x3F800000;
    auto context=[](){recomp_context c{};c.r29=int32_t(0x80040000);c.r31=0x12345678;return c;};
    auto verify=[&](const recomp_context& c,unsigned expected){
        check(c.r29==int32_t(0x80040000) && c.r31==0x12345678,"Native egg stack or return damaged");
        check(uint32_t(MEM_W(8,drawing))==uint32_t(palette)+expected*512,"Egg palette differs from its selected color");
        check(uint32_t(MEM_W(0x1EC,egg))==expected,"Egg state and displayed palette disagree");
    };
    unsigned cases=0;
    for(unsigned color=0;color<14;color++){
        fprintf(stderr,"Egg color %u: direct palettes\n",color);
        MEM_W(0,int32_t(0x807F0008))=color;MEM_W(0,int32_t(0x800F8DB0))=color<8?color:0;
        for(unsigned selected=0;selected<14;selected++){
            auto c=context();c.r4=egg;c.r5=selected;func_8003B8B0(rdram,&c);verify(c,selected);cases++;
        }
        fprintf(stderr,"Egg color %u: default factory\n",color);
        auto c=context();func_8003BA24(rdram,&c);verify(c,color);cases++;
        fprintf(stderr,"Egg color %u: swallowed native colors\n",color);
        custom=false;rewards::custom_enemy_variant=variant_for;
        for(unsigned selected=0;selected<8;selected++){
            MEM_W(0x1EC,enemy)=selected;c=context();c.r4=yoshi;c.r5=enemy;
            func_800329B8(rdram,&c);verify(c,selected);cases++;
        }
        custom=true;
        for(variant=0;variant<6;variant++){
            c=context();c.r7=enemy;func_8003BA24(rdram,&c);
            verify(c,color==6 || color==7?color:8+variant);cases++;
            MEM_W(0x1EC,enemy)=variant%4;c=context();c.r4=yoshi;c.r5=enemy;
            func_800329B8(rdram,&c);verify(c,color==6 || color==7?variant%4:8+variant);cases++;
        }
    }
    rewards::custom_enemy_variant=nullptr;
    printf("Native eggs passed: %u current/default/enemy-color cases, 14 palettes, six custom Maskass colors, unchanged black/white paths and preserved native stack/state.\n",cases);
}
