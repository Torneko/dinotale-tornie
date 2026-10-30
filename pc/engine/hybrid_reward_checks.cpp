#include "recomp.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include "hybrid_rewards.hpp"
static unsigned custom_variant=0,custom_lucky=6;
static unsigned requested_fruit=0,created_fruit=0;
static std::optional<unsigned> variant_for_actor(uint32_t actor){return actor==0x80010000?std::optional<unsigned>(custom_variant):std::nullopt;}
static unsigned lucky_type_for_test(){return custom_lucky;}
extern "C" void func_8002B470(uint8_t*,recomp_context*);
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
extern "C" void func_80047090(uint8_t*,recomp_context* c){requested_fruit=unsigned(c->r4);c->r2=0;}
extern "C" void func_8004613C(uint8_t*,recomp_context* c){created_fruit=unsigned(c->r4)>>16;c->r2=int32_t(0x80019000);}
extern "C" void func_80044194(uint8_t*,recomp_context*){}
static void fruit_effect(uint8_t*,recomp_context*){}
extern "C" recomp_func_t* get_function(int32_t){return fruit_effect;}
extern "C" void switch_error(const char*,uint32_t,uint32_t){std::abort();}
static void check(bool ok,const char* text){if(!ok){fprintf(stderr,"Reward regression: %s\n",text);std::exit(1);}}
struct Word {uint32_t address,value;};
static const Word tables[]={
#include "tables.inc"
};
int main(){
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
    for(auto t:tables)MEM_W(0,int32_t(t.address))=t.value;
    constexpr int life[8]={-1,0,1,2,3,2,8,0},points[9]={-1,0,1,2,3,0,8,8,4};
    constexpr unsigned pairs[6][2]={{1,2},{1,3},{3,0},{2,0},{1,0},{3,2}};
    auto mean=[](int a,int b){return int(std::ceil((a+b)/2.0));};
    auto setup=[&](unsigned color,unsigned id,uint32_t props,unsigned health,unsigned combo){
        recomp_context c{};c.r29=int32_t(0x80020000);c.r4=int32_t(0x80016000);c.r5=int32_t(0x80016000);
        MEM_W(0,int32_t(0x807F0008))=color;MEM_W(0,int32_t(0x807F0020))=0;
        MEM_W(0,int32_t(0x800F8DB0))=color<8?color:0;
        MEM_W(int((color<8?color:0)*4),int32_t(0x800F8DB4))=health;
        MEM_W(0,int32_t(0x800F8940))=99;MEM_W(0,int32_t(0x800F8948))=99;MEM_W(0,int32_t(0x800F894C))=0;
        MEM_W(0,int32_t(0x800F8950))=100;MEM_W(0,int32_t(0x800F8954))=100;
        MEM_W(0,int32_t(0x8001000C))=id<<16;
        MEM_W(0,int32_t(0x80010174))=0x80014000;MEM_W(0,int32_t(0x80014000))=0x80015000;
        MEM_W(0,int32_t(0x80015018))=props;
        MEM_W(0,int32_t(0x80016164))=0x80010000;MEM_W(0,int32_t(0x80016174))=0x80018000;
        MEM_B(0,int32_t(0x8001801C))=combo;
        MEM_W(0,int32_t(0x800161F4))=0x80030000;
        MEM_W(0,int32_t(0x80031304))=0;MEM_W(0,int32_t(0x8003130C))=0;
        const uint32_t shifts[]={24,20,24,16,16,28,20,28,24,28,20,16};
        for(unsigned i=0;i<12;i++)MEM_W(int(i*4),int32_t(0x807F0200))=shifts[i];
        MEM_H(0,int32_t(0x807F5600))=0x403A;MEM_H(2,int32_t(0x807F5600))=0;
        return c;
    };
    auto popup_total=[&](){
        int total=0;unsigned count=MEM_W(0,int32_t(0x8003130C));
        check(count<=3,"Unsupported popup entered queue");
        for(unsigned i=0;i<count;i++){
            unsigned kind=MEM_BU(int(i*8),int32_t(0x80031310));check(kind<=8,"Literal amount escaped native popup adapter");
            total+=points[kind]*MEM_W(int(i*8),int32_t(0x80031314));
        }
        return total;
    };
    unsigned fruit_cases=0,eat_cases=0,kill_cases=0;
    fprintf(stderr,"Checking full fruit reward path.\n");
    for(unsigned fruit=0;fruit<6;fruit++)for(unsigned color=0;color<14;color++)for(unsigned health=0;health<=8;health++){
        auto c=setup(color,0x4F00+fruit,0x22222222,health,0);
        func_8002B470(rdram,&c);
        const unsigned family=color==4?3:color==5?1:color;
        const bool favorite=color==6 || color==7 || color==8+fruit || family==pairs[fruit][0] || family==pairs[fruit][1];
        const int gain=color==8+fruit?4:favorite?2:1;
        check(MEM_W(int((color<8?color:0)*4),int32_t(0x800F8DB4))==std::min(8u,health+gain),"Associated fruit health or cap");
        check(MEM_W(0,int32_t(0x800F8950))==100+gain,"Associated fruit points");
        check(popup_total()==gain,"Associated fruit popup amount");
        if(color==8+fruit)check(MEM_W(0,int32_t(0x8003130C))==1 && MEM_BU(0,int32_t(0x80031310))==8,"Favorite hybrid fruit must show exactly one four-point bubble");
        fruit_cases++;
    }
    for(unsigned hybrid=0;hybrid<6;hybrid++)for(unsigned a=0;a<8;a++)for(unsigned b=0;b<8;b++){
        // Other families deliberately disagree with both component colors.
        uint32_t props=0x77777777;
        for(unsigned i=0;i<2;i++){unsigned shift=28-4*pairs[hybrid][i];props=(props&~(15u<<shift))|((i?b:a)<<shift);}
        const int expected_life=mean(life[a],life[b]),expected_points=mean(points[a],points[b]);
        for(unsigned id:{0x403Au,0x4199u,0x4698u,0x4041u}){
            for(unsigned health=0;health<=8;health++){
                auto c=setup(8+hybrid,id,props,health,0);func_8002B470(rdram,&c);
                const int after=MEM_W(0,int32_t(0x800F8DB4));
                if(after!=std::clamp(int(health)+expected_life,0,8))fprintf(stderr,"Eat hybrid=%u classes=%u/%u health=%u actual=%d expected=%d\n",hybrid,a,b,health,after,std::clamp(int(health)+expected_life,0,8));
                check(after==std::clamp(int(health)+expected_life,0,8),"Enemy mean health/clamp");
                check(MEM_W(0,int32_t(0x800F8954))==100+expected_points,"Enemy mean points");
                check(popup_total()==(expected_points==0?0:expected_points),"Enemy consumed popup");eat_cases++;
            }
            for(unsigned combo=0;combo<5;combo++){
                auto c=setup(8+hybrid,id,props,4,combo);c.r5=int32_t(0x80010000);
                func_8003D8DC(rdram,&c);
                const int base=mean(std::max(points[a],life[a]),std::max(points[b],life[b]));
                const int multiplier=std::min(2<<combo,16),expected=std::max(base,0)*multiplier;
                check(MEM_W(0,int32_t(0x800F8954))==100+expected,"Kill mean must retain x2/combo and life-only conversion");
                check(popup_total()==expected,"Kill popup total differs from points");
                check(MEM_W(0,int32_t(0x800F8DB4))==4,"Kill must not heal player");kill_cases++;
            }
        }
    }
    workshop::rewards::custom_enemy_variant=variant_for_actor;
    for(custom_variant=0;custom_variant<6;custom_variant++)for(unsigned color=0;color<14;color++){
        auto standard=[&](unsigned family){family=family==4?3:family==5?1:family;return family==pairs[custom_variant][0] || family==pairs[custom_variant][1]?3:1;};
        const int expected=color==6 || color==7?3:color<6?standard(color):mean(standard(pairs[color-8][0]),standard(pairs[color-8][1]));
        auto c=setup(color,0x403A,0x22222244,1,0);func_8002B470(rdram,&c);
        check(MEM_W(int((color<8?color:0)*4),int32_t(0x800F8DB4))==1+expected,"Custom Maskass consumption life mismatch");
        check(MEM_W(0,int32_t(0x800F8954))==100+expected && popup_total()==expected,"Custom Maskass score/popup mismatch");
        for(unsigned combo=0;combo<5;combo++){
            c=setup(color,0x403A,0x22222244,4,combo);c.r5=int32_t(0x80010000);func_8003D8DC(rdram,&c);
            const unsigned total=expected*std::min(2<<combo,16);
            check(MEM_W(0,int32_t(0x800F8954))==100+total && popup_total()==int(total),"Custom Maskass kill lost x2/combo or preferred-color mean");
        }
    }
    workshop::rewards::custom_enemy_variant=nullptr;
    unsigned no_score_cases=0;
    for(unsigned color=0;color<14;color++)for(unsigned id:{0x41DBu,0x403Au,0x4041u})for(unsigned nativeClass:{1u,5u}){
        const uint32_t props=nativeClass*0x11111111u;
        for(bool custom:{false,true}){
            workshop::rewards::custom_enemy_variant=custom?variant_for_actor:nullptr;
            custom_variant=0;
            auto c=setup(color,id,props,1,0);func_8002B470(rdram,&c);
            check(MEM_W(0,int32_t(0x800F8954))==100 && popup_total()==0,"No-score enemy awarded consumption points");
            check(MEM_W(int((color<8?color:0)*4),int32_t(0x800F8DB4))==1+(nativeClass==5?2:0),"No-score source lost native healing");
            check(MEM_W(0,int32_t(0x8003130C))==(nativeClass==5?1:0),"Empty native heart bubble missing or a zero-health bubble was invented");
            if(nativeClass==5)check(MEM_BU(0,int32_t(0x80031310))==5,"Healing with zero score must show the empty heart");
            for(unsigned combo=0;combo<5;combo++){
                c=setup(color,id,props,4,combo);c.r5=int32_t(0x80010000);func_8003D8DC(rdram,&c);
                check(MEM_W(0,int32_t(0x800F8954))==100 && popup_total()==0,"Repeatable enemy generated kill/combo points");no_score_cases++;
            }
        }
    }
    workshop::rewards::custom_enemy_variant=nullptr;
    workshop::rewards::lucky_fruit_type=lucky_type_for_test;
    for(unsigned lucky=0;lucky<6;lucky++)for(unsigned color=0;color<14;color++){
        custom_lucky=6+lucky;
        auto c=setup(color,0x4F00+lucky,0x22222222,0,0);func_8002B470(rdram,&c);
        check(MEM_W(int((color<8?color:0)*4),int32_t(0x800F8DB4))==8,"Hybrid Lucky Fruit did not restore native full health");
        check(MEM_W(0,int32_t(0x800F8950))==108 && popup_total()==8,"Hybrid Lucky Fruit native score/popup mismatch");
    }
    workshop::rewards::lucky_fruit_type=nullptr;
    MEM_W(0,int32_t(0x800F8908))=int32_t(0x80022000);
    MEM_W(0x10,int32_t(0x80022000))=int32_t(0x80023000);
    MEM_W(0x10,int32_t(0x80023000))=int32_t(0x80025000);
    for(unsigned native=0;native<4;native++){
        MEM_W(0,int32_t(0x800F8940))=native;
        auto c=setup(0,0x403A,0x22222244,4,0);MEM_W(0,int32_t(0x800F8940))=native;c.r4=int32_t(0x80010000);c.r31=0x12345678;
        const auto sp=c.r29;func_80022360(rdram,&c);
        const unsigned original=MEM_HU(int(native*2),int32_t(0x80099C38));
        check(requested_fruit==original && created_fruit==original && c.r29==sp && c.r31==0x12345678,"Normal Lucky transformation changed");
        workshop::rewards::lucky_fruit_type=lucky_type_for_test;
        for(unsigned lucky=6;lucky<12;lucky++){
            custom_lucky=lucky;c={};c.r29=sp;c.r31=0x12345678;c.r4=int32_t(0x80010000);
            func_80022360(rdram,&c);
            check(requested_fruit==0x4F00+lucky-6 && created_fruit==requested_fruit,"Invincible ground-pound transformation created a standard Lucky fruit");
            check(c.r29==sp && c.r31==0x12345678,"Lucky transformation damaged native stack/return");
        }
        workshop::rewards::lucky_fruit_type=nullptr;
    }
    printf("Native rewards passed: %u fruit health/score cases, %u enemy consumption cases, %u kills with x2/combos, all 6 custom Maskass for 14 Yoshis (preferred 3), empty-heart bubbles, %u no-score/combo cases and all 6 hybrid Lucky Fruit bonuses.\n",fruit_cases,eat_cases,kill_cases,no_score_cases);
}
