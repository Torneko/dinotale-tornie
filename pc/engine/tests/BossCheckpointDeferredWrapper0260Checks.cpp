#include "recomp.h"
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#ifndef CHECKPOINT_DEFER_NATIVE_SLICE
#error Supply the unchanged native scheduler slice.
#endif
#ifndef CHECKPOINT_DEFER_WRAPPER_SLICE
#error Supply the actual generated dispatcher wrapper.
#endif
static unsigned checks=0,enters=0,originals=0,leaves=0,movements=0,sleeps=0;
static uint64_t next_ticket=UINT64_MAX,last_ticket=0;
static recomp_context saved_entry{};
static void need(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
static int32_t base(uint32_t a){return int32_t(a);}
static constexpr uint32_t actor=0x80110000,container=0x80100000,queue=0x80101000,slots=0x80102000,empty=0x80103000,scene=0x80104000,stack=0x807A0000;
extern "C" uint64_t workshop_boss_checkpoint_init_enter(uint8_t*,const recomp_context* c){++enters;saved_entry=*c;return next_ticket;}
extern "C" void workshop_boss_checkpoint_init_leave(uint8_t*,const recomp_context* c,uint64_t ticket){++leaves;last_ticket=ticket;need(!std::memcmp(c,&saved_entry,sizeof(*c)),"Wrapper forwards entry registers to leave");}
// This initializer boundary is synthetic. Core's Actor oracle separately runs
// the real6600. Here its visible writes make accidental calls while deferred fail.
extern "C" void battle_original_func_805A6600(uint8_t* rdram,recomp_context* c){++originals;MEM_B(0x18,c->r4)=1;MEM_W(0x20,c->r4)=base(0x805A5E68);MEM_W(0x130,c->r4)=base(0x800483B8);c->r2=0x1234;}
#include CHECKPOINT_DEFER_WRAPPER_SLICE
namespace original_native {
recomp_func_t* lookup(int32_t);
void func_800448BC(uint8_t*,recomp_context*);void func_80057F24(uint8_t*,recomp_context*);void func_80057F74(uint8_t*,recomp_context*);
void func_80043E10(uint8_t*,recomp_context*);void func_80043E88(uint8_t*,recomp_context*);void func_80048EDC(uint8_t*,recomp_context*);void func_80048320(uint8_t*,recomp_context*);
void func_80048A30(uint8_t*,recomp_context*);void func_80048A84(uint8_t*,recomp_context*);void func_80048AE0(uint8_t*,recomp_context*);void func_80048B24(uint8_t*,recomp_context*);void func_800489BC(uint8_t*,recomp_context*);
void func_8004A028(uint8_t*,recomp_context*);void func_80047C38(uint8_t*,recomp_context*);void func_80065490(uint8_t*,recomp_context*);
void func_80060CF0(uint8_t*,recomp_context*);void func_800623C8(uint8_t*,recomp_context*);void func_8004700C(uint8_t*,recomp_context*);void func_800474B0(uint8_t*,recomp_context*);void Fault_HungUp(uint8_t*,recomp_context*);
void func_80047648(uint8_t*,recomp_context*);void func_80049018(uint8_t*,recomp_context*);void func_80066E88(uint8_t*,recomp_context*);void func_80066DE4(uint8_t*,recomp_context*);
#undef LOOKUP_FUNC
#define LOOKUP_FUNC(v) lookup(int32_t(v))
#include CHECKPOINT_DEFER_NATIVE_SLICE
// Legacy fixture boundaries remain explicit; the final bounded oracle below
// invokes only the unchanged callback and null130 movement gates.
void func_80060CF0(uint8_t*,recomp_context* c){need(c->r5==0,"Scheduler collision queue is empty");}
void func_800623C8(uint8_t*,recomp_context* c){need(c->r5==0,"Scheduler attack queue is empty");}
void func_8004700C(uint8_t*,recomp_context* c){c->r2=0;}
void func_800474B0(uint8_t*,recomp_context*){}
void func_80047648(uint8_t*,recomp_context*){throw std::runtime_error("Deferred checkpoint must remain unparented");}
void func_80049018(uint8_t*,recomp_context*){throw std::runtime_error("Unparented fixture must not use attached rotation");}
void func_80066E88(uint8_t*,recomp_context*){throw std::runtime_error("Unparented fixture must not use attached sine");}
void func_80066DE4(uint8_t*,recomp_context*){throw std::runtime_error("Unparented fixture must not use attached cosine");}
void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected native scheduler assertion");}
void sleep(uint8_t*,recomp_context*){++sleeps;}
void move(uint8_t*,recomp_context*){++movements;}
recomp_func_t* lookup(int32_t address){switch(uint32_t(address)){
 case 0x80048A30:return func_80048A30;case 0x80048A84:return func_80048A84;case 0x80048AE0:return func_80048AE0;case 0x80048B24:return func_80048B24;
 case 0x800489BC:return func_800489BC;
 case 0x805A6600:return ::func_805A6600;case 0x805A5E68:return sleep;case 0x800483B8:return move;
 default:throw std::runtime_error("Unexpected native scheduler indirect target");}}
}
struct Fixture {
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* rdram=memory.data();recomp_context c{};
 explicit Fixture(unsigned controls){
  w(container+0x18,queue);h(queue+6,1);w(queue+0x10,slots);w(slots,actor);w(empty+0x10,slots+8);
  w(0x800F8894,empty);w(0x800F889C,empty);w(0x80109258,scene);b(0x800F7EA5,controls);
  w(0x800A0540,0x80048A30);w(0x800A0544,0x80048A84);w(0x800A0548,0x80048AE0);w(0x800A054C,0x80048B24);
  w(actor,0x19971110);h(actor+12,0x4508);b(actor+0x10,2);b(actor+0x17D,2);b(actor+0x165,1);w(actor+0x14,0x800489BC);w(actor+0x20,0x805A6600);
  for(unsigned i=0;i<3;++i){const float pos=i==0?160.f:i==1?448.f:500.3f;fl(actor+0x3C+4*i,pos);fl(actor+0x84+4*i,pos);fl(actor+0xA8+4*i,pos);fl(actor+0x6C+4*i,1);fl(actor+0x9C+4*i,1);}
  c.r29=base(stack);c.r31=base(0x80012340);c.r16=0x12345678;
 }
 void w(uint32_t a,uint32_t value){MEM_W(0,base(a))=int32_t(value);}void h(uint32_t a,unsigned value){MEM_H(0,base(a))=uint16_t(value);}void b(uint32_t a,unsigned value){MEM_B(0,base(a))=uint8_t(value);}void fl(uint32_t a,float value){w(a,std::bit_cast<uint32_t>(value));}
 std::array<uint8_t,0x280> actor_bytes()const{std::array<uint8_t,0x280> out{};std::memcpy(out.data(),memory.data()+(actor&0x7FFFFF),out.size());return out;}
 void frame(){
  // Exercise exactly the callback and movement functions selected by448BC.
  // The incomplete full448BC fixture is archived as an explicit limitation.
  const bool controlled=MEM_BU(0,base(0x800F7EA5))!=0;c.r4=base(actor);
  if(controlled)original_native::func_80048A84(rdram,&c);else original_native::func_80048A30(rdram,&c);
  c.r4=base(actor);if(controlled)original_native::func_80048B24(rdram,&c);else original_native::func_80048AE0(rdram,&c);
  need(uint32_t(c.r29)==stack&&uint32_t(c.r31)==0x80012340,"Actual native callback/movement gates restore stack/return");
 }
};
int main(){try{
 for(unsigned controls:{0u,1u}){
  Fixture x(controls);const auto before=x.actor_bytes();const auto first_enters=enters,first_originals=originals,first_leaves=leaves,first_movements=movements;
  next_ticket=UINT64_MAX;
  for(unsigned frame=0;frame<144;++frame){x.frame();need(enters==first_enters+frame+1,"Native scheduler retries unchanged phase0 initializer every frame");need(x.actor_bytes()==before,"Deferred wrapper preserves every checkpoint actor byte");need(originals==first_originals&&leaves==first_leaves&&movements==first_movements,"Deferred frame calls neither initializer nor leave nor physics");}
  next_ticket=37;x.frame();need(originals==first_originals+1&&leaves==first_leaves+1&&last_ticket==37,"Support-ready ticket executes initializer and its matching leave once");auto* rdram=x.rdram;need(MEM_BU(0x18,base(actor))==1&&MEM_BU(0x17D,base(actor))==2,"Native boundary may initialize without changing queue ownership");
  const auto awake_enters=enters;x.frame();need(enters==awake_enters&&sleeps>0,"Next native frame uses sleep callback rather than repeating initializer");
 }
 {Fixture x(0);next_ticket=0;x.c.r4=base(actor);const auto o=originals,l=leaves;func_805A6600(x.rdram,&x.c);need(originals==o+1&&leaves==l+1&&last_ticket==0,"Unowned Vanilla ticket0 still invokes original and leave");}
 std::cout<<"Checkpoint deferred wrapper/native callback gates: "<<checks<<" PASS; 144 deferred calls for controls0/1; actual48A30/48A84/48AE0/48B24; synthetic initializer boundary; full448BC fixture NOT validated\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
