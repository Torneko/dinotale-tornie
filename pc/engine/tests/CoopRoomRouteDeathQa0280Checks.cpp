// Focused QA pad decisions only; no gameplay/body/room/lifecycle execution.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#include "../coop_room_route_driver0280.hpp"
#pragma clang diagnostic pop
#include <iostream>
#include <stdexcept>
namespace d=workshop::coop_room_route_driver0280;
static unsigned checks=0;static void need(bool b,const char* m){++checks;if(!b)throw std::runtime_error(m);}
int main(){
 d::DeathFrame f;need(!d::decide_death(f).handled,"Outside dedicated scope remains inactive");
 f.active=true;f.player=0;auto out=d::decide_death(f);need(out.handled&&out.buttons==0&&out.x==0&&out.y==0,"Victim stays neutral without artificial damage");
 f.enemy_valid=true;f.victim_x=56;f.enemy_x=409;out=d::decide_death(f);need(out.handled&&out.buttons==0&&out.x==1&&out.y==0,"Victim approaches observed typed enemy with stick only");
 f.victim_x=385;out=d::decide_death(f);need(out.buttons==0&&out.x==0&&out.y==0,"Victim stops within24 of the observed enemy");
 f.player=1;f.survivor_valid=true;f.survivor_x=56;out=d::decide_death(f);need(out.handled&&out.buttons==0x8000&&out.x==-1&&out.y==0,"Before death survivor moves left with ordinary A pulse and no B");
 f.vi=80;f.survivor_x=15;out=d::decide_death(f);need(out.buttons==0&&out.x==0,"At left wall release stick/A during pulse off phase");
 f.vi=0;f.victim_pending=true;out=d::decide_death(f);need(out.buttons==0x8000&&out.x==1&&out.y==0,"During genuine victim wait survivor moves right with A and no B");
 f.victim_reborn=true;const auto p2=d::decide_death(f);f.player=0;const auto p1=d::decide_death(f);need(p1.handled&&p2.handled&&!p1.buttons&&!p2.buttons&&p1.x==0&&p2.x==0&&p1.y==0&&p2.y==0,"After new birth both neutral");
 std::cout<<"PASS "<<checks<<" after-room-death QA decisions; no gameplay execution\n";
}
