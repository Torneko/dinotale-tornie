#pragma once
#include <cstdint>
#include <optional>
#include <vector>

namespace workshop::bonus_routes0351 {
inline constexpr unsigned route_schema=2,bonus_count=6,course_count=24;
inline constexpr uint32_t course_mask=0xFFFFFFu;
// Preparation metadata only. There is no native adapter or playability claim.
inline constexpr bool native_playable=false;
enum class StepRole:uint8_t {Ordinary,Finale};
enum class EntryRequirement:uint8_t {NativeRootSpawn,NativeAnchorPending,FinaleSetupPending};
enum class FinaleFamily:uint8_t {Marmots,Lips,YellowCandy,Fish,Genie,Bowser};
struct RoomKey {
 unsigned room=177;
 // Catalogue ownership, not a guessed origin for an external boss arena.
 std::optional<unsigned> course{},root{};
 bool operator==(const RoomKey&)const=default;
};
struct Step {
 StepRole role=StepRole::Ordinary;
 RoomKey key{};
 EntryRequirement entry=EntryRequirement::NativeAnchorPending;
 bool operator==(const Step&)const=default;
};
struct FinaleSpec {
 FinaleFamily family=FinaleFamily::Marmots;
 std::vector<unsigned> arena_sequence{};
 unsigned members=0;
 std::optional<unsigned> hp_per_member{};
 // Includes Bowser's preparatory composite; native chaining is not certified.
 bool native_sequence_verified=false;
 bool operator==(const FinaleSpec&)const=default;
};
struct Route {
 unsigned schema=route_schema,bonus=0; // bonus is 1..6, displayed as7-1..7-6
 uint64_t seed=0;
 std::vector<Step> steps{};
 FinaleSpec finale{};
 static constexpr bool native_playable=false;
 bool operator==(const Route&)const=default;
};
// Invalid bonus numbers or bits outside the24-course masks throw invalid_argument.
bool unlocked(unsigned bonus,uint32_t classic_clears,uint32_t hybrid_clears);
uint8_t unlocked_mask(uint32_t classic_clears,uint32_t hybrid_clears);
std::vector<RoomKey> world_pool(unsigned bonus);
// Known113 catalogue rooms only; external arenas return nullopt.
std::optional<RoomKey> source_key(unsigned room) noexcept;
// Does not grant access or touch progression. The caller separately checks unlocked.
Route make_route(unsigned bonus,uint64_t seed);
// Exact schema1 canonical reconstruction, including origin, order and finale.
bool validate(const Route& route) noexcept;
} // namespace workshop::bonus_routes0351
