#include "../boss_fish_cp6_test0284.hpp"
#include <cstring>
#include <stdexcept>
#include <vector>
#include <iostream>
namespace g=workshop::boss_fish_cp6_test0284;namespace b=workshop::boss_battle0250;namespace n=workshop::boss_native;namespace inv=workshop::boss_checkpoint_inventory0260;namespace pose=workshop::boss_checkpoint_pose0260;namespace a=workshop::boss_checkpoint_actor0260;
static unsigned checks=0;static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
// Host boundary fixtures only. No native victory, wake, death or transport is
// executed here. Every call asserts byte-for-byte read-only8MiB and registers.
struct Fixture {
 static constexpr uint32_t player=0x80050000,pd=0x80051000,point=0x80060000,pp=0x80061000,res=0x80062000,record=0x80063000,overlay=0x80200000,sp=0x80700000;
 std::vector<uint8_t> mem=std::vector<uint8_t>(0x800000);uint8_t* rdram=mem.data();g::Controller controller;b::Snapshot session;a::Snapshot characters;inv::Payload saved;pose::Payload plane;g::Frame f;
 unsigned hp=8,count=4;
 Fixture(unsigned color=0,unsigned health=8,unsigned stock=4):hp(health),count(stock){
  actor(player,0x4001);w(player+0x1F4,pd);w(0x800F8828,player);w(0x807F0008,color);w(0x800F8DB0,color<8?color:0);set_health(health);set_stock(stock);
  fl(player+0x84,160);fl(player+0x88,449);fl(player+0x44,500.34f);fl(player+0x8C,500.34f);
  session.phase=b::Phase::Fighting;session.player=player;session.diagnostic=true;session.token="fish-cp6-host-boundary";
  f.session=&session;f.characters=&characters;f.saved=&saved;f.saved_pose=&plane;f.vi=100;f.seed=g::required_seed;f.diagnostic=f.driver=f.controls=f.target_plan=f.baseline=true;f.room=f.logical_room=162;f.ledger.valid=true;
  f.ledger.treeConsumed=2;f.ledger.treeMask=3;f.ledger.hybridConsumed=1;f.ledger.hybridMask=1;for(unsigned i=0;i<6;++i)f.ledger.hybridOrder[i]=uint16_t(0x4F00+i);
  controller.configure(true);step();
 }
 void w(uint32_t at,uint32_t v){MEM_W(0,n::base(at))=v;}void h(uint32_t at,unsigned v){MEM_H(0,n::base(at))=uint16_t(v);}void by(uint32_t at,unsigned v){MEM_B(0,n::base(at))=uint8_t(v);}void fl(uint32_t at,float v){w(at,std::bit_cast<uint32_t>(v));}
 void actor(uint32_t at,unsigned kind){w(at,0x19971110);h(at+12,kind);by(at+6,0);by(at+0x17D,2);by(at+0x18,1);}
 void set_health(unsigned v){w(0x800F8DB4+n::word(rdram,0x800F8DB0)*4,v);}
 void set_stock(unsigned v){
  for(unsigned i=0;i<20;++i)w(player+0x188+i*4,0);for(unsigned i=0;i<8;++i)w(pd+0x11FC+i*24,0);by(pd+0x12C8,v);h(pd+0x12BE,0);h(player+0x184,v);
  constexpr unsigned colors[]{0,8,13,3,9,5};for(unsigned i=0;i<v;++i){const auto egg=0x80080000+i*0x400;actor(egg,0x4004);w(egg+0x180,player);w(egg+0x1EC,colors[i]);for(unsigned j=0;j<3;++j)fl(egg+0x78+j*4,.56f);w(player+0x188+i*4,egg);w(pd+0x11FC+i*24,egg);}
 }
 g::Output step(){const auto before=mem;const auto out=controller.frame(rdram,f);need(before==mem,"QA frame is read-only across8MiB");return out;}
 void phase(g::Phase p){if(controller.snapshot().phase!=p)std::cerr<<"phase="<<unsigned(controller.snapshot().phase)<<" expected="<<unsigned(p)<<" reason="<<controller.snapshot().failure<<'\n';need(controller.snapshot().phase==p,"Expected QA phase");}
 void rest(){f.vi=200;session.phase=b::Phase::Rest;session.index=5;session.completed=session.max_completed=6;f.room=f.logical_room=83;f.prefix_receipts=true;
  characters.enabled=true;characters.room=83;actor(point,0x4509);w(point+0x1F4,pp);w(point+0x168,res);w(point+0x1D8,record);w(point+0x1DC,a::source_packed);w(res+0x40,overlay);w(res+0x44,overlay+a::initializer_offset);w(record,0xFFFFFFFF);fl(record+4,160);fl(record+8,448);w(pp+0x27C,2);fl(point+0x84,160);fl(point+0x88,448);
  auto& p=characters.binding;p.actor=point;p.private_data=pp;p.record=record;p.packed=a::source_packed;p.resource=res;p.overlay=overlay;p.x=n::word(rdram,record+4);p.y=n::word(rdram,record+8);p.birth=20;p.room=83;p.milestone=6;p.index=2;characters.next_birth=p.birth;step();phase(g::Phase::Wake);
 }
 void wake(){f.vi=210;need(inv::capture(rdram,player,f.vi,saved),"Native-shaped current FIFO capture");need(pose::capture(rdram,saved,plane),"Native-shaped plane capture");session.checkpoint_completed=6;session.checkpoint_awake_mask=4;step();phase(g::Phase::Companion);}
 void fight(){f.vi=300;session.phase=b::Phase::Fighting;session.index=6;f.room=173;f.logical_room=97;f.fight_ready=f.members_live=true;f.members={0x80070000,0x80071000};f.births={30,31};actor(f.members[0],0x4511);actor(f.members[1],0x4072);const auto out=step();phase(g::Phase::Companion);need(out.handled&&out.companion,"Only companion pilot owns this attempt");}
 void companion(){f.vi=400;session.defeated=2;f.companion_receipt=f.companion_consumed=true;step();phase(g::Phase::SeekDeath);}
 recomp_context fatal(){f.vi=500;set_health(0);by(player+0x18,0xA);by(player+0x30,0xC);w(player+0x20,0x8003085C);w(pd+0xC0,0x8002F5F4);w(sp+0x14,0x80030878);w(sp+0x1C,0x8002F5F4);w(sp+0x20,player);w(sp+0x24,pd);f.ledger.treeConsumed=3;f.ledger.treeMask=7;recomp_context c{};c.r4=n::base(player);c.r31=n::base(0x8004217C);c.r29=n::base(sp);return c;}
 void death(){auto ctx=fatal();const auto before=mem;const auto regs=ctx;controller.native_death(rdram,&ctx,f);need(before==mem&&!std::memcmp(&regs,&ctx,sizeof(ctx)),"Death witness is read-only RAM/registers");phase(g::Phase::AwaitRetry);}
 void arrived(){f.vi=600;session.phase=b::Phase::Rest;session.index=5;session.retry_epoch=1;session.defeated=0;f.room=f.logical_room=83;f.retry_restored_epoch=1;by(player+0x18,1);by(player+0x30,0);w(player+0x20,0);w(pd+0xC0,0);set_health(hp);set_stock(count);step();phase(g::Phase::Replay);}
 void replay(){f.vi=700;session.phase=b::Phase::Fighting;session.index=6;f.room=173;f.logical_room=97;f.members_live=true;f.births={40,41};step();phase(g::Phase::Replay);need(controller.snapshot().replayed_births==f.births,"Both fresh replay births captured");}
 void win(){f.vi=800;session.phase=b::Phase::Victory;session.completed=session.max_completed=7;session.defeated=3;f.replay_receipts=true;f.cargo_epoch=5;step();phase(g::Phase::ReturnRest);}
 void done(){f.vi=810;session.phase=b::Phase::ToRest;f.cargo_epoch=6;step();f.vi=900;session.phase=b::Phase::Rest;f.room=f.logical_room=83;step();phase(g::Phase::Complete);need(controller.complete(),"Exact CP6 replay completed only after native-shaped pot/rest");}
 void until_death(){rest();wake();fight();companion();death();}
};
int main(){try{
 const auto p=b::make_plan(g::required_seed);need(b::valid(p)&&p.fights[6].family==b::Family::Fish&&p.fights[6].variant==2&&p.fights[6].terrain==1&&p.fights[6].members[0].kind==0x4511&&p.fights[6].members[0].hits==3&&p.fights[6].members[1].kind==0x4072,"Actual current planner matches6November Fish7 roster");
 for(unsigned color=0;color<14;++color)for(unsigned hp=1;hp<=8;++hp)for(unsigned count=0;count<=6;++count){Fixture f(color,hp,count);f.until_death();f.arrived();f.replay();f.win();f.done();}
 {Fixture f;f.rest();const auto out=f.step();need(out.buttons==0,"Wake needs neutral VI beforeA");++f.f.vi;need(f.step().buttons==0x8000,"Wake uses ordinary boundedA");}
 for(unsigned which=0;which<10;++which){Fixture f;f.rest();f.wake();f.fight();f.companion();auto ctx=f.fatal();switch(which){case 0:ctx.r31=n::base(0x800301D4);break;case 1:ctx.r4=0;break;case 2:f.w(Fixture::sp+0x14,0);break;case 3:f.w(Fixture::sp+0x1C,0);break;case 4:f.w(Fixture::sp+0x20,0);break;case 5:f.w(Fixture::sp+0x24,0);break;case 6:f.by(Fixture::player+0x30,2);break;case 7:f.w(Fixture::player+0x20,0);break;case 8:f.w(Fixture::pd+0xC0,0);break;case 9:ctx.r29=n::base(0x807FFFFC);break;}const auto before=f.mem;f.controller.native_death(f.rdram,&ctx,f.f);need(before==f.mem,"Rejected native proof remains read-only");f.phase(g::Phase::SeekDeath);}
 for(unsigned which=0;which<8;++which){Fixture f;f.until_death();f.f.vi=600;f.session.phase=b::Phase::Rest;f.session.index=5;f.session.retry_epoch=1;f.session.defeated=0;f.f.room=f.f.logical_room=83;f.f.retry_restored_epoch=1;f.by(Fixture::player+0x18,1);f.by(Fixture::player+0x30,0);f.set_health(f.hp);f.set_stock(f.count);switch(which){case 0:f.set_health(7);break;case 1:f.set_stock(3);break;case 2:f.w(0x80080000+0x1EC,9);break;case 3:f.fl(0x80080000+0x78,.8f);break;case 4:f.fl(Fixture::player+0x8C,1);break;case 5:f.f.ledger.treeConsumed=2;break;case 6:f.saved.activation_vi++;break;case 7:f.session.retry_epoch=2;break;}f.step();need(f.controller.failed(),"Changed CP6 inventory/pose/ledger/epoch rejected");}
 for(unsigned which=0;which<6;++which){Fixture f;switch(which){case 0:f.f.seed=1;break;case 1:f.f.target_plan=false;break;case 2:f.session.token="foreign";break;case 3:f.f.vi=72001;break;case 4:f.session.retry_epoch=1;break;case 5:f.session.phase=b::Phase::Failed;f.session.failure="yoshi_defeated";break;}f.step();need(f.controller.failed(),"Foreign scope/progress rejected");}
 {Fixture f;f.rest();f.wake();f.fight();f.f.vi=400;f.session.defeated=1;f.step();need(f.controller.failed(),"Fish-first defeat cannot certify intended loss");}
 for(unsigned which=0;which<2;++which){Fixture f;f.rest();f.wake();f.fight();f.f.vi=400;f.session.defeated=2;f.f.companion_receipt=which!=0;f.f.companion_consumed=which!=1;f.step();need(f.controller.failed(),"Receipt and consumed companion record both required");}
 {Fixture f;f.until_death();f.arrived();f.f.vi=700;f.session.phase=b::Phase::Fighting;f.session.index=6;f.f.room=173;f.f.logical_room=97;f.f.members_live=true;f.step();need(f.controller.failed(),"Stale same-address replay births rejected");}
 {Fixture f;f.until_death();f.arrived();f.replay();f.win();f.f.vi=900;f.session.phase=b::Phase::Rest;f.f.room=f.f.logical_room=83;f.step();need(f.controller.failed(),"No pot cargo departure cannot certify rest arrival");}
 {Fixture f;f.until_death();f.arrived();f.replay();f.f.vi=800;f.session.phase=b::Phase::Victory;f.session.completed=7;f.session.defeated=3;f.step();need(f.controller.failed(),"Missing two causal postretry receipts rejected");}
 std::cout<<"BossFishCP6Test0284 PASS "<<checks<<" checks;784 FIFO/HP/color host-boundary flows; no native/GPU gameplay claim\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
