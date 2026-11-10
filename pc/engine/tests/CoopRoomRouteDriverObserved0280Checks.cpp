// Actual stopped GPU16 RAM, read-only. Birth/lifecycle services below are
// explicit HOST witnesses; no native physics, pose write or game is executed.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlogical-op-parentheses" // Existing coop_runtime.hpp127.
#include "../coop_room_route_driver0280.hpp"
#pragma clang diagnostic pop
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
namespace d=workshop::coop_room_route_driver0280;namespace n=workshop::coop_native;namespace ids=workshop::coop_collision;namespace rn=workshop::coop_rooms_native0280;namespace life=workshop::coop_lifecycle0280;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static uint32_t bits(float value){return std::bit_cast<uint32_t>(value);}
int main(){
 const char* source="D:/Yoshi-Story/pc/profiles/validation-coop-health-0260-9b7ad7d9d2d7/controller-test-final.rdram";
 std::ifstream file(source,std::ios::binary);need(bool(file),"Actual stopped GPU16 fixture missing");
 std::vector<uint8_t> ram(0x800000);file.read(reinterpret_cast<char*>(ram.data()),std::streamsize(ram.size()));need(file.gcount()==std::streamsize(ram.size())&&file.peek()==std::char_traits<char>::eof(),"Actual dump must be8MiB");
 auto* r=ram.data();const auto original=ram;constexpr uint32_t pot=0x800CBF88,p0=0x800C954C,p1=0x800CDFD4;
 const auto resource=n::word(r,pot+0x168);need(resource==0x803FEBD0&&n::actor(r,pot,0x4209),"Actual pot/resource identity changed");
 const auto raw_y=std::bit_cast<float>(n::word(r,pot+0x40)),world_y=std::bit_cast<float>(n::word(r,pot+0x88));
 need(std::abs(raw_y-185.2f)<.0001f&&world_y==181.f,"Actual native settling differs from observed failure");
 need(std::abs(raw_y-184.f)>.5f,"Old static QA Y guard did not reject this true snapshot");
 // These HOST creations supply only registry incarnation services. They do not
 // execute or claim the original45CAC chain or reproduce its birth numbers.
 ids::registry.configure(true);need(ids::registry.created(pot)&&ids::registry.created(p0)&&ids::registry.created(p1),"HOST identities unavailable");
 rn::current_room=81;rn::entry_overlay=n::word(r,resource+0x40);
 workshop::coop_rooms0280::Route route;need(rn::source(r,pot,route)&&route.destination()==43,"Actual Source route/resource invalid");
 need(d::return_pot(r)==pot,"Actual physically settled native return pot must be admitted");
 need(ram==original,"Pot observation wrote guest fixture");
 life::lifecycle.configure(true);need(life::lifecycle.bind(r,p0,p1,0,5,43,9600)&&life::lifecycle.transferred(r,81),"HOST lifecycle bind to actual native roots failed");
 for(unsigned player=0;player<2;player++){
  need(d::root_observed(r,player),"Actual root was rejected before the pot guard");
  const uint32_t actor=player?p1:p0,data=n::word(r,actor+0x1F4);
  d::Frame f;f.vi=9600;f.room=81;f.leader=1;f.player=player;f.enabled=f.bound=f.complete=f.owner_valid=f.pot_valid=true;
  f.x=std::bit_cast<float>(n::word(r,actor+0x84));f.y=std::bit_cast<float>(n::word(r,actor+0x88));f.pot_x=std::bit_cast<float>(n::word(r,pot+0x84));f.pot_y=world_y;f.grounded=n::byte(r,data+0xF4)==0;f.mouth_full=n::word(r,data+0x164)!=0;
  const auto out=d::decide(f);need(out.handled&&out.mode==d::Mode::jump&&out.x==-1&&out.y==0&&out.buttons==0x8000,"Actual blocked edge root must receive ordinary jump-left toward live pot");
 }
 need(ram==original,"Actual root/decision observation wrote guest fixture");
 for(unsigned bad=0;bad<10;bad++){
  ram=original;r=ram.data();switch(bad){
  case 0:n::word(r,pot+0x3C,bits(6266));break;
  case 1:n::word(r,pot+0x40,bits(201));break;
  case 2:n::word(r,pot+0x88,bits(201));break;
  case 3:n::word(r,pot+0x40,bits(std::numeric_limits<float>::quiet_NaN()));break;
  case 4:n::word(r,pot+0x88,bits(std::numeric_limits<float>::infinity()));break;
  case 5:n::word(r,resource+0x3C,0x42080000);break;
  case 6:n::word(r,resource+0x40,rn::entry_overlay+4);break;
  case 7:n::byte(r,n::word(r,n::word(r,pot+0x1F4)+0x260)+4,81);break;
  case 8:rn::current_room=43;break;
  case 9:ids::registry.forgotten(pot);break;
  }
  need(!d::return_pot(r),"Corrupt/wrong/out-of-band actual pot evidence admitted");
  rn::current_room=81;if(bad==9)need(ids::registry.created(pot),"Restore HOST pot identity");
 }
 ram=original;r=ram.data();need(d::return_pot(r)==pot,"Valid actual pot not restored after negative fixture copies");
 need(ram==original,"Unmodified actual snapshot must remain byte-exact");
 // Additional actual GPU17 capture: old QA repeatedly held A at the centre
 // and never tried Down. This validates commands only, not native admission.
 const char* source17="D:/Yoshi-Story/pc/profiles/validation-coop-health-0260-f6379edbf49f/controller-test-final.rdram";
 std::ifstream file17(source17,std::ios::binary);need(bool(file17),"Actual GPU17 fixture missing");
 file17.read(reinterpret_cast<char*>(ram.data()),std::streamsize(ram.size()));need(file17.gcount()==std::streamsize(ram.size())&&file17.peek()==std::char_traits<char>::eof(),"Actual GPU17 dump must be8MiB");
 const auto original17=ram;r=ram.data();const auto manager=n::word(r,0x800F8810),table=n::word(r,manager+16);const auto cap=ids::half(r,manager+8);uint32_t pot17=0;
 for(unsigned i=0;i<cap;i++){const auto a=n::word(r,table+i*4);if(n::actor(r,a,0x4209)){need(!pot17,"GPU17 return pot must be unique");pot17=a;}}
 need(pot17==0x800C8B5C&&n::kind(r,0x800CBF88)==0x46F4,"Track actual pot incarnation, not reused GPU16 address");
 ids::registry.configure(true);need(ids::registry.created(pot17)&&ids::registry.created(p0)&&ids::registry.created(p1),"HOST GPU17 identity services");rn::entry_overlay=n::word(r,n::word(r,pot17+0x168)+0x40);rn::current_room=81;
 need(d::return_pot(r)==pot17,"Actual GPU17 native pot/route/geometry invalid");
 for(unsigned player=0;player<2;player++){
  const auto actor=player?p1:p0,data=n::word(r,actor+0x1F4);
  d::Frame f;f.vi=9664;f.room=81;f.leader=1;f.player=player;f.enabled=f.bound=f.complete=f.owner_valid=f.pot_valid=true;
  f.x=std::bit_cast<float>(n::word(r,actor+0x84));f.y=std::bit_cast<float>(n::word(r,actor+0x88));f.pot_x=std::bit_cast<float>(n::word(r,pot17+0x84));f.pot_y=std::bit_cast<float>(n::word(r,pot17+0x88));f.grounded=n::byte(r,data+0xF4)==0;f.mouth_full=n::word(r,data+0x164)!=0;
  need(n::word(r,actor+0x180)==0&&std::abs(f.x-f.pot_x)<4&&f.y==177&&!f.mouth_full,"Actual GPU17 non-parent centre observation changed");
  const auto out=d::decide(f);
  if(player){need(f.grounded,"Actual chosen leader must have native grounded evidence");need(out.handled&&out.mode==d::Mode::on_pot&&out.buttons==0x0400&&out.x==0&&out.y==-1,"Actual grounded leader requests ordinary Down without asserting native parent");}
  else need(out.handled&&out.mode==d::Mode::on_pot&&out.buttons==0&&out.x==0&&out.y==0,"Actual partner releases A/B and waits");
  f.player=f.leader;f.grounded=false;const auto air=d::decide(f);need(air.handled&&air.buttons==0&&air.x==0&&air.y==0,"Airborne centre arrival releases A/B to land naturally");
  f.vi=9600;f.x=f.pot_x-8.1f;f.y=f.pot_y;const auto away=d::decide(f);need(away.handled&&away.buttons==0x8000&&away.x==1,"Outside centre preserve ordinary jump toward pot");
 }
 need(ram==original17,"GPU17 decision observation wrote guest memory");
 std::cout<<"PASS "<<checks<<" observed-RAM QA route checks; no native gameplay execution\n";
}
