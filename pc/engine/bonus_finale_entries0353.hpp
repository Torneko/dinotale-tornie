#pragma once
#include "bonus_routes0351.hpp"
#include "bonus_rooms0351.hpp"
#include <array>
#include <optional>
#include <span>
#include <vector>
namespace workshop::bonus_finale_entries0353 {
using Point=bonus_rooms0351::Point;
enum class Origin {NativeIncomingContactAuthoredWarmMarker,OriginalWarmMarker,PriorCompactWarmMarker};
enum class CourseOrigin {ExplicitCoursePolicy}; // Never claims an external-room catalogue origin.
struct Incoming {
 unsigned room=177,course=24,root=177,source_index=0;uint16_t source_kind=0;
 uint32_t source_packed=0,target_packed=0;Point target{};
 size_t resource_a=0,record_rom=0;std::array<uint8_t,24> native_payload{};
 bool operator==(const Incoming&)const=default;
};
struct EntrySpec {
 unsigned bonus=0,from_ordinal=0,room=177,course=24,root=177,marker_index=0;
 uint64_t seed=0;bonus_routes0351::RoomKey from_key{};
 Point point{};uint32_t target_packed=0x44CA2000;
 uint16_t prototype_kind=0x4268;size_t prototype_record_rom=0;
 std::array<uint8_t,24> native_payload{};
 Origin origin=Origin::OriginalWarmMarker;CourseOrigin course_origin=CourseOrigin::ExplicitCoursePolicy;
 std::vector<Incoming> original_incoming;
 std::optional<Point> prior_contact{};
 bool prior_geometry_data_match=false;
 static constexpr bool physical_gameplay_verified=false;
 bool operator==(const EntrySpec&)const=default;
};
// Pure original-ROM/recipe composition, no game memory, saves or source writes.
// previous must be the canonical last ordinary step. Admission course/root is
// an explicit Bonus policy validated against real native course metadata.
// Warm contacts are REAL unique44CA markers in the prepared recipe, never the
// original4001 placeholder0,0; the full marker factory is not claimed here.
EntrySpec prepare(std::span<const uint8_t> original,const bonus_routes0351::Route&,
                  unsigned previous_ordinal,const bonus_routes0351::RoomKey& previous);
bool valid(std::span<const uint8_t> original,const bonus_routes0351::Route&,const EntrySpec&) noexcept;
}
