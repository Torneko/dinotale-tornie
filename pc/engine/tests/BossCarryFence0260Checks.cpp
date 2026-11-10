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
namespace carry=workshop::boss_carry0260;
static unsigned checks=0,native_calls=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static void forbidden(){++native_calls;throw std::runtime_error("Carry-fence CPU fixture must not execute native game functions");}
extern "C" void func_80047090(uint8_t*,recomp_context*){forbidden();}
extern "C" void func_8004613C(uint8_t*,recomp_context*){forbidden();}
extern "C" void func_80044194(uint8_t*,recomp_context*){forbidden();}
extern "C" void func_80040C0C(uint8_t*,recomp_context*){forbidden();}
static bool payload_byte(unsigned at){return (at>=0xF8950&&at<0xF8978)||(at>=0xF89D0&&at<0xF8ACC)||(at>=0xF8AD4&&at<0xF8B10);}

struct Fixture {
    std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);
    uint8_t* rdram=memory.data();recomp_context ctx{};unsigned destination;
    static constexpr uint32_t player=0x80010000,pd=0x80011000,pot=0x80014000,pp=0x80015000,source_route=0x80016000,egg1=0x80018000,egg2=0x80019000,body=0x8001A000,shadow=0x8001B000,tongue=0x8001C000,info=0x80024000;
    static constexpr std::array<unsigned,11> fruit_types{0,1,2,3,4,6,7,8,9,10,11};
    void word(uint32_t a,uint32_t value){MEM_W(0,b::base(a))=int32_t(value);}
    void byte(uint32_t a,unsigned value){MEM_B(0,b::base(a))=uint8_t(value);}
    void half(uint32_t a,unsigned value){MEM_H(0,b::base(a))=uint16_t(value);}
    unsigned read_half(uint32_t a){return MEM_HU(0,b::base(a));}
    void actor(uint32_t a,uint16_t kind){std::fill_n(memory.begin()+(a&0x7FFFFF),0x200,0);word(a,0x19971110);half(a+12,kind);}
    void stats(unsigned count,unsigned salt=0){
        for(unsigned n=0;n<carry::score_words;++n)word(carry::score_base+4*n,1000+salt+31*n);
        word(carry::count_address,count);word(carry::backup_count_address,std::min(count,3u));word(carry::ending_address,0);
        for(unsigned n=0;n<carry::slots;++n){word(carry::fruits_base+4*n,fruit_types[(n+salt)%fruit_types.size()]);word(carry::backup_fruits_base+4*n,fruit_types[(n+3+salt)%fruit_types.size()]);half(carry::half_fruits_base+2*n,fruit_types[(n+salt)%fruit_types.size()]);}
    }
    explicit Fixture(bool to_fight=false,unsigned count=1,unsigned hp=7):destination(to_fight?168:83){
        for(unsigned n=0;n<memory.size();++n)memory[n]=uint8_t((n*29u+71u)^n/257u);
        for(auto [a,kind]:std::array<std::pair<uint32_t,uint16_t>,6>{{{player,0x4001},{pot,0x4273},{egg1,0x4004},{egg2,0x4004},{body,0x400B},{shadow,0x4007}}})actor(a,kind);
        actor(tongue,0x400C);std::fill_n(memory.begin()+(pd&0x7FFFFF),0x12D0,0);std::fill_n(memory.begin()+(pp&0x7FFFFF),0x264,0);
        word(player+0x1F4,pd);word(pot+0x1F4,pp);word(pp+0x260,source_route);word(0x800F8828,player);word(0x800F8DB0,0);word(0x800F8DB4,hp);word(0x807F0008,10);
        byte(player+0x18,0xA);byte(player+0x30,2);word(player+0x20,0x80031230);word(pd+0xD0,pot);byte(player+0x165,1);byte(player+0x17D,0);
        word(player+0x84,std::bit_cast<uint32_t>(120.f));word(player+0x88,std::bit_cast<uint32_t>(451.f));
        for(auto a:{egg1,egg2,body,shadow})word(a+0x180,player);word(tongue+0x180,body);word(body+0x188+19*4,tongue);half(body+0x184,1);
        for(auto [slot,a]:std::array<std::pair<unsigned,uint32_t>,4>{{{19,egg1},{3,egg2},{0,body},{7,shadow}}})word(player+0x188+slot*4,a);
        half(player+0x184,4);half(pd+0x12BE,7);byte(pd+0x12C8,2);word(pd+0x11FC+7*24,egg1);word(pd+0x11FC,egg2);
        stats(count);for(unsigned n=0;n<24;++n)byte(source_route+n,workshop::boss_rest_data0240::native_arena_pot_route[n]);
        ctx.r4=b::base(source_route);ctx.r5=0xABCD1234;ctx.r6=0x77889900;ctx.r29=b::base(0x800FF000);ctx.r31=b::base(0x80112233);ctx.f2.u32l=0xABCDEF01;
        need(r::configure("synthetic-carry-fence-qa",20261031,true,false),"Explicit synthetic QA host session");r::rooms::configure(r::session.plan());
        need(r::session.enter_fight(162,player)&&r::session.bind(0,0x80100000),"Bind first model-only encounter");r::receipt(rdram,{0,0,100,0x80100000,true,true,true,true});need(r::session.snapshot().phase==c::Phase::Victory,"Synthetic receipt reaches model victory only");r::exit_pot=pot;
        need(!r::pot_complete(rdram,&ctx)&&r::session.snapshot().phase==c::Phase::ToRest,"Owned native-shaped pot captures cargo and obtains model ToRest");
        if(to_fight){need(r::session.arrive_rest(player),"Model-only rest arrival");actor(pot,0x4274);word(pot+0x1F4,pp);ctx.r4=b::base(source_route);need(!r::pot_complete(rdram,&ctx)&&r::session.snapshot().phase==c::Phase::ToFight,"Owned rest pot recaptures cargo and obtains model ToFight");}
        r::physical_room=r::logical_room=destination;r::current_vi=5000;
        word(0x800F843C,info);half(info+0x114,0x37);word(0x800F8454,0x18);
        need(r::cargo.captured&&r::cargo_epoch==(to_fight?2u:1u)&&r::cargo.stats.count==count&&r::cargo.stats.scores[1]==1031,"Actual host capture stores exact ring/score payload and fresh epoch");
        need(b::word(rdram,r::route_address)==0x44CA2000&&b::byte(rdram,r::route_address+4)==destination,"Owned route contains exact marker and destination");
    }
    void trace(std::string_view name,uint32_t ra){ctx.r31=b::base(ra);const auto before=ctx;r::trace_carry(rdram,&ctx,name);need(std::memcmp(&ctx,&before,sizeof(ctx))==0,"Carry trace preserves all native caller registers");}
    void reset_trace(){trace("func_8006CFC0",carry::reset_caller);}
    void hud_trace(){trace("func_800519C0",carry::hud_caller);}
    void simulate_native_clear(){
        // Simulated CFC0 effects for a CPU fixture, not native MIPS execution.
        for(unsigned n=0;n<carry::score_words;++n)word(carry::score_base+4*n,0);
        word(carry::count_address,0);word(carry::backup_count_address,0);word(carry::ending_address,0);
        for(unsigned n=0;n<carry::slots;++n){word(carry::fruits_base+4*n,0);word(carry::backup_fruits_base+4*n,0);}
        word(0x800F8D14,0);word(0x800F8C1C,0);half(info+0x114,0x61);word(0x800F8454,0x61);
        for(unsigned n=0;n<0x40;++n)byte(0x800F8978+n,0); // later6D1CC cleanup must remain reset
    }
};

int main(int argc,char** argv){try{
    if(argc!=2)throw std::runtime_error("Explicit NEW CPU report path required");const std::filesystem::path report=argv[1];if(std::filesystem::exists(report))throw std::runtime_error("Refusing to replace an existing CPU report");
    for(bool to_fight:{false,true})for(unsigned count:{0u,1u,15u,30u})for(unsigned hp:{0u,7u}){
        Fixture f(to_fight,count,hp);const auto original=f.memory;const auto payload=r::cargo.stats;const auto epoch=r::cargo_epoch;
        f.reset_trace();need(r::stats_reset_pending&&r::stats_reset_epoch==epoch&&f.memory==original,"Reset entry authorizes a pending transaction without changing RAM");need(!r::carried_stats_match(f.rdram),"Pending stats cannot pass the destination arrival guard early");
        f.simulate_native_clear();const auto reset=f.memory;need(!carry::matches(f.rdram,payload),"Simulated native reset actually changes the proved payload");f.hud_trace();
        need(!r::stats_reset_pending&&r::stats_restored_epoch==epoch&&carry::matches(f.rdram,payload)&&r::carried_stats_match(f.rdram),"Exact pre-HUD fence restores the captured payload once");
        bool exact=true;for(unsigned n=0;n<f.memory.size();++n)if(f.memory[n]!=(payload_byte(n)?original[n]:reset[n])){exact=false;break;}need(exact,"Full8MiB comparison: only the carry payload is restored; native canaries/actors/HP/pose stay reset or unchanged");
        need(f.read_half(Fixture::info+0x114)==0x61&&b::word(f.rdram,0x800F8454)==0x61&&b::word(f.rdram,0x800F8978)==0,"Native room identity114 and later resetF8978 are not rolled back");
        const auto restored=f.memory;f.hud_trace();need(f.memory==restored&&r::stats_restored_epoch==epoch&&!r::stats_reset_pending,"HUD replay without pending reset does nothing");
    }
    for(bool to_fight:{false,true})for(uint32_t wrong:{0u,0x8006D220u,0x8006D224u,0x80068844u,0x80068848u}){
        Fixture f(to_fight);const auto before=f.memory;f.trace("func_8006CFC0",wrong);need(!r::stats_reset_pending&&f.memory==before&&r::session.snapshot().phase==(to_fight?c::Phase::ToFight:c::Phase::ToRest),"Wrong reset link cannot authorize restoration");f.simulate_native_clear();const auto reset=f.memory;f.hud_trace();need(f.memory==reset&&!r::stats_reset_pending,"Correct HUD caller without an authorized reset cannot restore anything");
        Fixture g(to_fight);g.reset_trace();g.simulate_native_clear();const auto cleared=g.memory;g.trace("func_800519C0",wrong);need(r::stats_reset_pending&&g.memory==cleared,"Wrong HUD link leaves payload cleared and pending transaction unconsumed");g.hud_trace();need(!r::stats_reset_pending&&carry::matches(g.rdram,r::cargo.stats),"A later correct HUD fence completes that one pending transaction");
    }
    for(bool to_fight:{false,true})for(unsigned fault=0;fault<11;++fault){
        Fixture f(to_fight);if(fault==0)r::physical_room=f.destination==83?168:83;if(fault==1)f.word(r::route_address,0x44CA0000);if(fault==2)f.byte(r::route_address+4,f.destination==83?168:83);if(fault==3)f.word(0x800F8828,Fixture::body);if(fault==4)f.half(Fixture::player+12,0x4004);if(fault==5)f.byte(Fixture::player+0x30,0x11);if(fault==6)r::cargo.stats.count=31;if(fault==7)r::cargo.stats.ending=1;if(fault==8)r::cargo.stats.fruits[0]=5;if(fault==9)r::cargo.stats.half_fruits[0]=11;if(fault==10)r::stats_restored_epoch=r::cargo_epoch;
        const auto before=f.memory;f.reset_trace();need(r::session.snapshot().phase==c::Phase::Failed&&!r::stats_reset_pending&&f.memory==before,"Wrong destination/player/payload or already restored epoch fails closed before RAM writes");f.hud_trace();need(f.memory==before&&!r::stats_reset_pending,"Failed authorization cannot later restore at the HUD fence");
    }
    for(bool to_fight:{false,true})for(unsigned fault=0;fault<5;++fault){
        Fixture f(to_fight);f.reset_trace();f.simulate_native_clear();if(fault==0)f.word(0x800F8828,Fixture::body);if(fault==1)f.byte(r::route_address+4,f.destination==83?168:83);if(fault==2)r::stats_reset_epoch=r::cargo_epoch-1;if(fault==3)r::cargo.stats.ending=1;if(fault==4)r::cargo.stats.half_fruits[0]=11;
        const auto before=f.memory;f.hud_trace();need(r::session.snapshot().phase==c::Phase::Failed&&f.memory==before,"HUD restore revalidates player/destination/epoch/entire payload before any mutation");
    }
    for(bool to_fight:{false,true}){
        Fixture f(to_fight);f.reset_trace();const auto before=f.memory;f.reset_trace();need(r::session.snapshot().phase==c::Phase::Failed&&f.memory==before,"Double pending reset is rejected without mutation");f.simulate_native_clear();const auto clear=f.memory;f.hud_trace();need(f.memory==clear,"A double-reset failure cannot authorize a stale restore");
        Fixture g(to_fight);g.reset_trace();g.simulate_native_clear();g.hud_trace();const auto restored=g.memory;g.reset_trace();need(r::session.snapshot().phase==c::Phase::Failed&&g.memory==restored,"A second reset for the same restored cargo epoch is rejected");
    }
    for(unsigned mode=0;mode<5;++mode){Fixture f;const auto before=f.memory;if(mode==0)r::enabled=false;if(mode==1)r::session=c::Session{};if(mode==2)r::cargo.captured=false;if(mode==3)r::cargo_epoch=0;if(mode==4)need(r::configure("new-armed-synthetic-qa",0,true,false),"New armed QA session configured");f.reset_trace();f.hud_trace();need(f.memory==before&&!r::stats_reset_pending,"Disabled/Idle/Armed/uncaptured/epoch0 operation is inert");}
    {Fixture f;const auto before=f.memory;const auto ctx=f.ctx;r::trace_carry(f.rdram,nullptr,"func_8006CFC0");need(f.memory==before&&!r::stats_reset_pending&&std::memcmp(&ctx,&f.ctx,sizeof(ctx))==0,"Null context cannot authorize any carry operation");}
    for(bool to_fight:{false,true}){
        Fixture f(to_fight);f.reset_trace();const auto old_epoch=r::cargo_epoch;f.stats(2,1);const auto newer=f.memory;need(r::capture(f.rdram,Fixture::pot)&&r::cargo_epoch==old_epoch+1&&!r::stats_reset_pending&&f.memory==newer,"New actual host capture invalidates the old pending epoch without RAM mutation");const auto latest=r::cargo.stats;f.simulate_native_clear();const auto reset=f.memory;f.hud_trace();need(f.memory==reset&&!r::stats_reset_pending,"Old HUD event cannot replay data after a new capture");f.reset_trace();f.hud_trace();need(carry::matches(f.rdram,latest)&&r::stats_restored_epoch==old_epoch+1,"New capture can authorize only its own latest payload");
        Fixture g(to_fight);g.reset_trace();g.simulate_native_clear();const auto cleared=g.memory;need(r::configure("replacement-synthetic-qa",0,true,false),"Replacement QA configure");g.hud_trace();need(g.memory==cleared&&!r::cargo.captured&&!r::cargo_epoch&&!r::stats_reset_pending&&!r::stats_reset_epoch&&!r::stats_restored_epoch,"Configure invalidates all pending cargo epochs and cannot restore old data");
    }
    need(native_calls==0,"No native function is executed in host carry-fence CPU checks");
    std::ofstream out(report,std::ios::binary);out<<"{\n  \"passed\": true,\n  \"checks\": "<<checks<<",\n  \"scope\": \"Synthetic QA host-session/pot/cargo and simulated native reset only; not native MIPS/GPU transition proof\",\n  \"syntheticQaReceipts\": true,\n  \"toRest83AndToFight168\": true,\n  \"exactMipsCallerFenceGating\": true,\n  \"full8MiBWhitelistComparison\": true,\n  \"roomIdentity114HpActorsPoseAndResetCanariesPreserved\": true,\n  \"wrongIdentityDestinationPayloadAndEpochRejected\": true,\n  \"newCaptureAndConfigureInvalidateOldPendingEpoch\": true,\n  \"nativeCalls\": "<<native_calls<<",\n  \"gameLaunched\": false,\n  \"ledgerWritten\": false,\n  \"personalProfileUsed\": false,\n  \"nativeTenFightRunVerified\": false\n}\n";if(!out)throw std::runtime_error("CPU report write failed");std::cout<<"BossCarryFence0260Checks: "<<checks<<" PASS; synthetic QA fixtures only, zero native calls/game/ledger.\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
