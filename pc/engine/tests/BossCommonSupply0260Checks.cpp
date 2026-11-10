#include "../boss_battle_runtime0250.hpp"
#include "../boss_driver_bridge0260.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
namespace d=workshop::boss_driver0260;
namespace s=workshop::boss_stick_aim0260;
namespace n=workshop::boss_native;
namespace bridge=workshop::boss_driver_bridge0260;
static unsigned checks=0;
static void need(bool yes,const char* why){++checks;if(!yes){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
static bool same(const d::Output&a,const d::Output&b){return a.handled==b.handled&&a.buttons==b.buttons&&a.x==b.x&&a.y==b.y;}
struct Memory {
 std::vector<uint8_t> bytes=std::vector<uint8_t>(0x800000);uint8_t* rdram=bytes.data();
 static constexpr uint32_t player=0x80100000,priv=0x80110000;
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}void h(uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}void b(uint32_t a,uint8_t v){MEM_B(0,n::base(a))=v;}
 Memory(){
  w(player,0x19971110);h(player+12,0x4001);b(player+0x18,1);b(player+0x17D,2);w(player+0x1F4,priv);w(0x800F8828,player);
  constexpr uint32_t body=0x80120000,tip=0x80121000;
  for(const auto actor:std::array<std::array<uint32_t,4>,2>{{{body,0x400B,0x80122000,0x80124000},{tip,0x400C,0x80123000,0x80125000}}}){w(actor[0],0x19971110);h(actor[0]+12,uint16_t(actor[1]));b(actor[0]+0x18,1);w(actor[0]+0x1F4,actor[2]);w(actor[0]+0x168,actor[3]);w(actor[0]+0x20,0x8003C9B0);}
  w(player+0x6C,std::bit_cast<uint32_t>(.8f));w(player+0x70,std::bit_cast<uint32_t>(.8f));w(body+0x78,std::bit_cast<uint32_t>(.001f));
  w(priv+0x160,body);w(priv+0x15C,tip);w(body+0x180,player);w(tip+0x180,body);w(player+0x188+5*4,body);w(body+0x188+2*4,tip);w(tip+0x174,0x80126000);w(0x80126000,0x80126100);
 }
};
struct Fixture {
 d::Controller controller;d::Input in;std::array<d::Target,1> targets{};std::array<d::Supply,1> supplies{};std::array<float,80> xs{},ys{};std::array<float,1> island{384};
 explicit Fixture(uint16_t kind=0x4072,bool right=true){
  need(controller.configure(true,true,{},false),"Pure controller is explicitly diagnostic");
  targets[0]={0x80140000,kind,534,248,0,0,true,true,false,kind==0x455A,true,6.6f,2};supplies[0]={0x80150000,0x4526,right?434.f:334.f,178,true,false,0xC1};
  in.player=0x80100000;in.room=170;in.encounter=3;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.x=384;in.y=248;in.min_x=40;in.max_x=724;in.targets=targets;in.supplies=supplies;
  in.native_stick_observed=in.native_tongue_observed=in.native_tongue_ready=in.native_tongue_known=true;xs.fill(14);ys.fill(-18);in.native_mouth={true,right,0,1,1,xs,ys};in.native_tongue_max_reach=*workshop::boss_tongue_geometry0260::max_centre_reach(1.f,.001f,false);
  if(kind==0x4511||kind==0x4631)in.stable_stands=island;
 }
 d::Output at(unsigned vi){in.vi=vi;return controller.step(in);}
 // This is input/filter/facing bookkeeping, not a physics simulation. The
 // exact native filter was independently verified by BossNativeStick checks.
 d::Output advance(unsigned vi){const auto out=at(vi);const auto raw=s::convert({out.x,out.y});in.native_stick=s::native_axes(raw,in.native_stick);if(!(out.buttons&d::tongue_button)&&in.native_tongue_phase==0&&!in.native_tongue_callback_pending&&in.native_stick.x)in.native_mouth.right=in.native_stick.x>0;return out;}
};
int main(){
 // Read-only source-admission guard: normal tongue, not every possible B
 // action. Special111 takes a different native branch and is excluded.
 {Memory f;const auto before=f.bytes;need(bridge::observe_native_tongue_ready(f.rdram,Memory::player),"Unblocked normal native P1 is ready for ordinary tongue request");need(f.bytes==before,"Readiness observation leaves all8MiB untouched");f.b(Memory::player+0x18,2);need(bridge::observe_native_tongue_ready(f.rdram,Memory::player),"Native state2 is also admitted for ordinary tongue");}
 for(unsigned fault=0;fault<18;++fault){Memory f;
  if(fault==0)f.b(Memory::player+0x18,0);if(fault==1)f.b(Memory::player+0x18,3);if(fault==2)f.b(Memory::player+0x18,0xA);if(fault==3)f.b(Memory::player+6,3);if(fault==4)f.h(Memory::player+12,0x4002);if(fault==5)f.w(0x800F8828,0);
  if(fault==6)f.w(Memory::player+0x1F4,0);if(fault==7)f.w(Memory::player+0x1F4,0x807FF000);if(fault==8)f.w(Memory::player+0x1F4,0x807FFFF0);
  if(fault==9)f.b(Memory::priv+0x1F8,1);if(fault==10)f.h(Memory::priv+0x150,1);if(fault==11)f.h(Memory::priv+0x150,0xFFFF);if(fault==12)f.w(Memory::priv+0x12EC,1);if(fault==13)f.b(Memory::priv+0x111,1);if(fault==14)f.b(Memory::priv+0x154,1);if(fault==15)f.b(Memory::priv+0x159,3);if(fault==16)f.w(Memory::priv+0xC0,0x80012340);if(fault==17)f.w(Memory::player,0);
  const auto before=f.bytes;need(!bridge::observe_native_tongue_ready(f.rdram,Memory::player),"Wrong identity/state/private range or native tongue lock fails closed");need(before==f.bytes,"Rejected readiness observation cannot alter memory");
 }
 {Memory f;f.b(Memory::priv+0x159,2);f.w(Memory::priv+0xC0,0x80012340);const auto before=f.bytes;const auto q=workshop::boss_native_mouth0260::observe_ordinary_tongue(f.rdram,Memory::player);need(!q.observed&&!q.ready&&q.phase==2&&q.callback_pending,"Shared query preserves raw159/callback without classifying an unproven extension as live");need(f.bytes==before,"Shared Runtime/Bridge query is read-only");}
 {Memory f;f.w(Memory::player+0x1F4,0x807FF000);const auto q=workshop::boss_native_mouth0260::observe_ordinary_tongue(f.rdram,Memory::player);need(!q.observed&&!q.ready&&!q.phase&&!q.callback_pending,"Insufficient private12F0 range cannot publish a recognized tongue observation");}
 need(!bridge::observe_native_tongue_ready(nullptr,Memory::player),"Null readiness memory rejected");
 const std::array<uint16_t,7> families{0x4599,0x4072,0x40A3,0x4511,0x4631,0x455A,0x41C6};
 for(uint16_t kind:families)for(bool right:{false,true})for(s::Axes old:std::array<s::Axes,5>{{{0,0},{54,54},{-54,-54},{54,-54},{-54,54}}}){
  Fixture f(kind,!right);f.in.reserves=1;f.in.native_stick=old;f.targets[0].x=right?534.f:234.f;
  for(unsigned vi=0;vi<4;++vi){const auto out=f.advance(vi);need(f.controller.snapshot().phase==d::Phase::Face&&!out.buttons,"Each supported family uses ordinary inverse Face without B/Z/A");need(std::abs(out.x)<=1&&std::abs(out.y)<=1,"Inverse Face remains bounded host input");}
  need(f.in.native_mouth.right==right,"Four normal Face observations reach requested native facing even from opposing axes");const auto begin=f.at(4);need(f.controller.snapshot().phase==d::Phase::AcquireAim&&begin.buttons==d::egg_button,"Facing change retains the existing egg-acquisition schedule");
 }
 for(uint16_t kind:families)for(bool right:{false,true}){
  Fixture f(kind,right);f.in.reserves=0;
  for(unsigned vi=0;vi<12;++vi){f.in.grounded=vi==0;const auto out=f.advance(vi);need(out.buttons==d::jump_button&&f.in.native_mouth.right==right,"Existing twelve-VI jump preparation stays A-only and preserves observed facing");}
  auto out=f.advance(12);need(out.buttons==d::tongue_button,"Reachable native-mouth target gets a new B request after real airborne delay");need(f.in.native_mouth.right==right,"Relative axes are paired with B rather than applied as prior global movement");
  f.in.native_tongue_ready=false;f.in.native_tongue_phase=2;f.in.native_tongue_live=true;
  for(unsigned vi=13;vi<80;++vi){out=f.advance(vi);need(out.buttons==d::tongue_button&&out.x==0&&out.y==0,"Observed native extension2 holds B without world steering when EA4 is not observed active");need(f.in.native_mouth.right==right,"Accepted native extension cannot become a global turn");need(same(out,f.at(vi)),"Same-VI extension request is idempotent");}
  f.in.native_tongue_phase=1;out=f.advance(80);need(!out.buttons&&!out.x&&!out.y,"Native retraction1 releases B and stays neutral");f.in.native_tongue_phase=3;out=f.advance(81);need(!out.buttons&&!out.x&&!out.y,"Native post-tongue3 stays neutral");f.in.native_tongue_phase=0;f.in.native_tongue_live=false;f.in.native_tongue_callback_pending=true;out=f.advance(82);need(!out.buttons&&!out.x&&!out.y,"Pending native callback outside extension2 stays neutral");
 }
 for(uint16_t kind:families)for(bool right:{false,true}){
  Fixture f(kind,right);f.in.reserves=0;f.at(0);f.in.grounded=false;f.in.native_stick={54,-54};const auto out=f.advance(12);need(!(out.buttons&d::tongue_button),"Impossible one-VI relative angle is not emitted as a B request");need(f.in.native_mouth.right==right,"Rejected precision candidate keeps the observed global facing");
 }
 for(bool right:{false,true}){
  Fixture f(0x4072,right);f.in.reserves=0;f.in.native_tongue_ready=false;f.at(0);f.in.grounded=false;auto out=f.advance(12);need(!(out.buttons&d::tongue_button)&&f.in.native_mouth.right==right,"Native cooldown/lock blocks new tongue request and preserves LEFT/RIGHT");
  f.in.native_tongue_ready=true;f.in.native_stick={0,0};f.in.native_mouth.observed=false;out=f.advance(13);need(!(out.buttons&d::tongue_button)&&!out.x&&!out.y,"Missing native mouth refuses relative motion and B");
 }
 for(float margin:{-.1f,0.f,.1f}){Fixture f;const float distance=f.in.native_tongue_max_reach+margin;f.in.reserves=0;f.supplies[0].x=398;f.supplies[0].y=230-distance;f.at(0);f.in.grounded=false;const auto out=f.at(12);need(bool(out.buttons&d::tongue_button)==(margin<=0),"Tongue reach uses the observed native geometry limit without changing body reach/jump strategy");}
 // A held B from a rejected first request is not another pressed edge. The
 // legacy pulse may stay held, but no relative axes accompany its later VI.
 {Fixture f;f.in.reserves=0;f.at(0);f.in.grounded=false;need(f.at(12).buttons==d::tongue_button,"Initial request enters existing B window");f.in.native_tongue_ready=false;for(unsigned vi=13;vi<=15;++vi){const auto out=f.at(vi);need(out.buttons==d::tongue_button&&!out.x&&!out.y,"Repeated held-B outputs are neutral rather than a second aimed request");}}
 for(auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress}){
  Fixture f(0x4511);f.in.reserves=0;f.in.throw_mode=mode;f.in.native_tongue_phase=2;f.in.native_tongue_live=true;f.in.native_tongue_ready=false;f.in.aiming_actor=0x80200000;f.in.reticle_actor=0x80201000;f.in.reticle_x=0;f.in.reticle_y=0;const auto out=f.at(10);need(f.controller.snapshot().phase==d::Phase::TrackAim&&out.buttons==(mode==d::ThrowMode::HoldAndRelease?d::egg_button:0),"True4005 keeps priority over tongue-hold observation in both modes");
 }
 {Fixture f(0x4511);f.in.reserves=0;f.in.native_tongue_phase=2;f.in.native_tongue_live=true;f.in.native_tongue_ready=false;f.in.mouth_full=true;auto out=f.at(10);need(out.y<0&&!out.buttons,"Actual held food keeps safe grounded Down priority over extending tongue");f.in.grounded=false;out=f.at(11);need(out.y==0&&!out.buttons,"Actual airborne held food never forces Down");}
 {Fixture f(0x4511);f.in.reserves=1;f.in.x=400;const auto out=f.at(10);need(out.x<0&&!out.buttons,"Existing fish island guard still recentres before pre-aim actions");}
 for(unsigned reset=0;reset<4;++reset){Fixture f;f.in.reserves=0;f.at(0);f.in.grounded=false;need(f.at(12).buttons==d::tongue_button,"Reset fixture has an actual output B window");unsigned vi=13;if(reset==0)f.in.player+=0x280;if(reset==1)f.in.room++;if(reset==2)f.in.encounter++;if(reset==3)vi=1;f.in.grounded=true;const auto out=f.at(vi);need(out.buttons==d::jump_button&&f.controller.snapshot().jump_vi==vi,"New identity or clock resets supply preparation rather than inheriting a B pulse");}
 std::printf("BossCommonSupply0260Checks: %u PASS (isolated read-only observations/controller arithmetic; no native physics/game/GPU/save/ledger/victory proof)\n",checks);
}
