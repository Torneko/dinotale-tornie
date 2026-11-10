#include "../boss_selection0260.hpp"
#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace s=workshop::boss_selection0260;
namespace n=workshop::boss_native;
namespace c=workshop::boss_battle0250;
static unsigned checks=0,calls=0;
static void need(bool value,const char* reason){++checks;if(!value)throw std::runtime_error(reason);}
extern "C" void func_8079F1C0(uint8_t*,recomp_context* ctx){
    ++calls;need(uint32_t(ctx->r4)==0x80010000 && ctx->r5==0 && ctx->r6==4,"native setter receives original selector and state0/4");
    ctx->r2=0xBAD;ctx->r4=0;ctx->r5=0xFFFF;ctx->r31=0;
}
struct Fixture {
    static constexpr uint32_t actor=0x80010000,priv=0x80011000,fade=0x80012000,draw=0x80013000,pad=0x80014000,overlay=0x80300000;
    std::vector<uint8_t> mem=std::vector<uint8_t>(0x800000);
    uint8_t* rdram=mem.data();recomp_context ctx{};
    s::Frame frame{true,c::Phase::Armed,overlay,false};
    void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=int32_t(v);}
    void b(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}
    void h(uint32_t a,unsigned v){MEM_H(0,n::base(a))=uint16_t(v);}
    void tag(uint32_t a,unsigned kind){w(a,0x19971110);h(a+12,kind);}
    Fixture(){
        tag(actor,s::selector_kind);b(actor+0x18,0);b(actor+0x30,3);
        w(actor+0x20,overlay+(s::input_callback-s::overlay_canonical));w(actor+0x1F8,overlay+0x10);w(actor+0x1F4,priv);
        w(priv,0);w(priv+4,2);w(priv+8,0);w(priv+0xC,0x40000000);w(priv+0x10,0);w(priv+0x88,fade);
        tag(fade,0x4014);w(fade+0x180,actor);w(fade+0x16C,draw);
        b(0x800F7EA2,0);b(0x800F841D,41);w(0x800F7EA8,pad);h(pad,0x8000);
        b(0x800F8DFC,2);b(0x800F8DFD,0);
        // A genuinely locked/fresh Trial3-1 is the target of this adapter.
        w(0x800F8E5C,1);w(0x800F8DF4,0xA5A5003F);
        ctx.r4=n::base(actor);ctx.r5=0x1234;ctx.r6=0x5678;ctx.r31=0x80043210;ctx.r29=0x807FE000;
    }
    void rejected(const char* reason){const auto before=mem;const auto saved=ctx;const auto count=calls;need(!s::handle(rdram,&ctx,frame),reason);need(mem==before,"rejected branch has no guest writes");need(std::memcmp(&ctx,&saved,sizeof(ctx))==0,"rejected branch preserves context");need(calls==count,"rejected branch never dispatches native setter");}
    void accepted(){
        auto expected=mem;auto* old=rdram;rdram=expected.data();b(0x800FAAF8,0x32);b(0x800FAB1C,0xA);b(0x800FAB20,9);w(priv+0x10,1);rdram=old;
        const auto before=ctx;const auto count=calls;need(s::handle(rdram,&ctx,frame),"armed first-stage selection handled");need(calls==count+1,"native setter called exactly once");need(std::memcmp(&ctx,&before,sizeof(ctx))==0,"successful native dispatch restores all guest registers");need(mem==expected,"adapter writes only the exact native confirmation branch");need(n::word(rdram,0x800F8E5C)==1 && n::word(rdram,0x800F8DF4)==0xA5A5003F,"fresh access and Yoshi life flags stay unchanged");rejected("a repeated confirmation cannot replay");
    }
};
int main(){try{
    {Fixture x;x.accepted();}
    {Fixture x;x.h(Fixture::pad,0x1000);x.accepted();}
    {Fixture x;x.h(Fixture::pad,0);x.rejected("neutral manual input stays native");}
    {Fixture x;x.h(Fixture::pad,0);x.frame.automatic_first_entry=true;x.accepted();}
    {Fixture x;x.frame.enabled=false;x.rejected("disabled Boss Battle remains native");}
    for(auto phase:{c::Phase::Idle,c::Phase::Fighting,c::Phase::Victory,c::Phase::ToRest,c::Phase::Rest,c::Phase::ToFight,c::Phase::Complete,c::Phase::Failed}){Fixture x;x.frame.phase=phase;x.rejected("only Armed Boss Battle can bypass access");}
    for(unsigned held:{0x4000u,0xC000u,0x5000u,0xD000u}){Fixture x;x.h(Fixture::pad,held);x.frame.automatic_first_entry=true;x.rejected("B always takes precedence over A/Start/automatic entry");}
    std::vector<std::function<void(Fixture&)>> invalid{
        [](auto& x){x.b(0x800F7EA2,7);},[](auto& x){x.b(0x800F841D,9);},[](auto& x){x.b(0x800F841D,42);},
        [](auto& x){x.ctx.r4=0x807FFFF0;},[](auto& x){x.ctx.r4=0;},[](auto& x){x.w(Fixture::actor,0);},
        [](auto& x){x.h(Fixture::actor+12,0x46BB);},[](auto& x){x.h(Fixture::actor+12,0x473E);},[](auto& x){x.h(Fixture::actor+12,0x473F);},
        [](auto& x){x.b(Fixture::actor+6,3);},[](auto& x){x.b(Fixture::actor+0x18,1);},[](auto& x){x.b(Fixture::actor+0x30,4);},
        [](auto& x){x.w(Fixture::actor+0x20,s::input_callback);},[](auto& x){x.w(Fixture::actor+0x1F8,0);},
        [](auto& x){x.w(Fixture::actor+0x1F4,0);},[](auto& x){x.w(Fixture::actor+0x1F4,Fixture::priv+1);},[](auto& x){x.w(Fixture::actor+0x1F4,0x807FFF00);},
        [](auto& x){x.w(0x800F7EA8,0);},[](auto& x){x.w(0x800F7EA8,Fixture::pad+1);},[](auto& x){x.w(0x800F7EA8,0x80800000);},
        [](auto& x){x.frame.selector_overlay=0;},[](auto& x){x.frame.selector_overlay=Fixture::overlay+1;},[](auto& x){x.frame.selector_overlay=0x807FF000;},
        [](auto& x){x.b(0x800F8DFC,0);},[](auto& x){x.b(0x800F8DFD,1);},
        [](auto& x){x.w(Fixture::priv,1);},[](auto& x){x.w(Fixture::priv+4,1);},[](auto& x){x.w(Fixture::priv+8,1);},[](auto& x){x.w(Fixture::priv+0xC,0x3F800000);},[](auto& x){x.w(Fixture::priv+0x10,1);},
        [](auto& x){x.w(Fixture::priv+0x88,0);},[](auto& x){x.h(Fixture::fade+12,0x4010);},[](auto& x){x.w(Fixture::fade+0x180,0);},[](auto& x){x.w(Fixture::fade+0x16C,0x807FFFF0);}
    };
    for(auto& change:invalid){Fixture x;change(x);x.rejected("invalid scene, selector, private allocation, input, course or fade rejected");}
    {Fixture x;const auto before=x.mem;need(!s::handle(nullptr,&x.ctx,x.frame),"null RAM rejected");need(!s::handle(x.rdram,nullptr,x.frame),"null context rejected");need(!s::handle(x.rdram,&x.ctx,x.frame,nullptr),"null native dispatcher rejected");need(x.mem==before,"null guards have no writes");}
    {Fixture x;x.frame.selector_overlay=s::overlay_canonical;x.w(Fixture::actor+0x20,s::input_callback);x.w(Fixture::actor+0x1F8,s::private_destructor);x.accepted();}
    std::cout<<checks<<" Boss Battle selection assertions passed; synthetic RAM only, no native routing proof.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
