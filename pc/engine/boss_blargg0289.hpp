#pragma once
#include "boss_native.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace workshop::boss_blargg0289 {
namespace n=boss_native;
inline constexpr uint16_t kind=0x41EC;
inline constexpr unsigned logical_room=19,member_count=2,required_hits=3;
inline constexpr uint32_t original_overlay=0x804F4290;
struct Config {
 std::string token;
 unsigned encounter=10,physical_room=177,retry_epoch=0;
 bool diagnostic=true;
};
struct Frame {
 unsigned room=177,logical=177,encounter=10,retry_epoch=0,vi=0;
 uint32_t overlay=0,player=0;
 bool combat_active=false,quiescent=false;
};
struct Identity {
 uint32_t actor=0,private_data=0,resource=0;
 uint64_t birth=0;
 uint16_t type=0;
 bool operator==(const Identity&)const=default;
};
struct Member {
 Identity identity{};
 uint64_t variant_birth=0;
 uint32_t record=0,packed=0,overlay=0;
 unsigned initial_vi=0,hits=0,last_hit_vi=0,retraction_vi=0,retirement_vi=0;
 bool initialized=false,retracted=false,retired=false,receipt_sent=false;
};
struct Receipt {
 std::string token;
 unsigned encounter=10,member=2,room=177,retry_epoch=0,vi=0,hits=0;
 Identity identity{};
 uint64_t variant_birth=0;
 bool initialized=false,active=false,causal_hit=false,terminal=false,diagnostic=true;
};
struct Snapshot {
 bool enabled=false,invalidated=false;
 Config config{};
 std::array<Member,member_count> members{};
 std::string failure;
};
// An independent incarnation registry is fed ONLY by the existing, real45CAC
// entry hook. It is not coupled to coop, actor positions, timers or address age.
// The HOST hit ledger adds a new three-hit mechanic to vanilla41EC, which has
// no native HP counter. Its accepted native hit/retreat states are never forged.
class Observer final {
 struct Pending {
  bool active=false,egg_returned=false,hit_entered=false,setter_called=false,setter_returned=false;
  unsigned member=2,vi=0;
  uint32_t stack=0;
  Identity egg{};
 } pending_{};
 struct Init {bool active=false;unsigned member=2,vi=0;uint32_t stack=0;} init_{};
 struct Retreat {bool active=false,setter_called=false,setter_returned=false;unsigned member=2,vi=0;uint32_t stack=0;} retreat_{};
 Snapshot state_{};
 std::map<uint32_t,uint64_t> births_;
 uint64_t next_birth_=0;
 std::array<Identity,member_count*required_hits> used_{};
 unsigned used_count_=0;
 std::vector<Receipt> receipts_;
 static bool aligned(uint32_t p,size_t bytes){return !(p&3)&&n::range(p,bytes);}
 static bool finite(float v){return std::isfinite(v)&&std::abs(v)<=1000000.f;}
 static float scalar(uint8_t* r,uint32_t p){return std::bit_cast<float>(n::word(r,p));}
 void fail(const char* reason){state_.invalidated=true;state_.failure=reason;pending_={};init_={};retreat_={};}
 bool frame(const Frame& f)const {
  return state_.enabled&&!state_.invalidated&&f.room==state_.config.physical_room&&f.logical==logical_room&&f.encounter==state_.config.encounter&&f.retry_epoch==state_.config.retry_epoch&&f.vi&&aligned(f.overlay,0x208C0);
 }
 uint32_t at(const Frame& f,uint32_t original)const{return f.overlay+(original-original_overlay);}
 std::optional<Identity> identity(uint8_t* r,uint32_t a,uint16_t type)const {
  const auto birth=births_.find(a);
  if(!r||birth==births_.end()||!birth->second||!aligned(a,0x1FC)||!n::actor(r,a,type))return {};
  return Identity{a,n::word(r,a+0x1F4),n::word(r,a+0x168),birth->second,type};
 }
 std::optional<unsigned> owner(uint32_t a)const {
  for(unsigned i=0;i<member_count;++i)if(state_.members[i].identity.actor==a)return i;
  return {};
 }
 bool resource(uint8_t* r,const Identity& id,const Frame& f)const {
  return aligned(id.resource,0x48)&&(n::word(r,id.resource+0x3C)>>16)==kind&&n::word(r,id.resource+0x40)==f.overlay&&n::word(r,id.resource+0x44)==at(f,0x804F42B8);
 }
 bool live(uint8_t* r,unsigned member,const Frame& f)const {
  if(member>=member_count)return false;
  const auto& m=state_.members[member];const auto now=identity(r,m.identity.actor,kind);
  return m.initialized&&!m.retired&&now&&*now==m.identity&&aligned(m.identity.private_data,0xB0)&&m.overlay==f.overlay&&resource(r,m.identity,f)&&n::word(r,m.identity.actor+0x2C)==0x80045BD8&&n::word(r,m.identity.actor+0x1F8)==at(f,0x804F43B8)&&n::byte(r,m.identity.actor+0x17D)==2&&n::word(r,m.identity.actor+0x1D8)==m.record&&n::word(r,m.identity.actor+0x1DC)==m.packed&&n::word(r,m.record)==0xFFFFFFFFu;
 }
 bool state(uint8_t* r,unsigned member,const Frame& f,unsigned major,unsigned sub,uint32_t callback)const {
  const auto a=state_.members[member].identity.actor;
  return live(r,member,f)&&n::byte(r,a+0x18)==major&&n::byte(r,a+0x30)==sub&&n::word(r,a+0x20)==at(f,callback);
 }
 bool native_egg(uint8_t* r,const Identity& egg)const {
  return aligned(egg.actor,0x27C)&&egg.private_data==egg.actor+0x24C&&aligned(egg.private_data,0x30)&&n::byte(r,egg.actor+0x1F2)==1&&aligned(egg.resource,0x48)&&(n::word(r,egg.resource+0x3C)>>16)==0x4002&&n::word(r,egg.resource+0x40)==0&&n::word(r,egg.resource+0x44)==0x8003B0D0;
 }
 bool used(const Identity& egg)const {for(unsigned i=0;i<used_count_;++i)if(used_[i].actor==egg.actor&&used_[i].birth==egg.birth)return true;return false;}
 bool pending_stack(uint8_t* r,const recomp_context* c,const Frame& f,uint32_t ra)const {
  if(!pending_.active||f.vi<pending_.vi||uint32_t(c->r29)!=pending_.stack||uint32_t(c->r31)!=at(f,ra)||!aligned(pending_.stack,0xB8))return false;
  const auto& m=state_.members[pending_.member];
  return live(r,pending_.member,f)&&n::word(r,pending_.stack+0xA8)==m.identity.actor&&n::word(r,pending_.stack+0xAC)==pending_.egg.actor;
 }
 static bool retired_actor(uint8_t* r,const Identity& id){return n::actor(r,id.actor,id.type)&&n::word(r,id.actor+0x1F4)==id.private_data&&n::word(r,id.actor+0x168)==id.resource&&n::byte(r,id.actor+0x18)==0xFF&&n::byte(r,id.actor+0x17D)==3;}
 bool egg_returned(uint8_t* r)const {const auto id=identity(r,pending_.egg.actor,0x4002);return id&&*id==pending_.egg&&native_egg(r,*id)&&retired_actor(r,pending_.egg);}
public:
 bool configure(Config config){
  *this=Observer{};
  if(config.token.empty()||config.token.size()>128||config.encounter<1||config.encounter>8||config.physical_room!=167+config.encounter)return false;
  for(unsigned char c:config.token)if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'))return false;
  state_.enabled=true;state_.config=std::move(config);return true;
 }
 void disable(){*this=Observer{};}
 const Snapshot& snapshot()const{return state_;}
 // Identity-only predicate for the scoped variants bridge. Native461C4/468A8
 // sets actor2C=45BD8; native441C separately sets PRIVATE cleanup in actor1F8.
 // Callers still supply the actual current campaign/source admission.
 bool initialized_member(uint8_t* r,unsigned member)const {
  if(!r||member>=member_count)return false;
  const auto& m=state_.members[member];const auto& cfg=state_.config;
  const Frame f{cfg.physical_room,logical_room,cfg.encounter,cfg.retry_epoch,m.initial_vi,m.overlay,0,false,false};
  return frame(f)&&live(r,member,f);
 }
 uint64_t birth(uint32_t actor)const {const auto p=births_.find(actor);return p==births_.end()?0:p->second;}
 // Call before the ORIGINAL45CAC, even when the actor still has old pool data.
 void created(uint32_t actor,const Frame& f){
  if(!frame(f)||!aligned(actor,0x1FC))return;
  if(const auto p=owner(actor);p&&!state_.members[*p].retired){fail("blargg_member_reused_before_retirement");return;}
  if((births_.size()>=1024&&!births_.contains(actor))||next_birth_==UINT64_MAX){fail("blargg_birth_capacity");return;}
  births_[actor]=++next_birth_;
 }
 void forget(uint32_t actor){
  if(const auto p=owner(actor);p&&!state_.members[*p].retired&&state_.enabled){fail("blargg_member_disposed_before_terminal");}
  births_.erase(actor);
 }
 // Bind at the real41EC constructor entry, after48E9C has consumed the exact
 // source placement. variant_birth is the existing campaign member generation;
 // native birth is independently observed by created(), not inferred from it.
 bool bind(uint8_t* r,unsigned member,uint32_t actor,uint64_t variant_birth,const Frame& f){
  if(!frame(f)||member>=member_count||!variant_birth||state_.members[member].identity.actor)return false;
  const auto id=identity(r,actor,kind);if(!id||!resource(r,*id,f))return false;
  for(unsigned i=0;i<member_count;++i)if(state_.members[i].identity.actor==actor)return false;
  const auto record=n::word(r,actor+0x1D8),packed=n::word(r,actor+0x1DC);
  if(!aligned(record,16)||packed!=(uint32_t(kind)<<16|member)||n::word(r,record)!=0xFFFFFFFFu)return false;
  auto& m=state_.members[member];m.identity=*id;m.variant_birth=variant_birth;m.record=record;m.packed=packed;m.overlay=f.overlay;return true;
 }
 // Trace before the named ORIGINAL function; no RAM/register writes occur.
 void enter(uint8_t* r,const recomp_context* c,std::string_view name,const Frame& f){
  if(!r||!c||!frame(f))return;
  const auto actor=uint32_t(c->r4);const auto p=owner(actor);
  if(name=="func_804F42B8"&&p){
   auto& m=state_.members[*p];const auto id=identity(r,actor,kind);
   if(m.initialized||init_.active||!id||*id!=m.identity){fail("blargg_initializer_identity");return;}
   init_={true,*p,f.vi,uint32_t(c->r29)};return;
  }
  if(name=="func_800447C8"&&uint32_t(c->r31)==at(f,0x804F4B8C)){
   const auto stack=uint32_t(c->r29);if(!aligned(stack,0xB8))return;
   const auto root=n::word(r,stack+0xA8);const auto member=owner(root);const auto egg=identity(r,actor,0x4002);
   if(!member||!egg||!native_egg(r,*egg)||f.vi<state_.members[*member].last_hit_vi||n::word(r,stack+0xAC)!=actor||n::byte(r,actor+0x18)!=0x68||n::byte(r,actor+0x17D)!=2||used(*egg)||!state(r,*member,f,1,5,0x804F5928)||state_.members[*member].hits>=required_hits)return;
   if(pending_.active){fail("blargg_overlapping_native_hit");return;}
   pending_={true,false,false,false,false,*member,f.vi,stack,*egg};return;
  }
  if(name=="func_804F57E4"&&pending_stack(r,c,f,0x804F4B94)&&actor==state_.members[pending_.member].identity.actor&&pending_.egg_returned&&egg_returned(r)&&state(r,pending_.member,f,1,5,0x804F5928)){
   pending_.hit_entered=true;return;
  }
  if(name=="func_804F571C"&&p&&state_.members[*p].hits==required_hits&&state(r,*p,f,1,7,0x804F571C)){
   if(retreat_.active){fail("blargg_overlapping_retraction");return;}
   retreat_={true,false,false,*p,f.vi,uint32_t(c->r29)};return;
  }
  if(name!="func_804F5CE8")return;
  if(pending_.active&&pending_.hit_entered&&f.vi>=pending_.vi&&actor==state_.members[pending_.member].identity.actor&&uint32_t(c->r29)==pending_.stack-0x18&&uint32_t(c->r31)==at(f,0x804F5818+8)&&uint32_t(c->r5)==1&&uint32_t(c->r6)==2&&state(r,pending_.member,f,1,5,0x804F5928)&&egg_returned(r))pending_.setter_called=true;
  if(retreat_.active&&f.vi>=retreat_.vi&&actor==state_.members[retreat_.member].identity.actor&&uint32_t(c->r29)==retreat_.stack-0x28&&uint32_t(c->r31)==at(f,0x804F57C0+8)&&uint32_t(c->r5)==1&&uint32_t(c->r6)==1&&state(r,retreat_.member,f,1,7,0x804F571C)){
   const auto priv=state_.members[retreat_.member].identity.private_data;const float y=scalar(r,actor+0x40),base=scalar(r,priv+0x3C);
   // Actual571C c.lt.s + bc1fl: setter occurs for Y<base, not Y>=base.
   if(finite(y)&&finite(base)&&y<base)retreat_.setter_called=true;
  }
 }
 // Trace immediately after the ORIGINAL returns, using its saved entry context.
 // VI may advance while a native constructor/call is on the stack: identity,
 // epoch, exact caller/stack and nested setter receipts authenticate it; a
 // wall-clock equality must not discard a genuine paused/slow native call.
 void leave(uint8_t* r,const recomp_context* entry,std::string_view name,const Frame& f){
  if(!r||!entry||!frame(f))return;
  const auto actor=uint32_t(entry->r4);
  if(name=="func_804F42B8"&&init_.active&&actor==state_.members[init_.member].identity.actor){
   const auto ticket=init_;init_={};auto& m=state_.members[ticket.member];const auto id=identity(r,actor,kind);
   if(f.vi<ticket.vi||ticket.stack!=uint32_t(entry->r29)||!id||id->birth!=m.identity.birth||id->resource!=m.identity.resource||!aligned(id->private_data,0xB0)){fail("blargg_initializer_return");return;}
   m.identity=*id;m.initialized=true;m.initial_vi=f.vi;
   if(!state(r,ticket.member,f,1,1,0x804F5384)){fail("blargg_initializer_state");}return;
  }
  if(name=="func_800447C8"&&pending_stack(r,entry,f,0x804F4B8C)&&actor==pending_.egg.actor){pending_.egg_returned=egg_returned(r);return;}
  if(name=="func_804F5CE8"){
   if(pending_.active&&pending_.setter_called&&f.vi>=pending_.vi&&uint32_t(entry->r29)==pending_.stack-0x18&&uint32_t(entry->r31)==at(f,0x804F5820)&&actor==state_.members[pending_.member].identity.actor&&uint32_t(entry->r5)==1&&uint32_t(entry->r6)==2&&state(r,pending_.member,f,1,2,0x804F5830))pending_.setter_returned=true;
   if(retreat_.active&&retreat_.setter_called&&f.vi>=retreat_.vi&&uint32_t(entry->r29)==retreat_.stack-0x28&&uint32_t(entry->r31)==at(f,0x804F57C8)&&actor==state_.members[retreat_.member].identity.actor&&uint32_t(entry->r5)==1&&uint32_t(entry->r6)==1&&state(r,retreat_.member,f,1,1,0x804F5384))retreat_.setter_returned=true;
   return;
  }
  if(name=="func_804F57E4"&&pending_.active&&actor==state_.members[pending_.member].identity.actor){
   const auto ticket=pending_;pending_={};const auto& m=state_.members[ticket.member];const auto priv=m.identity.private_data;
   if(f.vi<ticket.vi||ticket.stack!=uint32_t(entry->r29)||uint32_t(entry->r31)!=at(f,0x804F4B94)||!ticket.egg_returned||!ticket.hit_entered||!ticket.setter_called||!ticket.setter_returned||!state(r,ticket.member,f,1,2,0x804F5830)||n::word(r,priv+0x10)||n::word(r,priv)!=f.overlay+0x14250||n::word(r,priv+4)!=f.overlay+0x14250||used(ticket.egg))return;
   if(used_count_>=used_.size()){fail("blargg_hit_capacity");return;}
   used_[used_count_++]=ticket.egg;auto& target=state_.members[ticket.member];++target.hits;target.last_hit_vi=f.vi;return;
  }
  if(name=="func_804F46E4"&&pending_.active&&uint32_t(entry->r29)==pending_.stack+0xA8)pending_={};
  if(name=="func_804F571C"&&retreat_.active&&actor==state_.members[retreat_.member].identity.actor){
   const auto ticket=retreat_;retreat_={};auto& m=state_.members[ticket.member];const float y=scalar(r,actor+0x40),base=scalar(r,m.identity.private_data+0x3C);
   if(f.vi>=ticket.vi&&ticket.stack==uint32_t(entry->r29)&&ticket.setter_called&&ticket.setter_returned&&state(r,ticket.member,f,1,1,0x804F5384)&&finite(y)&&finite(base)&&y==base&&n::word(r,actor+0xC8)==0&&n::word(r,m.identity.private_data+0x34)==300){m.retracted=true;m.retraction_vi=f.vi;}
  }
 }
 bool ready_to_retire(uint8_t* r,unsigned member,const Frame& f)const {
  if(!r||!frame(f)||member>=member_count||!f.combat_active||!f.quiescent||pending_.active||retreat_.active||init_.active)return false;
  const auto& m=state_.members[member];const auto player=f.player;
  return m.hits==required_hits&&m.retracted&&!m.retired&&f.vi>=m.retraction_vi&&state(r,member,f,1,1,0x804F5384)&&n::actor(r,player,0x4001)&&n::word(r,0x800F8828)==player&&n::byte(r,player+0x18)!=0xA;
 }
 // Only native447C8 performs retirement. The caller supplies the real outer
 // completed-frame/quiescence proof, never a collider or an arbitrary timer.
 template<class Dispatch> bool retire(uint8_t* r,const recomp_context* caller,unsigned member,const Frame& f,Dispatch&& dispatch){
  if(!caller||!ready_to_retire(r,member,f))return false;
  const auto stack=uint32_t(caller->r29);if(stack<0x80000800||!aligned(stack-0x200,0x220))return false;
  auto& m=state_.members[member];const auto original=m.identity;
  recomp_context work=*caller;work.r29=n::base(stack-0x100);work.r4=n::base(original.actor);work.r5=work.r6=work.r7=0;
  dispatch(0x800447C8,r,&work);
  const auto after=identity(r,original.actor,kind);
  if(!after||*after!=original||!retired_actor(r,original)||n::word(r,original.actor+0x1D8)!=m.record||n::word(r,m.record)!=0xFFFFFFFFu){fail("blargg_native_retirement_readback");return false;}
  m.retired=true;m.retirement_vi=f.vi;
  if(!m.receipt_sent){receipts_.push_back({state_.config.token,state_.config.encounter,member,state_.config.physical_room,state_.config.retry_epoch,f.vi,m.hits,m.identity,m.variant_birth,true,true,true,true,state_.config.diagnostic});m.receipt_sent=true;}
  return true;
 }
 std::vector<Receipt> take_receipts(){auto result=std::move(receipts_);receipts_.clear();return result;}
 bool both_defeated()const{return state_.enabled&&!state_.invalidated&&state_.members[0].retired&&state_.members[1].retired;}
};
}