#include "../boss_driver0260.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>

namespace d=workshop::boss_driver0260;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
static bool near(float a,float b){return std::abs(a-b)<.00001f;}
struct Fixture {
 d::Controller controller;std::array<d::Target,1> targets{};std::array<d::Supply,4> supplies{};std::array<float,3> islands{152,384,616};d::Input in;
 Fixture(){
  need(controller.configure(true,true,{},false),"explicit synthetic diagnostic-only controller");
  targets[0]={0x80110000,0x4599,300,350,0,0,true,true,false};
  in.vi=1;in.room=164;in.encounter=2;in.player=0x80100000;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.reserves=0;in.x=150;in.y=450;in.min_x=40;in.max_x=724;in.targets=targets;in.supplies=supplies;
 }
 void bird(float x=200,float y=350){supplies[0]={0x80140000,0x4526,x,y,true,false,0xC1};}
 void feeder(unsigned side,float x=100,float y=376){supplies[0]={0x80141000,0x456E,x,y,true,false,0,0x4528,uint16_t(side),0};}
 void island(){in.stable_stands=islands;in.x=384;in.y=248;targets[0].kind=0x4511;targets[0].x=576;targets[0].y=328;}
 d::Output at(unsigned vi){in.vi=vi;return controller.step(in);}
};

int main(){
 // These fixtures provide observations to a controller only. No RAM, native
 // dispatcher, game process, victory receipt, save or ledger is available.
 for(unsigned start:{120u,240u,127u}){
  Fixture f;f.bird();auto out=f.at(start);need(out.buttons==d::jump_button&&f.controller.snapshot().jump_vi==start,"A edge is relative to actual supply-jump start, not global modulo");
  for(unsigned dt=1;dt<12;++dt){f.in.grounded=false;out=f.at(start+dt);need(out.buttons==d::jump_button&&!(out.buttons&d::tongue_button),"first12VI hold A without native B cancellation");const auto snap=f.controller.snapshot();const auto repeated=f.at(start+dt);need(repeated.buttons==out.buttons&&repeated.x==out.x&&repeated.y==out.y&&f.controller.snapshot().jump_vi==snap.jump_vi,"same-VI supply output and jump epoch are idempotent");}
  out=f.at(start+12);need(out.buttons==d::tongue_button&&!(out.buttons&d::jump_button),"earliest airborne B starts only after12 genuine elapsed VI");
  need(out.y>0&&out.x>.2f,"vertical tongue direction and native facing deadzone are prepared");
  for(unsigned dt=13;dt<16;++dt)need(f.at(start+dt).buttons==d::tongue_button,"B pulse remains bounded to four VI");
  need(!(f.at(start+16).buttons&d::tongue_button),"B pulse is released before next edge");
  f.in.encounter++;out=f.at(start+17);need(f.controller.snapshot().jump_vi==start+17&&!f.controller.snapshot().supply_jump_grounded&&!(out.buttons&d::jump_button),"new encounter resets supply provenance without inventing a ground jump for the still-airborne player");
 }
 for(unsigned start:{120u,127u}){
  Fixture f;f.bird();for(unsigned dt=0;dt<=64;++dt){const auto out=f.at(start+dt);need(!(out.buttons&d::tongue_button),"grounded bird acquisition never sends B");if(dt<45||dt==64)need(out.buttons==d::jump_button,"grounded retry starts with A only");else need(out.buttons==0,"blocked jump releases A from45 through63 before retry");}
  need(f.controller.snapshot().jump_vi==start+64,"retry epoch resets only at its bounded64VI boundary");
 }
 for(unsigned side:{0u,1u,2u,3u}){
  const float wait=100+((side&1)?32.f:-32.f);
  for(float offset:{-5.f,0.f,5.f}){Fixture f;f.feeder(side);f.in.x=wait+offset;const auto out=f.at(127);need(f.controller.snapshot().supply_actor==f.supplies[0].actor,"typed horizontal456E feeder is selected only as waiting destination");need(out.buttons==0&&out.y==0,"invisible feeder is never jumped toward or eaten");need(offset<0?out.x>0:offset>0?out.x<0:out.x==0,"feeder sidebit chooses exact sourceX plus32 or minus32");}
 }
 for(float delta:{-128.f,-127.5f,127.5f,128.f}){
  Fixture f;f.feeder(1,100,450+delta);f.in.x=150;const auto out=f.at(120);const bool allowed=std::abs(delta)<128;need((f.controller.snapshot().supply_actor!=0)==allowed,"feeder perpendicular distance is strict less-than128");need(out.buttons==0&&out.y==0,"feeder vertical test does not create edible controls");if(!allowed)need(out.x==0,"out-of-band feeder is ignored entirely");
 }
 for(unsigned fault=0;fault<6;++fault){Fixture f;f.feeder(1);auto& s=f.supplies[0];if(fault==0)s.feeder_kind=0x46DB;if(fault==1)s.feeder_axis=1;if(fault==2)s.kind=0x4528;if(fault==3)s.active=false;if(fault==4)s.actor=0;if(fault==5)s.x=std::numeric_limits<float>::quiet_NaN();need(!d::supply_valid(s),"wrong feeder kind/axis/identity/activity/position rejected");const auto out=f.at(127);need(!out.buttons&&!out.x&&!out.y&&!f.controller.snapshot().supply_actor,"invalid feeder cannot steer or supply anything");}
 for(uint16_t kind:{uint16_t(0x4526),uint16_t(0x46DA)})for(uint16_t flags:{uint16_t(0),uint16_t(0xC0),uint16_t(1),uint16_t(0xC1)}){
  Fixture f;f.bird();f.supplies[0].kind=kind;f.supplies[0].food_flags=flags;const bool valid=(flags&1)!=0;need(d::supply_valid(f.supplies[0])==valid,"bird must retain real egg-yield definition bit");const auto out=f.at(120);need(valid?out.buttons==d::jump_button:out.buttons==0,"only an egg-producing bird starts ammunition acquisition");
 }
 {Fixture f;f.feeder(1);f.supplies[1]={0x80142000,0x4526,200,350,true,false,0xC1};f.at(120);need(f.controller.snapshot().supply_actor==f.supplies[1].actor,"reachable real bird takes priority over invisible feeder waiting");}
 {Fixture f;f.supplies[0]={0x80142000,0x41DD,150,400,true,false,0xC1};need(!d::supply_valid(f.supplies[0]),"41DD cannot become egg supply even if unrelated flags are forged in pure fixture");const auto out=f.at(120);need(!out.buttons&&!out.x&&!out.y&&!f.controller.snapshot().supply_actor,"non-egg41DD is ignored by supply controls");}
 for(bool grounded:{false,true}){
  Fixture f;f.island();f.in.x=400;f.in.grounded=grounded;f.in.mouth_full=true;auto out=f.at(120);need(out.x<0&&out.y==0&&!out.buttons,"mouth-full island correction precedes Down while outside centre band");
  f.in.x=384;f.in.grounded=false;out=f.at(121);need(out.x==0&&out.y==0&&!out.buttons,"airborne mouth-full at island centre never sends Down");
  f.in.grounded=true;out=f.at(122);need(out.x==0&&out.y<0&&!out.buttons,"grounded mouth-full at centre sends ordinary Down");
 }
 for(float x:{100.f,616.f}){
  Fixture f;f.island();f.bird(x,148);auto out=f.at(120);need(out.x==0&&!out.buttons,"out-of-reach fish-arena bird cannot pull Yoshi off current island");
  f.in.x=400;out=f.at(121);need(out.x<0&&!out.buttons,"far bird still allows correction toward current centre");
 }
 {Fixture f;f.island();f.supplies[0]={0x80140000,0x4002,616,248,true,true};auto out=f.at(120);need(out.x>0&&!out.buttons,"loose egg destination clamps to near edge of current safe band");f.in.x=396;out=f.at(121);need(out.x==0&&!out.buttons,"loose egg beyond island cannot move beyond centre plus12");f.supplies[0].x=100;f.in.x=372;out=f.at(122);need(out.x==0&&!out.buttons,"opposite loose egg cannot move beyond centre minus12");}
 for(d::ThrowMode mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress}){
  Fixture f;f.island();f.in.reserves=2;f.in.throw_mode=mode;f.at(1);need(f.controller.snapshot().phase==d::Phase::Face,"safe island begins ordinary facing phase");
  f.in.x=400;auto out=f.at(2);need(out.x<0&&!out.buttons&&out.y==0&&f.controller.snapshot().phase==d::Phase::Face,"unsafe facing frame recentres before sending egg controls");
  f.in.x=384;f.in.grounded=false;out=f.at(3);need(!out.buttons&&!out.x&&!out.y,"airborne pre-aim facing waits without walking or Z");
  f.in.grounded=true;for(unsigned vi=4;vi<=7;++vi)out=f.at(vi);need(f.controller.snapshot().phase==d::Phase::AcquireAim&&out.buttons==d::egg_button,"safe grounded facing resumes genuine first Z");
  f.in.x=400;out=f.at(8);need(out.x<0&&!out.buttons&&out.y==0&&f.controller.snapshot().phase==d::Phase::AcquireAim,"unsafe AcquireAim before4005 recentres without native Z or aiming movement");
  f.in.aiming_actor=0x80150000;f.in.reticle_actor=0x80151000;f.in.reticle_x=0;f.in.reticle_y=0;f.in.grounded=false;out=f.at(9);const float dx=f.targets[0].x-f.in.x,dy=f.targets[0].y-f.in.y,length=std::hypot(dx,dy);
  need(f.controller.snapshot().phase==d::Phase::TrackAim&&near(out.x,.85f*dx/length)&&near(out.y,-.85f*dy/length),"real4005 keeps unchanged native aiming axes even outside island band");need(out.buttons==(mode==d::ThrowMode::HoldAndRelease?d::egg_button:0),"real4005 keeps configured genuine hold/tap Z mode");
 }
 std::printf("BossDriverSupply0260Checks: %u PASS (synthetic diagnostic controller fixtures; no RAM/native/game/save/ledger or victory proof)\n",checks);
}
