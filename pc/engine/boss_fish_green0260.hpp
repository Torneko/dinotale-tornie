#pragma once
#include "boss_native.hpp"
#include "boss_fish_up_pose0260.hpp"
#include <array>
#include <bit>
#include <cmath>
namespace workshop::boss_fish_green0260 {
namespace n=boss_native;
struct Frame {bool enabled=false,green=false,combat_active=false;unsigned room=177;uint32_t actor=0,private_data=0;uint64_t birth=0;};
struct DrawTicket {bool active=false;Frame frame{};uint32_t resource=0,original=0,replacement=0;};
inline bool pointer(uint32_t p,unsigned size){return p%4==0&&n::range(p,size);}
// Original native state1 table805CA030. Only free jump3..12 and free return15
// are admitted; passenger/intro/death states keep their complete native motion.
inline constexpr std::array<uint32_t,19> updates{0,0,0,0x805C8240,0x805C82C0,0x805C836C,0x805C8904,0x805C8988,0x805C8A44,0x805C8D30,0x805C8E20,0x805C9080,0x805C9154,0,0,0x805C954C,0,0,0};
inline bool admitted(uint8_t* rdram,const Frame& f){
 if(!rdram||!f.enabled||!f.green||!f.combat_active||f.room!=97||!f.birth||!n::actor(rdram,f.actor,0x4511)||n::byte(rdram,f.actor+0x17D)!=2||n::byte(rdram,f.actor+0x18)!=1||n::byte(rdram,0x800F7EA5)||n::byte(rdram,0x800F7EA6)!=1)return false;
 const auto sub=n::byte(rdram,f.actor+0x30);if(sub>=updates.size()||!updates[sub])return false;
 if(!pointer(f.private_data,0x188)||n::word(rdram,f.actor+0x1F4)!=f.private_data||n::word(rdram,f.private_data+0x40))return false;
 const auto resource=n::word(rdram,f.actor+0x168);if(!pointer(resource,0x48)||MEM_HU(0x3C,n::base(resource))!=0x4511)return false;
 const auto factory=n::word(rdram,resource+0x40);if(!pointer(factory,0x3FF0)||n::word(rdram,resource+0x44)!=factory+0x35A8||n::word(rdram,f.actor+0x20)!=factory+(updates[sub]-0x805C64F0))return false;
 for(unsigned offset:{0xC4u,0xDCu,0xE8u})if(!std::isfinite(std::bit_cast<float>(n::word(rdram,f.actor+offset))))return false;
 return true;
}
inline void zero_x(uint8_t* rdram,uint32_t actor){
 // 74C8 writes targetDC/stepE8;48F84(actor+C4,0) changes velocityC4 only.
 // No local/world position, vertical channel, angle, animation or state write.
 for(unsigned offset:{0xC4u,0xDCu,0xE8u})MEM_W(offset,n::base(actor))=0;
}
inline bool before_axis(uint8_t* rdram,const recomp_context* c,const Frame& f){
 if(!c||uint32_t(c->r4)!=f.actor+0xC4||uint32_t(c->r5)!=0||!admitted(rdram,f))return false;
 zero_x(rdram,f.actor);return true;
}
// Native836C sets C8=-5,E0=5,EC=.2 immediately before its74C8 call.
// This exact original return precedes state15. A single launch impulse -5.5
// gives about21% more ballistic height; native Y interpolation stays in charge.
inline bool boost_launch(uint8_t* rdram,const recomp_context* c,const Frame& f){
 if(!c||uint32_t(c->r4)!=f.actor||!admitted(rdram,f)||n::byte(rdram,f.actor+0x30)!=5)return false;
 const auto resource=n::word(rdram,f.actor+0x168),factory=n::word(rdram,resource+0x40);
 if(uint32_t(c->r31)!=factory+0x2378||n::word(rdram,f.actor+0xC8)!=std::bit_cast<uint32_t>(-5.f)||
    n::word(rdram,f.actor+0xE0)!=std::bit_cast<uint32_t>(5.f)||n::word(rdram,f.actor+0xEC)!=std::bit_cast<uint32_t>(.2f))return false;
 MEM_W(0xC8,n::base(f.actor))=std::bit_cast<uint32_t>(-5.5f);return true;
}
inline bool after_target(uint8_t* rdram,const recomp_context* c,const Frame& f){
 if(!c||uint32_t(c->r4)!=f.actor||!admitted(rdram,f))return false;
 zero_x(rdram,f.actor);boost_launch(rdram,c,f);return true;
}
// Type2's original5E620 renderer samples WORLD Z rotation98 at5E7B0.
// The local50 angle also controls native AI70B0 and must stay untouched.
// Scope this ticket around outer5F700, before the native matrix is baked;
// inner6500 palette-only draws must not consume the outer ticket.
inline DrawTicket draw_begin(uint8_t* rdram,const Frame& f){
 if(!admitted(rdram,f)||n::byte(rdram,f.actor+0x164)!=2)return {};
 const auto r=n::word(rdram,f.actor+0x168),factory=n::word(rdram,r+0x40),drawing=n::word(rdram,f.actor+0x16C);
 const auto original=n::word(rdram,f.actor+0x98);if(!pointer(drawing,0x38)||n::word(rdram,drawing+0x34)!=factory+0x10||!std::isfinite(std::bit_cast<float>(original)))return {};
 const auto up=boss_fish_up_pose0260::world_z(rdram,f.actor,drawing);if(!up)return {};
 DrawTicket ticket{true,f,r,original,*up};MEM_W(0x98,n::base(f.actor))=ticket.replacement;return ticket;
}
inline bool draw_end(uint8_t* rdram,const DrawTicket& ticket){
 if(!ticket.active||!admitted(rdram,ticket.frame)||n::word(rdram,ticket.frame.actor+0x168)!=ticket.resource||
    n::word(rdram,ticket.frame.actor+0x98)!=ticket.replacement)return false;
 MEM_W(0x98,n::base(ticket.frame.actor))=ticket.original;return true;
}
}
