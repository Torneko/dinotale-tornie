#include "../boss_battle_runtime0250.hpp"
#include "../boss_route_checkpoint_bridge0260.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace r=workshop::boss_campaign0250;
namespace q=r::checkpoints;
namespace n=workshop::boss_native;
namespace b=workshop::boss_battle0250;
namespace inv=workshop::boss_checkpoint_inventory0260;
// Optional literal public getters independently attest the constructor
// offsets, rather than mirroring the Runtime predicate in this fixture.
#ifdef BOSS_GROUP_NATIVE_GETTERS
static int32_t native_sections[300]{};int32_t* section_addresses=native_sections;
namespace native_getters {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include BOSS_GROUP_NATIVE_GETTERS
#pragma clang diagnostic pop
}
#endif
static unsigned checks=0,transports=0,discards=0,heals=0,egg_births=0,escort_retires=0,control_resets=0;
static bool expect_control_reset=false;
static void need(bool yes,const char* why){++checks;if(!yes)throw std::runtime_error(why);}
static void word(uint8_t* rdram,uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}
static void half(uint8_t* rdram,uint32_t a,unsigned v){MEM_H(0,n::base(a))=uint16_t(v);}
static void byte(uint8_t* rdram,uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}
static void actor(uint8_t* ram,uint32_t a,unsigned kind){word(ram,a,0x19971110);half(ram,a+12,kind);byte(ram,a+0x17D,2);byte(ram,a+0x18,1);}
// Explicit synthetic native boundaries only. These stubs verify Runtime call
// routing and values; the separate inventory/character suites execute real
// native slices. This fixture is not evidence of a real death or transport.
// This legacy Runtime fixture never enters a replayed Monsters cache.
// The dedicated0281 oracle executes these native bodies independently.
extern "C" void func_8006D6F8(uint8_t*,recomp_context*){need(false,"Unexpected Monsters cache lookup in legacy Runtime fixture");}
extern "C" void func_8006D34C(uint8_t*,recomp_context*){need(false,"Unexpected Monsters placement write in legacy Runtime fixture");}
extern "C" void func_80030B7C(uint8_t* ram,recomp_context* c){
 ++transports;const auto p=uint32_t(c->r4),route=uint32_t(c->r5);
 need(n::word(ram,route)==0x44CA2000&&n::byte(ram,route+0x16)==1,"Runtime selects original44CA arrival recipe1");
 need(q::pose::matches(ram,q::selected_pose(),q::selected_inventory())&&q::pose_restore.last_epoch()==r::session.snapshot().retry_epoch,"Stable activation depth and zero death depth motion are restored once BEFORE transport");
 for(unsigned i=0;i<20;++i){const auto child=n::word(ram,p+0x188+i*4);need(!child||!n::actor(ram,child,0x4015),"Death cinematic actors must retire BEFORE the native transport boundary");}
 byte(ram,p+0x18,0xA);byte(ram,p+0x30,2);word(ram,p+0x20,0x80031230);byte(ram,p+0x17D,0);byte(ram,p+0x165,1);
 c->r2=0xABCDEF01;c->r16=0xDEADBEEF;
}
extern "C" void func_8003ECCC(uint8_t* ram,recomp_context* c){
 ++discards;const auto p=uint32_t(c->r4),d=n::word(ram,p+0x1F4);const auto count=n::byte(ram,d+0x12C8),head=inv::half(ram,d+0x12BE);
 for(unsigned i=0;i<count;++i){const auto a=n::word(ram,d+0x11FC+((head+i)&7)*24);word(ram,a+0x180,0);}
 for(unsigned i=0;i<20;++i)if(n::word(ram,p+0x188+i*4))word(ram,p+0x188+i*4,0);
 half(ram,p+0x184,0);byte(ram,d+0x12C8,0);half(ram,d+0x12BE,0);for(unsigned i=0;i<8;++i)word(ram,d+0x11FC+i*24,0);
}
extern "C" void func_80040860(uint8_t* ram,recomp_context* c){
 ++egg_births;const auto p=uint32_t(c->r4),d=n::word(ram,p+0x1F4);const unsigned count=n::byte(ram,d+0x12C8),head=inv::half(ram,d+0x12BE);
 const auto a=0x80200000+count*0x400;actor(ram,a,0x4004);word(ram,a+0x180,p);word(ram,a+0x1EC,0);for(unsigned axis=0;axis<3;++axis){const auto scale=n::word(ram,uint32_t(c->r6)+4*axis);word(ram,a+0x6C+4*axis,scale);word(ram,a+0x78+4*axis,scale);}
 word(ram,p+0x188+count*4,a);half(ram,p+0x184,count+1);word(ram,d+0x11FC+((head+count)&7)*24,a);byte(ram,d+0x12C8,count+1);
}
extern "C" void func_8003B8B0(uint8_t* ram,recomp_context* c){word(ram,uint32_t(c->r4)+0x1EC,unsigned(c->r5));}
extern "C" void func_800422CC(uint8_t* ram,recomp_context* c){++heals;word(ram,0x800F8DB4+n::word(ram,0x800F8DB0)*4,unsigned(c->r4));}
extern "C" void func_800447C8(uint8_t* ram,recomp_context* c){
 ++escort_retires;const auto a=uint32_t(c->r4),parent=n::word(ram,a+0x180);const auto slot=inv::half(ram,a+0x17E);
 need(n::actor(ram,a,0x4015)&&slot<20&&n::word(ram,parent+0x188+slot*4)==a,"Synthetic retirement boundary receives only its exact cinematic child");
 word(ram,parent+0x188+slot*4,0);half(ram,parent+0x184,inv::half(ram,parent+0x184)-1);word(ram,a+0x180,0);byte(ram,a+0x18,0xFF);byte(ram,a+0x17D,3);
}
static void forbidden(){throw std::runtime_error("Unexpected boss/heart gameplay call in synthetic checkpoint routing fixture");}
extern "C" void func_80047090(uint8_t*,recomp_context*){forbidden();}
extern "C" void func_8004613C(uint8_t*,recomp_context*){forbidden();}
extern "C" void func_80044194(uint8_t*,recomp_context*){forbidden();}
extern "C" void func_80040C0C(uint8_t* ram,recomp_context* c){
 if(!expect_control_reset)forbidden();++control_resets;
 need(uint32_t(c->r4)==r::session.snapshot().player,"Control-reset boundary retains the exact session Yoshi");
 // Synthetic native clobbers demonstrate that Runtime restores its saved
 // actual pair and registers; this is not a gameplay/control-unlock oracle.
 byte(ram,0x800F7EA5,1);byte(ram,0x800F7EA6,1);c->r16=0xDEADBEEF;
}
#ifdef BOSS_CHECKPOINT_NATIVE42148
static unsigned swallowed_dispatches=0,swallowed_effects=0;
namespace native_swallow {
void func_80021A44(uint8_t*,recomp_context* c){c->r5=0xDEADC0DE;c->r2=0xBADF00D;} // Explicit input-reset service; deliberately clobber liveA1.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include BOSS_CHECKPOINT_NATIVE42148
#pragma clang diagnostic pop
}
static void swallowed_final(uint8_t* ram,recomp_context* c){
 ++swallowed_effects;need(uint32_t(c->r31)==0x8004217C&&uint32_t(c->r5)==0xDEADC0DE,"Literal42148 supplies actualRA and may retain clobbered A1");
 need(q::death_call(ram,c)&&q::intercept_death(ram,c),"Actual native42148 stack and pending callback admit the existing same-Yoshi checkpoint path");
}
extern "C" recomp_func_t* get_function(int32_t address){++swallowed_dispatches;need(uint32_t(address)==0x8002F5F4,"Literal42148 dispatches only the actual saved pending death callback");return swallowed_final;}
#endif
struct Fixture {
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* ram=memory.data();recomp_context c{};
 static constexpr uint32_t player=0x80010000,data=0x80011000,stack=0x800F0000;
 unsigned color=0;
 explicit Fixture(unsigned pc=0,unsigned hp=5,unsigned eggs=0,bool enable_retry=true):color(pc){
  need(r::configure("checkpoint-runtime-synthetic",1,true,false),"Explicit synthetic diagnostic Runtime");r::rooms::configure(r::session.plan());q::allow_retries(enable_retry);
  actor(ram,player,0x4001);word(ram,player+0x1F4,data);word(ram,0x800F8828,player);word(ram,0x807F0008,pc);word(ram,0x800F8DB0,pc<8?pc:0);health(hp);
  word(ram,player+0x44,std::bit_cast<uint32_t>(500.34f));word(ram,player+0x8C,std::bit_cast<uint32_t>(500.34f));
  byte(ram,0x800F7EA6,1);c.r29=n::base(stack);c.r31=n::base(0x80000300);c.r16=0x12345678;
  r::physical_room=r::logical_room=162;r::current_vi=100;r::variants::disable();need(r::session.enter_fight(162,player),"Model-only first fight entered");
  r::first_player=player;r::first_color=pc;set_eggs(eggs);need(q::capture_baseline(ram,100),"Initial same-Yoshi inventory captured without a checkpoint wake");
 }
 void health(unsigned hp){word(ram,0x800F8DB4+(color<8?color:0)*4,hp);}
 uint32_t attach_checkpoint(unsigned kind=0x4507){
  constexpr uint32_t point=0x803C0000;actor(ram,point,kind);word(ram,player+0x180,point);half(ram,player+0x17E,0);byte(ram,player+0x17C,0);half(ram,point+0x184,1);word(ram,point+0x188,player);
  word(ram,point+0x8C,std::bit_cast<uint32_t>(500.3f));word(ram,player+0x44,std::bit_cast<uint32_t>(std::bit_cast<float>(n::word(ram,player+0x8C))-500.3f));return point;
 }
 void set_eggs(unsigned count){
  for(unsigned i=0;i<20;++i)word(ram,player+0x188+i*4,0);half(ram,player+0x184,0);byte(ram,data+0x12C8,0);half(ram,data+0x12BE,0);
  for(unsigned i=0;i<8;++i)word(ram,data+0x11FC+i*24,0);
  for(unsigned i=0;i<count;++i){const auto a=0x80280000+i*0x400;actor(ram,a,0x4004);word(ram,a+0x180,player);word(ram,a+0x1EC,(color+i)%14);for(unsigned axis=0;axis<3;++axis)word(ram,a+0x78+4*axis,std::bit_cast<uint32_t>(.56f));word(ram,player+0x188+i*4,a);word(ram,data+0x11FC+i*24,a);}
  half(ram,player+0x184,count);byte(ram,data+0x12C8,count);
 }
 void reach(unsigned completed){
  while(r::session.snapshot().completed<completed){
   if(r::session.snapshot().phase==b::Phase::Rest)need(r::session.use_rest_pot(),"Synthetic ordinary onward pot");
   if(r::session.snapshot().phase!=b::Phase::Fighting)need(r::session.enter_fight(b::room(r::session.current().family),player),"Synthetic next encounter");
   const auto index=r::session.snapshot().index;auto& h=r::history[index];h.player=player;h.color=color;
   for(unsigned i=0;i<r::session.current().count;++i){const auto a=0x80380000+index*0x1000+i*0x200;need(r::session.bind(i,a),"Exact synthetic boss binding");auto proof=b::NativeReceipt{index,i,110+index,a,true,true,true,true,r::session.snapshot().retry_epoch};need(r::session.defeat(proof),"Explicit synthetic causal receipt");h.members[i]=proof;}
   need(r::session.use_victory_pot()&&r::session.arrive_rest(player),"Synthetic rest after each complete member set");
  }
 }
 void observe_member(unsigned member,bool active){
  const auto kind=r::session.current().members[member].kind;const auto cfg=r::witnesses::spec(*r::witnesses::family_for_kind(kind));
  const uint32_t root=0x80300000+member*0x4000,private_data=root+0x1000,resource=root+0x2000;actor(ram,root,kind);word(ram,root+0x1F4,private_data);word(ram,root+0x168,resource);r::bound[member]=root;need(r::variants::bind(ram,member,root)&&r::observers[member].bind(root),"Exact synthetic native-family observer and current-generation binding");
  const unsigned hits=r::session.current().members[member].hits;word(ram,cfg.counter_in_private?private_data+0x24:root+0x1E4,kind==0x4072?hits:kind==0x4599?hits-1:0);
  r::overlays[cfg.overlay_section]=0x80600000;r::overlays[285]=0x80700000;r::current_vi=150;word(ram,resource+0x40,r::overlays[cfg.overlay_section]);word(ram,resource+0x44,r::overlays[cfg.overlay_section]+(kind==0x4599?0x18B8u:cfg.init-cfg.rom_base));
  char init_name[14]{},active_name[14]{};std::snprintf(init_name,sizeof(init_name),"func_%08X",cfg.init);std::snprintf(active_name,sizeof(active_name),"func_%08X",cfg.active);
  const auto saved=c;c.r4=n::base(root);r::observers[member].observe_entry(ram,&c,init_name,r::witness_frame(member));
  if(active){byte(ram,root+0x18,cfg.active_state);byte(ram,root+0x30,cfg.active_substate);word(ram,root+0x20,r::overlays[cfg.overlay_section]+cfg.active-cfg.rom_base);r::observers[member].observe_entry(ram,&c,active_name,r::witness_frame(member));need(r::observers[member].snapshot().actors[0].active_seen,"Only the exact native-shaped family callback establishes activity");}
  c=saved;byte(ram,0x800F7EA5,0);byte(ram,0x800F7EA6,0);
 }
 void observed_family(b::Family family,bool active=false){
  unsigned target=10;for(unsigned i=1;i<10;++i)if(r::session.plan().fights[i].family==family){target=i;break;}need(target<10,"Real seed1 contains the requested observer family");
  reach(target);need(r::session.use_rest_pot(),"Synthetic normal onward entry to observer family");r::enter_room(r::source_for(r::session.snapshot().index));
  need(r::session.current().family==family&&r::session.enter_fight(r::logical_room,player),"Synthetic encounter uses its real seed1 native family");observe_member(0,active);
 }
 void observed_companion(bool candy,bool active){
  auto plan=r::session.plan();unsigned target=6;b::Random random{17};
  if(candy){for(unsigned attempt=0;attempt<1000;++attempt){plan.fights[target]=b::encounter(target+1,b::Family::Candy,0,random,plan.config);if(plan.fights[target].genie_companion)break;}need(plan.fights[target].genie_companion,"Native encounter builder produces the legal Candy+Genie companion");}
  else{target=10;for(unsigned i=1;i<6;++i)if(plan.fights[i].family==b::Family::Lips){target=i;break;}need(target<6,"Seed1 guaranteed family includes Lips");plan.fights[target]=b::encounter(target+1,b::Family::Lips,2,random,plan.config);}
  need(b::valid(plan)&&r::session.begin("checkpoint-runtime-mixed",plan,true),"Mixed encounter uses a valid complete plan and ordinary Session entry");r::rooms::configure(plan);
  reach(target);need(r::session.use_rest_pot(),"Synthetic normal onward entry to mixed encounter");r::enter_room(r::source_for(r::session.snapshot().index));need(r::session.enter_fight(r::logical_room,player),"Mixed encounter enters its configured owner source");
  need(r::session.current().count==2&&r::session.current().members[1].kind==(candy?0x4072:0x4599),"Only the legal borrowed native member is selected for control evidence");observe_member(1,active);
 }
 void lips(bool active=false){observed_family(b::Family::Lips,active);}
 void fatal(unsigned hp,unsigned vi=200,unsigned source=162){health(hp);r::physical_room=source;r::current_vi=vi;byte(ram,player+0x18,0xA);byte(ram,player+0x30,0x10);word(ram,data+0xC0,0x8002F5F4);word(ram,data+0xD8,3);word(ram,player+0x44,std::bit_cast<uint32_t>(451.74f));word(ram,player+0x8C,std::bit_cast<uint32_t>(457.74f));word(ram,player+0xCC,std::bit_cast<uint32_t>(-6.f));word(ram,player+0xD8,std::bit_cast<uint32_t>(-5.9f));word(ram,player+0x114,std::bit_cast<uint32_t>(-.25f));c.r4=n::base(player);c.r31=n::base(0x800301D4);}
 void leave(){const auto regs=c;need(q::intercept_death(ram,&c),"Strict final death callback intercepted before native life decrement");need(std::memcmp(&regs,&c,sizeof(c))==0,"Native route call preserves all caller registers");}
 void collect_later_heart(){
  namespace h=workshop::boss_rest_heart0260;
  r::physical_room=83;r::current_vi=175;r::rest_heart.enter_room(83);const auto frame=r::rest_heart_frame();
  const auto request=r::rest_heart.request_spawn(ram,frame);need(bool(request),"Big heart becomes eligible after actual model fifth victory");
  constexpr uint32_t heart=0x80500000,resource=0x80501000,metadata=0x80502000,definition=0x80503000,factory=0x80600000;
  actor(ram,heart,h::kind);word(ram,heart+0x168,resource);word(ram,heart+0x174,metadata);word(ram,heart+0x1F4,0x80504000);word(ram,resource+0x40,factory);word(ram,metadata,definition);
  word(ram,definition+4,0);word(ram,definition+8,0xF);word(ram,definition+0xC,0x000F000A);word(ram,definition+0x28,factory+0x334);
  word(ram,heart+0x20,factory+0x398);word(ram,heart+0x3C,std::bit_cast<uint32_t>(h::spawn_x));word(ram,heart+0x40,std::bit_cast<uint32_t>(h::spawn_y));word(ram,heart+0x44,std::bit_cast<uint32_t>(h::spawn_z));
  need(r::rest_heart.bind_spawn(ram,frame,*request,heart),"Synthetic exact40C5 resource/collider/position identity bound");
  auto event=[&](const char* name,uint32_t a,uint32_t other,uint32_t ra){recomp_context e{};e.r4=n::base(a);e.r5=n::base(other);e.r31=n::base(ra);return r::rest_heart.trace(ram,&e,name,frame);};
  // Synthetic-shaped contact/effect boundaries exercise the real host observer.
  // Real native collection bodies are covered by BossRestHeart0260Checks.
  event("func_806E7574",heart,player,0);event("func_806E7E4C",heart,3,factory+0x388);
  byte(ram,heart+0x18,3);word(ram,heart+0x20,factory+0x4E8);event("func_806E7728",heart,0,0);
  event("func_800447C8",heart,0,factory+0x560);r::forget(heart);
  word(ram,h::heart_count_address,1);event("func_80049624",0,0,factory+0x578);event("func_8002B428",8,0,factory+0x580);health(8);
  need(event("func_8002B3A0",0,0,0x8002B460)&&r::rest_heart.snapshot().consumed,"Real host observer consumes only after ordered contact/retirement/counter/heal proof");
 }
 void arrive(unsigned room,unsigned vi=300,bool reset_stats=false){
  r::enter_room(room);need(!r::failed,"Retry room is admitted through its dedicated phase");
  if(reset_stats){c.r31=n::base(workshop::boss_carry0260::reset_caller);q::trace_carry(ram,&c,"func_8006CFC0");word(ram,0x800F8958,0);c.r31=n::base(workshop::boss_carry0260::hud_caller);q::trace_carry(ram,&c,"func_800519C0");}
  byte(ram,player+0x18,1);byte(ram,player+0x30,0x11);byte(ram,player+0x17D,2);byte(ram,player+0x165,0);word(ram,player+0x20,0x80043450);r::ready_info=0x80022000;
  r::frame(ram,&c,vi,[](uint32_t,uint8_t*,recomp_context*){forbidden();});
 }
};
int main(){try{
#ifdef BOSS_GROUP_NATIVE_GETTERS
 for(uint32_t overlay:{0x80200000u,0x80310000u,0x80600000u}){std::vector<uint8_t> memory(0x800000);auto* ram=memory.data();recomp_context c{};constexpr uint32_t request=0x80010000,stack=0x807C0000;c.r4=n::base(request);c.r29=n::base(stack);half(ram,request,0x4599);native_sections[197]=int32_t(overlay);native_sections[16]=int32_t(overlay);native_sections[120]=int32_t(overlay);
  const auto before=memory;native_getters::func_806ADD40(ram,&c);need(uint32_t(c.r2)==overlay+0x18B8&&uint32_t(c.r2)!=overlay+0x1840&&memory==before,"Literal native getter returns public AF5F8, never internal AF580, with no RAM mutation");
  need(r::member_resource_constructor(r::witnesses::spec(r::witnesses::Family::invisible))-0x806ADD40==uint32_t(c.r2)-overlay,"Runtime constructor matches the independently executed relocated4599 getter");
  native_getters::func_8042A8A0(ram,&c);need(uint32_t(c.r2)==overlay+0x2A9C&&r::member_resource_constructor(r::witnesses::spec(r::witnesses::Family::genie))-0x8042A8A0==uint32_t(c.r2)-overlay,"Literal Genie getter already agrees with its observer initializer");
  native_getters::func_80557B50(ram,&c);need(uint32_t(c.r2)==overlay+0xAEC&&r::member_resource_constructor(r::witnesses::spec(r::witnesses::Family::giant_lips))-0x80557B50==uint32_t(c.r2)-overlay,"Literal Lips getter already agrees with its observer initializer");
 }
#endif
 for(const auto family:{b::Family::Lips,b::Family::Genie,b::Family::Invisible}){Fixture f;f.observed_family(family);const auto before=f.memory;r::remember_controls(f.ram,b::Phase::Fighting);need(!r::flags_ready&&!r::witnessed_zero_controls(f.ram,Fixture::player)&&f.memory==before,"A native-family intro at0/0 cannot establish a playable controls baseline before observed boss activity");}
 for(unsigned fault=0;fault<8;++fault){Fixture f;f.lips(true);
  if(fault==0)byte(f.ram,Fixture::player+0x18,0xA);
  if(fault==1)byte(f.ram,Fixture::player+0x17D,0);
  if(fault==2)byte(f.ram,Fixture::player+0x165,1);
  if(fault==3)word(f.ram,0x800F8828,0x80300000);
  if(fault==4)half(f.ram,Fixture::player+12,0x4004);
  if(fault==5)r::restore_pending=true;
  if(fault==6)byte(f.ram,0x800F7EA5,1);
  const auto before=f.memory;r::remember_controls(f.ram,fault==7?b::Phase::ToFight:b::Phase::Fighting);need(!r::flags_ready&&f.memory==before,"Unreleased, foreign, locked, nonfighting or already-pending player cannot replace the real controls baseline");
 }
 {Fixture f;f.lips(true);const auto before=f.memory;r::remember_controls(f.ram,b::Phase::Fighting);need(r::flags_ready&&!r::flags5&&!r::flags6&&f.memory==before,"Native-observed Lips activity with a playable Yoshi preserves actual0/0 controls without forcing6 to1");
  need(r::session.bind(0,0x80300000)&&r::session.defeat({1,0,160,0x80300000,true,true,true,true}),"Explicit model-only victory isolates control restoration from native defeat admission");
  r::restore_pending=true;r::exit_pot=0x80320000;actor(f.ram,r::exit_pot,0x4273);byte(f.ram,0x800F7EA5,1);byte(f.ram,0x800F7EA6,1);
  const auto context=f.c;const auto health=n::word(f.ram,0x800F8DB4),resets=control_resets;expect_control_reset=true;r::frame(f.ram,&f.c,170,[](uint32_t,uint8_t*,recomp_context*){forbidden();});expect_control_reset=false;
  need(!r::failed&&!r::restore_pending&&control_resets==resets+1&&!n::byte(f.ram,0x800F7EA5)&&!n::byte(f.ram,0x800F7EA6)&&n::word(f.ram,0x800F8DB4)==health&&!std::memcmp(&context,&f.c,sizeof(context)),"Victory restores the actual0/0 baseline after native reset with unchanged HP and caller registers");
 }
 {Fixture f;const auto before=f.memory;r::remember_controls(f.ram,b::Phase::Fighting);need(r::flags_ready&&!r::flags5&&r::flags6==1&&f.memory==before,"Existing actual0/1 controls remain a valid baseline for ordinary families");}
 {Fixture f;byte(f.ram,0x800F7EA6,0);const auto before=f.memory;r::remember_controls(f.ram,b::Phase::Fighting);need(!r::flags_ready&&!r::witnessed_zero_controls(f.ram,Fixture::player)&&f.memory==before,"Zero controls do not expand to a family without this native activity witness");}
 for(const auto family:{b::Family::Genie,b::Family::Invisible}){
  for(unsigned fault=0;fault<16;++fault){Fixture f;f.observed_family(family,true);
   if(fault==0)r::variants::state.enabled=false;
   if(fault==1)++r::variants::state.encounter;
   if(fault==2)r::variants::state.room=83;
   if(fault==3)r::variants::state.members[0].birth=0;
   if(fault==4)r::variants::state.members[0].actor=0;
   if(fault==5)++r::observer_epochs[0];
   if(fault==6)++r::variant_epoch;
   if(fault==7)word(f.ram,0x803001F4,0x80304000);
   if(fault==8)word(f.ram,0x80302040,0x80600004);
   if(fault==9)word(f.ram,0x80302044,0x80600000);
   if(fault==10)byte(f.ram,Fixture::player+0x17D,0);
   if(fault==11)word(f.ram,Fixture::data+0x12EC,1);
   if(fault==12)r::observers[0].forget(0x80300000);
   if(fault==13)half(f.ram,0x8030000C,0x4004);
   if(fault==14)r::logical_room=83;
   if(fault==15)byte(f.ram,0x800F7EA5,1);
   const auto before=f.memory;need(!r::witnessed_zero_controls(f.ram,Fixture::player),"Stale generation/epoch/source, foreign native identity, intro lock or platform transport cannot admit zero controls");r::remember_controls(f.ram,b::Phase::Fighting);need(!r::flags_ready&&f.memory==before,"Rejected live control evidence cannot replace a baseline or write native RAM");
  }
  {Fixture f;f.observed_family(family,true);const auto before=f.memory;
   need(!r::flags_ready&&r::witnessed_zero_controls(f.ram,Fixture::player)&&!r::witnessed_zero_controls(f.ram,Fixture::player+4)&&f.memory==before,"Shared active-family control witness is read-only and independent of an already-saved baseline");r::remember_controls(f.ram,b::Phase::Fighting);
   need(r::flags_ready&&!r::flags5&&!r::flags6&&f.memory==before,"Native-observed Genie and Inviso activity capture their actual playable0/0 controls");
   const auto index=r::session.snapshot().index;for(unsigned member=0;member<r::session.current().count;++member){const auto root=0x80300000+member*0x4000;need(r::session.bind(member,root)&&r::session.defeat({index,member,160,root,true,true,true,true}),"Explicit model-only member victories isolate control restoration from native defeat admission");}
   r::restore_pending=true;r::exit_pot=0x80320000;actor(f.ram,r::exit_pot,0x4273);byte(f.ram,0x800F7EA5,1);byte(f.ram,0x800F7EA6,1);
   const auto context=f.c;const auto health=n::word(f.ram,0x800F8DB4),resets=control_resets;expect_control_reset=true;r::frame(f.ram,&f.c,170,[](uint32_t,uint8_t*,recomp_context*){forbidden();});expect_control_reset=false;
   need(!r::failed&&!r::restore_pending&&control_resets==resets+1&&!n::byte(f.ram,0x800F7EA5)&&!n::byte(f.ram,0x800F7EA6)&&n::word(f.ram,0x800F8DB4)==health&&!std::memcmp(&context,&f.c,sizeof(context)),"Genie and Inviso victory restore saved0/0 exactly, preserving HP and all native caller registers");
  }
 }
 for(const bool candy:{true,false}){
  {Fixture f;f.observed_companion(candy,false);const auto before=f.memory;r::remember_controls(f.ram,b::Phase::Fighting);need(!r::flags_ready&&!r::witnessed_zero_controls(f.ram,Fixture::player)&&f.memory==before,"A legal borrowed companion still cannot admit0/0 during its unobserved introduction");}
  {Fixture f;f.observed_companion(candy,true);const auto& proof=r::observers[1].snapshot();const auto vanilla=r::witnesses::spec(proof.family).vanilla_room;const auto owner=r::logical_room;const auto before=f.memory;
   need(!r::bound[0]&&proof.source_room==owner&&vanilla!=owner&&owner==(candy?162u:165u)&&r::witnessed_zero_controls(f.ram,Fixture::player)&&f.memory==before,"Only the active Candy+Genie or Lips+Inviso companion proves controls in the configured owner room, independently of its vanilla room");r::remember_controls(f.ram,b::Phase::Fighting);need(r::flags_ready&&!r::flags5&&!r::flags6&&f.memory==before,"A legal mixed encounter captures the exact native0/0 pair without writes");
   r::flags_ready=false;r::variants::state.room=vanilla;need(!r::witnessed_zero_controls(f.ram,Fixture::player),"Borrowed companion's vanilla room cannot replace the owning encounter source");r::remember_controls(f.ram,b::Phase::Fighting);need(!r::flags_ready&&f.memory==before,"Foreign mixed-encounter source remains rejected without native writes");
  }
 }
 // Real resource4599 publishes AF5F8 (getter ADD40), not the internal
 // AF580 callback used to discover observer initialization. Both orders of
 // a mixed group must keep their *surviving* typed control witness.
 for(unsigned dead=0;dead<2;++dead){Fixture f;f.observed_companion(false,true);f.observe_member(0,true);
  const auto index=r::session.snapshot().index,quota=r::session.current().birds;
  need(quota>0&&r::session.current().count==2,"Real Lips+Inviso encounter owns a renewable group quota");
  for(unsigned member=0;member<2;++member)need(r::session.bind(member,r::bound[member]),"Group test binds each actual observer root to Session");
  r::current_vi=151;const auto initial_frame=r::variant_frame(f.ram);need(initial_frame.proven_zero_controls,"The actual public constructor and observed native activity admit initial0/0");
  r::variants::frame(f.ram,initial_frame);const auto initial=r::variants::take_actions();need(initial.size()==quota,"Native group begins with its complete bird quota");
  for(const auto& action:initial){const auto bird=0x80370000+action.member*0x400;actor(f.ram,bird,0x4526);need(r::variants::bind_bird(f.ram,action.member,bird)&&r::variants::renewable_actor(f.ram,bird),"Each native-shaped yellow bird retains zero-score provenance");}
  const b::NativeReceipt first{index,dead,160,r::bound[dead],true,true,true,true,r::session.snapshot().retry_epoch};
  need(r::session.defeat(first)&&r::variants::acknowledge_defeat(first)&&r::session.snapshot().phase==b::Phase::Fighting&&r::session.snapshot().defeated==(1u<<dead),"Explicit host-only first receipt cannot finish a two-member group");
  // Defeated root storage can already have been retired/reused. It must not
  // be the identity that authorizes replenishment for the remaining member.
  half(f.ram,r::bound[dead]+12,0x4592);
  need(r::witnessed_zero_controls(f.ram,Fixture::player),dead?"Living Lips alone keeps its observed0/0 admission":"Living Inviso alone uses its real AF5F8 public constructor");
  if(!dead){const auto survivor=r::bound[1],resource=n::word(f.ram,survivor+0x168);word(f.ram,resource+0x44,r::overlays[197]+0x1840);const auto old=f.memory;
   need(!r::witnessed_zero_controls(f.ram,Fixture::player)&&f.memory==old,"Old internalAF580 is not the real resource initializer and cannot impersonate live Inviso");word(f.ram,resource+0x44,r::overlays[197]+0x18B8);
   const auto birth=r::variants::state.members[1].birth;r::variants::state.members[1].birth=0;need(!r::witnessed_zero_controls(f.ram,Fixture::player),"Missing surviving generation cannot inherit the defeated Lips control witness");r::variants::state.members[1].birth=birth;
  }
  const auto eaten=r::variants::birds[0].actor;r::variants::forget(eaten);const auto delay=r::variants::ticks(r::session.current().bird_respawn_ms);const auto before=f.memory;
  r::current_vi+=delay-1;r::variants::frame(f.ram,r::variant_frame(f.ram));need(r::variants::take_actions().empty(),"First member death does not shorten the actual configured replenishment delay");
  ++r::current_vi;r::variants::frame(f.ram,r::variant_frame(f.ram));const auto refill=r::variants::take_actions();need(refill.size()==1&&refill[0].operation==r::variants::Operation::BirdSpawn&&refill[0].member==0&&r::variants::action_valid(f.ram,refill[0])&&f.memory==before,"Either surviving member keeps exactly the consumed slot renewable without RAM writes");
  const unsigned survivor=1-dead;const b::NativeReceipt last{index,survivor,r::current_vi+1,r::bound[survivor],true,true,true,true,r::session.snapshot().retry_epoch};need(r::session.defeat(last)&&r::variants::acknowledge_defeat(last)&&r::session.snapshot().phase==b::Phase::Victory&&r::session.snapshot().defeated==3,"Only the last explicit host receipt finishes the group");
  for(unsigned slot=0;slot<quota;++slot)if(r::variants::birds[slot].actor)r::variants::forget(r::variants::birds[slot].actor);
  const auto stopped=r::variants::snapshot().active_ticks;r::current_vi+=delay+1;const auto finished=r::variant_frame(f.ram);r::variants::frame(f.ram,finished);need(!finished.combat_active&&!finished.proven_zero_controls&&r::variants::take_actions().empty()&&r::variants::snapshot().active_ticks==stopped&&f.memory==before,"Complete group victory stops replenishment without fake receipts, control changes or catch-up ticks");
 }
 {Fixture f;f.lips(false);const auto quota=r::session.current().birds;need(quota==r::session.plan().config.reference_birds&&quota>0,"Configured Lips encounter supplies its actual renewable bird quota");
  const auto intro_memory=f.memory;const auto intro=r::variant_frame(f.ram);r::variants::frame(f.ram,intro);need(intro.combat_active&&!intro.proven_zero_controls&&!r::variants::snapshot().active_ticks&&r::variants::take_actions().empty()&&f.memory==intro_memory,"Runtime intro0/0 carries no live proof, starts no bird quota and writes no native RAM");
  const auto cfg=r::witnesses::spec(r::witnesses::Family::giant_lips);const auto root=r::bound[0];const auto saved=f.c;f.c.r4=n::base(root);byte(f.ram,root+0x18,cfg.active_state);byte(f.ram,root+0x30,cfg.active_substate);word(f.ram,root+0x20,r::overlays[cfg.overlay_section]+cfg.active-cfg.rom_base);r::observers[0].observe_entry(f.ram,&f.c,"func_80558794",r::witness_frame(0));f.c=saved;
  r::current_vi=151;const auto active=r::variant_frame(f.ram);const auto active_memory=f.memory;need(!r::flags_ready&&active.proven_zero_controls&&r::observers[0].snapshot().actors[0].active_seen,"Runtime obtains zero-control supply proof from genuine observer activity and released P1 before baseline capture");r::variants::frame(f.ram,active);const auto initial=r::variants::take_actions();need(initial.size()==quota&&r::variants::snapshot().active_ticks==1&&f.memory==active_memory,"The common Runtime frame starts exactly the native bird quota at playable0/0 without altering RAM or control flags");
  for(const auto& action:initial){need(action.operation==r::variants::Operation::BirdSpawn&&action.member<quota&&action.function==0x8004613C&&r::variants::action_valid(f.ram,action),"Each quota request retains its bounded native spawn action and live generation");const auto bird=0x80370000+action.member*0x400;actor(f.ram,bird,0x4526);need(r::variants::bind_bird(f.ram,action.member,bird),"Explicit synthetic native bird creation binds only the requested quota slot");}
  ++r::current_vi;r::variants::frame(f.ram,r::variant_frame(f.ram));need(r::variants::take_actions().empty(),"Already-populated native bird quota does not duplicate requests");
  const auto retired=r::variants::birds[0].actor;r::variants::forget(retired);const auto delay=r::variants::ticks(r::session.current().bird_respawn_ms);const auto supply_memory=f.memory;r::current_vi+=delay-1;r::variants::frame(f.ram,r::variant_frame(f.ram));need(r::variants::take_actions().empty(),"Playable0/0 retains the configured bird respawn delay");++r::current_vi;r::variants::frame(f.ram,r::variant_frame(f.ram));const auto respawn=r::variants::take_actions();
  need(respawn.size()==1&&respawn[0].operation==r::variants::Operation::BirdSpawn&&respawn[0].member==0&&r::variants::action_valid(f.ram,respawn[0])&&f.memory==supply_memory&&!n::byte(f.ram,0x800F7EA5)&&!n::byte(f.ram,0x800F7EA6),"At the exact configured delay, playable0/0 renews only the missing bird slot and preserves native RAM including both control flags");
 }
 for(unsigned color=0;color<14;++color)for(unsigned hp:{0u,5u})for(unsigned eggs:{0u,2u}){
  Fixture f(color,hp,eggs);const auto payload=q::saved_inventory[0];f.fatal(2);need(q::notice_death(f.ram,200)&&q::waiting_for_retry()&&!r::failed,"Allowed real-shaped fatality waits without ending UI session");f.leave();
  need(r::session.snapshot().phase==b::Phase::RetryToFight&&r::session.snapshot().retry_epoch==1&&!r::session.snapshot().completed,"Unwoken point resumes first fight, no fake victory");
  f.arrive(162);need(!r::failed&&!q::waiting_for_retry()&&r::session.snapshot().phase==b::Phase::Fighting&&q::retry_restored_epoch()==1,"Native-shaped release completes exactly one same-actor retry");
  need(inv::matches(f.ram,payload)&&r::session.snapshot().player==Fixture::player&&r::first_color==color,"Exact saved HP including0, eggs/palettes/order and all14 Yoshi colours restored");
  need(!q::restore_arrival(f.ram,&f.c,301),"Repeated arrival cannot manufacture eggs or healing");
 }
 {Fixture f(10,5,0);f.reach(3);f.health(6);f.set_eggs(2);need(inv::capture(f.ram,Fixture::player,150,q::saved_inventory[1])&&q::pose::capture(f.ram,q::saved_inventory[1],q::saved_pose[1])&&r::session.awaken_checkpoint(3),"Synthetic explicit wake stores activation inventory and stable depth at third rest");
  f.reach(5);const auto first=r::history[0].members[0];workshop::boss_tree0240::state.totalConsumed=8;workshop::boss_tree0240::state.regenerations=1;workshop::boss_tree0240::state.consumedmask=7;
  const auto order=workshop::boss_tree_hybrid0260::snapshot().order;f.collect_later_heart();need(r::rest_heart.snapshot().collected_vi>q::saved_inventory[1].activation_vi,"Heart was collected AFTER checkpoint activation");f.set_eggs(3);f.fatal(2,200,83);f.leave();
  need(n::word(f.ram,r::route_address+8)==workshop::u32(workshop::boss_rest_data0240::rest_descriptor,0x28)&&n::word(f.ram,r::route_address+12)==std::bit_cast<uint32_t>(256.f),"Warm rest retry uses the cloned ground camera0/256, not immutable native camera208");
  need(r::session.snapshot().completed==3&&r::session.snapshot().max_completed==5&&r::history[0].members[0].actor==first.actor&&r::history[0].members[0].vi==first.vi,"Retry retains original causal prefix and maximum actual progress");
  need(!r::history[3].members[0].actor&&!r::history[4].members[0].actor&&r::bound==std::array<uint32_t,4>{}&&r::scale_scopes.empty(),"Suffix receipts, old actor bindings and scopes discarded");
  f.arrive(83,300,true);need(!r::failed&&r::session.snapshot().phase==b::Phase::Rest&&inv::matches(f.ram,q::saved_inventory[1]),"Checkpoint3 uses rest83 and exact activation inventory");
  need(n::word(f.ram,0x800F8958)==1&&workshop::boss_tree0240::snapshot().totalConsumed==8&&workshop::boss_tree0240::snapshot().regenerations==1&&workshop::boss_tree_hybrid0260::snapshot().order==order,"Current heart statistic, tree budget and hybrid permutation survive rollback");
  need(r::rest_heart.snapshot().consumed&&!r::rest_heart.request_spawn(f.ram,r::rest_heart_frame()),"Heart collected AFTER activation stays consumed and cannot respawn after retry");
  need(r::rest_heart_frame().completed==5,"Reached fifth boss keeps big-heart eligibility after checkpoint rollback");
 }
 for(unsigned fault=0;fault<8;++fault){Fixture f;const auto inventory=q::saved_inventory[0];auto plane=q::saved_pose[0];const auto old=plane;
  if(fault==0)word(f.ram,Fixture::player+0x44,std::bit_cast<uint32_t>(451.f));
  if(fault==1){word(f.ram,Fixture::player+0x44,0x7FC00000);word(f.ram,Fixture::player+0x8C,0x7FC00000);}
  if(fault>=2&&fault<=4)word(f.ram,Fixture::player+(fault==2?0xCC:fault==3?0xD8:0x114),std::bit_cast<uint32_t>(-.1f));
  if(fault==5)byte(f.ram,Fixture::player+0x18,0xA);
  if(fault==6)word(f.ram,0x800F8828,Fixture::player+4);
  if(fault==7)word(f.ram,0x807F0008,1);
  const auto before=f.memory;need(!q::pose::capture(f.ram,inventory,plane)&&plane==old&&f.memory==before,"Unstable depth, native death or changed identity cannot replace an activation plane");
 }
 for(unsigned kind=0x4507;kind<=0x450A;++kind){Fixture f;const auto baseline=q::saved_pose[0];f.health(3);f.set_eggs(2);const auto point=f.attach_checkpoint(kind);inv::Payload inventory;q::pose::Payload plane;
  need(inv::capture(f.ram,Fixture::player,150,inventory)&&inventory.hp==3&&inventory.count==2,"HP and eggs are captured at the true attached wake, not at the earlier fallback");
  const auto before=f.memory;need(q::pose::capture(f.ram,inventory,plane,point,&baseline)&&plane.z==baseline.z&&plane.activation_vi==150&&n::word(f.ram,Fixture::player+0x44)!=plane.z&&f.memory==before,"All four owned native wake parents capture WORLD depth at the wake VI without changing their local attachment pose");
  need(!q::pose::capture(f.ram,inventory,plane)&&!q::pose::capture(f.ram,inventory,plane,point+4,&baseline),"An attachment needs the exact proved wake parent; neither default capture nor a foreign parent is admitted");
  f.fatal(0);const auto attached=f.memory;q::pose::Restore restore;need(!restore.apply(f.ram,plane,inventory,200,1)&&f.memory==attached,"A final death must be detached before local depth can be restored as world depth");
 }
 for(unsigned fault=0;fault<7;++fault){Fixture f;const auto baseline=q::saved_pose[0];const auto point=f.attach_checkpoint();inv::Payload inventory;need(inv::capture(f.ram,Fixture::player,150,inventory),"Synthetic attached wake inventory");auto plane=baseline;const auto old=plane;auto expected=baseline;
  if(fault==0)half(f.ram,point+12,0x4004);
  if(fault==1)word(f.ram,point+0x188,0);
  if(fault==2)half(f.ram,point+0x184,2);
  if(fault==3)byte(f.ram,Fixture::player+0x17C,1);
  if(fault==4)word(f.ram,Fixture::player+0x44,std::bit_cast<uint32_t>(4.f));
  if(fault==5)expected.z=std::bit_cast<uint32_t>(400.f);
  if(fault==6)expected.activation_vi=151;
  const auto before=f.memory;need(!q::pose::capture(f.ram,inventory,plane,point,&expected)&&plane==old&&f.memory==before,"Foreign or inconsistent attachment, native transform or baseline plane cannot authorize a checkpoint pose");
 }
 {Fixture f;const auto plane=q::saved_pose[0];const auto inventory=q::saved_inventory[0];f.fatal(0);f.leave();
  f.fatal(0,201);const auto before=f.memory;need(!q::pose_restore.apply(f.ram,plane,inventory,200,1)&&f.memory==before,"A repeated retry epoch cannot rewrite depth or motion a second time");
 }
 {Fixture f(12,5,2);const auto plane=q::saved_pose[0];for(unsigned epoch=1;epoch<=2;++epoch){f.fatal(0,200*epoch);f.leave();f.arrive(162,200*epoch+100);
   need(!r::failed&&q::pose_restore.last_epoch()==epoch&&q::saved_pose[0]==plane&&q::pose::matches(f.ram,plane,q::saved_inventory[0])&&inv::matches(f.ram,q::saved_inventory[0]),"A second real-shaped death restores the same activation plane and inventory under a new epoch");
  }
 }
 for(unsigned fault=0;fault<5;++fault){Fixture f;f.fatal(0);if(fault==0)f.c.r31=n::base(0x80000300);if(fault==1)f.c.r4=n::base(Fixture::player+4);if(fault==2)word(f.ram,Fixture::data+0xC0,0);if(fault==3)word(f.ram,Fixture::data+0xD8,2);if(fault==4)byte(f.ram,Fixture::player+0x30,2);const auto before=f.memory;const auto regs=f.c;need(!q::intercept_death(f.ram,&f.c)&&f.memory==before&&!std::memcmp(&regs,&f.c,sizeof(regs)),"Unproved caller/player/fatality cannot trigger retry mutation");}
 {Fixture f;f.fatal(0);f.c.r31=n::base(0x80030780);word(f.ram,Fixture::player+0x38,uint32_t(-41));need(q::death_call(f.ram,&f.c),"Source-proved falling-player death caller accepted");word(f.ram,Fixture::player+0x38,uint32_t(-40));need(!q::death_call(f.ram,&f.c),"Fall caller requires actual native threshold");}
 // Source42148 is the swallowed A/C route; the old30194/fall routes above
 // stay unchanged. No portable death or guessed liveA1 may authorize it.
 for(unsigned fault=0;fault<15;++fault){Fixture f;f.fatal(0);byte(f.ram,Fixture::player+0x30,0xC);word(f.ram,Fixture::player+0x20,0x8003085C);f.c.r31=n::base(0x8004217C);const auto sp=uint32_t(f.c.r29);
  word(f.ram,sp+0x14,0x80030878);word(f.ram,sp+0x1C,0x8002F5F4);word(f.ram,sp+0x20,Fixture::player);word(f.ram,sp+0x24,Fixture::data);
  switch(fault){case 0:break;case 1:f.c.r31+=4;break;case 2:byte(f.ram,Fixture::player+0x30,2);break;case 3:word(f.ram,Fixture::player+0x20,0x80031230);break;case 4:word(f.ram,Fixture::data+0xC0,0);break;case 5:word(f.ram,sp+0x14,0x80030874);break;case 6:word(f.ram,sp+0x1C,0x8002D3FC);break;case 7:word(f.ram,sp+0x20,Fixture::player+4);break;case 8:word(f.ram,sp+0x24,Fixture::data+4);break;case 9:f.c.r29=0;break;case 10:word(f.ram,Fixture::player+0x180,0x80310000);break;case 11:word(f.ram,0x800F8828,Fixture::player+4);break;case 12:byte(f.ram,Fixture::player+0x18,1);break;case 13:half(f.ram,Fixture::player+12,0x4002);break;case 14:f.c.r4=n::base(Fixture::player+4);break;}
  const auto before=f.memory;const auto regs=f.c;need(q::death_call(f.ram,&f.c)==(fault==0||fault==10)&&f.memory==before&&!std::memcmp(&regs,&f.c,sizeof(regs)),"Only the exact native swallowed callback/stack/posture/player provenance admits4217C read-only; caller proof does not invent a parent precondition");if(fault==10)need(!q::pose_restore.apply(f.ram,q::selected_pose(),q::selected_inventory(),200,1)&&f.memory==before,"The original pose restore still refuses an attached player without any RAM mutation");
 }
#ifdef BOSS_CHECKPOINT_NATIVE42148
 for(unsigned pending_stage:{0u,1u,3u}){Fixture f(4,1,0);const auto payload=q::saved_inventory[0];f.fatal(0);byte(f.ram,Fixture::player+0x30,0xC);word(f.ram,Fixture::player+0x20,0x8003085C);word(f.ram,Fixture::data+0xD8,pending_stage);
  need(q::notice_death(f.ram,200)&&q::waiting_for_retry()&&!r::failed,"Native swallowed fatality waits for its real final callback without opening the ended-run UI");
  f.c.r4=n::base(Fixture::player);f.c.r5=n::base(Fixture::data);f.c.r31=n::base(0x80030878);const auto sp=f.c.r29;const auto calls=swallowed_effects;native_swallow::func_80042148(f.ram,&f.c);
  need(swallowed_effects==calls+1&&f.c.r29==sp&&uint32_t(f.c.r31)==0x80030878&&f.c.r2==0,"Literal42148 returnsV0zero after ordinary checkpoint transport, so3085C skips its old-world physics branch");
  need(!r::failed&&r::session.snapshot().phase==b::Phase::RetryToFight&&q::transport_pending&&!q::death_pending&&!n::word(f.ram,Fixture::data+0xC0),"Exact swallowed final fence follows the same native transport once and then clears its callback naturally");
  f.arrive(162);need(!r::failed&&inv::matches(f.ram,payload)&&r::session.snapshot().player==Fixture::player&&r::session.snapshot().completed==0,"Swallowed checkpoint resumes the same Yoshi with the recorded HP1/emptyFIFO, without creating any boss victory");
 }
 need(swallowed_dispatches==3&&swallowed_effects==3,"Three literal dispatcher cases exercise real saved-stack/D8-independent native control flow");
#endif
 {Fixture f(0,5,0,false);f.fatal(0);need(!q::notice_death(f.ram,200)&&!q::intercept_death(f.ram,&f.c),"Automatic diagnostic remains fail-fast unless explicitly enabled");}
 {Fixture f(0,1);constexpr uint32_t body=0x80310000,shadow=0x80310400,resource=0x80350000,overlay=0x80360000;
  actor(f.ram,body,0x400B);actor(f.ram,shadow,0x4007);word(f.ram,body+0x180,Fixture::player);word(f.ram,shadow+0x180,Fixture::player);word(f.ram,Fixture::player+0x188,body);word(f.ram,Fixture::player+0x18C,shadow);half(f.ram,Fixture::player+0x184,6);
  half(f.ram,resource+0x3C,0x4015);word(f.ram,resource+0x40,overlay);word(f.ram,resource+0x44,overlay+0xB48);r::overlays[128]=overlay;
  for(unsigned i=0;i<4;++i){const auto a=0x80320000+i*0x400;actor(f.ram,a,0x4015);byte(f.ram,a+6,2);byte(f.ram,a+0x18,0);byte(f.ram,a+0x17D,0);half(f.ram,a+0x17E,i+2);word(f.ram,a+0x180,Fixture::player);word(f.ram,a+0x168,resource);word(f.ram,a+0x20,overlay+0xA80);word(f.ram,a+0x2C,overlay+0xAC0);word(f.ram,a+0x1E0,i);word(f.ram,Fixture::player+0x188+(i+2)*4,a);}
  std::array<uint8_t,0x1FC> old_body{},old_shadow{};std::memcpy(old_body.data(),f.ram+(body&0x7FFFFF),old_body.size());std::memcpy(old_shadow.data(),f.ram+(shadow&0x7FFFFF),old_shadow.size());
  f.fatal(0);const auto before=escort_retires;f.leave();need(escort_retires==before+4&&inv::half(f.ram,Fixture::player+0x184)==2,"Strict final death removes exactly four cinematic kidnappers through native retirement boundary");
  need(!std::memcmp(old_body.data(),f.ram+(body&0x7FFFFF),old_body.size())&&!std::memcmp(old_shadow.data(),f.ram+(shadow&0x7FFFFF),old_shadow.size()),"Body and shadow remain byte-exact across cinematic cleanup");
  f.arrive(162);need(!r::failed&&inv::half(f.ram,Fixture::player+0x184)==2&&inv::matches(f.ram,q::saved_inventory[0]),"Cinematic children do not travel into restored first arena");
 }
 {Fixture f;f.fatal(0);r::session.cancel("native_receipt_rejected");const auto before=f.memory;
  need(!q::notice_death(f.ram,200)&&!q::intercept_death(f.ram,&f.c)&&f.memory==before&&r::session.snapshot().failure=="native_receipt_rejected","Later death cannot reclassify an integrity failure and revive a rejected campaign");
 }
 {Fixture f(10,0,2);byte(f.ram,Fixture::player+0x30,0x11);inv::Payload living;
  need(inv::capture(f.ram,Fixture::player,101,living)&&living.hp==0&&living.count==2,"Living hungry primary1/sub11 may activate a checkpoint with HP0");
  q::baseline_ready=false;need(q::capture_baseline(f.ram,102)&&q::saved_inventory[0].activation_vi==102,"Initial same-Yoshi baseline also admits native living1/11");
  byte(f.ram,Fixture::player+0x18,0xA);need(!inv::capture(f.ram,Fixture::player,103,living),"Fatal primaryA/sub11 cannot impersonate a living checkpoint activation");
 }
 {Fixture f;q::enter_room(162);auto& objects=r::rooms::configured_objects[0];bool found=false;for(size_t p=0;p<objects.size();p+=12)found|=workshop::u32(objects,p)==0x45070000;
  if(!found){const auto p=objects.size();objects.resize(p+12);workshop::put32(objects,p,0x45070000);workshop::put32(objects,p+4,std::bit_cast<uint32_t>(136.f));workshop::put32(objects,p+8,std::bit_cast<uint32_t>(448.f));r::rooms::configured_arenas[0].objects=objects;}
  constexpr uint32_t table=0x80400000;unsigned index=0;for(size_t p=0;p<objects.size();p+=12){for(unsigned j=0;j<3;++j)word(f.ram,table+unsigned(p/12)*16+j*4,workshop::u32(objects,p+j*4));if(workshop::u32(objects,p)==0x45070000)index=unsigned(p/12);}
  r::overlays[138]=0x80600000;f.c.r4=0x45070000;f.c.r16=n::base(table+index*16);f.c.r31=n::base(0x8005A010);f.c.r5=workshop::u32(objects,index*12+4);f.c.r6=workshop::u32(objects,index*12+8);
  need(q::select_spawn(f.ram,&f.c).owned&&q::actor_frame(f.ram).record==table+index*16,"Unique ROM index plus complete native16-byte table proves source record");
  word(f.ram,table+4,n::word(f.ram,table+4)^1);need(!q::select_spawn(f.ram,&f.c).owned,"Unrelated native table-coordinate corruption invalidates placement proof");
  need(q::actor_frame(f.ram).record==table+index*16,"Construction provenance survives native wake moving the player spawn record");
  word(f.ram,table+4,workshop::u32(objects,4));for(size_t p=0;p<objects.size();p+=12)if(workshop::u16(objects,p)==0x4001){word(f.ram,table+unsigned(p/12)*16+4,std::bit_cast<uint32_t>(200.f));word(f.ram,table+unsigned(p/12)*16+8,std::bit_cast<uint32_t>(448.f));}
  need(q::select_spawn(f.ram,&f.c).owned,"Cached exact room/table/record proof permits only the player record XY changed by native wake");
  need(r::session.bind(0,0x803F0000)&&r::session.defeat({0,0,160,0x803F0000,true,true,true,true}),"Explicit model first victory for checkpoint recreation lifetime");
  need(q::actor_frame(f.ram).first_fight&&q::select_spawn(f.ram,&f.c).owned,"Opening checkpoint remains owned for recreation after completed1 without changing progress");
  constexpr uint32_t point=0x803C0000,resource=0x803C1000,metadata=0x803C2000,definition=0x803C2100,priv=0x803C3000,overlay=0x80600000;
  const auto record=table+index*16;actor(f.ram,point,0x4507);word(f.ram,point+0x168,resource);word(f.ram,point+0x174,metadata);word(f.ram,metadata,definition);word(f.ram,resource+0x40,overlay);word(f.ram,resource+0x44,overlay+q::actor::initializer_offset);
  word(f.ram,point+0x1D8,record);word(f.ram,point+0x1DC,0x45070000);word(f.ram,record,0xFFFFFFFF);word(f.ram,point+0x20,overlay+q::actor::initializer_offset);word(f.ram,point+0x3C,n::word(f.ram,record+4));word(f.ram,point+0x40,n::word(f.ram,record+8));f.c.r4=n::base(point);
  const auto init=q::init_enter(f.ram,&f.c);need(init!=0,"Exact owned No1 constructor is admitted during first victory");
  word(f.ram,point+0x1F4,priv);word(f.ram,priv+0x27C,0);word(f.ram,point+0x1F8,overlay+0x11BC);byte(f.ram,point+0x30,0);word(f.ram,point+0x20,overlay+0xBA8);need(q::init_leave(f.ram,&f.c,init),"Explicit synthetic initializer return binds the same sleeping No1 after first victory");
  f.health(2);f.set_eggs(2);r::current_vi=175;f.c.r31=n::base(overlay+0x6D0);const auto wake=q::wake_enter(f.ram,&f.c);need(wake!=0,"First victory retains the genuine No1 wake entry");
  byte(f.ram,point+0x30,1);word(f.ram,point+0x20,overlay+0xC1C);word(f.ram,q::actor::flags_address,1);word(f.ram,q::actor::count_address,1);f.attach_checkpoint();
  need(q::wake_leave(f.ram,&f.c,wake)&&!r::failed&&r::session.snapshot().phase==b::Phase::Victory&&r::session.snapshot().completed==1&&r::session.snapshot().checkpoint_awake_mask==1,"Host wake return activates only No1 without advancing or leaving first victory");
  need(q::saved_inventory[0].activation_vi==175&&q::saved_inventory[0].hp==2&&q::saved_inventory[0].count==2&&q::saved_pose[0].activation_vi==175,"Post-victory No1 captures HP and eggs at the actual wake instead of the initial fallback");
  word(f.ram,Fixture::player+0x180,0);f.fatal(0);f.leave();need(r::session.snapshot().completed==0&&r::session.snapshot().max_completed==1,"No1 retry restarts fight1 while preserving actual maximum progress");f.arrive(162);
  need(!r::failed&&inv::matches(f.ram,q::saved_inventory[0])&&n::word(f.ram,0x800F8DB4)==2&&n::byte(f.ram,Fixture::data+0x12C8)==2,"Retry0 restores the post-victory wake HP and eggs on the same Yoshi");
 }
 {Fixture f;const auto& a=r::rooms::arena(162);constexpr uint32_t table=0x80400000,floor=0x80410000,res=0x80411000,priv=0x80412000,overlay=0x80600000;uint32_t record=0,packed=0;
  for(size_t p=0;p<a.objects.size();p+=12){for(unsigned j=0;j<3;++j)word(f.ram,table+unsigned(p/12)*16+j*4,workshop::u32(a.objects,p+j*4));if(workshop::u32(a.objects,p)==0x45920000){record=table+unsigned(p/12)*16;packed=workshop::u32(a.objects,p);}}
  need(record!=0,"Candy owns the real first4592 floor source");actor(f.ram,floor,0x4592);word(f.ram,floor+0x168,res);word(f.ram,res+0x40,overlay);word(f.ram,res+0x44,overlay+0x5A4);r::floor_guard.reset(162,0);r::overlays[189]=overlay;
  f.c.r4=n::base(floor);f.c.r5=n::base(record);f.c.r31=n::base(0x8005A050);r::trace(f.ram,&f.c,"func_80048E9C",150,r::overlays.data());need(!r::failed&&r::floor_guard.owned(floor),"Runtime captures floor only through the exact native48E9C source boundary");
  word(f.ram,floor+0x1D8,record);word(f.ram,floor+0x1DC,packed);word(f.ram,record,0xFFFFFFFF);word(f.ram,floor+0x1F4,priv);word(f.ram,priv+8,overlay+0x714);word(f.ram,floor+0x20,overlay+0x2C0);word(f.ram,floor+0x2C,overlay+0x43C);word(f.ram,floor+0x3C,std::bit_cast<uint32_t>(40.f));word(f.ram,floor+0x40,std::bit_cast<uint32_t>(472.f));half(f.ram,floor+0xC0,96);half(f.ram,floor+0xC2,96);
  constexpr unsigned head[]{5,2,32,0,20};for(unsigned i=0;i<5;++i)half(f.ram,overlay+0x714+i*2,head[i]);for(unsigned i=0;i<10;++i){half(f.ram,overlay+0x714+10+i*4,((i%5)<<12)|((i/5)<<8)|i);half(f.ram,overlay+0x714+12+i*4,i<5?0x0601:0x0401);}
  const auto before=f.memory;r::hook_leave(f.ram,&f.c,"func_8069D05C");need(!r::failed&&r::floor_guard.snapshot()[0].pinned&&inv::half(f.ram,floor+0xC0)==32767&&inv::half(f.ram,floor+0xC2)==32767,"Runtime pins native stream margins only after the exact floor stamp returns");
  bool isolated=true;for(unsigned i=0;i<f.memory.size();++i)if(f.memory[i]!=before[i]&&(i<(floor&0x7FFFFF)+0xC0||i>=(floor&0x7FFFFF)+0xC4))isolated=false;need(isolated,"Floor persistence writes only the two culling margins, never floor tiles, physics, actor pose or player state");
 }
 // Composed host routing: Rest83 must reach capture/post-stamp before its
 // normal early return. Native floor geometry is separately executed by the
 // RestFloor oracle; this synthetic return checks only Runtime integration.
 {Fixture f;f.reach(3);r::physical_room=r::logical_room=83;r::rest_floor_encounter=2;r::rest_floor_guard.reset(83,2);
  const auto& objects=workshop::boss_rest_data0240::rest_objects;constexpr uint32_t table=0x80400000,floor=0x80410000,res=0x80411000,priv=0x80412000,overlay=0x80600000;uint32_t record=0,packed=0;
  for(size_t p=0;p<objects.size();p+=12){for(unsigned j=0;j<3;++j)word(f.ram,table+unsigned(p/12)*16+j*4,workshop::u32(objects,p+j*4));if(workshop::u16(objects,p)==0x4592&&workshop::u32(objects,p+4)==std::bit_cast<uint32_t>(168.f)&&workshop::u32(objects,p+8)==std::bit_cast<uint32_t>(472.f)){record=table+unsigned(p/12)*16;packed=workshop::u32(objects,p);}}
  need(record!=0,"Rest source at168/472 supports checkpoint2 at160/448");actor(f.ram,floor,0x4592);word(f.ram,floor+0x168,res);word(f.ram,res+0x40,overlay);word(f.ram,res+0x44,overlay+0x5A4);r::overlays[189]=overlay;
  f.c.r4=n::base(floor);f.c.r5=n::base(record);f.c.r31=n::base(0x8005A050);const auto before_capture=f.memory;r::trace(f.ram,&f.c,"func_80048E9C",150,r::overlays.data());
  need(!r::failed&&r::rest_floor_guard.owned(floor)&&f.memory==before_capture,"Runtime captures exact Rest floor source before Rest early return without RAM writes");
  word(f.ram,floor+0x1D8,record);word(f.ram,floor+0x1DC,packed);word(f.ram,record,0xFFFFFFFF);word(f.ram,floor+0x1F4,priv);word(f.ram,priv+8,overlay+0x714);word(f.ram,floor+0x20,overlay+0x2C0);word(f.ram,floor+0x2C,overlay+0x43C);word(f.ram,floor+0x3C,std::bit_cast<uint32_t>(168.f));word(f.ram,floor+0x40,std::bit_cast<uint32_t>(472.f));half(f.ram,floor+0xC0,96);half(f.ram,floor+0xC2,96);
  constexpr unsigned head[]{5,2,32,0,20};for(unsigned i=0;i<5;++i)half(f.ram,overlay+0x714+i*2,head[i]);for(unsigned i=0;i<10;++i){half(f.ram,overlay+0x714+10+i*4,((i%5)<<12)|((i/5)<<8)|i);half(f.ram,overlay+0x714+12+i*4,i<5?0x0601:0x0401);}
  auto expected=f.memory;auto* rdram=expected.data();MEM_H(0xC0,n::base(floor))=32767;MEM_H(0xC2,n::base(floor))=32767;const auto registers=f.c;r::hook_leave(f.ram,&f.c,"func_8069D05C");
  need(!r::failed&&inv::half(f.ram,floor+0xC0)==32767&&inv::half(f.ram,floor+0xC2)==32767,"Rest post-stamp hook pins native floor before its Rest early return");
  need(f.memory==expected&&!std::memcmp(&registers,&f.c,sizeof(f.c)),"Composed Rest hook changes only four owned margins and preserves caller registers");
  need(r::session.snapshot().phase==b::Phase::Rest&&r::session.snapshot().completed==3,"Floor protection cannot advance checkpoint/session progress");
  need(r::session.use_rest_pot()&&r::session.snapshot().index==3&&r::physical_room==83,"Ordinary onward pot advances encounter before leaving the physical Rest room");
  const auto departure=f.memory;r::hook_leave(f.ram,&f.c,"func_8069D05C");need(!r::failed&&f.memory==departure&&r::rest_floor_guard.owned(floor),"Rest floor ownership remains bound to its visit while the next fight index is already selected");
  r::forget(floor);need(!r::rest_floor_guard.owned(floor),"Rest native teardown forgets floor identity even on the Rest branch");
 }
 // Composed deferred initialization for all three later checkpoints. This
 // Runtime suite builds explicit synthetic native postconditions; the separate
 // Actor/RestFloor oracles execute the actual initializer/stamping bodies.
 for(unsigned completed:{3u,6u,9u}){Fixture f;f.reach(completed);r::physical_room=r::logical_room=83;r::rest_floor_encounter=completed-1;r::rest_floor_guard.reset(83,completed-1);q::enter_room(83);r::current_vi=150;
  const auto& objects=workshop::boss_rest_data0240::rest_objects;
  constexpr uint32_t table=0x80400000,floor=0x80410000,floor_resource=0x80411000,floor_private=0x80412000,floor_overlay=0x80610000;
  constexpr uint32_t point=0x803C0000,resource=0x803C1000,metadata=0x803C2000,definition=0x803C2100,priv=0x803C3000,overlay=0x80600000;
  uint32_t record=0,support_record=0,packed=0;
  for(size_t p=0;p<objects.size();p+=12){const auto expanded=table+unsigned(p/12)*16;for(unsigned j=0;j<3;++j)word(f.ram,expanded+j*4,workshop::u32(objects,p+j*4));
   if(workshop::u32(objects,p)==q::actor::source_packed)record=expanded;
   if(workshop::u16(objects,p)==0x4592&&workshop::u32(objects,p+4)==std::bit_cast<uint32_t>(168.f)&&workshop::u32(objects,p+8)==std::bit_cast<uint32_t>(472.f)){support_record=expanded;packed=workshop::u32(objects,p);}
  }
  need(record&&support_record,"Rest supplies unique immutable checkpoint and supporting4592 records");r::overlays[138]=overlay;r::overlays[189]=floor_overlay;
  f.c.r4=q::actor::source_packed;f.c.r16=n::base(record);f.c.r31=n::base(0x8005A010);f.c.r5=n::word(f.ram,record+4);f.c.r6=n::word(f.ram,record+8);
  const auto choice=q::select_spawn(f.ram,&f.c);const unsigned ordinal=completed/3;
  need(choice.owned&&!choice.suppress&&choice.packed==(uint32_t(0x4507+ordinal)<<16),"Actual Runtime source proof selects No2/No3/No4 only at its completed prefix");
  actor(f.ram,point,0x4507+ordinal);byte(f.ram,point+0x18,0);byte(f.ram,point+0x165,1);word(f.ram,point+0x168,resource);word(f.ram,point+0x174,metadata);word(f.ram,metadata,definition);word(f.ram,resource+0x40,overlay);word(f.ram,resource+0x44,overlay+q::actor::initializer_offset);
  word(f.ram,point+0x1D8,record);word(f.ram,point+0x1DC,q::actor::source_packed);word(f.ram,record,0xFFFFFFFF);word(f.ram,point+0x20,overlay+q::actor::initializer_offset);word(f.ram,point+0x3C,n::word(f.ram,record+4));word(f.ram,point+0x40,n::word(f.ram,record+8));f.c.r4=n::base(point);const auto point_context=f.c;
  const auto empty=f.memory;need(!q::actor_frame(f.ram).support_ready,"No bitmap or source-stamp witness means later checkpoint support is not ready");
  need(q::init_enter(f.ram,&f.c)==q::actor::deferred_initialization&&!q::characters.snapshot().invalidated&&!r::failed&&f.memory==empty&&!std::memcmp(&point_context,&f.c,sizeof(f.c))&&!q::characters.snapshot().next_birth,"Initial later-checkpoint attempt returns MAX without native flags, physics, registers or a consumed birth");
  need(q::init_enter(f.ram,&f.c)==q::actor::deferred_initialization&&f.memory==empty,"A repeated early update stays deferred without a native initializer side effect");
  actor(f.ram,floor,0x4592);word(f.ram,floor+0x168,floor_resource);word(f.ram,floor_resource+0x40,floor_overlay);word(f.ram,floor_resource+0x44,floor_overlay+0x5A4);f.c.r4=n::base(floor);f.c.r5=n::base(support_record);f.c.r31=n::base(0x8005A050);
  const auto capture_memory=f.memory;r::trace(f.ram,&f.c,"func_80048E9C",151,r::overlays.data());need(!r::failed&&r::rest_floor_guard.owned(floor)&&f.memory==capture_memory,"Later checkpoint support is owned through real Guard capture and the full source table");
  word(f.ram,floor+0x1D8,support_record);word(f.ram,floor+0x1DC,packed);word(f.ram,support_record,0xFFFFFFFF);word(f.ram,floor+0x1F4,floor_private);word(f.ram,floor_private+8,floor_overlay+0x714);word(f.ram,floor+0x20,floor_overlay+0x2C0);word(f.ram,floor+0x2C,floor_overlay+0x43C);word(f.ram,floor+0x3C,std::bit_cast<uint32_t>(168.f));word(f.ram,floor+0x40,std::bit_cast<uint32_t>(472.f));half(f.ram,floor+0xC0,96);half(f.ram,floor+0xC2,96);
  constexpr unsigned head[]{5,2,32,0,20};for(unsigned i=0;i<5;++i)half(f.ram,floor_overlay+0x714+i*2,head[i]);for(unsigned i=0;i<10;++i){half(f.ram,floor_overlay+0x714+10+i*4,((i%5)<<12)|((i/5)<<8)|i);half(f.ram,floor_overlay+0x714+12+i*4,i<5?0x0601:0x0401);}
  r::hook_leave(f.ram,&f.c,"func_8069D05C");need(!r::failed&&inv::half(f.ram,floor+0xC0)==32767&&!q::actor_frame(f.ram).support_ready,"Exact source-pinned native stamp alone cannot substitute for missing support pixels");f.c=point_context;const auto pinned_no_map=f.memory;
  need(q::init_enter(f.ram,&f.c)==q::actor::deferred_initialization&&f.memory==pinned_no_map,"Pinned support without the correct native map still defers the initializer without writes");
  // Explicit synthetic native 3x2 paged map, matching the real601/401 stamp.
  constexpr uint32_t scene=0x80500000,link1=0x80500100,link2=0x80500200,link3=0x80500300,link4=0x80500400,map=0x80500500,pages=0x80501000,tiles=0x80502000;
  word(f.ram,0x800F8810,scene);word(f.ram,scene+0x10,link1);word(f.ram,link1+4,link2);word(f.ram,link2+0x16C,link3);word(f.ram,link3+0x1B0,link4);word(f.ram,link4+0x20,map);half(f.ram,map+0x12,3);half(f.ram,map+0x14,2);half(f.ram,map+0x16,96);word(f.ram,map+0x18,pages);word(f.ram,map+0x24,tiles);for(unsigned i=0;i<6;++i)byte(f.ram,pages+i,i);
  constexpr uint32_t top=tiles+2*(13*96+3*16+10),under=tiles+2*(14*96+3*16+10);half(f.ram,top,0x0601);half(f.ram,under,0x0401);
  const auto supported=f.memory;need(q::actor_frame(f.ram).support_ready&&f.memory==supported,"Runtime derives read-only support readiness from its actual source/pin witness and native paged top/bottom pixels");
  half(f.ram,under,0);const auto missing_under=f.memory;need(!q::actor_frame(f.ram).support_ready&&q::init_enter(f.ram,&f.c)==q::actor::deferred_initialization&&f.memory==missing_under,"A missing bottom support row cannot admit the deferred later checkpoint");half(f.ram,under,0x0401);
  const auto registers=f.c;const auto ticket=q::init_enter(f.ram,&f.c);need(ticket&&ticket!=q::actor::deferred_initialization&&!r::failed&&!q::characters.snapshot().invalidated&&q::characters.snapshot().next_birth==1&&!std::memcmp(&registers,&f.c,sizeof(f.c)),"Source-proven ready support admits exactly one real initializer ticket without a fabricated ready flag");
  word(f.ram,point+0x1F4,priv);word(f.ram,priv+0x27C,ordinal);word(f.ram,point+0x1F8,overlay+0x11BC);byte(f.ram,point+0x18,1);byte(f.ram,point+0x165,0);byte(f.ram,point+0x30,0);word(f.ram,point+0x20,overlay+0xBA8);
  need(q::init_leave(f.ram,&f.c,ticket)&&!r::failed&&q::characters.snapshot().binding.actor==point&&q::characters.snapshot().binding.index==ordinal,"Explicit synthetic native initializer postconditions bind the selected later checkpoint after the witnessed stamp");
  need(r::session.snapshot().completed==completed&&!r::session.snapshot().checkpoint_awake_mask&&r::session.snapshot().phase==b::Phase::Rest,"Deferred construction and support readiness create no checkpoint wake, completion, healing or transport");
 }
 // Explicit green/purple preview remains an isolated diagnostic plan, with the
 // real terrain metadata driving all six feeder choices.
 {Fixture f;need(r::configure("special-fish-synthetic",1,true,false,4,true),"Explicit isolated special Fish plan accepted");
  const auto& e=r::session.current();need(r::session.snapshot().index==4&&e.family==b::Family::Fish&&e.count==2&&e.variant==1,"Special preview retains actual Fish source/family/member kinds");
  need(e.members[0].kind==0x4511&&e.members[0].green&&!e.members[0].purple&&e.members[1].kind==0x4511&&!e.members[1].green&&e.members[1].purple&&e.purple_mask==2,"Green vertical and purple alternating-jump identities use the actual4511 source without water");
  need(e.members[0].hits==2&&e.members[1].hits==2&&e.birds==3&&e.bird_respawn_ms==3000&&b::valid(r::session.plan()),"Special test keeps two HP, three feeder birds and three-second respawn in a valid complete plan");
  r::rooms::configure(r::session.plan());r::enter_room(52);need(!r::failed&&r::physical_room==171&&r::logical_room==97,"Diagnostic entry reaches the actual configured Fish arena");
  need(r::session.enter_fight(97,Fixture::player),"Synthetic native release enters Fish combat");r::current_vi=101;r::variants::frame(f.ram,r::variant_frame(f.ram));const auto requests=r::variants::take_actions(f.ram);need(requests.size()==3,"Runtime native feeder path requests exactly three birds");
  const auto& arena=r::rooms::arena(171);std::array<std::array<uint32_t,3>,3> used{};unsigned slot=0;
  for(const auto& request:requests){need(request.operation==r::variants::Operation::BirdSpawn&&request.encounter==4,"Feeder requests retain actual encounter provenance");bool supported=false;
   for(unsigned i=0;i<3;++i)for(unsigned side=0;side<2;++side){const float x=arena.safe_pot_xs[i]+(side?24.f:-24.f),y=arena.entry_y-(side?56.f:72.f);supported|=request.arguments[1]==std::bit_cast<uint32_t>(x)&&request.arguments[2]==std::bit_cast<uint32_t>(y)&&request.arguments[3]==std::bit_cast<uint32_t>(500.34f);}
   need(supported,"Actual feeder action selects one of six reachable points above native islands on the Yoshi plane");const std::array<uint32_t,3> point{request.arguments[1],request.arguments[2],request.arguments[3]};for(unsigned prior=0;prior<slot;++prior)need(used[prior]!=point,"Pending feeder requests do not share a spawn point");used[slot++]=point;
  }
  need(!r::configure("not-diagnostic",1,false,false,4,true)&&!r::enabled,"Special colours cannot be inserted into an ordinary campaign");
  need(!r::configure("wrong-case",1,true,false,0,true)&&!r::enabled,"Special preview refuses another entry index");
  need(!r::configure("driver",1,true,true,4,true)&&!r::enabled,"Special preview refuses automatic driving");
  need(r::configure("automatic-special-fish",1,true,true,4,true,true)&&r::driver,"Explicit automatic full-loadout diagnostic plan accepts special colours");
  need(r::session.current().members[0].green&&r::session.current().members[1].purple,"Automatic preview preserves the same native special pair");
  need(!r::configure("auto-production",1,false,true,4,true,true)&&!r::enabled,"Automatic test flag cannot admit an ordinary campaign");
  need(!r::configure("auto-no-driver",1,true,false,4,true,true)&&!r::enabled,"Automatic test requires its controller driver");
  need(!r::configure("auto-wrong-case",1,true,true,0,true,true)&&!r::enabled,"Automatic test refuses another fight index");
  need(!r::configure("auto-wrong-seed",2,true,true,4,true,true)&&!r::enabled,"Automatic test refuses another plan seed");
  need(r::configure("normal-fish",1,true,false,4),"Ordinary diagnostic API remains available without special flag");
  const auto& ordinary=r::session.current();need(ordinary.count==3&&ordinary.variant==0&&ordinary.members[0].kind==0x4511&&!ordinary.members[0].green&&!ordinary.members[0].purple&&ordinary.members[1].kind==0x4511&&ordinary.members[1].green&&!ordinary.members[1].purple&&ordinary.members[2].kind==0x4511&&!ordinary.members[2].green&&ordinary.members[2].purple&&ordinary.members[0].hits==1&&ordinary.members[1].hits==1&&ordinary.members[2].hits==1,"Ordinary seed1 keeps its descriptor-derived three4511 one-HP red/green/purple plan");
 }
 {Fixture f;q::enter_room(162);r::overlays[138]=0x80600000;constexpr uint32_t miss=0x80300000;actor(f.ram,miss,0x4507);word(f.ram,miss+0x1DC,0x45070000);f.c.r4=n::base(miss);
  need(!q::init_enter(f.ram,&f.c)&&q::characters.snapshot().invalidated&&r::failed&&r::session.snapshot().phase==b::Phase::Failed,"Owned malformed constructor propagates helper invalidation even when ticket is zero");
  need(!q::wake_enter(f.ram,&f.c)&&r::session.snapshot().failure=="checkpoint_character_wakeup_scope_rejected","Zero-ticket wake also preserves fail-closed helper state");
 }
 // Host-only adapter composition. These values do not claim a native route.
 {Fixture f(0,5,3);namespace route=workshop::boss_route_checkpoint_bridge0260;
  const auto before=f.memory;uint16_t buttons=0x1234;float x=.2f,y=-.3f;
  route::configure(false);need(!route::drive(f.ram,150,&buttons,&x,&y)&&buttons==0x1234&&x==.2f&&y==-.3f,"Disabled route QA cannot override ordinary controls");
  route::observe(f.ram,150);need(f.memory==before&&!route::complete(),"Disabled adapter never writes guest memory or fabricates completion");
  r::driver=true;route::configure(true);std::array<workshop::boss_route_checkpoint_test0260::Target,4> targets{};const auto observed=route::make_frame(f.ram,150,targets);
  need(observed.session==&r::session.snapshot()&&observed.characters==&q::characters.snapshot()&&observed.saved==&q::selected_inventory(),"Route adapter observes actual Runtime objects rather than independent copied progress");
  need(observed.seed==1&&observed.diagnostic&&observed.driver&&observed.room==162&&observed.logical_room==162&&observed.vi==150&&observed.controls,"Adapter supplies exact isolated campaign and native control context");
  need(observed.ledger.valid&&observed.ledger.treeConsumed==0&&observed.ledger.hybridConsumed==0&&!observed.ledger.heartConsumed,"Ledger adapter reads the live normal/hybrid tree and native giant-heart state");
  need(observed.saved->hp==5&&observed.saved->count==3&&observed.saved->eggs[1].palette==1,"Saved FIFO and palette remain the real value-only checkpoint payload");
  need(!route::drive(f.ram,150,&buttons,&x,&y)&&!route::controller.failed()&&route::controller.snapshot().player==Fixture::player,"Full route helper starts on the true first-fight player and leaves ordinary combat to the normal driver");
  route::observe(f.ram,150);need(!route::controller.failed()&&f.memory==before&&r::session.snapshot().completed==0,"Duplicate same-VI observation is read-only and cannot synthesize prefix victories");
  r::enabled=false;need(!route::drive(f.ram,151,&buttons,&x,&y),"Inactive campaign never enters the optional checkpoint pilot");r::enabled=true;route::configure(false);
 }
 std::cout<<"BossCheckpointRuntime0260Checks: "<<checks<<" PASS; synthetic routing only; transportStubs="<<transports<<" discardStubs="<<discards<<" healStubs="<<heals<<" eggStubs="<<egg_births<<" escortRetirementStubs="<<escort_retires<<"; no native gameplay/GPU/profile/ledger.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
