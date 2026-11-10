#include "../boss_driver0260.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>

namespace d=workshop::boss_driver0260;
static unsigned checks=0;
static void need(bool yes,const char* why){++checks;if(!yes){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
static bool same(d::Output a,d::Output b){return a.handled==b.handled&&a.buttons==b.buttons&&std::abs(a.x-b.x)<.00001f&&std::abs(a.y-b.y)<.00001f;}
struct Fixture {
 d::Controller controller;std::array<d::Target,1> targets{};std::array<float,1> safe{324};d::Input in;
 Fixture(){
  need(controller.configure(true,true,{},false),"explicit synthetic QA controller only");
  targets[0]={0x80110000,0x4599,300,350,0,0,true,true,false};
  in.vi=120;in.room=164;in.encounter=2;in.player=0x80100000;in.combat=in.controls=in.alive=in.grounded=true;in.spring_attached=true;in.primary_state=1;in.reserves=2;in.x=150;in.y=450;in.min_x=40;in.max_x=724;in.targets=targets;
 }
 d::Output at(unsigned vi){in.vi=vi;return controller.step(in);}
 void aim(d::ThrowMode mode){in.throw_mode=mode;in.primary_state=3;in.aiming_actor=0x80120000;in.reticle_actor=0x80121000;in.reticle_x=300;in.reticle_y=350;}
};
int main(){
 // Pure observation-to-controller fixtures: no RAM, game functions, process,
 // save files, victory receipts or permanent challenge APIs are available.
 for(unsigned start:{120u,127u})for(unsigned target_mode=0;target_mode<3;++target_mode){
  Fixture f;if(target_mode==1)f.in.targets={};if(target_mode==2)f.targets[0].vulnerable=false;
  for(unsigned elapsed=0;elapsed<67;++elapsed){const auto out=f.at(start+elapsed);const unsigned cycle=elapsed%30;const auto expected=cycle>=2&&cycle<6?d::jump_button:0;
   need(out.handled&&out.buttons==expected,"native rider gets2VI release then4VI A-only, repeated each30VI");
   need(!(out.buttons&(d::tongue_button|d::egg_button))&&out.x>0&&out.y==0,"spring release walks toward arena centre without B, Z or vertical input");
   need(f.controller.snapshot().phase==d::Phase::SpringRelease&&f.controller.snapshot().phase_vi==start,"missing or invulnerable target cannot reset or block rider release");
   const auto cached=f.at(start+elapsed);need(same(cached,out)&&f.controller.snapshot().phase_vi==start,"same-VI step is idempotent and cannot retrigger a jump edge");
  }
  need(f.controller.snapshot().launches==0&&f.controller.snapshot().attempts==0,"spring A pulses never claim egg attempts, launches or victory");
 }
 for(unsigned inactive=0;inactive<8;++inactive){Fixture f;if(inactive==0)f.in.controls=false;if(inactive==1)f.in.alive=false;if(inactive==2)f.in.combat=false;if(inactive==3)f.in.player=0;if(inactive==4)f.in.x=std::numeric_limits<float>::quiet_NaN();if(inactive==5)f.in.y=std::numeric_limits<float>::infinity();if(inactive==6)f.in.min_x=f.in.max_x;if(inactive==7)f.in.throw_mode=d::ThrowMode(2);const auto out=f.at(122);need(out.handled&&!out.buttons&&!out.x&&!out.y&&f.controller.snapshot().phase==d::Phase::Idle,"rider logic never overrides invalid/inactive native controls");}
 for(float x:{100.f,384.f,650.f}){Fixture f;f.in.x=x;const auto out=f.at(127);need(x<384?out.x>0:x>384?out.x<0:out.x==0,"without safe stand rider walks toward arena centre");need(!out.buttons&&out.y==0,"first spring-release frame always releases earlier buttons");}
 for(float x:{244.f,324.f,400.f}){Fixture f;f.in.x=x;f.in.stable_stands=f.safe;const auto out=f.at(120);need(x<324?out.x>0:x>324?out.x<0:out.x==0,"nearest valid stable stand supersedes broad arena midpoint");}
 {Fixture f;std::array<float,4> stands{std::numeric_limits<float>::quiet_NaN(),-200,900,324};f.in.stable_stands=stands;f.in.x=400;need(f.at(120).x<0,"nonfinite/out-of-bounds stands cannot redirect rider away from valid support");}
 {Fixture f;f.at(120);need(f.at(122).buttons==d::jump_button,"fixture reaches attached jump pulse");f.in.room=170;f.in.encounter=3;auto out=f.at(123);need(!out.buttons&&f.controller.snapshot().phase==d::Phase::SpringRelease&&f.controller.snapshot().phase_vi==123,"new room and encounter restart release interval before fresh A");need(!f.at(124).buttons&&f.at(125).buttons==d::jump_button,"new room retains2VI release and starts its own edge");out=f.at(10);need(!out.buttons&&f.controller.snapshot().phase_vi==10,"VI rollback cannot retain an old spring jump epoch");}
 {Fixture f;f.at(120);f.at(122);f.in.spring_attached=false;auto out=f.at(123);need(f.controller.snapshot().phase==d::Phase::SpringRelease&&f.controller.snapshot().spring_detached&&f.controller.snapshot().spring_detach_vi==123&&out.x>0&&out.buttons==d::jump_button,"native detach continues away from spring instead of immediately reversing toward target standoff");for(unsigned vi=124;vi<183;++vi){out=f.at(vi);need(f.controller.snapshot().phase==d::Phase::SpringRelease&&out.x>0&&!(out.buttons&(d::tongue_button|d::egg_button)),"postdetach escape keeps its direction for bounded60VI despite same selected target");need(vi<150?out.buttons==d::jump_button:out.buttons==0,"postdetach jump holds only until initial elapsed30VI");}out=f.at(183);need(f.controller.snapshot().phase==d::Phase::Face&&!out.buttons&&!f.controller.snapshot().spring_detached,"60VI escape deadline returns to ordinary facing without stale detached state");for(unsigned vi=184;vi<=187;++vi)out=f.at(vi);need(f.controller.snapshot().phase==d::Phase::AcquireAim&&out.buttons==d::egg_button&&f.controller.snapshot().attempts==1,"same selected target cannot strand controller after bounded escape");}
 {Fixture f;f.at(120);f.in.spring_attached=false;f.in.targets={};f.in.x=(f.in.min_x+f.in.max_x)*.5f;auto out=f.at(121);need(f.controller.snapshot().phase==d::Phase::Idle&&!out.buttons&&!out.x&&!out.y,"detachment safely at centre without target resumes neutral Idle");}
 {Fixture f;f.at(120);f.in.controls=false;auto out=f.at(122);need(!out.buttons&&f.controller.snapshot().phase==d::Phase::Idle,"control lock cancels pending rider A");f.in.controls=true;out=f.at(130);need(!out.buttons&&f.controller.snapshot().phase_vi==130,"restored controls must obtain a fresh release interval");need(f.at(132).buttons==d::jump_button,"fresh native A edge follows restored controls");}
 for(auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress}){
  Fixture attached,baseline;attached.aim(mode);baseline.aim(mode);baseline.in.spring_attached=false;
  for(unsigned vi=120;vi<=130;++vi){const auto a=attached.at(vi),b=baseline.at(vi);need(same(a,b)&&attached.controller.snapshot().phase==baseline.controller.snapshot().phase,"real4005 aim/hold/release/secondpress is identical with or without a spring parent");need(!(a.buttons&d::tongue_button)&&attached.controller.snapshot().phase!=d::Phase::SpringRelease,"spring release cannot cancel an owned aiming egg with B or fresh A");}
  need(attached.controller.snapshot().launches==0,"sent Z edges alone never certify native projectile launch");
  Fixture interrupted;interrupted.at(120);interrupted.aim(mode);auto out=interrupted.at(121);need(interrupted.controller.snapshot().phase==d::Phase::TrackAim&&out.buttons==(mode==d::ThrowMode::HoldAndRelease?d::egg_button:0),"4005 appearing during rider release immediately retains real native throw mode");
 }
 {Fixture f;need(f.controller.configure(true,false,{},false)==false,"production controller cannot be enabled diagnostically by spring input");uint16_t buttons=0x1234;float x=.4f,y=-.2f;need(!f.controller.drive(f.in,&buttons,&x,&y)&&buttons==0x1234&&x==.4f&&y==-.2f,"non-QA rider observation leaves original user input untouched");}

 for(auto pose:std::array<std::pair<float,float>,2>{{{224,344},{500,640}}})for(bool grounded:{false,true}){
  Fixture f;f.in.spring_attached=false;f.in.has_preferred_x=true;f.in.preferred_x=324;f.in.x=pose.first;f.targets[0].x=pose.second;f.targets[0].y=344;f.in.y=449;f.in.grounded=grounded;
  auto out=f.at(120);need(f.controller.snapshot().phase==d::Phase::Approach&&(f.in.x<324?out.x>0:out.x<0),"Inviso horizontal preference beats old target-minus150 counterexamples224/344 and500/640");need(out.buttons==d::jump_button&&out.y==0,"arrivalY449 retains normal A approach jump; preferredX is not a high stable-stand claim");
 }
 {Fixture f;f.in.has_preferred_x=true;f.in.preferred_x=324;f.in.x=335;f.targets[0].x=640;auto out=f.at(120);need(out.x<0&&f.controller.snapshot().phase==d::Phase::SpringRelease,"spring escape follows preferred324 rather than broader arena midpoint");f.in.spring_attached=false;out=f.at(123);need(f.controller.snapshot().phase==d::Phase::SpringRelease&&out.x<0&&out.buttons==d::jump_button,"detached actor335 remains outside centre8 tolerance and keeps escaping left");f.in.x=330;out=f.at(124);need(f.controller.snapshot().phase==d::Phase::Approach&&out.x<0&&!out.buttons,"centre6 offset ends escape but still recentres to preferred3 tolerance before Face");f.in.x=324;out=f.at(125);need(f.controller.snapshot().phase==d::Phase::Face&&!out.buttons,"native grounded preferred centre resumes ordinary facing");}
 {Fixture f;f.in.has_preferred_x=true;f.in.preferred_x=324;f.at(120);f.at(122);f.in.spring_attached=false;f.in.grounded=false;f.in.x=324;auto out=f.at(123);need(f.controller.snapshot().phase==d::Phase::SpringRelease&&out.x==0&&out.buttons==d::jump_button,"reaching centre while airborne retains initial jump until grounded or bounded deadline");f.in.grounded=true;out=f.at(124);need(f.controller.snapshot().phase==d::Phase::Face&&!out.buttons,"grounded centre ends escape early rather than waiting full60VI");}
 {Fixture f;f.in.has_preferred_x=true;f.in.preferred_x=324;f.at(120);f.at(122);f.in.spring_attached=false;f.at(123);f.in.spring_attached=true;auto out=f.at(124);need(f.controller.snapshot().phase==d::Phase::SpringRelease&&!f.controller.snapshot().spring_detached&&!out.buttons&&f.controller.snapshot().phase_vi==124,"native reattachment restarts release interval rather than reusing detached pulse");need(!f.at(125).buttons&&f.at(126).buttons==d::jump_button,"reattachment produces a fresh native A edge after2VI release");}
 for(float invalid:{std::numeric_limits<float>::quiet_NaN(),-1.f,800.f}){Fixture f;f.in.has_preferred_x=true;f.in.preferred_x=invalid;const auto out=f.at(120);need(f.controller.snapshot().phase==d::Phase::Idle&&!out.buttons&&!out.x&&!out.y,"nonfinite or out-of-bounds preferred coordinate is rejected before movement");}
 {Fixture f;f.in.preferred_x=std::numeric_limits<float>::quiet_NaN();const auto out=f.at(120);need(f.controller.snapshot().phase==d::Phase::SpringRelease&&out.x>0,"unused optional preferred coordinate cannot poison ordinary spring control");}
 {Fixture f;std::array<float,3> fish{152,384,616};f.in.spring_attached=false;f.in.stable_stands=fish;f.in.x=400;f.in.y=248;f.targets[0].x=576;f.targets[0].y=328;auto out=f.at(120);need(f.controller.snapshot().phase==d::Phase::Approach&&out.x<0&&!out.buttons&&out.y==0,"fish stable-stand safety still recentres before jump or Z");f.in.x=384;f.in.grounded=false;out=f.at(121);need(!out.buttons&&!out.x&&!out.y,"fish airborne stable-stand guard remains unchanged");}
 for(auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress}){Fixture f;f.in.has_preferred_x=true;f.in.preferred_x=324;f.at(120);f.at(122);f.in.spring_attached=false;f.in.grounded=false;f.at(123);f.aim(mode);const auto out=f.at(124);need(f.controller.snapshot().phase==d::Phase::TrackAim&&!f.controller.snapshot().spring_detached&&out.buttons==(mode==d::ThrowMode::HoldAndRelease?d::egg_button:0),"real4005 supersedes pending postdetach escape in both throw modes");}
 std::printf("BossSpringDriver0260Checks: %u PASS (synthetic diagnostic controller only; no RAM/native/game/ledger or victory proof)\n",checks);
}
