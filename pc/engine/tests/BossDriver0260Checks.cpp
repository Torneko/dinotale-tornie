#include "../boss_driver0260.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
namespace d=workshop::boss_driver0260;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
 d::Controller controller;std::array<d::Target,4> targets{};std::array<d::Supply,3> supplies{};d::Input in;
 Fixture(d::ThrowMode mode=d::ThrowMode::HoldAndRelease){
  need(controller.configure(true,true,{},false),"explicit diagnostic configuration");
  targets[0]={0x80110000,0x4599,300,350,0,0,true,true,false};
  in.vi=1;in.room=164;in.encounter=2;in.player=0x80100000;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.reserves=2;in.x=150;in.y=450;in.min_x=40;in.max_x=724;in.throw_mode=mode;in.targets=targets;
 }
 d::Output at(unsigned vi){in.vi=vi;return controller.step(in);}
 void start(){for(unsigned vi=1;vi<=5;++vi)at(vi);need(controller.snapshot().phase==d::Phase::AcquireAim,"native Z acquisition phase");}
 void aim(){in.primary_state=3;in.aiming_actor=0x80120000;in.reticle_actor=0x80121000;in.reticle_x=300;in.reticle_y=350;}
};
int main(){
 {
  d::Controller c;uint16_t buttons=123;float x=.3f,y=.4f;d::Input in;
  need(c.configure(false,false,{},false),"disabled controller allowed");need(!c.drive(in,&buttons,&x,&y)&&buttons==123&&x==.3f&&y==.4f,"disabled controller leaves input unchanged");
  need(!c.configure(true,false,{},false),"enabled controller requires diagnostic flag");need(!c.step(in).handled,"production session cannot run diagnostic controls");d::Config bad;bad.projectile_speed=0;need(!c.configure(true,true,bad,false),"invalid speed rejected");
 }
 {
  Fixture f;f.start();auto z=f.at(5);need(z.buttons==d::egg_button,"first native Z edge uses2000");f.aim();z=f.at(6);need(z.buttons==d::egg_button&&!(z.buttons&d::tongue_button),"hold mode keeps Z and never cancels with B");
  const auto count=f.controller.snapshot().aim_matches;auto repeated=f.at(6);need(repeated.buttons==z.buttons&&repeated.x==z.x&&f.controller.snapshot().aim_matches==count,"repeat VI is idempotent");
  need(f.at(7).buttons==d::egg_button,"second aligned frame still holds");need(f.at(8).buttons==0&&f.controller.snapshot().phase==d::Phase::Release,"third real aligned frame releases native Z");f.in.aiming_actor=0;f.in.primary_state=1;f.in.projectile_actor=0x80130000;f.at(9);need(f.controller.snapshot().launches==1,"launch requires a newly observed native projectile");f.at(10);need(f.controller.snapshot().phase==d::Phase::Cooldown,"hold mode does not require second press");need(!f.at(20).buttons,"cooldown releases all buttons");need(f.controller.snapshot().launches==1,"same projectile never counted twice");
 }
 {
  Fixture f(d::ThrowMode::SecondPress);f.start();f.aim();need(f.at(6).buttons==0,"tap mode releases initial Z while aiming");f.at(7);f.at(8);need(f.at(9).buttons==0,"tap mode creates release interval");need(f.at(10).buttons==d::egg_button&&f.controller.snapshot().phase==d::Phase::SecondPress,"tap mode sends genuine second edge");need(f.at(11).buttons==d::egg_button,"second edge held two frames");need(f.at(12).buttons==0,"second edge released");need(f.controller.snapshot().launches==0,"sending controls is not proof of native launch");
 }
 {
  Fixture f(d::ThrowMode::SecondPress);f.start();f.aim();for(unsigned vi=6;vi<=12;++vi)f.at(vi);
  need(f.controller.snapshot().phase==d::Phase::Cooldown,"fixture finishes second Z press");
  for(unsigned vi=13;vi<77;++vi){const auto out=f.at(vi);need(f.controller.snapshot().phase==d::Phase::Cooldown&&!out.buttons,"live delayed4005 cannot bypass configured cooldown");}
  f.at(77);need(f.controller.snapshot().phase==d::Phase::Approach,"configured cooldown eventually allows retry");
 }
 {
  Fixture f;f.start();f.aim();f.in.reticle_x=0;f.in.reticle_y=450;for(unsigned vi=6;vi<126;++vi){const auto out=f.at(vi);need(!(out.buttons&d::tongue_button),"misaligned aiming waits within its configured limit");}need(f.controller.snapshot().phase==d::Phase::TrackAim,"misaligned reticle waits within limit");const auto timed=f.at(126);need((timed.buttons&d::tongue_button)&&f.controller.snapshot().phase==d::Phase::CancelAim,"bounded timeout cancels a short or misaligned reticle rather than firing");need(f.controller.snapshot().launches==0,"timeout never fabricates launch or victory");
 }
 {
  Fixture f;f.in.reserves=0;f.supplies[0]={0x80140000,0x4002,170,450,true,true};f.in.supplies=f.supplies;auto out=f.at(1);need(f.controller.snapshot().phase==d::Phase::Supply&&out.x>0&&!out.buttons,"walk toward real collectible egg");
  f.supplies[0].collectible=false;out=f.at(2);need(!out.buttons&&!out.x&&!out.y,"flying egg not treated as collectible");
  f.supplies[1]={0x80141000,0x4086,150,360,true,false};out=f.at(3);need(out.buttons==d::jump_button&&out.x==0,"jump under genuine4086 egg block");
  f.supplies[1].active=false;f.supplies[2]={0x80142000,0x4526,200,350,true,false,1};out=f.at(24);need(out.buttons==d::jump_button&&out.y>0&&out.x>=.35f,"yellow bird starts with jump alone and prepares upward tongue");
  f.in.grounded=false;out=f.at(35);need(out.buttons==d::jump_button,"bird jump has twelve VI without tongue");out=f.at(36);need(out.buttons==d::tongue_button&&out.y>0,"bird tongue starts only after genuine airborne delay");
  f.in.mouth_full=true;f.in.grounded=true;out=f.at(37);need(out.y<0&&!out.buttons,"native down input converts swallowed enemy on ground");
 }
 {
  Fixture f;f.targets[0].active=false;for(unsigned n=1;n<4;++n)f.targets[n]={0x80110000+n*0x300,0x4599,300+100.f*n,350,0,0,true,true,false};f.at(1);need(f.controller.snapshot().target==f.targets[1].actor,"select nearest of four native roots");f.targets[1].defeated=true;f.at(2);need(f.controller.snapshot().target==f.targets[2].actor,"naturally defeated member replaced by next");f.targets[2].vulnerable=false;f.at(3);need(f.controller.snapshot().target==f.targets[3].actor,"skip native invulnerability phase");f.targets[3].kind=0x455A;f.at(4);need(!f.controller.snapshot().target,"Candy belongs to existing tongue driver");
 }
 {
  Fixture f;f.targets[0].kind=0x455A;need(!d::supported(0x455A),"ordinary supported families exclude Candy");need(!d::target_valid(f.targets[0]),"unmarked native455A is not an egg target");auto out=f.at(1);need(!out.buttons&&!f.controller.snapshot().target,"unmarked Candy cannot start Z acquisition");
  f.targets[0].red_candy=true;need(d::target_valid(f.targets[0]),"explicit red455A accepts true head or segment coordinates");f.at(2);need(f.controller.snapshot().target==f.targets[0].actor,"red Candy root retains native455A identity");
  for(uint16_t kind:{uint16_t(0x454F),uint16_t(0x4550),uint16_t(0x4001),uint16_t(0x0000)}){auto target=f.targets[0];target.kind=kind;need(!d::target_valid(target),"red opt-in does not admit arbitrary native kinds");}
  auto target=f.targets[0];target.active=false;need(!d::target_valid(target),"inactive red Candy excluded");target=f.targets[0];target.defeated=true;need(!d::target_valid(target),"defeated red Candy excluded");target=f.targets[0];target.x=std::numeric_limits<float>::quiet_NaN();need(!d::target_valid(target),"nonfinite red Candy coordinates excluded");
 }
 for(const auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress}){
  Fixture f(mode);f.targets[0].kind=0x455A;f.targets[0].red_candy=true;f.start();need(f.at(5).buttons==d::egg_button,"red Candy uses first genuine Z edge");f.aim();
  for(unsigned vi=6;vi<=8;++vi){const auto out=f.at(vi);need(!(out.buttons&d::tongue_button),"red Candy aim never cancels with B");if(vi<8)need(out.buttons==(mode==d::ThrowMode::HoldAndRelease?d::egg_button:0),"red Candy follows configured native Z mode");}
  need(f.controller.snapshot().phase==d::Phase::Release&&!f.at(8).buttons,"red Candy reticle alignment creates genuine release");need(!f.at(9).buttons,"red Candy release interval has no B or Z");auto out=f.at(10);
  if(mode==d::ThrowMode::SecondPress){need(out.buttons==d::egg_button,"red Candy tap mode sends second Z edge");need(f.at(11).buttons==d::egg_button&&!f.at(12).buttons,"red Candy second edge is bounded and released");}
  else need(!out.buttons&&f.controller.snapshot().phase==d::Phase::Cooldown,"red Candy hold mode finishes through Z release");
  need(f.controller.snapshot().launches==0,"red Candy input generation alone never claims a native launch");f.in.aiming_actor=0;f.in.primary_state=1;f.in.projectile_actor=0x80133000;f.at(13);need(f.controller.snapshot().launches==1,"red Candy launch requires genuinely observed projectile");
  f.targets[0].red_candy=false;out=f.at(14);need(!out.buttons&&!out.x&&!out.y&&!f.controller.snapshot().target,"removing red ownership stops the egg controller immediately");
 }
 for(uint16_t kind:{uint16_t(0x4599),uint16_t(0x4072),uint16_t(0x40A3),uint16_t(0x41C6),uint16_t(0x4511),uint16_t(0x4631)}){Fixture f;f.targets[0].kind=kind;f.start();need(f.at(5).buttons==d::egg_button,"all egg families use genuine Z");}
 for(const auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress}){
  Fixture f(mode);need(f.controller.configure(true,true,{},true),"QA release journal enabled as in the production bridge");f.targets[0].kind=0x4526;f.targets[0].birth=17;
  need(!d::supported(0x4526)&&!d::target_valid(f.targets[0]),"native4526 stays excluded from every ordinary boss target");
  f.in.qa_bird=true;need(d::target_valid(f.targets[0],true),"explicit QA input admits the real4526 generation");f.start();f.aim();for(unsigned vi=6;vi<=12;++vi)f.at(vi);
  const auto& release=f.controller.release_trace();need(release.kind==0x4526&&release.root==f.targets[0].actor&&release.serial,"QA release retains true4526 identity without a fake boss ID");
  need(f.controller.snapshot().launches==0,"QA Z/reticle alone never proves a native shot");f.in.aiming_actor=0;f.in.primary_state=1;f.in.projectile_actor=0x80133000;f.at(13);need(f.controller.snapshot().launches==1,"QA shot also requires the actually observed projectile");
  f.in.qa_bird=false;auto out=f.at(14);need(!out.buttons&&!out.x&&!out.y&&!f.controller.snapshot().target,"removing the QA opt-in immediately excludes the bird");
  auto bad=f.targets[0];bad.birth=0;need(!d::target_valid(bad,true),"zero-birth4526 cannot be a QA target");
  for(uint16_t kind:{uint16_t(0x46DA),uint16_t(0x4001),uint16_t(0x4002),uint16_t(0x454F)}){bad=f.targets[0];bad.kind=kind;need(!d::target_valid(bad,true),"QA bird opt-in admits no other native actor ID");}
 }
 {
  Fixture f;f.start();f.aim();f.at(6);f.in.controls=false;auto out=f.at(7);need(out.handled&&!out.buttons&&!out.x&&!out.y,"native cinematic/control freeze releases controller");
  f.in.controls=true;f.in.alive=false;out=f.at(8);need(!out.buttons&&!out.x,"fatal player produces neutral controls");f.in.alive=true;f.in.x=std::numeric_limits<float>::quiet_NaN();need(!f.at(9).buttons,"nonfinite position releases controls");
 }
 {
  Fixture f;f.start();f.aim();f.at(6);f.in.room=163;f.in.encounter=3;f.in.aiming_actor=0;f.in.primary_state=1;f.at(7);need(f.controller.snapshot().attempts==0&&f.controller.snapshot().phase==d::Phase::Face,"new native room resets old controller sequence");f.at(2);need(f.controller.snapshot().last_vi==2,"VI rollback resets stale sequence");
 }
 {
  Fixture f;std::array<float,3> islands{152,384,616};f.in.x=152;f.targets[0].x=576;f.in.stable_stands=islands;auto out=f.at(1);need(f.controller.snapshot().phase==d::Phase::Face&&out.x>0&&out.x<.5f,"separated-island arena retains nearby safe shooting position");
  f.in.vi=2;uint16_t b=0;float x=0,y=0;need(!f.controller.drive(f.in,nullptr,&x,&y),"invalid output pointer rejected");need(f.controller.drive(f.in,&b,&x,&y),"normal pointer API writes controller outputs");need(std::abs(x)<=1&&std::abs(y)<=1,"axes bounded to standard range");
 }
 std::printf("BossDriver0260Checks: %u PASS (controller fixtures only; no native win claim)\n",checks);
}
