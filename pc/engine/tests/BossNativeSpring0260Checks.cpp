#include "recomp.h"
#include "BossNativeSpring0260Source.hpp"
#include "../boss_battle_runtime0250.hpp"
#include "../boss_driver_bridge0260.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <vector>
namespace n=workshop::boss_native;
namespace d=workshop::boss_driver0260;
namespace bridge=workshop::boss_driver_bridge0260;
namespace native=native_spring_source0260;
static unsigned checks=0,assertions=0,attachments=0;
static void need(bool yes,const char* why){++checks;if(!yes){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
namespace native_spring_source0260 {void Fault_HungUp(uint8_t*,recomp_context*){++assertions;throw std::runtime_error("Native actor assertion");}}
struct Fixture {
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* rdram=memory.data();
 static constexpr uint32_t player=0x80100000,parent=0x80101000,queue=0x800F8814,table=0x80020000,stack=0x807B0000;
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}void h(uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}void b(uint32_t a,uint8_t v){MEM_B(0,n::base(a))=v;}
 uint32_t word(uint32_t a){return uint32_t(MEM_W(0,n::base(a)));}unsigned half(uint32_t a){return MEM_HU(0,n::base(a));}unsigned byte(uint32_t a){return MEM_BU(0,n::base(a));}
 void actor(uint32_t a,uint16_t kind){w(a,0x19971110);h(a+12,kind);b(a+0x18,1);b(a+0x17D,2);}
 explicit Fixture(uint16_t kind=0x469E,unsigned slot=0){
  actor(player,0x4001);actor(parent,kind);w(0x800F8828,player);w(0x800F8810,queue);w(queue+0x10,table);h(queue+8,150);
  h(player+0x17E,5);h(parent+0x17E,7);w(table+5*4,player);w(table+7*4,parent);h(parent+0x184,uint16_t(slot));
  for(unsigned i=0;i<slot;++i){const uint32_t dummy=0x80200000+i*0x280;actor(dummy,0x4002);b(dummy+0x17D,0);w(dummy+0x180,parent);h(dummy+0x17E,uint16_t(i));w(parent+0x188+4*i,dummy);}
 }
 void attach(){recomp_context ctx{};ctx.r4=int32_t(parent);ctx.r5=int32_t(player);ctx.r29=int32_t(stack);ctx.r31=int32_t(0x80000300);ctx.r16=0x12345678;native::func_80048114(rdram,&ctx);++attachments;need(uint32_t(ctx.r29)==stack&&uint32_t(ctx.r31)==0x80000300&&uint32_t(ctx.r16)==0x12345678,"Real native attachment preserves stack/RA/S0");}
};
static void controller(Fixture& f){
 for(auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress}){
  d::Controller controller;need(controller.configure(true,true,{},false),"Controller remains explicitly diagnostic");d::Input in;in.player=Fixture::player;in.room=168;in.encounter=1;in.combat=in.controls=in.alive=true;in.primary_state=1;in.reserves=6;in.x=224;in.y=363;in.min_x=40;in.max_x=724;in.has_preferred_x=true;in.preferred_x=324;in.throw_mode=mode;in.spring_attached=bridge::observe_spring_attachment(f.rdram,Fixture::player);
  std::array<d::Target,1> targets{{{0x80300000,0x4599,344,280,0,0,true,true,false}}};in.targets=targets;
  for(unsigned dt=0;dt<65;++dt){in.vi=127+dt;const auto out=controller.step(in);need(controller.snapshot().phase==d::Phase::SpringRelease,"Native-created phase0 rider reaches SpringRelease instead of ordinary aiming");need(out.buttons==((dt%30>=2&&dt%30<6)?d::jump_button:0),"Actual observed attachment yields2VI release and4VI fresh A per30VI");need(out.x>0&&!(out.buttons&(d::tongue_button|d::egg_button)),"Spring escape uses centre-directed movement and A only");const auto repeat=controller.step(in);need(repeat.buttons==out.buttons&&repeat.x==out.x&&repeat.y==out.y,"Same VI cannot consume a spring release interval");}
  in.vi++;in.aiming_actor=0x80301000;in.reticle_actor=0x80302000;in.reticle_x=0;in.reticle_y=0;const auto aimed=controller.step(in);need(controller.snapshot().phase==d::Phase::TrackAim,"True4005 retains priority over the native rider observation");need(aimed.buttons==(mode==d::ThrowMode::HoldAndRelease?d::egg_button:0)&&!(aimed.buttons&(d::jump_button|d::tongue_button)),"Real4005 keeps its original native throw protocol without spring A/B");
 }
}
int main(){
 for(uint16_t kind:{uint16_t(0x410A),uint16_t(0x469E)})for(unsigned slot=0;slot<20;++slot){
  Fixture f(kind,slot);need(!bridge::observe_spring_attachment(f.rdram,Fixture::player),"Active queue phase2 with no parent is not a spring rider");const auto before=f.memory;f.attach();
  need(!assertions,"Well-formed native attach never invokes actor assertion");need(f.byte(Fixture::player+0x17D)==0&&f.half(Fixture::player+0x17E)==slot,"Actual48114→444D0→43C94 removes active queue and47F9C sets exact sparse child index");
  need(f.word(Fixture::table+5*4)==1&&f.word(Fixture::table+7*4)==Fixture::parent,"Native detach marks only former player active slot as vacant sentinel");
  need(f.word(Fixture::player+0x180)==Fixture::parent&&f.word(Fixture::parent+0x188+4*slot)==Fixture::player&&f.half(Fixture::parent+0x184)==slot+1,"Native sparse child graph is reciprocal with a real count");
  const auto attached=f.memory;need(bridge::observe_spring_attachment(f.rdram,Fixture::player)&&attached==f.memory,"Strict observer accepts the real native-created phase0 rider without writing8MiB");need(before!=f.memory,"Native function execution, not fixture assignment, created attachment state");
  if(slot==0)controller(f);
 }
 for(unsigned fault=0;fault<20;++fault){Fixture f;f.attach();
  if(fault==0)f.b(Fixture::player+0x17D,2);if(fault==1)f.b(Fixture::player+0x17D,1);if(fault==2)f.b(Fixture::player+6,3);if(fault==3)f.h(Fixture::player+12,0x4002);if(fault==4)f.w(Fixture::player,0);
  if(fault==5)f.w(0x800F8828,Fixture::parent);if(fault==6)f.w(Fixture::player+0x180,0);if(fault==7)f.h(Fixture::parent+12,0x407D);if(fault==8)f.b(Fixture::parent+6,3);if(fault==9)f.b(Fixture::parent+0x17D,0);if(fault==10)f.b(Fixture::parent+0x17D,3);
  if(fault==11)f.h(Fixture::player+0x17E,20);if(fault==12)f.h(Fixture::player+0x17E,150);if(fault==13)f.h(Fixture::player+0x17E,0xFFFF);if(fault==14)f.h(Fixture::parent+0x184,0);if(fault==15)f.h(Fixture::parent+0x184,21);
  if(fault==16)f.w(Fixture::parent+0x188,0);if(fault==17)f.w(Fixture::parent+0x188,Fixture::parent);if(fault==18)f.w(Fixture::player+0x180,0x807FFFF0);if(fault==19)f.w(Fixture::parent,0);
  const auto immutable=f.memory;need(!bridge::observe_spring_attachment(f.rdram,Fixture::player),"Wrong phase/global/identity/parent/count/sparse slot is rejected");need(immutable==f.memory,"Rejected attachment observation cannot mutate any byte of8MiB");
 }
 // Queue index150 is an explicit native assertion precondition. This is
 // intentionally not an unsafe invalid table-pointer dereference.
 {Fixture f;f.h(Fixture::player+0x17E,150);const auto before=assertions;bool rejected=false;try{f.attach();}catch(const std::runtime_error&){rejected=true;}need(rejected&&assertions==before+1,"Actual444D0 rejects the unregistered active queue index150");need(f.word(Fixture::player+0x180)==0,"Native assertion cannot publish a fake parent graph");}
 {Fixture f;f.w(Fixture::player+0x180,Fixture::parent);const auto before=assertions;bool rejected=false;try{f.attach();}catch(const std::runtime_error&){rejected=true;}need(rejected&&assertions==before+1,"Actual48114 refuses a pre-parented actor before queue mutation");need(f.byte(Fixture::player+0x17D)==2&&f.word(Fixture::table+5*4)==Fixture::player,"Failed native attachment keeps original active queue identity");}
 need(!bridge::observe_spring_attachment(nullptr,Fixture::player),"Null RAM is rejected");
 std::printf("BossNativeSpring0260Checks: %u PASS; %u real48114 calls completed; exact4function native queue/attachment slice, only native assertion stubbed; isolated8MiB/controller fixtures, no game/GPU/ledger/victory claim.\n",checks,attachments);
}
