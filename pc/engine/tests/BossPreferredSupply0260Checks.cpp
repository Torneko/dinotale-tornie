#include "../boss_driver0260.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
namespace d=workshop::boss_driver0260;
static unsigned checks=0;static void need(bool yes,const char* why){++checks;if(!yes){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
 d::Controller controller;d::Input in;std::array<d::Target,1> targets{};std::array<d::Supply,3> supplies{};
 Fixture(){need(controller.configure(true,true,{},false),"explicit diagnostic only");in.vi=120;in.room=168;in.encounter=1;in.player=0x80100000;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.x=324;in.y=326;in.min_x=40;in.max_x=724;in.has_preferred_x=true;in.preferred_x=324;targets[0]={0x80110000,0x4599,344,344,0,0,true,true,false};in.targets=targets;in.supplies=supplies;}
 void feeder(unsigned side=1){supplies[0]={0x80120000,0x456E,88,376,true,false,0,0x4528,uint16_t(side),0};}
 void bird(float x=380,float y=226){supplies[1]={0x80121000,0x4526,x,y,true,false,0xC1};}
 d::Output at(unsigned vi){in.vi=vi;return controller.step(in);}
};
int main(){
 {Fixture f;f.feeder();f.in.x=224;auto out=f.at(120);need(out.x>0&&!out.buttons&&!out.y,"active-side preferred324 overrides feeder120 return toward the same spring");need(f.controller.snapshot().supply_actor==f.supplies[0].actor,"original456E remains the selected non-edible source");f.in.x=324;out=f.at(121);need(!out.x&&!out.y&&!out.buttons,"native Inviso pillar keeps generator active by waiting on its right");}
 {Fixture f;f.feeder(0);f.in.x=224;auto out=f.at(120);need(out.x<0&&!out.buttons,"preferred on wrong generator side cannot override legitimate left crossing");f.in.preferred_x=56;f.in.x=56;out=f.at(121);need(!out.x&&!out.buttons,"left-side feeder accepts a genuinely compatible preferred position");}
 {Fixture f;f.feeder();f.in.y=504;auto out=f.at(120);need(!f.controller.snapshot().supply_actor&&!out.buttons,"strict perpendicular distance128 still rejects feeder even with preferredX");}
 for(float px:{316.f,317.f,324.f,330.f,331.f,331.5f,332.f}){
  Fixture f;f.in.x=px;f.bird();auto out=f.at(120);need(out.buttons==d::jump_button&&out.y>0,"preferred supply retains genuine A-only bird jump and upward preparation");if(px>=331)need(out.x<=0,"bird380 cannot pull player past preferred plus8 during jump preparation");else need(out.x>=0,"safe near-band bird facing stays toward actual bird");
  f.in.grounded=false;out=f.at(132);need(out.buttons==d::tongue_button&&out.y>0,"bounded horizontal preference does not block real airborne B pulse");if(px>=331)need(out.x<=0,"airborne B cannot steer outside preferred plus8 either");
 }
 {Fixture f;f.feeder();f.bird(600,226);const auto out=f.at(120);need(!out.x&&!out.buttons&&f.controller.snapshot().supply_actor==f.supplies[0].actor,"far birds are awaited from active preferred generator side instead of chased");}
 {Fixture f;f.feeder();f.bird();const auto out=f.at(120);need(out.buttons==d::jump_button&&f.controller.snapshot().supply_actor==f.supplies[1].actor,"reachable native egg-producing bird keeps priority over feeder waiting");}
 {Fixture f;f.supplies[0]={0x80122000,0x4002,600,326,true,true};auto out=f.at(120);need(out.x>0&&!out.buttons,"far loose egg may approach only current preferred band edge");f.in.x=332;out=f.at(121);need(!out.x&&!out.buttons,"far loose egg cannot drive beyond324+8");f.supplies[0].x=100;f.in.x=316;out=f.at(122);need(!out.x&&!out.buttons,"opposite loose egg cannot drive beyond324-8");f.in.x=350;f.supplies[0].x=600;out=f.at(123);need(out.x<0&&!out.buttons,"out-of-band player first returns toward bounded loose-egg destination");}
 {Fixture f;f.in.mouth_full=true;f.in.x=340;auto out=f.at(120);need(out.x<0&&!out.y&&!out.buttons,"mouth-full player recentres instead of swallowing outside preferred band");f.in.x=324;f.in.grounded=false;out=f.at(121);need(!out.x&&!out.y&&!out.buttons,"mouth-full preference retains airborne no-Down safety");f.in.grounded=true;out=f.at(122);need(out.y<0&&!out.buttons,"native Down ingestion remains available at grounded preferred position");}
 {Fixture f;f.in.x=224;f.in.y=449;f.bird(280,349);const auto out=f.at(120);need(out.x>0&&out.buttons==d::jump_button,"preference has no fish-style grounded barrier preventing ordinary ascent toward pillar");}
 {Fixture f;f.in.has_preferred_x=false;f.in.x=332;f.supplies[0]={0x80122000,0x4002,600,326,true,true};const auto out=f.at(120);need(out.x>0&&!out.buttons,"default non-preferred supply behavior is unchanged");}
 std::printf("BossPreferredSupply0260Checks: %u PASS (QA controller fixtures only; no native physics, inventory or victory proof)\n",checks);
}
