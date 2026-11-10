#include "../coop_health_hud0250.hpp"
#include <iostream>
namespace h=workshop::coop_health0250;
namespace c=workshop::coop;
static unsigned checks=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
struct Memory final:c::ScalarMemory{
 std::map<uint32_t,uint32_t> data;unsigned reads=0,writes=0;
 uint32_t read(uint32_t a)override{reads++;return data[a];}
 void write(uint32_t a,uint32_t v)override{writes++;data[a]=v;}
};
int main(){try{
 Memory m;m.data[c::native_player]=0x80100000;m.data[c::custom_color]=8;m.data[c::native_color]=0;m.data[c::native_health]=7;
 c::OwnerContext owners(m);owners.bind(0,{{0x80100000,1},8,7});owners.bind(1,{{0x80101000,2},9,3});
 const std::array<h::Waiting,2> p1{{{true,8,300},{}}},p2{{{},{true,9,120}}};
 auto before=m.writes,reads=m.reads;
 need(h::publish_waiting_frame(20,true,true,&owners,0,0x80101000,true,p1),"P1 wait at completed fence published");auto s=h::snapshot();
 need(s.visible&&s.players[0].actor==0&&s.players[0].health==0&&s.players[0].waiting,"Dead P1 has no guest actor pointer");
 need(s.players[1].actor==0x80101000&&s.players[1].health==3&&!s.players[1].waiting,"Live P2 independently published despite hybrid alias0");
 need(h::visual_slot(s,0)==1&&h::visual_slot(s,1)==0,"Native singleton flower is labelled as surviving P2");
 need(h::player_label(s,0,true)=="J1  Retour 5s"&&h::player_label(s,0,false)=="P1  Return 5s","Countdown labels FR/EN");
 need(m.writes==before&&m.reads==reads,"Waiting HUD never accesses or writes guest memory");
 need(h::publish_waiting_frame(21,true,true,&owners,0x80100000,0,true,p2),"P2 wait published");s=h::snapshot();
 need(s.visible&&s.players[0].health==7&&s.players[1].actor==0,"Live P1 preserved and dead P2 opaque");
 need(h::visual_slot(s,0)==0&&h::visual_slot(s,1)==1,"P1 survivor keeps left flower");
 need(h::player_label(s,1,true)=="J2  Retour 2s","Second owner countdown");
 for(unsigned ticks:{0u,1u,59u,60u,61u,299u,300u}){
  auto wait=p1;wait[0].remaining_vi=ticks;need(h::publish_waiting_frame(30+ticks,true,true,&owners,0,0x80101000,true,wait),"Countdown observation admitted");
  s=h::snapshot();need(s.visible&&s.players[0].remaining_vi==ticks,"Countdown is an observation, not a native mutation");
 }
 {c::ScopedOwner owner(owners,1);m.write(c::native_health,0);}
 h::publish_waiting_frame(400,true,true,&owners,0,0x80101000,true,p1);s=h::snapshot();
 need(s.visible&&s.players[1].health==0&&!s.players[1].waiting,"Zero-health survivor remains alive; no fake heal");
 {c::ScopedOwner owner(owners,1);need(!h::publish_waiting_frame(401,true,true,&owners,0,0x80101000,true,p1),"Nested scopes do not publish");need(h::snapshot().frame==400,"Prior coherent snapshot preserved");}
 need(!h::publish_waiting_frame(402,true,true,&owners,0,0x80101000,false,p1),"Bridge fence enforced");
 auto bad=p1;bad[0].remaining_vi=301;h::publish_waiting_frame(403,true,true,&owners,0,0x80101000,true,bad);need(!h::snapshot().visible,"Impossible countdown rejected");
 bad=p1;bad[0].color=9;h::publish_waiting_frame(404,true,true,&owners,0,0x80101000,true,bad);need(!h::snapshot().visible,"Dead owner colour mismatch rejected");
 bad=p1;bad[1]={true,9,40};h::publish_waiting_frame(405,true,true,&owners,0,0,true,bad);need(!h::snapshot().visible,"Both down not presented as active return");
 h::publish_waiting_frame(406,true,true,&owners,0,0x80102000,true,p1);need(!h::snapshot().visible,"Unrelated live root rejected");
 h::publish_waiting_frame(407,true,false,&owners,0,0x80101000,true,p1);need(!h::snapshot().visible,"No waiting HUD outside native lifecycle");
 h::publish_waiting_frame(408,false,true,&owners,0,0x80101000,true,p1);need(!h::snapshot().visible,"Solo path remains hidden");
 h::publish_waiting_frame(409,true,true,nullptr,0,0x80101000,true,p1);need(!h::snapshot().visible,"Missing context hidden");
 const std::array<h::Waiting,2> none{};h::publish_waiting_frame(410,true,true,&owners,0x80100000,0x80101000,true,none);s=h::snapshot();
 need(s.visible&&!s.players[0].waiting&&!s.players[1].waiting,"Normal pair capture restored without replacing native flowers");
 need(h::player_label(s,0,true)=="J1  Orange","Usual identity labels unchanged");
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"guestWritesByHud\":0,\"gameLaunched\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
