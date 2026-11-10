#include "recomp.h"
#include <array>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <vector>
namespace {
unsigned checks=0,native_world_calls=0,native_saves=0,transition_calls=0,native_tallies=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
template<class T>T& access(uint8_t* ram,uint64_t a,uint64_t b,unsigned flip){const uint32_t address=uint32_t(a+b);if(address<0x80000000||address>0x80800000-sizeof(T))throw std::runtime_error("Native RAM bounds");return *reinterpret_cast<T*>(ram+((address-0x80000000)^flip));}
}
#undef MEM_W
#undef MEM_H
#undef MEM_HU
#undef MEM_B
#undef MEM_BU
#define MEM_W(a,b) access<int32_t>(rdram,uint64_t(a),uint64_t(b),0)
#define MEM_H(a,b) access<int16_t>(rdram,uint64_t(a),uint64_t(b),2)
#define MEM_HU(a,b) access<uint16_t>(rdram,uint64_t(a),uint64_t(b),2)
#define MEM_B(a,b) access<int8_t>(rdram,uint64_t(a),uint64_t(b),3)
#define MEM_BU(a,b) access<uint8_t>(rdram,uint64_t(a),uint64_t(b),3)
#include "../story_observer0320.hpp"
namespace so=workshop::story_observer0320;
extern "C" void switch_error(const char*,uint32_t,uint32_t){std::abort();}
static int32_t section_storage[300]{};
int32_t* section_addresses=section_storage;
static so::NativeObserver observer;
static std::optional<unsigned> result,bowser_result;
static unsigned save_return=0;
void func_80710B24(uint8_t*,recomp_context*);
void func_80710860(uint8_t*,recomp_context*);
void func_8006E0FC(uint8_t*,recomp_context*);
void func_8006E06C(uint8_t*,recomp_context*);
void func_8006DFC4(uint8_t*,recomp_context*);
void func_807106C8(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected active fade subpath");}
void func_807106E4(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected active fade subpath");}
void func_80710A64(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected page5 world save");}
void func_800409F0(uint8_t*,recomp_context*){++native_tallies;}
void func_80068CF8(uint8_t*,recomp_context* context){++transition_calls;context->r2=0;}
void func_8006988C(uint8_t*,recomp_context* context){++native_saves;context->r2=save_return;}
void world_arm(uint8_t* rdram,recomp_context* context){observer.arm_world_save(rdram,context);}
void world_finish(uint8_t* rdram,recomp_context* context){result=observer.finish_world_save(rdram,context);}
void bowser_arm(uint8_t* rdram,recomp_context* context){observer.arm_bowser(rdram,context);}
void bowser_finish(uint8_t* rdram,recomp_context* context){bowser_result=observer.finish_bowser(rdram,context);}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmicrosoft-goto"
#pragma clang diagnostic ignored "-Wunused-label"
#include WORLD_NATIVE_INC
#pragma clang diagnostic pop
namespace {
constexpr uint32_t root=0x800F7EA0,player=0x80100000,data=0x80110000,board=0x80200000,board_data=0x80210000,fade_owner=0x80220000,fade=0x80230000,boss=0x80300000,stack=0x807E0000;
uint32_t word(uint8_t* rdram,uint32_t address){return uint32_t(MEM_W(0,int32_t(address)));}
void word(uint8_t* rdram,uint32_t address,uint32_t value){MEM_W(0,int32_t(address))=value;}
void byte(uint8_t* rdram,uint32_t address,unsigned value){MEM_B(0,int32_t(address))=uint8_t(value);}
void half(uint8_t* rdram,uint32_t address,unsigned value){MEM_H(0,int32_t(address))=uint16_t(value);}
void setup(uint8_t* rdram,unsigned world,unsigned column){
 std::memset(rdram,0,0x800000);word(rdram,player,0x19971110);half(rdram,player+12,0x4001);word(rdram,player+0x1F4,data);word(rdram,0x800F8828,player);byte(rdram,0x800F8DFC,world);byte(rdram,0x800F8DFD,column);word(rdram,root+0xF54,63);
 word(rdram,board,0x19971110);half(rdram,board+12,0x41DF);word(rdram,board+0x1F4,board_data);word(rdram,board_data+0x14,fade_owner);word(rdram,fade_owner+0x16C,fade);byte(rdram,fade+0x10,0xF8);save_return=0;result.reset();
}
recomp_context manager(){recomp_context context{};context.r4=gpr(int32_t(root));return context;}
void frame(uint8_t* rdram,unsigned fruits,unsigned ending){word(rdram,0x800F89D0,fruits);word(rdram,0x800F8AC8,ending);auto context=manager();observer.frame(rdram,&context);}
void proof(uint8_t* rdram){frame(rdram,0,0);frame(rdram,30,1);check(observer.witness().finished,"Real-layout winner witness not captured");}
void save_world(uint8_t* rdram){recomp_context context{};context.r4=gpr(int32_t(board));context.r29=gpr(int32_t(stack));++native_world_calls;func_80710B24(rdram,&context);}
void direct_world_setup(uint8_t* rdram,recomp_context& context,unsigned world,unsigned column){context={};context.r29=gpr(int32_t(stack));byte(rdram,stack+0x3B,world);byte(rdram,stack+0x3A,column);byte(rdram,root+1,0);byte(rdram,root+0x57D,4);word(rdram,root+0xF60,1);byte(rdram,0x800F9255,world+1);word(rdram,0x800F8E5C,1u<<(world*4+column));}
void bowser_setup(uint8_t* rdram,recomp_context& context){context={};context.r16=gpr(int32_t(boss));context.r29=gpr(int32_t(stack));word(rdram,boss,0x19971110);half(rdram,boss+12,0x41A4);word(rdram,boss+0x1E4,55);}
void transition_bowser(uint8_t* rdram,recomp_context& context){context.r4=0;context.r5=0;context.r6=0x4C;func_8006E0FC(rdram,&context);}
}
int main(){try{
 std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();section_storage[240]=int32_t(0x8070F3F0);
 // The REAL original native fade/advance/continuation/EEPROM call bodies run
 // for every ordinary world and column. Only EEPROM I/O and scheduling are
 // stubs; observer hooks instrument their actual before/after PCs.
 for(unsigned world=0;world<5;world++)for(unsigned column=0;column<4;column++){
  setup(rdram,world,column);observer.start(5,true,123);proof(rdram);const auto witness=observer.witness();word(rdram,player,0);word(rdram,0x800F8828,0);save_world(rdram);
  check(result&&*result==world,"Real native world-save body not observed");check(word(rdram,0x800F8E5C)==(1u<<(world*4+column)),"Real native Story clear bit mismatch");check((MEM_BU(0,int32_t(0x800F9255))&7)==world+1,"Real native continuation mapping mismatch");check(observer.witness().player==witness.player&&observer.witness().private_data==witness.private_data,"Native cleanup lost captured identity");save_world(rdram);check(!result,"Repeated native save yielded duplicate event");
 }
 for(unsigned mode=0;mode<8;mode++)if(mode!=5){setup(rdram,0,0);observer.start(mode,true,55);frame(rdram,0,0);frame(rdram,30,1);save_world(rdram);check(!result,"Wrong session mode observed as StoryHybrid");}
 for(unsigned gate=0;gate<2;gate++){setup(rdram,0,0);observer.start(5,gate!=0,gate?0:55);frame(rdram,0,0);frame(rdram,30,1);save_world(rdram);check(!result,"No ordinary scope or nonce admitted");}
 for(unsigned fruits=0;fruits<33;fruits++){setup(rdram,0,0);observer.start(5,true,55);frame(rdram,0,0);frame(rdram,fruits,1);save_world(rdram);check(bool(result)==(fruits==30),"Wrong fruit count admitted");}
 for(unsigned ending=0;ending<6;ending++){setup(rdram,1,2);observer.start(5,true,55);frame(rdram,0,0);frame(rdram,30,ending);save_world(rdram);check(bool(result)==(ending==1),"Wrong ending state admitted");}
 setup(rdram,0,0);observer.start(5,true,55);frame(rdram,30,1);save_world(rdram);check(!result,"Missing initial zero-fruit witness accepted");
 for(unsigned invalid=0;invalid<6;invalid++){
  setup(rdram,0,0);observer.start(5,true,55);auto context=manager();
  if(invalid==0)context.r4=0;if(invalid==1)word(rdram,player,0);if(invalid==2)half(rdram,player+12,0x4002);if(invalid==3)word(rdram,player+0x1F4,0x807FF000);if(invalid==4)word(rdram,0x800F8828,0x80800000);if(invalid==5)word(rdram,0x800F8828,player+1);
  observer.frame(rdram,&context);frame(rdram,30,1);save_world(rdram);check(!result,"Invalid initial manager/player/private accepted");
 }
 // Legitimate room reload can replace player/private while retaining the
 // level attempt; only the final valid identity binds the winning witness.
 setup(rdram,2,3);observer.start(5,true,77);frame(rdram,0,0);word(rdram,0x800F8828,player+0x400);word(rdram,player+0x400,0x19971110);half(rdram,player+0x40C,0x4001);word(rdram,player+0x5F4,data+0x2000);frame(rdram,13,0);frame(rdram,30,1);check(observer.witness().player==player+0x400&&observer.witness().private_data==data+0x2000,"Final room player identity not bound");save_world(rdram);check(result&&*result==2,"Room replacement invalidated real level attempt");
 setup(rdram,1,0);observer.start(5,true,33);proof(rdram);save_return=1;save_world(rdram);check(!result,"Failed native EEPROM save yielded reward");
 // All mutations occur BETWEEN real observed victory and commit. No bad
 // stack/transition/continuation may consume a successful event.
 for(unsigned defect=0;defect<12;defect++){
  setup(rdram,3,1);observer.start(5,true,33);proof(rdram);recomp_context context{};direct_world_setup(rdram,context,3,1);
  if(defect==0)context.r29=0;if(defect==1)context.r29=stack+1;if(defect==2)byte(rdram,stack+0x3B,2);if(defect==3)byte(rdram,stack+0x3A,2);if(defect==4)byte(rdram,root+1,7);if(defect==5)byte(rdram,root+0x57D,9);if(defect==6)byte(rdram,0x800F9255,3);if(defect==7)word(rdram,root+0xF60,0);
  observer.arm_world_save(rdram,&context);
  if(defect==8)context.r29=stack+4;if(defect==9)word(rdram,0x800F8E5C,0);if(defect==10)context.r2=1;if(defect==11)observer.reset();
  check(!observer.finish_world_save(rdram,&context),"Malformed world commit accepted");
 }
 setup(rdram,1,0);observer.start(5,true,33);proof(rdram);frame(rdram,0,0);save_world(rdram);check(!result,"Stale victory survived fresh same-level attempt");
 // Ordinary world save NEVER yields final world5; only genuine Bowser's
 // actor identity,counter55 and exact native pending scene4C can do so.
 for(unsigned defect=0;defect<13;defect++){
  setup(rdram,5,2);observer.start(5,true,444);proof(rdram);recomp_context context{};bowser_setup(rdram,context);
  if(defect==1)half(rdram,boss+12,0x41A3);if(defect==2)word(rdram,boss,0);if(defect==3)word(rdram,boss+0x1E4,54);if(defect==4)word(rdram,boss+0x1E4,56);if(defect==5)context.r16=0x80800000;if(defect==6)context.r29=stack+1;
  observer.arm_bowser(rdram,&context);transition_bowser(rdram,context);
  if(defect==7)context.r16=boss+4;if(defect==8)context.r29=stack+4;if(defect==9)byte(rdram,root+0x57D,4);if(defect==10)context.r2=0;if(defect==11)word(rdram,boss+0x1E4,56);if(defect==12)observer.start(5,true,445);
  auto won=observer.finish_bowser(rdram,&context);check(bool(won)==(defect==0),"Bowser proof identity/transition mismatch");if(won)check(*won==5,"Final world incorrect");check(!observer.finish_bowser(rdram,&context),"Repeated Bowser event accepted");
 }
 setup(rdram,5,0);observer.start(5,true,33);proof(rdram);recomp_context context{};direct_world_setup(rdram,context,5,0);observer.arm_world_save(rdram,&context);check(!observer.finish_world_save(rdram,&context),"Normal score commit admitted final world");
 // Execute the actual native counter55/tally/transition/counter-increment
 // slice with NO fruits/ending witness. Bowser victory is its own proof.
 setup(rdram,5,1);observer.start(5,true,77);frame(rdram,0,0);bowser_setup(rdram,context);bowser_result.reset();native_bowser_defeat_slice(rdram,&context);check(bowser_result&&*bowser_result==5,"Real Bowser defeat slice failed with0 fruit/ending0");check(word(rdram,boss+0x1E4)==56&&native_tallies==1,"Real Bowser counter/tally sequence mismatch");check(word(rdram,0x800F89D0)==0&&word(rdram,0x800F8AC8)==0,"Native defeat slice invented30-fruit evidence");
 setup(rdram,5,1);observer.start(5,true,77);bowser_setup(rdram,context);bowser_result.reset();native_bowser_defeat_slice(rdram,&context);check(!bowser_result,"Bowser without real initial player attempt admitted");
 // Adapter methods only read their supplied RDRAM/registers.
 setup(rdram,0,0);observer.start(5,true,22);proof(rdram);direct_world_setup(rdram,context,0,0);const auto ram_before=memory;const auto context_before=context;observer.arm_world_save(rdram,&context);check(observer.finish_world_save(rdram,&context)==0,"Direct valid world event missing");check(memory==ram_before&&std::memcmp(&context,&context_before,sizeof(context))==0,"Observer mutated RDRAM or registers");
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"actualNativeWorldCalls\":"<<native_world_calls<<",\"stubEepromCalls\":"<<native_saves<<",\"stubSchedulingCalls\":"<<transition_calls<<",\"nativeBowserTallyStubs\":"<<native_tallies<<",\"gpu\":false,\"gameplay\":false}"<<std::endl;return 0;
 }catch(const std::exception& error){std::cerr<<"FAIL after "<<checks<<" checks: "<<error.what()<<std::endl;return 1;}}
