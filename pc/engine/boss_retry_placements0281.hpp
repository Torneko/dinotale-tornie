#pragma once
#include "boss_native.hpp"
#include "boss_rooms_data0250.hpp"
#include <array>

namespace workshop::boss_retry_placements0281 {
namespace n=boss_native;namespace core=boss_battle0250;namespace rooms=boss_rooms_data0250;
// Defeated bosses can leave their original native placements consumed:
// ordinary4072/4599/40A3 detach their respawn link after proven defeat too.
// Cached6DA0C skips6D85C. A genuine checkpoint rewind therefore rearms the
// captured declared boss words of every replayed family, never a whole room.
// No actor, HP, score, pose, resource, item or constructor is created here.
inline constexpr uint16_t kind=0x41C6;
inline bool capture_kind(const core::Encounter& e,uint16_t k){
 switch(e.family){
 case core::Family::Candy:return k==0x455A||(e.genie_companion&&k==0x4072);
 case core::Family::RedCandy:return k==0x455A;
 case core::Family::Invisible:return k==0x4599;
 case core::Family::Genie:return k==0x4072;
 case core::Family::Lips:return k==0x40A3||k==0x455A||k==0x4599;
 case core::Family::Fish:return k==0x4511||k==0x4072||k==0x4599;
 case core::Family::Monsters:return k==kind;
 case core::Family::Blargg:return k==0x41EC;
 }return false;
}
inline bool ordinary_shape(const core::Encounter& e){
 if(e.variant>2||e.terrain>2||!e.count||e.count>4)return false;
 auto member=[&](unsigned m,uint16_t k,unsigned hits,bool red=false){const auto& b=e.members[m];return b.kind==k&&b.hits==hits&&std::isfinite(b.scale)&&b.scale>0&&b.scale<=1&&b.red==red&&!b.purple&&!b.green;};
 switch(e.family){
 case core::Family::Candy:{
  const unsigned candies=e.variant?e.variant+1:1;
  if((e.variant&&e.genie_companion)||e.count!=candies+unsigned(e.genie_companion))return false;
  for(unsigned m=0;m<candies;++m)if(!member(m,0x455A,12))return false;
  return !e.genie_companion||(member(1,0x4072,2)&&e.members[1].scale==1.f);
 }
 case core::Family::RedCandy:return !e.variant&&!e.genie_companion&&e.count==1&&member(0,0x455A,12,true)&&e.members[0].scale==1.f;
 case core::Family::Invisible:case core::Family::Genie:{
  if(e.genie_companion||e.count!=(1u<<e.variant))return false;
  const auto k=uint16_t(e.family==core::Family::Invisible?0x4599:0x4072);
  for(unsigned m=0;m<e.count;++m)if(!member(m,k,3-e.variant)||e.members[m].scale!=1.f)return false;
  return true;
 }
 case core::Family::Lips:
  if(e.genie_companion||e.count!=(e.variant?2u:1u)||!member(0,0x40A3,3)||e.members[0].scale!=1.f)return false;
  return !e.variant||(e.variant==1?member(1,0x455A,12)&&e.members[1].scale<1.f:member(1,0x4599,3)&&e.members[1].scale==1.f);
 default:return false;
 }
}
struct Cache {
 uint32_t registry_table=0,entry=0,wrapper=0,header=0,table=0;unsigned count=0,native_source=177;
 bool operator==(const Cache&)const=default;
};
struct Placement {uint32_t record=0,packed=0,x=0,y=0;};
struct Room {
 bool captured=false;unsigned source=177,index=10,pending_epoch=0,restored_epoch=0;
 Cache cache{};unsigned count=0;std::array<Placement,4> placements{};
};
struct Fence {unsigned room=177,index=10,epoch=0,vi=0;core::Phase phase=core::Phase::Idle;};
struct Receipt {unsigned room=177,index=10,epoch=0,vi=0,count=0;uint32_t cache=0,table=0;};
inline bool aligned(uint32_t a,size_t size){return !(a&3)&&n::range(a,size);}
// Mirrors6D6F8/57A28/6D6E4, with bounded pointer validation before a call
// to the actual native getter. The11-slot registry is not actor teardown.
inline bool lookup(uint8_t* r,unsigned source,Cache& out,bool* absent=nullptr){
 if(absent)*absent=false;auto* rdram=r;
 if(!r||source>=177||MEM_HU(8,n::base(0x800F8440))!=11)return false;
 const auto table=n::word(r,0x800F8450);if(!aligned(table,44))return false;
 unsigned found=0;Cache value{};
 for(unsigned i=0;i<11;++i){const auto entry=n::word(r,table+i*4);if(!entry)continue;if(!aligned(entry,0xA94))return false;
  if(n::byte(r,entry+1)!=source)continue;if(++found!=1)return false;
  const auto wrapper=n::word(r,entry+0x60);if(!aligned(wrapper,4))return false;
  const auto header=n::word(r,wrapper);if(!aligned(header,8))return false;
  const auto count=MEM_HU(0,n::base(header));const auto objects=n::word(r,header+4);
  if(!count||count>149||!aligned(objects,0x960))return false;
  value={table,entry,wrapper,header,objects,count,source};
 }
 if(found!=1){if(absent&&found==0)*absent=true;return false;}out=value;return true;
}
inline bool owned(const rooms::Arena& a,const core::Encounter& e,unsigned index){
 if(index>=10||a.source!=rooms::source_for_encounter(index)||a.native_init!=0x01000000||e.number!=index+1||!e.count||e.count>4||a.boss_indices.size()!=e.count||a.objects.empty()||a.objects.size()%12||a.objects.size()/12>149)return false;
 if((index==0&&(e.family!=core::Family::Candy||e.variant||e.count!=1||e.genie_companion))||(index==9&&e.family!=core::Family::RedCandy)||(index>0&&index<9&&e.family==core::Family::RedCandy))return false;
 if(e.family==core::Family::Monsters){if(a.logical_source!=151||e.count!=2)return false;}
 else if(e.family==core::Family::Fish){if(a.logical_source!=97||!core::valid_fish(e))return false;}
 else if(e.family==core::Family::Blargg){
  if(a.logical_source!=19||e.variant!=0||e.terrain>2||e.count!=2||e.genie_companion||e.purple_mask||!e.no_flowers||!e.no_egg_blocks)return false;
  for(unsigned m=0;m<e.count;++m){const auto& b=e.members[m];if(b.kind!=0x41EC||b.hits!=3||b.scale!=1.f||b.red||b.purple||b.green)return false;}
 }
 else if(a.logical_source!=core::room(e.family)||!ordinary_shape(e))return false;
 for(unsigned m=0;m<e.count;++m){const auto at=size_t(a.boss_indices[m])*12;const auto k=e.members[m].kind;
  if(!capture_kind(e,k)||at+12>a.objects.size()||u32(a.objects,at)!=(uint32_t(k)<<16|m))return false;
  for(unsigned other=0;other<m;++other)if(a.boss_indices[other]==a.boss_indices[m])return false;
 }
 return true;
}
inline bool native_table(uint8_t* r,const Cache& cache,const rooms::Arena& a){
 if(cache.count!=a.objects.size()/12||!aligned(cache.table,0x960))return false;
 for(size_t p=0;p<a.objects.size();p+=12){const auto at=cache.table+uint32_t(p/12)*16,packed=n::word(r,at);
  if((packed!=0xFFFFFFFF&&packed!=u32(a.objects,p))||n::word(r,at+4)!=u32(a.objects,p+4)||n::word(r,at+8)!=u32(a.objects,p+8))return false;
 }return true;
}
inline bool retired_links(uint8_t* r,const Room& room){
 // Include pending/parented actors outside the main manager too. Native44658
 // writes17D=3 and stateFF at447A8..447B0; a retained live link is rejected.
 for(uint32_t a=0x80000400;a<=0x80800000-0x1FC;a+=4){
  if(n::word(r,a)!=0x19971110||n::byte(r,a+6)==3||n::byte(r,a+0x17D)==3)continue;
  const auto record=n::word(r,a+0x1D8);for(unsigned m=0;m<room.count;++m)if(record==room.placements[m].record)return false;
 }return true;
}
class Guard {
 std::array<Room,10> rooms_{};unsigned epoch_=0;const char* failure_="none";
 bool reject(const char* why){failure_=why;return false;}
public:
 void reset(){rooms_={};epoch_=0;failure_="none";}
 const auto& snapshot()const{return rooms_;}const char* failure()const{return failure_;}
 bool pending(unsigned index)const{return index<10&&rooms_[index].pending_epoch!=0;}
 bool capture(uint8_t* r,const recomp_context* c,const rooms::Arena& a,const core::Encounter& e,unsigned index,unsigned epoch){
  if(!r||!c||epoch!=epoch_||!owned(a,e,index)||uint32_t(c->r31)!=0x8005A050)return reject("capture_scope");
  const auto actor=uint32_t(c->r4),record=uint32_t(c->r5);
  if(!aligned(record,16)||!capture_kind(e,uint16_t(n::word(r,record)>>16))||!n::actor(r,actor,uint16_t(n::word(r,record)>>16))||!aligned(n::word(r,actor+0x168),0x50))return reject("capture_actor");
  Cache cache;bool canonical_absent=false;
  if(!lookup(r,a.source,cache,&canonical_absent)||!native_table(r,cache,a)){
   // The initial normal launch requests native52; only the HOST presentation
   // aliases that scene to firstCandy162.6DA0C publishes raw52 at6DAD4 before
   // any5A048->48E9C birth. It is not an absent/unfinished cache: attest the
   // unique raw52 table, only for the exact first encounter at epoch0.
   if(index!=0||epoch!=0||a.source!=162||!canonical_absent||!lookup(r,52,cache)||!native_table(r,cache,a))return reject("capture_cache");
  }
  bool matched=false;for(unsigned m=0;m<e.count;++m){const auto offset=size_t(a.boss_indices[m])*12;
   if(record==cache.table+uint32_t(a.boss_indices[m])*16&&n::word(r,record)==u32(a.objects,offset))matched=true;
  }
  // Native45CAC/45D54..6C replaces bits2..7 of actor byteE with
  // resource[2]<<2 for every supported4xxx kind, before48E9C. The ROM
  // record stays pristine. Keep its kind and all10 low role bits exact.
  const auto packed=n::word(r,record),resource=n::word(r,actor+0x168);
  const auto actor_packed=(packed&0xFFFF03FFu)|((uint32_t(n::byte(r,resource+2))<<10)&0xFC00u);
  if(!matched||n::word(r,actor+12)!=actor_packed)return reject("capture_placement");
  auto& saved=rooms_[index];if(saved.pending_epoch)return reject("capture_before_replay_restore");
  if(saved.captured&&saved.cache!=cache)return reject("capture_cache_changed");
  saved.captured=true;saved.source=a.source;saved.index=index;saved.cache=cache;saved.count=e.count;
  for(unsigned m=0;m<e.count;++m){const auto p=size_t(a.boss_indices[m])*12;saved.placements[m]={cache.table+uint32_t(a.boss_indices[m])*16,u32(a.objects,p),u32(a.objects,p+4),u32(a.objects,p+8)};}
  return true;
 }
 // Called only after Session::retry_checkpoint succeeds; the prior index is
 // captured before that transition. No guest write occurs at death/cleanup.
 bool arm(unsigned epoch,unsigned checkpoint,unsigned prior_index){
  if(epoch_==UINT32_MAX||epoch!=epoch_+1||prior_index>=10||checkpoint>prior_index||!boss_checkpoint0260::valid(checkpoint))return reject("retry_epoch");
  // A second death may occur earlier during replay. Later cached arenas
  // already owed by that rewind still need restoration when reached again.
  // Advancing the awakened checkpoint alone banks those earlier rows.
  epoch_=epoch;for(unsigned i=0;i<10;++i)rooms_[i].pending_epoch=rooms_[i].captured&&i>=checkpoint&&(i<=prior_index||rooms_[i].pending_epoch)?epoch:0;return true;
 }
 template<class Dispatch> bool restore(uint8_t* r,const recomp_context* c,const rooms::Arena& a,const core::Encounter& e,const Fence& f,Receipt& receipt,Dispatch&& dispatch){
  receipt={};if(!pending(f.index))return true;auto& saved=rooms_[f.index];
  if(!r||!c||!owned(a,e,f.index)||f.epoch!=epoch_||saved.pending_epoch!=f.epoch||saved.source!=f.room||a.source!=f.room||saved.count!=e.count||!f.vi||
     (f.phase!=core::Phase::ToFight&&f.phase!=core::Phase::RetryToFight)||uint32_t(c->r31)!=0x8006DDFC||(uint32_t(c->r5)&255)!=f.room||!aligned(uint32_t(c->r4),0x128))return reject("restore_fence");
  const auto stack=uint32_t(c->r29);if(stack<0x80000800||!aligned(stack-0x200,0x220))return reject("restore_stack");
  const auto key=saved.cache.native_source;
  const bool bootstrap=key==52&&f.index==0&&f.room==162;
  if(key!=f.room&&!bootstrap)return reject("restore_cache_key");
  Cache cache;if(!lookup(r,key,cache)||cache!=saved.cache||!native_table(r,cache,a)||!retired_links(r,saved))return reject("restore_cache_or_live_actor");
  recomp_context work=*c;work.r29=n::base(stack-0x100);work.r4=key;dispatch(0x8006D6F8,r,&work);
  if(uint32_t(work.r2)!=cache.entry)return reject("native_cache_getter");
  // Global preflight again after the native read, before the first setter.
  Cache again;if(!lookup(r,key,again)||again!=cache||!native_table(r,cache,a)||!retired_links(r,saved))return reject("restore_preflight");
  for(unsigned m=0;m<e.count;++m){const auto& p=saved.placements[m];const auto at=size_t(a.boss_indices[m])*12;
   if(p.record!=cache.table+uint32_t(a.boss_indices[m])*16||p.packed!=u32(a.objects,at)||p.x!=u32(a.objects,at+4)||p.y!=u32(a.objects,at+8))return reject("restore_pristine_identity");
  }
  if(bootstrap){
   // CP0 routes explicitly to162, never to the bootstrap raw52 alias. Only
   // a proved cache MISS may hand this obsolete capture to the real cold
   // 6DA0C/6D85C load. No placement setter runs; birth in that new canonical
   // cache must capture again. Warm162 and every later death stay strict.
   bool absent=false;Cache canonical;
   if(lookup(r,f.room,canonical,&absent)||!absent)return reject("restore_bootstrap_not_cold");
   saved={};return true;
  }
  unsigned count=0;for(unsigned m=0;m<saved.count;++m){const auto& p=saved.placements[m];if(n::word(r,p.record)==0xFFFFFFFF){work=*c;work.r29=n::base(stack-0x100);work.r4=n::base(p.record);work.r5=n::base(p.packed);dispatch(0x8006D34C,r,&work);++count;}}
  for(unsigned m=0;m<saved.count;++m){const auto& p=saved.placements[m];if(n::word(r,p.record)!=p.packed||n::word(r,p.record+4)!=p.x||n::word(r,p.record+8)!=p.y)return reject("native_placement_readback");}
  if(!native_table(r,cache,a))return reject("native_table_readback");
  saved.pending_epoch=0;saved.restored_epoch=f.epoch;receipt={f.room,f.index,f.epoch,f.vi,count,cache.entry,cache.table};return true;
 }
};
} // namespace workshop::boss_retry_placements0281
