#pragma once
#include "elemental_resources0355.hpp"
#include "recomp.h"
#include <cstdint>
namespace workshop::elemental_tongue0355 {
inline bool range(uint32_t a,uint32_t bytes){return a>=0x80000400u&&a<0x80800000u&&bytes<=0x80800000u-a;}
inline gpr at(uint32_t a){return gpr(int32_t(a));}
inline uint32_t word(uint8_t* rdram,uint32_t a){return uint32_t(MEM_W(0,at(a)));}
inline bool actor(uint8_t* rdram,uint32_t a,unsigned kind){return range(a,0x200)&&word(rdram,a)==0x19971110u&&MEM_HU(12,at(a))==kind&&MEM_BU(0x17D,at(a))!=3&&MEM_BU(0x18,at(a))!=0xFF;}
inline bool child(uint8_t* rdram,uint32_t owner,uint32_t wanted){for(unsigned i=0;i<20;++i)if(word(rdram,owner+0x188+i*4)==wanted)return true;return false;}
// Native27:8003CA5C installs private160(body400B)/15C(tip400C),
// player->body->tip parenting. Verify the complete current graph, not just a
// display color or a recycled actor address. Nothing survives the draw call.
inline uint32_t owner(uint8_t* rdram,uint32_t a){
 if(!rdram||!range(a,0x200))return 0;const auto kind=MEM_HU(12,at(a));if(kind!=0x400B&&kind!=0x400C)return 0;
 const auto body=kind==0x400B?a:word(rdram,a+0x180);if(!actor(rdram,body,0x400B))return 0;const auto player=word(rdram,body+0x180);
 if(!actor(rdram,player,0x4001)||word(rdram,0x800F8828)!=player)return 0;const auto data=word(rdram,player+0x1F4);if(!range(data,0x170)||word(rdram,data+0x160)!=body)return 0;
 const auto tip=word(rdram,data+0x15C);if(!actor(rdram,tip,0x400C)||word(rdram,tip+0x180)!=body||!child(rdram,player,body)||!child(rdram,body,tip)||(kind==0x400C&&a!=tip))return 0;
 return player;
}
inline bool palette(uint8_t* rdram,uint32_t base){
 if(!range(base,1536))return false;uint32_t hash=2166136261u;for(unsigned i=0;i<512;++i)hash=(hash^MEM_BU(i,at(base)))*16777619u;if(hash!=elemental_resources0355::original_tongue_fnv)return false;
 for(unsigned i=0;i<256;++i){const auto c=uint16_t(MEM_HU(i*2,at(base)));if(MEM_HU(512+i*2,at(base))!=elemental_resources0355::tongue_color(c,16)||MEM_HU(1024+i*2,at(base))!=elemental_resources0355::tongue_color(c,17))return false;}return true;
}
struct Ticket {bool active=false;uint32_t actor=0,kind=0,resource=0,drawing=0,private_data=0,original=0,drawing_original=0,replacement=0;};
inline Ticket begin(uint8_t* rdram,uint32_t a,bool solo){
 Ticket t;if(!solo||!rdram)return t;const auto color=word(rdram,0x807F0008),alias=word(rdram,0x800F8DB0);if((color!=16&&color!=17)||alias!=elemental_resources0355::native_alias(color)||!owner(rdram,a))return t;
 const auto resource=word(rdram,a+0x168),drawing=word(rdram,a+0x16C);if(!range(resource,0x90)||!range(drawing,0xC)||MEM_HU(0x3C,at(resource))!=MEM_HU(12,at(a)))return t;
 const auto source=word(rdram,resource+0x88),old_draw=word(rdram,drawing+8);uint32_t base=0;
 // A nested tip draw may see the temporary palette already selected by its
 // body draw. Recover the validated original row without inventing RAM slots.
 for(unsigned n=0;n<3&&!base;++n)if(source>=n*512&&palette(rdram,source-n*512))base=source-n*512;
 if(!base||(old_draw!=base&&old_draw!=base+512&&old_draw!=base+1024))return t;
 const auto replacement=base+(color-15)*512;t={true,a,unsigned(MEM_HU(12,at(a))),resource,drawing,word(rdram,a+0x1F4),source,old_draw,replacement};
 MEM_W(0x88,at(resource))=replacement;MEM_W(8,at(drawing))=replacement;return t;
}
// Caller owns LIFO tickets and invokes this even when native drawing throws.
// A resource/actor rebound during that boundary must never overwrite new state.
inline void finish(uint8_t* rdram,const Ticket& t){
 if(!rdram||!t.active||!actor(rdram,t.actor,t.kind)||word(rdram,t.actor+0x168)!=t.resource||word(rdram,t.actor+0x16C)!=t.drawing||word(rdram,t.actor+0x1F4)!=t.private_data)return;
 if(range(t.resource,0x90)&&word(rdram,t.resource+0x88)==t.replacement)MEM_W(0x88,at(t.resource))=t.original;
 if(range(t.drawing,0xC)&&word(rdram,t.drawing+8)==t.replacement)MEM_W(8,at(t.drawing))=t.drawing_original;
}
} // namespace workshop::elemental_tongue0355
