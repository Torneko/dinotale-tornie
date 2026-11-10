#pragma once
#include "boss_native.hpp"
#include <bit>
#include <cmath>
namespace workshop::boss_fish_up_pose0260 {
// Native5E620 mirrors the sprite before applying WORLD90/94/98 rotations.
// 6D0C..6D78 places its mouth along local-X. With the original native Z
// matrix662B8 and screenY inversion5E6E0, angle90 points it down;270 points
// it up. A vertical sprite mirror reverses this sign, an X mirror does not.
// Read the exact selected frame flags used by5E700..5E764; never mirror the
// sprite, change localAI50, or write a movement/collision field.
inline std::optional<uint32_t> world_z(uint8_t* rdram,uint32_t actor,uint32_t drawing){
 if(!rdram||!boss_native::actor(rdram,actor,0x4511)||drawing%4||!boss_native::range(drawing,0x38))return {};
 const auto frame=boss_native::word(rdram,actor+0x138),flags=boss_native::word(rdram,drawing);
 if(frame>1||flags%4||!boss_native::range(flags,8))return {};
 for(unsigned off:{0x90u,0x94u}){const float angle=std::bit_cast<float>(boss_native::word(rdram,actor+off));if(!std::isfinite(angle)||angle!=0)return {};}
 const auto native_flags=boss_native::word(rdram,flags+frame*4);
 if(native_flags&~3u)return {};
 return std::bit_cast<uint32_t>((native_flags&2u)?90.f:270.f);
}
}
