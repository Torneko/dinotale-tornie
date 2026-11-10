#include "../boss_floor_persistence0260.hpp"
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
namespace fp=workshop::boss_floor_persistence0260;namespace n=workshop::boss_native;namespace rooms=workshop::boss_rooms_data0250;namespace battle=workshop::boss_battle0250;
static unsigned checks=0,retirements=0,calls=0;static uint32_t allocator_result=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
extern "C" void do_break(uint32_t){std::abort();}
static int32_t sections[300]{};int32_t* section_addresses=sections;
// The fixture supplies already projected screen coordinates to native591EC.
static int32_t dinotale_actor_screen_x(int32_t x){return x;}
#undef LOOKUP_FUNC
#define LOOKUP_FUNC(v) lookup(int32_t(v))
namespace native_floor {
#include "native-floor.inc"
void func_80045A34(uint8_t*,recomp_context* c){need(c->r5==12,"Real floor initializer allocates12 private bytes");c->r2=n::base(allocator_result);}
void func_80045BD8(uint8_t*,recomp_context*){++retirements;}
void func_8004700C(uint8_t*,recomp_context* c){c->r2=0;} // No carried player in bounds test.
void func_8006F324(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected out-of-scope special terrain property");}
void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected native floor/map assertion");}
recomp_func_t* lookup(int32_t a){if(uint32_t(a)==uint32_t(sections[189])+fp::dtor_offset)return func_8069D12C;throw std::runtime_error("Unexpected floor dynamic callback");}
}
struct Fixture {
 static constexpr uint32_t table=0x80710000,actors=0x80700000,draws=0x80704000,privates=0x80706000,stack=0x807B0000;
 std::vector<uint8_t> bytes;uint8_t* rdram;const rooms::Arena& arena;unsigned room,index;uint32_t overlay=0,prototype=0;fp::Guard guard;recomp_context c{};
 Fixture(const std::vector<uint8_t>& input,unsigned encounter=0):bytes(input),rdram(bytes.data()),arena(rooms::configured_arenas[encounter]),room(rooms::source_for_encounter(encounter)),index(encounter){
  for(uint32_t p=0x80000400;p<0x80700000;p+=4)if(n::actor(rdram,p,0x4592)&&word(p+0x1F4)&&word(p+0x1DC)==0x45920000){prototype=p;break;}
  need(prototype!=0,"Actual RAM supplies native floor resource/drawing graph");overlay=word(word(prototype+0x168)+0x40);sections[189]=int32_t(overlay);guard.reset(room,index);c.r29=n::base(stack);c.r31=n::base(0x80000300);c.r16=0x12345678;c.r17=n::base(0x87654321);
  for(size_t at=0;at<arena.objects.size();at+=12){const auto r=table+uint32_t(at/12)*16;for(unsigned k=0;k<3;++k)w(r+k*4,workshop::u32(arena.objects,at+k*4));w(r+12,0);}
  for(unsigned i=0;i<12;++i){const auto a=actor(i),d=draws+i*0x100;std::memcpy(rdram+(a&0x7FFFFF),rdram+(prototype&0x7FFFFF),0x1FC);std::memcpy(rdram+(d&0x7FFFFF),rdram+(word(prototype+0x16C)&0x7FFFFF),0x80);w(a+0x16C,d);w(a+0x1F4,0);w(a+0x1D8,0);w(a+0x1DC,0);w(a+0x20,overlay+fp::init_offset);fl(a+0x3C,40.f+64.f*i);fl(a+0x84,40.f+64.f*i);fl(a+0x40,472);fl(a+0x88,472);sh(a+0xC0,96);sh(a+0xC2,96);b(a+0x1E8,1);}
 }
 uint32_t actor(unsigned i)const{return actors+i*0x300;}uint32_t word(uint32_t p)const{return n::word(rdram,p);}unsigned half(uint32_t p)const{return MEM_HU(0,n::base(p));}
 void w(uint32_t p,uint32_t v){MEM_W(0,n::base(p))=int32_t(v);}void sh(uint32_t p,unsigned v){MEM_H(0,n::base(p))=uint16_t(v);}void b(uint32_t p,unsigned v){MEM_B(0,n::base(p))=uint8_t(v);}void fl(uint32_t p,float v){w(p,std::bit_cast<uint32_t>(v));}
 uint32_t record(unsigned i)const{for(size_t p=0;p<arena.objects.size();p+=12)if(workshop::u32(arena.objects,p)==0x45920000u+i)return table+uint32_t(p/12)*16;throw std::runtime_error("Missing exact floor source");}
 void call(recomp_func_t* fn,uint32_t a,uint32_t a1=0,uint32_t a2=0,uint32_t a3=0){c.r4=n::base(a);c.r5=n::base(a1);c.r6=n::base(a2);c.r7=n::base(a3);const auto sp=c.r29,s0=c.r16,s1=c.r17;fn(rdram,&c);++calls;need(c.r29==sp&&c.r16==s0&&c.r17==s1,"Native body preserves SP/S0/S1");}
 void construct(unsigned i,bool pin=true){const auto a=actor(i),r=record(i);c.r4=n::base(a);c.r5=n::base(r);c.r31=n::base(0x8005A050);const auto before=bytes;need(guard.capture(rdram,&c,arena,room,index,overlay)&&bytes==before,"Owned source capture is read-only");call(native_floor::func_80048E9C,a,r);allocator_result=privates+i*0x20;call(native_floor::func_8069D294,a);need(half(a+0xC0)==96&&half(a+0xC2)==96,"Actual native initializer leaves ordinary culling margins");const auto entry=c;call(native_floor::func_8069D05C,a);
  if(pin){auto expected=bytes;MEM_H(0xC0,n::base(a))=32767;MEM_H(0xC2,n::base(a))=32767;expected=bytes;sh(a+0xC0,96);sh(a+0xC2,96);const auto regs=c;need(guard.pin(rdram,&entry,arena,room,index,overlay)&&bytes==expected&&!std::memcmp(&regs,&c,sizeof(c)),"Pin after genuine first stamp writes only two actor margins");}
 }
 unsigned tile(int x,int y){call(native_floor::func_8006ABF0,unsigned(x),unsigned(y));return unsigned(c.r2)&65535;}
 void bounds(unsigned i,int camera_x){const auto a=actor(i);w(a+0x34,uint32_t(40+64*int(i)-camera_x));w(a+0x38,216);call(native_floor::func_80048D74,a,overlay+fp::dtor_offset);}
 unsigned probe_pot(float y){constexpr uint32_t pot=0x800CE250;need(n::actor(rdram,pot,0x4273),"Actual RAM supplies real native pot collider");fl(pot+0x84,160);fl(pot+0x88,y);call(native_floor::func_80049C70,pot,word(pot+0x174),0,pot+0x84);const auto def=word(word(pot+0x174));return half(word(def+0x40));}
};
int main(int argc,char**argv){try{
 need(argc==2,"Immutable actual height-bug RAM required");std::ifstream in(argv[1],std::ios::binary);std::vector<uint8_t> input((std::istreambuf_iterator<char>(in)),{});need(input.size()==0x800000,"Actual snapshot is8MiB");const auto original=input;rooms::configure(battle::make_plan(1));
 for(unsigned encounter:{0u,9u}){Fixture f(input,encounter);for(unsigned i=0;i<12;++i)f.construct(i);need(f.guard.snapshot()[11].pinned,"All twelve proven Candy/Red floor sections pinned");const auto before_retire=retirements;const auto t0=f.tile(10,29),t1=f.tile(10,30),bottom=f.tile(10,31);std::cout<<"encounter="<<encounter<<" stamped="<<std::hex<<t0<<'/'<<t1<<" bottom="<<bottom<<std::dec<<'\n';need((t0&0x7FF)==0x601&&(t1&0x7FF)==0x401,"RealCD84/B1C8 stamp both native floor rows");
  for(int camera:{0,700,0})for(unsigned i=0;i<12;++i)f.bounds(i,camera);need(retirements==before_retire&&f.tile(10,29)==t0&&f.tile(10,30)==t1,"Real off-screen bounds tests cannot erase pinned floor on far-and-back traversal");
  for(unsigned i=0;i<12;++i)f.call(native_floor::func_8069D12C,f.actor(i));need(f.tile(10,29)==0&&f.tile(10,30)==0&&f.tile(10,31)==bottom,"Real destructor removes transient rows while preserving original bottom row");
 }
 {Fixture f(input);for(unsigned i=0;i<12;++i)f.construct(i,false);const auto old=retirements;f.bounds(0,700);need(retirements==old+1,"Unpinned real off-screen path reaches actual floor destructor");need(f.tile(1,29)==0,"Native off-screen retirement demonstrably erases its stamped map cells");}
 {Fixture f(input);for(unsigned i=0;i<12;++i)f.construct(i);unsigned stamped=0,cleared=0;for(float y:{448.f,464.f,480.f,496.f})stamped|=f.probe_pot(y);for(unsigned i=0;i<12;++i)f.call(native_floor::func_8069D12C,f.actor(i));for(float y:{448.f,464.f,480.f,496.f})cleared|=f.probe_pot(y);std::cout<<"nativePotProbe stamped="<<std::hex<<stamped<<" erased="<<cleared<<std::dec<<'\n';need(stamped!=cleared,"Real49C70 pot query observes the missing transient floor");}
 for(unsigned fault=0;fault<8;++fault){Fixture f(input);const auto a=f.actor(0),r=f.record(0);f.c.r4=n::base(a);f.c.r5=n::base(r);f.c.r31=n::base(0x8005A050);auto arena=f.arena;unsigned room=f.room,index=f.index;uint32_t overlay=f.overlay;if(fault==0)room=83;if(fault==1)index=1;if(fault==2)arena.logical_source=165;if(fault==3)f.c.r31+=4;if(fault==4)f.w(r,0x45910000);if(fault==5)f.fl(r+4,41);if(fault==6)overlay+=4;if(fault==7)f.w(f.actor(0)+0x168,0);const auto before=f.bytes;need(!f.guard.capture(f.rdram,&f.c,arena,room,index,overlay)&&f.bytes==before,"Foreign source, room, encounter, overlay or initializer cannot pin floor");}
 need(input==original,"Original captured RAM remains immutable");std::cout<<"BossFloorPersistence0260Checks: "<<checks<<" PASS; nativeCalls="<<calls<<" nativeRetirements="<<retirements<<"; fixture CPU only, no GPU or save claim.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
