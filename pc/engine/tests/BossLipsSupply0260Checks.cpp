#include "../boss_lips_supply0260.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace l=workshop::boss_lips_supply0260;namespace n=workshop::boss_native;
static std::array<int32_t,300> sections{};int32_t* section_addresses=sections.data();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#include "../../build/boss-0260/lips-supply-20261101/native-phase.inc"
#pragma clang diagnostic pop
static unsigned checks=0,native_calls=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
static std::vector<uint8_t> load(const std::string& path){std::ifstream stream(path,std::ios::binary);std::vector<uint8_t> data((std::istreambuf_iterator<char>(stream)),{});need(data.size()==0x800000,"Existing native b55 snapshot must be complete8MiB");return data;}
struct Memory {
 std::vector<uint8_t> bytes;uint8_t* rdram;
 explicit Memory(const std::vector<uint8_t>& source):bytes(source),rdram(bytes.data()){}
 void w(uint32_t x,uint32_t v){MEM_W(0,n::base(x))=int32_t(v);}void by(uint32_t x,unsigned v){MEM_B(0,n::base(x))=uint8_t(v);}void h(uint32_t x,unsigned v){MEM_H(0,n::base(x))=uint16_t(v);}void fl(uint32_t x,float v){w(x,std::bit_cast<uint32_t>(v));}
};
int main(int argc,char** argv){try{
 need(argc==2,"Pass the existing b55 profile path");const std::string base=argv[1];
 const auto early=load(base+"/controller-test-3000.rdram"),later=load(base+"/controller-test-3600.rdram");
 struct Sample {const std::vector<uint8_t>* ram;uint32_t actor;unsigned kind,phase;};
 const std::array<Sample,4> samples{{{&early,0x802E23D4,0x453A,2},{&early,0x802E2654,0x453A,1},{&later,0x802E41D4,0x4539,2},{&later,0x802E4454,0x4539,1}}};
 for(const auto& sample:samples){
  Memory m(*sample.ram);const auto before=m.bytes;const auto value=l::observe(m.rdram,sample.actor);
  need(value&&value->actor==sample.actor&&value->kind==sample.kind&&value->active&&!value->collectible&&value->food_flags==0xC1,"Both actual falling native types remain their own edible4539/453A identity");
  need(value->x==std::bit_cast<float>(n::word(m.rdram,sample.actor+0x84))&&value->y==std::bit_cast<float>(n::word(m.rdram,sample.actor+0x88)),"Observed supply position is the actual current native world position");
  need(n::byte(m.rdram,sample.actor+0x18)==sample.phase&&!n::word(m.rdram,sample.actor+0x1F4),"Actual snapshots contain both live phases with legitimate private0");
  need(m.bytes==before,"The observer leaves all8MiB unchanged");
  const auto resource=n::word(m.rdram,sample.actor+0x168),factory=n::word(m.rdram,resource+0x40);
  sections[160]=int32_t(factory);
  for(unsigned phase:{1u,2u,3u}){
   Memory q(*sample.ram);recomp_context ctx{};ctx.r4=n::base(sample.actor);ctx.r5=phase;ctx.r29=n::base(0x807B0000);
   func_805E11E0(q.rdram,&ctx);++native_calls;
   need(n::byte(q.rdram,sample.actor+0x18)==phase&&n::word(q.rdram,sample.actor+0x1E0)==0,"Literal original11E0 selects the requested native phase and resets only its timer");
   need(n::word(q.rdram,sample.actor+0x20)==factory+(phase==1?0x370u:phase==2?0x424u:0x528u),"Actual native state table confirms both live callbacks and the retirement callback");
   const auto native_result=q.bytes;const auto food=l::observe(q.rdram,sample.actor);need(bool(food)==(phase!=3),"Live native phases are admitted; native retirement never becomes food");need(q.bytes==native_result,"Observer remains read-only after the original native phase setter");
  }
 }
 for(const auto& sample:std::array<Sample,2>{samples[0],samples[2]})for(unsigned fault=0;fault<38;++fault){
  Memory m(*sample.ram);const auto actor=sample.actor,r=n::word(m.rdram,actor+0x168),md=n::word(m.rdram,actor+0x174),d=n::word(m.rdram,md),factory=n::word(m.rdram,r+0x40),scale=n::word(m.rdram,d+0x70);
  if(fault==0)m.w(actor,0);if(fault==1)m.h(actor+12,0x4001);if(fault==2)m.h(actor+12,0x40A7);if(fault==3)m.h(actor+12,0x4526);if(fault==4)m.by(actor+6,3);if(fault==5)m.by(actor+6,0);
  if(fault==6)m.by(actor+0x17D,0);if(fault==7)m.by(actor+0x17D,1);if(fault==8)m.by(actor+0x17D,3);if(fault==9)m.by(actor+0x18,0);if(fault==10)m.by(actor+0x18,3);if(fault==11)m.by(actor+0x18,0xFF);
  if(fault==12)m.by(actor+0x30,1);if(fault==13)m.w(actor+0x180,0x800C8B5C);if(fault==14)m.w(actor+0x1F4,0x80500000);if(fault==15)m.by(actor+0x165,1);if(fault==16)m.w(actor+0x168,0);
  if(fault==17)m.h(r+0x3C,0x4526);if(fault==18)m.w(r+0x40,0);if(fault==19)m.w(r+0x44,factory+0x1C20);if(fault==20)m.w(actor+0x20,factory+0x528);if(fault==21)m.w(actor+0x174,0);if(fault==22)m.w(md,0);
  if(fault==23)m.by(d+0x74,0);if(fault==24)m.w(d+0x28,0x80039F48);if(fault==25)m.h(d+0x1C,0);if(fault==26)m.h(d+0x1C,0xC0);if(fault==27)m.h(d+0xA,14);if(fault==28)m.h(d+0xC,14);if(fault==29)m.h(d+0xE,20);
  if(fault==30)m.h(d+4,1);if(fault==31)m.h(d+8,1);if(fault==32)m.w(d+0x70,0);if(fault==33)m.fl(scale,-1);if(fault==34)m.w(scale+4,0x7FC00000);if(fault==35)m.w(actor+0x84,0x7FC00000);if(fault==36)m.w(actor+0x88,0x7F800000);if(fault==37)m.w(actor+0x8C,0x7FC00000);
  const auto before=m.bytes;need(!l::observe(m.rdram,actor)&&m.bytes==before,"Foreign, stale, inedible, inactive, malformed or nonfinite objects are rejected without repair");
 }
 {Memory m(early);const auto before=m.bytes;for(uint32_t actor:{0u,1u,0x807FFFF0u,0x802E23D5u})need(!l::observe(m.rdram,actor),"Null/unaligned/out-of-range addresses cannot be observed");need(!l::observe(nullptr,0x802E23D4)&&m.bytes==before,"Null memory is rejected without mutation");}
 {Memory m(early);need(!l::observe(m.rdram,0x802E2154)&&!l::observe(m.rdram,0x802E3F54),"Actual retired453A/4539 remain rejected despite their unchanged edible definition");}
 std::cout<<"BossLipsSupply0260Checks: "<<checks<<" PASS; original11E0Calls="<<native_calls<<"; realB55ActiveSamples=4; allObservationsReadOnly=1; ingestionGpuVerified=0\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
