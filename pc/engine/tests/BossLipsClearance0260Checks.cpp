// Actual a78 terrain probes/map, with synthetic ceilings on RAM copies only.
// No native actor moves and no physical jump or victory is certified here.
#include "../boss_lips_jump0260.hpp"
#include <cstdio>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
namespace j=workshop::boss_lips_jump0260;namespace m=workshop::boss_monster_aim0260;namespace n=workshop::boss_native;
static unsigned checks=0;static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
struct Memory {
 std::vector<uint8_t> bytes;uint8_t* rdram;uint32_t player,metadata,definition,xs,ys;
 explicit Memory(const std::vector<uint8_t>& source):bytes(source),rdram(bytes.data()),player(n::word(rdram,0x800F8828)),metadata(n::word(rdram,player+0x174)),definition(n::word(rdram,metadata)),xs(n::word(rdram,definition+0x30)),ys(n::word(rdram,definition+0x34)){}
 void w(uint32_t at,uint32_t value){MEM_W(0,n::base(at))=int32_t(value);}void by(uint32_t at,unsigned value){MEM_B(0,n::base(at))=uint8_t(value);}void h(uint32_t at,unsigned value){MEM_H(0,n::base(at))=uint16_t(value);}void fl(uint32_t at,float value){w(at,std::bit_cast<uint32_t>(value));}
 float f(uint32_t at){return std::bit_cast<float>(n::word(rdram,at));}
 void tile(unsigned x,unsigned y,uint16_t value){const auto map=m::Map::read(rdram);need(bool(map),"Fixture retains actual map");const unsigned tx=x/16,ty=y/16,pi=(ty/16)*map->columns+tx/16,page=n::byte(rdram,map->pages+pi);need(page!=0xFF,"Fixture modifies a real mapped page");h(map->data+2*((ty%16)*map->stride+page*16+tx%16),value);}
};
int main(int argc,char** argv){try{
 need(argc==2,"Pass actual a78 controller-test-3600.rdram");std::ifstream file(argv[1],std::ios::binary);const std::vector<uint8_t> source((std::istreambuf_iterator<char>(file)),{});need(source.size()==0x800000,"Actual a78 snapshot is exactly8MiB");Memory real(source);
 need(n::actor(real.rdram,real.player,0x4001)&&std::abs(real.f(real.player+0x84)-72.625f)<.001f&&real.f(real.player+0x88)==193,"Actual stuck pose isX72.625/Y193");
 const auto map=m::Map::read(real.rdram);need(map&&map->columns==2&&map->rows==1,"Actual owned Lips bitmap remains2x1");
 need(real.f(real.xs+4)==-4&&real.f(real.xs+8)==4&&real.f(real.ys+4)==-14&&real.f(real.ys+8)==-14,"Actual native terrain head probes are±4/-14 world offsets");
 const auto wall=map->occupied(real.rdram,63,174),head=map->occupied(real.rdram,68,174);need(wall&&*wall&&head&&!*head,"Actual wall63/174 is solid while native head column68 is free");
 const auto before=real.bytes;j::Debug debug;const auto clear=j::observe(real.rdram,real.player,40,468,140,&debug);
 need(clear&&clear->current_clear&&clear->x==72.625f&&debug.reason==j::Reason::CurrentClear,"Actual a78 pose permits vertical jump without walking into the boss");
 need(clear->half_width==4&&clear->half_height==14&&debug.body_observed&&debug.half_width==4&&debug.half_height==14,"Stable diagnostic fields describe native head-probe envelope");need(real.bytes==before,"Actual clearance leaves all8MiB unchanged");
 {Memory q(source);for(unsigned x=0;x<=128;x+=16)q.tile(x,160,0x0601);const auto before=q.bytes;need(!j::observe(q.rdram,q.player,40,468,140)&&q.bytes==before,"Solid ceiling across every nearby candidate cannot license jump");}
 {Memory q(source);q.tile(68,160,0x0680);const auto before=q.bytes;need(!j::observe(q.rdram,q.player,40,468,140)&&q.bytes==before,"Unknown ceiling profile fails closed without fallback jump");}
 {Memory q(source);q.tile(68,160,0x0601);const auto before=q.bytes;const auto result=j::observe(q.rdram,q.player,40,468,140);need((!result||!result->current_clear)&&q.bytes==before,"Blocked current head column never reports immediate A permission");if(result){q.fl(q.player+0x84,result->x);const auto moved=j::observe(q.rdram,q.player,40,468,140);need(moved&&moved->current_clear,"Requested nearby column is independently proved after new position observation");}}
 {Memory q(source);q.w(q.definition+0x70,0);q.h(q.definition+0xA,1000);q.h(q.definition+0xC,0);const auto before=q.bytes;const auto result=j::observe(q.rdram,q.player,40,468,140);need(result&&result->current_clear&&q.bytes==before,"Object collider scales/extents do not replace terrain head probes");}
 {Memory q(source);q.by(q.player+0x164,0);q.w(q.player+0x16C,0x80780000);q.w(0x80780000,0x80780100);q.w(q.player+0x138,0);q.w(0x80780100,1);const auto before=q.bytes;const auto result=j::observe(q.rdram,q.player,40,468,140);need(result&&result->current_clear&&q.bytes==before,"Native49018 mirrored orientation preserves symmetric head clearance");}
 {Memory q(source);q.by(q.player+0x164,7);q.fl(q.xs+4,-9.625f);const auto before=q.bytes;const auto result=j::observe(q.rdram,q.player,40,468,140);need((!result||!result->current_clear)&&q.bytes==before,"Actual terrain table offsets are read rather than hardcoded±4");}
 for(unsigned fault=0;fault<21;++fault){Memory q(source);switch(fault){
 case 0:q.w(q.player+0x174,0);break;case 1:q.w(q.metadata,0);break;case 2:q.w(q.definition+0x30,0);break;case 3:q.w(q.definition+0x34,0);break;
 case 4:q.w(q.definition+0x30,0x807FFFFC);break;case 5:q.w(q.definition+0x34,q.ys+1);break;case 6:q.w(q.xs+4,0x7FC00000);break;case 7:q.w(q.ys+8,0x7F800000);break;
 case 8:q.fl(q.ys+4,0);break;case 9:q.fl(q.ys+8,-65);break;case 10:q.fl(q.xs+4,65);break;case 11:q.fl(q.xs+4,4);break;
 case 12:q.w(q.player+0x84,0x7FC00000);break;case 13:q.w(q.player+0x88,0x7F800000);break;case 14:q.w(0x800F8828,0);break;case 15:q.w(0x800F8810,0);break;
 case 16:q.by(q.player+0x164,0);q.w(q.player+0x16C,0x807FFFFE);break;case 17:q.by(q.player+0x164,0);q.w(q.player+0x16C,0x80780000);q.w(0x80780000,0x807FFFFC);q.w(q.player+0x138,3);break;
 case 18:q.w(q.player+0x174,q.metadata+1);break;case 19:q.w(q.metadata,q.definition+1);break;case 20:q.by(q.player+0x164,0);q.w(q.player+0x16C,0x80780000);q.w(0x80780000,0x80780100);q.w(q.player+0x138,0xFFFFFFFF);break;
 }const auto before=q.bytes;need(!j::observe(q.rdram,q.player,40,468,140)&&q.bytes==before,"Malformed player/probe/orientation/map fails closed and remains unchanged");}
 for(const auto bounds:std::array<std::array<float,3>,6>{{{73,468,140},{40,72,140},{468,40,140},{40,468,-1},{40,468,0},{40,468,std::numeric_limits<float>::quiet_NaN()}}})need(!j::observe(real.rdram,real.player,bounds[0],bounds[1],bounds[2]),"Invalid/out-of-arena/excessive-rise requests remain rejected");
 need(!j::observe(nullptr,real.player,40,468,140),"Null RAM remains rejected");
 std::printf("BossLipsClearance0260Checks: %u PASS; actualA78WallPoseClear=1; fullRamReadOnly=1; physicalJumpGpuVerified=0\n",checks);return 0;
}catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}}
