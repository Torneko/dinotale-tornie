#pragma once
#include "boss_checkpoint_runtime0260.hpp"
#include "boss_test_loadout0260.hpp"

namespace workshop::boss_fish_cp6_fixture0284 {
namespace c=boss_campaign0250;namespace b=boss_battle0250;namespace n=boss_native;
namespace inv=boss_checkpoint_inventory0260;namespace pose=boss_checkpoint_pose0260;
// Isolated INITIAL native Rest83 alias, with six UNPLAYED HOST fights and
// one initial native8/6 request. No cargo, transport, win or checkpoint exists.
struct State {bool enabled=false,started=false,entered=false,arrived=false,ready=false,failed=false;unsigned room_ready_vi=0,vi=0;uint32_t info=0,player=0;const char* failure="";};
inline State state;inline boss_test_loadout0260::Controller loadout;
inline void configure(bool on=false){state={};state.enabled=on;loadout.reset();}
inline bool fail(const char* why,unsigned vi=0){if(!state.failed){state.failed=true;state.failure=why;std::fprintf(stderr,"[BossFishCP6FixtureQA] phase=failed vi=%u reason=%s hostProgressFixture=6 forcedInitialHealthEggs=1 noGameplayFailure=1\n",vi,why);}return false;}
inline bool start(){
 if(!state.enabled||state.started||!c::enabled||!c::driver)return fail("focused_host_progress_scope_rejected");
 for(unsigned index=0;index<7;++index)for(const auto& p:c::history[index].members)if(p.actor||p.vi||p.initialized||p.active||p.terminal||p.causal_hit)return fail("focused_fixture_has_prior_native_receipt");
 if(!c::session.diagnostic_cp6_rest6_fixture())return fail("focused_host_progress_scope_rejected");
 state.started=true;std::fprintf(stderr,"[BossFishCP6FixtureQA] phase=configured index=5 completed=6 maxCompleted=6 initialHostProgressFixture=6 initialHealthEggsFixture=1 hp=8 eggs=6 earlierNativeWins=0 sixNativePrefixVictories=0 forcedCheckpoint=0 nativeReceipts=0 awakeMask=0 initialNativeRoom=83 initialAlias=52 once=1\n");return true;
}
inline bool pending(){return state.enabled&&state.started&&!state.ready&&!state.failed;}
inline bool cold_scope(){const auto& s=c::session.snapshot();return pending()&&c::enabled&&c::driver&&s.diagnostic&&!s.token.empty()&&c::session.plan().seed==37784098137700ull&&s.index==5&&s.completed==6&&s.max_completed==6&&!s.retry_epoch&&!s.defeated&&!s.checkpoint_completed&&!s.checkpoint_awake_mask&&(state.arrived?(s.phase==b::Phase::Rest&&s.player==state.player):(s.phase==b::Phase::Armed&&!s.player));}
inline unsigned initial_source(unsigned ordinary){return pending()?83u:ordinary;}
// Loader intercepts only the declared first52 alias. All subsequent room
// entries and ready receipts use the untouched campaign Runtime.
inline bool enter_room(unsigned raw){
 if(!pending()||raw!=52)return false;
 if(!cold_scope()||state.entered){fail("focused_initial_room_scope_rejected",c::current_vi);return true;}
 state.entered=true;c::monster_exit_support.reset();c::fish_exit_controller.reset();c::fish_exit_log_stage=-1;c::physical_room=c::logical_room=83;c::ready_info=0;c::variants::disable();c::rest_floor_encounter=5;c::rest_floor_guard.reset(83,5);boss_tree0240::enter_room(83);c::rest_heart.enter_room(83);c::checkpoints::enter_room(83);
 std::fprintf(stderr,"[BossFishCP6FixtureQA] phase=initial-native-room alias=52 room=83 index=5 completed=6 descriptorAndPlacements=1 hostInitialRoomFixture=1 noNativeTransportClaim=1\n");return true;
}
inline bool room_ready(uint8_t* ram,const recomp_context* ctx,unsigned vi){
 if(!pending()||!state.entered)return false;
 if(!cold_scope()||!ram||!ctx||!n::range(uint32_t(ctx->r4),0x128)||n::byte(ram,uint32_t(ctx->r4)+1)!=52){fail("focused_initial_room_ready_rejected",vi);return true;}
 const auto info=uint32_t(ctx->r4);if(state.info&&state.info!=info){fail("focused_initial_room_info_changed",vi);return true;}
 if(!state.info){state.info=info;state.room_ready_vi=vi;}c::ready_info=info;
 std::fprintf(stderr,"[BossFishCP6FixtureQA] phase=native-room-ready alias=52 room=83 info=%08X vi=%u nativeObservation=1\n",info,vi);return true;
}
// True holds only the campaign post-frame during this explicitly cold start.
// Native room/actor updates, floor stamps and checkpoint constructors continue.
inline bool pre_fence(uint8_t* ram,recomp_context* ctx,unsigned vi){
 if(state.enabled&&state.failed)return true;
 if(!pending())return false;c::current_vi=vi;
 if(!cold_scope()){fail("focused_initial_rest_scope_rejected",vi);return true;}
 if(!state.entered||!state.info||vi<=state.room_ready_vi)return true;
 if(vi-state.room_ready_vi>1800){fail("focused_initial_loadout_deadline",vi);return true;}
 if(!ram||!ctx){fail("focused_initial_native_context_missing",vi);return true;}
 const auto player=n::word(ram,0x800F8828);if(!n::actor(ram,player,0x4001)||n::byte(ram,player+0x17D)!=2||n::byte(ram,player+0x165)||n::byte(ram,player+0x18)!=1||n::word(ram,player+0x180))return true;
 const auto priv=n::word(ram,player+0x1F4);if(!inv::pointer(priv,0x12F0)||n::word(ram,priv+0x12EC)||n::byte(ram,0x800F7EA5)||n::byte(ram,0x800F7EA6)!=1)return true;
 if(!c::rest_floor_guard.checkpoint_support_ready(ram,83,5,c::overlays[boss_rest_floor0260::overlay_section]))return true;
 if(c::physical_room!=83||c::logical_room!=83||c::ready_info!=state.info||!n::range(state.info,0x128)||n::byte(ram,state.info+1)!=52||c::checkpoints::baseline_ready){fail("focused_native_ready_scope_rejected",vi);return true;}
 if(!state.arrived){if(!c::session.diagnostic_bind_initial_rest6(player)){fail("focused_initial_rest_binding_rejected",vi);return true;}state.player=player;state.arrived=true;c::rest_vi=vi;
  std::fprintf(stderr,"[BossBattle] rest-ready completed=6 player=%08X treeConsumed=%u treeRegens=%u vi=%u\n",player,boss_tree0240::snapshot().totalConsumed,boss_tree0240::snapshot().regenerations,vi);}
 if(state.player!=player){fail("focused_initial_player_changed",vi);return true;}
 boss_test_loadout0260::Frame f;f.requested=f.automatic=f.diagnostic=f.visible=f.rest_ready=f.cp6_focused_target=true;f.case_index=f.index=5;f.room=f.source_room=83;f.seed=c::session.plan().seed;f.vi=vi;f.player=player;f.ready={state.room_ready_vi,5,83,83,0,player,priv};
 if(!loadout.frame(ram,ctx,f)){fail(loadout.snapshot().failure.c_str(),vi);return true;}if(!loadout.snapshot().applied)return true;
 inv::Payload value;pose::Payload plane;if(!inv::capture(ram,player,vi,value)||!pose::capture(ram,value,plane))return true;
 if(value.hp!=8||value.count!=6||loadout.snapshot().player!=player){fail("focused_initial_native_readback_rejected",vi);return true;}
 c::checkpoints::saved_inventory[0]=value;c::checkpoints::saved_pose[0]=plane;c::checkpoints::baseline_ready=true;c::ready_info=0;state.ready=true;state.vi=vi;
 std::fprintf(stderr,"[BossFishCP6FixtureQA] phase=baseline-native-captured index=5 completed=6 player=%08X vi=%u hp=%u eggs=%u plane=%08X hostBaselineFixtureFromNativeRest6=1 baselineSlot=0 awakeMask=0 checkpoint=0 nativeObservation=1 once=1\n",player,vi,value.hp,value.count,plane.z);return false;
}
} // namespace workshop::boss_fish_cp6_fixture0284
