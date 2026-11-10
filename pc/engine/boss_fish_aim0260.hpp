#pragma once
#include "boss_fish_explosion0260.hpp"
#include "boss_fish_green0260.hpp"
namespace workshop::boss_fish_aim0260 {
namespace n=boss_native;
struct Motion {uint32_t actor=0;uint64_t birth=0;float x=0,y=0,vx=0,vy=0,speed=0;};
// Observation accepts native swimming1/1 and braking1/2 as well as free
// jumps. Green motion overrides deliberately keep their stricter jump table.
inline constexpr auto updates=[](){auto t=boss_fish_green0260::updates;t[1]=0x805C7C2C;t[2]=0x805C8140;return t;}();
inline std::optional<Motion> observe(uint8_t* rdram,uint32_t actor,uint16_t kind,uint64_t birth){
 if(!rdram||!birth||(kind!=0x4511&&kind!=0x4631)||!n::actor(rdram,actor,kind)||n::byte(rdram,actor+0x18)!=1||n::byte(rdram,actor+0x165)||n::word(rdram,actor+0x180)||n::word(rdram,actor+0x130)!=0x800483B8)return {};
 const auto resource=n::word(rdram,actor+0x168),data=n::word(rdram,actor+0x1F4);if(!n::range(resource,0x48)||!n::range(data,0x188)||n::word(rdram,data+0x40))return {};
 const auto factory=n::word(rdram,resource+0x40);const auto sub=n::byte(rdram,actor+0x30);
 if(!n::range(factory,0x3FF0)||MEM_HU(0x3C,n::base(resource))!=kind||n::word(rdram,resource+0x44)!=factory+0x35A8||sub>=updates.size()||!updates[sub]||n::word(rdram,actor+0x20)!=factory+(updates[sub]-0x805C64F0))return {};
 const auto box=boss_fish_explosion0260::observe(rdram,actor,kind);if(!box||box->scales!=actor+0x6C||n::word(rdram,box->definition+0x28)!=factory+0x410)return {};
 for(unsigned axis=0;axis<3;++axis)if(MEM_H(4+2*axis,n::base(box->definition))!=(axis==0?-30:0)||MEM_H(10+2*axis,n::base(box->definition))!=(axis==2?10:24)||std::bit_cast<float>(n::word(rdram,box->scales+4*axis))>4)return {};
 const float sx=std::bit_cast<float>(n::word(rdram,0x800B02D8)),sy=std::bit_cast<float>(n::word(rdram,0x800B02DC));if(sx!=6.6f||sy!=sx)return {};
 Motion out{actor,birth,box->center[0],box->center[1],0,0,sx};
 // Original483B8 integrates velocity*axisFactor+external displacement.
 // These are current native-world units per update, not pixels per VI.
 for(unsigned axis=0;axis<2;++axis){const float velocity=std::bit_cast<float>(n::word(rdram,actor+0xC4+4*axis)),factor=std::bit_cast<float>(n::word(rdram,actor+0x100+4*axis)),external=std::bit_cast<float>(n::word(rdram,actor+0x10C+4*axis));if(!std::isfinite(velocity)||!std::isfinite(factor)||factor<=0||factor>4||!std::isfinite(external))return {};const float effective=velocity*factor+external;if(!std::isfinite(effective)||std::abs(effective)>16)return {};if(axis)out.vy=effective;else out.vx=effective;}
 return out;
}
}
