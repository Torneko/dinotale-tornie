#pragma once
#include "editor_object_data.hpp"
#include "workshop_data.hpp"
#include "fruit_mix.hpp"
#include "recomp.h"
#include <map>
#include <unordered_map>
#include <optional>
#include <cstdio>
extern "C" void func_80067E68(uint8_t*,recomp_context*);
extern "C" void func_80067818(uint8_t*,recomp_context*);

namespace workshop::editor_objects {
using Key=std::array<uint32_t,4>;
struct Source {unsigned room,color;};
struct Egg {unsigned color;bool sounded=false;};
inline std::map<Key,unsigned> placements;
inline std::map<unsigned,uint16_t> aliases;
inline std::unordered_map<uint32_t,Source> sources;
inline std::unordered_map<uint32_t,Source> children;
inline std::unordered_map<uint32_t,Egg> eggs;
inline unsigned current_room=177;
inline uint32_t creating_parent=0;
inline int spawning_color=-1;
inline std::set<uint32_t> painted,unbound_painted;
inline std::set<uint32_t> native_egg_actors;
inline void configure(std::span<const uint8_t> original,std::span<const uint8_t> payload){
    placements.clear();aliases.clear();sources.clear();children.clear();eggs.clear();painted.clear();unbound_painted.clear();native_egg_actors.clear();current_room=177;creating_parent=0;spawning_color=-1;
    if(payload.empty())return;
    for(const auto& room:payload_rooms(payload)){
        const auto header=terrain::bank(original,u32(original,0xA84A0+room.source*8+4));
        const auto meta=terrain::bank(original,u32(original,terrain::bank(original,u32(original,header+0x5C))));
        const auto objects=terrain::bank(original,u32(original,meta+4));uint16_t alias=65535;
        for(unsigned i=0;i<u16(original,meta);i++){auto id=u16(original,objects+i*12);if(native(id))alias=std::min(alias,id);}
        if(alias!=65535)aliases.emplace(room.source,alias);
        for(unsigned i=0;i<room.count;i++){
            const auto p=room.offset+i*room.stride;auto id=u16(payload,p);if(!special(id))continue;
            require(alias!=65535,"No compatible native Yoshi egg bubble in source room");
            for(unsigned j=0;j<room.count;j++){
                const auto q=room.offset+j*room.stride;
                require(!native(u16(payload,q)) || u16(payload,q+2)!=u16(payload,p+2) || u32(payload,q+4)!=u32(payload,p+4) || u32(payload,q+8)!=u32(payload,p+8),"A native and editor egg bubble overlap");
            }
            const Key key{room.source,uint32_t(alias)<<16|u16(payload,p+2),u32(payload,p+4),u32(payload,p+8)};
            auto [it,added]=placements.emplace(key,color(id));require(added || it->second==color(id),"Overlapping editor egg bubbles have different colors");
        }
    }
}
inline uint32_t packed(unsigned room,uint32_t value){if(!special(value>>16))return value;auto a=aliases.find(room);require(a!=aliases.end(),"Missing native editor egg alias");return uint32_t(a->second)<<16|(value&65535);}
inline void enter_room(unsigned room){current_room=room&255;creating_parent=0;}
inline std::optional<unsigned> source_color(uint32_t actor){auto it=sources.find(actor);if(it==sources.end() || it->second.room!=current_room)return {};return it->second.color;}
inline void begin_child(recomp_context* ctx){creating_parent=source_color(uint32_t(ctx->r4)) && uint32_t(ctx->r5)==0x41CE?uint32_t(ctx->r4):0;}
inline void created(uint8_t*,recomp_context* ctx,unsigned room){
    const auto actor=uint32_t(ctx->r4),kind=uint32_t(ctx->r5)>>16;
    sources.erase(actor);children.erase(actor);eggs.erase(actor);painted.erase(actor);unbound_painted.erase(actor);native_egg_actors.erase(actor);
    if(kind==0x4002 || kind==0x401C || kind==0x401D || kind==0x401E)native_egg_actors.insert(actor);
    if(kind==0x401D && spawning_color>=6 && spawning_color<16)eggs[actor]={unsigned(spawning_color),false};
    auto it=placements.find({room&255,uint32_t(ctx->r5),uint32_t(ctx->r6),uint32_t(ctx->r7)});
    if(it!=placements.end()){
        sources.emplace(actor,Source{room&255,it->second});
        fprintf(stderr,"[EditorEgg] bubble room=%u actor=%08X color=%u alias=%04X\n",room&255,actor,it->second,kind);
    }
    if(kind==0x41CE && creating_parent){if(auto c=source_color(creating_parent))children.emplace(actor,Source{current_room,*c});creating_parent=0;}
}
inline std::optional<unsigned> release_color(uint8_t*,recomp_context* ctx){
    // EC8C preserves a3=the exact wrapper through its F2C0 release call.
    // Never infer a custom binding from a neighboring actor's coordinates.
    if(auto it=children.find(uint32_t(ctx->r7));it!=children.end() && it->second.room==current_room)return it->second.color;
    return {};
}
inline void spawned(uint32_t actor,unsigned c){if(actor && c>=6 && c<16)eggs[actor]={c,false};}
inline bool egg_state(uint32_t actor,unsigned){return eggs.contains(actor);}
inline bool pickup_sound(uint8_t* rdram,recomp_context* ctx,uint32_t actor,unsigned state){
    auto it=eggs.find(actor);if(it==eggs.end() || it->second.sounded || (state!=4 && state!=5))return false;
    it->second.sounded=true;const auto saved=*ctx;ctx->r4=6;func_80067E68(rdram,ctx);ctx->r5=ctx->r2;ctx->r4=0x251;func_80067818(rdram,ctx);*ctx=saved;
    fprintf(stderr,"[EditorEgg] picked actor=%08X color=%u sound=0251 canonicalUnlock=0\n",actor,it->second.color);return true;
}
// Resource-cache table used by native 80051100. Its byte count bounds the
// lookup; no arena scan, allocation, preload or changes to shared resources.
inline uint32_t native_egg_resource(uint8_t* rdram){
    const auto table=uint32_t(MEM_W(0,int32_t(0x800F84D4)));
    if(!fruit_mix::range(table,4))return 0;
    const auto count=unsigned(MEM_BU(0,int32_t(table)));
    if(!fruit_mix::range(table+4,count*4))return 0;
    const gpr table_base=static_cast<gpr>(static_cast<int32_t>(table));
    for(unsigned i=0;i<count;i++){
        const auto resource=uint32_t(MEM_W(4+i*4,table_base));
        if(!fruit_mix::range(resource,0x90))continue;
        const auto r=gpr(int32_t(resource));
        if(MEM_HU(0x3C,r)!=0x401D || MEM_W(0x30,r))continue;
        const auto palette=uint32_t(MEM_W(0x88,r)),texture=uint32_t(MEM_W(0x84,r));
        if(fruit_mix::range(palette,16*512) && fruit_mix::range(texture,0x20))return resource;
    }
    return 0;
}
inline void prepare_release(uint8_t* rdram,uint32_t actor){
    const auto child=children.find(actor);
    if(child==children.end() || child->second.room!=current_room || !fruit_mix::range(actor,0x1F8))return;
    const auto a=gpr(int32_t(actor));
    if(MEM_W(0,a)!=0x19971110 || MEM_HU(12,a)!=0x41CE)return;
    const auto item=uint32_t(MEM_W(0x1F4,a));if(!fruit_mix::range(item,2))return;
    const auto original=MEM_HU(0,int32_t(item));
    if(original==0x401C || original==0x401D)MEM_H(0,int32_t(item))=0x401D;
}
// 8004535C normally frees a texture descriptor when it differs from the
// actor's original resource. Restore its borrowed defaults before disposal.
// This is only render cleanup: 8005C9A8 can do it while the actor survives.
inline void restore(uint8_t* rdram,uint32_t actor){
    if(!children.contains(actor))return;painted.erase(actor);
    if(!fruit_mix::range(actor,0x170))return;
    const auto a=gpr(int32_t(actor));const auto resource=uint32_t(MEM_W(0x168,a)),drawing=uint32_t(MEM_W(0x16C,a));
    if(!fruit_mix::range(resource,0x90) || !fruit_mix::range(drawing,0x44))return;
    const auto r=gpr(int32_t(resource)),d=gpr(int32_t(drawing));
    if(MEM_HU(0x3C,r)!=0x41CE || MEM_W(0x30,r) || MEM_W(0x38,d) || MEM_W(0x3C,d))return;
    MEM_W(8,d)=MEM_W(0x88,r);MEM_B(4,d)=0;MEM_W(0x30,d)=MEM_W(0x84,r);MEM_W(0x40,d)=resource+0x74;
}
// Full actor cleanup (80045B64) also frees collision and private state. The
// arena release (800439F4) can forget a fast-free actor without reading RAM.
inline void forget(uint32_t actor){
    sources.erase(actor);children.erase(actor);eggs.erase(actor);painted.erase(actor);unbound_painted.erase(actor);native_egg_actors.erase(actor);
    if(creating_parent==actor)creating_parent=0;
}
inline void dispose(uint8_t* rdram,uint32_t actor){restore(rdram,actor);forget(actor);}
inline bool paint(uint8_t* rdram,uint32_t actor){
    std::optional<unsigned> c;
    if(auto it=children.find(actor);it!=children.end() && it->second.room==current_room)c=it->second.color;
    else if(auto it=eggs.find(actor);it!=eggs.end())c=it->second.color;
    if(!c || !fruit_mix::range(actor,0x170))return false;
    const auto a=gpr(int32_t(actor));const auto resource=uint32_t(MEM_W(0x168,a)),drawing=uint32_t(MEM_W(0x16C,a));
    if(MEM_W(0,a)!=0x19971110 || MEM_BU(0x164,a)>1 || !fruit_mix::range(resource,0x90) || !fruit_mix::range(drawing,0x44))return false;
    const auto bound=MEM_HU(0x3C,int32_t(resource));
    const auto eggresource=native_egg_resource(rdram);
    if(!eggresource)return false;
    const auto r=gpr(int32_t(eggresource)),d=gpr(int32_t(drawing));
    // Same borrowed static sprite references as 800455DC/fruit_resources.
    // Keep the wrapper's resource, subtype, collision and native animation.
    if(MEM_W(0x38,d) || MEM_W(0x3C,d))return false;
    const auto eggbase=uint32_t(MEM_W(0x88,r)),texture=uint32_t(MEM_W(0x84,r)),geometry=eggresource+0x74;
    MEM_W(8,d)=eggbase+*c*512;MEM_B(4,d)=0;MEM_W(0x30,d)=texture;MEM_W(0x40,d)=geometry;
    if(painted.insert(actor).second)fprintf(stderr,"[EditorEgg] draw actor=%08X color=%u id=%04X bound=%04X frame=%u palette=%08X texture=%08X geometry=%08X nativeEggResource=%08X nativeEggPalette=%08X eggState=%u type=%u\n",actor,*c,MEM_HU(12,a),bound,MEM_HU(0x144,a),uint32_t(MEM_W(8,d)),uint32_t(MEM_W(0x30,d)),uint32_t(MEM_W(0x40,d)),eggresource,eggbase,uint32_t(MEM_W(0x1EC,a)),unsigned(MEM_BU(0x30,a)));
    return eggs.contains(actor);
}
}
