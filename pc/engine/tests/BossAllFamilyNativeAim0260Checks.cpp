// Driver input -> original host conversion -> actual native22B20 for every
// egg-aimed boss family. Reticle rays below are arithmetic, not physical4003.
#include "../boss_driver0260.hpp"
#include "recomp.h"
#include "BossNativeStick0260Source.hpp"
#include <array>
#include <bit>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <vector>
namespace d=workshop::boss_driver0260;namespace s=workshop::boss_stick_aim0260;namespace native=native_stick_source0260;
static unsigned checks=0,frames=0,cases=0;static void need(bool b,const char*why){++checks;if(!b){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
namespace native_stick_source0260{void func_80022AC0(uint8_t*,recomp_context*){}}
struct Math{
 std::vector<uint8_t> data=std::vector<uint8_t>(0x800000);uint8_t*rdram=data.data();
 explicit Math(const std::vector<uint8_t>& captured){for(auto p:std::array<std::array<uint32_t,2>,4>{{{0x800AA3B0,2048},{0x800AFC98,8},{0x800B0F04,16},{0x800B3180,24}}})std::copy_n(captured.data()+(p[0]&0x7fffff),p[1],data.data()+(p[0]&0x7fffff));MEM_W(0,int32_t(0x800F7EA8))=int32_t(0x80010000);}
 s::Axes apply(const d::Output&o,s::Axes old){int8_t x=0,y=0;native::convert_to_n64_range(o.x,o.y,x,y);const auto expected=s::convert({o.x,o.y});need(x==expected.x&&y==expected.y,"every-family driver input matches original host conversion");MEM_B(2,int32_t(0x80010000))=x;MEM_B(3,int32_t(0x80010000))=y;MEM_H(0,int32_t(0x800F897E))=old.x;MEM_H(0,int32_t(0x800F8980))=old.y;recomp_context c{};c.r29=int32_t(0x807B0000);c.r31=int32_t(0x80000300);native::func_80022B20(rdram,&c);++frames;need(uint32_t(c.r29)==0x807B0000&&uint32_t(c.r31)==0x80000300,"actual22B20 stack/RA preserved");return {MEM_H(0,int32_t(0x800F897E)),MEM_H(0,int32_t(0x800F8980))};}
 float angle(){return std::bit_cast<float>(uint32_t(MEM_W(0,int32_t(0x800F8990))));}
};
int main(int argc,char**argv){need(argc==2,"immutable native math snapshot required");std::ifstream f(argv[1],std::ios::binary);need(bool(f),"captured math source opens");const std::vector<uint8_t> captured((std::istreambuf_iterator<char>(f)),{});need(captured.size()==0x800000,"exact8MiB native constants");Math math(captured);
 for(uint16_t kind:std::array<uint16_t,7>{0x4599,0x4072,0x40A3,0x41C6,0x4511,0x4631,0x455A})for(const auto geom:std::array<std::array<float,2>,3>{{{240,47},{-170,110},{200,0}}})for(const auto old:std::array<s::Axes,3>{{{0,0},{54,-54},{-54,54}}}){
  ++cases;d::Controller c;need(c.configure(true,true,{},false),"isolated diagnostic input generator");std::array<d::Target,1> t{{{0x80110000,kind,312+geom[0],193-geom[1],0,0,true,true,false,kind==0x455A,true,6.6f,2}}};d::Input in;in.player=0x80100000;in.room=168;in.encounter=1;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.reserves=6;in.x=312;in.y=193;in.min_x=40;in.max_x=724;in.has_preferred_x=true;in.preferred_x=312;in.native_stick_observed=true;in.native_stick=old;in.targets=t;in.vi=0;c.step(in);in.vi=4;c.step(in);in.aiming_actor=0x80120000;in.reticle_actor=0x80121000;float error=1e9;
  for(unsigned vi=5;vi<17;++vi){in.vi=vi;const auto out=c.step(in);need(c.snapshot().phase==d::Phase::TrackAim&&(out.buttons&d::egg_button),"every native family acquires and holds real observed4005 until reticle confirmation");in.native_stick=math.apply(out,in.native_stick);const float angle=math.angle()*s::pi/180;error=std::abs(std::cos(angle)*(-geom[1])+std::sin(angle)*geom[0]);if(error<=2)break;}need(error<=2,"all7families converge on horizontal/shallow/steep reversed rays through actual native filter including opposite old stick");
 }
 // Unobserved legacy input remains exactly the established normalized vector.
 for(uint16_t kind:std::array<uint16_t,7>{0x4599,0x4072,0x40A3,0x41C6,0x4511,0x4631,0x455A}){d::Controller c;need(c.configure(true,true,{},false),"legacy diagnostic fixture");std::array<d::Target,1> t{{{1,kind,552,146,0,0,true,true,false,kind==0x455A,true,6.6f,2}}};d::Input in;in.player=2;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.reserves=6;in.x=312;in.y=193;in.min_x=40;in.max_x=724;in.has_preferred_x=true;in.preferred_x=312;in.targets=t;c.step(in);in.vi=4;const auto out=c.step(in);const float length=std::hypot(240.f,47.f);need(std::abs(out.x-.85f*240/length)<.000001f&&std::abs(out.y-.85f*47/length)<.000001f,"unobserved original normalized input branch is byte-behavior equivalent");}
 std::printf("BossAllFamilyNativeAim0260Checks: %u PASS; %u target/old-axis cases, %u actual22B20 calls; original host conversion and native math only, no game/GPU/collision/victory proof.\n",checks,cases,frames);
}
