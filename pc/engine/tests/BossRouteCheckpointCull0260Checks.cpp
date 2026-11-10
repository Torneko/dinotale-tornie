#include "../boss_route_checkpoint_test0260.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace q=workshop::boss_route_checkpoint_test0260;
namespace a=workshop::boss_checkpoint_actor0260;
namespace b=workshop::boss_battle0250;
namespace inv=workshop::boss_checkpoint_inventory0260;
namespace n=workshop::boss_native;
static unsigned checks=0;
static void need(bool yes,const char* why){++checks;if(!yes)throw std::runtime_error(why);}

// This oracle composes the actual Actor and QA controllers. Stores between
// native entry/leave boundaries are explicit host fixtures, not native6600,
// 5628, camera scheduling, gameplay, or a GPU checkpoint-success claim.
struct Fixture {
 static constexpr uint32_t player=0x80050000,pd=0x80051000,point=0x80060000,
  pp=0x80061000,res=0x80062000,record=0x80063000,meta=0x80064000,
  def=0x80065000,overlay=0x80200000;
 std::vector<uint8_t> mem=std::vector<uint8_t>(0x800000);
 uint8_t* rdram=mem.data();a::Controller actors;q::Controller qa;
 a::Frame af;b::Snapshot session;inv::Payload saved;q::Frame f;
 recomp_context context{};uint64_t first_birth=0;
 Fixture(){
  live(player,0x4001);w(player+0x1F4,pd);w(0x800F8828,player);
  w(0x807F0008,0);w(0x800F8DB0,0);w(0x800F8DB4,8);stock(2);
  fl(player+0x84,120);fl(player+0x88,449);fl(player+0x44,500.34f);fl(player+0x8C,500.34f);
  session.diagnostic=true;session.token="cull-composed-host-fixture";
  session.phase=b::Phase::Fighting;session.player=player;
  actors.configure(true);actors.enter_room(83);qa.configure(true);
  f.diagnostic=f.driver=f.controls=true;f.room=f.logical_room=162;f.vi=100;f.seed=1;
  f.session=&session;f.characters=&actors.snapshot();f.saved=&saved;f.ledger.valid=true;
  for(unsigned i=0;i<6;++i)f.ledger.hybridOrder[i]=uint16_t(0x4F00+i);
  const auto before=mem;qa.frame(rdram,f);need(mem==before,"First-fight QA admission is read-only");
  af.active=af.rest_ready=af.support_ready=true;af.room=83;af.completed=3;
  af.vi=200;af.overlay=overlay;af.player=player;af.record=record;
  context.r4=n::base(point);context.r29=n::base(0x807B0000);context.r31=n::base(0x8005A058);
 }
 void w(uint32_t p,uint32_t v){MEM_W(0,n::base(p))=int32_t(v);}
 void by(uint32_t p,unsigned v){MEM_B(0,n::base(p))=uint8_t(v);}
 void half(uint32_t p,unsigned v){MEM_H(0,n::base(p))=uint16_t(v);}
 void fl(uint32_t p,float v){w(p,std::bit_cast<uint32_t>(v));}
 void live(uint32_t p,unsigned kind){w(p,0x19971110);half(p+12,kind);by(p+6,0);by(p+0x17D,2);by(p+0x18,1);}
 void stock(unsigned count){
  for(unsigned i=0;i<20;++i)w(player+0x188+i*4,0);
  for(unsigned i=0;i<8;++i)w(pd+0x11FC+i*24,0);
  by(pd+0x12C8,count);half(pd+0x12BE,0);half(player+0x184,count);
  for(unsigned i=0;i<count;++i){const uint32_t egg=0x80080000+i*0x400;
   live(egg,0x4004);w(egg+0x180,player);w(egg+0x1EC,i);for(unsigned axis=0;axis<3;++axis)fl(egg+0x78+4*axis,.56f);
   w(player+0x188+i*4,egg);w(pd+0x11FC+i*24,egg);}
 }
 uint64_t initialize(){
  live(point,0x4508);by(point+0x18,0);by(point+0x30,0);by(point+0x165,1);
  w(point+0x130,0);w(point+0x1F4,0);w(point+0x1D8,record);w(point+0x1DC,a::source_packed);
  w(record,0xFFFFFFFF);fl(record+4,160);fl(record+8,448);
  fl(point+0x3C,160);fl(point+0x40,448);fl(point+0x84,160);fl(point+0x88,448);
  w(point+0x168,res);w(point+0x174,meta);w(meta,def);
  w(res+0x40,overlay);w(res+0x44,overlay+a::initializer_offset);
  w(point+0x20,overlay+a::initializer_offset);
  context.r4=n::base(point);const auto entry=context;
  const uint64_t ticket=actors.init_enter(rdram,&context,af);
  need(ticket&&ticket!=a::deferred_initialization,"Actual Actor entry attests the owned ready-floor constructor");
  // Explicit native initializer boundary fixture; no original function runs.
  w(point+0x1F4,pp);w(point+0x1F8,overlay+0x11BC);w(pp+0x27C,1);
  by(point+0x18,1);by(point+0x30,0);by(point+0x165,0);w(point+0x20,overlay+0xBA8);
  const auto native_result=mem;
  need(actors.init_leave(rdram,&entry,af,ticket),"Actual Actor return validates the fixture's complete native postcondition");
  need(mem==native_result,"Initializer observer return is read-only");
  return actors.snapshot().binding.birth;
 }
 q::Output step(){++f.vi;af.vi=f.vi;const auto before=mem;const auto out=qa.frame(rdram,f);
  need(mem==before,"QA navigation and validation leave all8MiB unchanged");return out;}
 void enter_rest(bool bind=true){
  session.phase=b::Phase::Rest;session.index=2;session.completed=session.max_completed=3;
  f.room=f.logical_room=83;f.vi=200;if(bind)first_birth=initialize();
  step();need(qa.snapshot().phase==q::Phase::PrepareRest,"Incomplete inventory enters PrepareRest");
 }
 void cull(){
  actors.forget(point);by(point+6,3);by(point+0x17D,0);by(point+0x18,0xFF);
  w(point+0x1F4,0);w(point+0x174,0);w(record,a::source_packed);fl(point+0x88,463);
  need(!actors.snapshot().binding.actor&&actors.snapshot().next_birth==first_birth,
   "Actual forget clears the instance and retains the monotone birth frontier");
 }
 void prepare_culled(){enter_rest();fl(player+0x84,459.044067f);cull();}
 void full(){stock(6);w(0x800F8DB4,8);}
 void synthetic_wake(){
  context.r4=n::base(point);context.r31=n::base(overlay+0x6D0);af.vi=++f.vi;
  const auto entry=context,before_context=context;const auto before=mem;
  const auto ticket=actors.wake_enter(rdram,&context,af);
  need(ticket&&mem==before&&!std::memcmp(&context,&before_context,sizeof(context)),
   "Only the actual new Actor identity and genuine native caller admit wake entry");
  // Explicit native5628 boundary fixture; strict observer and QA receipts
  // still require the exact postcondition, sparse flags, inventory, and VI.
  by(point+0x30,1);w(point+0x20,overlay+0xC1C);
  w(a::count_address,1);w(a::flags_address+4,1);
  const auto native_result=mem;
  need(actors.wake_leave(rdram,&entry,af,ticket)&&mem==native_result,
   "Actual Actor wake return attests only exact native-shaped postconditions");
  const auto receipt=actors.take_wakeup();
  need(receipt&&receipt->actor==point&&receipt->birth>first_birth&&receipt->completed==3,
   "Recreated wake receipt refers to the fresh birth and milestone3");
  need(inv::capture(rdram,player,f.vi,saved),"Activation inventory comes from the actual readable FIFO graph");
  session.checkpoint_completed=3;session.checkpoint_awake_mask=2;af.awake_mask=2;
  step();
 }
};

int main(){try{
 {Fixture x;x.prepare_culled();
  auto out=x.step();need(!out.handled&&!x.qa.failed(),"Culled No2 withR2 delegates to the real rest refill pilot");
  x.full();out=x.step();need(out.handled&&out.x<0&&out.buttons==0&&!x.qa.failed(),
   "R6 turns toward the previously attested source using ordinary left stick only");
  need(x.qa.snapshot().phase==q::Phase::Wake,"Full inventory enters Wake while the native instance is absent");
  x.fl(Fixture::player+0x84,300);out=x.step();
  need(out.handled&&out.x<0&&!x.qa.failed(),"Camera-return input continues until a real new binding is available");
  const auto second=x.initialize();
  need(second>x.first_birth,"Same-address native recreation receives a distinct monotone birth");
  x.fl(Fixture::player+0x84,220);out=x.step();
  need(out.handled&&out.x<0&&!x.qa.failed(),"The new strict binding supplies actual wake approach input");
  x.fl(Fixture::player+0x84,160);out=x.step();
  need(out.handled&&!(out.buttons&0x8000),"Arriving at the proven point first releases A for a fresh native edge");
  bool pressed=false;
  for(unsigned i=0;i<8&&!pressed;++i){out=x.step();pressed=(out.buttons&0x8000)!=0;}
  need(pressed&&!x.qa.failed(),"The full grounded player receives an ordinary A edge near the newly attested point");
  x.by(Fixture::pd+0xF4,1);
  for(unsigned age=1;age<=18;++age){out=x.step();
   need(out.handled&&bool(out.buttons&0x8000)==(age<18),
    "The licensed jump holds ordinary A for18VI including its edge, then releases while awaiting real rider contact");
  }
  x.by(Fixture::pd+0xF4,0);
  x.synthetic_wake();
  need(x.qa.snapshot().phase==q::Phase::AwaitFight&&!x.qa.failed(),
   "Only exact fresh-birth wake and native-shaped inventory receipt complete activation");
  need(x.qa.snapshot().point==Fixture::point&&x.qa.snapshot().point_birth==second,
   "QA activation binds the recreated instance rather than the retired actor generation");
 }
 {Fixture x;x.enter_rest(false);x.fl(Fixture::player+0x84,459);x.w(Fixture::record,a::source_packed);
  x.fl(Fixture::record+4,160);x.fl(Fixture::record+8,448);x.full();const auto out=x.step();
  need(out.handled&&out.x==0&&x.qa.snapshot().phase==q::Phase::PrepareRest,
   "Source coordinates alone cannot invent the missing initial owned proof");
 }
 for(unsigned fault=0;fault<5;++fault){Fixture x;x.prepare_culled();x.full();
  if(fault==0)x.w(Fixture::record,0x45080000);
  if(fault==1)x.fl(Fixture::record+4,161);
  if(fault==2)x.fl(Fixture::record+8,449);
  if(fault==3)++x.session.retry_epoch;
  if(fault==4)x.session.token="foreign-run";
  const auto out=x.step();
  need(out.handled&&out.x==0&&out.buttons==0,
   "A changed cached source or run/epoch never licenses ordinary return navigation");
 }
 {Fixture x;x.prepare_culled();x.full();x.step();
  x.actors.configure(true);x.actors.enter_room(83);const auto same=x.initialize();
  need(same==x.first_birth,"Negative recreation fixture deliberately reuses the old generation");
  const auto out=x.step();need(out.handled&&out.x==0&&out.buttons==0,
   "A reset birth frontier cannot masquerade as the retired instance's fresh successor");
 }
 {Fixture x;x.prepare_culled();x.full();x.step();x.initialize();
  x.w(Fixture::point+0x1D8,Fixture::record+16);const auto out=x.step();
  need(out.handled&&out.x==0&&out.buttons==0,
   "A foreign source record on the newly born actor cannot license wake movement");
 }
 for(unsigned fault=0;fault<6;++fault){Fixture x;x.prepare_culled();x.full();x.step();x.initialize();
  x.fl(Fixture::player+0x84,160);
  if(fault==0)x.fl(Fixture::player+0x84,220);
  if(fault==1)x.by(Fixture::pd+0xF4,1);
  if(fault==2)x.by(Fixture::player+0x18,9);
  if(fault==3)x.w(Fixture::point+0x1D8,Fixture::record+16);
  if(fault==4)x.stock(5);
  if(fault==5)x.w(0x800F8DB4,6);
  for(unsigned i=0;i<10;++i){const auto out=x.step();
   if(fault>=4)need(x.qa.snapshot().phase==q::Phase::PrepareRest&&(!(out.buttons&0x8000)||out.x>0),
    "Incomplete inventory or healing returns to refill; an existing outbound jump is not a wake request");
   else need(!(out.buttons&0x8000),"Far/airborne/foreign observations cannot start a new wake jump");
  }
 }
 std::cout<<"BossRouteCheckpointCull0260Checks: "<<checks<<" PASS; composed host Actor/QA boundaries, no native game/GPU/wakeup or retry-success claim.\n";
 return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
