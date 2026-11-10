#include "../boss_fish_flash0260.hpp"
#include "../boss_native.hpp"
#include <bit>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#ifndef FISH_FLASH_NATIVE_SLICE
#error Supply original native80550B74 palette interpolation body
#endif
namespace original_native {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include FISH_FLASH_NATIVE_SLICE
#pragma clang diagnostic pop
}
namespace f=workshop::boss_fish_flash0260;namespace n=workshop::boss_native;
static unsigned checks=0;void need(bool b,const char* msg){++checks;if(!b)throw std::runtime_error(msg);}
int main(int argc,char** argv){try{
 need(argc==2,"Supply the unchanged captured native Monsters RAM");std::ifstream input(argv[1],std::ios::binary);std::vector<uint8_t> memory((std::istreambuf_iterator<char>(input)),{});need(memory.size()==0x800000,"Full8MiB capture required");auto* rdram=memory.data();
 constexpr uint32_t actor=0x800CA434,source=0x8039F750,dest=0x807D8000;
 need(n::actor(rdram,actor,0x41C6),"Captured source is actual41C6");const auto resource=n::word(rdram,actor+0x168);need(n::word(rdram,resource+0x88)==source,"Actual captured resource exposes both native palettes");
 const auto original=memory;
 for(unsigned i=0;i<256;++i){const auto normal=MEM_HU(i*2,n::base(source)),alternate=MEM_HU(0x200+i*2,n::base(source));need(f::color(normal,true)==alternate,"All256 native alternate entries exactly implement RGB->BRG with alpha unchanged");}
 for(unsigned phase=0;phase<2;++phase){recomp_context c{};c.r4=n::base(dest);c.r5=n::base(source+0x200);c.r6=n::base(source);c.r7=std::bit_cast<uint32_t>(float(phase));original_native::func_80550B74(rdram,&c);for(unsigned i=0;i<256;++i)need(MEM_HU(i*2,n::base(dest))==f::color(MEM_HU(i*2,n::base(source)),bool(phase)),"Literal80550B74 endpoint matches Fish flash colour");}
 need(std::equal(memory.begin()+(source&0x7FFFFF),memory.begin()+(source&0x7FFFFF)+1024,original.begin()+(source&0x7FFFFF)),"Both original native palettes are unchanged by native oracle");
 need(f::duration_ticks==50,"Native hit setsDE4=50");need(!f::alternate(0)&&!f::alternate(1)&&!f::alternate(2)&&f::alternate(3)&&f::alternate(6)&&!f::alternate(7),"First phases match native decrement and4tick bit");
 for(unsigned age=1;age<=50;++age){const int remaining=50-int(age);const int native_shift=remaining>>2;need(f::alternate(age)==(age<50&&((native_shift&1)!=0)),"Native remaining counter phase is preserved through expiry");}
 for(unsigned age:{50u,51u,60u,120u,0xFFFFFFFFu})need(!f::alternate(age),"Expired flash never resumes or underflows");
 for(uint16_t value:{uint16_t(0),uint16_t(1),uint16_t(0xF801),uint16_t(0x07C1),uint16_t(0x003F),uint16_t(0xFFFF),uint16_t(0xA9B5),uint16_t(0x169A)}){need(f::color(value,false)==value,"Normal phase preserves any current/future member colour exactly");need((f::color(value,true)&1)==(value&1),"Flashing preserves alpha");need(f::color(f::color(f::color(value,true),true),true)==value,"Channel cycle is reversible for every identity including purple/green");}
 std::cout<<"BossFishFlash0260Checks: "<<checks<<" PASS;2 literal80550B74 calls;256 native palette pairs; no GPU/game.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
