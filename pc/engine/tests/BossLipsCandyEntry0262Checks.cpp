#include "../boss_lips_candy_entry0262.hpp"
#include <array>
#include <fstream>
#include <cstring>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
namespace h=workshop::boss_lips_candy_entry0262;namespace n=workshop::boss_native;
static unsigned checks=0;static void need(bool b,const char* why){++checks;if(!b)throw std::runtime_error(why);}
struct Fixture {
 static constexpr uint32_t root=0x800C954C;std::vector<uint8_t> mem;uint8_t* rdram;h::Guard guard;h::Frame f;recomp_context c{};uint32_t p,head,hp;std::array<uint32_t,4> returned{};
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}void b(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}uint32_t word(uint32_t a)const{return n::word(rdram,a);}
 explicit Fixture(const std::vector<uint8_t>& source):mem(source),rdram(mem.data()){
  const auto resource=word(root+0x168),overlay=word(resource+0x40);p=word(root+0x1F4);head=word(p);hp=word(head+0x1F4);returned={word(root+0x20),word(root+0x3C),word(root+0x40),word(root+0x44)};
  f={true,true,165,168,1,0,1,2,1,root,overlay,12,"lips-candy-fresh8457-host"};c.r4=n::base(root);c.r29=n::base(0x800FFB80);c.r31=n::base(overlay+0x40);
  // Explicit HOST pre/post constructor boundary using actual native graph,
  // resource, consumed RAWrecord and twelve segments from fresh8457. This
  // fixture does not execute or pretend to emulate the native constructor.
  w(root+0x3C,std::bit_cast<uint32_t>(248.f));w(root+0x40,std::bit_cast<uint32_t>(152.f));b(root+0x18,0);b(root+0x30,0);w(root+0x20,overlay+0x10);w(root+0x1F4,0);
 }
 void native_return(){w(root+0x1F4,p);b(root+0x18,1);b(root+0x30,3);w(root+0x20,returned[0]);w(root+0x3C,returned[1]);w(root+0x40,returned[2]);w(root+0x44,returned[3]);}
};
int main(int argc,char**argv){try{
 need(argc==2,"Fresh8457 snapshot path required");std::ifstream in(argv[1],std::ios::binary);std::vector<uint8_t> source((std::istreambuf_iterator<char>(in)),{});need(source.size()==0x800000,"Fresh native capture is exactly8MiB");const auto immutable=source;auto* rdram=source.data();constexpr auto root=Fixture::root;
 need(n::actor(rdram,root,0x455A)&&n::word(rdram,root+0x3C)==std::bit_cast<uint32_t>(376.f)&&n::word(rdram,root+0x40)==std::bit_cast<uint32_t>(376.f)&&n::byte(rdram,root+0x18)==1&&n::byte(rdram,root+0x30)==3,"Fresh8457 is the actual synchronous constructor return, before any physics update");
 const auto record=n::word(rdram,root+0x1D8),resource=n::word(rdram,root+0x168);need(n::word(rdram,record)==0xFFFFFFFF&&n::word(rdram,record+4)==std::bit_cast<uint32_t>(248.f)&&n::word(rdram,record+8)==std::bit_cast<uint32_t>(152.f)&&n::word(rdram,resource+0x44)==n::word(rdram,resource+0x40)+0x10,"Fresh baseline independently attests the consumed248/152 record and real public constructor");
 const auto overlay=n::word(rdram,resource+0x40);need(n::word(rdram,overlay+0x2CD8+36+12)==overlay+0x8A8&&n::word(rdram,root+0x20)==overlay+0x8A8,"Unmodified real native state table and actual return independently identify65578, not655E4");
 {Fixture x(source);const auto before=x.mem;const auto regs=x.c;need(x.guard.capture(x.rdram,&x.c,x.f)&&x.mem==before,"Pre-constructor capture only records host identity and writes no RAM");need(!x.guard.capture(x.rdram,&x.c,x.f)&&x.mem==before,"Reentrant or duplicate pre-constructor capture is refused");x.native_return();need(x.mem==source,"Positive finish input is BYTE-IDENTICAL to fresh actual8457 constructor return, no assumed post-callback");const auto post=x.mem;auto expected=post;auto* rdram=expected.data();MEM_W(0x3C,n::base(root))=std::bit_cast<uint32_t>(248.f);MEM_W(0x40,n::base(root))=std::bit_cast<uint32_t>(152.f);
  need(x.guard.finish(x.rdram,&x.c,x.f)&&x.mem==expected,"Completed native-shaped constructor return changes EXACTLY its two RAWXY words");need(x.guard.snapshot().applied&&!x.guard.snapshot().pending&&!x.guard.finish(x.rdram,&x.c,x.f)&&x.mem==expected,"One-time adjustment cannot repeat or become a permanent clamp");need(!std::memcmp(&regs,&x.c,sizeof(regs)),"Host adaptation never alters guest registers");
 }
 for(unsigned fault=0;fault<25;++fault){Fixture x(source);switch(fault){case 0:x.f.enabled=false;break;case 1:x.f.entering=false;break;case 2:x.f.source=162;break;case 3:x.f.room=169;break;case 4:x.f.index=0;break;case 5:x.f.variant=0;break;case 6:x.f.count=1;break;case 7:x.f.member=0;break;case 8:x.f.birth=0;break;case 9:x.f.token={};break;case 10:x.c.r4+=4;break;case 11:x.c.r31+=4;break;case 12:x.c.r29=0;break;case 13:x.w(resource+0x44,x.f.overlay+0x144);break;case 14:x.w(root+0x1DC,0x455A0000);break;case 15:x.w(record+8,std::bit_cast<uint32_t>(376.f));break;case 16:x.b(root+0x17D,3);break;case 17:x.b(root+0x18,1);break;case 18:x.b(root+0x30,3);break;case 19:x.w(root+0x20,0);break;case 20:x.w(root+0x1F4,x.p);break;case 21:x.w(root+0x180,0x800C92D0);break;case 22:x.w(root+0x3C,std::bit_cast<uint32_t>(376.f));break;case 23:x.w(root+0x40,std::bit_cast<uint32_t>(376.f));break;case 24:x.w(root+0x44,0x7FC00000);break;}
  const auto before=x.mem;need(!x.guard.capture(x.rdram,&x.c,x.f)&&x.mem==before&&!x.guard.snapshot().pending,"Other rooms/plans/callers/generations/records/resources or late capture cannot adapt unknown native storage");
 }
 for(unsigned fault=0;fault<21;++fault){Fixture x(source);need(x.guard.capture(x.rdram,&x.c,x.f),"Valid native-shaped capture precedes each rejected return");x.native_return();switch(fault){case 0:++x.f.birth;break;case 1:++x.f.epoch;break;case 2:x.f.token="foreign";break;case 3:x.c.r29+=4;break;case 4:x.c.r31+=4;break;case 5:x.w(root+0x168,resource+4);break;case 6:x.w(resource+0x40,x.f.overlay+4);break;case 7:x.w(root+0x1D8,record+16);break;case 8:x.w(root+0x1F4,0);break;case 9:x.w(x.head+0x180,0);break;case 10:x.w(x.head+0x1F4,0);break;case 11:x.w(x.hp+0x84,11);break;case 12:x.w(x.p+0x20,1);break;case 13:x.b(root+0x18,0xFF);break;case 14:x.b(root+0x30,7);break;case 15:x.w(root+0x20,x.f.overlay+0x914);break;case 16:x.w(root+0x3C,std::bit_cast<uint32_t>(377.f));break;case 17:x.w(root+0x40,std::bit_cast<uint32_t>(375.f));break;case 18:x.w(root+0x44,std::bit_cast<uint32_t>(501.f));break;case 19:x.w(x.hp+0x54,0);break;case 20:x.w(x.hp+0x58,x.word(x.hp+0x54));break;}
  const auto before=x.mem;need(!x.guard.finish(x.rdram,&x.c,x.f)&&x.mem==before&&!x.guard.snapshot().applied,"Incomplete/foreign/dirty native graph or stale return cannot overwrite any RAM");
 }
 need(source==immutable,"Fresh baseline capture remains immutable");std::cout<<"BossLipsCandyEntry0262Checks: "<<checks<<" PASS; actual8457 provenance plus explicit HOST constructor boundaries, no GPU/physics/victory claim.\n";return 0;
}catch(const std::exception&e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
