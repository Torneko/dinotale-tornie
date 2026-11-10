#pragma once
#include "boss_battle_runtime0250.hpp"
#include "boss_checkpoint_death0260.hpp"
#include "boss_checkpoint_pose0260.hpp"
// Included after boss_battle_runtime0250.hpp's campaign definitions. The
// numbered-character observer and value-only inventory helper remain separate.
namespace workshop::boss_campaign0250::checkpoints {
namespace actor=boss_checkpoint_actor0260;namespace inventory=boss_checkpoint_inventory0260;
namespace pose=boss_checkpoint_pose0260;
inline actor::Controller characters;
inline inventory::Restore inventory_restore;
inline std::array<inventory::Payload,4> saved_inventory{};
inline pose::Restore pose_restore;
inline std::array<pose::Payload,4> saved_pose{};
inline bool retries_allowed=false,death_pending=false,transport_pending=false,baseline_ready=false;
inline unsigned death_vi=0,death_room=177,death_hp=0,retry_stats_epoch=0;
inline bool retry_stats_reset_pending=false;
inline boss_carry0260::Payload retry_stats;
inline uint32_t placement_record=0,placement_table=0;
inline unsigned placement_room=177;
inline bool waiting_for_retry(){return death_pending||transport_pending;}
inline void allow_retries(bool allow){retries_allowed=enabled&&allow;}
inline unsigned retry_restored_epoch(){return inventory_restore.last_epoch();}
inline void configure(bool on,bool diagnostic){
 characters.configure(on);inventory_restore.reset();saved_inventory={};pose_restore.reset();saved_pose={};retries_allowed=on&&!diagnostic;
 death_pending=transport_pending=baseline_ready=false;death_vi=death_hp=retry_stats_epoch=0;death_room=177;
 retry_stats_reset_pending=false;retry_stats={};placement_record=placement_table=0;placement_room=177;
}
inline void enter_room(unsigned room){characters.enter_room(room);placement_record=placement_table=0;placement_room=room;}
inline void forget(uint32_t a){characters.forget(a);}
inline void reject(const char* why){fprintf(stderr,"[BossCheckpoint] retry-rejected reason=%s vi=%u room=%u pending=%u transported=%u\n",why,current_vi,physical_room,unsigned(death_pending),unsigned(transport_pending));death_pending=transport_pending=false;fail(why);}
inline std::span<const uint8_t> source_objects(){if(physical_room==83)return boss_rest_data0240::rest_objects;if(physical_room==162&&session.snapshot().index==0)return rooms::arena(162).objects;return {};}
inline bool native_table(uint8_t* ram,uint32_t table,std::span<const uint8_t> objects,bool player_moved=false,std::optional<boss_rest_floor0260::SourceProof> source={}){
 if(!ram||objects.empty()||objects.size()%12||objects.size()/12>149||!actor::pointer(table,objects.size()/12*16))return false;
 for(size_t p=0;p<objects.size();p+=12){const auto a=table+uint32_t(p/12)*16,packed=native::word(ram,a);
  if(packed!=0xFFFFFFFF&&packed!=u32(objects,p))return false;
  if(player_moved&&u16(objects,p)==0x4001)continue;
  const auto x=native::word(ram,a+4),y=native::word(ram,a+8);
  if(x==u32(objects,p+4)&&y==u32(objects,p+8))continue;
  // The raw record was checked above, including legitimate consumed FFFFFFFF.
  // This shared exception is typed by the immutable 4001 source row only.
  if(u16(objects,p)==0x4001&&source&&boss_rest_floor0260::checkpoint_player_record(ram,u32(objects,p),x,y,physical_room,session.snapshot().index,*source))continue;
  return false;}
 return true;
}
// The ROM record's unique index proves the expanded native table. No actor
// search or guessed S0 establishes placement ownership.
inline bool prove_record(uint8_t* ram,const recomp_context* c){
 if(!ram||!c||!enabled||uint32_t(c->r31)!=0x8005A010||uint32_t(c->r4)!=actor::source_packed)return false;
 const auto objects=source_objects();if(objects.empty()||objects.size()%12)return false;
 unsigned found=0,index=0;for(size_t p=0;p<objects.size();p+=12)if(u32(objects,p)==actor::source_packed){++found;index=unsigned(p/12);}
 const auto record=uint32_t(c->r16),offset=index*16;
 if(found!=1||record<offset||!actor::pointer(record,16)||native::word(ram,record)!=actor::source_packed)return false;
 const auto table=record-offset;const bool cached=placement_record==record&&placement_table==table&&placement_room==physical_room;
 if(!native_table(ram,table,objects,cached,rest_source(ram)))return false;
 if(placement_record&&(placement_record!=record||placement_table!=table||placement_room!=physical_room))return false;
 placement_record=record;placement_table=table;placement_room=physical_room;return true;
}
inline actor::Frame actor_frame(uint8_t* ram,const recomp_context* =nullptr){
 const auto& s=session.snapshot();actor::Frame f;
 f.active=enabled&&s.phase!=core::Phase::Complete&&(s.phase!=core::Phase::Failed||waiting_for_retry());
 f.first_fight=s.index==0&&physical_room==162&&s.completed<=1;f.fighting=s.phase==core::Phase::Fighting;
 f.rest_ready=s.phase==core::Phase::Rest;f.victory=s.phase==core::Phase::Victory;f.room=physical_room;f.completed=s.completed;
 f.awake_mask=s.checkpoint_awake_mask;f.vi=current_vi;f.overlay=overlays[actor::overlay_section];f.player=s.player;
 f.support_ready=rest_floor_guard.checkpoint_support_ready(ram,physical_room,rest_floor_encounter,overlays[boss_rest_floor0260::overlay_section]);
 // Full-table proof belongs to construction. Native5628 legitimately moves
 // the PLAYER spawn record through48C2C; rechecking every ROM XY after that
 // call would reject the real wake. The character helper keeps checking its
 // own exact record/packed/XY/resource/birth identity until teardown.
 if(ram&&placement_room==physical_room&&placement_record&&actor::pointer(placement_record,16))f.record=placement_record;
 return f;
}
inline actor::SpawnChoice select_spawn(uint8_t* ram,const recomp_context* c){
 if(!prove_record(ram,c))return {};return actor::select_spawn(ram,c,actor_frame(ram,c));
}
inline uint64_t init_enter(uint8_t* ram,const recomp_context* c){
 const auto ticket=characters.init_enter(ram,c,actor_frame(ram,c));
 if(characters.snapshot().invalidated)reject("checkpoint_character_constructor_rejected");return ticket;
}
inline bool init_leave(uint8_t* ram,const recomp_context* c,uint64_t ticket){
 if(!ticket)return false;const bool ok=characters.init_leave(ram,c,actor_frame(ram,c),ticket);
 if(!ok||characters.snapshot().invalidated)reject("checkpoint_character_initializer_rejected");return ok;
}
inline uint64_t wake_enter(uint8_t* ram,const recomp_context* c){
 if(fatal_player(ram,session.snapshot().player))return 0;
 const auto ticket=characters.wake_enter(ram,c,actor_frame(ram,c));
 if(characters.snapshot().invalidated)reject("checkpoint_character_wakeup_scope_rejected");return ticket;
}
inline bool wake_leave(uint8_t* ram,const recomp_context* c,uint64_t ticket){
 if(!ticket)return false;
 if(!characters.wake_leave(ram,c,actor_frame(ram,c),ticket)){reject("checkpoint_character_wakeup_rejected");return false;}
 const auto wake=characters.take_wakeup();inventory::Payload payload;pose::Payload plane;
 if(!wake||wake->vi!=current_vi||!inventory::capture(ram,session.snapshot().player,wake->vi,payload)){reject("checkpoint_wakeup_inventory_rejected");return false;}
 const auto& binding=characters.snapshot().binding;
 if(binding.actor!=wake->actor||binding.birth!=wake->birth||binding.room!=physical_room||
    !pose::capture(ram,payload,plane,binding.actor,baseline_ready?&saved_pose[0]:nullptr)){reject("checkpoint_wakeup_pose_rejected");return false;}
 if(!session.awaken_checkpoint(wake->completed)){reject("checkpoint_wakeup_phase_rejected");return false;}
 saved_inventory[boss_checkpoint0260::number(wake->completed)-1]=payload;
 saved_pose[boss_checkpoint0260::number(wake->completed)-1]=plane;
 fprintf(stderr,"[BossCheckpoint] awakened number=%u completed=%u mask=%u player=%08X hp=%u eggs=%u vi=%u\n",boss_checkpoint0260::number(wake->completed),wake->completed,session.snapshot().checkpoint_awake_mask,payload.player,payload.hp,payload.count,wake->vi);
 return true;
}
inline bool suppress_warp(uint8_t* ram,const recomp_context* c){return characters.suppress_warp(ram,c,actor_frame(ram,c));}
inline bool capture_baseline(uint8_t* ram,unsigned vi){
 const auto& s=session.snapshot();if(!retries_allowed||baseline_ready||s.index||s.completed||s.phase!=core::Phase::Fighting)return true;
 inventory::Payload payload;pose::Payload plane;
 if(!inventory::capture(ram,s.player,vi,payload)||!pose::capture(ram,payload,plane))return false;
 saved_inventory[0]=payload;saved_pose[0]=plane;baseline_ready=true;return true;
}
inline const inventory::Payload& selected_inventory(){return saved_inventory[boss_checkpoint0260::number(session.snapshot().checkpoint_completed)-1];}
inline const pose::Payload& selected_pose(){return saved_pose[boss_checkpoint0260::number(session.snapshot().checkpoint_completed)-1];}
// Native5628/48C2C modifies the cached Rest player source after a proved
// wake. Both a normal return and a retry reuse that room cache (6DA0C).
// Permission is for exactly that source coordinate, never a claim that the
// selected HP/FIFO has been restored: restore_arrival owns that readback.
inline std::optional<boss_rest_floor0260::SourceProof> rest_source(uint8_t* ram){
 const auto& s=session.snapshot();const unsigned cp=s.checkpoint_completed;
 const bool retry=s.phase==core::Phase::RetryToRest;
 const bool normal=s.phase==core::Phase::ToRest||s.phase==core::Phase::Rest;
 if(!ram||!enabled||(!retry&&!normal)||physical_room!=83||logical_room!=83||
    (cp!=3&&cp!=6&&cp!=9)||s.index>=10||s.completed>=10||s.index+1!=s.completed||
    cp>s.completed||s.max_completed<s.completed||!(s.checkpoint_awake_mask&boss_checkpoint0260::awake_bit(cp))||
    native::word(ram,route_address)!=0x44CA2000||native::byte(ram,route_address+4)!=83)return {};
 // These paired saved values are populated only by the successful native
 // character wake receipt; configure clears them for each new campaign.
 const auto& p=selected_inventory();
 if(!inventory::valid(p)||p.player!=s.player||!inventory::same_player(ram,p)||
    !pose::owner(selected_pose(),p)||current_vi<p.activation_vi)return {};
 if(retry){
  if(!retries_allowed||!baseline_ready||!transport_pending||death_pending||!s.retry_epoch||
     s.completed!=cp||pose_restore.last_epoch()!=s.retry_epoch||
     inventory_restore.last_epoch()>=s.retry_epoch||death_vi<=p.activation_vi||current_vi<death_vi)return {};
  return boss_rest_floor0260::SourceProof{p.player,cp,s.retry_epoch,s.completed,boss_rest_floor0260::SourceMode::Retry};
 }
 // Normal transport and an already arrived Rest visit need no retry epoch.
 // Native wake authorization persists even when retries are disabled.
 if(transport_pending||death_pending||fatal_player(ram,p.player))return {};
 return boss_rest_floor0260::SourceProof{p.player,cp,s.retry_epoch,s.completed,boss_rest_floor0260::SourceMode::Normal};
}
inline bool notice_death(uint8_t* ram,unsigned vi){
 const auto& s=session.snapshot();
 const bool playable=s.phase==core::Phase::Fighting||s.phase==core::Phase::Victory||s.phase==core::Phase::Rest;
 if(!playable&&!(death_pending&&s.phase==core::Phase::Failed&&s.failure=="yoshi_defeated"))return false;
 if(!retries_allowed||!baseline_ready||!vi||!fatal_player(ram,s.player)||!inventory::valid(selected_inventory())||!pose::owner(selected_pose(),selected_inventory())||
    !inventory::same_player(ram,selected_inventory())||vi<=selected_inventory().activation_vi)return false;
 if(!death_pending){death_vi=vi;death_room=physical_room;session.cancel("yoshi_defeated");death_pending=true;failed=false;
  fprintf(stderr,"[BossCheckpoint] native-death-pending completed=%u checkpoint=%u player=%08X vi=%u\n",s.completed,s.checkpoint_completed,s.player,vi);}
 return true;
}
inline bool death_call(uint8_t* ram,const recomp_context* c){
 if(!ram||!c)return false;const auto player=session.snapshot().player;
 if(!player||native::word(ram,0x800F8828)!=player||uint32_t(c->r4)!=player||!fatal_player(ram,player))return false;
 const auto ra=uint32_t(c->r31),priv=native::word(ram,player+0x1F4);
 if(!actor::pointer(priv,0x12D0))return false;
 // The ordinary/fall callers retain their original guards. Swallowed P1
 // uses2D3FC ->A/C/3085C ->42148 instead of30194.42148 keepsC0 until
 // AFTER its42174 jalr returns. Its saved stack proves the real route;
 // A1 may be clobbered by21A44 and there is no D8 admission in42148.
 if(ra==0x8004217C){const auto stack=uint32_t(c->r29);
  return native::byte(ram,player+0x30)==0xC&&native::word(ram,player+0x20)==0x8003085C&&native::word(ram,priv+0xC0)==0x8002F5F4&&actor::pointer(stack,0x28)&&native::word(ram,stack+0x14)==0x80030878&&native::word(ram,stack+0x1C)==0x8002F5F4&&native::word(ram,stack+0x20)==player&&native::word(ram,stack+0x24)==priv;
 }
 if(ra==0x80030780)return int32_t(native::word(ram,player+0x38))<-40;
 return ra==0x800301D4&&native::word(ram,priv+0xC0)==0x8002F5F4&&int32_t(native::word(ram,priv+0xD8))>=3;
}
inline void clear_attempt(){
 const auto checkpoint=session.snapshot().checkpoint_completed;
 for(unsigned i=checkpoint;i<history.size();++i)history[i]={};
 bound={};observers={};candies={};observer_epochs={};variant_epoch=0;variants::disable();placement_guard.reset();floor_guard.reset();
 scaled={};scale_scopes.clear();ready_info=exit_pot=0;arrival_vi=rest_vi=0;flags_ready=restore_pending=false;
 cargo={};stats_reset_pending=false;completion_pending=false;finished=false;failed=false;
 candy_food={};candy_cycle_live=candy_have_log_vi=false;candy_log_vi=candy_previous_buttons=0;
 rest_controller.configure(enabled&&driver,session.snapshot().diagnostic);candy_controller.configure(enabled&&driver,session.snapshot().diagnostic);
 // A same-room retry is still a new native visit. Only expired bindings/visit
 // location change; all normal/hybrid consumption and heart one-use stay live.
 boss_tree0240::clear_bindings();rest_heart.enter_room(177);
}
inline bool intercept_death(uint8_t* ram,recomp_context* c){
 if(!enabled||!retries_allowed||!death_call(ram,c))return false;
 const auto stack=uint32_t(c->r29);
 if(stack<0x80000800||!actor::pointer(stack-0x400,0x480)){reject("checkpoint_death_stack_rejected");return true;}
 if(!death_pending&&!notice_death(ram,current_vi))return false;
 if(!death_pending||transport_pending||physical_room!=death_room||!inventory::same_player(ram,selected_inventory())){reject("checkpoint_death_scope_rejected");return true;}
 const auto payload=selected_inventory();death_hp=native::word(ram,0x800F8DB4+payload.native_color*4);
 if(death_hp>8||!boss_carry0260::capture(ram,retry_stats)){reject("checkpoint_death_stats_capture_failed");return true;}
 if(!inventory::discard_after_death(ram,c,payload,death_vi,death_room,death_hp)){reject("checkpoint_death_inventory_discard_failed");return true;}
 if(!boss_checkpoint_death0260::cleanup(ram,c,payload.player,overlays[boss_checkpoint_death0260::overlay_section])){reject("checkpoint_death_cinematic_cleanup_failed");return true;}
 const auto prior_index=session.snapshot().index;
 if(!session.retry_checkpoint()){reject("checkpoint_retry_phase_rejected");return true;}
 if(!retry_placements.arm(session.snapshot().retry_epoch,session.snapshot().checkpoint_completed,prior_index)){reject("checkpoint_monster_retry_scope_rejected");return true;}
 if(!pose_restore.apply(ram,selected_pose(),payload,death_vi,session.snapshot().retry_epoch)){reject("checkpoint_retry_depth_restore_rejected");return true;}
 fprintf(stderr,"[BossCheckpoint] retry-depth-restored epoch=%u player=%08X z=%.5f vi=%u\n",session.snapshot().retry_epoch,payload.player,std::bit_cast<float>(selected_pose().z),current_vi);
 clear_attempt();death_pending=false;transport_pending=true;retry_stats_reset_pending=false;retry_stats_epoch=0;
 const unsigned destination=session.snapshot().checkpoint_completed?83:source_for(0);make_route(ram,destination);
 const auto saved=*c;c->r29=ADD32(saved.r29,-0xC0);c->r4=native::base(payload.player);c->r5=native::base(route_address);
 func_80030B7C(ram,c);*c=saved;
 if(!inventory::same_player(ram,payload)||fatal_player(ram,payload.player)){reject("checkpoint_native_transport_rejected");return true;}
 fprintf(stderr,"[BossCheckpoint] retry-departure checkpoint=%u epoch=%u destination=%u player=%08X hp=%u eggs=%u vi=%u\n",session.snapshot().checkpoint_completed,session.snapshot().retry_epoch,destination,payload.player,payload.hp,payload.count,current_vi);
 return true;
}
inline bool retry_stats_match(uint8_t* ram){
 boss_carry0260::Payload actual;if(retry_stats_reset_pending||!boss_carry0260::capture(ram,actual)||actual.count!=retry_stats.count||actual.scores[1]!=retry_stats.scores[1]||actual.scores[2]!=retry_stats.scores[2])return false;
 return std::equal(retry_stats.fruits.begin(),retry_stats.fruits.begin()+retry_stats.count,actual.fruits.begin())&&
  std::equal(retry_stats.half_fruits.begin(),retry_stats.half_fruits.begin()+retry_stats.count,actual.half_fruits.begin());
}
inline void trace_carry(uint8_t* ram,const recomp_context* c,std::string_view name){
 const auto& s=session.snapshot();if(!transport_pending||!c||(s.phase!=core::Phase::RetryToRest&&s.phase!=core::Phase::RetryToFight))return;
 const auto destination=s.checkpoint_completed?83:source_for(0);
 const bool owned=physical_room==destination&&native::word(ram,route_address)==0x44CA2000&&native::byte(ram,route_address+4)==destination&&inventory::same_player(ram,selected_inventory())&&!fatal_player(ram,s.player);
 if(boss_carry0260::reset_fence(name,uint32_t(c->r31))){
  if(!owned||retry_stats_reset_pending||retry_stats_epoch==s.retry_epoch||!boss_carry0260::valid(retry_stats)){reject("checkpoint_stats_reset_unproved");return;}
  retry_stats_reset_pending=true;
 }else if(retry_stats_reset_pending&&boss_carry0260::hud_fence(name,uint32_t(c->r31))){
  if(!owned||!boss_carry0260::restore(ram,retry_stats)){reject("checkpoint_stats_restore_rejected");return;}
  retry_stats_reset_pending=false;retry_stats_epoch=s.retry_epoch;
 }
}
inline bool restore_arrival(uint8_t* ram,recomp_context* c,unsigned vi){
 const auto& s=session.snapshot();const auto& p=selected_inventory();
 if(!transport_pending)return false;
 if(!retry_stats_match(ram)||pose_restore.last_epoch()!=s.retry_epoch||!pose::matches(ram,selected_pose(),p)){transport_pending=false;return false;}
 inventory::RetryProof proof{p.player,p.activation_vi,death_vi,vi,s.retry_epoch,physical_room,s.checkpoint_completed,death_hp};
 if(!inventory_restore.apply(ram,c,p,proof)){transport_pending=false;return false;}
 transport_pending=false;
 fprintf(stderr,"[BossCheckpoint] retry-restored checkpoint=%u epoch=%u room=%u player=%08X hp=%u eggs=%u vi=%u\n",s.checkpoint_completed,s.retry_epoch,physical_room,s.player,p.hp,p.count,vi);
 return true;
}
} // namespace workshop::boss_campaign0250::checkpoints
