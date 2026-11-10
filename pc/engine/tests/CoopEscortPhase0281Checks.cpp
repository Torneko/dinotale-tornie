// Real nativeC6B0 selects each phase from the independently read ROM table.
// Existing lifecycle fixture executes the real dispatcher and468E8 cache return.
// Actor construction/rebinding, scheduler and animation BD1C are HOST boundaries.
#define main lifecycle_regression_main
#include "CoopLifecycle0280Checks.cpp"
#undef main
#include <fstream>
#include <iterator>
static int32_t escort_sections[512]{};
int32_t* section_addresses=escort_sections;
static unsigned animation_calls=0;
// RealBB68 skips its audio branch for native4001-owned escorts (1E4==0).
// Unexecuted audio endpoints fail explicitly rather than pretending success.
UNUSED_NATIVE(func_80067BBC) UNUSED_NATIVE(func_80067F78) UNUSED_NATIVE(func_80067838)
UNUSED_NATIVE(func_80067930) UNUSED_NATIVE(func_800678F0)
extern "C" void func_8057BD1C(uint8_t* r,recomp_context* x){
 need(n::actor(r,uint32_t(x->r4),0x4015),"HOST animation endpoint receives the real typed actor");++animation_calls;
}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#include "D:/Yoshi-Story/pc/build/coop-0281/native-escort-phase.inc"
#include "D:/Yoshi-Story/pc/build/coop-0281/native-escort-terminal.inc"
#pragma clang diagnostic pop
static std::vector<uint8_t> rom(){std::ifstream file("D:/Yoshi-Story/work/yoshis-story-decomp/baseroms/us/baserom.z64",std::ios::binary);return {std::istreambuf_iterator<char>(file),{}};}
static const auto native_rom=rom();
static uint32_t rom_word(unsigned offset){need(offset+4<=native_rom.size(),"USA ROM table is in bounds");return uint32_t(native_rom[offset])<<24|uint32_t(native_rom[offset+1])<<16|uint32_t(native_rom[offset+2])<<8|native_rom[offset+3];}
static constexpr uint32_t escort_overlay=0x80390000,escort_storage=0x80380010;
static void install_phase_table(Fixture& f){
 need(native_rom.size()==0x1000000,"Read-only USA ROM is the expected size");escort_sections[128]=int32_t(escort_overlay);
 for(unsigned i=0;i<14;i++)w(f.r,escort_overlay+0xF90+i*4,escort_overlay+rom_word(0xCB3C40+i*4)-0x8057BB10);
 for(unsigned i=0;i<4;i++)w(f.r,escort_overlay+0xFE8+i*4,escort_overlay+rom_word(0xCB3C98+i*4)-0x8057BB10);
}
static void select(Fixture& f,unsigned sub){for(unsigned i=0;i<4;i++){f.x.r4=int32_t(escort_storage+i*0x280+4);f.x.r5=sub;f.x.r31=int32_t(0x80048A74);func_8057C6B0(f.r,&f.x);}}
int main(){try{
 for(unsigned owner=0;owner<2;owner++)for(unsigned sub=0;sub<14;sub++){
  Fixture f;f.escorts(owner);install_phase_table(f);select(f,sub);const auto root=owner?second:first;
  for(unsigned i=0;i<4;i++){const auto a=escort_storage+i*0x280+4;need(n::byte(f.r,a+0x30)==sub&&u(f.r,a+0x20)==escort_overlay+rom_word(0xCB3C40+sub*4)-0x8057BB10,"True nativeC6B0 writes the ROM-selected sub/update");}
  need(f.kill(owner),"The native death dispatcher admits the original owner");
  // A real nonterminal native phase was previously rejected by the +A80-only check.
  for(unsigned i=0;i<4;i++){const auto a=escort_storage+i*0x280+4;const auto id=c::observed_identity(f.r,a);b::EscortCache cache;need(id&&b::escort_cache_capture(f.r,*id,root,cache),"Every true native4015 phase satisfies the exact incarnation contract");if(sub!=8&&sub!=12&&sub!=13)need(u(f.r,a+0x20)!=escort_overlay+0xA80,"Nonterminal positive is independent of the former terminal-only fixture");}
  f.fence(2501);need(!b::invalidated&&!l::lifecycle.snapshot().failed&&l::lifecycle.snapshot().players[owner].phase==l::Phase::waiting,"Real native468E8 returns all phased escorts without losing local retirement");
  for(const auto& [a,cache]:b::retiring_escorts[owner])need(b::escort_cache_returned(f.r,cache)&&u(f.r,a+0x20)==cache.update&&n::byte(f.r,a+0x30)==sub,"Cache return retains the captured update/sub/index exactly");
 }
 for(unsigned owner=0;owner<2;owner++)for(unsigned initial:{10u,11u}){
  Fixture f;f.escorts(owner);install_phase_table(f);select(f,initial);need(f.kill(owner),"Terminal transition starts after the real fatal dispatcher");const auto root=owner?second:first,private_data=u(f.r,root+0x1F4);
  // Terrain/scheduler status is a declared HOST boundary. First prove its
  // real zero gate, then let C4E0 itself produce12/13 from10/11.
  w(f.r,private_data+0x78,0);f.x.r4=int32_t(escort_storage+4);func_8057C4E0(f.r,&f.x);need(n::byte(f.r,escort_storage+4+0x30)==initial,"RealC4E0 waits while native parent terrain status is zero");
  w(f.r,private_data+0x78,1);
  for(unsigned i=0;i<4;i++){const auto a=escort_storage+i*0x280+4;f.x.r4=int32_t(a);func_8057C4E0(f.r,&f.x);need(n::byte(f.r,a+0x30)==initial+2&&u(f.r,a+0x20)==escort_overlay+0xA80&&u(f.r,a+0x1E0)==i,"ActualC570/C574/C578->C6B0 produces source terminal12/13 with retainedA80 and native index");}
  f.fence(2501);need(!b::invalidated&&!l::lifecycle.snapshot().failed&&l::lifecycle.snapshot().players[owner].phase==l::Phase::waiting,"Source terminal12/13 survives exact native cache return and local retirement");
  for(const auto& [a,cache]:b::retiring_escorts[owner])need(b::escort_cache_returned(f.r,cache)&&n::byte(f.r,a+0x30)==initial+2,"Terminal12/13 cache receipt preserves its exact sub/update");
 }
 for(unsigned i=0;i<4;i++){
  Fixture f;f.escorts(0);install_phase_table(f);need(f.kill(0),"Pending native initializer has a fatal original owner");const auto a=escort_storage+i*0x280+4;by(f.r,a+0x30,0);w(f.r,a+0x20,u(f.r,escort_overlay+0xFE8+i*4));const auto id=c::observed_identity(f.r,a);b::EscortCache cache;
  need(id&&b::escort_cache_capture(f.r,*id,first,cache),"C728 exact pending initializer is tied to its native index");
  w(f.r,a+0x20,escort_overlay+(i==0||i==3?0xB48:0xB74));need(!b::escort_cache_capture(f.r,*id,first,cache),"Initializer for a different index is never adopted");
 }
 for(unsigned bad=0;bad<14;bad++){
  Fixture f;f.escorts(0);install_phase_table(f);select(f,2);need(f.kill(0),"Negative starts with a true native phase");const auto a=escort_storage+4;auto id=*c::observed_identity(f.r,a);b::EscortCache cache;
  switch(bad){case 0:by(f.r,a+0x30,3);w(f.r,a+0x20,escort_overlay+0xA80);break;case 1:by(f.r,a+0x30,14);w(f.r,a+0x20,escort_overlay+0xA80);break;case 2:w(f.r,a+0x1E0,4);break;case 3:w(f.r,a+0x20,escort_overlay+0x639);break;case 4:id.generation++;break;case 5:w(f.r,a+0x180,second);break;case 6:w(f.r,a+8,0xFFFFFFFF);break;case 7:by(f.r,a+5,5);break;case 8:by(f.r,a-4,0);break;case 9:by(f.r,a+6,1);break;case 10:by(f.r,a+0x17D,2);break;case 11:w(f.r,0x80382044,escort_overlay+0xB74);break;case 12:w(f.r,a+0x2C,escort_overlay+0xAC4);break;case 13:w(f.r,a+0x188+19*4,second);break;}
  const auto before=f.ram;need(!b::escort_cache_capture(f.r,id,first,cache)&&f.ram==before,"Foreign/stale/unknown phase or malformed cache rejects without guest mutation");
 }
 {Fixture f;f.escorts(0);install_phase_table(f);select(f,2);need(f.kill(0),"First cached use starts a real local death");f.fence(2501);const auto old=b::retiring_escorts[0].begin()->second.id;f.fence(2800);need(l::lifecycle.snapshot().players[0].phase==l::Phase::alive,"First owner returns before the other owner dies");
  f.escorts(1);select(f,7);const auto fresh=*c::observed_identity(f.r,old.address);need(fresh.address==old.address&&fresh.generation!=old.generation,"HOST factory boundary records a new native incarnation at the same cached address");need(f.kill(1),"Other owner has the second actual fatal dispatcher");f.fence(2801);need(!b::invalidated&&!l::lifecycle.snapshot().failed&&l::lifecycle.snapshot().players[1].phase==l::Phase::waiting,"Reused4015 slots with a different native phase retire for the second owner");
  auto cache=b::retiring_escorts[1].begin()->second;const auto a=cache.id.address;need(b::escort_cache_returned(f.r,cache),"Second exact cache receipt is valid");const auto before=f.ram;w(f.r,a+0x20,escort_overlay+0xA80);need(!b::escort_cache_returned(f.r,cache),"A different valid callback after capture is not a cache receipt");f.ram=before;f.r=f.ram.data();b::memory.ram=f.r;by(f.r,a+0x30,8);need(!b::escort_cache_returned(f.r,cache),"Changed sub after capture cannot complete the lease");}
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"nativePhaseSetter\":\"8057C6B0\",\"sourcePhases\":14,\"sourceInitializers\":4,\"nativeTerminalTransition\":\"8057C4E0/BB68->C6B0\",\"nativeCacheReturn\":\"468E8\",\"constructionAnimationScheduler\":\"HOST\",\"gameLaunched\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
