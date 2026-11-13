#pragma once
#include <cstdint>

namespace workshop::bonus_goal0352 {
inline constexpr uint32_t native_frame_capacity=30;
// The native ring stores thirty entries. Further fruit replaces its last slot;
// it never indexes the ending flag or grows the native ring allocation.
constexpr uint32_t before_collect(uint32_t count,bool active){
    return active&&count>=native_frame_capacity?native_frame_capacity-1:count;
}
constexpr uint32_t after_collect(uint32_t count,bool active){
    return active&&count>native_frame_capacity?native_frame_capacity:count;
}
constexpr bool native_goal_allowed(bool active){return !active;}
} // namespace workshop::bonus_goal0352
