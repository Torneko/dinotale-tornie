#include "../boss_monster_aim0260.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <limits>
#include <vector>

namespace m=workshop::boss_monster_aim0260;
namespace d=workshop::boss_driver0260;
namespace n=workshop::boss_native;
static unsigned checks=0;
static void need(bool value,const char* message){++checks;if(!value){std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}}
// Synthetic native-shaped memory only. No game function, input dispatcher,
// progression file, or challenge ledger is called by this executable.
struct Fixture {
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);
 uint8_t* rdram=memory.data();
 static constexpr uint32_t scene=0x80010000,list=0x80011000,room=0x80012000,resource=0x80013000,layer=0x80014000,map=0x80015000,pages=0x80016000,data=0x80500000;
 static constexpr uint32_t root=0x80100000,priv=0x80102000,root_resource=0x80110000,factory=0x80300000;
 static constexpr uint32_t eyes[2]={0x80120000,0x80121000};
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}
 void h(uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}
 void b(uint32_t a,uint8_t v){MEM_B(0,n::base(a))=v;}
 void f(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}
 void actor(uint32_t a,uint16_t kind){w(a,0x19971110);h(a+12,kind);b(a+0x18,1);b(a+0x17D,2);}
 void eye(unsigned index,float x,float y){
  const uint32_t a=eyes[index],res=0x80130000+index*0x100,meta=0x80140000+index*0x100,def=meta+0x40;
  actor(a,0x46F9);b(a+0x17D,0);w(a+0x180,root);w(a+0x168,res);w(res+0x40,factory);w(a+0x174,meta);w(meta,def);w(def+0x28,factory+0x3E08);w(def+0xC,0x00080014);f(a+0x84,x);f(a+0x88,y);
 }
 Fixture(){
  w(0x800F8810,scene);w(scene+0x10,list);w(list+4,room);w(room+0x16C,resource);w(resource+0x1B0,layer);w(layer+0x20,map);
  h(map+0x12,3);h(map+0x14,2);h(map+0x16,96);w(map+0x18,pages);w(map+0x24,data);for(unsigned i=0;i<6;++i)b(pages+i,uint8_t(i));
  for(unsigned i=0;i<128;++i)w(0x800A0A24+4*i,0x800A0604);
  w(0x800A0A28,0x800A0624);for(unsigned i=0;i<16;++i)h(0x800A0624+2*i,0xFFFF);
  actor(root,0x41C6);w(root+0x1F4,priv);w(root+0x168,root_resource);w(root_resource+0x40,factory);
  eye(0,432,420);eye(1,512,380);w(root+0x188,eyes[0]);w(root+0x188+19*4,eyes[1]);
 }
 uint32_t address(unsigned x,unsigned y)const{const unsigned tx=x/16,ty=y/16,page=(ty/16)*3+tx/16;return data+2*((ty%16)*96+page*16+tx%16);}
 void tile(unsigned x,unsigned y,uint16_t v){h(address(x,y),v);}
};
static void masks(){
 Fixture f;auto* rdram=f.rdram;
 const auto map=m::Map::read(f.rdram);need(bool(map),"paged native map is identified");
 for(unsigned shape=0;shape<128;++shape){
  const uint32_t profile=0x800A0604+32*(shape%32);
  f.w(0x800A0A24+4*shape,profile);
  for(unsigned row=0;row<16;++row){const uint16_t mask=shape==0?0:shape==1?0xFFFF:uint16_t((0x9E37u*(shape+1)+0x45D9u*row)^(0x8000u>>row));f.h(profile+2*row,mask);}
  f.tile(320,400,uint16_t(0x0700|shape));const auto before=f.memory;
  for(unsigned y=0;y<16;++y)for(unsigned x=0;x<16;++x){
   const uint16_t row=MEM_HU(2*y,n::base(profile));const auto actual=map->occupied(f.rdram,320.f+x,400.f+y);
   need(actual.has_value()&&*actual==bool(row&(0x8000u>>x)),"each legal profile resolves its native row and bit");
  }
  need(f.memory==before,"all256pixel observations preserve every byte of8MiB");
 }
}
static void geometry(){
 Fixture f;auto* rdram=f.rdram;const auto map=m::Map::read(f.rdram);need(bool(map),"geometry fixture map valid");
 // Native shape48: the first half is empty; a widening triangle occupies
 // the lower half. It must not become one solid16x16 square.
 f.w(0x800A0A24+4*0x48,0x800A0724);
 for(unsigned row=0;row<16;++row)f.h(0x800A0724+2*row,row<8?0:uint16_t(0xFFFFu<<(14-2*(row-8))));
 f.tile(320,400,0x0648);const auto before=f.memory;
 for(unsigned row=0;row<16;++row)for(unsigned x=0;x<16;++x){
  const auto got=map->occupied(f.rdram,320.f+x,400.f+row);const uint16_t mask=MEM_HU(2*row,n::base(0x800A0724));
  need(got.has_value()&&*got==bool(mask&(0x8000u>>x)),"slope silhouette remains pixel-accurate");
 }
 need(map->clear(f.rdram,300,402,350,402,0),"ray through empty upper slope remains clear");
 need(!map->clear(f.rdram,300,415,350,415,0),"ray crossing actual lower slope is blocked");
 need(f.memory==before,"slope/ray operations are entirely read-only");
 for(unsigned material:{0u,0xFFFFu,0x3801u,0x3F01u,0x4001u,0x4701u}){
  f.tile(320,400,uint16_t(material));const auto saved=f.memory;
  need(map->occupied(f.rdram,320,408)==false,"native excluded materials do not obstruct LOS");need(f.memory==saved,"excluded-material lookup never edits memory");
 }
 // A parallel ray3px away hits a full tile while its centre line is clear.
 f.tile(320,400,0);f.tile(320,384,0x0701);
 need(map->clear(f.rdram,300,402,350,402,0),"centre ray alone misses nearby solid edge");
 need(!map->clear(f.rdram,300,402,350,402,3),"native egg-width margin rejects edge clipping");
 need(!map->clear(f.rdram,300,402,350,402,-1)&&!map->clear(f.rdram,300,402,350,402,5),"invalid margin fails closed");
 f.b(Fixture::pages,0xFF);need(map->tile(f.rdram,30,30)==0,"native FF page is empty");
 f.b(Fixture::pages,6);need(!map->tile(f.rdram,30,30),"page index beyond row stride is rejected");
}
static void negatives(){
 for(unsigned fault=0;fault<24;++fault){Fixture f;
  if(fault==0)f.w(0x800F8810,0);if(fault==1)f.w(0x800F8810,0x80010002);
  if(fault==2)f.w(Fixture::scene+0x10,0x807FFFFC);if(fault==3)f.w(Fixture::list+4,0);if(fault==4)f.w(Fixture::room+0x16C,0);
  if(fault==5)f.w(Fixture::resource+0x1B0,0);if(fault==6)f.w(Fixture::layer+0x20,0);
  if(fault==7)f.h(Fixture::map+0x12,0);if(fault==8)f.h(Fixture::map+0x14,0);if(fault==9)f.h(Fixture::map+0x12,9);if(fault==10)f.h(Fixture::map+0x14,9);
  if(fault==11)f.h(Fixture::map+0x16,0);if(fault==12)f.h(Fixture::map+0x16,95);if(fault==13)f.h(Fixture::map+0x16,1040);
  if(fault==14)f.w(Fixture::map+0x18,0x807FFFFF);if(fault==15)f.w(Fixture::map+0x24,0);if(fault==16)f.w(Fixture::map+0x24,0x80500002);if(fault==17)f.w(Fixture::map+0x24,0x807FFFF0);
  const auto before=f.memory;
  if(fault<18)need(!m::Map::read(f.rdram),"corrupt scene chain or dimensions reject map before sampling");
  else{const auto map=m::Map::read(f.rdram);need(bool(map),"pixel-negative case starts from valid map");f.tile(320,400,0x0701);
   if(fault==18)f.tile(320,400,0x0780);if(fault==19)f.w(0x800A0A28,0);if(fault==20)f.w(0x800A0A28,0x800A0608);if(fault==21)f.w(0x800A0A28,0x800A0A24);if(fault==22)f.w(0x800A0A28,0x807FFFFC);if(fault==23)f.w(0x800A0A28,0x800A05E4);
   const auto pixel_before=f.memory;need(!map->occupied(f.rdram,324,404)&&!map->clear(f.rdram,324,404,350,400),"bad shape or profile pointer rejects ray without dereferencing arbitrary memory");need(f.memory==pixel_before,"rejected pixel observation leaves all8MiB untouched");continue;
  }
  need(f.memory==before,"map rejection has no mutation side effects");
 }
 Fixture f;const auto map=m::Map::read(f.rdram);const auto before=f.memory;
 for(auto point:std::array<std::array<float,2>,7>{{{-1,0},{0,-1},{768,0},{0,512},{std::numeric_limits<float>::quiet_NaN(),400},{320,std::numeric_limits<float>::infinity()},{100000,400}}})
  need(!map->tile(f.rdram,point[0],point[1])&&!map->clear(f.rdram,point[0],point[1],400,400),"nonfinite/out-of-map endpoints fail closed");
 need(f.memory==before,"all invalid-coordinate checks preserve memory");need(!m::Map::read(nullptr),"null RDRAM is rejected");
}
static void two_eyes(){
 Fixture f;f.tile(432,416,0x0701);const auto before=f.memory;m::Eye eye;
 need(m::observe_eye(f.rdram,Fixture::root,Fixture::priv,17,Fixture::eyes[0],eye)&&eye.x==432,"first genuine native eye observed");
 need(m::observe_eye(f.rdram,Fixture::root,Fixture::priv,17,Fixture::eyes[1],eye)&&eye.x==512,"second genuine native eye observed through sparse slot19");
 m::History history;d::Input in;in.x=400;in.y=380;
 auto target=m::choose(f.rdram,history,Fixture::root,Fixture::priv,17,169,2,100,in,true);
 need(target&&target->clear_path&&target->x==512&&target->y==380,"farther clear second eye outranks nearer occluded first eye");
 need(target->kind==0x41C6&&target->actor==Fixture::root&&target->native_speed==6.6f&&target->aim_radius==2,"selected eye preserves typed root and exact native aiming constants");
 need(f.memory==before,"choosing either eye never mutates actors/bitmap/native HP");
 f.tile(496,368,0x0701);f.tile(496,384,0x0701);const auto blocked_before=f.memory;
 target=m::choose(f.rdram,history,Fixture::root,Fixture::priv,17,169,2,101,in,true);
 need(target&&!target->clear_path,"all blocked eyes remain explicit navigation targets rather than becoming shootable");need(f.memory==blocked_before,"blocked-target selection is read-only");
 for(unsigned fault=0;fault<10;++fault){Fixture bad;m::Eye out{0x80700000};const uint32_t child=Fixture::eyes[0];
  if(fault==0)bad.w(child+0x180,Fixture::root+4);if(fault==1)bad.h(child+12,0x4002);if(fault==2)bad.b(child+6,3);if(fault==3)bad.b(child+0x18,0xFF);
  if(fault==4)bad.w(child+0x168,0x807FFFFC);if(fault==5)bad.w(child+0x174,0x80140002);if(fault==6)bad.w(0x80140000,0);
  if(fault==7)bad.w(0x80140040+0x28,Fixture::factory+0x3E04);if(fault==8)bad.w(0x80140040+0xC,0x00080010);if(fault==9)bad.w(0x80130000+0x40,Fixture::factory+4);
  const auto saved=bad.memory;need(!m::observe_eye(bad.rdram,Fixture::root,Fixture::priv,17,child,out)&&out.actor==0x80700000,"wrong eye identity/callback/shape cannot publish partial output");need(saved==bad.memory,"eye rejection preserves all native bytes");
 }
}
static void native_snapshot(const char* path){
 std::ifstream file(path,std::ios::binary);need(bool(file),"explicit native snapshot path opens");std::vector<uint8_t> memory((std::istreambuf_iterator<char>(file)),{});need(memory.size()==0x800000,"captured native snapshot is exactly8MiB");const auto before=memory;auto* rdram=memory.data();const auto map=m::Map::read(rdram);
 need(map&&map->columns==3&&map->rows==2&&map->stride==96,"8e65 native scene/map chain agrees with independent source audit");
 for(auto start:std::array<std::array<float,2>,4>{{{312,390},{312,400},{472,390},{472,400}}})for(auto endpoint:std::array<std::array<float,2>,4>{{{492,440},{483,440},{458,441},{449,441}}})
  need(!map->clear(rdram,start[0],start[1],endpoint[0],endpoint[1]),"all16original-platform rays remain correctly blocked in real failed snapshot");
 need(map->occupied(rdram,458,441)==true&&map->occupied(rdram,449,441)==true,"actual second-root eye pixels are inside native full bricks");
 need(!map->clear(rdram,436.617584f,465,492,440),"actual final P1-to-eye counterexample remains blocked");
 for(float x:{496.f,520.f})for(auto endpoint:std::array<std::array<float,2>,2>{{{492,440},{483,440}}})need(map->clear(rdram,x,465,endpoint[0],endpoint[1]),"independently identified alternate right-side ray remains available with egg margin");
 for(const auto root:{0x800CA434u,0x800CA92Cu}){
  const auto priv=n::word(rdram,root+0x1F4);unsigned eyes=0;for(unsigned i=0;i<20;++i){m::Eye e;if(m::observe_eye(rdram,root,priv,17,n::word(rdram,root+0x188+4*i),e))++eyes;}need(eyes==2,"real native root exposes both typed46F9eyes");
 }
 m::History history;d::Input input;input.x=520;input.y=465;auto target=m::choose(rdram,history,0x800CA434,n::word(rdram,0x800CA434+0x1F4),17,169,2,15000,input,true);
 need(target&&target->clear_path&&target->x==492&&target->y==440,"helper selects genuine clear native eye from independent alternate origin");
 need(memory==before,"all real-snapshot observations preserve the captured8MiB byte for byte");
 std::printf("Captured8e65 RAM verified read-only; no actual game execution or hit/victory claimed.\n");
}
int main(int argc,char** argv){masks();geometry();negatives();two_eyes();if(argc>1)native_snapshot(argv[1]);else std::printf("Optional real-snapshot checks not requested; fixtures only.\n");std::printf("BossMonsterRay0260Checks: %u PASS (CPU observations only; no native hits/victory/ledger writes)\n",checks);}
