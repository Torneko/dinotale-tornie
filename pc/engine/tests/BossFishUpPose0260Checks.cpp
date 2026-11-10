#include "../boss_fish_green0260.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
#ifndef UP_POSE_NATIVE_SLICE
#error Supply literal native662B8 and80440 matrix bodies
#endif
namespace literal {
void func_80066DE4(uint8_t*,recomp_context* c){c->f0.fl=std::sin(c->f12.fl*3.14159265358979323846f/180.f);}
void func_80066E88(uint8_t*,recomp_context* c){c->f0.fl=std::cos(c->f12.fl*3.14159265358979323846f/180.f);}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include UP_POSE_NATIVE_SLICE
#pragma clang diagnostic pop
}
namespace p=workshop::boss_fish_up_pose0260;namespace n=workshop::boss_native;namespace g=workshop::boss_fish_green0260;
unsigned checks=0;void need(bool result,const char* message){++checks;if(!result)throw std::runtime_error(message);}
struct Fixture {
 std::vector<uint8_t> bytes=std::vector<uint8_t>(0x800000);uint8_t* rdram=bytes.data();
 static constexpr uint32_t actor=0x80010000,drawing=0x80012000,flags=0x80013000,private_data=0x80014000,resource=0x80015000,matrix=0x80020000,factory=0x80200000;
 g::Frame frame{true,true,true,97,actor,private_data,123};
 void w(uint32_t at,uint32_t value){MEM_W(0,n::base(at))=value;}void fl(uint32_t at,float value){w(at,std::bit_cast<uint32_t>(value));}
 Fixture(){w(actor,0x19971110);MEM_H(12,n::base(actor))=0x4511;w(drawing,flags);MEM_B(0x17D,n::base(actor))=2;MEM_B(0x18,n::base(actor))=1;MEM_B(0x30,n::base(actor))=4;MEM_B(0x164,n::base(actor))=2;MEM_B(0,n::base(0x800F7EA6))=1;w(actor+0x168,resource);w(actor+0x16C,drawing);w(actor+0x1F4,private_data);w(actor+0x20,factory+(g::updates[4]-0x805C64F0));MEM_H(0x3C,n::base(resource))=0x4511;w(resource+0x40,factory);w(resource+0x44,factory+0x35A8);w(drawing+0x34,factory+0x10);fl(actor+0x50,315);fl(actor+0x98,315);fl(actor+0xC8,-5.5f);fl(actor+0x84,312);fl(actor+0x88,200);}
 float mouth_y(unsigned native_flags,float degrees){
  for(unsigned i=0;i<16;++i)fl(matrix+i*4,i%5==0?1.f:0.f);
  recomp_context c{};c.r29=n::base(0x807C0000);c.r4=n::base(matrix);c.r5=std::bit_cast<uint32_t>((native_flags&1)?-1.f:1.f);c.r6=std::bit_cast<uint32_t>((native_flags&2)?-1.f:1.f);
  literal::func_80080440(rdram,&c);c.r4=n::base(matrix);c.r5=std::bit_cast<uint32_t>(degrees);c.r6=0;c.r7=1;
  literal::func_800662B8(rdram,&c);
  // The fish mouth is local-X. Native5E6E0 makes screen-up positive
  // renderY. Evaluate the actual native matrix, not an assumed angle sign.
  return -std::bit_cast<float>(n::word(rdram,matrix+4));
 }
};
int main(){try{
 for(unsigned frame=0;frame<2;++frame)for(unsigned flags=0;flags<4;++flags){Fixture f;f.w(Fixture::actor+0x138,frame);f.w(Fixture::flags+frame*4,flags);const auto before=f.bytes;const auto pose=p::world_z(f.rdram,Fixture::actor,Fixture::drawing);need(pose.has_value(),"Native frame/mirror witness admitted");need(f.bytes==before,"Pose selection never mutates RAM");const auto ticket=g::draw_begin(f.rdram,f.frame);need(ticket.active&&ticket.replacement==*pose,"Owned vertical Green draw installs native-facing-aware pose");auto expected=before;auto* rdram=expected.data();MEM_W(0x98,n::base(Fixture::actor))=*pose;need(f.bytes==expected,"Entire guest RAM differs ONLY at worldZ; AI, trajectory, collisions, tint and position stay exact");need(g::draw_end(f.rdram,ticket)&&f.bytes==before,"Draw exit restores every guest byte");need(std::bit_cast<float>(*pose)==((flags&2)?90.f:270.f),"Vertical native mirror selects exact upward worldZ without changing horizontal facing");need(f.mouth_y(flags,std::bit_cast<float>(*pose))>.999f,"Literal native mirror+Z rotation points local-X mouth UP for both native facing frames");if(flags<2)need(f.mouth_y(flags,90.f)<-.999f,"Previous90-degree pose reproduces DOWN direction with either native horizontal mirror");}
 for(unsigned fault=0;fault<12;++fault){Fixture f;uint32_t actor=Fixture::actor,drawing=Fixture::drawing;uint8_t* ram=f.rdram;switch(fault){case 0:ram=nullptr;break;case 1:f.w(actor,0);break;case 2:f.w(actor+12,0x46310000);break;case 3:drawing=0;break;case 4:drawing+=2;break;case 5:f.w(actor+0x138,2);break;case 6:f.w(drawing,0);break;case 7:f.w(drawing,Fixture::flags+2);break;case 8:f.w(Fixture::flags,4);break;case 9:f.fl(actor+0x90,1);break;case 10:f.fl(actor+0x94,1);break;case 11:f.fl(actor+0x94,std::numeric_limits<float>::quiet_NaN());break;}const auto before=f.bytes;need(!p::world_z(ram,actor,drawing)&&f.bytes==before,"Unknown frame/flags/tilt/pointer/foreign actor rejected without writes");}
  for(unsigned fault=0;fault<8;++fault){Fixture f;switch(fault){case 0:f.frame.green=false;break;case 1:f.frame.combat_active=false;break;case 2:f.frame.room=162;break;case 3:f.frame.birth=0;break;case 4:f.w(Fixture::private_data+0x40,Fixture::actor);break;case 5:{auto* rdram=f.rdram;MEM_B(0,n::base(0x800F7EA6))=0;break;}case 6:f.w(Fixture::drawing+0x34,Fixture::factory+0x14);break;case 7:{auto* rdram=f.rdram;MEM_B(0x30,n::base(Fixture::actor))=13;break;}}const auto before=f.bytes;need(!g::draw_begin(f.rdram,f.frame).active&&f.bytes==before,"Nonvertical/foreign/paused/passenger/invalid renderer cannot change draw pose");}
 {Fixture f;const auto outer=g::draw_begin(f.rdram,f.frame),inner=g::draw_begin(f.rdram,f.frame);need(outer.active&&inner.active&&g::draw_end(f.rdram,inner)&&n::word(f.rdram,Fixture::actor+0x98)==outer.replacement,"Nested native drawing restores outer temporary pose");need(g::draw_end(f.rdram,outer)&&n::word(f.rdram,Fixture::actor+0x98)==std::bit_cast<uint32_t>(315.f),"Final outer exit restores original native angle");}
 {Fixture f;const auto ticket=g::draw_begin(f.rdram,f.frame);f.fl(Fixture::actor+0x98,17);const auto before=f.bytes;need(!g::draw_end(f.rdram,ticket)&&f.bytes==before,"A stale draw ticket cannot overwrite a new native angle");}std::cout<<"BossFishUpPose0260Checks: "<<checks<<" PASS; literal native80440+662B8, trigonometric services are host doubles; no GPU image claim.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
