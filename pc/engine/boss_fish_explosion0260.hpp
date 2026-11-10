#pragma once
#include "boss_native.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <optional>
namespace workshop::boss_fish_explosion0260 {
namespace n=boss_native;
struct Box {uint32_t actor=0,metadata=0,definition=0,scales=0;std::array<float,3> center{},half{};};
inline bool pointer(uint32_t p,std::size_t bytes){return p%4==0&&n::range(p,bytes);}
// Exact boolean branch of native49018 used by5FEBC to mirror offsetX.
inline std::optional<bool> mirrored(uint8_t* ram,uint32_t actor){
 if(n::byte(ram,actor+0x164)==7)return false;
 const auto drawing=n::word(ram,actor+0x16C);if(!drawing)return false;if(!pointer(drawing,4))return {};
 const auto table=n::word(ram,drawing);if(!table)return false;
 const auto index=n::word(ram,actor+0x138);if(!pointer(table,std::size_t(index)*4+4))return {};
 return bool(n::word(ram,table+index*4)&1);
}
// Read the original current collision definition, never the visual radius.
// Native600CC/60D90: half=LH(def+A/C/E)*float(def70+axis4);
// centerYZ=actor88/8C+LH(def6/8)*scaleYZ;5FEBC mirrors centerX.
inline std::optional<Box> observe(uint8_t* rdram,uint32_t actor,uint16_t kind){
 if(!rdram||(kind!=0x4018&&kind!=0x4511&&kind!=0x4631&&kind!=0x4526)||!n::actor(rdram,actor,kind)||n::byte(rdram,actor+0x17D)!=2||n::byte(rdram,actor+0x18)==0xFF)return {};
 if(kind==0x4018&&n::byte(rdram,actor+0x18)!=0x68)return {};
 const auto metadata=n::word(rdram,actor+0x174);if(!pointer(metadata,4))return {};
 const auto definition=n::word(rdram,metadata);if(!pointer(definition,0x75)||!n::byte(rdram,definition+0x74))return {};
 const auto scales=n::word(rdram,definition+0x70);if(!pointer(scales,12))return {};
 if(kind==0x4018){const auto p=n::word(rdram,actor+0x1F4);if(!pointer(p,0x20)||n::word(rdram,p)!=0||scales!=p+0x14)return {};}
 const auto mirror=mirrored(rdram,actor);if(!mirror)return {};
 Box box{actor,metadata,definition,scales};
 for(unsigned axis=0;axis<3;++axis){const float scale=std::bit_cast<float>(n::word(rdram,scales+4*axis)),position=std::bit_cast<float>(n::word(rdram,actor+0x84+4*axis));const int offset=MEM_H(4+2*axis,n::base(definition)),extent=MEM_H(0xA+2*axis,n::base(definition));
  if(!std::isfinite(scale)||scale<=0||!std::isfinite(position)||extent<=0)return {};
  const float displacement=float(offset)*scale;box.center[axis]=(axis==0&&*mirror)?position-displacement:position+displacement;box.half[axis]=float(extent)*scale;
  if(!std::isfinite(box.center[axis])||!std::isfinite(box.half[axis])||box.half[axis]<=0)return {};
 }
 return box;
}
inline std::optional<bool> overlaps(uint8_t* rdram,uint32_t pulse,uint32_t fish){
 if(!rdram||pulse==fish||!n::range(fish,0x1FC))return {};
 const auto kind=uint16_t(MEM_HU(12,n::base(fish)));if(kind!=0x4511&&kind!=0x4631)return {};
 const auto a=observe(rdram,pulse,0x4018),b=observe(rdram,fish,kind);if(!a||!b)return {};
 // Native6009C uses inclusive comparisons on all three summed half-extents.
 for(unsigned axis=0;axis<3;++axis){const float distance=std::abs(b->center[axis]-a->center[axis]),extent=a->half[axis]+b->half[axis];if(!std::isfinite(distance)||!std::isfinite(extent))return {};if(distance>extent)return false;}
 return true;
}
// The same original6009C volume test, restricted to the native yellow bird.
// Campaign ownership, generation and PRIMARY egg provenance are caller guards.
inline std::optional<bool> overlaps_bird(uint8_t* rdram,uint32_t pulse,uint32_t bird){
 if(!rdram||pulse==bird)return {};
 const auto a=observe(rdram,pulse,0x4018),b=observe(rdram,bird,0x4526);if(!a||!b)return {};
 for(unsigned axis=0;axis<3;++axis){const float distance=std::abs(b->center[axis]-a->center[axis]),extent=a->half[axis]+b->half[axis];if(!std::isfinite(distance)||!std::isfinite(extent))return {};if(distance>extent)return false;}
 return true;
}
}
