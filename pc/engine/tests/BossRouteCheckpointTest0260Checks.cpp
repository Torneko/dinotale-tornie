#include "../boss_route_checkpoint_test0260.hpp"
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <vector>
namespace q=workshop::boss_route_checkpoint_test0260;namespace b=workshop::boss_battle0250;
namespace a=workshop::boss_checkpoint_actor0260;namespace inv=workshop::boss_checkpoint_inventory0260;namespace n=workshop::boss_native;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
// Read-only host observations exercise the QA state machine. These fixtures
// do not execute a native wake, victory, egg release, death or transport.
struct Fixture {
 static constexpr uint32_t player=0x80050000,pd=0x80051000,point=0x80060000,pp=0x80061000,res=0x80062000,record=0x80063000,overlay=0x80200000,boss=0x80070000,aim=0x80074000;
 std::vector<uint8_t> mem=std::vector<uint8_t>(0x800000);uint8_t* rdram=mem.data();q::Controller c;b::Snapshot s;a::Snapshot chars;inv::Payload saved;q::Ledger ledger;q::Frame f;std::array<q::Target,1> targets{};
 explicit Fixture(unsigned color=0,bool initialize=true){
  actor(player,0x4001);w(player+0x1F4,pd);w(0x800F8828,player);w(0x807F0008,color);w(0x800F8DB0,color<8?color:0);health(7);stock(6);
  fl(player+0x84,120);fl(player+0x88,449);fl(player+0x44,500.34f);fl(player+0x8C,500.34f);
  s.phase=b::Phase::Fighting;s.player=player;s.diagnostic=true;s.token="route-host-fixture";
  chars.enabled=true;chars.room=83;
  ledger.valid=true;ledger.treeConsumed=2;ledger.treeMask=3;ledger.hybridConsumed=1;ledger.hybridMask=1;
  for(unsigned i=0;i<6;++i)ledger.hybridOrder[i]=uint16_t(0x4F00+i);
  f.egg_admission_observed=f.egg_admitted=f.pad_observed=f.z_released=true;
  f.diagnostic=f.driver=f.controls=true;f.room=f.logical_room=162;f.vi=100;f.seed=1;f.session=&s;f.characters=&chars;f.saved=&saved;f.ledger=ledger;f.targets=targets;
  c.configure(true);if(initialize)step();
 }
 void w(uint32_t x,uint32_t v){MEM_W(0,n::base(x))=int32_t(v);}void by(uint32_t x,unsigned v){MEM_B(0,n::base(x))=uint8_t(v);}void h(uint32_t x,unsigned v){MEM_H(0,n::base(x))=uint16_t(v);}void fl(uint32_t x,float v){w(x,std::bit_cast<uint32_t>(v));}
 void actor(uint32_t x,unsigned kind){w(x,0x19971110);h(x+12,kind);by(x+6,0);by(x+0x17D,2);by(x+0x18,1);}
 void health(unsigned hp){w(0x800F8DB4+n::word(rdram,0x800F8DB0)*4,hp);}
 void stock(unsigned count,unsigned first=0){
  for(unsigned i=0;i<20;++i)w(player+0x188+i*4,0);
  for(unsigned i=0;i<8;++i)w(pd+0x11FC+i*24,0);
  by(pd+0x12C8,count);h(pd+0x12BE,0);h(player+0x184,count);
  constexpr std::array<unsigned,6> palettes{0,8,13,3,9,5};
  for(unsigned i=0;i<count;++i){const auto egg=0x80080000+i*0x400;actor(egg,0x4004);w(egg+0x180,player);w(egg+0x1EC,palettes[(i+first)%6]);for(unsigned axis=0;axis<3;++axis)fl(egg+0x78+4*axis,.56f);w(player+0x188+i*4,egg);w(pd+0x11FC+i*24,egg);}
 }
 q::Output step(){f.ledger=ledger;const auto out=c.frame(rdram,f);return out;}
 void phase(q::Phase wanted,const char* why){if(c.snapshot().phase!=wanted){std::cerr<<"phase="<<unsigned(c.snapshot().phase)<<" failure="<<c.snapshot().failure<<'\n';need(false,why);}need(true,why);}
 void rest(unsigned cp,bool full=true){
  f.vi+=100;s.phase=b::Phase::Rest;s.index=cp-1;s.completed=s.max_completed=cp;f.room=f.logical_room=83;
  actor(point,0x4507+cp/3);w(point+0x1F4,pp);w(point+0x168,res);w(point+0x1D8,record);w(point+0x1DC,a::source_packed);
  w(res+0x40,overlay);w(res+0x44,overlay+a::initializer_offset);w(record,0xFFFFFFFF);fl(record+4,160);fl(record+8,448);w(pp+0x27C,cp/3);fl(point+0x84,160);fl(point+0x88,448);
  auto& p=chars.binding;p={};p.actor=point;p.private_data=pp;p.record=record;p.packed=a::source_packed;p.resource=res;p.overlay=overlay;p.x=n::word(rdram,record+4);p.y=n::word(rdram,record+8);p.birth=100+cp;p.room=83;p.milestone=cp;p.index=cp/3;chars.next_birth=p.birth;
  health(full?7:3);stock(full?6:2);step();
 }
 a::Binding culled_point(){
  const auto old=chars.binding;chars.binding={};w(record,a::source_packed);w(point+0x1F4,0);by(point+6,3);by(point+0x17D,0);by(point+0x18,0xFF);return old;
 }
 void recreated_point(a::Binding old,bool newer=true){
  // Explicit host boundary of native teardown/constructor, not a native call.
  actor(point,0x4507+old.index);w(point+0x1F4,pp);w(record,0xFFFFFFFF);old.birth+=newer?1:0;chars.binding=old;chars.next_birth=old.birth;
 }
 void wake(){
  ++f.vi;need(inv::capture(rdram,player,f.vi,saved),"Host wake observation records exact native-shaped FIFO values");
  s.checkpoint_completed=s.completed;s.checkpoint_awake_mask|=workshop::boss_checkpoint0260::awake_bit(s.completed);
  w(a::count_address,unsigned(std::popcount(s.checkpoint_awake_mask)));for(unsigned i=0;i<4;++i)w(a::flags_address+i*4,(s.checkpoint_awake_mask>>i)&1);
  step();
 }
 void fight(){
  ++f.vi;s.phase=b::Phase::Fighting;s.index=s.completed;f.room=166+s.index;f.logical_room=s.completed==9?162:151;
  actor(boss,s.completed==9?0x455A:0x41C6);fl(boss+0x84,328);targets[0]={boss,20+s.completed};step();++f.vi;step();
 }
 void release(){
  stock(5,1);actor(aim,0x4005);w(aim+0x180,player);w(player+0x188+5*4,aim);h(player+0x184,6);
  f.vi+=30;step();phase(q::Phase::ShotRelease,"Observed native-shaped4005 then released ordinary Z");
  stock(5,1);f.vi+=2;step();phase(q::Phase::SeekDeath,"Reduced FIFO and absent4005 attest the completed ordinary release");need(c.snapshot().fifo_changed,"The planned loss actually changes the saved FIFO");
 }
 recomp_context fatal(unsigned sub=0xB,uint32_t ra=0x800301D4){
  ++f.vi;health(0);by(player+0x18,0xA);by(player+0x30,sub);w(pd+0xC0,0x8002F5F4);w(pd+0xD8,3);w(player+0x38,uint32_t(-41));
  s.phase=b::Phase::Failed;s.failure="yoshi_defeated";ledger.treeConsumed=3;ledger.treeMask=7;ledger.hybridConsumed=2;ledger.hybridMask=3;
  if(s.completed>=6){ledger.heartConsumed=true;ledger.heartCount=1;}f.ledger=ledger;
  recomp_context ctx{};ctx.r4=n::base(player);ctx.r31=n::base(ra);return ctx;
 }
 void die(){auto ctx=fatal();const auto before=mem;const auto regs=ctx;c.native_death(rdram,&ctx,f);phase(q::Phase::AwaitRetry,"Only the strict native-shaped F5F4 entry records the loss");need(mem==before&&!std::memcmp(&regs,&ctx,sizeof(ctx)),"Death witness changes neither RAM nor registers");}
 void arrived(){
  ++f.vi;++s.retry_epoch;s.phase=b::Phase::Rest;s.index=s.completed-1;s.failure.clear();f.retry_restored_epoch=s.retry_epoch;f.room=f.logical_room=83;
  by(player+0x18,1);by(player+0x30,1);w(pd+0xC0,0);w(player+0x180,0);health(saved.hp);stock(saved.count);
  fl(player+0x44,500.34f);fl(player+0x8C,500.34f);w(player+0xCC,0);w(player+0xD8,0);w(player+0x114,0);
 }
 void setup_loss(){rest(3);wake();phase(q::Phase::AwaitFight,"Real-shaped wake evidence precedes any forced loss inputs");fight();release();}
};
int main(){try{
 {q::Controller c;const q::Frame f{};need(!c.frame(nullptr,f).handled&&!c.complete()&&!c.failed(),"Default false is entirely inert");c.configure(false);need(!c.frame(nullptr,f).handled,"Explicit false leaves ordinary play unchanged");}
 for(unsigned color=0;color<14;++color){Fixture x(color);
  for(unsigned cp:{3u,6u,9u}){
   x.rest(cp);x.phase(q::Phase::Wake,"A restored ordinary rest stock enables contact with the numbered point");const auto before=x.mem;const auto o=x.step();need(o.handled&&o.x>0&&x.mem==before,"Wake movement uses only ordinary stick output and reads RAM");
   x.wake();x.phase(q::Phase::AwaitFight,"Actual-shaped mask/birth/inventory receipt activates the exact milestone");need(x.c.snapshot().activation_epoch==cp/3-1,"Each activation uses the previous retry epoch");
   x.fight();x.release();x.die();x.arrived();const auto restored=x.mem;need(x.step().handled&&x.mem==restored,"Restoration is certified without RAM mutation");
   need(x.c.snapshot().completed_tests==cp/3&&x.c.snapshot().restored_epoch==cp/3,"Every selected checkpoint requires a separate exact epoch");
   if(cp<9){x.phase(q::Phase::WaitPrefix,"After retry the ordinary driver earns the next genuine prefix");need(!x.step().handled,"Same-VI repeated observations cannot replay a successful checkpoint");}
  }
  need(x.c.complete()&&!x.c.failed()&&x.c.snapshot().completed_tests==3,"All fourteen Yoshi colours retain three separately proved retries");need(!x.step().handled,"Completed QA releases input back to the ordinary Full10 driver");
 }
 {Fixture x;x.rest(3,false);x.phase(q::Phase::PrepareRest,"Unfilled rest must precede wake");const auto before=x.mem;auto o=x.step();need(o.handled&&(o.buttons&0x8000)&&o.x>0&&x.mem==before,"A sleeping point is crossed by an ordinary jump before healing");x.fl(Fixture::player+0x84,240);++x.f.vi;o=x.step();need(!o.handled,"Away from the sleeping point the existing rest pilot heals/refills");x.health(7);x.stock(6);++x.f.vi;o=x.step();need(o.handled&&o.x<0,"After real refill the tester walks back to the point");}
 {
  Fixture x;x.rest(3);x.fl(Fixture::player+0x84,160);x.fl(Fixture::player+0xC8,4.2f);++x.f.vi;
  const auto before=x.mem;const auto released=x.step();need(released.handled&&!released.buttons,"A live centred full-stock point first receives an explicit neutral A-release VI");
  need(!x.step().buttons,"Same-VI post-fence/input observations cannot consume the release VI and invent a fresh edge");
  ++x.f.vi;const auto pressed=x.step();need(pressed.handled&&pressed.buttons==0x8000&&!pressed.x&&x.mem==before,"Grounded F4 permits one ordinary centred A edge despite normal residual C8=4.2");
  need(x.step().buttons==pressed.buttons,"Repeated input/observe calls in the same VI return the same bounded A output");
  x.by(Fixture::pd+0xF4,1);const auto airborne=x.mem;
  for(unsigned age=1;age<18;++age){++x.f.vi;need(x.step().buttons==0x8000&&x.mem==airborne,"A bounded native jump keeps A through exactly18VI under the same live source/posture even when airborne");}
  ++x.f.vi;need(!x.step().buttons,"A releases exactly at age18 rather than remaining held through the recovery interval");
  x.f.vi+=100;need(!x.step().buttons,"An airborne native F4 cannot start another wake attempt even after the cadence interval");
  x.by(Fixture::pd+0xF4,0);++x.f.vi;need(!x.step().buttons,"Observed landing requires another explicit neutral VI before retry");
  ++x.f.vi;need(x.step().buttons==0x8000,"A second observed-ground attempt creates a fresh bounded edge without wake or guest writes");
  need(x.c.snapshot().phase==q::Phase::Wake&&!x.c.snapshot().activation_vi,"Ordinary jump commands alone are never activation evidence");
 }
 for(unsigned fault=0;fault<12;++fault){
  Fixture x;x.rest(3);x.fl(Fixture::player+0x84,160);++x.f.vi;x.step();++x.f.vi;
  if(fault==0)x.fl(Fixture::player+0x84,169);if(fault==1)x.fl(Fixture::player+0x88,497);if(fault==2)x.by(Fixture::pd+0xF4,1);
  if(fault==3)x.by(Fixture::player+0x18,8);if(fault==4)x.by(Fixture::player+0x17D,0);if(fault==5)x.by(Fixture::player+0x165,1);
  if(fault==6)x.w(Fixture::player+0x180,Fixture::point);if(fault==7)x.w(Fixture::pd+0x12EC,1);if(fault==8)x.f.controls=false;
  if(fault==9)x.chars.binding.birth=0;if(fault==10)x.chars.invalidated=true;if(fault==11)x.w(Fixture::point+0x1F4,0);
  const auto before=x.mem;const auto out=x.step();need(!(out.buttons&0x8000)&&x.mem==before,"Unknown/airborne/attached/unreleased/remote posture or foreign point cannot produce a Wake A edge");
 }
 for(bool health:{false,true}){
  Fixture x;x.rest(3);x.fl(Fixture::player+0x84,160);++x.f.vi;x.step();if(health)x.health(6);else x.stock(5);++x.f.vi;
  const auto before=x.mem;x.step();need(x.c.snapshot().phase==q::Phase::PrepareRest&&x.mem==before,"HP/FIFO are revalidated before each Wake edge; partial stock returns to ordinary refill and preserves its separate outbound crossing behaviour");
 }
 for(unsigned fault=0;fault<5;++fault){
  Fixture x;x.rest(3);x.fl(Fixture::player+0x84,160);++x.f.vi;x.step();++x.f.vi;need(x.step().buttons==0x8000,"Hold-cancellation fixture starts from an admitted fresh edge");const auto valid=x.mem;
  if(fault==0)x.w(Fixture::player+0x180,Fixture::point);if(fault==1)x.by(Fixture::player+0x165,1);if(fault==2)x.by(Fixture::player+0x17D,0);if(fault==3)x.by(Fixture::player+0x18,0xA);if(fault==4)x.w(Fixture::pd+0x11FC,0);
  const auto invalid=x.mem;need(!x.step().buttons&&x.mem==invalid,"Same-VI new attachment/transport/unreleased posture or unreadable FIFO cancels a cached A without guest writes");
  x.mem=valid;x.rdram=x.mem.data();++x.f.vi;need(!x.step().buttons,"An interrupted hold never resumes inside the old window without a fresh released edge");
 }
 {
  Fixture x;x.rest(3);x.fl(Fixture::player+0x84,160);++x.f.vi;
  for(unsigned attempt=0;attempt<6;++attempt){need(!x.step().buttons,"Every bounded attempt begins with released A");++x.f.vi;need(x.step().buttons==0x8000,"Each attempt begins with one genuine ordinary input edge");x.f.vi+=18;need(!x.step().buttons,"Each bounded18VI jump releases rather than becoming permanent held input");x.f.vi+=90;}
  x.step();++x.f.vi;const auto before=x.mem;need(!x.step().buttons&&x.c.failed()&&x.mem==before,"Six unconfirmed rider attempts fail explicitly without manufacturing native wake or activation");
 }
 {
  Fixture x;
  for(unsigned cp:{3u,6u,9u}){
   x.rest(cp,false);const auto previous_activation=x.c.snapshot().activation_vi;x.fl(Fixture::player+0x84,459.044f);const auto old=x.culled_point();++x.f.vi;
   const auto retired=x.mem;auto out=x.step();need(!out.handled&&x.mem==retired,"After exact native source re-arm, off-screen No2/3/4 delegates ordinary R2 refill without reading its dead private data");x.phase(q::Phase::PrepareRest,"Culling is neither a wake nor a checkpoint receipt");
   x.health(7);x.stock(6);++x.f.vi;const auto full=x.mem;out=x.step();need(out.handled&&out.x<0&&!out.buttons&&x.mem==full,"HP7/R6 returns by ordinary stick to the attested native placement X while the character is absent");x.phase(q::Phase::Wake,"Full stock starts returning but cannot activate a missing character");
   x.fl(Fixture::player+0x84,160);++x.f.vi;const auto absent=x.mem;out=x.step();need(out.handled&&!out.x&&!out.buttons&&x.mem==absent&&x.c.snapshot().activation_vi==previous_activation,"Reaching source X cannot manufacture its missing instance or an activation");
   x.recreated_point(old);x.fl(Fixture::player+0x84,200);++x.f.vi;const auto rebound=x.mem;out=x.step();need(out.handled&&out.x<0&&x.mem==rebound&&x.chars.binding.birth>old.birth,"The genuine-shaped new generation is re-attested before contact movement resumes");
   x.wake();x.phase(q::Phase::AwaitFight,"Only a fresh live point plus its exact wake inventory/mask activates after culling");need(x.c.snapshot().point_birth==old.birth+1,"Activation reports the new point birth, never its retired predecessor");
   x.fight();x.release();x.die();x.arrived();x.step();
  }
  need(x.c.complete()&&x.c.snapshot().completed_tests==3,"Source-return lifecycle preserves all three separately validated retry scenarios");
 }
 for(unsigned fault=0;fault<16;++fault){
  Fixture x;x.rest(3,false);x.fl(Fixture::player+0x84,459.044f);const auto old=x.culled_point();++x.f.vi;
  if(fault==0)x.w(Fixture::record,0xFFFFFFFF);if(fault==1)x.w(Fixture::record,0x45080000);if(fault==2)x.fl(Fixture::record+4,161);if(fault==3)x.fl(Fixture::record+8,449);
  if(fault==4)x.chars.enabled=false;if(fault==5)x.chars.invalidated=true;if(fault==6)x.chars.room=162;if(fault==7)x.chars.next_birth=old.birth-1;
  if(fault==8)x.chars.binding.actor=Fixture::point;if(fault==9)x.chars.binding.birth=old.birth;if(fault==10)++x.s.retry_epoch;if(fault==11)x.s.token="foreign";
  if(fault==12)x.f.room=162;if(fault==13)x.f.controls=false;if(fault==14)x.f.seed=2;if(fault==15)x.s.player=Fixture::boss;
  const auto before=x.mem;const auto out=x.step();need(out.handled&&!out.x&&!out.buttons&&x.mem==before,"Unproved source re-arm, partial binding, stale scope/generation or locked controls cannot delegate refill or walk on cached coordinates");
 }
 for(unsigned fault=0;fault<3;++fault){
  Fixture x;x.rest(3,false);x.fl(Fixture::player+0x84,459.044f);const auto old=x.culled_point();++x.f.vi;need(!x.step().handled,"Reappearance negative starts with an attested re-armed source");
  x.health(7);x.stock(6);x.recreated_point(old,fault!=0);++x.f.vi;
  if(fault==1){const auto other=Fixture::record+32;x.w(other,0xFFFFFFFF);x.fl(other+4,160);x.fl(other+8,448);x.w(Fixture::point+0x1D8,other);x.chars.binding.record=other;}
  if(fault==2){x.fl(Fixture::record+4,161);x.chars.binding.x=n::word(x.rdram,Fixture::record+4);}
  const auto before=x.mem;x.step();need(x.c.failed()&&x.mem==before,"A recreated point cannot reuse its retired birth or replace the originally attested placement even with a fresh-looking live identity");
 }
 {
  Fixture x;x.rest(3,false);x.fl(Fixture::player+0x84,459.044f);x.culled_point();++x.f.vi;need(!x.step().handled,"Reset fixture first proves its cached source path");
  x.c.configure(true);x.s.phase=b::Phase::Fighting;x.s.index=x.s.completed=x.s.max_completed=0;x.f.room=x.f.logical_room=162;++x.f.vi;x.step();
  x.s.phase=b::Phase::Rest;x.s.index=2;x.s.completed=x.s.max_completed=3;x.f.room=x.f.logical_room=83;++x.f.vi;const auto before=x.mem;need(x.step().handled&&x.mem==before,"Configure clears cached placement; a re-armed record alone cannot license refill in a new run");
 }
 for(unsigned fault=0;fault<9;++fault){Fixture x(0,false);if(fault==0)x.f.diagnostic=false;if(fault==1)x.f.driver=false;if(fault==2)x.f.seed=2;if(fault==3)x.s.diagnostic=false;if(fault==4)x.s.token.clear();if(fault==5)x.s.index=4;if(fault==6)x.w(0x800F8828,0);if(fault==7)x.w(0x807F0008,14);if(fault==8)x.s.completed=1;const auto before=x.mem;need(x.step().handled&&x.c.failed()&&x.mem==before,"Wrong mode/seed/start/primary singleton/colour cannot launch the route test");}
 for(unsigned fault=0;fault<9;++fault){Fixture y;y.rest(3);++y.f.vi;need(inv::capture(y.rdram,Fixture::player,y.f.vi,y.saved),"Negative receipt starts with readable inventory");y.s.checkpoint_completed=3;y.s.checkpoint_awake_mask=2;y.w(a::count_address,1);y.w(a::flags_address+4,1);
  if(fault==0)y.saved.activation_vi=100;if(fault==1)y.saved.count=5;if(fault==2)y.saved.hp=6;if(fault==3)y.chars.binding.birth=0;if(fault==4)y.w(Fixture::res+0x44,Fixture::overlay);if(fault==5)y.s.checkpoint_awake_mask=6;if(fault==6)y.s.checkpoint_completed=0;if(fault==7)y.saved.eggs[0].palette=13;if(fault==8)y.ledger.valid=false;
  const auto before=y.mem;y.step();need(y.c.failed()&&y.mem==before,"Stale/partial/foreign/mismatched wake cannot certify prepared activation");
 }
 for(unsigned fault=0;fault<7;++fault){Fixture x;x.setup_loss();auto ctx=x.fatal();if(fault==0)ctx.r31=n::base(0x80000300);if(fault==1)ctx.r4=n::base(Fixture::player+4);if(fault==2)x.w(Fixture::pd+0xC0,0);if(fault==3)x.w(Fixture::pd+0xD8,2);if(fault==4)x.by(Fixture::player+0x18,1);if(fault==5)x.by(Fixture::player+0x30,2);if(fault==6)x.w(0x800F8828,0);const auto before=x.mem;x.c.native_death(x.rdram,&ctx,x.f);need(x.c.snapshot().phase==q::Phase::SeekDeath&&x.mem==before,"Wrong native caller, singleton or fatal state cannot witness a death");}
 for(unsigned sub:{0xAu,0xBu,0xCu,0xDu,0x10u,0x11u})for(uint32_t ra:{0x800301D4u,0x80030780u}){Fixture x;x.setup_loss();auto ctx=x.fatal(sub,ra);x.c.native_death(x.rdram,&ctx,x.f);x.phase(q::Phase::AwaitRetry,"Both source-proved callers admit every actual fatal substate");}
 for(unsigned fault=0;fault<12;++fault){Fixture x;x.setup_loss();x.die();x.arrived();if(fault==0)x.health(6);if(fault==1)x.w(0x80080000+0x1EC,2);if(fault==2)x.w(0x807F0008,1);if(fault==3)x.s.completed=4;if(fault==4)x.s.max_completed=2;if(fault==5)x.s.checkpoint_awake_mask=6;if(fault==6)x.ledger.treeConsumed=2;if(fault==7)x.ledger.hybridMask=1;if(fault==8)x.ledger.heartCount=1;if(fault==9)x.fl(Fixture::player+0x44,499);if(fault==10)x.fl(Fixture::player+0xCC,1);if(fault==11){x.actor(0x80090000,0x4015);x.w(0x80090000+0x180,Fixture::player);x.w(Fixture::player+0x188+6*4,0x80090000);x.h(Fixture::player+0x184,7);}const auto before=x.mem;x.step();need(x.c.failed()&&x.mem==before,"Wrong restoration/progress/FIFO/consumption/depth or death carrier fails without repair");}
 {Fixture x;x.setup_loss();x.die();x.arrived();x.f.retry_restored_epoch=0;need(x.step().handled&&x.c.snapshot().completed_tests==0&&!x.c.failed(),"Arrival without Runtime's native restore epoch stays pending");x.f.retry_restored_epoch=1;x.f.controls=false;need(x.step().handled&&x.c.snapshot().completed_tests==0,"Native controls release is mandatory after the restore epoch");x.f.controls=true;x.step();need(x.c.snapshot().completed_tests==1,"Readback completes only after both native fences");}
 {Fixture x;x.setup_loss();x.health(0);need(x.step().handled&&x.c.snapshot().phase==q::Phase::SeekDeath,"Living HP0 cannot prove death or advance the test");x.arrived();x.step();need(x.c.failed(),"A retry epoch without the F5F4 observer cannot be certified");}
 {Fixture x;x.setup_loss();x.s.defeated=1;need(x.step().handled&&x.c.failed(),"Even one genuine member victory invalidates the intended losing attempt");}
 {Fixture x;x.rest(3);x.f.vi+=7201;x.step();need(x.c.failed(),"Unfinished rest has an explicit deadline and no fabricated success");}
 {Fixture x;x.setup_loss();x.die();x.f.vi+=1801;x.step();need(x.c.failed(),"Missing native transport fails at its bounded deadline");}
 {Fixture x;x.f.vi=99;x.step();need(x.c.failed(),"Rewound VI cannot manufacture another event");}
 {
  Fixture x;workshop::boss_tree0240::Snapshot tree;tree.enabled=true;tree.totalConsumed=2;tree.consumedmask=3;
  workshop::boss_tree_hybrid0260::Snapshot hybrid;hybrid.enabled=true;hybrid.consumed=1;hybrid.consumed_mask=1;for(unsigned i=0;i<6;++i)hybrid.order[i]=uint16_t(0x4F00+i);
  workshop::boss_rest_heart0260::Snapshot heart;heart.enabled=true;
  const auto before=x.mem;auto l=q::observe_ledger(x.rdram,tree,hybrid,heart);need(l.valid&&l==x.ledger&&x.mem==before,"Shared ledger reads exact normal/hybrid/heart state without mutation");
  heart.consumed=true;x.w(workshop::boss_rest_heart0260::heart_count_address,1);need(q::observe_ledger(x.rdram,tree,hybrid,heart).valid,"Consumed heart has a matching native counter");
  x.w(workshop::boss_rest_heart0260::heart_count_address,0);need(!q::observe_ledger(x.rdram,tree,hybrid,heart).valid,"Consumed heart with a reset native statistic is not certified");
  heart.consumed=false;hybrid.order[1]=hybrid.order[0];need(!q::observe_ledger(x.rdram,tree,hybrid,heart).valid,"Malformed hybrid permutation is rejected");
 }
 std::cout<<"BossRouteCheckpointTest0260Checks: "<<checks<<" PASS (read-only host observations; no native/GPU checkpoint or Full10 claim)\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
