// The existing Runtime fixture supplies explicit HOST native service boundaries.
// Its broad regression main is compiled but is not executed by this suite.
#define main checkpoint_runtime_regression_not_executed
#include "BossCheckpointRuntime0260Checks.cpp"
#undef main
#include <fstream>
#include <iterator>

struct ExitRam {
 std::vector<uint8_t> memory;uint8_t* ram;uint32_t player,data,pot=0x800C8B5C,support=0x800C88E0;
 explicit ExitRam(const std::vector<uint8_t>& captured):memory(captured),ram(memory.data()),player(n::word(ram,0x800F8828)),data(n::word(ram,player+0x1F4)){}
 void scalar(uint32_t address,float value){word(ram,address,std::bit_cast<uint32_t>(value));}
 void victory(unsigned index=3){
  r::enabled=r::driver=true;r::failed=false;r::finished=false;
  r::monster_exit_support.reset();r::monster_exit_log_count=0;r::monster_exit_log_stage=-1;
  need(r::session.begin("MonsterExitControls",b::make_plan(1),true)&&r::session.diagnostic_entry(index),"Explicit HOST Session selects the existing seed1 encounter without claiming a game victory");
  need(r::session.enter_fight(b::room(r::session.current().family),player),"The actual Session accepts the captured same-player identity");
  for(unsigned member=0;member<r::session.current().count;++member){const auto root=0x80380000+member*0x400;need(r::session.bind(member,root)&&r::session.defeat({index,member,100+member,root,true,true,true,true,0}),"Explicit HOST native-receipt boundary reaches Victory only through real Session validation");}
  r::physical_room=170;r::logical_room=b::room(r::session.current().family);r::exit_pot=pot;r::exit_x=312;
 }
 struct Output{uint16_t buttons;float x,y;};
 Output drive(unsigned vi){const auto before=memory;Output out{};r::drive_input(ram,vi,&out.buttons,&out.x,&out.y);need(memory==before,"The actual Runtime input consumer preserves every one of the captured8MiB");return out;}
 void detach(float x,float y){
  // Explicit HOST pose/graph boundary, not a claim that stick input landed.
  for(unsigned slot=0;slot<20;++slot)if(n::word(ram,support+0x188+slot*4)==player)word(ram,support+0x188+slot*4,0);
  half(ram,support+0x184,0);word(ram,player+0x180,0);word(ram,n::word(ram,player+0x174)+4,0);
  scalar(player+0x84,x);scalar(player+0x88,y);byte(ram,data+0xF4,1);
 }
};

int main(int argc,char** argv){
 checks=0;need(argc==2,"Supply the real Victory4 capture; no native movement or victory is synthesized");
 std::ifstream stream(argv[1],std::ios::binary);std::vector<uint8_t> captured((std::istreambuf_iterator<char>(stream)),{});
 need(captured.size()==0x800000,"Actual diagnostic capture is exactly8MiB");
 {ExitRam f(captured);need(n::actor(f.ram,f.player,0x4001)&&n::actor(f.ram,f.pot,0x4273)&&n::actor(f.ram,f.support,0x415F),"Actual captured P1, native exit pot and moving support retain their exact species");
  f.victory();const auto out=f.drive(25000);need(out.x>0&&!out.buttons&&!out.y&&r::monster_exit_support,"The ACTUAL Runtime consumer walks sideways from the actual native415F instead of repeating A over the lower pot");
  f.detach(340,340);const auto airborne=f.drive(25001);need(airborne.x>0&&!airborne.buttons&&!airborne.y&&r::monster_exit_support,"A HOST detach above the actual support retains the attested side objective; it never recentres312 prematurely");
  f.detach(356,340);const auto wait=f.drive(25002);need(!wait.x&&!wait.buttons&&!wait.y&&r::monster_exit_support,"At the attested escape column, the consumer releases input and lets native gravity descend");
  f.detach(356,440);const auto below=f.drive(25003);need(below.x<0&&!below.buttons&&!below.y&&r::monster_exit_support,"Only a HOST pose truly below the revalidated415F volume transfers control to the existing pot approach");
  const auto continued=f.drive(25004);need(continued.x<0&&!continued.buttons&&!continued.y&&r::monster_exit_support,"The next detached Approach retains the source witness and continues toward the actual pot");
  f.detach(330,440);const auto falling=f.drive(25080);need(falling.x<0&&!falling.buttons&&!falling.y,"Approach keeps A released during the observed native fall even inside the generic rim-jump radius");
  word(f.ram,f.player+0x180,f.pot);half(f.ram,f.player+0x17E,0);word(f.ram,f.pot+0x188,f.player);half(f.ram,f.pot+0x184,1);byte(f.ram,f.data+0xF4,0);
  const auto rim=f.drive(25081);need(!rim.buttons&&!rim.x&&rim.y<0&&!r::monster_exit_support,"An explicit HOST reciprocal native-pot attachment transfers to ordinary Down and retires the route cache");
 }
 {ExitRam f(captured);f.victory();f.detach(392,321);byte(f.ram,f.data+0xF4,0);ExitRam::Output expected{};
  workshop::boss_solo0240::drive_pot_input(f.ram,f.player,25080,312,0x4273,f.pot,&expected.buttons,&expected.x,&expected.y);
  const auto out=f.drive(25080);need(out.buttons==expected.buttons&&out.x==expected.x&&out.y==expected.y&&!r::monster_exit_support,"A parent0 victory from the already-proved static upper platform keeps the existing ordinary pot controller");
 }
 for(unsigned index:{0u,1u,2u,4u,5u,9u}){ExitRam f(captured);f.victory(index);ExitRam::Output expected{};
  workshop::boss_solo0240::drive_pot_input(f.ram,f.player,25080,312,0x4273,f.pot,&expected.buttons,&expected.x,&expected.y);
  const auto out=f.drive(25080);need(out.buttons==expected.buttons&&out.x==expected.x&&out.y==expected.y&&!r::monster_exit_support,"Candy/Lips/Genie/Fish/Invisible/RedCandy keep exactly their pre-existing ordinary pot commands");
 }
 {ExitRam f(captured);f.victory();const uint32_t held=0x807E0000;actor(f.ram,held,0x4005);word(f.ram,held+0x180,f.player);word(f.ram,f.player+0x188+19*4,held);word(f.ram,f.data+0x1E0,0);
  const auto out=f.drive(25080);need((out.buttons&0x2000)&&!(out.buttons&0x8000)&&!out.x&&!out.y&&!r::monster_exit_support,"The existing owned4005 cancellation retains priority over all Monster descent commands");
 }
 {ExitRam f(captured);f.victory();f.drive(25000);f.detach(340,340);word(f.ram,f.support+0x168,0);
  const auto out=f.drive(25001);need(!out.buttons&&!out.x&&!out.y,"A changed cached support resource cannot continue the ordinary descent using a stale attestation");
 }
 for(unsigned fault=0;fault<6;++fault){ExitRam f(captured);f.victory();
  if(fault==0)r::logical_room=165;if(fault==1)r::physical_room=169;if(fault==2)r::exit_pot=0;if(fault==3)half(f.ram,f.pot+12,0x4274);if(fault==4)word(f.ram,0x800F8828,0);if(fault==5)word(f.ram,f.player+0x180,0);
  const auto out=f.drive(25000);need(!r::monster_exit_support,"Wrong scope, foreign pot/global player or unattested detach cannot create a support-route cache");
  if(fault>=2)need(!out.x&&!out.buttons&&!out.y,"Unknown Monster exit geometry remains neutral rather than granting a speculative route");
 }
 std::printf("BossMonsterExitControls0260Checks: %u PASS (explicit HOST Session receipt boundaries; actual Runtime controls preserve captured RAM; no landing/pot-entry/transport/victory proof)\n",checks);
 return 0;
}
