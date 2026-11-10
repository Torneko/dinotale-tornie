#include "../boss_driver0260.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
namespace d=workshop::boss_driver0260;
namespace s=workshop::boss_stick_aim0260;
namespace g=workshop::boss_tongue_geometry0260;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
static bool neutral(const d::Output&o){return o.handled&&!o.buttons&&!o.x&&!o.y;}
static bool same(const d::Output&a,const d::Output&b){return a.handled==b.handled&&a.buttons==b.buttons&&a.x==b.x&&a.y==b.y;}
static void travel(const d::Output&o){need(o.handled&&!(o.buttons&(d::tongue_button|d::egg_button))&&!o.y,"Travel emits only ordinary A/horizontal input, never tongue/Z/Down");}
struct Fixture {
 d::Controller controller;d::Input in;std::array<d::Target,2> targets{};std::array<d::Supply,1> supplies{};std::array<float,80> xs{},ys{};
 Fixture(){
  need(controller.configure(true,true,{},false),"Explicit isolated diagnostic controller");
  targets[0]={0x80110000,0x41C6,450,360,0,0,true,true,false,false,true,6.6f,2};
  targets[1]={0x80111000,0x41C6,500,350,0,0,true,true,false,false,true,6.6f,2};
  in.vi=100;in.player=0x80100000;in.room=169;in.encounter=2;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;
  in.x=312;in.y=390;in.min_x=232;in.max_x=552;in.targets=targets;
  in.has_preferred_x=true;in.preferred_x=312;in.has_supply_stand=true;in.supply_stand_x=392;in.supply_stand_y=326;
  in.monster_navigation=in.combat_supported=in.horizontal_speed_observed=true;in.horizontal_speed=0;
  in.native_stick_observed=in.native_tongue_observed=in.native_tongue_known=in.native_tongue_ready=true;
  xs.fill(14);ys.fill(-18);in.native_mouth={true,true,0,.8f,.8f,xs,ys};
  in.native_tongue_max_reach=*g::max_centre_reach(.8f,.001f,false);in.native_tongue_heading_degrees=48;in.native_tongue_heading_active=true;
 }
 d::Output at(unsigned vi){in.vi=vi;return controller.step(in);}
 void upper(bool ground=true){in.x=392;in.y=326;in.grounded=ground;in.combat_supported=false;}
 void lower(bool supported=true){in.x=312;in.y=390;in.grounded=true;in.combat_supported=supported;}
 void alternate(unsigned vi){targets[0].vulnerable=vi%2;targets[1].vulnerable=!(vi%2);targets[0].clear_path=!(vi%2);targets[1].clear_path=vi%2;if(vi%5==0)targets[0].vulnerable=targets[1].vulnerable=false;}
 void idle(){in.mouth_full=false;in.native_tongue_phase=0;in.native_tongue_known=in.native_tongue_ready=true;in.native_tongue_live=in.native_tongue_cancelled_idle=in.native_tongue_callback_pending=false;}
 void live(unsigned phase){in.native_tongue_phase=phase;in.native_tongue_live=true;in.native_tongue_ready=false;}
 void begin_upper(unsigned vi=100){upper();in.reserves=0;at(vi);need(controller.snapshot().monster_refilling&&controller.snapshot().supply_ready,"A genuine observed zero reserve starts refill on an already supported upper fixture");}
 unsigned begin_return(unsigned vi=100){begin_upper(vi);in.reserves=6;need(neutral(at(vi+1))&&controller.snapshot().monster_settle_count==1,"First stopped upper observation only starts settling");need(neutral(at(vi+2))&&controller.snapshot().monster_returning&&!controller.snapshot().monster_refilling&&controller.snapshot().phase==d::Phase::ReturnCombat,"Only second consecutive stopped upper observation commits return phase");return vi+2;}
};
int main(){
 // These are synthetic read-only input observations. The suite has no RAM,
 // game dispatcher, physics, save, ledger, ingestion or victory mechanism.
 // Setting grounded/support in a fixture is explicit input, not a claim that
 // a requested A edge landed on a real native platform.
 for(unsigned start:{100u,127u,240u}){
  Fixture f;f.targets[0].vulnerable=f.targets[1].vulnerable=false;auto out=f.at(start);travel(out);
  need(f.controller.snapshot().monster_refilling&&f.controller.snapshot().phase==d::Phase::SupplyTravel&&neutral(out),"Zero reserve starts one refill route without requiring a vulnerable eye");
  const auto epoch=f.controller.snapshot().phase_vi;
  for(unsigned dt=1;dt<=125;++dt){f.alternate(dt);f.in.reserves=1+(dt%5);out=f.at(start+dt);travel(out);
   const auto snap=f.controller.snapshot();const unsigned age=dt%120;
   need(snap.monster_refilling&&!snap.monster_returning&&snap.phase==d::Phase::SupplyTravel&&snap.phase_vi==epoch,"Observed reserves1..5 and alternating/occluded eyes cannot break the refill epoch");
   need(out.buttons==((age>=2&&age<47)?d::jump_button:0),"Refill travel preserves release2/Athrough46/then-release timing");
   need(!snap.supply_ready&&snap.monster_settle_count==0,"Elapsed time and partial ammo never invent upper landing or settled stock");
   need(same(out,f.at(start+dt))&&f.controller.snapshot().phase_vi==epoch,"Same VI is idempotent even during target changes");
  }
  f.upper(false);f.in.reserves=5;out=f.at(start+126);need(!f.controller.snapshot().supply_ready&&f.controller.snapshot().monster_refilling&&neutral(out),"Upper coordinates while airborne cannot admit a supply landing");
  f.in.grounded=true;out=f.at(start+127);need(f.controller.snapshot().supply_ready&&f.controller.snapshot().monster_refilling&&f.controller.snapshot().phase==d::Phase::Supply&&neutral(out),"Only explicit native grounded observation admits upper supply without fabricating any egg");
  f.in.reserves=6;out=f.at(start+128);need(neutral(out)&&f.controller.snapshot().monster_settle_count==1&&!f.controller.snapshot().monster_returning,"Reserve6 does not immediately abandon the upper platform");
  out=f.at(start+129);need(neutral(out)&&f.controller.snapshot().phase==d::Phase::ReturnCombat&&f.controller.snapshot().monster_returning,"Two consecutive stopped full-stock observations start one return");
 }
 for(float stopped:{0.f,-0.f}){Fixture f;f.begin_upper();f.in.reserves=6;f.in.horizontal_speed=stopped;f.at(101);f.at(102);need(f.controller.snapshot().monster_returning,"Native-clamped positive or negative zero is stopped");}
 for(float moving:{.0001f,-.0001f,.05f,-.05f,.5f,-.5f,2.f}){
  Fixture f;f.begin_upper();f.in.reserves=6;f.in.horizontal_speed=moving;
  for(unsigned vi=101;vi<107;++vi){const auto out=f.at(vi);need(neutral(out)&&f.controller.snapshot().monster_refilling&&!f.controller.snapshot().monster_returning&&!f.controller.snapshot().monster_settle_count,"Any observed nonzero C4 prevents upper departure; position alone cannot prove stopped");}
  f.in.horizontal_speed=0;f.at(107);need(f.controller.snapshot().monster_settle_count==1,"First actual clamped-zero frame begins settling after drift");f.at(108);need(f.controller.snapshot().monster_returning,"Second consecutive zero frame permits the return");
 }
 for(unsigned mutation=0;mutation<5;++mutation){
  Fixture f;f.begin_upper();f.in.reserves=6;f.at(101);need(f.controller.snapshot().monster_settle_count==1,"Settle fixture starts with one observed stopped frame");
  if(mutation==0)f.in.grounded=false;if(mutation==1)f.in.x=396;if(mutation==2)f.in.y=339;if(mutation==3)f.in.y=301;if(mutation==4)f.in.horizontal_speed=.0001f;
  f.at(102);need(!f.controller.snapshot().monster_returning&&!f.controller.snapshot().monster_settle_count,"Airborne/outside-upper/drifting observation interrupts consecutive settling");
  f.upper();f.in.horizontal_speed=0;f.at(103);need(f.controller.snapshot().monster_settle_count==1&&!f.controller.snapshot().monster_returning,"Recovery requires a fresh first stopped frame");f.at(104);need(f.controller.snapshot().monster_returning,"Only next consecutive valid frame finishes recovery settling");
 }
 {Fixture f;f.begin_upper();f.in.reserves=6;f.at(101);f.at(103);need(f.controller.snapshot().monster_settle_count==1&&!f.controller.snapshot().monster_returning,"A skipped VI cannot be counted as two consecutive stopped observations");f.at(104);need(f.controller.snapshot().monster_returning,"Consecutive sampling resumes after the gap");}
 {Fixture f;f.begin_upper();f.in.reserves=6;const auto first=f.at(101);const auto state=f.controller.snapshot();f.in.horizontal_speed=3;f.in.grounded=false;const auto repeated=f.at(101);need(same(first,repeated)&&f.controller.snapshot().monster_settle_count==state.monster_settle_count&&!f.controller.snapshot().monster_returning,"Conflicting same-VI observations cannot increase or undo the cached settle count");}
 // Finishing a real tongue/held-food lifecycle has priority over full stock.
 for(unsigned mode=0;mode<6;++mode){
  Fixture f;f.begin_upper();f.in.reserves=6;
  if(mode==0)f.in.mouth_full=true;if(mode==1)f.live(2);if(mode==2)f.live(1);if(mode==3){f.in.native_tongue_phase=3;f.in.native_tongue_live=true;}if(mode==4)f.in.native_tongue_known=false;if(mode==5)f.in.native_tongue_callback_pending=true;
  for(unsigned vi=101;vi<105;++vi){const auto out=f.at(vi);need(f.controller.snapshot().monster_refilling&&!f.controller.snapshot().monster_returning&&!f.controller.snapshot().monster_settle_count,"Held/live/unknown/pending native tongue prevents stock completion");need(!(out.buttons&(d::jump_button|d::egg_button)),"Finishing tongue cannot start travel or Z");if(mode==0)need(out.y<0&&!out.buttons,"Actual held food retains grounded Down priority");if(mode==1)need(out.buttons==d::tongue_button,"Live extension2 maintains B rather than prematurely retracting");if(mode==2)need(!out.buttons,"Live retraction1 releases B while native heading may continue");if(mode>=3)need(neutral(out),"Unknown/callback/phase3 lifecycle remains neutral");}
  f.idle();f.at(105);need(f.controller.snapshot().monster_settle_count==1,"Recognized native idle allows first settled frame");f.at(106);need(f.controller.snapshot().monster_returning,"Recognized idle then second stopped frame allows return");
 }
 {Fixture f;f.begin_upper();f.in.reserves=6;f.live(2);need(f.at(101).buttons==d::tongue_button,"Cancellation fixture starts after a real observed live hold");f.in.native_tongue_live=false;f.in.native_tongue_cancelled_idle=true;f.in.native_tongue_ready=true;need(neutral(f.at(102))&&!f.controller.snapshot().monster_settle_count,"Known idle cancellation must first release previously emitted B");f.at(103);need(f.controller.snapshot().monster_settle_count==1,"Raw stale159 does not block recognized cancellation after release");f.at(104);need(f.controller.snapshot().monster_returning,"Known cancellation can then complete ordinary two-frame settling");}
 {Fixture f;f.begin_upper();f.in.reserves=6;f.in.mouth_full=true;f.in.grounded=false;const auto out=f.at(101);need(!out.buttons&&out.y==0&&!f.controller.snapshot().monster_returning,"Airborne held food never sends Down or certifies full-stock landing");}
 // Return is another input route, never a teleport or support fabrication.
 for(unsigned start:{100u,127u}){
  Fixture f;const unsigned route=f.begin_return(start);auto out=f.at(route+1);need(neutral(out),"Return keeps its second neutral VI before the new A edge");
  out=f.at(route+2);travel(out);need(out.buttons==d::jump_button&&out.x<0&&f.controller.snapshot().monster_returning,"Return begins ordinary A+left toward fixed312 after two neutral VI");
  f.in.x=350;f.in.y=355;f.in.grounded=false;out=f.at(route+10);travel(out);need(out.buttons==d::jump_button&&out.x<0&&f.controller.snapshot().monster_returning,"Observed airborne transit keeps steering to312 without claiming arrival");
  f.lower(false);out=f.at(route+11);need(f.controller.snapshot().monster_returning&&f.controller.snapshot().phase==d::Phase::ReturnCombat,"Matching lower coordinates and grounded alone cannot substitute for true native combat support");
  f.in.combat_supported=true;f.in.grounded=false;f.at(route+12);need(f.controller.snapshot().monster_returning,"Support flag without grounded cannot finish the return");
  f.in.grounded=true;f.in.horizontal_speed=.0001f;out=f.at(route+13);need(f.controller.snapshot().monster_returning&&!out.buttons,"Native lower support with residual C4 waits rather than entering aiming");
  f.in.x=318;f.in.horizontal_speed=0;out=f.at(route+14);need(f.controller.snapshot().monster_returning&&out.x<0&&!out.buttons,"Supported near312 still recenters inside the strict three-pixel band");
  f.in.x=312;out=f.at(route+15);need(neutral(out)&&!f.controller.snapshot().monster_returning&&f.controller.snapshot().phase==d::Phase::Approach,"Only actual grounded supported stopped312 returns to ordinary combat approach");
  need(f.controller.snapshot().launches==0&&!f.controller.snapshot().aim_seen,"Neither route can invent a projectile or a successful aim");
 }
 // A failed native return landed on the dangerous lower floor Y465. The two
 // previous successful returns did use multiple A epochs above real support,
 // so keep those upper attempts but never issue a new A on the lower floor.
 // This is only a safety guard; it does not prove the jump or any victory.
 {Fixture f;const unsigned route=f.begin_return();
  f.in.x=300;f.in.y=465;f.in.grounded=true;f.in.combat_supported=false;
  for(unsigned dt:{47u,119u,120u,121u,122u,123u,239u,240u,241u,242u}){
   const auto out=f.at(route+dt);need(!(out.buttons&d::jump_button)&&f.controller.snapshot().monster_returning&&f.controller.snapshot().phase==d::Phase::ReturnCombat,"Lower-floor fall cannot reissue A in any later return cycle");
  }
 }
 {Fixture f;const unsigned route=f.begin_return();
  f.in.x=320;f.in.y=325;f.in.grounded=false;f.in.combat_supported=false;
  for(unsigned dt:{2u,120u,121u,122u,123u,240u,241u,242u}){
   const auto out=f.at(route+dt);const bool expected=dt%120>=2;
   need(bool(out.buttons&d::jump_button)==expected&&f.controller.snapshot().monster_returning,"Native above-support multi-epoch jump must remain unchanged");
  }
 }
 {Fixture f;const unsigned route=f.begin_return();f.in.reserves=0;f.at(route+1);need(f.controller.snapshot().monster_refilling&&!f.controller.snapshot().monster_returning&&f.controller.snapshot().phase==d::Phase::Supply,"A genuinely observed empty reserve starts a new refill instead of continuing stale return");}
 // Native4005 and its native cancel protocol outrank every new route guard.
 for(auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress})for(bool returning:{false,true}){
  Fixture f;unsigned vi=101;if(returning)vi=f.begin_return()+1;else f.begin_upper();
  f.in.throw_mode=mode;f.in.aiming_actor=0x80150000;f.in.reticle_actor=0x80151000;f.in.reticle_x=0;f.in.reticle_y=0;
  f.in.has_supply_stand=false;f.in.has_preferred_x=false;f.in.horizontal_speed_observed=false;f.in.horizontal_speed=std::numeric_limits<float>::quiet_NaN();f.in.reserves=7;
  auto out=f.at(vi);need(f.controller.snapshot().phase==d::Phase::TrackAim&&out.buttons==(mode==d::ThrowMode::HoldAndRelease?d::egg_button:0),"Genuine4005 bypasses only new nav-only admission guards and preserves native hold/tap Z");need(!(out.buttons&(d::jump_button|d::tongue_button)),"Existing aiming cannot be stolen by full-stock route actions");
  f.targets[0].vulnerable=f.targets[1].vulnerable=false;out=f.at(vi+1);need(f.controller.snapshot().phase==d::Phase::CancelAim&&out.buttons==(d::tongue_button|(mode==d::ThrowMode::HoldAndRelease?d::egg_button:0)),"Actual4005 with no vulnerable eye follows native B-cancel/Z-mode rules");
 }
 for(unsigned reset=0;reset<4;++reset)for(bool returning:{false,true}){
  Fixture f;unsigned vi=101;if(returning)vi=f.begin_return()+1;else f.begin_upper();
  f.in.reserves=1;f.lower();if(reset==0)f.in.player+=0x280;if(reset==1)f.in.room++;if(reset==2)f.in.encounter++;if(reset==3)vi=2;
  f.at(vi);need(!f.controller.snapshot().monster_refilling&&!f.controller.snapshot().monster_returning&&!f.controller.snapshot().monster_settle_count,"Player/room/encounter/backward-VI reset clears old refill and return provenance");need(f.controller.snapshot().player==f.in.player&&f.controller.snapshot().room==f.in.room&&f.controller.snapshot().encounter==f.in.encounter,"Fresh route state follows exact observed identity");
  f.in.reserves=0;f.at(vi+1);need(f.controller.snapshot().monster_refilling&&!f.controller.snapshot().monster_returning,"Only a fresh zero-ammo observation restarts refill after provenance reset");
 }
 for(unsigned guard=0;guard<8;++guard){Fixture f;if(guard==0)f.in.horizontal_speed_observed=false;if(guard==1)f.in.horizontal_speed=std::numeric_limits<float>::quiet_NaN();if(guard==2)f.in.horizontal_speed=65;if(guard==3)f.in.has_supply_stand=false;if(guard==4)f.in.has_preferred_x=false;if(guard==5)f.in.preferred_x=472;if(guard==6)f.in.reserves=7;if(guard==7)f.in.horizontal_speed=std::numeric_limits<float>::infinity();const auto out=f.at(100);need(neutral(out)&&f.controller.snapshot().phase==d::Phase::Idle&&!f.controller.snapshot().monster_refilling,"Missing/invalid explicit Monster navigation evidence stays neutral");}
 for(unsigned invalid=0;invalid<5;++invalid){Fixture f;for(auto&t:f.targets){if(invalid==0)t.active=false;if(invalid==1)t.defeated=true;if(invalid==2)t.kind=0x40A3;if(invalid==3)t.actor=0;if(invalid==4)t.x=std::numeric_limits<float>::quiet_NaN();}need(neutral(f.at(100))&&f.controller.snapshot().phase==d::Phase::Idle&&!f.controller.snapshot().monster_refilling,"Explicit navigation requires a remaining valid owned41C6, not another family or invalid actor");}
 {Fixture f;f.in.monster_navigation=false;f.at(100);f.in.reserves=1;f.at(101);need(!f.controller.snapshot().monster_refilling&&!f.controller.snapshot().monster_returning&&f.controller.snapshot().phase!=d::Phase::SupplyTravel&&f.controller.snapshot().phase!=d::Phase::Supply,"Without explicit Monster navigation the original one-reserve combat flow remains unchanged");}
 for(uint16_t kind:std::array<uint16_t,6>{0x4599,0x4072,0x40A3,0x4511,0x4631,0x455A}){Fixture f;f.in.monster_navigation=false;f.in.has_supply_stand=false;for(auto&t:f.targets){t.kind=kind;t.red_candy=kind==0x455A;}f.at(100);f.in.reserves=1;f.at(101);need(!f.controller.snapshot().monster_refilling&&!f.controller.snapshot().monster_returning,"Other supported families do not acquire a six-stock Monster latch by default");}
 for(unsigned guard=0;guard<3;++guard){Fixture f;f.begin_upper();if(guard==0)f.in.controls=false;if(guard==1)f.in.alive=false;if(guard==2)f.in.combat=false;need(neutral(f.at(101)),"Inactive native controls/alive/combat always remain neutral");}
 {Fixture f;need(!f.controller.configure(true,false,{},false),"Production cannot enable this diagnostic input pilot");uint16_t buttons=0x1234;float x=.2f,y=-.3f;need(!f.controller.drive(f.in,&buttons,&x,&y)&&buttons==0x1234&&x==.2f&&y==-.3f,"Disabled production pilot preserves user controls");}
 std::printf("BossMonsterRestock0260Checks: %u PASS (synthetic diagnostic observations only; no RAM/native physics/ingestion/game/GPU/save/ledger/victory proof)\n",checks);
 return 0;
}
