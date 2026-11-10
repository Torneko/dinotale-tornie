#include "recomp.h"
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <vector>
namespace {unsigned checks=0,native_inits=0,native_losses=0,native_rescues=0,find_resets=0,audio_calls=0,scene_calls=0;void need(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}template<class T>T& access(uint8_t* ram,uint64_t a,uint64_t b,unsigned flip){const uint32_t p=uint32_t(a+b);if(p<0x80000000||p>0x80800000-sizeof(T))throw std::runtime_error("Hard native RAM bounds");return *reinterpret_cast<T*>(ram+((p-0x80000000)^flip));}}
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
#include "../story_hard_native0330.hpp"
namespace hard=workshop::story_hard_native0330;
namespace {hard::Adapter adapter;hard::Team team;unsigned selected=16,high_before=0;std::optional<unsigned> lost,rescued;bool projected=false,rescue_armed=false;constexpr uint32_t root=0x800F7EA0,player=0x80100000,data=0x80110000,stack=0x807D0000;}
void workshop_find_reset(){++find_resets;}
int workshop_metal_death(uint8_t* rdram,recomp_context* c){return adapter.loss_enter(rdram,c,team);}
void workshop_metal_remaining(uint8_t* rdram,recomp_context* c){c->r12|=uint32_t(MEM_W(0,int32_t(0x807F1000)))&0xC0;}
void workshop_metal_rescued(uint8_t* rdram,recomp_context* c){rescue_armed=adapter.rescue_enter(rdram,c,team);}
void workshop_807F1A00(uint8_t*,recomp_context* c){c->r2=selected;}
void func_8006E0FC(uint8_t*,recomp_context* c){++scene_calls;c->r2=1;}
void func_8002AEE4(uint8_t*,recomp_context*){}
void func_80067878(uint8_t*,recomp_context*){++audio_calls;}
void hard_init(uint8_t* rdram,recomp_context* c){high_before=uint32_t(MEM_W(0,int32_t(0x800F8DF4)))&~255u;projected=adapter.init_projection(rdram,c,team);}
void hard_loss_finish(uint8_t* rdram,recomp_context* c){lost=adapter.loss_finish(rdram,c);}
void hard_rescue_finish(uint8_t* rdram,recomp_context* c){rescued=adapter.rescue_finish(rdram,c);}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmicrosoft-goto"
#pragma clang diagnostic ignored "-Wunused-label"
#include HARD_NATIVE_BODY
#pragma clang diagnostic pop
namespace {
void word(uint8_t* rdram,uint32_t p,uint32_t v){MEM_W(0,int32_t(p))=v;}
uint32_t word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,int32_t(p)));}
void byte(uint8_t* rdram,uint32_t p,unsigned v){MEM_B(0,int32_t(p))=uint8_t(v);}
void half(uint8_t* rdram,uint32_t p,unsigned v){MEM_H(0,int32_t(p))=uint16_t(v);}
unsigned ram_team(uint8_t* rdram){return (word(rdram,0x800F8DF4)&255)|((word(rdram,0x807F1000)&255)<<8);}
void setup(uint8_t* rdram,hard::Team t={0xFFFF,0xFFFF}){std::memset(rdram,0,0x800000);team=t;adapter.start(6,true,88);word(rdram,player,0x19971110);half(rdram,player+12,0x4001);word(rdram,player+0x1F4,data);word(rdram,0x800F8828,player);word(rdram,0x800F8DF4,0xA5A54000);need(adapter.project(rdram,t),"Team fixture projection failed");lost.reset();rescued.reset();projected=rescue_armed=false;}
recomp_context context(){recomp_context c{};c.r29=gpr(int32_t(stack));c.r4=gpr(int32_t(player));return c;}
}
int main(){try{
 std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
 need(hard::valid({1,1})&&hard::valid({1,0})&&hard::valid({0xFFFF,0xFFFF}),"Valid teams rejected");for(auto t:std::array<hard::Team,5>{{{0,0},{2,2},{1,2},{0x10001,1},{1,0x10000}}})need(!hard::valid(t),"Invalid team accepted");
 // Pure capacity/picker property: every subset of the16 owned colors yields
 // exactly its dead local colors, never an alive or unacquired Yoshi.
 for(unsigned alive=0;alive<65536;alive++){const hard::Team t{0xFFFF,alive};const unsigned missing=(~alive)&0xFFFF,count=unsigned(std::popcount(missing));need(hard::dead(t)==missing,"Dead16 field mismatch");unsigned found=0;for(unsigned n=0;n<count;n++){const auto c=hard::rescue_pick(t,n);need(c<16&&(missing&(1u<<c))&&!(found&(1u<<c)),"Picker granted wrong or duplicate Yoshi");found|=1u<<c;}need(found==missing&&hard::rescue_pick(t,count)==16,"Picker capacity/sentinel mismatch");}
 setup(rdram,{1,1});need(adapter.capacity(rdram,team)==0&&adapter.picker(rdram,team,0)==16,"Green-only run granted a free classic Yoshi");
 // Execute the real canonical base-lives initializer, then our exact return
 // fence. Projection overrides only low8 bits and the separate extra word.
 for(bool continuing:{false,true})for(auto t:std::array<hard::Team,4>{{{1,1},{0xFFFF,0x9009},{0xC001,0x8001},{1,0}}}){
  setup(rdram,t);half(rdram,0x800F9254,continuing?1:0);byte(rdram,0x800F9253,continuing?8:0);word(rdram,0x800F8E58,0x30000000);auto c=context();c.r3=gpr(int32_t(root));++native_inits;workshop_804445A4(rdram,&c);need(projected&&ram_team(rdram)==t.alive,"Actual native base init overrode local Hard team");need((word(rdram,0x800F8DF4)&~255u)==high_before,"Init changed high native carry flags");need(adapter.continuing(rdram)==continuing,"Native continuation predicate mismatch");
 }
 for(unsigned color=0;color<16;color++){
  setup(rdram);const auto flags=word(rdram,0x800F8DF4)&~255u;word(rdram,0x807F0008,color);auto c=context();++native_losses;workshop_807F1100(rdram,&c);need(lost&&*lost==color,"Actual native loss fence missing exact PC color");need(ram_team(rdram)==(0xFFFFu&~(1u<<color)),"Native alias loss cleared wrong life");need((word(rdram,0x800F8DF4)&~255u)==flags,"Loss changed high carry flags");need(word(rdram,0x807F1004)==1&&word(rdram,0x807F1008)==color,"Native death counters not preserved");need(!adapter.loss_finish(rdram,&c),"Repeated loss receipt admitted");
  setup(rdram,{0xFFFF,0xFFFFu&~(1u<<color)});const auto old=word(rdram,0x800F8DF4);selected=hard::rescue_pick(team,0);need(selected==color,"Local dead picker color mismatch");c=context();++native_rescues;workshop_807F1C00(rdram,&c);need(rescue_armed&&rescued&&*rescued==color&&ram_team(rdram)==0xFFFF,"Actual native rescue did not restore PC color");const unsigned expected_flags=(old&~0x1C0FFu)|(((((old>>14)&7)-1)&7)<<14)|255;need(word(rdram,0x800F8DF4)==expected_flags,"Native White Shy Guy count or flags changed unexpectedly");need(word(rdram,0x807F1E00)==1&&word(rdram,0x807F1E04)==color,"Native rescue counters not preserved");need(!adapter.rescue_finish(rdram,&c),"Repeated rescue receipt admitted");
 }
 // Strict external mode/scope gates preserve old Story and StoryHybrid.
 for(unsigned mode=0;mode<9;mode++)if(mode!=6){setup(rdram);adapter.start(mode,true,88);const auto before=memory;auto c=context();c.r9=0;need(!adapter.project(rdram,{1,1})&&!adapter.loss_enter(rdram,&c,team)&&!adapter.capacity(rdram,team)&&!adapter.picker(rdram,team,0),"Wrong mode enabled Hard adapter");need(memory==before,"Wrong mode mutated RAM");}
 for(unsigned gate=0;gate<3;gate++){setup(rdram);adapter.start(6,gate!=0,gate==1?0:88);if(gate==2)word(rdram,0x800F8B88,1);const auto before=memory;need(!adapter.project(rdram,team)&&!adapter.available(rdram,team),"Missing Hard scope/nonce/nativeStory guard");need(memory==before,"Disabled adapter changed RAM");}
 for(unsigned bad=0;bad<10;bad++){setup(rdram);word(rdram,0x807F0008,3);auto c=context();c.r9=3;if(bad==0)c.r4=0;if(bad==1)c.r29=stack+1;if(bad==2)c.r9=16;if(bad==3)word(rdram,0x807F0008,2);if(bad==4)word(rdram,player,0);if(bad==5)half(rdram,player+12,0x4002);if(bad==6)word(rdram,player+0x1F4,0x807FF000);if(bad==7)word(rdram,0x800F8828,player+4);if(bad==8)word(rdram,0x807F1000,0);if(bad==9)team.alive&=~8u;const auto before=memory;const auto regs=c;need(!adapter.loss_enter(rdram,&c,team),"Invalid loss context admitted");need(memory==before&&std::memcmp(&c,&regs,sizeof(c))==0,"Rejected loss mutated RAM/registers");}
 for(unsigned bad=0;bad<4;bad++){setup(rdram);word(rdram,0x807F0008,14);auto c=context();c.r9=14;need(adapter.loss_enter(rdram,&c,team),"Valid loss arm failed");if(bad==0)c.r29=stack+4;if(bad==1)word(rdram,player+0x1F4,data+4);if(bad==2)word(rdram,0x807F1000,255);if(bad==3)adapter.start(6,true,89);need(!adapter.loss_finish(rdram,&c),"Mismatched loss confirmation accepted");}
 for(unsigned bad=0;bad<6;bad++){setup(rdram,{0xFFFF,0xFFFFu&~(1u<<15)});auto c=context();c.r2=15;c.r8=0x80100000;c.r10=16;if(bad==0)c.r2=14;if(bad==1)c.r2=16;if(bad==2)c.r29=0;if(bad==3)c.r8=0;if(bad==4)c.r10=15;if(bad==5)team.found=1;const auto before=memory;need(!adapter.rescue_enter(rdram,&c,team),"Invalid rescue context admitted");need(memory==before,"Rejected rescue mutated RAM");}
 // Read-only14-byte continuation and native save-success fence.
 setup(rdram);for(unsigned i=0;i<14;i++)byte(rdram,0x800F9248+i,0x21+i);auto c=context();const auto before=memory;auto continuation=hard::read_continuation(rdram);need(continuation&&continuation->front()==0x21&&continuation->back()==0x2E,"Native continuation14 mapping");need(adapter.saved_continuation(rdram,&c)==continuation,"Save return0 rejected");c.r2=1;need(!adapter.saved_continuation(rdram,&c),"Save failure produced continuation receipt");c.r2=0;c.r29=0;need(!adapter.saved_continuation(rdram,&c)&&!hard::read_continuation(nullptr),"Invalid continuation context admitted");need(memory==before,"Continuation reader wrote RAM");
 // Compose the frozen world observer under strictPC6. Frame0→30 is local
 // proof only; no journal or persisted world claim is made by this test.
 setup(rdram,{1,1});byte(rdram,0x800F8DFC,0);byte(rdram,0x800F8DFD,0);c=context();c.r4=gpr(int32_t(root));adapter.frame(rdram,&c);word(rdram,0x800F89D0,30);word(rdram,0x800F8AC8,1);adapter.frame(rdram,&c);need(adapter.witness().finished&&adapter.witness().run_nonce==88,"Frozen observer not composed forHard");adapter.start(5,true,88);adapter.frame(rdram,&c);need(!adapter.witness().armed,"Hard wrapper leaked intoStoryHybrid");
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"nativeInitCalls\":"<<native_inits<<",\"nativeLossCalls\":"<<native_losses<<",\"nativeRescueCalls\":"<<native_rescues<<",\"sceneStubCalls\":"<<scene_calls<<",\"soundStubCalls\":"<<audio_calls<<",\"findResetStubCalls\":"<<find_resets<<",\"gpu\":false,\"game\":false}"<<std::endl;return 0;
 }catch(const std::exception& error){std::cerr<<"FAIL after "<<checks<<" checks: "<<error.what()<<std::endl;return 1;}}
