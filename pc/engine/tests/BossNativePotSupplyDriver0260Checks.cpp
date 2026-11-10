#include "../boss_driver0260.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
namespace d=workshop::boss_driver0260;namespace s=workshop::boss_stick_aim0260;
static unsigned checks=0;static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct Fixture {
 d::Controller c;d::Input in;d::Target target{0x80060000,0x40A3,300,143,0,0,true,true,false};std::array<d::Supply,1> supplies{};std::array<float,80> xs{},ys{};
 Fixture(uint16_t kind){need(c.configure(true,true,{},false),"Native edible pot pilot is diagnostic input only");in.player=0x80050000;in.room=168;in.encounter=1;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.x=200;in.y=193;in.min_x=40;in.max_x=468;in.targets=std::span<const d::Target>(&target,1);in.supplies=supplies;in.native_stick_observed=in.native_tongue_observed=in.native_tongue_known=in.native_tongue_ready=true;xs.fill(14);ys.fill(-18);in.native_mouth={true,true,0,1,1,xs,ys};in.native_tongue_max_reach=100;supplies[0]={0x80070000,kind,240,193,true,false,0xC1};}
 d::Output at(unsigned vi){in.vi=vi;return c.step(in);}
};
int main(){try{
 for(uint16_t kind:{uint16_t(0x4539),uint16_t(0x453A)}){
  Fixture x(kind);need(d::pot_supply(x.supplies[0])&&d::edible_supply(x.supplies[0])&&d::supply_valid(x.supplies[0])&&!d::bird_supply(x.supplies[0]),"True4539/453A food flags admit the actual native species without a fake bird ID");const auto first=x.at(100);need(first.buttons==d::tongue_button&&!(first.buttons&d::jump_button)&&!x.c.snapshot().jump_active,"Reachable grounded pot receives normal B immediately, never forced bird A");need(x.c.snapshot().supply_actor==x.supplies[0].actor,"Normal tongue request retains the true edible pot source");const auto repeated=x.at(100);need(first.buttons==repeated.buttons&&first.x==repeated.x&&first.y==repeated.y,"Same-VI input remains idempotent");const auto release=x.at(101);need(!release.buttons,"Rejected or delayed B gets a clean release instead of a stuck held edge");
  x.in.native_tongue_live=true;x.in.native_tongue_phase=2;x.in.native_tongue_heading_active=true;x.in.native_tongue_heading_degrees=-30;x.in.native_tongue_ready=false;x.supplies[0].x=246;const auto hold=x.at(102);need(hold.buttons==d::tongue_button&&!(hold.buttons&d::jump_button),"Actual live pot tongue extends under ordinary native B state, without a synthetic jump");
  const auto ray=s::from_mouth(x.in.native_mouth,x.in.x,x.in.y,x.supplies[0].x,x.supplies[0].y);need(bool(ray),"Source model provides actual mouth ray");const float heading=std::atan2(ray->up,ray->forward)*180.f/s::pi;const auto expected=workshop::boss_tongue_geometry0260::hold_heading_world(heading,true,x.in.native_stick);need(expected&&hold.x==expected->host.x&&hold.y==expected->host.y,"Live heading follows the same true pot rather than the stale heading or a new bird");
  x.in.mouth_full=true;x.in.native_tongue_live=false;x.in.native_tongue_phase=0;const auto swallow=x.at(103);need(!swallow.buttons&&swallow.y<0,"Only actually held food licenses ordinary grounded Down conversion");
 }
 for(unsigned fault=0;fault<7;++fault){Fixture x(0x4539);if(fault==0)x.supplies[0].food_flags=0xC0;if(fault==1)x.supplies[0].kind=0x4538;if(fault==2)x.supplies[0].active=false;if(fault==3)x.in.native_tongue_ready=false;if(fault==4)x.in.native_tongue_observed=false;if(fault==5)x.in.native_mouth.observed=false;if(fault==6)x.in.native_tongue_max_reach=1;const auto out=x.at(100);need(!(out.buttons&(d::tongue_button|d::jump_button)),"Non-food/foreign/inactive/unobserved/not-ready/out-of-reach pots cannot start B or a forced jump");}
 {Fixture x(0x453A);x.in.native_mouth.right=false;const auto out=x.at(100);need(!out.buttons&&out.x>0,"Wrong facing first uses ordinary world movement before a relative B request");}
 {Fixture x(0x4539);x.in.grounded=false;const auto out=x.at(100);need(out.buttons==d::tongue_button&&!(out.buttons&d::jump_button),"A reachable pot in the air also uses the true mouth ray rather than re-triggering flutter");}
 std::cout<<"BossNativePotSupplyDriver0260Checks: "<<checks<<" PASS (ordinary input fixtures only; no native food or egg receipt fabricated)\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
