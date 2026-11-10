#pragma once
#include "boss_native.hpp"
#include <bit>
#include <cmath>
#include <optional>
namespace workshop::boss_lips_aim0260 {
namespace n=boss_native;
struct WeakPoint {uint32_t actor=0,metadata=0,definition=0,resource=0;float x=0,y=0;};
inline bool pointer(uint32_t a,unsigned bytes){return a%4==0&&n::range(a,bytes);}
// Native80558484 creates40A7, stores it at rootPrivate10 (8055852C),
// then attaches it through47F9C (80558604).40A4 is a drawing child, not
// the80557D44 weak-point collider. Observe only this exact reciprocal graph.
// Child private data is legitimately zero and queue phase0 while attached.
inline std::optional<WeakPoint> observe(uint8_t* rdram,uint32_t root,uint32_t private_data,uint64_t birth){
 if(!rdram||!birth||!n::actor(rdram,root,0x40A3)||n::word(rdram,root+0x1F4)!=private_data||!pointer(private_data,0x34)||n::byte(rdram,root+0x17D)!=2||n::byte(rdram,root+0x18)!=1)return {};
 const auto child=n::word(rdram,private_data+0x10);
 if(!n::actor(rdram,child,0x40A7)||n::byte(rdram,child+0x18)==0xFF||n::byte(rdram,child+0x17D)==3||n::word(rdram,child+0x180)!=root)return {};
 const auto slot=MEM_HU(0x17E,n::base(child)),count=MEM_HU(0x184,n::base(root));
 if(slot>=20||count==0||count>20||n::word(rdram,root+0x188+4*slot)!=child)return {};
 unsigned matches=0;for(unsigned i=0;i<20;++i)if(n::word(rdram,root+0x188+4*i)==child)++matches;if(matches!=1)return {};
 const auto root_resource=n::word(rdram,root+0x168),resource=n::word(rdram,child+0x168),metadata=n::word(rdram,child+0x174);
 if(!pointer(root_resource,0x44)||!pointer(resource,0x44)||!pointer(metadata,4))return {};
 const auto factory=n::word(rdram,root_resource+0x40),definition=n::word(rdram,metadata);
 //40A7 uses a shared drawing factory, so it must NOT equal the40A3 factory.
 // Its actual collision callback80557D44 is relative to root factory80557B50.
 if(!pointer(factory,0x1F8)||!pointer(definition,0x2C)||n::word(rdram,definition+0x28)!=factory+0x1F4||n::word(rdram,definition+4)!=0||MEM_H(8,n::base(definition))!=0||MEM_H(0xA,n::base(definition))!=20||n::word(rdram,definition+0xC)!=0x0014000A)return {};
 WeakPoint result{child,metadata,definition,resource,std::bit_cast<float>(n::word(rdram,child+0x84)),std::bit_cast<float>(n::word(rdram,child+0x88))};
 if(!std::isfinite(result.x)||!std::isfinite(result.y)||std::abs(result.x)>=100000||std::abs(result.y)>=100000)return {};
 return result;
}
}
