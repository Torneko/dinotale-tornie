#pragma once
#include <bitset>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace workshop::heart_wallet0283 {
inline constexpr unsigned level_count=24,hearts_per_level=3,slot_count=72;
inline constexpr unsigned life_cost=5,eggs_cost=3,start_life=8,start_eggs=6;
inline constexpr const char* filename="pc-heart-wallet.json";
using Held=std::bitset<slot_count>;
// The adapter must supply an independently proved PHYSICAL slot, never the
// collection order, current heart counter, bestHearts, or actor allocation order.
constexpr std::optional<unsigned> index(unsigned level,unsigned slot){
    if(level>=level_count||slot>=hearts_per_level)return {};
    return level*hearts_per_level+slot; // zero-based Trial level and physical slot
}
struct Pending {
    bool life=false,eggs=false;
    bool operator==(const Pending&)const=default;
    bool empty()const{return !life&&!eggs;}
};
struct Data {
    Held held{};
    Pending pending{};
    std::string last_attempt; // successful current-attempt consumption; no achievements
    bool operator==(const Data&)const=default;
    unsigned balance()const{return unsigned(held.count());}
};
struct SpendPlan {Held remaining{},removed{};};
// Positive costs only. Partial Fisher-Yates with rejection sampling: every
// held subset of the requested size has equal probability; seed behavior is
// fixed across platforms and independent of std::uniform_int_distribution.
std::optional<SpendPlan> plan_spend(const Held& held,unsigned cost,uint64_t seed);
enum class Bonus {Life,Eggs};
enum class Status {Committed,AlreadyHeld,InvalidSlot,InvalidCost,InsufficientFunds,
    Unavailable,PersistenceFailed,AlreadyPending,NoPending,InvalidBonus,
    AlreadyConsumed,InvalidAttempt,PendingChanged};
struct Reply {
    Status status=Status::Unavailable;
    Held removed{};
    Pending bonuses{}; // nonempty only after consume_pending commits
    unsigned balance=0;
    bool committed()const{return status==Status::Committed;}
};
struct Snapshot {
    Data value{};
    bool readable=false,last_write_succeeded=false;
    std::string reason;
    bool purchases_enabled()const{return readable;}
};
// Explicit profile directory only. Missing wallet starts empty, without
// touching existing achievements. Invalid JSON is retained and disables every
// update until an explicit successful load. No game/RAM/menu API is called.
class Wallet {
public:
    bool load(const std::filesystem::path& profile);
    Snapshot snapshot()const;
    bool refresh(); // bounded read-only disk validation before native effects
    Reply claim(unsigned level,unsigned slot);
    Reply spend(unsigned cost,uint64_t seed);
    Reply purchase(Bonus bonus,uint64_t seed);
    // Caller invokes at the actual next-attempt boundary, after launch
    // preflight. This module grants no health/eggs; a failed commit returns no
    // bonus, and a successful consume clears both flags exactly once.
    Reply consume_pending();
    // Called only AFTER native readback. Retrying this same token is harmless.
    // Clears only the applied flags, preserving another newly queued bonus.
    // Tokens are 32 lowercase hex characters, unique to a native attempt.
    Reply consume_pending(std::string_view attempt,Pending applied);
private:
    enum class Action {Claim,Spend,Life,Eggs,Consume,ConsumeScoped};
    Reply update(Action action,unsigned a,unsigned b,uint64_t seed,std::string_view attempt={},Pending applied={});
    std::filesystem::path profile_,file_;
    Snapshot view_{};
    bool initialized_=false,present_=false;
};
} // namespace workshop::heart_wallet0283
