// Reuse HOST Runtime service boundaries, never its broad regression entry.
#define main checkpoint_runtime_entry_not_executed
#include "BossCheckpointRuntime0260Checks.cpp"
#undef main
#include "../boss_driver_bridge0260.hpp"
#include <fstream>
#include <iterator>
namespace bridge=workshop::boss_driver_bridge0260;
namespace d=workshop::boss_driver0260;
int main(int argc,char** argv){try{
 checks=0;need(argc==2,"Supply an actual Fish arena diagnostic capture");std::ifstream stream(argv[1],std::ios::binary);std::vector<uint8_t> ram((std::istreambuf_iterator<char>(stream)),{});need(ram.size()==0x800000,"Actual Fish snapshot contains8MiB");
 const auto player=n::word(ram.data(),0x800F8828);need(n::actor(ram.data(),player,0x4001),"Actual global P1 is typed4001");
 need(r::configure("FishPilotProbe",1,true,true,4),"Explicit HOST diagnostic campaign configuration");r::rooms::configure(r::session.plan());r::enter_room(52);need(r::session.enter_fight(97,player),"Real Session binds captured P1 to the configured Fish source");
 unsigned found=0;for(unsigned member=0;member<r::session.current().count;++member)for(uint32_t a=0x80000000;a<0x807FFE00;a+=4)if(n::actor(ram.data(),a,r::session.current().members[member].kind)){r::bound[member]=a;need(r::session.bind(member,a),"Explicit HOST bound-root boundary uses an actual typed Fish actor");++found;break;}
 need(found>0,"Actual737e capture has a surviving4631; no absent4511 is fabricated for the test");bridge::configure(false);
 const auto before=ram;uint16_t buttons=0;float x=0,y=0;bridge::drive(ram.data(),3000,&buttons,&x,&y);need(ram==before&&bridge::fish_pilot_trace.samples==1,"Actual Bridge drive emits one bounded Fish diagnostic while preserving every native byte");
 const auto first_buttons=buttons;const auto first_x=x,first_y=y;bridge::drive(ram.data(),3000,&buttons,&x,&y);need(ram==before&&bridge::fish_pilot_trace.samples==1&&buttons==first_buttons&&x==first_x&&y==first_y,"Repeated same-VI Bridge input does not duplicate the log or change the ordinary command");
 const auto diagnostic=r::session.snapshot().diagnostic;need(diagnostic,"Diagnostic-only emission is explicit");r::logical_room=151;bridge::drive(ram.data(),3004,&buttons,&x,&y);need(bridge::fish_pilot_trace.samples==1&&ram==before,"A foreign logical source cannot emit a Fish log or mutate RAM");
 bridge::fish_pilot_trace={};d::Input in;in.player=player;in.room=171;in.encounter=4;
 unsigned count=0;for(unsigned vi=100;vi<700;++vi){in.vi=vi;const bool admitted=bridge::fish_pilot_window(in,0);count+=admitted;need(admitted==(vi<580&&(vi-100)%4==0),"Only four-VI samples inside the first480VI are admitted");if(admitted)need(!bridge::fish_pilot_window(in,0),"A repeated admitted VI is deduplicated");}
 need(count==120&&bridge::fish_pilot_trace.samples==120,"Bound is exactly120 samples, not an unbounded per-frame log");in.vi=900;need(bridge::fish_pilot_window(in,1)&&bridge::fish_pilot_trace.samples==1,"A new genuine retry epoch has its own bounded observation window");in.vi=50;need(bridge::fish_pilot_window(in,1)&&bridge::fish_pilot_trace.first_vi==50,"A fresh diagnostic clock resets its host-only trace window");
 std::cout<<"BossFishPilotProbe0260Checks: "<<checks<<" PASS; real Bridge read-only composition and bounded trace only, no movement/damage/victory claim.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
