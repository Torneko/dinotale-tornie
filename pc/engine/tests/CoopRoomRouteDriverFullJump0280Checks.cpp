// Six bounded QA command checks using actual stopped GPU18 RAM.
// Registry births are HOST witnesses; physics/landing/admission are not run.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlogical-op-parentheses" // Existing coop_runtime.hpp127.
#include "../coop_room_route_driver0280.hpp"
#pragma clang diagnostic pop
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace d=workshop::coop_room_route_driver0280;namespace n=workshop::coop_native;namespace ids=workshop::coop_collision;namespace rn=workshop::coop_rooms_native0280;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
int main(){
 std::ifstream file("D:/Yoshi-Story/pc/profiles/validation-coop-health-0260-a4c8b1637732/controller-test-final.rdram",std::ios::binary);
 std::vector<uint8_t> ram(0x800000);file.read(reinterpret_cast<char*>(ram.data()),std::streamsize(ram.size()));
 if(file.gcount()!=std::streamsize(ram.size())||file.peek()!=std::char_traits<char>::eof())throw std::runtime_error("Actual GPU18 snapshot missing/invalid");
 auto* r=ram.data();const auto original=ram;const auto manager=n::word(r,0x800F8810),table=n::word(r,manager+16);const auto cap=ids::half(r,manager+8);uint32_t pot=0;
 if(!cap||cap>150||!n::range(table,size_t(cap)*4))throw std::runtime_error("Invalid actual native table");
 for(unsigned i=0;i<cap;i++){const auto a=n::word(r,table+i*4);if(n::actor(r,a,0x4209)){if(pot)throw std::runtime_error("Ambiguous actual native pot");pot=a;}}
 if(!pot)throw std::runtime_error("Missing actual4209");ids::registry.configure(true);if(!ids::registry.created(pot))throw std::runtime_error("HOST birth service failed");rn::current_room=81;rn::entry_overlay=n::word(r,n::word(r,pot+0x168)+0x40);
 constexpr uint32_t player=0x800CDFD4;const auto data=n::word(r,player+0x1F4);
 d::Frame f;f.vi=9600;f.room=81;f.leader=f.player=1;f.enabled=f.bound=f.complete=f.owner_valid=f.pot_valid=true;
 f.x=std::bit_cast<float>(n::word(r,player+0x84));f.y=std::bit_cast<float>(n::word(r,player+0x88));f.pot_x=std::bit_cast<float>(n::word(r,pot+0x84));f.pot_y=std::bit_cast<float>(n::word(r,pot+0x88));f.grounded=n::byte(r,data+0xF4)==0;f.mouth_full=n::word(r,data+0x164)!=0;
 auto out=d::decide(f);need(d::return_pot(r)==pot&&std::abs(f.x-f.pot_x)<=8&&f.y==177&&f.grounded&&!f.mouth_full&&n::word(r,player+0x180)==0&&out.handled&&out.buttons==0x8000&&out.x==0&&out.y==0,"Actual centred grounded GPU18 leader begins complete ordinary A pulse");
 f.vi=9663;f.y=80;f.grounded=false;out=d::decide(f);need(out.buttons==0x8000&&out.mode==d::Mode::jump,"A stays held through phase63 despite high altitude");
 f.vi=9664;out=d::decide(f);need(out.buttons==0&&out.x==0&&out.y==0,"Release A in air at phase64 to descend naturally");
 f.grounded=true;out=d::decide(f);need(out.buttons==0x400&&out.x==0&&out.y==-1,"After jump pulse grounded leader requests ordinary Down");
 f.vi=9600;f.player=0;out=d::decide(f);need(out.buttons==0&&out.x==0&&out.y==0,"Partner waits during leader pulse");
 f.player=1;f.parent_is_pot=true;out=d::decide(f);need(out.buttons==0x400&&out.x==0&&out.y==-1&&ram==original,"Actual-parent command takes priority over A without writing guest RAM");
 std::cout<<"PASS "<<checks<<" full-jump QA checks over actual GPU18 pose; no native gameplay execution\n";
}
