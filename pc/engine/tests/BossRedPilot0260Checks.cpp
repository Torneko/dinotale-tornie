#include "../boss_battle_runtime0250.hpp"
#include "../boss_driver_bridge0260.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
namespace c=workshop::boss_campaign0250;
namespace d=workshop::boss_driver0260;
static unsigned checks=0;
static void need(bool yes,const char* why){++checks;if(!yes){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
 d::Controller controller;d::Input in;std::array<d::Target,1> targets{};std::array<d::Supply,1> supplies{};
 Fixture(d::ThrowMode mode=d::ThrowMode::HoldAndRelease){need(controller.configure(true,true,{},false),"explicit diagnostic controller");in.vi=120;in.room=178;in.encounter=9;in.player=0x80100000;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.x=150;in.y=450;in.min_x=40;in.max_x=724;in.throw_mode=mode;targets[0]={0x80110000,0x455A,300,350,0,0,true,true,false,true};in.targets=targets;supplies[0]={0x80140000,0x4526,200,350,true,false,0xC1};in.supplies=supplies;}
 d::Output at(unsigned vi){in.vi=vi;return controller.step(in);}
};
int main(){
 for(unsigned reserves:{0u,1u,6u})for(bool aim:{false,true}){
  Fixture f;f.in.reserves=reserves;f.in.aiming_actor=aim?0x80120000:0;if(aim){f.in.primary_state=3;f.in.reticle_actor=0x80121000;f.in.reticle_x=300;f.in.reticle_y=350;}
  const auto out=f.at(120);need(!(out.buttons&d::tongue_button),"closed-red routing cannot emit tongue instead of supply or native aim");
  if(aim)need(f.controller.snapshot().phase==d::Phase::TrackAim&&out.buttons==d::egg_button,"real aiming egg retains control independently of current reserve count");else if(!reserves)need(f.controller.snapshot().phase==d::Phase::Supply&&out.buttons==d::jump_button,"empty closed red genuinely enters native bird-supply controls");else need(f.controller.snapshot().phase==d::Phase::Face,"available ammunition reaches native egg approach rather than supply");
  need(!c::candy_tongue_pilot(0x455A,true,false,false),"closed red never changes to tongue when reserves or aiming egg are absent");
  need(c::candy_tongue_pilot(0x455A,true,true,false),"real yellow window permits native tongue pilot regardless of ammunition");
  need(c::candy_tongue_pilot(0x455A,false,false,false)&&c::candy_tongue_pilot(0x455A,false,true,false),"ordinary candy retains tongue pilot with or without reserves/aim");
 }
 for(uint16_t kind:{uint16_t(0x4072),uint16_t(0x4599),uint16_t(0x40A3),uint16_t(0x4511),uint16_t(0x4631),uint16_t(0x41C6),uint16_t(0x454F),uint16_t(0x4550),uint16_t(0x4001)})for(bool red:{false,true})for(bool yellow:{false,true})need(!c::candy_tongue_pilot(kind,red,yellow,false),"non-candy roots and component actors cannot own tongue pilot");
 for(bool red:{false,true})for(bool yellow:{false,true})need(!c::candy_tongue_pilot(0x455A,red,yellow,true),"a defeated candy member never steals the remaining controller");
 {std::array<workshop::boss_battle0250::Member,4> members{};members[0].kind=0x4072;members[1].kind=0x455A;members[1].red=true;members[2].kind=0x455A;members[3].kind=0x4599;unsigned selected=4;for(unsigned n=0;n<4;++n)if(c::candy_tongue_pilot(members[n].kind,members[n].red,false,false)){selected=n;break;}need(selected==2,"closed red member cannot hide a later ordinary candy companion");selected=4;for(unsigned n=0;n<4;++n)if(c::candy_tongue_pilot(members[n].kind,members[n].red,false,n==2)){selected=n;break;}need(selected==4,"remaining closed red after ordinary companion defeat returns control to egg/Supply driver");}
 for(auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress}){
  Fixture f(mode);need(!c::candy_tongue_pilot(f.targets[0].kind,true,false,false),"closed red reaches egg controller at zero ammunition");auto out=f.at(120);need(f.controller.snapshot().phase==d::Phase::Supply&&out.buttons==d::jump_button&&!(out.buttons&d::tongue_button),"closed red with zero reserves pursues genuine native bird supply instead of inedible candy segments");
  f.in.reserves=1;f.at(121);for(unsigned vi=122;vi<=125;++vi)out=f.at(vi);need(out.buttons==d::egg_button&&f.controller.snapshot().phase==d::Phase::AcquireAim,"closed red with reserves starts genuine Z acquisition");
  f.in.reserves=0;f.in.aiming_actor=0x80120000;f.in.reticle_actor=0x80121000;f.in.reticle_x=300;f.in.reticle_y=350;f.in.primary_state=3;out=f.at(126);need(f.controller.snapshot().phase==d::Phase::TrackAim&&out.buttons==(mode==d::ThrowMode::HoldAndRelease?d::egg_button:0),"last real4005 remains under native aiming control after reserve ring becomes empty");
  need(!(out.buttons&d::tongue_button),"last aiming egg is never cancelled by closed-red tongue fallback");
  f.at(127);out=f.at(128);need(f.controller.snapshot().phase==d::Phase::Release&&!out.buttons,"genuine aligned reticle releases in both native throw modes");out=f.at(130);need(mode==d::ThrowMode::SecondPress?out.buttons==d::egg_button:out.buttons==0,"last egg preserves real secondpress versus hold-release semantics");
  need(!f.controller.snapshot().launches,"controller routing itself never claims a launched egg or an opened yellow window");
 }
 std::printf("BossRedPilot0260Checks: %u PASS (QA routing/controller fixtures only; no native egg, damage or victory proof)\n",checks);
}
