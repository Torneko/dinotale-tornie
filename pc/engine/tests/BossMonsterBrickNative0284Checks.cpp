#include "../boss_monster_aim0260.hpp"
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#ifndef MONSTER_BRICK_NATIVE0284
#error Supply unchanged native brick stamping and coordinate helpers.
#endif
static int32_t section_storage[228]{};
int32_t* section_addresses=section_storage;
extern "C" void do_break(uint32_t){std::abort();}
namespace actual {
void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected native map assertion");}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#include MONSTER_BRICK_NATIVE0284
#pragma clang diagnostic pop
}
namespace n=workshop::boss_native;namespace m=workshop::boss_monster_aim0260;
static unsigned checks=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
struct Ram {
 std::vector<uint8_t> bytes=std::vector<uint8_t>(0x800000);uint8_t* ram=bytes.data();uint8_t* rdram=ram;
 static constexpr uint32_t scene=0x80010000,table=0x80011000,room=0x80012000,res=0x80013000,layer=0x80014000,map=0x80015000,pages=0x80016000,data=0x80020000,actor=0x80100000,stack=0x807A0000;
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}void h(uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}void b(uint32_t a,uint8_t v){MEM_B(0,n::base(a))=v;}void f(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}
 Ram(){w(0x800F8810,scene);w(scene+0x10,table);w(table+4,room);w(room+0x16C,res);w(res+0x1B0,layer);w(layer+0x20,map);h(map+0x12,3);h(map+0x14,2);h(map+0x16,96);w(map+0x18,pages);w(map+0x24,data);for(unsigned i=0;i<6;++i)b(pages+i,uint8_t(i));for(unsigned i=0;i<128;++i)w(0x800A0A24+4*i,0x800A0604);w(0x800A0A28,0x800A0624);for(unsigned i=0;i<16;++i)h(0x800A0624+2*i,0xFFFF);w(actor,0x19971110);h(actor+12,0x464F);}
 void stamp(float x,float y){f(actor+0x84,x);f(actor+0x88,y);recomp_context c{};c.r4=n::base(actor);c.r29=n::base(stack);actual::func_806F5C20(ram,&c);need(uint32_t(c.r29)==stack,"Actual native stamp restores stack");need(n::word(ram,actor+0x20)==0x806F5BE0,"Actual stamp installs original native next callback");}
};
int main(){try{
 section_storage[227]=int32_t(0x806F5360u);
 for(const auto xyz:std::array<std::array<unsigned,4>,3>{{{248,344,224,320},{392,360,368,336},{536,344,512,320}}}){
  Ram r;r.stamp(float(xyz[0]),float(xyz[1]));auto map=m::Map::read(r.ram);need(bool(map),"Engine reader accepts real-shaped native paged map");
  for(unsigned y=0;y<512;y+=16)for(unsigned x=0;x<768;x+=16){const bool inside=x>=xyz[2]&&x<xyz[2]+48&&y>=xyz[3]&&y<xyz[3]+48;const auto value=map->tile(r.ram,float(x),float(y));need(value&&*value==(inside?0x5F01:0),"Actual unchanged464F code stamps exactly3x3 native solid tiles");}
  for(float dx:{-4.f,0.f,4.f}){const float x=float(xyz[0])+dx;need(map->occupied(r.ram,x,float(xyz[3]-1))==false&&map->occupied(r.ram,x,float(xyz[3]))==true&&map->occupied(r.ram,x,float(xyz[3]+1))==true,"World foot probes observe genuine surface independent of render scale");}
  const auto before=r.bytes;map->occupied(r.ram,float(xyz[0]),float(xyz[3]));need(before==r.bytes,"Engine bitmap observation leaves all8MiB unchanged");
 }
 for(int x:{-33,-32,-17,-16,-1,0,15,16,32767}){Ram r;recomp_context c{};c.r4=n::base(uint32_t(x));c.r29=n::base(Ram::stack);actual::func_8006A99C(r.ram,&c);const int expected=x>=0?x/16:-((-x+15)/16);need(int32_t(c.r2)==expected,"Actual native signed coordinate helper floors16px cells including negatives");}
 std::printf("BossMonsterBrickNative0284Checks: %u PASS;8 unchanged native bodies; HOST map/actor fixture, no constructor, physics, landing, engine or GPU claim\n",checks);return 0;
 }catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}}
