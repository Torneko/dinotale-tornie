#pragma once
#include "bonus_rooms0351.hpp"
#include "bonus_routes0351.hpp"
#include "bonus_session0351.hpp"

// A research/admission plan, not a gameplay adapter. Ordinal links are desired
// transport contracts; no native pot is remapped and no coordinates are chosen.
namespace workshop::bonus_plan0351 {
enum class EntryState {NativeRootSpawn,NativeSpawnKnown,PlacedTargetKnown,
 UnplacedNativeTarget,NeedsAuthoredEntry,FinaleNeedsSetup};
enum class TransportRequirement {NeedsAuthoredTransport};
struct Link {
 unsigned ordinal=0;
 TransportRequirement requirement=TransportRequirement::NeedsAuthoredTransport;
 bool native_transport_verified=false;
};
struct Step {
 unsigned ordinal=0;
 bonus_routes0351::Step route_step{};
 EntryState entry=EntryState::NeedsAuthoredEntry;
 std::optional<bonus_rooms0351::Anchor> known_anchor{};
 std::vector<bonus_rooms0351::CandidateNativeTarget> unplaced_candidates{};
 std::optional<Link> previous{},next{};
 // A known native coordinate is not proof of admission to an authored route.
 bool native_admission_verified=false;
};
struct Plan {
 std::string original_sha256{};
 unsigned bonus=0,cursor=0;
 uint64_t seed=0,revision=0;
 bonus_session0351::Phase phase=bonus_session0351::Phase::Ready;
 std::vector<Step> steps{};
 static constexpr bool native_playable=false;
};
// Parses the exact original ROM through Catalog::read rather than trusting a
// caller-mutated Catalog. A supplied revision is a caller fence, not disk I/O.
// Throws on corrupt ROM/catalogue origins, noncanonical routes, invalid ledger
// state, or a route whose seed/order differs from the saved attempt.
Plan build(std::span<const uint8_t> original,const bonus_routes0351::Route&,
           const bonus_session0351::Snapshot&,uint64_t expected_revision);
} // namespace workshop::bonus_plan0351
