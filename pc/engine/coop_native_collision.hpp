#pragma once
#include "recomp.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace workshop::coop_collision {
// Native adapter only, disabled by default. All calls belong to the native
// game thread. This registry never writes guest memory; filter_geometry may
// only set V0=0 in the supplied CPU context. No gameplay capability is implied.
inline bool range(uint32_t address, size_t bytes, size_t alignment=4) {
    return address>=0x80000400u && address<0x80800000u &&
        address%alignment==0 && bytes<=0x80800000u-address;
}
inline uint32_t word(uint8_t* rdram,uint32_t address) {
    const gpr at=static_cast<gpr>(static_cast<int32_t>(address));
    return uint32_t(MEM_W(0,at));
}
inline uint16_t half(uint8_t* rdram,uint32_t address) {
    const gpr at=static_cast<gpr>(static_cast<int32_t>(address));
    return MEM_HU(0,at);
}
inline uint8_t byte(uint8_t* rdram,uint32_t address) {
    const gpr at=static_cast<gpr>(static_cast<int32_t>(address));
    return MEM_BU(0,at);
}
inline bool actor(uint8_t* rdram,uint32_t address) {
    if(!rdram || !range(address,0x1FC) || word(rdram,address)!=0x19971110u)return false;
    const auto life=byte(rdram,address+6);
    //45B2C initializes byte+6 to0; normal scheduled4001 and its body children
    //remain0 in real QA snapshots.46804 also uses2.439F4 marks free as3.
    return life<=2;
}
inline bool geometry_function(uint32_t function) {
    return function==0x8006009Cu || function==0x80060274u ||
        function==0x800603E8u || function==0x80060798u || function==0x80060960u;
}

struct Identity {
    uint32_t address=0,private_data=0;
    uint64_t generation=0;
    uint16_t kind=0;
    bool operator==(const Identity&)const=default;
};
struct Owned {
    Identity actor{},root{};
    unsigned slot=0;
    bool operator==(const Owned&)const=default;
};
enum class Rejection:unsigned {none,body,egg};
struct Statistics {
    uint64_t body_rejections=0,egg_rejections=0;
    uint64_t private_finalizations=0;
    std::array<uint64_t,5> geometry_rejections{};
    uint32_t last_function=0,last_actor_a=0,last_actor_b=0;
    unsigned last_owner_a=UINT32_MAX,last_owner_b=UINT32_MAX;
    Rejection last_reason=Rejection::none;
};

class Registry final {
    bool enabled_=false,interactions_=false;
    uint64_t next_generation_=0;
    uint64_t next_private_ticket_=0;
    // Native actor pools are bounded. Stop admitting events instead of letting
    // corrupt pointers turn this into an unbounded host allocation.
    static constexpr size_t capacity=1024;
    std::map<uint32_t,uint64_t> births_;
    std::array<std::optional<Identity>,2> roots_{};
    std::map<uint32_t,Owned> bodies_,eggs_;
    std::map<uint64_t,Owned> private_initializers_;
    mutable Statistics statistics_{};

    std::optional<Identity> identity(uint8_t* rdram,uint32_t address)const {
        if(!enabled_ || !actor(rdram,address))return std::nullopt;
        const auto birth=births_.find(address);
        if(birth==births_.end())return std::nullopt;
        return Identity{address,word(rdram,address+0x1F4),birth->second,half(rdram,address+12)};
    }
    bool live(uint8_t* rdram,const Identity& expected)const {
        const auto actual=identity(rdram,expected.address);
        return actual && *actual==expected;
    }
    bool live_root(uint8_t* rdram,unsigned slot)const {
        return slot<2 && roots_[slot] && live(rdram,*roots_[slot]) &&
            roots_[slot]->kind==0x4001 && range(roots_[slot]->private_data,0x1550);
    }
    bool live_owned(uint8_t* rdram,const Owned& owned)const {
        return live_root(rdram,owned.slot) && roots_[owned.slot]==owned.root && live(rdram,owned.actor);
    }
    //39B20 installs the resident shared4002 provider;3B8F0 allocates here.
    //45BD8 ->468E8 returns a pooled projectile without45B64 resource teardown.
    // Keep ownership for incarnation/collision proofs, but this exact free
    // cache slot is no longer an outstanding projectile for room/death gates.
    bool returned_egg_cache(uint8_t* r,const Owned& egg)const {
        const auto& id=egg.actor;const auto a=id.address;constexpr uint32_t pool=0x800C4280,storage=0x800C4290;
        if(!live_owned(r,egg)||id.kind!=0x4002||id.private_data!=a+0x24C||!range(id.private_data,0x30)||byte(r,a+6)!=1||byte(r,a+4)||byte(r,a+0x17D)||half(r,a+0x17E)!=150||word(r,a+0x180)||half(r,a+0x184)||word(r,a+0x1D8))return false;
        const auto slot=byte(r,a+5);
        if(word(r,0x800C4268)!=pool||word(r,a+8)!=pool||word(r,pool+4)!=8||word(r,pool+8)!=storage||slot<1||slot>8||a!=storage+(slot-1)*0x280+4||byte(r,a-4))return false;
        const auto resource=word(r,a+0x168);
        if(!range(resource,0x48)||half(r,resource+0x3C)!=0x4002||word(r,resource+0x40)||word(r,resource+0x44)!=0x8003B0D0||word(r,a+0x20)!=0x8003A854||word(r,a+0x2C)!=0x80045BD8||!range(word(r,a+0x16C),0x50)||!range(word(r,a+0x174),0x18))return false;
        for(unsigned i=0;i<20;i++)if(word(r,a+0x188+i*4))return false;
        for(const auto global:{0x800F887Cu,0x800F8810u}){
            const auto manager=word(r,global);if(!range(manager,0x18))return false;
            const auto count=half(r,manager+8);const auto table=word(r,manager+0x10);if(!count||count>150||!range(table,count*4))return false;
            unsigned found=0;for(unsigned i=0;i<count;i++){
                const auto value=word(r,table+i*4);if(global==0x800F8810u&&value==a)return false;
                if(global!=0x800F887Cu||!value)continue;
                if(value==pool){if(++found>1)return false;continue;}
                if((value>pool&&value<storage+8*0x280)||!range(value,12))return false;
                const auto slots=word(r,value+4),records=word(r,value+8);
                if(!slots||slots>150||!range(records,slots*0x280))return false;
                if(records<storage+8*0x280&&records+slots*0x280>storage)return false;
            }
            //46560 may unregister a quiescent provider. Its fixed descriptor,
            // slot/birth/resource proof remains authoritative; aliases do not.

        }
        return true;
    }
    // Resolve only a native body chain: never a carried fruit/enemy or shadow.
    // Each non-root link needs a known birth; loops and excessive depth reject.
    std::optional<unsigned> body_chain(uint8_t* rdram,uint32_t address)const {
        std::array<uint32_t,16> seen{};
        for(size_t depth=0;depth<seen.size();depth++) {
            for(size_t prior=0;prior<depth;prior++)if(seen[prior]==address)return std::nullopt;
            seen[depth]=address;
            for(unsigned slot=0;slot<2;slot++)
                if(live_root(rdram,slot) && roots_[slot]->address==address)return slot;
            const auto id=identity(rdram,address);
            if(!id || (id->kind!=0x400B && id->kind!=0x400C))return std::nullopt;
            const auto registered=bodies_.find(address);
            if(registered!=bodies_.end() && !live_owned(rdram,registered->second))return std::nullopt;
            address=word(rdram,address+0x180);
        }
        return std::nullopt;
    }
    bool record_egg(uint8_t* rdram,uint32_t address,unsigned slot) {
        const auto id=identity(rdram,address);
        if(!id || id->kind!=0x4002 || !live_root(rdram,slot))return false;
        const auto existing=eggs_.find(address);
        // A changed private pointer is not evidence of a new incarnation.
        // created() must precede a pooled reuse or reconstruction.
        if(existing!=eggs_.end() && existing->second.actor!=*id)return false;
        eggs_[address]=Owned{*id,*roots_[slot],slot};
        return true;
    }
    void forget_initializers(uint32_t address) {
        for(auto it=private_initializers_.begin();it!=private_initializers_.end();) {
            if(it->second.actor.address==address || it->second.root.address==address)
                it=private_initializers_.erase(it);
            else ++it;
        }
    }
public:
    void configure(bool enabled=false,bool interactions=false) {
        enabled_=enabled;interactions_=interactions;
        births_.clear();roots_={};bodies_.clear();eggs_.clear();private_initializers_.clear();statistics_={};
        // Do not reuse generations across configurations, even at the same
        // address/private pointer. No native addresses leave this process.
    }
    bool enabled()const{return enabled_;}
    // A proved native retry already constructed its canonical P1 and fruit.
    // Retire old ownership only; those observed births must remain usable.
    bool clear_ownership(){
        if(!enabled_||!private_initializers_.empty())return false;
        roots_={};bodies_.clear();eggs_.clear();statistics_={};return true;
    }
    void interactions(bool enabled){interactions_=enabled;}
    bool interactions()const{return interactions_;}
    const Statistics& statistics()const{return statistics_;}

    // Call BEFORE45CAC initialization, even when its actor still has old data.
    // This deliberately accepts an address, not a pre-existing actor identity.
    bool created(uint32_t address) {
        if(!enabled_ || !range(address,0x1FC))return false;
        if((births_.size()>=capacity && !births_.contains(address)) || next_generation_==UINT64_MAX) {
            configure(false);return false;
        }
        bodies_.erase(address);eggs_.erase(address);
        forget_initializers(address);
        for(auto& root:roots_)if(root && root->address==address)root.reset();
        births_[address]=++next_generation_;
        return true;
    }
    // Fast free439F4 and destruction45B64 may both report the same address.
    void forgotten(uint32_t address) {
        births_.erase(address);bodies_.erase(address);eggs_.erase(address);
        forget_initializers(address);
        for(auto& root:roots_)if(root && root->address==address)root.reset();
    }
    bool bind_root(uint8_t* rdram,unsigned slot,uint32_t address) {
        if(slot>=2)return false;
        const auto id=identity(rdram,address);
        if(!id || id->kind!=0x4001 || !range(id->private_data,0x1550))return false;
        if(roots_[1-slot] && roots_[1-slot]->address==address)return false;
        // Binding cannot legitimize a silent private-data change. A replacement
        // must first pass through a real creation/free event.
        if(roots_[slot] && *roots_[slot]!=*id)return false;
        roots_[slot]=*id;return true;
    }
    bool register_body(uint8_t* rdram,uint32_t address) {
        const auto id=identity(rdram,address);
        if(!id || (id->kind!=0x400B && id->kind!=0x400C))return false;
        const auto slot=body_chain(rdram,address);
        if(!slot)return false;
        const auto existing=bodies_.find(address);
        if(existing!=bodies_.end() && existing->second.actor!=*id)return false;
        bodies_[address]=Owned{*id,*roots_[*slot],*slot};return true;
    }
    // Read-only incarnation proof shared by the native food transaction.
    // Generations are issued only by the real45CAC creation hook.
    std::optional<Identity> observed_identity(uint8_t* rdram,uint32_t address)const {return identity(rdram,address);}
    std::optional<unsigned> body_owner(uint8_t* rdram,uint32_t address)const {
        if(!enabled_)return std::nullopt;
        for(unsigned slot=0;slot<2;slot++)
            if(live_root(rdram,slot) && roots_[slot]->address==address)return slot;
        const auto found=bodies_.find(address);
        if(found==bodies_.end() || !live_owned(rdram,found->second))return std::nullopt;
        const auto actual=body_chain(rdram,address);
        return actual && *actual==found->second.slot?actual:std::nullopt;
    }
    std::optional<unsigned> egg_owner(uint8_t* rdram,uint32_t address)const {
        if(!enabled_)return std::nullopt;
        const auto found=eggs_.find(address);
        return found!=eggs_.end() && live_owned(rdram,found->second)?
            std::optional<unsigned>(found->second.slot):std::nullopt;
    }

    std::vector<Identity> eggs_for_owner(uint8_t* rdram,unsigned slot)const {
        std::vector<Identity> result;if(slot>=2)return result;
        for(const auto& [address,egg]:eggs_)if(egg.slot==slot&&live_owned(rdram,egg)&&!returned_egg_cache(rdram,egg)){(void)address;result.push_back(egg.actor);}
        return result;
    }
    //3B8F0: F12/F14=x/y, A2=z bits, A3=colour, V0=4002 or0. Capture
    // owner_slot from an already validated enclosing scope BEFORE the call.
    // A0/A1 and current singleton8828 are not ownership evidence here.
    bool egg_created(uint8_t* rdram,uint32_t native_v0,unsigned owner_slot) {
        return record_egg(rdram,native_v0,owner_slot);
    }
    //3B8F0 may return an attributed4002 with private1F4 still0. Later3AE00
    //initializes it (405C8 ->3AE4C, or3B1BC/3B26C). Capture immediately BEFORE
    //that exact native function; this ticket proves the already-owned birth.
    uint64_t begin_egg_private_init(uint8_t* rdram,uint32_t address) {
        const auto existing=eggs_.find(address);
        if(existing==eggs_.end() || !live_owned(rdram,existing->second) ||
            existing->second.actor.private_data!=0 || !range(address,0x27C) ||
            private_initializers_.size()>=32 || next_private_ticket_==UINT64_MAX)return 0;
        // Reject duplicate/reentrant initializers for the same incarnation.
        for(const auto& pending:private_initializers_)
            if(pending.second.actor==existing->second.actor)return 0;
        const auto ticket=++next_private_ticket_;
        private_initializers_.emplace(ticket,existing->second);
        return ticket;
    }
    //Call immediately AFTER a normal3AE00 return, before its caller can mutate
    //the private bytes.45A34(size14) uses the embedded30-byte area, sets1F2=1,
    //then3AE00 clears exactly14 bytes. No arbitrary private replacement passes.
    //The ticket is consumed even on failure and cannot reassign an owner.
    bool finish_egg_private_init(uint8_t* rdram,uint64_t ticket) {
        const auto pending=private_initializers_.find(ticket);
        if(pending==private_initializers_.end())return false;
        const auto prior=pending->second;
        private_initializers_.erase(pending);
        const auto actual=identity(rdram,prior.actor.address);
        const auto existing=eggs_.find(prior.actor.address);
        if(!actual || existing==eggs_.end() || existing->second!=prior ||
            !live_root(rdram,prior.slot) || roots_[prior.slot]!=prior.root ||
            actual->generation!=prior.actor.generation || actual->kind!=0x4002 ||
            prior.actor.private_data!=0 || actual->private_data!=actual->address+0x24C ||
            !range(actual->private_data,0x30) || byte(rdram,actual->address+0x1F2)!=1)return false;
        for(uint32_t offset=0;offset<0x14;offset++)
            if(byte(rdram,actual->private_data+offset)!=0)return false;
        existing->second.actor=*actual;
        if(statistics_.private_finalizations!=UINT64_MAX)++statistics_.private_finalizations;
        return true;
    }
    void cancel_egg_private_init(uint64_t ticket){private_initializers_.erase(ticket);}
    // Call AFTER a real native attachment; absence of a crash/return is not
    // enough. The native child.parent must equal the supplied actual parent.
    bool attached(uint8_t* rdram,uint32_t item,uint32_t parent) {
        const auto id=identity(rdram,item);
        if(!id || id->kind!=0x4002 || word(rdram,item+0x180)!=parent)return false;
        const auto slot=body_owner(rdram,parent);
        return slot && record_egg(rdram,item,*slot);
    }
    // Capture a proven attachment before48060 clears parent180. A thrown egg
    // keeps its recorded owner afterward; detach is deliberately not forget.
    bool before_detach(uint8_t* rdram,uint32_t item) {
        const auto id=identity(rdram,item);
        if(!id || id->kind!=0x4002)return false;
        const auto parent=word(rdram,item+0x180);
        return parent?attached(rdram,item,parent):egg_owner(rdram,item).has_value();
    }
    Rejection decision(uint8_t* rdram,uint32_t function,uint32_t work)const {
        if(!enabled_ || !geometry_function(function) || !rdram || !range(work,0x94) ||
            !live_root(rdram,0) || !live_root(rdram,1))return Rejection::none;
        const auto a=word(rdram,work+4),b=word(rdram,work+0x3C);
        if(a==b || !actor(rdram,a) || !actor(rdram,b))return Rejection::none;
        const auto body_a=body_owner(rdram,a),body_b=body_owner(rdram,b);
        if(body_a && body_b)return *body_a!=*body_b && !interactions_?Rejection::body:Rejection::none;
        const auto egg_a=egg_owner(rdram,a),egg_b=egg_owner(rdram,b);
        return (egg_a && body_b && *egg_a!=*body_b) ||
            (egg_b && body_a && *egg_b!=*body_a)?Rejection::egg:Rejection::none;
    }
    bool ignore_geometry(uint8_t* rdram,uint32_t function,uint32_t work)const {
        return decision(rdram,function,work)!=Rejection::none;
    }
    bool filter_geometry(uint8_t* rdram,recomp_context* context,uint32_t function)const {
        if(!context)return false;
        const auto work=uint32_t(context->r4);
        const auto reason=decision(rdram,function,work);
        if(reason==Rejection::none)return false;
        // Counters describe actual wrapper rejections; read-only predicate
        // calls and ownership getters do not increment them. Never log here.
        const auto increment=[](uint64_t& value){if(value!=UINT64_MAX)++value;};
        increment(reason==Rejection::body?statistics_.body_rejections:statistics_.egg_rejections);
        const uint32_t functions[]={0x8006009C,0x80060274,0x800603E8,0x80060798,0x80060960};
        for(size_t index=0;index<5;index++)if(functions[index]==function)increment(statistics_.geometry_rejections[index]);
        statistics_.last_function=function;statistics_.last_actor_a=word(rdram,work+4);
        statistics_.last_actor_b=word(rdram,work+0x3C);statistics_.last_reason=reason;
        const auto owner_a=body_owner(rdram,statistics_.last_actor_a),owner_b=body_owner(rdram,statistics_.last_actor_b);
        statistics_.last_owner_a=owner_a.value_or(egg_owner(rdram,statistics_.last_actor_a).value_or(UINT32_MAX));
        statistics_.last_owner_b=owner_b.value_or(egg_owner(rdram,statistics_.last_actor_b).value_or(UINT32_MAX));
        context->r2=0;return true;
    }
};

inline Registry registry;
inline void configure(bool enabled=false,bool interactions=false){registry.configure(enabled,interactions);}
inline bool created(uint32_t actor){return registry.created(actor);}
inline void forgotten(uint32_t actor){registry.forgotten(actor);}
inline bool bind_root(uint8_t* r,unsigned slot,uint32_t actor){return registry.bind_root(r,slot,actor);}
inline bool register_body(uint8_t* r,uint32_t actor){return registry.register_body(r,actor);}
inline bool egg_created(uint8_t* r,uint32_t result,unsigned slot){return registry.egg_created(r,result,slot);}
inline uint64_t begin_egg_private_init(uint8_t* r,uint32_t actor){return registry.begin_egg_private_init(r,actor);}
inline bool finish_egg_private_init(uint8_t* r,uint64_t ticket){return registry.finish_egg_private_init(r,ticket);}
inline void cancel_egg_private_init(uint64_t ticket){registry.cancel_egg_private_init(ticket);}
inline bool attached(uint8_t* r,uint32_t item,uint32_t parent){return registry.attached(r,item,parent);}
inline bool before_detach(uint8_t* r,uint32_t item){return registry.before_detach(r,item);}
inline std::optional<Identity> observed_identity(uint8_t* r,uint32_t actor){return registry.observed_identity(r,actor);}
inline std::optional<unsigned> owner_of_body(uint8_t* r,uint32_t actor){return registry.body_owner(r,actor);}
inline std::optional<unsigned> owner_of_egg(uint8_t* r,uint32_t actor){return registry.egg_owner(r,actor);}
inline const Statistics& statistics(){return registry.statistics();}
inline bool ignore_geometry(uint8_t* r,uint32_t function,uint32_t work){return registry.ignore_geometry(r,function,work);}
inline bool filter_geometry(uint8_t* r,recomp_context* c,uint32_t function){return registry.filter_geometry(r,c,function);}
}
