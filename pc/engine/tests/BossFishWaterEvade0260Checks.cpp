#include "../boss_fish_water_evade0260.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>
namespace q=workshop::boss_fish_water_evade0260;namespace n=workshop::boss_native;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static std::vector<uint8_t> load(const char* path){std::ifstream file(path,std::ios::binary);need(bool(file),"Actual Fish snapshot opens");std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),{});need(bytes.size()==0x800000,"Snapshot is exactly8MiB");return bytes;}
struct Fixture {
 std::vector<uint8_t> memory;uint8_t* rdram;uint32_t player=0,p=0,meta=0,def=0,table=0,water=0,resource=0,data=0,wd=0,factory=0;
 explicit Fixture(const std::vector<uint8_t>& source,uint32_t actor):memory(source),rdram(memory.data()),water(actor){player=n::word(rdram,0x800F8828);p=n::word(rdram,player+0x1F4);meta=n::word(rdram,player+0x174);def=n::word(rdram,meta);table=n::word(rdram,0x800F8824);resource=n::word(rdram,water+0x168);data=n::word(rdram,water+0x1F4);wd=n::word(rdram,n::word(rdram,water+0x174));factory=n::word(rdram,resource+0x40);}
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=int32_t(v);}void h(uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}void by(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}void fl(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}
 q::Identity id()const{return {water,resource,data};}
 void only(){w(table,water);h(0x800F881C,1);}
 q::Threat read(const q::Identity* old=nullptr,unsigned room=171,unsigned index=4){const auto original=memory;const auto result=q::observe(rdram,player,room,index,old);need(memory==original,"Every observation preserves all8MiB bytes");return result;}
 void at(float x,float y,float z=500.34f){fl(water+0x84,x);fl(water+0x88,y);fl(water+0x8C,z);}
 void player_at(float x,float y,float vx=0,float vy=0){fl(player+0x84,x);fl(player+0x88,y);fl(player+0xA8,x-vx);fl(player+0xAC,y-vy);}
 uint32_t tile_address(unsigned x,unsigned y){const auto map=q::m::Map::read(rdram);need(bool(map),"Actual native Fish bimap is available");const unsigned tx=x/16,ty=y/16,page=n::byte(rdram,map->pages+(ty/16)*map->columns+tx/16);return map->data+2*((ty%16)*map->stride+page*16+tx%16);}
};
int main(int argc,char** argv){try{
 need(argc==3,"Pass actualb2 and2c Fish captures");const auto b2=load(argv[1]),two=load(argv[2]);
 {Fixture f(b2,0x800CC204);const auto out=f.read();need(out.observed&&out.imminent&&out.head_clear&&out.actor==f.water&&out.step==0,"Actualb2 unlatched4632 already overlaps the exact native P1 volume");need(q::same(out.identity,f.id()),"Reported identity is the actual actor/resource/private triple");const auto projectile=q::water(f.rdram,f.water);need(projectile&&projectile->box.half==std::array<float,3>{12,6,10}&&projectile->velocity[0]<-5&&projectile->velocity[1]<-3,"Reader consumes actual4632 scale1/.5/1 and483B8 movement");}
 {Fixture f(two,0x800CC204);const auto old=f.id();const auto out=f.read(&old);need(out.observed&&!out.imminent&&out.previous_valid,"Actual2c remaining unlatched projectile moves away; no invented new threat");}
 {Fixture f(two,0x800CCBF4);const auto old=f.id();const auto out=f.read(&old);need(out.observed&&!out.imminent&&!out.previous_valid,"Actual2c overlapping projectile has already latched private8 and cannot rearm A");}
 {Fixture f(b2,0x800CC204);f.only();const auto old=f.id();const auto out=f.read(&old);need(out.previous_valid&&out.imminent,"Live exact identity permits bounded hold continuation");f.w(f.data+8,1);const auto latched=f.read(&old);need(latched.observed&&!latched.imminent&&!latched.previous_valid,"Native contact latch immediately revokes continuation");}
 {Fixture f(b2,0x800CC204);f.only();f.player_at(360,273);f.at(410,273);f.fl(f.water+0xC4,-7);f.fl(f.water+0xC8,0);const auto out=f.read();need(out.observed&&out.imminent&&out.step==4,"ExplicitHOST precontact frontier is detected four current-motion updates ahead");f.at(410,273,550);need(!f.read().imminent,"Native depth separation excludes a two-dimensional false contact");}
 {Fixture f(b2,0x800CC204);f.only();f.player_at(350,273,2,0);f.at(400,273);f.fl(f.water+0xC4,0);f.fl(f.water+0xC8,0);const auto out=f.read();need(out.observed&&out.imminent&&out.step==14,"Sweep includes P1's actual last resolved WORLD displacement");f.fl(f.player+0xA8,350);need(!f.read().imminent,"Same boxes with stationary P1 remain separated for the bounded horizon");}
 {Fixture f(b2,0x800CC204);f.only();auto old=f.id();old.resource+=4;need(!f.read(&old).previous_valid,"Foreign resource cannot continue a previous water gesture");old=f.id();old.private_data+=4;need(!f.read(&old).previous_valid,"Foreign private allocation cannot continue a previous gesture");old=f.id();old.actor+=4;need(!f.read(&old).previous_valid,"Foreign actor cannot continue a previous gesture");f.w(f.table,0);need(!f.read(&old).previous_valid,"An old address missing from the live queue is never sufficient evidence");}
 for(unsigned primary:{1u,3u,4u}){Fixture f(b2,0x800CC204);f.by(f.player+0x18,primary);need(f.read().observed,"Reader permits observation in the existing normal/aim/throw postures; consumer owns A admission");}
 for(unsigned fault=0;fault<28;++fault){Fixture f(b2,0x800CC204);f.only();switch(fault){
  case 0:f.w(0x800F8828,0);break;case 1:f.h(f.player+12,0x4002);break;case 2:f.by(f.player+0x165,1);break;case 3:f.w(f.player+0x180,f.water);break;case 4:f.by(f.player+0x18,10);break;
  case 5:f.w(f.def+0x70,f.player+0x6C);break;case 6:f.h(f.def+0xA,15);break;case 7:f.w(f.meta+4,f.water);break;case 8:f.w(f.player+0x130,0);break;case 9:f.fl(f.player+0xA8,std::numeric_limits<float>::infinity());break;
  case 10:f.h(0x800F881C,151);break;case 11:f.w(0x800F8824,0x807FFFFC);break;case 12:f.w(f.resource+0x44,f.factory+0x10);break;case 13:f.w(f.factory+8,0);break;case 14:f.w(f.wd+0x28,0);break;
  case 15:f.w(f.water+0x20,f.factory+0x10);break;case 16:f.w(f.water+0x130,0);break;case 17:f.w(f.water+0x180,f.player);break;case 18:f.by(f.water+0x165,1);break;case 19:f.by(f.water+0x30,1);break;
  case 20:f.h(f.wd+0xA,11);break;case 21:f.h(f.wd+0x16,0);break;case 22:f.w(f.wd+0x70,0);break;case 23:f.fl(f.water+0x70,0);break;case 24:f.fl(f.water+0xC4,std::numeric_limits<float>::quiet_NaN());break;
  case 25:f.fl(f.water+0x100,0);break;case 26:f.w(f.factory+0xF70,0);break;case 27:f.h(f.resource+0x3C,0x4526);break;
  }need(!f.read().observed,"Malformed player/queue/factory/source/profile/motion refuses water observation");}
 {Fixture f(b2,0x800CC204);f.only();const auto old=f.id();f.by(f.water+0x17D,0);const auto out=f.read(&old);need(out.observed&&!out.imminent&&!out.previous_valid,"Retired projectile cannot license an A gesture");}
 {Fixture f(b2,0x800CC204);f.w(f.meta+8,0xF000);const auto out=f.read();need(out.imminent&&!out.head_clear,"Native overhead contact blocks A without hiding the observed threat");}
 for(unsigned tile:{0x701u,0x780u}){Fixture f(b2,0x800CC204);f.h(f.tile_address(368,224),uint16_t(tile));const auto out=f.read();need(out.observed&&out.imminent&&!out.head_clear,"Solid or unknown head-probe column refuses ordinary A");}
 {Fixture f(b2,0x800CC204);f.fl(n::word(f.rdram,f.def+0x30)+4,std::numeric_limits<float>::quiet_NaN());need(!f.read().observed,"Malformed native head-probe coordinate fails closed");}
 {Fixture f(b2,0x800CC204);need(!f.read(nullptr,170,4).observed&&!f.read(nullptr,171,9).observed,"Foreign physical/index domain is inert");need(!q::observe(nullptr,f.player,171,4).observed,"Null RAM is inert");}
 std::cout<<"BossFishWaterEvade0260Checks: "<<checks<<" PASS; actual8MiB captures + explicitHOST risk frontiers; read-only, no future damage/landing claim\n";return 0;
 }catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
