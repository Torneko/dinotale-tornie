#pragma once
#include "coop_native_collision.hpp"
#include <array>
#include <cstdio>
#include <cmath>
#include <bit>

namespace workshop::coop_lifecycle0280 {
namespace n=coop_collision;
inline constexpr unsigned respawn_delay=300,respawn_health=2;
enum class Phase {unbound,alive,death_pending,retiring,waiting};
struct Player {n::Identity root{};unsigned color=14,death_vi=0,due_vi=0,respawn_vi=0;Phase phase=Phase::unbound;};
struct State {bool enabled=false,bound=false,failed=false,ending=false;unsigned room=0,vi=0;uint64_t epoch=0;std::array<Player,2> players{};};
inline bool fatal(uint8_t* r,uint32_t a){
 if(!n::actor(r,a)||n::half(r,a+12)!=0x4001||n::byte(r,a+0x18)!=0xA)return false;
 const unsigned sub=n::byte(r,a+0x30);return sub==0xA||sub==0xB||sub==0xC||sub==0xD||sub==0x10||sub==0x11;
}
inline bool death_call(uint8_t* r,const recomp_context* c,const n::Identity& id){
 if(!r||!c||!id.address||uint32_t(c->r4)!=id.address||n::word(r,0x800F8828)!=id.address||!fatal(r,id.address))return false;
 const auto now=n::observed_identity(r,id.address);if(!now||*now!=id||!n::range(id.private_data,0x1550))return false;
 const auto ra=uint32_t(c->r31),data=id.private_data;
 if(ra==0x800301D4)return n::word(r,data+0xC0)==0x8002F5F4&&int32_t(n::word(r,data+0xD8))>=3;
 if(ra==0x80030780)return int32_t(n::word(r,id.address+0x38))<-40;
 if(ra!=0x8004217C||n::byte(r,id.address+0x30)!=0xC||n::word(r,id.address+0x20)!=0x8003085C||n::word(r,data+0xC0)!=0x8002F5F4)return false;
 const auto sp=uint32_t(c->r29);return n::range(sp,0x28)&&n::word(r,sp+0x14)==0x80030878&&n::word(r,sp+0x1C)==0x8002F5F4&&n::word(r,sp+0x20)==id.address&&n::word(r,sp+0x24)==data;
}
//48114 unschedules a standing4001 to queue0 and47F9C links it to its
// native support. A known reciprocal parent chain is living, not a death.
inline bool living_registration(uint8_t* r,uint32_t actor){
 std::array<uint32_t,16> seen{};
 for(unsigned depth=0;depth<seen.size();depth++){
  const auto id=n::observed_identity(r,actor);if(!id)return false;
  for(unsigned i=0;i<depth;i++)if(seen[i]==actor)return false;seen[depth]=actor;
  const unsigned queue=n::byte(r,actor+0x17D);if(queue==2)return true;if(queue!=0)return false;
  const auto parent=n::word(r,actor+0x180);const auto parent_id=n::observed_identity(r,parent);if(!parent_id)return false;
  const auto slot=n::half(r,actor+0x17E),count=n::half(r,parent+0x184);if(slot>=20||!count||count>20||n::word(r,parent+0x188+slot*4)!=actor)return false;
  unsigned used=0,matches=0;for(unsigned i=0;i<20;i++){const auto child=n::word(r,parent+0x188+i*4);used+=child!=0;matches+=child==actor;}if(used!=count||matches!=1)return false;
  actor=parent;
 }
 return false;
}
class Lifecycle final {
 State state_{};uint64_t serial_=0;
public:
 const State& snapshot()const{return state_;}
 void configure(bool on){state_={};state_.enabled=on;}
 void fail(const char* reason){if(!state_.failed)std::fprintf(stderr,"[CoopLife] failed reason=%s vi=%u epoch=%llu\n",reason,state_.vi,(unsigned long long)state_.epoch);state_.failed=true;}
 bool bind(uint8_t* r,uint32_t first,uint32_t second,unsigned c0,unsigned c1,unsigned room,unsigned vi){
  if(!state_.enabled||state_.bound||room!=43||c0>=14||c1>=14||first==second)return false;
  const auto a=n::observed_identity(r,first),b=n::observed_identity(r,second);
  if(!a||!b||a->kind!=0x4001||b->kind!=0x4001||!n::range(a->private_data,0x1550)||!n::range(b->private_data,0x1550))return false;
  state_.bound=true;state_.room=room;state_.vi=vi;state_.epoch=++serial_;state_.players={Player{*a,c0,0,0,0,Phase::alive},Player{*b,c1,0,0,0,Phase::alive}};return true;
 }
 bool alive(uint8_t* r,unsigned p)const {
  if(p>=2||!state_.bound||state_.players[p].phase!=Phase::alive)return false;
  const auto& id=state_.players[p].root;const auto now=n::observed_identity(r,id.address);
  return now&&*now==id&&living_registration(r,id.address)&&!fatal(r,id.address);
 }
 std::optional<unsigned> owner(uint32_t address)const{for(unsigned p=0;p<2;p++)if(state_.players[p].root.address==address&&state_.players[p].phase!=Phase::unbound)return p;return {};}
 std::optional<unsigned> anchor(uint8_t* r)const{for(unsigned p=0;p<2;p++)if(alive(r,p))return p;return {};}
 bool expected_forget(uint32_t address)const{const auto p=owner(address);return p&&state_.players[*p].phase!=Phase::alive;}
 bool waiting(unsigned p)const{return p<2&&state_.players[p].phase!=Phase::alive&&state_.players[p].phase!=Phase::unbound;}
 // Return true only when the caller must NOT execute the ordinary global
 // death body. False on the single final death deliberately keeps it native.
 bool intercept(uint8_t* r,const recomp_context* c,unsigned vi,unsigned room){
  if(!r||!c||!state_.enabled||!state_.bound||(room!=43&&room!=81)||state_.room!=room||state_.failed)return false;
  const auto p=owner(uint32_t(c->r4));if(!p||!death_call(r,c,state_.players[*p].root))return false;
  state_.vi=vi;if(state_.ending)return true;
  auto& dead=state_.players[*p];
  if(dead.phase!=Phase::alive)return true;
  dead.phase=Phase::death_pending;dead.death_vi=vi;dead.due_vi=vi>UINT32_MAX-respawn_delay?UINT32_MAX:vi+respawn_delay;
  std::fprintf(stderr,"[CoopLife] death owner=%u actor=%08X birth=%llu color=%u ra=%08X vi=%u dueVI=%u epoch=%llu nativeObservation=1\n",*p,dead.root.address,(unsigned long long)dead.root.generation,dead.color,uint32_t(c->r31),vi,dead.due_vi,(unsigned long long)state_.epoch);
  if(alive(r,1-*p))return true;
  const auto& other=state_.players[1-*p];const auto other_now=n::observed_identity(r,other.root.address);
  const bool recorded=other.death_vi&&(other.phase==Phase::death_pending||other.phase==Phase::retiring||other.phase==Phase::waiting);
  const bool native_fatal=other_now&&*other_now==other.root&&fatal(r,other.root.address);
  if(!recorded&&!native_fatal){fail("survivor_liveness_not_attested");return true;}
  // Diagnostic only: other_now already validated the CURRENT actor header.
  // A retired/reused expected root never licenses reads of its old private.
  std::fprintf(stderr,"[CoopLife] ending-proof owner=%u actor=%08X birth=%llu private=%08X color=%u ra=%08X otherOwner=%u vi=%u epoch=%llu room=%u recorded=%u nativeFatal=%u otherLifePhase=%u otherDeathVI=%u otherActor=%08X otherExpectedBirth=%llu otherExpectedPrivate=%08X otherExpectedKind=%04X otherColor=%u otherObserved=%u otherSameIdentity=%u otherObservedBirth=%llu otherObservedPrivate=%08X otherObservedKind=%04X otherNativeState=%u otherNativeSub=%u otherNativeQueue=%u ramWrites=0\n",
   *p,dead.root.address,(unsigned long long)dead.root.generation,dead.root.private_data,dead.color,uint32_t(c->r31),1-*p,vi,(unsigned long long)state_.epoch,state_.room,unsigned(recorded),unsigned(native_fatal),unsigned(other.phase),other.death_vi,other.root.address,(unsigned long long)other.root.generation,other.root.private_data,unsigned(other.root.kind),other.color,unsigned(bool(other_now)),unsigned(other_now&&*other_now==other.root),other_now?(unsigned long long)other_now->generation:0ULL,other_now?other_now->private_data:0u,other_now?unsigned(other_now->kind):0u,other_now?unsigned(n::byte(r,other_now->address+0x18)):255u,other_now?unsigned(n::byte(r,other_now->address+0x30)):255u,other_now?unsigned(n::byte(r,other_now->address+0x17D)):255u);
  state_.ending=true;std::fprintf(stderr,"[CoopLife] both-down owner=%u vi=%u epoch=%llu nativeEndingOnce=1\n",*p,vi,(unsigned long long)state_.epoch);return false;
 }
 void clock(unsigned vi,unsigned room){state_.vi=vi;if(room!=state_.room&&!state_.ending)fail("unexpected_room_change");}
 // Called only after the native Rooms coordinator proves both warm arrivals
 // with their unchanged incarnation/HP/FIFO/shared payload at an outer fence.
 bool transferred(uint8_t* r,unsigned room){
  if(!state_.bound||state_.failed||state_.ending||!((state_.room==43&&room==81)||(state_.room==81&&room==43))||!alive(r,0)||!alive(r,1))return false;
  state_.room=room;return true;
 }
 bool retired(unsigned p){if(p>=2||state_.players[p].phase!=Phase::death_pending)return false;state_.players[p].phase=Phase::retiring;return true;}
 bool freed(unsigned p){if(p>=2||state_.players[p].phase!=Phase::retiring)return false;state_.players[p].phase=Phase::waiting;return true;}
 bool due(uint8_t* r,unsigned p,unsigned vi)const{return p<2&&!state_.failed&&!state_.ending&&state_.players[p].phase==Phase::waiting&&vi>=state_.players[p].due_vi&&alive(r,1-p);}
 bool reborn(uint8_t* r,unsigned p,uint32_t address,unsigned vi){
  if(!due(r,p,vi))return false;const auto id=n::observed_identity(r,address);
  if(!id||id->kind!=0x4001||id->generation==state_.players[p].root.generation||!n::range(id->private_data,0x1550)||n::byte(r,address+0x17D)!=2||fatal(r,address))return false;
  auto& player=state_.players[p];const auto old=player.root;player.root=*id;player.phase=Phase::alive;player.respawn_vi=vi;
  std::fprintf(stderr,"[CoopLife] respawn owner=%u old=%08X oldBirth=%llu actor=%08X birth=%llu color=%u hp=2 eggs=0 vi=%u deathVI=%u epoch=%llu nativeConstruction=1\n",p,old.address,(unsigned long long)old.generation,id->address,(unsigned long long)id->generation,player.color,vi,player.death_vi,(unsigned long long)state_.epoch);return true;
 }
};
inline Lifecycle lifecycle;
}
