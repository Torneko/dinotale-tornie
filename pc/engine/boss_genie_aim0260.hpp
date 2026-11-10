#pragma once
#include "boss_native.hpp"
#include <bit>
#include <cmath>
namespace workshop::boss_genie_aim0260 {
namespace n=boss_native;
struct Motion {float x=0,y=0,vx=0,vy=0;};
// D33C installs483B8. Native BD04 updates rootC4/C8 for physical tracking;
// B938/BB3C deform vertex buffers instead. Read the exact per-update motion
// consumed by483B8, including its distinct X100/Y104 scales and additive terms.
inline std::optional<Motion> observe(uint8_t* rdram,uint32_t actor,uint32_t data,uint64_t birth){
 if(!rdram||!birth||!n::actor(rdram,actor,0x4072)||n::byte(rdram,actor+0x17D)!=2||n::byte(rdram,actor+0x18)!=1||!n::range(data,0x28)||n::word(rdram,actor+0x1F4)!=data||n::word(rdram,actor+0x180)||n::word(rdram,actor+0x130)!=0x800483B8)return {};
 const auto resource=n::word(rdram,actor+0x168),metadata=n::word(rdram,actor+0x174);
 if(!n::range(resource,0x48)||!n::range(metadata,4))return {};
 const auto factory=n::word(rdram,resource+0x40),definition=n::word(rdram,metadata);
 if(!n::range(factory,0x2AA0)||!n::range(definition,0x74)||MEM_HU(0x3C,n::base(resource))!=0x4072||n::word(rdram,resource+0x44)!=factory+0x2A9C||n::word(rdram,definition+0x28)!=factory+0x2928||n::word(rdram,definition+0x70)!=actor+0x6C)return {};
 if(MEM_H(4,n::base(definition))||MEM_H(6,n::base(definition))!=-10||MEM_H(8,n::base(definition))||MEM_H(10,n::base(definition))!=24||MEM_H(12,n::base(definition))!=60||MEM_H(14,n::base(definition))!=20)return {};
 const auto fl=[&](uint32_t a){return std::bit_cast<float>(n::word(rdram,a));};
 const float x=fl(actor+0x84),y=fl(actor+0x88),sx=fl(actor+0x6C),sy=fl(actor+0x70),vx=fl(actor+0xC4)*fl(actor+0x100)+fl(actor+0x10C),vy=fl(actor+0xC8)*fl(actor+0x104)+fl(actor+0x110);
 if(!std::isfinite(x)||!std::isfinite(y)||std::abs(x)>100000||std::abs(y)>100000||!std::isfinite(sx)||!std::isfinite(sy)||sx<=0||sy<=0||sx>4||sy>4||!std::isfinite(vx)||!std::isfinite(vy)||std::abs(vx)>16||std::abs(vy)>16)return {};
 return Motion{x,y-10.f*sy,vx,vy};
}
}
