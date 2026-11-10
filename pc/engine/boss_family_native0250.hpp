#pragma once
#include "boss_native.hpp"
#include "boss_battle0250.hpp"
#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace workshop::boss_family_native0250 {
// Read-only native evidence, not a fight simulator, spawner, damage writer or award.
// Section indices below are the actual RELOC indices used by RecompiledFuncs.
// Vanilla genie4072: D33C initializes1E4=3; D090 decrements; state1/5 C3A0
// calls globalF8D78 atC3F8 after1E0 becomes121. The first post-intro combat
// callback is C428/state1/6: C750 selects it after timer637. C9FC/state1/1
// is a later attack that need not occur before the active C428/C604 loops.
// Vanilla invisible4599: AF580 initializes1E4=2; AEFDC decrements first two hits;
// the next real hit selects1/7 AED30, which callsF8D78 atAED7C when1E0=30.
// Both admit damage before their first combat callback. A returned, exact
// native damage effect proves activity, including a direct one-hit terminal.
// Giant lips40A3: private24 is phase0..2, not actor1E4 (a reused timer!).
// Intro58C50/state1/13 clears private30 before the first58794 witness;
// 57D44 accepts a4002 hit then, with no active-state gate. Count only its
// returned native lock0->1 effect; still require active/terminal proof below.
// Final collision selects2/0 ->59458 ->2/12;58EA0 callsF8D78 at58EE8
// when actor1E4 becomes30. Native807AEA78 receives THE BOSS andA1=1.
// HP0 is a living, hungry Yoshi: native2B798 checks health BEFORE damage,
// so reducing it to zero does not kill the player until a later hit. Substate11
// is only fatal together with primaryA; primary1/sub11 remains a live action.
// Victory callbacks require an active player, not nativeA/2 pot transport or
// any primaryA cinematic/death state. Keep frozen0240 predicates unchanged.
inline bool active_player(uint8_t* rdram,uint32_t player){
    return boss_native::actor(rdram,player,0x4001)&&boss_native::byte(rdram,player+0x18)!=0xA;
}
enum class Family { invisible, genie, giant_lips, candy };
struct Spec {
    Family family;
    unsigned vanilla_room,vanilla_stage,overlay_section;
    uint16_t kind;
    uint32_t rom_base,init,active,terminal,terminal_prepare,completion_ra,hit;
    unsigned active_state,active_substate,terminal_state,terminal_substate;
    unsigned counter_initial,counter_terminal,private_size;
    bool counter_in_private;
};
inline constexpr std::array<Spec,3> specs{{
    {Family::invisible,164,0,197,0x4599,0x806ADD40,0x806AF580,0x806AEE78,0x806AED30,0,0x806AED84,0x806AEFDC,1,2,1,7,2,0,0x2C,false},
    {Family::genie,163,0,16,0x4072,0x8042A8A0,0x8042D33C,0x8042C428,0x8042C3A0,0,0x8042C400,0x8042D090,1,6,1,5,3,0,0x28,false},
    {Family::giant_lips,165,11,120,0x40A3,0x80557B50,0x8055863C,0x80558794,0x80558EA0,0x80559458,0x80558EF0,0x80557D44,1,1,2,12,0,2,0x34,true}
}};
inline constexpr const Spec& spec(Family f){return specs[static_cast<unsigned>(f)];}
struct Frame {
    unsigned source_room=177,vi=0;
    uint32_t family_overlay=0,scene_overlay=0;
};
struct ActorEvidence {
    uint32_t actor=0,private_data=0,transition=0,player=0,head=0,head_private=0;
    unsigned initial_vi=0,active_vi=0,terminal_vi=0,completion_vi=0;
    int initial_counter=-1,counter=-1;
    unsigned causal_hits=0;
    bool initialized=false,active_seen=false,terminal_prepare_seen=false,
         terminal_seen=false,completion_called=false,allocation_seen=false,
         completion_returned=false,cinematic_suppressed=false,disposed=false,delivered=false;
};
struct Snapshot {
    bool enabled=false,invalidated=false,diagnostic=true,encounter_delivered=false;
    Family family=Family::invisible;
    unsigned source_room=177,expected_actors=0,registered_actors=0,expected_hits=0;
    std::string run_token,failure;
    std::array<ActorEvidence,4> actors{};
};
struct DefeatObservation {
    std::string run_token;
    Family family=Family::invisible;
    unsigned source_room=177,vi=0;
    uint32_t actor=0,player=0,transition_actor=0;
    int initial_native_counter=-1,terminal_native_counter=-1;
    bool diagnostic=true;
    unsigned encounter=0,member=0,causal_hits=0;
    bool cinematic_suppressed=false;
};
struct EncounterObservation {
    std::string run_token;
    Family family=Family::invisible;
    unsigned source_room=177;
    bool diagnostic=true;
    std::vector<DefeatObservation> defeated;
};
class Observer {
    Snapshot state_;
    unsigned pending_=4,pending_vi_=0,hit_pending_=4,hit_vi_=0;
    int hit_before_=-1;
    unsigned initial_counter()const{return spec(state_.family).kind==0x4072?state_.expected_hits:spec(state_.family).kind==0x4599?state_.expected_hits-1:0;}
    static bool token_valid(std::string_view t){
        if(t.empty()||t.size()>128)return false;
        for(unsigned char c:t)if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'))return false;
        return true;
    }
    void fail(std::string reason){state_.invalidated=true;state_.failure=std::move(reason);pending_=4;}
    unsigned find(uint32_t a)const{for(unsigned n=0;n<state_.registered_actors;++n)if(state_.actors[n].actor==a)return n;return 4;}
    static bool named(std::string_view name,uint32_t address){
        constexpr char hex[]="0123456789ABCDEF";std::array<char,13> s{'f','u','n','c','_'};
        for(unsigned n=0;n<8;++n)s[5+n]=hex[(address>>(28-4*n))&15];
        return name==std::string_view(s.data(),s.size());
    }
    bool same_state(uint8_t* rdram,const ActorEvidence& a,unsigned state,unsigned sub)const{
        return boss_native::byte(rdram,a.actor+0x18)==state && boss_native::byte(rdram,a.actor+0x30)==sub;
    }
    bool sample(uint8_t* rdram,ActorEvidence& a){
        const auto& cfg=spec(state_.family);
        if(!boss_native::actor(rdram,a.actor,cfg.kind)){fail("family_actor_identity_changed");return false;}
        const auto p=boss_native::word(rdram,a.actor+0x1F4);
        if(!boss_native::range(p,cfg.private_size))return false; // constructor may be in progress
        if(a.private_data&&a.private_data!=p){fail("family_private_identity_changed");return false;}
        a.private_data=p;
        const int value=int32_t(boss_native::word(rdram,cfg.counter_in_private?p+0x24:a.actor+0x1E4));
        const int lo=int(std::min(initial_counter(),cfg.counter_terminal)),hi=int(std::max(initial_counter(),cfg.counter_terminal));
        if(value<lo||value>hi){fail("native_family_counter_out_of_range");return false;}
        if(a.initial_counter<0){
            if(value!=int(initial_counter())){fail("native_family_initial_counter_missing");return false;}
            a.initial_counter=value;
        }else if(a.counter>=0 && (cfg.counter_in_private?value<a.counter:value>a.counter)){
            fail("native_family_counter_reversed");return false;
        }
        a.counter=value;return true;
    }
    bool overlay_valid(const Frame& f)const{
        const auto& cfg=spec(state_.family);const auto extent=std::max({cfg.init,cfg.active,cfg.terminal,cfg.terminal_prepare,cfg.completion_ra})-cfg.rom_base+4;
        return boss_native::range(f.family_overlay,extent)&&boss_native::range(f.scene_overlay,0x2800);
    }
    uint32_t relocate(const Frame& f,uint32_t original)const{return f.family_overlay+(original-spec(state_.family).rom_base);}
    // NativeD1C8 first admits4002/state68, calls its impact callback, then
    // callsD090 with A2=egg/A3=root and saved root/egg at SP18/1C.
    // A1 is no longer the projectile here. Prove that exact native caller.
    bool genie_projectile(uint8_t* rdram,const recomp_context* c,const Frame& f,uint32_t actor)const{
        const auto stack=uint32_t(c->r29),projectile=uint32_t(c->r6);
        return overlay_valid(f)&&uint32_t(c->r31)==relocate(f,0x8042D270)&&!(stack&3)&&boss_native::range(stack,0x20)&&
            uint32_t(c->r7)==actor&&boss_native::word(rdram,stack+0x18)==actor&&boss_native::word(rdram,stack+0x1C)==projectile&&boss_native::actor(rdram,projectile,0x4002);
    }
    bool counter_terminal(const ActorEvidence& a)const{return a.counter==int(spec(state_.family).counter_terminal);}
    DefeatObservation receipt(const ActorEvidence& a)const{
        DefeatObservation out{state_.run_token,state_.family,state_.source_room,a.completion_vi,a.actor,a.player,a.transition,a.initial_counter,a.counter,state_.diagnostic};out.causal_hits=a.causal_hits;out.cinematic_suppressed=a.cinematic_suppressed;return out;
    }
public:
    bool configure(Family family,std::string token,unsigned source_room,unsigned expected_actors=1,bool diagnostic=true,unsigned configured_hits=0){
        state_={};pending_=4;pending_vi_=0;hit_pending_=4;hit_vi_=0;hit_before_=-1;
        if(static_cast<unsigned>(family)>=specs.size()||!token_valid(token)||source_room>255||!expected_actors||expected_actors>4)return false;
        if(!configured_hits)configured_hits=3;if(configured_hits>3||(family==Family::giant_lips&&configured_hits!=3))return false;
        state_.expected_hits=configured_hits;state_.enabled=true;state_.family=family;state_.run_token=std::move(token);state_.source_room=source_room;state_.expected_actors=expected_actors;state_.diagnostic=diagnostic;return true;
    }
    const Snapshot& snapshot()const{return state_;}
    void enter_room(unsigned source_room){if(state_.enabled&&!state_.invalidated&&source_room!=state_.source_room)fail("left_configured_family_arena");}
    // Complete actor teardown/reuse only, not graphics refresh. A proven defeated
    // actor may disappear naturally while companions are still fighting.
    void forget(uint32_t p){
        if(!state_.enabled||state_.invalidated)return;const auto n=find(p);if(n==4)return;
        auto& a=state_.actors[n];if(!a.completion_returned)fail("family_actor_disposed_before_proof");else a.disposed=true;
    }
    void frame(uint8_t* rdram,const Frame& f){
        if(!state_.enabled||state_.invalidated||!rdram)return;
        if(f.source_room!=state_.source_room){enter_room(f.source_room);return;}
        for(unsigned n=0;n<state_.registered_actors&&!state_.invalidated;++n){auto& a=state_.actors[n];if(!a.completion_returned&&!a.disposed)sample(rdram,a);}
    }
    void observe_entry(uint8_t* rdram,const recomp_context* c,std::string_view name,const Frame& f){
        if(!state_.enabled||state_.invalidated||!rdram||!c)return;
        if(f.source_room!=state_.source_room){enter_room(f.source_room);return;}
        const auto& cfg=spec(state_.family);const auto arg=uint32_t(c->r4);
        if(named(name,cfg.init)){
            if(!boss_native::actor(rdram,arg,cfg.kind))return;
            if(find(arg)!=4){fail("native_family_actor_reinitialized_or_reused");return;}
            if(state_.registered_actors>=state_.expected_actors){fail("too_many_native_family_actors");return;}
            auto& a=state_.actors[state_.registered_actors++];a.initialized=true;a.actor=arg;a.initial_vi=f.vi;return;
        }
        auto n=find(arg);
        if(named(name,cfg.hit)&&state_.family==Family::giant_lips&&boss_native::range(arg,0x1FC))n=find(boss_native::word(rdram,arg+0x180));
        if(named(name,cfg.hit)&&n<4){
            auto& a=state_.actors[n];if(!a.completion_returned&&sample(rdram,a)){
                bool accepted=false;const unsigned sub=boss_native::byte(rdram,a.actor+0x30);
                if(cfg.kind==0x4072)accepted=sub!=4&&sub!=5&&a.counter>0&&genie_projectile(rdram,c,f,a.actor);
                if(cfg.kind==0x4599){const auto projectile=uint32_t(c->r5);accepted=sub!=1&&sub!=6&&sub!=7&&boss_native::range(projectile,0x1FC)&&boss_native::word(rdram,projectile)==0x19971110&&boss_native::byte(rdram,projectile+0x18)==0x68;}
                if(state_.family==Family::giant_lips)accepted=boss_native::actor(rdram,uint32_t(c->r5),0x4002)&&boss_native::word(rdram,arg+0x180)==a.actor&&boss_native::word(rdram,a.private_data+0x30)==0;
                if(accepted){if(hit_pending_!=4){fail("overlapping_native_family_hit");return;}hit_pending_=n;hit_vi_=f.vi;hit_before_=a.counter;}
            }
        }
        const bool proof_hook=named(name,cfg.active)||named(name,cfg.hit)||named(name,cfg.terminal)||(cfg.terminal_prepare&&named(name,cfg.terminal_prepare))||name=="func_807AEA78";
        if(n<4&&proof_hook){
            auto& a=state_.actors[n];
            if(!a.completion_returned && sample(rdram,a)&&overlay_valid(f)){
                if(named(name,cfg.active)&&same_state(rdram,a,cfg.active_state,cfg.active_substate)&&boss_native::word(rdram,arg+0x20)==relocate(f,cfg.active)&&a.initial_counter==int(initial_counter())){
                    a.active_seen=true;if(!a.active_vi)a.active_vi=f.vi;
                }
                if(cfg.terminal_prepare&&named(name,cfg.terminal_prepare)&&a.active_seen&&a.causal_hits==state_.expected_hits&&counter_terminal(a)&&same_state(rdram,a,2,0)&&boss_native::word(rdram,arg+0x20)==relocate(f,cfg.terminal_prepare))a.terminal_prepare_seen=true;
                if(named(name,cfg.terminal)&&a.active_seen&&a.causal_hits==state_.expected_hits&&counter_terminal(a)&&same_state(rdram,a,cfg.terminal_state,cfg.terminal_substate)&&boss_native::word(rdram,arg+0x20)==relocate(f,cfg.terminal)&&(!cfg.terminal_prepare||a.terminal_prepare_seen)){
                    a.terminal_seen=true;if(!a.terminal_vi)a.terminal_vi=f.vi;
                }
            }
        }
        if(state_.invalidated)return;
        if(name=="func_807AEA78"){
            if(n==4||!overlay_valid(f))return;auto& a=state_.actors[n];
            if(a.completion_returned){fail("duplicate_native_family_completion");return;}
            if(!a.active_seen||!a.terminal_seen||a.causal_hits!=state_.expected_hits||!counter_terminal(a)||!sample(rdram,a)||!same_state(rdram,a,cfg.terminal_state,cfg.terminal_substate))return;
            const auto player=boss_native::word(rdram,0x800F8828);
            const unsigned timer=boss_native::word(rdram,arg+(state_.family==Family::giant_lips?0x1E4:0x1E0));
            const unsigned required_timer=cfg.kind==0x4072?121:30;
            if(uint32_t(c->r31)!=relocate(f,cfg.completion_ra)||boss_native::word(rdram,0x800F8D78)!=f.scene_overlay+0x2418||uint32_t(c->r5)!=1||timer!=required_timer||!active_player(rdram,player))return;
            if(pending_!=4){fail("overlapping_native_family_completion");return;}
            a.completion_called=true;a.player=player;pending_=n;pending_vi_=f.vi;
        }
        if(name=="func_807AEA24"&&pending_<4&&pending_vi_==f.vi&&overlay_valid(f)&&uint32_t(c->r31)==f.scene_overlay+0x23AC&&uint32_t(c->r5)==1&&uint32_t(c->r6)==0&&boss_native::actor(rdram,arg,0x46EE)&&boss_native::range(boss_native::word(rdram,arg+0x1F4),4)){
            auto& a=state_.actors[pending_];if(a.allocation_seen){fail("duplicate_native_family_cinematic");return;}
            for(unsigned j=0;j<state_.registered_actors;++j)if(j!=pending_&&state_.actors[j].transition==arg){fail("native_family_cinematic_reused");return;}
            a.allocation_seen=true;a.transition=arg;
        }
    }
    // Call immediately after the ORIGINAL native hit function returns. Merely
    // entering a collider is not a hit: verify its real damage/state effect.
    void observe_hit_return(uint8_t* rdram,std::string_view name,const Frame& f){
        if(!state_.enabled||state_.invalidated||hit_pending_==4||!rdram)return;
        const auto& cfg=spec(state_.family);if(!named(name,cfg.hit))return;
        const auto n=hit_pending_;hit_pending_=4;auto& a=state_.actors[n];
        if(f.source_room!=state_.source_room||hit_vi_!=f.vi||!sample(rdram,a)){fail("native_family_hit_return_scope_mismatch");return;}
        bool effect=false;
        if(cfg.kind==0x4072)effect=a.counter==hit_before_-1;
        if(cfg.kind==0x4599)effect=hit_before_>0?a.counter==hit_before_-1:(a.counter==0&&same_state(rdram,a,1,7));
        if(state_.family==Family::giant_lips)effect=boss_native::word(rdram,a.private_data+0x30)==1;
        if(effect){
            if(++a.causal_hits>state_.expected_hits)fail("too_many_native_family_hits");
            // D090 and AEFDC admit genuine damage before their optional
            // combat callbacks. The returned native effect proves activity,
            // including one-hit variants that go straight to terminal.
            if(cfg.kind==0x4072||cfg.kind==0x4599){a.active_seen=true;if(!a.active_vi)a.active_vi=f.vi;}
        }
    }
    bool completion_pending()const{return state_.enabled&&!state_.invalidated&&pending_<4;}
    uint32_t pending_actor()const{return completion_pending()?state_.actors[pending_].actor:0;}
    // The route owner may suppress only an already-proven native victory call
    // for an intermediate member. No46EE is fabricated, and this receipt is
    // explicitly marked suppressed. Native callers ignore V0 and continue
    // their normal retirement; the owner restores controls after those callers.
    bool acknowledge_suppressed_completion(uint8_t* rdram,const Frame& f){
        if(!completion_pending()||!rdram)return false;auto& a=state_.actors[pending_];
        if(f.source_room!=state_.source_room||pending_vi_!=f.vi||a.allocation_seen||!sample(rdram,a)||!active_player(rdram,a.player)||boss_native::word(rdram,0x800F8828)!=a.player)return false;
        a.completion_returned=true;a.cinematic_suppressed=true;a.completion_vi=f.vi;pending_=4;pending_vi_=0;return true;
    }
    // Must be called after ORIGINAL807AEA78 returns. It confirms returned46EE,
    // successful init plus final private0=1 (AEA6C), rather than a speculative
    // constructor entry or an allocation-failure branch. Never invoke native code.
    void completion_return(uint8_t* rdram,const recomp_context* c,const Frame& f){
        if(!state_.enabled||state_.invalidated||pending_==4||!c||!rdram)return;
        auto& a=state_.actors[pending_];
        if(f.source_room!=state_.source_room||pending_vi_!=f.vi||!a.allocation_seen||uint32_t(c->r2)!=a.transition||!boss_native::actor(rdram,a.transition,0x46EE)||!active_player(rdram,a.player)||boss_native::word(rdram,0x800F8828)!=a.player){fail("native_family_cinematic_return_rejected");return;}
        const auto p=boss_native::word(rdram,a.transition+0x1F4);
        if(!boss_native::range(p,4)||boss_native::word(rdram,p)!=1){fail("native_family_cinematic_initialization_failed");return;}
        a.completion_returned=true;a.completion_vi=f.vi;pending_=4;pending_vi_=0;
    }
    std::optional<DefeatObservation> take_defeat_observation(){
        if(!state_.enabled||state_.invalidated)return std::nullopt;
        for(unsigned n=0;n<state_.registered_actors;++n){auto& a=state_.actors[n];if(a.completion_returned&&!a.delivered){a.delivered=true;return receipt(a);}}
        return std::nullopt;
    }
    std::optional<EncounterObservation> take_encounter_observation(){
        if(!state_.enabled||state_.invalidated||state_.encounter_delivered||state_.registered_actors!=state_.expected_actors)return std::nullopt;
        for(unsigned n=0;n<state_.registered_actors;++n)if(!state_.actors[n].completion_returned)return std::nullopt;
        EncounterObservation out{state_.run_token,state_.family,state_.source_room,state_.diagnostic,{}};
        for(unsigned n=0;n<state_.registered_actors;++n)out.defeated.push_back(receipt(state_.actors[n]));
        state_.encounter_delivered=true;return out;
    }
};
}

namespace workshop::boss_family_native0250 {
inline std::optional<Family> family_for_kind(uint16_t kind){switch(kind){case 0x4072:return Family::genie;case 0x4599:return Family::invisible;case 0x40A3:return Family::giant_lips;default:return std::nullopt;}}
inline boss_battle0250::NativeReceipt core_receipt(const DefeatObservation& proof){return {proof.encounter,proof.member,proof.vi,proof.actor,true,true,true,proof.causal_hits>0};}
class MemberObserver {
    Observer observer_;uint32_t bound_=0;unsigned encounter_=0,member_=0;bool configured_=false;
public:
    bool configure(const boss_battle0250::Encounter& e,unsigned member,std::string token,unsigned room,bool diagnostic=true){
        configured_=false;bound_=0;if(!e.number||e.number>10||member>=e.count||e.count>4||!e.members[member].hits)return false;const auto family=family_for_kind(e.members[member].kind);if(!family)return false;
        encounter_=e.number-1;member_=member;configured_=observer_.configure(*family,std::move(token),room,1,diagnostic,e.members[member].hits);return configured_;
    }
    bool bind(uint32_t actor){if(!configured_||bound_||!boss_native::range(actor,0x1FC))return false;bound_=actor;return true;}
    uint32_t bound_actor()const{return bound_;}
    const Snapshot& snapshot()const{return observer_.snapshot();}
    void enter_room(unsigned room){observer_.enter_room(room);}
    void forget(uint32_t actor){observer_.forget(actor);}
    void frame(uint8_t* rdram,const Frame& f){observer_.frame(rdram,f);}
    void observe_entry(uint8_t* rdram,const recomp_context* c,std::string_view name,const Frame& f){
        if(!configured_||!bound_||!c)return;const auto& cfg=spec(observer_.snapshot().family);
        // All shared completion/allocation hooks must still reach a pending
        // member; only constructor discovery is filtered by its explicit binding.
        std::array<char,14> init{};constexpr char hex[]="0123456789ABCDEF";init[0]='f';init[1]='u';init[2]='n';init[3]='c';init[4]='_';for(unsigned n=0;n<8;++n)init[5+n]=hex[(cfg.init>>(28-n*4))&15];
        if(name==std::string_view(init.data(),13)&&uint32_t(c->r4)!=bound_)return;
        observer_.observe_entry(rdram,c,name,f);
    }
    void observe_hit_return(uint8_t* rdram,std::string_view name,const Frame& f){observer_.observe_hit_return(rdram,name,f);}
    bool completion_pending()const{return observer_.completion_pending();}
    bool acknowledge_suppressed_completion(uint8_t* rdram,const Frame& f){return observer_.acknowledge_suppressed_completion(rdram,f);}
    void completion_return(uint8_t* rdram,const recomp_context* c,const Frame& f){observer_.completion_return(rdram,c,f);}
    std::optional<DefeatObservation> take_defeat_observation(){auto p=observer_.take_defeat_observation();if(p){p->encounter=encounter_;p->member=member_;}return p;}
};
}

namespace workshop::boss_family_native0250 {
// Source80665C80 saves root at sp+48 and callsF8D78 withA0=player.
// This exact caller frame distinguishes simultaneous small candies safely.
inline uint32_t candy_owner_from_completion(uint8_t* rdram,const recomp_context* c,const Frame& f){
    if(!rdram||!c||!boss_native::range(f.family_overlay,0x3000)||uint32_t(c->r31)!=f.family_overlay+0x109C)return 0;
    const auto stack=uint32_t(c->r29);if(!boss_native::range(stack,0x4C))return 0;
    const auto root=boss_native::word(rdram,stack+0x48);if(!boss_native::actor(rdram,root,0x455A)||boss_native::word(rdram,root+0x1E0)!=0)return 0;
    const auto p=boss_native::word(rdram,root+0x1F4);return boss_native::range(p,0x24)&&boss_native::word(rdram,p+0x20)==1?root:0;
}
class CandyMemberObserver {
    Snapshot state_;uint32_t bound_=0;unsigned encounter_=0,member_=0,pending_vi_=0,terminal_call_vi_=0,head_call_vi_=0;
    int before_head_count_=-1;std::array<uint32_t,12> eaten_{};bool head_call_pending_=false;
    ActorEvidence& a(){return state_.actors[0];}const ActorEvidence& a()const{return state_.actors[0];}
    void fail(std::string reason){state_.invalidated=true;state_.failure=std::move(reason);}
    bool sample(uint8_t* rdram){
        auto& e=a();if(!boss_native::actor(rdram,bound_,0x455A)){fail("candy_member_identity_changed");return false;}
        const auto p=boss_native::word(rdram,bound_+0x1F4);if(!boss_native::range(p,0x24))return false;
        if(e.private_data&&e.private_data!=p){fail("candy_member_private_changed");return false;}e.private_data=p;
        const auto head=boss_native::word(rdram,p);
        if(!boss_native::actor(rdram,head,0x454F)||boss_native::word(rdram,head+0x180)!=bound_)return false;
        const auto hp=boss_native::word(rdram,head+0x1F4);if(!boss_native::range(hp,0x88))return false;
        if((e.head&&e.head!=head)||(e.head_private&&e.head_private!=hp)){fail("candy_head_identity_changed");return false;}
        e.head=head;e.head_private=hp;const int value=int32_t(boss_native::word(rdram,hp+0x84));
        if(value<0||value>12||(e.counter>=0&&value>e.counter)){fail("candy_native_segments_invalid");return false;}
        if(e.initial_counter<0){if(value!=12){fail("candy_initial_twelve_missing");return false;}
            std::array<uint32_t,12> initial{};for(unsigned i=0;i<12;++i){initial[i]=boss_native::word(rdram,hp+0x54+i*4);if(!boss_native::actor(rdram,initial[i],0x4550)){fail("candy_initial_segment_identity_invalid");return false;}for(unsigned j=0;j<i;++j)if(initial[j]==initial[i]){fail("candy_initial_segment_alias");return false;}}
            e.initial_counter=12;}e.counter=value;return true;
    }
    bool room(const Frame& f){if(f.source_room!=state_.source_room){fail("left_candy_member_arena");return false;}return true;}
    bool terminal(uint8_t* rdram)const{return a().counter==0&&a().causal_hits==12&&a().active_seen&&boss_native::word(rdram,a().private_data+0x20)==1;}
    DefeatObservation proof()const{DefeatObservation p{state_.run_token,Family::candy,state_.source_room,a().completion_vi,bound_,a().player,a().transition,12,0,state_.diagnostic};p.encounter=encounter_;p.member=member_;p.causal_hits=a().causal_hits;p.cinematic_suppressed=a().cinematic_suppressed;return p;}
public:
    bool configure(const boss_battle0250::Encounter& e,unsigned member,std::string token,unsigned source_room,bool diagnostic=true){
        state_={};bound_=0;pending_vi_=terminal_call_vi_=head_call_vi_=0;before_head_count_=-1;head_call_pending_=false;eaten_={};
        if(!e.number||e.number>10||member>=e.count||e.count>4||source_room>255||e.members[member].kind!=0x455A||e.members[member].hits!=12||token.empty()||token.size()>128)return false;
        for(unsigned char c:token)if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'))return false;
        state_.enabled=true;state_.family=Family::candy;state_.source_room=source_room;state_.expected_actors=1;state_.expected_hits=12;state_.run_token=std::move(token);state_.diagnostic=diagnostic;encounter_=e.number-1;member_=member;return true;
    }
    bool bind(uint32_t actor){if(!state_.enabled||bound_||!boss_native::range(actor,0x1FC))return false;bound_=actor;return true;}
    uint32_t bound_actor()const{return bound_;}
    const Snapshot& snapshot()const{return state_;}
    void enter_room(unsigned source_room){if(state_.enabled&&source_room!=state_.source_room)fail("left_candy_member_arena");}
    void forget(uint32_t actor){if(state_.enabled&&actor==bound_&&a().initialized){if(!a().completion_returned)fail("candy_member_disposed_before_proof");else a().disposed=true;}}
    void frame(uint8_t* rdram,const Frame& f){if(state_.enabled&&!state_.invalidated&&rdram&&room(f)&&a().initialized&&!a().completion_returned)sample(rdram);}
    void observe_entry(uint8_t* rdram,const recomp_context* c,std::string_view name,const Frame& f){
        if(!state_.enabled||state_.invalidated||!bound_||!rdram||!c||!room(f))return;const auto arg=uint32_t(c->r4);auto& e=a();
        if(name=="func_80664E18"&&arg==bound_&&boss_native::actor(rdram,arg,0x455A)){
            if(e.initialized){fail("candy_member_reinitialized");return;}e.initialized=true;e.actor=bound_;e.initial_vi=f.vi;state_.registered_actors=1;return;
        }
        if(!e.initialized||e.completion_returned)return;
        if(name=="func_806657A0"&&arg==bound_&&sample(rdram)&&boss_native::byte(rdram,arg+0x18)==1&&boss_native::byte(rdram,arg+0x30)==1&&boss_native::word(rdram,e.private_data+0x20)==0){e.active_seen=true;if(!e.active_vi)e.active_vi=f.vi;}
        // Native67024 updates the head during intro too.66ACC removes eaten64
        // segments without a root-state gate; all12 can disappear before57A0.
        // Bind both real arguments and verify the original return's effect.
        if(name=="func_80666ACC"&&sample(rdram)&&arg==e.head_private&&uint32_t(c->r5)==e.head){
            if(head_call_pending_){fail("overlapping_candy_head_update");return;}head_call_pending_=true;head_call_vi_=f.vi;before_head_count_=e.counter;eaten_={};
            for(unsigned i=0;i<12;++i){const auto segment=boss_native::word(rdram,arg+0x54+i*4);if(boss_native::actor(rdram,segment,0x4550)&&boss_native::byte(rdram,segment+0x18)==0x64)eaten_[i]=segment;}
        }
        if(name=="func_80665C80"&&arg==bound_&&sample(rdram)&&terminal(rdram)&&boss_native::byte(rdram,arg+0x18)==1&&boss_native::byte(rdram,arg+0x30)==8){e.terminal_seen=true;if(!e.terminal_vi)e.terminal_vi=f.vi;if(!boss_native::word(rdram,arg+0x1E0))terminal_call_vi_=f.vi;}
        if(name=="func_807AEA78"&&e.terminal_seen&&terminal_call_vi_==f.vi&&terminal(rdram)&&candy_owner_from_completion(rdram,c,f)==bound_&&uint32_t(c->r5)==0&&boss_native::range(f.scene_overlay,0x2800)&&boss_native::word(rdram,0x800F8D78)==f.scene_overlay+0x2418&&active_player(rdram,arg)&&boss_native::word(rdram,0x800F8828)==arg){
            if(e.completion_called){fail("duplicate_candy_native_completion");return;}e.completion_called=true;e.player=arg;pending_vi_=f.vi;
        }
        if(name=="func_807AEA24"&&completion_pending()&&pending_vi_==f.vi&&uint32_t(c->r31)==f.scene_overlay+0x23AC&&uint32_t(c->r5)==1&&uint32_t(c->r6)==0&&boss_native::actor(rdram,arg,0x46EE)&&boss_native::range(boss_native::word(rdram,arg+0x1F4),4)){
            if(e.allocation_seen){fail("duplicate_candy_cinematic");return;}e.allocation_seen=true;e.transition=arg;
        }
    }
    void observe_hit_return(uint8_t* rdram,std::string_view name,const Frame& f){
        if(!state_.enabled||state_.invalidated||!head_call_pending_||name!="func_80666ACC"||!rdram)return;head_call_pending_=false;
        if(!room(f)||head_call_vi_!=f.vi||!sample(rdram)){fail("candy_head_return_scope_mismatch");return;}
        unsigned removed=0;for(unsigned i=0;i<12;++i)if(eaten_[i]&&!boss_native::word(rdram,a().head_private+0x54+i*4))++removed;
        if(a().counter!=before_head_count_-int(removed)){fail("candy_segments_lack_native_eaten_cause");return;}
        a().causal_hits+=removed;if(a().causal_hits>12)fail("too_many_candy_native_segments");
        // Successful native ingestion is combat activity even if its first
        // optional57A0 callback has not run. An unchanged return proves none.
        if(removed){a().active_seen=true;if(!a().active_vi)a().active_vi=f.vi;}
    }
    bool completion_pending()const{return state_.enabled&&!state_.invalidated&&a().completion_called&&!a().completion_returned;}
    bool acknowledge_suppressed_completion(uint8_t* rdram,const Frame& f){
        if(!completion_pending()||!rdram||!room(f)||pending_vi_!=f.vi||a().allocation_seen||!active_player(rdram,a().player))return false;
        a().completion_returned=true;a().cinematic_suppressed=true;a().completion_vi=f.vi;return true;
    }
    void completion_return(uint8_t* rdram,const recomp_context* c,const Frame& f){
        if(!completion_pending()||!rdram||!c)return;auto& e=a();
        if(!room(f)||pending_vi_!=f.vi||!e.allocation_seen||uint32_t(c->r2)!=e.transition||!boss_native::actor(rdram,e.transition,0x46EE)||!active_player(rdram,e.player)||boss_native::word(rdram,0x800F8828)!=e.player){fail("candy_cinematic_return_rejected");return;}
        const auto p=boss_native::word(rdram,e.transition+0x1F4);if(!boss_native::range(p,4)||boss_native::word(rdram,p)!=0){fail("candy_cinematic_type_mismatch");return;}
        e.completion_returned=true;e.completion_vi=f.vi;
    }
    std::optional<DefeatObservation> take_defeat_observation(){if(!state_.enabled||state_.invalidated||!a().completion_returned||a().delivered)return std::nullopt;a().delivered=true;return proof();}
};
}
