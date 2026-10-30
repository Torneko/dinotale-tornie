#include "hybrid_pickup_sound.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

static unsigned banks=0,cues=0;
static void check(bool ok,const char* why){if(!ok){fprintf(stderr,"Pickup sound: %s\n",why);std::exit(1);}}
extern "C" void func_80067E68(uint8_t*,recomp_context* ctx){
    check(ctx->r4==6,"wrong native bank");banks++;
    ctx->r2=gpr(int32_t(0x80345678));ctx->r4=99;ctx->r31=123;ctx->f0.u32l=456;
}
extern "C" void func_80067818(uint8_t*,recomp_context* ctx){
    check(ctx->r4==0x251 && uint32_t(ctx->r5)==0x80345678,"native cue did not receive bank result");cues++;
    ctx->r4=55;ctx->r2=66;ctx->r31=77;ctx->f12.u32l=88;
}
int main(int argc,char** argv){
    check(argc==2,"expected an isolated ledger directory");
    const auto folder=std::filesystem::path(argv[1]);check(!std::filesystem::exists(folder),"existing profile preserved");
    std::filesystem::create_directories(folder);
    using namespace workshop;
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
    progress::State empty{0,255,{255,255,255,255},0,0};
    progress::write(rdram,gpr(int32_t(0x807F2000)),empty);
    finds::initialize(folder/"pc-hybrid-finds.json");finds::migrate(rdram,0);
    recomp_context ctx{};ctx.r4=gpr(int32_t(0x80030000));ctx.r29=gpr(int32_t(0x80070000));ctx.r31=0x12345678;ctx.f12.u32l=0x42F00000;
    const auto original=ctx;const auto original_memory=memory;
    auto sound=[&](uint32_t actor,unsigned state){
        bool result=finds::pickup_sound(rdram,&ctx,actor,state);
        check(!std::memcmp(&ctx,&original,sizeof(ctx)),"audio changed native caller registers");return result;
    };
    for(unsigned color=0;color<6;color++){
        finds::reset_attempt();finds::enter(color,43,true);
        auto slot=finds::reserve(1u<<color,0x464D,1,2,false);check(slot && *slot==color,"wrong eligible color");
        const uint32_t scene=0x80030000+color*0x1000,pool=scene+0x400;
        finds::spawned(scene,color,true);
        for(unsigned state=0;state<4;state++){finds::egg_state(scene,state);check(!sound(scene,state),"sound before pickup");}
        finds::egg_state(scene,4);check(sound(scene,4),"first carry was silent");
        finds::egg_state(scene,5);check(!sound(scene,5),"second carried state replayed fanfare");
        finds::spawned(pool,color,false);finds::egg_state(pool,4);check(!sound(pool,4),"scene-to-pool transfer replayed fanfare");
        finds::enter(color,44,true);finds::egg_state(pool,5);check(!sound(pool,5),"room revisit replayed fanfare");
        finds::egg_state(pool,0);finds::egg_state(pool,4);check(!sound(pool,4),"same attempt replayed fanfare");
        finds::reset_attempt();finds::enter(color,43,true);finds::spawned(pool,color,false);
        finds::egg_state(pool,5);check(sound(pool,5),"new attempt lost its pickup cue");
    }
    check(!sound(0x80060000,4) && !sound(0x80061000,5),"ordinary white/black egg was intercepted");
    check(banks==12 && cues==12,"cue count does not match successful pickups");
    check(memory==original_memory,"sound wrote game progress");
    check(finds::view().found==0,"pickup sound unlocked a Yoshi before level completion");
    puts("Hybrid pickup audio: all six colors, states4/5, native6/0251 cue, scene/pool transfer, room revisit, drop/retry, next attempt, register/RAM preservation and ordinary egg exclusion passed.");
}
