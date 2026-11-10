#include "../boss_rooms_data0250.hpp"
#include "../boss_native.hpp"
#define boss_rooms_data0250 prior_boss_rooms_data0250
#include "previous-rooms.inc"
#undef boss_rooms_data0250
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace n=workshop::boss_native;namespace now=workshop::boss_rooms_data0250;namespace old=workshop::prior_boss_rooms_data0250;namespace plan=workshop::boss_battle0250;
static unsigned checks=0;static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
int32_t sections[512]{};int32_t* section_addresses=sections;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-label"
extern "C" {
#include "native-completion-getter.inc"
}
#pragma clang diagnostic pop
static std::vector<uint16_t> ids(std::span<const uint8_t> descriptor){std::vector<uint16_t> out;for(size_t q=0x60;q<0x100;q+=2){const auto kind=workshop::u16(descriptor,q);if(kind==0xFFFF)return out;out.push_back(kind);}throw std::runtime_error("Unterminated native preload list");}
static void descriptor(std::span<const uint8_t> before,std::span<const uint8_t> after,bool fish){
 auto expected=std::vector<uint8_t>(before.begin(),before.end());if(fish){const auto prior=ids(before),current=ids(after);need(prior.size()==78&&current.size()==79&&current.back()==0x46EE&&std::equal(prior.begin(),prior.end(),current.begin()),"Only46EE is appended after all78 original Fish preloads, within80slots");need(workshop::u16(expected,0xFC)==0xFFFF&&workshop::u16(expected,0xFE)==0,"Exact original terminator/tail is known and unused");workshop::terrain::half(expected,0xFC,0x46EE);workshop::terrain::half(expected,0xFE,0xFFFF);}
 need(expected.size()==after.size()&&std::equal(expected.begin(),expected.end(),after.begin()),"Only Fish's known two preload words change; all other descriptors/music/metadata stay byte exact");
}
int main(){try{
 for(unsigned source:{162u,163u,164u,165u,97u,151u}){const auto& before=old::base_arena(source);const auto& after=now::base_arena(source);descriptor(before.descriptor,after.descriptor,source==97);need(before.objects.size()==after.objects.size()&&std::equal(before.objects.begin(),before.objects.end(),after.objects.begin()),"Provider preload adds no actor or other native placement");}
 for(uint64_t seed=0;seed<120;++seed){const auto p=plan::make_plan(seed);old::configure(p);now::configure(p);for(unsigned index=0;index<10;++index){const auto& before=old::arena(old::source_for_encounter(index));const auto& after=now::arena(now::source_for_encounter(index));descriptor(before.descriptor,after.descriptor,after.logical_source==97);
  need(before.objects.size()==after.objects.size()&&std::equal(before.objects.begin(),before.objects.end(),after.objects.begin())&&before.entry_x==after.entry_x&&before.entry_y==after.entry_y&&before.native_init==after.native_init&&before.safe_pot_y==after.safe_pot_y&&std::equal(before.safe_pot_xs.begin(),before.safe_pot_xs.end(),after.safe_pot_xs.begin()),"All120plan objects/704Inviso entry/other placements/routes/native initializers remain unchanged");
  for(size_t q=0;q<after.objects.size();q+=12)if(after.logical_source==97)need(workshop::u16(after.objects,q)!=0x46EE,"A native46EE resource is preloaded, never placed as a scene actor");
 }}
 // Execute the complete real getter, without any substituted native service.
 // Its46EE branch is the ONLY writer of F8D78; matching EF/F0 alone fails.
 for(uint32_t relocated:{0x80200000u,0x803F1000u,0x80600000u})for(uint32_t previous:{0u,0x80345678u})for(uint16_t kind:{0x46EE,0x46EF,0x46F0,0x46F2,0x46F3,0x473B,0}){
  std::vector<uint8_t> memory(8*1024*1024);auto* rdram=memory.data();sections[285]=int32_t(relocated);constexpr uint32_t request=0x80010000;MEM_H(0,n::base(request))=kind;MEM_W(0,n::base(0x800F8D78))=previous;const auto before=memory;recomp_context c{};c.r4=n::base(request);c.r29=n::base(0x800FF800);c.r31=n::base(0x80012344);c.r16=0x11223344;c.f0.u32l=0x12345678;
  func_807AC660(rdram,&c);const uint32_t offset=kind==0x46EE?0x2340:kind==0x46EF?0x69C:kind==0x46F0?0xC8C:kind==0x46F2?0x1360:0;
  need(c.r2==(offset?n::base(relocated+offset):0),"Real getter returns the exact sign-extended native constructor for its own resource kind");
  need(n::word(rdram,0x800F8D78)==(kind==0x46EE?relocated+0x2418:previous),"Only native46EE publishes the actual section285+2418 callback; other kinds preserve stale/missing callback");
  constexpr size_t slot=0xF8D78;need(!std::memcmp(before.data(),memory.data(),slot)&&!std::memcmp(before.data()+slot+4,memory.data()+slot+4,memory.size()-slot-4),"Native getter modifies no RAM outside the one4-byte callback; no HP/control/timer/actor writes");
  need(c.r4==n::base(request)&&c.r29==n::base(0x800FF800)&&c.r31==n::base(0x80012344)&&c.r16==0x11223344&&c.f0.u32l==0x12345678,"Native getter preserves its caller identity/stack/return/callee-saved/FPU state");
 }
 std::cout<<"FishCompletionProvider0260Checks: "<<checks<<" PASS; actual native getter +120plan data comparison, no GPU victory/callback residency claim.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
