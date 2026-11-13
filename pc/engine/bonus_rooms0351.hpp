#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>
namespace workshop::bonus_rooms0351 {
inline constexpr const char* original_sha256="e8a63388c38f8f0bea37e9b13a97f0898ac5496e08ca2028afb8db9a866e0ce9";
using Point=std::array<uint32_t,2>; // Exact native IEEE754 bits, never invented coordinates.
struct Placement {unsigned index=0;size_t rom_offset=0;uint32_t packed=0;Point point{};};
enum class AnchorKind {PlayerSpawn,PlacedIncoming,UnplacedNativeTarget};
struct Anchor {
    AnchorKind kind=AnchorKind::PlayerSpawn;unsigned room=177;Placement placement;
    std::optional<unsigned> via_room;std::optional<uint16_t> via_kind;
};
struct Passage {
    uint16_t kind=0;unsigned destination=177;uint32_t target_packed=0;
    size_t resource_a=0,record_rom=0;bool editable_pot=false;
    std::array<uint8_t,24> native_payload{};
};
// native_target preserves even off-page native arrivals; target is usable only
// inside a loaded terrain page. Neither grants graph or gameplay admission.
struct PlacedPassage {Placement source;Passage native;std::optional<Placement> native_target;std::optional<Anchor> target;};
// A real target of a native resource without a placed incoming source. This is
// not promoted to Room::entry: its dynamic admission protocol is unverified.
struct CandidateNativeTarget {Passage native;Anchor target;};
struct Checkpoint {unsigned number=0,destination=177;Placement placement;};
struct Room {
    unsigned id=177,course=24,world=0,root=177;size_t header_rom=0,meta_rom=0,placements_rom=0;
    uint32_t native_init=0;Placement spawn;std::vector<Placement> placements;
    std::optional<Anchor> entry;std::vector<PlacedPassage> passages;
    std::vector<CandidateNativeTarget> candidates;std::vector<Checkpoint> checkpoints;
};
struct Course {unsigned index=24,world=0,root=177;std::vector<unsigned> rooms;std::array<unsigned,4> warp_destinations{};};
struct World {unsigned number=0;std::vector<unsigned> courses,rooms;};
struct Catalog {
    std::string source_sha256;std::vector<Room> rooms;
    std::array<Course,24> courses;std::array<World,6> worlds;
    const Room* find(unsigned room)const noexcept;
};
// Pure CPU parser: normalized original USA ROM only; no I/O, patch, unlock,
// source writes or gameplay admission. Throws on malformed/inexact input.
Catalog read(std::span<const uint8_t> original);
}
