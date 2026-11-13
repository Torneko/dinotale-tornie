#pragma once
#include "solo_inventory0300.hpp"
#include "heart_wallet0283.hpp"
#include "heart_sources0283.hpp"
#include <atomic>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <limits>
#include <sstream>
#include <unordered_map>

namespace workshop::starting_bonus0283 {
namespace iv=solo_inventory0300;
namespace n=boss_native;
namespace wallet=heart_wallet0283;
enum class Mode {None,Story,Trial,Hybrid,BossBattle,Practice,Custom};
enum class Phase {Idle,WaitingEvidence,WaitingPlayer,Applying,AppliedAwaitingCommit,Complete,Failed,WaitingGameStage};
struct Environment {
    Mode mode=Mode::None;
    unsigned vi=0;
    bool custom=false,boss_armed=false;
    unsigned boss_index=10,retry_epoch=0;
};
struct State {
    Phase phase=Phase::Idle;
    Mode mode=Mode::None;
    unsigned level=24,source=177,scene6=0,selection_vi=0,armed_vi=0,entry_vi=0,ready_vi=0,applied_vi=0,added=0;
    uint32_t selection_owner=0,selection_ready=0,entry_owner=0,ready_info=0,player=0,private_data=0;
    uint64_t arm_birth=0,player_birth=0;
    wallet::Pending requested{};
    std::string attempt,reason;
};
inline bool allowed(Mode mode){return mode==Mode::Story||mode==Mode::Trial||mode==Mode::Hybrid||mode==Mode::BossBattle;}
inline uint64_t fresh_session(){
    static std::atomic_uint64_t sequence{0};
    return uint64_t(std::chrono::steady_clock::now().time_since_epoch().count())+sequence.fetch_add(1);
}
inline std::string attempt_token(uint64_t session,uint64_t attempt){
    std::ostringstream token;token<<std::hex<<std::setfill('0')<<std::setw(16)<<session<<std::setw(16)<<attempt;return token.str();
}
struct NativePlan {
    iv::Payload before{},target{};
    iv::Inventory original{};
    uint32_t stack=0;
    wallet::Pending requested{};
    bool solo_extended=false;
};
inline bool released(uint8_t* ram,uint32_t player){
    if(!ram||!n::actor(ram,player,0x4001)||n::word(ram,0x800F8828)!=player||
       n::byte(ram,player+0x17D)!=2||n::byte(ram,player+0x165)||n::word(ram,player+0x180))return false;
    const auto state=n::byte(ram,player+0x18);
    if(!state||state==0xA||state==0xFF)return false;
    const auto data=n::word(ram,player+0x1F4);
    return iv::pointer(data,0x12F0)&&!n::word(ram,data+0x12EC)&&!n::byte(ram,0x800F7EA5)&&n::byte(ram,0x800F7EA6)<=1;
}
// No mutation and no wallet consumption. Uses the actual selected P1/native
// alias, exact FIFO/graph, native scale recipe, and bounded caller scratch.
inline bool prepare(uint8_t* ram,const recomp_context* context,uint32_t player,unsigned vi,wallet::Pending pending,NativePlan& out,bool solo_extended=false){
    if(!context||!vi||pending.empty()||!released(ram,player))return false;
    NativePlan result;result.solo_extended=solo_extended;result.requested=pending;result.stack=uint32_t(context->r29);
    if(result.stack<0x80000800||!iv::pointer(result.stack-0x400,0x480)||!iv::capture(ram,player,vi,result.before)||!iv::read(ram,player,result.original))return false;
    result.target=result.before;
    if(pending.life)result.target.hp=wallet::start_life;
    if(pending.eggs){
        if(iv::half(ram,player+0x184)+(wallet::start_eggs-result.original.count)>20)return false;
        iv::Scale scale{};
        if(result.original.count<wallet::start_eggs&&!iv::normal_scale(ram,player,scale))return false;
        result.target.count=wallet::start_eggs;
        for(unsigned i=result.original.count;i<wallet::start_eggs;++i)result.target.eggs[i]={0x4004,yoshi_colors0300::elemental(result.target.pc_color)?yoshi_colors0300::native_alias(result.target.pc_color):result.target.pc_color,scale};
    }
    if(!iv::valid(result.target))return false;
    if(!solo_extended&&(!boss_checkpoint_inventory0260::valid(result.before)||!boss_checkpoint_inventory0260::valid(result.target)))return false;
    out=result;return true;
}
inline bool apply_native(uint8_t* ram,recomp_context* context,const NativePlan& plan){
    if(!context||uint32_t(context->r29)!=plan.stack||!iv::matches(ram,plan.before))return false;
    if(!plan.solo_extended&&(!boss_checkpoint_inventory0260::valid(plan.before)||!boss_checkpoint_inventory0260::valid(plan.target)))return false;
    struct Scratch {
        uint8_t* ram;uint32_t address;recomp_context* context;recomp_context saved;
        std::array<uint8_t,0x480> bytes{};
        Scratch(uint8_t* r,uint32_t sp,recomp_context* c):ram(r),address(sp-0x400),context(c),saved(*c){std::memcpy(bytes.data(),r+(address&0x7FFFFF),bytes.size());}
        ~Scratch(){std::memcpy(ram+(address&0x7FFFFF),bytes.data(),bytes.size());*context=saved;}
    } scratch(ram,plan.stack,context);
    context->r29=ADD32(context->r29,-0x100);
    for(unsigned i=plan.original.count;i<plan.target.count;++i){
        iv::Inventory before;
        if(!iv::same_player(ram,plan.target)||!iv::read(ram,plan.target.player,before)||before.count!=i)return false;
        const auto scale=uint32_t(context->r29)+0xC0;
        if(!iv::stage_scale(ram,scale,plan.target.eggs[i].saved_scale))return false;
        context->r4=n::base(plan.target.player);context->r5=n::base(plan.target.player+0x84);context->r6=n::base(scale);func_80040860(ram,context);
        iv::Inventory after;if(!iv::read(ram,plan.target.player,after)||after.count!=i+1)return false;
        for(unsigned j=0;j<i;++j)if(after.actors[j]!=before.actors[j]||after.eggs[j]!=before.eggs[j])return false;
        if(after.eggs[i].kind!=0x4004)return false;
        context->r4=n::base(after.actors[i]);context->r5=plan.target.eggs[i].palette;func_8003B8B0(ram,context);
        if(!iv::read(ram,plan.target.player,after)||after.count!=i+1||after.eggs[i]!=plan.target.eggs[i])return false;
    }
    if(!iv::same_player(ram,plan.target))return false;
    if(plan.requested.life&&plan.before.hp!=plan.target.hp){context->r4=plan.target.hp;func_800422CC(ram,context);}
    return iv::matches(ram,plan.target);
}
class Controller {
    State state_{};
    uint64_t session_=0,attempts_=0,birth_serial_=0;
    std::unordered_map<uint32_t,uint64_t> births_;
    bool scene_seen_=false;
    unsigned previous_stage_=0,last_commit_vi_=0;
    std::atomic_bool unsettled_{false};
    void cancel(const char* why){state_.phase=Phase::Idle;state_.reason=why;}
    bool mode_scope(uint8_t* ram,const Environment& env)const{
        if(!ram||env.custom||!allowed(env.mode)||env.mode!=state_.mode)return false;
        if(env.mode==Mode::BossBattle)return env.boss_armed&&env.boss_index==0&&!env.retry_epoch;
        const auto mode=n::word(ram,0x800F8B88);
        return env.mode==Mode::Story?mode==0:env.mode==Mode::Trial?mode==1:(mode==0||mode==1);
    }
    bool same_level(uint8_t* ram)const{
        const auto page=n::byte(ram,0x800F8DFC),column=n::byte(ram,0x800F8DFD);
        return page<6&&column<4&&page*4+column==state_.level;
    }
    bool scope(uint8_t* ram,const Environment& env)const{
        return mode_scope(ram,env)&&same_level(ram)&&n::byte(ram,0x800F7EA2)==7&&n::byte(ram,0x800F8424)==state_.source;
    }
    void commit(wallet::Wallet& purse,unsigned vi){
        if(state_.phase!=Phase::AppliedAwaitingCommit)return;
        if(last_commit_vi_&&vi>=last_commit_vi_&&vi-last_commit_vi_<60)return;
        last_commit_vi_=vi;
        const auto result=purse.consume_pending(state_.attempt,state_.requested);
        if(result.committed()||result.status==wallet::Status::AlreadyConsumed){state_.phase=Phase::Complete;state_.reason.clear();unsettled_=false;}
        else state_.reason="wallet-consume-awaiting-commit";
    }
public:
    explicit Controller(uint64_t session=fresh_session()):session_(session){}
    // Game-thread only. UI may read unsettled(), which is atomic.
    const State& snapshot()const{return state_;}
    bool unsettled()const{return unsettled_.load();}
    // Stage6/scene2 is Yoshi selection, whose room2 has no4001 placement.
    // Only its completed selection metadata followed by a real6->7/root edge
    // arms a game attempt. Native804A2714 reads the page/column entrance table
    // and requests7/root; stage7 scene is F8424, not the stage6 field F8433.
    void observe_scene(uint8_t* ram,const Environment& env){
        if(!ram)return;const unsigned stage=n::byte(ram,0x800F7EA2);
        if(!scene_seen_){scene_seen_=true;previous_stage_=stage;return;}
        const unsigned previous=previous_stage_;previous_stage_=stage;
        if(state_.phase==Phase::Applying||state_.phase==Phase::AppliedAwaitingCommit)return;
        if(stage==6&&previous!=6&&n::byte(ram,0x800F8433)==2&&!env.custom&&allowed(env.mode)){
            if(env.mode==Mode::BossBattle&&(!env.boss_armed||env.boss_index!=0||env.retry_epoch))return;
            const unsigned page=n::byte(ram,0x800F8DFC),column=n::byte(ram,0x800F8DFD);
            if(page>=6||column>=4)return;
            state_={};state_.phase=Phase::WaitingGameStage;state_.mode=env.mode;state_.level=page*4+column;
            state_.source=env.mode==Mode::BossBattle?52:heart_sources0283::sources[state_.level*3].root;
            state_.scene6=2;state_.selection_vi=env.vi;
            if(!mode_scope(ram,env))cancel("selection-mode-mismatch");
            return;
        }
        if(state_.phase==Phase::WaitingGameStage){
            if(!mode_scope(ram,env)||!same_level(ram)){cancel("left-yoshi-selection-scope");return;}
            if(stage==6&&n::byte(ram,0x800F8433)==2)return;
            if(stage==7&&previous==6&&state_.selection_ready&&n::byte(ram,0x800F8424)==state_.source&&attempts_!=std::numeric_limits<uint64_t>::max()){
                state_.phase=Phase::WaitingEvidence;state_.armed_vi=env.vi;state_.arm_birth=birth_serial_;state_.attempt=attempt_token(session_,++attempts_);return;
            }
            cancel("selection-did-not-enter-initial-game");return;
        }
        if((state_.phase==Phase::WaitingEvidence||state_.phase==Phase::WaitingPlayer)&&!scope(ram,env))cancel("left-initial-attempt-scope");
    }
    // Type is not initialized yet at45CAC: count every new actor incarnation,
    // then prove at the safe fence that the current singleton is new4001.
    void created(uint32_t actor){
        if(!iv::pointer(actor,0x1FC)||birth_serial_==std::numeric_limits<uint64_t>::max())return;
        births_[actor]=++birth_serial_;
    }
    void forget(uint32_t actor){births_.erase(actor);}
    void trace(uint8_t* ram,const recomp_context* context,std::string_view name,const Environment& env){
        observe_scene(ram,env);if(!ram||!context)return;
        if(name=="func_80045CAC"){created(uint32_t(context->r4));return;}
        if(name=="func_800439F4"||name=="func_80045B64"){forget(uint32_t(context->r4));return;}
        if(state_.phase==Phase::WaitingGameStage){
            if(!mode_scope(ram,env)||n::byte(ram,0x800F7EA2)!=6||n::byte(ram,0x800F8433)!=2){cancel("selection-scope-changed");return;}
            if(name=="func_8006DA0C"){
                const auto owner=uint32_t(context->r4);const auto source=unsigned(context->r5)&255;
                if(source!=2||!iv::pointer(owner,0x24)){cancel("unexpected-selection-source");return;}
                state_.selection_owner=owner;
            }else if(name=="func_8006DD34"){
                const auto info=uint32_t(context->r4);
                if(!state_.selection_owner||!iv::pointer(info,0x128)||n::word(ram,state_.selection_owner+0x20)!=info||n::byte(ram,info+1)!=2){cancel("selection-ready-metadata-mismatch");return;}
                state_.selection_ready=info;
            }
            return;
        }
        if(state_.phase!=Phase::WaitingEvidence&&state_.phase!=Phase::WaitingPlayer)return;
        if(!scope(ram,env)){cancel("initial-attempt-scope-changed");return;}
        if(name=="func_8006DA0C"){
            if(state_.ready_info){cancel("left-initial-room");return;}
            const auto owner=uint32_t(context->r4);const auto source=unsigned(context->r5)&255;
            if(source!=state_.source||!iv::pointer(owner,0x24)){cancel("unexpected-initial-source");return;}
            if(state_.entry_owner&&state_.entry_owner!=owner){cancel("initial-entry-owner-changed");return;}
            state_.entry_owner=owner;state_.entry_vi=env.vi;return;
        }
        if(name=="func_8006DD34"){
            const auto info=uint32_t(context->r4);
            if(!state_.entry_owner||!iv::pointer(info,0x128)||n::word(ram,state_.entry_owner+0x20)!=info||n::byte(ram,info+1)!=state_.source){cancel("initial-ready-metadata-mismatch");return;}
            state_.ready_info=info;state_.ready_vi=env.vi;state_.phase=Phase::WaitingPlayer;
        }
    }
    // Root calls only at completed675F0, before campaign::frame/capture_baseline.
    // No return value requests a game/campaign abort. I/O failure merely leaves
    // a host commit pending; native effects are never replayed for that token.
    void frame(uint8_t* ram,recomp_context* context,const Environment& env,wallet::Wallet& purse){
        observe_scene(ram,env);
        if(state_.phase==Phase::AppliedAwaitingCommit){commit(purse,env.vi);return;}
        if(state_.phase!=Phase::WaitingPlayer||!scope(ram,env)||!env.vi)return;
        const auto player=n::word(ram,0x800F8828);const auto found=births_.find(player);
        if(!released(ram,player))return;
        // A playable carried/old incarnation closes this initial window.
        // A later birth cannot turn this room visit into a new attempt.
        if(found==births_.end()||found->second<=state_.arm_birth){cancel("initial-released-player-not-new");return;}
        if(!purse.refresh()){cancel("wallet-not-ready-at-initial-fence");return;}
        const auto view=purse.snapshot();
        if(view.value.last_attempt==state_.attempt){state_.phase=Phase::Complete;return;}
        const auto pending=view.value.pending;
        if(pending.empty()){state_.phase=Phase::Complete;return;}
        NativePlan plan;if(!prepare(ram,context,player,env.vi,pending,plan,env.mode!=Mode::BossBattle)){cancel("native-start-buffers-not-ready-at-initial-fence");return;}
        state_.player=player;state_.private_data=n::word(ram,player+0x1F4);state_.player_birth=found->second;state_.requested=pending;
        state_.added=plan.target.count-plan.original.count;state_.applied_vi=env.vi;state_.phase=Phase::Applying;
        bool applied=false;try{applied=apply_native(ram,context,plan);}catch(...){state_.reason="native-start-bonus-exception";}
        if(!applied){state_.phase=Phase::Failed;if(state_.reason.empty())state_.reason="native-start-bonus-readback-failed";return;}
        state_.phase=Phase::AppliedAwaitingCommit;unsettled_=true;last_commit_vi_=0;commit(purse,env.vi);
    }
};
} // namespace workshop::starting_bonus0283
