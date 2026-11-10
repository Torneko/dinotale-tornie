#define FISH_RECOVERY_FIXTURE_ONLY
#include "BossFishRecovery0260Checks.cpp"
#undef FISH_RECOVERY_FIXTURE_ONLY
static unsigned native4(const d::Output& out,stick::Axes old){const auto raw=stick::convert({out.x,out.y});return unsigned(std::abs(stick::force_x(raw)?(raw.x>=0?54:-54):stick::filter(raw.x,old.x)));}
int main(int argc,char** argv){try{
 checks=0;need(argc==2,"Supply actual e9 diagnostic RAM for the unchanged Bridge source adapter");auto captured=load(argv[1]);auto* ram=captured.data();const auto player=n::word(ram,0x800F8828);
 for(auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress}){
  Control f(player);f.in.throw_mode=mode;f.at(2433);f.at(2434);f.at(2435);need(f.controller.snapshot().phase==d::Phase::Face,"Fixture reaches its true initial Face epoch");
  // HOST observations model the actual long recovery. They are not movement
  // writes, inferred landings or native input-admission receipts.
  f.in.grounded=f.in.fish_on_island=false;f.in.horizontal_speed=3;f.in.x=374;
  const auto counter=f.at(2436);need(native_x(counter,{})==-54&&f.controller.snapshot().fish_recovering,"Observed displacement interrupts Face and starts the existing native-strength counter");
  f.at(2636);f.in.grounded=f.in.fish_on_island=true;f.in.horizontal_speed=0;f.in.x=360;
  need(neutral(f.at(2637))&&f.controller.snapshot().fish_settle_count==1,"The actual first stopped ground observation stays neutral");
  const auto face=f.at(2638);need(!(face.buttons&d::egg_button)&&f.controller.snapshot().phase==d::Phase::Face&&f.controller.snapshot().phase_vi==2638,"The second stopped observation rearms Face at2638 instead of using its obsolete2435 epoch");
  need(equal(face,f.at(2638)),"Repeated sameVI cannot advance the freshly rearmed facing interval");
  need(native4(face,{})==4,"Fresh facing requests only native4 when the observed axes allow that magnitude");
  for(unsigned vi=2639;vi<2642;++vi){f.in.horizontal_speed=.2f;const auto out=f.at(vi);need(!(out.buttons&d::egg_button)&&f.controller.snapshot().phase==d::Phase::Face&&f.controller.snapshot().phase_vi==2638&&!f.controller.snapshot().fish_recovering,"Ordinary small native4-facing velocity cannot restart Face or create a recovery loop");}
  const auto z=f.at(2642);need(z.buttons==d::egg_button&&z.x==0&&z.y==0&&f.controller.snapshot().phase==d::Phase::AcquireAim,"Exactly four true FaceVI precede the initial Z edge, with zero directional reticle axes");
  for(unsigned vi=2643;vi<=2645;++vi){const auto pending=f.at(vi);need(pending.buttons==d::egg_button&&!pending.x&&!pending.y,"A Z request alone never invents native4005 or move its nonexistent reticle");}
  f.in.primary_state=3;f.in.aiming_actor=0x800C6AB4;f.in.reticle_actor=0x800C3D6C;f.in.reticle_x=f.in.x+2;f.in.reticle_y=f.in.y+2;
  const auto aiming=f.at(2646);need((aiming.x!=0||aiming.y!=0)&&f.controller.snapshot().phase==d::Phase::TrackAim,"Reticle axes begin only after an explicit HOST observation of the native owned4005");
 }
 {Control f(player);const auto epoch=f.cooldown();f.in.primary_state=1;f.in.grounded=f.in.fish_on_island=false;f.in.horizontal_speed=3;f.in.x=374;f.at(epoch+10);
  f.in.grounded=f.in.fish_on_island=true;f.in.horizontal_speed=0;f.in.x=360;f.at(epoch+11);f.at(epoch+12);need(f.controller.snapshot().phase==d::Phase::Cooldown&&f.controller.snapshot().phase_vi==epoch,"Recovery does NOT rearm or reset the existing Cooldown clock");}
 {Control f(player);f.at(100);f.at(101);f.in.fish_navigation=false;f.in.targets=f.targets;
  d::Output z;for(unsigned vi=102;vi<110;++vi){z=f.at(vi);if(f.controller.snapshot().phase==d::Phase::AcquireAim)break;}
  need(z.buttons==d::egg_button&&(z.x!=0||z.y!=0),"Default-false other-family acquisition retains its existing directional Z command");}
 // The REAL shared Bridge source reader on actual e9 bytes is unchanged.
 {need(r::configure("FishFaceBridge",1,true,true,4),"Explicit HOST runtime source scope");r::rooms::configure(r::session.plan());r::enter_room(52);need(r::session.enter_fight(97,player),"Real Session accepts the actual same-player Fish source");bridge::configure(false);
  const auto before=captured;d::Input in;std::array<float,3> centers{152,360,600};bridge::observe_fish_navigation(ram,player,171,4,centers,in);need(captured==before&&in.fish_navigation,"The actual Bridge observation adapter remains read-only on real e9 capture");}
 std::cout<<"BossFishFace0260Checks: "<<checks<<" PASS; HOST motion/4005 boundaries explicit, real Bridge source composition, no native landing/Z-admission/victory claim.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
