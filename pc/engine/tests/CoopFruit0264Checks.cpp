// HOST/native boundary test. Exact native contact/capture/hierarchy/ingestion
// bodies are included verbatim; scheduler, fruit-event and reward services are
// explicit HOST boundaries. This is not a native physics/playthrough claim.
#include "../coop_fruit0264.hpp"
#include "../coop_native_dispatch.hpp"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#include "../coop_runtime.hpp"
#pragma clang diagnostic pop
#include <bit>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#ifndef COOP_FRUIT_NATIVE_SLICE
#error Provide the unchanged native food corpus.
#endif
extern "C" void Fault_HungUp(uint8_t*,recomp_context*);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#include COOP_FRUIT_NATIVE_SLICE
#pragma clang diagnostic pop
namespace n=workshop::coop_collision;
namespace f=workshop::coop_fruit0264;
namespace coop=workshop::coop;
namespace d=workshop::coop::dispatch;
static unsigned checks=0,captures=0,score_calls=0,ring_calls=0,retire_calls=0;
static bool throw_capture=false,invalid_detach=false,forget_on_retire=true;
static constexpr uint32_t roots[2]{0x80100000,0x80101000},tongues[2]{0x80104000,0x80105000},privates[2]{0x80200000,0x80202000};
static constexpr uint32_t object=0x80108000,carrier=0x80109000,callback=0x80001000;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
static void w(uint8_t* rdram,uint32_t p,uint32_t value){MEM_W(0,int32_t(p))=value;}
static void h(uint8_t* rdram,uint32_t p,unsigned value){MEM_H(0,int32_t(p))=uint16_t(value);}
static void by(uint8_t* rdram,uint32_t p,unsigned value){MEM_B(0,int32_t(p))=uint8_t(value);}
static uint32_t u(uint8_t* r,uint32_t p){return n::word(r,p);}
struct Memory final:coop::ScalarMemory{uint8_t* r;explicit Memory(uint8_t* ram):r(ram){}uint32_t read(uint32_t p)override{return u(r,p);}void write(uint32_t p,uint32_t value)override{w(r,p,value);}};
static std::unique_ptr<Memory> memory;
static std::unique_ptr<coop::OwnerContext> owners;
static std::vector<unsigned> owner_stack;
extern "C" uintptr_t workshop_coop_dispatch_enter(uint8_t* r,recomp_context*,uint32_t fn,uint32_t a,uint32_t){
 const auto owner=n::owner_of_body(r,a);if(!owner)return 0;
 if(fn!=f::contact&&fn!=f::ingestion)throw std::runtime_error("Unexpected dispatch test scope");
 owners->enter(*owner);owner_stack.push_back(*owner);return *owner+1;
}
extern "C" void workshop_coop_dispatch_leave(uint8_t*,recomp_context*,uint32_t,uint32_t,uint32_t,uintptr_t ticket,int)noexcept{if(ticket){owner_stack.pop_back();owners->leave();}}
extern "C" uint64_t workshop_coop_food_begin(uint8_t* r,recomp_context*,uint32_t fn,uint32_t a,uint32_t arg)noexcept{
 try{return f::registry.begin(r,fn,a,arg,owner_stack.empty()?0:u(r,coop::native_player));}catch(...){f::registry.host_error();return f::blocked;}
}
extern "C" void workshop_coop_food_finish(uint8_t* r,recomp_context*,uint64_t token,int completed)noexcept{try{f::registry.finish(r,token,completed!=0);}catch(...){f::registry.host_error();}}
extern "C" void func_8004700C(uint8_t* r,recomp_context* c){c->r2=int32_t(u(r,coop::native_player));}
extern "C" void func_8003C3C0(uint8_t*,recomp_context* c){c->r2=1;}
extern "C" void func_8003C1B0(uint8_t*,recomp_context*){throw std::runtime_error("Excluded non-fruit branch");}
extern "C" void func_80060FE0(uint8_t*,recomp_context*){throw std::runtime_error("Excluded specialized metadata branch");}
extern "C" void func_8005FE48(uint8_t*,recomp_context*){}
extern "C" void func_800444D0(uint8_t* r,recomp_context* c){by(r,uint32_t(c->r4)+0x17D,0);}
extern "C" void func_800447C8(uint8_t* r,recomp_context* c){
 ++retire_calls;const auto a=uint32_t(c->r4);need(f::registry.pending()==1,"Native retirement occurs during the proved ingestion transaction");
 recomp_context detach=*c;detach.r4=int32_t(a);if(u(r,a+0x180))func_80048060(r,&detach);
 by(r,a+0x17D,3);if(forget_on_retire){f::registry.forget(a);n::forgotten(a);}
}
extern "C" void func_800329B8(uint8_t*,recomp_context*){throw std::runtime_error("Fruit must not enter enemy-to-egg branch");}
extern "C" void func_8002B470(uint8_t* r,recomp_context* c){++score_calls;need(!owner_stack.empty()&&uint32_t(c->r4)==roots[owner_stack.back()]&&uint32_t(c->r5)==privates[owner_stack.back()],"Native reward service runs in actual eater scope");w(r,0x807F1000,u(r,0x807F1000)+5);}
extern "C" void func_8001D770(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_80067BF8(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_80067818(uint8_t*,recomp_context*){}
extern "C" void func_80065490(uint8_t* r,recomp_context* c){for(unsigned i=0;i<12;i+=4)w(r,uint32_t(c->r4)+i,u(r,uint32_t(c->r5)+i));}
extern "C" void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected native hierarchy fault");}
static void food_event(uint8_t* r,recomp_context* c){
 if(uint32_t(c->r5)==0x64){++captures;if(throw_capture)throw std::runtime_error("Native capture test exception");return;}
 need(uint32_t(c->r5)==0x66,"Native ingestion event66");++ring_calls;w(r,0x807F1004,u(r,0x807F1004)+1);
}
extern "C" recomp_func_t* get_function(int32_t address){if(uint32_t(address)==callback)return food_event;throw std::runtime_error("Unexpected native callback lookup");}
struct Fixture {
 std::vector<uint8_t> bytes=std::vector<uint8_t>(0x800000);uint8_t* r=bytes.data();recomp_context c{};unsigned vi=2401;
 Fixture(){
  captures=score_calls=ring_calls=retire_calls=0;throw_capture=invalid_detach=false;forget_on_retire=true;owner_stack.clear();
  n::configure(true);f::registry.configure(true);
  for(unsigned p=0;p<2;p++){
   actor(roots[p],0x4001,privates[p],2);actor(tongues[p],0x400C,0x80208000+p*0x1000,0);
   w(r,tongues[p]+0x180,roots[p]);h(r,roots[p]+0x184,1);w(r,roots[p]+0x188,tongues[p]);
   need(n::bind_root(r,p,roots[p])&&n::register_body(r,tongues[p]),"Actual birth/root/tongue ownership registered");
   const auto meta=u(r,tongues[p]+0x174);by(r,meta+0x12,1);w(r,tongues[p]+0x3C,std::bit_cast<uint32_t>(20.f));
  }
  w(r,0x800B01EC,std::bit_cast<uint32_t>(10.f));w(r,0x800B01F0,std::bit_cast<uint32_t>(-10.f));
  w(r,coop::native_player,roots[0]);w(r,coop::native_color,0);w(r,coop::custom_color,0);for(unsigned i=0;i<8;i++)w(r,coop::native_health+i*4,8);
  memory=std::make_unique<Memory>(r);owners=std::make_unique<coop::OwnerContext>(*memory);owners->bind(0,{{roots[0],1},0,8});owners->bind(1,{{roots[1],2},5,8});
  c.r29=int32_t(0x807A0000);c.r31=int32_t(0x80000300);
  food(0x4030);frame();need(f::registry.active(),"Real registered pair enables only room43 food adapter");
 }
 void actor(uint32_t a,unsigned kind,uint32_t private_data,unsigned queue){
  need(n::created(a),"Creation event precedes every incarnation");for(unsigned i=0;i<0x280;i++)by(r,a+i,0);
  w(r,a,0x19971110);h(r,a+12,kind);by(r,a+0x17D,queue);by(r,a+0x18,1);w(r,a+0x1F4,private_data);
  const uint32_t meta=0x80300000+((a-0x80100000)/0x1000)*0x200,def=meta+0x80;w(r,a+0x174,meta);w(r,meta,def);h(r,def+0x1C,0xC2);w(r,a+0x1C,callback);
 }
 void food(unsigned kind,unsigned queue=2,unsigned parent_kind=0){
  if(u(r,object+0x180)){recomp_context detach=c;detach.r4=int32_t(object);func_80048060(r,&detach);}
  actor(object,kind,0x8020A000,queue);
  if(parent_kind){actor(carrier,parent_kind,0x8020B000,2);recomp_context attach=c;attach.r4=int32_t(carrier);attach.r5=int32_t(object);func_80047F9C(r,&attach);}
 }
 void frame(unsigned room=43){f::registry.frame(r,vi++,room,true,roots[0],roots[1]);}
 void contact(unsigned p,uint32_t target=object){c.r4=int32_t(tongues[p]);c.r5=int32_t(target);d::invoke_food(r,&c,f::contact,func_8003C510);need(owner_stack.empty()&&owners->depth()==0&&u(r,coop::native_player)==roots[0],"Contact restores owner singleton");}
 void eat(unsigned p){c.r4=int32_t(roots[p]);c.r5=int32_t(privates[p]);d::invoke_food(r,&c,f::ingestion,func_80032A8C);need(owner_stack.empty()&&owners->depth()==0&&u(r,coop::native_player)==roots[0],"Ingestion restores owner singleton");}
 void spit(unsigned p){c.r4=int32_t(object);func_80048060(r,&c);w(r,privates[p]+0x164,0);by(r,privates[p]+0x155,0);by(r,object+0x17D,2);by(r,object+0x18,1);frame();}
};
int main(){try{
 for(unsigned kind:{0x4030u,0x4031u,0x4032u,0x4033u,0x4034u,0x4F00u,0x4F01u,0x4F02u,0x4F03u,0x4F04u,0x4F05u})for(unsigned p=0;p<2;p++)for(unsigned carrier_kind:{0u,0x4525u,0x41D1u}){
  Fixture x;x.food(kind,carrier_kind?0:2,carrier_kind);x.contact(p);need(u(x.r,privates[p]+0x164)==object&&u(x.r,object+0x180)==tongues[p]&&n::byte(x.r,object+0x17D)==0,"Unchanged native contact captures free/carried/bubble fruit and makes native childqueue0");
  const auto before=x.bytes;x.contact(1-p);need(x.bytes==before,"Losing tongue cannot mutate the winner fruit or mouth");
  x.eat(p);need(ring_calls==1&&score_calls==1&&retire_calls==1&&f::registry.statistics().consumed==1,"One real native transaction gives one event/reward/cleanup");need(!u(x.r,privates[p]+0x164)&&!n::byte(x.r,privates[p]+0x155)&&!f::registry.failed(),"Native32984 clears mouth even after retirement forgets birth");
  need(u(x.r,0x807F1000)==5&&u(x.r,0x807F1004)==1,"Adapter adds no second score or shared fruit count");
 }
 {Fixture x;const uint32_t other=object+0x800;x.actor(other,0x4F02,0x8020C000,2);x.contact(0);x.contact(1,other);need(u(x.r,privates[0]+0x164)==object&&u(x.r,privates[1]+0x164)==other,"Different fruits can be held concurrently in one world frame");x.eat(1);x.eat(0);need(score_calls==2&&ring_calls==2&&f::registry.statistics().consumed==2,"Different same-frame fruits contribute two original transactions");}
 {Fixture x;x.contact(0);h(x.r,object+0x17E,1);x.eat(0);need(score_calls==0&&u(x.r,privates[0]+0x164)==object,"Stale child slot cannot detach another object during ingestion");h(x.r,object+0x17E,0);x.eat(0);need(score_calls==1,"Original native reciprocal link restores valid ingestion");}
 {Fixture x;x.contact(0);x.spit(0);need(!f::registry.claims(),"Real native detach plus clearedmouth releases spit claim");x.contact(1);x.eat(1);need(score_calls==1&&f::registry.statistics().consumed==1,"Spit fruit may be claimed by partner once");}
 {Fixture x;x.contact(0);x.spit(0);const auto old=n::observed_identity(x.r,object);x.food(0x4030);const auto fresh=n::observed_identity(x.r,object);need(old&&fresh&&old->address==fresh->address&&old->private_data==fresh->private_data&&old->generation!=fresh->generation,"Same address/kind/private receives a new real creation generation");x.contact(1);x.eat(1);need(score_calls==1,"New incarnation remains collectible");}
 {Fixture x;auto p=f::registry.begin(x.r,f::contact,tongues[0],object,roots[0]);need(p&&p!=f::blocked,"Provisional claim admitted");w(x.r,coop::native_player,roots[1]);need(f::registry.begin(x.r,f::contact,tongues[1],object,roots[1])==f::blocked,"Provisional reservation rejects a competing call before attachment");w(x.r,coop::native_player,roots[0]);f::registry.finish(x.r,p,true);need(f::registry.claims()==0,"Native capture refusal releases provisional claim");x.contact(1);x.eat(1);}
 for(unsigned bad=0;bad<17;bad++){
  Fixture x;
  switch(bad){case 0:n::forgotten(object);break;case 1:by(x.r,object+0x17D,3);break;case 2:by(x.r,object+0x17D,4);break;case 3:by(x.r,object+0x17D,1);break;case 4:by(x.r,object+0x17D,0);break;case 5:by(x.r,object+0x18,0xFF);break;case 6:w(x.r,privates[1]+0x164,object);break;case 7:w(x.r,object+0x180,tongues[1]);break;case 8:w(x.r,roots[0]+0x1F4,0x80206000);break;case 9:n::forgotten(tongues[0]);break;case 10:by(x.r,roots[1]+0x18,0xA);break;case 11:x.food(0x4030,0,0x41D1);h(x.r,carrier+0x184,2);break;case 12:x.contact(0);break;case 13:x.food(0x4030,0,0x41D1);n::forgotten(carrier);break;case 14:x.food(0x4030,0,0x41D1);by(x.r,carrier+0x17D,3);break;case 15:x.food(0x4030,0,0x41D1);h(x.r,object+0x17E,1);break;case 16:w(x.r,object+0x180,0x1234);break;}
  const auto score=score_calls;x.contact(0);if(score_calls!=score||captures!=unsigned(bad==12))throw std::runtime_error("Invalid identity/queue/owner/parent case "+std::to_string(bad));++checks;
 }
 {Fixture x;x.contact(0);w(x.r,privates[1]+0x164,object);x.eat(1);need(score_calls==0&&u(x.r,privates[0]+0x164)==object,"Loser cannot ingest or clean the winner mouth");w(x.r,privates[1]+0x164,0);x.eat(0);need(score_calls==1,"Winner still executes native cleanup");}
 {Fixture x;throw_capture=true;bool threw=false;try{x.contact(1);}catch(const std::runtime_error&){threw=true;}need(threw&&f::registry.pending()==0&&owner_stack.empty()&&owners->depth()==0,"Native exception closes pending and owner scopes");}
 {Fixture x;x.contact(0);x.frame(44);need(!f::registry.active()&&f::registry.claims()==0,"Room departure deactivates and releases host claims");}
 {Fixture x;forget_on_retire=false;for(unsigned i=0;i<30;i++){x.food(i%2?0x4F00:0x4030);x.contact(i%2);x.eat(i%2);}need(ring_calls==30&&score_calls==30&&f::registry.statistics().consumed==30&&u(x.r,0x807F1004)==30,"Thirty shared fruits contribute exactly30 original events, never60");}
 for(unsigned survivor=0;survivor<2;survivor++){
  Fixture x;x.contact(survivor);workshop::coop_lifecycle0280::State life;life.enabled=life.bound=true;life.room=43;life.epoch=1;
  for(unsigned p=0;p<2;p++){life.players[p].root=*n::observed_identity(x.r,roots[p]);life.players[p].phase=p==survivor?workshop::coop_lifecycle0280::Phase::alive:workshop::coop_lifecycle0280::Phase::retiring;}
  life.players[1-survivor].death_vi=x.vi;life.players[1-survivor].due_vi=x.vi+300;
  f::registry.forget(roots[1-survivor],&life);n::forgotten(roots[1-survivor]);by(x.r,roots[1-survivor]+6,3);
  need(f::registry.claims()==1,"Expected native partner cleanup preserves the survivor incarnation claim");
  f::registry.single_frame(x.r,x.vi++,43,life);need(f::registry.active(),"Typed one-survivor receipt enables only its actual owner");x.eat(survivor);
  need(score_calls==1&&ring_calls==1&&f::registry.statistics().consumed==1&&!f::registry.failed(),"Survivor's already-held fruit executes original32A8C and cleanup once");
 }
 for(unsigned survivor=0;survivor<2;survivor++){
  Fixture x;workshop::coop_lifecycle0280::State life;life.enabled=life.bound=true;life.room=81;life.epoch=2;for(unsigned p=0;p<2;p++){life.players[p].root=*n::observed_identity(x.r,roots[p]);life.players[p].phase=workshop::coop_lifecycle0280::Phase::alive;}
  f::registry.frame(x.r,x.vi++,81,true,roots[0],roots[1],&life);need(f::registry.active(),"Room81 needs the sealed two-root lifecycle receipt");x.contact(survivor);life.players[1-survivor].phase=workshop::coop_lifecycle0280::Phase::retiring;life.players[1-survivor].death_vi=x.vi;life.players[1-survivor].due_vi=x.vi+300;f::registry.forget(roots[1-survivor],&life);n::forgotten(roots[1-survivor]);by(x.r,roots[1-survivor]+6,3);need(f::registry.claims()==1,"Room81 cleanup preserves only the actual survivor's already-held claim");f::registry.single_frame(x.r,x.vi++,81,life);x.eat(survivor);need(score_calls==1&&ring_calls==1&&f::registry.statistics().consumed==1&&!f::registry.failed(),"Room81 survivor ingests once through original32A8C/32984");
 }
 {Fixture x;f::registry.frame(x.r,x.vi++,81,true,roots[0],roots[1]);need(!f::registry.active(),"Room81 without a typed lifecycle handoff remains rejected");}
 for(unsigned fault=0;fault<5;fault++){
  Fixture x;x.contact(0);workshop::coop_lifecycle0280::State life;life.enabled=life.bound=true;life.room=43;life.epoch=1;
  for(unsigned p=0;p<2;p++){life.players[p].root=*n::observed_identity(x.r,roots[p]);life.players[p].phase=p?workshop::coop_lifecycle0280::Phase::retiring:workshop::coop_lifecycle0280::Phase::alive;}
  life.players[1].death_vi=x.vi;
  switch(fault){case 0:life.failed=true;break;case 1:life.room=81;break;case 2:life.players[1].root.generation++;break;case 3:life.players[0].root.generation++;break;case 4:life.players[1].phase=workshop::coop_lifecycle0280::Phase::alive;break;}
  f::registry.forget(roots[1],&life);need(f::registry.claims()==0,"Unknown/invalidation/wrong incarnation root cleanup retains old fail-closed behavior");
 }
 need(!f::fruit(0x4035)&&!f::fruit(0x4003)&&!f::fruit(0x4526),"Heart/reticle/enemy paths remain outside fruit-only scope");
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"exactNativeBodies\":8,\"schedulerRewardFruitEventBoundaries\":\"HOST\",\"gameLaunched\":false,\"nativeGameplayVerified\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
