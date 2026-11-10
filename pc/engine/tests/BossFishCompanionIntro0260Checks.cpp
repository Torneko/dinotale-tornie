#include "../boss_fish_companion_intro0260.hpp"
#include <bit>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace h=workshop::boss_fish_companion_intro0260;namespace n=workshop::boss_native;
static unsigned checks=0,native_updates=0,native_contacts=0,leaves=0,release_calls=0,companion_contacts=0,movement_calls=0;
static uint32_t native_player=0;static h::Frame dispatch_frame;
static void need(bool ok,const char* what){++checks;if(!ok)throw std::runtime_error(what);}
int32_t sections[512]{};int32_t* section_addresses=sections;
extern "C" void func_8002FE38(uint8_t*,recomp_context*){++release_calls;}
extern "C" void func_8004700C(uint8_t*,recomp_context* c){c->r2=n::base(native_player);}
extern "C" void func_800653D8(uint8_t*,recomp_context* c){c->f0.fl=.25f;}
#define SERVICE(name) extern "C" void name(uint8_t*,recomp_context*){}
SERVICE(func_800492C0) SERVICE(func_800677F8) SERVICE(func_80067878) SERVICE(func_80067930)
SERVICE(func_80067BBC) SERVICE(func_80067F78) SERVICE(func_8006CAE0) SERVICE(func_8042A9DC)
SERVICE(func_8042BA84) SERVICE(func_8042BB3C) SERVICE(func_8042D564) SERVICE(func_8042D5C0)
SERVICE(func_806ADE38) SERVICE(func_806ADEE0)
#undef SERVICE
extern "C" void func_8042D50C(uint8_t*,recomp_context*);
extern "C" void func_806AF64C(uint8_t*,recomp_context*);
// Complete native callback bodies are included bytewise. Animation/audio,
// singleton lookup and player-release are explicit external test services.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-label"
extern "C" {
#include "native-intro.inc"
}
#pragma clang diagnostic pop
// Actual contact's player-damage and animation services are isolated, not
// a simulated health claim. Its unconditional native state changes stay real.
extern "C" void func_8002F6E4(uint8_t*,recomp_context* c){need(uint32_t(c->r4)==native_player,"Native contact invokes player-damage with actual P1");++companion_contacts;}
extern "C" void func_800447C8(uint8_t*,recomp_context*){}
extern "C" void func_80058B44(uint8_t*,recomp_context*){}
extern "C" void func_80058C10(uint8_t*,recomp_context*){}
extern "C" void func_806ADD68(uint8_t*,recomp_context*){}
extern "C" void func_806AE458(uint8_t*,recomp_context* c){c->r2=1;}
static void contact_movement(uint8_t*,recomp_context*){++movement_calls;}
static recomp_func_t* contact_lookup(int32_t address){need(uint32_t(address)==0x803007FC,"Native1/5 keeps the actual initial AE53C movement service");return contact_movement;}
#undef LOOKUP_FUNC
#define LOOKUP_FUNC(val) contact_lookup(int32_t(val))
#define func_806AEFDC battle_original_func_806AEFDC
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-label"
extern "C" {
#include "native-contact.inc"
}
#pragma clang diagnostic pop
#undef func_806AEFDC
extern "C" int workshop_battle_hook_enter(uint8_t* r,recomp_context* c,const char* name){return h::suppress(r,c,name,dispatch_frame);}
extern "C" void workshop_battle_hook_leave(uint8_t*,const recomp_context*,const char*){++leaves;}
extern "C" int workshop_battle_paint_begin(uint8_t*,recomp_context*,unsigned){return 0;}
extern "C" void workshop_battle_paint_finish(uint8_t*){}
// Counters attest only whether the ORIGINAL wrapper would call its native
// body; these substitutes do not assert simulated gameplay/damage behavior.
extern "C" void battle_original_func_805C7C2C(uint8_t*,recomp_context*){++native_updates;}
extern "C" void battle_original_func_805C6900(uint8_t*,recomp_context*){++native_contacts;}
#include "actual-fish-dispatch.inc"
#include "actual-companion-dispatch.inc"
struct Fixture {
 std::vector<uint8_t> ram=std::vector<uint8_t>(8*1024*1024);h::Frame frame;recomp_context context{};
 static constexpr uint32_t player=0x80010000,pdata=0x80012000,fish=0x80020000,fdata=0x80021000,companion=0x80030000,cdata=0x80031000,factory=0x80200000,cfactory=0x80300000,fresource=0x80040000,cresource=0x80040100;
 void w(uint32_t a,uint32_t v){auto* rdram=ram.data();MEM_W(0,n::base(a))=v;}
 void by(uint32_t a,unsigned v){auto* rdram=ram.data();MEM_B(0,n::base(a))=uint8_t(v);}
 void half(uint32_t a,unsigned v){auto* rdram=ram.data();MEM_H(0,n::base(a))=uint16_t(v);}
 void actor(uint32_t a,uint16_t kind){w(a,0x19971110);half(a+12,kind);by(a+0x17D,2);by(a+0x18,1);}
 void sub(unsigned value){by(fish+0x30,value);w(fish+0x20,factory+h::fish_callbacks[value]);}
 Fixture(bool genie=false,unsigned index=3,unsigned epoch=0){
  native_player=player;release_calls=native_updates=native_contacts=leaves=companion_contacts=movement_calls=0;
  actor(player,0x4001);w(player+0x1F4,pdata);w(0x800F8828,player);
  actor(fish,0x4511);w(fish+0x1F4,fdata);w(fish+0x168,fresource);half(fresource+0x3C,0x4511);w(fresource+0x40,factory);w(fresource+0x44,factory+0x35A8);sub(1);
  const uint16_t kind=genie?0x4072:0x4599;actor(companion,kind);w(companion+0x1F4,cdata);w(companion+0x168,cresource);half(cresource+0x3C,kind);w(cresource+0x40,cfactory);w(cresource+0x44,cfactory+(genie?0x2A9C:0x18B8));w(companion+0x20,cfactory+(genie?0x1EB0:0xDAC));
  frame={true,true,true,2,2,index,167+index,97,epoch,player,"Synthetic-owned-run",{fish,fdata,factory,11,epoch,0x4511},{companion,cdata,cfactory,12,epoch,kind}};
  context.r4=n::base(fish);context.r5=n::base(player);context.r29=n::base(0x800FF800);context.r2=0x12345678;
  sections[16]=int32_t(cfactory);sections[197]=int32_t(cfactory);w(cfactory+0x1EFC+3*4,cfactory+0x1178);w(cfactory+0x1EFC+4*4,cfactory+0x11C4);w(cfactory+0x1EFC+5*4,cfactory+0x1248);w(cdata+0x10,cfactory+0x7FC);w(cfactory+0x2E64+6*4,cfactory+0x1B88);
 }
};
int main(){try{
 for(bool genie:{false,true})for(unsigned index=1;index<=8;++index)for(unsigned epoch:{0u,1u,3u}){Fixture f(genie,index,epoch);dispatch_frame=f.frame;const auto original=f.ram;const auto registers=f.context;
  need(h::introducing(f.ram.data(),f.frame),"Every owned variant2 Fish encounter and retry epoch recognizes only its real companion intro");
  func_805C7C2C(f.ram.data(),&f.context);func_805C6900(f.ram.data(),&f.context);
  need(!native_updates&&!native_contacts&&leaves==2&&f.ram==original&&!std::memcmp(&registers,&f.context,sizeof(registers)),"Actual existing wrappers defer only native Fish bodies, with all guest RAM/registers intact");
  need(!h::suppress(f.ram.data(),&f.context,genie?"func_8042C750":"func_806AEAEC",f.frame),"Companion animation/timer/intro wrapper always proceeds");
  f.sub(2);dispatch_frame=f.frame;func_805C7C2C(f.ram.data(),&f.context);func_805C6900(f.ram.data(),&f.context);need(native_updates==1&&!native_contacts,"A Fish already1/2 is not frozen as idle, but its actual harmful contact still waits");
 }
 for(unsigned fault=0;fault<42;++fault){Fixture f;switch(fault){
  case 0:f.frame.active=false;break;case 1:f.frame.fighting=false;break;case 2:f.frame.fish_encounter=false;break;case 3:f.frame.variant=1;break;case 4:f.frame.count=1;break;case 5:f.frame.index=0;break;case 6:f.frame.index=9;break;case 7:f.frame.room=169;break;case 8:f.frame.source=164;break;case 9:f.frame.token={};break;
  case 10:f.frame.fish.birth=0;break;case 11:f.frame.companion.birth=0;break;case 12:f.frame.fish.epoch=1;break;case 13:f.frame.companion.epoch=1;break;case 14:f.frame.fish.kind=0x4631;break;case 15:f.frame.companion.kind=0x455A;break;case 16:f.w(0x800F8828,Fixture::fish);break;case 17:f.by(Fixture::player+0x18,0xA);break;case 18:f.by(Fixture::player+0x165,1);break;case 19:f.by(Fixture::player+0x17D,3);break;
  case 20:f.by(0x800F7EA5,1);break;case 21:f.by(0x800F7EA6,1);break;case 22:f.w(Fixture::fish+0x1F4,Fixture::fdata+4);break;case 23:f.w(Fixture::companion+0x1F4,Fixture::cdata+4);break;case 24:f.frame.fish.overlay+=4;break;case 25:f.frame.companion.overlay+=4;break;case 26:f.w(Fixture::fresource+0x44,Fixture::factory+0x35AC);break;case 27:f.w(Fixture::cresource+0x44,Fixture::cfactory+0x1840);break;case 28:f.w(Fixture::companion+0x20,Fixture::cfactory+0x1138);break;case 29:f.by(Fixture::companion+0x30,2);break;
  case 30:f.w(Fixture::fish+0x20,Fixture::factory+0x173C+4);break;case 31:f.by(Fixture::fish+0x17D,1);break;case 32:f.by(Fixture::companion+0x17D,3);break;case 33:f.w(Fixture::fdata+0x40,Fixture::player);break;case 34:f.context.r4=n::base(Fixture::companion);break;case 35:f.w(Fixture::player+0x1F4,0);break;case 36:f.frame.companion.actor=Fixture::fish;break;case 37:f.frame.fish.actor=Fixture::player;break;case 38:f.frame.companion.private_data=Fixture::fdata;break;case 39:f.w(Fixture::fish,0);break;case 40:f.w(Fixture::companion,0);break;case 41:f.by(Fixture::player+0x18,0xFF);break;
  }const auto before=f.ram;const auto registers=f.context;need(!h::suppress(f.ram.data(),&f.context,"func_805C7C2C",f.frame)&&!h::suppress(f.ram.data(),&f.context,"func_805C6900",f.frame)&&f.ram==before&&!std::memcmp(&registers,&f.context,sizeof(registers)),"Foreign/solo/pause/dead/victory/reset/stale-epoch/overlay/ctor/identity never suppresses any native callback");
  if(fault!=30){auto contact=f.context;contact.r4=n::base(fault==34?Fixture::fish:Fixture::companion);need(!h::suppress(f.ram.data(),&contact,"func_806AEFDC",f.frame)&&f.ram==before,"The same invalid global/typed scope or wrong contact owner cannot suppress the companion callback");}
 }
 {Fixture f;f.context.r5=n::base(Fixture::companion);need(!h::suppress(f.ram.data(),&f.context,"func_805C6900",f.frame),"Foreign contact target never skipped");need(!h::suppress(nullptr,&f.context,"func_805C6900",f.frame)&&!h::suppress(f.ram.data(),nullptr,"func_805C6900",f.frame),"Null inputs are inert");}
 // Execute complete unchanged native final-intro callbacks and their actual
 // state setters. Services above isolate animation/audio/player release only.
 for(bool genie:{false,true}){Fixture f(genie);if(!genie)f.w(Fixture::companion+0x20,Fixture::cfactory+0xCE4);f.w(Fixture::companion+0x1E0,genie?635u:67u);
  need(h::introducing(f.ram.data(),f.frame),"Exact final native intro callback is admitted before its real terminal tick");
  auto intro=f.context;intro.r4=n::base(Fixture::companion);if(genie)func_8042C750(f.ram.data(),&intro);else func_806AEA24(f.ram.data(),&intro);
  need(!n::byte(f.ram.data(),0x800F7EA6)&&!release_calls&&h::introducing(f.ram.data(),f.frame),"Native intro below69/637 keeps original controls0 and gate without invented time advancement");
  intro=f.context;intro.r4=n::base(Fixture::companion);if(genie)func_8042C750(f.ram.data(),&intro);else func_806AEA24(f.ram.data(),&intro);
  need(n::byte(f.ram.data(),0x800F7EA6)==1&&release_calls==1&&n::byte(f.ram.data(),Fixture::companion+0x18)==1&&n::byte(f.ram.data(),Fixture::companion+0x30)==(genie?6u:3u)&&n::word(f.ram.data(),Fixture::companion+0x20)==Fixture::cfactory+(genie?0x1B88u:0x1178u),"Actual native intro and setter produce the proved combat callback and controls1");
  dispatch_frame=f.frame;func_805C7C2C(f.ram.data(),&f.context);func_805C6900(f.ram.data(),&f.context);need(native_updates==1&&native_contacts==1&&!h::introducing(f.ram.data(),f.frame),"Both actual wrappers resume their original Fish body immediately after native combat transition");
 }
 // Reproduce the exact causal bug with the real contact body and setter.
 {Fixture f;auto c=f.context;c.r4=n::base(Fixture::companion);battle_original_func_806AEFDC(f.ram.data(),&c);
  need(companion_contacts==1&&!release_calls&&!n::byte(f.ram.data(),0x800F7EA6)&&n::byte(f.ram.data(),Fixture::companion+0x30)==5&&n::word(f.ram.data(),Fixture::companion+0x20)==Fixture::cfactory+0x1248,"Unguarded actual AEFDC cancels intro0->5 without native control release");
  c=f.context;c.r4=n::base(Fixture::companion);func_806AEF88(f.ram.data(),&c);
  need(movement_calls==1&&!release_calls&&!n::byte(f.ram.data(),0x800F7EA6)&&n::byte(f.ram.data(),Fixture::companion+0x30)==4&&n::word(f.ram.data(),Fixture::companion+0x20)==Fixture::cfactory+0x11C4,"Actual AEF88 at native animation completion continues5->4 while context remains0, matching real capture");
 }
 for(unsigned callback:{0xDACu,0xCE4u}){Fixture f;f.w(Fixture::companion+0x20,Fixture::cfactory+callback);dispatch_frame=f.frame;f.context.r4=n::base(Fixture::companion);const auto before=f.ram;const auto regs=f.context;func_806AEFDC(f.ram.data(),&f.context);
  need(!companion_contacts&&!release_calls&&f.ram==before&&!std::memcmp(&regs,&f.context,sizeof(regs)),"Existing AEFDC wrapper defers only the real owned P1 contact during both exact intro callbacks, without guest writes");
  auto other=f.context;other.r5=n::base(Fixture::fish);need(!h::suppress(f.ram.data(),&other,"func_806AEFDC",f.frame),"Companion contacts with unrelated actors remain fully native");
  other.r5=n::base(Fixture::player);f.by(Fixture::player+0x18,0x68);need(!h::suppress(f.ram.data(),&other,"func_806AEFDC",f.frame),"The native projectile68 branch is never suppressed, even with a malformed P1 action");f.by(Fixture::player+0x18,1);
  // The guard does not block the unchanged intro callback or its own timer.
  f.w(Fixture::companion+0x20,Fixture::cfactory+0xCE4);f.w(Fixture::companion+0x1E0,68);auto intro=f.context;intro.r4=n::base(Fixture::companion);func_806AEA24(f.ram.data(),&intro);
  need(release_calls==1&&n::byte(f.ram.data(),0x800F7EA6)==1&&n::byte(f.ram.data(),Fixture::companion+0x30)==3,"The preserved true intro still reaches its own native control release");
  f.context.r4=n::base(Fixture::companion);func_806AEFDC(f.ram.data(),&f.context);need(companion_contacts==1&&n::byte(f.ram.data(),Fixture::companion+0x30)==5&&n::byte(f.ram.data(),0x800F7EA6)==1,"The same P1 contact uses the original actual damage/state-change body immediately after real intro completion");
 }
 {Fixture f(true);f.context.r4=n::base(Fixture::companion);need(!h::suppress(f.ram.data(),&f.context,"func_806AEFDC",f.frame),"No contact generalization to Genie or any unproved callback");}
 std::cout<<"BossFishCompanionIntro0260Checks: "<<checks<<" PASS; actual wrapper dispatch and unchanged native intro/setters; explicit external services, no GPU/mixed victory claim.\n";return 0;
}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 1;}}
