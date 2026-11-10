#include "../boss_driver0260.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
namespace d=workshop::boss_driver0260;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
static bool same(const d::Output& a,const d::Output& b){return a.handled==b.handled&&a.buttons==b.buttons&&a.x==b.x&&a.y==b.y;}
struct Fixture {
 d::Controller controller;std::array<d::Target,2> targets{};std::array<d::Supply,1> supplies{};d::Input in;
 Fixture(){
  need(controller.configure(true,true,{},false),"Explicitly synthetic diagnostic controller");
  targets[0]={0x80110000,0x41C6,450,360,0,0,true,true,false,false,true,6.6f,2};
  targets[1]={0x80111000,0x41C6,500,350,0,0,true,true,false,false,false,6.6f,2};
  supplies[0]={0x80140000,0x4526,362,290,true,false,0xC1};
  in.player=0x80100000;in.room=169;in.encounter=2;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.reserves=0;in.x=312;in.y=390;in.min_x=232;in.max_x=552;in.has_preferred_x=true;in.preferred_x=312;in.targets=targets;in.supplies=supplies;
 }
 void alternate(unsigned dt){
  targets[0].vulnerable=dt%2==0;targets[1].vulnerable=dt%2!=0;
  targets[0].clear_path=dt%2!=0;targets[1].clear_path=dt%2==0;
  if(dt%7==5||dt%7==6)targets[0].vulnerable=targets[1].vulnerable=false;
 }
 d::Output at(unsigned vi){in.vi=vi;return controller.step(in);}
 void route(){in.has_supply_stand=true;in.supply_stand_x=392;in.supply_stand_y=326;supplies[0].x=392;supplies[0].y=230;}
};
int main(){
 // The fixture supplies observations only: it has no RAM, physics runner,
 // native dispatcher, save/ledger, or ability to produce a victory receipt.
 for(unsigned start:{120u,127u,240u}){
  Fixture f;f.alternate(0);f.at(start);const auto first=f.controller.snapshot();need(first.phase==d::Phase::Supply&&first.jump_vi==start,"A valid root starts one supply epoch");
  for(unsigned dt=1;dt<=130;++dt){f.alternate(dt);const auto out=f.at(start+dt);const auto snap=f.controller.snapshot();const unsigned age=dt%64;
   need(snap.phase==d::Phase::Supply&&snap.phase_vi==start&&snap.target==first.target,"Changing vulnerable/clear root never restarts existing supply phase");
   need(snap.jump_vi==start+(dt/64)*64&&snap.jump_active,"Only native jump retry64 resets its local A epoch");
   need(out.handled&&!(out.buttons&(d::tongue_button|d::egg_button)),"Grounded supply cannot tongue or aim an egg during target alternation");
   need(out.buttons==(age<45?d::jump_button:0),"Supply keeps A0..44, release45..63, and retry64 across all-invulnerable windows");
   const auto repeated=f.at(start+dt);need(same(out,repeated)&&f.controller.snapshot().jump_vi==snap.jump_vi&&f.controller.snapshot().phase_vi==snap.phase_vi,"Same-VI replay cannot advance phase or jump epoch");
  }
 }
 for(unsigned start:{120u,127u}){
  Fixture f;f.at(start);f.in.grounded=false;
  for(unsigned dt=1;dt<=40;++dt){f.alternate(dt);const auto out=f.at(start+dt);
   need(f.controller.snapshot().phase==d::Phase::Supply&&f.controller.snapshot().jump_vi==start,"Airborne supply retains the original epoch despite two-root flicker");
   const bool pulse=dt>=12&&(dt-12)%24<4;
   need(bool(out.buttons&d::tongue_button)==pulse,"B pulses only after12 observed elapsed airborne VI, without target-driven reset");
   if(dt<12)need(out.buttons==d::jump_button,"Initial twelve-VI jump period contains A only");
   if(pulse)need(!(out.buttons&(d::jump_button|d::egg_button)),"Reachable airborne bird pulse never mixes A or Z with B");
  }
 }
 for(unsigned start:{120u,127u}){
  Fixture f;f.route();const auto out=f.at(start);need(f.controller.snapshot().phase==d::Phase::SupplyTravel&&!out.buttons,"Mapped upper supply route begins with release");
  const auto first=f.controller.snapshot();
  for(unsigned dt=1;dt<=125;++dt){f.alternate(dt);const auto step=f.at(start+dt);const unsigned age=dt%120;
   need(f.controller.snapshot().phase==d::Phase::SupplyTravel&&f.controller.snapshot().phase_vi==first.phase_vi,"SupplyTravel epoch survives alternating/all-invulnerable roots");
   need(step.buttons==((age>=2&&age<47)?d::jump_button:0),"SupplyTravel release2/Athrough46/then release retains one route clock");
   need(!(step.buttons&(d::tongue_button|d::egg_button))&&!f.controller.snapshot().supply_ready,"Time and requested movement never invent native upper-platform arrival");
   need(same(step,f.at(start+dt)),"Same-VI route replay cannot consume release or jump interval");
  }
  f.in.x=392;f.in.y=326;f.in.grounded=false;f.alternate(126);f.at(start+126);need(!f.controller.snapshot().supply_ready,"Upper coordinates still require actual native grounded observation");
  f.in.grounded=true;f.alternate(127);f.targets[0].vulnerable=f.targets[1].vulnerable=false;const auto arrived=f.at(start+127);need(f.controller.snapshot().supply_ready&&f.controller.snapshot().phase==d::Phase::Supply&&f.controller.snapshot().jump_vi==start+127&&arrived.buttons==d::jump_button,"Actual grounded arrival starts a new bird jump epoch even if all roots are invulnerable");
 }
 for(auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress})for(bool travel:{false,true}){
  Fixture f;if(travel)f.route();f.in.throw_mode=mode;f.at(120);f.in.aiming_actor=0x80150000;f.in.reticle_actor=0x80151000;f.in.reticle_x=0;f.in.reticle_y=0;f.in.grounded=false;f.targets[0].vulnerable=true;f.targets[0].clear_path=true;f.targets[1].vulnerable=false;
  auto out=f.at(121);need(f.controller.snapshot().phase==d::Phase::TrackAim,"Real4005 preempts existing Supply/SupplyTravel at zero reserve");need(out.buttons==(mode==d::ThrowMode::HoldAndRelease?d::egg_button:0)&&!(out.buttons&(d::tongue_button|d::jump_button)),"Actual aiming preserves the selected hold/tap native Z protocol");
  f.targets[0].vulnerable=f.targets[1].vulnerable=false;out=f.at(122);need(f.controller.snapshot().phase==d::Phase::CancelAim&&out.buttons==(d::tongue_button|(mode==d::ThrowMode::HoldAndRelease?d::egg_button:0)),"All-invulnerable roots cancel real4005 instead of stealing it for resupply");
  f.targets[0].active=f.targets[1].active=false;out=f.at(123);need(f.controller.snapshot().phase==d::Phase::CancelAim&&bool(out.buttons&d::egg_button)==(mode==d::ThrowMode::HoldAndRelease),"Pending real cancel remains prior even when the last target vanishes");
  f.in.aiming_actor=0;out=f.at(124);need(f.controller.snapshot().phase==d::Phase::Cooldown&&!out.buttons&&!out.x&&!out.y,"Cancellation ends only after actual4005 disappears");need(!f.controller.snapshot().launches,"Diagnostic output cannot fabricate a projectile observation");
 }
 for(bool travel:{false,true}){
  Fixture f;if(travel)f.route();f.at(120);f.targets[0].vulnerable=false;f.targets[1].vulnerable=true;f.targets[1].clear_path=true;f.in.reserves=1;auto out=f.at(121);
  need(f.controller.snapshot().phase!=d::Phase::Supply&&f.controller.snapshot().phase!=d::Phase::SupplyTravel&&f.controller.snapshot().target==f.targets[1].actor,"One genuinely observed reserve resumes current vulnerable-root selection");need(!(out.buttons&d::tongue_button),"Available ammunition cannot keep the old supply B pulse alive");
 }
 for(unsigned reset=0;reset<4;++reset)for(bool travel:{false,true}){
  Fixture f;if(travel)f.route();f.at(120);f.alternate(4);f.at(124);f.targets[0].vulnerable=f.targets[0].clear_path=true;f.targets[1].vulnerable=false;unsigned vi=125;
  if(reset==0)f.in.player+=0x280;if(reset==1)f.in.room++;if(reset==2)f.in.encounter++;if(reset==3)vi=3;
  const auto out=f.at(vi);const auto snap=f.controller.snapshot();need(snap.phase_vi==vi&&snap.player==f.in.player&&snap.room==f.in.room&&snap.encounter==f.in.encounter,"Native player/room/encounter/VI rollback resets provenance and phase epoch");
  if(travel)need(snap.phase==d::Phase::SupplyTravel&&!out.buttons&&!snap.supply_ready,"Fresh route identity requires a new release interval and native landing");else need(snap.phase==d::Phase::Supply&&snap.jump_vi==vi&&out.buttons==d::jump_button,"Fresh supply identity starts new A epoch and no inherited B pulse");
 }
 for(unsigned fault=0;fault<6;++fault){Fixture f;f.at(120);for(auto& target:f.targets){if(fault==0)target.actor=0;if(fault==1)target.active=false;if(fault==2)target.defeated=true;if(fault==3)target.kind=0x4002;if(fault==4)target.x=std::numeric_limits<float>::quiet_NaN();if(fault==5)target.vx=std::numeric_limits<float>::infinity();}const auto out=f.at(121);need(f.controller.snapshot().phase==d::Phase::Idle&&!f.controller.snapshot().target&&!f.controller.snapshot().jump_active&&!out.buttons&&!out.x&&!out.y,"No remaining valid owned target ends supply with neutral Idle output");}
 {Fixture f;f.at(120);const auto before=f.at(121);const auto snap=f.controller.snapshot();f.in.reserves=1;f.in.aiming_actor=0x80150000;f.targets[0].active=f.targets[1].active=false;const auto repeated=f.at(121);need(same(before,repeated)&&f.controller.snapshot().phase==snap.phase&&f.controller.snapshot().jump_vi==snap.jump_vi,"Conflicting observations at one VI cannot advance an already sampled controller epoch");}
 std::printf("BossSupplyEpoch0260Checks: %u PASS (synthetic diagnostic observations only; no RAM/native physics/game/GPU/ledger/victory proof)\n",checks);
}
