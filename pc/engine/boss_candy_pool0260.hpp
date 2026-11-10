#pragma once
#include "boss_native.hpp"
#include <algorithm>
#include <array>
namespace workshop::boss_candy_pool0260 {
namespace n=boss_native;
inline constexpr uint32_t storage=0x807D0000,end=0x807D5A00,stride=0x280,registry_address=0x800F8880;
static_assert(storage+36*stride==end&&end<=0x807D8000);
struct Frame {bool enabled=false;unsigned source=177,room=177,index=10,candies=0;uint32_t overlay=0;};
struct Lease {bool held=false,cleaned=false;uint32_t header=0,registry=0,table=0;unsigned slots=0;};
struct Pending {bool active=false;uint32_t header=0,registry=0,table=0,stack=0;unsigned slots=0;};
struct State {Lease lease;Pending pending;};
inline bool pointer(uint32_t at,unsigned size){return !(at&3)&&n::range(at,size);}
inline unsigned half(uint8_t* r,uint32_t at){return (n::byte(r,at)<<8)|n::byte(r,at+1);}
// Native44F78/44FA4 publish this150-slot registry for the only selected
// context0 (F828C). No guessed enumeration of dormant contexts is used.
inline bool registry(uint8_t* r,uint32_t at,uint32_t* table=nullptr){
 if(!r||!pointer(at,0x14)||half(r,at+8)!=150||half(r,at+6)>150)return false;
 const auto p=n::word(r,at+0x10);if(!pointer(p,600))return false;if(table)*table=p;return true;
}
inline int registration(uint8_t* r,uint32_t manager,uint32_t header){
 uint32_t table=0;if(!registry(r,manager,&table))return -1;int found=150;
 for(unsigned i=0;i<150;++i)if(n::word(r,table+4*i)==header){if(found!=150)return -1;found=int(i);}return found;
}
inline bool intersects(uint32_t at,uint32_t size){return at<end&&uint64_t(at)+size>storage;}
inline bool no_storage_reference(uint8_t* r,uint32_t manager){
 uint32_t table=0;if(!registry(r,manager,&table))return false;
 std::array<uint32_t,150> seen{};unsigned used=0;
 for(unsigned i=0;i<150;++i){const auto h=n::word(r,table+4*i);if(!h)continue;if(!pointer(h,12)||std::find(seen.begin(),seen.begin()+used,h)!=seen.begin()+used)return false;seen[used++]=h;
  const auto count=n::word(r,h+4),data=n::word(r,h+8);if(!count||count>4096||!pointer(data,count*stride)||intersects(data,count*stride))return false;
 }return true;
}
inline bool layout(uint8_t* r,const Lease& l){return l.held&&pointer(l.header,12)&&(l.slots==24||l.slots==36)&&n::word(r,l.header+4)==l.slots&&n::word(r,l.header+8)==storage;}
enum class Choice:int {Refused=-1,Original=0,Expanded=1};
class Guard {
 State state_{};
public:
 const State& snapshot()const{return state_;}
 // Deliberately no configure/epoch reset. Native cleanup and unregistration,
 // not a new campaign token or retry, are the only release authority.
 Choice enter(uint8_t* r,recomp_context* ctx,const Frame& f){
  if(!r||!ctx||!f.enabled||f.source!=162||f.index>=9||f.room!=(f.index?167+f.index:162)||(f.candies!=2&&f.candies!=3)||!pointer(f.overlay,0x2090)||uint32_t(ctx->r4)!=f.overlay+0x280||uint32_t(ctx->r5)!=f.overlay+0x290||uint32_t(ctx->r6)!=12||uint32_t(ctx->r31)!=f.overlay+0x24)return Choice::Original;
  if(n::word(r,0x800F828C)!=0||n::word(r,0x800F887C)!=registry_address)return Choice::Original;
  uint32_t table=0;if(!registry(r,registry_address,&table)||state_.pending.active)return Choice::Refused;
  const auto header=uint32_t(ctx->r4);const int present=registration(r,registry_address,header);
  if(present<0)return Choice::Refused;if(present!=150)return Choice::Original; // Native4667C is idempotent for registered headers.
  const auto& old=state_.lease;
  if(old.held&&(!old.cleaned||!registry(r,old.registry)||n::word(r,old.registry+0x10)!=old.table||registration(r,old.registry,old.header)!=150||!no_storage_reference(r,old.registry)))return Choice::Refused;
  if(!no_storage_reference(r,registry_address))return Choice::Refused;
  if(!pointer(uint32_t(ctx->r29),0x30))return Choice::Refused;
  state_.pending={true,header,registry_address,table,uint32_t(ctx->r29),12*f.candies};
  ctx->r5=n::base(storage);ctx->r6=state_.pending.slots;return Choice::Expanded;
 }
 bool leave(uint8_t* r,const recomp_context* ctx){
  const auto p=state_.pending;state_.pending={};if(!p.active||!r||!ctx||uint32_t(ctx->r29)!=p.stack||!registry(r,p.registry)||n::word(r,p.registry+0x10)!=p.table||registration(r,p.registry,p.header)<0||registration(r,p.registry,p.header)==150||n::word(r,p.header+4)!=p.slots||n::word(r,p.header+8)!=storage)return false;
  state_.lease={true,false,p.header,p.registry,p.table,p.slots};return true;
 }
 bool cleanup_return(uint8_t* r,const recomp_context* entry,const recomp_context* result){
  auto& l=state_.lease;if(!r||!entry||!result||!layout(r,l)||uint32_t(entry->r4)!=l.header||uint32_t(result->r2)!=1||!registry(r,l.registry)||n::word(r,l.registry+0x10)!=l.table||registration(r,l.registry,l.header)<0||registration(r,l.registry,l.header)==150)return false;
  l.cleaned=true;return true;
 }
};
} // namespace workshop::boss_candy_pool0260
