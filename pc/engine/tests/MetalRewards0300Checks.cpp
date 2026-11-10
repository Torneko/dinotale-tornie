#include "../hybrid_rewards.hpp"
#include "json/json.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>
static unsigned checks=0,food_calls=0,custom_lucky=6;
static unsigned lucky_type(){return custom_lucky;}
static void need(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
extern "C" void func_8002B470(uint8_t*,recomp_context*);
extern "C" void func_8002B200(uint8_t*,recomp_context*);
extern "C" void func_8003D8DC(uint8_t*,recomp_context*);
extern "C" void func_8002B03C(uint8_t*,recomp_context*);
extern "C" void func_80022360(uint8_t*,recomp_context*);
extern "C" void func_8001D770(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_80067BF8(uint8_t*,recomp_context*){}
extern "C" void func_80067818(uint8_t*,recomp_context*){}
extern "C" void func_80067D28(uint8_t*,recomp_context*){}
extern "C" void func_8002B3A0(uint8_t*,recomp_context*){}
extern "C" void func_8002B124(uint8_t*,recomp_context*){}
extern "C" void func_80049674(uint8_t*,recomp_context*){}
extern "C" void func_80049724(uint8_t*,recomp_context*){}
extern "C" void func_8004700C(uint8_t*,recomp_context* c){c->r2=int32_t(0x80016000);}
extern "C" void func_80047090(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_8004613C(uint8_t*,recomp_context* c){c->r2=int32_t(0x80019000);}
extern "C" void func_80044194(uint8_t*,recomp_context*){}
static void fruit_effect(uint8_t*,recomp_context*){}
extern "C" recomp_func_t* get_function(int32_t){return fruit_effect;}
extern "C" void switch_error(const char*,uint32_t,uint32_t){std::abort();}
struct Word {uint32_t address,value;};
static const Word tables[]={
#include "tables.inc"
};
static recomp_context setup(uint8_t* rdram,unsigned color,unsigned id,unsigned health){
    recomp_context c{};c.r29=int32_t(0x80020000);c.r4=int32_t(0x80016000);c.r5=int32_t(0x80016000);
    const unsigned native=workshop::yoshi_colors0300::native_alias(color);
    MEM_W(0,int32_t(0x807F0008))=color;MEM_W(0,int32_t(0x807F0020))=0;MEM_W(0,int32_t(0x800F8DB0))=native;MEM_W(int(native*4),int32_t(0x800F8DB4))=health;
    MEM_W(0,int32_t(0x800F8940))=99;MEM_W(0,int32_t(0x800F8948))=99;MEM_W(0,int32_t(0x800F894C))=0;MEM_W(0,int32_t(0x800F8950))=100;MEM_W(0,int32_t(0x800F8954))=100;
    MEM_W(0,int32_t(0x8001000C))=id<<16;MEM_W(0,int32_t(0x80010174))=0x80014000;MEM_W(0,int32_t(0x80014000))=0x80015000;
    MEM_W(0,int32_t(0x80015018))=id==0x45B6?0x00000022:0x44444444;
    MEM_W(0,int32_t(0x80016164))=0x80010000;MEM_W(0,int32_t(0x80016174))=0x80018000;MEM_B(0,int32_t(0x8001801C))=0;MEM_W(0,int32_t(0x800161F4))=0x80030000;
    MEM_W(0,int32_t(0x80031304))=0;MEM_W(0,int32_t(0x8003130C))=0;
    const uint32_t shifts[]={24,20,24,16,16,28,20,28,24,28,20,16};for(unsigned i=0;i<12;i++)MEM_W(int(i*4),int32_t(0x807F0200))=shifts[i];
    MEM_H(0,int32_t(0x807F5600))=0x403A;MEM_H(2,int32_t(0x807F5600))=0;
    return c;
}
static int popups(uint8_t* rdram){constexpr int values[]={-1,0,1,2,3,0,8,8,4};int sum=0;const auto count=MEM_W(0,int32_t(0x8003130C));need(count>=0&&count<=3,"Popup count left native queue bounds");for(int n=0;n<count;++n){const auto kind=MEM_BU(n*8,int32_t(0x80031310));need(kind<9,"Unsupported popup class escaped adapter");sum+=values[kind]*MEM_W(n*8,int32_t(0x80031314));}return sum;}
int main(){try{
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();for(auto item:tables)MEM_W(0,int32_t(item.address))=item.value;
    for(unsigned color:{14u,15u})for(unsigned health=0;health<=8;++health){
        for(unsigned id:{0x4030u,0x4031u,0x4032u,0x4033u,0x4034u,0x4F00u,0x4F01u,0x4F02u,0x4F03u,0x4F04u,0x4F05u,0x45B6u}){
            workshop::rewards::lucky_fruit_type=nullptr;auto c=setup(rdram,color,id,health);const auto sp=c.r29;const int life=id==0x45B6?4:id>=0x4F00?3:2,points=id==0x45B6?1:life;
            func_8002B470(rdram,&c);++food_calls;const auto alias=workshop::yoshi_colors0300::native_alias(color);
            need(MEM_W(int(alias*4),int32_t(0x800F8DB4))==int(std::min(8u,health+unsigned(life))),"Actual native metal food health/clamp disagrees with approved rule");
            if(MEM_W(0,int32_t(0x800F8950))!=100+(id==0x45B6?0:points)||MEM_W(0,int32_t(0x800F8954))!=100+(id==0x45B6?points:0))std::fprintf(stderr,"Metal reward diagnostic color=%u id=%04X hp=%u fruitScore=%d enemyScore=%d expectedPoints=%d\n",color,id,health,MEM_W(0,int32_t(0x800F8950)),MEM_W(0,int32_t(0x800F8954)),points);
            need(MEM_W(0,int32_t(0x800F8950))==100+(id==0x45B6?0:points)&&MEM_W(0,int32_t(0x800F8954))==100+(id==0x45B6?points:0),"Food points did not retain native fruit versus non-fruit counters");need(popups(rdram)==points,"Native popup disagrees with metal food points");need(c.r29==sp,"Native consumption damaged the stack");
        }
        // Native classic Lucky classification is class6, before any food rule.
        for(unsigned id=0x4030;id<=0x4033;++id){auto c=setup(rdram,color,id,health);MEM_W(0,int32_t(0x800F8940))=id-0x4030;func_8002B470(rdram,&c);++food_calls;need(MEM_W(int(workshop::yoshi_colors0300::native_alias(color)*4),int32_t(0x800F8DB4))==8&&MEM_W(0,int32_t(0x800F8950))==108&&popups(rdram)==8,"Native classic Lucky lost8/8 before metal overrides");}
        workshop::rewards::lucky_fruit_type=lucky_type;
        for(unsigned lucky=0;lucky<6;++lucky){custom_lucky=6+lucky;auto c=setup(rdram,color,0x4F00+lucky,health);func_8002B470(rdram,&c);++food_calls;need(MEM_W(int(workshop::yoshi_colors0300::native_alias(color)*4),int32_t(0x800F8DB4))==8&&MEM_W(0,int32_t(0x800F8950))==108&&popups(rdram)==8,"Hybrid Lucky lost8/8 before metal overrides");}
        workshop::rewards::lucky_fruit_type=nullptr;
    }
    // Direct policy readers preserve native enemy classes for both aliases.
    for(unsigned color:{14u,15u})for(unsigned food=0;food<8;++food){MEM_W(0,int32_t(0x8001000C))=0x403A0000;MEM_W(0,int32_t(0x80015018))=food*0x11111111u;need(workshop::rewards::food_gain(rdram,0x80010000,color,false)==workshop::rewards::food_gain(rdram,0x80010000,workshop::yoshi_colors0300::native_alias(color),false)&&workshop::rewards::food_gain(rdram,0x80010000,color,true)==workshop::rewards::food_gain(rdram,0x80010000,workshop::yoshi_colors0300::native_alias(color),true),"Metal enemy rules replaced White/Black native properties");}
    nlohmann::json result{{"passed",true},{"checks",checks},{"actualNativeConsumptionCalls",food_calls},{"syntheticFoodProperties",true},{"nativeBodiesExecuted",true},{"gameLaunched",false},{"qualification","Real original consumption/health/scoring/popup paths with synthetic RAM/resource/sound boundaries. No GPU or natural gameplay result."}};std::cout<<result.dump()<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
