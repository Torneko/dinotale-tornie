#pragma once
#include "recomp.h"
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>
namespace workshop::bonus_finales0353 {
enum class Family {Marmots,Lips,Candy,Fish,Genie,Bowser};
enum class FishColor {Red,Green,Purple};
enum class Origin {OriginalPlacement,PriorAuthoredArena,NativePointCopy,AuthoredClearance};
struct Placement {uint32_t packed=0,x=0,y=0;Origin origin=Origin::OriginalPlacement;unsigned original_index=0;};
struct Arena {
 unsigned room=177;uint32_t native_init=0;size_t original_header=0;
 std::array<uint8_t,0x118> descriptor{};std::vector<Placement> placements;
 std::vector<unsigned> boss_indices;std::vector<uint16_t> preload;
 bool compact_fish_geometry=false;
};
struct Plan {
 unsigned bonus=0;Family family=Family::Marmots;std::string original_sha256;
 std::vector<Arena> arenas;unsigned members=0;std::optional<unsigned> hits_each;
 std::array<FishColor,2> fish_colors{FishColor::Red,FishColor::Red};
 bool birds=false,native_lips_pans=false,enhanced_bowser=false,structurally_prepared=false;
 bool bowser_cadence_required=false; // requires the scoped Bonus6 Cadence guard; recipe alone does not install hooks
 std::string pending;
 static constexpr bool physical_gameplay_verified=false;
};
// Reads the exact original USA ROM. Fish colors are explicit input: no hidden
// roll or blue-water fish is introduced. Bowser uses ONE arena167, preserving
// its native internal phases. Bird points are authored clearance, not a
// physical factory/gameplay claim; the separate cadence guard remains required.
Plan prepare(std::span<const uint8_t> original,unsigned bonus,
             std::array<FishColor,2> colors={FishColor::Red,FishColor::Red});
bool valid(const Plan&) noexcept;
// Pure private-ROM operation. Slab must be unused0xFF, caller-owned and disjoint
// from every other patch. Does not export ROM or mutate the original span.
std::vector<uint8_t> install(std::vector<uint8_t> private_rom,const Plan&,
                            size_t slab_begin,size_t slab_end);
struct Frame {uint64_t epoch=0;unsigned room=177,vi=0;uint32_t family_overlay=0,scene_overlay=0;};
struct Member {uint32_t actor=0,private_data=0;uint64_t birth=0;unsigned hits=0,remaining=0;bool initialized=false,active=false,defeated=false;};
struct Snapshot {bool enabled=false,failed=false,ready_to_exit=false;uint64_t epoch=0;unsigned bonus=0,room=177,expected=0,defeated=0;std::array<Member,2> members{};std::string failure;};
struct RetireAction {uint64_t epoch=0,birth=0;unsigned member=0;uint32_t actor=0,function=0x800447C8;};
// This observer reads actual actor/private/callback/stack identities. It never
// accepts a caller's defeated=true flag, invents an HP decrement, awards Story
// completion or changes a Boss Battle campaign singleton. The caller supplies
// the real actor birth generation and native entry/return hooks.
class Observer {
public:
 Observer();~Observer();Observer(Observer&&)noexcept;Observer& operator=(Observer&&)noexcept;
 Observer(const Observer&)=delete;Observer& operator=(const Observer&)=delete;
 bool configure(const Plan&,std::string token,uint64_t epoch,unsigned room);
 bool bind(uint8_t* rdram,unsigned member,uint32_t actor,uint64_t native_birth,const Frame&);
 void entry(uint8_t*,const recomp_context*,std::string_view name,const Frame&);
 void leave(uint8_t*,const recomp_context*,std::string_view name,const Frame&);
 void frame(uint8_t*,const Frame&);
 // Call only at the shared807AEA78 wrapper after entry() observed its real ABI.
 // A successful call suppresses only that proven member's native cinematic.
 bool suppress_completion(uint8_t*,const recomp_context*,const Frame&);
 void forget(uint32_t actor,uint64_t native_birth);
 void arm_bowser(uint8_t*,const recomp_context*,const Frame&);
 bool commit_bowser(uint8_t*,const recomp_context*,const Frame&);
 std::vector<RetireAction> take_retire_actions(uint8_t*,const Frame&);
 bool retired(uint8_t*,const RetireAction&,const Frame&);
 Snapshot snapshot()const;
private:struct Impl;std::unique_ptr<Impl> impl_;
};
} // namespace workshop::bonus_finales0353
