#pragma once
#include "boss_native.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <optional>
namespace workshop::boss_fish_purple0260 {
namespace n=boss_native;
struct Frame {bool enabled=false,purple=false,combat_active=false;unsigned room=177;uint32_t actor=0,private_data=0;uint64_t birth=0;};
inline bool pointer(uint32_t p,unsigned size){return p%4==0&&n::range(p,size);}
inline constexpr std::array<uint32_t,19> updates{0,0,0,0x805C8240,0x805C82C0,0x805C836C,0x805C8904,0x805C8988,0x805C8A44,0x805C8D30,0x805C8E20,0x805C9080,0x805C9154,0,0,0x805C954C,0,0,0};
struct Basin {uint32_t descriptor=0;float left=0,right=0;};
inline std::optional<Basin> basin(uint8_t* rdram){
 if(!rdram)return {};auto p=n::word(rdram,0x800A60D0);if(!n::actor(rdram,p,0x80A5))return {};
 for(unsigned off:{0x16Cu,0x1B0u,0x20u}){if(!pointer(p,off+4))return {};p=n::word(rdram,p+off);}
 if(!pointer(p,0x28)||MEM_HU(0x12,n::base(p))!=3||MEM_HU(0x14,n::base(p))!=2||MEM_HU(0x16,n::base(p))!=96)return {};
 const auto pages=n::word(rdram,p+0x18),data=n::word(rdram,p+0x24);if(!n::range(pages,6)||!pointer(data,3072))return {};
 for(unsigned i=0;i<6;++i)if(n::byte(rdram,pages+i)>=6)return {};
 // The cropped native Fish geometry is three256px pages wide with a16px
 // outer tile border. Stay strictly inside it so native6A70 can find water.
 return Basin{p,16.f,3.f*256.f-16.f};
}
inline bool admitted(uint8_t* rdram,const Frame& f){
 if(!rdram||!f.enabled||!f.purple||!f.combat_active||f.room!=97||!f.birth||!pointer(f.actor,0x1FC)||n::byte(rdram,0x800F7EA5)||n::byte(rdram,0x800F7EA6)!=1)return false;
 const auto kind=uint16_t(MEM_HU(12,n::base(f.actor)));if((kind!=0x4511&&kind!=0x4631)||!n::actor(rdram,f.actor,kind)||n::byte(rdram,f.actor+0x17D)!=2||n::byte(rdram,f.actor+0x18)!=1)return false;
 const auto sub=n::byte(rdram,f.actor+0x30);if(sub>=updates.size()||!updates[sub])return false;
 if(!pointer(f.private_data,0x188)||n::word(rdram,f.actor+0x1F4)!=f.private_data||n::word(rdram,f.private_data+0x40))return false;
 const auto resource=n::word(rdram,f.actor+0x168);if(!pointer(resource,0x48)||MEM_HU(0x3C,n::base(resource))!=kind)return false;
 const auto factory=n::word(rdram,resource+0x40);return pointer(factory,0x3FF0)&&n::word(rdram,resource+0x44)==factory+0x35A8&&n::word(rdram,f.actor+0x20)==factory+(updates[sub]-0x805C64F0);
}
// Native9154 returns a free jump to1/1 at91E4. The constructor and
// passenger return paths must never alternate a purple attack. No RAM writes.
inline bool completes_jump(uint8_t* rdram,const recomp_context* c,const Frame& f){
 if(!c||uint32_t(c->r4)!=f.actor||uint32_t(c->r5)!=1||uint32_t(c->r6)!=1||!admitted(rdram,f)||n::byte(rdram,f.actor+0x30)!=12)return false;
 const auto resource=n::word(rdram,f.actor+0x168),factory=n::word(rdram,resource+0x40);
 return uint32_t(c->r31)==factory+(0x805C91EC-0x805C64F0);
}
// Exact next speed of native48F84(actor+C4,0), without changing RAM.
inline float next_speed(float velocity,float target,float step){return velocity<target?std::min(target,velocity+step):velocity>target?std::max(target,velocity-step):velocity;}
inline bool before_axis(uint8_t* rdram,const recomp_context* c,const Frame& f){
 if(!c||uint32_t(c->r4)!=f.actor+0xC4||uint32_t(c->r5)!=0||!admitted(rdram,f))return false;
 const auto map=basin(rdram);if(!map)return false;
 auto scalar=[&](unsigned off){return std::bit_cast<float>(n::word(rdram,f.actor+off));};
 const float x=scalar(0x84),velocity=scalar(0xC4),target=scalar(0xDC),step=scalar(0xE8);
 if(!std::isfinite(x)||!std::isfinite(velocity)||!std::isfinite(target)||!std::isfinite(step)||step<0||x<0||x>=map->right+16)return false;
 const float next=next_speed(velocity,target,step),projected=x+next;if(!std::isfinite(next)||!std::isfinite(projected)||std::abs(next)>=map->right-map->left)return false;
 const bool left=projected<map->left,right=projected>=map->right;if(!left&&!right)return false;
 const float sign=left?1.f:-1.f;
 // Reflect only the native horizontal speed/target. No position, vertical
 // channel, acceleration, animation, passenger or native state is written.
 MEM_W(0xC4,n::base(f.actor))=std::bit_cast<uint32_t>(sign*std::abs(velocity));
 MEM_W(0xDC,n::base(f.actor))=std::bit_cast<uint32_t>(sign*std::abs(target));return true;
}
// Some native jump branches do not call48F84. Their movement still reaches
// this original actor130 callback every frame (9B08 installs483B8).
inline bool before_integrate(uint8_t* rdram,const recomp_context* c,const Frame& f){
 if(!c||uint32_t(c->r4)!=f.actor||!admitted(rdram,f)||n::word(rdram,f.actor+0x130)!=0x800483B8||n::word(rdram,f.actor+0x180))return false;
 const auto map=basin(rdram);if(!map)return false;
 auto scalar=[&](unsigned off){return std::bit_cast<float>(n::word(rdram,f.actor+off));};
 const float x=scalar(0x3C),velocity=scalar(0xC4),target=scalar(0xDC),factor=scalar(0x100),external=scalar(0x10C);
 if(!std::isfinite(x)||!std::isfinite(velocity)||!std::isfinite(target)||!std::isfinite(factor)||factor<=0||external!=0||x<0||x>=map->right+16)return false;
 // Exact483B8 delta: speedC4*factor100+external10C. No interpretation of
 // a foreign external force is attempted. Respect its native side blockers.
 const float delta=velocity*factor+external,projected=x+delta;if(!std::isfinite(delta)||!std::isfinite(projected)||std::abs(delta)>=map->right-map->left)return false;
 const auto metadata=n::word(rdram,f.actor+0x174);if(metadata){if(!pointer(metadata,12))return false;const auto sides=n::word(rdram,metadata+8);if((delta<0&&(sides&0xF0))||(delta>0&&(sides&0xF)))return false;}
 const bool left=projected<map->left,right=projected>=map->right;if(!left&&!right)return false;
 const float sign=left?1.f:-1.f;
 MEM_W(0xC4,n::base(f.actor))=std::bit_cast<uint32_t>(sign*std::abs(velocity));
 MEM_W(0xDC,n::base(f.actor))=std::bit_cast<uint32_t>(sign*std::abs(target));return true;
}

}
