// Native services are explicit HOST boundaries; their broad regression main
// is compiled but never executed by this focused input-controller suite.
#define main checkpoint_runtime_entry_not_executed
#include "BossCheckpointRuntime0260Checks.cpp"
#undef main
#include "../boss_driver_bridge0260.hpp"
#include <fstream>
#include <iterator>
namespace bridge=workshop::boss_driver_bridge0260;
namespace d=workshop::boss_driver0260;namespace stick=workshop::boss_stick_aim0260;
static std::vector<uint8_t> load(const char* path){std::ifstream f(path,std::ios::binary);std::vector<uint8_t> ram((std::istreambuf_iterator<char>(f)),{});need(ram.size()==0x800000,"Actual diagnostic RAM has8MiB");return ram;}
static bool neutral(const d::Output& o){return o.handled&&!o.buttons&&!o.x&&!o.y;}
static bool equal(const d::Output&a,const d::Output&b){return a.handled==b.handled&&a.buttons==b.buttons&&a.x==b.x&&a.y==b.y;}
static int native_x(const d::Output& out,stick::Axes old){const auto raw=stick::convert({out.x,out.y});return stick::force_x(raw)?(raw.x>=0?54:-54):stick::filter(raw.x,old.x);}
struct Control {
 d::Controller controller;d::Input in;std::array<d::Target,1> targets{};std::array<float,3> centers{152,360,600};
 explicit Control(uint32_t player){need(controller.configure(true,true,{},false),"Explicit isolated diagnostic controller");
  in.player=player;in.room=171;in.encounter=4;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.reserves=5;in.x=360;in.y=273;in.min_x=40;in.max_x=724;in.stable_stands=centers;
  targets[0]={0x800C9CC0,0x4631,455.2998f,338,0,0,true,true,false};in.targets=targets;
  in.native_stick_observed=in.native_tongue_observed=in.native_tongue_known=in.native_tongue_ready=true;
  in.fish_navigation=in.fish_island_observed=in.fish_on_island=in.horizontal_speed_observed=true;in.fish_anchor_x=360;
 }
 d::Output at(unsigned vi){in.vi=vi;return controller.step(in);}
 unsigned cooldown(){
  need(neutral(at(2409))&&controller.snapshot().fish_settle_count==1,"First grounded/stopped VI attests the anchor but does not start aiming");
  at(2410);need(controller.snapshot().fish_anchor_attested,"Second grounded/stopped observation retains the source anchor");
  unsigned vi=2411;for(;vi<2430&&controller.snapshot().phase!=d::Phase::AcquireAim;++vi)at(vi);
  need(controller.snapshot().phase==d::Phase::AcquireAim,"Ordinary native Z acquisition still follows settling");
  in.primary_state=3;in.aiming_actor=0x800C6AB4;in.reticle_actor=0x800C3D6C;in.reticle_x=targets[0].x;in.reticle_y=targets[0].y;
  for(;vi<2440&&controller.snapshot().phase!=d::Phase::Release;++vi)at(vi);
  need(controller.snapshot().phase==d::Phase::Release,"Explicit HOST actual-reticle observations reach the ordinary release phase");
  in.primary_state=4;in.aiming_actor=0;in.reticle_actor=0;for(;vi<2450&&controller.snapshot().phase!=d::Phase::Cooldown;++vi)at(vi);
  need(controller.snapshot().phase==d::Phase::Cooldown,"Native-shaped primary4 has priority while the existing cooldown begins");return controller.snapshot().phase_vi;
 }
};
#ifndef FISH_RECOVERY_FIXTURE_ONLY
int main(int argc,char** argv){try{
 checks=0;need(argc==2,"Supply the real ce3 unfenced diagnostic capture");auto captured=load(argv[1]);auto* ram=captured.data();const auto player=n::word(ram,0x800F8828),p=n::word(ram,player+0x1F4);
 need(n::actor(ram,player,0x4001)&&n::byte(ram,p+0xF4)&&std::bit_cast<float>(n::word(ram,player+0xC4))==3,"Actual ce3 snapshot proves displaced airborne P1 withC4+3, not a hypothetical walk command");
 {Control f(player);const auto epoch=f.cooldown();f.in.primary_state=1;f.in.x=364.17844f;f.in.grounded=f.in.fish_on_island=false;f.in.horizontal_speed=3;
  auto out=f.at(epoch+10);need(native_x(out,f.in.native_stick)==-54&&(out.buttons&d::jump_button)&&!(out.buttons&(d::egg_button|d::tongue_button)),"The observed rightward drift gets strong NATIVE−54 countersteer plus ordinary bounded A during Cooldown");
  need(f.controller.snapshot().phase==d::Phase::Cooldown&&f.controller.snapshot().phase_vi==epoch,"Recovery never resets the existing native-shot cooldown epoch");
  f.in.horizontal_speed=-3;f.in.x=355;out=f.at(epoch+11);need(native_x(out,f.in.native_stick)==54,"The symmetric leftward drift gets strong NATIVE+54 countersteer");
  f.in.horizontal_speed=0;f.in.x=360;out=f.at(epoch+55);need(!(out.buttons&d::jump_button)&&f.controller.snapshot().phase_vi==epoch,"Airborne A is bounded to45VI without inventing a landing or restarting Cooldown");
  f.in.grounded=f.in.fish_on_island=true;need(neutral(f.at(epoch+56))&&f.controller.snapshot().fish_settle_count==1,"A later true ground/zero-speed observation only begins settling");
  f.at(epoch+56);need(f.controller.snapshot().fish_settle_count==1,"Repeated sameVI cannot fabricate the second stopped frame");
  out=f.at(epoch+57);need(!(out.buttons&d::egg_button)&&!f.controller.snapshot().fish_recovering,"Second distinct stopped/supported VI permits the existing controller to resume without a forced release");
 }
 for(unsigned fault=0;fault<7;++fault){Control f(player);const auto epoch=f.cooldown();f.in.primary_state=1;f.in.fish_island_observed=false;
  if(fault==0){f.in.aiming_actor=0x800C6AB4;f.in.reticle_actor=0x800C3D6C;f.in.primary_state=3;}if(fault==1)f.in.primary_state=4;if(fault==2)f.in.mouth_full=true;if(fault==3)f.in.native_tongue_known=false;if(fault==4)f.in.native_tongue_observed=false;if(fault==5)f.in.native_tongue_live=true;if(fault==6)f.in.native_tongue_callback_pending=true;
  auto legacy=f.controller;auto old=f.in;old.fish_navigation=false;old.vi=epoch+10;const auto expected=legacy.step(old);need(equal(f.at(epoch+10),expected),"Unknown support cannot steal native4005/primary4/held/live/unknown/pending-mouth priority");
 }
 for(unsigned fault=0;fault<6;++fault){Control f(player);const auto epoch=f.cooldown();f.in.primary_state=1;f.in.fish_island_observed=false;f.in.grounded=f.in.fish_on_island=false;f.in.horizontal_speed=3;
  if(fault==1){f.in.fish_island_observed=true;f.in.horizontal_speed_observed=false;}if(fault==2){f.in.fish_island_observed=true;f.in.horizontal_speed=std::numeric_limits<float>::quiet_NaN();}if(fault==3){f.in.fish_island_observed=true;f.in.fish_anchor_x=152;}if(fault==4){f.in.fish_island_observed=true;f.in.native_stick_observed=false;}if(fault==5){f.in.fish_island_observed=true;f.in.fish_anchor_x=900;}
  need(neutral(f.at(epoch+10))&&f.controller.snapshot().phase_vi==epoch,"Unproved/foreign island, velocity or native axes cannot authorize recovery or alter the cooldown timer");
 }
 {Control f(player);const auto epoch=f.cooldown();f.in.primary_state=3;f.in.aiming_actor=0x800C6AB4;f.in.reticle_actor=0x800C3D6C;f.in.fish_island_observed=false;
  // Both sides keep the already-proved Fish cursor feedback. Disabling the
  // entire Fish strategy would compare a different aiming controller after
  // A6A9; only vary the map witness whose priority this test actually checks.
  auto legacy=f.controller;auto old=f.in;old.fish_island_observed=true;
  for(unsigned age:{66u,67u}){old.vi=epoch+age;const auto expected=legacy.step(old),actual=f.at(epoch+age);need(equal(actual,expected),"A live4005 with temporarily unknown map keeps the real aim transition beyond the old cooldown");if(age==67)need((actual.buttons&d::egg_button)!=0,"The priority regression reaches a NONNEUTRAL native Z hold, so a premature fail-closed neutral return is detected");}
 }
 {Control f(player);const auto epoch=f.cooldown();f.in.primary_state=1;f.in.fish_navigation=false;f.in.horizontal_speed=3;f.in.grounded=false;need(neutral(f.at(epoch+10)),"Other-family/default-false control keeps its pre-existing cooldown command");}
 // The actual Bridge consumer reads the ce3 RAM, revalidates the memorized
 // source anchor and emits input. The preceding ground/aim observations are
 // explicit HOST boundaries, not native physics or successful shots.
 {Control f(player);const auto epoch=f.cooldown();need(r::configure("FishRecoveryComposite",1,true,true,4),"Explicit HOST diagnostic Runtime");r::rooms::configure(r::session.plan());r::enter_room(52);need(r::session.enter_fight(97,player),"Real Session owns the actual same-player Fish room");r::bound={0x800C9A44,0x800C9CC0,0,0};bridge::configure(false);bridge::controller=f.controller;
  const auto before=captured;uint16_t buttons=0;float x=0,y=0;bridge::drive(ram,2531,&buttons,&x,&y);need(captured==before,"Actual Bridge map/physics observation and recovery keep the real ce3 native8MiB unchanged");
  const d::Output out{true,buttons,x,y};need(native_x(out,{})==-54&&(buttons&d::jump_button)&&bridge::controller.snapshot().fish_recovering,"ACTUAL Bridge ce3 outside-island observation reaches the strong-counter recovery consumer");need(bridge::controller.snapshot().phase==d::Phase::Cooldown&&bridge::controller.snapshot().phase_vi==epoch,"Composed recovery preserves the real Controller cooldown timestamp");
 }
 std::cout<<"BossFishRecovery0260Checks: "<<checks<<" PASS; ordinary-input/real-Bridge composition, HOST pose/action boundaries explicit; no native landing/damage/victory claim.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
#endif
