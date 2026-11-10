#pragma once
#include "coop_native_bridge.hpp"
#include "coop_native_collision.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <stdexcept>

extern "C" void func_8003B8F0(uint8_t*,recomp_context*);
extern "C" void func_80039F48(uint8_t*,recomp_context*);

namespace workshop::coop_probe {
// Explicit QA provisioning only. Native construction, real pickup callback,
// then controller-only aiming/throwing. Never write actor state/parent/speed/HP.
struct Inventory {
    uint32_t table=0;unsigned count=0,head=0;
    std::array<uint32_t,8> items{};
};
struct Player {
    uint32_t root=0,root_private=0,collectible=0,reserve=0,aiming=0,projectile=0;
    unsigned created_vi=0,reserve_vi=0,aiming_vi=0,flight_vi=0;
    uint64_t born=0; // Diagnostic seed ordinal, NOT the native registry generation.
    Inventory before{};
    bool created=false,picked=false,aim_seen=false,flight_seen=false,pickup_callback_invoked=false;
};
struct Snapshot {
    bool enabled=false,diagnostic_valid=true,roots_bound=false,both_reserves=false,both_aiming=false,both_flying=false;
    unsigned bound_vi=0;
    uint64_t created_count=0;
    std::array<Player,2> players{};
};
inline Snapshot state{};
inline bool unknown_flight_logged=false;
inline const Snapshot& snapshot(){return state;}
inline float scalar(uint8_t* r,uint32_t address){return std::bit_cast<float>(coop_native::word(r,address));}
inline void configure(bool enabled){state={};state.enabled=enabled;unknown_flight_logged=false;}
inline bool inventory(uint8_t* r,uint32_t root,Inventory& result){
    if(!coop_native::actor(r,root,0x4001))return false;
    const uint32_t data=coop_native::word(r,root+0x1F4);
    if(!coop_native::range(data,0x1550))return false;
    result={};result.count=coop_native::byte(r,data+0x12C8);
    result.head=coop_collision::half(r,data+0x12BE);result.table=data+0x11FC;
    if(result.count>8 || result.head>=8)return false;
    for(unsigned n=0;n<result.count;n++){
        const unsigned index=(result.head+n)&7;
        result.items[n]=coop_native::word(r,result.table+index*24);
        if(!coop_native::actor(r,result.items[n]))return false;
    }
    return true;
}
inline void trace(uint8_t* r,unsigned vi,unsigned slot,const char* stage,const Player& player,const Inventory& inv){
    const uint32_t item=player.projectile?player.projectile:player.aiming?player.aiming:player.reserve?player.reserve:player.collectible;
    const bool valid=coop_native::actor(r,item);
    const auto owner=valid?coop_collision::owner_of_egg(r,item):std::nullopt;
    fprintf(stderr,"[CoopProbe] vi=%u owner=%u stage=%s born=%llu root=%08X collectible=%08X reserve=%08X aiming=%08X projectile=%08X parent=%08X count=%u table=%08X registryOwner=%d slots=",vi,slot,stage,(unsigned long long)player.born,player.root,player.collectible,player.reserve,player.aiming,player.projectile,valid?coop_native::word(r,item+0x180):0,inv.count,inv.table,owner?int(*owner):-1);
    for(unsigned n=0;n<inv.count;n++)fprintf(stderr,"%s%08X",n?",":"",inv.items[n]);
    fprintf(stderr,"\n");
}
inline void fail(unsigned vi,unsigned slot,const char* reason)noexcept{
    state.diagnostic_valid=false;state.enabled=false;
    fprintf(stderr,"[CoopProbe] vi=%u owner=%u stage=failed reason=%s\n",vi,slot,reason);
}
inline void create(uint8_t* r,recomp_context* c,unsigned vi,unsigned slot){
    auto& player=state.players[slot];
    if(!inventory(r,player.root,player.before) || player.before.count>=6)throw std::runtime_error("invalid-or-full-native-inventory");
    coop_native::ScopedContext registers(c);
    coop_bridge::Scope owner(r,c,slot);
    c->f12.u32l=coop_native::word(r,player.root+0x84);
    c->f14.u32l=coop_native::word(r,player.root+0x88);
    c->r6=coop_native::word(r,player.root+0x8C);
    c->r7=coop_native::word(r,coop::native_color);
    func_8003B8F0(r,c);
    const uint32_t egg=uint32_t(c->r2);
    if(!coop_native::actor(r,egg,0x4002))throw std::runtime_error("native-egg-allocation-failed");
    player.collectible=egg;player.created=true;player.created_vi=vi;player.born=++state.created_count;
    trace(r,vi,slot,"created",player,player.before);
}
inline bool ready_collectible(uint8_t* r,const Player& player){
    const uint32_t egg=player.collectible;
    if(!coop_native::actor(r,egg,0x4002))return false;
    const uint32_t data=coop_native::word(r,egg+0x1F4),collision=coop_native::word(r,egg+0x174);
    //39F48 dereferences private byte0 before dispatch; zero means eligible.
    return coop_native::range(data,0x14) && coop_native::range(collision,0x18) &&
        coop_native::byte(r,data)==0 && coop_native::byte(r,egg+0x18)<8;
}
inline bool collectible_retired(uint8_t* r,const Player& player){
    const auto egg=player.collectible;
    return !coop_native::actor(r,egg,0x4002) || coop_native::byte(r,egg+0x18)==0xFF || coop_native::byte(r,egg+0x17D)==3;
}
inline bool observe_reserve(uint8_t* r,unsigned vi,unsigned slot){
    auto& player=state.players[slot];Inventory inv{};
    if(!inventory(r,player.root,inv))throw std::runtime_error("invalid-native-inventory-after-creation");
    if(inv.count==player.before.count)return false;
    if(inv.count!=player.before.count+1)throw std::runtime_error("native-inventory-delta-not-one");
    if(!collectible_retired(r,player))throw std::runtime_error("reserve-added-before-seed-retirement");
    uint32_t added=0;
    for(unsigned n=0;n<inv.count;n++){
        const auto item=inv.items[n];bool prior=false;
        for(unsigned k=0;k<player.before.count;k++)if(player.before.items[k]==item)prior=true;
        if(!prior && coop_native::actor(r,item,0x4004) && coop_native::word(r,item+0x180)==player.root){
            if(added)throw std::runtime_error("multiple-new-reserves");added=item;
        }
    }
    if(!added)throw std::runtime_error("native-reserve-parent-or-kind-invalid");
    player.reserve=added;player.picked=true;player.reserve_vi=vi;trace(r,vi,slot,"reserve",player,inv);
    fprintf(stderr,"[CoopProbe] vi=%u owner=%u stage=pickup-proof collectible=%08X reserve=%08X callbackInvoked=%u retired=1 countBefore=%u countAfter=%u parent=%08X\n",vi,slot,player.collectible,added,unsigned(player.pickup_callback_invoked),player.before.count,inv.count,coop_native::word(r,added+0x180));
    return true;
}
inline void pickup(uint8_t* r,recomp_context* c,unsigned vi,unsigned slot){
    auto& player=state.players[slot];
    if(collectible_retired(r,player))throw std::runtime_error("refusing-pickup-of-retired-collectible");
    {coop_native::ScopedContext registers(c);coop_bridge::Scope owner(r,c,slot);
        c->r4=coop_native::base(player.collectible);c->r5=coop_native::base(player.root);
        player.pickup_callback_invoked=true;func_80039F48(r,c);
    }
    if(!observe_reserve(r,vi,slot))throw std::runtime_error("native-pickup-did-not-add-reserve");
}

inline uint32_t aiming_child(uint8_t* r,uint32_t root){
    const auto count=coop_collision::half(r,root+0x184);
    if(count>20)return 0;
    // Attached children occupy sparse native slots after detach/reuse.
    for(unsigned n=0;n<20;n++){
        const uint32_t item=coop_native::word(r,root+0x188+n*4);
        if(coop_native::actor(r,item,0x4005) && coop_native::word(r,item+0x180)==root)return item;
    }
    return 0;
}
inline uint32_t flying_egg(uint8_t* r,unsigned slot){
    const uint32_t manager=coop_native::word(r,0x800F8810);
    if(!coop_native::range(manager,0x14))return 0;
    const int count=int16_t(coop_collision::half(r,manager+6));
    const uint32_t table=coop_native::word(r,manager+0x10);
    if(count<1 || count>150 || !coop_native::range(table,size_t(count)*4))return 0;
    for(int n=0;n<count;n++){
        const uint32_t egg=coop_native::word(r,table+uint32_t(n)*4);
        if(!coop_native::actor(r,egg,0x4002) || coop_native::byte(r,egg+0x18)!=0x68 || coop_native::word(r,egg+0x180))continue;
        const auto owner=coop_collision::owner_of_egg(r,egg);
        if(!owner){if(!unknown_flight_logged){unknown_flight_logged=true;fprintf(stderr,"[CoopProbe] owner=-1 stage=unattributed-flying projectile=%08X private=%08X\n",egg,coop_native::word(r,egg+0x1F4));}continue;}
        if(*owner==slot)return egg;
    }
    return 0;
}
inline void flight_trace(uint8_t* r,unsigned vi,unsigned slot,const Player& player){
    const auto egg=player.projectile;if(!coop_native::actor(r,egg,0x4002))return;
    const auto other=state.players[1-slot].root;
    const auto owner=coop_collision::owner_of_egg(r,egg);
    const auto& stats=coop_collision::statistics();
    const unsigned hp1=coop_bridge::context->player(0).health,hp2=coop_bridge::context->player(1).health;
    fprintf(stderr,"[CoopProbe] vi=%u owner=%u stage=flight born=%llu projectile=%08X parent=%08X registryOwner=%d state=%u xy=%.3f,%.3f velocity=%.3f,%.3f other=%08X otherXY=%.3f,%.3f hp=%u,%u eggReject=%llu lastFn=%08X lastA=%08X lastB=%08X lastOwners=%u,%u lastReason=%u\n",vi,slot,(unsigned long long)player.born,egg,coop_native::word(r,egg+0x180),owner?int(*owner):-1,coop_native::byte(r,egg+0x18),scalar(r,egg+0x84),scalar(r,egg+0x88),scalar(r,egg+0xC4),scalar(r,egg+0xC8),other,scalar(r,other+0x84),scalar(r,other+0x88),hp1,hp2,(unsigned long long)stats.egg_rejections,stats.last_function,stats.last_actor_a,stats.last_actor_b,stats.last_owner_a,stats.last_owner_b,unsigned(stats.last_reason));
}
inline void frame(uint8_t* r,recomp_context* c,unsigned vi)noexcept{
    if(!state.enabled || !state.diagnostic_valid || !coop_bridge::context || coop_bridge::invalidated || !coop_bridge::owner_stack.empty() || vi<2440)return;
    try{
        if(!state.roots_bound){
            for(unsigned slot=0;slot<2;slot++){
                const auto root=slot?coop_bridge::bound_secondary:coop_bridge::bound_primary;
                const auto owner=coop_collision::owner_of_body(r,root);
                if(!owner || *owner!=slot || !coop_native::actor(r,root,0x4001))return;
                state.players[slot].root=root;state.players[slot].root_private=coop_native::word(r,root+0x1F4);
            }
            state.roots_bound=true;state.bound_vi=vi;
        }
        for(unsigned slot=0;slot<2;slot++){
            auto& player=state.players[slot];
            if(!coop_native::actor(r,player.root,0x4001) || coop_native::word(r,player.root+0x1F4)!=player.root_private){fail(vi,slot,"root-lifetime-changed");return;}
            if(!player.created){create(r,c,vi,slot);continue;}
            if(!player.picked){
                // Collision may collect the diagnostic seed between our frame
                // samples. Recognize the exact native replacement first.
                if(observe_reserve(r,vi,slot))continue;
                if(collectible_retired(r,player)){fail(vi,slot,"seed-retired-without-owned-reserve");return;}
                if(ready_collectible(r,player))pickup(r,c,vi,slot);
                else if(vi>player.created_vi+120){fail(vi,slot,"native-collectible-initializer-timeout");return;}
                continue;
            }
            Inventory inv{};if(!inventory(r,player.root,inv)){fail(vi,slot,"invalid-native-inventory");return;}
            if(!player.aim_seen){const uint32_t aim=aiming_child(r,player.root);if(aim){player.aiming=aim;player.aim_seen=true;player.aiming_vi=vi;trace(r,vi,slot,"aiming",player,inv);}}
            if(!player.flight_seen){const uint32_t egg=flying_egg(r,slot);if(egg){player.projectile=egg;player.flight_seen=true;player.flight_vi=vi;trace(r,vi,slot,"launched",player,inv);}}
            if(player.flight_seen && vi<=player.flight_vi+120)flight_trace(r,vi,slot,player);
        }
        state.both_reserves=state.players[0].picked && state.players[1].picked;
        state.both_aiming=state.players[0].aim_seen && state.players[1].aim_seen;
        state.both_flying=state.players[0].flight_seen && state.players[1].flight_seen;
    }catch(const std::exception& error){fail(vi,2,error.what());}
    catch(...){fail(vi,2,"unexpected-native-probe-exception");}
}
}
