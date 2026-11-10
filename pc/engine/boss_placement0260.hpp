#pragma once
#include "boss_family_native0250.hpp"
#include "boss_rooms_data0250.hpp"
#include <array>
#include <cstring>
#include <optional>

namespace workshop::boss_placement0260 {
namespace native=boss_native;namespace core=boss_battle0250;namespace rooms=boss_rooms_data0250;namespace witnesses=boss_family_native0250;
// Placement records are12 bytes in ROM but16 bytes after native scene loading.
// 5A048->48E9C(actor,record), RA5A050, captures1D8/1DC before6D340
// consumes the record (FFFFFFFF).48D4C later restores it through48CA4.
// After a strict native defeat only,48BE8(actor,0,0) detaches that respawn
// link. The original animation, actor graph and eventual retirement continue.
// This module never writes HP, scores, pose, collisions or actor queues.
inline bool supported(uint16_t kind){return kind==0x4599||kind==0x4072||kind==0x40A3;}
struct Binding {
 uint32_t actor=0,record=0,packed=0,private_data=0,resource=0,player=0;
 uint64_t birth=0;unsigned receipt_vi=0,hits=0;bool captured=false,bound=false,initialized=false,pending=false,applied=false;
};
struct Fence {unsigned room=177,encounter=0,vi=0;uint32_t player=0;core::Phase phase=core::Phase::Idle;bool after_675=false;};
class Guard {
 std::array<Binding,4> slots_{};unsigned room_=177,encounter_=0,proof_room_=177;uint32_t table_=0;
 static bool aligned(uint32_t a,unsigned size){return !(a&3)&&native::range(a,size);}
 static unsigned private_size(uint16_t k){return k==0x4599?0x2C:k==0x4072?0x28:k==0x40A3?0x34:0;}
 static bool link(uint8_t* rdram,const Binding& b){return aligned(b.record,16)&&native::word(rdram,b.record)==0xFFFFFFFF&&native::word(rdram,b.actor+0x1D8)==b.record&&native::word(rdram,b.actor+0x1DC)==b.packed;}
 static bool actor_identity(uint8_t* rdram,const Binding& b){return native::actor(rdram,b.actor,uint16_t(b.packed>>16))&&native::word(rdram,b.actor+0x168)==b.resource&&native::byte(rdram,b.actor+0x17D)==2;}
 static bool native_table(uint8_t* rdram,uint32_t start,const rooms::Arena& arena){
  if(arena.objects.empty()||arena.objects.size()%12||arena.objects.size()/12>149||!aligned(start,unsigned(arena.objects.size()/12)*16))return false;
  for(size_t p=0;p<arena.objects.size();p+=12){const auto a=start+uint32_t(p/12)*16,packed=native::word(rdram,a);if(packed!=0xFFFFFFFF&&packed!=u32(arena.objects,p))return false;if(native::word(rdram,a+4)!=u32(arena.objects,p+4)||native::word(rdram,a+8)!=u32(arena.objects,p+8))return false;}return true;
 }
public:
 void reset(unsigned room=177,unsigned encounter=0){slots_={};room_=room;encounter_=encounter;proof_room_=177;table_=0;}
 const auto& snapshot()const{return slots_;}
 bool pending()const{for(const auto& b:slots_)if(b.pending)return true;return false;}
 std::optional<unsigned> member(uint32_t actor)const{for(unsigned i=0;i<4;++i)if(slots_[i].captured&&slots_[i].actor==actor)return i;return {};}
 bool capture(uint8_t* rdram,const recomp_context* c,const rooms::Arena& arena,const core::Encounter& e,unsigned room,unsigned encounter){
  if(!rdram||!c||room!=room_||encounter!=encounter_||arena.source!=room||e.number!=encounter+1||e.count<1||e.count>4||arena.boss_indices.size()!=e.count||uint32_t(c->r31)!=0x8005A050)return false;
  const auto actor=uint32_t(c->r4),record=uint32_t(c->r5);if(!aligned(actor,0x1FC)||!aligned(record,16))return false;
  const auto packed=native::word(rdram,record);const auto kind=uint16_t(packed>>16);if(!supported(kind)||!native::actor(rdram,actor,kind))return false;
  unsigned member=4;size_t offset=0;for(unsigned i=0;i<e.count;++i){const size_t p=size_t(arena.boss_indices[i])*12;if(p+12>arena.objects.size())return false;if(u32(arena.objects,p)==packed&&e.members[i].kind==kind){if(member!=4)return false;member=i;offset=p;}}
  if(member==4||slots_[member].captured||native::word(rdram,record+4)!=u32(arena.objects,offset+4)||native::word(rdram,record+8)!=u32(arena.objects,offset+8))return false;
  for(const auto& b:slots_)if(b.captured&&(b.actor==actor||b.record==record))return false;
  const unsigned native_offset=unsigned(offset/12)*16;if(record<native_offset)return false;const auto table=record-native_offset;if((table_&&table_!=table)||!native_table(rdram,table,arena))return false;
  const auto resource=native::word(rdram,actor+0x168);if(!aligned(resource,0x50))return false;
  table_=table;proof_room_=core::room(e.family);auto& b=slots_[member];b.actor=actor;b.record=record;b.packed=packed;b.resource=resource;b.hits=e.members[member].hits;b.captured=true;return true;
 }
 bool bind(uint8_t* rdram,unsigned member,uint32_t actor,uint64_t birth){
  if(member>=4||!birth)return false;auto& b=slots_[member];if(!b.captured||b.bound||b.actor!=actor||!actor_identity(rdram,b)||!link(rdram,b))return false;b.birth=birth;b.bound=true;return true;
 }
 bool initialized(uint8_t* rdram,unsigned member,uint32_t actor,uint64_t birth){
  if(member>=4)return false;auto& b=slots_[member];if(!b.bound||b.initialized||b.actor!=actor||b.birth!=birth||!actor_identity(rdram,b)||!link(rdram,b))return false;
  const auto p=native::word(rdram,actor+0x1F4);if(!aligned(p,private_size(uint16_t(b.packed>>16))))return false;b.private_data=p;b.initialized=true;return true;
 }
 bool authorize(uint8_t* rdram,const core::NativeReceipt& receipt,const witnesses::Snapshot& proof,uint32_t player){
  if(receipt.member>=4||receipt.encounter!=encounter_||!receipt.initialized||!receipt.active||!receipt.terminal||!receipt.causal_hit)return false;
  auto& b=slots_[receipt.member];if(!b.initialized||b.pending||b.applied||b.actor!=receipt.actor||!actor_identity(rdram,b)||!link(rdram,b)||native::word(rdram,b.actor+0x1F4)!=b.private_data)return false;
  const auto& a=proof.actors[0];if(!proof.enabled||proof.invalidated||proof.source_room!=proof_room_||proof.family!=*witnesses::family_for_kind(uint16_t(b.packed>>16))||proof.registered_actors!=1||proof.expected_hits!=b.hits||!a.initialized||!a.active_seen||!a.terminal_seen||!a.completion_called||!a.completion_returned||!a.cinematic_suppressed||!a.delivered||a.disposed||a.actor!=b.actor||a.private_data!=b.private_data||a.completion_vi!=receipt.vi||a.player!=player||a.causal_hits!=b.hits)return false;
  if(!witnesses::active_player(rdram,player)||native::word(rdram,0x800F8828)!=player)return false;
  b.player=player;b.receipt_vi=receipt.vi;b.pending=true;return true;
 }
 template<class Dispatch> bool post_675(uint8_t* rdram,recomp_context* c,const Fence& f,const std::array<uint64_t,4>& births,Dispatch&& dispatch){
  if(!rdram||!c||!aligned(uint32_t(c->r29),16)||uint32_t(c->r29)<0x800004C0||!aligned(uint32_t(c->r29)-0xC0,0xD0)||!f.after_675||f.room!=room_||f.encounter!=encounter_||(f.phase!=core::Phase::Fighting&&f.phase!=core::Phase::Victory))return false;
  // Validate every pending member before the first native write, so a corrupt
  // companion cannot leave an otherwise rejected batch partially committed.
  for(unsigned i=0;i<4;++i){const auto& b=slots_[i];if(!b.pending)continue;const auto kind=uint16_t(b.packed>>16);const auto family=witnesses::family_for_kind(kind);if(!family||births[i]!=b.birth||!b.initialized||b.applied||f.vi<b.receipt_vi||b.player!=f.player||!witnesses::active_player(rdram,f.player)||native::word(rdram,0x800F8828)!=f.player||!actor_identity(rdram,b)||!link(rdram,b)||native::word(rdram,b.actor+0x1F4)!=b.private_data)return false;
   const auto& cfg=witnesses::spec(*family);if(native::byte(rdram,b.actor+0x18)!=cfg.terminal_state||native::byte(rdram,b.actor+0x30)!=cfg.terminal_substate||native::word(rdram,cfg.counter_in_private?b.private_data+0x24:b.actor+0x1E4)!=cfg.counter_terminal)return false;
  }
  for(auto& b:slots_)if(b.pending){const auto saved=*c;c->r29=ADD32(saved.r29,-0xC0);c->r4=native::base(b.actor);c->r5=0;c->r6=0;dispatch(0x80048BE8,rdram,c);*c=saved;
   if(native::word(rdram,b.actor+0x1D8)||native::word(rdram,b.actor+0x1DC)||native::word(rdram,b.record)!=0xFFFFFFFF)return false;b.pending=false;b.applied=true;
  }
  return true;
 }
};
} // namespace workshop::boss_placement0260
