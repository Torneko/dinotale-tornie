#pragma once
#include "boss_native.hpp"
#include "boss_battle0250.hpp"
#include <array>
#include <bit>
#include <limits>
#include <optional>
#include <string>

namespace workshop::boss_tree_hybrid0260 {
namespace n=boss_native;
inline constexpr unsigned rest_room=83,hybrid_count=6;
inline constexpr uint64_t suppress_spawn=std::numeric_limits<uint64_t>::max();
inline constexpr std::array<uint16_t,6> native_kinds{0x4035,0x4034,0x4033,0x4030,0x4032,0x4031};
struct Frame {
 bool active=false,in_rest=false;unsigned room=177,vi=0;
 uint32_t tree=0,tree_private=0,tree_overlay=0,fruit_overlay=0;
};
struct Token {uint32_t actor=0,resource=0,metadata=0,definition=0,origin_tree=0;uint16_t kind=0;uint64_t birth=0;};
struct Snapshot {
 bool enabled=false,invalidated=false;std::array<uint16_t,6> order{};
 unsigned consumed=0,consumed_mask=0;Token token{};std::string failure;
};
struct Birth {uint32_t actor=0;uint16_t kind=0;unsigned position=0;uint64_t birth=0;};
inline bool aligned(uint32_t p,unsigned bytes){return p%4==0&&n::range(p,bytes);}
inline bool hybrid(uint16_t kind){return kind>=0x4F00&&kind<0x4F06;}
class Controller {
 struct Scope {uint64_t ticket=0;uint32_t record=0,sp=0,tree=0,tree_private=0,caller_ra=0;unsigned slot=0,position=0;uint16_t original=0,selected=0;};
 Snapshot state_;std::array<Scope,8> scopes_{};unsigned depth_=0;uint64_t serial_=0;
 void fail(const char* why){state_.invalidated=true;state_.failure=why;}
 static uint16_t half(uint8_t* rdram,uint32_t a){return MEM_HU(0,n::base(a));}
 static void half(uint8_t* rdram,uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}
 static bool child(uint8_t* rdram,uint32_t parent,uint32_t wanted){
  if(!aligned(parent,0x1FC)||!aligned(wanted,0x1FC))return false;
  unsigned count=0,matches=0;for(unsigned i=0;i<20;++i){const auto a=n::word(rdram,parent+0x188+4*i);count+=a!=0;matches+=a==wanted;}
  return count==half(rdram,parent+0x184)&&count<=20&&matches==1&&n::word(rdram,wanted+0x180)==parent;
 }
 static bool live_fruit(uint8_t* rdram,uint32_t a,uint16_t kind){
  return rdram&&n::actor(rdram,a,kind)&&n::byte(rdram,a+0x17D)!=3&&n::byte(rdram,a+0x18)!=0xFF;
 }
 bool token_matches(uint8_t* rdram)const{
  const auto& t=state_.token;
  return t.actor&&t.birth&&hybrid(t.kind)&&live_fruit(rdram,t.actor,t.kind)&&n::word(rdram,t.actor+0x168)==t.resource&&n::word(rdram,t.actor+0x174)==t.metadata&&aligned(t.resource,0x90)&&half(rdram,t.resource+0x3C)==t.kind&&aligned(t.metadata,4)&&n::word(rdram,t.metadata)==t.definition&&aligned(t.definition,0x50);
 }
 bool tree_matches(uint8_t* rdram,const Frame& f)const{
  return rdram&&state_.enabled&&f.active&&f.in_rest&&f.room==rest_room&&aligned(f.tree_overlay,0x600)&&n::actor(rdram,f.tree,0x4708)&&n::byte(rdram,f.tree+0x17D)==2&&n::byte(rdram,f.tree+0x18)==1&&n::word(rdram,f.tree+0x20)==f.tree_overlay+0x30C&&aligned(f.tree_private,0x50)&&n::word(rdram,f.tree+0x1F4)==f.tree_private;
 }
 void prune(uint8_t* rdram){if(state_.token.actor&&!token_matches(rdram))state_.token={};}
public:
 void configure(bool enabled,uint64_t seed){
  state_={};scopes_={};depth_=0;serial_=0;state_.enabled=enabled;
  for(unsigned i=0;i<6;++i)state_.order[i]=uint16_t(0x4F00+i);
  // Independent PRNG instance: never consumes or changes the boss-plan stream.
  boss_battle0250::Random random{seed^0x4859425249445452ull};
  for(unsigned i=5;i>0;--i)std::swap(state_.order[i],state_.order[random.pick(i+1)]);
 }
 const Snapshot& snapshot()const{return state_;}
 bool ledger_valid()const{
  if(state_.consumed>6||(state_.consumed_mask&~63u))return false;unsigned all=0,prefix=0;
  for(unsigned i=0;i<6;++i){const auto k=state_.order[i];if(!hybrid(k))return false;const unsigned bit=1u<<(k-0x4F00);if(all&bit)return false;all|=bit;if(i<state_.consumed)prefix|=bit;}
  return all==63&&prefix==state_.consumed_mask&&unsigned(std::popcount(state_.consumed_mask))==state_.consumed;
 }
 std::optional<uint16_t> next()const{
  if(!state_.enabled||state_.invalidated||!ledger_valid()||state_.consumed>=6)return {};return state_.order[state_.consumed];
 }
 // Invoke AFTER the existing normal-tree allowances. This changes only the
 // original heart slot's supply, never the five normal slots or their ledger.
 void frame(uint8_t* rdram,const Frame& f){
  if(!rdram||!state_.enabled||!f.active)return;if(!ledger_valid()){fail("hybrid_tree_ledger_invalid");return;}prune(rdram);
  if(!tree_matches(rdram,f))return;
  const auto current=n::word(rdram,f.tree_private);
  if(aligned(current,0x1FC)&&n::word(rdram,current)==0x19971110&&n::word(rdram,current+0x180)==f.tree&&current!=state_.token.actor){
   fail("hybrid_tree_unproved_slot0_actor");
  }
  bool pending=false;for(unsigned i=0;i<depth_;++i)pending|=scopes_[i].slot==0;
  half(rdram,f.tree_private+0x1C,uint16_t(!state_.invalidated&&next()&&!state_.token.actor&&!pending));
 }
 // Wrap the ORIGINAL80574B44. Zero means not an owned native tree birth;
 // suppress_spawn means an owned slot0 must return nativeV0=0 without birth.
 // Positive tickets must always be paired with spawn_leave, even on failure.
 uint64_t spawn_enter(uint8_t* rdram,const recomp_context* c,const Frame& f){
  if(!c||!tree_matches(rdram,f))return 0;
  const auto sp=uint32_t(c->r29),record=uint32_t(c->r4);
  if(!aligned(sp,0x58)||record!=sp+0x34||uint32_t(c->r31)!=f.tree_overlay+0x15C||n::word(rdram,record)!=f.tree||n::word(rdram,record+0x1C)!=2)return 0;
  const auto slot=n::word(rdram,f.tree_private+0x18);if(slot>=6||half(rdram,record+4)!=native_kinds[slot])return 0;
  prune(rdram);
  bool pending=false;for(unsigned i=0;i<depth_;++i)pending|=scopes_[i].slot==0;
  if(slot==0&&(state_.invalidated||!next()||state_.token.actor||pending))return suppress_spawn;
  if(state_.invalidated)return 0;
  if(depth_>=scopes_.size()){fail("hybrid_tree_spawn_scope_overflow");return slot==0?suppress_spawn:0;}
  const auto selected=slot?native_kinds[slot]:*next();const auto ticket=++serial_;
  auto& s=scopes_[depth_++];s={ticket,record,sp,f.tree,f.tree_private,uint32_t(c->r31),slot,state_.consumed,native_kinds[slot],selected};
  if(!slot)half(rdram,record+4,selected);return ticket;
 }
 // NativeB44 calls workshop_mix_spawn before its first nativejal. Preserve
 // the exact request inside this scope, including all five normal fruit types.
 std::optional<uint16_t> mix_override(uint8_t* rdram,const recomp_context* c,uint16_t requested)const{
  if(!rdram||!c||!depth_)return {};const auto& s=scopes_[depth_-1];
  if(uint32_t(c->r17)!=s.record||uint32_t(c->r29)!=s.sp-0x38||uint32_t(c->r31)!=s.caller_ra||requested!=s.selected||!aligned(s.record,0x20)||half(rdram,s.record+4)!=s.selected||n::word(rdram,s.record)!=s.tree)return {};
  return s.selected;
 }
 std::optional<Birth> spawn_leave(uint8_t* rdram,const recomp_context* returned,uint64_t ticket){
  if(!ticket||ticket==suppress_spawn)return {};if(!rdram||!returned||!depth_||scopes_[depth_-1].ticket!=ticket){fail("hybrid_tree_spawn_return_scope");return {};}
  const auto s=scopes_[--depth_];scopes_[depth_]={};
  const bool type_intact=half(rdram,s.record+4)==s.selected;
  if(!s.slot)half(rdram,s.record+4,s.original); // Restore even when another return guard fails.
  if(uint32_t(returned->r29)!=s.sp){fail("hybrid_tree_spawn_stack_changed");return {};}
  if(!type_intact){fail("hybrid_tree_spawn_record_changed");return {};}
  const auto a=uint32_t(returned->r2);if(!a)return {}; // Allocation failure consumes nothing.
  if(!live_fruit(rdram,a,s.selected)||!n::actor(rdram,s.tree,0x4708)||n::word(rdram,s.tree+0x1F4)!=s.tree_private||!child(rdram,s.tree,a)){
   fail(s.slot?"native_tree_normal_kind_or_attachment_changed":"hybrid_tree_birth_not_attached");return {};
  }
  if(s.slot)return {};
  const auto resource=n::word(rdram,a+0x168),metadata=n::word(rdram,a+0x174);
  if(state_.token.actor||state_.consumed!=s.position||!next()||*next()!=s.selected||!aligned(resource,0x90)||half(rdram,resource+0x3C)!=s.selected||!aligned(metadata,4)||!aligned(n::word(rdram,metadata),0x50)){
   fail("hybrid_tree_birth_identity_changed");return {};
  }
  state_.token={a,resource,metadata,n::word(rdram,metadata),s.tree,s.selected,++serial_};
  return Birth{a,s.selected,s.position,state_.token.birth};
 }
 // 80574060 invokes80573F94 only for the actual native ingestion state66.
 // An unlinked/disappearing actor never advances the permutation.
 bool collect(uint8_t* rdram,const recomp_context* c,const Frame& f){
  if(!rdram||!c||!state_.enabled||state_.invalidated||!f.active||!ledger_valid()||!aligned(f.fruit_overlay,0x208)||uint32_t(c->r31)!=f.fruit_overlay+0x204||(uint32_t(c->r5)&255)!=0x66||uint32_t(c->r4)!=state_.token.actor||!token_matches(rdram)||!next()||*next()!=state_.token.kind)return false;
  state_.consumed_mask|=1u<<(state_.token.kind-0x4F00);++state_.consumed;state_.token={};return true;
 }
 // The same type may be retried after actual disposal or address reuse, but
 // tree teardown alone does not discard a live independently carried fruit.
 void forget(uint32_t actor){if(actor&&state_.token.actor==actor)state_.token={};}
};
inline Controller controller;
inline void configure(bool enabled,uint64_t seed){controller.configure(enabled,seed);}
inline const Snapshot& snapshot(){return controller.snapshot();}
inline void frame(uint8_t* r,const Frame& f){controller.frame(r,f);}
inline uint64_t spawn_enter(uint8_t* r,const recomp_context* c,const Frame& f){return controller.spawn_enter(r,c,f);}
inline std::optional<uint16_t> mix_override(uint8_t* r,const recomp_context* c,uint16_t id){return controller.mix_override(r,c,id);}
inline std::optional<Birth> spawn_leave(uint8_t* r,const recomp_context* c,uint64_t t){return controller.spawn_leave(r,c,t);}
inline bool collect(uint8_t* r,const recomp_context* c,const Frame& f){return controller.collect(r,c,f);}
inline void forget(uint32_t a){controller.forget(a);}
}
