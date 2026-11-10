#include "../boss_checkpoint_actor0260.hpp"
#include <array>
#include <bit>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#ifndef BOSS_CHECKPOINT_NATIVE_SLICE
#error Supply the unchanged generated native initializer/wakeup slice.
#endif
namespace h=workshop::boss_checkpoint_actor0260;namespace n=workshop::boss_native;
static unsigned checks=0,native_inits=0,native_wakes=0,sounds=0,palette_calls=0,source_spawns=0,suppressed_spawns=0,native_destructors=0;
static int32_t sections[300]{};int32_t* section_addresses=sections;
static void need(bool yes,const char* why){++checks;if(!yes)throw std::runtime_error(why);}
static uint32_t be32(const std::vector<uint8_t>& b,size_t p){need(p+4<=b.size(),"ROM bounded word");return uint32_t(b[p])<<24|uint32_t(b[p+1])<<16|uint32_t(b[p+2])<<8|b[p+3];}
struct Fixture;static Fixture* current=nullptr;
namespace native_checkpoint {
void func_805A64A8(uint8_t*,recomp_context*);void func_805A6730(uint8_t*,recomp_context*);void func_805A65C0(uint8_t*,recomp_context*);
void func_8005FD1C(uint8_t*,recomp_context*);void func_8004590C(uint8_t*,recomp_context*);void func_800495D4(uint8_t*,recomp_context*);
void func_8004700C(uint8_t*,recomp_context*);void func_80048C2C(uint8_t*,recomp_context*);void func_80048BD8(uint8_t*,recomp_context*);void func_80048C18(uint8_t*,recomp_context*);
void func_805A5DF0(uint8_t*,recomp_context*);void func_80067BBC(uint8_t*,recomp_context*);void func_80067818(uint8_t*,recomp_context*);
void func_8004613C(uint8_t*,recomp_context*);void func_80048E9C(uint8_t*,recomp_context*);void func_8006D340(uint8_t*,recomp_context*);void func_80044194(uint8_t*,recomp_context*);void Fault_HungUp(uint8_t*,recomp_context*);
void func_805A5440(uint8_t*,recomp_context*);void func_80045BD8(uint8_t*,recomp_context*);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include BOSS_CHECKPOINT_NATIVE_SLICE
#pragma clang diagnostic pop
}
struct Fixture {
 static constexpr uint32_t actor=0x80100000,priv=0x80101000,record=0x80102000,resource=0x80103000,metadata=0x80104000,definition=0x80105000,drawing=0x80106000;
 static constexpr uint32_t player=0x80110000,pmeta=0x80111000,pdef=0x80112000,extents=0x80113000,offsets=0x80114000,overlay=0x80200000,palette=0x80300000,stack=0x807B0000;
 std::vector<uint8_t> mem=std::vector<uint8_t>(0x800000);uint8_t* ram=mem.data();uint8_t* rdram=ram;h::Controller control;h::Frame f;recomp_context c{};unsigned selected=0;
 Fixture(const std::vector<uint8_t>& rom,unsigned milestone=3,unsigned mask=0,unsigned completed=99):selected(milestone/3){
  current=this;sections[138]=int32_t(overlay);control.configure(true);f.active=true;f.first_fight=milestone==0;f.fighting=milestone==0;f.rest_ready=milestone!=0;f.room=milestone?83:162;f.completed=completed==99?milestone:completed;f.awake_mask=mask;f.vi=100;f.overlay=overlay;f.player=player;f.record=record;f.support_ready=true;control.enter_room(f.room);
  for(unsigned i=0;i<h::overlay_extent;i+=4)w(overlay+i,be32(rom,0xCDBA50+i));
  for(unsigned i=0;i<8;++i)w(overlay+0x1548+i*4,overlay+be32(rom,0xCDBA50+0x1548+i*4)-h::original_overlay);
  live(actor,uint16_t(h::source_kind+selected));b(actor+0x165,1);w(actor+0x1D8,record);w(actor+0x1DC,h::source_packed);w(record,0xFFFFFFFF);fl(record+4,168);fl(record+8,408);fl(actor+0x3C,168);fl(actor+0x40,408);fl(actor+0x84,168);fl(actor+0x88,408);
  w(actor+0x168,resource);w(actor+0x174,metadata);w(metadata,definition);w(resource+0x40,overlay);w(resource+0x44,overlay+h::initializer_offset);w(resource+0x88,palette);w(actor+0x16C,drawing);w(actor+0x20,overlay+h::initializer_offset);w(definition+0x34,offsets);fl(offsets,0);
  live(player,0x4001);w(0x800F8828,player);w(player+0x174,pmeta);w(pmeta,pdef);w(pdef+0x3C,extents);sh(extents+0x1A,16);w(0x800D9A80,0x80115000);w(0x80115000,0x40010000);c.r29=n::base(stack);c.r31=n::base(0x80000300);c.r4=n::base(actor);
 }
 void w(uint32_t p,uint32_t v){MEM_W(0,n::base(p))=int32_t(v);}void sh(uint32_t p,uint16_t v){MEM_H(0,n::base(p))=v;}void b(uint32_t p,unsigned v){MEM_B(0,n::base(p))=uint8_t(v);}void fl(uint32_t p,float v){w(p,std::bit_cast<uint32_t>(v));}uint32_t word(uint32_t p){return n::word(ram,p);}
 void live(uint32_t a,uint16_t kind){w(a,0x19971110);sh(a+12,kind);b(a+0x17D,2);}
 bool init(){c.r4=n::base(actor);const auto entry=c;const auto t=control.init_enter(ram,&c,f);need(bool(t),"Owned actual initializer entry receives birth");const auto regs=c;native_checkpoint::func_805A6600(ram,&c);++native_inits;need(c.r29==regs.r29&&c.r16==regs.r16,"Actual initializer restores SP/S0");return control.init_leave(ram,&entry,f,t);}
 bool wake(unsigned ra=0x6D0){c.r4=n::base(actor);c.r31=n::base(overlay+ra);const auto entry=c;const auto before=mem;const auto t=control.wake_enter(ram,&c,f);need(mem==before,"Wake entry observer changes no native bytes");if(!t)return false;native_checkpoint::func_805A5628(ram,&c);++native_wakes;const auto native_result=mem;const bool result=control.wake_leave(ram,&entry,f,t);need(mem==native_result,"Wake return observer changes no native bytes");return result;}
};
namespace native_checkpoint {
// Allocator/renderer/physics services are outside this oracle. The initializer,
// state setter, wakeup stores and final wake animation below are unchanged
// generated native bodies, with genuine relocated ROM callback/data tables.
void func_805A64A8(uint8_t* rdram,recomp_context* c){need(uint32_t(c->r4)==Fixture::actor,"Native init allocates exact point");MEM_W(0x1F4,c->r4)=int32_t(Fixture::priv);MEM_W(0x1F8,c->r4)=int32_t(Fixture::overlay+0x11BC);MEM_W(0x27C,n::base(Fixture::priv))=int32_t(c->r5);c->r2=1;}
void func_8005FD1C(uint8_t*,recomp_context*){}
void func_8004590C(uint8_t*,recomp_context* c){++palette_calls;need(current&&uint32_t(c->r5)==Fixture::palette+current->selected*512,"Actual initializer selects native numbered palette");c->r2=1;}
void func_800495D4(uint8_t*,recomp_context*){}
void func_8004700C(uint8_t*,recomp_context* c){c->r2=n::base(Fixture::player);}
void func_805A5DF0(uint8_t*,recomp_context* c){c->r2=1;}
void func_805A5440(uint8_t*,recomp_context*){} // Destructor sound stop only.
void func_80045BD8(uint8_t*,recomp_context* c){need(current&&uint32_t(c->r4)==Fixture::actor,"Actual destructor retires its exact actor");++native_destructors;}
void func_80067BBC(uint8_t*,recomp_context* c){c->r2=0;}
void func_80067818(uint8_t*,recomp_context* c){need(uint32_t(c->r4)==0x275,"Actual completed wake uses native0275");++sounds;}
void func_8004613C(uint8_t* ram,recomp_context* c){
 need(current&&uint32_t(c->r31)==0x8005A010,"Actual59FD4 calls allocation at expected native RA");const auto choice=h::select_spawn(ram,c,current->f);need(choice.owned,"Exact source placement owns factory request");
 if(choice.suppress){++suppressed_spawns;c->r2=0;return;}
 ++source_spawns;current->selected=(choice.packed>>16)-h::source_kind;current->sh(Fixture::actor+12,uint16_t(choice.packed>>16));c->r2=n::base(Fixture::actor);
}
void func_80044194(uint8_t* ram,recomp_context* c){
 need(current&&uint32_t(c->r4)==Fixture::actor,"Source constructor initializes allocated actor");const auto entry=*c;const auto ticket=current->control.init_enter(ram,c,current->f);need(bool(ticket),"True48E9C attached source record before initializer");func_805A6600(ram,c);++native_inits;need(current->control.init_leave(ram,&entry,current->f,ticket),"True source constructor initializer return binds checkpoint");c->r2=1;
}
void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected native generic constructor assertion");}
}
static void context_fault(Fixture& x,unsigned fault){
 switch(fault){case 0:x.f.active=false;break;case 1:x.f.awake_mask=16;break;case 2:x.f.room=43;break;case 3:x.f.overlay=0;break;case 4:x.f.record+=12;break;case 5:x.w(Fixture::actor+0x1DC,0x45070001);break;case 6:x.w(Fixture::actor+0x1D8,0);break;case 7:x.w(Fixture::record,0x45070000);break;case 8:x.w(Fixture::resource+0x44,Fixture::overlay+4);break;case 9:x.w(Fixture::actor+0x20,0);break;case 10:x.sh(Fixture::actor+12,0x4209);break;case 11:x.w(Fixture::metadata,0);break;case 12:x.fl(Fixture::actor+0x40,400);break;case 13:x.f.completed=2;break;case 14:x.control.configure(false);break;case 15:x.w(Fixture::actor+0x168,0x807FFFFC);break;}
}
int main(int argc,char** argv){try{
 need(argc==2,"Original USA ROM required");std::ifstream in(argv[1],std::ios::binary);std::vector<uint8_t> rom((std::istreambuf_iterator<char>(in)),{});need(rom.size()==0x1000000,"Original ROM size");
 need(be32(rom,0x547180+0x10)==h::original_overlay&&be32(rom,0xCDBA50+0x368)==0x27BDFFE0,"Native resource/wake identity");
 const auto original=rom;
 for(unsigned m:{3u,6u,9u}){Fixture x(rom,m,1);x.f.support_ready=false;x.f.rest_ready=false;x.w(h::count_address,99);for(unsigned i=0;i<4;++i)x.w(h::flags_address+i*4,7);
  const auto before=x.mem;const auto regs=x.c;const unsigned inits=native_inits;
  for(unsigned tick=0;tick<160;++tick){x.f.vi=100+tick;need(x.control.init_enter(x.ram,&x.c,x.f)==h::deferred_initialization,"Exact Rest constructor waits for source-proven native floor through arrival intro");}
  need(x.mem==before&&!std::memcmp(&regs,&x.c,sizeof(regs)),"Deferred constructor preserves all8MiB and native registers");need(!x.control.snapshot().binding.actor&&!x.control.snapshot().next_birth&&!x.control.snapshot().invalidated&&!x.control.take_wakeup()&&native_inits==inits,"Deferral consumes neither birth nor native flags nor wake receipt");
  need(!x.control.init_leave(x.ram,&x.c,x.f,h::deferred_initialization)&&!x.control.snapshot().invalidated,"Explicit deferred ticket is inert even if a caller forwards it to helper leave");
  x.f.support_ready=true;need(x.init()&&native_inits==inits+1&&x.control.snapshot().binding.birth==1,"Actual6600 executes once after the native floor fence with first real birth");need(x.word(h::count_address)==1&&x.word(h::flags_address)==1,"Only actual initializer synchronizes the existing sparse wake mask");
  x.f.rest_ready=true;need(x.wake()&&x.control.take_wakeup()->completed==m,"Native5628 wake remains the only checkpoint activation after deferred init");
 }
 {Fixture x(rom,0);x.f.support_ready=false;need(x.init()&&x.wake(),"Opening No1 retains native init/wake without a Rest-floor fence");}
 for(unsigned fault=0;fault<16;++fault){Fixture x(rom);x.f.support_ready=false;context_fault(x,fault);const auto before=x.mem;need(x.control.init_enter(x.ram,&x.c,x.f)==0&&x.mem==before,"Invalid source is rejected before deferral rather than receiving the skip ticket");}
 for(unsigned fault=0;fault<6;++fault){Fixture x(rom);x.f.support_ready=false;if(fault==0)x.b(Fixture::actor+0x18,1);if(fault==1)x.b(Fixture::actor+0x17D,1);if(fault==2)x.w(Fixture::actor+0x130,0x800483B8);if(fault==3)x.w(Fixture::actor+0x1F4,Fixture::priv);if(fault==4)x.w(Fixture::actor+0x180,Fixture::player);if(fault==5)x.b(Fixture::actor+0x165,0);const auto before=x.mem;need(x.control.init_enter(x.ram,&x.c,x.f)==0&&x.control.snapshot().invalidated&&x.mem==before,"Only the actual untouched state0/movement0/unparented and draw-disabled native constructor may defer");}
 for(unsigned m:{0u,3u,6u,9u})for(unsigned mask=0;mask<16;++mask){
  Fixture x(rom,m,mask);for(unsigned i=0;i<4;++i)x.w(h::flags_address+i*4,7);x.w(h::count_address,99);need(x.init(),"Actual native initializer binds exact record/private/number");
  need(x.word(h::count_address)==unsigned(std::popcount(mask)),"Native count reflects real sparse mask");for(unsigned i=0;i<4;++i)need(x.word(h::flags_address+i*4)==((mask>>i)&1),"Native flags reflect each actual bit independently");
  const bool awake=(mask>>(m/3))&1;need(n::byte(x.ram,Fixture::actor+0x30)==(awake?2:0),"Actual native init selects awake or sleeping from exact bit");
  need(x.control.suppress_warp(x.ram,&x.c,x.f)==awake,"Only native awakened state blocks free warp");
  if(!awake){need(x.wake((mask%3)==0?0x514:(mask%3)==1?0x564:0x6D0),"Actual native wake earns exact checkpoint once");auto receipt=x.control.take_wakeup();need(receipt&&receipt->completed==m&&receipt->actor==Fixture::actor&&receipt->birth==x.control.snapshot().binding.birth,"Wake receipt binds exact actor birth and milestone");need(!x.control.take_wakeup()&&!x.wake(),"Duplicate wake and duplicate receipt earn nothing");x.f.awake_mask|=1u<<(m/3);need(!x.control.suppress_warp(x.ram,&x.c,x.f),"Wake animation state1 remains native");
   for(unsigned step=0;step<1000&&n::byte(x.ram,Fixture::actor+0x30)==1;++step){x.c.r4=n::base(Fixture::actor);native_checkpoint::func_805A5EDC(x.ram,&x.c);}
   x.c.r4=n::base(Fixture::actor);need(n::byte(x.ram,Fixture::actor+0x30)==2&&x.control.suppress_warp(x.ram,&x.c,x.f),"Unchanged native wake animation finishes with blocked free warp");
  }else need(!x.wake()&&!x.control.take_wakeup(),"Initialization of already awakened point never fabricates wake receipt");
  for(unsigned phase=0;phase<8;++phase){x.c.r4=n::base(Fixture::actor);x.c.r5=phase;native_checkpoint::func_805A6730(x.ram,&x.c);need(x.control.suppress_warp(x.ram,&x.c,x.f)==(phase==2||phase==7),"Warp guard affects exactly native states2/7");}
 }
 for(unsigned completed:{3u,4u,5u,6u,7u,8u,9u}){Fixture x(rom,completed/3*3,0,completed);need(x.init()&&x.wake(),"Late rest wake allowed with actual native evidence");need(x.control.take_wakeup()->completed==completed/3*3,"Late wake saves beginning of its group");}
 for(unsigned fault=0;fault<16;++fault){Fixture x(rom);context_fault(x,fault);const auto bytes=x.mem;const auto regs=x.c;need(!x.control.init_enter(x.ram,&x.c,x.f),"Invalid constructor context rejected");need(x.mem==bytes&&!std::memcmp(&regs,&x.c,sizeof(regs)),"Rejected constructor preserves all bytes/registers");}
 for(unsigned fault=0;fault<18;++fault){Fixture x(rom);need(x.init(),"Negative wake fixture initialized");x.c.r4=n::base(Fixture::actor);x.c.r31=n::base(Fixture::overlay+0x6D0);
  switch(fault){case 0:x.f.active=false;break;case 1:x.f.rest_ready=false;break;case 2:x.f.player=0;break;case 3:x.f.awake_mask=2;break;case 4:x.f.vi=0;break;case 5:x.c.r31+=4;break;case 6:x.c.r4+=4;break;case 7:x.w(Fixture::actor+0x1F4,0);break;case 8:x.w(Fixture::priv+0x27C,3);break;case 9:x.w(Fixture::actor+0x1D8,0);break;case 10:x.w(Fixture::actor+0x1DC,0);break;case 11:x.w(Fixture::record+4,0);break;case 12:x.w(Fixture::actor+0x168,0);break;case 13:x.w(h::flags_address+4,1);break;case 14:x.w(h::count_address,1);break;case 15:x.b(Fixture::actor+0x30,1);break;case 16:x.control.forget(Fixture::actor);break;case 17:x.control.enter_room(162);break;}
  const auto before=x.mem;need(!x.control.wake_enter(x.ram,&x.c,x.f)&&!x.control.take_wakeup(),"Wrong owner/state/caller/record/mask earns no wake");need(x.mem==before,"Rejected wake observer is read-only");
 }
 for(unsigned fault=0;fault<9;++fault){Fixture x(rom);need(x.init(),"Native return rejection initialized");x.c.r4=n::base(Fixture::actor);x.c.r31=n::base(Fixture::overlay+0x514);const auto entry=x.c;const auto ticket=x.control.wake_enter(x.ram,&x.c,x.f);need(bool(ticket),"Owned wake pending");native_checkpoint::func_805A5628(x.ram,&x.c);
  switch(fault){case 0:x.w(h::count_address,0);break;case 1:x.w(h::flags_address+4,0);break;case 2:x.b(Fixture::actor+0x30,2);break;case 3:x.w(Fixture::actor+0x20,Fixture::overlay+0xD34);break;case 4:x.f.vi++;break;case 5:x.w(Fixture::priv+0x27C,0);break;case 6:x.w(Fixture::actor+0x1D8,0);break;case 7:x.w(h::flags_address,1);break;case 8:x.f.completed=6;break;}
  const auto before=x.mem;need(!x.control.wake_leave(x.ram,&entry,x.f,ticket)&&!x.control.take_wakeup(),"Missing/wrong native wake effects fail closed");need(x.mem==before,"Failed wake return changes no native bytes");
 }
 for(unsigned completed=0;completed<10;++completed){Fixture x(rom,3,0,completed);x.w(Fixture::record,h::source_packed);x.c.r4=h::source_packed;x.c.r5=x.word(Fixture::record+4);x.c.r6=x.word(Fixture::record+8);x.c.r16=n::base(Fixture::record);x.c.r31=n::base(0x8005A010);const auto before=x.mem;const auto choice=h::select_spawn(x.ram,&x.c,x.f);need(choice.owned&&choice.suppress==(completed<3),"Only rest before3 suppresses real source before allocation");if(completed>=3)need(choice.packed==uint32_t(0x4507+completed/3)<<16,"Native kind selected without invented instance bits");need(x.mem==before,"Spawn selection preserves source record");for(unsigned field=0;field<5;++field){auto bad=x.c;auto f=x.f;if(field==0)bad.r31+=4;if(field==1)bad.r16+=4;if(field==2)bad.r4++;if(field==3)bad.r5++;if(field==4)f.record+=12;need(!h::select_spawn(x.ram,&bad,f).owned,"Foreign generic constructor is untouched");}}
 for(unsigned completed=0;completed<10;++completed){Fixture x(rom,3,0,completed);x.w(Fixture::record,h::source_packed);x.w(Fixture::actor+0x1D8,0);x.w(Fixture::actor+0x1DC,0);x.c.r4=n::base(Fixture::record);x.c.r16=0x12345678;const auto sp=x.c.r29;native_checkpoint::func_80059FD4(x.ram,&x.c);need(x.c.r29==sp&&x.c.r16==0x12345678,"Actual source constructor restores stack/S0 after success or null allocation");
  if(completed<3)need(!x.c.r2&&x.word(Fixture::record)==h::source_packed&&!x.control.snapshot().binding.birth&&!x.word(Fixture::actor+0x1D8),"Actual59FD4 null allocation skips record consumption, attachment and initialization");
  else need(x.c.r2==1&&x.word(Fixture::record)==0xFFFFFFFF&&x.word(Fixture::actor+0x1D8)==Fixture::record&&x.word(Fixture::actor+0x1DC)==h::source_packed&&x.control.snapshot().binding.index==completed/3,"Actual generic source attachment preserves original45070000 while native actor has selected number");
 }
 {Fixture x(rom,0);x.f.fighting=false;need(x.init(),"First fight binds before Fighting");need(!x.wake(),"Armed binding alone cannot wake No1");x.f.fighting=true;need(x.wake(),"No1 real wake accepted in first fight");}
 {Fixture x(rom,0,1);need(x.init(),"Awakened No1 bound before first victory");x.f.completed=1;x.f.first_fight=false;x.f.fighting=false;x.c.r4=n::base(Fixture::actor);need(x.control.suppress_warp(x.ram,&x.c,x.f),"Victory0to1 retains No1 warp suppression until actual room teardown");x.c.r5=7;native_checkpoint::func_805A6730(x.ram,&x.c);need(x.control.suppress_warp(x.ram,&x.c,x.f),"Victory phase also retains state7 warp suppression");need(!x.wake(),"Post-victory No1 cannot earn another wake receipt");x.control.enter_room(83);need(!x.control.suppress_warp(x.ram,&x.c,x.f),"Actual room teardown ends old No1 binding");}
 for(unsigned mask:{0u,1u,5u}){Fixture x(rom,0,mask);need(x.init(),"No1 initial lifetime bound before off-screen retirement");const auto birth=x.control.snapshot().binding.birth;
  x.f.completed=1;x.f.fighting=false;x.f.first_fight=true;x.f.victory=true;
  // The lifecycle dispatcher notifies forget before the original destructor.
  // Allocator retirement and sound are explicit fixture boundaries below.
  x.control.forget(Fixture::actor);x.c.r4=n::base(Fixture::actor);const auto regs=x.c;native_checkpoint::func_805A65D8(x.ram,&x.c);
  need(x.c.r29==regs.r29&&!x.control.snapshot().binding.actor,"Native destructor and lifetime notification clear old No1 binding");
  x.w(Fixture::record,h::source_packed);x.w(Fixture::actor+0x1D8,0);x.w(Fixture::actor+0x1DC,0);x.w(Fixture::actor+0x20,Fixture::overlay+h::initializer_offset);x.c.r4=n::base(Fixture::record);
  native_checkpoint::func_80059FD4(x.ram,&x.c);const auto& b=x.control.snapshot().binding;
  need(x.c.r2==1&&b.actor==Fixture::actor&&b.birth>birth&&b.milestone==0&&b.index==0,"True generic source constructor rebinds a fresh No1 birth after victory");
  need(x.word(h::count_address)==unsigned(std::popcount(mask)),"Recreated No1 preserves actual sparse awake mask");
  need(h::milestone(x.f)==0&&h::placement_milestone(x.f)==0,"Recreated post-victory No1 retains checkpoint0 ownership and wake eligibility");
  if(!(mask&1)){need(x.wake(),"True5628 wakes recreated sleeping No1 after victory");const auto receipt=x.control.take_wakeup();need(receipt&&receipt->completed==0&&receipt->birth==b.birth&&receipt->actor==Fixture::actor,"Post-victory recreated wake records only exact new birth and checkpoint0");x.f.awake_mask=mask|1;}
  else need(!x.wake()&&!x.control.take_wakeup(),"Recreated already-awakened No1 never earns a second activation");
  for(unsigned phase:{2u,7u}){x.c.r4=n::base(Fixture::actor);x.c.r5=phase;native_checkpoint::func_805A6730(x.ram,&x.c);need(x.control.suppress_warp(x.ram,&x.c,x.f),"Recreated post-victory No1 still blocks both native free-warp states");}
 }
 for(unsigned fault=0;fault<6;++fault){Fixture x(rom,0,1,1);x.f.fighting=false;x.w(Fixture::record,h::source_packed);x.c.r4=h::source_packed;x.c.r5=x.word(Fixture::record+4);x.c.r6=x.word(Fixture::record+8);x.c.r16=n::base(Fixture::record);x.c.r31=n::base(0x8005A010);
  if(fault==0)x.f.first_fight=false;if(fault==1)x.f.room=43;if(fault==2)x.f.completed=2;if(fault==3)x.c.r16+=16;if(fault==4)x.c.r31+=4;if(fault==5)x.w(Fixture::record,h::source_packed|1);
  const auto before=x.mem;need(!h::select_spawn(x.ram,&x.c,x.f).owned&&x.mem==before,"Post-victory rebind requires first encounter, source room, completed1 and exact proven native source");
 }
 for(unsigned ra:{0x6D0u,0x514u,0x564u}){Fixture x(rom,0);need(x.init(),"Sleeping No1 exact identity bound");x.f.completed=1;x.f.fighting=false;x.f.victory=true;
  need(x.wake(ra),"True native5628 first wake remains available after first victory at every admitted caller");const auto receipt=x.control.take_wakeup();need(receipt&&receipt->completed==0&&receipt->actor==Fixture::actor&&receipt->birth==x.control.snapshot().binding.birth,"Victory wake delivers checkpoint0 from exact native birth only");need(x.f.completed==1&&x.f.victory&&!x.f.fighting&&x.word(h::count_address)==1&&x.word(h::flags_address)==1,"Native victory wake sets only actual bit0 while preserving victory/completed1");x.f.awake_mask=1;need(!x.wake(ra)&&!x.control.take_wakeup(),"Victory wake cannot repeat or grant another activation");
  for(unsigned phase:{2u,7u}){x.c.r4=n::base(Fixture::actor);x.c.r5=phase;native_checkpoint::func_805A6730(x.ram,&x.c);need(x.control.suppress_warp(x.ram,&x.c,x.f),"Post-victory newly-awakened No1 still blocks free transport2/7");}
 }
 for(unsigned fault=0;fault<16;++fault){Fixture x(rom,0);need(x.init(),"Post-victory wake guard negative initialized");x.f.completed=1;x.f.fighting=false;x.f.victory=true;x.c.r31=n::base(Fixture::overlay+0x6D0);
  if(fault==0)x.f.first_fight=false;if(fault==1)x.f.room=83;if(fault==2)x.f.completed=2;if(fault==3)x.f.fighting=true;if(fault==4)x.c.r31+=4;if(fault==5)x.c.r4+=4;if(fault==6)x.f.awake_mask=1;if(fault==7)x.b(Fixture::actor+0x30,1);if(fault==8)x.w(Fixture::actor+0x20,Fixture::overlay+0xC1C);if(fault==9)x.control.forget(Fixture::actor);if(fault==10)x.w(Fixture::actor+0x1DC,h::source_packed|1);if(fault==11)x.w(h::count_address,1);if(fault==12)x.f.victory=false;if(fault==13)x.f.overlay=0;if(fault==14)x.w(0x800F8828,Fixture::actor);if(fault==15)x.f.vi=0;
  const auto before=x.mem;need(!x.control.wake_enter(x.ram,&x.c,x.f)&&!x.control.take_wakeup()&&x.mem==before,"Victory wake refuses foreign phase/encounter/source/state/caller/lifetime/player/flags without writes");
 }
 {Fixture x(rom);need(x.init(),"Address reuse fixture initialized");const auto old=x.control.snapshot().binding.birth;x.control.forget(Fixture::actor);x.w(Fixture::actor+0x20,Fixture::overlay+h::initializer_offset);need(x.init()&&x.control.snapshot().binding.birth>old,"Same address receives distinct later birth");}
 need(rom==original,"Original source ROM bytes remain unchanged");std::cout<<"BossCheckpointActor0260Checks: "<<checks<<" PASS; nativeInits="<<native_inits<<" nativeWakes="<<native_wakes<<" nativeWakeSounds="<<sounds<<" numberedPaletteCalls="<<palette_calls<<" sourceSpawns="<<source_spawns<<" nullSourceSkips="<<suppressed_spawns<<" nativeDestructors="<<native_destructors<<"; no GPU or save claim.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
