#pragma once
namespace workshop::game_modes0320 {
enum Mode : int { Story=0, Trial=1, Practice=2, HybridTrial=3, BossBattle=4, StoryHybrid=5, StoryHard=6, Bonus=7 };
constexpr bool valid(int mode){return mode>=Story&&mode<=Bonus;}
constexpr bool story(int mode){return mode==Story||mode==StoryHybrid||mode==StoryHard;}
constexpr bool hybrid_content(int mode){return mode==HybridTrial||mode==StoryHybrid;}
constexpr bool hybrid_trial_records(int mode){return mode==HybridTrial;}
// Canonical ownership/life services exclude the independent Hard team.
constexpr bool solo_playable(int mode){return mode==Story||mode==Trial||mode==Practice||mode==HybridTrial||mode==StoryHybrid;}
constexpr int native_choice(int mode){return !valid(mode)?-1:story(mode)?0:mode==Practice?3:1;}
inline constexpr const char8_t* hybrid_save=u8"yoshisstory.n64.us.story-hybrid0320";
inline constexpr const char8_t* hard_save=u8"yoshisstory.n64.us.story-hard0330";
inline constexpr const char8_t* hard_preview_save=u8"yoshisstory.n64.us.story-hard0330-preview";
inline constexpr const char8_t* canonical_save=u8"yoshisstory.n64.us";
}
