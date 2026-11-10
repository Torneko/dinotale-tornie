#include "../boss_native.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>
#ifndef MONSTER_AIRBURST_NATIVE_SLICE
#define MONSTER_AIRBURST_NATIVE_SLICE "native-airburst-slice.inc"
#endif
namespace n=workshop::boss_native;
static unsigned checks=0,box_calls=0,hit_calls=0,init_calls=0,retire_calls=0,reactions=0;
static void need(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
static int32_t native_sections[512]{};int32_t* section_addresses=native_sections;
void do_break(uint32_t){std::fputs("Unexpected native break\n",stderr);std::abort();}
namespace literal {
void func_80049018(uint8_t*,recomp_context*);void func_8005FEBC(uint8_t*,recomp_context*);void func_8006009C(uint8_t*,recomp_context*);void func_80065954(uint8_t*,recomp_context*);
void func_806F3A1C(uint8_t*,recomp_context*);void func_806F38EC(uint8_t*,recomp_context*);void func_806F38D4(uint8_t*,recomp_context*);void func_806F35BC(uint8_t*,recomp_context*);
void func_80065490(uint8_t*,recomp_context*);void func_800656BC(uint8_t*,recomp_context*);void func_800657CC(uint8_t*,recomp_context*);void func_80554928(uint8_t*,recomp_context*);
void func_8003ACEC(uint8_t*,recomp_context*);void func_8003B540(uint8_t*,recomp_context*);void func_8003B3B0(uint8_t*,recomp_context*);
// Explicitly excluded services: allocation, animation, sound and reaction pose.
// None chooses the collision result, primary/sub, callback or Monster HP.
void func_80045A34(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected allocation: fixture must supply private data");}
void func_800445A4(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected allocation failure");}
void func_80071380(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected native fault");}
void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected native fault");}
void func_80067BBC(uint8_t*,recomp_context* c){c->r2=0;}
void func_80067818(uint8_t*,recomp_context*){}
void func_8055394C(uint8_t*,recomp_context*){++reactions;}
void func_80039EAC(uint8_t*,recomp_context*){}
void func_8003A060(uint8_t*,recomp_context*){}
void func_8005FE48(uint8_t*,recomp_context*){++retire_calls;}
void func_800447C8(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected actor retirement");}
void func_8003B694(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected setter mode69");}
void func_8003B6F8(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected projectile rearm");}
void workshop_find_egg_state(uint8_t*,recomp_context*){} // Historical read-only instrumentation.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#include MONSTER_AIRBURST_NATIVE_SLICE
#pragma clang diagnostic pop
}
recomp_func_t* get_function(int32_t address){
 if(uint32_t(address)==0x8003B3B0)return literal::func_8003B3B0;
 if(uint32_t(address)==0x8003ACEC)return literal::func_8003ACEC;
 std::fprintf(stderr,"Unexpected indirect native callback %08X\n",uint32_t(address));std::abort();
}
static std::vector<uint8_t> load(const char* path){std::ifstream f(path,std::ios::binary);need(bool(f),"Captured RAM opens");std::vector<uint8_t> v((std::istreambuf_iterator<char>(f)),{});need(v.size()==0x800000,"Captured RAM is exactly8MiB");return v;}
struct Fixture {
 static constexpr uint32_t pulse=0x80400000,priv=0x80400300,meta=0x80400400,def=0x80400500,drawing=0x80400600,scratch=0x80400700;
 static constexpr uint32_t eye=0x800CAE24,root=0x800C9F3C,egg=0x800C4294,old_pulse=0x803480C4;
 std::vector<uint8_t> memory;uint8_t* rdram;
 uint32_t eye_def=0,root_private=0;
 explicit Fixture(const std::vector<uint8_t>& battle,const std::vector<uint8_t>& old):memory(battle),rdram(memory.data()){
  auto ow=[&](uint32_t a){return *reinterpret_cast<const uint32_t*>(old.data()+(a&0x7FFFFF));};
  const auto old_meta=ow(old_pulse+0x174),old_def=ow(old_meta),old_priv=ow(old_pulse+0x1F4),old_draw=ow(old_pulse+0x16C);
  auto copy=[&](uint32_t dst,uint32_t src,unsigned bytes){std::memcpy(rdram+(dst&0x7FFFFF),old.data()+(src&0x7FFFFF),bytes);};
  copy(pulse,old_pulse,0x280);copy(priv,old_priv,0x20);copy(meta,old_meta,0x30);copy(def,old_def,0x80);copy(drawing,old_draw,0x40);
  w(pulse+0x1F4,priv);w(pulse+0x174,meta);w(pulse+0x16C,drawing);w(pulse+0x180,0);w(meta,def);by(pulse+0x164,7);by(pulse+0x17D,2);by(def+0x74,1);
  // Section226's native3C10 callback is at offset7C0. Its real b22 tables
  // remain in place; no scale or callback table is synthesized or relocated.
  native_sections[226]=int32_t(n::word(rdram,0x800F8D18)-0x7C0);
  eye_def=n::word(rdram,n::word(rdram,eye+0x174));root_private=n::word(rdram,root+0x1F4);
  need(n::actor(rdram,eye,0x46F9)&&n::word(rdram,eye+0x180)==root,"Real b22 reciprocal46F9 belongs to Monster root");
  need(n::word(rdram,root_private+0xDE8)==4,"Actual b22 Monster starts with4HP");
  need(MEM_HU(0xA,n::base(def))==24&&MEM_HU(0xC,n::base(def))==24&&MEM_HU(0xE,n::base(def))==20,"Real7ee4018 definition carries24/24/20 extents");
  need(MEM_HU(0xA,n::base(eye_def))==8&&MEM_HU(0xC,n::base(eye_def))==8,"Actual b22 eye half8/8");
  w(root_private+0xDE4,0);w(root_private+0xDF8,0);w(meta+0x14,0);
 }
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=int32_t(v);}void by(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}void fl(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}float f(uint32_t a)const{return std::bit_cast<float>(n::word(rdram,a));}
 recomp_context context(){recomp_context c{};c.r29=n::base(0x807EF000);return c;}
 void configure(unsigned param){auto c=context();c.r4=n::base(pulse);c.r5=n::base(egg);c.r6=param;c.r7=0;literal::func_806F3A1C(rdram,&c);++init_calls;need(c.r2==1,"Literal3A1C initialized existing native private storage");}
 void initialize(){auto c=context();c.r4=n::base(pulse);literal::func_806F38EC(rdram,&c);++init_calls;}
 bool overlap(uint32_t a){auto c=context();const auto ad=n::word(rdram,n::word(rdram,a+0x174)),as=n::word(rdram,ad+0x70);for(unsigned axis=0;axis<3;++axis){fl(scratch+0x24+4*axis,f(a+0x84+4*axis)+float(MEM_H(4+2*axis,n::base(ad)))*f(as+4*axis));fl(scratch+0x30+4*axis,float(MEM_H(0xA+2*axis,n::base(ad)))*f(as+4*axis));}w(scratch+0x3C,eye);w(scratch+0x40,n::word(rdram,eye+0x174));c.r4=n::base(scratch);literal::func_8006009C(rdram,&c);++box_calls;return c.r2!=0;}
 void hit(uint32_t a){auto c=context();c.r4=n::base(eye);c.r5=n::base(a);literal::func_80554928(rdram,&c);++hit_calls;}
 void at(float dx,float dy,float dz=0){fl(pulse+0x84,f(eye+0x84)+dx);fl(pulse+0x88,f(eye+0x88)+dy);fl(pulse+0x8C,f(eye+0x8C)+dz);}
 unsigned hp()const{return n::word(rdram,root_private+0xDE8);}
};
int main(int argc,char** argv){try{
 need(argc==3,"Pass actual b22-3000 and7ee-final captures");const auto battle=load(argv[1]),old=load(argv[2]);
 for(unsigned param=0;param<3;++param){Fixture x(battle,old);x.configure(param);const float scale=std::array<float,3>{.75f,1.5f,2.25f}[param];need(x.f(Fixture::priv+0x14)==scale&&n::word(x.rdram,Fixture::def+0x70)==Fixture::priv+0x14,"Literal initializer selects exact source table scale and private collider pointer");x.by(Fixture::pulse+0x18,0);x.hit(Fixture::pulse);need(x.hp()==4,"Queued preinit state0 cannot damage an eye");x.initialize();need(n::byte(x.rdram,Fixture::pulse+0x18)==0x68,"Literal38EC admits68 at first actual initializer invocation");need(x.f(Fixture::priv+0x14)==scale,"Initializer preserves collision scale from3A1C");}
 for(unsigned axis=0;axis<3;++axis){Fixture x(battle,old);x.configure(1);x.initialize();const float boundary=axis==2?50.f:44.f;std::array<float,3> delta{};delta[axis]=boundary;x.at(delta[0],delta[1],delta[2]);need(x.overlap(Fixture::pulse),"Actual native6009C includes exact44/44/50 puff-eye face");delta[axis]=boundary+.01f;x.at(delta[0],delta[1],delta[2]);need(!x.overlap(Fixture::pulse),"Native6009C separates just beyond real summed half-extents");}
 {Fixture x(battle,old);x.configure(1);x.initialize();x.at(-30,0);need(x.overlap(Fixture::pulse),"A detonation30px before the eye is inside native puff volume");x.hit(Fixture::pulse);need(x.hp()==3,"Actual549284018 branch decrements nativeDE8 exactly once");need(n::word(x.rdram,x.root_private+0xDE4)==50&&n::word(x.rdram,x.root_private+0xDF8)==120,"Native50/120 invulnerability timers are retained");x.hit(Fixture::pulse);need(x.hp()==3,"The same pulse cannot bypass the native refractory timers");}
 for(unsigned lock:{0xDE4u,0xDF8u}){Fixture x(battle,old);x.configure(1);x.initialize();x.w(x.root_private+lock,1);x.hit(Fixture::pulse);need(x.hp()==4,"Either existing native timer rejects splash damage");}
 {Fixture x(battle,old);x.configure(1);x.initialize();const auto scale=std::array<uint32_t,3>{n::word(x.rdram,Fixture::priv+0x14),n::word(x.rdram,Fixture::priv+0x18),n::word(x.rdram,Fixture::priv+0x1C)};for(unsigned tick=1;tick<=32;++tick){auto c=x.context();c.r4=n::base(Fixture::pulse);c.r5=n::base(Fixture::priv);literal::func_806F35BC(x.rdram,&c);++init_calls;need(x.f(Fixture::priv+0x14)==std::bit_cast<float>(scale[0])&&n::word(x.rdram,Fixture::def+0x70)==Fixture::priv+0x14,"Literal alpha lifecycle never grows or shrinks the fixed private collision box");need(n::byte(x.rdram,Fixture::def+0x74)==unsigned(tick<32),"Param1 primary remains active through31 alpha updates and disables on32");}}
 {Fixture x(battle,old);x.by(Fixture::egg+0x18,0x68);x.by(Fixture::egg+0x30,0);x.w(Fixture::egg+0x1C,0x8003ACEC);x.hit(Fixture::egg);need(x.hp()==4,"Actual direct4002 eye impact never decrementsDE8");need(n::byte(x.rdram,Fixture::egg+0x18)==1&&n::byte(x.rdram,Fixture::egg+0x30)==1&&n::word(x.rdram,Fixture::egg+0x20)==0x8003A958,"Actual54928→ACEC→B540→B3B0 turns the egg into collectible1/1");need(x.f(Fixture::egg+0xC4)==0&&x.f(Fixture::egg+0xC8)==0,"Eye impact zeros projectile velocity after the native setter");}
 std::cout<<"BossMonsterAirburst0260Checks: "<<checks<<" PASS; native box="<<box_calls<<" hit="<<hit_calls<<" init/alpha="<<init_calls<<"; animation/sound/allocation excluded; no scheduler/GPU/victory claim\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
