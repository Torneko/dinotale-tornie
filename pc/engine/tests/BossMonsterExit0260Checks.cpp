#include "../boss_monster_exit0260.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
namespace q=workshop::boss_monster_exit0260;namespace n=workshop::boss_native;
static unsigned checks=0;static void need(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
struct Fixture {
 std::vector<uint8_t> memory;uint8_t* rdram;uint32_t player=0,metadata=0,priv=0;
 static constexpr uint32_t pot=0x800C8B5C,platform=0x800C88E0;
 explicit Fixture(const std::vector<uint8_t>& source):memory(source),rdram(memory.data()){player=n::word(rdram,0x800F8828);metadata=n::word(rdram,player+0x174);priv=n::word(rdram,player+0x1F4);}
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=int32_t(v);}void h(uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}void by(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}void fl(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}
 std::optional<q::Route> read(const q::Support* s=nullptr,unsigned room=170,unsigned encounter=3){const auto before=memory;const auto v=q::observe(rdram,player,pot,room,encounter,s);need(memory==before,"Every route observation preserves all8MiB");return v;}
 void detach(){w(player+0x180,0);w(metadata+4,0);by(player+0x17D,2);by(priv+0xF4,1);}
};
int main(int argc,char** argv){try{
 need(argc==2,"Pass actual303f live Victory4 capture");std::ifstream in(argv[1],std::ios::binary);need(bool(in),"Actual capture opens");std::vector<uint8_t> source((std::istreambuf_iterator<char>(in)),{});need(source.size()==0x800000,"Actual capture is8MiB");
 {Fixture f(source);const auto first=f.read();need(first&&first->stage==q::Stage::Descend&&first->support_attested&&!first->clear_below,"Actual415F rider and lower4273 establish a descent route");need(first->support.actor==Fixture::platform&&first->target_x==356&&first->support.escape_x==356&&first->pot_x==312&&first->pot_y==469,"Actual36px physical half and4px foot probe choose the clear right column356");
  const auto retained=first->support;f.detach();const auto edge=f.read(&retained);need(edge&&edge->stage==q::Stage::Descend&&edge->support_attested&&edge->target_x==356,"Host-detached frontier retains side while P1 is still above the platform bottom");need(!f.read(),"A detached player cannot manufacture a prior platform witness");
  f.fl(f.player+0x88,430);const auto below=f.read(&retained);need(below&&below->stage==q::Stage::Approach&&below->clear_below&&below->target_x==312,"Only P1 object top below physical platform bottom permits approach to actual potX");
  f.fl(Fixture::platform+0x88,400);const auto moving=f.read(&retained);need(moving&&moving->stage==q::Stage::Descend,"Current moving platform bottom is rechecked rather than cached as a stale altitude");f.fl(f.player+0x88,450);need(f.read(&retained)->clear_below,"Clearance uses the new current platform transform");
  f.w(f.player+0x180,Fixture::pot);f.w(f.metadata+4,Fixture::pot);f.h(f.player+0x17E,0);f.h(Fixture::pot+0x184,1);f.w(Fixture::pot+0x188,f.player);const auto rim=f.read();need(rim&&rim->stage==q::Stage::Rim&&rim->target_x==312,"A real reciprocal4273 parent delegates its native entry independently of an old415F witness");
 }
 for(unsigned fault=0;fault<28;++fault){Fixture f(source);const auto first=f.read();need(bool(first),"Actual source fixture initially attests");const auto& s=first->support;const auto pd=n::word(f.rdram,f.metadata);switch(fault){
  case 0:f.w(0x800F8828,0);break;case 1:f.by(f.player+0x18,10);break;case 2:f.by(f.player+0x165,1);break;case 3:f.by(Fixture::pot+0x17D,3);break;case 4:f.h(Fixture::pot+12,0x4274);break;
  case 5:f.by(Fixture::platform+0x17D,3);break;case 6:f.h(Fixture::platform+12,0x415E);break;case 7:f.w(Fixture::platform+0x20,0x80065284);break;case 8:f.w(Fixture::platform+0x130,0);break;
  case 9:f.w(f.metadata+4,Fixture::pot);break;case 10:f.h(f.player+0x17E,20);break;case 11:f.h(Fixture::platform+0x184,0);break;case 12:f.w(Fixture::platform+0x188+19*4,f.player);break;
  case 13:f.w(s.record,0x45070000);break;case 14:f.fl(s.record+4,313);break;case 15:f.fl(s.record+8,377);break;case 16:f.fl(s.private_data+4,400);break;
  case 17:f.w(s.definition+0x64,0);break;case 18:f.w(s.definition+0x68,0);break;case 19:f.w(s.metadata+4,Fixture::pot);break;
  case 20:f.fl(n::word(f.rdram,pd+0x34)+12,12.8f);break;case 21:f.fl(n::word(f.rdram,pd+0x30)+12,-3);break;case 22:f.fl(Fixture::pot+0x88,320);break;case 23:f.w(Fixture::platform+0x168,s.resource+4);break;
  case 24:f.h(s.definition+0x50,1);break;case 25:f.h(s.definition+0x52,5);break;case 26:f.h(s.definition+0x56,25);break;case 27:f.h(s.definition+0x58,13);break;
  }need(!f.read(),"Foreign/stale/partial source, native probes, platform or pot refuses route without writes");
 }
 for(unsigned bad=0;bad<5;++bad){Fixture f(source);const auto first=f.read();need(bool(first),"Identity-cache fixture initially attested");auto saved=first->support;f.detach();if(bad==0)++saved.encounter;if(bad==1)++saved.room;if(bad==2)saved.pot+=4;if(bad==3)saved.actor+=4;if(bad==4)saved.escape_x=312;need(!f.read(&saved),"A saved side is scoped to the exact room/encounter/player/pot/platform and proved column");}
 {Fixture f(source);need(!f.read(nullptr,169,3)&&!f.read(nullptr,170,9),"Foreign physical room and encounter domain remain closed");}
 std::cout<<"BossMonsterExit0260Checks: "<<checks<<" PASS; actual capture observations only, no game/pose/parent mutation\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
