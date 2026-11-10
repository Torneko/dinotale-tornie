// CPU-only: verbatim native phase/retraction/extension functions, isolated
// from rendering, collision and food. This is not an ingestion proof.
#include "boss_rest_aim0260.hpp"
#include <array>
#include <bit>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#ifndef BOSS_TONGUE_NATIVE_SLICE
#error Supply the exact generated native slice.
#endif
#include BOSS_TONGUE_NATIVE_SLICE
namespace aim=workshop::boss_rest_aim0260;
static unsigned checks=0;static constexpr uint32_t player=0x80100000,priv=0x80110000,body=0x80120000,tip=0x80121000,meta=0x80122000;
static void need(bool b,const char* why){++checks;if(!b)throw std::runtime_error(why);}
extern "C" void func_8004700C(uint8_t*,recomp_context* c){c->r2=int32_t(player);}
// These are presentation-only or branches outside the159=2/1 fixture.
extern "C" void func_8003D090(uint8_t*,recomp_context*){}
extern "C" void func_8003CE84(uint8_t*,recomp_context*){}
extern "C" void func_8003CF70(uint8_t*,recomp_context*){std::abort();}
extern "C" void func_800327D0(uint8_t*,recomp_context*){std::abort();}
extern "C" void func_80032878(uint8_t*,recomp_context*){std::abort();}
extern "C" void func_8001D770(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_80067E98(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_80067818(uint8_t*,recomp_context*){}
struct Result{unsigned reversal=0;float maximum=0;};
int main(int argc,char**argv){try{
 need(argc==2,"Original USA ROM path required");std::ifstream f(argv[1],std::ios::binary);need(bool(f),"Original ROM readable");std::vector<uint8_t> rom((std::istreambuf_iterator<char>(f)),{});need(rom.size()==0x1000000,"Original ROM exact16MiB");
 auto be32=[&](uint32_t a){const auto p=a-0x80000400+0x1000;need(p+4<=rom.size(),"ROM constant bounded");return(uint32_t(rom[p])<<24)|(uint32_t(rom[p+1])<<16)|(uint32_t(rom[p+2])<<8)|rom[p+3];};
 for(unsigned slow:{0u,1u})for(float scale:{.5f,.8f,1.f}){
  auto run=[&](unsigned pulse,bool controlled){
   std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();auto w=[&](uint32_t a,uint32_t v){MEM_W(0,int32_t(a))=int32_t(v);};auto fl=[&](uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));};auto read=[&](uint32_t a){return std::bit_cast<float>(uint32_t(MEM_W(0,int32_t(a))));};
   for(auto a:{0x800B0214u,0x800B0218u,0x800B021Cu,0x800B01FCu,0x8009EF60u,0x8009EF64u})w(a,be32(a));
   need(std::abs(read(0x800B0214)-1.9f)<1e-6f&&std::abs(read(0x8009EF60)-.15f)<1e-6f&&std::abs(read(0x8009EF64)-.10f)<1e-6f,"Real ROM extension initial speed and native deceleration");
   w(player+0x1F4,priv);w(player+0x174,meta);w(tip+0x174,meta+0x10);w(priv+0x160,body);w(priv+0x15C,tip);w(body+0x180,player);w(body+0x1F4,meta+0x20);w(tip+0x1F4,meta+0x40);fl(player+0x6C,scale);fl(player+0x70,scale);fl(body+0x78,.2f);fl(body+0x7C,1.f);w(0x800F8944,slow);MEM_B(0x156,int32_t(priv))=1;
   recomp_context c{};c.r29=int32_t(0x807B0000);c.r4=int32_t(player);c.r5=int32_t(priv);func_8003D6E4(rdram,&c);need(MEM_BU(0x159,int32_t(priv))==2,"Native start enters extending phase2");c.r4=int32_t(body);func_8003D54C(rdram,&c);need(std::abs(read(body+0xC4)-scale*1.9f)<1e-6f,"Real native initializer supplies outgoing tongue speed");
   aim::Controller controller;controller.configure(true,true);aim::Input in;in.room=83;in.visit=1;in.player=player;in.fruit=0x80140000;in.active=in.controls=in.alive=in.stick_observed=true;in.health=5;in.tongue_observed=in.tongue_live=true;in.max_reach=*workshop::boss_tongue_geometry0260::max_centre_reach(scale,.2f,slow);in.heading_degrees=0;in.min_x=40;in.max_x=724;Result result;
   for(unsigned vi=0;vi<64;vi++){
    in.vi=vi;in.mouth_phase=MEM_BU(0x159,int32_t(priv));bool held=vi<pulse;
    if(controlled){const auto o=controller.step(in);need(o.handled&&std::isfinite(o.x)&&std::isfinite(o.y),"Extension maintenance supplies bounded native heading axes");need(o.buttons==(in.mouth_phase==2?aim::B:0),"B follows observed native phase, not a fixed timer");held=o.buttons==aim::B;}
    MEM_B(0,int32_t(0x800F8997))=held; c.r4=int32_t(player);func_800328C0(rdram,&c);
    if(!controlled&&vi==pulse&&in.mouth_phase==2)need(MEM_BU(0x159,int32_t(priv))==1,"Releasing B changes real native extension to retraction");
    const bool retracting=MEM_BU(0x159,int32_t(priv))==1;c.r4=int32_t(body);func_8003D0B0(rdram,&c);result.maximum=std::max(result.maximum,read(body+0x6C));
    if(retracting||MEM_BU(0x159,int32_t(priv))==1){result.reversal=vi+1;break;}
   }need(result.reversal>0,"Native velocity changes extension to retraction without a helper deadline");need(uint32_t(c.r29)==0x807B0000,"Native stack restored");return result;
  };
  const auto pulse=run(1,false),short_hold=run(4,false),native_hold=run(64,false),controller_hold=run(0,true);
  need(pulse.maximum<short_hold.maximum&&short_hold.maximum<native_hold.maximum,"One/four-VI B truncates true native tongue extension");need(controller_hold.reversal==native_hold.reversal&&std::abs(controller_hold.maximum-native_hold.maximum)<1e-6f,"Controller maintains the complete original native cycle");
  need(slow?(native_hold.reversal>=20&&native_hold.reversal<=21):native_hold.reversal==14,"Actual ROM velocity model reaches its native reversal, allowing float crossing on20/21 for slow mode");
  std::cout<<"slow="<<slow<<" scale="<<scale<<" nativeUpdates="<<native_hold.reversal<<" maxScale="<<native_hold.maximum<<" pulseMax="<<pulse.maximum<<" fourVIMax="<<short_hold.maximum<<'\n';
 }
 std::cout<<"BossNativeTongueHold0260 "<<checks<<" CPU checks passed; no physical collision or ingestion proof\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
