#include "../boss_driver0260.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
namespace d=workshop::boss_driver0260;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
static bool same(const d::Output&a,const d::Output&b){return a.handled==b.handled&&a.buttons==b.buttons&&a.x==b.x&&a.y==b.y;}
static void travel_only(const d::Output& out){need(out.handled&&!(out.buttons&(d::tongue_button|d::egg_button)),"Route emits only ordinary movement/jump, never B or Z");need(std::abs(out.x)<=1&&std::abs(out.y)<=1,"Route axes stay bounded");}
struct Fixture{
 d::Controller controller;std::array<d::Target,1> targets{};std::array<d::Supply,1> supplies{};d::Input in;
 explicit Fixture(unsigned room=151,d::ThrowMode mode=d::ThrowMode::HoldAndRelease){
  need(controller.configure(true,true,{},false),"Explicit isolated diagnostic controller");
  targets[0]={0x80110000,0x41C6,500,300,0,0,true,true,false};
  supplies[0]={0x80140000,0x4526,392,230,true,false,0xC1};
  in.vi=100;in.room=room;in.encounter=4;in.player=0x80100000;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.reserves=0;
  in.x=312;in.y=390;in.min_x=232;in.max_x=552;in.targets=targets;in.supplies=supplies;in.throw_mode=mode;
  in.has_preferred_x=true;in.preferred_x=312;in.has_supply_stand=true;in.supply_stand_x=392;in.supply_stand_y=326;
 }
 d::Output at(unsigned vi){in.vi=vi;return controller.step(in);}
 void upper(bool grounded=true){in.x=392;in.y=326;in.grounded=grounded;}
};
int main(){
 // Native observations are supplied explicitly; the test never simulates
 // physical movement or marks a requested A edge as successful arrival.
 for(unsigned room:{151u,170u,176u}){
  Fixture f(room);auto out=f.at(100);travel_only(out);need(f.controller.snapshot().phase==d::Phase::SupplyTravel&&!f.controller.snapshot().supply_ready,"Zero reserve on lower312/390 starts upper-platform route");need(!out.buttons&&!out.x&&!out.y,"Route releases previous controls before first jump edge");
  out=f.at(101);travel_only(out);need(!out.buttons,"Route keeps a real release interval");
  out=f.at(102);travel_only(out);need(out.buttons==d::jump_button&&out.x>0,"Lower312/390 approaches real392platform with A and right movement");
  const auto before=f.controller.snapshot();const auto repeated=f.at(102);need(same(out,repeated)&&f.controller.snapshot().phase_vi==before.phase_vi&&!f.controller.snapshot().supply_ready,"Same VI is idempotent, never invents platform arrival");
  f.in.x=350;f.in.y=355;f.in.grounded=false;out=f.at(110);travel_only(out);need(out.x>0&&!f.controller.snapshot().supply_ready,"Airborne transit keeps moving toward392 without B");
  f.upper(false);out=f.at(120);travel_only(out);need(f.controller.snapshot().phase==d::Phase::SupplyTravel&&!f.controller.snapshot().supply_ready,"Coordinates326 alone cannot certify platform without native grounded observation");
  f.in.grounded=true;out=f.at(121);need(f.controller.snapshot().supply_ready&&f.controller.snapshot().phase==d::Phase::Supply,"Native grounded392/326 admits the bird-acquisition controller");need(out.buttons==d::jump_button&&!(out.buttons&d::tongue_button),"Arrival begins genuine bird jump, never simultaneous A/B");need(f.controller.snapshot().supply_actor==f.supplies[0].actor,"Only real eligible4526 becomes tongue target");
  f.in.grounded=false;f.in.y=280;out=f.at(132);need(!(out.buttons&d::tongue_button),"Tongue waits at least12 elapsed airborne VI after real arrival");out=f.at(133);need(out.buttons==d::tongue_button&&!(out.buttons&d::jump_button),"Reachable real bird receives B only after jump edge has time to act");
 }
 // Deep-floor entry must regain an existing lower platform before asking for
 // the upper one; directly jumping139px from465 is not a valid64px step.
 for(unsigned side=0;side<2;side++){
  Fixture f;f.in.x=side?440.f:350.f;f.in.y=465;f.in.preferred_x=side?472.f:312.f;const float launch=side?512.f:272.f;
  auto out=f.at(100);travel_only(out);need(!out.buttons&&(side?out.x>0:out.x<0),"Deep-floor recovery first walks sideways to272/512, without jumping under the lower brick");
  out=f.at(102);travel_only(out);need(!out.buttons&&(side?out.x>0:out.x<0),"Elapsed time alone cannot certify reaching the grounded launch point");
  f.in.x=launch;out=f.at(103);travel_only(out);need(!out.buttons&&!out.x,"Observed grounded launch starts a fresh release interval");
  out=f.at(104);travel_only(out);need(!out.buttons&&!out.x,"Second launch-release frame remains neutral");
  out=f.at(105);travel_only(out);need(out.buttons==d::jump_button&&!out.x,"Native jump edge begins alone at safe side launch");
  f.in.grounded=false;f.in.y=440;out=f.at(116);travel_only(out);need(out.buttons==d::jump_button&&!out.x,"First13 airborne VI rise beside the brick rather than steering through it");
  out=f.at(117);travel_only(out);need(out.buttons==d::jump_button&&(side?out.x<0:out.x>0),"After14VI airborne recovery steers to lower312/472, not upper392");
  f.in.x=f.in.preferred_x;f.in.y=390;out=f.at(120);travel_only(out);need(!out.x&&!f.controller.snapshot().supply_lower_ready&&!f.controller.snapshot().supply_ready,"Matching lower coordinates while airborne does not certify a platform landing");
  f.in.grounded=true;out=f.at(121);travel_only(out);need(!out.buttons&&!out.x&&f.controller.snapshot().supply_lower_ready&&!f.controller.snapshot().supply_ready,"Real grounded312/472 at390 starts a separate upper-jump release interval");
  out=f.at(122);travel_only(out);need(!out.buttons&&!out.x,"Lower landing keeps second release frame before next A edge");
  out=f.at(123);travel_only(out);need(out.buttons==d::jump_button&&(side?out.x<0:out.x>0),"Only after actual lower landing and release does A steer toward392");
  need(!f.controller.snapshot().supply_ready,"Sending upper-jump controls cannot fabricate326standing");
 }
 for(float y:{390.f,345.f,339.f}){Fixture f;f.in.x=392;f.in.y=y;f.at(100);need(f.controller.snapshot().phase==d::Phase::SupplyTravel&&!f.controller.snapshot().supply_ready,"Grounded atwrong height cannot certify upper platform");}
 for(float x:{370.f,413.f}){Fixture f;f.in.x=x;f.in.y=326;f.at(100);need(!f.controller.snapshot().supply_ready,"Grounded outside observed upper platform band stays in travel");}
 // A genuine4005 aim/throw wins over ammunition routing even at zero reserve.
 for(auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress}){
  Fixture f(151,mode);f.in.aiming_actor=0x80150000;f.in.reticle_actor=0x80151000;f.in.reticle_x=500;f.in.reticle_y=300;f.in.grounded=false;
  const auto out=f.at(100);need(f.controller.snapshot().phase==d::Phase::TrackAim&&!f.controller.snapshot().supply_ready,"Actual4005 keeps native aiming priority over new upper-platform route");need(!(out.buttons&(d::jump_button|d::tongue_button)),"Native aiming cannot become route jump/tongue input");need(out.buttons==(mode==d::ThrowMode::HoldAndRelease?d::egg_button:0),"Real4005 retains configured hold/tap Z mode");need(out.x>0&&out.y>0,"Native aim keeps target vector instead of route steering");need(f.controller.snapshot().launches==0,"Controller never fabricates an actual projectile");
 }
 {Fixture f;f.in.reserves=1;const auto out=f.at(100);need(f.controller.snapshot().phase!=d::Phase::SupplyTravel&&!f.controller.snapshot().supply_ready,"Existing reserve bypasses optional zero-ammo route");need(!(out.buttons&d::tongue_button),"Available ammunition never starts bird tongue acquisition");}
 // Filled mouth is handled by the ordinary safe grounded Down action once
 // the real upper support exists; airborne frames never force swallowing.
 {Fixture f;f.upper();f.in.mouth_full=true;auto out=f.at(100);need(f.controller.snapshot().supply_ready&&out.y<0&&!out.buttons&&!out.x,"Grounded upper-platform mouth-full converts food with Down alone");f.in.grounded=false;out=f.at(101);need(out.y==0&&!out.buttons,"Airborne mouth-full never sends Down or B");}
 {Fixture f;f.in.mouth_full=true;f.at(100);const auto out=f.at(102);travel_only(out);need(out.y==0&&!f.controller.snapshot().supply_ready,"Unreached-platform mouth-full cannot invent arrival or tongue target");}
 // Leaving the platform after acquisition invalidates only the observed route
 // state. Route travel restarts from ordinary input, not an actor teleport.
 {Fixture f;f.upper();f.at(100);need(f.controller.snapshot().supply_ready,"Upper fixture acquired");f.in.x=312;f.in.y=390;auto out=f.at(101);travel_only(out);need(!f.controller.snapshot().supply_ready&&f.controller.snapshot().phase==d::Phase::SupplyTravel,"Falling back to lower support clears stale ready marker");}
 for(unsigned reset=0;reset<4;reset++){
  Fixture f;f.upper();f.at(100);f.in.x=312;f.in.y=390;unsigned vi=101;
  if(reset==0)f.in.room++;if(reset==1)f.in.encounter++;if(reset==2)f.in.player+=0x280;if(reset==3)vi=2;
  const auto out=f.at(vi);travel_only(out);need(!out.buttons&&!f.controller.snapshot().supply_ready&&f.controller.snapshot().phase_vi==vi,"New room/encounter/player/VI rollback starts fresh route release interval");need(f.controller.snapshot().room==f.in.room&&f.controller.snapshot().encounter==f.in.encounter&&f.controller.snapshot().player==f.in.player,"Route provenance follows exact observed native identity");
 }
 // Caller opt-in is required; original families and existing supply behavior
 // receive no forced new waypoint when has_supply_stand is absent.
 {Fixture f;f.in.has_supply_stand=false;f.at(100);need(f.controller.snapshot().phase==d::Phase::Supply&&!f.controller.snapshot().supply_ready,"No observed supply waypoint retains original bird-acquisition behavior");}
 for(unsigned invalid=0;invalid<3;invalid++){Fixture f;if(invalid==0)f.in.supply_stand_x=std::numeric_limits<float>::quiet_NaN();if(invalid==1)f.in.supply_stand_y=std::numeric_limits<float>::infinity();if(invalid==2)f.in.supply_stand_x=600;const auto out=f.at(100);need(out.handled&&!out.buttons&&!out.x&&!out.y,"Invalid/outside native waypoint produces only neutral controller output");}
 for(unsigned guard=0;guard<3;guard++){Fixture f;if(guard==0)f.in.combat=false;if(guard==1)f.in.controls=false;if(guard==2)f.in.alive=false;const auto out=f.at(100);need(out.handled&&!out.buttons&&!out.x&&!out.y&&!f.controller.snapshot().supply_ready,"Inactive native controls cannot travel or invent a waypoint landing");}
 {Fixture f;need(!f.controller.configure(true,false,{},false),"Production session cannot enable QA route");uint16_t buttons=0x1234;float x=.2f,y=-.3f;need(!f.controller.drive(f.in,&buttons,&x,&y)&&buttons==0x1234&&x==.2f&&y==-.3f,"Non-diagnostic route leaves user inputs untouched");}
 std::printf("BossSupplyRoute0260Checks: %u PASS (pure read-only-observation controller fixtures; no RAM/physics/capture/native victory proof)\n",checks);return 0;
}
