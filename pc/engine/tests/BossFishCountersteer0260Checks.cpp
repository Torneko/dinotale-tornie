#include "../boss_native.hpp"
#include <bit>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
namespace n=workshop::boss_native;
static unsigned checks=0,native_updates=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
void do_break(uint32_t){std::abort();}
namespace literal {
void func_80023A2C(uint8_t*,recomp_context*);void func_80023DD0(uint8_t*,recomp_context*);void func_80023E18(uint8_t*,recomp_context*);void func_80023E64(uint8_t*,recomp_context*);void func_80023EA0(uint8_t*,recomp_context*);void func_8006593C(uint8_t*,recomp_context*);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#include "native-countersteer.inc"
#pragma clang diagnostic pop
}
struct Fixture {
 std::vector<uint8_t> memory;uint8_t* rdram;uint32_t player=0,priv=0;
 explicit Fixture(const std::vector<uint8_t>& source):memory(source),rdram(memory.data()){
  player=n::word(rdram,0x800F8828);priv=n::word(rdram,player+0x1F4);
  need(n::actor(rdram,player,0x4001)&&n::range(priv,0x1F8),"Real ce3 player/private are valid");
  need(n::word(rdram,priv+0x1F4)==0&&n::word(rdram,priv+0x24)==0,"Actual postlaunch snapshot has no reticle and no force-zero mask");
 }
 void fl(uint32_t p,float value){MEM_W(0,n::base(p))=std::bit_cast<uint32_t>(value);}float f(uint32_t p)const{return std::bit_cast<float>(n::word(rdram,p));}
 void speed(float value){fl(player+0xC4,value);}
 void update(int16_t axis){
  MEM_H(0,n::base(0x800F897E))=axis;const auto before=memory;
  recomp_context c{};c.r29=n::base(0x807EF000);c.r4=n::base(player);literal::func_80023A2C(rdram,&c);need(uint32_t(c.r29)==0x807EF000,"Native command conversion preserves stack");
  c.r4=n::base(player);literal::func_80023EA0(rdram,&c);++native_updates;need(uint32_t(c.r29)==0x807EF000,"Native speed integration preserves stack");
  // All writes occur on the copied test image. Only the actor's velocity,
  // desired velocity, acceleration, direction and native scratch stack vary.
  bool unchanged=true;
  for(unsigned i=0;i<memory.size();++i){const uint32_t a=0x80000000u+i;const bool allowed=(a>=player+0xC4&&a<player+0xCC)||(a>=player+0xDC&&a<player+0xF0)||(a>=player+0x12C&&a<player+0x130)||(a>=0x807EEF00&&a<0x807EF100);if(!allowed&&memory[i]!=before[i]){unchanged=false;break;}}
  need(unchanged,"No pose, parent, HP, inventory, enemies or map changed by the native velocity oracle");
 }
};
int main(int argc,char** argv){try{
 need(argc==2,"Pass real ce3 postloadout capture");std::ifstream file(argv[1],std::ios::binary);need(bool(file),"Real capture opens");std::vector<uint8_t> source((std::istreambuf_iterator<char>(file)),{});need(source.size()==0x800000,"Real capture is exactly8MiB");
 for(float sign:{-1.f,1.f}){
  Fixture neutral(source);neutral.speed(sign*3);for(unsigned i=0;i<20;++i){neutral.update(0);need(neutral.f(neutral.player+0xC4)==sign*3,"Neutral native input with E8zero preserves the observed drift");need(neutral.f(neutral.player+0xDC)==0&&neutral.f(neutral.player+0xE8)==0,"Neutral native input explicitly requests no acceleration");}
  Fixture counter(source);counter.speed(sign*3);float last=sign*3;
  for(unsigned i=0;i<14;++i){counter.update(int16_t(-sign*54));const float now=counter.f(counter.player+0xC4);need(std::abs(now-(sign*3-sign*.23f*float(i+1)))<.00001f,"Actual23A2C/23EA0 apply .23counter acceleration per native update");need(sign*now<sign*last,"Opposite native input reduces the outgoing speed each update");need(counter.f(counter.player+0xDC)==-sign*4&&std::abs(counter.f(counter.player+0xE8)-.23f)<.000001f,"Native counter direction requests target4 and acceleration.23");last=now;}
  need(sign*last<0&&std::abs(last)<.23f,"Drift crosses zero at update14; this is not an exact-stop or landing claim");
  counter.update(0);need(counter.f(counter.player+0xC4)==last,"Airborne neutral after crossing preserves the residual; settling must be observed");
 }
 {Fixture partial(source);partial.speed(3);partial.update(-27);need(std::abs(partial.f(partial.player+0xC4)-2.885f)<.00001f,"Half native axis halves the counter acceleration rather than forcing speed");}
 std::cout<<"BossFishCountersteer0260Checks: "<<checks<<" PASS; "<<native_updates<<" native updates, six unchanged bodies; velocity only, no landing claim\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
