#include "../boss_fish_explosion0260.hpp"
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
#ifndef FISH_NATIVE_BOX_SLICE
#error Supply literal original49018/5FEBC/6009C/65954 bodies
#endif
namespace actual_native {
void func_80071380(uint8_t*,recomp_context*){throw std::runtime_error("Null native actor path is forbidden");}
void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Null native actor path is forbidden");}
void func_80049018(uint8_t*,recomp_context*);void func_8005FEBC(uint8_t*,recomp_context*);void func_8006009C(uint8_t*,recomp_context*);void func_80065954(uint8_t*,recomp_context*);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include FISH_NATIVE_BOX_SLICE
#pragma clang diagnostic pop
}
namespace q=workshop::boss_fish_explosion0260;namespace n=workshop::boss_native;
static unsigned checks=0,native_calls=0;static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
struct Fixture {
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* r=memory.data();
 static constexpr uint32_t pulse=0x80010000,fish=0x80011000,pdef=0x80030000,fdef=0x80030100,pmeta=0x80040000,fmeta=0x80040040,priv=0x80050000,drawing=0x80060000,table=0x80060100,scratch=0x80070000;
 void w(uint32_t a,uint32_t x){auto* rdram=r;MEM_W(0,n::base(a))=int32_t(x);}void h(uint32_t a,int16_t x){auto* rdram=r;MEM_H(0,n::base(a))=x;}void by(uint32_t a,unsigned x){auto* rdram=r;MEM_B(0,n::base(a))=uint8_t(x);}void fl(uint32_t a,float x){w(a,std::bit_cast<uint32_t>(x));}float f(uint32_t a)const{return std::bit_cast<float>(n::word(r,a));}
 Fixture(){for(auto a:{pulse,fish}){w(a,0x19971110);h(a+12,a==pulse?0x4018:0x4511);by(a+0x17D,2);by(a+0x18,a==pulse?0x68:1);w(a+0x174,a==pulse?pmeta:fmeta);fl(a+0x84,a==pulse?100:150);fl(a+0x88,a==pulse?100:90);fl(a+0x8C,a==pulse?500:505);}w(pmeta,pdef);w(fmeta,fdef);w(pulse+0x1F4,priv);w(pdef+0x70,priv+0x14);w(fdef+0x70,fish+0x6C);for(auto d:{pdef,fdef})by(d+0x74,1);const int dims[2][3]={{10,12,16},{30,40,24}};const float scales[2][3]={{2,.75f,1},{1.5f,1.25f,.5f}};for(unsigned j=0;j<3;++j){h(pdef+0xA+2*j,int16_t(dims[0][j]));h(fdef+0xA+2*j,int16_t(dims[1][j]));fl(priv+0x14+4*j,scales[0][j]);fl(fish+0x6C+4*j,scales[1][j]);}h(fdef+4,6);h(fdef+6,-8);h(fdef+8,2);}
 bool native(){auto copy=memory;auto* rdram=copy.data();auto wf=[&](uint32_t a,float x){MEM_W(0,n::base(a))=int32_t(std::bit_cast<uint32_t>(x));};recomp_context c{};c.r29=n::base(0x807EF000);c.r4=n::base(pulse);c.r5=n::base(pmeta);actual_native::func_8005FEBC(rdram,&c);wf(scratch+0x24,c.f0.fl);for(unsigned j=1;j<3;++j)wf(scratch+0x24+4*j,f(pulse+0x84+4*j)+float(MEM_H(4+2*j,n::base(pdef)))*f(priv+0x14+4*j));for(unsigned j=0;j<3;++j)wf(scratch+0x30+4*j,float(MEM_H(0xA+2*j,n::base(pdef)))*f(priv+0x14+4*j));MEM_W(0,n::base(scratch+0x3C))=n::base(fish);MEM_W(0,n::base(scratch+0x40))=n::base(fmeta);c.r4=n::base(scratch);actual_native::func_8006009C(rdram,&c);++native_calls;return c.r2!=0;}
 void compare_bird(const char* why){const auto before=memory;const auto overlap=q::overlaps_bird(r,pulse,fish);need(bool(overlap)&&*overlap==native(),why);need(memory==before,"Bird volume observer preserves complete8MiB fixture");}
 void compare(const char* why){const auto before=memory;const auto overlap=q::overlaps(r,pulse,fish);need(bool(overlap)&&*overlap==native(),why);need(memory==before,"Read-only helper preserves complete8MiB fixture");}
};
int main(){try{
 {Fixture x;x.compare("Native6009C overlap matches full3D current scaled collision boxes");need(q::overlaps(x.r,Fixture::pulse,Fixture::fish)==true,"Known physical overlap, no global radius");x.fl(Fixture::priv+0x14,.5f);x.compare("Shrinking real pulse scale shrinks actual splash volume");need(q::overlaps(x.r,Fixture::pulse,Fixture::fish)==false,"Visual/dynamic scale does not inherit invented radius");}
 for(unsigned axis=0;axis<3;++axis){Fixture x;auto* rdram=x.r;const float half= x.f(Fixture::priv+0x14+4*axis)*float(MEM_H(0xA+2*axis,n::base(Fixture::pdef)))+x.f(Fixture::fish+0x6C+4*axis)*float(MEM_H(0xA+2*axis,n::base(Fixture::fdef)));const float offset=x.f(Fixture::fish+0x6C+4*axis)*float(MEM_H(4+2*axis,n::base(Fixture::fdef)));const float world=x.f(Fixture::pulse+0x84+4*axis)+half-offset;x.fl(Fixture::fish+0x84+4*axis,world);x.compare("Exact axis-face touching is inclusive like native6009C");need(q::overlaps(x.r,Fixture::pulse,Fixture::fish)==true,"Exact face touch remains inside native volume");x.fl(Fixture::fish+0x84+4*axis,std::nextafter(world,std::numeric_limits<float>::infinity()));x.compare("One representable step beyond each native axis separates");need(q::overlaps(x.r,Fixture::pulse,Fixture::fish)==false,"No tolerance silently expands splash volume");}
 {Fixture x;x.w(Fixture::fish+0x16C,Fixture::drawing);x.w(Fixture::drawing,Fixture::table);x.w(Fixture::table,1);x.h(Fixture::pdef+4,-3);x.w(Fixture::pulse+0x16C,Fixture::drawing);x.compare("Original49018/5FEBC mirrors signed source and target offsets exactly");x.by(Fixture::fish+0x164,7);x.by(Fixture::pulse+0x164,7);x.compare("Native class7 explicitly disables mirror");}
 for(unsigned fault=0;fault<12;++fault){Fixture x;switch(fault){case 0:x.w(Fixture::priv,1);break;case 1:x.by(Fixture::pulse+0x18,1);break;case 2:x.by(Fixture::pulse+0x17D,3);break;case 3:x.by(Fixture::pdef+0x74,0);break;case 4:x.w(Fixture::pulse+0x174,0x807FFFFF);break;case 5:x.w(Fixture::fdef+0x70,0x807FFFFC);break;case 6:x.fl(Fixture::priv+0x14,std::numeric_limits<float>::infinity());break;case 7:x.fl(Fixture::fish+0x84,std::numeric_limits<float>::quiet_NaN());break;case 8:x.h(Fixture::fdef+0xA,-1);break;case 9:x.w(Fixture::pdef+0x70,Fixture::priv+0x10);break;case 10:x.h(Fixture::fish+12,0x4001);break;case 11:x.w(Fixture::fish+0x16C,Fixture::drawing);x.w(Fixture::drawing,0x807FFFFC);x.w(Fixture::fish+0x138,0xFFFFFFFF);break;}const auto before=x.memory;need(!q::overlaps(x.r,Fixture::pulse,Fixture::fish).has_value()&&x.memory==before,"Visual/stale/inactive/foreign/corrupt/nonfinite volume fails closed without RAM writes");}
 {Fixture x;x.h(Fixture::fish+12,0x4526);x.compare_bird("Original6009C matches native4526 scaled3D overlap");need(q::overlaps_bird(x.r,Fixture::pulse,Fixture::fish)==true&&!q::overlaps(x.r,Fixture::pulse,Fixture::fish),"Bird-only API admits4526 while Fish API keeps its original kind gate");x.w(Fixture::fish+0x16C,Fixture::drawing);x.w(Fixture::drawing,Fixture::table);x.w(Fixture::table,1);x.compare_bird("Original49018/5FEBC mirrors native4526 collision offset");x.fl(Fixture::fish+0x8C,600);x.compare_bird("Native bird far inZ remains outside the real explosion volume");need(q::overlaps_bird(x.r,Fixture::pulse,Fixture::fish)==false,"No2D approximation reaches off-plane birds");}
 for(unsigned fault=0;fault<5;++fault){Fixture x;x.h(Fixture::fish+12,0x4526);if(fault==0)x.h(Fixture::fish+12,0x46DA);if(fault==1)x.h(Fixture::fish+12,0x4511);if(fault==2)x.by(Fixture::fdef+0x74,0);if(fault==3)x.by(Fixture::fish+0x17D,3);if(fault==4)x.by(Fixture::fish+0x17D,0);const auto before=x.memory;need(!q::overlaps_bird(x.r,Fixture::pulse,Fixture::fish).has_value()&&x.memory==before,"Foreign/damaged/inactive/unregistered/retired bird is rejected without RAM mutation");}
 std::cout<<"BossFishExplosion0260Checks: "<<checks<<" PASS; "<<native_calls<<" literal original native box comparisons, no GPU/game.\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
