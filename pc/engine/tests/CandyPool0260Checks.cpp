#include "../boss_candy_pool0260.hpp"
#include <vector>
#include <bit>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <iostream>
#include <stdexcept>
#include <cstring>
namespace q=workshop::boss_candy_pool0260;namespace n=workshop::boss_native;
static bool outside_stack_same(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b){for(size_t i=0;i<a.size();++i)if((i<0xFF700||i>=0xFF900)&&a[i]!=b[i])return false;return true;}
static unsigned checks=0;static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
static std::vector<uint32_t> live;static q::Guard* current_guard=nullptr;static unsigned cleaned=0;
int32_t sections[512]{};int32_t* section_addresses=sections;
extern "C" int workshop_resource_slot(uint8_t*,recomp_context*,int){return 0;} // Keep prepared cache hook inert to exercise native150-slot registry.
extern "C" void func_80065728(uint8_t* rdram,recomp_context* c){const auto p=uint32_t(c->r4),size=uint32_t(c->r5);need(n::range(p,size),"Bounded memset native endpoint");std::memset(rdram+(p&0x7FFFFF),int(c->r6)&255,size);}
extern "C" void Fault_HungUp(uint8_t*,recomp_context*){std::cerr<<"Unexpected native Fault_HungUp\n";std::abort();}
extern "C" void func_800439F4(uint8_t*,recomp_context*){}
extern "C" void func_80045414(uint8_t*,recomp_context*){}
extern "C" void func_8004597C(uint8_t*,recomp_context*){}
extern "C" void func_800528C8(uint8_t*,recomp_context*){}
extern "C" void func_80052BA4(uint8_t*,recomp_context*){}
extern "C" void func_8005FDC4(uint8_t*,recomp_context*){}
extern "C" void func_80667D6C(uint8_t*,recomp_context*){}
// Explicit resource/render endpoint. The actual native allocator selects and
// marks the slot, preserves its embedded index, and calls this initializer.
extern "C" void func_80046014(uint8_t* rdram,recomp_context* c){const auto a=uint32_t(c->r4);MEM_W(0,n::base(a))=0x19971110;MEM_W(12,n::base(a))=uint32_t(c->r5);MEM_B(0x18,n::base(a))=1;MEM_B(0x17D,n::base(a))=2;MEM_W(0x174,n::base(a))=0;MEM_W(0x1F4,n::base(a))=0;live.push_back(a);c->r2=1;}
extern "C" void func_80045CAC(uint8_t* r,recomp_context* c){func_80046014(r,c);}
extern "C" void func_80046D2C(uint8_t* rdram,recomp_context* c){const auto key=uint32_t(c->r5);const auto kind=q::half(rdram,key),instance=q::half(rdram,key+2)&0x3FF;c->r2=0;for(auto a:live)if(q::half(rdram,a+12)==kind&&(q::half(rdram,a+14)&0x3FF)==instance){c->r2=n::base(a);break;}}
extern "C" void original_func_80046324(uint8_t*,recomp_context*);
extern "C" void func_80046324(uint8_t* r,recomp_context* c){const auto entry=*c;original_func_80046324(r,c);if(current_guard&&current_guard->cleanup_return(r,&entry,c))++cleaned;}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-label"
#pragma clang diagnostic ignored "-Wunused-parameter"
extern "C" {
#define func_80046324 original_func_80046324
#include "native-candy-pool.inc"
#undef func_80046324
#include "native-candy-pool-provider.inc"
#include "native-pool-scene-cleanup.inc"
}
#pragma clang diagnostic pop
struct Fixture {
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* rdram=memory.data();recomp_context ctx{};q::Guard guard;q::Frame f;
 static constexpr uint32_t overlay=0x80300000,table=0x80020000,vector=0x80021000,scale=0x80021010,other=0x80021100;
 Fixture(unsigned count):f{true,162,174,7,count,overlay}{live.clear();cleaned=0;sections[168]=int32_t(overlay);w(0x800F887C,q::registry_address);h(q::registry_address+8,150);w(q::registry_address+0x10,table);h(q::registry_address+6,0);ctx.r29=n::base(0x800FF800);for(unsigned i=0;i<3;++i){fl(vector+4*i,0);fl(scale+4*i,1);}std::fill(memory.begin()+0x7CFFF0,memory.begin()+0x7D0000,0xA5);std::fill(memory.begin()+0x7D5A00,memory.begin()+0x7D5A10,0x5A);std::fill(memory.begin()+0x300290,memory.begin()+0x302090,0xCC);current_guard=&guard;}
 ~Fixture(){current_guard=nullptr;}
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=int32_t(v);}void h(uint32_t a,unsigned v){MEM_H(0,n::base(a))=uint16_t(v);}void by(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}void fl(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}
 void args(){ctx.r4=n::base(overlay+0x280);ctx.r5=n::base(overlay+0x290);ctx.r6=12;ctx.r31=n::base(overlay+0x24);}
 q::Choice enter(){args();return guard.enter(rdram,&ctx,f);}
 void initialize(){need(enter()==q::Choice::Expanded,"Only owned fresh native4550 pool expands");need(uint32_t(ctx.r5)==q::storage&&uint32_t(ctx.r6)==12*f.candies,"A1 storage AND A2 count are replaced together");func_8004667C(rdram,&ctx);need(guard.leave(rdram,&ctx),"Native allocator registered expected expanded pool");need(n::word(rdram,overlay+0x284)==12*f.candies&&n::word(rdram,overlay+0x288)==q::storage,"Original native header owns the new capacity/storage");}
 uint32_t provide(){ctx.r4=n::base(vector);ctx.r5=n::base(scale);func_80667E08(rdram,&ctx);return uint32_t(ctx.r2);}
 void canaries(){for(unsigned i=0;i<16;++i){need(memory[0x7CFFF0+i]==0xA5&&memory[0x7D5A00+i]==0x5A,"Both scratch boundary canaries remain exact");}for(unsigned i=0x300290;i<0x302090;++i)need(memory[i]==0xCC,"Native12-slot BSS is untouched; no count-only overflow");}
 void cleanup(){current_guard=&guard;func_80046560(rdram,&ctx);}
};
int main(int argc,char** argv){try{
 for(unsigned count:{2u,3u}){Fixture t(count);t.initialize();std::vector<uint32_t> actors;
  for(unsigned root=0;root<count;++root)for(unsigned segment=0;segment<12;++segment){const auto actor=t.provide();need(actor==q::storage+q::stride*(root*12+segment)+4,"Actual67E08/46804 provides distinct native slots, exactly12 per head group");need(n::actor(t.rdram,actor,0x4550),"Actual native provider preserves4550 identity");need(std::find(actors.begin(),actors.end(),actor)==actors.end(),"No segment address aliases another root/head group");actors.push_back(actor);}
  need(t.provide()==0,"Actual native provider refuses slot after24/36 capacity");const auto before=t.memory;need(t.enter()==q::Choice::Original&&t.memory==before,"Registered live pool is never repointed or zeroed");func_8004667C(t.rdram,&t.ctx);need(outside_stack_same(t.memory,before),"Actual native repeated initialization leaves live registered pool untouched outside its real stack frame");
  t.f.overlay+=0x3000;t.args();t.ctx.r4=n::base(t.f.overlay+0x280);t.ctx.r5=n::base(t.f.overlay+0x290);t.ctx.r31=n::base(t.f.overlay+0x24);need(t.guard.enter(t.rdram,&t.ctx,t.f)==q::Choice::Refused,"New header cannot overwrite scratch while old lease lives");t.f.overlay=Fixture::overlay;
  t.cleanup();need(cleaned==1&&t.guard.snapshot().lease.cleaned,"True46324 return1 is observed during native46560 cleanup");need(q::registration(t.rdram,q::registry_address,Fixture::overlay+0x280)==150,"True native cleanup57BAC removed old header before reuse");live.clear();t.initialize();need(t.provide()==q::storage+4,"Actual native allocator/provider reuse only after cleanup and unregister");t.canaries();
 }
 // Native vanilla capacity1 head remains12; helper never expands the solo.
 {Fixture t(1);t.args();const auto args=t.ctx;need(t.guard.enter(t.rdram,&t.ctx,t.f)==q::Choice::Original&&t.ctx.r5==args.r5&&t.ctx.r6==args.r6,"One Candy is completely outside expansion scope");func_8004667C(t.rdram,&t.ctx);for(unsigned i=0;i<12;++i)need(t.provide()!=0,"Native solo still supplies12 segments");need(!t.provide(),"Native solo pool stays12");}
 for(unsigned fault=0;fault<15;++fault){Fixture t(3);t.args();switch(fault){case 0:t.f.enabled=false;break;case 1:t.f.source=165;break;case 2:t.f.room=173;break;case 3:t.f.index=9;break;case 4:t.f.candies=4;break;case 5:t.ctx.r4+=4;break;case 6:t.ctx.r5+=4;break;case 7:t.ctx.r6=36;break;case 8:t.ctx.r31+=4;break;case 9:t.w(0x800F828C,1);break;case 10:t.w(0x800F887C,Fixture::other);break;case 11:t.h(q::registry_address+8,149);break;case 12:t.w(Fixture::table,Fixture::other);t.w(Fixture::other+4,24);t.w(Fixture::other+8,q::storage);break;case 13:t.w(Fixture::table,Fixture::other);t.w(Fixture::table+4,Fixture::other);t.w(Fixture::other+4,1);t.w(Fixture::other+8,0x80022000);break;case 14:t.ctx.r29=0;break;}
  const auto old=t.memory;const auto c=t.ctx;const auto choice=t.guard.enter(t.rdram,&t.ctx,t.f);need(choice!=q::Choice::Expanded&&t.ctx.r5==c.r5&&t.ctx.r6==c.r6&&t.memory==old,"Wrong caller/context/registry/storage never changes arguments or RAM");}
 {Fixture t(3);need(t.enter()==q::Choice::Expanded,"Reentrant fixture acquires pending fresh call");const auto before=t.memory;t.args();need(t.guard.enter(t.rdram,&t.ctx,t.f)==q::Choice::Refused&&t.memory==before,"Second pending allocation never reenters initialization");}
 {Fixture t(3);t.initialize();t.h(q::registry_address+6,0);t.w(Fixture::table,0);need(t.enter()==q::Choice::Refused,"Registry clear without nativecleanup attestation is never lease release");}
 {Fixture t(3);t.initialize();t.by(q::storage+8,1);t.cleanup();need(!t.guard.snapshot().lease.cleaned&&q::registration(t.rdram,q::registry_address,Fixture::overlay+0x280)!=150,"Protected native slot makes46324 return0 and preserves live registration");}

 {need(argc==2,"Actual b73 failed-construction capture required");std::ifstream file(argv[1],std::ios::binary);std::vector<uint8_t> memory((std::istreambuf_iterator<char>(file)),{});need(memory.size()==0x800000,"Actual b73 native snapshot is8MiB");auto* ram=memory.data();const auto before=memory;need(n::word(ram,0x800F887C)==q::registry_address&&q::half(ram,q::registry_address+6)==32&&q::no_storage_reference(ram,q::registry_address),"Actual32-header150-slot native registry validates with all unrelated pools intact");
  const uint32_t header=0x802E3F80,overlay=header-0x280;need(n::word(ram,header+4)==12&&n::word(ram,header+8)==header+0x10,"Actual existing twelve-slot4550 pool retained from failure");q::Guard g;recomp_context c{};c.r4=n::base(header);c.r5=n::base(header+0x10);c.r6=12;c.r31=n::base(overlay+0x24);c.r29=n::base(0x800FF800);need(g.enter(ram,&c,{true,162,174,7,3,overlay})==q::Choice::Original&&memory==before,"Actual already-registered native12 pool is never expanded retroactively");
 }
 std::cout<<"CandyPool0260Checks: "<<checks<<" PASS; actual native allocator/provider/cleanup with explicit resource/render endpoints, HOST head grouping only; complete multiple-boss initialization and victory still need GPU proof.\n";return 0;
 }catch(const std::exception&ex){std::cerr<<ex.what()<<'\n';return 1;}}
