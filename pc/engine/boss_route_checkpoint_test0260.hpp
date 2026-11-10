#pragma once
#include "boss_battle0250.hpp"
#include "boss_checkpoint_actor0260.hpp"
#include "boss_checkpoint_inventory0260.hpp"
#include "boss_tree0240.hpp"
#include "boss_tree_hybrid0260.hpp"
#include "boss_rest_heart0260.hpp"
#include "boss_lips_body0260.hpp"
#include "boss_lips_jump0260.hpp"
#include "boss_monster_aim0260.hpp"
#include <bit>
#include <cmath>
#include <cstdio>
#include <span>

namespace workshop::boss_route_checkpoint_test0260 {
namespace core=boss_battle0250;namespace n=boss_native;
namespace actor=boss_checkpoint_actor0260;namespace inventory=boss_checkpoint_inventory0260;
enum class Phase:uint8_t { Disabled,WaitPrefix,PrepareRest,Wake,AwaitFight,ShotHold,ShotRelease,SeekDeath,AwaitRetry,Done,Failed };
struct Ledger {
 bool valid=false;unsigned treeConsumed=0,treeRegens=0,treeMask=0,hybridConsumed=0,hybridMask=0;
 std::array<uint16_t,6> hybridOrder{};bool heartConsumed=false;unsigned heartCount=0;
 bool operator==(const Ledger&)const=default;
};
struct Target {uint32_t actor=0;uint64_t birth=0;uint32_t candy_root=0,candy_private=0,private_data=0,player=0;};
struct Frame {
 bool diagnostic=false,driver=false,controls=false;
 bool egg_admission_observed=false,egg_admitted=false,pad_observed=false,z_released=false;
 unsigned room=177,logical_room=177,vi=0;uint64_t seed=0;
 const core::Snapshot* session=nullptr;
 const actor::Snapshot* characters=nullptr;
 const inventory::Payload* saved=nullptr;
 unsigned retry_restored_epoch=0;Ledger ledger{};std::span<const Target> targets{};
};
struct Output {bool handled=false;uint16_t buttons=0;float x=0,y=0;bool intro_ack_only=false;};
struct SeekContactState {uint32_t actor=0,root=0;uint64_t birth=0;unsigned jump_vi=0,release_vi=0;bool released=false,cancelled=false;};
struct Snapshot {
 bool enabled=false;Phase phase=Phase::Disabled;unsigned checkpoint=3,completed_tests=0,last_vi=0,stage_vi=0;
 uint32_t player=0,point=0;uint64_t point_birth=0;unsigned native_color=8,pc_color=14,activation_vi=0;
 unsigned activation_epoch=0,death_vi=0,restored_epoch=0;bool fifo_changed=false;
 const char* failure="";
};
inline Ledger observe_ledger(uint8_t* ram,const boss_tree0240::Snapshot& tree,const boss_tree_hybrid0260::Snapshot& hybrid,const boss_rest_heart0260::Snapshot& heart);
class Controller {
 Snapshot state_{};std::string token_;uint64_t seed_=0;mutable SeekContactState seek_contact_{};
 inventory::Payload activation_{};Ledger activation_ledger_{},death_ledger_{};uint32_t plane_=0;
 unsigned before_mask_=0,jump_vi_=0,shot_vi_=0,maximum_=0;
 unsigned admission_vi_=0,neutral_z_vi_=0,shot_pause_vi_=0;bool shot_paused_=false;bool crossing_=false,aim_seen_=false,second_press_=false;
 struct PointPlacement {actor::Binding attested{};unsigned epoch=0,stage_vi=0;bool retired=false;};
 std::optional<PointPlacement> point_;
 unsigned wake_release_vi_=0,wake_jump_vi_=0,wake_attempts_=0,wake_output_vi_=0;
 uint64_t wake_jump_birth_=0;bool wake_released_=false,wake_output_valid_=false,wake_hold_cancelled_=false;Output wake_output_{};
 void fail(const char* reason);
 bool scope(uint8_t* ram,const Frame& f);
 bool current_inventory(uint8_t* ram,const Frame& f,inventory::Payload& out,bool fatal=false)const;
 bool point_identity(uint8_t* ram,const Frame& f)const;
 bool remember_point(uint8_t* ram,const Frame& f);
 bool rearmed_point(uint8_t* ram,const Frame& f)const;
 Output return_to_point(uint8_t* ram,const Frame& f)const;
 void reset_contact();
 Output contact_point(uint8_t* ram,const Frame& f);
 void emit(const char* event,uint8_t* ram,const Frame& f,const inventory::Payload& value,uint32_t death_ra=0)const;
 Output approach(uint8_t* ram,const Frame& f,bool point)const;
 bool activation(uint8_t* ram,const Frame& f);
public:
 void configure(bool enabled=false);
 const Snapshot& snapshot()const{return state_;}
 bool complete()const{return state_.enabled&&state_.phase==Phase::Done;}
 bool failed()const{return state_.enabled&&state_.phase==Phase::Failed;}
 Output frame(uint8_t* ram,const Frame& f);
 void native_death(uint8_t* ram,const recomp_context* context,const Frame& f);
};
} // namespace workshop::boss_route_checkpoint_test0260

namespace workshop::boss_route_checkpoint_test0260 {
inline Ledger observe_ledger(uint8_t* ram,const boss_tree0240::Snapshot& tree,const boss_tree_hybrid0260::Snapshot& hybrid,const boss_rest_heart0260::Snapshot& heart){
 Ledger out;out.treeConsumed=tree.totalConsumed;out.treeRegens=tree.regenerations;out.treeMask=tree.consumedmask;
 out.hybridConsumed=hybrid.consumed;out.hybridMask=hybrid.consumed_mask;out.hybridOrder=hybrid.order;
 out.heartConsumed=heart.consumed;if(ram)out.heartCount=n::word(ram,boss_rest_heart0260::heart_count_address);
 unsigned all=0,prefix=0;bool order=true;
 for(unsigned i=0;i<6;++i){const auto kind=out.hybridOrder[i];if(kind<0x4F00||kind>0x4F05){order=false;continue;}const unsigned bit=1u<<(kind-0x4F00);if(all&bit)order=false;all|=bit;if(i<out.hybridConsumed)prefix|=bit;}
 out.valid=ram&&tree.enabled&&!tree.invalidated&&tree.regenerations<=2&&tree.totalConsumed<=15&&!(tree.consumedmask&~31u)&&tree.totalConsumed==tree.regenerations*5+unsigned(std::popcount(tree.consumedmask))&&
  hybrid.enabled&&!hybrid.invalidated&&hybrid.consumed<=6&&!(hybrid.consumed_mask&~63u)&&order&&all==63&&prefix==hybrid.consumed_mask&&
  heart.enabled&&!heart.invalidated&&out.heartCount<=1&&(!out.heartConsumed||out.heartCount==1);
 return out;
}
inline void Controller::configure(bool enabled){
 state_={};seek_contact_={};state_.enabled=enabled;state_.phase=enabled?Phase::WaitPrefix:Phase::Disabled;
 token_.clear();seed_=0;activation_={};activation_ledger_=death_ledger_={};plane_=0;before_mask_=jump_vi_=shot_vi_=maximum_=0;crossing_=aim_seen_=second_press_=false;admission_vi_=neutral_z_vi_=shot_pause_vi_=0;shot_paused_=false;point_.reset();reset_contact();
}
inline void Controller::fail(const char* why){
 if(state_.phase==Phase::Failed)return;state_.phase=Phase::Failed;state_.failure=why;
 std::fprintf(stderr,"[BossRouteCheckpoint] failed checkpoint=%u tests=%u epoch=%u player=%08X vi=%u reason=%s controlsOnly=1\n",state_.checkpoint,state_.completed_tests,state_.restored_epoch,state_.player,state_.last_vi,why);
}
inline bool Controller::scope(uint8_t* ram,const Frame& f){
 if(!ram||!f.session||!f.diagnostic||!f.driver||!f.session->diagnostic||f.seed!=1||f.session->token.empty()){fail("invalid_diagnostic_route_scope");return false;}
 const auto& s=*f.session;
 if(f.vi<state_.last_vi){fail("vi_went_backwards");return false;}state_.last_vi=f.vi;
 if(!state_.player){
  if(!s.player)return true;
  if(s.index||s.completed||s.phase!=core::Phase::Fighting||!n::actor(ram,s.player,0x4001)||n::word(ram,0x800F8828)!=s.player){fail("route_did_not_start_at_first_fight");return false;}
  state_.player=s.player;state_.pc_color=n::word(ram,0x807F0008);state_.native_color=n::word(ram,0x800F8DB0);token_=s.token;seed_=f.seed;
  if(state_.pc_color>=14||state_.native_color!=(state_.pc_color<8?state_.pc_color:0)){fail("initial_yoshi_colour_invalid");return false;}
 }
 if(state_.player&&(s.player!=state_.player||s.token!=token_||f.seed!=seed_)){fail("run_or_yoshi_identity_changed");return false;}
 if(s.phase==core::Phase::Failed&&s.failure!="yoshi_defeated"){fail("campaign_failed_before_checkpoint");return false;}
 return true;
}
inline bool Controller::current_inventory(uint8_t* ram,const Frame& f,inventory::Payload& out,bool fatal)const{
 if(!state_.player||!f.vi)return false;
 if(!fatal)return inventory::capture(ram,state_.player,f.vi,out);
 inventory::Payload value;value.player=state_.player;value.activation_vi=f.vi;value.pc_color=state_.pc_color;value.native_color=state_.native_color;
 if(!inventory::same_player(ram,value))return false;value.hp=n::word(ram,0x800F8DB4+value.native_color*4);
 inventory::Inventory stock;if(!inventory::read(ram,value.player,stock))return false;value.count=stock.count;value.eggs=stock.eggs;
 if(!inventory::valid(value))return false;out=value;return true;
}
inline bool Controller::point_identity(uint8_t* ram,const Frame& f)const{
 if(!f.characters||!f.characters->enabled||f.characters->invalidated||f.characters->room!=83)return false;
 const auto& b=f.characters->binding;const unsigned index=state_.checkpoint/3;
 return b.birth&&b.room==83&&b.milestone==state_.checkpoint&&b.index==index&&n::actor(ram,b.actor,uint16_t(0x4507+index))&&
  actor::pointer(b.record,12)&&actor::pointer(b.resource,0x48)&&actor::pointer(b.private_data,0x280)&&actor::pointer(b.overlay,actor::overlay_extent)&&
  n::word(ram,b.actor+0x1D8)==b.record&&n::word(ram,b.actor+0x1DC)==b.packed&&n::word(ram,b.record)==0xFFFFFFFF&&n::word(ram,b.record+4)==b.x&&n::word(ram,b.record+8)==b.y&&
  n::word(ram,b.actor+0x168)==b.resource&&n::word(ram,b.resource+0x40)==b.overlay&&n::word(ram,b.resource+0x44)==b.overlay+actor::initializer_offset&&
  n::word(ram,b.actor+0x1F4)==b.private_data&&n::word(ram,b.private_data+0x27C)==index;
}
inline bool Controller::remember_point(uint8_t* ram,const Frame& f){
 if(!point_identity(ram,f))return false;
 const auto& b=f.characters->binding;const auto& s=*f.session;
 const float x=std::bit_cast<float>(b.x),y=std::bit_cast<float>(b.y);
 if(b.packed!=actor::source_packed||f.characters->next_birth<b.birth||!std::isfinite(x)||!std::isfinite(y)||std::abs(x)>100000||std::abs(y)>100000){fail("checkpoint_source_placement_invalid");return false;}
 if(point_){const auto& old=point_->attested;
  if(point_->epoch!=s.retry_epoch||point_->stage_vi!=state_.stage_vi||old.room!=b.room||old.milestone!=b.milestone||old.index!=b.index||old.record!=b.record||old.packed!=b.packed||old.x!=b.x||old.y!=b.y||b.birth<old.birth||(point_->retired&&b.birth==old.birth)||
     (b.birth==old.birth&&(old.actor!=b.actor||old.private_data!=b.private_data||old.resource!=b.resource||old.metadata!=b.metadata||old.definition!=b.definition||old.overlay!=b.overlay))){fail("checkpoint_source_placement_changed");return false;}
  if(b.birth!=old.birth){wake_released_=false;wake_output_valid_=false;wake_hold_cancelled_=true;}
 }
 point_=PointPlacement{b,s.retry_epoch,state_.stage_vi};return true;
}
inline bool Controller::rearmed_point(uint8_t* ram,const Frame& f)const{
 if(!point_||!f.session||!f.characters||!f.characters->enabled||f.characters->invalidated||f.characters->room!=83)return false;
 const auto& s=*f.session;const auto& old=point_->attested;const auto& current=f.characters->binding;
 // Native teardown legitimately clears the binding and re-arms this exact
 // placement. Its old actor/private pointer is never a live target. The cache
 // belongs to the player/token/seed checked by scope(), this Rest and this epoch.
 return f.room==83&&s.phase==core::Phase::Rest&&s.completed==state_.checkpoint&&old.room==83&&old.milestone==state_.checkpoint&&old.index==state_.checkpoint/3&&
  point_->epoch==s.retry_epoch&&point_->stage_vi==state_.stage_vi&&old.birth&&f.characters->next_birth>=old.birth&&
  !current.actor&&!current.private_data&&!current.record&&!current.birth&&actor::pointer(old.record,12)&&old.packed==actor::source_packed&&
  n::word(ram,old.record)==old.packed&&n::word(ram,old.record+4)==old.x&&n::word(ram,old.record+8)==old.y;
}
inline Output Controller::return_to_point(uint8_t* ram,const Frame& f)const{
 Output out{true};if(!f.controls||!rearmed_point(ram,f)||inventory::fatal_death_state(ram,state_.player))return out;
 const float x=std::bit_cast<float>(n::word(ram,state_.player+0x84)),target=std::bit_cast<float>(point_->attested.x);
 if(std::isfinite(x)&&std::isfinite(target)&&std::abs(target-x)>1)out.x=target>x?.65f:-.65f;
 return out;
}
inline void Controller::reset_contact(){wake_release_vi_=wake_jump_vi_=wake_attempts_=wake_output_vi_=0;wake_jump_birth_=0;wake_released_=wake_output_valid_=wake_hold_cancelled_=false;wake_output_={};}
inline Output Controller::contact_point(uint8_t* ram,const Frame& f){
 // 58AC wakes from its native4001 rider list; plain5758 contact ignores
 // primary1 walking. Produce an ordinary bounded A jump, never a rider or wake
 // call. The18VI hold already produced a48px native rise in this same Rest.
 auto out=approach(ram,f,true);
 const auto finish=[&](){wake_output_vi_=f.vi;wake_output_valid_=true;wake_output_=out;return out;};
 const auto player=state_.player,priv=n::word(ram,player+0x1F4),point=f.characters->binding.actor;
 const float x=std::bit_cast<float>(n::word(ram,player+0x84)),y=std::bit_cast<float>(n::word(ram,player+0x88));
 const float px=std::bit_cast<float>(n::word(ram,point+0x84)),py=std::bit_cast<float>(n::word(ram,point+0x88));
 const bool finite=std::isfinite(x)&&std::isfinite(y)&&std::isfinite(px)&&std::isfinite(py);
 const bool posture=inventory::pointer(priv,0x12F0)&&n::byte(ram,player+0x18)==1&&n::byte(ram,player+0x17D)==2&&!n::byte(ram,player+0x165)&&
  !n::word(ram,player+0x180)&&!n::word(ram,priv+0x12EC);
 if(wake_output_valid_&&wake_output_vi_==f.vi){
  // Idempotence applies to unchanged native observations. A rider/transport
  // change after input in this same VI must cancel the cached A immediately.
  if((wake_output_.buttons&0x8000)&&(!finite||!posture||wake_jump_birth_!=f.characters->binding.birth)){wake_hold_cancelled_=true;wake_output_={true};}
  return wake_output_;
 }
 if(wake_jump_vi_&&f.vi-wake_jump_vi_<18){
  if(!finite||!posture||wake_jump_birth_!=f.characters->binding.birth)wake_hold_cancelled_=true;
  if(!wake_hold_cancelled_)out.buttons=0x8000;
  return finish();
 }
 if(!finite||std::abs(x-px)>8||std::abs(y-py)>48){wake_released_=false;return finish();}
 // F4 is the source player's native airborne flag; positive residual C8 at a
 // resolved floor is normal. Grounding is required for a fresh jump, while
 // its bounded hold may continue during the resulting native airborne state.
 if(!posture||n::byte(ram,priv+0xF4)){wake_released_=false;return finish();}
 if(wake_jump_vi_&&f.vi-wake_jump_vi_<90)return finish();
 if(!wake_released_){wake_released_=true;wake_release_vi_=f.vi;return finish();}
 if(f.vi<=wake_release_vi_)return finish();
 if(wake_attempts_>=6){fail("native_checkpoint_rider_contact_not_observed");out={true};return finish();}
 ++wake_attempts_;wake_jump_vi_=f.vi;wake_jump_birth_=f.characters->binding.birth;wake_released_=false;wake_hold_cancelled_=false;out.buttons=0x8000;
 return finish();
}
inline void Controller::emit(const char* event,uint8_t* ram,const Frame& f,const inventory::Payload& value,uint32_t death_ra)const{
 const auto& s=*f.session;const auto& l=f.ledger;
 std::fprintf(stderr,"[BossRouteCheckpoint] %s checkpoint=%u number=%u epoch=%u vi=%u player=%08X nativeColor=%u pcColor=%u activationVi=%u hp=%u count=%u fifo=",event,state_.checkpoint,state_.checkpoint/3+1,s.retry_epoch,f.vi,state_.player,state_.native_color,state_.pc_color,state_.activation_vi,value.hp,value.count);
 if(!value.count)std::fprintf(stderr,"-");
 for(unsigned i=0;i<value.count;++i)std::fprintf(stderr,"%s%04X:%u",i?",":"",unsigned(value.eggs[i].kind),value.eggs[i].palette);
 std::fprintf(stderr," fifoScales=");if(!value.count)std::fprintf(stderr,"-");
 for(unsigned i=0;i<value.count;++i){const auto& scale=value.eggs[i].saved_scale;std::fprintf(stderr,"%s%08X:%08X:%08X",i?",":"",scale[0],scale[1],scale[2]);}
 std::fprintf(stderr," completed=%u maxCompleted=%u mask=%u seed=%llu point=%08X kind=%04X birth=%llu treeConsumed=%u treeRegens=%u treeMask=%u hybridConsumed=%u hybridMask=%u hybridOrder=",s.completed,s.max_completed,s.checkpoint_awake_mask,static_cast<unsigned long long>(seed_),state_.point,0x4507+state_.checkpoint/3,static_cast<unsigned long long>(state_.point_birth),l.treeConsumed,l.treeRegens,l.treeMask,l.hybridConsumed,l.hybridMask);
 for(unsigned i=0;i<6;++i)std::fprintf(stderr,"%s%04X",i?",":"",unsigned(l.hybridOrder[i]));
 std::fprintf(stderr," heartConsumed=%u heartCount=%u primary=%u sub=%u deathCallback=%08X deathRA=%08X fifoChanged=%u controlsOnly=1\n",unsigned(l.heartConsumed),l.heartCount,n::byte(ram,state_.player+0x18),n::byte(ram,state_.player+0x30),death_ra?0x8002F5F4u:0u,death_ra,unsigned(state_.fifo_changed));
}
// Ordinary Candy's real head is a sparse native child (queue0), not an
// independent queued enemy. Admit only its exact live root/private/slot chain.
inline bool candy_contact(uint8_t* ram,const Target& t,uint32_t player){
 if(!ram||!t.birth||t.player!=player||!n::actor(ram,player,0x4001)||n::word(ram,0x800F8828)!=player||!n::actor(ram,t.candy_root,0x455A)||n::byte(ram,t.candy_root+0x17D)!=2||n::byte(ram,t.candy_root+0x18)!=1||!n::range(t.candy_private,0x24)||n::word(ram,t.candy_root+0x1F4)!=t.candy_private||n::word(ram,t.candy_private+0x20)!=0||!n::actor(ram,t.actor,0x454F)||n::byte(ram,t.actor+0x18)!=1)return false;
 const auto phase=n::byte(ram,t.actor+0x17D);if((phase!=0&&phase!=2)||n::word(ram,t.candy_private)!=t.actor||n::word(ram,t.actor+0x180)!=t.candy_root||!n::range(t.private_data,0x88)||n::word(ram,t.actor+0x1F4)!=t.private_data)return false;
 const auto slot=inventory::half(ram,t.actor+0x17E),count=inventory::half(ram,t.candy_root+0x184);
 return slot<20&&count>0&&count<=20&&n::word(ram,t.candy_root+0x188+4*slot)==t.actor;
}
inline std::optional<boss_lips_jump0260::Clearance> candy_jump_launch(uint8_t* ram,const Target& t,uint32_t player){
 if(!candy_contact(ram,t,player))return {};const auto p=n::word(ram,player+0x1F4);if(!n::range(p,0xF5)||n::byte(ram,p+0xF4)!=0)return {};
 const auto pm=n::word(ram,player+0x174),tm=n::word(ram,t.actor+0x174);if(!n::range(pm,4)||!n::range(tm,4))return {};
 const auto a=boss_lips_body0260::read_box(ram,player,n::word(ram,pm)),b=boss_lips_body0260::read_box(ram,t.actor,n::word(ram,tm));const auto map=boss_monster_aim0260::Map::read(ram);if(!a||!b||!map)return {};
 const float gap=(a->y-a->hy)-(b->y+b->hy);if(gap<=2||gap>96||std::abs(a->x-b->x)>128||std::abs(a->z-b->z)>a->hz+b->hz)return {};
 // Use the actual native upward terrain probes (definition30/34), not
 // the wider object collider: an adjacent wall is not a ceiling.
 return boss_lips_jump0260::observe(ram,player,0,float(map->columns*256),std::bit_cast<float>(n::word(ram,player+0x88))-gap-4);
}
inline bool candy_jump_clear(uint8_t* ram,const Target& t,uint32_t player){const auto launch=candy_jump_launch(ram,t,player);return launch&&launch->current_clear;}
inline Output seek_death_input(uint8_t* ram,const Frame& f,uint32_t player,unsigned checkpoint,SeekContactState* jump=nullptr){
 Output out{true};if(!f.controls||inventory::fatal_death_state(ram,player)){if(jump){jump->cancelled=true;jump->released=false;}return out;}
 const float x=std::bit_cast<float>(n::word(ram,player+0x84));if(!std::isfinite(x))return out;
 float target=x,best=1e30f;const Target* nearest=nullptr;
 for(const auto& t:f.targets){if(!t.birth||!n::range(t.actor,0x1FC)||n::byte(ram,t.actor+0x18)==0xFF)continue;
  const auto kind=inventory::half(ram,t.actor+12);if(!n::actor(ram,t.actor,uint16_t(kind))||(checkpoint<9&&kind!=0x41C6)||(checkpoint==9&&kind!=0x455A&&kind!=0x454F))continue;
  if(kind==0x454F){if(checkpoint!=9||!candy_contact(ram,t,player))continue;}
  else if(t.candy_root||n::byte(ram,t.actor+0x17D)!=2)continue;
  const float tx=std::bit_cast<float>(n::word(ram,t.actor+0x84));if(!std::isfinite(tx)||std::abs(tx-x)>=best)continue;target=tx;best=std::abs(tx-x);nearest=&t;
 }
 if(std::isfinite(target)&&std::abs(target-x)>1)out.x=target>x?.65f:-.65f;
 if(!jump)return out;
 if(!nearest||!nearest->candy_root||checkpoint!=9){*jump={};return out;}
 if(jump->actor!=nearest->actor||jump->root!=nearest->candy_root||jump->birth!=nearest->birth){*jump={};jump->actor=nearest->actor;jump->root=nearest->candy_root;jump->birth=nearest->birth;}
 if(jump->jump_vi&&f.vi-jump->jump_vi<18){if(!jump->cancelled)out.buttons=0x8000;return out;}
 if(jump->jump_vi&&f.vi-jump->jump_vi<90)return out;
 const auto launch=candy_jump_launch(ram,*nearest,player);if(!launch){jump->released=false;return out;}
 if(!launch->current_clear){jump->released=false;out.x=std::abs(launch->x-x)>1?(launch->x>x?.65f:-.65f):0;return out;}
 const auto pad=n::word(ram,0x800F7EA8);const bool released=!(pad&3)&&n::range(pad,16)&&!(inventory::half(ram,pad)&0x8000)&&!(inventory::half(ram,pad+0xC)&0x8000);
 if(!released){jump->released=false;return out;}
 if(!jump->released){jump->released=true;jump->release_vi=f.vi;return out;}
 if(f.vi<=jump->release_vi)return out;
 jump->jump_vi=f.vi;jump->released=jump->cancelled=false;out.buttons=0x8000;return out;
}
inline Output Controller::approach(uint8_t* ram,const Frame& f,bool point)const{
 if(!point)return seek_death_input(ram,f,state_.player,state_.checkpoint,&seek_contact_);
 Output out{true};if(!f.controls||inventory::fatal_death_state(ram,state_.player))return out;
 const float x=std::bit_cast<float>(n::word(ram,state_.player+0x84));if(!std::isfinite(x))return out;
 const float target=f.characters?std::bit_cast<float>(n::word(ram,f.characters->binding.actor+0x84)):x;
 if(std::isfinite(target)&&std::abs(target-x)>1)out.x=target>x?.65f:-.65f;return out;
}
inline bool Controller::activation(uint8_t* ram,const Frame& f){
 const auto& s=*f.session;const unsigned bit=boss_checkpoint0260::awake_bit(state_.checkpoint);
 if(!(s.checkpoint_awake_mask&bit))return false;
 if(!point_||!remember_point(ram,f)){fail("native_wakeup_placement_not_attested");return false;}
 if(!point_identity(ram,f)||!f.saved||!inventory::valid(*f.saved)||f.saved->player!=state_.player||f.saved->native_color!=state_.native_color||f.saved->pc_color!=state_.pc_color||f.saved->activation_vi<state_.stage_vi||f.saved->activation_vi>f.vi||s.phase!=core::Phase::Rest||s.completed!=state_.checkpoint||s.checkpoint_completed!=state_.checkpoint||s.checkpoint_awake_mask!=(before_mask_|bit)||s.retry_epoch!=state_.completed_tests||!f.ledger.valid){fail("native_wakeup_receipt_mismatch");return false;}
 if(f.saved->hp<7||f.saved->count!=6){fail("checkpoint_woke_before_rest_refill");return false;}
 if(!inventory::matches(ram,*f.saved)||n::word(ram,actor::count_address)!=unsigned(std::popcount(s.checkpoint_awake_mask))){fail("native_wakeup_inventory_or_flags_mismatch");return false;}
 for(unsigned i=0;i<4;++i)if(n::word(ram,actor::flags_address+i*4)!=((s.checkpoint_awake_mask>>i)&1)){fail("native_wakeup_mask_mismatch");return false;}
 const auto z=n::word(ram,state_.player+0x8C);if(!std::isfinite(std::bit_cast<float>(z))){fail("activation_depth_invalid");return false;}
 activation_=*f.saved;activation_ledger_=f.ledger;maximum_=s.max_completed;plane_=z;
 state_.activation_vi=activation_.activation_vi;state_.activation_epoch=s.retry_epoch;state_.point=f.characters->binding.actor;state_.point_birth=f.characters->binding.birth;state_.fifo_changed=false;
 emit("activated",ram,f,activation_);state_.phase=Phase::AwaitFight;state_.stage_vi=f.vi;return true;
}
inline Output Controller::frame(uint8_t* ram,const Frame& f){
 if(!state_.enabled||state_.phase==Phase::Disabled||state_.phase==Phase::Done)return {};
 if(state_.phase==Phase::Failed)return {true};
 if(!scope(ram,f))return {true};if(!state_.player)return {};
 const auto& s=*f.session;
 if(state_.phase==Phase::WaitPrefix){
  if(s.retry_epoch!=state_.completed_tests){fail("unexpected_retry_before_checkpoint");return {true};}
  if(s.completed>state_.checkpoint){fail("checkpoint_rest_was_skipped");return {true};}
  if(s.phase!=core::Phase::Rest||s.completed!=state_.checkpoint)return {};
  state_.phase=Phase::PrepareRest;state_.stage_vi=f.vi;before_mask_=s.checkpoint_awake_mask;crossing_=false;point_.reset();reset_contact();
 }
 if(state_.phase==Phase::PrepareRest||state_.phase==Phase::Wake){
  if(f.room!=83||s.phase!=core::Phase::Rest||s.completed!=state_.checkpoint){fail("left_rest_without_checkpoint_wakeup");return {true};}
  if((s.checkpoint_awake_mask&boss_checkpoint0260::awake_bit(state_.checkpoint))!=0){activation(ram,f);return state_.phase==Phase::AwaitFight?Output{}:Output{true};}
  if(f.vi-state_.stage_vi>7200){fail("rest_refill_or_wakeup_deadline");return {true};}
  if(!f.controls){wake_released_=false;wake_output_valid_=false;wake_hold_cancelled_=true;return {true};}
  const bool live_point=point_identity(ram,f);
  if(live_point){if(!remember_point(ram,f))return {true};}
  else {wake_released_=false;wake_output_valid_=false;wake_hold_cancelled_=true;if(!rearmed_point(ram,f))return {true};point_->retired=true;}
  inventory::Payload live;if(!current_inventory(ram,f,live)){wake_released_=false;wake_output_valid_=false;wake_hold_cancelled_=true;return {true};}
  if(state_.phase==Phase::Wake&&(live.hp<7||live.count!=6)){state_.phase=Phase::PrepareRest;reset_contact();}
  if(state_.phase==Phase::PrepareRest){
   if(live.hp>=7&&live.count==6){state_.phase=Phase::Wake;crossing_=false;reset_contact();}
   else {
    // A sleeping Miss Warp is normally culled while the rest pilot visits the
    // distant tree/block. Keep ordinary refill running only after its exact
    // attested source record has been re-armed. Do not fake a living binding.
    if(!live_point){crossing_=false;return {};}
    const float x=std::bit_cast<float>(n::word(ram,state_.player+0x84)),y=std::bit_cast<float>(n::word(ram,state_.player+0x88));
    const float px=std::bit_cast<float>(n::word(ram,f.characters->binding.actor+0x84)),py=std::bit_cast<float>(n::word(ram,f.characters->binding.actor+0x88));
    if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(px)||!std::isfinite(py)){fail("rest_position_invalid");return {true};}
    // Cross the sleeping point through ordinary jump/walk input before the
    // existing rest pilot visits the tree/block. Never move the actor in RAM.
    if(x<px+58&&x>px-80&&y>py-110&&y<py+40){
     if(!crossing_){crossing_=true;jump_vi_=f.vi;}if(f.vi-jump_vi_>240){fail("could_not_jump_past_sleeping_checkpoint");return {true};}
     return {true,uint16_t((f.vi-jump_vi_)%60<18?0x8000:0),.65f,0};
    }
    crossing_=false;return {};
   }
  }
  return live_point?contact_point(ram,f):return_to_point(ram,f);
 }
 if(state_.phase==Phase::AwaitFight){
  if(s.retry_epoch!=state_.activation_epoch||s.completed!=state_.checkpoint){fail("progress_changed_before_checkpoint_death");return {true};}
  if(s.phase!=core::Phase::Fighting)return {};
  if(s.index!=state_.checkpoint||f.logical_room!=(state_.checkpoint==9?162u:151u)){fail("wrong_post_checkpoint_fight");return {true};}
  if(!admission_vi_)admission_vi_=f.vi;
  if(f.vi-admission_vi_>3600){fail("native_egg_admission_deadline");return {true};}
  // Admission is action-specific, not the broad scene/control witness. Keep
  // the scripted A acknowledgements, never delegate Candy B/combat here.
  if(!f.controls||!f.egg_admission_observed||!f.egg_admitted){neutral_z_vi_=0;return {true,0,0,0,true};}
  // A neutral output alone is not an input receipt: require a later VI and
  // the actual native pad showing both held/pressed Z/C edges released.
  if(!neutral_z_vi_){neutral_z_vi_=f.vi;return {true};}
  if(f.vi<=neutral_z_vi_||!f.pad_observed||!f.z_released)return {true};
  state_.phase=Phase::ShotHold;state_.stage_vi=shot_vi_=f.vi;aim_seen_=second_press_=false;shot_paused_=false;
 }
 if(state_.phase==Phase::ShotHold||state_.phase==Phase::ShotRelease||state_.phase==Phase::SeekDeath){
  if(s.retry_epoch!=state_.activation_epoch){fail("retry_without_native_death_event");return {true};}
  if(s.completed!=state_.checkpoint||s.index!=state_.checkpoint||s.defeated){fail("boss_defeated_during_intended_loss");return {true};}
  if(inventory::fatal_death_state(ram,state_.player))return {true};
  if(s.phase!=core::Phase::Fighting){fail("unexpected_phase_during_intended_loss");return {true};}
  if(state_.phase==Phase::SeekDeath){if(f.vi-shot_vi_>12000){fail("native_defeat_deadline");return {true};}return approach(ram,f,false);}
  bool aiming=false;for(unsigned i=0;i<20;++i){const auto child=n::word(ram,state_.player+0x188+i*4);if(n::actor(ram,child,0x4005)&&n::word(ram,child+0x180)==state_.player)aiming=true;}
  const auto priv=n::word(ram,state_.player+0x1F4);
  // Pause only the QA's local deadlines while native controls are locked.
  // The guest VI, intro, physics and global bounded test clock never change.
  const bool shot_controls=f.controls&&(aiming||aim_seen_||(f.egg_admission_observed&&f.egg_admitted));
  if(!shot_controls){if(!shot_paused_){shot_paused_=true;shot_pause_vi_=f.vi;}if(f.vi-shot_pause_vi_>3600){fail("native_egg_control_wait_deadline");return {true};}return {true,0,0,0,true};}
  if(shot_paused_){const auto blocked=f.vi-shot_pause_vi_;state_.stage_vi+=blocked;shot_vi_+=blocked;shot_paused_=false;}
  if(f.vi-shot_vi_>12000){fail("native_defeat_deadline");return {true};}
  if(state_.phase==Phase::ShotHold){
   if(aiming){aim_seen_=true;second_press_=n::word(ram,priv+0x1E0)==1;}
   if(!aim_seen_&&f.vi-state_.stage_vi>=180){fail("native_aim_not_observed");return {true};}
   if(aim_seen_&&f.vi-state_.stage_vi>=30){state_.phase=Phase::ShotRelease;state_.stage_vi=f.vi;return {true};}
   return {true,0x2000,0,0};
  }
  const unsigned age=f.vi-state_.stage_vi;
  if(!aiming){inventory::Payload live;if(current_inventory(ram,f,live)&&live.count<activation_.count){state_.fifo_changed=true;state_.phase=Phase::SeekDeath;return approach(ram,f,false);}}
  if(age>=240){fail("native_egg_release_not_observed");return {true};}
  return {true,uint16_t(second_press_&&age>=2&&age<4?0x2000:0),0,0};
 }
 if(state_.phase==Phase::AwaitRetry){
  if(f.vi-state_.stage_vi>1800){fail("native_retry_arrival_deadline");return {true};}
  if(s.retry_epoch==state_.activation_epoch)return {true};
  if(s.retry_epoch!=state_.activation_epoch+1){fail("retry_epoch_skipped");return {true};}
  if(f.retry_restored_epoch!=s.retry_epoch)return {true};
  if(f.room!=83||s.phase!=core::Phase::Rest||s.completed!=state_.checkpoint||s.index!=state_.checkpoint-1||s.max_completed!=maximum_||s.checkpoint_completed!=state_.checkpoint||s.checkpoint_awake_mask!=(before_mask_|boss_checkpoint0260::awake_bit(state_.checkpoint))||!f.saved||*f.saved!=activation_){fail("retry_rest_identity_or_progress_mismatch");return {true};}
  if(!f.controls)return {true};
  const auto player=state_.player;inventory::Payload live;
  if(!current_inventory(ram,f,live)||!inventory::matches(ram,activation_)||n::byte(ram,player+0x17D)!=2||n::byte(ram,player+0x165)||n::word(ram,player+0x180)){fail("retry_activation_inventory_not_restored");return {true};}
  if(n::word(ram,player+0x44)!=plane_||n::word(ram,player+0x8C)!=plane_||n::word(ram,player+0xCC)||n::word(ram,player+0xD8)||n::word(ram,player+0x114)){fail("retry_pose_not_restored");return {true};}
  for(unsigned i=0;i<20;++i)if(n::actor(ram,n::word(ram,player+0x188+i*4),0x4015)){fail("death_carrier_survived_retry");return {true};}
  if(!f.ledger.valid||f.ledger!=death_ledger_){fail("consumed_resources_rolled_back");return {true};}
  state_.restored_epoch=s.retry_epoch;emit("restored",ram,f,live);++state_.completed_tests;
  admission_vi_=neutral_z_vi_=shot_pause_vi_=0;shot_paused_=false;seek_contact_={};
  if(state_.completed_tests==3)state_.phase=Phase::Done;
  else{state_.checkpoint+=3;state_.phase=Phase::WaitPrefix;state_.stage_vi=f.vi;crossing_=false;point_.reset();}
  return {true};
 }
 return {};
}
inline void Controller::native_death(uint8_t* ram,const recomp_context* c,const Frame& f){
 if(!state_.enabled||state_.phase==Phase::Done||state_.phase==Phase::Failed||state_.phase==Phase::AwaitRetry)return;
 if(state_.phase!=Phase::AwaitFight&&state_.phase!=Phase::ShotHold&&state_.phase!=Phase::ShotRelease&&state_.phase!=Phase::SeekDeath)return;
 if(!scope(ram,f)||!c)return;const auto& s=*f.session;const auto player=state_.player;
 if(!inventory::fatal_death_state(ram,player)||n::word(ram,0x800F8828)!=player||uint32_t(c->r4)!=player)return;
 const auto ra=uint32_t(c->r31),priv=n::word(ram,player+0x1F4);if(!inventory::pointer(priv,0x12D0))return;
 const bool caller=ra==0x80030780?int32_t(n::word(ram,player+0x38))<-40:
  ra==0x800301D4&&n::word(ram,priv+0xC0)==0x8002F5F4&&int32_t(n::word(ram,priv+0xD8))>=3;
 if(!caller)return;
 inventory::Payload live;
 if(s.retry_epoch!=state_.activation_epoch||s.completed!=state_.checkpoint||s.index!=state_.checkpoint||s.defeated||!f.saved||*f.saved!=activation_||!current_inventory(ram,f,live,true)||!f.ledger.valid){fail("native_death_scope_mismatch");return;}
 if(f.ledger.treeConsumed<activation_ledger_.treeConsumed||f.ledger.hybridConsumed<activation_ledger_.hybridConsumed||(activation_ledger_.heartConsumed&&!f.ledger.heartConsumed)||f.vi<=state_.activation_vi){fail("resource_consumption_rewound_before_death");return;}
 death_ledger_=f.ledger;state_.death_vi=f.vi;emit("death",ram,f,live,ra);state_.phase=Phase::AwaitRetry;state_.stage_vi=f.vi;
}
} // namespace workshop::boss_route_checkpoint_test0260
