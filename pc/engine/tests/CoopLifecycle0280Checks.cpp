// Exact dispatcher, registration, health setter and destructor bodies execute.
// Actor allocator/init, scheduler/resource services are explicit HOST boundaries.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#include "../coop_native_bridge.hpp"
#pragma clang diagnostic pop
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <vector>
extern "C" void Fault_HungUp(uint8_t*,recomp_context*);
extern "C" void workshop_find_reset();
static unsigned camera_releases=0;
extern "C" void workshop_trace_entry(uint8_t*,recomp_context*,const char*);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#include "D:/Yoshi-Story/pc/build/coop-0280/native-life.inc"
#include "D:/Yoshi-Story/pc/build/coop-0280/native-cache.inc"
#define WORKSHOP_SPECIAL_CONTENT
#include "D:/Yoshi-Story/pc/build/coop-0280/native-death-pause.inc"
#undef WORKSHOP_SPECIAL_CONTENT
#pragma clang diagnostic pop
#ifndef COOP_REAL_REJOIN
extern "C" void workshop_trace_entry(uint8_t*,recomp_context*,const char* fn){if(std::strcmp(fn,"func_8002AF8C")==0)++camera_releases;}
#endif
namespace b=workshop::coop_bridge;namespace n=workshop::coop_native;namespace c=workshop::coop_collision;namespace l=workshop::coop_lifecycle0280;
static unsigned checks=0,ends=0,created=0,cleaned=0;static unsigned vi=0,room_key=43;static uint32_t next_root=0x80108000;static bool dirty_fifo=false,defer_resources=false;
static constexpr uint32_t first=0x80100000,second=0x80102000,manager=0x800F8814,table=0x800D82E8;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
static uint32_t u(uint8_t* r,uint32_t a){return n::word(r,a);}static void w(uint8_t* r,uint32_t a,uint32_t v){n::word(r,a,v);}static void by(uint8_t* r,uint32_t a,unsigned v){n::byte(r,a,v);}
static void h(uint8_t* rdram,uint32_t a,unsigned v){MEM_H(0,int32_t(a))=uint16_t(v);}
static uint32_t data_for(uint32_t a){return 0x80200000+(a-0x80100000)*2;}
static void birth(uint8_t* r,uint32_t a,unsigned kind,uint32_t data){
 need(c::created(a),"Every actor incarnation has a real creation event");recomp_context x{};x.r4=int32_t(a);n::created(&x);
 std::memset(r+(a&0x7fffff),0,0x280);w(r,a,0x19971110);h(r,a+12,kind);w(r,a+0x1F4,data);by(r,a+0x18,1);by(r,a+0x17D,2);
}
static void init(uint8_t* r,uint32_t a){
 const auto data=data_for(a);std::memset(r+(data&0x7fffff),0,0x1550);w(r,a+0x1F4,data);w(r,a+0x20,0x80043450);w(r,a+0x130,0x800233B4);w(r,a+0x16C,data+0x1600);w(r,a+0x174,data+0x1700);by(r,a+0x164,3);
 h(r,a+0x184,3);for(unsigned i=0;i<3;i++){const auto child=a+0x300+i*0x300;birth(r,child,i==0?0x400B:i==1?0x400C:0x4007,data+0x1800+i*0x100);w(r,child+0x180,a);h(r,child+0x17E,i);w(r,a+0x188+i*4,child);}
 w(r,data+0x160,a+0x300);w(r,data+0x15C,a+0x600);if(dirty_fifo)w(r,data+0x11FC,0x1234);
}
extern "C" void func_80021A44(uint8_t*,recomp_context* x){x->r5=0x1234;} // Native dispatch may clobber A1.
extern "C" void func_80022E0C(uint8_t* rdram,recomp_context*){
 // HOST pad-service boundary: no native smoothing/geometry claim.
 const auto raw=n::word(rdram,0x800F7EA8);MEM_H(0,int32_t(0x800F897E))=MEM_B(2,int32_t(raw));MEM_H(0,int32_t(0x800F8980))=MEM_B(3,int32_t(raw));
 const auto held=MEM_HU(0,int32_t(raw)),edge=MEM_HU(12,int32_t(raw));MEM_B(0,int32_t(0x800F8995))=(held&0x8000)!=0;MEM_B(0,int32_t(0x800F8996))=(edge&0x8000)!=0;MEM_B(0,int32_t(0x800F8997))=(held&0x4000)!=0;MEM_B(0,int32_t(0x800F8998))=(edge&0x4000)!=0;
}
extern "C" void func_8002B3A0(uint8_t*,recomp_context*){}
extern "C" void func_8002AEE4(uint8_t*,recomp_context*){}
extern "C" void func_80067878(uint8_t*,recomp_context*){}
extern "C" void workshop_find_reset(){}
extern "C" void func_8006E0FC(uint8_t*,recomp_context*){++ends;}
extern "C" void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Native fault");}
#ifndef COOP_REAL_REJOIN
extern "C" void func_80043830(uint8_t*,recomp_context* x){x->r2=1;} // Scheduler registration predicate boundary.
#endif
#ifndef COOP_REAL_REJOIN
extern "C" void func_80057BAC(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected occupied registration");}
#endif
#ifndef COOP_REAL_REJOIN
extern "C" void func_80057B28(uint8_t* r,recomp_context* x){const auto t=u(r,uint32_t(x->r4)+0x10);need(u(r,t+uint32_t(x->r5)*4)==0,"Registration never overwrites another root");w(r,t+uint32_t(x->r5)*4,uint32_t(x->r6));x->r2=1;}
#endif
extern "C" void func_8005FDC4(uint8_t*,recomp_context*){}
extern "C" void func_80045414(uint8_t* r,recomp_context* x){w(r,uint32_t(x->r4)+0x16C,0);}
extern "C" void func_800528C8(uint8_t* r,recomp_context* x){w(r,uint32_t(x->r4)+0x174,0);}
extern "C" void func_8004597C(uint8_t* r,recomp_context* x){if(!defer_resources)w(r,uint32_t(x->r4)+0x1F4,0);}
extern "C" void func_80045AE8(uint8_t*,recomp_context*){} // Actor pool disposal boundary after real45B64.
extern "C" void func_8004613C(uint8_t* r,recomp_context* x){
 ++created;const auto a=next_root;next_root+=0x2000;const auto packed=uint32_t(x->r4),xb=uint32_t(x->r5),yb=uint32_t(x->r6),zb=uint32_t(x->r7);birth(r,a,0x4001,0);w(r,a+12,packed);w(r,a+0x20,0x800436E0);for(auto pair:{std::pair{0x3Cu,xb},std::pair{0x40u,yb},std::pair{0x44u,zb},std::pair{0x84u,xb},std::pair{0x88u,yb},std::pair{0x8Cu,zb}})w(r,a+pair.first,pair.second);x->r2=int32_t(a);
}
extern "C" void func_80048A30(uint8_t* r,recomp_context* x){init(r,uint32_t(x->r4));}
extern "C" void func_800445A4(uint8_t* r,recomp_context* x){
 const auto a=uint32_t(x->r4);const auto graph=n::tree(r,a);need(!graph.empty(),"Cleanup starts with a real graph");
 for(auto it=graph.rbegin();it!=graph.rend();++it){const auto actor=*it;for(unsigned i=0;i<150;i++)if(u(r,table+i*4)==actor)w(r,table+i*4,0);h(r,actor+0x184,0);for(unsigned i=0;i<20;i++)w(r,actor+0x188+i*4,0);w(r,actor+0x180,0);recomp_context dead=*x;dead.r4=int32_t(actor);n::native_cleanup(r,&dead);if(!defer_resources){if(n::byte(r,actor+5)&&n::byte(r,actor+6)==1){by(r,actor+0x17D,0);}else{by(r,actor+6,3);b::forget(actor);}}++cleaned;}
}
extern "C" void func_8006CB3C(uint8_t* r,recomp_context*){w(r,u(r,0x800F8294)+0x68,0);}
static void death(uint8_t* r,recomp_context* x){if(!b::intercept_death(r,x,vi,room_key))workshop_807F1100(r,x);}
extern "C" recomp_func_t* get_function(int32_t a){if(uint32_t(a)==0x8002F5F4)return death;throw std::runtime_error("Unexpected native callback");}
// Unexecuted capacity/graphics/egg service boundaries, required by Bridge::frame.
#define UNUSED_NATIVE(name) extern "C" void name(uint8_t*,recomp_context*){throw std::runtime_error(#name " not admitted in targeted fixture");}
UNUSED_NATIVE(func_8004661C) UNUSED_NATIVE(func_80064DD0) UNUSED_NATIVE(func_8004667C) UNUSED_NATIVE(func_80064E84) UNUSED_NATIVE(func_80046790) UNUSED_NATIVE(func_80046324)
struct Fixture{
 std::vector<uint8_t> ram=std::vector<uint8_t>(0x800000);uint8_t* r=ram.data();recomp_context x{};
 Fixture(){ends=created=cleaned=camera_releases=0;room_key=43;next_root=0x80108000;dirty_fifo=defer_resources=false;vi=2500;b::configure(true,5,nullptr,false,false);n::attempted=true;
 w(r,0x800F7EA8,0x803E0000);w(r,0x800F8810,manager);h(r,manager+8,150);w(r,manager+0x10,table);w(r,n::pool_header+0x10,0x803F0000);h(r,n::pool_header+8,8);for(unsigned i=0;i<8;i++){const auto a=0x80130000+i*0x300;w(r,0x803F0000+i*4,a);by(r,a+6,3);}w(r,0x800A6380,8);w(r,0x800A6384,0x803F0100);
 for(auto a:{first,second}){birth(r,a,0x4001,0);init(r,a);w(r,a+0x84,std::bit_cast<uint32_t>(200.f));w(r,a+0x88,std::bit_cast<uint32_t>(449.f));w(r,a+0x8C,std::bit_cast<uint32_t>(500.f));}
 w(r,table+20,first);w(r,table+24,second);h(r,first+0x17E,5);h(r,second+0x17E,6);need(c::bind_root(r,0,first)&&c::bind_root(r,1,second),"Fresh actual roots bind");
 n::primary=first;n::secondary=second;n::remember(r,second);b::bound_primary=first;b::bound_secondary=second;b::memory.ram=r;b::context=std::make_unique<workshop::coop::OwnerContext>(b::memory);b::context->bind(0,{{first,1},0,8});b::context->bind(1,{{second,2},5,8});b::owner_packed={u(r,first+12),u(r,second+12)};
 w(r,workshop::coop::native_player,first);w(r,workshop::coop::native_color,0);w(r,workshop::coop::custom_color,0);for(unsigned i=0;i<8;i++)w(r,workshop::coop::native_health+i*4,8);w(r,0x807F1000,0x3FFF);w(r,0x800F8DF4,0xFF);w(r,0x800F89D0,1);w(r,0x800F89D4,7);w(r,0x800F8950,1234);x.r29=int32_t(0x807A0000);
 need(l::lifecycle.bind(r,first,second,0,5,43,vi),"Lifecycle captures exact birth pair");
 }
 // Each HOST fixture owns and discards its full8MiB, including simulated heap metadata.
 ~Fixture(){workshop::coop_egg_pools::state={};}
 void escorts(unsigned p){
  const auto root=p?second:first;constexpr uint32_t pool=0x80380000,storage=0x80380010,resource=0x80382000,overlay=0x80390000;
  w(r,pool+4,4);w(r,pool+8,storage);h(r,resource+0x3C,0x4015);w(r,resource+0x40,overlay);w(r,resource+0x44,overlay+0xB48);h(r,root+0x184,7);
  for(unsigned i=0;i<4;i++){const auto a=storage+i*0x280+4;birth(r,a,0x4015,0);by(r,a-4,1);by(r,a+5,i+1);by(r,a+6,2);by(r,a+0x18,0);by(r,a+0x30,8);w(r,a+0x1E0,i);by(r,a+0x17D,0);w(r,a+8,pool);w(r,a+0x168,resource);w(r,a+0x20,overlay+0xA80);w(r,a+0x2C,overlay+0xAC0);w(r,a+0x16C,0x80385000+i*0x200);w(r,a+0x174,0x80385100+i*0x200);w(r,a+0x180,root);h(r,a+0x17E,3+i);w(r,root+0x188+(3+i)*4,a);}
 }
 void reserves(unsigned p,unsigned count,unsigned head=7){
  namespace e=workshop::coop_egg_pools;auto& state=e::state;
  if(!state.installed){
   state.enabled=state.installed=true;state.initialized={true,true};state.allocation=0x80360000;state.manager=0x800F8880;state.table=0x80362000;state.manager_capacity=150;state.primary=e::stock_descriptors;
   h(r,state.allocation-0x10,0x913);h(r,state.allocation-0xE,1);w(r,state.allocation-4,e::block_size+0x10);w(r,e::manager_global,state.manager);h(r,state.manager+8,150);h(r,state.manager+6,4);w(r,state.manager+0x10,state.table);
   for(unsigned owner=0;owner<2;owner++)for(unsigned k=0;k<2;k++){const auto d=e::pool(owner,k);w(r,d+4,e::capacities[k]);w(r,d+8,owner?state.allocation+e::record_offsets[k]:e::stock_records[k]);if(owner)w(r,d+12,e::guard|uint32_t(0x4004+k));w(r,state.table+(owner*2+k)*4,d);}
   w(r,e::globals[0],state.primary[0]);w(r,e::globals[1],state.primary[1]);
  }
  const auto root=p?second:first,data=u(r,root+0x1F4),pool=e::pool(p,0),storage=u(r,pool+8);constexpr uint32_t resource=0x80364000;
  h(r,resource+0x3C,0x4004);w(r,resource+0x40,0);w(r,resource+0x44,0x8003B2B0);const unsigned old=c::half(r,root+0x184);h(r,root+0x184,old+count);h(r,data+0x12BE,head);by(r,data+0x12C8,count);
  unsigned child=0;for(unsigned i=0;i<count;i++){
   while(u(r,root+0x188+child*4))++child;const auto a=storage+i*0x280+4;birth(r,a,0x4004,a+0x24C);by(r,a-4,1);by(r,a+5,i+1);by(r,a+6,2);by(r,a+0x17D,0);h(r,a+0x17E,child);w(r,a+8,pool);w(r,a+0x168,resource);w(r,a+0x16C,a+0x1FC);w(r,a+0x174,0x80365000+(p*8+i)*0x20);w(r,a+0x20,0x8003AA10);w(r,a+0x2C,0x80045BD8);w(r,a+0x180,root);w(r,root+0x188+child*4,a);w(r,data+0x11FC+((head+i)&7)*24,a);
  }
  need(e::matches(r),"Both owner-specific native reserve pools remain registered and disjoint");
 }
 void pause(){
  constexpr uint32_t camera=0x80368000,drawing=0x80368100;
  w(r,0x800F8294,camera);w(r,0x800F8318,drawing);w(r,camera+0x80,drawing);w(r,camera+0x78,0x8006CF04);w(r,drawing+0xF4,0x8006920C);w(r,0x800F7EB8,0x80022E4C);w(r,camera+0x68,0);recomp_context setter=x;func_8002AF3C(r,&setter);
  need(n::byte(r,0x800F7EA5)==1&&n::byte(r,0x800F7EB4)==1,"Exact native2AF3C establishes both global death-pause bytes");
 }
 void hp(unsigned p,unsigned value){b::Scope scope(r,&x,p);w(r,workshop::coop::native_health+workshop::coop::native_alias(b::context->player(p).color)*4,value);}
 bool kill(unsigned p,unsigned dispatcher=0){const auto a=p?b::bound_secondary:b::bound_primary,data=u(r,a+0x1F4);by(r,a+0x18,0xA);by(r,a+0x30,0xC);w(r,a+0x20,0x8003085C);w(r,data+0xC0,0x8002F5F4);w(r,data+0xD8,3);b::Scope scope(r,&x,p);x.r4=int32_t(a);x.r5=int32_t(data);if(dispatcher==0){x.r31=int32_t(0x80030878);func_80042148(r,&x);return uint32_t(x.r2)==0;}x.r31=int32_t(dispatcher);return b::intercept_death(r,&x,vi,room_key);}
 void fence(unsigned at){vi=at;b::frame(r,&x,vi,room_key);}
};
int main(){try{
 for(unsigned owner=0;owner<2;owner++)for(unsigned survivor_hp:{0u,8u}){
  Fixture f;const auto a=owner?second:first;const auto old=c::observed_identity(f.r,a);f.hp(1-owner,survivor_hp);const auto shared=b::progress_hash(f.r);need(f.kill(owner),"Real42148 intercepted and returnsV0=0");need(!ends&&cleaned==0,"No global ending or destruction on dispatcher stack");need(u(f.r,old->private_data+0xC0)==0,"Real42148 clears C0 after callback");f.fence(2501);need(!b::invalidated&&!l::lifecycle.snapshot().failed,"Full Bridge fence after cleanup remains admitted");need((owner?b::bound_secondary:b::bound_primary)==0,"Missing bound slot is explicitly zero during delay");need(u(f.r,workshop::coop::native_player)==(owner?first:second),"Freed singleton never survives cleanup");need(b::context->player(1-owner).health==survivor_hp,"Survivor HP0 remains playable and is never healed");f.fence(2799);need(created==0,"No reconstruction before300VI");f.fence(2800);need(created==1&&!l::lifecycle.snapshot().failed,"Exactly one reconstruction at300VI on native grounded anchor");const auto fresh=owner?b::bound_secondary:b::bound_primary;need(n::primary_ready(f.r,fresh)&&b::empty_respawn_inventory(f.r,fresh)&&b::context->player(owner).health==2,"Read back2HP and genuinely empty eight-record FIFO");const auto id=c::observed_identity(f.r,fresh);need(id&&old&&id->generation!=old->generation,"Rebirth has new native creation generation");need(u(f.r,table+20)==b::bound_primary&&c::half(f.r,fresh+0x17E)==(owner?6:5),"P1 canonical slot5/P2 generic registration preserved");need(b::progress_hash(f.r)==shared,"Nonempty native shared fruit/score untouched by cleanup and constructor");f.fence(2801);need(created==1,"No repeated respawn while alive");
 }
 for(unsigned owner=0;owner<2;owner++){
  Fixture f;f.escorts(owner);f.kill(owner);f.fence(2501);need(l::lifecycle.snapshot().players[owner].phase==l::Phase::waiting,"Exact native468E8 cached escorts do not block dynamic-root retirement");need(b::retiring_escorts[owner].size()==4,"Exactly four typed escorts were captured before native retirement");
  for(const auto& [a,receipt]:b::retiring_escorts[owner]){need(b::escort_cache_returned(f.r,receipt)&&!n::cleanup_leases.contains(a)&&n::word(f.r,a+0x16C)&&n::word(f.r,a+0x174),"Returned4015 retains native resources while its cleanup lease is released");need(!b::retired_identity(f.r,receipt.id),"Resource-retaining actor is not accepted without its exact cache receipt");}
  const auto receipt=b::retiring_escorts[owner].begin()->second;const auto a=receipt.id.address;const auto before=f.ram;
  for(unsigned bad=0;bad<13;bad++){f.ram=before;f.r=f.ram.data();b::memory.ram=f.r;switch(bad){case 0:w(f.r,a+0x180,first);break;case 1:w(f.r,a+8,receipt.pool+4);break;case 2:by(f.r,a-4,1);break;case 3:by(f.r,a+5,receipt.slot+1);break;case 4:h(f.r,a+0x184,1);break;case 5:by(f.r,a+6,2);break;case 6:h(f.r,a+0x17E,0);break;case 7:by(f.r,a+0x17D,2);break;case 8:w(f.r,receipt.pool+4,5);break;case 9:w(f.r,receipt.pool+8,receipt.storage+4);break;case 10:w(f.r,a+0x1D8,1);break;case 11:w(f.r,a+0x168,0xFFFFFFFF);break;case 12:w(f.r,a+0x188,second);break;}need(!b::escort_cache_returned(f.r,receipt),"Foreign/busy/stale/attached cache state cannot complete retirement");}
  f.ram=before;f.r=f.ram.data();b::memory.ram=f.r;f.fence(2800);need(!l::lifecycle.snapshot().failed&&created==1,"300VI native rebuild proceeds after all four escort slots returned");
 }
 // The true45BD8 ->468E8 bodies return reserve4004 to their owner pool.
 // Allocation/init/render and recursive tree scheduling remain HOST services.
 for(unsigned owner=0;owner<2;owner++)for(unsigned count:{1u,3u,6u}){
  Fixture f;f.reserves(owner,count);f.reserves(1-owner,2);f.hp(1-owner,0);const auto shared=b::progress_hash(f.r);const auto survivor=owner?first:second,survivor_data=u(f.r,survivor+0x1F4);const auto survivor_pool=workshop::coop_egg_pools::pool(1-owner,0),survivor_storage=u(f.r,survivor_pool+8);const auto survivor_bytes=std::vector<uint8_t>(f.r+(survivor_storage&0x7fffff),f.r+(survivor_storage&0x7fffff)+8*0x280);const auto survivor_fifo=std::vector<uint8_t>(f.r+((survivor_data+0x11FC)&0x7fffff),f.r+((survivor_data+0x11FC)&0x7fffff)+8*24);
  need(f.kill(owner),"Nonempty-FIFO owner has a real intercepted fatal dispatcher");f.fence(2501);need(!l::lifecycle.snapshot().failed&&l::lifecycle.snapshot().players[owner].phase==l::Phase::waiting,"Native cached reserve return completes retirement for either owner");need(b::retiring_reserves[owner].size()==count,"Every captured reserve is an exact native FIFO member");
  for(const auto& [a,receipt]:b::retiring_reserves[owner]){need(b::reserve_cache_returned(f.r,receipt)&&!n::cleanup_leases.contains(a),"Returned reserve receipt releases its cleanup lease");need(u(f.r,a+0x1F4)&&u(f.r,a+0x16C)&&u(f.r,a+0x174)&&!b::retired_identity(f.r,receipt.id),"Retained resources do not imply full free without the precise reserve receipt");}
  need(std::memcmp(f.r+(survivor_storage&0x7fffff),survivor_bytes.data(),survivor_bytes.size())==0&&std::memcmp(f.r+((survivor_data+0x11FC)&0x7fffff),survivor_fifo.data(),survivor_fifo.size())==0&&n::byte(f.r,survivor_data+0x12C8)==2,"Survivor cached eggs and wrapped FIFO remain byte-identical");
  const auto receipt=b::retiring_reserves[owner].begin()->second;const auto a=receipt.id.address;const auto before=f.ram;
  for(unsigned bad=0;bad<21;bad++){f.ram=before;f.r=f.ram.data();b::memory.ram=f.r;switch(bad){case 0:w(f.r,a+0x180,survivor);break;case 1:w(f.r,a+8,receipt.pool+4);break;case 2:by(f.r,a-4,1);break;case 3:by(f.r,a+5,receipt.slot+1);break;case 4:h(f.r,a+0x184,1);break;case 5:by(f.r,a+6,2);break;case 6:h(f.r,a+0x17E,0);break;case 7:by(f.r,a+0x17D,2);break;case 8:w(f.r,receipt.pool+4,7);break;case 9:w(f.r,receipt.pool+8,receipt.storage+4);break;case 10:w(f.r,a+0x1D8,1);break;case 11:w(f.r,a+0x168,0xFFFFFFFF);break;case 12:w(f.r,a+0x188,survivor);break;case 13:w(f.r,a+0x1F4,a+0x250);break;case 14:w(f.r,receipt.resource+0x44,0x8003B2B4);break;case 15:h(f.r,receipt.resource+0x3C,0x4005);break;case 16:w(f.r,a+0x20,0x8003AA14);break;case 17:w(f.r,a+0x2C,0x80045B64);break;case 18:w(f.r,a+0x16C,0);break;case 19:by(f.r,a+4,1);break;case 20:w(f.r,receipt.table+8,0);break;}need(!b::reserve_cache_returned(f.r,receipt),"Foreign/busy/corrupt/unregistered reserve does not attest cache return");}
  f.ram=before;f.r=f.ram.data();b::memory.ram=f.r;auto other=receipt;other.owner=1-owner;need(!b::reserve_cache_returned(f.r,other),"Other owner's valid pool never substitutes for the captured reserve pool");
  f.fence(2799);need(created==0,"Reserve return cannot shorten the native300VI delay");f.fence(2800);const auto fresh=owner?b::bound_secondary:b::bound_primary;need(!l::lifecycle.snapshot().failed&&created==1&&b::empty_respawn_inventory(f.r,fresh),"Nonempty old FIFO still respawns once with a genuinely empty new FIFO");need(b::context->player(1-owner).health==0&&b::progress_hash(f.r)==shared,"Survivor HP0 and common fruit/score are conserved with reserve cleanup");
 }
 {Fixture f;f.escorts(0);f.reserves(0,3);need(f.kill(0),"Actual-sized11-member root has a native fatal call");f.fence(2501);need(!l::lifecycle.snapshot().failed&&b::retiring_graphs[0].size()==11&&b::retiring_escorts[0].size()==4&&b::retiring_reserves[0].size()==3&&l::lifecycle.snapshot().players[0].phase==l::Phase::waiting,"Three4004 caches and four4015 caches complete the actual11-member graph");}
 for(unsigned bad=0;bad<7;bad++){Fixture f;f.reserves(0,3);need(f.kill(0),"Reserve capture negative begins with a true fatal call");const auto root=l::lifecycle.snapshot().players[0].root;const auto a=u(f.r,root.private_data+0x11FC+7*24);const auto id=c::observed_identity(f.r,a);b::ReserveCache receipt;need(id&&b::reserve_cache_capture(f.r,*id,0,receipt),"Known wrapped FIFO reserve capture before mutation");switch(bad){case 0:w(f.r,root.private_data+0x11FC+7*24,0);break;case 1:w(f.r,root.private_data+0x11FC, a);break;case 2:by(f.r,root.private_data+0x12C8,7);break;case 3:h(f.r,root.private_data+0x12BE,8);break;case 4:w(f.r,a+0x180,second);break;case 5:h(f.r,a+0x17E,19);break;case 6:h(f.r,a+12,0x4005);break;}need(!b::reserve_cache_capture(f.r,*id,0,receipt),"Unknown/duplicated FIFO, reciprocal mismatch, wrong owner or4005 cannot create a reserve receipt");}
 {Fixture f;f.reserves(0,1);f.kill(0);f.fence(2501);const auto receipt=b::retiring_reserves[0].begin()->second;need(c::created(receipt.id.address),"Reused cached slot receives a real new incarnation event");need(!b::reserve_cache_returned(f.r,receipt),"Old reserve receipt never admits a reused incarnation");need(!n::cleanup_leases.contains(receipt.id.address),"No cleanup lease survives native cache return for reuse");}
 // Real native pause/unpause leaves every other guest byte untouched.
 {Fixture f;by(f.r,0x800F7EA4,0xA5);by(f.r,0x800F7EA6,0x5A);by(f.r,0x800F7EB5,0xCC);auto expected=f.ram;recomp_context setter=f.x;func_8002AF3C(f.r,&setter);expected[(0xF7EA5u)^3]=1;expected[(0xF7EB4u)^3]=1;need(f.ram==expected,"Native2AF3C writes exactly two bytes with surrounding sentinels preserved");func_8002AF8C(f.r,&setter);expected[(0xF7EA5u)^3]=0;expected[(0xF7EB4u)^3]=0;need(f.ram==expected&&camera_releases==1,"Native2AF8C writes exactly its two inverse bytes, no camera rectangle or pose");}
 for(unsigned owner=0;owner<2;owner++)for(unsigned room:{43u,81u}){
  Fixture f;if(room==81)need(l::lifecycle.transferred(f.r,room),"HOST paired-arrival fence sets room81 for the lifecycle guard");room_key=room;f.reserves(owner,3);f.reserves(1-owner,2);f.hp(1-owner,0);f.pause();const auto progress=b::progress_hash(f.r);const auto survivor=owner?first:second;const auto sx=u(f.r,survivor+0x84),sy=u(f.r,survivor+0x88);const auto camera=u(f.r,0x800F8294),drawing=u(f.r,0x800F8318);w(f.r,camera+0x84,0x44AAAAAA);w(f.r,camera+0x9C,0x45BBBBBB);w(f.r,drawing+0x30,0x44CCCCCC);
  need(f.kill(owner)&&b::death_pauses[owner].has_value(),"Exact fatal call captures the paused camera receipt in either room/owner");need(camera_releases==0,"Death callback never unpauses while still on the native dispatcher stack");f.fence(2501);need(!l::lifecycle.snapshot().failed&&l::lifecycle.snapshot().players[owner].phase==l::Phase::waiting&&camera_releases==1&&!b::death_pauses[owner],"Freed receipt releases native camera once before the300VI wait ends");need(!n::byte(f.r,0x800F7EA5)&&!n::byte(f.r,0x800F7EB4)&&u(f.r,camera+0x84)==0x44AAAAAA&&u(f.r,camera+0x9C)==0x45BBBBBB&&u(f.r,drawing+0x30)==0x44CCCCCC,"Death pause inverse does not rewrite camera geometry");need(u(f.r,survivor+0x84)==sx&&u(f.r,survivor+0x88)==sy&&b::context->player(1-owner).health==0&&n::byte(f.r,u(f.r,survivor+0x1F4)+0x12C8)==2&&b::progress_hash(f.r)==progress,"Survivor HP0/FIFO2/pose and common progress remain unchanged");f.fence(2799);need(camera_releases==1&&created==0,"No repeated camera setter or early respawn during the delay");f.fence(2800);need(camera_releases==1&&created==1&&!l::lifecycle.snapshot().failed,"Native return uses the already resumed camera without a second inverse");
 }
 for(unsigned bad=0;bad<19;bad++){
  Fixture f;f.pause();need(f.kill(0)&&b::death_pauses[0].has_value(),"Negative starts with an actual captured fatal pause");const auto receipt=*b::death_pauses[0];b::death_pauses[0].reset();f.fence(2501);b::death_pauses[0]=receipt; // HOST test suppresses only auto-release to examine guards after real cleanup.
  switch(bad){case 0:b::owner_stack.push_back(1);break;case 1:workshop::coop_egg_pools::state.depth=1;break;case 2:workshop::coop_native_input::depth=1;break;case 3:n::constructing=true;break;case 4:b::invalidated=true;break;case 5:b::death_pauses[0]->epoch++;break;case 6:b::death_pauses[0]->room=81;break;case 7:b::death_pauses[0]->root.generation++;break;case 8:w(f.r,0x800F7EB8,0x80022E50);break;case 9:w(f.r,0x800F8294,0xFFFFFFFF);break;case 10:w(f.r,receipt.camera+0x80,0);break;case 11:w(f.r,0x800F8318,0xFFFFFFFF);break;case 12:w(f.r,receipt.camera+0x78,0x8006CF40);break;case 13:w(f.r,receipt.drawing+0xF4,0x80069210);break;case 14:by(f.r,0x800F7EA5,2);break;case 15:by(f.r,0x800F7EB4,0);break;case 16:by(f.r,second+0x18,0xA);by(f.r,second+0x30,0xC);break;case 17:need(c::created(second),"Unknown survivor incarnation has a real creation event");break;case 18:workshop::coop_rooms_native0280::admission.emplace();break;}
  const auto before=f.ram;const auto regs=f.x;const bool released=b::release_death_pause(f.r,&f.x,0,2501,43);need(released==(bad==16)&&camera_releases==0&&f.ram==before&&std::memcmp(&regs,&f.x,sizeof(regs))==0,"Unknown contexts refuse; exact partnerfatal defers without writes or an invented ending");b::owner_stack.clear();workshop::coop_egg_pools::state.depth=0;workshop::coop_native_input::depth=0;n::constructing=false;
 }
 {Fixture f;f.pause();f.kill(0);need(!b::release_death_pause(f.r,&f.x,0,2500,43)&&camera_releases==0,"A pending death cannot release before retirement and freed proof");}
 {Fixture f;f.pause();f.kill(0);b::death_pauses[0].reset();f.fence(2501);need(camera_releases==0&&n::byte(f.r,0x800F7EA5)==1,"Missing typed death-pause receipt never clears a global pause");}
 {Fixture f;f.pause();f.kill(0);by(f.r,second+0x18,0xA);by(f.r,second+0x30,0xC);f.fence(2501);need(!l::lifecycle.snapshot().failed&&!l::lifecycle.snapshot().ending&&b::death_pauses[0]&&camera_releases==0&&n::byte(f.r,0x800F7EA5)==1,"Freed first owner waits safely while known partnerfatal has not reached its callback");f.fence(2502);need(!l::lifecycle.snapshot().failed&&camera_releases==0,"Subsequent waiting fence does not turn legitimate fatal animation into an invariant failure");need(f.kill(1)&&ends==1&&l::lifecycle.snapshot().ending,"Only the second real fatal callback selects nativebothdown");}
 {Fixture f;f.pause();f.kill(1);f.kill(0);need(ends==1&&l::lifecycle.snapshot().ending&&!b::release_death_pause(f.r,&f.x,1,2500,43)&&camera_releases==0,"Both-down keeps native ending and its pause untouched");}
 for(unsigned p=0;p<2;p++){Fixture f;const auto old=c::observed_identity(f.r,p?second:first);f.kill(p);f.fence(2501);next_root=old->address;f.fence(2800);need(!l::lifecycle.snapshot().failed&&l::lifecycle.snapshot().players[p].root.address==old->address&&l::lifecycle.snapshot().players[p].root.generation!=old->generation,"Actual same-address pool reuse is admitted only after a new native birth");}
 {Fixture f;for(unsigned slot=0;slot<48;slot++){w(f.r,data_for(first)+0x11FC+slot*4,1);need(!b::empty_respawn_inventory(f.r,first),"Every native FIFO payload word is read back, not merely its counter");w(f.r,data_for(first)+0x11FC+slot*4,0);}h(f.r,data_for(first)+0x12BE,1);need(!b::empty_respawn_inventory(f.r,first),"Nonempty native FIFO head refused");h(f.r,data_for(first)+0x12BE,0);by(f.r,data_for(first)+0x12C8,1);need(!b::empty_respawn_inventory(f.r,first),"Native reserve count must be zero");}
 {Fixture f;w(f.r,first+0x300+0x180,0xFFFFFFFF);std::vector<c::Identity> graph;need(!b::retirement_plan(f.r,0,graph),"Invalid graph parent refuses retirement before a foreign dereference");}
 for(unsigned owner=0;owner<2;owner++){
  Fixture f;const uint32_t egg=0x80120000,other=0x80121000;birth(f.r,egg,0x4002,0x80270000);birth(f.r,other,0x4002,0x80271000);need(c::registry.egg_created(f.r,egg,owner)&&c::registry.egg_created(f.r,other,1-owner),"Both actual flying egg births attributed before retirement");
  f.kill(owner);f.fence(2501);need(!c::observed_identity(f.r,egg)&&c::observed_identity(f.r,other).has_value(),"Native cleanup retires only the dead owner's detached flying egg");need(!l::lifecycle.snapshot().failed,"Flying egg cleanup preserves survivor and lifecycle");
 }
 {Fixture f;f.kill(1);f.fence(2501);dirty_fifo=true;f.fence(2800);need(l::lifecycle.snapshot().failed,"Dirty constructor FIFO cannot emit a reborn receipt");}
 {Fixture f;f.kill(1);defer_resources=true;f.fence(2501);f.fence(2800);need(created==0&&l::lifecycle.snapshot().players[1].phase==l::Phase::retiring,"Live resources defer reuse even after300VI");}
 {Fixture f;need(f.kill(1),"First death waits");need(f.kill(0),"Second real dispatcher completes");need(ends==1&&u(f.r,0x807F1004)==1,"Both dead invoke exact native global ending once");f.kill(0,0x800301D4);need(ends==1,"Repeated fatal callback cannot repeat global ending");}
 for(unsigned bad=0;bad<8;bad++){Fixture f;const auto id=c::observed_identity(f.r,first);by(f.r,first+0x18,0xA);by(f.r,first+0x30,0xC);w(f.r,first+0x20,0x8003085C);w(f.r,id->private_data+0xC0,0x8002F5F4);f.x.r4=int32_t(first);f.x.r31=int32_t(0x8004217C);w(f.r,uint32_t(f.x.r29)+0x14,0x80030878);w(f.r,uint32_t(f.x.r29)+0x1C,0x8002F5F4);w(f.r,uint32_t(f.x.r29)+0x20,first);w(f.r,uint32_t(f.x.r29)+0x24,id->private_data);need(l::death_call(f.r,&f.x,*id),"Exact swallowed native call admitted before mutation");switch(bad){case 0:by(f.r,first+0x30,2);break;case 1:w(f.r,first+0x20,0x80043450);break;case 2:c::forgotten(first);break;case 3:w(f.r,uint32_t(f.x.r29)+0x14,0);break;case 4:w(f.r,uint32_t(f.x.r29)+0x1C,0);break;case 5:w(f.r,uint32_t(f.x.r29)+0x20,second);break;case 6:w(f.r,uint32_t(f.x.r29)+0x24,id->private_data+4);break;case 7:f.x.r31=int32_t(0x80030BC4);break;}
 need(!l::death_call(f.r,&f.x,*id),"Pot/unknown/reused/stale dispatcher cannot count as death");
 }
 {Fixture f;need(l::lifecycle.alive(f.r,0),"Normal player alive");f.hp(0,0);need(l::lifecycle.alive(f.r,0),"HP0 is not a death");}
 for(unsigned owner=0;owner<2;owner++)for(auto ra:{0x800301D4u,0x80030780u}){Fixture f;if(ra==0x80030780u)w(f.r,(owner?second:first)+0x38,uint32_t(-41));need(f.kill(owner,ra),"Existing native fatal callers still admitted");need(!ends,"One existing death never ends both players");}
 // Ending diagnostics add no admission rule. The first case has ONLY ONE
 // real42148 callback: its partner is already in nativeA/C, with no HOST death.
 for(unsigned owner=0;owner<2;++owner){
  Fixture f;const unsigned other=1-owner;const auto partner=other?second:first;const auto id=c::observed_identity(f.r,partner);need(id.has_value(),"Partner native identity is known before fatal animation");by(f.r,partner+0x18,0xA);by(f.r,partner+0x30,0xC);
  std::fprintf(stderr,"[CoopEndingProofQA] case=native-fatal owner=%u begin=1\n",owner);need(f.kill(owner)&&ends==1&&l::lifecycle.snapshot().ending&&!l::lifecycle.snapshot().failed,"One real callback selects ending when the same bound partner is already nativefatal");need(!l::lifecycle.snapshot().players[other].death_vi&&l::lifecycle.snapshot().players[other].phase==l::Phase::alive&&created==0,"No second death callback or respawn is invented");f.kill(owner,0x800301D4);need(ends==1,"Repeated callback cannot repeat ending or proof");std::fprintf(stderr,"[CoopEndingProofQA] case=native-fatal owner=%u end=1\n",owner);
 }
 for(unsigned owner=0;owner<2;++owner){
  Fixture f;const unsigned other=1-owner;need(f.kill(other),"Recorded partner first uses real fatal dispatcher");f.fence(2501);const auto& dead=l::lifecycle.snapshot().players[other];need(dead.phase==l::Phase::waiting&&!c::observed_identity(f.r,dead.root.address),"Recorded partner has genuinely released resources and registry incarnation");
  std::fprintf(stderr,"[CoopEndingProofQA] case=freed owner=%u begin=1\n",owner);need(f.kill(owner)&&ends==1&&l::lifecycle.snapshot().ending&&!l::lifecycle.snapshot().failed,"Known recorded/freed partner allows existing native ending without dereferencing stale memory");std::fprintf(stderr,"[CoopEndingProofQA] case=freed owner=%u end=1\n",owner);
 }
 for(unsigned owner=0;owner<2;++owner){
  Fixture f;const unsigned other=1-owner;need(f.kill(other),"Reused-address negative starts with an actual prior fatal call");f.fence(2501);const auto expected=l::lifecycle.snapshot().players[other].root;birth(f.r,expected.address,0x4002,0xFFFF0000);const auto now=c::observed_identity(f.r,expected.address);need(now&&*now!=expected&&now->kind==0x4002&&now->generation!=expected.generation,"Same address now belongs to a different known actor/private and generation");
  std::fprintf(stderr,"[CoopEndingProofQA] case=reused owner=%u begin=1\n",owner);need(f.kill(owner)&&ends==1&&l::lifecycle.snapshot().ending&&!l::lifecycle.snapshot().failed,"Existing recorded old-death proof remains distinct from the current unrelated actor");std::fprintf(stderr,"[CoopEndingProofQA] case=reused owner=%u end=1\n",owner);
 }
 // Direct read-only boundary isolates the entire intercept, including new
 // printf arguments, from the actual42148 body's expected C0/global changes.
 for(unsigned owner=0;owner<2;++owner){Fixture f;const unsigned other=1-owner;const auto a=owner?second:first,p=other?second:first;const auto id=c::observed_identity(f.r,a);need(id.has_value(),"Read-only ending boundary retains current owner");for(auto actor:{a,p}){by(f.r,actor+0x18,0xA);by(f.r,actor+0x30,0xC);}w(f.r,a+0x20,0x8003085C);w(f.r,id->private_data+0xC0,0x8002F5F4);w(f.r,0x800F8828,a);f.x.r4=int32_t(a);f.x.r31=int32_t(0x8004217C);const auto sp=uint32_t(f.x.r29);w(f.r,sp+0x14,0x80030878);w(f.r,sp+0x1C,0x8002F5F4);w(f.r,sp+0x20,a);w(f.r,sp+0x24,id->private_data);const auto before=f.ram;const auto regs=f.x;need(!l::lifecycle.intercept(f.r,&f.x,vi,room_key)&&l::lifecycle.snapshot().ending&&before==f.ram&&!std::memcmp(&regs,&f.x,sizeof(regs)),"Ending proof is read-only across all8MiB and every guest register");}
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"exactNativeBodies\":11,\"constructorSchedulerResourceBoundaries\":\"HOST\",\"gameLaunched\":false}\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
