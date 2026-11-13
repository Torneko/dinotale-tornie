#pragma once
#include "bonus_routes0351.hpp"
#include <array>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>

// Preparation ledger only: no native room transition, checkpoint placement,
// health, eggs, ownership, wallet or original24-course progression is changed.
namespace workshop::bonus_session0351 {
inline constexpr const char* filename="pc-bonus-session0353.json";
inline constexpr bool native_playable=false;
enum class Phase:unsigned {Ready,Active,Finale,Completed};
struct Attempt {
 unsigned bonus=0;
 uint64_t seed=0;
 Phase phase=Phase::Ready;
 unsigned cursor=0,visited_mask=0,cleared_prefix=0;
 std::vector<unsigned> order{};
 bool operator==(const Attempt&)const=default;
};
enum class Transition {Changed,Unchanged,InvalidState,WrongRoom,Unqualified};
Attempt make_ready(unsigned bonus,uint64_t seed);
bool valid(const Attempt&) noexcept;
Transition enter(Attempt&,unsigned expected_room);
// qualified is evidence supplied by the caller for the CURRENT step only.
Transition advance(Attempt&,unsigned expected_room,bool qualified);
Transition backtrack(Attempt&,unsigned visited_index);
// The caller chooses a genuine checkpoint index. This restores only position;
// it never restores health or inventory and never redraws the route.
Transition retry(Attempt&,unsigned visited_index);
struct Snapshot {
 uint64_t revision=0;
 unsigned completed_mask=0;
 std::array<std::optional<Attempt>,6> attempts{};
 bool readable=false,last_write_succeeded=false;
 std::string reason{};
};
enum class Status {Committed,Unchanged,InvalidArgument,InvalidTransition,WrongRoom,
 Unqualified,Locked,Conflict,Unavailable,PersistenceFailed,RevisionExhausted};
struct Reply {
 Status status=Status::Unavailable;
 uint64_t revision=0;
 bool accepted()const{return status==Status::Committed||status==Status::Unchanged;}
};
class Store {
public:
 // Missing journal is empty and read-only; malformed journals are preserved.
 // The profile is bound to its canonical absolute path at load, independent
 // of later process working-directory changes.
 bool load(const std::filesystem::path& profile);
 bool refresh();
 Snapshot snapshot()const;
 // Explicit NewAttempt is the only operation allowed to reset seed/order.
 // Completion eligibility is the union of the two externally supplied masks;
 // those masks are never stored or modified by this ledger.
 Reply new_attempt(uint64_t revision,unsigned bonus,uint64_t seed,
                   uint32_t classic_clears,uint32_t hybrid_clears);
 Reply enter(uint64_t revision,unsigned bonus,unsigned expected_room);
 Reply advance(uint64_t revision,unsigned bonus,unsigned expected_room,bool qualified);
 Reply backtrack(uint64_t revision,unsigned bonus,unsigned visited_index);
 Reply retry(uint64_t revision,unsigned bonus,unsigned visited_index);
private:
 Reply apply(uint64_t,const std::function<Status(Snapshot&)>&);
 Reply step(uint64_t,unsigned,const std::function<Transition(Attempt&)>&);
 std::filesystem::path profile_,file_;
 Snapshot view_{};
 bool initialized_=false,present_=false;
};
} // namespace workshop::bonus_session0351
