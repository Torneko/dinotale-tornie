#include "../boss_driver0260.hpp"
#include "../boss_native.hpp"
#include <array>
#include <bit>
#include <iostream>
#include <stdexcept>
#include <vector>
#ifndef RETICLE_EXPIRY_NATIVE_SLICE
#error Supply literal nativeA5EC/AAB8/65954 bodies.
#endif
namespace d=workshop::boss_driver0260;namespace n=workshop::boss_native;
static unsigned checks=0,native_calls=0,retirements=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
#undef LOOKUP_FUNC
#define LOOKUP_FUNC(v) lookup(int32_t(v))
namespace native_range {
void func_8003A620(uint8_t*,recomp_context*);void func_8003A524(uint8_t*,recomp_context*);void func_8003A7B0(uint8_t*,recomp_context*);void func_800447C8(uint8_t*,recomp_context*);void func_80065954(uint8_t*,recomp_context*);
void func_8001D770(uint8_t*,recomp_context*);void func_8004700C(uint8_t*,recomp_context*);void func_80067818(uint8_t*,recomp_context*);void func_80067D5C(uint8_t*,recomp_context*);void func_80067D94(uint8_t*,recomp_context*);recomp_func_t* lookup(int32_t);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include RETICLE_EXPIRY_NATIVE_SLICE
#pragma clang diagnostic pop
// Unchanged nativeAAB8 owns the expiry decision. These boundaries provide no
// terrain/contact result, no hit, no boss state and no explosion callback.
void func_8003A620(uint8_t*,recomp_context*){}void func_8003A524(uint8_t*,recomp_context*){}void func_8003A7B0(uint8_t*,recomp_context*){}
void func_800447C8(uint8_t*,recomp_context*){++retirements;}
void func_8001D770(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected particle branch");}void func_8004700C(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected player lookup");}void func_80067818(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected sound branch");}void func_80067D5C(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected particle branch");}void func_80067D94(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected particle branch");}
recomp_func_t* lookup(int32_t){throw std::runtime_error("Unexpected native callback; no synthetic hit is provided");}
}
struct Shot {float px,py,tx,ty,rx,ry,vx,vy;unsigned release_vi,launch_vi;};
static constexpr std::array<Shot,6> shots{{
 {96.098259f,193,246.300720f,143,289,129.207123f,6.266042f,-2.072231f,2700,2700},
 {97.813255f,193,247.200760f,143,179.801895f,165.748688f,6.263021f,-2.081698f,2805,2813},
 {76.277023f,193,196.499695f,163.800003f,160.114059f,172.116547f,6.404218f,-1.595263f,3649,3657},
 {77.642021f,193,217.299698f,156,161.281296f,171.343964f,6.389111f,-1.654280f,3733,3741},
 {79.637024f,193,222.699692f,156,163.276306f,171.343964f,6.389111f,-1.654280f,3817,3825},
 {81.352020f,193,221.199677f,156,164.991302f,171.343964f,6.389111f,-1.654280f,3902,3910}
}};
struct Pilot {
 d::Controller c;d::Input in;d::Target target;
 Pilot(const Shot& s,uint16_t kind=0x40A3):target{0x80060000,kind,s.tx,s.ty,0,0,true,true,false,false,true,6.6f,2.f}{need(c.configure(true,true,{},false),"Diagnostic input pilot configured");target.red_candy=kind==0x455A;target.birth=kind==0x4526?17:0;in.qa_bird=kind==0x4526;in.player=0x80050000;in.room=168;in.encounter=1;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.reserves=6;in.x=s.px;in.y=s.py;in.min_x=40;in.max_x=468;in.has_preferred_x=true;in.preferred_x=in.x;in.targets=std::span<const d::Target>(&target,1);in.vi=1;c.step(in);in.vi=5;c.step(in);in.aiming_actor=0x80061000;in.reticle_actor=0x80062000;in.reticle_x=s.rx;in.reticle_y=s.ry;}
 d::Output step(unsigned vi){in.vi=vi;return c.step(in);}
};
struct Flight {
 static constexpr uint32_t egg=0x80010000,p=0x80011000,meta=0x80012000,stack=0x807B0000;
 std::vector<uint8_t> bytes=std::vector<uint8_t>(0x800000);uint8_t* rdram=bytes.data();recomp_context c{};unsigned updates=0;bool expired=false,reached=false;
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=int32_t(v);}void fl(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}float value(uint32_t a){return std::bit_cast<float>(n::word(rdram,a));}
 Flight(const Shot& s,float reach){c.r29=n::base(stack);c.r31=n::base(0x80000300);w(egg+0x174,meta);w(egg+0x1F4,p);const float native_distance=std::hypot(s.rx-s.px,s.ry-s.py);fl(p+8,s.px+(s.rx-s.px)*reach/native_distance);fl(p+12,s.py+(s.ry-s.py)*reach/native_distance);fl(egg+0x3C,s.px);fl(egg+0x40,s.py);fl(egg+0x44,500.338989f);fl(egg+0x84,s.px);fl(egg+0x88,s.py);fl(egg+0x8C,500.338989f);fl(egg+0xC4,s.vx);fl(egg+0xC8,s.vy);}
 void run(){const auto old_retirements=retirements;for(unsigned tick=0;tick<80;++tick){const auto sp=c.r29,ra=c.r31;c.r4=n::base(egg);native_range::func_8003AAB8(rdram,&c);++native_calls;need(c.r29==sp&&c.r31==ra,"ActualAAB8 preserves stack and RA");if(retirements!=old_retirements){expired=true;break;}
   // Read-only geometric box-crossing of the captured40A7-scale model, not
   // a native hit callback. It is checked only while nativeAAB8 keeps flying.
   reached|=std::abs(value(egg+0x84)-210.101013f)<=26+7.84f&&std::abs(value(egg+0x88)-141)<=26+8.96f;
   c.r4=n::base(egg);native_range::func_8003A5EC(rdram,&c);++native_calls;++updates;for(unsigned axis=0;axis<3;++axis)w(egg+0x84+axis*4,n::word(rdram,egg+0x3C+axis*4));
  }}
};
int main(){try{
 for(unsigned i=0;i<shots.size();++i){const auto& s=shots[i];const float range=std::hypot(s.rx-s.px,s.ry-s.py),distance=std::hypot(s.tx-s.px,s.ty-s.py),speed=std::hypot(s.vx,s.vy),cross=std::abs(s.vx*(s.ty-s.py)-s.vy*(s.tx-s.px))/speed;need(cross<2&&std::abs(speed-6.6f)<.001f,"All six actual native launch vectors were correctly aligned: angle alone was insufficient");need((range>=distance)==(i==0),"Only the first actual shot had enough reticle reach for its observed target");Pilot p(s);for(unsigned vi=6;vi<9;++vi)p.step(vi);need(i==0?p.c.snapshot().phase==d::Phase::Release:p.c.snapshot().phase==d::Phase::TrackAim,"Range-aware release accepts the real203px case and retains each real86px case");if(i){for(unsigned vi=9;vi<=126;++vi)p.step(vi);need(p.c.snapshot().phase==d::Phase::CancelAim&&p.c.snapshot().launches==0,"A short native reticle timeout cancels without fabricating a shot");}need(s.launch_vi-s.release_vi==(i?8u:0u),"Fixture preserves the actual release-to-native-launch delay");}
 for(uint16_t kind:{uint16_t(0x4599),uint16_t(0x4072),uint16_t(0x40A3),uint16_t(0x41C6),uint16_t(0x4511),uint16_t(0x4631),uint16_t(0x455A),uint16_t(0x4526)})for(const auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress}){Pilot p(shots[5],kind);p.in.throw_mode=mode;for(unsigned vi=6;vi<=20;++vi)p.step(vi);need(p.c.snapshot().phase==d::Phase::TrackAim&&p.c.snapshot().aim_matches==0,"Every actual4002 shot family and both input modes wait for native reticle range");p.in.reticle_x=p.target.x;p.in.reticle_y=p.target.y;for(unsigned vi=21;vi<=23;++vi)p.step(vi);need(p.c.snapshot().phase==d::Phase::Release&&p.c.snapshot().launches==0,"Three true angle-and-range matches release ordinary Z only");}
 {Flight short_shot(shots[5],std::hypot(shots[5].rx-shots[5].px,shots[5].ry-shots[5].py));short_shot.run();need(short_shot.expired&&short_shot.updates==13&&!short_shot.reached,"LiteralAAB8 expires the real86px shot after13 native moves before captured weak-point volume");need(std::abs(short_shot.value(Flight::egg+0x84)-164.410507f)<.002f&&std::abs(short_shot.value(Flight::egg+0x88)-171.494339f)<.002f,"Literal native motion/expiry reproduces finalRAM retired eggXY");Flight long_shot(shots[5],200);long_shot.run();need(long_shot.expired&&long_shot.reached&&long_shot.updates>short_shot.updates,"With genuine200px reticle range the same native flight reaches the box model before native expiry");}
 {Pilot p(shots[0],0x4511);p.in.reticle_x=p.in.x-300;p.in.reticle_y=p.in.y;for(unsigned vi=6;vi<=126;++vi)p.step(vi);need(p.c.snapshot().phase==d::Phase::CancelAim&&p.c.snapshot().launches==0,"Long enough but wrong-angle native reticle also cancels at timeout");}
 std::cout<<"BossReticleRange0260Checks: "<<checks<<" PASS; literalNativeCalls="<<native_calls<<"; no native damage/collision/victory fabricated\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
