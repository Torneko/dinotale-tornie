#include "../boss_battle_runtime0250.hpp"
#include <fstream>
#include <iterator>
#include <iostream>
#include <stdexcept>
namespace h=workshop::boss_rest_heart0260;namespace n=workshop::boss_native;namespace run=workshop::boss_campaign0250;
static unsigned checks=0,lookups=0,allocations=0;static constexpr uint32_t actual_heart=0x800C9A44,new_heart=0x807A0000;static uint32_t last_return=0;
static int32_t sections[300]{};int32_t* section_addresses=sections;
static void need(bool yes,const char* why){++checks;if(!yes)throw std::runtime_error(why);}
static void word(uint8_t* rdram,uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}static void half(uint8_t* rdram,uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}static void byte(uint8_t* rdram,uint32_t a,uint8_t v){MEM_B(0,n::base(a))=v;}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#define func_80047090 fixture_native_47090
#define func_8004613C fixture_native_4613C
#define func_80044194 fixture_native_44194
namespace real {
void func_806E7F18(uint8_t*,recomp_context*);
#include "native-heart-queue.inc"
#include "native-heart-factory.inc"
void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected native queue fault");}
// Explicit allocator/resource/rendering service boundaries. Original factory,
// initialization, state setter, registration and queue insertion execute their
// unchanged native bodies against a COPY of the actual final8MiB snapshot.
void func_80047090(uint8_t*,recomp_context* c){++lookups;if(c->r4==0x40C5)c->r2=0x201;else{need(c->r4==0x4027,"Only optional cosmetic child lookup may be skipped");c->r2=0x3FF;}}
void func_8004613C(uint8_t* ram,recomp_context* c){++allocations;need(uint32_t(c->r4)==0x40C52201,"Real7E70 uses40C5 dynamic packed factory instance");std::copy_n(ram+(actual_heart&0x7FFFFF),0x1FC,ram+(new_heart&0x7FFFFF));word(ram,new_heart+0x3C,uint32_t(c->r5));word(ram,new_heart+0x40,uint32_t(c->r6));word(ram,new_heart+0x44,uint32_t(c->r7));byte(ram,new_heart+0x17D,0);half(ram,new_heart+0x17E,150);half(ram,new_heart+0x184,0);for(unsigned i=0;i<20;++i)word(ram,new_heart+0x188+4*i,0);c->r2=n::base(new_heart);}
void func_8005FCA8(uint8_t*,recomp_context* c){c->r2=1;}void func_8005FD1C(uint8_t*,recomp_context*){}
void func_80047F9C(uint8_t*,recomp_context*){throw std::runtime_error("Cosmetic child unexpectedly allocated");}
void func_806E7F18(uint8_t*,recomp_context*){throw std::runtime_error("Cosmetic child unexpectedly initialized");}
}
#undef func_80047090
#undef func_8004613C
#undef func_80044194
#pragma clang diagnostic pop
static h::Frame frame(uint8_t* ram){return {83,5,28163,n::word(ram,0x800F8828),true};}
static h::Bonus bind(uint8_t* ram){h::Bonus bonus;bonus.configure(true);bonus.enter_room(83);const auto req=bonus.request_spawn(ram,frame(ram));need(req.has_value(),"Actual Rest5 authorizes its single logical bonus");need(bonus.bind_spawn(ram,frame(ram),*req,actual_heart),"Actual queued1 native heart binds without changing its state");return bonus;}
static void event(h::Bonus& bonus,uint8_t* ram,const char* name,uint32_t actor,uint32_t other=0,uint32_t ra=0){recomp_context c{};c.r4=n::base(actor);c.r5=n::base(other);c.r31=n::base(ra);bonus.trace(ram,&c,name,frame(ram));}
int main(int argc,char** argv){try{
 need(argc==2,"Supply actual failed Full0de98 snapshot");std::ifstream file(argv[1],std::ios::binary);std::vector<uint8_t> source((std::istreambuf_iterator<char>(file)),{});need(source.size()==0x800000,"Actual8MiB snapshot");const auto original=source;auto* rdram=source.data();const auto resource=n::word(rdram,actual_heart+0x168),factory=n::word(rdram,resource+0x40);need(n::actor(rdram,actual_heart,0x40C5)&&n::byte(rdram,actual_heart+0x17D)==1&&n::byte(rdram,actual_heart+0x18)==5&&factory==0x803AC300,"Actual frame failed with a real queued1/state5 birth already created");need(n::word(rdram,0x800F8D38)==factory+h::spawn_offset,"Actual native global points to the same7E70 factory");auto bonus=bind(rdram);bonus.frame(rdram,frame(rdram));need(!bonus.snapshot().invalidated&&!bonus.snapshot().consumed&&!bonus.request_spawn(rdram,frame(rdram))&&source==original,"Queued birth remains bound without promotion/reward/duplicate or any RAM write");
 for(unsigned queue:{0u,3u,255u}){auto bad=source;auto* ram=bad.data();byte(ram,actual_heart+0x17D,uint8_t(queue));h::Bonus denied;denied.configure(true);denied.enter_room(83);auto req=denied.request_spawn(ram,frame(ram));const auto before=bad;need(req&&!denied.bind_spawn(ram,frame(ram),*req,actual_heart)&&bad==before,"Only queued1 or active2 birth admits binding; no unknown/dead queue");}
 {auto ramcopy=source;auto* ram=ramcopy.data();auto b=bind(ram);const auto player=frame(ram).player;event(b,ram,"func_806E7574",actual_heart,player);event(b,ram,"func_806E7E4C",actual_heart,3,factory+0x388);byte(ram,actual_heart+0x17D,2);byte(ram,actual_heart+0x18,3);word(ram,actual_heart+0x20,factory+0x4E8);const auto before=ramcopy;event(b,ram,"func_806E7728",actual_heart);need(b.snapshot().invalidated&&!b.snapshot().consumed&&ramcopy==before,"Contact while queued1 cannot be laundered into collection after a HOST active2 frontier");}
 {auto ramcopy=source;auto* ram=ramcopy.data();auto b=bind(ram);byte(ram,actual_heart+0x18,3);word(ram,actual_heart+0x20,factory+0x4E8);const auto before=ramcopy;event(b,ram,"func_806E7728",actual_heart);need(b.snapshot().invalidated&&!b.snapshot().consumed&&ramcopy==before,"Collection entry while queued1 cannot grant health or consume token");}
 {auto ramcopy=source;auto* ram=ramcopy.data();auto b=bind(ram);const auto player=frame(ram).player,count=n::word(ram,h::heart_count_address);byte(ram,actual_heart+0x17D,2);event(b,ram,"func_806E7574",actual_heart,player);event(b,ram,"func_806E7E4C",actual_heart,3,factory+0x388);byte(ram,actual_heart+0x18,3);word(ram,actual_heart+0x20,factory+0x4E8);event(b,ram,"func_806E7728",actual_heart);event(b,ram,"func_800447C8",actual_heart,0,factory+0x560);byte(ram,actual_heart+0x17D,3);b.forget(actual_heart);word(ram,h::heart_count_address,count+1);event(b,ram,"func_80049624",0,0,factory+0x578);event(b,ram,"func_8002B428",8,0,factory+0x580);event(b,ram,"func_8002B3A0",0,0,0x8002B460);need(b.snapshot().consumed&&!b.snapshot().invalidated,"Explicit HOST active-contact/native-effect frontiers still allow retirement3 before completion; no global active2 gate breaks pending proof");}
 {auto ramcopy=source;auto* ram=ramcopy.data();need(run::configure("HeartQueuedAdapter",1,true,false),"HOST Runtime configuration");auto& state=const_cast<workshop::boss_battle0250::Snapshot&>(run::session.snapshot());state.phase=workshop::boss_battle0250::Phase::Rest;state.index=4;state.completed=state.max_completed=5;state.player=frame(ram).player;run::physical_room=run::logical_room=83;run::current_vi=28163;run::rest_heart.enter_room(83);run::overlays[220]=factory;sections[220]=int32_t(factory);recomp_context c{};c.r29=n::base(0x800FFC00);c.r31=n::base(0x80012340);c.r16=0x12345678;const auto registers=c;const auto hp=n::word(ram,0x800F8DB4),count=n::word(ram,h::heart_count_address);std::array<uint32_t,3> stack{};for(unsigned i=0;i<3;++i)stack[i]=n::word(ram,0x800FFBE0+4*i);
  const bool spawned=run::spawn_rest_heart(ram,&c,[&](uint32_t fn,uint8_t* memory,recomp_context* ctx){need(fn==factory+h::spawn_offset,"Actual Runtime dispatches original7E70 relocated address");real::func_806E7E70(memory,ctx);last_return=uint32_t(ctx->r2);});need(spawned&&last_return==new_heart&&run::rest_heart.snapshot().binding.actor==new_heart,"Runtime binds the TRUE native7E70 return");need(n::byte(ram,new_heart+0x17D)==1&&n::byte(ram,new_heart+0x18)==5&&n::word(ram,new_heart+0x20)==factory+0x878,"Unmodified native44194 plus original7E70 produce queue1/state5, never a forged active2");need(allocations==1&&lookups==2,"One real heart allocation boundary plus one optional cosmetic lookup only");need(!std::memcmp(&c,&registers,sizeof(c))&&n::word(ram,0x800F8DB4)==hp&&n::word(ram,h::heart_count_address)==count,"Runtime preserves all guest registers, HP and heart count while spawning");for(unsigned i=0;i<3;++i)need(n::word(ram,0x800FFBE0+4*i)==stack[i],"Runtime restores its borrowed vector stack words");need(run::spawn_rest_heart(ram,&c,[](uint32_t,uint8_t*,recomp_context*){throw std::runtime_error("Duplicate spawn");})&&allocations==1,"Same visit cannot create a second pending native heart");}
 need(source==original,"Original snapshot stays immutable throughout");std::cout<<"BossRestHeartQueued0260Checks: "<<checks<<" PASS; actual queued1 binding, inactive-contact negatives, Runtime through unchanged7E70/44194/queue. Allocation/render services explicit; promotion/collection gameplay remain GPU proof.\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
