#include "../boss_battle_runtime0250.hpp"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace r=workshop::boss_campaign0250;
namespace b=workshop::boss_native;
namespace c=workshop::boss_battle0250;
static unsigned checks=0,native_calls=0,dispatch_calls=0,hook_calls=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static void forbidden(){++native_calls;throw std::runtime_error("Death-relay CPU fixture must not call native gameplay");}
extern "C" void func_80047090(uint8_t*,recomp_context*){forbidden();}
extern "C" void func_8004613C(uint8_t*,recomp_context*){forbidden();}
extern "C" void func_80044194(uint8_t*,recomp_context*){forbidden();}
extern "C" void func_80040C0C(uint8_t*,recomp_context*){forbidden();}
static void dispatch(uint32_t,uint8_t*,recomp_context*){++dispatch_calls;throw std::runtime_error("Unexpected native dispatch in death-relay CPU fixture");}
static void completed(uint8_t*){++hook_calls;throw std::runtime_error("Death/nonfatal guard must never complete or award");}

struct Fixture {
    std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);
    uint8_t* rdram=memory.data();recomp_context ctx{};
    static constexpr uint32_t player=0x80010000,pd=0x80011000,pot=0x80014000;
    void word(uint32_t a,uint32_t v){MEM_W(0,b::base(a))=int32_t(v);}
    void half(uint32_t a,unsigned v){MEM_H(0,b::base(a))=uint16_t(v);}
    void byte(uint32_t a,unsigned v){MEM_B(0,b::base(a))=uint8_t(v);}
    explicit Fixture(c::Phase target,unsigned hp=0){
        need(r::configure("synthetic-death-relay-qa-0260",20261031,true,false),"Explicit synthetic QA configuration");
        r::completion_hook=completed;r::variants::disable();
        word(player,0x19971110);half(player+12,0x4001);word(player+0x1F4,pd);word(0x800F8828,player);
        word(0x800F8DB0,0);word(0x800F8DB4,hp);word(0x807F0008,0);byte(0x800F7EA6,1);
        // All phase transitions below are host-model fixtures. No native room,
        // collision, victory or pot is executed and no ledger is opened.
        need(r::session.enter_fight(162,player),"Synthetic host-model first encounter");
        if(target!=c::Phase::Fighting){
            constexpr uint32_t synthetic_member=0x80100000;
            need(r::session.bind(0,synthetic_member),"Synthetic member identity bound");
            need(r::session.defeat({0,0,100,synthetic_member,true,true,true,true}),"Explicit synthetic QA receipt only");
            if(target==c::Phase::ToRest||target==c::Phase::Rest||target==c::Phase::ToFight){
                need(r::session.use_victory_pot(),"Host-model victory pot only");
                if(target!=c::Phase::ToRest){need(r::session.arrive_rest(player),"Host-model rest arrival only");if(target==c::Phase::ToFight)need(r::session.use_rest_pot(),"Host-model rest departure only");}
            }
        }
        r::physical_room=r::logical_room=target==c::Phase::Rest?83:162;
        workshop::boss_tree0240::enter_room(r::physical_room);
        r::ready_info=0;r::bound={};r::observers={};r::candies={};r::restore_pending=false;r::exit_pot=pot;
        ctx.r4=b::base(player);ctx.r5=0xA1234567;ctx.r29=b::base(0x800FF000);ctx.r31=b::base(0x800675F0);ctx.f2.u32l=0x12345678;
        need(r::session.snapshot().phase==target&&r::session.snapshot().diagnostic,"Requested phase obtained only through explicit QA model");
        state(1,0x11);
    }
    void state(unsigned primary,unsigned sub){
        byte(player+0x18,primary);byte(player+0x30,sub);byte(player+0x17D,primary==0xA&&sub==2?0:2);byte(player+0x165,primary==0xA&&sub==2?1:0);
        word(player+0x20,primary==0xA&&sub==2?0x80031230:0x80043450);word(pd+0xD0,primary==0xA&&sub==2?pot:0);
    }
    void frame(){r::frame(rdram,&ctx,1000,dispatch);}
    void observe(bool fatal,c::Phase phase){
        const auto ram=memory;const auto saved_ctx=ctx;const auto before=r::session.snapshot();const auto tree=workshop::boss_tree0240::snapshot();
        need(r::fatal_player(rdram,player)==fatal,"Fatality is the exact native A/substate predicate, not HP");
        frame();const auto& after=r::session.snapshot();
        need(after.phase==(fatal?c::Phase::Failed:phase),"Fatal state relayed before rest83 return and native dispatch; nonfatal phase retained");
        need(r::failed.load()==fatal&&r::active()==!fatal,"UI failure signal agrees with host phase");
        need(after.player==player&&after.index==before.index&&after.completed==before.completed,"Guard retains player, encounter and proven count");
        need(!fatal||after.failure=="yoshi_defeated","Fatal native state supplies existing failure reason");
        need(!r::finished.load()&&!r::completion_pending&&hook_calls==0,"Guard never schedules completion, hook or reward");
        need(memory==ram&&std::memcmp(&ctx,&saved_ctx,sizeof(ctx))==0,"Full8MiB and all caller registers remain unchanged");
        const auto& now=workshop::boss_tree0240::snapshot();
        need(now.totalConsumed==tree.totalConsumed&&now.regenerations==tree.regenerations&&now.consumedmask==tree.consumedmask&&!now.invalidated,"Empty/uninitialized rest-tree fixture remains unchanged");
        if(fatal){frame();need(r::session.snapshot().phase==c::Phase::Failed&&memory==ram&&std::memcmp(&ctx,&saved_ctx,sizeof(ctx))==0,"Repeated fence cannot revive, mutate or award after death");}
    }
};

int main(int argc,char** argv){try{
    if(argc!=2)throw std::runtime_error("Explicit NEW CPU report path required");const std::filesystem::path report=argv[1];if(std::filesystem::exists(report))throw std::runtime_error("Refusing to replace earlier CPU report");
    for(auto phase:{c::Phase::Fighting,c::Phase::Victory,c::Phase::Rest})for(unsigned hp:{0u,7u}){
        for(unsigned sub=0;sub<0x14;++sub){Fixture f(phase,hp);f.state(0xA,sub);const bool fatal=sub==0xA||sub==0xB||sub==0xC||sub==0xD||sub==0x10||sub==0x11;f.observe(fatal,phase);}
        for(auto state:std::array<std::pair<unsigned,unsigned>,3>{{{1,0x11},{9,0xE},{1,2}}}){Fixture f(phase,hp);f.state(state.first,state.second);f.observe(false,phase);}
        // Exact native transport posture remains nonfatal even with an empty
        // flower. It is never reclassified as a death by the expanded phases.
        {Fixture f(phase,hp);f.state(0xA,2);need(b::word(f.rdram,Fixture::player+0x20)==0x80031230&&b::byte(f.rdram,Fixture::player+0x165)==1,"A/2 transport callback/hidden fixture exact");f.observe(false,phase);}
    }
    for(auto phase:{c::Phase::ToRest,c::Phase::ToFight}){
        Fixture f(phase,0);f.state(0xA,2);r::physical_room=83;f.observe(false,phase);
    }
    for(auto phase:{c::Phase::Fighting,c::Phase::Victory,c::Phase::Rest}){
        Fixture f(phase);f.state(0xA,0x11);f.half(Fixture::player+12,0x4002);const auto ram=f.memory;const auto ctx=f.ctx;
        need(!r::fatal_player(f.rdram,Fixture::player),"Non4001 actor cannot be classified as defeated Yoshi");f.frame();need(r::session.snapshot().phase==phase&&!r::failed.load()&&f.memory==ram&&std::memcmp(&ctx,&f.ctx,sizeof(ctx))==0,"Wrong actor kind cannot spuriously fail or mutate runtime");
    }
    need(native_calls==0&&dispatch_calls==0&&hook_calls==0,"No native execution, dispatch or completion hook in any synthetic CPU case");
    std::ofstream out(report,std::ios::binary);out<<"{\n  \"passed\": true,\n  \"checks\": "<<checks<<",\n  \"scope\": \"Synthetic QA host phases and native-shaped state bytes only; not native death/gameplay/GPU proof\",\n  \"fatalPrimaryAStates\": [10,11,12,13,16,17],\n  \"phases\": [2,3,5],\n  \"hpZeroAndSeven\": true,\n  \"normalStatesAndA2TransportRemainNonfatal\": true,\n  \"rest83ReturnCannotHideFatality\": true,\n  \"ramAndRegistersUnchanged\": true,\n  \"nativeCalls\": "<<native_calls<<",\n  \"nativeDispatches\": "<<dispatch_calls<<",\n  \"completionHooks\": "<<hook_calls<<",\n  \"gameLaunched\": false,\n  \"ledgerWritten\": false,\n  \"personalProfileUsed\": false,\n  \"nativeTenFightRunVerified\": false\n}\n";if(!out)throw std::runtime_error("CPU report write failed");std::cout<<"BossDeathRelay0260Checks: "<<checks<<" PASS; synthetic QA only, zero native calls/game/ledger.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
