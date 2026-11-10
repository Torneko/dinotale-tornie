#include "../boss_carry0260.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <vector>
namespace carry=workshop::boss_carry0260;namespace native=workshop::boss_native;
static unsigned checks=0;
static void need(bool v,const char* why){++checks;if(!v){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
 std::vector<uint8_t> mem=std::vector<uint8_t>(0x800000);uint8_t* rdram=mem.data();
 static constexpr std::array<unsigned,11> types{0,1,2,3,4,6,7,8,9,10,11};
 void w(uint32_t a,uint32_t v){MEM_W(0,native::base(a))=v;}
 void h(uint32_t a,uint16_t v){MEM_H(0,native::base(a))=v;}
 Fixture(unsigned count=15,unsigned backup=3){
  for(unsigned i=0;i<mem.size();++i)mem[i]=uint8_t((i*37u+23u)^i/257u);
  for(unsigned i=0;i<10;++i)w(0x800F8950+4*i,0x10203040+i*31);
  w(0x800F89D0,count);w(0x800F8A4C,backup);w(0x800F8AC8,0);
  for(unsigned i=0;i<30;++i){w(0x800F89D4+4*i,types[i%types.size()]);w(0x800F8A50+4*i,types[(i+3)%types.size()]);h(0x800F8AD4+2*i,uint16_t(types[i%types.size()]));}
 }
 // The affected native CFC0 effects, plus unrelated reset-owned data. This
 // fixture proves carry whitelisting, not that native game code was executed.
 void simulate_reset(){
  for(unsigned i=0;i<10;++i)w(0x800F8950+4*i,0);
  w(0x800F89D0,0);w(0x800F8A4C,0);w(0x800F8AC8,0);
  for(unsigned i=0;i<30;++i){w(0x800F89D4+4*i,0);w(0x800F8A50+4*i,0);}
  w(0x800F8D14,0);w(0x800F8C1C,0); // CFC0-owned state outside the carry whitelist
  for(unsigned i=0;i<0x40;++i)mem[(0xF8978+i)^3]=0; // later6D1CC cleanup remains native
 }
};
static bool allowed_byte(unsigned at){
 return (at>=0xF8950&&at<0xF8978)||(at>=0xF89D0&&at<0xF8AC8)||(at>=0xF8AC8&&at<0xF8ACC)||(at>=0xF8AD4&&at<0xF8B10);
}
int main(){
 need(carry::reset_fence("func_8006CFC0",0x8006D228)&&carry::hud_fence("func_800519C0",0x8006884C),"exact native scene reset and pre-HUD fences");
 for(auto ra:{0u,0x8006D220u,0x8006D224u,0x80068844u,0x80068848u})need(!carry::reset_fence("func_8006CFC0",ra)&&!carry::hud_fence("func_800519C0",ra),"wrong or pre-delay-slot native caller refused");
 need(!carry::reset_fence("func_800519C0",0x8006D228)&&!carry::hud_fence("func_8006CFC0",0x8006884C),"native callback names cannot be interchanged");
 for(unsigned count:{0u,1u,15u,29u,30u})for(unsigned backup:{0u,count/2,count}){
  Fixture f(count,backup);carry::Payload p;const auto initial=f.mem;
  need(carry::capture(f.rdram,p)&&carry::valid(p)&&p.count==count&&p.backup_count==backup,"capture valid native ring and checkpoint data including30slots");need(f.mem==initial&&carry::matches(f.rdram,p),"capture and comparison are read-only");
  f.simulate_reset();const auto reset=f.mem;need(carry::restore(f.rdram,p)&&carry::matches(f.rdram,p),"restore coherent IDs/count/scores before HUD creation");
  bool exact=true;for(unsigned i=0;i<f.mem.size();++i)if(f.mem[i]!=(allowed_byte(i)?initial[i]:reset[i])){exact=false;break;}
  need(exact,"entire8MB restored only through primitive carry whitelist; unrelated native reset effects retained");
  const auto restored=f.mem;need(carry::restore(f.rdram,p)&&f.mem==restored,"primitive restore is idempotent; campaign owns one-shot authorization");
 }
 for(unsigned kind=0;kind<13;++kind){Fixture f(1,0);f.w(0x800F89D4,kind);f.h(0x800F8AD4,uint16_t(kind));carry::Payload p;need(carry::capture(f.rdram,p)==(kind<=4||(kind>=6&&kind<=11)),"all five ordinary and six custom native types validated;heart5 and12 rejected");}
 for(unsigned scenario=0;scenario<9;++scenario){Fixture f;carry::Payload p;need(carry::capture(f.rdram,p),"valid negative-test baseline");const auto saved=p;
  switch(scenario){case 0:p.captured=false;break;case 1:p.count=31;break;case 2:p.backup_count=31;break;case 3:p.backup_count=p.count+1;break;case 4:p.ending=1;break;case 5:p.fruits[0]=5;break;case 6:p.half_fruits[0]=11;break;case 7:p.backup_fruits[0]=12;break;case 8:p.fruits[0]=0xFFFFFFFF;break;}
  const auto before=f.mem;need(!carry::valid(p)&&!carry::restore(f.rdram,p)&&!carry::matches(f.rdram,p)&&f.mem==before,"invalid payload rejected before any RDRAM write");
  // Native capture failure also leaves the prior host payload unchanged.
  if(scenario==1||scenario==2||scenario==4||scenario==5||scenario==6||scenario==7){if(scenario==1)f.w(0x800F89D0,31);if(scenario==2)f.w(0x800F8A4C,31);if(scenario==4)f.w(0x800F8AC8,1);if(scenario==5)f.w(0x800F89D4,5);if(scenario==6)f.h(0x800F8AD4,11);if(scenario==7)f.w(0x800F8A50,12);auto output=saved;const auto badram=f.mem;need(!carry::capture(f.rdram,output)&&output==saved&&f.mem==badram,"failed capture commits neither partial payload nor native mutation");}
 }
 {Fixture f;carry::Payload p;need(carry::capture(f.rdram,p),"null-test baseline");const auto before=p;need(!carry::capture(nullptr,p)&&p==before&&!carry::restore(nullptr,p)&&!carry::matches(nullptr,p),"null memory rejects all operations");carry::Payload uninitialized;const auto mem=f.mem;need(!carry::valid(uninitialized)&&!carry::restore(f.rdram,uninitialized)&&f.mem==mem,"uncaptured host defaults cannot clear a game ring");}
 std::printf("BossCarry0260Checks: %u PASS (CPU payload/fence fixtures only; no native gameplay claim)\n",checks);
}
