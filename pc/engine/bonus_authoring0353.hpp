#pragma once
#include "bonus_rooms0351.hpp"
#include "bonus_routes0351.hpp"
namespace workshop::bonus_authoring0353 {
using Point=bonus_rooms0351::Point;
enum class Direction {Backward,Forward};
enum class Provenance {NativePot,NativeExit,NativeArrival,CollisionDerived};
struct CellEvidence {uint16_t resource=0,mask=0;unsigned cell=0;int x=0,y=0;bool floor=false;bool operator==(const CellEvidence&)const=default;};
struct Clearance {
    uint16_t resource=0,floor_mask=0;unsigned floor_cell=0,width_tiles=0,height_tiles=0;
    int floor_x=0,floor_y=0;uint32_t crc32=0;Point contact{};
    std::vector<CellEvidence> cells;
};
struct ContactCalibration {
    unsigned room=177,placement_index=0;uint16_t kind=0,resource=0;
    Point native_point{};uint32_t foot_offset_bits=0;Clearance evidence;
};
struct Site {
    unsigned room=177,ordinal=0;Direction direction=Direction::Forward;Point point{};
    Provenance provenance=Provenance::CollisionDerived;
    std::optional<unsigned> source_placement_index;uint32_t source_packed=0;
    uint16_t template_kind=0;std::optional<Clearance> collision;
    bool reuse_native_pot=false,needs_factory=true,needs_dynamic_admission=false,physical_verified=false;
};
struct AuthoredRoom {
    unsigned ordinal=0;bonus_routes0351::RoomKey key;bonus_routes0351::StepRole role=bonus_routes0351::StepRole::Ordinary;
    std::optional<bonus_rooms0351::Anchor> native_arrival;
    std::vector<bonus_rooms0351::CandidateNativeTarget> dynamic_arrivals;
    std::optional<Site> backward,forward;
    unsigned original_count=0,authored_static_count=0;
    std::vector<uint16_t> preload_types;std::vector<bonus_rooms0351::PlacedPassage> unselected_native_exits;
    bool needs_dynamic_admission=false,needs_second_site=false,needs_native_exit_policy=false;
    static constexpr bool native_playable=false;
};
struct Plan {
    unsigned bonus=0;uint64_t seed=0;std::string original_sha256;
    ContactCalibration calibration;std::vector<AuthoredRoom> rooms;
    bool all_sites_present=false;static constexpr bool native_playable=false;
};
struct PreloadClone {unsigned room=177;size_t original_header=0,cloned_header=0;unsigned before_count=0,after_count=0;uint32_t descriptor_crc32=0;};
struct Prepared {std::vector<uint8_t> rom;std::vector<PreloadClone> clones;size_t allocation_begin=0,allocation_end=0;};
// Pure original-ROM authoring. A new canonical route is reconstructed; native
// metadata/terrain are read locally. Neither this plan nor clearance certifies
// actor factory admission, dynamic entrances or physical accessibility.
Plan build(std::span<const uint8_t> original,const bonus_routes0351::Route&);
// Append descriptor-only preload clones in bank15, preserving every placement
// and all existing bytes except selected same-room table pointers/bank15 base.
// extras is at most two validated original resource kinds (e.g. pot/checkpoint).
// Input prepared content may be16..20MiB; an occupied foreign bank15 is refused.
Prepared clone_preloads(std::span<const uint8_t> original,std::span<const uint8_t> prepared,
                        const bonus_routes0351::Route&,std::span<const uint16_t> extras={});
}
