#pragma once
#include <filesystem>
#include <optional>
#include <string>
namespace workshop::elemental_yoshis0355 {
// Separate permanent ownership and temporary Story lives; never legacy saves.
constexpr unsigned bit(unsigned color){return color==16?1u:color==17?2u:0u;}
struct View {unsigned found=0,story_alive=0,hybrid_alive=0;bool initialized=false,readable=true,saved=true;std::string reason;};
void initialize(const std::filesystem::path& path="pc-elemental-yoshis.json");
void start_session(unsigned mode,bool solo=true);void set_scope(bool solo);
View view();
// Eligibility only: each Hard run recruits its own team, including owned colors.
std::optional<unsigned> search_color(unsigned level);
unsigned unlocked_extra();unsigned story_extra(bool continuing);
// Called only at the proven carried-egg + matching successful level fence.
bool acquire(unsigned color,unsigned level,bool trusted_carry_finish);
bool confirm_owned(unsigned color);
bool save_story_alive(unsigned two_bits);bool restore_life(unsigned color);bool clear_save();
}
