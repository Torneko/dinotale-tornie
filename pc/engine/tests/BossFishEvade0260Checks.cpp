#include "../boss_fish_evade0260.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>
namespace q=workshop::boss_fish_evade0260;namespace n=workshop::boss_native;
static unsigned checks=0;static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static std::vector<uint8_t> load(const char* file){std::ifstream f(file,std::ios::binary);need(bool(f),"Actual Fish capture opens");std::vector<uint8_t> out((std::istreambuf_iterator<char>(f)),{});need(out.size()==0x800000,"Actual Fish capture is8MiB");return out;}
struct Fixture {
 static constexpr uint32_t red=0x800C9A44,blue=0x800C9CC0;
 std::vector<uint8_t> memory;uint8_t* rdram;uint32_t player=0,priv=0,meta=0,def=0,fish_private=0,fish_resource=0,fish_definition=0,factory=0;
 std::array<q::Root,2> roots{{{red,11},{blue,12}}};
 explicit Fixture(const std::vector<uint8_t>& source):memory(source),rdram(memory.data()){player=n::word(rdram,0x800F8828);priv=n::word(rdram,player+0x1F4);meta=n::word(rdram,player+0x174);def=n::word(rdram,meta);fish_private=n::word(rdram,red+0x1F4);fish_resource=n::word(rdram,red+0x168);factory=n::word(rdram,fish_resource+0x40);fish_definition=n::word(rdram,n::word(rdram,red+0x174));}
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=int32_t(v);}void h(uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}void by(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}void fl(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}
 void pose2653(){
  // HOST reconstruction uses the logged2653 position/velocity/callback;
  // actual captured collider definitions and animated scales stay untouched.
  fl(player+0x84,372.44635f);fl(player+0x88,273);by(red+0x18,1);by(red+0x30,15);w(red+0x20,factory+(0x805C954C-0x805C64F0));fl(red+0x84,387.79984f);fl(red+0x88,301.60001f);fl(red+0xC4,-.9f);fl(red+0xC8,-4.1f);
 }
 q::Threat read(unsigned count=2,unsigned room=171,unsigned encounter=4){const auto before=memory;const auto out=q::observe(rdram,player,{roots.data(),count},room,encounter);need(memory==before,"Every threat read preserves the complete8MiB image");return out;}
 void mirror(bool value){const auto drawing=n::word(rdram,red+0x16C),table=n::word(rdram,drawing),index=n::word(rdram,red+0x138);w(table+4*index,(n::word(rdram,table+4*index)&~1u)|unsigned(value));}
 uint32_t tile_address(unsigned x,unsigned y){const auto map=q::m::Map::read(rdram);need(bool(map),"Real Fish map exists for an explicit ceiling frontier");const unsigned tx=x/16,ty=y/16,page=n::byte(rdram,map->pages+(ty/16)*map->columns+tx/16);return map->data+2*((ty%16)*map->stride+page*16+tx%16);}
};
int main(int argc,char** argv){try{
 need(argc==3,"Pass actualb2 andce3 Fish captures");const auto b2=load(argv[1]),ce3=load(argv[2]);
 for(const auto* source:{&b2,&ce3}){Fixture f(*source);const auto out=f.read();need(out.observed&&!out.imminent,"Actual stationary/underwater Fish poses do not invent a rising threat");}
 {Fixture f(b2);f.pose2653();const auto out=f.read();need(out.observed&&out.imminent&&out.head_clear&&out.actor==Fixture::red&&out.birth==11&&out.step==0,"Logged2653 reconstruction overlaps current native boxes immediately; ordinary A may be proposed");const auto box=q::boxes::observe(f.rdram,Fixture::red,0x4511);need(box&&std::abs(box->center[0]-358.54984f)<.001f&&std::abs(box->half[0]-23.4f)<.001f&&std::abs(box->half[1]-24.6f)<.001f,"Threat uses captured animated scales and native offset, not actor origin or a magic radius");}
 {Fixture f(b2);f.pose2653();
  f.fl(f.player+0x84,350);f.fl(Fixture::red+0x84,400);f.fl(Fixture::red+0xC4,0);f.mirror(false);need(f.read(1).imminent,"Native unmirrored negative offset places the fish volume near P1");f.mirror(true);const auto away=f.read(1);need(away.observed&&!away.imminent,"Native mirrored offset moves the same volume away from P1");}
 {Fixture f(b2);f.pose2653();f.fl(f.player+0x84,360);f.fl(Fixture::red+0x84,390);f.fl(Fixture::red+0x88,303);f.fl(Fixture::red+0xC4,0);f.fl(Fixture::red+0xC8,-.1f);f.mirror(false);const auto scales=n::word(f.rdram,f.fish_definition+0x70);f.fl(scales,1);f.fl(scales+4,1);need(f.read(1).imminent,"Current full native height overlaps");f.fl(scales+4,.1f);const auto small=f.read(1);need(small.observed&&!small.imminent,"Changed valid current scale can remove all16-update predicted overlaps");}
 for(unsigned state:{13u,14u,16u,17u,18u}){Fixture f(b2);f.pose2653();f.by(Fixture::red+0x30,state);need(!f.read().observed,"Passenger/transport phases never license an avoidance jump");}
 for(unsigned fault=0;fault<18;++fault){Fixture f(b2);f.pose2653();switch(fault){
  case 0:f.w(0x800F8828,0);break;case 1:f.h(f.player+12,0x4002);break;case 2:f.by(f.player+0x165,1);break;case 3:f.w(f.player+0x180,Fixture::blue);break;
  case 4:f.w(f.def+0x70,0);break;case 5:f.h(f.def+0xA,15);break;case 6:f.fl(n::word(f.rdram,f.def+0x30)+4,std::numeric_limits<float>::quiet_NaN());break;
  case 7:f.roots[0].birth=0;break;case 8:f.w(Fixture::red+0x130,0);break;case 9:f.w(f.fish_private+0x40,f.player);break;case 10:f.w(Fixture::red+0x180,f.player);break;
  case 11:f.w(f.fish_resource+0x44,f.factory+0x3804);break;case 12:f.w(Fixture::red+0x20,0x80065284);break;case 13:f.w(f.fish_definition+0x28,0);break;case 14:f.by(Fixture::red+0x17D,3);break;
  case 15:f.fl(n::word(f.rdram,f.fish_definition+0x70)+4,0);break;case 16:f.fl(Fixture::red+0xC8,std::numeric_limits<float>::infinity());break;case 17:f.by(f.player+0x18,10);break;
  }need(!f.read().observed,"Foreign source/profile/generation/movement/passenger/scales/probes refuse threat observation");}
 {Fixture f(b2);f.pose2653();f.w(f.meta+8,0xF000);const auto out=f.read();need(out.observed&&out.imminent&&!out.head_clear,"Known threat with native ceiling contact never licenses A");}
 {Fixture f(b2);f.pose2653();f.h(f.tile_address(368,224),0x701);const auto out=f.read();need(out.observed&&out.imminent&&!out.head_clear,"Solid pixels in either native head-probe column block avoidance");}
 {Fixture f(b2);f.pose2653();f.h(f.tile_address(368,224),0x780);const auto out=f.read();need(out.observed&&out.imminent&&!out.head_clear,"Unknown head-column shape fails closed for A");}
 {Fixture f(b2);need(!f.read(0).observed&&!f.read(2,170,4).observed&&!f.read(2,171,9).observed,"Empty roots and foreign ownership domain are inert");}
 std::cout<<"BossFishEvade0260Checks: "<<checks<<" PASS; real snapshots plus explicitHOST2653 frontier, full8MiB read-only; no future collision/landing claim\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
