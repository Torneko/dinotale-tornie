#pragma once
#include "story_observer0320.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <optional>

namespace workshop::story_hard_native0330 {
inline constexpr unsigned mode=6,all_colors=0x3FFFF,none=18;
struct Team {unsigned found=1,alive=1;};
using NativeContinuation14=std::array<uint8_t,14>;
inline std::optional<NativeContinuation14> read_continuation(uint8_t* rdram){
    if(!rdram)return {};NativeContinuation14 result{};
    for(unsigned i=0;i<result.size();i++)result[i]=MEM_BU(i,gpr(int32_t(0x800F9248)));
    return result;
}
constexpr bool valid(Team team){return team.found<=all_colors&&team.alive<=all_colors&&(team.found&1)&&!(team.alive&~team.found);}
constexpr unsigned dead(Team team){return valid(team)?team.found&~team.alive&all_colors:0;}
// Ordinal comes from the caller's existing RNG. No implicit global pool,
// native aliases, newly owned color or journal mutation is introduced here.
constexpr unsigned rescue_pick(Team team,unsigned ordinal){
    const auto candidates=dead(team);
    if(ordinal>=unsigned(std::popcount(candidates)))return none;
    for(unsigned color=0;color<18;color++)if(candidates&(1u<<color)){if(!ordinal)return color;--ordinal;}
    return none;
}
// Game-thread adapter only. Scope must exclude custom/diagnostic/deferred
// modes before start. It owns no journal, filesystem, RNG or global service.
// Team and run nonce are supplied by the independent Hard journal owner.
class Adapter {
    bool active_=false;
    uint64_t nonce_=0;
    story_observer0320::NativeObserver world_;
    struct Pending {bool armed=false;uint64_t nonce=0;unsigned color=none,before=0,after=0;uint32_t stack=0,player=0,data=0;};
    Pending loss_{},rescue_{};
    static bool range(uint32_t p,unsigned bytes){return !(p&3)&&p>=0x80000400&&p<0x80800000&&bytes<=0x80800000-p;}
    static gpr at(uint32_t p){return gpr(int32_t(p));}
    static uint32_t word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,at(p)));}
    static void word(uint8_t* rdram,uint32_t p,uint32_t value){MEM_W(0,at(p))=value;}
    static unsigned ram_team(uint8_t* rdram){return (word(rdram,0x800F8DF4)&255)|((word(rdram,0x807F1000)&1023)<<8);}
    static bool player_identity(uint8_t* rdram,uint32_t player,uint32_t data){return range(player,0x200)&&range(data,0x1300)&&word(rdram,player)==0x19971110&&MEM_HU(12,at(player))==0x4001&&word(rdram,player+0x1F4)==data&&word(rdram,0x800F8828)==player;}
    void clear_pending(){loss_={};rescue_={};}
public:
    void start(unsigned selected_mode,bool ordinary_scope,uint64_t nonce){reset();active_=selected_mode==mode&&ordinary_scope&&nonce!=0;nonce_=active_?nonce:0;world_.start(active_?5:0,active_,nonce_);}
    void reset(){active_=false;nonce_=0;clear_pending();world_.reset();}
    bool enabled(uint8_t* rdram)const{return active_&&rdram&&word(rdram,0x800F8B88)==0;}
    static bool continuing(uint8_t* rdram){return rdram&&(MEM_HU(0,at(0x800F9254))&7)&&(MEM_BU(0,at(0x800F9253))&8);}
    // BEFORE807F3D98, after native write's IO_WAIT and boolean normalization.
    // r2=0 means success; early pack/codec failures return1 at this same fence.
    std::optional<NativeContinuation14> saved_continuation(uint8_t* rdram,const recomp_context* context)const{
        if(!context||!enabled(rdram)||context->r2!=0||!range(uint32_t(context->r29),0x20))return {};
        return read_continuation(rdram);
    }
    std::optional<unsigned> available(uint8_t* rdram,Team team)const{if(!enabled(rdram)||!valid(team))return {};return team.alive;}
    std::optional<unsigned> saved_extras(uint8_t* rdram,Team team)const{if(!enabled(rdram)||!valid(team))return {};return (team.alive>>8)&1023;}
    std::optional<unsigned> capacity(uint8_t* rdram,Team team)const{if(!enabled(rdram)||!valid(team))return {};const auto pending=(word(rdram,0x800F8DF4)>>14)&7;return pending<unsigned(std::popcount(dead(team)));}
    std::optional<unsigned> picker(uint8_t* rdram,Team team,unsigned ordinal)const{if(!enabled(rdram)||!valid(team))return {};return rescue_pick(team,ordinal);}
    bool project(uint8_t* rdram,Team team)const{
        if(!enabled(rdram)||!valid(team))return false;
        const auto flags=word(rdram,0x800F8DF4);
        word(rdram,0x800F8DF4,(flags&~255u)|(team.alive&255));
        word(rdram,0x807F1000,(team.alive>>8)&1023);return true;
    }
    // BEFORE80444838, after native reconstruction of base lives/continuation.
    // Caller decides fresh/new_run versus continuing using its journal; this
    // projection alone never resets progression or grants global ownership.
    bool init_projection(uint8_t* rdram,const recomp_context* context,Team team)const{
        return context&&uint32_t(context->r3)==0x800F7EA0&&project(rdram,team);
    }
    // BEFORE807F1128. r9 is the exact PC color just loaded from807F0008;
    // r4 remains the genuine dying4001. Returntrue means gotoL_807F1174,
    // preserving native scene/audio/counters while avoiding alias BW losses.
    bool loss_enter(uint8_t* rdram,recomp_context* context,Team team){
        loss_={};if(!context||!enabled(rdram)||!valid(team)||ram_team(rdram)!=team.alive)return false;
        const unsigned color=uint32_t(context->r9);const auto player=uint32_t(context->r4),stack=uint32_t(context->r29);
        if(color>=18||word(rdram,0x807F0008)!=color||!(team.alive&(1u<<color))||!range(stack,0x18)||!range(player,0x200))return false;
        const auto data=word(rdram,player+0x1F4);if(!player_identity(rdram,player,data))return false;
        const auto after=team.alive&~(1u<<color);
        loss_={true,nonce_,color,team.alive,after,stack,player,data};
        if(!project(rdram,{team.found,after})){loss_={};return false;}
        context->r10=(after>>8)&1023;context->r12=word(rdram,0x800F8DF4);return true;
    }
    // BEFORE807F1174, after either the native clear or the exact Hard clear.
    // Root persists lose_yoshi ONLY for this matching receipt. Remaining-life
    // code still needs its existing metal-remaining hook to retain bits14/15.
    std::optional<unsigned> loss_finish(uint8_t* rdram,const recomp_context* context){
        const auto pending=loss_;loss_={};
        if(!context||!pending.armed||!enabled(rdram)||pending.nonce!=nonce_||uint32_t(context->r29)!=pending.stack||uint32_t(context->r4)!=pending.player||word(rdram,0x807F0008)!=pending.color||!player_identity(rdram,pending.player,pending.data)||ram_team(rdram)!=pending.after)return {};
        return pending.color;
    }
    // BEFORE807F1C20: native picker has returnedPC0..15, before the actual
    // 1C34..1C58 OR into base/extra life storage. Never restore an unfound color.
    bool rescue_enter(uint8_t* rdram,const recomp_context* context,Team team){
        rescue_={};if(!context||!enabled(rdram)||!valid(team)||ram_team(rdram)!=team.alive)return false;
        const unsigned color=uint32_t(context->r2);const auto stack=uint32_t(context->r29);
        if(color>=18||!(dead(team)&(1u<<color))||!range(stack,0x18)||uint32_t(context->r8)!=0x80100000||uint32_t(context->r10)!=color+1)return false;
        rescue_={true,nonce_,color,team.alive,team.alive|(1u<<color),stack,0,0};return true;
    }
    // BEFORE807F1C5C: the real native OR has happened, while its subsequent
    // White Shy Guy counter/animation bookkeeping is untouched. This receipt
    // restores local alive state only; everLost and global rights stay withRoot.
    std::optional<unsigned> rescue_finish(uint8_t* rdram,const recomp_context* context){
        const auto pending=rescue_;rescue_={};
        if(!context||!pending.armed||!enabled(rdram)||pending.nonce!=nonce_||uint32_t(context->r29)!=pending.stack||uint32_t(context->r2)!=pending.color||ram_team(rdram)!=pending.after)return {};
        return pending.color;
    }
    // Frozen0320 observer is composed as a native-Story receipt reader. Its
    // internal mode5 is unreachable from here unless external scope isPC6.
    void frame(uint8_t* rdram,const recomp_context* context){if(enabled(rdram))world_.frame(rdram,context);}
    void arm_world_save(uint8_t* rdram,const recomp_context* context){if(enabled(rdram))world_.arm_world_save(rdram,context);}
    std::optional<unsigned> finish_world_save(uint8_t* rdram,const recomp_context* context){if(!enabled(rdram))return {};return world_.finish_world_save(rdram,context);}
    void arm_bowser(uint8_t* rdram,const recomp_context* context){if(enabled(rdram))world_.arm_bowser(rdram,context);}
    std::optional<unsigned> finish_bowser(uint8_t* rdram,const recomp_context* context){if(!enabled(rdram))return {};return world_.finish_bowser(rdram,context);}
    story_observer0320::Witness witness()const{return world_.witness();}
};
}
