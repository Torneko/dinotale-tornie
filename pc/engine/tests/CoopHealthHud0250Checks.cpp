#include "../coop_health_hud0250.hpp"
#include <atomic>
#include <iostream>
#include <thread>
namespace h=workshop::coop_health0250;
namespace c=workshop::coop;
static unsigned checks=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
struct Memory:c::ScalarMemory {std::map<uint32_t,uint32_t> data;unsigned writes=0;uint32_t read(uint32_t p)override{return data[p];}void write(uint32_t p,uint32_t v)override{writes++;data[p]=v;}};
int main(){try{
 Memory memory;memory.data[c::native_player]=0x80100000;memory.data[c::native_color]=0;memory.data[c::custom_color]=8;memory.data[c::native_health]=8;
 c::OwnerContext owners(memory);owners.bind(0,{{0x80100000,1},8,8});owners.bind(1,{{0x80101000,2},9,8});
 {c::ScopedOwner p1(owners,0);memory.write(c::native_health,7);}
 {c::ScopedOwner p2(owners,1);memory.write(c::native_health,3);}
 need(memory.read(c::native_health)==7,"Native primary slot restored");
 const auto writes=memory.writes;need(h::publish_owner_frame(10,true,true,&owners,0x80100000,0x80101000,true),"Completed frame published");auto s=h::snapshot();need(s.visible&&s.frame==10&&s.players[0].health==7&&s.players[1].health==3,"Two independent owner health values despite shared hybrid alias0");need(memory.writes==writes,"HUD never writes native memory");need(h::player_label(s,0,true)=="J1  Orange"&&h::player_label(s,1,false)=="P2  Purple","FR/EN identity labels distinct");need(h::health_label(s,0)=="7/8"&&h::health_label(s,1)=="3/8","Each health value labelled separately");
 for(unsigned p=0;p<2;p++){unsigned n=0;for(unsigned i=0;i<8;i++)n+=h::filled(s,p,i);need(n==s.players[p].health,"Gauge shows correct segment count");}need(!h::filled(s,2,0)&&!h::filled(s,0,8),"Out-of-range segments empty");
 {c::ScopedOwner p2(owners,1);memory.write(c::native_health,2);need(!h::publish_owner_frame(11,true,true,&owners,0x80100000,0x80101000,true),"Nested owner transaction not published");need(h::snapshot().frame==10,"No torn pair during P2 transaction");}
 need(!h::publish_owner_frame(11,true,true,&owners,0x80100000,0x80101000,false),"Bridge scope fence enforced");need(h::snapshot().players[1].health==3,"Previous coherent frame preserved until fence");need(h::publish_owner_frame(12,true,true,&owners,0x80100000,0x80101000,true)&&h::snapshot().players[1].health==2,"Next completed pair published");
 {c::ScopedOwner p2(owners,1);memory.write(c::native_health,0);}h::publish_owner_frame(13,true,true,&owners,0x80100000,0x80101000,true);s=h::snapshot();need(s.visible&&s.players[1].health==0&&h::health_label(s,1)=="0/8","Zero health remains visible without pretending healing");need(!h::filled(s,1,0),"Dead player's gauge empty");
 for(unsigned color=0;color<14;color++){need(!h::identities[color].french.empty()&&!h::identities[color].english.empty(),"Every original and hybrid colour labelled");need(h::identities[color].tint.a==255,"All colour accents opaque");}
 h::publish_owner_frame(14,false,true,&owners,0x80100000,0x80101000,true);need(!h::snapshot().visible,"Hidden for solo");h::publish_owner_frame(15,true,false,&owners,0x80100000,0x80101000,true);need(!h::snapshot().visible,"Hidden outside native gameplay");h::publish_owner_frame(16,true,true,nullptr,0,0,true);need(!h::snapshot().visible,"Hidden until both owners bound");h::publish_owner_frame(17,true,true,&owners,0x80100000,0x80100000,true);need(!h::snapshot().visible,"Shared/mismatched identity rejected");
 {c::ScopedOwner p2(owners,1);memory.write(c::native_health,9);}h::publish_owner_frame(18,true,true,&owners,0x80100000,0x80101000,true);need(!h::snapshot().visible,"Corrupt health rejected instead of showing incorrect gauge");
 std::atomic_bool running=true,bad=false;std::thread writer([&]{for(unsigned f=1;f<=20000;f++)h::publish({f,true,{{{0x80100000,0,f%9},{0x80101000,5,(f+1)%9}}}});running=false;});
 std::thread reader([&]{while(running){auto v=h::snapshot();if(v.visible&&v.frame>0&&v.frame<=20000&&(v.players[0].health!=v.frame%9||v.players[1].health!=(v.frame+1)%9))bad=true;}});writer.join();reader.join();need(!bad,"UI/game-thread snapshots never mix two frames");s=h::snapshot();need(s.frame==20000&&s.players[0].health==20000%9&&s.players[1].health==20001%9,"Final coherent frame retained");h::hide(20001);need(!h::snapshot().visible&&h::snapshot().frame==20001,"Explicit reset clears stale co-op HUD");
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"gameLaunched\":false,\"nativeMemoryWritesByHud\":0,\"concurrentFrames\":20000}\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
