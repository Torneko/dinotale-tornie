#pragma once
#include "workshop_data.hpp"
#include "fruit_mix.hpp"
#include "recomp.h"
#include <map>
#include <optional>
#include <cstdio>
namespace workshop::black_flyguy {
using Key=std::array<uint32_t,4>;
struct Binding{unsigned room;std::optional<uint8_t> visibility;uint32_t collision=0;int32_t inhibition=0;bool announced=false;};
inline std::set<Key> placements;
inline std::map<uint32_t,Binding> controllers;
inline std::map<uint32_t,uint32_t> children;
inline unsigned current_room=177;
inline uint32_t packed(uint32_t p){return p>>16==0x45E3?0x45E50000|(p&65535):p;}
inline void configure(std::span<const uint8_t> payload){placements.clear();controllers.clear();children.clear();current_room=177;if(payload.empty())return;for(const auto& room:payload_rooms(payload))for(unsigned i=0;i<room.count;i++){auto p=room.offset+i*room.stride;if(u16(payload,p)==0x45E3)placements.insert({room.source,packed(u32(payload,p)),u32(payload,p+4),u32(payload,p+8)});}}
inline void enter_room(unsigned room){current_room=room&255;controllers.clear();children.clear();}
inline void created(recomp_context* c,unsigned room){const auto a=uint32_t(c->r4);controllers.erase(a);children.erase(a);if(placements.contains({room&255,uint32_t(c->r5),uint32_t(c->r6),uint32_t(c->r7)})){controllers.emplace(a,Binding{room&255});fprintf(stderr,"[BlackFlyGuy] controller room=%u actor=%08X native=45E5 explicit=45E3\n",room&255,a);}}
inline bool bound(uint8_t* rdram,uint32_t a){return controllers.contains(a) && controllers.at(a).room==current_room && fruit_mix::range(a,0x1F8) && uint32_t(MEM_W(0,int32_t(a)))==0x19971110 && MEM_HU(12,int32_t(a))==0x45E5;}
// Both fields belong to this live controller. Keep sprite type, native collision
// object and its configuration intact. The native collision loop decrements
// +14 and skips positive values; its explicit callback is also guarded.
inline void prepare(uint8_t* rdram,uint32_t actor){if(!bound(rdram,actor))return;auto& b=controllers.at(actor);const auto a=gpr(int32_t(actor));if(!b.visibility)b.visibility=MEM_BU(0x165,a);MEM_B(0x165,a)=1;const auto c=uint32_t(MEM_W(0x174,a));if(fruit_mix::range(c,0x18)){if(!b.collision){b.collision=c;b.inhibition=MEM_W(0x14,int32_t(c));}MEM_W(0x14,int32_t(c))=0x7FFFFFFF;}if(!b.announced && b.collision){b.announced=true;fprintf(stderr,"[BlackFlyGuy] hidden actor=%08X visibility=1 collision=%08X inhibit=7FFFFFFF type=%u\n",actor,c,unsigned(MEM_BU(0x164,a)));}}
inline bool skip_collision(uint8_t* rdram,uint32_t actor){return bound(rdram,actor);}
inline void attached(uint8_t* rdram,recomp_context* c){const auto parent=uint32_t(c->r4),child=uint32_t(c->r5);if(bound(rdram,parent) && fruit_mix::range(child,0x1F8) && MEM_HU(12,int32_t(child))==0x45E3){children[child]=parent;fprintf(stderr,"[BlackFlyGuy] attached parent=%08X child=%08X nativeParentLink=80047F9C\n",parent,child);}}
// Logical bindings survive graphics-only cleanup. Restore native fields at
// complete actor cleanup, while collision state still belongs to the actor.
inline void forget(uint32_t actor){controllers.erase(actor);for(auto c=children.begin();c!=children.end();)if(c->first==actor || c->second==actor)c=children.erase(c);else ++c;}
inline void destroy(uint8_t* rdram,uint32_t actor){auto it=controllers.find(actor);if(it!=controllers.end() && bound(rdram,actor)){const auto a=gpr(int32_t(actor));if(it->second.visibility)MEM_B(0x165,a)=*it->second.visibility;if(it->second.collision && uint32_t(MEM_W(0x174,a))==it->second.collision && fruit_mix::range(it->second.collision,0x18))MEM_W(0x14,int32_t(it->second.collision))=it->second.inhibition;}forget(actor);}
}
