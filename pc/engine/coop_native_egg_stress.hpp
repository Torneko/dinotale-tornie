#pragma once
#include "coop_native_probe.hpp"
#include <array>
#include <cstdio>
#include <fstream>
#include <cstring>
#include <memory>
#include <vector>
#include <stdexcept>

namespace workshop::coop_egg_stress {
// Explicit bounded QA only. Native collectible creation and pickup fill P2,
// then P1. No actor state, parent, velocity or health is written by this probe.
struct Snapshot {
    bool enabled=false,valid=true,bound=false,pending=false,filled=false,complete=false,aim_seen=false,shared=false;
    unsigned completed=0,reloads=0,created_vi=0,next_vi=2440;
    uint32_t observed_projectile=0;
    std::array<uint32_t,2> roots{},privates{};
    std::array<unsigned,2> counts{};
    std::array<uint32_t,12> reserves{};
    coop_probe::Player item{};
};
inline Snapshot state{};
struct BufferedDump {unsigned step=0;std::unique_ptr<uint8_t[]> bytes;bool flushed=false;};
inline std::vector<BufferedDump> buffered_dumps;
inline const Snapshot& snapshot(){return state;}
inline void configure(bool enabled,bool shared=false){state={};state.enabled=enabled;state.shared=shared;buffered_dumps.clear();buffered_dumps.reserve(16);}
inline unsigned step(){return state.filled?13+state.reloads:state.completed+1;}
inline unsigned current_owner(){return state.filled?state.reloads%2:(state.completed<6?1u:0u);}
inline void fail(unsigned vi,const char* reason)noexcept {
    state.valid=false;state.enabled=false;
    fprintf(stderr,"[CoopEggStress] phase=failed vi=%u step=%u reason=%s\n",vi,step(),reason);
}
inline bool inventories(uint8_t* r,std::array<coop_probe::Inventory,2>& inv){
    for(unsigned owner=0;owner<2;owner++)if(!coop_probe::inventory(r,state.roots[owner],inv[owner]))return false;
    return true;
}
inline void trace_pool(uint8_t* r,unsigned vi,unsigned step,unsigned owner,uint32_t global,const char* kind){
    const uint32_t descriptor=coop_native::word(r,global);
    if(!coop_native::range(descriptor,12)){
        fprintf(stderr,"[CoopEggStressPool] vi=%u step=%u owner=%u kind=%s descriptor=%08X valid=0\n",vi,step,owner,kind,descriptor);return;
    }
    const auto cursor=coop_native::word(r,descriptor),capacity=coop_native::word(r,descriptor+4),base=coop_native::word(r,descriptor+8);
    const bool valid=capacity>0 && capacity<=64 && coop_native::range(base,capacity*0x280);
    unsigned free=0;
    if(valid)for(unsigned n=0;n<capacity;n++)if(coop_native::byte(r,base+n*0x280)==0)free++;
    fprintf(stderr,"[CoopEggStressPool] vi=%u step=%u owner=%u kind=%s descriptor=%08X cursor=%u capacity=%u base=%08X free=%u valid=%u\n",vi,step,owner,kind,descriptor,cursor,capacity,base,free,unsigned(valid));
    if(!valid)return;
    for(unsigned n=0;n<capacity;n++){
        const uint32_t raw=base+n*0x280,actor=raw+4;
        fprintf(stderr,"[CoopEggStressSlot] vi=%u step=%u owner=%u pool=%s index=%u raw=%08X actor=%08X byte0=%u life=%u state=%u phase=%u kind=%04X parent=%08X private=%08X\n",vi,step,owner,kind,n,raw,actor,coop_native::byte(r,raw),coop_native::byte(r,actor+6),coop_native::byte(r,actor+0x18),coop_native::byte(r,actor+0x17D),coop_collision::half(r,actor+12),coop_native::word(r,actor+0x180),coop_native::word(r,actor+0x1F4));
    }
}
inline void trace_pools(uint8_t* r,unsigned vi,unsigned step,unsigned owner){
    trace_pool(r,vi,step,owner,0x800C426C,"4004");
    trace_pool(r,vi,step,owner,0x800C4270,"4005");
}
inline void dump(uint8_t* r,unsigned step){
    if(step!=buffered_dumps.size()+1 || step>16)throw std::runtime_error("unexpected-buffered-dump-sequence");
    // No disk IO on this game fence: copying first keeps the scripted clock
    // independent of storage latency. Buffers are host-only and bounded128MiB.
    BufferedDump entry;entry.step=step;entry.bytes.reset(new uint8_t[0x800000]);
    std::memcpy(entry.bytes.get(),r,0x800000);buffered_dumps.push_back(std::move(entry));
}
inline bool flush_dumps()noexcept{
    try{
        unsigned files=0;
        for(auto& entry:buffered_dumps){
            if(entry.flushed){files++;continue;}
            char name[96];std::snprintf(name,sizeof(name),"coop-egg-stress-before-%02u.rdram",entry.step);
            std::ofstream file(name,std::ios::binary);if(!file)throw std::runtime_error("cannot-open-before-pickup-dump");
            file.write(reinterpret_cast<const char*>(entry.bytes.get()),0x800000);file.close();
            if(!file)throw std::runtime_error("incomplete-before-pickup-dump");
            entry.flushed=true;entry.bytes.reset();files++;
        }
        fprintf(stderr,"[CoopEggStress] phase=dump-flush files=%u complete=1\n",files);return true;
    }catch(const std::exception& error){fail(0,error.what());return false;}
    catch(...){fail(0,"unexpected-dump-flush-exception");return false;}
}
inline uint32_t newly_added(uint8_t* r,const coop_probe::Inventory& before,const coop_probe::Inventory& after,uint32_t root){
    if(after.count!=before.count+1)return 0;
    uint32_t result=0;
    for(unsigned n=0;n<after.count;n++){
        const auto actor=after.items[n];bool prior=false;
        for(unsigned k=0;k<before.count;k++)if(before.items[k]==actor)prior=true;
        if(!prior){
            if(result || !coop_native::actor(r,actor,0x4004) || coop_native::word(r,actor+0x180)!=root)return 0;
            result=actor;
        }
    }
    return result;
}
inline bool observe_pickup(uint8_t* r,unsigned vi,unsigned owner){
    std::array<coop_probe::Inventory,2> inv{};
    if(!inventories(r,inv))throw std::runtime_error("invalid-inventory-after-pickup");
    if(inv[owner].count==state.item.before.count)return false;
    const auto added=newly_added(r,state.item.before,inv[owner],state.roots[owner]);
    if(!added || !coop_probe::collectible_retired(r,state.item))throw std::runtime_error("invalid-reserve-replacement");
    const unsigned expected_other=state.filled?6:(owner==1?0:6);
    if(inv[1-owner].count!=expected_other)throw std::runtime_error("other-inventory-changed");
    // A recycled address is legitimate only after leaving every live inventory.
    // Compare current queues, never all historically observed reserve addresses.
    std::array<uint32_t,12> live{};unsigned live_count=0;
    for(unsigned slot=0;slot<2;slot++)for(unsigned n=0;n<inv[slot].count;n++){
        const auto actor=inv[slot].items[n];
        if(!coop_native::actor(r,actor,0x4004) || coop_native::word(r,actor+0x180)!=state.roots[slot])throw std::runtime_error("live-reserve-parent-invalid");
        for(unsigned k=0;k<live_count;k++)if(live[k]==actor)throw std::runtime_error("reserve-shared-between-live-inventories");
        if(live_count>=live.size())throw std::runtime_error("too-many-live-reserves");live[live_count++]=actor;
    }
    const unsigned completed_step=step();const bool recycling=state.filled;
    if(recycling)state.reloads++;else{state.reserves[state.completed]=added;state.completed++;}
    state.pending=false;state.aim_seen=false;state.next_vi=vi+4;
    state.counts={inv[0].count,inv[1].count};
    fprintf(stderr,"[CoopEggStress] phase=%s vi=%u step=%u owner=%u collectible=%08X reserve=%08X parent=%08X counts=%u,%u retired=1 projectile=%08X\n",recycling?"reloaded":"pickup",vi,completed_step,owner,state.item.collectible,added,coop_native::word(r,added+0x180),state.counts[0],state.counts[1],state.observed_projectile);
    return true;
}
inline void create(uint8_t* r,recomp_context* c,unsigned vi,unsigned owner){
    std::array<coop_probe::Inventory,2> inv{};
    if(!inventories(r,inv))throw std::runtime_error("invalid-inventory-before-create");
    const unsigned expected=state.filled?5:state.completed%6;
    if(inv[owner].count!=expected || inv[1-owner].count!=(state.filled?6u:(owner==1?0u:6u)))throw std::runtime_error("unexpected-inventory-count-before-create");
    state.item={};state.item.root=state.roots[owner];state.item.before=inv[owner];state.counts={inv[0].count,inv[1].count};
    coop_native::ScopedContext registers(c);coop_bridge::Scope scope(r,c,owner);
    c->f12.u32l=coop_native::word(r,state.roots[owner]+0x84);c->f14.u32l=coop_native::word(r,state.roots[owner]+0x88);
    c->r6=coop_native::word(r,state.roots[owner]+0x8C);c->r7=coop_native::word(r,coop::native_color);
    func_8003B8F0(r,c);const uint32_t egg=uint32_t(c->r2);
    if(!coop_native::actor(r,egg,0x4002))throw std::runtime_error("native-collectible-allocation-failed");
    state.item.collectible=egg;state.pending=true;state.created_vi=vi;
    fprintf(stderr,"[CoopEggStress] phase=created vi=%u step=%u owner=%u collectible=%08X counts=%u,%u\n",vi,step(),owner,egg,inv[0].count,inv[1].count);
}
inline void frame(uint8_t* r,recomp_context* c,unsigned vi)noexcept {
    if(!state.enabled || !state.valid || state.complete || !coop_bridge::context || coop_bridge::invalidated || !coop_bridge::owner_stack.empty() || vi<state.next_vi)return;
    try{
        if(!state.bound){
            state.roots={coop_bridge::bound_primary,coop_bridge::bound_secondary};
            for(unsigned owner=0;owner<2;owner++){
                const auto actual=coop_collision::owner_of_body(r,state.roots[owner]);
                if(!actual || *actual!=owner)return;
                state.privates[owner]=coop_native::word(r,state.roots[owner]+0x1F4);
            }
            std::array<coop_probe::Inventory,2> inv{};
            if(!inventories(r,inv) || inv[0].count || inv[1].count)throw std::runtime_error("stress-requires-empty-inventories");
            state.bound=true;fprintf(stderr,"[CoopEggStress] phase=bound vi=%u primary=%08X secondary=%08X counts=0,0\n",vi,state.roots[0],state.roots[1]);
        }
        for(unsigned owner=0;owner<2;owner++)if(!coop_native::actor(r,state.roots[owner],0x4001) || coop_native::word(r,state.roots[owner]+0x1F4)!=state.privates[owner])throw std::runtime_error("root-lifetime-changed");
        if(!state.filled && state.completed<12){
            const auto raw=coop_native_input::word(r,coop_native_input::pad_pointer);
            if(!coop_native_input::range(raw,16))throw std::runtime_error("invalid-primary-pad-pointer");
            const auto primary=coop_collision::half(r,raw);
            if((primary|coop_native_input::latest.buttons)&0x2000)throw std::runtime_error("fill-not-complete-before-first-native-Z");
        }
        if((!state.filled && state.completed==12) || (state.filled && state.reloads==4)){
            std::array<coop_probe::Inventory,2> inv{};
            if(!inventories(r,inv) || inv[0].count!=6 || inv[1].count!=6)throw std::runtime_error("final-inventory-count-mismatch");
            for(unsigned owner=0;owner<2;owner++)for(unsigned n=0;n<6;n++){
                if(!coop_native::actor(r,inv[owner].items[n],0x4004) || coop_native::word(r,inv[owner].items[n]+0x180)!=state.roots[owner])throw std::runtime_error("final-reserve-owner-mismatch");
                for(unsigned other=0;other<owner;other++)for(unsigned m=0;m<6;m++)if(inv[owner].items[n]==inv[other].items[m])throw std::runtime_error("final-reserves-shared");
            }
            state.counts={6,6};
            if(!state.filled){
                state.filled=true;state.aim_seen=false;
                fprintf(stderr,"[CoopEggStress] phase=complete vi=%u counts=6,6 uniqueReserves=12 valid=1\n",vi);return;
            }
            state.complete=true;
            for(unsigned owner=0;owner<2;owner++){coop_native::ScopedContext regs(c);coop_bridge::Scope scope(r,c,owner);trace_pools(r,vi,17,owner);}
            fprintf(stderr,"[CoopEggStress] phase=recycle-complete vi=%u counts=6,6 reloads=4 uniqueReserves=12 valid=1\n",vi);return;
        }
        const auto owner=current_owner();
        if(state.filled && !state.pending){
            std::array<coop_probe::Inventory,2> inv{};
            if(!inventories(r,inv) || inv[1-owner].count!=6 || inv[owner].count<5 || inv[owner].count>6)throw std::runtime_error("unexpected-live-inventory-during-throw");
            const auto aiming=coop_probe::aiming_child(r,state.roots[owner]);
            if(aiming && inv[owner].count==5 && !state.aim_seen){
                state.aim_seen=true;fprintf(stderr,"[CoopEggStress] phase=aiming vi=%u step=%u owner=%u aiming=%08X parent=%08X counts=%u,%u\n",vi,step(),owner,aiming,coop_native::word(r,aiming+0x180),inv[0].count,inv[1].count);
            }
            if(inv[owner].count==5 && !aiming && state.aim_seen){
                const auto projectile=coop_probe::flying_egg(r,owner);
                if(projectile){
                    state.observed_projectile=projectile;
                    fprintf(stderr,"[CoopEggStress] phase=launch-observed vi=%u step=%u owner=%u projectile=%08X registryOwner=%u parent=%08X counts=%u,%u\n",vi,step(),owner,projectile,owner,coop_native::word(r,projectile+0x180),inv[0].count,inv[1].count);
                    create(r,c,vi,owner);return;
                }
            }
            if(vi>2850)throw std::runtime_error("native-throw-observation-timeout");return;
        }
        if(!state.pending){create(r,c,vi,owner);return;}
        if(observe_pickup(r,vi,owner))return;
        if(coop_probe::collectible_retired(r,state.item))throw std::runtime_error("retired-collectible-without-reserve");
        if(!coop_probe::ready_collectible(r,state.item)){
            if(vi>state.created_vi+120)throw std::runtime_error("collectible-init-timeout");return;
        }
        {
            coop_native::ScopedContext registers(c);coop_bridge::Scope scope(r,c,owner);
            fprintf(stderr,"[CoopEggStress] phase=before-pickup vi=%u step=%u owner=%u collectible=%08X counts=%u,%u\n",vi,step(),owner,state.item.collectible,state.counts[0],state.counts[1]);
            trace_pools(r,vi,step(),owner);dump(r,step());
            // The expected crash never reaches the final fence; preserve its
            // nine snapshots before invoking the failing native allocation.
            if(state.shared && step()==9 && !flush_dumps())return;
            c->r4=coop_native::base(state.item.collectible);c->r5=coop_native::base(state.roots[owner]);func_80039F48(r,c);
        }
        if(!observe_pickup(r,vi,owner))throw std::runtime_error("native-pickup-did-not-add-reserve");
    }catch(const std::exception& error){fail(vi,error.what());}
    catch(...){fail(vi,"unexpected-native-stress-exception");}
}
}
