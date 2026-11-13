#pragma once
#include "bonus_session0351.hpp"
#include "bonus_finale_entries0353.hpp"
#include <mutex>

namespace workshop::bonus_runtime0352 {
inline constexpr const char8_t* bonus_save=u8"yoshisstory.n64.us.bonus0353";
enum class LaunchPolicy {FullRoute,PrivateTwoRoomBridge,PrivateAuthoredOrdinary,PrivateAuthoredComplete};
enum class Status {Idle,Accepted,Ready,Locked,InvalidArgument,NoAttempt,Completed,
 StaleTicket,WrongRoom,PlayerNotReady,NeedsAuthoredRoute,NeedsFinaleSetup,
 InvalidState,PersistenceUnavailable,PersistenceFailed,Conflict};
struct Access {
 uint32_t classic_mask=0,hybrid_mask=0;
 unsigned unlocked_mask=0;
 bool classic_present=false,hybrid_present=false,classic_readable=false,hybrid_readable=false;
 std::string reason{};
};
struct Ticket {
 uint64_t epoch=0,revision=0,seed=0;
 unsigned bonus=0,ordinal=0,room=177,course=24,root=177;
 bonus_routes0351::EntryRequirement entry=bonus_routes0351::EntryRequirement::NativeAnchorPending;
 LaunchPolicy policy=LaunchPolicy::FullRoute;
 std::u8string save_stem=bonus_save;
 bool operator==(const Ticket&)const=default;
};
// One-shot native bridge receipt. No caller-supplied "defeated" boolean is
// accepted; the native bridge owns causal defeat/arena-generation evidence.
struct FinaleReceipt {
 unsigned bonus=0,room=177,course=24,defeated_count=0;
 uint64_t seed=0,attempt_epoch=0,ticket_epoch=0,arena_generation=0;
};
struct Observation {unsigned room=177,course=24,root=177;bool player_ready=false;};
struct Admission {Status status=Status::PersistenceUnavailable;std::optional<Ticket> ticket{};};
// Thread-safe UI/native-event service. Read-only getters perform no filesystem
// I/O. No method writes canonical saves,24-course records, wallets or ownership.
class Service {
public:
 bool load(const std::filesystem::path& profile);
 void refresh_access();
 Access access()const;
 bonus_session0351::Snapshot session()const;
 std::optional<Ticket> current_ticket()const;
 Status status()const;
 // The trusted caller must first verify BOTH its diagnostic flag and marker in
 // a separate private profile. This attestation NEVER changes completion rights.
 void enable_private_bridge(bool trusted_private_profile);
 // Independent0353 QA marker only. All ordinary links/factories must have
 // been prepared by the trusted caller for this exact canonical route.
 bool enable_private_authored(const bonus_routes0351::Route&,bool trusted_private_profile);
 // Attests one exact canonical finale recipe/real warm marker for a private
 // native attempt. This stores RAM metadata only; it never grants progression.
 bool enable_private_finale(std::span<const uint8_t> original,const bonus_routes0351::Route&,
                            const bonus_finale_entries0353::EntrySpec&,uint64_t native_attempt_epoch,
                            bool trusted_private_profile);
 Admission complete_finale(const Ticket&,const FinaleReceipt&);
 Admission new_attempt(unsigned bonus,uint64_t seed);
 Admission resume(unsigned bonus);
 // FullRoute is deliberately gated until physical links/finales are authored.
 Admission prepare(LaunchPolicy policy=LaunchPolicy::FullRoute);
 Admission room_ready(const Ticket&,const Observation&);
 // qualified must come from a genuine current-room departure event supplied
 // by the native adapter. This service does not invent that evidence.
 Admission depart(const Ticket&,const Observation&,bool qualified,LaunchPolicy);
 Admission backtrack(const Ticket&,const Observation&,unsigned visited_index,LaunchPolicy);
 // Position only: the native caller owns genuine death/checkpoint evidence and
 // health/inventory policy. Retrying never rerolls or heals anything here.
 Admission retry(const Ticket&,unsigned visited_index,LaunchPolicy);
 void cancel();
private:
 void access_locked();
 void invalidate_locked();
 Admission reply_locked(Status,bool with_ticket=false);
 Admission prepare_locked(LaunchPolicy);
 bool ticket_locked(const Ticket&)const;
 bool finale_locked(unsigned bonus,uint64_t seed)const;
 bool policy_locked(LaunchPolicy,unsigned bonus,uint64_t seed,unsigned ordinal)const;
 bool observed_locked(const Ticket&,const Observation&)const;
 Admission move_locked(const Ticket&,unsigned,LaunchPolicy,bool backward);
 mutable std::mutex mutex_;
 std::filesystem::path profile_;
 bonus_session0351::Store store_;
 Access rights_{};
 unsigned selected_=0;
 uint64_t epoch_=0;
 bool initialized_=false,bridge_enabled_=false,authored_enabled_=false,ready_=false;
 unsigned authored_bonus_=0;uint64_t authored_seed_=0;
 std::optional<bonus_finale_entries0353::EntrySpec> finale_entry_{};
 uint64_t finale_attempt_epoch_=0;
 std::optional<Ticket> ticket_{};
 Status status_=Status::PersistenceUnavailable;
};
} // namespace workshop::bonus_runtime0352
