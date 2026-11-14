#pragma once
#include "boss_battle0250.hpp"
#include "boss_native.hpp"
#include "boss_blargg0289.hpp"
#include "boss_fish_explosion0260.hpp"
#include "boss_fish_flash0260.hpp"
#include "boss_fish_green0260.hpp"
#include "boss_fish_purple0260.hpp"
#include "boss_variant_action_probe0281.hpp"
#include "boss_bird_null0281.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace workshop::boss_variants0250 {
namespace battle=boss_battle0250;
// All mutations require an explicitly bound campaign member. None of these
// hooks change the vanilla game merely because an actor has a matching ID.
inline constexpr uint16_t monster_kind=0x41C6,red_fish=0x4511,blue_fish=0x4631,blargg_kind=0x41EC;
inline constexpr uint32_t monster_hp=0xDE8,monster_private_size=0x1000,fish_private_size=0x188,blargg_private_size=0xB0;
inline constexpr unsigned fish_hits=battle::fish_hits,fish_refractory_vi=60;
enum class PurplePhase:uint8_t { Arc,Vertical };
enum class Operation:uint8_t { Water,Jump,Retire,GenieFire,BirdSpawn,FireStart,FireStop,BirdInjury };
struct Action {
 Operation operation=Operation::Water;unsigned encounter=0,member=0,vi=0;uint32_t actor=0;uint64_t birth=0;
 uint32_t function=0;std::array<uint32_t,4> arguments{};
};
struct Frame {unsigned room=177,vi=0;bool combat_active=false,proven_zero_controls=false;};
struct Binding {
 uint32_t actor=0,private_data=0;uint64_t birth=0;unsigned initial=0,remaining=0,last_hit_tick=0,hits=0;
 bool initialized=false,active=false,terminal_requested=false,terminal=false,causal=false,receipt_sent=false,retire_queued=false;
 PurplePhase purple_phase=PurplePhase::Arc;bool jump_started=false;
 uint32_t fire_generator=0,fire_private=0;unsigned fire_until=0,fire_requested_tick=0;bool fire_pending=false,fire_streaming=false;
};
struct Pulse {uint64_t birth=0;uint32_t source=0,private_data=0;uint8_t hit_mask=0;bool primary_egg=false;uint8_t bird_mask=0;};
struct PendingDamage {uint32_t stack=0,root=0;unsigned member=0,before=0;uint64_t birth=0;};
struct PaintSwap {uint32_t resource=0,original=0,replacement=0,drawing=0,drawing_original=0;boss_fish_green0260::DrawTicket green_draw{};};
using BirdSpawnPoint=std::array<float,3>;
struct Bird {uint32_t actor=0;uint64_t birth=0;unsigned next_tick=0;bool pending=false,positioned=false,chosen=false;std::array<uint32_t,3> position{};std::array<std::array<uint32_t,3>,8> options{};unsigned option_count=0;uint32_t private_data=0,resource=0,factory=0,source_actor=0,source_private=0,injury_stack=0;uint64_t source_birth=0,injury_birth=0;bool injury_queued=false;};
struct CandyChild {unsigned member=0;uint64_t birth=0;uint16_t kind=0;uint32_t resource=0;};
struct Snapshot {
 bool enabled=false,invalidated=false;unsigned encounter=0,room=177,active_ticks=0,fire_rolls=0,fire_requests=0,yellow_until=0;
 std::array<Binding,4> members{};std::string failure;
};
inline Snapshot state{};inline battle::Encounter encounter{};inline battle::Config config{};inline battle::Random random{0};
inline battle::Random bird_random{0};
inline unsigned last_vi=0;inline bool have_vi=false;inline uint64_t serial=0;
inline std::unordered_map<uint32_t,Pulse> pulses;
inline std::vector<PendingDamage> damage_stack;
inline std::vector<Action> actions;
inline bool bird_null_logged=false;
inline std::vector<battle::NativeReceipt> receipts;
inline std::array<Bird,8> birds{};inline std::unordered_map<uint32_t,CandyChild> candy_children;
// Explicit campaign scratch reservation; disjoint from route807DFF00,
// passage807E0000 and existing workshop807F buffers. Never clone a resource.
inline constexpr uint32_t palette_base=0x807D8000,palette_end=0x807DC000;
inline std::vector<PaintSwap> paint_stack;
inline unsigned palette_cursor=0;
inline std::unordered_map<uint64_t,uint32_t> palette_slots;
inline uint32_t word(uint8_t* rdram,uint32_t a){return boss_native::word(rdram,a);}
inline unsigned byte(uint8_t* rdram,uint32_t a){return boss_native::byte(rdram,a);}
// Runtime supplies the source/member/player witness; a bare scene0/0 is not
// permission to run variants during an intro, pause or native transport.
inline bool admitted_controls(uint8_t* rdram,const Frame& f){
 if(!rdram)return false;const auto scene=byte(rdram,0x800F7EA6);
 return scene==1||(f.proven_zero_controls&&!byte(rdram,0x800F7EA5)&&scene==0);
}

inline bool range(uint32_t p,unsigned n){return p%4==0&&boss_native::range(p,n);}
inline bool actor(uint8_t* rdram,uint32_t a,uint16_t kind){return rdram&&range(a,0x1FC)&&boss_native::actor(rdram,a,kind);}
inline void fail(const char* reason){state.invalidated=true;state.failure=reason;actions.clear();}
inline unsigned ticks(unsigned ms){return unsigned((uint64_t(ms)*60+999)/1000);}
inline bool begin(const battle::Encounter& e,unsigned index,const battle::Config& c={},uint64_t seed=0){
 boss_variant_action_probe0281::reset();
 bird_null_logged=false;
 state={};encounter=e;config=c;random={seed};bird_random={seed^0xA5B35705C87F216Bull};state.encounter=index;state.room=battle::room(e.family);
 pulses.clear();damage_stack.clear();actions.clear();receipts.clear();paint_stack.clear();palette_cursor=0;palette_slots.clear();last_vi=0;have_vi=false;
 birds={};candy_children.clear();state.enabled=c.valid()&&e.count>0&&e.count<=4&&index<10&&e.birds<=8;
 for(unsigned n=0;n<e.count&&state.enabled;++n){const auto& m=e.members[n];state.enabled=m.kind&&m.hits>0&&(!m.green||(e.family==battle::Family::Fish&&m.kind==red_fish&&!m.purple)); if(e.family==battle::Family::Fish)state.enabled=state.enabled&&battle::valid_fish(e); if(e.family==battle::Family::Monsters)state.enabled=state.enabled&&m.kind==monster_kind&&m.hits>=3&&m.hits<=5;}
 if(e.family==battle::Family::Blargg)state.enabled=state.enabled&&index>=1&&index<=8&&e.number==index+1&&battle::valid_blargg(e,c);
 return state.enabled;
}
inline void disable(){state={};actions.clear();pulses.clear();damage_stack.clear();receipts.clear();paint_stack.clear();birds={};candy_children.clear();boss_variant_action_probe0281::reset();}
inline void enter_room(unsigned source){if(state.enabled&&source!=state.room){fail("variant_left_owned_arena");pulses.clear();damage_stack.clear();}}
inline bool bind(uint8_t* rdram,unsigned member,uint32_t a){
 if(!state.enabled||state.invalidated||member>=encounter.count||state.members[member].actor||!actor(rdram,a,encounter.members[member].kind))return false;
 for(auto& b:state.members)if(b.actor==a)return false;
 auto& b=state.members[member];b.actor=a;b.birth=++serial;b.initial=b.remaining=encounter.members[member].hits;return true;
}
inline std::optional<unsigned> owner(uint8_t* rdram,uint32_t a){
 if(!state.enabled||state.invalidated)return {};
 for(unsigned n=0;n<encounter.count;++n){auto& b=state.members[n];if(b.actor==a&&actor(rdram,a,encounter.members[n].kind))return n;}return {};
}
inline bool live(uint8_t* rdram,unsigned n){
 if(n>=encounter.count)return false;auto& b=state.members[n];if(!owner(rdram,b.actor))return false;
 const auto p=word(rdram,b.actor+0x1F4);return b.initialized&&p==b.private_data&&range(p,encounter.members[n].kind==monster_kind?monster_private_size:encounter.family==battle::Family::Blargg&&encounter.members[n].kind==blargg_kind?blargg_private_size:fish_private_size);
}
// The independent ledger accepts the actual42B8 return. Import only that
// authenticated constructor witness into HOST variant state;41EC has no HP
// counter, and no guest field is written by this adapter.
inline bool initialize_blargg(uint8_t* rdram,unsigned member,const boss_blargg0289::Observer& ledger){
 if(!rdram||!state.enabled||state.invalidated||encounter.family!=battle::Family::Blargg||state.room!=19||member>=2||member>=encounter.count||state.encounter<1||state.encounter>8||!battle::valid_blargg(encounter,config))return false;
 const auto& proof=ledger.snapshot();const auto& m=proof.members[member];auto& b=state.members[member];const auto& id=m.identity;
 if(!proof.enabled||proof.invalidated||proof.config.token.empty()||proof.config.encounter!=state.encounter||proof.config.physical_room!=167+state.encounter||!m.initialized||!m.initial_vi||m.retired||m.retracted||m.receipt_sent||m.hits||!b.birth||m.variant_birth!=b.birth||b.actor!=id.actor||!id.birth||id.birth!=ledger.birth(id.actor)||id.type!=blargg_kind||b.initial!=3||b.remaining!=3||b.initialized||b.active||b.terminal||b.terminal_requested||b.receipt_sent||b.retire_queued||b.hits)return false;
 // Typed native predicate distinguishes the common2C pool cleanup from
 // private1F8 cleanup, using the real constructor/resource/birth witness.
 if(!ledger.initialized_member(rdram,member))return false;
 if(byte(rdram,id.actor+0x18)!=1||byte(rdram,id.actor+0x30)!=1||word(rdram,id.actor+0x20)!=m.overlay+0x10F4)return false;
 b.private_data=id.private_data;b.initialized=true;return true;
}
inline std::optional<boss_fish_green0260::Frame> green_frame(uint8_t* rdram,uint32_t a,const Frame& f){
 if(!f.combat_active||f.room!=state.room||encounter.family!=battle::Family::Fish||!admitted_controls(rdram,f))return {};
 const auto n=owner(rdram,a);if(!n||!live(rdram,*n))return {};
 const auto& member=encounter.members[*n];if((member.green&&member.purple)||(!member.green&&!(member.purple&&state.members[*n].purple_phase==PurplePhase::Vertical)))return {};
 const auto& b=state.members[*n];if(!b.birth||!b.active||!b.remaining||b.terminal||b.terminal_requested||b.retire_queued||b.receipt_sent)return {};
 return boss_fish_green0260::Frame{true,true,true,f.room,a,b.private_data,b.birth};
}
inline std::optional<boss_fish_purple0260::Frame> purple_frame(uint8_t* rdram,uint32_t a,const Frame& f){
 if(!f.combat_active||f.room!=state.room||encounter.family!=battle::Family::Fish||!admitted_controls(rdram,f))return {};
 const auto n=owner(rdram,a);if(!n||!encounter.members[*n].purple||encounter.members[*n].green||!live(rdram,*n))return {};
 const auto& b=state.members[*n];if(!b.birth||!b.active||!b.remaining||b.purple_phase!=PurplePhase::Arc||b.terminal||b.terminal_requested||b.retire_queued||b.receipt_sent)return {};
 return boss_fish_purple0260::Frame{true,true,true,f.room,a,b.private_data,b.birth};
}
// Native45EC8..EE8 installs7FFF in actorC0/C2 for an always-visible
// resource.48E30 passes these two culling extents to59EBC, whose7FFF
// branch returns visible without changing gameplay collision data.45520
// also copies them to the drawing object, not the fish's private colliders.
// Keep the shared resourceB intact: only this live campaign generation owns
// these four actor bytes. Revalidate every guard before either halfword write.
inline bool preserve_fish_bounds(uint8_t* rdram,unsigned n,uint64_t birth,const Frame& f){
 if(!rdram||!state.enabled||state.invalidated||encounter.family!=battle::Family::Fish||!f.combat_active||f.room!=state.room||!admitted_controls(rdram,f)||n>=encounter.count)return false;
 const auto& member=encounter.members[n];const auto& b=state.members[n];
 if(member.kind!=red_fish||!birth||b.birth!=birth||!b.initialized||b.terminal_requested||b.terminal||b.receipt_sent||b.retire_queued||!b.remaining||!live(rdram,n)||byte(rdram,b.actor+0x17D)!=2||byte(rdram,b.actor+0x18)!=1)return false;
 MEM_H(0xC0,boss_native::base(b.actor))=0x7FFF;
 MEM_H(0xC2,boss_native::base(b.actor))=0x7FFF;
 return true;
}
inline bool preserve_fish_visibility(uint8_t* rdram,uint32_t a,const Frame& f){
 const auto n=owner(rdram,a);return n&&preserve_fish_bounds(rdram,*n,state.members[*n].birth,f);
}
inline void forget(uint32_t a){
 pulses.erase(a);candy_children.erase(a);for(auto& b:state.members)if(a&&b.fire_generator==a){b.fire_generator=b.fire_private=0;b.fire_streaming=b.fire_pending=false;}for(auto& bird:birds){if(a&&bird.source_actor==a){bird.source_actor=bird.source_private=0;bird.source_birth=0;}if(a&&bird.actor==a){bird.actor=bird.private_data=bird.resource=bird.factory=bird.source_actor=bird.source_private=bird.injury_stack=0;bird.source_birth=bird.injury_birth=0;bird.injury_queued=false;bird.next_tick=state.active_ticks+ticks(encounter.bird_respawn_ms);bird.pending=false;}}
 for(unsigned member=0;member<state.members.size();++member){const auto& b=state.members[member];if(a&&b.actor==a&&!b.terminal){if(!state.invalidated)std::fprintf(stderr,"[BossMemberDisposed] encounter=%u member=%u actor=%08X expectedKind=%04X remaining=%u hits=%u birth=%llu hostObservation=1\n",state.encounter,member,a,unsigned(encounter.members[member].kind),b.remaining,b.hits,(unsigned long long)b.birth);fail("variant_actor_disposed_or_reused");return;}}
}
inline void created(uint32_t a){forget(a);}
inline bool pulse(uint8_t* rdram,uint32_t a){
 auto it=pulses.find(a);if(it==pulses.end()||!it->second.primary_egg||!it->second.birth||!actor(rdram,a,0x4018))return false;
 const auto p=word(rdram,a+0x1F4);return actor(rdram,a,0x4018)&&byte(rdram,a+0x18)==0x68&&byte(rdram,a+0x17D)==2&&range(p,0x20)&&p==it->second.private_data&&word(rdram,p)==0;
}
inline bool bird_ready(uint8_t* rdram,unsigned slot){
 if(!state.enabled||state.invalidated||slot>=encounter.birds||slot>=birds.size())return false;auto& b=birds[slot];const auto a=b.actor;
 if(!b.birth||!actor(rdram,a,0x4526)||byte(rdram,a+0x17D)!=2||byte(rdram,a+0x18)!=1||word(rdram,a+0x180)||byte(rdram,a+0x165))return false;
 const auto sub=byte(rdram,a+0x30),p=word(rdram,a+0x1F4),resource=word(rdram,a+0x168),metadata=word(rdram,a+0x174);
 if((sub!=5&&sub!=6)||!range(p,12)||!range(resource,0x48)||!range(metadata,4))return false;
 const auto factory=word(rdram,resource+0x40),definition=word(rdram,metadata);
 if(!range(factory,0x4000)||!range(definition,0x75)||MEM_HU(0x3C,boss_native::base(resource))!=0x4526||word(rdram,resource+0x44)!=factory+0x1B1C||word(rdram,definition+0x28)!=factory+0x1A14||!byte(rdram,definition+0x74)||word(rdram,a+0x20)!=factory+(sub==5?0x9BC:0x884))return false;
 if((b.private_data&&b.private_data!=p)||(b.resource&&b.resource!=resource)||(b.factory&&b.factory!=factory))return false;
 b.private_data=p;b.resource=resource;b.factory=factory;return true;
}
inline bool bird_successor(uint8_t* rdram,unsigned slot,uint32_t old,uint64_t birth,unsigned vi){
 if(!state.enabled||state.invalidated||slot>=encounter.birds||slot>=birds.size())return false;auto& b=birds[slot];
 if(b.actor!=old)return b.source_actor==old&&b.source_birth==birth&&actor(rdram,b.actor,0x46DA)&&word(rdram,old+0x180)==b.actor;
 if(b.birth!=birth||!birth||!b.factory||!actor(rdram,old,0x4526)||word(rdram,old+0x1F4)!=b.private_data||word(rdram,old+0x168)!=b.resource||byte(rdram,old+0x17D)!=0||byte(rdram,old+0x165)!=1||byte(rdram,old+0x17C)||word(rdram,old+0x20)!=b.factory+0x1A08)return false;
 // 46804/45CAC installs the public35B4 initializer;44194 then queues
 // lifecycle1/state0. The native35B4 body runs only on the next actor update.
 const auto next=word(rdram,old+0x180);if(!actor(rdram,next,0x46DA)||word(rdram,next+0x180)||byte(rdram,next+0x17D)!=1||byte(rdram,next+0x18)!=0||word(rdram,next+0x20)!=b.factory+0x1B84)return false;
 // Native45CAC reuses pooled46DA storage without clearing private1F4.
 // A queued initializer may therefore retain its previous allocation. Do
 // not treat that old pointer as the new private identity: bind it only
 // after the real35B4 initializer reaches active phase1/7 below.
 const auto pending_private=word(rdram,next+0x1F4);if(pending_private&&!range(pending_private,12))return false;
 const auto resource=word(rdram,next+0x168);const auto child=MEM_HU(0x17E,boss_native::base(old)),count=MEM_HU(0x184,boss_native::base(next));
 if(!range(resource,0x48)||MEM_HU(0x3C,boss_native::base(resource))!=0x46DA||word(rdram,resource+0x40)!=b.factory||word(rdram,resource+0x44)!=b.factory+0x1B84||!count||count>20||child>=20||word(rdram,next+0x188+4*child)!=old)return false;
 for(unsigned other=0;other<encounter.birds;++other)if(other!=slot&&birds[other].actor==next)return false;
 b.source_actor=old;b.source_private=b.private_data;b.source_birth=birth;b.actor=next;b.private_data=0;b.resource=resource;b.birth=++serial;b.pending=false;b.injury_queued=false;b.injury_stack=0;b.injury_birth=0;
 fprintf(stderr,"[BossBirdInjury] source=%08X wounded=%08X slot=%u vi=%u native3444=1 score=0\n",old,next,slot,vi);return true;
}
inline void remember_wounded_birds(uint8_t* rdram){
 for(auto& b:birds)if(b.actor&&b.source_actor&&b.source_birth&&!b.private_data&&actor(rdram,b.actor,0x46DA)&&word(rdram,b.actor+0x168)==b.resource&&range(b.resource,0x48)&&word(rdram,b.resource+0x40)==b.factory&&word(rdram,b.resource+0x44)==b.factory+0x1B84&&byte(rdram,b.actor+0x17D)==2&&byte(rdram,b.actor+0x18)==1&&byte(rdram,b.actor+0x30)==7&&word(rdram,b.actor+0x20)==b.factory+0xAE4){const auto p=word(rdram,b.actor+0x1F4);if(range(p,12))b.private_data=p;}
}
inline void remember_candy(uint8_t* rdram){
 for(unsigned n=0;n<encounter.count;++n){if(!encounter.members[n].red||!live(rdram,n))continue;const auto head=word(rdram,state.members[n].private_data);if(!actor(rdram,head,0x454F)||word(rdram,head+0x180)!=state.members[n].actor)continue;
  auto remember=[&](uint32_t a){if(range(a,0x1FC)&&word(rdram,a)==0x19971110)candy_children[a]={n,state.members[n].birth,uint16_t(MEM_HU(12,boss_native::base(a))),word(rdram,a+0x168)};};remember(head);
  const auto p=word(rdram,head+0x1F4);if(!range(p,0x88))continue;for(unsigned i=0;i<12;++i){const auto a=word(rdram,p+0x54+i*4);if(range(a,0x1FC)&&word(rdram,a+0x180)==head)remember(a);}
 }
}
inline std::optional<unsigned> candy_owner(uint8_t* rdram,uint32_t a){
 auto marked=candy_children.find(a);if(marked!=candy_children.end()){const auto& c=marked->second;if(c.member<encounter.count&&c.birth==state.members[c.member].birth&&actor(rdram,a,c.kind)&&word(rdram,a+0x168)==c.resource&&live(rdram,c.member))return c.member;}
 // Parent links survive normal display changes. Detached candy segments are
 // additionally found through the native head's private54 twelve-slot list.
 auto cur=a;for(unsigned depth=0;depth<4&&range(cur,0x1FC);++depth){if(auto n=owner(rdram,cur);n&&encounter.members[*n].red)return n;cur=word(rdram,cur+0x180);}
 for(unsigned n=0;n<encounter.count;++n){if(!encounter.members[n].red||!live(rdram,n))continue;const auto head=word(rdram,state.members[n].private_data);if(!actor(rdram,head,0x454F))continue;const auto p=word(rdram,head+0x1F4);if(!range(p,0x88))continue;for(unsigned i=0;i<12;++i)if(word(rdram,p+0x54+i*4)==a)return n;}
 return {};
}
inline bool yellow(){return state.enabled&&!state.invalidated&&state.active_ticks<state.yellow_until;}
inline std::optional<int> healing_override(uint8_t* rdram,uint32_t food){if(!candy_owner(rdram,food))return {};return yellow()?int(config.yellow_healing):0;}
inline void request(Operation op,unsigned n,unsigned vi,uint32_t fn,std::array<uint32_t,4> args={}){
 const auto& b=state.members[n];actions.push_back({op,state.encounter,n,vi,b.actor,b.birth,fn,args});
}
inline bool fish_contact_ready(uint8_t* rdram,unsigned n){
 if(!rdram||encounter.family!=battle::Family::Fish||n>=encounter.count||encounter.members[n].kind!=red_fish||!live(rdram,n))return false;
 const auto& b=state.members[n];const auto sub=byte(rdram,b.actor+0x30);
 // Native6900:6968..6978 admits ordinary states0..12 AND airborne15.
 // States13/14/16/17/18 move/animate a swallowed Yoshi; private40 also
 // carries the pending passenger countdown into15/1 before native2FCBC.
 return byte(rdram,b.actor+0x17D)==2&&byte(rdram,b.actor+0x18)==1&&(sub<0xD||sub==0xF)&&word(rdram,b.private_data+0x40)==0;
}
inline bool fish_hit(uint8_t* rdram,unsigned n,uint32_t projectile,unsigned vi){
 auto& b=state.members[n];if(!fish_contact_ready(rdram,n)||!b.active||b.terminal||b.retire_queued||!pulse(rdram,projectile))return false;
 auto& hit=pulses.at(projectile);const auto bit=uint8_t(1u<<n);if(hit.hit_mask&bit)return false;
 if(b.hits&&state.active_ticks-b.last_hit_tick<fish_refractory_vi)return false;
 hit.hit_mask|=bit;b.last_hit_tick=state.active_ticks;b.causal=true;++b.hits;--b.remaining;
 fprintf(stderr,"[BossFishHit] burst=%08X member=%u remaining=%u vi=%u nativeObservation=1\n",projectile,n,b.remaining,vi);
 if(!b.remaining){b.retire_queued=true;request(Operation::Retire,n,vi,0x800447C8,{b.actor,0,0,0});}
 return true;
}
// Return true only when the reciprocal native4018 callback must skip generic
// one-shot enemy destruction. Ordinary actors/eggs/food keep vanilla behavior.
inline bool collision(uint8_t* rdram,const recomp_context* c,std::string_view name,const Frame& f){
 if(!rdram||!c||!state.enabled||state.invalidated||!f.combat_active||f.room!=state.room||!admitted_controls(rdram,f))return false;
 const auto a=uint32_t(c->r4),other=uint32_t(c->r5);
 if(name=="func_806F3498"&&pulse(rdram,a)){
  if(auto n=owner(rdram,other);n&&encounter.family==battle::Family::Fish&&encounter.members[*n].kind==red_fish){fish_hit(rdram,*n,a,f.vi);return true;}
  if(auto n=candy_owner(rdram,other);n){auto& p=pulses.at(a);const auto bit=uint8_t(1u<<*n);if(!(p.hit_mask&bit)){p.hit_mask|=bit;state.yellow_until=state.active_ticks+ticks(config.yellow_window_ms);fprintf(stderr,"[BossRed] real-egg-window pulse=%08X source=%08X target=%08X member=%u untilTick=%u vi=%u\n",a,p.source,other,*n,state.yellow_until,f.vi);}}
 }
 if(name=="func_805C6900"){if(auto n=owner(rdram,a);n&&encounter.family==battle::Family::Fish)fish_hit(rdram,*n,other,f.vi);}
 return false;
}
inline void trace(uint8_t* rdram,const recomp_context* c,std::string_view name,const Frame& f){
 if(!rdram||!c||!state.enabled||state.invalidated||f.room!=state.room)return;
 const auto a=uint32_t(c->r4),other=uint32_t(c->r5);
 // Refresh immediately before the original native cull check as well as at
 // frame fences; a native state may have recomputed its normal extents.
 if(name=="func_80048E30"){preserve_fish_visibility(rdram,a,f);return;}
 if(name=="func_800483B8"){
  if(const auto purple=purple_frame(rdram,a,f)){if(boss_fish_purple0260::admitted(rdram,*purple))if(const auto n=owner(rdram,a))state.members[*n].jump_started=true;boss_fish_purple0260::before_integrate(rdram,c,*purple);}
  else if(const auto green=green_frame(rdram,a,f);green&&boss_fish_green0260::admitted(rdram,*green))if(const auto n=owner(rdram,a);n&&encounter.members[*n].purple)state.members[*n].jump_started=true;
  return;
 }
 if(name=="func_805D3444"){if(range(uint32_t(c->r29)-0x50,0x60)&&(actor(rdram,other,0x4002)||actor(rdram,other,0x4018))&&byte(rdram,other+0x18)==0x68)for(unsigned slot=0;slot<encounter.birds;++slot)if(birds[slot].actor==a&&bird_ready(rdram,slot)){birds[slot].injury_stack=uint32_t(c->r29);birds[slot].injury_birth=birds[slot].birth;break;}return;}
 if(name=="func_80048F84"){if(a>=0xC4){if(const auto green=green_frame(rdram,a-0xC4,f))boss_fish_green0260::before_axis(rdram,c,*green);else if(const auto purple=purple_frame(rdram,a-0xC4,f))boss_fish_purple0260::before_axis(rdram,c,*purple);}return;}
 if(name=="func_80047F9C"){remember_candy(rdram);return;}
 if(name=="func_805C7C2C"&&encounter.family==battle::Family::Fish){
  // Init9CF4 selects native1/1, whose table entry is7C2C. This real
  // callback can run during warm arrival before the player's release;
  // retain its activity witness even if the fish has changed state later.
  if(auto n=owner(rdram,a);n&&live(rdram,*n)){
   auto& b=state.members[*n];const auto resource=word(rdram,a+0x168);
   if(b.birth&&!b.terminal&&!b.retire_queued&&!b.receipt_sent&&byte(rdram,a+0x17D)==2&&byte(rdram,a+0x18)==1&&byte(rdram,a+0x30)==1&&range(resource,0x48)){
    const auto factory=word(rdram,resource+0x40);
    if(range(factory,0x3FF0)&&MEM_HU(0x3C,boss_native::base(resource))==encounter.members[*n].kind&&word(rdram,a+0x20)==factory+0x173C)b.active=true;
   }
  }return;
 }
 if(name=="func_806F3A1C"){
  // Actual native explosion rearm takes source4002 and mode0 for its causal
  // collision volume. Mode1 is only its attached visual duplicate.
  pulses.erase(a);
  if(actor(rdram,a,0x4018)&&actor(rdram,other,0x4002)&&byte(rdram,other+0x18)==0x68&&uint32_t(c->r7)==0&&uint32_t(c->r6)<=2)pulses[a]={++serial,other,word(rdram,a+0x1F4),0,true};
  return;
 }
 if(name=="func_8055484C"){
  if(auto n=owner(rdram,a);n&&encounter.family==battle::Family::Monsters){auto& b=state.members[*n];const auto p=word(rdram,a+0x1F4);
   if(!b.initialized&&uint32_t(c->r5)==1&&uint32_t(c->r6)==1&&range(p,monster_private_size)&&word(rdram,p+monster_hp)==3){b.private_data=p;b.initialized=true;MEM_W(monster_hp,boss_native::base(p))=b.initial;}
   else if(live(rdram,*n)&&uint32_t(c->r5)==1&&uint32_t(c->r6)==2&&word(rdram,p+monster_hp)==0&&b.causal)b.terminal_requested=true;
  }return;
 }
 if(name=="func_805C9D10"){
  if(auto n=owner(rdram,a);n&&encounter.family==battle::Family::Fish){auto& b=state.members[*n];const auto p=word(rdram,a+0x1F4);
   if(!b.initialized&&uint32_t(c->r5)==1&&uint32_t(c->r6)==1&&range(p,fish_private_size)){b.private_data=p;b.initialized=true;}
   else if(live(rdram,*n)&&encounter.members[*n].purple&&b.jump_started&&f.combat_active&&admitted_controls(rdram,f)&&boss_fish_purple0260::completes_jump(rdram,c,{true,true,true,f.room,a,b.private_data,b.birth})){
    b.purple_phase=b.purple_phase==PurplePhase::Arc?PurplePhase::Vertical:PurplePhase::Arc;b.jump_started=false;
    fprintf(stderr,"[BossFishPurple] member=%u vi=%u nextJump=%s nativeReturn=1 water=0\n",*n,f.vi,b.purple_phase==PurplePhase::Arc?"arc":"vertical");
   }
  }return;
 }
 if(name=="func_8066622C"){
  if(auto n=owner(rdram,a);n&&encounter.members[*n].red&&!state.members[*n].initialized){const auto p=word(rdram,a+0x1F4);if(range(p,0x24)){state.members[*n].private_data=p;state.members[*n].initialized=true;}}
 }
 if(name=="func_80554928"&&actor(rdram,other,0x4018)&&byte(rdram,other+0x18)==0x68){
  const auto root=range(a,0x1FC)?word(rdram,a+0x180):0;
  if(auto n=owner(rdram,root);n&&encounter.family==battle::Family::Monsters&&live(rdram,*n))damage_stack.push_back({uint32_t(c->r29),root,*n,word(rdram,state.members[*n].private_data+monster_hp),state.members[*n].birth});
 }
}
inline void leave(uint8_t* rdram,const recomp_context* c,std::string_view name,const Frame& f){
 if(!rdram||!c||!state.enabled||state.invalidated||f.room!=state.room)return;
 if(name=="func_805D3444"){for(unsigned slot=0;slot<encounter.birds;++slot){auto& b=birds[slot];if(b.actor==uint32_t(c->r4)&&b.injury_stack==uint32_t(c->r29)&&b.injury_birth==b.birth){const auto birth=b.birth;bird_successor(rdram,slot,b.actor,birth,f.vi);b.injury_stack=0;b.injury_birth=0;break;}}return;}
 if(name=="func_805C74C8"){if(const auto green=green_frame(rdram,uint32_t(c->r4),f))boss_fish_green0260::after_target(rdram,c,*green);return;}
 if(name=="func_806F3A1C"){
  // The callback may have allocated private data; capture it after return.
  for(auto& [a,p]:pulses)if(actor(rdram,a,0x4018)&&range(word(rdram,a+0x1F4),0x20))p.private_data=word(rdram,a+0x1F4);
 }
 if(name!="func_80554928"||damage_stack.empty())return;
 const auto d=damage_stack.back();if(d.stack!=uint32_t(c->r29))return;damage_stack.pop_back();
 auto& b=state.members[d.member];if(b.birth!=d.birth||b.actor!=d.root||!live(rdram,d.member))return;
 const auto after=word(rdram,b.private_data+monster_hp);if(d.before>0&&after+1==d.before){b.causal=true;++b.hits;b.remaining=after;}
}
inline bool skip_native_update(uint8_t*,const recomp_context*,std::string_view,const Frame&){
 // All campaign fish now use the original red-fish jump state machine.
 return false;
}
inline bool same_bird_point(const std::array<uint32_t,3>& a,const std::array<uint32_t,3>& b){for(unsigned axis=0;axis<3;++axis)if(std::bit_cast<float>(a[axis])!=std::bit_cast<float>(b[axis]))return false;return true;}
inline bool choose_bird_spawn(unsigned slot){
 if(slot>=encounter.birds||slot>=birds.size())return false;auto& bird=birds[slot];if(!bird.positioned||!bird.option_count||bird.option_count>bird.options.size())return false;
 std::array<unsigned,8> candidates{},free{};unsigned count=0,free_count=0;
 for(unsigned p=0;p<bird.option_count;++p){const auto& point=bird.options[p];if(bird.chosen&&bird.option_count>1&&same_bird_point(point,bird.position))continue;
  candidates[count++]=p;bool occupied=false;for(unsigned other=0;other<encounter.birds;++other)if(other!=slot&&(birds[other].actor||birds[other].pending)&&birds[other].chosen&&same_bird_point(point,birds[other].position)){occupied=true;break;}
  if(!occupied)free[free_count++]=p;
 }
 if(!count)return false;const auto& choices=free_count?free:candidates;const unsigned size=free_count?free_count:count,index=choices[size==1?0:bird_random.pick(size)];bird.position=bird.options[index];bird.chosen=true;return true;
}
inline void frame(uint8_t* rdram,const Frame& f){
 if(!rdram||!state.enabled||state.invalidated||f.room!=state.room)return;
 // This identity-only maintenance is idempotent even when native culling
 // and the frame fence occur in the same VI. Timed actions still advance once.
 for(unsigned n=0;n<encounter.count;++n)preserve_fish_bounds(rdram,n,state.members[n].birth,f);
 remember_wounded_birds(rdram);
 if(have_vi&&f.vi<=last_vi)return;const unsigned elapsed=have_vi?f.vi-last_vi:1;last_vi=f.vi;have_vi=true;
 if(!f.combat_active||!admitted_controls(rdram,f))return;state.active_ticks+=elapsed;remember_candy(rdram);
 for(unsigned n=0;n<encounter.count;++n){auto& b=state.members[n];if(!b.actor||b.terminal)continue;
  if(!live(rdram,n))continue;const auto phase=byte(rdram,b.actor+0x17D),st=byte(rdram,b.actor+0x18),sub=byte(rdram,b.actor+0x30);
  if(phase==2&&st==1&&sub==1)b.active=true;
  if(encounter.family==battle::Family::Monsters){
   const auto hp=word(rdram,b.private_data+monster_hp);
   if(hp>b.initial){fail("monster_native_hp_increased");return;}
   // Native80554928 is the only combat writer of DE8. It decrements exactly
   // one after a genuine4018/state68 collision; the post-hook counts that
   // effect. A terminal HP0 alone cannot replace the configured3/4/5 hits.
   if(b.hits>b.initial||hp!=b.initial-b.hits){fail("monster_native_hp_without_causal_hit");return;}
   b.remaining=hp;
   if(b.terminal_requested&&b.active&&b.causal&&b.hits==b.initial&&!hp&&st==1&&sub==2){b.terminal=true;receipts.push_back({state.encounter,n,f.vi,b.actor,true,true,true,true});b.receipt_sent=true;}
   continue;
  }
  if(encounter.members[n].red&&b.fire_generator){
   const auto fire=b.fire_generator;
   if(!actor(rdram,fire,0x44D0)||word(rdram,fire+0x180)!=b.actor){fail("red_fire_controller_identity");return;}
   const auto p=word(rdram,fire+0x1F4);
   if(range(p,0x34)&&word(rdram,p+4)>0&&word(rdram,p+4)<=10){
    if(b.fire_private&&b.fire_private!=p){fail("red_fire_private_changed");return;}b.fire_private=p;
    const auto head=word(rdram,b.private_data);const auto player=word(rdram,0x800F8828);
    if(actor(rdram,head,0x454F)&&actor(rdram,player,0x4001)){
     const float hx=std::bit_cast<float>(word(rdram,head+0x84)),hy=std::bit_cast<float>(word(rdram,head+0x88)),rx=std::bit_cast<float>(word(rdram,b.actor+0x84)),ry=std::bit_cast<float>(word(rdram,b.actor+0x88)),px=std::bit_cast<float>(word(rdram,player+0x84));
     if(std::isfinite(hx)&&std::isfinite(hy)&&std::isfinite(rx)&&std::isfinite(ry)&&std::isfinite(px)){MEM_W(0x3C,boss_native::base(fire))=std::bit_cast<uint32_t>(hx-rx+(px>=hx?30.f:-30.f));MEM_W(0x40,boss_native::base(fire))=std::bit_cast<uint32_t>(hy-ry-10.f);MEM_W(0x44,boss_native::base(fire))=0;}
    }
    if(b.fire_pending){b.fire_pending=false;b.fire_streaming=true;b.fire_until=state.active_ticks+120;request(Operation::FireStart,n,f.vi,0x804326C4,{fire,1,1,0});}
    else if(b.fire_streaming&&state.active_ticks>=b.fire_until){b.fire_streaming=false;request(Operation::FireStop,n,f.vi,0x804326C4,{fire,1,0,0});}
   }else if(b.fire_pending&&state.active_ticks-b.fire_requested_tick>120){fail("red_fire_initialization_timeout");return;}
  }
  // Purple follows the original red-fish jump machine. Only its next
  // completed native jump selects arc versus vertical; no water emitter,
  // timed state request or frame-time velocity reset is performed here.
 }
 // Native fish only install a Yoshi contact handler. Check the original
 // collision volume of already-proven primary egg explosions against the
 // owned fish, without relying on that unrelated callback or a guessed radius.
 // fish_hit retains the shared per-burst/member mask, refractory and configured member hit ledger.
 if(encounter.family==battle::Family::Fish)for(const auto& [projectile,evidence]:pulses)if(evidence.primary_egg&&pulse(rdram,projectile))for(unsigned n=0;n<encounter.count;++n)if(encounter.members[n].kind==red_fish&&live(rdram,n)&&boss_fish_explosion0260::overlaps(rdram,projectile,state.members[n].actor).value_or(false))fish_hit(rdram,n,projectile,f.vi);
 for(auto& [projectile,evidence]:pulses)if(evidence.primary_egg&&pulse(rdram,projectile))for(unsigned slot=0;slot<encounter.birds;++slot){auto& b=birds[slot];const auto bit=uint8_t(1u<<slot);if(!(evidence.bird_mask&bit)&&!b.injury_queued&&bird_ready(rdram,slot)&&boss_fish_explosion0260::overlaps_bird(rdram,projectile,b.actor).value_or(false)){evidence.bird_mask|=bit;b.injury_queued=true;actions.push_back({Operation::BirdInjury,state.encounter,slot,f.vi,b.actor,b.birth,0x805D3444,{b.actor,projectile,uint32_t(evidence.birth),uint32_t(evidence.birth>>32)}});}}
 for(unsigned n=0;n<encounter.birds;++n){auto& bird=birds[n];if(bird.actor&&((!actor(rdram,bird.actor,0x4526)&&!actor(rdram,bird.actor,0x46DA))||byte(rdram,bird.actor+0x17D)==3))forget(bird.actor);}
 for(unsigned n=0;n<encounter.birds;++n){auto& bird=birds[n];if(!bird.actor&&!bird.pending&&state.active_ticks>=bird.next_tick&&choose_bird_spawn(n)){bird.pending=true;bird.birth=++serial;actions.push_back({Operation::BirdSpawn,state.encounter,n,f.vi,0,bird.birth,0x8004613C,{0x45262000,bird.position[0],bird.position[1],bird.position[2]}});}}
 while(encounter.family==battle::Family::RedCandy&&state.fire_rolls<state.active_ticks/1800){++state.fire_rolls;if(random.pick(100)<20){for(unsigned n=0;n<encounter.count;++n)if(encounter.members[n].red&&live(rdram,n)&&state.members[n].active&&!state.members[n].terminal){request(Operation::GenieFire,n,f.vi,0x8004613C,{state.members[n].actor,0,0,0});++state.fire_requests;}}}
}
inline std::vector<Action> take_actions(){auto result=std::move(actions);actions.clear();return result;}
inline bool set_bird_spawn_options(unsigned slot,std::span<const BirdSpawnPoint> points){
 if(!state.enabled||state.invalidated||slot>=encounter.birds||slot>=birds.size()||points.empty()||points.size()>8||birds[slot].actor||birds[slot].pending)return false;
 std::array<std::array<uint32_t,3>,8> options{};
 for(unsigned p=0;p<points.size();++p){for(unsigned axis=0;axis<3;++axis){if(!std::isfinite(points[p][axis]))return false;options[p][axis]=std::bit_cast<uint32_t>(points[p][axis]);}for(unsigned previous=0;previous<p;++previous)if(same_bird_point(options[p],options[previous]))return false;}
 auto& bird=birds[slot];bird.options=options;bird.option_count=unsigned(points.size());bird.positioned=true;if(!bird.chosen)bird.position=options[0];return true;
}
inline bool set_bird_spawn(unsigned slot,float x,float y,float z){if(!state.enabled||slot>=encounter.birds||slot>=birds.size()||!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(z))return false;auto& bird=birds[slot];bird.position={std::bit_cast<uint32_t>(x),std::bit_cast<uint32_t>(y),std::bit_cast<uint32_t>(z)};bird.options={};bird.options[0]=bird.position;bird.option_count=1;bird.positioned=true;bird.chosen=false;return true;}
inline bool renewable_actor(uint8_t* rdram,uint32_t a){if(!state.enabled||state.invalidated)return false;for(const auto& bird:birds){if(a&&bird.actor==a&&(actor(rdram,a,0x4526)||actor(rdram,a,0x46DA))&&(!bird.private_data||word(rdram,a+0x1F4)==bird.private_data)&&(!bird.resource||word(rdram,a+0x168)==bird.resource))return true;if(a&&bird.source_actor==a&&bird.source_birth&&actor(rdram,a,0x4526)&&actor(rdram,bird.actor,0x46DA)&&word(rdram,a+0x180)==bird.actor&&word(rdram,a+0x1F4)==bird.source_private&&word(rdram,bird.actor+0x1F4)==bird.private_data)return true;}return false;}
inline bool renewable_actor(uint32_t a){if(!state.enabled||state.invalidated)return false;for(const auto& bird:birds)if(a&&(bird.actor==a||bird.source_actor==a))return true;return false;}
inline bool bind_bird(uint8_t* rdram,unsigned slot,uint32_t a){if(!state.enabled||state.invalidated||slot>=encounter.birds||(!actor(rdram,a,0x4526)&&!actor(rdram,a,0x46DA)))return false;for(unsigned i=0;i<birds.size();++i)if(i!=slot&&birds[i].actor==a)return false;auto& bird=birds[slot];if(bird.actor&&bird.actor!=a)return false;bird.actor=a;bird.birth=++serial;bird.pending=false;bird.private_data=bird.resource=bird.factory=bird.source_actor=bird.source_private=bird.injury_stack=0;bird.source_birth=bird.injury_birth=0;bird.injury_queued=false;return true;}
// Native41C6 death uses447C8 directly (80554704), which permanently
// consumes its placement.445A4 first calls48CC0 and restores the spawn
// record, so it is inappropriate for a genuinely defeated campaign fish.
inline bool fish_retire_proven(const Action& a){
 if(a.member>=encounter.count||encounter.family!=battle::Family::Fish||a.function!=0x800447C8||a.arguments[0]!=a.actor||a.arguments[1]||a.arguments[2]||a.arguments[3])return false;
 const auto& m=encounter.members[a.member];const auto& b=state.members[a.member];
 return m.kind==red_fish&&m.hits>=1&&m.hits<=3&&b.actor==a.actor&&b.birth==a.birth&&a.birth&&b.initial==m.hits&&b.retire_queued&&b.initialized&&b.active&&b.causal&&b.hits==m.hits&&!b.remaining&&!b.terminal&&!b.receipt_sent;
}
inline std::vector<Action> take_actions(uint8_t* rdram){
 auto queued=take_actions();std::vector<Action> ready;
 for(const auto& a:queued){bool passenger=false;
  if(rdram&&state.enabled&&!state.invalidated&&a.encounter==state.encounter&&a.operation==Operation::Retire&&fish_retire_proven(a)&&live(rdram,a.member)&&byte(rdram,a.actor+0x17D)==2&&byte(rdram,a.actor+0x18)==1){
   const auto sub=byte(rdram,a.actor+0x30),count=word(rdram,state.members[a.member].private_data+0x40);
   passenger=sub==0xD||sub==0xE||sub==0x10||sub==0x11||sub==0x12||(count>0&&count<=120);
  }
  // A second collision in the same VI may start native Yoshi transport.
  // Keep only this valid, bounded terminal request until its passenger clears;
  // stale/malformed requests remain visible to the existing failure checks.
  if(passenger)actions.push_back(a);else ready.push_back(a);
 }
 return ready;
}
inline const char* action_invalid_reason(uint8_t* rdram,const Action& a){
 if(!state.enabled)return "disabled";if(state.invalidated)return "invalidated";if(a.encounter!=state.encounter)return "encounter";
 if(a.operation==Operation::Water||a.operation==Operation::Jump)return "disabled-native-operation";
 if(a.operation==Operation::BirdSpawn){if(a.member>=encounter.birds)return "spawn-slot";const auto& b=birds[a.member];if(!b.pending)return "spawn-not-pending";if(b.actor)return "spawn-slot-occupied";if(b.birth!=a.birth)return "spawn-birth";return nullptr;}
 if(a.operation==Operation::BirdInjury){
  if(a.member>=encounter.birds||a.member>=birds.size())return "injury-slot";
  const auto& b=birds[a.member];if(a.function!=0x805D3444||a.arguments[0]!=a.actor)return "injury-native-abi";if(b.actor!=a.actor)return "injury-actor";if(b.birth!=a.birth)return "injury-birth";if(!b.injury_queued)return "injury-not-queued";if(!bird_ready(rdram,a.member))return "injury-bird-not-ready";if(!pulse(rdram,a.arguments[1]))return "injury-primary-pulse";
  const auto& p=pulses.at(a.arguments[1]);if(p.birth!=(uint64_t(a.arguments[2])|(uint64_t(a.arguments[3])<<32)))return "injury-pulse-birth";if(!(p.bird_mask&(1u<<a.member)))return "injury-pulse-mask";if(!boss_fish_explosion0260::overlaps_bird(rdram,a.arguments[1],a.actor).value_or(false))return "injury-overlap";return nullptr;
 }
 if(a.operation==Operation::Retire){if(!fish_retire_proven(a))return "fish-terminal-proof";if(!fish_contact_ready(rdram,a.member))return "fish-contact-not-ready";}
 if(a.member>=encounter.count)return "member-slot";const auto& b=state.members[a.member];if(b.actor!=a.actor)return "member-actor";if(b.birth!=a.birth)return "member-birth";if(!live(rdram,a.member))return "member-not-live";if(byte(rdram,a.actor+0x17D)!=2)return "member-registration";return nullptr;
}
inline bool action_valid(uint8_t* rdram,const Action& a){return !action_invalid_reason(rdram,a);}
inline bool rejected_action(uint8_t* rdram,const recomp_context* c,const Action& a,const char* reason,uint32_t subject=0){
 uint64_t birth=0;uint32_t factory=0;
 const bool bird=a.operation==Operation::BirdSpawn||a.operation==Operation::BirdInjury;
 if(bird&&a.member<birds.size())birth=birds[a.member].birth;else if(a.member<state.members.size())birth=state.members[a.member].birth;
 if(bird)for(const auto& b:birds){if(range(b.factory,0x2A2C)){factory=b.factory;break;}if(!actor(rdram,b.actor,0x4526)&&!actor(rdram,b.actor,0x46DA))continue;const auto resource=word(rdram,b.actor+0x168);if(!range(resource,0x48))continue;const auto kind=MEM_HU(0x3C,boss_native::base(resource));const auto base=word(rdram,resource+0x40);if((kind==0x4526||kind==0x46DA)&&range(base,0x2A2C)&&word(rdram,resource+0x44)==base+(kind==0x4526?0x1B1C:0x1B84)){factory=base;break;}}
 return boss_variant_action_probe0281::record(rdram,c,reason,unsigned(a.operation),a.encounter,a.member,a.vi,a.function,subject?subject:a.actor,a.arguments[1],a.birth,birth,factory,state.members[0].actor,state.enabled,state.invalidated);
}
inline void action_completed(uint8_t* rdram,const Action& a,bool succeeded,unsigned vi){
 if(a.operation==Operation::BirdSpawn){if(a.member<birds.size()&&!succeeded){birds[a.member].pending=false;birds[a.member].next_tick=state.active_ticks+ticks(encounter.bird_respawn_ms);}return;}if(a.operation!=Operation::Retire||a.member>=encounter.count)return;auto& b=state.members[a.member];
 if(a.actor!=b.actor||a.birth!=b.birth||!b.retire_queued)return;
 if(!fish_retire_proven(a)){fail("fish_retirement_without_terminal_proof");return;}
 if(!succeeded||!actor(rdram,b.actor,encounter.members[a.member].kind)||byte(rdram,b.actor+0x17D)!=3||byte(rdram,b.actor+0x18)!=0xFF){fail("fish_native_retirement_failed");return;}
 b.terminal=true;if(!b.receipt_sent&&b.initialized&&b.active&&b.causal&&b.hits==b.initial&&!b.remaining){receipts.push_back({state.encounter,a.member,vi,b.actor,true,true,true,true});b.receipt_sent=true;}
}
inline bool acknowledge_defeat(const battle::NativeReceipt& receipt){
 if(!state.enabled||state.invalidated||receipt.encounter!=state.encounter||receipt.member>=encounter.count||!receipt.actor||!receipt.vi||!receipt.initialized||!receipt.active||!receipt.terminal||!receipt.causal_hit)return false;
 auto& b=state.members[receipt.member];if(b.actor!=receipt.actor||!b.birth)return false;b.initialized=b.active=b.causal=b.terminal=true;b.remaining=0;b.receipt_sent=true;return true;
}
inline std::vector<battle::NativeReceipt> take_receipts(){auto result=std::move(receipts);receipts.clear();return result;}
// Explicit native action execution supplied by the router's dispatch lambda.
// It must dispatch the named original function, not this adapter recursively.
template<class Dispatch> inline bool execute(uint8_t* rdram,recomp_context* c,const Action& a,Dispatch&& native){
 if(!rdram||!c)return rejected_action(rdram,c,a,"missing-native-context");if(const auto reason=action_invalid_reason(rdram,a))return rejected_action(rdram,c,a,reason);const auto saved=*c;
 if(!range(uint32_t(saved.r29)-0xC0,0x100))return rejected_action(rdram,c,a,"native-stack");c->r29=ADD32(saved.r29,-0xC0);
 if(a.operation==Operation::GenieFire){auto& b=state.members[a.member];bool ok=true;if(!b.fire_generator){
  if(MEM_HU(0x184,boss_native::base(a.actor))>=20){rejected_action(rdram,c,a,"genie-child-capacity");*c=saved;return false;}
  c->r4=0x44D0;native(0x80047090,rdram,c);const auto id=uint32_t(c->r2);if(id>=0x3FF)ok=false;
  else {c->r4=0x44D00000u|id;c->r5=c->r6=c->r7=0;native(0x8004613C,rdram,c);const auto fire=uint32_t(c->r2);if(!actor(rdram,fire,0x44D0)||word(rdram,fire+0x180))ok=false;else{c->r4=boss_native::base(a.actor);c->r5=boss_native::base(fire);native(0x80047F9C,rdram,c);ok=word(rdram,fire+0x180)==a.actor;if(ok){b.fire_generator=fire;MEM_B(0x17C,boss_native::base(fire))=0;}}}
 }else ok=actor(rdram,b.fire_generator,0x44D0)&&word(rdram,b.fire_generator+0x180)==a.actor;
 if(ok){b.fire_pending=true;b.fire_requested_tick=state.active_ticks;}else rejected_action(rdram,c,a,"genie-fire-native-result");*c=saved;return ok;}
 if(a.operation==Operation::FireStart||a.operation==Operation::FireStop){const auto& b=state.members[a.member];const auto fire=a.arguments[0];if(fire!=b.fire_generator||!actor(rdram,fire,0x44D0)||word(rdram,fire+0x180)!=a.actor||word(rdram,fire+0x1F4)!=b.fire_private){rejected_action(rdram,c,a,"fire-controller-identity",fire);*c=saved;return false;}}
 if(a.operation==Operation::BirdSpawn){c->r4=0x4526;native(0x80047090,rdram,c);const auto id=uint32_t(c->r2);bool ok=false;if(id<0x3FF){c->r4=boss_native::base(0x45262000u|id);c->r5=a.arguments[1];c->r6=a.arguments[2];c->r7=a.arguments[3];native(0x8004613C,rdram,c);const auto bird=uint32_t(c->r2);if(actor(rdram,bird,0x4526)){c->r4=boss_native::base(bird);native(0x80044194,rdram,c);ok=bind_bird(rdram,a.member,bird);if(!ok)rejected_action(rdram,c,a,"yellow-bird-binding",bird);}else rejected_action(rdram,c,a,"yellow-bird-allocation",bird);}else rejected_action(rdram,c,a,"yellow-bird-instance");*c=saved;return ok;}
 if(a.operation==Operation::BirdInjury){
  const auto ticket=boss_bird_null0281::capture(rdram,a.actor,a.birth,birds[a.member].factory,uint32_t(c->r29));
  c->r4=boss_native::base(a.actor);c->r5=boss_native::base(a.arguments[1]);c->r6=c->r7=0;native(0x805D3444,rdram,c);
  const bool ok=bird_successor(rdram,a.member,a.actor,a.birth,a.vi);
  const bool deferred=!ok&&ticket&&birds[a.member].actor==a.actor&&birds[a.member].birth==a.birth&&birds[a.member].injury_queued&&boss_bird_null0281::completed(rdram,c,*ticket);
  if(deferred&&!bird_null_logged){bird_null_logged=true;std::fprintf(stderr,"[BossBirdInjury] deferred-native-null actor=%08X birth=%llu slot=%u vi=%u provider=%08X capacity=5 retiredNative447C8=1 noSuccessor=1 scoreWrites=0\n",a.actor,(unsigned long long)a.birth,a.member,a.vi,ticket->factory+0x2A20);}
  if(!ok&&!deferred)rejected_action(rdram,c,a,"wounded-bird-successor");*c=saved;return ok||deferred;
 }
 if(a.operation==Operation::Jump){const auto p=state.members[a.member].private_data;MEM_W(0x4C,boss_native::base(p))=0;for(unsigned off:{0x18u,0x1Cu,0x20u,0x24u,0x28u,0x2Cu})MEM_W(off,boss_native::base(p))=0;MEM_W(0x50,boss_native::base(a.actor))=std::bit_cast<uint32_t>(180.f);}
 c->r4=boss_native::base(a.arguments[0]);c->r5=a.arguments[1];c->r6=a.arguments[2];c->r7=a.arguments[3];native(a.function,rdram,c);//447C8 has no defined V0 result. Its actual native retirement state is
 // the success witness, followed by action_completed's identical phase guard.
 const bool ok=a.operation!=Operation::Retire||(actor(rdram,a.actor,encounter.members[a.member].kind)&&word(rdram,a.actor+0x1F4)==state.members[a.member].private_data&&byte(rdram,a.actor+0x17D)==3&&byte(rdram,a.actor+0x18)==0xFF);if(!ok)rejected_action(rdram,c,a,"native-retirement-result");*c=saved;return ok;
}
inline uint16_t tint(uint16_t v,bool purple){
 unsigned red=(v>>11)&31,green=(v>>6)&31,blue=(v>>1)&31;const unsigned hi=std::max({red,green,blue}),lo=std::min({red,green,blue});
 if(hi<5||hi-lo<5)return v; // preserve outlines, white eyes and neutral shading
 if(purple){red=hi;green=(lo+hi)/3;blue=hi;}else{red=hi;green=lo/2;blue=lo/2;}
 return uint16_t((red<<11)|(green<<6)|(blue<<1)|(v&1));
}
inline uint16_t tint_green(uint16_t v){
 const unsigned red=(v>>11)&31,green=(v>>6)&31,blue=(v>>1)&31,hi=std::max({red,green,blue}),lo=std::min({red,green,blue});
 if(hi<5||hi-lo<5)return v;
 return uint16_t((green<<11)|(hi<<6)|(blue<<1)|(v&1));
}
inline bool palette_begin(uint8_t* rdram,uint32_t a,bool fish_draw=false,const Frame& f={}){
 if(!rdram||!state.enabled||state.invalidated||paint_stack.size()>=16)return false;bool purple=false,red=false,flash=false,green=false;
 if(auto n=owner(rdram,a);n&&encounter.family==battle::Family::Fish){purple=encounter.members[*n].purple;green=encounter.members[*n].green;const auto& b=state.members[*n];flash=b.birth&&b.hits&&b.causal&&live(rdram,*n)&&byte(rdram,a+0x17D)==2&&boss_fish_flash0260::alternate(state.active_ticks-b.last_hit_tick);}
 if(candy_owner(rdram,a))red=!yellow();if(!purple&&!red&&!flash&&!green)return false;
 boss_fish_green0260::DrawTicket draw{};if(fish_draw&&(green||purple))if(const auto witness=green_frame(rdram,a,f))draw=boss_fish_green0260::draw_begin(rdram,*witness);
 auto angle_only=[&](){if(!draw.active)return false;PaintSwap p;p.green_draw=draw;paint_stack.push_back(p);return true;};
 const auto resource=range(a,0x1FC)?word(rdram,a+0x168):0,drawing=range(a,0x1FC)?word(rdram,a+0x16C):0;
 if(!range(resource,0x90)||!range(drawing,0xC))return angle_only();const auto source=word(rdram,resource+0x88);if(!range(source,512))return angle_only();
 if(source>=palette_base&&source<palette_end)return angle_only();const uint64_t key=(uint64_t(source)<<4)|unsigned(purple)|(unsigned(red)<<1)|(unsigned(flash)<<2)|(unsigned(green)<<3);auto found=palette_slots.find(key);uint32_t replacement=0;if(found!=palette_slots.end())replacement=found->second;else{if(palette_cursor>=32)return angle_only();replacement=palette_base+512*palette_cursor++;palette_slots.emplace(key,replacement);}
 if(replacement+512>palette_end)return angle_only();
 for(unsigned i=0;i<256;++i){const auto original=uint16_t(MEM_HU(int(i*2),boss_native::base(source)));const auto colored=green?tint_green(original):purple||red?tint(original,purple):original;MEM_H(int(i*2),boss_native::base(replacement))=boss_fish_flash0260::color(colored,flash);}
 paint_stack.push_back({resource,source,replacement,drawing,word(rdram,drawing+8),draw});MEM_W(0x88,boss_native::base(resource))=replacement;MEM_W(8,boss_native::base(drawing))=replacement;return true;
}
inline void palette_finish(uint8_t* rdram){if(!rdram||paint_stack.empty())return;const auto p=paint_stack.back();paint_stack.pop_back();if(range(p.resource,0x90)&&word(rdram,p.resource+0x88)==p.replacement)MEM_W(0x88,boss_native::base(p.resource))=p.original;if(range(p.drawing,0xC)&&word(rdram,p.drawing+8)==p.replacement)MEM_W(8,boss_native::base(p.drawing))=p.drawing_original;if(p.green_draw.active)if(const auto n=owner(rdram,p.green_draw.frame.actor);n&&state.members[*n].birth==p.green_draw.frame.birth)boss_fish_green0260::draw_end(rdram,p.green_draw);}
inline const Snapshot& snapshot(){return state;}
} // namespace workshop::boss_variants0250
