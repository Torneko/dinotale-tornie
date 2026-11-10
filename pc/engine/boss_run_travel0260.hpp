#pragma once
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
namespace workshop::boss_run_travel0260 {
// Pure controller policy. The caller must attest the current native player,
// ordinary idle mouth/aim, and a continuous walkable floor path in Rest83.
// This helper does not read RAM, prove floor geometry, or change game physics.
struct Frame {
 bool enabled=false,diagnostic=false,rest_active=false,controls=false,alive=false,
      grounded=false,path_attested=false,aim_clear=false,mouth_clear=false,
      tongue_clear=false,precision_target=false;
 unsigned source_room=177;
 uint32_t player=0,singleton=0;
 uint16_t buttons=0;
 float x=std::numeric_limits<float>::quiet_NaN(),target_x=x,velocity_x=x,
       world_factor=x,external_x=x;
};
inline constexpr float minimum_distance=192.f,min_x=40.f,max_x=724.f;
inline std::optional<float> axis(const Frame& f){
 if(!f.enabled||!f.diagnostic||!f.rest_active||f.source_room!=83||!f.controls||!f.alive||
    !f.grounded||!f.path_attested||!f.aim_clear||!f.mouth_clear||!f.tongue_clear||
    f.precision_target||f.buttons||f.player!=f.singleton||f.player<0x80000400u||
    f.player>0x807FFE04u||(f.player&3u))return {};
 if(!std::isfinite(f.x)||!std::isfinite(f.target_x)||!std::isfinite(f.velocity_x)||
    !std::isfinite(f.world_factor)||!std::isfinite(f.external_x)||f.x<min_x||f.x>max_x||
    f.target_x<min_x||f.target_x>max_x||f.world_factor<=0||f.world_factor>1||
    f.external_x!=0||std::abs(f.velocity_x)>4.01f)return {};
 const float delta=f.target_x-f.x;
 if(std::abs(delta)<minimum_distance)return {};
 // Let the existing walking/braking controller settle a motion away from the
 // target before admitting a faster course. Unknown forces never grant it.
 const float direction=delta>0?1.f:-1.f;
 if(direction*f.velocity_x<0)return {};
 // Native filtered horizontal stick >=53 for20 updates selects running.
 // Host +/-1 reaches that range through the unchanged native input filter.
 // Returning to the ordinary walk192px before the goal leaves room for its
 // native deceleration. No speed, position, timer, button or vertical axis is
 // written here; nearby food, pots and checkpoints keep their fine controls.
 return direction;
}
} // namespace workshop::boss_run_travel0260
