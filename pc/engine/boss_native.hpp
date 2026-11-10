#pragma once
#include "recomp.h"
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace workshop::boss_native {
// Read-only observer for the original SOLO 3-1 candy encounter. This adapter
// neither launches a fight nor awards a Boss Battle completion/challenge.
inline constexpr unsigned first_stage=8, first_room=162;
inline constexpr uint16_t candy_kind=0x455A, head_kind=0x454F;
inline constexpr unsigned original_segments=12;
inline constexpr uint32_t candy_rom_base=0x80664CD0, scene_rom_base=0x807AC660;
struct Frame {
    unsigned source_room=177,vi=0;
    uint32_t candy_overlay=0,scene_overlay=0; // section_addresses[167], [285]
};
struct Snapshot {
    bool enabled=false,initialized=false,active_seen=false,terminal_seen=false,
         completion_called=false,completion_ready=false,invalidated=false;
    uint32_t actor=0,private_data=0,head=0,player=0,transition_actor=0;
    unsigned source_room=177,initial_vi=0,active_vi=0,terminal_vi=0,completion_vi=0;
    int segments=-1,max_segments=-1,min_segments=-1;
    std::string failure;
};
struct DefeatObservation {
    std::string run_token;
    unsigned source_room=first_room,stage=first_stage,vi=0;
    uint32_t actor=0,player=0,transition_actor=0;
    int initial_segments=original_segments,remaining_segments=-1;
    bool diagnostic=true; // evidence provenance; never a challenge award
};
inline Snapshot state;
inline std::string run_token;
inline bool diagnostic=true,delivered=false;
inline unsigned current_room=177,pending_vi=0;
inline bool range(uint32_t p,size_t n=4){return p>=0x80000400u && p<0x80800000u && n<=0x80800000u-p;}
inline gpr base(uint32_t p){return gpr(int32_t(p));}
inline uint32_t word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,base(p)));}
inline unsigned byte(uint8_t* rdram,uint32_t p){return MEM_BU(0,base(p));}
inline bool actor(uint8_t* rdram,uint32_t p,uint16_t kind){return range(p,0x1FC) && word(rdram,p)==0x19971110 && byte(rdram,p+6)!=3 && MEM_HU(12,base(p))==kind;}
inline void invalidate(std::string reason){if(!state.completion_ready){state.invalidated=true;state.failure=std::move(reason);}}
inline bool configure(bool enabled,std::string token,bool is_diagnostic=true){
    state={};run_token=std::move(token);diagnostic=is_diagnostic;delivered=false;current_room=177;pending_vi=0;
    state.enabled=enabled && !run_token.empty() && run_token.size()<=128;
    return state.enabled;
}
inline void enter_room(unsigned source){
    if(state.enabled && state.initialized && source!=first_room)invalidate("left_first_arena");
    current_room=source;state.source_room=source;
}
// Invoke at complete actor cleanup or address reuse, never graphics-only reset.
inline void forget(uint32_t p){
    if(state.enabled && p==state.actor)invalidate("candy_actor_disposed_or_reused");
}
inline bool sample(uint8_t* rdram){
    if(!state.enabled || state.invalidated || !state.initialized || current_room!=first_room)return false;
    if(!actor(rdram,state.actor,candy_kind)){invalidate("candy_identity_changed");return false;}
    const auto priv=word(rdram,state.actor+0x1F4);
    if(!range(priv,0x24))return false; // constructor may still be initializing
    if(state.private_data && state.private_data!=priv){invalidate("candy_private_changed");return false;}
    state.private_data=priv;
    const auto h=word(rdram,priv);
    if(actor(rdram,h,head_kind) && word(rdram,h+0x180)==state.actor){
        const auto hp=word(rdram,h+0x1F4);
        if(range(hp,0x88)){
            const int n=int32_t(word(rdram,hp+0x84));
            if(n<0 || n>int(original_segments)){invalidate("invalid_native_segment_count");return false;}
            state.head=h;state.segments=n;state.max_segments=std::max(state.max_segments,n);
            state.min_segments=state.min_segments<0?n:std::min(state.min_segments,n);
        }
    }
    return true;
}
inline void frame(uint8_t* rdram,const Frame& f){
    if(!state.enabled)return;
    if(f.source_room!=current_room)enter_room(f.source_room);
    if(!sample(rdram))return;
    if(state.active_seen && word(rdram,state.private_data+0x20)==1 && byte(rdram,state.actor+0x18)==1 && byte(rdram,state.actor+0x30)==8){
        state.terminal_seen=true;if(!state.terminal_vi)state.terminal_vi=f.vi;
    }
}
inline bool alive_player(uint8_t* rdram,uint32_t p){return actor(rdram,p,0x4001) && byte(rdram,p+0x18)!=0xA && byte(rdram,p+0x30)!=0x11;}
inline void observe_entry(uint8_t* rdram,const recomp_context* c,std::string_view name,const Frame& f){
    if(!state.enabled || state.invalidated || !c)return;
    if(f.source_room!=current_room)enter_room(f.source_room);
    if(current_room!=first_room)return;
    const auto a=uint32_t(c->r4);
    if(name=="func_80664E18"){
        if(!actor(rdram,a,candy_kind))return;
        if(state.initialized){invalidate("more_than_one_native_candy_initialization");return;}
        state.initialized=true;state.actor=a;state.initial_vi=f.vi;return;
    }
    if(name=="func_806657A0" && a==state.actor && sample(rdram) && word(rdram,state.private_data+0x20)==0 && byte(rdram,a+0x18)==1 && byte(rdram,a+0x30)==1){
        state.active_seen=true;if(!state.active_vi)state.active_vi=f.vi;
    }
    // This is the terminal native state, not an intro/dialogue timer. Polling
    // keeps the same identity and the native private defeat flag bound together.
    if(name=="func_80665C80" || name=="func_807AEA78")frame(rdram,f);
    if(name=="func_807AEA78" && state.active_seen && state.terminal_seen && state.max_segments==int(original_segments) && sample(rdram)){
        if(!range(f.candy_overlay,0x3000) || !range(f.scene_overlay,0x2600))return;
        if(uint32_t(c->r31)!=f.candy_overlay+0x109C || word(rdram,0x800F8D78)!=f.scene_overlay+0x2418 || uint32_t(c->r5)!=0 || !alive_player(rdram,a) || word(rdram,state.actor+0x1E0)!=0)return;
        state.completion_called=true;state.player=a;pending_vi=f.vi;
    }
    // The native allocation must have succeeded. Other 46EE uses (including
    // boss initialization failures) have a different caller and no terminal
    // candy evidence. This setter is reached only after 807AE95C succeeds.
    if(name=="func_807AEA24" && state.completion_called && pending_vi==f.vi && uint32_t(c->r31)==f.scene_overlay+0x23AC && uint32_t(c->r5)==1 && uint32_t(c->r6)==0 && actor(rdram,a,0x46EE) && range(word(rdram,a+0x1F4),4)){
        state.completion_ready=true;state.completion_vi=f.vi;state.transition_actor=a;
    }
}
inline const Snapshot& snapshot(){return state;}
inline std::optional<DefeatObservation> take_defeat_observation(){
    if(!state.enabled || state.invalidated || !state.completion_ready || delivered)return std::nullopt;
    delivered=true;
    return DefeatObservation{run_token,first_room,first_stage,state.completion_vi,state.actor,state.player,state.transition_actor,state.max_segments,state.segments,diagnostic};
}
} // namespace workshop::boss_native
