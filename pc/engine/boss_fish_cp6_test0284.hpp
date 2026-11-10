#pragma once
#include "boss_route_checkpoint_test0260.hpp"
#include "boss_checkpoint_pose0260.hpp"
#include "boss_rest_data0240.hpp"

namespace workshop::boss_fish_cp6_test0284 {
namespace b=boss_battle0250;namespace n=boss_native;namespace q=boss_route_checkpoint_test0260;
namespace inv=boss_checkpoint_inventory0260;namespace pose=boss_checkpoint_pose0260;namespace a=boss_checkpoint_actor0260;
inline constexpr uint64_t required_seed=37784098137700ull;
enum class StartingScope:uint8_t {Full,FixtureRest6};
enum class Goal:uint8_t {ConsumedReplay,Rebirth};
enum class Phase:uint8_t {Disabled,Prefix,Wake,Companion,SeekDeath,AwaitRetry,Replay,ReturnRest,Complete,Failed};
struct Frame:q::Frame {
 bool target_plan=false,prefix_receipts=false,baseline=false,fight_ready=false,focused_fixture_ready=false;
 bool companion_receipt=false,companion_consumed=false,companion_pending=false,members_live=false,replay_receipts=false;
 std::array<uint32_t,2> members{};std::array<uint64_t,2> births{};
 const pose::Payload* saved_pose=nullptr;unsigned cargo_epoch=0,fight_ready_vi=0;
};
struct Output {bool handled=false,companion=false;uint16_t buttons=0;float x=0,y=0;bool intro_ack_only=false;};
struct Snapshot {
 bool enabled=false;StartingScope starting_scope=StartingScope::Full;Goal goal=Goal::ConsumedReplay;Phase phase=Phase::Disabled;uint32_t player=0;unsigned last_vi=0,stage_vi=0;
 unsigned activation_vi=0,companion_vi=0,death_vi=0,restored_vi=0,win_vi=0,complete_vi=0,live_loss_vi=0;
 std::array<uint32_t,2> first_members{},replayed_members{};std::array<uint64_t,2> first_births{},replayed_births{};
 const char* failure="";
};
// Read-only witness copied from production checkpoints::death_call, restricted
// here to the actual swallowed path42148->42174. RA alone is insufficient.
inline bool swallow_death(uint8_t* ram,const recomp_context* c,uint32_t player){
 if(!ram||!c||!inv::fatal_death_state(ram,player)||!n::actor(ram,player,0x4001)||n::word(ram,0x800F8828)!=player||uint32_t(c->r4)!=player||uint32_t(c->r31)!=0x8004217C)return false;
 const auto p=n::word(ram,player+0x1F4),sp=uint32_t(c->r29);
 return inv::pointer(p,0x12D0)&&inv::pointer(sp,0x28)&&n::byte(ram,player+0x18)==0xA&&n::byte(ram,player+0x30)==0xC&&n::word(ram,player+0x20)==0x8003085C&&n::word(ram,p+0xC0)==0x8002F5F4&&
  n::word(ram,sp+0x14)==0x80030878&&n::word(ram,sp+0x1C)==0x8002F5F4&&n::word(ram,sp+0x20)==player&&n::word(ram,sp+0x24)==p;
}
class Controller {
 Snapshot s_{};std::string token_;inv::Payload activation_{};pose::Payload plane_{};q::Ledger death_ledger_{};
 unsigned wake_jump_=0,wake_release_=0,wake_attempts_=0,victory_cargo_=0;bool released_=false,replay_bound_=false;
 uint64_t wake_birth_=0;unsigned first_pair_ready_vi_=0,replay_pair_ready_vi_=0;
 const char* label()const{if(s_.goal==Goal::Rebirth)return "BossFishCP6RebirthQA";return s_.starting_scope==StartingScope::FixtureRest6?"BossFishCP6FocusedQA":"BossFishCP6QA";}
 void fail(const char* why){if(s_.phase==Phase::Failed)return;s_.phase=Phase::Failed;s_.failure=why;std::fprintf(stderr,"[%s] phase=failed vi=%u player=%08X reason=%s inputOnly=1 noGameplayFailure=1\n",label(),s_.last_vi,s_.player,why);}
 void emit(const char* event,const Frame& f)const{
  const auto& v=*f.session;
  if(s_.goal==Goal::Rebirth){std::fprintf(stderr,"[%s] phase=%s vi=%u player=%08X index=%u completed=%u epoch=%u savedCheckpoint=%u checkpoint=6 mask=%u activationVI=%u companionVI=%u deathVI=%u restoredVI=%u members=%08X,%08X births=%llu,%llu defeated=%u ledgerEqual=%u nativeRestoreVerified=%u inputOnly=1\n",label(),event,f.vi,s_.player,v.index,v.completed,v.retry_epoch,v.checkpoint_completed,v.checkpoint_awake_mask,s_.activation_vi,s_.companion_vi,s_.death_vi,s_.restored_vi,f.members[0],f.members[1],static_cast<unsigned long long>(f.births[0]),static_cast<unsigned long long>(f.births[1]),unsigned(v.defeated),unsigned(f.ledger.valid&&f.ledger==death_ledger_),unsigned(s_.restored_vi!=0));return;}
  std::fprintf(stderr,"[%s] phase=%s vi=%u player=%08X index=%u completed=%u epoch=%u savedCheckpoint=%u checkpoint=6 mask=%u activationVI=%u companionVI=%u deathVI=%u restoredVI=%u members=%08X,%08X births=%llu,%llu inputOnly=1\n",label(),event,f.vi,s_.player,v.index,v.completed,v.retry_epoch,v.checkpoint_completed,v.checkpoint_awake_mask,s_.activation_vi,s_.companion_vi,s_.death_vi,s_.restored_vi,f.members[0],f.members[1],static_cast<unsigned long long>(f.births[0]),static_cast<unsigned long long>(f.births[1]));
 }
 bool scope(uint8_t* ram,const Frame& f){
  if(!ram||!f.session||!f.diagnostic||!f.driver||!f.session->diagnostic||f.seed!=required_seed||!f.target_plan||f.session->token.empty()){fail("wrong_qa_scope");return false;}
  const auto& v=*f.session;if(f.vi<s_.last_vi){fail("vi_reversed");return false;}s_.last_vi=f.vi;
  if(f.vi>(s_.starting_scope==StartingScope::FixtureRest6?36000u:72000u)){fail("bounded_deadline");return false;}
  if(!s_.player){if(!v.player)return true;const bool focused=s_.starting_scope==StartingScope::FixtureRest6;if(focused&&!f.focused_fixture_ready)return true;
   const bool start=focused?(v.index==5&&v.completed==6&&v.max_completed==6&&f.room==83&&f.logical_room==83&&f.baseline):(!v.index&&!v.completed&&!v.max_completed);
   if(!start||v.retry_epoch||(focused&&(v.checkpoint_completed||v.checkpoint_awake_mask))||v.phase!=(focused?b::Phase::Rest:b::Phase::Fighting)||!n::actor(ram,v.player,0x4001)||n::word(ram,0x800F8828)!=v.player){fail(focused?"not_declared_focused_entry":"not_genuine_first_entry");return false;}s_.player=v.player;token_=v.token;s_.stage_vi=f.vi;emit("first-entry",f);}
  if(v.player!=s_.player||v.token!=token_||!n::actor(ram,s_.player,0x4001)||n::word(ram,0x800F8828)!=s_.player){fail("player_or_run_changed");return false;}
  if(v.phase==b::Phase::Failed&&v.failure!="yoshi_defeated"){fail("campaign_rejected");return false;}return true;
 }
 bool point(uint8_t* ram,const Frame& f)const{
  if(!f.characters||!f.characters->enabled||f.characters->invalidated||f.characters->room!=83)return false;const auto& p=f.characters->binding;
  return p.birth&&p.room==83&&p.milestone==6&&p.index==2&&p.packed==a::source_packed&&n::actor(ram,p.actor,0x4509)&&a::pointer(p.record,12)&&a::pointer(p.resource,0x48)&&a::pointer(p.private_data,0x280)&&a::pointer(p.overlay,a::overlay_extent)&&
   n::word(ram,p.actor+0x1D8)==p.record&&n::word(ram,p.actor+0x1DC)==p.packed&&n::word(ram,p.record)==0xFFFFFFFF&&n::word(ram,p.record+4)==p.x&&n::word(ram,p.record+8)==p.y&&n::word(ram,p.actor+0x168)==p.resource&&n::word(ram,p.resource+0x40)==p.overlay&&n::word(ram,p.resource+0x44)==p.overlay+a::initializer_offset&&n::word(ram,p.actor+0x1F4)==p.private_data&&n::word(ram,p.private_data+0x27C)==2;
 }
 bool activate(uint8_t* ram,const Frame& f){
  const auto& v=*f.session;if(v.checkpoint_completed!=6||!(v.checkpoint_awake_mask&4))return false;
  if(!f.saved||!f.saved_pose||!inv::valid(*f.saved)||!pose::owner(*f.saved_pose,*f.saved)||f.saved->player!=s_.player||f.saved->activation_vi<s_.stage_vi||f.saved->activation_vi>f.vi||v.completed!=6||v.index!=5||v.phase!=b::Phase::Rest||v.retry_epoch||!f.ledger.valid||!inv::same_player(ram,*f.saved)){fail("unproved_cp6_wake_inventory");return false;}
  activation_=*f.saved;plane_=*f.saved_pose;s_.activation_vi=activation_.activation_vi;s_.phase=Phase::Companion;s_.stage_vi=f.vi;
  std::fprintf(stderr,"[%s] phase=activated checkpoint=6 index=5 epoch=0 vi=%u player=%08X activationVI=%u hp=%u eggs=%u plane=%08X inputOnly=1 nativeWake=1\n",label(),f.vi,s_.player,activation_.activation_vi,activation_.hp,activation_.count,plane_.z);return true;
 }
 Output wake(uint8_t* ram,const Frame& f){
  Output out{true};if(!f.controls||inv::fatal_death_state(ram,s_.player)){released_=false;return out;}
  const auto priv=n::word(ram,s_.player+0x1F4);if(!inv::pointer(priv,0x12F0))return out;
  const float px=std::bit_cast<float>(n::word(ram,s_.player+0x84)),py=std::bit_cast<float>(n::word(ram,s_.player+0x88));if(!std::isfinite(px)||!std::isfinite(py)){fail("wake_position_invalid");return out;}
  const bool live=point(ram,f);const float target=live?std::bit_cast<float>(n::word(ram,f.characters->binding.actor+0x84)):boss_rest_data0240::checkpoint_x;
  if(!std::isfinite(target)){fail("point_position_invalid");return out;}if(std::abs(target-px)>1)out.x=target>px?.65f:-.65f;
  const bool posture=n::byte(ram,s_.player+0x18)==1&&n::byte(ram,s_.player+0x17D)==2&&!n::byte(ram,s_.player+0x165)&&!n::word(ram,s_.player+0x180)&&!n::word(ram,priv+0x12EC);
  if(wake_jump_&&f.vi-wake_jump_<18){if(live&&posture&&wake_birth_==f.characters->binding.birth)out.buttons=0x8000;return out;}
  if(!live||!posture||std::abs(target-px)>8||std::abs(py-std::bit_cast<float>(n::word(ram,f.characters->binding.actor+0x88)))>48||n::byte(ram,priv+0xF4)){released_=false;return out;}
  if(wake_jump_&&f.vi-wake_jump_<90)return out;
  if(!released_){released_=true;wake_release_=f.vi;return out;}if(f.vi<=wake_release_)return out;
  if(wake_attempts_>=8){fail("native_no3_wake_missing");return out;}++wake_attempts_;wake_jump_=f.vi;wake_birth_=f.characters->binding.birth;released_=false;out.buttons=0x8000;return out;
 }
public:
 void configure(bool on=false,StartingScope start=StartingScope::Full,Goal goal=Goal::ConsumedReplay){*this=Controller{};s_.starting_scope=start;s_.goal=goal;s_.enabled=on;s_.phase=on?Phase::Prefix:Phase::Disabled;}
 const Snapshot& snapshot()const{return s_;}bool complete()const{return s_.enabled&&s_.phase==Phase::Complete;}bool failed()const{return s_.enabled&&s_.phase==Phase::Failed;}
 Output frame(uint8_t* ram,const Frame& f){
  if(!s_.enabled||s_.phase==Phase::Disabled||s_.phase==Phase::Complete)return {};if(s_.phase==Phase::Failed)return {true};if(!scope(ram,f))return {true};if(!s_.player)return {};
  const auto& v=*f.session;
  if(s_.phase==Phase::Prefix){
   if(s_.starting_scope==StartingScope::FixtureRest6){
    if(v.retry_epoch||v.completed!=6||v.max_completed!=6||v.index!=5||v.phase!=b::Phase::Rest||f.room!=83||f.logical_room!=83||!f.focused_fixture_ready||!f.baseline){fail("unproved_initial_native_rest6");return {true};}
    s_.phase=Phase::Wake;s_.stage_vi=f.vi;emit("fixture-rest-entry",f);
   }else{
    if(v.retry_epoch||v.completed>6||v.index>5){fail("prefix_skipped_or_retried");return {true};}
    if(v.phase==b::Phase::Failed){fail("prefix_death");return {true};}
    if(v.phase!=b::Phase::Rest||v.completed!=6)return {};
    if(v.index!=5||v.max_completed!=6||!f.prefix_receipts||!f.baseline){fail("six_native_victories_missing");return {true};}s_.phase=Phase::Wake;s_.stage_vi=f.vi;emit("prefix-six",f);
   }
  }
  if(s_.phase==Phase::Wake){
   if(f.room!=83||v.phase!=b::Phase::Rest||v.completed!=6||v.index!=5||v.retry_epoch){fail("left_rest_without_no3");return {true};}
   if(activate(ram,f))return {};if(failed())return {true};if(f.vi-s_.stage_vi>7200){fail("no3_wake_deadline");return {true};}return wake(ram,f);
  }
  if(s_.phase==Phase::Companion){
   if(v.retry_epoch||v.completed!=6||v.checkpoint_completed!=6){fail("progress_before_intended_death");return {true};}
   if(v.phase==b::Phase::Rest||v.phase==b::Phase::ToFight)return {};
   if(v.phase!=b::Phase::Fighting||v.index!=6||f.room!=173||f.logical_room!=97){fail("wrong_fish_entry");return {true};}
   if(!f.fight_ready)return {};
   if(!s_.first_births[0]){
    if(!first_pair_ready_vi_)first_pair_ready_vi_=f.fight_ready_vi?f.fight_ready_vi:f.vi;
    if(first_pair_ready_vi_>f.vi||f.vi-first_pair_ready_vi_>=600){fail("first_fish_pair_missing");return {true};}
    if(!f.members_live){Output waiting{true};waiting.intro_ack_only=true;return waiting;}
    s_.first_members=f.members;s_.first_births=f.births;s_.stage_vi=f.vi;emit("first-pair",f);
   }
   if(s_.goal==Goal::Rebirth){
    if(v.defeated){fail("live_loss_started_after_defeat");return {true};}
    s_.live_loss_vi=f.vi;s_.phase=Phase::SeekDeath;s_.stage_vi=f.vi;emit("live-loss-start",f);return {true};
   }
   if(v.defeated&1){fail("fish_defeated_before_loss");return {true};}if(f.vi-s_.stage_vi>12000){fail("companion_deadline");return {true};}
   if(v.defeated==2){if(f.companion_receipt&&f.companion_pending)return {true};if(!f.companion_receipt||!f.companion_consumed){fail("companion_defeat_not_consumed");return {true};}s_.companion_vi=f.vi;s_.phase=Phase::SeekDeath;s_.stage_vi=f.vi;emit("companion-consumed",f);}
   else return {true,true};
  }
  if(s_.phase==Phase::SeekDeath){
   if(v.retry_epoch||v.completed!=6||v.index!=6||v.defeated!=(s_.goal==Goal::Rebirth?0u:2u)||f.room!=173||f.logical_room!=97){fail("loss_scope_changed");return {true};}
   if(f.vi-s_.stage_vi>7200){fail("swallow_death_deadline");return {true};}
   Output out{true};if(inv::fatal_death_state(ram,s_.player))return out;
   if(!f.controls){out.intro_ack_only=s_.goal==Goal::Rebirth;return out;}
   const float px=std::bit_cast<float>(n::word(ram,s_.player+0x84));if(!std::isfinite(px)){fail("death_position_invalid");return out;}
   // Exact source97 Fish arena: ordinary walk near the central mouth lane.
   // This reproduces the existing swallow pilot380/388 target, never a warp.
   const float target=((f.vi-s_.stage_vi)/40)%2?388.f:380.f;out.x=std::abs(px-target)>1.25f?(px<target?.55f:-.55f):0;return out;
  }
  if(s_.phase==Phase::AwaitRetry){
   if(f.vi-s_.stage_vi>1800){fail("retry_deadline");return {true};}if(!v.retry_epoch)return {true};
   if(v.retry_epoch!=1){fail("retry_epoch_changed");return {true};}if(f.retry_restored_epoch!=1)return {true};
   if(f.room!=83||v.phase!=b::Phase::Rest||v.completed!=6||v.index!=5||v.max_completed!=6||v.checkpoint_completed!=6||!f.saved||*f.saved!=activation_||!f.saved_pose||*f.saved_pose!=plane_||!inv::matches(ram,activation_)||!pose::matches(ram,plane_,activation_)||!f.ledger.valid||f.ledger!=death_ledger_){fail("retry_checkpoint_payload_changed");return {true};}
   s_.restored_vi=f.vi;s_.phase=Phase::Replay;s_.stage_vi=f.vi;emit("restored",f);return {true};
  }
  if(s_.phase==Phase::Replay){
   if(v.retry_epoch!=1||v.checkpoint_completed!=6||v.completed<6||v.completed>7){fail("replay_scope_changed");return {true};}
   if(v.phase==b::Phase::Rest&&v.completed==6)return {};
   if(v.phase==b::Phase::ToFight)return {};
   if(v.index!=6||f.room!=173||f.logical_room!=97){fail("replay_room_changed");return {true};}
   if(!replay_bound_&&f.fight_ready){
    if(!replay_pair_ready_vi_)replay_pair_ready_vi_=f.fight_ready_vi?f.fight_ready_vi:f.vi;
    if(replay_pair_ready_vi_>f.vi||f.vi-replay_pair_ready_vi_>=600){fail("replayed_pair_missing_or_stale");return {true};}
    if(!f.members_live){Output waiting{true};waiting.intro_ack_only=true;return waiting;}
    if(f.births[0]<=s_.first_births[0]||f.births[1]<=s_.first_births[1]){fail("replayed_pair_missing_or_stale");return {true};}
    replay_bound_=true;s_.replayed_members=f.members;s_.replayed_births=f.births;emit("replayed-pair",f);
   }
   if(s_.goal==Goal::Rebirth&&replay_bound_){
    if(v.phase!=b::Phase::Fighting||v.completed!=6||v.defeated||!f.members_live||!s_.restored_vi||!f.ledger.valid||f.ledger!=death_ledger_){fail("live_rebirth_scope_changed");return {true};}
    s_.complete_vi=f.vi;s_.phase=Phase::Complete;emit("live-rebirth-verified",f);emit("complete-rebirth",f);return {true};
   }
   if(v.phase==b::Phase::Failed){fail("replay_death");return {true};}
   if(v.phase==b::Phase::Victory){if(!replay_bound_||v.completed!=7||v.defeated!=3||!f.replay_receipts){fail("two_native_replay_victories_missing");return {true};}s_.win_vi=f.vi;victory_cargo_=f.cargo_epoch;s_.phase=Phase::ReturnRest;s_.stage_vi=f.vi;emit("won-pair",f);}
   else if(f.vi-s_.stage_vi>18000){fail("replay_victory_deadline");return {true};}
   return {};
  }
  if(s_.phase==Phase::ReturnRest){
   if(v.retry_epoch!=1||v.completed!=7||v.checkpoint_completed!=6||v.index!=6){fail("post_win_progress_changed");return {true};}
   if(f.vi-s_.stage_vi>7200){fail("victory_pot_deadline");return {true};}
   if(v.phase==b::Phase::Rest&&f.room==83&&f.logical_room==83){if(f.cargo_epoch<=victory_cargo_||!f.replay_receipts){fail("native_pot_or_receipts_missing");return {true};}s_.complete_vi=f.vi;s_.phase=Phase::Complete;emit("complete",f);return {true};}
   if(v.phase!=b::Phase::Victory&&v.phase!=b::Phase::ToRest){fail("post_win_phase_changed");return {true};}return {};
  }return {};
 }
 void native_death(uint8_t* ram,const recomp_context* c,const Frame& f){
  if(!s_.enabled||s_.phase!=Phase::SeekDeath||!scope(ram,f)||!swallow_death(ram,c,s_.player))return;
  const auto& v=*f.session;if(v.retry_epoch||v.index!=6||v.completed!=6||v.defeated!=(s_.goal==Goal::Rebirth?0u:2u)||v.checkpoint_completed!=6||f.room!=173||f.logical_room!=97||!f.saved||*f.saved!=activation_||!f.ledger.valid||f.vi<=(s_.goal==Goal::Rebirth?s_.live_loss_vi:s_.companion_vi)){fail("swallow_death_scope_changed");return;}
  death_ledger_=f.ledger;s_.death_vi=f.vi;s_.stage_vi=f.vi;s_.phase=Phase::AwaitRetry;emit("native-swallow-death",f);
 }
};
} // namespace workshop::boss_fish_cp6_test0284
