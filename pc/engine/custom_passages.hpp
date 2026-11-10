#pragma once
#include "workshop_data.hpp"
#include "fruit_mix.hpp"
#include "recomp.h"
#include <cstdio>
#include <optional>
#include <map>
extern "C" void func_8005D29C(uint8_t*,recomp_context*);
extern "C" void func_8002BEB0(uint8_t*,recomp_context*);
extern "C" void func_800443EC(uint8_t*,recomp_context*);
extern "C" void func_80040C0C(uint8_t*,recomp_context*);
extern "C" void func_80047AB8(uint8_t*,recomp_context*);
namespace workshop::passages {
// The unmodified game allocator stops at 80400000 (80064BAC). The 64 KiB
// expansion-RAM reservation is disjoint from all 807F workshop buffers.
// Stable per-project slots are never recycled when a room is revisited.
inline constexpr uint32_t slab=0x807E0000,slab_end=0x807F0000,slot_bytes=40,max_slots=1490;
static_assert(uint64_t(slot_bytes)*max_slots<=slab_end-slab);
using Key=std::array<uint32_t,4>;
struct Binding{unsigned plan;uint32_t original=0,private_data=0;};
enum class Phase{departed,entered,ready};
struct Pending{uint64_t epoch;unsigned plan;uint32_t player;Phase phase=Phase::departed;uint32_t level_info=0;unsigned health=0,fruits=0,children=0;uint32_t child_signature=0;};
inline std::vector<navigation::Plan> routes;
inline std::map<Key,unsigned> route_keys;
inline std::map<uint32_t,Binding> bindings;
inline std::optional<Pending> pending;
inline unsigned current_room=177;
inline uint64_t next_epoch=0,completed_epoch=0;
inline bool restoring=false;
inline uint32_t slot(unsigned index){return slab+index*slot_bytes;}
inline bool range(uint32_t p,size_t bytes){return fruit_mix::range(p,bytes);}
inline uint32_t read_word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,int32_t(p)));}
inline bool actor(uint8_t* rdram,uint32_t p,uint16_t kind=0){return range(p,0x1FC) && read_word(rdram,p)==0x19971110 && (!kind || MEM_HU(12,int32_t(p))==kind);}
inline uint32_t child_signature(uint8_t* rdram,uint32_t player){
    if(!actor(rdram,player,0x4001))return 0;
    const unsigned count=MEM_HU(0x184,int32_t(player));if(count>32 || !range(player+0x188,count*4))return 0;
    uint32_t hash=2166136261u;auto mix=[&](uint32_t value){hash=(hash^value)*16777619u;};mix(count);
    const gpr base=gpr(int32_t(player));
    for(unsigned n=0;n<count;n++){uint32_t child=uint32_t(MEM_W(0x188+n*4,base));mix(child);if(actor(rdram,child)){mix(MEM_HU(12,int32_t(child)));mix(read_word(rdram,child+0x180));mix(MEM_BU(0x17C,int32_t(child)));}}
    return hash;
}
inline void configure(std::span<const uint8_t> original,std::span<const uint8_t> payload){
    routes.clear();route_keys.clear();bindings.clear();pending.reset();current_room=177;next_epoch=completed_epoch=0;restoring=false;
    if(payload.empty())return;
    routes=navigation::plans(original,payload,payload_rooms(payload));
    require(routes.size()<=max_slots,"Too many custom passage instances");
    for(unsigned n=0;n<routes.size();n++){const auto& r=routes[n];auto [it,added]=route_keys.emplace(Key{r.room,r.packed,r.x,r.y},n);require(added || (routes[it->second].destination==r.destination && routes[it->second].arrival==r.arrival),"Conflicting passage bindings");}
}
inline void cancel(const char* reason){
    if(pending)fprintf(stderr,"[CustomPassage] cancelled epoch=%llu reason=%s room=%u\n",static_cast<unsigned long long>(pending->epoch),reason,current_room);
    pending.reset();
}
inline void enter_room(unsigned room){
    current_room=room&255;
    if(!pending || restoring)return;
    if(current_room!=routes[pending->plan].destination){cancel("different-room");return;}
    if(pending->phase==Phase::departed)pending->phase=Phase::entered;
    else cancel("repeated-room-load");
}
inline void created(recomp_context* c,unsigned room){
    if(routes.empty())return;
    const auto a=uint32_t(c->r4);bindings.erase(a);
    const auto found=route_keys.find({room&255,uint32_t(c->r5),uint32_t(c->r6),uint32_t(c->r7)});
    if(found!=route_keys.end())bindings.emplace(a,Binding{found->second});
}
inline void prepare(uint8_t* rdram,uint32_t a){
    auto found=bindings.find(a);if(found==bindings.end())return;
    auto& b=found->second;const auto& r=routes[b.plan];
    if(current_room!=r.room || !actor(rdram,a,uint16_t(r.packed>>16)))return;
    const auto data=read_word(rdram,a+0x1F4);if(!range(data,0x264))return;
    const auto original=read_word(rdram,data+0x260),address=slot(b.plan);
    if(original==address)return;
    if(!range(original,24))return;
    const gpr original_base=gpr(int32_t(original)),copy_base=gpr(int32_t(address));
    for(unsigned n=0;n<24;n++)if(MEM_BU(n,original_base)!=r.native[n])return;
    const auto origin=navigation::camera_origin(r.arrival);
    for(unsigned n=0;n<24;n++)MEM_B(n,copy_base)=r.native[n];
    MEM_W(0,int32_t(address))=-1;
    MEM_B(4,int32_t(address))=uint8_t(r.destination);
    MEM_W(8,int32_t(address))=origin[0];MEM_W(12,int32_t(address))=origin[1];
    b.original=original;b.private_data=data;
    MEM_W(0x260,int32_t(data))=address;
    fprintf(stderr,"[CustomPassage] bound room=%u index=%u actor=%08X private=%08X native=%08X copy=%08X dest=%u xy=(%.3f,%.3f) camera=(%.3f,%.3f)\n",r.room,r.index,a,data,original,address,r.destination,std::bit_cast<float>(r.arrival[0]),std::bit_cast<float>(r.arrival[1]),std::bit_cast<float>(origin[0]),std::bit_cast<float>(origin[1]));
}
inline void forget(uint32_t a){bindings.erase(a);}
inline void destroy(uint8_t* rdram,uint32_t a){
    auto found=bindings.find(a);if(found==bindings.end())return;
    const auto& b=found->second;
    if(b.original && actor(rdram,a) && read_word(rdram,a+0x1F4)==b.private_data && range(b.private_data,0x264) && read_word(rdram,b.private_data+0x260)==slot(b.plan))MEM_W(0x260,int32_t(b.private_data))=b.original;
    forget(a);
}
inline void departing(uint8_t* rdram,recomp_context* c){
    if(routes.empty() || restoring)return;
    const uint32_t address=uint32_t(c->r4);
    if(address<slab || address>=slab_end || (address-slab)%slot_bytes){cancel("native-route");return;}
    const unsigned n=(address-slab)/slot_bytes;
    if(n>=routes.size() || routes[n].room!=current_room){cancel("stale-route");return;}
    bool live=false;
    for(const auto& [a,b]:bindings)if(b.plan==n && b.private_data && actor(rdram,a) && read_word(rdram,a+0x1F4)==b.private_data && read_word(rdram,b.private_data+0x260)==address){live=true;break;}
    const uint32_t player=read_word(rdram,0x800F8828);
    if(!live || !actor(rdram,player,0x4001)){cancel("invalid-source");return;}
    if(pending && pending->plan==n && pending->phase==Phase::departed)return;
    pending=Pending{++next_epoch,n,player,Phase::departed,0,MEM_HU(0,int32_t(0x800F897E)),read_word(rdram,0x800F89D0),MEM_HU(0x184,int32_t(player)),child_signature(rdram,player)};
    fprintf(stderr,"[CustomPassage] depart epoch=%llu room=%u index=%u target=%u player=%08X route=%08X health=%u fruits=%u\n",static_cast<unsigned long long>(pending->epoch),current_room,routes[n].index,routes[n].destination,player,address,pending->health,pending->fruits);
}
inline void room_ready(uint8_t* rdram,recomp_context* c){
    if(!pending || pending->phase!=Phase::entered || restoring)return;
    const auto p=uint32_t(c->r4);
    if(range(p,0x128) && MEM_BU(1,int32_t(p))==routes[pending->plan].destination){pending->phase=Phase::ready;pending->level_info=p;}
}
inline void frame(uint8_t* rdram,recomp_context* c){
    if(!pending || pending->phase!=Phase::ready || restoring)return;
    const auto work=*pending;const auto& route=routes[work.plan];
    if(current_room!=route.destination || !range(work.level_info,0x128) || MEM_BU(1,int32_t(work.level_info))!=route.destination){cancel("room-not-ready");return;}
    const auto queue=read_word(rdram,0x800F8770);
    if(!range(queue,0x84) || !actor(rdram,work.player,0x4001)){cancel("invalid-carry-context");return;}
    // Remove the epoch before native calls: their traced helpers must never
    // re-enter this restoration or consume the same carry list twice.
    pending.reset();restoring=true;const auto saved=*c;
    unsigned count=0;uint32_t restored=0;
    for(;count<150;count++){
        *c=saved;c->r4=gpr(int32_t(queue));func_8005D29C(rdram,c);
        const uint32_t item=uint32_t(c->r2);if(!item)break;
        if(!actor(rdram,item))break;
        if(MEM_HU(12,int32_t(item))!=0x4001)continue;
        if(item!=work.player || restored)break;
        *c=saved;c->r4=gpr(int32_t(item));func_8002BEB0(rdram,c);
        // Native warm exit 805ABBE8: first register the actor for updates,
        // then place it, resume controls, expose and update its world matrix.
        *c=saved;c->r4=gpr(int32_t(item));func_800443EC(rdram,c);
        MEM_W(0x3C,int32_t(item))=route.arrival[0];MEM_W(0x40,int32_t(item))=route.arrival[1];
        const float native_z=std::bit_cast<float>(u32(route.native,16));
        MEM_W(0x44,int32_t(item))=native_z<500.5f?int32_t(0x43FA2B85):int32_t(0x43FA547B);
        *c=saved;c->r4=gpr(int32_t(item));func_80040C0C(rdram,c);
        MEM_B(0x165,int32_t(item))=0;
        *c=saved;c->r4=gpr(int32_t(item+0x84));c->r5=gpr(int32_t(item));func_80047AB8(rdram,c);
        MEM_W(0x130,int32_t(item))=int32_t(0x800233B4);restored=item;
    }
    *c=saved;restoring=false;
    if(restored){completed_epoch=work.epoch;
        fprintf(stderr,"[CustomPassage] arrive epoch=%llu room=%u player=%08X samePlayer=%u carried=%u xy=(%.3f,%.3f) world=(%.3f,%.3f) health=%u->%u fruits=%u->%u visibility=%u callback=%08X camera=(%.3f,%.3f) children=%u->%u childSignature=%08X->%08X parent=%08X\n",static_cast<unsigned long long>(work.epoch),route.destination,restored,unsigned(restored==work.player),count,std::bit_cast<float>(route.arrival[0]),std::bit_cast<float>(route.arrival[1]),std::bit_cast<float>(read_word(rdram,restored+0x84)),std::bit_cast<float>(read_word(rdram,restored+0x88)),work.health,unsigned(MEM_HU(0,int32_t(0x800F897E))),work.fruits,read_word(rdram,0x800F89D0),unsigned(MEM_BU(0x165,int32_t(restored))),read_word(rdram,restored+0x130),std::bit_cast<float>(read_word(rdram,work.level_info+0x11C)),std::bit_cast<float>(read_word(rdram,work.level_info+0x120)),work.children,unsigned(MEM_HU(0x184,int32_t(restored))),work.child_signature,child_signature(rdram,restored),read_word(rdram,restored+0x180));
    }else fprintf(stderr,"[CustomPassage] failed epoch=%llu reason=missing-native-carried-player room=%u carried=%u\n",static_cast<unsigned long long>(work.epoch),current_room,count);
}
}
