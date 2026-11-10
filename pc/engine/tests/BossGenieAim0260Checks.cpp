#include "../boss_genie_aim0260.hpp"
#include "../boss_driver0260.hpp"
#include <bit>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>
#ifndef GENIE_NATIVE_MOTION_SLICE
#error Supply literal native800483B8 and8003A5EC bodies.
#endif
namespace literal {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include GENIE_NATIVE_MOTION_SLICE
#pragma clang diagnostic pop
}
namespace g=workshop::boss_genie_aim0260;namespace d=workshop::boss_driver0260;namespace n=workshop::boss_native;
static unsigned checks=0,native_calls=0;
static void need(bool yes,const char* why){++checks;if(!yes)throw std::runtime_error(why);}
static bool near(float a,float b,float e=.001f){return std::abs(a-b)<=e;}
struct Memory {
 std::vector<uint8_t> bytes;uint8_t* rdram;
 explicit Memory(const char* path){std::ifstream stream(path,std::ios::binary);bytes.assign(std::istreambuf_iterator<char>(stream),{});need(bytes.size()==0x800000,"Captured native graph must be exactly8MiB");rdram=bytes.data();}
 explicit Memory(const std::vector<uint8_t>& source):bytes(source),rdram(bytes.data()){}
 void word(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}
 void byte(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}
 void half(uint32_t a,unsigned v){MEM_H(0,n::base(a))=uint16_t(v);}
 void fl(uint32_t a,float v){word(a,std::bit_cast<uint32_t>(v));}
 float fl(uint32_t a)const{return std::bit_cast<float>(n::word(rdram,a));}
};
static constexpr uint32_t root=0x800CB31C;
static float ballistic(const d::Input& origin,const d::Target& target,bool predict){
 std::vector<uint8_t> bytes(0x800000);auto* rdram=bytes.data();constexpr uint32_t egg=0x80010000,moving=0x80010400;auto put=[&](uint32_t a,float v){MEM_W(0,n::base(a))=std::bit_cast<uint32_t>(v);};auto value=[&](uint32_t a){return std::bit_cast<float>(n::word(rdram,a));};
 float tx=target.x,ty=target.y;if(predict)d::predicted_target(origin,target,tx,ty);const float length=std::hypot(tx-origin.x,ty-origin.y);
 put(egg+0x3C,origin.x);put(egg+0x40,origin.y);put(egg+0xC4,6.6f*(tx-origin.x)/length);put(egg+0xC8,6.6f*(ty-origin.y)/length);put(moving+0x3C,target.x);put(moving+0x40,target.y);put(moving+0xC4,target.vx);put(moving+0xC8,target.vy);
 float best=1e9f;recomp_context context{};
 for(unsigned step=0;step<=60;++step){best=std::min(best,std::hypot(value(egg+0x3C)-value(moving+0x3C),value(egg+0x40)-value(moving+0x40)));for(auto a:{egg,moving}){context.r4=n::base(a);literal::func_8003A5EC(rdram,&context);++native_calls;}}
 return best;
}
int main(int argc,char** argv){try{
 need(argc==4,"Supply actual4315 snapshots3000/3600 and5176 snapshot3000");
 for(unsigned snapshot=1;snapshot<4;++snapshot){Memory captured(argv[snapshot]);const auto unchanged=captured.bytes;const auto data=n::word(captured.rdram,root+0x1F4);const auto observed=g::observe(captured.rdram,root,data,77);
  need(bool(observed),"The unmodified actual4072 source graph admits a read-only motion observation");
  need(near(observed->x,captured.fl(root+0x84))&&near(observed->y,captured.fl(root+0x88)-10*captured.fl(root+0x70)),"Actual native collider centre includes the source Y offset minus10 scaled by actor70");
  need(near(observed->vx,captured.fl(root+0xC4)*captured.fl(root+0x100)+captured.fl(root+0x10C))&&near(observed->vy,captured.fl(root+0xC8)*captured.fl(root+0x104)+captured.fl(root+0x110)),"Actual moving root supplies non-invented per-native-update velocity");
  need(captured.bytes==unchanged,"Observing actual native motion preserves every captured byte");
  Memory pass(unchanged);const float x=pass.fl(root+0x3C),y=pass.fl(root+0x40);recomp_context context{};context.r4=n::base(root);literal::func_800483B8(pass.rdram,&context);++native_calls;
  need(near(pass.fl(root+0x3C)-x,observed->vx,.002f)&&near(pass.fl(root+0x40)-y,observed->vy,.002f),"Literal native483B8 integration agrees with observed motion on the actual graph");
 }
 Memory actual(argv[1]);const auto baseline=actual.bytes;const auto data=n::word(actual.rdram,root+0x1F4),resource=n::word(actual.rdram,root+0x168),metadata=n::word(actual.rdram,root+0x174),definition=n::word(actual.rdram,metadata);const auto factory=n::word(actual.rdram,resource+0x40);
 for(unsigned fault=0;fault<18;++fault){Memory f(baseline);uint32_t actor=root,private_data=data;uint64_t birth=77;
  if(fault==0)birth=0;if(fault==1)private_data+=4;if(fault==2)f.half(root+12,0x4599);if(fault==3)f.byte(root+0x17D,0);if(fault==4)f.byte(root+0x18,0xFF);if(fault==5)f.word(root+0x180,0x80010000);if(fault==6)f.word(root+0x130,0x8003A5EC);if(fault==7)f.word(resource+0x44,factory+0x2AA0);if(fault==8)f.half(resource+0x3C,0x4599);if(fault==9)f.word(definition+0x28,factory+0x293C);if(fault==10)f.word(definition+0x70,root+0x78);if(fault==11)f.half(definition+6,0);if(fault==12)f.half(definition+10,25);if(fault==13)f.fl(root+0xC4,std::numeric_limits<float>::infinity());if(fault==14)f.fl(root+0x84,std::numeric_limits<float>::quiet_NaN());if(fault==15)f.fl(root+0x70,0);if(fault==16)f.fl(root+0xC8,17);if(fault==17)actor=0;
  const auto before=f.bytes;need(!g::observe(f.rdram,actor,private_data,birth)&&f.bytes==before,"Foreign/broken/nonfinite native source identity cannot license predictive motion or write RAM");
 }
 {Memory f(baseline);f.fl(root+0xC4,2);f.fl(root+0xC8,-3);f.fl(root+0x100,1.5f);f.fl(root+0x104,.5f);f.fl(root+0x10C,.25f);f.fl(root+0x110,.75f);const auto observed=g::observe(f.rdram,root,data,77);need(observed&&near(observed->vx,3.25f)&&near(observed->vy,-.75f),"Distinct X100/Y104 scales and additive offsets cannot be interchanged");const float x=f.fl(root+0x3C),y=f.fl(root+0x40);recomp_context context{};context.r4=n::base(root);literal::func_800483B8(f.rdram,&context);++native_calls;need(near(f.fl(root+0x3C)-x,3.25f)&&near(f.fl(root+0x40)-y,-.75f),"Literal native483B8 independently verifies the constructed asymmetric-scale fixture");}
 d::Input origin;origin.x=328;origin.y=673;
 for(const auto velocity:std::array<std::array<float,2>,3>{{{2,0},{-2,.7f},{1.5f,-1}}}){d::Target target{root,0x4072,450,500,velocity[0],velocity[1],true,true,false,false,true,6.6f,2.f};float tx=0,ty=0;d::predicted_target(origin,target,tx,ty);const float flight=std::hypot(tx-origin.x,ty-origin.y)/6.6f;
  need(near(tx,target.x+target.vx*flight,.02f)&&near(ty,target.y+target.vy*flight,.02f),"Opt-in4072 prediction solves interception at the measured native projectile speed6.6");const float previous=std::clamp(std::hypot(target.x-origin.x,target.y-origin.y)/14.f,0.f,24.f);need(std::hypot(tx-(target.x+target.vx*previous),ty-(target.y+target.vy*previous))>5,"Measured6.6 travel differs materially from the old generic14 lead");const float predicted=ballistic(origin,target,true),unled=ballistic(origin,target,false);need(predicted<5&&unled>predicted+10,"Literal native projectile updates intercept the explicit constant-motion model; no claim of full Genie AI/collision");
 }
 for(uint16_t kind:{uint16_t(0x4072),uint16_t(0x4599),uint16_t(0x4511)}){d::Target target{1,kind,450,500,2,.5f,true,true,false,false,true,0,0};float tx=0,ty=0;d::predicted_target(origin,target,tx,ty);const float legacy=std::clamp(std::hypot(target.x-origin.x,target.y-origin.y)/14.f,0.f,24.f);need(near(tx,target.x+2*legacy)&&near(ty,target.y+.5f*legacy),"Unobserved/default native_speed0 retains legacy predictions in Genie and other families");}
 {d::Controller controller;need(controller.configure(true,true,{},false),"Composed predictor remains an input-only diagnostic controller");d::Target target{root,0x4072,450,500,2,0,true,true,false,false,true,6.6f,2.f};d::Input shot=origin;shot.targets=std::span<const d::Target>(&target,1);shot.player=0x80070000;shot.room=169;shot.encounter=2;shot.combat=shot.controls=shot.alive=shot.grounded=true;shot.primary_state=1;shot.reserves=1;shot.min_x=40;shot.max_x=724;shot.has_preferred_x=true;shot.preferred_x=shot.x;shot.vi=100;controller.step(shot);shot.vi=104;controller.step(shot);shot.aiming_actor=0x80071000;shot.reticle_actor=0x80072000;shot.reticle_x=target.x;shot.reticle_y=target.y;for(unsigned vi=105;vi<108;++vi){shot.vi=vi;controller.step(shot);}need(controller.snapshot().phase==d::Phase::TrackAim&&controller.snapshot().aim_matches==0,"Reticle reaching only the old current4072 position cannot release the predictive shot");d::predicted_target(shot,target,shot.reticle_x,shot.reticle_y);for(unsigned vi=108;vi<111;++vi){shot.vi=vi;controller.step(shot);}need(controller.snapshot().phase==d::Phase::Release&&!controller.snapshot().launches,"Three native-reticle future-point matches release ordinary input without claiming a native launch/hit");}
 need(actual.bytes==baseline,"All actual immutable4315 graph observations remain byte-exact");std::cout<<"BossGenieAim0260Checks: "<<checks<<" PASS; literalNativeMotionCalls="<<native_calls<<"; actual graph plus explicit constant-motion models only; no GPU/Genie victory/Full10 proof.\n";return 0;
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
