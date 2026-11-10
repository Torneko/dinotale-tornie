#include "../boss_driver0260.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
namespace d=workshop::boss_driver0260;
static unsigned checks=0;
static void need(bool ok,const char* why){++checks;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
 d::Controller controller;d::Input in;d::Target target{0x80120000,0x4072,650,450,0,0,true,true,false};
 std::array<float,1> shooting_stand{328};std::array<d::Supply,2> sources{};
 Fixture(){need(controller.configure(true,true,{},false),"Explicit diagnostic controller");in.vi=1;in.player=0x80100000;in.room=169;in.encounter=2;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.x=360;in.y=673;in.min_x=256;in.max_x=415;in.genie_navigation=in.genie_support_observed=true;in.genie_anchor_x=328;in.has_preferred_x=true;in.preferred_x=328;in.stable_stands=shooting_stand;in.targets={&target,1};in.supplies=sources;}
 d::Output step(unsigned vi=1){in.vi=vi;return controller.step(in);}
};
int main(){
 {Fixture f;f.sources[0]={0x80130000,0x4002,390,673,true,true};const auto out=f.step();need(out.x>0&&!out.buttons&&f.controller.snapshot().phase==d::Phase::Supply,"R0 walks toward a real collectible beyond336, within the proved plateau");need(f.in.has_preferred_x&&f.in.stable_stands.size()==1&&f.in.stable_stands[0]==328,"Forage copy never changes caller shooting preference or island observations");f.in.x=380;const auto next=f.step(2);need(next.x>0&&f.controller.snapshot().phase==d::Phase::Supply,"Crossing anchor band never restarts Approach while R0");f.in.reserves=1;const auto stocked=f.step(3);need(stocked.x<0&&!stocked.buttons&&f.controller.snapshot().phase==d::Phase::Approach,"First real observed reserve restores return328 priority");}
 {Fixture f;f.sources[0]={0x80130000,0x4526,390,600,true,false,1};auto out=f.step();need(out.x>0&&out.buttons==d::jump_button,"Real edible bird over plateau starts ordinary A toward bird rather than return328");f.in.x=370;f.in.y=650;f.in.grounded=false;out=f.step(14);need(f.controller.snapshot().phase==d::Phase::Supply&&out.buttons==d::tongue_button,"R0 keeps supply jump epoch across anchor band and uses ordinary delayed B in synthetic control fixture");need(f.controller.snapshot().jump_vi==1,"Forage does not continually restart jump timing");}
 {Fixture f;f.in.x=390;f.in.mouth_full=true;auto out=f.step();need(out.y<0&&!out.buttons&&!out.x,"Held native food uses ordinary down at any grounded point of proved support");}
 for(float x:{255.f,416.f}){Fixture f;f.in.x=x;f.sources[0]={0x80130000,0x4526,x<328?280.f:400.f,600,true,false,1};const auto out=f.step();need(!out.buttons&&((x<328&&out.x>0)||(x>328&&out.x<0)),"R0 outside proved interior returns anchor instead of foraging over gaps");}
 for(float x:{240.f,431.f}){Fixture f;f.sources[0]={0x80130000,0x4002,x,673,true,true};const auto out=f.step();need(!out.x&&!out.buttons&&!f.controller.snapshot().supply_actor,"Remote source outside conservative support cannot become a walking destination");}
 for(unsigned reserves:{1u,6u}){Fixture f;f.in.reserves=reserves;const auto out=f.step();need(out.x<0&&!out.buttons,"Stocked Genie retains original shooting-anchor priority including hot6");}
 for(unsigned fault=0;fault<3;++fault){Fixture f;f.sources[0]={0x80130000,0x4002,390,673,true,true};if(fault==0)f.in.genie_support_observed=false;else if(fault==1)f.target.kind=0x4073;else f.target.defeated=true;const auto out=f.step();need(!out.x&&!out.y&&!out.buttons,"Unknown support, foreign species or defeated root cannot license forage");}
 {Fixture f;f.target.vulnerable=false;f.sources[0]={0x80130000,0x4002,390,673,true,true};const auto out=f.step();need(out.x>0&&f.controller.snapshot().phase==d::Phase::Supply,"Real owned root may temporarily be invulnerable during independent refill");}
 {Fixture f;f.sources[0]={0x80130000,0x4526,390,600,true,false,1};f.in.native_tongue_observed=true;f.in.native_tongue_known=false;const auto out=f.step();need(!out.x&&!out.y&&!out.buttons,"Unknown native tongue graph remains neutral before forage requests");}
 {Fixture f;f.in.aiming_actor=0x80140000;f.in.reticle_actor=0x80141000;f.in.reticle_x=650;f.in.reticle_y=450;const auto out=f.step();need(f.controller.snapshot().phase==d::Phase::TrackAim&&!(out.buttons&d::tongue_button),"Genuine4005 keeps aim ownership even at R0 beyond anchor band");}
 for(float x:{256.f,415.f}){Fixture f;f.in.x=x;f.sources[0]={0x80130000,0x4002,x==256?250.f:420.f,673,true,true};const auto out=f.step();need((x==256?out.x>=0:out.x<=0)&&!out.buttons,"Forage boundary cannot request outward motion toward an out-of-bounds source");}
 std::printf("BossGenieRefill0260Checks: %u PASS (ordinary control fixtures, no native consumption/damage/win claim)\n",checks);
}
