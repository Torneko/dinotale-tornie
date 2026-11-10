#pragma once
#include <array>
#include <bit>
#include <cstdint>

namespace workshop::coop_attempt0281 {
// HOST-only handshake. The Bridge must attest every receipt from native
// callbacks/registrations at an outer fence. This model never reads or writes
// guest RAM, creates actors, changes input affinity or revives a life flag.
inline constexpr unsigned color_count=14, color_mask=0x3FFF;
struct Identity {
 uint32_t address=0,private_data=0;uint64_t birth=0;uint16_t kind=0;
 bool operator==(const Identity&)const=default;
};
inline bool valid(const Identity& id){
 return id.kind==0x4001&&id.birth&&id.address>=0x80000400u&&id.address<=0x807FFE04u&&!(id.address&3)&&
        id.private_data>=0x80000400u&&id.private_data<=0x807FEAB0u&&!(id.private_data&3);
}
inline bool valid(const std::array<Identity,2>& roots){
 return valid(roots[0])&&valid(roots[1])&&roots[0].address!=roots[1].address&&
        roots[0].private_data!=roots[1].private_data&&roots[0].birth!=roots[1].birth;
}
enum class Phase {unbound,active,awaiting_native_retry,rearmed};
enum class Reason {
 disabled,not_waiting,scene_exit_not_observed,native_scene_not_ready,teardown_incomplete,
 mask_not_observed,canonical_not_ready,stale_primary,primary_not_available,
 fewer_than_two_colors,ticket_exhausted,ready
};
inline const char* name(Reason r){
 switch(r){
 case Reason::disabled:return "disabled";case Reason::not_waiting:return "not_waiting";
 case Reason::scene_exit_not_observed:return "scene_exit_not_observed";
 case Reason::native_scene_not_ready:return "native_scene_not_ready";
 case Reason::teardown_incomplete:return "teardown_incomplete";
 case Reason::mask_not_observed:return "mask_not_observed";
 case Reason::canonical_not_ready:return "canonical_not_ready";
 case Reason::stale_primary:return "stale_primary";
 case Reason::primary_not_available:return "primary_not_available";
 case Reason::fewer_than_two_colors:return "fewer_than_two_colors";
 case Reason::ticket_exhausted:return "ticket_exhausted";case Reason::ready:return "ready";
 }return "invalid";
}
struct Terminal {
 uint64_t epoch=0;std::array<Identity,2> roots{};
 // Set both_down_positive ONLY at Lifecycle's proved native final-death
 // branch. An invalidation, a missing root or an offscreen Y are not evidence.
 bool both_down_positive=false,invalidated=false;
};
struct Teardown {
 bool context_gone=false,owner_stack_empty=false,scopes_zero=false,owned_actors_empty=false;
 bool factory_idle=false,rooms_idle=false,pools_closed=false,old_roots_absent=false;
 bool complete()const{return context_gone&&owner_stack_empty&&scopes_zero&&owned_actors_empty&&
                            factory_idle&&rooms_idle&&pools_closed&&old_roots_absent;}
};
struct Frame {
 unsigned vi=0,stage=0,scene=0,room=0,actual_alive_mask=0,primary_color=color_count;
 bool mask_observed=false,canonical_ready=false;
 Identity canonical_primary{};Teardown teardown{};
};
struct Decision {
 bool ready=false;Reason reason=Reason::disabled;
 uint64_t ticket=0,epoch=0;unsigned vi=0,eligible_mask=0,primary_color=color_count,secondary_color=color_count;
 Identity primary{};
 bool operator==(const Decision&)const=default;
};
class Coordinator final {
 bool enabled_=false,left_gameplay_=false;
 unsigned preferred_secondary_=5,lost_mask_=0;uint64_t epoch_=0,next_ticket_=0;
 Phase phase_=Phase::unbound;std::array<Identity,2> roots_{};
 std::array<unsigned,2> colors_{color_count,color_count};Decision pending_{};
public:
 // Explicit fresh startup only. The Bridge must never call this merely to
 // unstick a live attempt. Lost colors survive successful retry commits.
 void configure(bool on,unsigned preferred_secondary=5){
  enabled_=on;preferred_secondary_=preferred_secondary;lost_mask_=0;epoch_=0;
  phase_=Phase::unbound;left_gameplay_=false;roots_={};colors_={color_count,color_count};pending_={};
 }
 bool enabled()const{return enabled_;}Phase phase()const{return phase_;}
 unsigned lost_mask()const{return lost_mask_;}uint64_t epoch()const{return epoch_;}
 unsigned eligible(unsigned actual_alive_mask)const{return enabled_?(actual_alive_mask&color_mask&~lost_mask_):actual_alive_mask;}
 bool bind(uint64_t epoch,const std::array<Identity,2>& roots,const std::array<unsigned,2>& colors){
  if(!enabled_||!epoch||!valid(roots)||colors[0]>=color_count||colors[1]>=color_count)return false;
  if(phase_==Phase::active){
   if(epoch!=epoch_||colors!=colors_)return false;
   // Individual native respawns retain the attempt and its colors. A changed
   // incarnation must be newer, never an old address/birth replay.
   for(unsigned p=0;p<2;p++)if(roots[p]!=roots_[p]&&roots[p].birth<=roots_[p].birth)return false;
  }else{
   if((phase_!=Phase::unbound&&phase_!=Phase::rearmed)||epoch<=epoch_)return false;
   if(phase_==Phase::rearmed&&(roots[0]!=pending_.primary||colors[0]!=pending_.primary_color||colors[1]!=pending_.secondary_color))return false;
  }
  epoch_=epoch;roots_=roots;colors_=colors;phase_=Phase::active;pending_={};left_gameplay_=false;return true;
 }
 bool finish(const Terminal& t){
  if(!enabled_||t.epoch!=epoch_||t.roots!=roots_||(!t.both_down_positive&&!t.invalidated))return false;
  if(phase_!=Phase::active)return false;
  if(t.both_down_positive)lost_mask_|=(1u<<colors_[0])|(1u<<colors_[1]);
  phase_=Phase::awaiting_native_retry;left_gameplay_=false;pending_={};return true;
 }
 Decision frame(const Frame& f){
  // An uncommitted proposal belongs to this exact fence; a later pending
  // observation revokes it. A committed grant survives until the new bind.
  if(phase_==Phase::awaiting_native_retry)pending_={};Decision d;d.epoch=epoch_;d.vi=f.vi;d.eligible_mask=eligible(f.actual_alive_mask);
  auto reject=[&](Reason reason){d.reason=reason;return d;};
  if(!enabled_)return reject(Reason::disabled);
  if(phase_!=Phase::awaiting_native_retry)return reject(Reason::not_waiting);
  if(f.stage!=7)left_gameplay_=true;
  if(!left_gameplay_)return reject(Reason::scene_exit_not_observed);
  // This small adapter admits the real native Trial retry observed in 1-1.
  // It does not infer a fresh attempt from a warm 43<->81 room transition.
  if(f.stage!=7||f.scene!=5||f.room!=43)return reject(Reason::native_scene_not_ready);
  if(!f.teardown.complete())return reject(Reason::teardown_incomplete);
  if(!f.mask_observed)return reject(Reason::mask_not_observed);
  if(!f.canonical_ready||!valid(f.canonical_primary))return reject(Reason::canonical_not_ready);
  for(const auto& old:roots_)if(f.canonical_primary.birth<=old.birth)return reject(Reason::stale_primary);
  if(f.primary_color>=color_count||!(d.eligible_mask&(1u<<f.primary_color)))return reject(Reason::primary_not_available);
  if(std::popcount(d.eligible_mask)<2)return reject(Reason::fewer_than_two_colors);
  const auto remaining=d.eligible_mask&~(1u<<f.primary_color);
  d.secondary_color=preferred_secondary_<color_count&&(remaining&(1u<<preferred_secondary_))?
                    preferred_secondary_:unsigned(std::countr_zero(remaining));
  if(next_ticket_==UINT64_MAX)return reject(Reason::ticket_exhausted);
  d.ready=true;d.reason=Reason::ready;d.ticket=++next_ticket_;d.primary=f.canonical_primary;d.primary_color=f.primary_color;
  pending_=d;return d;
 }
 // The caller has completed only the selective HOST metadata reset and kept
 // the freshly registered canonical P1/Registry births and startup settings.
 // No callback, actor, guest flag or FIFO may be forged to obtain this commit.
 bool commit(const Decision& d){
  if(phase_!=Phase::awaiting_native_retry||!d.ready||!d.ticket||d!=pending_)return false;
  phase_=Phase::rearmed;return true;
 }
};
inline Coordinator attempts;
inline unsigned eligible(unsigned mask){return attempts.eligible(mask);}
} // namespace workshop::coop_attempt0281
