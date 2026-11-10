#include "../boss_retry_placements0281.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <vector>
#ifndef BOSS_RETRY_NATIVE_SLICE
#error Supply unchanged native getter, setter, consumer and cached loader bodies.
#endif
namespace g=workshop::boss_retry_placements0281;namespace n=workshop::boss_native;namespace b=workshop::boss_battle0250;namespace rooms=workshop::boss_rooms_data0250;
static unsigned checks=0;static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
extern "C" void do_break(uint32_t){std::abort();}
namespace actual {
recomp_func_t* lookup(int32_t);
void func_8006D59C(uint8_t*,recomp_context*){throw std::runtime_error("Cache miss must not allocate in this oracle");}
void func_8006D85C(uint8_t*,recomp_context*){throw std::runtime_error("Cache hit must not reload consumed rows");}
void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected native assertion");}
#undef LOOKUP_FUNC
#define LOOKUP_FUNC(v) lookup(int32_t(v))
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#include BOSS_RETRY_NATIVE_SLICE
#pragma clang diagnostic pop
recomp_func_t* lookup(int32_t a){if(uint32_t(a)==0x8006D6E4)return func_8006D6E4;throw std::runtime_error("Foreign native callback");}
}
struct Fixture {
 static constexpr uint32_t cache=0x80100000,registry=0x80020000,resource=0x80030000,info=0x80040000,stack=0x807A0000;
 std::vector<uint8_t> bytes=std::vector<uint8_t>(0x800000);uint8_t* r=bytes.data();uint8_t* rdram=r;g::Guard guard;recomp_context c{};b::Plan plan=b::make_plan(196893949495000ULL);unsigned index=6;rooms::Arena arena{};b::Encounter encounter{};unsigned setters=0,getters=0;
 Fixture(){rooms::configure(plan);need(plan.fights[index].family==b::Family::Monsters,"Personal failing seed really has Monsters7");arena=rooms::arena(rooms::source_for_encounter(index));encounter=plan.fights[index];
  word(0x800F8450,registry);half(0x800F8448,11);word(registry+24,cache);byte(cache,1);byte(cache+1,arena.source);word(cache+0x60,cache+0x128);word(cache+0x128,cache+0x12C);half(cache+0x12C,unsigned(arena.objects.size()/12));word(cache+0x130,table());
  for(size_t p=0;p<arena.objects.size();p+=12)for(unsigned j=0;j<3;++j)word(table()+unsigned(p/12)*16+j*4,workshop::u32(arena.objects,p+j*4));
  for(unsigned i=0;i<2;++i){word(actor(i),0x19971110);half(actor(i)+12,0x41C6);half(actor(i)+14,i);byte(actor(i)+0x17D,2);word(actor(i)+0x168,resource+i*0x100);}
  word(0x800F8DB4,5);word(0x800F89D0,12345);word(0x807F1000,0x15);c.r29=n::base(stack);c.r31=0x8005A050;c.r16=0x88776655;c.f2.u32l=0xAABBCCDD;
 }
 uint32_t actor(unsigned i=0)const{return 0x80010000+i*0x1000;}uint32_t table()const{return cache+0x134;}uint32_t record(unsigned i=0)const{return table()+unsigned(arena.boss_indices[i])*16;}
 void word(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}void half(uint32_t a,unsigned v){MEM_H(0,n::base(a))=uint16_t(v);}void byte(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}
 void arguments(unsigned i=0){c.r4=n::base(actor(i));c.r5=n::base(record(i));c.r31=n::base(0x8005A050);}
 void capture(unsigned i=0,unsigned epoch=0){arguments(i);auto before=bytes;need(guard.capture(r,&c,arena,encounter,index,epoch)&&bytes==before,"Pristine placement capture is read-only at real48E9C entry");}
 void consume(unsigned i=0){arguments(i);actual::func_80048E9C(r,&c);need(n::word(r,record(i))==0xFFFFFFFF&&n::word(r,actor(i)+0x1D8)==record(i)&&n::word(r,actor(i)+0x1DC)==workshop::u32(arena.objects,size_t(arena.boss_indices[i])*12),"Actual48E9C/48BD8/6D340 consumes original and attaches respawn identity");}
 void retire(unsigned i=0){byte(actor(i)+0x17D,3);byte(actor(i)+0x18,0xFF);}
 void ready(unsigned epoch=1,unsigned cp=6){capture();consume();capture(1);consume(1);retire();retire(1);need(guard.arm(epoch,cp,8),"Checkpoint retry arms only captured replay range");fence_arguments();}
 void fence_arguments(){c.r4=n::base(info);c.r5=arena.source;c.r31=n::base(0x8006DDFC);c.r29=n::base(stack);}
 g::Fence fence(unsigned epoch=1)const{return {arena.source,index,epoch,40000,b::Phase::ToFight};}
 void dispatch(uint32_t fn,uint8_t* ram,recomp_context* state){if(fn==0x8006D6F8){++getters;actual::func_8006D6F8(ram,state);}else if(fn==0x8006D34C){++setters;actual::func_8006D34C(ram,state);}else throw std::runtime_error("Unexpected restoration service");}
 bool restore(g::Fence f,g::Receipt& p){return guard.restore(r,&c,arena,encounter,f,p,[&](uint32_t fn,uint8_t* ram,recomp_context* state){dispatch(fn,ram,state);});}
};
static void same_except_stack(const std::vector<uint8_t>& before,const std::vector<uint8_t>& after){auto a=before,z=after;for(uint32_t p=Fixture::stack-0x200;p<Fixture::stack+0x20;++p)a[p&0x7FFFFF]=z[p&0x7FFFFF];need(a==z,"Entire8MiB unchanged except native reserved scratch and explicitly expected packed words");}
int main(){try{
 for(unsigned checkpoint:{0u,3u,6u}){Fixture f;f.ready(1,checkpoint);
  // Demonstrate the actual native cache-hit bug before restoration, without
  // calling any constructor, death logic, ROM IO, or forced completion.
  auto call=f.c;actual::func_8006DA0C(f.r,&call);need(n::word(f.r,Fixture::info+0x20)==Fixture::cache&&n::word(f.r,f.record())==0xFFFFFFFF,"Actual6DA0C cache hit preserves the consumed Monsters record");
  const auto before=f.bytes;const auto registers=f.c;g::Receipt receipt;need(f.restore(f.fence(),receipt),"Bounded replay dispatches actual native getter and placement setter");need(receipt.count==2&&receipt.epoch==1&&receipt.index==6&&receipt.room==173&&receipt.table==f.table()&&f.setters==2&&f.getters==1,"Exactly two original41C6 words restored for replay7");
  auto expected=before;
  for(unsigned i=0;i<2;++i){const uint32_t value=workshop::u32(f.arena.objects,size_t(f.arena.boss_indices[i])*12);std::memcpy(expected.data()+(f.record(i)&0x7FFFFF),&value,4);}
  same_except_stack(expected,f.bytes);need(!std::memcmp(&registers,&f.c,sizeof(registers)),"Caller registers including FP preserved");need(!f.guard.pending(6)&&f.guard.snapshot()[6].restored_epoch==1,"Native readback closes only the matching replay epoch");
  const auto count=f.setters;need(f.restore(f.fence(),receipt)&&f.setters==count&&!receipt.epoch,"Duplicate scene entry cannot restore twice");
  f.capture(1,1);f.consume(1);f.capture(0,1);f.consume(0);f.retire();f.retire(1);need(f.guard.arm(2,6,8),"Another genuine retry has a new epoch");f.fence_arguments();need(f.restore(f.fence(2),receipt)&&receipt.count==2,"Replayed native consumption can be restored again only after another retry");
 }
 {Fixture f;f.capture();f.consume();f.retire();need(f.guard.arm(1,6,8),"Partially streamed encounter is captured");f.fence_arguments();g::Receipt p;need(f.restore(f.fence(),p)&&p.count==1&&f.setters==1,"Pristine unspawned second monster is not rewritten");}
 {Fixture f;f.ready();need(f.guard.arm(2,9,9),"Checkpoint9 excludes earlier Monsters7");g::Receipt p;const auto before=f.bytes;need(f.restore(f.fence(2),p)&&!f.setters&&f.bytes==before,"Already banked encounters are never restored");}
 for(unsigned fault=0;fault<18;++fault){Fixture f;f.ready();auto fence=f.fence();
  if(fault==0)f.c.r31+=4;if(fault==1)f.c.r5=83;if(fault==2)f.c.r4=0;if(fault==3)f.c.r29=0;if(fault==4)fence.phase=b::Phase::Fighting;if(fault==5)fence.epoch=2;
  if(fault==6)f.byte(Fixture::cache+1,83);if(fault==7)f.half(0x800F8448,12);if(fault==8)f.word(Fixture::cache+0x60,0);if(fault==9)f.word(f.record()+4,0);
  if(fault==10)f.word(f.record(1),0x41C60017);if(fault==11)f.word(Fixture::registry+28,Fixture::cache);if(fault==12)f.byte(f.actor()+0x17D,2);
  if(fault==13){const auto a=0x80210000u;f.word(a,0x19971110);f.half(a+12,0x41C6);f.byte(a+0x17D,1);f.word(a+0x1D8,f.record(1));}
  if(fault==14)f.arena.logical_source=165;if(fault==15)f.arena.boss_indices={};if(fault==16)f.half(Fixture::cache+0x12C,149);if(fault==17)f.word(Fixture::registry+24,0);
  const auto before=f.bytes;const auto regs=f.c;g::Receipt p;need(!f.restore(fence,p)&&f.setters==0&&!p.epoch,"Foreign fence/cache/corrupt second record/live old actor rejected before first setter");same_except_stack(before,f.bytes);need(!std::memcmp(&regs,&f.c,sizeof(regs)),"Negative path preserves caller registers");
 }
 for(unsigned fault=0;fault<8;++fault){Fixture f;f.arguments();if(fault==0)f.c.r31+=4;if(fault==1)f.word(f.record()+8,0);if(fault==2)f.word(f.record(),0xFFFFFFFF);if(fault==3)f.byte(Fixture::cache+1,83);if(fault==4)f.word(f.actor()+0x168,0);if(fault==5)f.encounter.family=b::Family::Lips;if(fault==6)f.c.r5+=16;if(fault==7)f.byte(f.actor()+0x17D,3),f.byte(f.actor()+6,3);const auto before=f.bytes;need(!f.guard.capture(f.r,&f.c,f.arena,f.encounter,f.index,0)&&f.bytes==before,"Pristine provenance failure never captures or writes guest RAM");}
 {Fixture f;f.ready();g::Receipt p;need(!f.guard.restore(f.r,&f.c,f.arena,f.encounter,f.fence(),p,[&](uint32_t fn,uint8_t* ram,recomp_context* state){f.dispatch(fn,ram,state);if(fn==0x8006D6F8)state->r2=0;} )&&!f.setters,"Native getter must agree with the read-only registry proof");}
 {Fixture f;f.ready();g::Receipt p;need(!f.guard.restore(f.r,&f.c,f.arena,f.encounter,f.fence(),p,[&](uint32_t fn,uint8_t* ram,recomp_context* state){f.dispatch(fn,ram,state);if(fn==0x8006D34C)f.word(uint32_t(state->r4),0);} )&&f.guard.pending(6),"Native setter readback failure never emits a restoration receipt");}
 std::printf("BossRetryPlacements0281Checks: %u PASS (actual prepared native getter/cache-hit/consume/setter; HOST cache/retirement fixture; no GPU replay claim)\n",checks);return 0;
 }catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}}
