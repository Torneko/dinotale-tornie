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
// Genuine16-color evidence requires the strict schema2 Boss ledger. Schema1
// receipts remain readable at width14; no Gold/Silver victories are invented.
bool entitled(const Evidence& evidence);
// Read-only schema1/schema2 inspection; no migration or file is written here.
Evidence inspect(const story_rights0320::View& canonical,
 const finds::View& hybrids,const metal_yoshis0300::View& metals,
 const std::filesystem::path& boss_file="pc-boss-challenges.json");
} // namespace workshop::story_hard_access0330
