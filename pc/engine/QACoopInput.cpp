#include "coop_native_input.hpp"
#include <bit>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>
using namespace workshop::coop_native_input;
static unsigned calls=0;
static void need(bool ok,const char* error){if(!ok)throw std::runtime_error(error);}
extern "C" void func_80022E0C(uint8_t* rdram,recomp_context* c){
    calls++;const uint32_t raw=word(rdram,pad_pointer);const gpr p=b(raw),cur=b(current),old=b(previous),flags=b(0x800F8995);
    for(int i=0;i<24;i++)MEM_B(i,old)=MEM_BU(i,cur);
    // Stand-in observes only the proven native call contract. It deliberately
    // does not reimplement the game's smoothing/magnitude/angle algorithm.
    MEM_H(2,cur)=MEM_B(2,p);MEM_H(4,cur)=MEM_B(3,p);
    MEM_B(0,cur)=0; // Simulates native singleton facing lookup selecting P1.
    const uint16_t held=MEM_HU(0,p),edge=MEM_HU(12,p);
    MEM_B(0,flags)=(held&0x8000)!=0;MEM_B(1,flags)=(edge&0x8000)!=0;
    MEM_B(2,flags)=(held&0x4000)!=0;MEM_B(3,flags)=(edge&0x4000)!=0;
    c->r2=0xABCDEF; // Must not leak native temporaries into its caller.
}
int main(){
    try{
        std::vector<uint8_t> memory(8*1024*1024,0xA5);auto* rdram=memory.data();
        const uint32_t raw=0x80100000,owner=0x800C0000;
        const gpr rawb=b(raw),global=b(pad_pointer),actor=b(owner),health=b(0x800F8DB4);
        MEM_W(0,global)=int32_t(raw);MEM_W(0,b(0x800F8828))=int32_t(owner);
        MEM_W(0,actor)=0x19971110;MEM_W(0x138,actor)=1;MEM_W(0,health)=8;
        Raw{0x4000,0x4000,-70,30}.install(rdram,raw);State baseline=State::capture(rdram);auto initial=memory;
        recomp_context c{};c.r29=b(0x800F7000);c.r2=0x1234;auto saved=c;
        reset();need(!update({0,std::numeric_limits<float>::quiet_NaN(),0}) && generation==0,"Nonfinite input was admitted");
        need(update({0x8000,1.5f,-0.5f}) && latest.x==1 && latest.y==-0.5f,"Normalized axes were not bounded");
        {
            Scope outer(rdram,&c);
            need(calls==1 && MEM_H(2,b(current))==80 && MEM_H(4,b(current))==-40,"P2 raw axes did not reach native processing");
            need(MEM_HU(0,rawb)==0x8000 && MEM_HU(12,rawb)==0x8000,"P2 held/pressed raw state incorrect");
            need(MEM_BU(0,b(0x800F8995))==1 && MEM_BU(1,b(0x800F8995))==1,"Native A held/pressed flags missing");
            MEM_B(1,b(0x800F8995))=0; // The real native action can consume this edge.
            {
                Scope nested_same(rdram,&c);
                need(calls==1 && MEM_BU(1,b(0x800F8995))==0,"Nested same-owner scope regenerated a consumed button edge");
            }
            {
                Scope nested_primary(rdram,&c,false);
                need(Raw::capture(rdram,raw).buttons==0x4000 && State::capture(rdram).now==baseline.now,"Nested P1 callback saw P2 input");
                {
                    Scope nested_secondary(rdram,&c);
                    need(calls==1 && MEM_H(2,b(current))==80 && MEM_BU(1,b(0x800F8995))==0,"Nested P2 callback reprocessed the same frame");
                }
                need(Raw::capture(rdram,raw).buttons==0x4000,"P2 inner callback did not restore P1 raw buttons");
            }
            need(Raw::capture(rdram,raw).buttons==0x8000 && MEM_H(2,b(current))==80,"P1 inner callback did not restore P2 raw axes");
        }
        need(memory==initial,"P2 scope wrote outside or failed to restore the original P1 RAM state");
        need(std::memcmp(&saved,&c,sizeof(c))==0,"Native input processing leaked registers");
        {Scope repeated(rdram,&c);need(calls==1 && MEM_BU(1,b(0x800F8995))==0,"Repeated wrapper regenerated pressed within the same sample");}
        need(update({0x8000,0,0}),"Held sample rejected");
        {
            Scope next_frame(rdram,&c);need(calls==2 && MEM_HU(12,rawb)==0,"Holding A produced a second pressed edge");
            need(MEM_BU(0,b(current))==1,"Zero-axis facing did not use the scoped owner actor138");
        }
        need(memory==initial && MEM_W(0,health)==8,"Input scope changed health or P1 input");
        update({0,0,0});{Scope released(rdram,&c);need(MEM_BU(0,b(0x800F8995))==0 && MEM_HU(12,rawb)==0,"Release produced held/pressed flags");}
        update({0x4000,-1,0});{Scope press_b(rdram,&c);need(MEM_BU(2,b(0x800F8995))==1 && MEM_BU(3,b(0x800F8995))==1,"Independent B held/pressed state missing");}
        need(memory==initial && depth==0 && active_owner==-1,"Input scopes left memory or nesting state active");
        bool blocked=false;try{Scope active(rdram,&c);reset();}catch(const std::logic_error&){blocked=true;}
        need(blocked && depth==0 && memory==initial,"Reset within an active scope did not fail safely");
        update({0xC000,1,-1});const auto before_neutral=memory;const auto count_before=calls;
        {Scope blocked_input(rdram,&c,true,true);need(Raw::capture(rdram,raw).buttons==0 && MEM_H(2,b(current))==0 && MEM_H(4,b(current))==0,"Busy-room scope did not process neutral actual input");need(MEM_BU(0,b(0x800F8995))==0 && MEM_BU(2,b(0x800F8995))==0,"Busy-room scope retained A/B");}
        need(calls==count_before+1 && memory==before_neutral && depth==0,"Neutral processing was not scoped/restored");
        MEM_W(0,global)=0;bool invalid=false;try{Scope missing(rdram,&c);}catch(const std::invalid_argument&){invalid=true;}
        need(invalid && depth==0,"Invalid native pad pointer was accepted");
        std::cout<<"Native P2 input memory checks PASS: exact write restoration, normalized raw conversion, one native processing pass per sample, held/edge state, consumed flags, nested P2/P1/P2, facing and register isolation. Native movement proof remains separate.\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}
}
