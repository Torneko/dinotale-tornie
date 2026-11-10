#include "../boss_driver0260.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
namespace d=workshop::boss_driver0260;
namespace s=workshop::boss_stick_aim0260;
static unsigned checks=0;
static void need(bool ok,const char* why){++checks;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
static bool neutral(const d::Output& out){return out.handled&&!out.buttons&&!out.x&&!out.y;}
struct Fixture {
 d::Controller controller;d::Input in;std::array<d::Target,1> targets{};std::array<d::Supply,1> supplies{};std::array<float,80> xs{},ys{};
 Fixture(uint16_t kind,bool right){
  need(controller.configure(true,true,{},false),"Synthetic diagnostic controller only");targets[0]={0x80120000,kind,500,248,0,0,true,true,false,kind==0x455A,true,6.6f,2};supplies[0]={0x80130000,0x4526,right?434.f:334.f,178,true,false,0xC1};
  in.player=0x80100000;in.room=170;in.encounter=3;in.combat=in.controls=in.alive=true;in.primary_state=1;in.x=384;in.y=248;in.min_x=40;in.max_x=724;in.targets=targets;in.supplies=supplies;in.native_stick_observed=in.native_tongue_observed=in.native_tongue_known=in.native_tongue_ready=true;xs.fill(14);ys.fill(-18);in.native_mouth={true,right,0,1,1,xs,ys};in.native_tongue_max_reach=*workshop::boss_tongue_geometry0260::max_centre_reach(1.f,.001f,false);
 }
 d::Output at(unsigned vi){in.vi=vi;return controller.step(in);}
 void sample(const d::Output& out){in.native_stick=s::native_axes(s::convert({out.x,out.y}),in.native_stick);}
 void begin(){in.grounded=true;sample(at(0));in.grounded=false;for(unsigned vi=1;vi<12;++vi)sample(at(vi));const auto out=at(12);need(out.buttons==d::tongue_button,"Observed ready initial tongue receives a genuine input request");sample(out);}
};
int main(){
 const std::array<uint16_t,7> kinds{0x4599,0x4072,0x40A3,0x4511,0x4631,0x455A,0x41C6};
 for(uint16_t kind:kinds)for(bool right:{false,true})for(unsigned stale:{1u,2u}){
  Fixture f(kind,right);f.begin();f.in.native_tongue_live=true;f.in.native_tongue_ready=false;f.in.native_tongue_phase=2;
  auto out=f.at(13);need(out.buttons==d::tongue_button&&!out.x&&!out.y,"A proven live extension holds B without steering when EA4 is not observed active");f.sample(out);
  // Observations describe the known native3D76C idle cancellation: raw159
  // remains1/2, but neither actor updates the tongue any more. No RAM write.
  f.in.native_tongue_live=false;f.in.native_tongue_cancelled_idle=true;f.in.native_tongue_ready=true;f.in.native_tongue_phase=stale;
  out=f.at(14);need(neutral(out),"Known native cancellation first releases a previously held B");f.sample(out);
  need(f.in.native_tongue_phase==stale,"Controller never clears or fabricates the observed raw159");
  out=f.at(15);need(out.buttons==d::tongue_button,"Next observed-ready request can create a fresh B edge despite known stale159");need(f.in.native_tongue_phase==stale,"Fresh request leaves raw native state untouched");need(f.controller.snapshot().launches==0,"Controller cannot fabricate projectile or victory observations");
 }
 for(unsigned raw:{0u,1u,2u,3u,255u})for(bool route:{false,true}){
  Fixture f(0x4072,false);f.in.native_tongue_known=false;f.in.native_tongue_ready=false;f.in.native_tongue_phase=raw;f.in.native_tongue_live=false;f.in.native_tongue_cancelled_idle=false;f.in.has_supply_stand=route;f.in.supply_stand_x=392;f.in.supply_stand_y=326;
  for(unsigned vi:{0u,2u,12u,40u,120u})need(neutral(f.at(vi)),"Unknown graph is neutral even for raw0 and cannot enter unsafe route or held-B loop");
 }
 for(unsigned raw:{1u,3u}){
  Fixture f(0x40A3,true);f.begin();f.in.native_tongue_live=true;f.in.native_tongue_ready=false;f.in.native_tongue_phase=raw;need(neutral(f.at(13)),"Live native retraction/post-phase releases B; inactive EA4 requires no heading steering");
 }
 {Fixture f(0x4072,false);f.begin();f.in.native_tongue_phase=2;f.in.native_tongue_cancelled_idle=true;f.in.native_tongue_ready=false;need(neutral(f.at(13)),"A cancelled request releases old B even while cooldown rejects the next one");for(unsigned vi:{14u,15u,36u})need(!(f.at(vi).buttons&d::tongue_button),"Known cancellation does not bypass actual native ordinary-tongue readiness");}
 for(auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress})for(bool known:{false,true}){
  Fixture f(0x4511,true);f.in.native_tongue_known=known;f.in.native_tongue_live=false;f.in.native_tongue_cancelled_idle=true;f.in.native_tongue_phase=2;f.in.throw_mode=mode;f.in.aiming_actor=0x80200000;f.in.reticle_actor=0x80201000;f.in.reticle_x=0;f.in.reticle_y=0;
  const auto out=f.at(12);need(f.controller.snapshot().phase==d::Phase::TrackAim&&out.buttons==(mode==d::ThrowMode::HoldAndRelease?d::egg_button:0),"Real4005 remains prioritary over cancelled or unknown tongue observations");
 }
 {Fixture f(0x4072,false);f.in.native_tongue_known=false;f.in.native_tongue_phase=2;f.in.native_tongue_ready=false;f.in.mouth_full=true;f.in.grounded=true;auto out=f.at(12);need(out.y<0&&!out.buttons,"A genuine held actor keeps grounded swallowing priority even if the tongue graph is unknown");f.in.grounded=false;out=f.at(13);need(!out.y&&!out.buttons,"Airborne held actor is never forced to swallow");}
 for(unsigned reset=0;reset<4;++reset){Fixture f(0x4072,true);f.begin();f.in.native_tongue_phase=2;f.in.native_tongue_cancelled_idle=true;f.in.native_tongue_live=false;f.in.native_tongue_ready=true;unsigned vi=14;if(reset==0)f.in.player+=0x280;if(reset==1)f.in.room++;if(reset==2)f.in.encounter++;if(reset==3)vi=1;f.in.grounded=true;const auto out=f.at(vi);need(out.buttons==d::jump_button&&f.controller.snapshot().jump_vi==vi,"New player/room/encounter/clock starts a fresh supply epoch without inheriting a cancellation B hold");}
 std::printf("BossSupplyCancellation0260Checks: %u PASS (synthetic controller observations only; raw159 immutable; no native/game/GPU/save/ledger/victory proof)\n",checks);
}
