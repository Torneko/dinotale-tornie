#include "../boss_rooms_data0250.hpp"
#include "../boss_native.hpp"
#define boss_rooms_data0250 prior_boss_rooms_data0250
#include "previous-rooms.inc"
#undef boss_rooms_data0250
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace now=workshop::boss_rooms_data0250;namespace old=workshop::prior_boss_rooms_data0250;namespace plan=workshop::boss_battle0250;namespace n=workshop::boss_native;
static unsigned checks=0,moved=0;static void need(bool ok,const char* what){++checks;if(!ok)throw std::runtime_error(what);}
static float real(std::span<const uint8_t> d,size_t at){return std::bit_cast<float>(workshop::u32(d,at));}
int32_t sections[512]{};int32_t* section_addresses=sections;
extern "C" void func_800677F8(uint8_t*,recomp_context*){}
extern "C" void func_806AE458(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_80067BBC(uint8_t*,recomp_context*){}
extern "C" void func_80067F78(uint8_t*,recomp_context*){}
extern "C" void func_800492C0(uint8_t*,recomp_context*){}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-label"
extern "C" {
#include "native-inviso-position.inc"
}
#pragma clang diagnostic pop
struct Native {
 std::vector<uint8_t> memory=std::vector<uint8_t>(8*1024*1024);uint8_t* rdram=memory.data();recomp_context c{};
 static constexpr uint32_t root=0x80020000,priv=0x80021000,overlay=0x80300000;
 Native(float x){MEM_W(0x1F4,n::base(root))=priv;MEM_W(0x3C,n::base(root))=std::bit_cast<uint32_t>(x);MEM_W(0x1F9C,n::base(overlay))=0x3ECCCCCD;c.r29=n::base(0x800FF800);sections[197]=int32_t(overlay);}
 float x()const{return std::bit_cast<float>(n::word(const_cast<uint8_t*>(memory.data()),root+0x3C));}
 void step(unsigned phase){MEM_W(0x1E0,n::base(root))=phase;c.r4=n::base(root);func_806AEAEC(rdram,&c);}
};
int main(){try{
 for(uint64_t seed=0;seed<120;++seed){const auto p=plan::make_plan(seed);old::configure(p);now::configure(p);
  for(unsigned index=0;index<10;++index){const auto& before=old::arena(old::source_for_encounter(index));const auto& after=now::arena(now::source_for_encounter(index));const auto& e=p.fights[index];
   auto expected_descriptor=std::vector<uint8_t>(before.descriptor.begin(),before.descriptor.end());if(after.logical_source==97){need(workshop::u16(expected_descriptor,0xFC)==0xFFFF&&workshop::u16(expected_descriptor,0xFE)==0,"Placement fixture retains the exact before-provider preload tail");workshop::terrain::half(expected_descriptor,0xFC,0x46EE);workshop::terrain::half(expected_descriptor,0xFE,0xFFFF);}
   need(before.objects.size()==after.objects.size()&&before.boss_indices.size()==after.boss_indices.size()&&std::equal(before.boss_indices.begin(),before.boss_indices.end(),after.boss_indices.begin())&&std::equal(expected_descriptor.begin(),expected_descriptor.end(),after.descriptor.begin()),"Only the independently proved46EE preload may alter the descriptor; counts/order/indices/camera remain exact");
   need(before.entry_x==after.entry_x&&before.entry_y==after.entry_y&&before.safe_pot_y==after.safe_pot_y&&before.native_init==after.native_init&&std::equal(before.safe_pot_xs.begin(),before.safe_pot_xs.end(),after.safe_pot_xs.begin()),"Native init/entry/three islands/pots remain exact");
   unsigned changes=0;for(size_t at=0;at<after.objects.size();at+=12){const bool target=after.logical_source==97&&e.variant==2&&workshop::u16(after.objects,at)==0x4599;auto expected=std::vector<uint8_t>(before.objects.begin()+at,before.objects.begin()+at+12);
    if(target){need(real(before.objects,at+4)==600&&real(after.objects,at+4)==704&&real(before.objects,at+8)==288&&real(after.objects,at+8)==288,"Only the real Inviso companion record moves104px at native birth");workshop::put32(expected,4,std::bit_cast<uint32_t>(704.f));++changes;++moved;}
    need(std::equal(expected.begin(),expected.end(),after.objects.begin()+at),"Every other native record word/parameter including Y and packed identity remains byte-exact");
   }
   need(changes==(after.logical_source==97&&e.variant==2&&e.members[1].kind==0x4599?1u:0u),"No change to Genie/Fish/other family, solo or unrelated encounter");
  }
 }
 need(moved>0,"Mixed Inviso plans were actually exercised");
 // Both paths execute the complete original AEAEC. Animation completion is
 // an explicit external service; this samples relative native displacement,
 // not a claim about the actual intro duration or GPU exit coordinates.
 Native baseline(600),shifted(704);need(shifted.x()-28>=16&&shifted.x()+28<752,"Spawn collider fits strictly within the unchanged inner basin");
 for(unsigned phase=0;phase<8;++phase)for(unsigned sample=0;sample<66;++sample){const float bx=baseline.x(),sx=shifted.x();baseline.step(phase);shifted.step(phase);
  need(std::abs((shifted.x()-baseline.x())-104.f)<.02f,"Actual native relative translation preserves the104px delta within accumulated float tolerance");
  need(baseline.x()>184&&shifted.x()>184&&shifted.x()-28>=16&&shifted.x()+28<752,"Both sampled native paths stay on the same184-threshold branch and inside basin walls");
  if(phase==7)need(baseline.x()==bx&&shifted.x()==sx,"Native phase7 does not shift either path, unchanged behavior");
 }
 for(unsigned terrain=0;terrain<3;++terrain){const float middle=terrain==0?384.f:terrain==1?360.f:408.f;
  need(shifted.x()-28>middle+14,"Even full-scale P1 body is horizontally separate after the sampled native intro envelope on all three terrains");
 }
 std::cout<<"FishInvisoPlacement0260Checks: "<<checks<<" PASS; "<<moved<<" target records across120plans; relative native translation only, no GPU positioning/victory claim.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
