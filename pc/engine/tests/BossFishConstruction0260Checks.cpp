#define FISH_RECOVERY_FIXTURE_ONLY
#include "BossFishRecovery0260Checks.cpp"
#undef FISH_RECOVERY_FIXTURE_ONLY
static stick::Axes native(const d::Output& out,stick::Axes old){return stick::native_axes(stick::convert({out.x,out.y}),old);}
static unsigned prepare(Control& f,d::ThrowMode mode=d::ThrowMode::HoldAndRelease){
 const auto epoch=f.cooldown();f.in.throw_mode=mode;f.in.primary_state=3;f.in.aiming_actor=0x800C6AB4;f.in.reticle_actor=0x800C3D6C;f.in.reticle_x=468.17502f;f.in.reticle_y=273;f.at(epoch+66);return epoch+67;
}
int main(int argc,char** argv){try{
 checks=0;need(argc==2,"Supply the real2c held4005 diagnostic8MiB; all phase3 transitions below are explicitly HOST observations");const auto captured=load(argv[1]);auto* actual=const_cast<uint8_t*>(captured.data());const auto player=n::word(actual,0x800F8828),data=n::word(actual,player+0x1F4);
 need(n::actor(actual,n::word(actual,data+0x1F0),0x4005)&&n::actor(actual,n::word(actual,data+0x1F4),0x4003),"Use actual same-player held4005/4003 identities, not invented game objects");
 for(auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress})for(auto old:{stick::Axes{54,-54},stick::Axes{-54,54},stick::Axes{54,0},stick::Axes{13,-7},stick::Axes{0,0}}){
  Control f(player);const auto vi=prepare(f,mode);f.in.native_stick=old;const auto first=f.at(vi);const auto a=native(first,old);const auto before=f.controller.snapshot();
  need(a.x==0&&a.y==0&&first.buttons==(mode==d::ThrowMode::HoldAndRelease?d::egg_button:0),"Primary3 requests exact NATIVE0 from observed old axes while retaining the mode-correct Z command");
  need(before.phase==d::Phase::TrackAim&&before.phase_vi==vi&&before.fish_release_not_before==before.fish_cooldown_vi+60,"Construction does not alter the normal TrackAim epoch or saved release deadline");
  need(equal(first,f.at(vi)),"Repeated sameVI is still idempotent");
  const auto second=f.at(vi+1);need(native(second,old).x==0&&native(second,old).y==0&&f.controller.snapshot().phase_vi==before.phase_vi&&f.controller.snapshot().fish_release_not_before==before.fish_release_not_before,"Another constructor observation stays native0 without restarting any clock");
  f.in.primary_state=1;f.in.native_stick={};const auto resumed=f.at(vi+2);const auto b=native(resumed,{});need((b.x||b.y)&&resumed.buttons==first.buttons&&f.controller.snapshot().phase_vi==before.phase_vi&&f.controller.snapshot().fish_release_not_before==before.fish_release_not_before,"Primary1 resumes the existing cursor feedback immediately with unchanged buttons and timing");
 }
 {Control f(player);const auto vi=prepare(f);f.in.reticle_actor=0;f.in.native_stick={54,-54};const auto out=f.at(vi);const auto a=native(out,f.in.native_stick);need(!a.x&&!a.y&&(out.buttons&d::egg_button),"The first genuine4005 before its4003 is ready also requests native0, retaining HoldZ and no invented ready reticle");}
 {Control f(player);const auto vi=prepare(f);f.in.native_stick={54,-54};f.in.fish_evade_observed=f.in.fish_evade_imminent=f.in.fish_evade_head_clear=f.in.fish_evade_generation_valid=true;f.in.fish_threat_actor=0x800C9A44;f.in.fish_threat_birth=1;const auto out=f.at(vi);const auto a=native(out,f.in.native_stick);need(!a.x&&!a.y&&(out.buttons&(d::egg_button|d::jump_button))==(d::egg_button|d::jump_button),"The existing ordinaryA overlay keeps A+HoldZ on top of constructor native0");}
 for(unsigned scope=0;scope<3;++scope){Control f(player);const auto vi=prepare(f);if(scope==0)f.in.fish_navigation=false;if(scope==1)f.in.qa_bird=true;if(scope==2)f.targets[0].kind=0x4072;const auto out=f.at(vi);const auto a=native(out,{});need(a.x||a.y,"Other-family/defaultfalse/qaBird constructor input retains its prior aiming command");}
 // Real Bridge -> actual Controller, with explicit HOST constructor state on
 // a copy of real2c; native2EF78/scheduler are NOT executed in this CPU test.
 for(bool constructor:{true,false}){
  auto scene=captured;auto* ram=scene.data();word(ram,player+0xC4,0);byte(ram,player+0x18,constructor?3:1);word(ram,player+0x20,constructor?0x8003BF88:0x80043450);word(ram,data,1);byte(ram,data+0xF4,0);half(ram,0x800F897E,54);half(ram,0x800F8980,uint16_t(-22));
  Control f(player);const auto vi=prepare(f);need(r::configure("FishConstructionAxes",1,true,true,4),"Explicit HOST diagnostic Runtime");r::rooms::configure(r::session.plan());r::enter_room(52);need(r::session.enter_fight(97,player),"Actual source97 Session");r::bound={0x800C9A44,0x800C9CC0,0,0};for(unsigned m=0;m<2;++m){need(r::session.bind(m,r::bound[m])&&r::variants::bind(ram,m,r::bound[m]),"Real bind and birth for both actual members");recomp_context c{};c.r4=n::base(r::bound[m]);c.r5=c.r6=1;r::variants::trace(ram,&c,"func_805C9D10",{97,2408,true,false});}
  bridge::configure(false);bridge::controller=f.controller;const auto before=scene;uint16_t buttons=0;float x=0,y=0;bridge::drive(ram,vi,&buttons,&x,&y);const auto a=native(d::Output{true,buttons,x,y},{54,-22});need(scene==before&&(buttons&d::egg_button)&&bridge::controller.snapshot().phase==d::Phase::TrackAim,"Actual Bridge keeps the constructor/ready control observation read-only and retains ordinary HoldZ");need(constructor?(!a.x&&!a.y):(a.x||a.y),"Actual Bridge primary3 yields native0, primary1 resumes live collision-center6.6 feedback");
 }
 need(native_x(d::Output{true,0,-1,0},{})==-54&&neutral(d::Output{true}),"Existing input helpers remain intact");
 std::cout<<"BossFishConstruction0260Checks: "<<checks<<" PASS; actual Bridge/Controller with explicit HOST constructor boundary; no native movement, hit or victory claim.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
