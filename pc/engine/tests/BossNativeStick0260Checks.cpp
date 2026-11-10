// Executes the real recompiled22A58/22B20, native atan continued fraction,
// native integer sine/cosine lookup, and original host-pad conversion.
// Only22AC0's facing notification is stubbed. No game/ledger function is run.
#include "recomp.h"
#include "../boss_stick_aim0260.hpp"
#include <array>
#include <bit>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <limits>
#include <vector>
#include "BossNativeStick0260Source.hpp"
namespace s=workshop::boss_stick_aim0260;
namespace native=native_stick_source0260;
static unsigned checks=0,frames=0,facing_calls=0;
static void need(bool value,const char* why){++checks;if(!value){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
namespace native_stick_source0260 {void func_80022AC0(uint8_t*,recomp_context*){++facing_calls;}}
struct Fixture {
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* rdram=memory.data();
 static constexpr uint32_t pad=0x80010000,stack=0x807B0000;
 void w(uint32_t a,uint32_t v){MEM_W(0,int32_t(a))=v;}
 void h(uint32_t a,int v){MEM_H(0,int32_t(a))=uint16_t(v);}
 int half(uint32_t a){return MEM_H(0,int32_t(a));}
 float scalar(uint32_t a){return std::bit_cast<float>(uint32_t(MEM_W(0,int32_t(a))));}
 explicit Fixture(const std::vector<uint8_t>& captured){
  // Exact immutable native constants and1024-entry signed sine table from
  // the explicitly supplied captured RAM. Never overwrite that source file.
  for(const auto part:std::array<std::array<uint32_t,2>,4>{{{0x800AA3B0,2048},{0x800AFC98,8},{0x800B0F04,16},{0x800B3180,24}}}){
   const unsigned offset=part[0]&0x7FFFFF;std::copy_n(captured.data()+offset,part[1],memory.data()+offset);
  }
  w(0x800F7EA8,pad);
  need(uint32_t(MEM_W(0,int32_t(0x800AFC98)))==0x40490FD8,"real captured native degree conversion constant retained");
  need(uint32_t(MEM_W(0,int32_t(0x800B0F04)))==0x433609F5&&uint32_t(MEM_W(0,int32_t(0x800B0F08)))==0x38000100,"native phase and sine scaling constants retained");
 }
 s::Axes frame(s::Axes raw,s::Axes old){
  MEM_B(2,int32_t(pad))=uint8_t(raw.x);MEM_B(3,int32_t(pad))=uint8_t(raw.y);
  h(0x800F897E,old.x);h(0x800F8980,old.y);h(0x800F8982,-17);h(0x800F8984,12);
  recomp_context ctx{};ctx.r29=int32_t(stack);ctx.r31=int32_t(0x80000300);ctx.r16=0x12345678;ctx.f20.u64=0x400123456789ABCDull;
  native::func_80022B20(rdram,&ctx);++frames;
  need(uint32_t(ctx.r29)==stack&&uint32_t(ctx.r16)==0x12345678&&uint32_t(ctx.r31)==0x80000300,"actual native stack/S0/RA preserved");
  need(ctx.f20.u64==0x400123456789ABCDull,"actual native atan preserves callee-saved F20");
  need(half(0x800F8982)==s::filter(raw.x,-17)&&half(0x800F8984)==s::filter(raw.y,12),"secondary native filter channels use their own old values");
  const auto result=s::Axes{half(0x800F897E),half(0x800F8980)};
  need(std::isfinite(scalar(0x800F8990))&&std::abs(scalar(0x800F8990))<=180.01f,"native angle is finite and bounded");
  return result;
 }
 void same(s::Axes raw,s::Axes old,const char* why){const auto actual=frame(raw,old),model=s::native_axes(raw,old);if(actual.x!=model.x||actual.y!=model.y)std::fprintf(stderr,"raw=%d,%d old=%d,%d actual=%d,%d model=%d,%d\n",raw.x,raw.y,old.x,old.y,actual.x,actual.y,model.x,model.y);need(actual.x==model.x&&actual.y==model.y,why);}
};
static s::Axes actual_convert(s::Host host){int8_t x=0,y=0;native::convert_to_n64_range(host.x,host.y,x,y);return {int(x),int(y)};}
static void conversion(){
 for(int x=-125;x<=125;++x)for(int y=-125;y<=125;++y){const s::Host host{float(x)/100,float(y)/100};const auto actual=actual_convert(host),model=s::convert(host);if(actual.x!=model.x||actual.y!=model.y)std::fprintf(stderr,"host=%g,%g actualraw=%d,%d modelraw=%d,%d\n",host.x,host.y,actual.x,actual.y,model.x,model.y);need(actual.x==model.x&&actual.y==model.y,"helper pad conversion matches exact runtime source including octagonal truncation");}
 for(int x=-82;x<=82;++x)for(int y=-82;y<=82;++y)if(auto host=s::inverse_raw({x,y})){const auto actual=actual_convert(*host);need(actual.x==x&&actual.y==y,"inverse raw round-trips through actual runtime conversion");need(std::hypot(host->x,host->y)<=1.00001f,"inverse raw host magnitude remains bounded");}
 for(s::Axes invalid:std::array<s::Axes,6>{{{-83,0},{83,0},{0,-83},{0,83},{1000000,0},{0,-1000000}}})need(!s::inverse_raw(invalid),"out-of-range inverse raw fails closed");
}
static void filter(Fixture& f){
 const auto before=f.memory;
 for(int raw=-128;raw<=127;++raw)for(int old=-54;old<=54;++old){recomp_context ctx{};ctx.r4=int64_t(raw);ctx.r5=int64_t(old);native::func_80022A58(f.rdram,&ctx);need(int32_t(ctx.r2)==s::filter(raw,old),"entire signed pad domain and every legal old axis match actual22A58");}
 need(f.memory==before,"native scalar filtering leaves every byte of8MiB unchanged");
}
static void global(Fixture& f){
 for(int x=-82;x<=82;++x)for(int y=-82;y<=82;++y)f.same({x,y},{0,0},"all raw-axis pairs match actual native22B20 including quantified force-X branch");
 for(s::Axes old:std::array<s::Axes,8>{{{-54,-54},{54,54},{-54,54},{54,-54},{0,41},{0,-41},{16,19},{-17,20}}})
  for(s::Axes raw:std::array<s::Axes,20>{{{0,0},{28,70},{-28,70},{69,69},{-69,69},{60,25},{44,47},{48,52},{50,54},{57,57},{57,-57},{57,59},{57,-59},{57,60},{57,-60},{-57,59},{-57,-59},{82,0},{0,82},{0,-82}}})f.same(raw,old,"old-axis extremes and exact gate-boundary counterexamples match actual native22B20");
 const auto defective=actual_convert({.35f,.85f});need(defective.x==28&&defective.y==70,"original broken diagonal reaches raw28/70");
 for(int x=-54;x<=54;++x){s::Axes old{x,41};for(unsigned i=0;i<12;++i)old=f.frame(defective,old);need(old.x==0,"original diagonal loses all horizontal aim before12VI tongue edge for every oldX");}
 for(auto raw:std::array<s::Axes,3>{{{60,25},{44,47},{48,52}}}){const s::Axes old=raw.x==60?s::Axes{0,41}:raw.x==44?s::Axes{16,19}:s::Axes{0,0};auto host=s::inverse_raw(raw);need(bool(host),"known exact one-VI inverse is legal");const auto actual=f.frame(actual_convert(*host),old);const auto expected=raw.x==48?s::Axes{10,12}:s::Axes{16,19};need(actual.x==expected.x&&actual.y==expected.y,"one-VI inverse reaches requested actual native axes, not just matching a model");}
}
static void inverse(Fixture& f){
 for(s::Axes old:std::array<s::Axes,7>{{{0,0},{0,41},{0,-41},{54,54},{-54,-54},{54,-54},{-54,54}}})for(unsigned i=0;i<24;++i){const float angle=float(i)*s::pi/12.f,x=100*std::cos(angle),y=100*std::sin(angle);auto result=s::toward(x,y,old);need(bool(result),"valid target and previous native axes produce bounded host candidate");const auto raw=actual_convert(result->host);need(raw.x==result->raw.x&&raw.y==result->raw.y,"toward candidate survives original host-to-pad conversion exactly");const auto actual=f.frame(raw,old);need(actual.x==result->filtered.x&&actual.y==result->filtered.y,"toward result predicts actual native22B20 in one VI");need(std::isfinite(result->cosine)&&result->cosine>=-1.00001f&&result->cosine<=1.00001f,"candidate cosine is factual finite normalized direction");}
 for(s::Axes initial:std::array<s::Axes,3>{{{0,0},{0,41},{54,-54}}}){
  s::Axes observed=initial;float cosine=-1;
  for(unsigned step=0;step<8;++step){const auto result=s::toward(85,101,observed);need(bool(result),"reported native bird direction has an inverse candidate");const auto raw=actual_convert(result->host);need(raw.x==result->raw.x&&raw.y==result->raw.y,"each converging bird inverse survives actual runtime conversion");const auto actual=f.frame(raw,observed);need(actual.x==result->filtered.x&&actual.y==result->filtered.y,"each converging bird inverse matches real native filter and force-X branch");observed=actual;cosine=result->cosine;if(cosine>.999f)break;}
  need(cosine>.999f,"reported bird direction converges within8VI from real observed axes, including initially opposite input");
 }
 for(s::Axes invalid:std::array<s::Axes,4>{{{-55,0},{55,0},{0,-55},{0,55}}})need(!s::toward(85,101,invalid),"invalid previous native axes are rejected");
 need(!s::toward(0,0,{})&&!s::toward(std::numeric_limits<float>::quiet_NaN(),1,{})&&!s::toward(1,std::numeric_limits<float>::infinity(),{})&&!s::toward(100001,0,{}),"invalid target vectors are rejected");
}
int main(int argc,char** argv){
 need(argc==2,"Pass an existing native8MiB captured RAM path; this CPU test never launches a game");std::ifstream input(argv[1],std::ios::binary);need(bool(input),"read-only captured RAM source opens");const std::vector<uint8_t> captured((std::istreambuf_iterator<char>(input)),{});need(captured.size()==0x800000,"native math source has exactly8MiB");Fixture f(captured);const auto original=f.memory;
 conversion();filter(f);global(f);inverse(f);
 need(facing_calls==frames,"one harmless facing-notification stub per actual native22B20 frame");
 for(unsigned offset=0;offset<f.memory.size();++offset)if(f.memory[offset]!=original[offset]){const uint32_t a=0x80000000u+(offset^3);const bool permitted=(a>=Fixture::pad&&a<Fixture::pad+6)||(a>=0x800F897E&&a<0x800F8986)||(a>=0x800F8988&&a<0x800F8994)||(a>=Fixture::stack-0x100&&a<Fixture::stack+0x10);need(permitted,"actual native writes stay within pad/filter outputs and isolated call stack");}
 std::printf("BossNativeStick0260Checks: %u PASS; %u actual native22B20 calls; native atan/sin/cos/abs and runtime conversion executed; facing notification only stubbed; no game/GPU/ledger writes.\n",checks,frames);
}
