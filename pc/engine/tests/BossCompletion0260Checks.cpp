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
static unsigned checks=0,hook_calls=0,native_calls=0,dispatch_calls=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static void forbidden(const char* name){++native_calls;throw std::runtime_error(std::string("Completion CPU fixture unexpectedly called native function ")+name);}
// These four symbols are referenced by the instantiated runtime frame. Every
// call is an error: this test must never implement or fake native gameplay.
extern "C" void func_80047090(uint8_t*,recomp_context*){forbidden("80047090");}
extern "C" void func_8004613C(uint8_t*,recomp_context*){forbidden("8004613C");}
extern "C" void func_80044194(uint8_t*,recomp_context*){forbidden("80044194");}
extern "C" void func_80040C0C(uint8_t*,recomp_context*){forbidden("80040C0C");}
static void dispatch(uint32_t,uint8_t*,recomp_context*){++dispatch_calls;throw std::runtime_error("Unexpected native dispatch in completion-only CPU fixture");}

struct Fixture {
    std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);
    uint8_t* rdram=memory.data();recomp_context ctx{};unsigned color;
    static constexpr uint32_t player=0x80010000,pd=0x80011000,pot=0x80014000,pp=0x80015000,route=0x80016000,wrong_route=0x80017000,egg1=0x80018000,egg2=0x80019000,body=0x8001A000,tongue=0x8001B000,shadow=0x8001C000,popup=0x8001D000,info=0x80022000;
    static constexpr unsigned fence_vi=10001;
    void word(uint32_t a,uint32_t value){MEM_W(0,b::base(a))=int32_t(value);}
    void byte(uint32_t a,unsigned value){MEM_B(0,b::base(a))=uint8_t(value);}
    void half(uint32_t a,unsigned value){MEM_H(0,b::base(a))=uint16_t(value);}
    void actor(uint32_t a,uint16_t kind){word(a,0x19971110);half(a+12,kind);}
    void active_player(){byte(player+0x18,1);byte(player+0x30,2);byte(player+0x17D,2);byte(player+0x165,0);word(player+0x20,0x80043450);}
    void transport(){byte(player+0x18,0xA);byte(player+0x30,2);byte(player+0x17D,0);byte(player+0x165,1);word(player+0x20,0x80031230);word(pd+0xD0,pot);}
    explicit Fixture(unsigned yoshi=10,unsigned hp=7):color(yoshi){
        hook_calls=0;need(r::configure("synthetic-completion-qa-0260",20261031,true,false),"Explicit synthetic QA configuration");r::completion_hook=nullptr;
        r::rooms::configure(r::session.plan());r::variants::disable();
        actor(player,0x4001);word(player+0x1F4,pd);word(0x800F8828,player);const unsigned alias=color<8?color:0;word(0x800F8DB0,alias);word(0x800F8DB4+4*alias,hp);word(0x807F0008,color);word(0x800F89D0,5);byte(0x800F7EA6,1);
        actor(pot,0x4273);word(pot+0x1F4,pp);word(pp+0x260,route);r::exit_pot=pot;
        for(auto [a,kind]:std::array<std::pair<uint32_t,uint16_t>,6>{{{egg1,0x4004},{egg2,0x4004},{body,0x400B},{tongue,0x400C},{shadow,0x4007},{popup,0x4016}}}){actor(a,kind);word(a+0x180,player);}
        for(auto [slot,a]:std::array<std::pair<unsigned,uint32_t>,6>{{{19,egg1},{3,egg2},{0,body},{7,tongue},{12,shadow},{17,popup}}})word(player+0x188+slot*4,a);
        half(player+0x184,6);half(pd+0x12BE,7);byte(pd+0x12C8,2);word(pd+0x11FC+7*24,egg1);word(pd+0x11FC,egg2);
        ctx.r4=b::base(route);ctx.r5=0x12345678;ctx.r6=0x55667788;ctx.r29=b::base(0x800FF000);ctx.r31=b::base(0x80112233);ctx.f2.u32l=0xABCDEF01;active_player();
    }
    void begin_fight(){
        const auto index=r::session.snapshot().index;r::physical_room=r::source_for(index);r::logical_room=c::room(r::session.current().family);r::ready_info=0;r::bound={};r::observers={};r::candies={};r::restore_pending=false;r::flags_ready=false;r::variants::disable();
        // Only a native-shaped RAM/cargo model is provided. No room loading,
        // actor initialization, damage, native transport or gameplay is run.
        active_player();byte(info+1,r::physical_room);const auto before_ctx=ctx;ctx.r4=b::base(info);r::room_ready(rdram,&ctx);ctx=before_ctx;
        const auto before=memory;r::frame(rdram,&ctx,index*1000+1,dispatch);
        need(memory==before&&std::memcmp(&ctx,&before_ctx,sizeof(ctx))==0,"Synthetic fight-entry observer is read-only");
        need(r::session.snapshot().phase==c::Phase::Fighting&&r::history[index].player==player&&r::history[index].color==color,"Runtime captures first/same native-shaped player and color into history");
        need(r::first_player==player&&r::first_color==color,"First player/color identity remains fixed");
    }
    c::NativeReceipt member(unsigned n)const{const auto index=r::session.snapshot().index;return {index,n,index*1000+100+n,0x80100000+index*0x2000+n*0x400,true,true,true,true};}
    void complete_current_model_fight(){
        const auto index=r::session.snapshot().index;
        for(unsigned n=0;n<r::session.current().count;++n){const auto proof=member(n);need(r::session.bind(n,proof.actor),"Bind synthetic member fixture");r::receipt(proof);need(r::history[index].members[n].actor==proof.actor&&r::history[index].members[n].causal_hit,"Only accepted synthetic receipt reaches history");}
        need(r::session.snapshot().phase==c::Phase::Victory&&r::session.snapshot().completed==index+1,"Model counts one complete encounter only after every member");
    }
    void next_model_fight(){
        transport();need(r::capture(rdram,pot),"Strict native-shaped sparse cargo captured for next model entry");
        need(r::session.use_victory_pot()&&r::session.arrive_rest(player)&&r::session.use_rest_pot(),"Synthetic host-model rest progression");
    }
    void through_ten(){for(unsigned i=0;i<10;++i){begin_fight();complete_current_model_fight();if(i<9)next_model_fight();}transport();ctx.r4=b::base(route);r::exit_pot=pot;}
    void fence(){r::frame(rdram,&ctx,fence_vi,dispatch);}
};

static void observe_completion_only(uint8_t* rdram){
    ++hook_calls;need(r::session.snapshot().diagnostic,"Synthetic QA provenance must remain explicit inside completion hook");
    need(r::session.snapshot().phase==c::Phase::Complete&&!r::completion_pending&&!r::finished.load(),"Post-fence hook runs after Complete, pending consumption and before UI finished publication");
    need(r::session.snapshot().completed==10&&r::session.snapshot().index==9,"Hook sees all ten model encounters");
    need(r::cargo.pc_color==r::first_color&&r::cargo.eggs[0]==Fixture::egg1&&r::cargo.eggs[1]==Fixture::egg2,"Hook sees same color and exact wrapped egg identities");
    need(b::word(rdram,0x800F8828)==Fixture::player&&r::session.snapshot().player==r::first_player,"Hook sees original player singleton");
    for(unsigned i=0;i<10;++i){const auto& h=r::history[i];need(h.player==r::first_player&&h.color==r::first_color,"Every history entry keeps player/color");for(unsigned m=0;m<r::session.plan().fights[i].count;++m){const auto& p=h.members[m];need(p.encounter==i&&p.member==m&&p.actor&&p.initialized&&p.active&&p.terminal&&p.causal_hit,"Hook receives all and only accepted model native-receipt-shaped flags");}}
    // Deliberately no call to boss_challenges0260::record or any ledger/file API.
}

int main(int argc,char** argv){try{
    if(argc!=2)throw std::runtime_error("Explicit NEW CPU report destination required");const std::filesystem::path report=argv[1];if(std::filesystem::exists(report))throw std::runtime_error("Refusing to replace an earlier CPU report");
    for(unsigned color=0;color<14;++color)for(unsigned hp:{0u,7u}){
        Fixture f(color,hp);f.through_ten();r::completion_hook=observe_completion_only;const auto memory=f.memory;const auto ctx=f.ctx;
        need(r::session.snapshot().diagnostic&&hook_calls==0&&!r::finished.load()&&!r::completion_pending,"Ten synthetic victories alone do not publish a result");
        need(r::pot_complete(f.rdram,&f.ctx)&&r::session.snapshot().phase==c::Phase::Complete,"Bound final native-shaped pot is required for Complete");
        need(r::completion_pending&&!r::finished.load()&&hook_calls==0,"Native pot callback defers hook and UI publication to the fence");
        need(f.memory==memory&&std::memcmp(&ctx,&f.ctx,sizeof(ctx))==0,"Final pot capture changes no RAM or caller registers");
        need(!r::pot_complete(f.rdram,&f.ctx)&&hook_calls==0&&r::completion_pending,"Replayed final pot cannot commit immediately or duplicate pending work");
        f.fence();need(hook_calls==1&&!r::completion_pending&&r::finished.load(),"One post-fence completion hook then finished flag");
        f.fence();r::frame(f.rdram,&f.ctx,Fixture::fence_vi+1,dispatch);need(hook_calls==1&&r::finished.load(),"Repeated/same/next fence cannot run completion twice");
        need(f.memory==memory&&std::memcmp(&ctx,&f.ctx,sizeof(ctx))==0,"Completion-only frames leave all8MiB and registers unchanged");
    }
    for(unsigned fault=0;fault<8;++fault){Fixture f;f.through_ten();r::completion_hook=observe_completion_only;if(fault==0)f.word(Fixture::egg1+0x180,Fixture::pot);if(fault==1)f.byte(Fixture::pd+0x12C8,7);if(fault==2)f.word(Fixture::player+0x188+4*4,Fixture::egg2);if(fault==3)f.word(Fixture::pd+0x11FC,Fixture::body);if(fault==4)f.byte(Fixture::player+0x30,0x11);if(fault==5)f.word(0x800F8828,Fixture::body);if(fault==6)f.word(Fixture::player+0x20,0x80043450);if(fault==7)f.word(0x800F8DB4,9);const auto memory=f.memory;const auto ctx=f.ctx;
        need(r::pot_complete(f.rdram,&f.ctx)&&r::session.snapshot().phase==c::Phase::Failed,"Invalid final cargo blocks completion before any pending commit");f.fence();need(hook_calls==0&&!r::completion_pending&&!r::finished.load()&&f.memory==memory&&std::memcmp(&ctx,&f.ctx,sizeof(ctx))==0,"Rejected cargo cannot call hook, award or mutate caller/RAM");
    }
    {Fixture f;for(unsigned i=0;i<9;++i){f.begin_fight();f.complete_current_model_fight();f.next_model_fight();}f.begin_fight();auto bad=f.member(0);need(r::session.bind(0,bad.actor),"Final partial synthetic fixture bound");bad.causal_hit=false;r::receipt(bad);f.transport();r::completion_hook=observe_completion_only;need(r::session.snapshot().phase==c::Phase::Failed&&!r::history[9].members[0].actor,"Missing causal proof cannot enter history or Complete");need(!r::pot_complete(f.rdram,&f.ctx),"Incomplete/failed tenth encounter cannot complete through its pot");f.fence();need(!hook_calls&&!r::finished.load()&&!r::completion_pending,"Incomplete receipt cannot reach deferred hook");}
    {Fixture f;f.begin_fight();f.complete_current_model_fight();f.next_model_fight();f.begin_fight();need(r::session.current().count==4,"Seeded second encounter gives a genuine model four-member requirement");auto proof=f.member(0);need(r::session.bind(0,proof.actor),"Partial multi-member fixture bound");r::receipt(proof);f.transport();r::completion_hook=observe_completion_only;need(r::session.snapshot().phase==c::Phase::Fighting&&r::session.snapshot().completed==1&&!r::pot_complete(f.rdram,&f.ctx)&&!r::completion_pending&&!hook_calls,"One member of a four-member encounter cannot become a complete victory or pending award");}
    {Fixture f;f.through_ten();r::completion_hook=observe_completion_only;need(r::pot_complete(f.rdram,&f.ctx)&&r::completion_pending,"Create isolated pending completion");need(r::configure("next-synthetic-qa",0,true,false)&&!r::completion_pending&&!r::finished.load(),"New QA session clears stale pending completion");r::frame(f.rdram,&f.ctx,Fixture::fence_vi,dispatch);need(!hook_calls,"Old pending hook cannot leak into a new session");}
    need(native_calls==0&&dispatch_calls==0,"No native stub/dispatch may run during completion-boundary checks");
    std::ofstream output(report,std::ios::binary);output<<"{\n  \"passed\": true,\n  \"checks\": "<<checks<<",\n  \"scope\": \"Synthetic QA host-session receipts and native-shaped cargo only; deferred completion boundary, not ten native gameplay victories\",\n  \"syntheticQaFixtures\": true,\n  \"allFourteenColorsAndHungryHpZero\": true,\n  \"hookDeferredUntilPostFrame\": true,\n  \"hookOnceOnly\": true,\n  \"invalidCargoAndPartialProofCannotComplete\": true,\n  \"nativeCalls\": "<<native_calls<<",\n  \"nativeDispatches\": "<<dispatch_calls<<",\n  \"ledgerWritten\": false,\n  \"gameLaunched\": false,\n  \"personalProfileUsed\": false,\n  \"nativeTenFightRunVerified\": false\n}\n";if(!output)throw std::runtime_error("CPU report write failed");std::cout<<"BossCompletion0260Checks: "<<checks<<" PASS; synthetic QA only, no native calls/game/ledger.\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
