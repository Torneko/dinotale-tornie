#pragma once
#include "yoshi_colors0300.hpp"
#include <filesystem>
#include <string>
namespace workshop::metal_yoshis0300 {
struct View {
    unsigned found=0,story_alive=0; // Independent TWO-bit fields:0Gold,1Silver.
    bool initialized=false,readable=true,saved=true;
    std::string reason;
};
void initialize(const std::filesystem::path& path="pc-metal-yoshis.json");
void start_session(unsigned mode,bool ordinary_solo=true);
// Root must exclude custom content, cooperative play and both Boss runtimes
// before native selection/room admission. No delayed mode is enabled here.
void set_scope(bool ordinary_solo);
View view();
// Search after24 durable Hybrid clears, in Story/Trial/Hybrid (not Practice).
// Returns missing TWO-bit field, without granting or writing anything.
unsigned search_mask();
// These projections use extension indices6/7, suitable for807F1000/ext_unlocked.
unsigned unlocked_extra();
unsigned story_extra(bool continuing);
// Called only after the exact carried native egg reaches a successful tally.
// The acquisition caller owns that proof; this service owns atomic persistence.
bool acquire(unsigned pc_color);
// Re-read under the journal lock for recovery of an already-owned colour.
// Never grants ownership or writes the journal, even after external changes.
bool confirm_owned(unsigned pc_color);
// Root supplies the current TWO-bit live field at genuine Story save/death
// fences. Native EEPROM's six legacy lives and codec bytes remain unchanged.
bool save_story_alive(unsigned two_bits);
bool restore_life(unsigned pc_color);
bool clear_save();
}
