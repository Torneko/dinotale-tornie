#include "../boss_blargg0289.hpp"
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
#ifndef BLARGG_NATIVE_SLICE
#error Supply the unchanged original native function corpus.
#endif
#ifndef BLARGG_ORIGINAL_DATA
#error Supply verified original US ROM collision coefficients and FSM table.
#endif
namespace q=workshop::boss_blargg0289;namespace n=workshop::boss_native;
static unsigned checks=0,body_calls=0,readonly_hooks=0,impact_allocations=0;
static std::map<std::string,unsigned> body_counts;static bool advance_setter_clock=false;
static q::Observer* active_observer=nullptr;static q::Frame* active_frame=nullptr;
static void need(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
extern "C" void do_break(uint32_t){throw std::runtime_error("Unexpected native break");}
int32_t sections[512]{};int32_t* section_addresses=sections;
static void trace_before(uint8_t*,const recomp_context*,std::string_view);
static void trace_after(uint8_t*,const recomp_context*,std::string_view);
namespace actual {
recomp_func_t* lookup(int32_t);
void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected original native assertion");}
void func_8002F6E4(uint8_t*,recomp_context*){throw std::runtime_error("Player damage is outside the egg collider fixture");}
void func_80048060(uint8_t*,recomp_context*){throw std::runtime_error("Retirement fixture has no parent attachment");}
void func_8005FD1C(uint8_t*,recomp_context*){throw std::runtime_error("Declared fixture has no registered box provider");}
void func_8005F9F4(uint8_t*,recomp_context*){throw std::runtime_error("Declared fixture has no registered collision provider");}
void func_804F4F50(uint8_t*,recomp_context*){} // Explicit animation/integration boundary.
void func_80047AB8(uint8_t* rdram,recomp_context* c){for(unsigned i=0;i<3;++i)MEM_W(i*4,c->r4)=MEM_W(0x84+i*4,c->r5);} // HOST current world point.
void func_80067BBC(uint8_t*,recomp_context* c){c->r2=0;}void func_80067818(uint8_t*,recomp_context*){}
void impact_effect(uint8_t*,recomp_context* c){++impact_allocations;c->r2=0;}
#undef LOOKUP_FUNC
#define LOOKUP_FUNC(v) lookup(int32_t(v))
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include BLARGG_NATIVE_SLICE
#pragma clang diagnostic pop
recomp_func_t* lookup(int32_t address){switch(uint32_t(address)){case 0x80043E88:return func_80043E88;case 0x80044658:return func_80044658;case 0x803FF000:return impact_effect;default:throw std::runtime_error("Unknown original native indirect callback");}}
}
#include BLARGG_ORIGINAL_DATA
static void trace_before(uint8_t* r,const recomp_context* c,std::string_view name){if(!active_observer||!active_frame)return;if(advance_setter_clock&&name=="func_804F5CE8")++active_frame->vi;const std::vector<uint8_t> before(r,r+0x800000);const auto context=*c;active_observer->enter(r,c,name,*active_frame);need(std::equal(before.begin(),before.end(),r)&&std::memcmp(c,&context,sizeof(context))==0,"Native-entry observer preserves full8MiB RAM and registers");++readonly_hooks;}
static void trace_after(uint8_t* r,const recomp_context* c,std::string_view name){if(!active_observer||!active_frame)return;const std::vector<uint8_t> before(r,r+0x800000);const auto context=*c;active_observer->leave(r,c,name,*active_frame);need(std::equal(before.begin(),before.end(),r)&&std::memcmp(c,&context,sizeof(context))==0,"Native-return observer preserves full8MiB RAM and saved context");++readonly_hooks;}
struct Fixture {
 static constexpr uint32_t overlay=0x80100000,player=0x80070000,egg=0x80020000,egg_private=0x8002024C,egg_resource=0x80041000,egg_meta=0x80050000,egg_definition=0x80051000,egg_scale=0x80052000,manager=0x80060000,manager_flags=0x80060100,garbage=0x80062000,effect_provider=0x80063000,effect_factory=0x80063100,stack=0x807E0000;
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* r=memory.data();uint8_t* rdram=r;
 q::Observer observer;q::Frame frame{169,19,2,0,10,overlay,player,true,true};recomp_context c{};
 static uint32_t root(unsigned i){return 0x80010000+i*0x4000;}static uint32_t priv(unsigned i){return root(i)+0x1000;}static uint32_t resource(unsigned i){return 0x80040000+i*0x100;}static uint32_t record(unsigned i){return 0x80043000+i*16;}
 void w(uint32_t a,uint32_t value){MEM_W(0,n::base(a))=value;}void b(uint32_t a,unsigned value){MEM_B(0,n::base(a))=uint8_t(value);}void h(uint32_t a,unsigned value){MEM_H(0,n::base(a))=uint16_t(value);}void fl(uint32_t a,float value){w(a,std::bit_cast<uint32_t>(value));}
 void actor(uint32_t a,uint16_t type,unsigned slot){w(a,0x19971110);h(a+12,type);b(a+6,0);b(a+0x17D,2);h(a+0x17E,slot);}
 void state(unsigned i,unsigned major,unsigned sub){b(root(i)+0x18,major);b(root(i)+0x30,sub);w(root(i)+0x20,sub<9&&original_states[sub]?overlay+original_states[sub]-q::original_overlay:0);}
 Fixture(){
  sections[90]=int32_t(overlay);need(observer.configure({"blargg-0289",2,169,0,true}),"Owned two-member encounter config");
  for(unsigned i=0;i<18;++i)w(overlay+0x20764+i*4,original_coefficients[i]);for(unsigned i=0;i<9;++i)w(overlay+0x20878+36+i*4,original_states[i]?overlay+original_states[i]-q::original_overlay:0);
  actor(player,0x4001,3);w(0x800F8828,player);w(0x800F8DB4,5);w(0x800F89D0,17);w(0x807F1000,0x13579BDF);w(0x800F8810,manager);w(manager+0x10,manager_flags);h(0x800F8854+4,0);h(0x800F8854+6,0);h(0x800F8854+8,150);w(0x800F8854+0x10,garbage);
  w(0x800F8908,effect_provider);w(effect_provider+0x10,effect_factory);w(effect_factory+0x18,0x803FF000);
  for(unsigned i=0;i<2;++i){observer.created(root(i),frame);actor(root(i),q::kind,i);w(root(i)+0x168,resource(i));w(resource(i)+0x3C,uint32_t(q::kind)<<16);w(resource(i)+0x40,overlay);w(resource(i)+0x44,overlay+0x28);w(root(i)+0x2C,0x80045BD8);w(root(i)+0x1F8,overlay+0x128);w(root(i)+0x1D8,record(i));w(root(i)+0x1DC,uint32_t(q::kind)<<16|i);w(record(i),0xFFFFFFFF);need(observer.bind(r,i,root(i),100+i,frame),"Bind exact consumed native placement and two independent births");initialize(i);}
  c.r29=n::base(stack);c.r31=n::base(0x800675F0);c.r16=0x12345678;c.f2.u32l=0xAABBCCDD;
 }
 void initialize(unsigned i){recomp_context entry{};entry.r4=n::base(root(i));entry.r29=n::base(stack);observer.enter(r,&entry,"func_804F42B8",frame);w(root(i)+0x1F4,priv(i));fl(priv(i)+0x3C,232);fl(priv(i)+0x60,1);for(unsigned axis=0;axis<3;++axis)fl(root(i)+0x6C+axis*4,1);fl(root(i)+0x84,100+float(i)*200);fl(root(i)+0x88,200);fl(root(i)+0x8C,500);fl(root(i)+0x40,232);state(i,1,1);observer.leave(r,&entry,"func_804F42B8",frame);need(observer.snapshot().members[i].initialized&&!observer.snapshot().invalidated&&observer.initialized_member(r,i),"Declared native-shaped constructor return with separate root/private cleanup admitted");}
 void make_egg(bool new_birth=true,unsigned member=0){if(new_birth)observer.created(egg,frame);std::fill(memory.begin()+(egg&0x7FFFFF),memory.begin()+(egg&0x7FFFFF)+0x500,0);actor(egg,0x4002,2);b(egg+0x18,0x68);w(egg+0x1F4,egg_private);w(egg+0x168,egg_resource);b(egg+0x1F2,1);w(egg_resource+0x3C,0x40020000);w(egg_resource+0x40,0);w(egg_resource+0x44,0x8003B0D0);w(egg+0x174,egg_meta);w(egg_meta,egg_definition);w(egg_definition+0x70,egg_scale);for(unsigned j=0;j<3;++j){h(egg_definition+0xA+2*j,8);fl(egg_scale+4*j,1);}fl(egg+0x84,100+float(member)*200);fl(egg+0x88,180);fl(egg+0x8C,500);}
 void activate(){active_observer=&observer;active_frame=&frame;}
 void hit(unsigned member=0,bool open=true,bool new_birth=true){state(member,1,open?5:1);make_egg(new_birth,member);c.r4=n::base(root(member));c.r5=n::base(egg);c.r6=c.r7=0;activate();actual::func_804F46E4(r,&c);active_observer=nullptr;active_frame=nullptr;need(!observer.snapshot().invalidated,"Original collider trace preserves campaign identity");++frame.vi;}
 void retreat(unsigned member=0,float delta=-1){state(member,1,7);fl(root(member)+0x40,232+delta);fl(root(member)+0xC8,1);c.r4=n::base(root(member));c.r29=n::base(stack);activate();actual::func_804F571C(r,&c);active_observer=nullptr;active_frame=nullptr;++frame.vi;}
 bool retire(unsigned member=0){activate();const auto before=c;const bool ok=observer.retire(r,&c,member,frame,[](uint32_t fn,uint8_t* ram,recomp_context* context){if(fn!=0x800447C8)throw std::runtime_error("Only original native retirement is allowed");actual::func_800447C8(ram,context);});active_observer=nullptr;active_frame=nullptr;need(std::memcmp(&before,&c,sizeof(c))==0,"Native retirement uses bounded copied context, preserving caller registers");need(n::word(r,0x800F8DB4)==5&&n::word(r,0x800F89D0)==17&&n::word(r,0x807F1000)==0x13579BDF,"Native retirement changes no player health, fruit count or custom progression sentinels");return ok;}
};
int main(){try{
 {Fixture f;need(!f.retire(),"Neither init nor timer can retire a member");f.hit();need(f.observer.snapshot().members[0].hits==1,"Literal native46E4→447C8→57E4→5CE8 return creates exactly one hit");need(!f.observer.both_defeated()&&!f.retire(),"One accepted hit cannot win");f.hit(0,true,false);need(f.observer.snapshot().members[0].hits==1,"Replayed same egg incarnation cannot count twice");f.hit();f.hit();need(f.observer.snapshot().members[0].hits==3&&!f.retire(),"Three true hits still require native retraction");f.retreat(0,0);need(!f.observer.snapshot().members[0].retracted&&!f.retire(),"Actual571C Y==base does not call its terminal setter");f.retreat(0,1);need(!f.observer.snapshot().members[0].retracted,"Actual571C Y>base also skips setter");f.retreat();need(f.observer.snapshot().members[0].retracted,"Actual571C Y<base setter and returned clamp prove retraction");f.frame.quiescent=false;need(!f.retire(),"Collider/unfinished frame cannot execute retirement");f.frame.quiescent=true;need(f.retire(),"Completed frame calls original447C8 and verifies native terminal marks");const auto first=f.observer.take_receipts();need(first.size()==1&&first[0].hits==3&&first[0].member==0&&first[0].identity.birth&&first[0].variant_birth==100&&!f.observer.both_defeated(),"One retired Blargg gives one receipt and does not win encounter");for(unsigned i=0;i<3;++i)f.hit(1);f.retreat(1);need(f.retire(1)&&f.observer.both_defeated(),"Second independently hit/retracted/retired Blargg completes pair");const auto second=f.observer.take_receipts();need(second.size()==1&&second[0].member==1&&second[0].variant_birth==101&&f.observer.take_receipts().empty(),"Second receipt is exact and cannot be redelivered");}
 {Fixture f;f.hit(0,false);need(!f.observer.snapshot().members[0].hits,"Closed-mouth literal native collider retirement is not damage");}
 {Fixture f;f.state(0,1,5);f.make_egg();f.fl(Fixture::egg+0x8C,900);f.c.r4=n::base(Fixture::root(0));f.c.r5=n::base(Fixture::egg);f.activate();actual::func_804F46E4(f.r,&f.c);active_observer=nullptr;active_frame=nullptr;need(!f.observer.snapshot().members[0].hits&&n::byte(f.r,Fixture::egg+0x18)==0x68,"Literal native collider rejects separated Z before retire/damage");}
 for(unsigned fault=0;fault<19;++fault){Fixture f;f.state(0,1,5);f.make_egg();const auto sp=Fixture::stack-0xA8;f.w(sp+0xA8,Fixture::root(0));f.w(sp+0xAC,Fixture::egg);recomp_context e{};e.r4=n::base(Fixture::egg);e.r29=n::base(sp);e.r31=n::base(Fixture::overlay+0x8FC);q::Frame frame=f.frame;if(fault==0)e.r31+=4;if(fault==1)frame.retry_epoch++;if(fault==2)frame.room++;if(fault==3)frame.logical=97;if(fault==4)frame.encounter++;if(fault==5)f.w(sp+0xAC,Fixture::root(1));if(fault==6)f.b(Fixture::egg+0x18,1);if(fault==7)f.h(Fixture::egg+12,0x4018);if(fault==8)f.state(0,1,1);if(fault==9)f.observer.forget(Fixture::egg);if(fault==10)f.w(Fixture::egg+0x1F4,Fixture::egg_private+4);if(fault==11)f.b(Fixture::egg+0x1F2,0);if(fault==12)f.w(Fixture::egg_resource+0x44,0x8003B0D4);if(fault==13)f.w(Fixture::root(0)+0x1D8,Fixture::record(1));if(fault==14)f.w(Fixture::root(0)+0x20,Fixture::overlay+0x20);if(fault==15)f.w(Fixture::egg_resource+0x3C,0x40180000);if(fault==16)f.w(Fixture::root(0)+0x2C,Fixture::overlay+0x128);if(fault==17)f.w(Fixture::root(0)+0x1F8,0x80045BD8);if(fault==18)f.w(Fixture::root(0)+0x1F8,0);const auto before=f.memory;f.observer.enter(f.r,&e,"func_800447C8",frame);need(f.memory==before&&!f.observer.snapshot().members[0].hits,"Wrong native caller/epoch/room/member/stack/egg/open-state/birth cannot create a hit");}
 {Fixture f;f.hit();const auto before=f.memory;f.observer.created(Fixture::root(0),f.frame);need(f.observer.snapshot().invalidated&&f.memory==before&&!f.observer.both_defeated(),"Real member address reuse before terminal invalidates old receipt");}
 {Fixture f;f.hit();auto cfg=f.observer.snapshot().config;cfg.retry_epoch++;need(f.observer.configure(cfg)&&!f.observer.snapshot().members[0].hits&&!f.observer.bind(f.r,0,Fixture::root(0),100,f.frame),"Retry requires fresh observed native births and does not inherit prior hits");}
 {Fixture f;advance_setter_clock=true;f.hit();need(f.observer.snapshot().members[0].hits==1,"VI may advance during the exact native nested setter without losing its causal return");f.hit();f.hit();f.retreat();advance_setter_clock=false;need(f.retire(),"VI advance during real571C setter still admits exact returned retraction and retirement");}
 {Fixture f;for(unsigned i=0;i<3;++i)f.hit();f.retreat();const auto before=f.memory;const auto context=f.c;unsigned dispatched=0;need(!f.observer.retire(f.r,&f.c,0,f.frame,[&](uint32_t,uint8_t*,recomp_context*){++dispatched;}),"Dispatch no-op never creates a defeat from three HOST ledger hits alone");need(dispatched==1&&f.observer.snapshot().invalidated&&!f.observer.both_defeated()&&f.observer.take_receipts().empty()&&f.memory==before&&std::memcmp(&f.c,&context,sizeof(context))==0,"Missing original retirement writes fails closed without any guest mutation or receipt");}
 for(unsigned fault=0;fault<6;++fault){q::Observer o;q::Config cfg{"config",2,169,0,true};if(fault==0)cfg.encounter=0;if(fault==1)cfg.encounter=9;if(fault==2)cfg.physical_room=19;if(fault==3)cfg.token.clear();if(fault==4)cfg.token="not a token";if(fault==5)cfg.token=std::string(129,'x');need(!o.configure(cfg),"Vanilla/final/first/source/token scopes cannot arm new boss adaptation");}
 std::cout<<"BossBlargg0289Checks: "<<checks<<" PASS; "<<body_calls<<" literal native body calls; "<<readonly_hooks<<" readonly hook checks; no GPU/game.\n";std::cout<<"{\"checks\":"<<checks<<",\"nativeBodyCalls\":"<<body_calls<<",\"readonlyHookChecks\":"<<readonly_hooks<<",\"bodyCallsByName\":{";bool comma=false;for(const auto&[name,count]:body_counts){if(comma)std::cout<<",";comma=true;std::cout<<"\""<<name<<"\":"<<count;}std::cout<<"}}\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}