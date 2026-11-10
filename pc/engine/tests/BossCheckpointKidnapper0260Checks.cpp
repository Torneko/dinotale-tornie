#include "../boss_checkpoint_death0260.hpp"
#include <array>
#include <bit>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#ifndef BOSS_KIDNAPPER_NATIVE_SLICE
#error Supply unchanged native retirement/unlink/queue bodies.
#endif
namespace h=workshop::boss_checkpoint_death0260;namespace n=workshop::boss_native;
static unsigned checks=0,retire_calls=0,lookups=0,destructor_calls=0,sound_stops=0;
static int32_t sections[300]{};int32_t* section_addresses=sections;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
namespace original_native {
void func_800447C8(uint8_t*,recomp_context*);void func_80043E88(uint8_t*,recomp_context*);void func_80044658(uint8_t*,recomp_context*);void func_80048060(uint8_t*,recomp_context*);
void func_80057F74(uint8_t*,recomp_context*);void func_80065490(uint8_t*,recomp_context*);void func_80043D34(uint8_t*,recomp_context*);void func_80057C08(uint8_t*,recomp_context*);void func_8006515C(uint8_t*,recomp_context*);void func_8005FE48(uint8_t*,recomp_context*);
void func_8004457C(uint8_t*,recomp_context*);void func_8005FD1C(uint8_t*,recomp_context*);void func_8005F9F4(uint8_t*,recomp_context*);void Fault_HungUp(uint8_t*,recomp_context*);
void func_8057C5D0(uint8_t*,recomp_context*);void func_8057BC48(uint8_t*,recomp_context*);void func_80045BD8(uint8_t*,recomp_context*);void func_800468E8(uint8_t*,recomp_context*);void func_80045B64(uint8_t*,recomp_context*);void func_80045AE8(uint8_t*,recomp_context*);void func_80067878(uint8_t*,recomp_context*);
recomp_func_t* lookup(int32_t);
#undef LOOKUP_FUNC
#define LOOKUP_FUNC(v) lookup(int32_t(v))
#include BOSS_KIDNAPPER_NATIVE_SLICE
recomp_func_t* lookup(int32_t a){++lookups;if(uint32_t(a)==0x80043E88)return func_80043E88;if(uint32_t(a)==0x80044658)return func_80044658;throw std::runtime_error("Unexpected native retirement indirect call");}
void func_8004457C(uint8_t*,recomp_context*){throw std::runtime_error("Attached kidnapper must use48060, not ordinary queue removal");}
void func_8005FD1C(uint8_t*,recomp_context*){throw std::runtime_error("Death kidnapper fixture has no active contact collider");}
void func_8005F9F4(uint8_t*,recomp_context*){throw std::runtime_error("Death kidnapper fixture has no active attack collider");}
void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Native retirement invariant rejected fixture");}
void func_80045B64(uint8_t*,recomp_context*){throw std::runtime_error("Death kidnapper is pooled, not a generic heap actor");}
void func_80045AE8(uint8_t*,recomp_context*){throw std::runtime_error("Pooled4015 must use native468E8 pool release");}
void func_80067878(uint8_t*,recomp_context* c){need(uint32_t(c->r4)==0x57,"Real4015 destructor stops only native sound57");++sound_stops;}
}
extern "C" void func_800447C8(uint8_t* r,recomp_context* c){++retire_calls;original_native::func_800447C8(r,c);}
struct Fixture {
 static constexpr uint32_t player=0x80100000,priv=0x80101000,body=0x80110000,shadow=0x80111000,fruit=0x80112000,egg=0x80113000;
 static constexpr uint32_t first=0x80120000,resource=0x80130000,meta=0x80131000,queue=0x800F8854,queue_slots=0x80140000,overlay=0x80200000,stack=0x807B0000;
 static constexpr uint32_t pool=0x80160000,pool_slots=0x80170000;
 std::vector<uint8_t> mem=std::vector<uint8_t>(0x800000);uint8_t* rdram=mem.data();recomp_context c{};unsigned count=0;
 Fixture(unsigned kids=4,unsigned fatal_sub=0xB):count(kids){
  actor(player,0x4001);w(0x800F8828,player);w(player+0x1F4,priv);b(player+0x18,0xA);b(player+0x30,fatal_sub);b(player+0x17D,2);
  actor(body,0x400B);attach(body,0);actor(shadow,0x4007);attach(shadow,1);actor(fruit,0x4F03);attach(fruit,19);actor(egg,0x4004);attach(egg,17);
  for(unsigned i=0;i<kids;++i){const auto a=kid(i);actor(a,0x4015);attach(a,i+2);b(a+5,i+1);b(a+6,2);b(a+0x17D,0);b(a+0x18,0);w(a+8,pool);w(a+0x168,resource);w(a+0x174,meta+i*0x100);w(meta+i*0x100,meta+i*0x100+0x30);w(a+0x20,overlay+0xA80);w(a+0x2C,overlay+0xAC0);w(a+0x1E0,i);sh(a+0x1F0,0xA);fl(a+0x84,float(i+20));fl(a+0x88,float(i+40));fl(a+0x8C,500.34f);b(pool_slots+i*0x280,1);}
  w(pool+8,pool_slots);sections[128]=int32_t(overlay);
  sh(resource+0x3C,0x4015);w(resource+0x40,overlay);w(resource+0x44,overlay+0xB48);
  sh(queue+4,0);sh(queue+6,0);sh(queue+8,20);w(queue+0x10,queue_slots);w(0x800F8810,0x80150000);
  // Values outside the child graph must survive, including native lives/color,
  // health, the player's mouth/FIFO, scores and PC extension availability.
  for(unsigned i=0;i<0x1400;i+=4)w(priv+i,0x12340000+i);for(unsigned i=0;i<8;++i)w(0x800F8DB4+i*4,i);
  b(priv+0x12C8,1);sh(priv+0x12BE,3);w(priv+0x11FC+3*24,egg);w(egg+0x1EC,13);
  w(0x807F1000,0x3F);w(0x807F1004,7);w(0x807F1008,13);w(0x800F8DF4,0xA5);w(0x807F0008,13);
  c.r29=n::base(stack);c.r4=n::base(player);c.r31=n::base(0x800301D4);c.r16=0x12345678;c.r17=0x87654321;c.f12.u32l=0x3F800000;
 }
 static uint32_t kid(unsigned i){return first+i*0x400;}
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=int32_t(v);}void sh(uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}void b(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}void fl(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}uint32_t word(uint32_t a){return n::word(rdram,a);}unsigned half(uint32_t a){return MEM_HU(0,n::base(a));}
 void actor(uint32_t a,uint16_t kind){w(a,0x19971110);sh(a+12,kind);b(a+0x17D,0);}
 void attach(uint32_t a,unsigned slot){w(a+0x180,player);sh(a+0x17E,uint16_t(slot));w(player+0x188+slot*4,a);sh(player+0x184,uint16_t(half(player+0x184)+1));}
 bool clean(){return h::cleanup(rdram,&c,player,overlay);}
};
static bool in(uint32_t p,uint32_t a,uint32_t size){return p>=a&&p-a<size;}
static bool changed_allowed(uint32_t p,const Fixture& x){
 if(in(p,Fixture::stack-0x400,0x480)||in(p,Fixture::queue,0x20)||in(p,Fixture::queue_slots,20*4)||in(p,Fixture::pool,0x10)||in(p,Fixture::pool_slots,5*0x280)||p==0x800D8150)return true;
 if(in(p,Fixture::player+0x184,2))return true;
 for(unsigned i=0;i<x.count;++i)if(in(p,Fixture::kid(i),0x280)||in(p,Fixture::player+0x188+(i+2)*4,4))return true;
 return false;
}
static void verify_clean(Fixture& x,const std::vector<uint8_t>& before,const recomp_context& regs,unsigned calls){
 need(!std::memcmp(&regs,&x.c,sizeof(regs)),"Cleanup preserves all caller registers and stack pointer");need(retire_calls==calls+x.count,"Each and only each admitted4015 uses true native447C8 once");need(x.half(Fixture::player+0x184)==4,"Native48060 leaves all four non-kidnapper children");
 for(unsigned i=0;i<x.count;++i){const auto a=Fixture::kid(i);need(!x.word(a+0x180)&&x.half(a+0x17E)==i&&n::byte(x.rdram,a+0x18)==0xFF&&n::byte(x.rdram,a+0x17D)==3,"Actual unlink clears parent, queues retirement, marks native retirement state");need(!x.word(Fixture::player+0x188+(i+2)*4),"Actual48060 clears exact original child slot");need(x.word(Fixture::queue_slots+i*4)==a,"Actual43D34/57C08 enqueues exact actor for native disposal");}
 for(auto a:{Fixture::body,Fixture::shadow,Fixture::fruit,Fixture::egg})need(!std::memcmp(x.mem.data()+(a&0x7FFFFF),before.data()+(a&0x7FFFFF),0x280),"Body/shadow/carried fruit/egg bytes remain exact");
 // N64 RAM is word swapped: compare addresses after undoing that byte lane.
 for(size_t i=0;i<x.mem.size();++i)if(x.mem[i]!=before[i])need(changed_allowed(0x80000000u|uint32_t(i^3),x),"No HP/life/FIFO/mouth/stats/global/other-actor writes outside native retirement");
}
int main(int argc,char** argv){try{
 for(unsigned sub:{0xAu,0xBu,0xCu,0xDu,0x10u,0x11u})for(unsigned kids=0;kids<=4;++kids){Fixture x(kids,sub);const auto before=x.mem;const auto regs=x.c;const auto calls=retire_calls;need(x.clean(),"Fatal same-player graph admits zero through four real death kidnappers");verify_clean(x,before,regs,calls);const auto after=x.mem;need(x.clean()&&x.mem==after,"Repeated cleanup with no live4015 is an exact native no-op");
  for(unsigned i=0;i<kids;++i){x.c.r4=n::base(Fixture::kid(i));original_native::func_8057C5D0(x.rdram,&x.c);++destructor_calls;need(n::byte(x.rdram,Fixture::kid(i)+6)==1&&!n::byte(x.rdram,Fixture::pool_slots+i*0x280),"True4015 destructor releases only its native pool slot");}
  for(auto a:{Fixture::body,Fixture::shadow,Fixture::fruit,Fixture::egg})need(!std::memcmp(x.mem.data()+(a&0x7FFFFF),before.data()+(a&0x7FFFFF),0x280),"Actual C5D0 destructor leaves all non4015 actor bytes exact");
  for(size_t i=0;i<x.mem.size();++i)if(x.mem[i]!=before[i])need(changed_allowed(0x80000000u|uint32_t(i^3),x),"True C5D0/BC48/45BD8/468E8 has no HP/mouth/life/globaldeath side effects");
 }
 for(unsigned fault=0;fault<33;++fault){Fixture x;const auto a=Fixture::kid(3);switch(fault){
  case 0:x.b(Fixture::player+0x18,1);break;case 1:x.b(Fixture::player+0x30,2);break;case 2:x.w(0x800F8828,Fixture::body);break;case 3:x.sh(Fixture::player+0x184,3);break;case 4:x.w(Fixture::player+0x188+10*4,Fixture::kid(0));x.sh(Fixture::player+0x184,9);break;
  case 5:x.w(a+0x180,Fixture::body);break;case 6:x.sh(a+0x17E,2);break;case 7:x.w(a+0x20,Fixture::overlay+0xA84);break;case 8:x.w(a+0x2C,Fixture::overlay+0xAC4);break;case 9:x.w(a+0x1F4,Fixture::priv);break;case 10:x.w(a+0x1D8,Fixture::priv);break;case 11:x.w(a+0x1DC,1);break;case 12:x.w(a+0x1E0,2);break;case 13:x.w(a+0x1E0,4);break;
  case 14:x.b(a+6,0);break;case 15:x.b(a+0x17D,2);break;case 16:x.b(a+0x18,1);break;case 17:x.w(Fixture::resource+0x40,Fixture::overlay+4);break;case 18:x.w(Fixture::resource+0x44,Fixture::overlay+0xB4C);break;case 19:x.sh(Fixture::resource+0x3C,0x4014);break;case 20:x.w(Fixture::resource+0x30,1);break;
  case 21:x.sh(a+0x184,1);x.w(a+0x188,Fixture::fruit);break;case 22:x.sh(Fixture::player+0x184,21);break;case 23:x.c.r29=n::base(0x80000400);break;case 24:x.w(a+0x168,0x807FFFFC);break;case 25:x.w(a,0);break;case 26:x.w(Fixture::player+0x188+3*4,0x807FFFFC);break;
  case 27:x.b(a+4,1);break;case 28:x.w(a+0x174,0x807FFFFC);break;case 29:x.w(Fixture::meta+3*0x100,0x807FFFFC);break;case 30:x.b(Fixture::meta+3*0x100+0x30+0x74,1);break;case 31:x.b(Fixture::meta+3*0x100+0x30+0x78,1);break;case 32:x.w(a+0x174,Fixture::meta+1);break;}
  const auto before=x.mem;const auto regs=x.c;const auto calls=retire_calls;need(!x.clean(),"Whole-group malformed/foreign/nonfatal preflight rejects before retiring any sibling");need(x.mem==before&&retire_calls==calls&&!std::memcmp(&regs,&x.c,sizeof(regs)),"Failed whole-group preflight changes no native byte/register and calls no destructor");
 }
 {Fixture x(5);const auto before=x.mem;const auto calls=retire_calls;need(!x.clean()&&x.mem==before&&retire_calls==calls,"Five4015 exceeds native death group bound and is rejected atomically");}
 {Fixture x;need(!h::cleanup(nullptr,&x.c,Fixture::player,Fixture::overlay),"Null RAM rejected");need(!h::cleanup(x.rdram,nullptr,Fixture::player,Fixture::overlay),"Null CPU context rejected");need(!h::cleanup(x.rdram,&x.c,Fixture::player,0x807FFFFC),"Unbounded death overlay rejected");}
 {Fixture x(1);need(x.clean(),"Active-sound destructor fixture retires exact kidnapper");x.sh(Fixture::kid(0)+0x1F0,2);x.c.r4=n::base(Fixture::kid(0));const auto stops=sound_stops;original_native::func_8057C5D0(x.rdram,&x.c);++destructor_calls;need(sound_stops==stops+1&&x.half(Fixture::kid(0)+0x1F0)==0xA,"True C5D0/BC48 stops active sound57 and releases pool");}
 {Fixture x(1);x.w(Fixture::kid(0)+0x174,0);x.w(Fixture::kid(0)+0x178,0x12345678);need(x.clean()&&x.word(Fixture::kid(0)+0x178)==0x12345678,"Native5FE48 accepts missing metadata and never reads or patches actor178");}
 // Optional real c99 snapshot. Only its already-proved post-retry graph is
 // reused; the fatal request is explicit synthetic input, never a game claim.
 if(argc==2){std::ifstream in(argv[1],std::ios::binary);std::vector<uint8_t> captured((std::istreambuf_iterator<char>(in)),{});need(captured.size()==0x800000,"Real c99 snapshot size");auto* rdram=captured.data();const uint32_t player=n::word(rdram,0x800F8828);const auto before=captured;const uint32_t native_color=n::word(rdram,0x800F8DB0);recomp_context c{};c.r29=n::base(0x807B0000);c.r4=n::base(player);MEM_B(0x18,n::base(player))=0xA;MEM_B(0x30,n::base(player))=0xB;
  uint32_t overlay=0;unsigned real_count=0;std::array<uint32_t,20> keep{};for(unsigned i=0;i<20;++i){const auto a=n::word(rdram,player+0x188+i*4);if(!a)continue;if(MEM_HU(12,n::base(a))==0x4015){++real_count;overlay=n::word(rdram,n::word(rdram,a+0x168)+0x40);}else keep[i]=a;}
  need(real_count==4&&overlay,"Actual c99 graph has four bound native kidnappers");sections[128]=int32_t(overlay);const auto calls=retire_calls;std::array<uint32_t,4> originals{};unsigned at=0;for(unsigned i=0;i<20;++i){const auto a=n::word(rdram,player+0x188+i*4);if(a&&MEM_HU(12,n::base(a))==0x4015)originals[at++]=a;}need(h::cleanup(rdram,&c,player,overlay)&&retire_calls==calls+4,"Actual c99 resources/metadata/ordinal graph passes full preflight and real native retirement");
  for(auto a:originals){c.r4=n::base(a);original_native::func_8057C5D0(rdram,&c);++destructor_calls;need(n::byte(rdram,a+6)==1,"Actual c99 pooled4015 destructor completes native release");}
  for(unsigned i=0;i<20;++i)if(keep[i])need(n::word(rdram,player+0x188+i*4)==keep[i]&&!std::memcmp(captured.data()+(keep[i]&0x7FFFFF),before.data()+(keep[i]&0x7FFFFF),0x280),"Actual c99 body/shadow actor bytes and parent slots remain exact");
  need(n::word(rdram,0x800F8DB4+4*native_color)==n::word(const_cast<uint8_t*>(before.data()),0x800F8DB4+4*native_color),"Actual c99 native HP unchanged");
 }
 std::cout<<"BossCheckpointKidnapper0260Checks: "<<checks<<" PASS; nativeRetirements="<<retire_calls<<" realIndirectCalls="<<lookups<<" native4015Destructors="<<destructor_calls<<" soundStops="<<sound_stops<<"; real unlink/retirement/queue/pool release, synthetic death request, no GPU or save claim.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
