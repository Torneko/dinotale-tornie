#pragma once
#include <algorithm>

namespace workshop::boss_checkpoint0260 {
// Numbered Miss Warp characters mark the beginning and the three completed
// groups. Eligibility never changes the route: only a proved wake activates it.
inline constexpr unsigned rest_room=83,last_completed=9;
inline constexpr bool valid(unsigned completed){return completed<=last_completed&&completed%3==0;}
inline constexpr unsigned number(unsigned completed){return valid(completed)?completed/3+1:0;}
inline constexpr unsigned awake_bit(unsigned completed){return valid(completed)?1u<<(number(completed)-1):0;}
inline constexpr unsigned eligible(unsigned completed){return completed<=10?std::min(last_completed,completed/3*3):0;}
inline constexpr bool can_awaken(unsigned completed,unsigned highest,unsigned milestone){
    return completed<10&&valid(highest)&&valid(milestone)&&milestone>highest&&milestone==eligible(completed);
}
inline constexpr unsigned resume_index(unsigned completed){return completed?completed-1:0;}
} // namespace workshop::boss_checkpoint0260
