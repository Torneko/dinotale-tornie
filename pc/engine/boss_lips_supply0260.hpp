#pragma once
#include "boss_driver0260.hpp"
#include "boss_native.hpp"
#include <bit>
#include <optional>

namespace workshop::boss_lips_supply0260 {
namespace n=boss_native;namespace d=boss_driver0260;
inline constexpr uint32_t original_factory=0x805E0C90;
inline constexpr unsigned overlay_section=160,initializer_offset=0x78;
inline constexpr bool kind(uint16_t value){return value==0x4539||value==0x453A;}
inline bool pointer(uint32_t address,unsigned bytes){return !(address&3)&&n::range(address,bytes);}
// Read-only native food observation. The diagnostic caller supplies the live
// queue of an owned Lips encounter; no actor is created, relabelled, moved or
// made edible here. Native1208 repeatedly creates4539/453A with20200 flags.
// 0D08 initializes them without private storage.11E0 selects state1/1000 or
// state2/10B4; the state3/11B8 retirement is never offered as food.
inline std::optional<d::Supply> observe(uint8_t* ram,uint32_t actor){
 auto* rdram=ram;
 if(!ram||!pointer(actor,0x1FC))return {};
 const auto native_kind=uint16_t(MEM_HU(12,n::base(actor)));
 if(!kind(native_kind)||!n::actor(ram,actor,native_kind)||n::byte(ram,actor+6)!=2||n::byte(ram,actor+0x17D)!=2||n::byte(ram,actor+0x165)||n::word(ram,actor+0x180)||n::word(ram,actor+0x1F4)||n::byte(ram,actor+0x30))return {};
 const auto phase=n::byte(ram,actor+0x18);if(phase!=1&&phase!=2)return {};
 const auto resource=n::word(ram,actor+0x168),metadata=n::word(ram,actor+0x174);
 if(!pointer(resource,0x48)||!pointer(metadata,4)||MEM_HU(0x3C,n::base(resource))!=native_kind)return {};
 const auto factory=n::word(ram,resource+0x40),definition=n::word(ram,metadata);
 if(!pointer(factory,0x680)||!pointer(definition,0x7C)||n::word(ram,resource+0x44)!=factory+initializer_offset||n::word(ram,actor+0x20)!=factory+(phase==1?0x370u:0x424u))return {};
 // Their original definitions have no collision callback, zero offsets and
 // half-extents15/15/10. The native egg-yield capability is read, never set.
 const auto flags=uint16_t(MEM_HU(0x1C,n::base(definition)));
 if(!(flags&1)||!n::byte(ram,definition+0x74)||n::word(ram,definition+0x28)||n::word(ram,definition+4)||MEM_H(8,n::base(definition))||MEM_H(0xA,n::base(definition))!=15||MEM_H(0xC,n::base(definition))!=15||MEM_H(0xE,n::base(definition))!=10)return {};
 const auto scales=n::word(ram,definition+0x70);if(!pointer(scales,12))return {};
 for(unsigned axis=0;axis<3;++axis){const float scale=std::bit_cast<float>(n::word(ram,scales+axis*4));if(!std::isfinite(scale)||scale<=0||scale>4)return {};}
 const float x=std::bit_cast<float>(n::word(ram,actor+0x84)),y=std::bit_cast<float>(n::word(ram,actor+0x88));
 if(!d::point(x,y)||!std::isfinite(std::bit_cast<float>(n::word(ram,actor+0x8C))))return {};
 d::Supply supply{actor,native_kind,x,y,true,false,flags};
 // Literal nativeB470 selects one nibble of definition18 per native color.
 // All eight nibbles are5 for these original pots; real lifeTable[5]=+2.
 // The egg-yield flag alone never licenses a healing request. Other edible
 // profiles remain available for ordinary empty-stock ammunition forage.
 if(n::word(ram,definition+0x18)==0x55555555&&n::word(ram,0x8009DF84+5*4)==2)supply.healing_gain=2;
 return supply;
}
} // namespace workshop::boss_lips_supply0260
