#pragma once
#include "boss_battle0250.hpp"
#include "boss_family_native0250.hpp"
#include "boss_blargg0289.hpp"
#include "boss_variants0250.hpp"
#include "boss_rooms_data0250.hpp"
#include "boss_solo_runtime0240.hpp"
#include "boss_carry0260.hpp"
#include "boss_placement0260.hpp"
#include "boss_retry_placements0281.hpp"
#include "boss_pot_aim0282.hpp"
#include "boss_native_mouth0260.hpp"
#include "boss_rest_aim0260.hpp"
#include "boss_tree_hybrid0260.hpp"
#include "boss_rest_heart0260.hpp"
#include "boss_checkpoint_actor0260.hpp"
#include "boss_checkpoint_inventory0260.hpp"
#include "boss_floor_persistence0260.hpp"
#include "boss_rest_floor0260.hpp"
#include "boss_monster_exit0260.hpp"
#include "boss_fish_exit0260.hpp"
#include "boss_fish_companion_intro0260.hpp"
#include "boss_run_travel0260.hpp"
#include "boss_candy_pool0260.hpp"
#include "boss_lips_candy_entry0262.hpp"
#include <atomic>
#include <cstdio>
extern "C" void func_8006D6F8(uint8_t*,recomp_context*);
extern "C" void func_8006D34C(uint8_t*,recomp_context*);
extern "C" void func_80038F98(uint8_t*,recomp_context*);
extern "C" void func_8004700C(uint8_t*,recomp_context*);
namespace workshop::boss_campaign0250 {
namespace core=boss_battle0250;namespace native=boss_native;namespace witnesses=boss_family_native0250;namespace variants=boss_variants0250;namespace rooms=boss_rooms_data0250;
namespace checkpoints {
void configure(bool enabled,bool diagnostic);void enter_room(unsigned room);void forget(uint32_t actor);
void trace_carry(uint8_t*,const recomp_context*,std::string_view);bool capture_baseline(uint8_t*,unsigned);
bool notice_death(uint8_t*,unsigned);bool waiting_for_retry();bool restore_arrival(uint8_t*,recomp_context*,unsigned);
std::optional<boss_rest_floor0260::SourceProof> rest_source(uint8_t*);
}
struct FightReadyWitness {unsigned vi=0,index=10,room=177,source=177,epoch=0;uint32_t player=0,private_data=0;};inline FightReadyWitness fight_ready{};
inline bool fish_intro_hold_logged=false,fish_intro_release_logged=false,fish_intro_contact_logged=false;
inline bool enabled=false,driver=false;inline core::Session session;inline unsigned physical_room=177,logical_room=177,current_vi=0,arrival_vi=0,rest_vi=0;
inline boss_placement0260::Guard placement_guard;
inline boss_retry_placements0281::Guard retry_placements;
inline boss_blargg0289::Observer blargg_observer;
inline std::array<uint64_t,2> blargg_active_births{};
inline boss_rest_heart0260::Bonus rest_heart;
inline boss_rest_aim0260::Controller rest_controller,candy_controller;
struct CandyFoodTarget {
 uint32_t root=0,root_private=0,head=0,head_private=0,actor=0,private_data=0,metadata=0,definition=0,resource=0;
 uint64_t birth=0;unsigned room=177,encounter=10,member=4,slot=12;
};inline CandyFoodTarget candy_food;
inline bool candy_cycle_live=false,candy_have_log_vi=false;inline unsigned candy_log_vi=0;inline uint16_t candy_previous_buttons=0;
inline std::array<uint32_t,4> bound{};inline std::array<witnesses::MemberObserver,4> observers{};inline std::array<witnesses::CandyMemberObserver,4> candies{};
inline std::array<unsigned,4> observer_epochs{};inline unsigned variant_epoch=0;
inline std::array<uint32_t,300> overlays{};inline uint32_t ready_info=0,exit_pot=0;inline float exit_x=0;
inline std::optional<boss_monster_exit0260::Support> monster_exit_support;
inline unsigned monster_exit_log_count=0;inline int monster_exit_log_stage=-1;
inline boss_fish_exit0260::Controller fish_exit_controller;inline int fish_exit_log_stage=-1;
inline std::array<bool,4> scaled{};inline std::vector<std::pair<uint32_t,unsigned>> scale_scopes;
inline boss_floor_persistence0260::Guard floor_guard;
inline boss_rest_floor0260::Guard rest_floor_guard;inline unsigned rest_floor_encounter=10;
inline unsigned flags5=0,flags6=1;inline bool flags_ready=false,restore_pending=false;
struct VictoryRecord {unsigned color=18;uint32_t player=0;std::array<core::NativeReceipt,4> members{};};
inline std::array<VictoryRecord,10> history{};inline uint32_t first_player=0;inline unsigned first_color=18;
inline void(*completion_hook)(uint8_t*)=nullptr;inline bool completion_pending=false;
inline std::atomic_bool finished=false,failed=false;inline std::atomic<unsigned> revealed{0};
struct Cargo {unsigned hp=0,color=0,fruits=0,pc_color=0;std::array<uint32_t,20> children{};std::array<uint32_t,6> eggs{};boss_carry0260::Payload stats;bool captured=false;};inline Cargo cargo;
inline unsigned cargo_epoch=0,stats_reset_epoch=0,stats_restored_epoch=0;inline bool stats_reset_pending=false;
inline constexpr uint32_t route_address=0x807DFF00;
inline void fail(const char* why){if(session.snapshot().phase!=core::Phase::Failed)fprintf(stderr,"[BossBattle] failed encounter=%u room=%u reason=%s\n",session.snapshot().index+1,physical_room,why);session.cancel(why);failed=true;}
// Empty flower HP0 is still playable; only native fatal stateA branches
// count as death. A/2 belongs to owned pot transport and is excluded here.
inline bool fatal_player(uint8_t* rdram,uint32_t player){if(!native::actor(rdram,player,0x4001)||MEM_BU(0x18,native::base(player))!=0xA)return false;const auto sub=MEM_BU(0x30,native::base(player));return sub==0xA||sub==0xB||sub==0xC||sub==0xD||sub==0x10||sub==0x11;}
inline bool active(){const auto p=session.snapshot().phase;return enabled&&p!=core::Phase::Failed&&p!=core::Phase::Complete;}
inline bool blargg_owned(){
 const auto& s=session.snapshot();const auto phase=s.phase;
 const bool entering=phase==core::Phase::ToFight||phase==core::Phase::RetryToFight||phase==core::Phase::Fighting||phase==core::Phase::Victory||phase==core::Phase::Armed;
 return active()&&entering&&session.plan().config.blargg&&s.index>=1&&s.index<=8&&physical_room==167+s.index&&logical_room==19&&session.current().family==core::Family::Blargg&&core::valid_blargg(session.current(),session.plan().config);
}
inline boss_blargg0289::Frame blargg_frame(uint8_t* rdram=nullptr,bool completed_frame=false){
 if(!blargg_owned())return {};
 const auto& s=session.snapshot();
 const bool fighting=rdram&&s.phase==core::Phase::Fighting&&fight_ready.index==s.index&&fight_ready.room==physical_room&&fight_ready.source==19&&fight_ready.epoch==s.retry_epoch&&fight_ready.player==s.player&&native::actor(rdram,s.player,0x4001)&&native::word(rdram,0x800F8828)==s.player&&!fatal_player(rdram,s.player);
 const bool quiescent=completed_frame&&scale_scopes.empty()&&variants::paint_stack.empty()&&variants::damage_stack.empty();
 return {physical_room,logical_room,s.index,s.retry_epoch,current_vi,overlays[90],s.player,fighting,quiescent};
}
// Loader calls these once at the real45CAC /439F4 /45B64 entries AFTER
// trace() refreshed currentVI/overlays, and before the original native body.
inline void blargg_created(uint32_t actor){if(blargg_owned())blargg_observer.created(actor,blargg_frame());}
inline void blargg_forgotten(uint32_t actor){if(blargg_owned()){blargg_observer.forget(actor);if(blargg_observer.snapshot().invalidated)fail(blargg_observer.snapshot().failure.c_str());}}
inline bool blargg_active_member(uint8_t* rdram,unsigned member){
 if(!rdram||!blargg_owned()||member>=2)return false;
 const auto& s=session.snapshot();const auto& proof=blargg_observer.snapshot();const auto& m=proof.members[member];const auto& binding=variants::snapshot().members[member];
 if(!proof.enabled||proof.invalidated||!m.initialized||m.retired||!m.identity.birth||blargg_active_births[member]!=m.identity.birth||blargg_observer.birth(m.identity.actor)!=m.identity.birth||binding.actor!=m.identity.actor||binding.birth!=m.variant_birth||!binding.initialized||binding.private_data!=m.identity.private_data||observer_epochs[member]!=s.retry_epoch||(s.defeated&(1u<<member))||bound[member]!=m.identity.actor)return false;
 const auto actor=m.identity.actor;
 return blargg_observer.initialized_member(rdram,member)&&native::byte(rdram,actor+0x18)==1;
}

inline boss_candy_pool0260::Guard candy_pool;
inline boss_lips_candy_entry0262::Guard lips_candy_entry;
inline boss_candy_pool0260::Frame candy_pool_frame(uint32_t overlay){
 const auto& s=session.snapshot();const auto& e=session.current();const bool entering=s.phase==core::Phase::Armed||s.phase==core::Phase::ToFight||s.phase==core::Phase::RetryToFight||s.phase==core::Phase::Fighting;
 if(!active()||!entering||logical_room!=162||physical_room!=(s.index?167+s.index:162)||s.index>=9||e.family!=core::Family::Candy||(e.count!=2&&e.count!=3))return {};
 for(unsigned i=0;i<e.count;++i)if(e.members[i].kind!=0x455A||e.members[i].hits!=12)return {};
 return {true,logical_room,physical_room,s.index,e.count,overlay};
}
inline boss_lips_candy_entry0262::Frame lips_candy_entry_frame(){
 const auto& s=session.snapshot();const auto& e=session.current();const auto& v=variants::snapshot();
 const bool entering=s.phase==core::Phase::Armed||s.phase==core::Phase::ToFight||s.phase==core::Phase::RetryToFight||s.phase==core::Phase::Fighting;
 if(!active()||!entering||s.index<1||s.index>8||logical_room!=165||physical_room!=167+s.index||e.family!=core::Family::Lips||e.variant!=1||e.count!=2||e.members[0].kind!=0x40A3||e.members[1].kind!=0x455A||e.members[1].hits!=12||e.members[1].scale!=session.plan().config.two_candy_scale||!v.enabled||v.invalidated||v.encounter!=s.index||v.room!=165||variant_epoch!=s.retry_epoch||observer_epochs[1]!=s.retry_epoch||(s.defeated&2))return {};
 const auto& proof=candies[1].snapshot();const auto& a=proof.actors[0];const auto& birth=v.members[1];
 if(!bound[1]||birth.actor!=bound[1]||!birth.birth||birth.receipt_sent||birth.terminal||!proof.enabled||proof.invalidated||proof.source_room!=165||proof.run_token!=s.token||proof.registered_actors!=1||!a.initialized||a.actor!=bound[1]||a.completion_returned||a.disposed)return {};
 return {true,true,165,physical_room,s.index,s.retry_epoch,1,2,1,bound[1],overlays[167],birth.birth,s.token};
}
inline boss_rest_heart0260::Frame rest_heart_frame(){
 const auto& state=session.snapshot();return {physical_room,std::max(state.completed,state.max_completed),current_vi,state.player,physical_room==83&&state.phase==core::Phase::Rest};
}
inline boss_tree_hybrid0260::Frame hybrid_tree_frame(){
 const auto& tree=boss_tree0240::snapshot();
 return {active(),physical_room==83,physical_room,current_vi,tree.tree,tree.privateData,overlays[293],overlays[126]};
}
// Native GenieC774/Lips586CC/InvisoF640 setF7EA6=0; their later scripted
// sequences may write1. A proved active fight can remain playable at0/0.
// Share this read-only live witness with diagnostic input admission. Saved
// controls and pending restoration are caller-owned, not prerequisites here.
// The public resource getter may wrap the internal initializer observed by
// MemberObserver. ADD40 returns AF5F8, which calls AF580; confusing those
// addresses loses a live Inviso witness after its companion is defeated.
inline constexpr uint32_t member_resource_constructor(const witnesses::Spec& cfg){return cfg.kind==0x4599?0x806AF5F8:cfg.init;}
inline bool witnessed_zero_controls(uint8_t* rdram,uint32_t player){
 const auto& s=session.snapshot();const auto& v=variants::snapshot();
 if(!rdram||!active()||s.phase!=core::Phase::Fighting||player!=s.player||native::byte(rdram,0x800F7EA5)||native::byte(rdram,0x800F7EA6)||!witnesses::active_player(rdram,player)||native::word(rdram,0x800F8828)!=player||native::byte(rdram,player+0x17D)!=2||native::byte(rdram,player+0x165)||!native::byte(rdram,player+0x18)||native::byte(rdram,player+0x18)==0xFF)return false;
 const auto p=native::word(rdram,player+0x1F4);
 if(!native::range(p,0x12F0)||native::word(rdram,p+0x12EC)||!v.enabled||v.invalidated||v.encounter!=s.index||v.room!=logical_room||core::room(session.current().family)!=logical_room||variant_epoch!=s.retry_epoch)return false;
 if(blargg_owned())for(unsigned n=0;n<2;++n)if(blargg_active_member(rdram,n))return true;
 for(unsigned n=0;n<session.current().count&&n<4;++n){
  const auto kind=session.current().members[n].kind;const auto family=witnesses::family_for_kind(kind);if(!family||(s.defeated&(1u<<n)))continue;
  const auto& cfg=witnesses::spec(*family);const auto& birth=v.members[n];const auto& observer=observers[n];const auto& proof=observer.snapshot();const auto& a=proof.actors[0];const auto root=bound[n];
  // Companions use their configured encounter source (Genie in Candy,
  // Inviso in Lips), while keeping their own native overlay and resource.
  if(!root||birth.actor!=root||!birth.birth||birth.receipt_sent||birth.retire_queued||observer_epochs[n]!=s.retry_epoch||observer.bound_actor()!=root||!proof.enabled||proof.invalidated||proof.family!=*family||proof.source_room!=logical_room||proof.run_token!=s.token||proof.expected_actors!=1||proof.registered_actors!=1||proof.expected_hits!=session.current().members[n].hits||!a.initialized||!a.active_seen||a.disposed||a.completion_returned||a.actor!=root||!native::actor(rdram,root,kind)||native::byte(rdram,root+0x17D)!=2||!native::range(a.private_data,cfg.private_size)||native::word(rdram,root+0x1F4)!=a.private_data)continue;
  const auto resource=native::word(rdram,root+0x168),overlay=overlays[cfg.overlay_section],init_offset=member_resource_constructor(cfg)-cfg.rom_base;
  if(native::range(resource,0x48)&&native::range(overlay,init_offset+4)&&native::word(rdram,resource+0x40)==overlay&&native::word(rdram,resource+0x44)==overlay+init_offset)return true;
 }
 return false;
}
// Remember the actual pair after native activity, never an intro/cargo state.
inline void remember_controls(uint8_t* rdram,core::Phase phase){
 if(!rdram||phase!=core::Phase::Fighting||restore_pending)return;const auto player=session.snapshot().player;
 if(!witnesses::active_player(rdram,player)||native::word(rdram,0x800F8828)!=player||native::byte(rdram,player+0x17D)!=2||native::byte(rdram,player+0x165))return;
 const auto current5=native::byte(rdram,0x800F7EA5),current6=native::byte(rdram,0x800F7EA6);
 const bool playable=current6==1||(!current5&&!current6&&witnessed_zero_controls(rdram,player));
 if(playable){flags5=current5;flags6=current6;flags_ready=true;}
}
inline unsigned source_for(unsigned index){return rooms::source_for_encounter(index);}
inline unsigned effective_room(unsigned raw){return enabled&&raw==52?source_for(session.snapshot().index):raw;}
inline witnesses::Frame witness_frame(unsigned member){const auto kind=session.current().members[member].kind;unsigned section=kind==0x455A?167:kind==0x4072?16:kind==0x4599?197:120;return {logical_room,current_vi,overlays[section],overlays[285]};}
inline variants::Frame variant_frame(uint8_t* rdram){const auto& s=session.snapshot();const bool fighting=s.phase==core::Phase::Fighting;return {logical_room,current_vi,fighting,fighting&&witnessed_zero_controls(rdram,s.player)};}
inline bool configure(std::string token,uint64_t seed,bool diagnostic,bool drive,int diagnostic_entry=-1,bool special_fish=false,bool automatic_test=false,bool enable_blargg=false){
 blargg_observer.disable();blargg_active_births={};
 monster_exit_support.reset();fish_exit_controller.reset();fish_exit_log_stage=-1;
 monster_exit_log_count=0;monster_exit_log_stage=-1;
 core::Config plan_config;plan_config.blargg=enable_blargg;auto plan=core::make_plan(seed,plan_config);
 if(automatic_test&&(!diagnostic||!drive||diagnostic_entry!=4||seed!=1)){enabled=false;return false;}
 if(special_fish){
  if(!diagnostic||(drive&&!automatic_test)||diagnostic_entry!=4||seed!=1||plan.fights[4].family!=core::Family::Fish){enabled=false;return false;}
  auto& fish=plan.fights[4];fish.variant=1;fish.count=2;fish.purple_mask=2;fish.genie_companion=false;fish.members={};
  fish.members[0]={0x4511,core::fish_hits,1,false,false,true};fish.members[1]={0x4511,core::fish_hits,1,false,true,false};
 }
 enabled=session.begin(std::move(token),plan,diagnostic);if(enabled&&diagnostic_entry>=0)enabled=session.diagnostic_entry(unsigned(diagnostic_entry));driver=drive;if(enabled&&special_fish)fprintf(stderr,"[BossFishTest] colors=green,purple index=4 birds=%u respawnMs=%u\n",session.plan().fights[4].birds,session.plan().fights[4].bird_respawn_ms);lips_candy_entry.reset();placement_guard.reset();retry_placements.reset();floor_guard.reset();rest_floor_guard.reset();rest_floor_encounter=10;physical_room=logical_room=177;current_vi=arrival_vi=rest_vi=0;fight_ready={};fish_intro_hold_logged=fish_intro_release_logged=fish_intro_contact_logged=false;ready_info=exit_pot=0;bound={};observer_epochs={};variant_epoch=0;overlays={};flags_ready=restore_pending=false;finished=false;failed=false;revealed=0;completion_pending=false;cargo={};cargo_epoch=stats_reset_epoch=stats_restored_epoch=0;stats_reset_pending=false;history={};first_player=0;first_color=18;boss_tree0240::configure(enabled);boss_tree_hybrid0260::configure(enabled,seed);rest_heart.configure(enabled);variants::disable();rest_controller.configure(enabled&&drive,diagnostic);candy_controller.configure(enabled&&drive,diagnostic);candy_food={};candy_cycle_live=candy_have_log_vi=false;candy_log_vi=candy_previous_buttons=0;checkpoints::configure(enabled,diagnostic);return enabled;
}
inline void prepare_encounter(){
 blargg_observer.disable();blargg_active_births={};
 monster_exit_support.reset();fish_exit_controller.reset();fish_exit_log_stage=-1;
 monster_exit_log_count=0;monster_exit_log_stage=-1;
 candy_controller.configure(enabled&&driver,session.snapshot().diagnostic);candy_food={};candy_cycle_live=candy_have_log_vi=false;candy_log_vi=candy_previous_buttons=0;
 lips_candy_entry.reset();
 placement_guard.reset(physical_room,session.snapshot().index);bound={};observers={};candies={};scaled={};scale_scopes.clear();exit_pot=ready_info=0;arrival_vi=0;fight_ready={};fish_intro_hold_logged=fish_intro_release_logged=fish_intro_contact_logged=false;flags_ready=restore_pending=false;
 floor_guard.reset(physical_room,session.snapshot().index);
 observer_epochs.fill(session.snapshot().retry_epoch);variant_epoch=session.snapshot().retry_epoch;
 const auto& e=session.current();logical_room=core::room(e.family);
 if(e.family==core::Family::Blargg&&!blargg_observer.configure({session.snapshot().token,session.snapshot().index,physical_room,session.snapshot().retry_epoch,session.snapshot().diagnostic})){fail("blargg_observer_configuration");return;}
 for(unsigned n=0;n<e.count;++n){bool ok=true;if(e.members[n].kind==0x455A)ok=candies[n].configure(e,n,session.snapshot().token,logical_room,session.snapshot().diagnostic);else if(witnesses::family_for_kind(e.members[n].kind))ok=observers[n].configure(e,n,session.snapshot().token,logical_room,session.snapshot().diagnostic);if(!ok)fail("member_observer_configuration");}
 if(!variants::begin(e,session.snapshot().index,session.plan().config,session.plan().seed^session.snapshot().index))fail("variant_configuration");
 const auto& arena=rooms::arena(physical_room);
 if(e.family==core::Family::Blargg){
  if(arena.logical_source!=19||e.birds!=2){fail("blargg_bird_arena_rejected");return;}
  const std::array<variants::BirdSpawnPoint,3> points{{{224.f,104.f,500.34f},{288.f,104.f,500.34f},{256.f,88.f,500.34f}}};
  for(unsigned n=0;n<e.birds;++n)if(!variants::set_bird_spawn_options(n,points)){fail("blargg_bird_spawn_options_rejected");return;}
 }else if(e.family==core::Family::Fish){
  if(arena.safe_pot_xs.size()!=3){fail("fish_bird_platforms_rejected");return;}
  std::array<variants::BirdSpawnPoint,6> points{};
  for(unsigned i=0;i<3;++i){points[i*2]={arena.safe_pot_xs[i]-24.f,std::max(48.f,arena.entry_y-72.f),500.34f};points[i*2+1]={arena.safe_pot_xs[i]+24.f,std::max(48.f,arena.entry_y-56.f),500.34f};}
  for(unsigned n=0;n<e.birds;++n)if(!variants::set_bird_spawn_options(n,points)){fail("fish_bird_spawn_options_rejected");return;}
 }else if(e.family==core::Family::Genie||e.family==core::Family::Invisible){
  const bool genie=e.family==core::Family::Genie;
  if(arena.logical_source!=(genie?163u:164u)||!e.birds){fail("native_bird_delivery_source_rejected");return;}
  // Real f706/58A8 bitmaps leave these columns clear. Genie birds must be
  // above its actual continuous256..415 support, not remote egg blocks.
  // Native4526 oscillates around its birthY; no motion/physics is changed.
  const float left=genie?304.f:300.f,right=genie?368.f:348.f,y=genie?624.f:280.f;
  for(unsigned n=0;n<e.birds;++n){const float fraction=e.birds>1?float(n)/float(e.birds-1):.5f;
   if(!variants::set_bird_spawn(n,left+(right-left)*fraction,y,500.34f)){fail("native_bird_delivery_point_rejected");return;}
  }
 }else for(unsigned n=0;n<e.birds;++n){const float x=arena.min_x+(arena.max_x-arena.min_x)*float(n+1)/float(e.birds+1);variants::set_bird_spawn(n,x,arena.logical_source==162?280.f:std::max(48.f,arena.entry_y-128.f),500.34f);}
 revealed=session.snapshot().index+1;fprintf(stderr,"[BossBattle] arena-entered fight=%u family=%u variant=%u members=%u terrain=%u room=%u\n",e.number,unsigned(e.family),e.variant,e.count,e.terrain,physical_room);
}
inline void enter_room(unsigned raw){
 monster_exit_support.reset();fish_exit_controller.reset();fish_exit_log_stage=-1;
 if(!enabled)return;const auto phase=session.snapshot().phase;if(phase==core::Phase::Armed&&raw!=52)return;physical_room=effective_room(raw);ready_info=0;
 if(physical_room==boss_rest_data0240::source_room){if(phase!=core::Phase::ToRest&&phase!=core::Phase::RetryToRest){fail("unexpected_rest_room");return;}logical_room=physical_room;variants::disable();blargg_observer.disable();blargg_active_births={};}
 else {if((phase!=core::Phase::Armed&&phase!=core::Phase::ToFight&&phase!=core::Phase::RetryToFight)||physical_room!=source_for(session.snapshot().index)){fail("unexpected_boss_room");return;}prepare_encounter();}
 rest_floor_encounter=physical_room==83?session.snapshot().index:10;rest_floor_guard.reset(physical_room,rest_floor_encounter);boss_tree0240::enter_room(physical_room);rest_heart.enter_room(physical_room);checkpoints::enter_room(physical_room);
}
inline void room_ready(uint8_t* rdram,const recomp_context* c){if(enabled&&native::range(uint32_t(c->r4),0x128)&&effective_room(MEM_BU(1,c->r4))==physical_room)ready_info=uint32_t(c->r4);}
inline uint16_t init_kind(std::string_view name){if(name=="func_80664E18")return 0x455A;if(name=="func_8042D33C")return 0x4072;if(name=="func_806AF580")return 0x4599;if(name=="func_8055863C")return 0x40A3;if(name=="func_805C9A98")return 0x4511;if(name=="func_80551518")return 0x41C6;if(name=="func_804F42B8")return 0x41EC;return 0;}
inline void bind_actor(uint8_t* rdram,uint32_t actor,uint16_t kind){
 if(!active()||physical_room==83||!native::actor(rdram,actor,kind))return;for(auto a:bound)if(a==actor)return;const auto& e=session.current();
 auto placed=boss_placement0260::supported(kind)?placement_guard.member(actor):std::optional<unsigned>{};
 if(kind==0x41EC){
  if(!blargg_owned()){fail("blargg_constructor_outside_owned_arena");return;}
  const auto& captured=retry_placements.snapshot()[session.snapshot().index];const auto record=native::word(rdram,actor+0x1D8),packed=native::word(rdram,actor+0x1DC);
  if(captured.captured&&captured.source==physical_room&&captured.count==2)for(unsigned n=0;n<2;++n)if(captured.placements[n].record==record&&captured.placements[n].packed==packed&&packed==(uint32_t(kind)<<16|n))placed=n;
  if(!placed){fail("blargg_constructor_without_pristine_placement");return;}
 }
 if(boss_placement0260::supported(kind)&&!placed){fail("boss_placement_constructor_without_provenance");return;}
 for(unsigned n=0;n<e.count;++n)if(!bound[n]&&e.members[n].kind==kind&&(!placed||*placed==n)){bound[n]=actor;bool ok=variants::bind(rdram,n,actor);if(boss_placement0260::supported(kind))ok=ok&&placement_guard.bind(rdram,n,actor,variants::snapshot().members[n].birth);if(kind==0x455A)ok=ok&&candies[n].bind(actor);else if(kind==0x41EC)ok=ok&&blargg_observer.bind(rdram,n,actor,variants::snapshot().members[n].birth,blargg_frame(rdram));else if(witnesses::family_for_kind(kind))ok=ok&&observers[n].bind(actor);if(!ok)fail("member_identity_binding");if(session.snapshot().phase==core::Phase::Fighting&&!session.bind(n,actor))fail("session_member_binding");fprintf(stderr,"[BossBattle] member-bound fight=%u member=%u actor=%08X kind=%04X\n",e.number,n,actor,kind);return;}
 fail("unexpected_extra_boss");
}
inline void forget(uint32_t actor){if(!enabled)return;floor_guard.forget(actor);rest_floor_guard.forget(actor);boss_tree_hybrid0260::forget(actor);rest_heart.forget(actor);checkpoints::forget(actor);if(physical_room==83)return;for(unsigned n=0;n<session.current().count;++n){observers[n].forget(actor);candies[n].forget(actor);}variants::forget(actor);}
// Native per-level bookkeeping is reset when the next arena has a different
// identity114. Keep that native reset, then restore the proved session payload
// at the exact pre-HUD scene fence. No actor/HP/pose/save-index is written here.
inline void trace_carry(uint8_t* rdram,const recomp_context* c,std::string_view name){
 if(!active()||!c||!cargo.captured||!cargo_epoch)return;const auto phase=session.snapshot().phase;
 if(phase!=core::Phase::ToRest&&phase!=core::Phase::ToFight)return;
 const auto destination=phase==core::Phase::ToRest?83:source_for(session.snapshot().index),player=session.snapshot().player;
 const bool route_owned=physical_room==destination&&native::word(rdram,route_address)==0x44CA2000&&native::byte(rdram,route_address+4)==destination;
 const bool identity=route_owned&&native::actor(rdram,player,0x4001)&&native::word(rdram,0x800F8828)==player&&!fatal_player(rdram,player);
 if(boss_carry0260::reset_fence(name,uint32_t(c->r31))){
  if(!identity||stats_reset_pending||stats_restored_epoch==cargo_epoch||!boss_carry0260::valid(cargo.stats)){fail("unproved_campaign_stats_reset");return;}
  stats_reset_pending=true;stats_reset_epoch=cargo_epoch;fprintf(stderr,"[BossCarry] reset-owned epoch=%u room=%u player=%08X fruits=%u score=%u vi=%u\n",cargo_epoch,physical_room,player,cargo.stats.count,cargo.stats.scores[1],current_vi);return;
 }
 if(stats_reset_pending&&boss_carry0260::hud_fence(name,uint32_t(c->r31))){
  if(!identity||stats_reset_epoch!=cargo_epoch||!boss_carry0260::restore(rdram,cargo.stats)){fail("campaign_stats_restore_rejected");return;}
  stats_reset_pending=false;stats_restored_epoch=cargo_epoch;fprintf(stderr,"[BossCarry] restored-before-hud epoch=%u room=%u player=%08X fruits=%u score=%u vi=%u\n",cargo_epoch,physical_room,player,cargo.stats.count,cargo.stats.scores[1],current_vi);
 }
}
inline bool carried_stats_match(uint8_t* rdram){
 boss_carry0260::Payload actual;if(stats_reset_pending||!cargo.captured||!boss_carry0260::capture(rdram,actual)||actual.count!=cargo.stats.count||actual.scores[1]!=cargo.stats.scores[1])return false;
 // Native HUD initialization may refresh the backup lists. The authoritative
 // eaten types and paired IDs must remain exactly those from the owned pot.
 return std::equal(cargo.stats.fruits.begin(),cargo.stats.fruits.begin()+cargo.stats.count,actual.fruits.begin())&&std::equal(cargo.stats.half_fruits.begin(),cargo.stats.half_fruits.begin()+cargo.stats.count,actual.half_fruits.begin());
}
inline void trace(uint8_t* rdram,const recomp_context* c,std::string_view name,unsigned vi,const uint32_t* sections){
 if(!active())return;current_vi=vi;std::copy_n(sections,300,overlays.begin());trace_carry(rdram,c,name);checkpoints::trace_carry(rdram,c,name);
 if(name=="func_8002F7AC"){
  const auto& s=session.snapshot();const auto pot=uint32_t(c->r5);
  const bool owned=(s.phase==core::Phase::Victory&&pot==exit_pot)||(s.phase==core::Phase::Rest&&physical_room==83);
  const boss_pot_aim0282::Frame f{owned,s.phase,s.player,pot,overlays[144],physical_room,s.index};
  recomp_context staged=*c;
  const auto result=boss_pot_aim0282::cancel(rdram,&staged,f,[](uint32_t fn,uint8_t* r,recomp_context* state){if(fn==0x8004700C)func_8004700C(r,state);else if(fn==0x80038F98)func_80038F98(r,state);});
  if(!result.ok){fprintf(stderr,"[BossPotAim] rejected reason=%s player=%08X pot=%08X reticle=%08X room=%u vi=%u\n",result.reason,s.player,pot,result.reticle,physical_room,vi);fail("native_pot_aim_cancel_rejected");return;}
  if(result.cleaned)fprintf(stderr,"[BossPotAim] cancelled player=%08X pot=%08X reticle=%08X room=%u vi=%u nativeCleanup=1\n",s.player,pot,result.reticle,physical_room,vi);
 }

 if(rest_heart.trace(rdram,c,name,rest_heart_frame()))fprintf(stderr,"[BossRestHeart] collected vi=%u completed=%u nativeHeal=8 oneUse=1\n",vi,session.snapshot().completed);
 if(name=="func_80573F94"&&boss_tree_hybrid0260::collect(rdram,c,hybrid_tree_frame()))fprintf(stderr,"[BossTreeHybrid] consumed=%u mask=%02X vi=%u room=%u\n",boss_tree_hybrid0260::snapshot().consumed,boss_tree_hybrid0260::snapshot().consumed_mask,vi,physical_room);
 if(physical_room==83&&name=="func_80048E9C"&&native::actor(rdram,uint32_t(c->r4),0x4592))if(!rest_floor_guard.capture(rdram,c,physical_room,rest_floor_encounter,overlays[boss_floor_persistence0260::overlay_section],checkpoints::rest_source(rdram))){fail("rest_floor_source_rejected");return;}
 if(physical_room==83||physical_room!=source_for(session.snapshot().index))return;
 if(name=="func_8006DA0C"&&retry_placements.pending(session.snapshot().index)){
  const auto& s=session.snapshot();boss_retry_placements0281::Receipt proof;
  if(!retry_placements.restore(rdram,c,rooms::arena(physical_room),session.current(),{physical_room,s.index,s.retry_epoch,vi,s.phase},proof,[](uint32_t fn,uint8_t* r,recomp_context* state){if(fn==0x8006D6F8)func_8006D6F8(r,state);else if(fn==0x8006D34C)func_8006D34C(r,state);})){fprintf(stderr,"[BossRetryPlacement] rejected reason=%s room=%u fight=%u epoch=%u vi=%u\n",retry_placements.failure(),physical_room,s.index+1,s.retry_epoch,vi);fail("checkpoint_boss_placement_rejected");return;}
  if(proof.epoch)fprintf(stderr,"[BossRetryPlacement] restored room=%u fight=%u epoch=%u count=%u cache=%08X table=%08X vi=%u nativeSetter=6D34C nativeObservation=1\n",proof.room,proof.index+1,proof.epoch,proof.count,proof.cache,proof.table,proof.vi);
 }
 if(name=="func_80048E9C"&&native::range(uint32_t(c->r4),0x1FC)&&boss_retry_placements0281::capture_kind(session.current(),MEM_HU(12,native::base(uint32_t(c->r4))))&&!retry_placements.capture(rdram,c,rooms::arena(physical_room),session.current(),session.snapshot().index,session.snapshot().retry_epoch)){fprintf(stderr,"[BossRetryPlacement] rejected reason=%s room=%u fight=%u epoch=%u vi=%u\n",retry_placements.failure(),physical_room,session.snapshot().index+1,session.snapshot().retry_epoch,vi);fail("checkpoint_boss_pristine_capture_rejected");return;}
 if(name=="func_80048E9C"&&logical_room==162&&native::actor(rdram,uint32_t(c->r4),0x4592))if(!floor_guard.capture(rdram,c,rooms::arena(physical_room),physical_room,session.snapshot().index,overlays[boss_floor_persistence0260::overlay_section])){fail("boss_floor_source_rejected");return;}
 if(name=="func_80048E9C"&&native::range(uint32_t(c->r4),0x1FC)&&boss_placement0260::supported(MEM_HU(12,native::base(uint32_t(c->r4)))))if(!placement_guard.capture(rdram,c,rooms::arena(physical_room),session.current(),physical_room,session.snapshot().index)){fail("boss_placement_source_rejected");return;}
 if(auto kind=init_kind(name)){const auto a=uint32_t(c->r4);if(kind==0x4511&&native::actor(rdram,a,0x4631))kind=0x4631;bind_actor(rdram,a,kind);}
 variants::trace(rdram,c,name,variant_frame(rdram));
 if(blargg_owned()){
  blargg_observer.enter(rdram,c,name,blargg_frame(rdram));
  if(blargg_observer.snapshot().invalidated){fail(blargg_observer.snapshot().failure.c_str());return;}
  if(name=="func_804F5384")for(unsigned n=0;n<2;++n){const auto& m=blargg_observer.snapshot().members[n];const auto actor=m.identity.actor;
   if(m.initialized&&!m.retired&&actor==uint32_t(c->r4)&&blargg_observer.birth(actor)==m.identity.birth&&native::actor(rdram,actor,0x41EC)&&native::word(rdram,actor+0x1F4)==m.identity.private_data&&native::byte(rdram,actor+0x17D)==2&&native::byte(rdram,actor+0x18)==1&&native::byte(rdram,actor+0x30)==1&&native::word(rdram,actor+0x20)==overlays[90]+0x10F4)blargg_active_births[n]=m.identity.birth;
  }
 }
 for(unsigned n=0;n<session.current().count;++n)if(bound[n]){if(session.current().members[n].kind==0x455A)candies[n].observe_entry(rdram,c,name,witness_frame(n));else if(witnesses::family_for_kind(session.current().members[n].kind))observers[n].observe_entry(rdram,c,name,witness_frame(n));}
 if(name=="func_80664E18"){const auto f=lips_candy_entry_frame();if(f.enabled&&!lips_candy_entry.capture(rdram,c,f))fail("lips_candy_entry_capture_rejected");}
}
inline void receipt(uint8_t* rdram,const core::NativeReceipt& r){if(!session.defeat(r)){fail("native_receipt_rejected");return;}if(boss_placement0260::supported(session.current().members[r.member].kind)&&!placement_guard.authorize(rdram,r,observers[r.member].snapshot(),session.snapshot().player)){fail("boss_placement_receipt_rejected");return;}history[r.encounter].members[r.member]=r;variants::acknowledge_defeat(r);fprintf(stderr,"[BossBattle] member-defeated fight=%u member=%u actor=%08X vi=%u remainingMask=%u\n",r.encounter+1,r.member,r.actor,r.vi,unsigned(session.snapshot().defeated));if(session.current().family!=core::Family::Blargg)restore_pending=true;}
inline void drain_receipts(uint8_t* rdram){for(unsigned n=0;n<session.current().count;++n){if(auto p=candies[n].take_defeat_observation()){
 const auto before=session.snapshot().defeated;auto r=witnesses::core_receipt(*p);r.retry_epoch=observer_epochs[n];receipt(rdram,r);
 if(session.snapshot().diagnostic&&session.snapshot().index==9&&session.current().family==core::Family::RedCandy&&session.snapshot().phase!=core::Phase::Failed&&!(before&(1u<<n))&&(session.snapshot().defeated&(1u<<n)))fprintf(stderr,"[BossFinalCandy] actor=%08X player=%08X initial=%d remaining=%d causalHits=%u vi=%u index=%u nativeObservation=1\n",p->actor,p->player,p->initial_native_counter,p->terminal_native_counter,p->causal_hits,p->vi,p->encounter);
}if(auto p=observers[n].take_defeat_observation()){auto r=witnesses::core_receipt(*p);r.retry_epoch=observer_epochs[n];receipt(rdram,r);}}for(auto p:variants::take_receipts()){p.retry_epoch=variant_epoch;receipt(rdram,p);}
 for(const auto& p:blargg_observer.take_receipts()){
  const auto& s=session.snapshot();
  if(!blargg_owned()||p.member>=2||p.token!=s.token||p.encounter!=s.index||p.room!=physical_room||p.retry_epoch!=s.retry_epoch||p.hits!=3||p.identity.actor!=bound[p.member]||p.variant_birth!=variants::snapshot().members[p.member].birth||p.identity.birth!=blargg_observer.snapshot().members[p.member].identity.birth||!blargg_observer.snapshot().members[p.member].retired){fail("blargg_terminal_receipt_rejected");return;}
  core::NativeReceipt proof{p.encounter,p.member,p.vi,p.identity.actor,p.initialized,p.active,p.terminal,p.causal_hit,p.retry_epoch};receipt(rdram,proof);
  fprintf(stderr,"[BossBlargg] member-defeated fight=%u member=%u actor=%08X nativeBirth=%llu variantBirth=%llu hits=%u epoch=%u vi=%u native447C8=1 nativeObservation=1\n",p.encounter+1,p.member,p.identity.actor,(unsigned long long)p.identity.birth,(unsigned long long)p.variant_birth,p.hits,p.retry_epoch,p.vi);
 }
}
// The original callers all ignore V0. Their native defeat cleanup continues,
// while the story46EE cinematic is suppressed for this campaign only.
inline bool shared_completion(uint8_t* rdram,recomp_context* c){
 if(!active()||physical_room==83)return false;trace(rdram,c,"func_807AEA78",current_vi,overlays.data());bool accepted=false;
 for(unsigned n=0;n<session.current().count;++n){const auto f=witness_frame(n);if(candies[n].completion_pending()){accepted=candies[n].acknowledge_suppressed_completion(rdram,f)||accepted;}if(observers[n].completion_pending()){accepted=observers[n].acknowledge_suppressed_completion(rdram,f)||accepted;}}
 if(!accepted){
  // At most one context and four member rows: failure immediately closes this
  // session. Observe the rejected proof without changing admission or RDRAM.
  const auto player=session.snapshot().player;const bool player_ok=native::actor(rdram,player,0x4001);
  fprintf(stderr,"[BossProofReject] vi=%u fight=%u room=%u source=%u a0=%08X a1=%08X ra=%08X sceneOverlay=%08X completionCallback=%08X player=%08X singleton=%08X playerState=%u/%u playerCallback=%08X nativeObservation=1\n",current_vi,session.snapshot().index+1,physical_room,logical_room,uint32_t(c->r4),uint32_t(c->r5),uint32_t(c->r31),overlays[285],native::word(rdram,0x800F8D78),player,native::word(rdram,0x800F8828),player_ok?native::byte(rdram,player+0x18):255,player_ok?native::byte(rdram,player+0x30):255,player_ok?native::word(rdram,player+0x20):0);
  for(unsigned n=0;n<session.current().count;++n){
   const auto kind=session.current().members[n].kind;const auto& proof=kind==0x455A?candies[n].snapshot():observers[n].snapshot();const auto& a=proof.actors[0];
   const auto root=bound[n];const bool root_ok=native::actor(rdram,root,kind);const auto priv=root_ok?native::word(rdram,root+0x1F4):0;const bool private_ok=native::range(priv,0x34);const auto f=witness_frame(n);
   fprintf(stderr,"[BossProofMember] member=%u kind=%04X bound=%08X enabled=%u invalidated=%u failure=%s registered=%u expectedHits=%u initialized=%u active=%u activeVI=%u counter=%d initialCounter=%d causalHits=%u prepare=%u terminal=%u terminalVI=%u completionCalled=%u completionReturned=%u evidenceActor=%08X evidencePrivate=%08X familyOverlay=%08X rawState=%u/%u rawCallback=%08X rawPrivate=%08X rawPhase=%u rawHitFlag=%u rawTimer=%u nativeObservation=1\n",n,unsigned(kind),root,unsigned(proof.enabled),unsigned(proof.invalidated),proof.failure.empty()?"none":proof.failure.c_str(),proof.registered_actors,proof.expected_hits,unsigned(a.initialized),unsigned(a.active_seen),a.active_vi,a.counter,a.initial_counter,a.causal_hits,unsigned(a.terminal_prepare_seen),unsigned(a.terminal_seen),a.terminal_vi,unsigned(a.completion_called),unsigned(a.completion_returned),a.actor,a.private_data,f.family_overlay,root_ok?native::byte(rdram,root+0x18):255,root_ok?native::byte(rdram,root+0x30):255,root_ok?native::word(rdram,root+0x20):0,priv,private_ok?native::word(rdram,priv+0x24):0xFFFFFFFF,private_ok?native::word(rdram,priv+0x30):0xFFFFFFFF,root_ok?native::word(rdram,root+0x1E4):0xFFFFFFFF);
  }
  fail("shared_completion_without_native_proof");
 }else drain_receipts(rdram);c->r2=0;return true;
}
inline void scale_candy(uint8_t* rdram,unsigned n,float factor){const auto root=bound[n],p=native::word(rdram,root+0x1F4),head=native::range(p,0x24)?native::word(rdram,p):0;if(!native::actor(rdram,root,0x455A)||!native::actor(rdram,head,0x454F)||native::word(rdram,head+0x180)!=root)return;for(auto a:{root,head})for(unsigned off:{0x6Cu,0x70u}){const float value=std::bit_cast<float>(native::word(rdram,a+off));if(std::isfinite(value)&&value>=0&&value<100)MEM_W(off,native::base(a))=std::bit_cast<uint32_t>(value*factor);}}
inline boss_fish_companion_intro0260::Frame fish_companion_intro_frame(uint8_t* ram){
 namespace intro=boss_fish_companion_intro0260;intro::Frame out;const auto& s=session.snapshot();const auto& v=variants::snapshot();
 if(!ram||!active()||s.phase!=core::Phase::Fighting||logical_room!=97||physical_room!=167+s.index||session.current().family!=core::Family::Fish||session.current().variant!=2||session.current().count!=2||session.current().members[0].kind!=0x4511||(session.current().members[1].kind!=0x4072&&session.current().members[1].kind!=0x4599)||!v.enabled||v.invalidated||v.encounter!=s.index||v.room!=97||variant_epoch!=s.retry_epoch||observer_epochs[1]!=s.retry_epoch||(s.defeated&3))return out;
 const auto& fish=v.members[0];const auto& companion=v.members[1];const auto& proof=observers[1].snapshot();const auto& a=proof.actors[0];const auto family=witnesses::family_for_kind(session.current().members[1].kind);
 if(!fish.actor||fish.actor!=bound[0]||!fish.birth||!fish.initialized||fish.terminal||fish.receipt_sent||fish.retire_queued||companion.actor!=bound[1]||!companion.birth||companion.terminal||companion.receipt_sent||companion.retire_queued||!family||!proof.enabled||proof.invalidated||proof.family!=*family||proof.source_room!=97||proof.run_token!=s.token||proof.expected_actors!=1||proof.registered_actors!=1||proof.expected_hits!=session.current().members[1].hits||!a.initialized||a.disposed||a.terminal_seen||a.completion_returned||a.actor!=bound[1]||observers[1].bound_actor()!=bound[1])return out;
 out={true,true,true,2,2,s.index,physical_room,logical_room,s.retry_epoch,s.player,s.token,{bound[0],fish.private_data,overlays[148],fish.birth,variant_epoch,0x4511},{bound[1],a.private_data,witness_frame(1).family_overlay,companion.birth,observer_epochs[1],session.current().members[1].kind}};return out;
}
inline bool hold_fish_companion_intro(uint8_t* ram,const recomp_context* ctx,std::string_view name){
 const auto f=fish_companion_intro_frame(ram);const bool hold=boss_fish_companion_intro0260::suppress(ram,ctx,name,f);
 if(hold&&name=="func_806AEFDC"&&!fish_intro_contact_logged){
  fprintf(stderr,"[BossFishIntro] action=contact-held index=%u epoch=%u fish=%08X companion=%08X companionCallback=%08X player=%08X vi=%u nativeVoidWrapper=1 ramWrites=0\n",f.index,f.epoch,f.fish.actor,f.companion.actor,native::word(ram,f.companion.actor+0x20),f.player,current_vi);fish_intro_contact_logged=true;
 }
 const bool live=f.active&&ctx&&uint32_t(ctx->r4)==f.fish.actor&&native::actor(ram,f.fish.actor,0x4511)&&native::actor(ram,f.companion.actor,f.companion.kind)&&native::actor(ram,f.player,0x4001);
 const auto callback=live?native::word(ram,f.companion.actor+0x20):0,sub=live?native::byte(ram,f.companion.actor+0x30):0;
 const bool released=live&&!hold&&!boss_fish_companion_intro0260::introducing(ram,f)&&!native::byte(ram,0x800F7EA5)&&native::byte(ram,0x800F7EA6)==1&&native::byte(ram,f.companion.actor+0x18)==1&&(f.companion.kind==0x4072?(sub==6&&callback==f.companion.overlay+0x1B88):((sub==3&&callback==f.companion.overlay+0x1178)||(sub==2&&callback==f.companion.overlay+0x1138)));
 if(live&&((hold&&!fish_intro_hold_logged)||(released&&fish_intro_hold_logged&&!fish_intro_release_logged))){
  fprintf(stderr,"[BossFishIntro] action=%s index=%u epoch=%u fish=%08X fishCallback=%08X companion=%08X companionCallback=%08X player=%08X playerState=%u/%u controls=%u/%u vi=%u nativeVoidWrapper=1 ramWrites=0\n",hold?"hold":"release",f.index,f.epoch,f.fish.actor,native::word(ram,f.fish.actor+0x20),f.companion.actor,native::word(ram,f.companion.actor+0x20),f.player,native::byte(ram,f.player+0x18),native::byte(ram,f.player+0x30),native::byte(ram,0x800F7EA5),native::byte(ram,0x800F7EA6),current_vi);
  if(hold)fish_intro_hold_logged=true;else fish_intro_release_logged=true;
 }
 return hold;
}
inline bool hook_enter(uint8_t* rdram,const recomp_context* c,std::string_view name){
 if(!active()||physical_room==83)return false;
 if((name=="func_805C7C2C"||name=="func_805C6900"||name=="func_806AEFDC")&&hold_fish_companion_intro(rdram,c,name))return true;
 if(name=="func_80666E34")for(unsigned n=0;n<session.current().count;++n){const auto& m=session.current().members[n];if(m.kind!=0x455A||m.scale>=1||!native::actor(rdram,bound[n],0x455A))continue;const auto p=native::word(rdram,bound[n]+0x1F4),head=native::range(p,0x24)?native::word(rdram,p):0;if(uint32_t(c->r5)==head&&native::actor(rdram,head,0x454F)&&native::word(rdram,head+0x1F4)==uint32_t(c->r4)){if(scaled[n])scale_candy(rdram,n,1.f/m.scale);scale_scopes.emplace_back(uint32_t(c->r29),n);break;}}
 return variants::collision(rdram,c,name,variant_frame(rdram))||variants::skip_native_update(rdram,c,name,variant_frame(rdram));
}
inline void hook_leave(uint8_t* rdram,const recomp_context* entry,std::string_view name){
 if(!active()||!entry)return;const auto a=uint32_t(entry->r4);
 if(blargg_owned()){
  const auto before=blargg_observer.snapshot();blargg_observer.leave(rdram,entry,name,blargg_frame(rdram));
  const auto& after=blargg_observer.snapshot();if(after.invalidated){fail(after.failure.c_str());return;}
  for(unsigned n=0;n<2;++n){const auto& m=after.members[n];
   if(name=="func_804F42B8"&&m.identity.actor==a&&m.initialized&&!variants::snapshot().members[n].initialized){if(!variants::initialize_blargg(rdram,n,blargg_observer)){fail("blargg_variant_initializer_rejected");return;}fprintf(stderr,"[BossBlargg] initialized fight=%u member=%u actor=%08X private=%08X nativeBirth=%llu variantBirth=%llu epoch=%u vi=%u nativeConstructorReturned=1\n",session.snapshot().index+1,n,a,m.identity.private_data,(unsigned long long)m.identity.birth,(unsigned long long)m.variant_birth,session.snapshot().retry_epoch,current_vi);}
   if(m.hits>before.members[n].hits)fprintf(stderr,"[BossBlarggHit] fight=%u member=%u actor=%08X hits=%u remaining=%u nativeBirth=%llu epoch=%u vi=%u native57E4Returned=1 nativeObservation=1\n",session.snapshot().index+1,n,m.identity.actor,m.hits,3-m.hits,(unsigned long long)m.identity.birth,session.snapshot().retry_epoch,current_vi);
   if(m.retracted&&!before.members[n].retracted)fprintf(stderr,"[BossBlargg] retracted fight=%u member=%u actor=%08X nativeBirth=%llu epoch=%u vi=%u native571CReturned=1 nativeObservation=1\n",session.snapshot().index+1,n,m.identity.actor,(unsigned long long)m.identity.birth,session.snapshot().retry_epoch,current_vi);
  }
 }
 if(physical_room==83){
  if(name=="func_8069D05C"&&rest_floor_guard.owned(a)){
   if(!rest_floor_guard.pin(rdram,entry,physical_room,rest_floor_encounter,overlays[boss_floor_persistence0260::overlay_section])){fail("rest_floor_native_stamp_rejected");return;}
   fprintf(stderr,"[BossRestFloor] pinned actor=%08X room=%u vi=%u nativeStamp=1\n",a,physical_room,current_vi);
  }return;
 }
 if(name=="func_8069D05C"&&floor_guard.owned(a)){
  bool pinned=false;for(const auto& b:floor_guard.snapshot())if(b.actor==a)pinned=b.pinned;
  if(!floor_guard.pin(rdram,entry,rooms::arena(physical_room),physical_room,session.snapshot().index,overlays[boss_floor_persistence0260::overlay_section])){fail("boss_floor_native_stamp_rejected");return;}
  if(!pinned)fprintf(stderr,"[BossFloor] pinned actor=%08X record=%08X instance=%u room=%u vi=%u nativeStamp=1\n",a,native::word(rdram,a+0x1D8),native::word(rdram,a+0x1DC)&65535,physical_room,current_vi);
 }
 if(boss_placement0260::supported(init_kind(name))){const auto member=placement_guard.member(a);if(!member||!placement_guard.initialized(rdram,*member,a,variants::snapshot().members[*member].birth)){fail("boss_placement_initializer_rejected");return;}}
 if(name=="func_80664E18"){const auto f=lips_candy_entry_frame();if(f.enabled){if(!lips_candy_entry.finish(rdram,entry,f)){fail("lips_candy_entry_return_rejected");return;}fprintf(stderr,"[BossLipsCandyEntry] actor=%08X birth=%llu room=%u index=%u epoch=%u xy=248.00,152.00 vi=%u nativeConstructorReturned=1 oneTime=1 physicsUnchanged=1\n",f.actor,static_cast<unsigned long long>(f.birth),f.room,f.index,f.epoch,current_vi);}}
 if(name=="func_80666E34"&&!scale_scopes.empty()&&scale_scopes.back().first==uint32_t(entry->r29)){const auto n=scale_scopes.back().second;scale_scopes.pop_back();scale_candy(rdram,n,session.current().members[n].scale);scaled[n]=true;}
 // Construction retains its real native init, then the configured initial HP
 // is applied before the first active callback or causal-hit observation.
 for(unsigned n=0;n<session.current().count;++n)if(bound[n]==a){const auto& m=session.current().members[n];if(name=="func_8042D33C"&&m.kind==0x4072)MEM_W(0x1E4,native::base(a))=m.hits;if(name=="func_806AF580"&&m.kind==0x4599)MEM_W(0x1E4,native::base(a))=m.hits-1;}
 variants::leave(rdram,entry,name,variant_frame(rdram));for(unsigned n=0;n<session.current().count;++n){
  const auto& before=observers[n].snapshot();const auto& a=before.actors[0];
  const auto kind=session.current().members[n].kind;
  const bool family_hit=(name=="func_80557D44"&&kind==0x40A3)||(name=="func_8042D090"&&kind==0x4072)||(name=="func_806AEFDC"&&kind==0x4599);
  const bool early_family_hit=family_hit&&before.enabled&&!before.invalidated&&before.registered_actors==1&&a.actor==bound[n]&&a.initialized&&!a.active_seen&&!a.causal_hits;
  const auto& candy_before=candies[n].snapshot();const unsigned candy_hits_before=candy_before.actors[0].causal_hits;
  const bool early_candy=name=="func_80666ACC"&&session.current().members[n].kind==0x455A&&candy_before.enabled&&!candy_before.invalidated&&candy_before.registered_actors==1&&!candy_before.actors[0].active_seen;
  candies[n].observe_hit_return(rdram,name,witness_frame(n));observers[n].observe_hit_return(rdram,name,witness_frame(n));
  const auto& candy_after=candies[n].snapshot();if(early_candy&&!candy_after.invalidated&&candy_after.actors[0].causal_hits>candy_hits_before)fprintf(stderr,"[BossCandyEarlyHit] member=%u actor=%08X vi=%u retryEpoch=%u before=%u after=%u nativeReturned=1 nativeObservation=1\n",n,candy_after.actors[0].actor,current_vi,session.snapshot().retry_epoch,candy_hits_before,candy_after.actors[0].causal_hits);

  const auto& after=observers[n].snapshot();if(early_family_hit&&!after.invalidated&&after.actors[0].causal_hits==1)fprintf(stderr,"[BossEarlyHit] member=%u actor=%08X vi=%u room=%u source=%u activeBefore=0 causalBefore=0 causalAfter=1 nativeObservation=1\n",n,after.actors[0].actor,current_vi,physical_room,logical_room);
 }drain_receipts(rdram);
}
inline bool passenger(uint8_t* rdram,uint32_t pot){const auto a=session.snapshot().player,p=native::word(rdram,a+0x1F4);const auto hp=boss_solo0240::health(rdram);return a&&native::actor(rdram,a,0x4001)&&native::word(rdram,0x800F8828)==a&&hp&&*hp<=8&&native::range(p,0x12D0)&&MEM_BU(0x18,native::base(a))==0xA&&MEM_BU(0x30,native::base(a))==2&&native::word(rdram,a+0x20)==0x80031230&&native::word(rdram,p+0xD0)==pot&&!native::word(rdram,p+0xC0)&&!native::word(rdram,a+0x180)&&MEM_BU(0x165,native::base(a))==1&&MEM_BU(0x17D,native::base(a))==0;}
inline bool capture(uint8_t* rdram,uint32_t pot){const auto a=session.snapshot().player;const auto hp=boss_solo0240::health(rdram);if(!hp||!passenger(rdram,pot)||!boss_solo0240::check_children(rdram,a,cargo.children)||!boss_solo0240::read_reserves(rdram,a,cargo.eggs))return false;cargo.hp=*hp;cargo.color=native::word(rdram,0x800F8DB0);cargo.fruits=native::word(rdram,0x800F89D0);cargo.pc_color=native::word(rdram,0x807F0008);if(!boss_carry0260::capture(rdram,cargo.stats))return false;cargo.captured=true;++cargo_epoch;stats_reset_pending=false;fprintf(stderr,"[BossCarry] captured epoch=%u player=%08X fruits=%u score=%u vi=%u\n",cargo_epoch,a,cargo.stats.count,cargo.stats.scores[1],current_vi);return true;}
inline void make_route(uint8_t* rdram,unsigned destination){std::array<uint8_t,24> route{};if(destination==83){route=boss_rest_data0240::native_arena_pot_route;for(unsigned n=0;n<4;++n){route[8+n]=uint8_t(u32(boss_rest_data0240::rest_descriptor,0x28)>>(24-8*n));route[12+n]=uint8_t(u32(boss_rest_data0240::rest_descriptor,0x2C)>>(24-8*n));}}else route=rooms::route_to(destination);for(unsigned n=0;n<24;++n)MEM_B(n,native::base(route_address))=route[n];}
inline bool pot_complete(uint8_t* rdram,recomp_context* c){
 if(!active()||!native::range(uint32_t(c->r4),24))return false;const auto phase=session.snapshot().phase;uint32_t pot=0;
 if(phase==core::Phase::Victory&&native::actor(rdram,exit_pot,0x4273))pot=exit_pot;
 else if(phase==core::Phase::Rest){const auto player=session.snapshot().player;const auto p=native::word(rdram,player+0x1F4);if(native::range(p,0x12D0))pot=native::word(rdram,p+0xD0);if(!native::actor(rdram,pot,0x4274))return false;}
 else return false;
 const auto data=native::word(rdram,pot+0x1F4);if(!native::range(data,0x264)||native::word(rdram,data+0x260)!=uint32_t(c->r4))return false;
 if(!capture(rdram,pot)){fail("pot_cargo_identity");return true;}
 if(phase==core::Phase::Victory){if(!session.use_victory_pot()){fail("victory_pot_phase");return true;}if(session.snapshot().phase==core::Phase::Complete){completion_pending=true;fprintf(stderr,"[BossBattle] complete fights=10 yoshi=%u diagnostic=%u\n",cargo.pc_color,unsigned(session.snapshot().diagnostic));return true;}make_route(rdram,83);}
 else {if(!session.use_rest_pot()){fail("rest_pot_phase");return true;}make_route(rdram,source_for(session.snapshot().index));}
 ready_info=0;arrival_vi=0;c->r4=native::base(route_address);fprintf(stderr,"[BossBattle] native-pot-departure fight=%u destination=%u health=%u eggs=%u fruits=%u\n",session.snapshot().index+1,phase==core::Phase::Victory?83:source_for(session.snapshot().index),cargo.hp,unsigned(std::count_if(cargo.eggs.begin(),cargo.eggs.end(),[](auto a){return a!=0;})),cargo.fruits);return false;
}
inline std::optional<float> pot_position(uint8_t* rdram){const auto a=session.snapshot().player;if(!witnesses::active_player(rdram,a))return {};const float px=std::bit_cast<float>(native::word(rdram,a+0x84));if(!std::isfinite(px))return {};const auto& arena=rooms::arena(physical_room);if(!arena.safe_pot_xs.empty())return *std::min_element(arena.safe_pot_xs.begin(),arena.safe_pot_xs.end(),[&](float a,float b){return std::abs(a-px)<std::abs(b-px);});return std::clamp(px+80,arena.min_x,arena.max_x);}
inline bool spawn_exit(uint8_t* rdram,recomp_context* c){
 const auto x=pot_position(rdram);if(!x)return false;exit_x=*x;const auto& arena=rooms::arena(physical_room);const auto saved=*c;c->r29=ADD32(saved.r29,-0xC0);c->r4=0x4273;func_80047090(rdram,c);const auto resource=uint32_t(c->r2);*c=saved;if(resource==0x3FF)return false;c->r29=ADD32(saved.r29,-0xC0);c->r4=0x42732000|(resource&0x3FF);c->r5=std::bit_cast<uint32_t>(exit_x);c->r6=std::bit_cast<uint32_t>(arena.safe_pot_y);c->r7=0x43FA3AE1;func_8004613C(rdram,c);exit_pot=uint32_t(c->r2);*c=saved;if(!native::actor(rdram,exit_pot,0x4273))return false;MEM_H(0xC0,native::base(exit_pot))=0x7FFF;MEM_H(0xC2,native::base(exit_pot))=0x7FFF;c->r29=ADD32(saved.r29,-0xC0);c->r4=native::base(exit_pot);func_80044194(rdram,c);const bool ok=c->r2!=0;*c=saved;if(ok)fprintf(stderr,"[BossBattle] exit-pot-ready fight=%u actor=%08X x=%.2f y=%.2f vi=%u\n",session.snapshot().index+1,exit_pot,exit_x,arena.safe_pot_y,current_vi);return ok;
}
template<class Dispatch> inline bool spawn_rest_heart(uint8_t* rdram,recomp_context* c,Dispatch&& dispatch){
 const auto request=rest_heart.request_spawn(rdram,rest_heart_frame());if(!request)return true;
 const auto saved=*c;const uint32_t sp=uint32_t(saved.r29),vec=sp-0x20;
 if(sp<0x80000600||!native::range(vec,12)||!native::range(overlays[220],0xF18)){rest_heart.spawn_failed(request->birth);return false;}
 std::array<uint32_t,3> previous{};for(unsigned i=0;i<3;++i)previous[i]=native::word(rdram,vec+4*i);
 const std::array<float,3> xyz{request->x,request->y,request->z};for(unsigned i=0;i<3;++i)MEM_W(4*i,native::base(vec))=std::bit_cast<uint32_t>(xyz[i]);
 c->r29=ADD32(saved.r29,-0xD0);c->r4=native::base(vec);uint32_t heart=0;
 try{dispatch(overlays[220]+boss_rest_heart0260::spawn_offset,rdram,c);heart=uint32_t(c->r2);}
 catch(...){for(unsigned i=0;i<3;++i)MEM_W(4*i,native::base(vec))=previous[i];*c=saved;throw;}
 for(unsigned i=0;i<3;++i)MEM_W(4*i,native::base(vec))=previous[i];*c=saved;
 if(!heart){rest_heart.spawn_failed(request->birth);return false;}
 if(!rest_heart.bind_spawn(rdram,rest_heart_frame(),*request,heart))return false;
 MEM_H(0xC0,native::base(heart))=boss_rest_heart0260::cull_extent;MEM_H(0xC2,native::base(heart))=boss_rest_heart0260::cull_extent;
 fprintf(stderr,"[BossRestHeart] spawned actor=%08X completed=%u visit=%u xy=%.2f,%.2f vi=%u queue=%u\n",heart,session.snapshot().completed,request->visit,request->x,request->y,current_vi,native::byte(rdram,heart+0x17D));return true;
}
template<class Dispatch> inline void frame(uint8_t* rdram,recomp_context* c,unsigned vi,Dispatch&& dispatch){
 if(enabled)current_vi=vi;
 if(enabled&&completion_pending&&session.snapshot().phase==core::Phase::Complete){completion_pending=false;if(completion_hook)completion_hook(rdram);blargg_observer.disable();blargg_active_births={};finished=true;return;}
 if(!active())return;current_vi=vi;rest_heart.frame(rdram,rest_heart_frame());if(rest_heart.snapshot().invalidated){fail(rest_heart.snapshot().failure.c_str());return;}boss_tree0240::frame(rdram);boss_tree_hybrid0260::frame(rdram,hybrid_tree_frame());if(boss_tree_hybrid0260::snapshot().invalidated){fail(boss_tree_hybrid0260::snapshot().failure.c_str());return;}if(boss_tree0240::snapshot().invalidated){fail("rest_tree_identity_or_supply_invalidated");return;}auto phase=session.snapshot().phase;
 if((phase==core::Phase::Armed||phase==core::Phase::ToFight||phase==core::Phase::ToRest||phase==core::Phase::RetryToRest||phase==core::Phase::RetryToFight)&&ready_info){
  const auto player=native::word(rdram,0x800F8828);const bool released=witnesses::active_player(rdram,player)&&MEM_BU(0x17D,native::base(player))==2&&!MEM_BU(0x165,native::base(player));if(!arrival_vi)arrival_vi=vi;
  if(!released){if(vi-arrival_vi>1800)fail("native_arrival_timeout");return;}
  const bool retry=phase==core::Phase::RetryToRest||phase==core::Phase::RetryToFight;
  if(retry){if(!checkpoints::restore_arrival(rdram,c,vi)){fail("checkpoint_retry_inventory_restore_rejected");return;}}
  else if(phase!=core::Phase::Armed){std::array<uint32_t,20> children{};std::array<uint32_t,6> eggs{};const auto hp=boss_solo0240::health(rdram);const bool children_ok=boss_solo0240::check_children(rdram,player,children),eggs_ok=boss_solo0240::read_reserves(rdram,player,eggs),stats_ok=carried_stats_match(rdram);
   if(player!=session.snapshot().player||!hp||*hp!=cargo.hp||native::word(rdram,0x800F8DB0)!=cargo.color||native::word(rdram,0x807F0008)!=cargo.pc_color||native::word(rdram,0x800F89D0)!=cargo.fruits||!children_ok||children!=cargo.children||!eggs_ok||eggs!=cargo.eggs||!stats_ok){fprintf(stderr,"[BossCarry] mismatch player=%u hp=%u color=%u pcColor=%u fruits=%u children=%u eggs=%u stats=%u actualFruits=%u expectedFruits=%u\n",unsigned(player==session.snapshot().player),unsigned(hp&&*hp==cargo.hp),unsigned(native::word(rdram,0x800F8DB0)==cargo.color),unsigned(native::word(rdram,0x807F0008)==cargo.pc_color),unsigned(native::word(rdram,0x800F89D0)==cargo.fruits),unsigned(children_ok&&children==cargo.children),unsigned(eggs_ok&&eggs==cargo.eggs),unsigned(stats_ok),native::word(rdram,0x800F89D0),cargo.fruits);fail("native_cargo_changed");return;}cargo.captured=false;}
  ready_info=0;if(phase==core::Phase::ToRest||phase==core::Phase::RetryToRest){if(!(retry?session.arrive_retry(83,player):session.arrive_rest(player)))fail("rest_arrival_phase");rest_vi=vi;fprintf(stderr,"[BossBattle] rest-ready completed=%u player=%08X treeConsumed=%u treeRegens=%u vi=%u\n",session.snapshot().completed,player,boss_tree0240::snapshot().totalConsumed,boss_tree0240::snapshot().regenerations,vi);return;}
  if(!(retry?session.arrive_retry(logical_room,player):session.enter_fight(logical_room,player))){fail("fight_arrival_phase");return;}for(unsigned n=0;n<session.current().count;++n)if(bound[n]&&!session.bind(n,bound[n]))fail("deferred_member_binding");
  auto& record=history[session.snapshot().index];record.player=player;record.color=native::word(rdram,0x807F0008);
  if(!first_player){first_player=player;first_color=record.color;}
  if(record.player!=first_player||record.color!=first_color){fail("battle_yoshi_identity_changed");return;}
  if(!checkpoints::capture_baseline(rdram,vi)){fail("checkpoint_initial_inventory_capture_failed");return;}
  fight_ready={vi,session.snapshot().index,physical_room,logical_room,session.snapshot().retry_epoch,player,native::word(rdram,player+0x1F4)};
  fprintf(stderr,"[BossBattle] fight-ready fight=%u player=%08X vi=%u\n",session.snapshot().index+1,player,vi);phase=session.snapshot().phase;
 }
 if((phase==core::Phase::Fighting||phase==core::Phase::Victory||phase==core::Phase::Rest)&&session.snapshot().player){const auto player=session.snapshot().player;if(fatal_player(rdram,player)){if(!checkpoints::notice_death(rdram,vi))fail("yoshi_defeated");return;}}
 if(physical_room==83){if(!spawn_rest_heart(rdram,c,dispatch))fail("native_rest_red_heart_spawn_failed");return;}
 for(unsigned n=0;n<session.current().count;++n)if(bound[n]){candies[n].frame(rdram,witness_frame(n));observers[n].frame(rdram,witness_frame(n));const auto& a=session.current().members[n].kind==0x455A?candies[n].snapshot():observers[n].snapshot();if(a.enabled&&a.invalidated){fail(a.failure.c_str());return;}}
 remember_controls(rdram,phase);
 variants::frame(rdram,variant_frame(rdram));for(const auto& action:variants::take_actions(rdram)){const bool ok=variants::execute(rdram,c,action,dispatch);variants::action_completed(rdram,action,ok,vi);if(!ok){fail("native_variant_action_failed");return;}}
 if(blargg_owned()){
  const auto completed=blargg_frame(rdram,true);
  for(unsigned n=0;n<2;++n)if(blargg_observer.ready_to_retire(rdram,n,completed)&&!blargg_observer.retire(rdram,c,n,completed,dispatch)){fail(blargg_observer.snapshot().invalidated?blargg_observer.snapshot().failure.c_str():"blargg_native_retirement_rejected");return;}
  if(blargg_observer.snapshot().invalidated){fail(blargg_observer.snapshot().failure.c_str());return;}
 }
 drain_receipts(rdram);
 if(variants::snapshot().invalidated){fail(variants::snapshot().failure.c_str());return;}
 const auto before_unlink=placement_guard.snapshot();std::array<uint64_t,4> births{};for(unsigned n=0;n<session.current().count;++n)births[n]=variants::snapshot().members[n].birth;
 if(placement_guard.pending()&&!placement_guard.post_675(rdram,c,{physical_room,session.snapshot().index,vi,session.snapshot().player,session.snapshot().phase,true},births,dispatch)){fail("boss_placement_post_fence_rejected");return;}
 for(unsigned n=0;n<session.current().count;++n){const auto& b=placement_guard.snapshot()[n];if(!before_unlink[n].applied&&b.applied)fprintf(stderr,"[BossPlacement] consumed fight=%u member=%u actor=%08X record=%08X packed=%08X vi=%u\n",session.snapshot().index+1,n,b.actor,b.record,b.packed,vi);}
 if(restore_pending){if(!flags_ready){fail("missing_active_controls_baseline");return;}const auto saved=*c;c->r29=ADD32(saved.r29,-0xC0);c->r4=native::base(session.snapshot().player);func_80040C0C(rdram,c);*c=saved;MEM_B(0,int32_t(0x800F7EA5))=flags5;MEM_B(0,int32_t(0x800F7EA6))=flags6;restore_pending=false;}
 if(session.snapshot().phase==core::Phase::Victory&&!exit_pot&&!spawn_exit(rdram,c))fail("native_exit_pot_failed");
}
// Diagnostic controller only: the native candy actors and defeat observers
// remain in charge of movement, ingestion, health and victory.
// A closed red body is ordinary edible4550 with zero healing. The native
// reserve FIFO, rather than colour, decides whether the diagnostic pilot must
// finish by tongue. An unknown stock or an owned aim is never called empty.
inline bool candy_empty_stock(uint8_t* rdram,uint32_t player){
 if(!rdram||!native::actor(rdram,player,0x4001)||native::word(rdram,0x800F8828)!=player||fatal_player(rdram,player))return false;
 std::array<uint32_t,6> eggs{};if(!boss_solo0240::read_reserves(rdram,player,eggs)||std::any_of(eggs.begin(),eggs.end(),[](auto a){return a!=0;}))return false;
 const auto data=native::word(rdram,player+0x1F4);if(native::word(rdram,data+0x1F4))return false;
 for(unsigned slot=0;slot<20;++slot){const auto child=native::word(rdram,player+0x188+4*slot);if(native::actor(rdram,child,0x4005)&&native::word(rdram,child+0x180)==player)return false;}
 return true;
}
inline bool candy_tongue_pilot(uint16_t kind,bool red,bool yellow,bool defeated,bool proved_empty_stock=false){
 return kind==0x455A&&!defeated&&(!red||yellow||proved_empty_stock);
}
struct CandyControls {uint16_t buttons=0;float x=0,y=0;};
inline CandyControls candy_controls(unsigned vi,float player_x,float player_y,float target_x,float target_y,float min_x,float max_x){
 CandyControls out;if(!std::isfinite(player_x)||!std::isfinite(player_y)||!std::isfinite(target_x)||!std::isfinite(target_y)||!std::isfinite(min_x)||!std::isfinite(max_x)||min_x>=max_x)return out;
 const float wanted=std::clamp(target_x-38.f,min_x,max_x),move=wanted-player_x,face=target_x-player_x;
 out.x=move>4?.65f:move< -4?-.65f:face>4?.35f:face< -4?-.35f:0;
 // Prepare facing and upward tongue direction before the B edge. The native
 // input dispatcher415FC accepts B before A, so a B pulse is sent alone.
 out.y=target_y<player_y-70?.8f:0;const unsigned cycle=vi%120;
 if(cycle<70)out.buttons=0x8000;
 if(cycle>=18&&cycle<100&&vi%20<5)out.buttons=0x4000;
 return out;
}
// Read-only typed food selection. Rerank only before a prepared request or
// after a proved native live-to-idle cycle; never chase a new segment mid-tongue.
inline std::optional<CandyFoodTarget> observe_candy_food(uint8_t* rdram,unsigned member){
 if(!rdram||!active()||session.snapshot().phase!=core::Phase::Fighting||member>=session.current().count||physical_room!=source_for(session.snapshot().index)||session.current().members[member].kind!=0x455A)return {};
 const auto& proof=candies[member].snapshot();const auto& seen=proof.actors[0];const auto& binding=variants::snapshot().members[member];const auto root=bound[member];
 if(!proof.enabled||proof.invalidated||proof.source_room!=logical_room||!seen.initialized||!seen.active_seen||seen.terminal_seen||seen.completion_returned||seen.disposed||seen.actor!=root||!variants::snapshot().enabled||variants::snapshot().invalidated||variants::snapshot().encounter!=session.snapshot().index||binding.actor!=root||!binding.birth||!native::actor(rdram,root,0x455A)||MEM_BU(0x17D,native::base(root))!=2||native::word(rdram,root+0x1F4)!=seen.private_data||!native::range(seen.private_data,0x24)||native::word(rdram,seen.private_data)!=seen.head||!native::actor(rdram,seen.head,0x454F)||native::word(rdram,seen.head+0x180)!=root||native::word(rdram,seen.head+0x1F4)!=seen.head_private||!native::range(seen.head_private,0x88))return {};
 if(candy_food.root==root&&candy_food.member==member&&candy_food.encounter==session.snapshot().index&&candy_food.room==physical_room&&candy_food.birth&&candy_food.birth!=binding.birth)return {};
 const auto player=session.snapshot().player;if(!native::actor(rdram,player,0x4001)||native::word(rdram,0x800F8828)!=player||fatal_player(rdram,player))return {};
 const auto request=boss_native_mouth0260::observe_ordinary_tongue(rdram,player);if(!request.observed)return {};
 const bool same_owner=candy_food.root==root&&candy_food.birth==binding.birth&&candy_food.member==member&&candy_food.encounter==session.snapshot().index&&candy_food.room==physical_room;
 if(!same_owner)candy_cycle_live=false;
 const auto phase=candy_controller.snapshot().phase;
 const bool idle=!request.live&&!request.callback_pending&&(request.phase==0||request.cancelled_idle);
 const bool finished_cycle=same_owner&&candy_cycle_live&&idle;
 const bool rerank=finished_cycle||(idle&&request.phase==0&&(phase==boss_rest_aim0260::Phase::Idle||phase==boss_rest_aim0260::Phase::Approach));
 if(request.live)candy_cycle_live=true;else if(finished_cycle)candy_cycle_live=false;
 const float player_x=std::bit_cast<float>(native::word(rdram,player+0x84)),player_y=std::bit_cast<float>(native::word(rdram,player+0x88));
 if(!std::isfinite(player_x)||!std::isfinite(player_y)||std::abs(player_x)>=100000||std::abs(player_y)>=100000)return {};
 std::array<float,80> mouth_x{},mouth_y{};boss_stick_aim0260::Mouth mouth;boss_native_mouth0260::observe_native_mouth(rdram,player,mouth_x,mouth_y,mouth);
 auto candidate=[&](uint32_t a,uint16_t kind,unsigned slot)->std::optional<CandyFoodTarget>{
  if(!native::actor(rdram,a,kind)||MEM_BU(0x18,native::base(a))!=1||native::word(rdram,a+0x180)!=(kind==0x4550?seen.head:root))return {};
  const auto metadata=native::word(rdram,a+0x174);if(!native::range(metadata,4))return {};const auto definition=native::word(rdram,metadata);if(!native::range(definition,0x20)||(MEM_HU(0x1C,native::base(definition))&0xC0)!=0xC0)return {};
  const float px=std::bit_cast<float>(native::word(rdram,a+0x84)),py=std::bit_cast<float>(native::word(rdram,a+0x88));if(!std::isfinite(px)||!std::isfinite(py)||std::abs(px)>=100000||std::abs(py)>=100000)return {};
  return CandyFoodTarget{root,seen.private_data,seen.head,seen.head_private,a,native::word(rdram,a+0x1F4),metadata,definition,native::word(rdram,a+0x168),binding.birth,physical_room,session.snapshot().index,member,slot};
 };
 if(candy_food.root==root&&candy_food.birth==binding.birth&&candy_food.member==member&&candy_food.encounter==session.snapshot().index&&candy_food.room==physical_room&&candy_food.root_private==seen.private_data&&candy_food.head==seen.head&&candy_food.head_private==seen.head_private&&candy_food.slot<12&&native::word(rdram,seen.head_private+0x54+4*candy_food.slot)==candy_food.actor){
  if(const auto keep=candidate(candy_food.actor,0x4550,candy_food.slot)){if(keep->private_data!=candy_food.private_data||keep->metadata!=candy_food.metadata||keep->definition!=candy_food.definition||keep->resource!=candy_food.resource)return {};if(!rerank)return keep;}
 }
 // The ordinary-edible head fallback, when actually licensed by native
 // data, has the same request lock as a body. The usual head has flags0.
 if(same_owner&&candy_food.root_private==seen.private_data&&candy_food.head==seen.head&&candy_food.head_private==seen.head_private&&candy_food.slot==12&&candy_food.actor==seen.head&&native::word(rdram,seen.head_private+0x84)==0){
  bool active_body=false;for(unsigned slot=0;slot<12;++slot){const auto a=native::word(rdram,seen.head_private+0x54+4*slot);if(native::actor(rdram,a,0x4550)&&MEM_BU(0x18,native::base(a))==1)active_body=true;}
  if(!active_body)if(const auto keep=candidate(seen.head,0x454F,12)){if(keep->private_data!=candy_food.private_data||keep->metadata!=candy_food.metadata||keep->definition!=candy_food.definition||keep->resource!=candy_food.resource)return {};if(!rerank)return keep;}
 }
 // An eaten/disposed target is unavailable, not permission to rotate a live,
 // prepared, jumping, facing or swallowing request onto another body actor.
 if(!rerank)return {};
 std::optional<CandyFoodTarget> best;float nearest=std::numeric_limits<float>::infinity();bool active_segment=false;
 for(unsigned slot=0;slot<12;++slot){const auto a=native::word(rdram,seen.head_private+0x54+4*slot);if(native::actor(rdram,a,0x4550)&&MEM_BU(0x18,native::base(a))==1)active_segment=true;if(const auto next=candidate(a,0x4550,slot)){const float x=std::bit_cast<float>(native::word(rdram,a+0x84)),y=std::bit_cast<float>(native::word(rdram,a+0x88));float distance=std::hypot(x-player_x,y-player_y);if(const auto ray=boss_stick_aim0260::from_mouth(mouth,player_x,player_y,x,y))distance=ray->distance;if(distance<nearest){nearest=distance;best=next;}}}
 if(best)return best;
 // Native3C510/3C660 accepts ordinary edible actors only for definition1C&C0.
 // A head with flags0 (the normal captured Candy head) is never fabricated as food.
 if(!active_segment&&native::word(rdram,seen.head_private+0x84)==0)return candidate(seen.head,0x454F,12);
 return {};
}
inline void drive_candy_input(uint8_t* rdram,unsigned member,unsigned vi,uint16_t* buttons,float* x,float* y){
 // F7EA6 selects a scene/menu context; it is not native320C4 admission.
 // Preserve scripted input only for an actual pause/context5 or unproved fight.
 if(!rdram||!buttons||!x||!y||!active()||!driver||!session.snapshot().diagnostic||session.snapshot().phase!=core::Phase::Fighting||member>=session.current().count||session.current().members[member].kind!=0x455A||physical_room!=source_for(session.snapshot().index)||MEM_BU(0,int32_t(0x800F7EA5))!=0)return;
 const auto& proof=candies[member].snapshot();const auto& seen=proof.actors[0];const auto& binding=variants::snapshot().members[member];const auto root=bound[member];
 if(!proof.enabled||proof.invalidated||proof.source_room!=logical_room||!seen.initialized||!seen.active_seen||seen.completion_returned||seen.disposed||seen.actor!=root||!variants::snapshot().enabled||variants::snapshot().invalidated||variants::snapshot().encounter!=session.snapshot().index||!binding.birth||binding.actor!=root||!native::actor(rdram,root,0x455A)||native::word(rdram,root+0x1F4)!=seen.private_data)return;
 const auto player=session.snapshot().player;if(!native::actor(rdram,player,0x4001)||native::word(rdram,0x800F8828)!=player||fatal_player(rdram,player))return;
 const auto p=native::word(rdram,player+0x1F4);if(!native::range(p,0x12D0))return;*buttons=0;*x=*y=0;
 // A real aimed egg can outlive the red-to-yellow window. Cancel it through
 // the existing native input convention before starting an ordinary tongue.
 for(unsigned slot=0;slot<20;++slot){const auto child=native::word(rdram,player+0x188+4*slot);if(native::actor(rdram,child,0x4005)&&native::word(rdram,child+0x180)==player){const bool second=native::word(rdram,p+0x1E0)==1;*buttons=(second?0:0x2000)|(vi%30<3?0x4000:0);return;}}
 const auto hp=boss_solo0240::health(rdram);if(!hp)return;const auto& arena=rooms::arena(physical_room);
 boss_rest_aim0260::Input in;in.goal=boss_rest_aim0260::Goal::ConsumeOwnedCandy;in.vi=vi;in.room=physical_room;in.visit=session.snapshot().index;in.player=player;in.active=true;in.controls=true;in.alive=true;in.grounded=MEM_BU(0xF4,native::base(p))==0;in.health=*hp;in.x=std::bit_cast<float>(native::word(rdram,player+0x84));in.y=std::bit_cast<float>(native::word(rdram,player+0x88));in.min_x=arena.min_x;in.max_x=arena.max_x;
 const auto held=native::word(rdram,p+0x164);in.held_valid=native::range(held,0x1FC)&&native::word(rdram,held)==0x19971110&&MEM_BU(6,native::base(held))!=3&&MEM_BU(0x18,native::base(held))!=0xFF;in.mouth_phase=MEM_BU(0x159,native::base(p));in.callback_pending=native::word(rdram,p+0xC0)!=0;
 const auto request=boss_native_mouth0260::observe_ordinary_tongue(rdram,player);in.request_ready=request.observed&&request.ready;in.tongue_observed=request.observed;in.tongue_live=request.live;in.tongue_cancelled_idle=request.cancelled_idle;in.max_reach=request.max_reach;in.heading_degrees=request.heading_degrees;in.heading_active=request.heading_active;
 in.old_stick={int(MEM_H(0,int32_t(0x800F897E))),int(MEM_H(0,int32_t(0x800F8980)))};in.stick_observed=boss_stick_aim0260::valid(in.old_stick);std::array<float,80> mouth_x{},mouth_y{};boss_native_mouth0260::observe_native_mouth(rdram,player,mouth_x,mouth_y,in.mouth);
 const auto selected=observe_candy_food(rdram,member);const bool changed=selected&&(selected->actor!=candy_food.actor||selected->birth!=candy_food.birth||selected->root!=candy_food.root);
 if(selected){candy_food=*selected;in.fruit=candy_food.actor;in.fruit_available=true;in.fruit_x=std::bit_cast<float>(native::word(rdram,candy_food.actor+0x84));in.fruit_y=std::bit_cast<float>(native::word(rdram,candy_food.actor+0x88));}
 const auto out=candy_controller.step(in);if(out.handled){*buttons=out.buttons;*x=out.x;*y=out.y;}
 const bool b_front=(*buttons&0x4000)&&!(candy_previous_buttons&0x4000);candy_previous_buttons=*buttons;
 if((b_front||(request.live&&vi%4==0))&&(!candy_have_log_vi||candy_log_vi!=vi)){
  candy_have_log_vi=true;candy_log_vi=vi;const auto body=native::word(rdram,p+0x160),tip=native::word(rdram,p+0x15C);const bool live_pose=request.live&&native::actor(rdram,body,0x400B)&&native::actor(rdram,tip,0x400C);const float absent=std::numeric_limits<float>::quiet_NaN();
  const float tip_x=live_pose?std::bit_cast<float>(native::word(rdram,tip+0x84)):absent,tip_y=live_pose?std::bit_cast<float>(native::word(rdram,tip+0x88)):absent;
  fprintf(stderr,"[BossCandyMouth] vi=%u member=%u target=%08X raw159=%u live=%u cancelled=%u bFront=%u native174=%.4f inputF8990=%.4f targetXY=%.3f,%.3f tipLive=%u tipXY=%.3f,%.3f bodyCallback=%08X tipCallback=%08X maxReach=%.4f controlsOnly=1\n",vi,member,in.fruit,in.mouth_phase,unsigned(request.live),unsigned(request.cancelled_idle),unsigned(b_front),request.heading_degrees,std::bit_cast<float>(native::word(rdram,0x800F8990)),in.fruit_x,in.fruit_y,unsigned(live_pose),tip_x,tip_y,native::actor(rdram,body,0x400B)?native::word(rdram,body+0x20):0,native::actor(rdram,tip,0x400C)?native::word(rdram,tip+0x20):0,in.max_reach);
 }
 if(changed||vi%120==0)fprintf(stderr,"[BossCandyInput] vi=%u member=%u target=%08X birth=%llu phase=%u hp=%u facing=%u xy=%.2f,%.2f food=%.2f,%.2f axes=%d,%d buttons=%04X nativeObservation=1 controlsOnly=1\n",vi,member,in.fruit,static_cast<unsigned long long>(selected?selected->birth:0),unsigned(candy_controller.snapshot().phase),in.health,unsigned(in.mouth.right),in.x,in.y,in.fruit_x,in.fruit_y,in.old_stick.x,in.old_stick.y,unsigned(*buttons));
}
inline bool drive_monster_exit_input(uint8_t* rdram,uint32_t player,unsigned vi,uint16_t* buttons,float* x,float* y){
 const auto& state=session.snapshot();
 if(state.phase!=core::Phase::Victory||session.current().family!=core::Family::Monsters||logical_room!=151||physical_room!=source_for(state.index)){
  monster_exit_support.reset();return false;
 }
 const auto parent=native::word(rdram,player+0x180);
 // A victory from the independently proved static upper platform has no
 // moving-rider witness. Preserve its existing ordinary pot controller.
 if(!monster_exit_support&&parent!=exit_pot&&!native::actor(rdram,parent,0x415F)&&!native::actor(rdram,parent,0x415E))return false;
 const auto route=boss_monster_exit0260::observe(rdram,player,exit_pot,physical_room,state.index,monster_exit_support?&*monster_exit_support:nullptr);
 *buttons=0;*x=*y=0;
 if(!route)return true; // Unknown geometry never grants a walk or a jump.
 if(route->support_attested)monster_exit_support=route->support;
 const int stage=int(route->stage);
 if(stage!=monster_exit_log_stage&&monster_exit_log_count<12){
  ++monster_exit_log_count;monster_exit_log_stage=stage;
  fprintf(stderr,"[BossMonsterExit] vi=%u room=%u encounter=%u stage=%u player=%08X support=%08X pot=%08X targetX=%.2f potXY=%.2f,%.2f clearBelow=%u controlsOnly=1\n",vi,physical_room,state.index,unsigned(stage),player,route->support.actor,exit_pot,route->target_x,route->pot_x,route->pot_y,unsigned(route->clear_below));
 }
 if(route->stage==boss_monster_exit0260::Stage::Descend){
  const float px=std::bit_cast<float>(native::word(rdram,player+0x84)),dx=route->target_x-px;
  *x=dx>1?.85f:dx< -1?-.85f:0;return true;
 }
 if(route->stage==boss_monster_exit0260::Stage::Rim)monster_exit_support.reset();
 boss_solo0240::drive_pot_input(rdram,player,vi,route->pot_x,0x4273,exit_pot,buttons,x,y);
 // Finish the ordinary fall before requesting a fresh rim-entry jump.
 // Reusing the generic periodic A while airborne can sustain native flutter.
 if(route->stage==boss_monster_exit0260::Stage::Approach){const auto data=native::word(rdram,player+0x1F4);if(native::range(data,0xF5)&&native::byte(rdram,data+0xF4))*buttons&=uint16_t(~0x8000u);}
 return true;
}
inline bool drive_fish_exit_input(uint8_t* rdram,uint32_t player,unsigned vi,uint16_t* buttons,float* x,float* y){
 const auto& state=session.snapshot();const bool scope=state.diagnostic&&state.phase==core::Phase::Victory&&session.current().family==core::Family::Fish&&logical_room==97&&physical_room==source_for(state.index)&&variant_epoch==state.retry_epoch;
 const auto terrain=session.current().terrain;const std::array<float,3> centers{152.f,terrain==2?408.f:360.f,terrain==2?632.f:600.f};
 const auto out=fish_exit_controller.step(rdram,scope,player,exit_pot,physical_room,state.index,state.retry_epoch,vi,centers);
 if(!out.handled)return false;*buttons=out.buttons;*x=out.x;*y=out.y;
 if(int(out.stage)!=fish_exit_log_stage||vi%120==0){fish_exit_log_stage=int(out.stage);fprintf(stderr,"[BossFishExit] vi=%u room=%u encounter=%u stage=%u player=%08X pot=%08X xy=%.3f,%.3f C4=%.3f axes=%.3f,%.3f buttons=%04X controlsOnly=1\n",vi,physical_room,state.index,unsigned(out.stage),player,exit_pot,boss_monster_aim0260::scalar(rdram,player+0x84),boss_monster_aim0260::scalar(rdram,player+0x88),boss_monster_aim0260::scalar(rdram,player+0xC4),*x,*y,unsigned(*buttons));}
 return true;
}
inline bool rest_run_path(uint8_t* rdram,float px,float py,float target){
 const auto map=boss_monster_aim0260::Map::read(rdram);if(!map||map->columns!=3||map->rows!=2||!std::isfinite(py)||std::abs(py-448.f)>1)return false;
 if(!boss_fish_exit0260::body_clear(rdram,*map,px,py,target,py))return false;
 const unsigned steps=std::max(1u,unsigned(std::ceil(std::abs(target-px)/2)));
 for(unsigned i=0;i<=steps;++i){const float at=px+(target-px)*float(i)/steps;for(float dx:{-4.f,0.f,4.f}){const auto floor=map->occupied(rdram,at+dx,py+17);if(!floor||!*floor)return false;}}
 return true;
}
inline void drive_input(uint8_t* rdram,unsigned vi,uint16_t* buttons,float* x,float* y){
 if(!active()||!driver)return;const auto phase=session.snapshot().phase;const auto player=session.snapshot().player;if(!native::actor(rdram,player,0x4001)||fatal_player(rdram,player))return;
 if(phase!=core::Phase::Victory||session.current().family!=core::Family::Monsters||logical_room!=151)monster_exit_support.reset();
 if(phase==core::Phase::Victory&&exit_pot){
  // A genuinely owned native aiming egg can survive the final target. Cancel
  // through ordinary B input before the pot's native1F8 gate is evaluated.
  for(unsigned slot=0;slot<20;++slot){const auto child=native::word(rdram,player+0x188+4*slot);if(native::actor(rdram,child,0x4005)&&native::word(rdram,child+0x180)==player){const auto p=native::word(rdram,player+0x1F4);if(!native::range(p,0x1E4))return;const bool second=native::word(rdram,p+0x1E0)==1;*buttons=(second?0:0x2000)|(vi%30<3?0x4000:0);*x=*y=0;return;}}
  // QA waits through the real terminal cleanup to expose any respawn loop.
  const auto family=session.current().family;if(family==core::Family::Invisible||family==core::Family::Genie||family==core::Family::Lips){unsigned victory_vi=0;for(unsigned n=0;n<session.current().count;++n)victory_vi=std::max(victory_vi,history[session.snapshot().index].members[n].vi);if(vi-victory_vi<600){*buttons=0;*x=*y=0;return;}}
  if(drive_fish_exit_input(rdram,player,vi,buttons,x,y))return;
  if(drive_monster_exit_input(rdram,player,vi,buttons,x,y))return;
  boss_solo0240::drive_pot_input(rdram,player,vi,exit_x,0x4273,exit_pot,buttons,x,y);
  // A spring keeps its native rider attached until a pressed-A edge. Walking
  // toward a distant pot alone cannot release that grip (nativeC928/CCEC).
  const auto parent=native::word(rdram,player+0x180);if((native::actor(rdram,parent,0x410A)||native::actor(rdram,parent,0x469E))&&native::byte(rdram,parent+0x17D)==2&&vi%120<30)*buttons|=0x8000;return;
 }
 for(unsigned candy=0;phase==core::Phase::Fighting&&candy<session.current().count;++candy)if(candy_tongue_pilot(session.current().members[candy].kind,session.current().members[candy].red,variants::yellow(),(session.snapshot().defeated&(1u<<candy))!=0,candy_empty_stock(rdram,player))){
  drive_candy_input(rdram,candy,vi,buttons,x,y);return;
 }
 if(phase!=core::Phase::Rest)return;*buttons=0;*x=*y=0;if(vi<rest_vi+180)return;
 // Diagnostic rest controls use the same native mouth/stick observations as
 // the combat pilot. Native food, HP, fruit ledger and quotas remain in charge.
 const auto hp=boss_solo0240::health(rdram);const float px=std::bit_cast<float>(native::word(rdram,player+0x84)),py=std::bit_cast<float>(native::word(rdram,player+0x88));
 const auto mouth_private=native::word(rdram,player+0x1F4);if(!native::range(mouth_private,0x12D0))return;
 const auto held=native::word(rdram,mouth_private+0x164);
 boss_rest_aim0260::Input in;in.vi=vi;in.room=physical_room;in.visit=session.snapshot().completed;in.player=player;in.active=session.snapshot().diagnostic;in.controls=MEM_BU(0,int32_t(0x800F7EA6))==1;in.alive=!fatal_player(rdram,player);in.grounded=MEM_BU(0xF4,native::base(mouth_private))==0;in.health=hp?*hp:9;in.x=px;in.y=py;in.min_x=40;in.max_x=724;
 in.held_valid=native::range(held,0x1FC)&&native::word(rdram,held)==0x19971110&&MEM_BU(6,native::base(held))!=3&&MEM_BU(0x18,native::base(held))!=0xFF;in.mouth_phase=MEM_BU(0x159,native::base(mouth_private));in.callback_pending=native::word(rdram,mouth_private+0xC0)!=0;
 const auto request=boss_native_mouth0260::observe_ordinary_tongue(rdram,player);in.request_ready=request.observed&&request.ready;in.tongue_observed=request.observed;in.tongue_live=request.live;in.tongue_cancelled_idle=request.cancelled_idle;in.max_reach=request.max_reach;in.heading_degrees=request.heading_degrees;in.heading_active=request.heading_active;
 in.old_stick={int(MEM_H(0,int32_t(0x800F897E))),int(MEM_H(0,int32_t(0x800F8980)))};in.stick_observed=boss_stick_aim0260::valid(in.old_stick);
 std::array<float,80> mouth_x{},mouth_y{};boss_native_mouth0260::observe_native_mouth(rdram,player,mouth_x,mouth_y,in.mouth);
 float nearest_fruit=1e9;const auto& tree=boss_tree0240::snapshot();
 for(unsigned n=0;n<5;++n){const auto a=tree.fruitActors[n];if((tree.consumedmask&(1u<<n))||!native::actor(rdram,a,boss_tree0240::kinds[n+1])||MEM_BU(0x18,native::base(a))==0xFF)continue;const float fx=std::bit_cast<float>(native::word(rdram,a+0x84)),fy=std::bit_cast<float>(native::word(rdram,a+0x88));const float distance=std::hypot(fx-px,fy-py);if(std::isfinite(distance)&&distance<nearest_fruit){nearest_fruit=distance;in.fruit=a;in.fruit_x=fx;in.fruit_y=fy;in.fruit_available=true;}}
 const auto& hybrid=boss_tree_hybrid0260::snapshot();const auto& token=hybrid.token;
 if(!hybrid.invalidated&&token.actor&&token.birth&&native::actor(rdram,token.actor,token.kind)&&MEM_BU(0x18,native::base(token.actor))!=0xFF&&native::word(rdram,token.actor+0x168)==token.resource&&native::word(rdram,token.actor+0x174)==token.metadata&&native::range(token.metadata,4)&&native::word(rdram,token.metadata)==token.definition){
  const float fx=std::bit_cast<float>(native::word(rdram,token.actor+0x84)),fy=std::bit_cast<float>(native::word(rdram,token.actor+0x88));const float distance=std::hypot(fx-px,fy-py);
  if(std::isfinite(distance)&&distance<nearest_fruit){in.fruit=token.actor;in.fruit_x=fx;in.fruit_y=fy;in.fruit_available=true;}
 }
 const auto food_input=rest_controller.step(in);if(food_input.handled){*buttons=food_input.buttons;*x=food_input.x;*y=food_input.y;return;}
 std::array<uint32_t,6> eggs{};if(boss_solo0240::read_reserves(rdram,player,eggs)&&std::count_if(eggs.begin(),eggs.end(),[](auto a){return a!=0;})==6){boss_solo0240::drive_pot_input(rdram,player,vi,boss_rest_data0240::pot_x,0x4274,0,buttons,x,y);return;}
 uint32_t nearest=0;float best=1e9;constexpr uint32_t q=0x800F8814;const auto cap=MEM_H(8,native::base(q));const auto table=native::word(rdram,q+0x10);if(cap>0&&cap<=150&&native::range(table,unsigned(cap)*4))for(int n=0;n<cap;++n){auto a=native::word(rdram,table+n*4);if(native::actor(rdram,a,0x4002)&&MEM_BU(0x17D,native::base(a))==2&&!native::word(rdram,a+0x180)){float dx=std::bit_cast<float>(native::word(rdram,a+0x84))-px;if(std::abs(dx)<best){nearest=a;best=std::abs(dx);}}}
 const float target=nearest?std::bit_cast<float>(native::word(rdram,nearest+0x84)):boss_rest_data0240::egg_block_x;const float dx=target-px;*x=std::abs(dx)>5?(dx>0?.65f:-.65f):0;if(!nearest&&std::abs(dx)<18)*buttons=vi%80<20?0x8000:0;
 boss_run_travel0260::Frame run;run.enabled=driver&&boss_solo0240::read_reserves(rdram,player,eggs)&&std::none_of(eggs.begin(),eggs.end(),[](auto a){return a!=0;})&&!native::word(rdram,player+0x180);run.diagnostic=session.snapshot().diagnostic;run.rest_active=phase==core::Phase::Rest;run.source_room=physical_room;run.controls=in.controls;run.alive=in.alive;run.grounded=in.grounded;run.player=player;run.singleton=native::word(rdram,0x800F8828);run.buttons=*buttons;run.x=px;run.target_x=target;run.velocity_x=boss_monster_aim0260::scalar(rdram,player+0xC4);run.world_factor=boss_monster_aim0260::scalar(rdram,player+0x100);run.external_x=boss_monster_aim0260::scalar(rdram,player+0x10C);run.aim_clear=!native::word(rdram,mouth_private+0x1F4);for(unsigned slot=0;slot<20;++slot){const auto child=native::word(rdram,player+0x188+4*slot);if((native::actor(rdram,child,0x4005)||native::actor(rdram,child,0x4003))&&native::word(rdram,child+0x180)==player)run.aim_clear=false;}run.mouth_clear=!held&&!in.callback_pending&&!in.mouth_phase;run.tongue_clear=request.observed&&request.ready&&!request.live;run.path_attested=std::abs(dx)>=boss_run_travel0260::minimum_distance&&rest_run_path(rdram,px,py,target);
 if(const auto fast=boss_run_travel0260::axis(run))*x=*fast;

}
} // namespace workshop::boss_campaign0250
#include "boss_checkpoint_runtime0260.hpp"
