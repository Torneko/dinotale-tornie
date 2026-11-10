#include "coop_native_dispatch.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace {
unsigned checks=0,originals=0,captures=0,posts=0,skips=0,mode=0;
bool enabled=true,skip_pair=false;
uint32_t owner=7,capture_slot=UINT32_MAX,last_post_slot=0,last_post_actor=0;
uint32_t selected_player=0,last_skip_fn=0,last_skip_work=0;
unsigned init_begins=0,init_finishes=0;uint32_t last_init_actor=0;
uint64_t init_token=0x1122334455667788ULL,last_finished_token=0;int last_finished_completed=-1;
std::vector<uint32_t> owner_stack;
struct Event{uint32_t fn,a0,a1;int complete;};
std::vector<Event> events;
recomp_context native_result{},expected_input{};
void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
recomp_context incoming(){recomp_context c{};c.r2=123;c.r3=456;c.r4=gpr(int32_t(0x800CA000));c.r5=0x800CB000;c.r6=0x43FA0000;c.r7=5;c.r29=0x803FFFE0;c.r31=0x80010000;c.f0.u64=0x0102030405060708ULL;c.f12.u32l=0x41200000;c.f14.u32l=0x41A00000;return c;}
void poison(recomp_context* c){std::memset(c,0xA5,sizeof(*c));}
bool same(const recomp_context& a,const recomp_context& b){return std::memcmp(&a,&b,sizeof(a))==0;}
}
extern "C" uintptr_t workshop_coop_dispatch_enter(uint8_t*,recomp_context* c,uint32_t fn,uint32_t a0,uint32_t a1){
 if(!enabled)return 0;
 owner_stack.push_back(owner);owner=a0;events.push_back({fn,a0,a1,-1});poison(c);return owner_stack.size();
}
extern "C" void workshop_coop_dispatch_leave(uint8_t*,recomp_context* c,uint32_t fn,uint32_t a0,uint32_t a1,uintptr_t ticket,int completed)noexcept{
 if(!ticket){events.push_back({fn,a0,a1,completed});poison(c);return;}
 if(owner_stack.empty() || ticket!=owner_stack.size())std::terminate();
 owner=owner_stack.back();owner_stack.pop_back();events.push_back({fn,a0,a1,completed});poison(c);
}
extern "C" uint32_t workshop_coop_scoped_player(uint8_t*){return selected_player;}
extern "C" int workshop_coop_skip_collision(uint8_t*,recomp_context* c,uint32_t fn,uint32_t work){
 skips++;last_skip_fn=fn;last_skip_work=work;poison(c);
 if(mode==3)throw std::runtime_error("skip-hook-test");return skip_pair?1:0;
}
extern "C" uint32_t workshop_coop_capture_egg_owner(uint8_t*,recomp_context* c)noexcept{captures++;poison(c);return capture_slot;}
extern "C" void workshop_coop_egg_created(uint8_t*,recomp_context* c,uint32_t actor,uint32_t slot)noexcept{
 posts++;last_post_slot=slot;last_post_actor=actor;
 // Post must occur before leave, while the native scope still exists.
 if(enabled && owner_stack.empty())std::terminate();poison(c);
}
extern "C" uint64_t workshop_coop_begin_egg_private_init(uint8_t*,recomp_context* c,uint32_t actor)noexcept{
 init_begins++;last_init_actor=actor;const auto result=init_token;poison(c);return result;
}
extern "C" void workshop_coop_finish_egg_private_init(uint8_t*,recomp_context* c,uint64_t token,int completed)noexcept{
 init_finishes++;last_finished_token=token;last_finished_completed=completed;
 if(enabled && owner_stack.empty())std::terminate();poison(c);
}
extern "C" void func_8005E884(uint8_t*,recomp_context*);
static void original(uint8_t* r,recomp_context* c,uint32_t fn){
 originals++;need(c->r4!=gpr(0xA5A5A5A5A5A5A5A5ULL),"Hook register clobbers leaked into native A0");
 const auto a0=uint32_t(c->r4);
 if(enabled)need(owner==a0,"Native original executed outside captured owner");
 if(mode==1 && fn==0x80043450){c->r4=gpr(int32_t(0x800CE000));c->r5=0x99;func_8005E884(r,c);need(owner==a0,"Nested render failed to restore outer owner");}
 if(fn==0x8003B8F0)need(c->f12.u32l==0x41200000 && c->f14.u32l==0x41A00000 && c->r6==0x43FA0000 && c->r7==5,"Egg factory F12/F14/A2/A3 were changed");
 std::memset(c,0x3C,sizeof(*c));c->r2=fn==0x8003B8F0?(mode==4?0:gpr(int32_t(0x800CD000))):fn;c->r29=0x803FFF00;c->r31=0x80012345;c->f0.u64=0x1122334455667788ULL;native_result=*c;
 if(fn==0x8003AE00)init_token=0;
 if(fn==0x8003B8F0)capture_slot=0; // Ownership must have been captured before native code.
 if(mode==2)throw std::runtime_error("native-test-exception");
}
#define STUB(a) extern "C" void func_##a(uint8_t*,recomp_context*); \
 extern "C" void coop_original_func_##a(uint8_t* r,recomp_context* c){original(r,c,0x##a##u);}
STUB(8001E094) STUB(8001E20C) STUB(80022448) STUB(800233B4) STUB(8002387C)
STUB(80039F48) STUB(8003FFD8) STUB(80043450) STUB(800434EC) STUB(800439F4)
STUB(80043E10) STUB(80043E88) STUB(80045B64) STUB(80048A30) STUB(80048AE0)
STUB(800675F0) STUB(80048060) STUB(8005E884) STUB(8006009C) STUB(80060274)
STUB(800603E8) STUB(80060798) STUB(80060960) STUB(8003B8F0) STUB(8003AE00)
#undef STUB
extern "C" void func_8004700C(uint8_t*,recomp_context*);
extern "C" void coop_original_func_8004700C(uint8_t*,recomp_context* c){originals++;std::memset(c,0x69,sizeof(*c));c->r2=gpr(int32_t(0x800CA000));native_result=*c;selected_player=0;}
struct Entry{recomp_func_t* fn;uint32_t id;};
#define E(a) Entry{func_##a,0x##a##u}
int main(){try{
 uint8_t ram[4]{};recomp_context c{};
 const std::array ordinary={E(8001E094),E(8001E20C),E(80022448),E(800233B4),E(8002387C),E(80039F48),E(8003FFD8),E(80043450),E(800434EC),E(800439F4),E(80043E10),E(80043E88),E(80045B64),E(80048A30),E(80048AE0),E(800675F0),E(80048060),E(8005E884),E(8006009C),E(80060274),E(800603E8),E(80060798),E(80060960)};
 for(auto e:ordinary){
  c=incoming();const auto count=originals;const auto events_before=events.size();e.fn(ram,&c);
  need(originals==count+1 && events.size()==events_before+2,"Public wrapper did not run its original exactly once");need(same(c,native_result),"Complete native result context was not preserved");
  need(events[events_before].a0==0x800CA000 && events.back().a0==0x800CA000 && events.back().a1==0x800CB000 && events.back().complete==1,"Captured A0/A1/normal completion changed");need(owner==7 && owner_stack.empty(),"Ordinary owner scope leaked");
 }
 const std::array pairs={E(8006009C),E(80060274),E(800603E8),E(80060798),E(80060960)};
 skip_pair=true;
 for(auto e:pairs){c=incoming();auto expected=c;expected.r2=0;auto n=originals;auto before=events.size();e.fn(ram,&c);need(same(c,expected),"Skipped pair changed guest registers beyond V0=0");need(originals==n && events.size()==before,"Skipped pair called native original or entered scope");need(last_skip_fn==e.id && last_skip_work==0x800CA000,"Pair-work API lost canonical function or incoming work pointer");}
 skip_pair=false;mode=3;c=incoming();auto saved=c;bool caught=false;try{func_8006009C(ram,&c);}catch(const std::runtime_error&){caught=true;}need(caught && same(c,saved) && owner_stack.empty(),"Throwing collision hook lost input context or scope");
 mode=0;
 for(bool scoped:{true,false})for(uint32_t slot:{0u,1u,UINT32_MAX})for(bool fail:{false,true}){
  enabled=scoped;capture_slot=slot;mode=fail?4:0;c=incoming();auto n=originals,p=posts,k=captures;auto before=events.size();func_8003B8F0(ram,&c);
  need(originals==n+1 && captures==k+1 && posts==p+1,"Creator capture/post skipped or duplicated, including ticket0/nativeV0=0");need(last_post_slot==slot && last_post_actor==(fail?0u:0x800CD000u),"Creator did not preserve entry owner or native returned actor");need(same(c,native_result),"Creator post changed native V0 or another result register");need(events.size()==before+(scoped?2:0) && owner_stack.empty() && owner==7,"Creator scope lifetime/bypass mismatch");
 }
 enabled=true;mode=1;c=incoming();func_80043450(ram,&c);need(owner==7 && owner_stack.empty() && events.back().complete==1,"Nested draw scope failed");
 mode=2;c=incoming();auto p=posts;caught=false;try{func_8003B8F0(ram,&c);}catch(const std::runtime_error&){caught=true;}need(caught && posts==p && same(c,native_result) && owner_stack.empty() && events.back().complete==0,"Native exception bypassed RAII or emitted a false birth");
 mode=0;enabled=false;c=incoming();auto before=events.size();func_8005E884(ram,&c);need(events.size()==before && same(c,native_result),"Disabled draw wrapper changed native result");
 for(auto e:std::array{E(80039F48),E(8003FFD8)}){
  c=incoming();auto before=events.size();e.fn(ram,&c);need(events.size()==before+1 && events.back().complete==1 && events.back().a0==0x800CA000,"Ticket0 attachment did not emit its normal completion");need(same(c,native_result) && owner_stack.empty(),"Ticket0 attachment post changed native output or entered an owner");
 }
 for(bool scoped:{true,false})for(uint64_t token:{uint64_t(0),uint64_t(0x1122334455667788ULL)}){
  enabled=scoped;mode=0;init_token=token;c=incoming();auto b=init_begins,f=init_finishes,n=originals;func_8003AE00(ram,&c);
  need(init_begins==b+1 && init_finishes==f+1 && originals==n+1,"Private init did not run begin/native/finish exactly once");need(last_init_actor==0x800CA000 && last_finished_token==token && last_finished_completed==1,"Private init lost actor or captured one-shot token");need(same(c,native_result) && owner_stack.empty() && owner==7,"Private init result/owner scope changed");
 }
 enabled=true;mode=2;init_token=0x8877665544332211ULL;c=incoming();auto finishes=init_finishes;caught=false;
 try{func_8003AE00(ram,&c);}catch(const std::runtime_error&){caught=true;}
 need(caught && init_finishes==finishes+1 && last_finished_token==0x8877665544332211ULL && last_finished_completed==0,"Throwing native private initialization did not cancel its captured token");need(same(c,native_result) && owner_stack.empty() && owner==7 && events.back().complete==0,"Private init cancel did not preserve native registers/RAII");
 mode=0;enabled=false;
 selected_player=0;c=incoming();func_8004700C(ram,&c);need(same(c,native_result),"Unscoped current-player lookup changed native clobbers");
 selected_player=0x800CE000;c=incoming();func_8004700C(ram,&c);auto expected=native_result;expected.r2=gpr(int32_t(0x800CE000));need(same(c,expected),"Scoped current-player lookup lost sign extension or native clobbers");
 need(static_cast<uint32_t>(workshop::coop::dispatch::Function::actor_draw)==0x8005E884 && static_cast<uint32_t>(workshop::coop::dispatch::Function::legacy_actor_kind_handler)==0x80022448,"Draw identifiers still conflate legacy22448 with true type3 drawing");
 std::cout<<"Dispatch0231 PASS: 26 real public symbols; "<<checks<<" assertions; collision skip, owner capture, egg post ticket0/failure, registers and RAII. No game launched.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
