#include "../boss_battle_runtime0250.hpp"
#include "../boss_driver_bridge0260.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace q=workshop::boss_bird_test0260;namespace n=workshop::boss_native;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct Fixture {
 static constexpr uint32_t player=0x80050000,bird=0x80060000,priv=0x80061000,res=0x80062000,factory=0x80200000,meta=0x80063000,def=0x80063100,egg=0x80065000,wounded=0x80068000,newpriv=0x80069000,newres=0x8006A000;
 std::vector<uint8_t> mem=std::vector<uint8_t>(0x800000);uint8_t* rdram=mem.data();q::Controller controller;std::array<q::Bird,3> birds{};q::Frame f{true,true,true,true,171,97,4,2500,player,6,0,152,248,0,768};
 void w(uint32_t a,uint32_t value){MEM_W(0,n::base(a))=int32_t(value);}void by(uint32_t a,unsigned value){MEM_B(0,n::base(a))=uint8_t(value);}void h(uint32_t a,unsigned value){MEM_H(0,n::base(a))=uint16_t(value);}void fl(uint32_t a,float value){w(a,std::bit_cast<uint32_t>(value));}
 void actor(uint32_t a,unsigned kind){w(a,0x19971110);h(a+12,kind);by(a+0x17D,2);by(a+0x18,1);}
 Fixture(){controller.configure(true);actor(player,0x4001);w(0x800F8828,player);actor(bird,0x4526);by(bird+0x30,5);w(bird+0x1F4,priv);w(bird+0x168,res);w(bird+0x174,meta);w(meta,def);w(def+0x28,factory+0x1A14);by(def+0x74,1);h(res+0x3C,0x4526);w(res+0x40,factory);w(res+0x44,factory+0x1B1C);w(bird+0x20,factory+0x9BC);fl(bird+0x84,176);fl(bird+0x88,176);fl(bird+0xC4,1);fl(bird+0xC8,0);birds[0]={bird,0,0,0,17};actor(egg,0x4002);}
 std::optional<workshop::boss_driver0260::Target> observe(){return controller.observe(rdram,f,birds);}
 void shot(){controller.native_launch(rdram,player,egg,bird,1,f.vi+1);f.vi+=2;}
 void successor(bool initialized=false){
  // Host fixture of the native3444 return/46DA35B4 staging.
  // No test claims this fixture itself executed the N64 routine.
  actor(wounded,0x46DA);w(wounded+0x168,newres);h(newres+0x3C,0x46DA);w(newres+0x40,factory);w(newres+0x44,factory+0x1B84);by(wounded+0x17D,1);by(wounded+0x18,0);w(wounded+0x20,factory+0x1B84);
  w(bird+0x180,wounded);h(bird+0x17E,1);h(wounded+0x184,2);w(wounded+0x18C,bird);by(bird+0x17D,0);by(bird+0x165,1);by(bird+0x17C,0);w(bird+0x20,factory+0x1A08);
  birds[0]={wounded,0,newres,factory,18,bird,priv,17};
  if(initialized){by(wounded+0x17D,2);by(wounded+0x18,1);by(wounded+0x30,7);w(wounded+0x20,factory+0xAE4);w(wounded+0x1F4,newpriv);birds[0].private_data=newpriv;}
 }
};
int main(){try{
 {Fixture x;x.controller.configure(false);const auto before=x.mem;need(!x.observe()&&x.controller.complete()&&x.controller.snapshot().phase==q::Phase::Disabled&&x.mem==before,"Default false is inert and imposes no QA goal on ordinary drivers");}
 for(bool initialized:{false,true}){Fixture x;const auto before=x.mem;const auto t=x.observe();need(t&&t->kind==0x4526&&t->actor==Fixture::bird&&t->birth==17&&t->vx==1,"QA selects the true live4526 identity, generation and native motion");need(x.mem==before&&!x.controller.complete(),"Selection cannot mutate guest memory or prove an injury");x.shot();need(x.controller.snapshot().shots==1&&x.controller.snapshot().phase==q::Phase::AwaitInjury&&!x.observe(),"Real404C8 entry moves to neutral native-injury observation");need(!x.controller.complete(),"A native launched4002 does not by itself prove damage");x.successor(initialized);const auto graph=x.mem;need(!x.observe()&&x.controller.complete()&&x.controller.snapshot().wounded==Fixture::wounded,"Only exact sourcebirth and native queued-or-initialized46DA graph complete the QA goal");need(x.mem==graph,"Successor observation is read-only");x.birds={};x.f.vi+=100;need(!x.observe()&&x.controller.complete(),"One successful bird test stays complete after later native disposal");}
 for(unsigned fault=0;fault<16;++fault){Fixture x;if(fault==0)x.f.diagnostic=false;if(fault==1)x.f.driver=false;if(fault==2)x.f.room=83;if(fault==3)x.f.source_room=171;if(fault==4)x.f.index=3;if(fault==5)x.f.alive=false;if(fault==6)x.w(0x800F8828,0);if(fault==7)x.by(Fixture::bird+0x30,7);if(fault==8)x.w(Fixture::bird+0x20,Fixture::factory+0x9C0);if(fault==9)x.w(Fixture::def+0x28,Fixture::factory+0x1A18);if(fault==10)x.by(Fixture::def+0x74,0);if(fault==11)x.birds[0].birth=0;if(fault==12)x.w(Fixture::bird+0x180,Fixture::wounded);if(fault==13)x.h(Fixture::res+0x3C,0x46DA);if(fault==14)x.birds[0].resource=Fixture::newres;if(fault==15)x.fl(Fixture::bird+0x84,700);const auto before=x.mem;need(!x.observe()&&!x.controller.complete()&&x.mem==before,"Wrong mode/source/ownership/callback/kind or remote bird cannot create a target or mutate memory");}
 for(unsigned fault=0;fault<12;++fault){Fixture x;need(bool(x.observe()),"Successor negative case selects its owned source");x.shot();x.successor();if(fault==0)x.birds[0].source_birth=16;if(fault==1)x.birds[0].source_actor=Fixture::player;if(fault==2)x.birds[0].source_private=Fixture::newpriv;if(fault==3)x.birds[0].birth=17;if(fault==4)x.w(Fixture::wounded+0x18C,0);if(fault==5)x.h(Fixture::wounded+0x184,21);if(fault==6)x.w(Fixture::wounded+0x20,Fixture::factory+0x1C24);if(fault==7)x.w(Fixture::newres+0x40,Fixture::factory+4);if(fault==8)x.w(Fixture::bird+0x20,Fixture::factory+0x1A0C);if(fault==9)x.by(Fixture::bird+0x165,0);if(fault==10)x.w(Fixture::wounded+0x1F4,Fixture::newpriv);if(fault==11)x.birds[0].factory=0;const auto before=x.mem;need(!x.observe()&&!x.controller.complete()&&x.controller.snapshot().phase==q::Phase::Failed&&x.mem==before,"Unproved46DA or stale generation never releases the priority or edits guest memory");}
 {Fixture x;x.observe();x.successor();need(!x.observe()&&!x.controller.complete(),"Unrelated replacement before a proven native QA shot is never accepted");}
 for(bool initialized:{false,true}){Fixture x;x.observe();x.shot();x.successor(initialized);x.w(Fixture::newres+0x44,Fixture::factory+0x1C20);x.w(Fixture::wounded+0x20,Fixture::factory+(initialized?0xE24:0x1C20));if(initialized)x.by(Fixture::wounded+0x30,9);need(!x.observe()&&!x.controller.complete()&&x.controller.snapshot().phase==q::Phase::Failed,"Species4046 initializer3650 and update2854 can never prove a46DA successor");}
 for(unsigned fault=0;fault<4;++fault){Fixture x;x.observe();x.controller.native_launch(x.rdram,fault==0?0:Fixture::player,Fixture::egg,fault==1?Fixture::player:Fixture::bird,fault==2?0:1,fault==3?2499:2501);need(x.controller.snapshot().shots==0&&x.controller.snapshot().phase==q::Phase::Aim,"Wrong source/release/epoch cannot manufacture a native-shot witness");}
 {Fixture x;x.observe();x.f.vi+=1201;const auto before=x.mem;need(!x.observe()&&x.controller.snapshot().phase==q::Phase::Failed&&x.mem==before,"Unsuccessful bird goal has a bounded deadline and no forced result");}
 {Fixture x;x.observe();for(unsigned shot=1;shot<=3;++shot){x.shot();need(x.controller.snapshot().shots==shot&&!x.observe(),"Each actual shot waits for real native injury");x.f.vi+=180;const auto next=x.observe();need(shot<3?bool(next):x.controller.snapshot().phase==q::Phase::Failed,"At most three genuine shots precede explicit QA failure");}need(!x.controller.complete(),"Three missed shots cannot claim success or resume boss targeting");}
 {Fixture x;x.f.controls=false;need(!x.observe()&&!x.controller.snapshot().player,"Locked controls wait without consuming the test deadline");x.f.controls=true;x.f.vi=7000;need(bool(x.observe())&&x.controller.snapshot().start_vi==7000,"Test deadline starts when the native controller is released");}
 {Fixture x;x.observe();x.birds[0].birth=18;need(!x.observe()&&x.controller.snapshot().phase==q::Phase::Failed,"An address reused with another birth cannot retain the target");}
 {Fixture x;x.observe();x.f.reserves=0;x.f.aiming_actor=0x80066000;need(bool(x.observe()),"The last reserve already held as native4005 can finish its genuine shot");x.f.aiming_actor=0;need(!x.observe()&&x.controller.snapshot().phase==q::Phase::Failed,"Exhausted stock cannot fall back to eating its QA target");}
 {
  namespace r=workshop::boss_campaign0250;namespace bridge=workshop::boss_driver_bridge0260;namespace d=workshop::boss_driver0260;
  Fixture x;need(r::configure("bird-bridge-host",1,true,true,4,false,true),"Composed host fixture uses the strict automatic diagnostic case4");r::physical_room=171;r::logical_room=97;need(r::session.enter_fight(97,Fixture::player),"Composed host fixture enters the real Fish source identity");bridge::configure(true);
  const auto target=bridge::bird_test.observe(x.rdram,x.f,x.birds);need(target&&!bridge::qa_bird_complete()&&!bridge::qa_bird_failed(),"Bridge opt-in starts its single pending bird goal");
  constexpr uint32_t pd=0x80051000,ep=0x80066000,er=0x80067000;x.w(Fixture::player+0x1F4,pd);x.w(Fixture::egg+0x1F4,ep);x.w(Fixture::egg+0x168,er);
  std::array<float,3> stands{152,384,616};d::Input in;in.room=171;in.encounter=4;in.player=Fixture::player;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.reserves=6;in.x=152;in.y=248;in.min_x=0;in.max_x=768;in.targets=std::span<const d::Target>(&*target,1);in.qa_bird=true;in.stable_stands=stands;
  // A real release journal is produced by ordinary Z/reticle state samples.
  // Native4005/404C8 boundaries below remain explicit host fixtures.
  for(unsigned vi=2500;vi<=2504;++vi){in.vi=vi;bridge::controller.step(in);}in.primary_state=3;in.aiming_actor=0x80071000;in.reticle_actor=0x80072000;
  const float lead=std::hypot(target->x-in.x,target->y-in.y)/14.f;in.reticle_x=target->x+target->vx*lead;in.reticle_y=target->y;
  for(unsigned vi=2505;vi<=2507;++vi){in.vi=vi;bridge::controller.step(in);}
  need(bridge::controller.release_trace().kind==0x4526&&bridge::controller.release_trace().root==Fixture::bird,"Bridge release journal names true4526 before native creation");
  recomp_context c{};c.r4=n::base(Fixture::egg);c.r5=n::base(Fixture::player);bridge::trace_native_launch(x.rdram,&c,"func_800404C8",2508);
  need(bridge::bird_test.snapshot().shots==1&&bridge::bird_test.snapshot().phase==q::Phase::AwaitInjury&&!bridge::qa_bird_complete(),"Existing404C8 observer routes a real-shaped source4001/4002 launch to the QA witness");
  x.successor();x.f.vi=2509;bridge::bird_test.observe(x.rdram,x.f,x.birds);need(bridge::qa_bird_complete()&&!bridge::qa_bird_failed(),"Bridge goal getter becomes true only after the matching native-shaped successor graph");
  bridge::configure();need(bridge::qa_bird_complete()&&!bridge::qa_bird_failed()&&!bridge::bird_test.snapshot().enabled,"Default configure restores the original unrestricted boss driver contract");r::enabled=false;
 }
 std::cout<<"BossBirdTest0260Checks: "<<checks<<" PASS (read-only host fixtures; no native gameplay victory claim)\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
