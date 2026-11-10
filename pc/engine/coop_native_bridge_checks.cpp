#include "coop_native_bridge.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
constexpr uint32_t p0=0x80100000,p1=0x80101000,item=0x80102000,pad=0x80110000;
unsigned conversions=0,cleanups=0;
void need(bool value,const char* why){if(!value)throw std::runtime_error(why);}
void half(uint8_t* rdram,uint32_t address,unsigned value){const gpr at=gpr(int32_t(address));MEM_H(0,at)=uint16_t(value);}
bool input(int index,uint16_t* buttons,float* x,float* y){*buttons=index==1?0x8000:0;*x=index==1?1.0f:0;*y=0;return true;}
}

// Genuine adapter headers and guest-memory accessors, stubbed native callees.
// This checks bridge transactions, not a claim about playing two avatars.
extern "C" void func_8004613C(uint8_t*,recomp_context*){throw std::logic_error("Unexpected factory construction in bridge unit");}
extern "C" void func_800443EC(uint8_t*,recomp_context*){throw std::logic_error("Unexpected active-list insertion in bridge unit");}
extern "C" void func_800445A4(uint8_t* r,recomp_context* c){
    ++cleanups;const auto actors=workshop::coop_native::tree(r,uint32_t(c->r4));
    for(auto actor:actors){workshop::coop_native::byte(r,actor+6,3);workshop::coop_bridge::forget(actor);}
}
extern "C" void func_80045BD8(uint8_t*,recomp_context*){}
extern "C" void func_80022E0C(uint8_t* r,recomp_context*){
    ++conversions;
    const auto pointer=workshop::coop_native::word(r,workshop::coop_native_input::pad_pointer);
    const auto raw=workshop::coop_native_input::Raw::capture(r,pointer);
    half(r,workshop::coop_native_input::current+2,uint16_t(int16_t(raw.x)));
}

int main(){
    using namespace workshop;
    using coop_native::word;using coop_native::byte;
    try{
        std::vector<uint8_t> ram(0x900000);auto* r=ram.data();recomp_context c{};
        c.r29=gpr(int32_t(0x803FF000));
        auto make_actor=[&](uint32_t a,unsigned kind,uint32_t data){
            word(r,a,0x19971110);half(r,a+12,kind);byte(r,a+6,1);
            word(r,a+0x1F4,data);word(r,a+0x138,1);
        };
        make_actor(p0,0x4001,0x80200000);make_actor(p1,0x4001,0x80202000);make_actor(item,0x4030,0);
        // Supply the same structural precondition as the bounded factory:
        // independent4001 +400B/400C/4007 trees, drawing, private and collision.
        for(unsigned player=0;player<2;player++){
            const auto root=player?p1:p0;
            word(r,root+0x16C,0x80210000+player*0x100);
            word(r,root+0x174,0x80211000+player*0x100);
            half(r,root+0x184,3);
            for(unsigned index=0;index<3;index++){
                const auto child=0x80103000+player*0x1000+index*0x400;
                make_actor(child,index==0?0x400B:index==1?0x400C:0x4007,
                           index<2?0x80220000+player*0x4000+index*0x2000:0);
                word(r,root+0x188+index*4,child);word(r,child+0x180,root);
            }
        }
        word(r,coop::native_player,p0);word(r,coop::native_color,0);word(r,coop::custom_color,8);
        for(unsigned i=0;i<8;i++)word(r,coop::native_health+i*4,i?3:7);
        byte(r,0x800F89B4,21);word(r,0x800F89BC,0x1111);
        word(r,coop_native_input::pad_pointer,pad);
        coop_native_input::Raw{0x4000,0,-20,0}.install(r,pad);
        half(r,coop_native_input::current+2,uint16_t(int16_t(-20)));
        coop_bridge::configure(true,9,input);
        coop_native::primary=p0;coop_native::secondary=p1;
        coop_native::remember(r,p1);
        need(coop_native::independent_graph(r,p0,p1),"Fixture does not satisfy native graph precondition");
        coop_bridge::frame(r,&c,2401,43);
        need(coop_bridge::context && coop_bridge::context->depth()==0,"Bridge failed to bind independent actors");
        const auto primary_input=coop_native_input::State::capture(r);
        {
            coop_bridge::Scope second(r,&c,1);
            need(word(r,coop::native_player)==p1 && word(r,coop::custom_color)==9,"Secondary owner was not installed");
            need(coop_native_input::Raw::capture(r,pad).x==80,"Secondary raw controller not installed");
            word(r,coop::native_health,5);byte(r,0x800F89B4,32);word(r,0x800F89BC,0x2222);
            {
                coop_bridge::Scope first(r,&c,0);
                need(word(r,coop::native_player)==p0 && word(r,coop::custom_color)==8,"Nested primary owner was not installed");
                need(coop_native_input::Raw::capture(r,pad).x==-20,"Nested primary consumed secondary input");
                need(byte(r,0x800F89B4)==21 && word(r,0x800F89BC)==0x1111,"Nested primary received secondary transient state");
                word(r,coop::native_health,6);byte(r,0x800F89B4,22);
            }
            need(word(r,coop::native_player)==p1 && word(r,coop::native_health)==5,"Nested alias health was not restored to secondary");
            need(coop_native_input::Raw::capture(r,pad).x==80 && byte(r,0x800F89B4)==32,"Secondary input/transient state was not restored");
        }
        need(word(r,coop::native_player)==p0 && word(r,coop::native_health)==6 && byte(r,0x800F89B4)==22,"Final primary state was not restored");
        need(coop_bridge::context->player(0).health==6 && coop_bridge::context->player(1).health==5,"Aliased players lost independent health");
        need(coop_native_input::Raw::capture(r,pad).x==-20 && coop_native_input::State::capture(r).now==primary_input.now,"Raw/processed primary input was not preserved");
        need(conversions==1 && coop_bridge::owner_stack.empty() && !coop_native_input::depth,"Input converted more than once or scopes leaked");
        auto ticket=coop_bridge::enter(r,&c,0x8003FFD8,item,p1);
        need(ticket && word(r,coop::native_player)==p1,"Item attach did not select its explicit A1 owner");
        coop_bridge::leave(ticket);
        // The invalid pad is an actual helper rejection, after owner entry.
        // A throwing constructor has no destructor: bridge must undo entry.
        const auto saved_form=byte(r,0x800F89B4),saved_mode=word(r,0x800F89BC);
        word(r,coop_native_input::pad_pointer,0);bool refused=false;
        try{coop_bridge::Scope invalid(r,&c,1);}catch(const std::invalid_argument&){refused=true;}
        need(refused,"Invalid native pad was accepted");
        need(coop_bridge::context->depth()==0 && coop_bridge::owner_stack.empty(),"Failed pad constructor leaked owner context depth");
        need(word(r,coop::native_player)==p0 && word(r,coop::custom_color)==8 && byte(r,0x800F89B4)==saved_form && word(r,0x800F89BC)==saved_mode,"Failed pad constructor left secondary globals installed");
        word(r,coop_native_input::pad_pointer,pad);
        {
            coop_bridge::Scope second(r,&c,1);
            byte(r,p0+6,3);coop_bridge::forget(p0);
        }
        need(!word(r,coop::native_player),"An exiting scope resurrected a freed primary pointer");
        need(coop_bridge::invalidated && coop_bridge::owner_stack.empty(),"Lifecycle invalidation was lost");
        coop_bridge::frame(r,&c,2402,43);coop_bridge::frame(r,&c,2403,43);
        need(cleanups==1 && !coop_bridge::context && !coop_native::enabled && coop_native::owned.empty(),"Invalid session did not clean up at safe frame");
        std::cout<<"Native bridge header checks PASS: nested P2/P1 input, aliased health, transients, A1 attachment, throwing-pad rollback, freed-pointer guard and safe-frame cleanup. Stub native callees; no game run.\n";
        return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
