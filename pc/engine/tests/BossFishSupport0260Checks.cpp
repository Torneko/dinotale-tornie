#include "../boss_fish_support0260.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>
namespace q=workshop::boss_fish_support0260;namespace n=workshop::boss_native;
static unsigned checks=0;static void need(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
static std::vector<uint8_t> load(const char* name){std::ifstream f(name,std::ios::binary);need(bool(f),"Actual Fish capture opens");std::vector<uint8_t> out((std::istreambuf_iterator<char>(f)),{});need(out.size()==0x800000,"Actual Fish capture is8MiB");return out;}
struct Fixture {
 std::vector<uint8_t> memory;uint8_t* rdram;uint32_t player=0,priv=0,meta=0,def=0;std::array<float,3> centers{152,360,600};
 explicit Fixture(const std::vector<uint8_t>& source):memory(source),rdram(memory.data()){player=n::word(rdram,0x800F8828);priv=n::word(rdram,player+0x1F4);meta=n::word(rdram,player+0x174);def=n::word(rdram,meta);}
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=int32_t(v);}void h(uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}void by(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}void fl(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}
 uint32_t tile_address(const q::m::Map& map,unsigned x,unsigned y){const unsigned tx=x/16,ty=y/16,page=n::byte(rdram,map.pages+(ty/16)*map.columns+tx/16);return map.data+2*((ty%16)*map.stride+page*16+tx%16);}
 std::optional<q::Island> read(std::optional<float> previous={},unsigned room=171,unsigned encounter=4,std::span<const float> points={}){const auto before=memory;const auto result=q::observe(rdram,player,room,encounter,points.empty()?std::span<const float>(centers):points,previous);need(memory==before,"Read-only support observation preserves the complete8MiB");return result;}
};
int main(int argc,char** argv){try{
 need(argc==3,"Pass real737e grounded andce3 airborne Fish captures");const auto ground=load(argv[1]),air=load(argv[2]);
 {Fixture f(ground);const auto first=f.read();need(first&&first->x==360&&first->on_island,"Real737e/3000 attests the native middle island while grounded");const auto continued=f.read(first->x);need(continued&&continued->on_island&&continued->x==360,"Unchanged native context retains the same current island proof");
  f.by(f.priv+0xF4,1);const auto airborne=f.read(first->x);need(airborne&&!airborne->on_island,"Host airborne frontier retains anchor without inventing grounded support");need(!f.read(),"An airborne player cannot establish an initial anchor");
  f.by(f.priv+0xF4,0);f.fl(f.player+0x84,380);need(!f.read(),"Outside the first-proof centre window cannot create a new anchor");const auto edge=f.read(first->x);need(edge&&!edge->on_island,"Native foot outside the stamped platform is not supported despite grounded byte");
 }
 {Fixture f(air);need(!f.read(),"Actualce3 LIVE drift cannot manufacture initial support");const auto observed=f.read(360);need(observed&&observed->x==360&&!observed->on_island,"Actualce3 airborne drift keeps only the previously attested map anchor");need(!f.read(359),"A previous anchor absent from configured centres refuses");}
 for(unsigned fault=0;fault<21;++fault){Fixture f(ground);const auto first=f.read();need(bool(first),"Actual positive fixture precedes each independent rejection");const auto map=q::m::Map::read(f.rdram);need(bool(map),"Actual native map is present");switch(fault){
  case 0:f.w(0x800F8828,0);break;case 1:f.h(f.player+12,0x4002);break;case 2:f.by(f.player+0x165,1);break;case 3:f.w(f.player+0x180,0x800C954C);break;case 4:f.w(f.meta+4,0x800C954C);break;
  case 5:f.w(f.player+0x174,0);break;case 6:f.w(f.meta,0);break;case 7:f.w(f.def+0x2C,11);break;case 8:f.w(f.def+0x30,0);break;case 9:f.w(f.def+0x34,0);break;
  case 10:f.fl(n::word(f.rdram,f.def+0x30)+12,-3.2f);break;case 11:f.fl(n::word(f.rdram,f.def+0x34)+12,12.8f);break;
  case 12:f.h(map->descriptor+0x12,2);break;case 13:f.h(map->descriptor+0x14,1);break;case 14:f.w(0x800F8810,0);break;
  case 15:f.h(f.tile_address(*map,360,288),0);break;case 16:f.h(f.tile_address(*map,360,288),0x601);break;case 17:f.h(f.tile_address(*map,360,287),0x601);break;case 18:f.h(f.tile_address(*map,360,304),0);break;
  case 19:f.fl(f.player+0x84,std::numeric_limits<float>::quiet_NaN());break;case 20:f.fl(f.player+0x88,std::numeric_limits<float>::infinity());break;
  }need(!f.read()&&!f.read(first->x),"Foreign identity, native probes, unknown geometry and changed platform refuse even with a previous anchor");
 }
 {Fixture f(ground);need(!f.read({},170,4)&&!f.read({},171,9),"Room and encounter ownership domain remain strict");std::array<float,2> short_list{152,360};need(!f.read({},171,4,short_list),"Configured list requires exactly three native platforms");f.centers[0]=std::numeric_limits<float>::quiet_NaN();need(!f.read(),"Nonfinite configured centre refuses");f.centers={152,361,600};need(!f.read(360),"Changed configured centres cannot retain an old anchor");}
 std::cout<<"BossFishSupport0260Checks: "<<checks<<" PASS; actual737e/ce3 snapshots, full8MiB read-only, no native physics/landing claim\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
