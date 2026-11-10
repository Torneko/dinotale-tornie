#include "../boss_battle_runtime0250.hpp"
#include "../boss_driver_bridge0260.hpp"
#include <fstream>
#include <iterator>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <limits>
namespace n=workshop::boss_native;namespace b=workshop::boss_driver_bridge0260;namespace m=workshop::boss_monster_aim0260;
static unsigned checks=0;
static void need(bool v,const char* why){++checks;if(!v){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
 std::vector<uint8_t> memory;uint8_t* rdram;uint32_t player,p;
 explicit Fixture(const std::vector<uint8_t>& source):memory(source),rdram(memory.data()),player(n::word(rdram,0x800F8828)),p(n::word(rdram,player+0x1F4)){
  need(n::actor(rdram,player,0x4001)&&n::range(p,0x12D0),"Actual read-only snapshot provides typed player and private storage");
  w(player+0x180,0);by(player+0x17D,2);by(player+0x18,1);by(p+0xF4,0);fl(player+0x84,312);fl(player+0x88,385);
 }
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}void by(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}void h(uint32_t a,unsigned v){MEM_H(0,n::base(a))=uint16_t(v);}void fl(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}
 bool observed(){const auto before=memory;const bool value=b::observe_monster_combat_support(rdram,player);need(before==memory,"Support observation preserves every byte of8MiB");return value;}
 void parent(float py){constexpr uint32_t a=0x800C88E0;need(n::actor(rdram,a,0x415F),"Actual snapshot source415F platform is typed");by(a+0x17D,2);by(a+0x18,1);fl(a+0x84,312);fl(a+0x88,py);w(player+0x180,a);by(player+0x17D,0);h(player+0x17E,19);h(a+0x184,1);w(a+0x188+19*4,player);fl(player+0x88,py-15);}
};
int main(int argc,char** argv){
 need(argc==2,"Explicit immutable failed-run8MiB capture is required");std::ifstream input(argv[1],std::ios::binary);std::vector<uint8_t> source((std::istreambuf_iterator<char>(input)),{});need(source.size()==0x800000,"Capture has exact8MiB length");
 {Fixture f(source);const auto map=m::Map::read(f.rdram);need(map&&map->columns==3&&map->rows==2,"Native151 terrain2 map has the expected real geometry");for(float x:{310.f,312.f,314.f})need(map->occupied(f.rdram,x,399)==false&&map->occupied(f.rdram,x,400)==true&&map->occupied(f.rdram,x,402)==true,"Actual lower static support has a400px surface");need(f.observed(),"Native grounded position on actual static lower support is observed");f.fl(f.player+0x88,380);need(f.observed(),"Static support does not require one hardcoded Y385 actor pose");}
 for(unsigned fault=0;fault<13;++fault){Fixture f(source);
  if(fault==0)f.w(0x800F8828,0);if(fault==1)f.h(f.player+12,0x4004);if(fault==2)f.by(f.player+0x17D,3);if(fault==3)f.by(f.player+0x18,0xFF);if(fault==4)f.by(f.p+0xF4,1);if(fault==5)f.w(f.player+0x1F4,0);if(fault==6)f.fl(f.player+0x84,321);if(fault==7)f.fl(f.player+0x88,465);if(fault==8)f.fl(f.player+0x88,393);if(fault==9)f.fl(f.player+0x88,375);if(fault==10)f.fl(f.player+0x84,std::numeric_limits<float>::quiet_NaN());if(fault==11)f.w(f.player+0x180,0x800CA434);if(fault==12){const auto map=m::Map::read(f.rdram);const unsigned tx=312/16,ty=400/16,page=n::byte(f.rdram,map->pages+(ty/16)*map->columns+tx/16);f.h(map->data+2*((ty%16)*map->stride+page*16+tx%16),0);}
  need(!f.observed(),"Foreign/airborne/disposed/deep-floor/unknown/static-erased support fails closed");
 }
 for(float y:{328.f,351.6f,376.f,400.4f,424.f}){Fixture f(source);f.parent(y);need(f.observed(),"Reciprocal native415F support works over its real changing vertical travel");}
 for(unsigned fault=0;fault<12;++fault){Fixture f(source);constexpr uint32_t a=0x800C88E0;f.parent(376);
  if(fault==0)f.h(a+12,0x415E);if(fault==1)f.by(a+0x17D,1);if(fault==2)f.by(a+0x18,0xFF);if(fault==3)f.fl(a+0x84,313);if(fault==4)f.fl(a+0x88,std::numeric_limits<float>::quiet_NaN());if(fault==5)f.fl(f.player+0x88,300);if(fault==6)f.h(f.player+0x17E,20);if(fault==7)f.h(a+0x184,0);if(fault==8)f.h(a+0x184,21);if(fault==9)f.w(a+0x188+19*4,0);if(fault==10)f.by(f.p+0xF4,1);if(fault==11)f.by(f.player+0x17D,4);
  need(!f.observed(),"Wrong platform type/lifetime/position/slot/reciprocity cannot certify an arrival");
 }
 std::printf("BossMonsterSupport0260Checks: %u PASS (read-only native-bitmap/ABI fixtures; no movement/landing/physics/GPU/victory claim)\n",checks);return 0;
}
