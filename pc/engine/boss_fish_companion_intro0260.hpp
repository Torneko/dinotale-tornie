#pragma once
#include "boss_native.hpp"
#include <array>
#include <string_view>

namespace workshop::boss_fish_companion_intro0260 {
namespace n=boss_native;
// Read-only admission for the existing void Fish update/contact wrappers.
// Only an owned Fish encounter waits for its actual companion introduction;
// the companion's callbacks, animation and native control setter still run.
struct Member {uint32_t actor=0,private_data=0,overlay=0;uint64_t birth=0;unsigned epoch=0;uint16_t kind=0;};
struct Frame {
 bool active=false,fighting=false,fish_encounter=false;unsigned variant=3,count=0,index=10,room=177,source=177,epoch=0;
 uint32_t player=0;std::string_view token;Member fish{},companion{};
};
inline bool pointer(uint32_t p,unsigned size){return !(p&3)&&n::range(p,size);}
inline bool member(uint8_t* ram,const Member& m,unsigned epoch,unsigned private_size,unsigned overlay_size,uint32_t constructor){
 if(!m.birth||m.epoch!=epoch||!pointer(m.actor,0x1FC)||!n::actor(ram,m.actor,m.kind)||n::byte(ram,m.actor+0x17D)!=2||n::byte(ram,m.actor+0x18)!=1||!pointer(m.private_data,private_size)||n::word(ram,m.actor+0x1F4)!=m.private_data||!pointer(m.overlay,overlay_size))return false;
 const auto resource=n::word(ram,m.actor+0x168);
 return pointer(resource,0x48)&&((n::byte(ram,resource+0x3C)<<8)|n::byte(ram,resource+0x3D))==m.kind&&n::word(ram,resource+0x40)==m.overlay&&n::word(ram,resource+0x44)==m.overlay+constructor;
}
inline bool introducing(uint8_t* ram,const Frame& f){
 if(!ram||!f.active||!f.fighting||!f.fish_encounter||f.variant!=2||f.count!=2||f.index<1||f.index>8||f.room!=167+f.index||f.source!=97||f.token.empty()||f.fish.kind!=0x4511||(f.companion.kind!=0x4072&&f.companion.kind!=0x4599))return false;
 if(f.fish.actor==f.companion.actor||f.fish.actor==f.player||f.companion.actor==f.player||f.fish.private_data==f.companion.private_data||!n::actor(ram,f.player,0x4001)||n::word(ram,0x800F8828)!=f.player||n::byte(ram,f.player+0x17D)!=2||n::byte(ram,f.player+0x165))return false;
 const auto primary=n::byte(ram,f.player+0x18);if(!primary||primary==0xA||primary==0xFF||!pointer(n::word(ram,f.player+0x1F4),0x12F0)||n::byte(ram,0x800F7EA5)||n::byte(ram,0x800F7EA6))return false;
 if(!member(ram,f.fish,f.epoch,0x188,0x3FF0,0x35A8)||n::word(ram,f.fish.private_data+0x40))return false;
 const auto& c=f.companion;const bool genie=c.kind==0x4072;
 // The external4599 constructor is AF5F8, not the shared AF580 initializer.
 // Real resources:4072 D33C-baseA8A0=2A9C;4599 F5F8-baseDD40=18B8.
 if(!member(ram,c,f.epoch,genie?0x28u:0x2Cu,genie?0x3200u:0x2040u,genie?0x2A9Cu:0x18B8u)||n::byte(ram,c.actor+0x30)!=0)return false;
 const auto callback=n::word(ram,c.actor+0x20);
 return genie?callback==c.overlay+0x1EB0:(callback==c.overlay+0xDAC||callback==c.overlay+0xCE4);
}
inline constexpr std::array<uint32_t,19> fish_callbacks{0,0x173C,0x1C50,0x1D50,0x1DD0,0x1E7C,0x2414,0x2498,0x2554,0x2840,0x2930,0x2B90,0x2C64,0,0,0x305C,0,0,0};
inline bool suppress(uint8_t* ram,const recomp_context* ctx,std::string_view function,const Frame& f){
 if(!ctx||!introducing(ram,f))return false;
 // AEFDC's P1-contact branch can cancel the intro through AF64C(1,5).
 // Keep every projectile68 and unrelated actor on its original native path.
 if(function=="func_806AEFDC")return f.companion.kind==0x4599&&uint32_t(ctx->r4)==f.companion.actor&&uint32_t(ctx->r5)==f.player&&n::byte(ram,f.player+0x18)!=0x68;
 if((function!="func_805C7C2C"&&function!="func_805C6900")||uint32_t(ctx->r4)!=f.fish.actor)return false;
 const auto sub=n::byte(ram,f.fish.actor+0x30);
 if(sub>=fish_callbacks.size()||!fish_callbacks[sub]||n::word(ram,f.fish.actor+0x20)!=f.fish.overlay+fish_callbacks[sub])return false;
 // A fish already past1/1 keeps its native movement. Its harmful contact is
 // still deferred until the exact intro callbacks end, including native1/2.
 return function=="func_805C7C2C"?sub==1:uint32_t(ctx->r5)==f.player;
}
}
