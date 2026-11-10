#include "../boss_battle_runtime0250.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
namespace r=workshop::boss_campaign0250;
static unsigned checks=0;
static void need(bool yes,const char* why){++checks;if(!yes){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
static bool near(float a,float b){return std::abs(a-b)<.00001f;}
int main(){
 // Actual read-only6a1 snapshot coordinates, not a synthetic victory fixture.
 {const float px=40.811302185058594f,target=65.30000305175781f;const auto out=r::candy_controls(3720,px,449,target,353.7610168457031f,40,724);const float old_wanted=std::max(8.f,target-38.f);need(old_wanted<40&&old_wanted-px< -4,"old6a1 pilot provably asks to walk into left boundary");need(near(out.x,.35f)&&out.y>0&&out.buttons==0x8000,"clamped6a1 approach turns toward actual candy on right and starts A only");}
 // Successful f70 also approached the same wall; the new facing is independent
 // of the four-frame start difference and native oscillation phase.
 {auto out=r::candy_controls(3600,45.42485427856445f,449,57.885772705078125f,329.1485290527344f,40,724);need(out.x<0,"f70 approach may still walk left until inside real safe bound");out=r::candy_controls(3601,40.5f,449,57.885772705078125f,329.1485290527344f,40,724);need(near(out.x,.35f)&&out.y>0,"f70 near-wall fixture faces right rather than old constant deadzone input");}
 for(float target:{45.f,65.3f,78.f,300.f,700.f,760.f})for(float px:{40.f,41.f,150.f,400.f,723.f,724.f}){
  const auto out=r::candy_controls(120,px,449,target,350,40,724);const float wanted=std::clamp(target-38,40.f,724.f),delta=wanted-px,face=target-px;
  need(std::abs(out.x)<=.65f&&out.y==.8f,"controller axes remain ordinary bounded input");if(delta>4)need(out.x>0,"approach points toward clamped destination on right");else if(delta< -4)need(out.x<0,"approach points toward clamped destination on left");else need(face>4?near(out.x,.35f):face< -4?near(out.x,-.35f):out.x==0,"close approach faces actual target using native deadzone-safe axis");
 }
 for(unsigned start:{0u,120u,127u,240u})for(unsigned dt=0;dt<240;++dt){const auto vi=start+dt;const auto out=r::candy_controls(vi,40.81f,449,65.3f,353.76f,40,724);const unsigned cycle=vi%120;need((out.buttons&0xC000)!=0xC000,"A and B are never emitted together");need(!(out.buttons&0x3FFF),"candy helper cannot generate Z or unrelated controls");if(cycle<18)need(out.buttons==0x8000,"initial18VI are genuine A-only preparation before tongue");if(out.buttons==0x4000)need(cycle>=18&&cycle<100&&vi%20<5,"tongue pulse preserves original bounded timing");}
 {const auto up=r::candy_controls(127,100,449,150,300,40,724),flat=r::candy_controls(127,100,449,150,400,40,724);need(up.y>0&&up.buttons==0x8000,"up direction is prepared before B rather than on its edge");need(flat.y==0,"nearby low segment uses horizontal tongue direction");}
 {const auto out=r::candy_controls(140,300,449,262,400,40,724);need(near(out.x,-.65f),"leftward approach works before facing band");const auto face=r::candy_controls(140,40,449,42,400,40,724);need(face.x==0,"target within four pixels does not force a constant right drift");const auto left=r::candy_controls(140,300,449,262,400,270,724);need(near(left.x,-.65f),"nonstandard native arena bounds are respected");}
 for(unsigned fault=0;fault<8;++fault){float px=40,py=449,tx=65,ty=353,min=40,max=724;const auto nan=std::numeric_limits<float>::quiet_NaN();if(fault==0)px=nan;if(fault==1)py=nan;if(fault==2)tx=nan;if(fault==3)ty=nan;if(fault==4)min=nan;if(fault==5)max=nan;if(fault==6)min=max;if(fault==7)min=max+1;const auto out=r::candy_controls(140,px,py,tx,ty,min,max);need(!out.buttons&&!out.x&&!out.y,"bad coordinates or bounds cannot create diagnostic actions");}
 std::printf("BossCandyDriver0260Checks: %u PASS (pure controller observations; no RAM access, native damage or victory proof)\n",checks);
}
