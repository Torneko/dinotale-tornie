#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>
namespace workshop::story_hard_rooms0330 {
inline constexpr size_t allocation_start=0xFF0000,allocation_limit=0x1000000;
inline constexpr uint32_t bubble_packed=0x46920000;
enum class AnchorKind {Warp4,NativeEntry};
struct Anchor {
 AnchorKind kind=AnchorKind::Warp4;
 uint32_t packed=0,x=0,y=0;
 unsigned room=177,via_room=177;
 uint32_t via_packed=0;
 size_t record=0;
};
struct Plan {
 unsigned level=24,room=177,original_count=0,added_index=0;
 uint32_t packed=bubble_packed,x=0,y=0;
 Anchor anchor{};
 size_t source_header=0,source_meta=0,source_objects=0;
 size_t objects=0,meta=0,header=0,record=0;
 bool terrain_page_validated=false;
 // Existing page presence is not proof of a safe floor or lack of occlusion.
 bool floor_or_occlusion_certified=false;
};
struct ExtraEggSource {unsigned level=24,room=177;uint32_t packed=0,x=0,y=0;unsigned slot=255;};
struct Applied {std::vector<uint8_t> rom;std::array<Plan,24> plans{};size_t allocation_end=allocation_start;std::vector<ExtraEggSource> extra_eggs;};
// Pure in-memory ROM transform. Adds one native4692 question-bubble record per
// course, preserves every original object and all shared/canonical progression.
// The caller supplies its already validated USA source and owns mode6 gating.
Applied apply(std::span<const uint8_t> original);
} // namespace workshop::story_hard_rooms0330
