#include "../boss_native.hpp"
#include <array>
#include <bit>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#ifndef BIRD_SPECIES_NATIVE_SLICE
#error Supply literal native1A30/32D4/3B18/35B4 bodies.
#endif
namespace n=workshop::boss_native;
static unsigned checks=0,native_calls=0,preloads=0,registrations=0,allocations=0,animations=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static int32_t sections[300]{};int32_t* section_addresses=sections;
extern "C" void switch_error(const char*,uint32_t,uint32_t){std::fputs("Unexpected native factory jump table case\n",stderr);std::abort();}
namespace native_bird {
static uint32_t private_result=0,expected_actor=0;
void func_8004667C(uint8_t*,recomp_context*);void func_80057B28(uint8_t*,recomp_context*);void func_80045A34(uint8_t*,recomp_context*);
void func_805D1CC0(uint8_t*,recomp_context*);void func_805D1D7C(uint8_t*,recomp_context*);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmicrosoft-goto"
#include BIRD_SPECIES_NATIVE_SLICE
#pragma clang diagnostic pop
// Explicit service boundaries only. No stub chooses the factory result,
// primary/substate, callback pointer, private size or physical launch values.
void func_8004667C(uint8_t*,recomp_context* c){need(c->r6==5,"Real4526/46DA/4046 cold factory requests its native5 resources");++preloads;}
void func_80057B28(uint8_t*,recomp_context*){++registrations;}
void func_80045A34(uint8_t*,recomp_context* c){need(uint32_t(c->r4)==expected_actor&&c->r5==12,"Literal32D4 requests exactly12 private bytes for this actor");++allocations;c->r2=n::base(private_result);}
void func_805D1CC0(uint8_t*,recomp_context* c){need(uint32_t(c->r4)==expected_actor&&c->r5==1,"Literal3B18 sends native animation mode1 for the same actor");++animations;}
void func_805D1D7C(uint8_t*,recomp_context* c){need(uint32_t(c->r4)==expected_actor,"Literal3B18 completes animation on the same actor");++animations;}
}
struct Fixture {
 static constexpr uint32_t a=0x80700000,p=0x80700400,drawing=0x80700300,request=0x80700500,stack=0x807B0000;
 std::vector<uint8_t> memory;uint8_t* rdram;recomp_context c{};uint32_t factory;
 Fixture(const std::vector<uint8_t>& input,uint32_t overlay):memory(input),rdram(memory.data()),factory(overlay){sections[156]=int32_t(factory);std::fill(memory.begin()+(a&0x7FFFFF),memory.begin()+(request&0x7FFFFF)+16,0);c.r29=n::base(stack);c.r31=n::base(0x80000300);c.r16=0x12345678;c.r17=0x87654321;}
 void w(uint32_t at,uint32_t value){MEM_W(0,n::base(at))=int32_t(value);}void h(uint32_t at,unsigned value){MEM_H(0,n::base(at))=uint16_t(value);}void b(uint32_t at,unsigned value){MEM_B(0,n::base(at))=uint8_t(value);}void fl(uint32_t at,float value){w(at,std::bit_cast<uint32_t>(value));}uint32_t word(uint32_t at){return n::word(rdram,at);}
 void call(recomp_func_t* fn,uint32_t actor){c.r4=n::base(actor);const auto sp=c.r29,ra=c.r31,s0=c.r16,s1=c.r17;fn(rdram,&c);++native_calls;need(c.r29==sp&&c.r31==ra&&c.r16==s0&&c.r17==s1,"Literal body preserves native callee-save registers and stack");}
 uint32_t select(unsigned kind,bool cold){for(unsigned offset:{0x2160u,0x2164u,0x216Cu})w(factory+offset,cold?0:1);h(request,kind);call(native_bird::func_805D1A30,request);return uint32_t(c.r2);}
 void initialize(bool reuse_private){w(a,0x19971110);h(a+12,0x46DA);b(a+0x17D,2);b(a+0x165,1);w(a+0x16C,drawing);w(a+0x1F4,reuse_private?p:0);fl(a+0x3C,176);fl(a+0x40,184);fl(a+0xC4,1);w(a+0x20,factory+0x1B84);native_bird::private_result=p;native_bird::expected_actor=a;call(native_bird::func_805D35B4,a);}
};
int main(int argc,char**argv){try{
 need(argc==2,"Immutable actual ed971 final RAM required");std::ifstream in(argv[1],std::ios::binary);std::vector<uint8_t> source((std::istreambuf_iterator<char>(in)),{});need(source.size()==0x800000,"Actual ed971 capture is exactly8MiB");const auto original=source;auto* rdram=source.data();
 constexpr uint32_t real=0x80322E14;need(n::actor(rdram,real,0x46DA),"Actual final actor is native species46DA");const auto resource=n::word(rdram,real+0x168);need(n::range(resource,0x48)&&MEM_HU(0x3C,n::base(resource))==0x46DA,"Actual species46DA owns its native resource");const auto factory=n::word(rdram,resource+0x40);need(factory==0x8031F740&&n::word(rdram,resource+0x44)==factory+0x1B84,"Actual46DA resource public initializer is35B4, not3650");need(n::word(rdram,real+0x20)==factory+0xAE4&&n::byte(rdram,real+0x30)==7,"Actual retired wounded bird retained native2514/sub7, not2854/sub9");
 need(n::word(rdram,factory+0x2468+48+7*4)==factory+0xAE4,"Unmodified actual native1/7 callback table independently names2514");
 const std::array<std::array<unsigned,2>,5> species{{{0x4526,0x1B1C},{0x46DA,0x1B84},{0x4046,0x1C20},{0x4527,0x1BEC},{0x4001,0x19A8}}};
 for(uint32_t overlay:{factory,0x80200000u,0x805D1A30u})for(bool cold:{false,true})for(const auto& s:species){Fixture x(source,overlay);const auto result=x.select(s[0],cold);need(result==overlay+s[1],"Literal factory returns species-specific initializer under hot/cold and relocated overlays");if(s[0]==0x46DA)need(result!=overlay+0x1C20,"Native46DA can never acquire4046's3650 initializer");}
 for(bool reuse:{false,true}){Fixture x(source,factory);const auto old_allocations=allocations,old_animations=animations;need(x.select(0x46DA,false)==factory+0x1B84,"Native factory supplies the exact initializer under test");x.initialize(reuse);need(allocations==old_allocations+(reuse?0:1)&&animations==old_animations+2,"Literal initializer allocates only when needed and executes both animation service boundaries");need(n::byte(x.rdram,Fixture::a+0x18)==1&&n::byte(x.rdram,Fixture::a+0x30)==7&&x.word(Fixture::a+0x20)==factory+0xAE4,"Literal35B4 and3B18 select1/7/2514 through the actual native callback table");need(x.word(Fixture::a+0x1F4)==Fixture::p&&x.word(Fixture::p)==std::bit_cast<uint32_t>(176.f)&&x.word(Fixture::p+4)==std::bit_cast<uint32_t>(184.f),"Literal32D4 installs private12B and copies actual localXY");need(x.word(Fixture::a+0xC8)==std::bit_cast<uint32_t>(-3.f)&&x.word(Fixture::a+0xE0)==std::bit_cast<uint32_t>(4.f)&&x.word(Fixture::a+0xEC)==x.word(factory+0x2558),"Literal35B4 writes native wounded launch/gravity fields");
  auto* saved_rdram=rdram;rdram=x.rdram;need(MEM_HU(0x144,n::base(Fixture::a))==4&&x.word(Fixture::a+0xC4)==std::bit_cast<uint32_t>(1.f)&&x.word(Fixture::a+0x3C)==std::bit_cast<uint32_t>(176.f)&&x.word(Fixture::a+0x40)==std::bit_cast<uint32_t>(184.f),"Native initializer preserves horizontal channel and position while installing wounded flags");rdram=saved_rdram;}
 need(source==original,"Actual RAM evidence remains immutable");std::cout<<"BossBirdSpeciesNative0260Checks: "<<checks<<" PASS; literalNativeCalls="<<native_calls<<" preloads="<<preloads<<" registrations="<<registrations<<" allocations="<<allocations<<" animationServices="<<animations<<"; no GPU or gameplay victory claim\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
