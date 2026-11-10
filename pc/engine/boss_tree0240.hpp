#pragma once
#include "boss_native.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <string_view>

namespace workshop::boss_tree0240 {
// Native4708 has six slots. Slot0 is the invincibility heart and is never
// supplied in Boss Battle. The five fruit slots share two whole-tree regens.
inline constexpr unsigned rest_room=83,all_consumed=0x1F,fruit_slots=5,max_regenerations=2,max_consumed=fruit_slots*(max_regenerations+1);
inline constexpr uint16_t tree_kind=0x4708;
inline constexpr std::array<uint16_t,6> kinds{0x4035,0x4034,0x4033,0x4030,0x4032,0x4031};
struct Snapshot {
    bool enabled=false,inRestRoom=false,initialized=false,invalidated=false;
    unsigned room=177,consumedmask=0,regenerations=0,totalConsumed=0;
    uint32_t tree=0,privateData=0;
    std::array<uint32_t,5> fruitActors{},fruitOrigins{};
};
inline Snapshot state{};
inline const Snapshot& snapshot(){return state;}
inline void configure(bool enabled){state={};state.enabled=enabled;}
inline bool range(uint32_t p,unsigned bytes){return p%4==0 && boss_native::range(p,bytes);}
inline uint32_t word(uint8_t* rdram,uint32_t p){return boss_native::word(rdram,p);}
inline void half(uint8_t* rdram,uint32_t p,unsigned v){MEM_H(0,boss_native::base(p))=uint16_t(v);}
inline bool actor(uint8_t* rdram,uint32_t p,uint16_t kind){return rdram && range(p,0x1FC) && boss_native::actor(rdram,p,kind);}
// Native room/tree bindings expire at teardown, but a detached fruit can
// still be carried by the same Yoshi. Preserve its provenance until its real
// collection or generic actor-forget hook; otherwise room hopping refills it.
inline void clear_bindings(){state.tree=state.privateData=0;state.initialized=false;}
inline bool ledger_valid(){return state.regenerations<=max_regenerations && state.totalConsumed<=max_consumed && !(state.consumedmask&~all_consumed) && state.totalConsumed==state.regenerations*fruit_slots+std::popcount(state.consumedmask);}
inline bool validate_ledger(){if(ledger_valid())return true;state.invalidated=true;return false;}
inline void enter_room(unsigned room){
    if(state.room!=room)clear_bindings();
    state.room=room;state.inRestRoom=state.enabled && room==rest_room;
}
inline bool tree_matches(uint8_t* rdram){
    return state.enabled && state.inRestRoom && state.initialized && !state.invalidated &&
        actor(rdram,state.tree,tree_kind) && range(state.privateData,0x50) && word(rdram,state.tree+0x1F4)==state.privateData;
}
inline unsigned slot(uint16_t kind){for(unsigned i=1;i<kinds.size();++i)if(kinds[i]==kind)return i;return 0;}
// Retired pooled actors may retain their magic, kind and allocation byte until
// reuse. They no longer own a supply slot; a detached live tongue fruit does.
inline bool fruit_matches(uint8_t* rdram,unsigned i){
    if(i<1 || i>fruit_slots)return false;const auto child=state.fruitActors[i-1];
    return actor(rdram,child,kinds[i]) && boss_native::byte(rdram,child+0x17D)!=3 && boss_native::byte(rdram,child+0x18)!=0xFF;
}
inline bool held_by_player(uint8_t* rdram,uint32_t child){
    const auto player=word(rdram,0x800F8828);if(!actor(rdram,player,0x4001))return false;
    const auto data=word(rdram,player+0x1F4);return range(data,0x168) && word(rdram,data+0x164)==child;
}
inline void bind(uint8_t* rdram,uint32_t child,unsigned i){
    if(!tree_matches(rdram) || i<1 || i>5 || !actor(rdram,child,kinds[i]) || (state.consumedmask&(1u<<(i-1))))return;
    auto& previous=state.fruitActors[i-1];
    if(previous && previous!=child && fruit_matches(rdram,i)){state.invalidated=true;return;}
    previous=child;state.fruitOrigins[i-1]=state.tree;
}
inline void allowances(uint8_t* rdram){
    if(!tree_matches(rdram))return;
    half(rdram,state.privateData+0x1C,0); // no4035; preserve other invincibility sources
    for(unsigned i=1;i<=5;++i){
        auto& child=state.fruitActors[i-1];
        if(child && !fruit_matches(rdram,i)){child=0;state.fruitOrigins[i-1]=0;}
        // A detached fruit already on the tongue still owns its slot. Room
        // teardown alone cannot replenish it; actual disposal releases the token.
        const bool eaten=state.totalConsumed>=max_consumed || (state.consumedmask&(1u<<(i-1)))!=0;
        half(rdram,state.privateData+0x1C+i*2,eaten || child?0:1);
    }
}
inline void frame(uint8_t* rdram){
    if(!rdram || !state.enabled || state.invalidated || !validate_ledger())return;
    // Lifetime pruning is read-only outside rest and never authorizes a new
    // native supply. A carried token survives a room change while it is live.
    for(unsigned i=1;i<=fruit_slots;++i)if(state.fruitActors[i-1] && !fruit_matches(rdram,i)){state.fruitActors[i-1]=0;state.fruitOrigins[i-1]=0;}
    if(!state.inRestRoom || !state.initialized)return;
    if(!tree_matches(rdram)){clear_bindings();return;}
    // A late-bound native heart is just as invalid as one seen by the attach
    // hook. Supply0 alone does not remove an already existing4035 actor.
    const auto heart=word(rdram,state.privateData);
    if(actor(rdram,heart,kinds[0]) && word(rdram,heart+0x180)==state.tree){state.invalidated=true;return;}
    // This also captures slots created before the generic attachment hook was
    // seen. Only real current children may establish a new binding.
    for(unsigned i=1;i<=5;++i){
        const auto child=word(rdram,state.privateData+i*4);
        if(actor(rdram,child,kinds[i]) && word(rdram,child+0x180)==state.tree)bind(rdram,child,i);
    }
    allowances(rdram);
}
inline void forget(uint32_t p){
    if(p && p==state.tree){clear_bindings();return;}
    for(unsigned i=0;i<fruit_slots;++i)if(state.fruitActors[i]==p){state.fruitActors[i]=0;state.fruitOrigins[i]=0;}
}
inline void trace(uint8_t* rdram,recomp_context* ctx,std::string_view name){
    if(!rdram || !ctx || !state.enabled || state.invalidated || !validate_ledger())return;
    const auto a=uint32_t(ctx->r4);
    // Collection can finish after the Yoshi carries a tree fruit out of83.
    // Account only a previously proven token; writes to tree counters still
    // require a current matching rest-tree allocation through allowances().
    if(name=="func_80573F94"){
        for(unsigned i=1;i<=fruit_slots;++i){
            if(state.fruitActors[i-1]!=a || !fruit_matches(rdram,i))continue;
            const auto bit=1u<<(i-1);if(state.consumedmask&bit)return;
            if(state.totalConsumed>=max_consumed){state.invalidated=true;return;}
            state.fruitActors[i-1]=state.fruitOrigins[i-1]=0;state.consumedmask|=bit;++state.totalConsumed;
            if(state.consumedmask==all_consumed && state.regenerations<max_regenerations){++state.regenerations;state.consumedmask=0;}
            allowances(rdram);return;
        }
        return;
    }
    if(!state.inRestRoom)return;
    if(name=="func_807B810C" && uint32_t(ctx->r5)==1 && uint32_t(ctx->r6)==0 && actor(rdram,a,tree_kind)){
        const auto priv=word(rdram,a+0x1F4);if(!range(priv,0x50))return;
        if(state.initialized){
            if(state.tree!=a || state.privateData!=priv)state.invalidated=true;
            // A repeated state setter cannot replenish an existing batch.
            allowances(rdram);return;
        }
        // Only detached/carried provenance may survive a native tree rebind.
        // An uneaten fruit still attached to its origin tree did not travel
        // with Yoshi, even if a pooled header remains typed after teardown.
        // Current genuine children are rediscovered by frame()/bind().
        for(unsigned i=0;i<fruit_slots;++i)if(state.fruitActors[i] && state.fruitOrigins[i] && fruit_matches(rdram,i+1) && word(rdram,state.fruitActors[i]+0x180)==state.fruitOrigins[i] && !held_by_player(rdram,state.fruitActors[i])){
            state.fruitActors[i]=state.fruitOrigins[i]=0;
        }
        state.tree=a;state.privateData=priv;state.initialized=true;allowances(rdram);return;
    }
    if(!tree_matches(rdram))return;
    if(name=="func_80047F9C" && a==state.tree){
        const auto child=uint32_t(ctx->r5);
        if(actor(rdram,child,kinds[0])){state.invalidated=true;return;}
        if(range(child,0x1FC))bind(rdram,child,slot(uint16_t(MEM_HU(12,boss_native::base(child)))));
        allowances(rdram);return;
    }

}
} // namespace workshop::boss_tree0240
