#pragma once
#include "boss_native.hpp"
#include <array>
#include <optional>
#include <span>

namespace workshop::boss_intro0260 {
namespace n=boss_native;
// An explicitly bound Boss Battle introduction may use the original popup
// allocation-failure branch. This observer writes neither RAM nor registers.
// Native intro animation, timers, controls and the real combat setter continue.
struct Member {uint32_t actor=0,private_data=0;uint16_t kind=0;uint32_t family_overlay=0;};
struct Frame {
 bool active=false,fighting=false;unsigned encounter=10,physical_room=177,logical_room=177;
 uint32_t dialog_overlay=0;std::span<const Member> members;
};
struct Request {unsigned member=0;uint32_t actor=0;int32_t text_id=0;};
struct Spec {uint16_t kind;unsigned room;uint32_t rom_base,intro,caller_ra;int32_t text_id;unsigned private_size;};
inline constexpr std::array<Spec,4> specs{{
 {0x455A,162,0x80664CD0,0x806655E4,0x8066562C,-99,0x24},
 {0x4599,164,0x806ADD40,0x806AEAEC,0x806AEBB8,-100,0x2C},
 {0x4072,163,0x8042A8A0,0x8042C750,0x8042C80C,-101,0x28},
 {0x40A3,165,0x80557B50,0x80558B88,0x80558BF4,-102,0x34}
}};
inline bool aligned(uint32_t p,unsigned bytes){return p%4==0&&n::range(p,bytes);}
inline bool source_allows(unsigned room,uint16_t kind){
 // Fish97 companions retain their own exact native intro caller and text.
 return (room==97&&(kind==0x4072||kind==0x4599))||(room==162&&(kind==0x455A||kind==0x4072))||(room==163&&kind==0x4072)||(room==164&&kind==0x4599)||(room==165&&(kind==0x40A3||kind==0x455A||kind==0x4599));
}
inline std::optional<Request> observe_dialog_creation(uint8_t* rdram,const recomp_context* c,const Frame& f){
 if(!rdram||!c||!f.active||!f.fighting||f.encounter>=10||f.physical_room!=(f.encounter?167+f.encounter:162)||f.members.empty()||f.members.size()>4||!aligned(f.dialog_overlay,0x1160))return {};
 // 800492C0 ->80515DF8 ->80046804. Only this specific factory call creates
 // the4088 popup. Its native null return constructs no4024 and changes no
 // scene/control flags. It is not an instruction to retire an existing actor.
 const auto sp=uint32_t(c->r29);
 if(!aligned(sp,0x7C)||uint32_t(c->r31)!=f.dialog_overlay+0x98C||uint32_t(c->r4)!=f.dialog_overlay+0x1130||uint32_t(c->r5)!=0x40882200||n::word(rdram,sp+0x1C)!=0x80049308||n::word(rdram,sp+0x28)!=sp+0x48)return {};
 const auto caller_ra=n::word(rdram,sp+0x3C);const auto text=int32_t(n::word(rdram,sp+0x54));
 std::optional<Request> found;
 for(unsigned member=0;member<f.members.size();++member){const auto& m=f.members[member];
  for(const auto& s:specs){if(m.kind!=s.kind||!source_allows(f.logical_room,m.kind)||text!=s.text_id||!aligned(m.family_overlay,s.caller_ra-s.rom_base+4)||caller_ra!=m.family_overlay+(s.caller_ra-s.rom_base))continue;
   const auto root=m.kind==0x455A?n::word(rdram,sp+0x78):uint32_t(c->r16);
   if(root!=m.actor||!m.actor||!n::actor(rdram,m.actor,m.kind)||n::byte(rdram,m.actor+0x17D)!=2||n::byte(rdram,m.actor+0x18)!=1||!aligned(m.private_data,s.private_size)||n::word(rdram,m.actor+0x1F4)!=m.private_data||n::word(rdram,m.actor+0x20)!=m.family_overlay+(s.intro-s.rom_base))continue;
   if(found)return {}; // Ambiguous/duplicate bindings do not suppress a popup.
   found=Request{member,m.actor,text};
  }
 }
 return found;
}
}
