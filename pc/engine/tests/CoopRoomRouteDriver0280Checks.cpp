// HOST controller checks only: no scheduler, native game body or GPU.
#pragma clang diagnostic push
// Existing coop_runtime.hpp:127 precedes this QA-only helper.
#pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#include "../coop_room_route_driver0280.hpp"
#pragma clang diagnostic pop
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
namespace d=workshop::coop_room_route_driver0280;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static d::Frame frame(){d::Frame f;f.vi=96;f.room=81;f.leader=1;f.player=0;f.enabled=f.bound=f.complete=f.owner_valid=f.pot_valid=f.grounded=true;f.x=6200;f.y=184;return f;}
int main(){
 auto f=frame();auto out=d::decide(f);need(out.handled&&out.mode==d::Mode::jump&&out.buttons==0x8000&&out.x==1&&out.y==0,"Ordinary near-pot jump");
 f.vi=128;out=d::decide(f);need(out.mode==d::Mode::approach&&out.buttons==0&&out.x==1,"Jump released at32/96");
 f.vi=48;out=d::decide(f);need(out.buttons==0x4000&&out.mode==d::Mode::approach,"Ordinary8/48 B tongue outside A jump pulse");
 f.vi=56;need(d::decide(f).buttons==0,"B released after8frames");
 f.x=6280;out=d::decide(f);need(out.x==-1,"Approach from either side");
 for(float x:{6256.f,6264.f,6272.f}){f.x=x;need(d::decide(f).x==0,"Inclusive8px approach deadzone");}
 f=frame();f.parent_is_pot=true;out=d::decide(f);need(out.handled&&out.mode==d::Mode::on_pot&&out.buttons==0&&out.x==0&&out.y==0,"Partner waits on actual pot");
 f.player=1;out=d::decide(f);need(out.buttons==0x400&&out.x==0&&out.y==-1,"Only chosen leader enters grounded actual pot");
 f.grounded=false;out=d::decide(f);need(out.buttons==0x400&&out.x==0&&out.y==-1,"Actual parent requests ordinary Down while F4 settles; native decides admission");
 f.mouth_full=true;out=d::decide(f);need(out.mode==d::Mode::swallow&&out.buttons==0&&out.x==0&&out.y==-1,"Ordinary down to swallow, no forced inventory");
 for(unsigned vi=0;vi<288;vi++)for(unsigned player=0;player<2;player++){
  f=frame();f.vi=vi;f.player=player;out=d::decide(f);
  need(out.handled&&std::abs(out.x)<=1&&std::abs(out.y)<=1,"Normalized backend domain");
  need(bool(out.buttons&0x8000)==((vi%96)<32),"Exactly32A frames in96");
  need(bool(out.buttons&0x4000)==((vi%96)>=32&&(vi%48)<8),"Ordinary B8/48 outside active A pulse");
  need(!(out.buttons&0x400)&&out.y==0,"No pot-entry input away from actual pot centre/support");
 }
 for(unsigned gate=0;gate<13;gate++){
  f=frame();switch(gate){case 0:f.enabled=false;break;case 1:f.bound=false;break;case 2:f.complete=false;break;case 3:f.busy=true;break;case 4:f.owner_valid=false;break;case 5:f.room=43;break;case 6:f.x=5999.9f;break;case 7:f.pot_valid=false;break;case 8:f.x=std::numeric_limits<float>::quiet_NaN();break;case 9:f.y=std::numeric_limits<float>::infinity();break;case 10:f.leader=2;break;case 11:f.player=2;break;case 12:f.pot_y=std::numeric_limits<float>::quiet_NaN();break;}
  need(!d::decide(f).handled,"Outside confirmed QA approach must preserve input files");
 }
 for(unsigned player=0;player<2;player++){f=frame();f.room=43;f.finished=true;f.player=player;f.mouth_full=true;out=d::decide(f);need(out.handled&&out.mode==d::Mode::finished&&out.buttons==0&&out.x==0&&out.y==0,"Actual completed return neutralizes both players");}
 std::vector<uint8_t> ram(0x800000,0xA5);const auto original=ram;uint16_t buttons=0x1234;float x=.25f,y=-.5f;d::configure(false);
 need(!d::input(ram.data(),3000,81,1,0,&buttons,&x,&y)&&buttons==0x1234&&x==.25f&&y==-.5f,"Disabled native adapter is inert");
 need(ram==original,"No guest write from QA input adapter");
 need(!d::input(nullptr,3000,81,1,0,&buttons,&x,&y),"Null guest refused");
 need(!d::input(ram.data(),3000,81,2,0,&buttons,&x,&y),"Invalid leader refused");
 need(!d::input(ram.data(),3000,81,1,2,&buttons,&x,&y),"Invalid owner refused");
 need(!d::input(ram.data(),3000,81,1,0,nullptr,&x,&y),"Null output refused");
 std::cout<<"PASS "<<checks<<" HOST route-controller checks; no native gameplay execution\n";
}
