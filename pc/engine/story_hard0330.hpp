#pragma once
#include "story_hard_access0330.hpp"
#include <array>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
namespace workshop::story_hard0330 {
inline constexpr unsigned mode=6,base_yoshis=0xFFFF,all_yoshis=0x3FFFF,all_worlds=63,unassigned=255;
inline constexpr uint32_t maximum_runs=1000000;
inline constexpr const char* production_filename="pc-story-hard.json";
inline constexpr const char* preview_filename="pc-story-hard-preview.json";
enum class Scope {Production,Preview};
enum class Phase {Empty,Active,Lost,Won,Abandoned};
// Production accepts only the confirmed vanilla-like survivor rule.
enum class CompletionPolicy {Undecided,SurvivingRecruits,AllSixWorlds}; // Last value is rejected.
using Checkpoint=std::array<uint8_t,14>;
struct Run {
 std::string token;
 uint64_t serial=0;
 Phase phase=Phase::Empty;
 unsigned owned=0,found=0,alive=0,ever_lost=0,worlds=0,claimed_levels=0;
 std::array<unsigned,18> yoshi_worlds{};
 std::array<unsigned,24> egg_choice{};
 std::array<unsigned,6> world_levels{};
 bool checkpoint_bound=false;
 Checkpoint checkpoint{};
 Run(){egg_choice.fill(unassigned);world_levels.fill(unassigned);}
 bool operator==(const Run&)const=default;
};
struct View {
 bool initialized=false,present=false,active=false,bound=false,readable=true,saved=true;
 Scope scope=Scope::Production;
 Run run;
 uint32_t runs_started=0,runs_won=0;
 unsigned completed=0;
 CompletionPolicy completion_policy=CompletionPolicy::Undecided;
 std::array<uint32_t,18> yoshi_wins{};
 bool solo_boss_route_entitled=false;
 std::string reason;
};
// No RAM, EEPROM, wallets, canonical ownership or Boss writer is called here.
// Root proves native events and supplies unique32lowercasehex run tokens.
class Journal final {
 struct Impl;std::unique_ptr<Impl> impl_;
public:
 explicit Journal(std::function<unsigned(unsigned)> choose_index={});~Journal();
 Journal(const Journal&)=delete;Journal&operator=(const Journal&)=delete;
 // Missing state is read only. Scope must match both filename and file header.
 void initialize(const std::filesystem::path& path=production_filename,Scope=Scope::Production);
 View view()const;
 bool prepare(unsigned selected_mode,const story_hard_access0330::Evidence&,bool preview_verified=false);
 bool new_run(const std::string& token,bool trusted_new_run);
 bool bind_checkpoint(const std::string& token,std::span<const uint8_t> continuation,bool trusted_save);
 bool resume_run(const std::string& token,std::span<const uint8_t> continuation,bool native_continuing);
 bool reserve_elemental(unsigned level,unsigned color,const std::string& token,bool trusted_entry);
 std::optional<unsigned> egg_for(unsigned level,const std::string& token,bool trusted_entry);
 bool found_egg(unsigned level,unsigned color,const std::string& token,bool trusted_carry_finish);
 bool lose_yoshi(unsigned color,const std::string& token,bool trusted_death);
 bool restore_yoshi(unsigned color,const std::string& token,bool trusted_rescue);
 bool world_finished(unsigned level,const std::string& token,bool trusted_receipt);
 bool complete_run(const std::string& token,CompletionPolicy,bool trusted_bowser);
 bool abandon_run(const std::string& token,bool trusted_abandon);
 void deactivate();
};
void initialize(const std::filesystem::path& path=production_filename,Scope=Scope::Production);
View view();
bool prepare(unsigned,const story_hard_access0330::Evidence&,bool preview_verified=false);
bool new_run(const std::string&,bool trusted_new_run);
bool bind_checkpoint(const std::string&,std::span<const uint8_t>,bool trusted_save);
bool resume_run(const std::string&,std::span<const uint8_t>,bool native_continuing);
bool reserve_elemental(unsigned,unsigned,const std::string&,bool trusted_entry);
std::optional<unsigned> egg_for(unsigned,const std::string&,bool trusted_entry);
bool found_egg(unsigned,unsigned,const std::string&,bool trusted_carry_finish);
bool lose_yoshi(unsigned,const std::string&,bool trusted_death);
bool restore_yoshi(unsigned,const std::string&,bool trusted_rescue);
bool world_finished(unsigned,const std::string&,bool trusted_receipt);
bool complete_run(const std::string&,CompletionPolicy,bool trusted_bowser);
bool abandon_run(const std::string&,bool trusted_abandon);
void deactivate();
} // namespace workshop::story_hard0330
