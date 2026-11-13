#pragma once
#include "bonus_runtime0352.hpp"
#include "bonus_rooms0351.hpp"
#include "bonus_finale_entries0353.hpp"
#include "recomp.h"
#include <memory>
namespace workshop::bonus_retry0353 {
inline constexpr uint32_t saved_player_record_ram=0x807EF000; // Root reserves this16-byte region from transports
struct Environment {
 int pc_mode=-1;unsigned bonus=0;uint64_t seed=0,attempt_epoch=0;
 unsigned owned_mask=0;
 bool independent_bonus_save=false,canonical=true,custom_active=true,deferred=true;
};
enum class Phase {Disabled,Live,DeathObserved,SelectionPending,GameOver,AdmissionPending,Installed,Arrived,Rejected};
struct Checkpoint {
 unsigned ordinal=0,room=177,course=24,root=177,index=4,awake_mask=0;
 std::array<uint8_t,24> native_record{};
 std::array<uint8_t,12> player_record{}; // exact native saved4001 record after wake
 bonus_rooms0351::Point point{};
};
struct RetryPlan {
 uint64_t death_epoch=0;bonus_runtime0352::Ticket previous;
 unsigned target_ordinal=0,room=177,course=24,root=177,post_death_team=0,dead_color=18;
 uint32_t dead_player=0;uint64_t dead_birth=0;
 std::optional<Checkpoint> checkpoint{};
};
struct Receipt {RetryPlan retry;bonus_runtime0352::Ticket admission;uint32_t player=0;uint64_t birth=0;unsigned native_health=0,eggs=0;};
struct View {Phase phase=Phase::Disabled;std::optional<RetryPlan> retry;std::optional<Checkpoint> checkpoint;std::string reason;};
// Native game-thread observer/admission adapter. It owns no file/save/RNG and
// never writes team, HP, eggs, scales, actor coordinates or controller fields.
class Adapter {
public:
 Adapter();~Adapter();Adapter(const Adapter&)=delete;Adapter&operator=(const Adapter&)=delete;
 bool configure(std::span<const uint8_t> original,const bonus_routes0351::Route&,const Environment&);
 // Explicit native final-arena proof for the same canonical attempt. This
 // never extends checkpoint placement or changes the ordinary fallback.
 bool attest_finale(std::span<const uint8_t> original,const bonus_finale_entries0353::EntrySpec&,const Environment&);
 bool bind_room(const bonus_runtime0352::Ticket&,const bonus_session0351::Snapshot&,const Environment&);
 void created(const recomp_context*,const Environment&); //80045CAC entry, before magic init
 void forget(uint32_t actor,const Environment&);
 void wake_begin(uint8_t*,const recomp_context*,const Environment&); //805A5628 entry
 bool wake_finish(uint8_t*,const Environment&); //next genuine manager frame after wake body
 bool death_begin(uint8_t*,const recomp_context*,const Environment&); //807F1100 ENTRY
 //8006E0FC entry called by C214 at807F1190. Does not rewrite scene8/29.
 std::optional<RetryPlan> death_scene(uint8_t*,const recomp_context*,const Environment&);
 // Root's authored admission service chooses position only. This verifies the
 // returned ticket; scene29 requires the separate native Continue flow first.
 bool bind_admission(const bonus_runtime0352::Admission&,const Environment&);
 //Call only before native8006D7E8 after Root has installed target course. Writes
 //authentic checkpoint24/awake state/F84CC, or clears checkpoint for first-root
 //fallback. Never changes the original4001 placement, even when it is0,0.
 bool before_room_selection(uint8_t*,const Environment&);
 //Requires a NEW observed native birth; no old death graph is reactivated.
 std::optional<Receipt> arrival(uint8_t*,unsigned observed_room,const Environment&);
 View view()const;
 void reset(uint8_t* rdram=nullptr); // Owned pointer requires RAM; nullptr refuses to discard ownership.
private:struct Impl;std::unique_ptr<Impl> p;
};
}
