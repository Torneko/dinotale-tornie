#pragma once
#include "boss_checkpoint0260.hpp"
#include "boss_native.hpp"
#include <bit>
#include <limits>

namespace workshop::boss_checkpoint_actor0260 {
namespace n=boss_native;namespace cp=boss_checkpoint0260;
inline constexpr unsigned overlay_section=138;
inline constexpr uint32_t original_overlay=0x805A52C0,overlay_extent=0x1720;
inline constexpr uint32_t initializer_offset=0x1340,wake_offset=0x368,contact_offset=0x5EC;
inline constexpr uint16_t source_kind=0x4507;
inline constexpr uint32_t source_packed=uint32_t(source_kind)<<16;
inline constexpr uint32_t flags_address=0x800F845C,count_address=0x800F8458;
// The native scheduler re-enters callback6600 while the untouched actor remains
// state0/movement0. Its Rest floor may not stamp until the arrival intro ends.
inline constexpr uint64_t deferred_initialization=UINT64_MAX;
struct Frame {
 bool active=false,first_fight=false,fighting=false,rest_ready=false,victory=false;
 unsigned room=177,completed=0,awake_mask=0,vi=0;uint32_t overlay=0,player=0,record=0;
 bool support_ready=false;
};
struct Binding {
 uint32_t actor=0,private_data=0,record=0,packed=0,resource=0,metadata=0,definition=0,overlay=0,x=0,y=0;
 uint64_t birth=0;unsigned room=177,milestone=0,index=0;
};
struct Wakeup {uint32_t actor=0;uint64_t birth=0;unsigned completed=0,vi=0;};
struct Snapshot {
 bool enabled=false,invalidated=false;unsigned room=177;uint64_t next_birth=0;
 Binding binding{};std::string failure;
};
inline bool pointer(uint32_t p,size_t bytes){return !(p&3)&&n::range(p,bytes);}
inline std::optional<unsigned> milestone(const Frame& f){
 if(!f.active||f.awake_mask>15||!pointer(f.overlay,overlay_extent))return {};
 if(f.first_fight&&f.room==162&&(f.completed==0||(f.completed==1&&f.victory)))return 0;
 if(f.room==cp::rest_room&&f.completed>=3&&f.completed<10)return cp::eligible(f.completed);
 return {};
}
// Placement ownership survives the first victory and native off-screen
// teardown/recreation. A first wake remains available during this victory.
inline std::optional<unsigned> placement_milestone(const Frame& f){
 if(f.active&&f.awake_mask<=15&&pointer(f.overlay,overlay_extent)&&
    f.first_fight&&f.room==162&&f.completed<=1)return 0;
 return milestone(f);
}
inline std::optional<uint16_t> placement_kind(const Frame& f){
 if(auto m=placement_milestone(f))return uint16_t(source_kind+cp::number(*m)-1);return {};
}
struct SpawnChoice {bool owned=false,suppress=false;uint32_t packed=0;};
// The caller supplies the exact source placement address proved by its cloned
// room/header slot. Never infer ownership from a kind, position, or S0 alone.
// Native59FD4 tests4613C's null result before consuming the source placement.
inline SpawnChoice select_spawn(uint8_t* ram,const recomp_context* c,const Frame& f){
 if(!ram||!c||!f.active||f.awake_mask>15||!pointer(f.record,12)||
    uint32_t(c->r31)!=0x8005A010||uint32_t(c->r16)!=f.record||uint32_t(c->r4)!=source_packed||
    n::word(ram,f.record)!=source_packed||uint32_t(c->r5)!=n::word(ram,f.record+4)||uint32_t(c->r6)!=n::word(ram,f.record+8))return {};
 if(f.room==cp::rest_room&&f.completed<3)return {true,true,0};
 if(auto kind=placement_kind(f))return {true,false,(uint32_t(*kind)<<16)|(source_packed&65535)};
 return {};
}
class Controller {
 Snapshot state_;
 struct Init {Binding binding{};uint64_t ticket=0;uint32_t sp=0;unsigned mask=0;} init_;
 struct Wake {Binding binding{};uint64_t ticket=0;uint32_t sp=0,ra=0,count=0;unsigned vi=0,mask=0;} wake_;
 std::optional<Wakeup> receipt_;
 uint64_t delivered_birth_=0;
 void fail(const char* why){state_.invalidated=true;state_.failure=why;init_={};wake_={};receipt_.reset();}
 bool context(const Frame& f)const{return state_.enabled&&!state_.invalidated&&state_.room==f.room&&bool(placement_milestone(f));}
 static bool flags(uint8_t* ram,unsigned mask){
  if(!ram||mask>15||n::word(ram,count_address)!=unsigned(std::popcount(mask)))return false;
  for(unsigned i=0;i<4;++i)if(n::word(ram,flags_address+i*4)!=((mask>>i)&1))return false;
  return true;
 }
 static bool base_identity(uint8_t* ram,const Binding& b){
  if(!ram||!b.birth||!n::actor(ram,b.actor,uint16_t(source_kind+b.index))||n::byte(ram,b.actor+0x17D)==3||
     !pointer(b.record,12)||!pointer(b.resource,0x48)||!pointer(b.metadata,4)||!pointer(b.definition,4)||!pointer(b.overlay,overlay_extent))return false;
  return n::word(ram,b.actor+0x1D8)==b.record&&n::word(ram,b.actor+0x1DC)==b.packed&&
   n::word(ram,b.record)==0xFFFFFFFF&&n::word(ram,b.record+4)==b.x&&n::word(ram,b.record+8)==b.y&&
   n::word(ram,b.actor+0x168)==b.resource&&n::word(ram,b.actor+0x174)==b.metadata&&
   n::word(ram,b.metadata)==b.definition&&n::word(ram,b.resource+0x40)==b.overlay&&
   n::word(ram,b.resource+0x44)==b.overlay+initializer_offset;
 }
 static bool identity(uint8_t* ram,const Binding& b){
  return base_identity(ram,b)&&pointer(b.private_data,0x280)&&n::word(ram,b.actor+0x1F4)==b.private_data&&
   n::word(ram,b.private_data+0x27C)==b.index&&n::word(ram,b.actor+0x1F8)==b.overlay+0x11BC;
 }
 bool bound_context(uint8_t* ram,const Frame& f)const{
  const auto& b=state_.binding;
  // A victory changes completed before the old room/point is torn down. Its
  // transport must remain blocked throughout that lifetime, even after the
  // original wake eligibility no longer matches the current campaign phase.
  return state_.enabled&&!state_.invalidated&&f.active&&state_.room==f.room&&b.room==f.room&&
   b.overlay==f.overlay&&b.record==f.record&&identity(ram,b);
 }
 bool wake_context(uint8_t* ram,const Frame& f)const{
  const auto m=milestone(f);
  const bool first=f.first_fight&&((f.fighting&&!f.victory&&f.completed==0)||(f.victory&&!f.fighting&&f.completed==1));
  return m&&bound_context(ram,f)&&state_.binding.milestone==*m&&f.vi&&(first||(f.room==cp::rest_room&&f.rest_ready))&&
   n::actor(ram,f.player,0x4001)&&n::word(ram,0x800F8828)==f.player;
 }
public:
 void configure(bool enabled){state_={};state_.enabled=enabled;init_={};wake_={};receipt_.reset();delivered_birth_=0;}
 const Snapshot& snapshot()const{return state_;}
 void enter_room(unsigned room){
  if(!state_.enabled||state_.invalidated)return;
  if(room>255){fail("checkpoint_actor_room_invalid");return;}
  state_.room=room;state_.binding={};init_={};wake_={};receipt_.reset();
 }
 void forget(uint32_t actor){
  if(state_.binding.actor==actor){state_.binding={};wake_={};receipt_.reset();}
  if(init_.binding.actor==actor)init_={};
 }
 // Native59FD4 calls48E9C(record), which stores its original packed value in
 // actor1DC and marks record0=FFFFFFFF, BEFORE44194 invokes this initializer.
 // Only this exact owned constructor may synchronize transient Miss Warp flags.
 uint64_t init_enter(uint8_t* ram,const recomp_context* c,const Frame& f){
  auto* rdram=ram;
  if(!ram||!c||!context(f)||init_.ticket)return 0;
  const auto a=uint32_t(c->r4);const auto selected=placement_kind(f);
  if(!selected||!n::actor(ram,a,*selected)||n::word(ram,a+0x1DC)!=source_packed)return 0;
  if(state_.binding.actor){fail("checkpoint_actor_duplicate_constructor");return 0;}
  Binding b;b.actor=a;b.record=n::word(ram,a+0x1D8);b.packed=source_packed;b.resource=n::word(ram,a+0x168);
  b.metadata=n::word(ram,a+0x174);b.overlay=f.overlay;b.room=f.room;b.milestone=*placement_milestone(f);b.index=cp::number(b.milestone)-1;
  if(!pointer(b.record,12)||b.record!=f.record||!pointer(b.metadata,4)){fail("checkpoint_actor_constructor_record_invalid");return 0;}
  b.x=n::word(ram,b.record+4);b.y=n::word(ram,b.record+8);b.definition=n::word(ram,b.metadata);
  if(state_.next_birth>=deferred_initialization-1){fail("checkpoint_actor_birth_overflow");return 0;}
  b.birth=state_.next_birth+1;
  if(!base_identity(ram,b)||n::word(ram,a+0x20)!=f.overlay+initializer_offset||n::word(ram,a+0x3C)!=b.x||n::word(ram,a+0x40)!=b.y){fail("checkpoint_actor_constructor_identity_invalid");return 0;}
  // Return the wrapper's explicit defer ticket before flags, birth, or native
  // state change. The real6600 body runs once after the actual owned floor's
  // D05C stamp. Unlike suppressing allocation, this needs no placement rescan.
  if(f.room==cp::rest_room&&!f.support_ready){
   // Native45CAC keeps drawing disabled (165=1) until6600 allocates private
   // state and clears it. Native5DCB4 rejects rendering while this bit is set.
   if(n::byte(ram,a+0x18)||n::byte(ram,a+0x17D)!=2||n::byte(ram,a+0x165)!=1||n::word(ram,a+0x130)||n::word(ram,a+0x1F4)||n::word(ram,a+0x180)){fail("checkpoint_actor_deferred_state_invalid");return 0;}
   return deferred_initialization;
  }
  state_.next_birth=b.birth;init_={b,b.birth,uint32_t(c->r29),f.awake_mask};
  for(unsigned i=0;i<4;++i)MEM_W(0,n::base(flags_address+i*4))=int32_t((f.awake_mask>>i)&1);
  MEM_W(0,n::base(count_address))=int32_t(std::popcount(f.awake_mask));
  return b.birth;
 }
 bool init_leave(uint8_t* ram,const recomp_context* entry,const Frame& f,uint64_t ticket){
  if(ticket==deferred_initialization)return false;
  if(!ticket||!init_.ticket)return false;
  if(!ram||!entry||!context(f)||ticket!=init_.ticket||uint32_t(entry->r4)!=init_.binding.actor||uint32_t(entry->r29)!=init_.sp||f.awake_mask!=init_.mask||f.record!=init_.binding.record||f.overlay!=init_.binding.overlay||*placement_milestone(f)!=init_.binding.milestone){fail("checkpoint_actor_constructor_return_invalid");return false;}
  auto b=init_.binding;b.private_data=n::word(ram,b.actor+0x1F4);
  const bool awake=(f.awake_mask>>b.index)&1;const unsigned phase=awake?2:0;
  if(!identity(ram,b)||n::byte(ram,b.actor+0x18)!=1||n::byte(ram,b.actor+0x30)!=phase||n::word(ram,b.actor+0x20)!=b.overlay+(awake?0xD34:0xBA8)||!flags(ram,f.awake_mask)){fail("checkpoint_actor_initializer_postcondition_invalid");return false;}
  state_.binding=b;init_={};return true;
 }
 uint64_t wake_enter(uint8_t* ram,const recomp_context* c,const Frame& f){
  if(!c||!wake_context(ram,f)||wake_.ticket||receipt_)return 0;
  const auto& b=state_.binding;const auto ra=uint32_t(c->r31);const unsigned bit=1u<<b.index;
  if(uint32_t(c->r4)!=b.actor||delivered_birth_==b.birth||(f.awake_mask&bit)||n::byte(ram,b.actor+0x30)!=0||n::word(ram,b.actor+0x20)!=b.overlay+0xBA8||!flags(ram,f.awake_mask)||
     (ra!=b.overlay+0x6D0&&ra!=b.overlay+0x514&&ra!=b.overlay+0x564))return 0;
  wake_={b,b.birth,uint32_t(c->r29),ra,n::word(ram,count_address),f.vi,f.awake_mask};return b.birth;
 }
 bool wake_leave(uint8_t* ram,const recomp_context* entry,const Frame& f,uint64_t ticket){
  if(!ticket||!wake_.ticket)return false;
  const auto p=wake_;const unsigned expected=p.mask|(1u<<p.binding.index);
  if(!entry||!wake_context(ram,f)||ticket!=p.ticket||state_.binding.birth!=p.binding.birth||uint32_t(entry->r4)!=p.binding.actor||uint32_t(entry->r29)!=p.sp||uint32_t(entry->r31)!=p.ra||f.vi!=p.vi||f.awake_mask!=p.mask||
     !identity(ram,p.binding)||n::byte(ram,p.binding.actor+0x30)!=1||n::word(ram,p.binding.actor+0x20)!=p.binding.overlay+0xC1C||!flags(ram,expected)||n::word(ram,count_address)!=p.count+1){fail("checkpoint_actor_wakeup_postcondition_invalid");return false;}
  receipt_=Wakeup{p.binding.actor,p.binding.birth,p.binding.milestone,f.vi};delivered_birth_=p.binding.birth;wake_={};return true;
 }
 std::optional<Wakeup> take_wakeup(){auto out=receipt_;receipt_.reset();return out;}
 // The native contact routine's state0/1 branches retain waking/bounce/audio.
 // Its state2/7 branches are the only entries into free transport. The caller
 // returns V0=1 when true; this observer itself changes no native RAM/registers.
 bool suppress_warp(uint8_t* ram,const recomp_context* c,const Frame& f)const{
  if(!c||!bound_context(ram,f)||uint32_t(c->r4)!=state_.binding.actor)return false;
  const auto phase=n::byte(ram,state_.binding.actor+0x30);return phase==2||phase==7;
 }
};
} // namespace workshop::boss_checkpoint_actor0260
