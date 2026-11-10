#pragma once
#include <filesystem>
#include <optional>
#include <string>
namespace workshop::story_rights0320 {struct View;}
namespace workshop::finds {struct View;}
namespace workshop::metal_yoshis0300 {struct View;}
namespace workshop::story_hard_access0330 {
inline constexpr unsigned all_yoshis=0xFFFF;
struct Evidence {
 unsigned owned16=0,bossWins16=0,bossWidth=0;
 bool readable=false,saved=false;
 std::string reason;
};
// Only genuinely found Trial colors count: ordinary six, canonical earned
// White/Black, migrated hybrid finds, and the two-bit metallic ownership ledger.
// Purchases, Story alive flags and mere eligibility never grant ownership.
std::optional<unsigned> owned(const story_rights0320::View& canonical,
 const finds::View& hybrids,const metal_yoshis0300::View& metals);
// Pure policy for a future validated16-color native evidence adapter. This
// module has NO16-color file protocol or writer and manufactures no victories.
bool entitled(const Evidence& evidence);
// Strict READ-ONLY legacy schema1 reader: evidence width is0 or14, never16.
// Current production evidence therefore cannot open Story Hard, including a
// complete0x3FFF legacy ledger. Private QA authorization is a separate concern.
Evidence inspect(const story_rights0320::View& canonical,
 const finds::View& hybrids,const metal_yoshis0300::View& metals,
 const std::filesystem::path& boss_file="pc-boss-challenges.json");
} // namespace workshop::story_hard_access0330
