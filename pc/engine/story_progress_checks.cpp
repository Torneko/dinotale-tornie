#include "story_progress.hpp"
#include "hybrid_finds.hpp"
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <bit>
using namespace workshop::progress;
static void check(bool b,const char* s){if(!b){fprintf(stderr,"%s\n",s);std::exit(1);}}
extern "C" void workshop_ys_ext_complete_trial(uint8_t*,recomp_context*);
extern "C" void workshop_ys_ext_unlocked_mask(uint8_t*,recomp_context*);
extern "C" void workshop_ys_ext_trial_color_unlocked(uint8_t*,recomp_context*);
extern "C" void workshop_ys_ext_pack(uint8_t*,recomp_context*);
extern "C" void workshop_ys_ext_unpack(uint8_t*,recomp_context*);
extern "C" void func_800409F0(uint8_t*,recomp_context*);
extern "C" void func_800409F0_before(uint8_t*,recomp_context*);
extern "C" void func_807B4A74(uint8_t*,recomp_context*);
extern "C" void func_807B4A74_before(uint8_t*,recomp_context*);
extern "C" void func_8006D774(uint8_t*,recomp_context*){}
extern "C" void func_8006E0FC(uint8_t*,recomp_context*){}
static unsigned draws=0;
extern "C" void do_break(uint32_t){check(false,"Native progression divided by zero");}
static void random_native(uint8_t*,recomp_context* ctx){ctx->r2=100+draws++;}
extern "C" recomp_func_t* get_function(int32_t address){check(uint32_t(address)==0x80012340,"Unexpected native indirect target");return random_native;}
static void native_paths(){
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
    const auto at=gpr(int32_t(0x807F2000)),out=gpr(int32_t(0x80011000));
    MEM_W(0,int32_t(0x800F8E5C))=0x333333;
    State p{0,3,{0,1,255,255},0,11};write(rdram,at,p);
    recomp_context ctx{};ctx.r29=gpr(int32_t(0x80020000));const auto sp=ctx.r29;
    auto mask=[&](){ctx.r4=at;workshop_ys_ext_unlocked_mask(rdram,&ctx);return unsigned(ctx.r2);};
    check(mask()==11,"Native selector did not expose three recovered colors");
    for(unsigned row=0;row<6;row++){
        ctx.r4=at;ctx.r5=row;ctx.r6=0;ctx.r7=gpr(int32_t(0x80012340));MEM_W(0x10,sp)=0;MEM_W(0x14,sp)=uint32_t(out);MEM_B(0,out)=0;
        workshop_ys_ext_complete_trial(rdram,&ctx);
        check(ctx.r29==sp && ctx.r2==1 && MEM_BU(0,out)==255,"Native Trial catch-up changed reward result/output/stack");
        check(mask()==11 && read(rdram,at).columns[0]==0 && draws==0,"Trial catch-up drew a duplicate Story reward");
    }
    // The other columns still use the native draw routine. Completing all
    // Trial stages also exposes the sixth color, but neither special egg.
    for(unsigned row=0;row<6;row++)for(unsigned col=1;col<4;col++){
        ctx.r4=at;ctx.r5=row;ctx.r6=col;ctx.r7=gpr(int32_t(0x80012340));MEM_W(0x10,sp)=0;MEM_W(0x14,sp)=uint32_t(out);
        workshop_ys_ext_complete_trial(rdram,&ctx);check(ctx.r29==sp && ctx.r2>0,"Native Trial progression failed");
    }
    check(read(rdram,at).trial==0xFFFFFF && mask()==63 && read(rdram,at).eggs==0,"Final Trial reward/egg separation failed");
    for(unsigned color:{6,7}){ctx.r4=at;ctx.r5=color;workshop_ys_ext_trial_color_unlocked(rdram,&ctx);check(ctx.r2==0,"White/black unlocked without finding its egg");}
    const auto original=read(rdram,at);ctx.r4=at;ctx.r5=out;workshop_ys_ext_pack(rdram,&ctx);check(ctx.r2==1,"Native reward codec rejected Story columns");
    ctx.r4=at+0x100;ctx.r5=out;workshop_ys_ext_unpack(rdram,&ctx);check(ctx.r2==1 && used(read(rdram,at+0x100))==used(original),"Native reward codec round-trip changed colors");
}
static void score_paths(){
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
    const auto at=gpr(int32_t(0x807F2000));
    MEM_W(0,int32_t(0x800F8E5C))=0x333333;
    for(auto a:{0x800F8950u,0x800F8954u,0x800F8958u,0x800F895Cu,0x800F8960u})MEM_W(0,int32_t(a))=0;
    State personal{0x101,3,{0,1,255,255},0,11};write(rdram,at,personal);
    MEM_W(0,int32_t(0x800F8DF4))=63;MEM_W(0,int32_t(0x807F1000))=11;
    recomp_context ctx{};ctx.r29=gpr(int32_t(0x80020000));
    func_807B4A74_before(rdram,&ctx);check(ctx.r2==600,"Native pause control must reproduce missing extra Yoshi bonus");
    func_807B4A74(rdram,&ctx);check(ctx.r2==900,"Three earned living extras must raise the pause bonus to 900");
    unsigned cases=0;
    for(unsigned mode=0;mode<2;mode++)for(unsigned complete=0;complete<2;complete++){
        auto p=complete?State{0xFFFFFF,0,{1,2,3,4},3,63}:personal;write(rdram,at,p);
        const unsigned unlocked=complete?63:11;
        MEM_W(0,int32_t(0x800F8B88))=mode;
        for(unsigned native=0;native<256;native++)for(unsigned extra=0;extra<64;extra++){
            MEM_W(0,int32_t(0x800F8DF4))=native|0xABCD0000u;
            MEM_W(0,int32_t(0x807F1000))=extra;
            const unsigned classics=mode?((native&63)|(complete?192:0)):native;
            const unsigned count=std::popcount(classics)+std::popcount(extra&unlocked);
            ctx.r29=gpr(int32_t(0x80020000));const auto sp=ctx.r29;
            func_807B4A74(rdram,&ctx);
            if(ctx.r29!=sp || ctx.r2!=100*count)fprintf(stderr,"Live score case mode=%u full=%u native=%02X extra=%02X expected=%u actual=%u\n",mode,complete,native,extra,100*count,unsigned(ctx.r2));
            check(ctx.r29==sp && ctx.r2==100*count,"Live pause score disagrees with earned/alive Yoshis");
            func_800409F0(rdram,&ctx);
            check(ctx.r29==sp && uint32_t(MEM_W(0,int32_t(0x800F8B68)))==count,"Native end tally disagrees with live pause count");cases++;
        }
    }
    MEM_W(0,int32_t(0x800F8950))=10000;
    func_807B4A74(rdram,&ctx);check(ctx.r2==9999,"Native live score cap changed");
    printf("Live score verified: pre-fix 600 versus corrected 900 for three extras; %u native pause/end comparisons with deaths, locked colors and Story/Trial gates.\n",cases);
}
static void trial_access_paths(){
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
    const auto at=gpr(int32_t(0x807F2000));
    // This is the personal save's five stored colors: brown is the sixth.
    State p{0x28903,3,{2,1,0,5},0,63};write(rdram,at,p);
    MEM_W(0,int32_t(0x800F8E5C))=0xFFFFFF;
    recomp_context ctx{};ctx.r29=gpr(int32_t(0x80020000));ctx.r4=at;
    workshop_ys_ext_unlocked_mask(rdram,&ctx);
    check(ctx.r2==63 && ctx.r29==gpr(int32_t(0x80020000)),"All Trial access did not expose sixth hybrid or damaged stack");
    for(unsigned color=8;color<14;color++){ctx.r4=at;ctx.r5=color;workshop_ys_ext_trial_color_unlocked(rdram,&ctx);check(ctx.r2==1,"A hybrid remains locked after all Trial access");}
    for(unsigned color:{6,7}){ctx.r4=at;ctx.r5=color;workshop_ys_ext_trial_color_unlocked(rdram,&ctx);check(ctx.r2==0,"Access-only condition bypassed egg requirements");}
    check(read(rdram,at).trial==p.trial && read(rdram,at).eggs==0 && read(rdram,at).lives==63,"Access-only unlock modified actual Trial results, eggs or lives");
    MEM_W(0,int32_t(0x800F8DF4))=63;MEM_W(0,int32_t(0x807F1000))=63;MEM_W(0,int32_t(0x800F8B88))=1;
    func_807B4A74(rdram,&ctx);check(ctx.r2==1200,"Six hybrid living bonuses not reflected in live Trial score");
    for(unsigned missing=0;missing<24;missing++){
        State partial{0,3,{255,255,255,255},0,8};unsigned next=0;
        for(unsigned c=0;c<4;c++)if(c!=missing%4){while(next==3)next++;partial.columns[c]=uint8_t(next++);}
        auto story=0xFFFFFFu&~(1u<<missing);write(rdram,at,partial);MEM_W(0,int32_t(0x800F8E5C))=story;
        ctx.r4=at;workshop_ys_ext_unlocked_mask(rdram,&ctx);
        check(ctx.r2==used(partial) && ctx.r2!=63,"Sixth color granted with a Trial course still inaccessible");
    }
    printf("Sixth hybrid verified: 24 access boundaries, native brown selection, 6/24 Trial unchanged, eggs still locked and living score 1200.\n");
}
static void finish_unlock_paths(){
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
    const auto at=gpr(int32_t(0x807F2000));
    recomp_context ctx{};ctx.r29=gpr(int32_t(0x80020000));
    auto setup=[&](const State& p,unsigned story,unsigned mode,unsigned row,unsigned col,unsigned flags,unsigned extras){
        write(rdram,at,p);MEM_W(0,int32_t(0x800F8E5C))=story;
        MEM_W(0,int32_t(0x800F8B88))=mode;
        MEM_B(0,int32_t(0x800F8DFC))=row;MEM_B(0,int32_t(0x800F8DFD))=col;
        MEM_W(0,int32_t(0x800F8DF4))=flags;MEM_W(0,int32_t(0x807F1000))=extras;
    };
    auto tally=[&](unsigned expected){
        std::array<uint8_t,12> original{};for(int i=0;i<12;i++)original[i]=MEM_BU(i,at);
        const auto flags=MEM_W(0,int32_t(0x800F8DF4)),extras=MEM_W(0,int32_t(0x807F1000));
        const auto sp=ctx.r29;
        func_800409F0(rdram,&ctx);
        check(ctx.r29==sp && unsigned(MEM_W(0,int32_t(0x800F8B68)))==expected,"Newly earned Yoshi missing or duplicated in native result count");
        // Result fields and scratch stack may change; progress and living flags may not.
        for(int i=0;i<12;i++)check(MEM_BU(i,at)==original[i],"Result preview wrote progression");
        check(MEM_W(0,int32_t(0x800F8DF4))==flags && MEM_W(0,int32_t(0x807F1000))==extras,"Result preview promoted an egg or revived a life before native commit");
    };
    State empty{0,255,{255,255,255,255},0,0};
    setup(empty,0,0,0,0,63|0x800,0);
    func_800409F0_before(rdram,&ctx);check(MEM_W(0,int32_t(0x800F8B68))==6,"Control did not reproduce missing black egg bonus");
    tally(7);tally(7);
    func_807B4A74(rdram,&ctx);check(ctx.r2==600,"Provisional egg was counted in pause before level completion");
    unsigned cases=0;const auto previous_draws=draws;
    for(unsigned mode=0;mode<2;mode++)for(unsigned earned=0;earned<4;earned++)for(unsigned carried=0;carried<4;carried++)for(unsigned native=0;native<256;native++){
        State p=mode?State{0xFFFFFF,0,{1,2,3,4},uint8_t(earned),0}:empty;
        const unsigned living=mode?((native&63)|(earned<<6)):native;
        setup(p,0,mode,0,0,native|(carried<<10),0);
        tally(std::popcount(living|(carried<<6)));cases++;
    }
    setup(empty,0,1,0,0,63|0xC00,0);tally(6); // Trial egg requirements still apply.
    for(unsigned mode=0;mode<2;mode++)for(unsigned col=0;col<4;col++)for(unsigned row=0;row<6;row++)for(unsigned first=0;first<6;first++){
        const auto prior=column(col)&~(1u<<(row*4+col));
        State p{mode?prior:0,uint8_t(first),{255,255,255,255},0,uint8_t(1u<<first)};
        setup(p,mode?0:prior,mode,row,col,63,1u<<first);tally(8);tally(8);
        // Real later progression chooses a color; whichever one it picks,
        // the result must not add this same reward again after the commit.
        const unsigned added=first==0?1:0;p.columns[col]=uint8_t(added);
        if(mode)p.trial|=1u<<(row*4+col);
        p.lives|=1u<<added;
        setup(p,mode?0:column(col),mode,row,col,63,p.lives);tally(8);
        // Existing dead colors remain dead, while a new reward joins alive.
        p.columns[col]=255;p.lives=0;if(mode)p.trial=prior;
        setup(p,mode?0:prior,mode,row,col,1,0);tally(2);
        cases+=4;
    }
    setup(empty,column(0)&~(1u<<20),0,5,0,63,0);tally(8); // First Story + first column.
    setup(empty,0,0,4,0,63,0);tally(6); // No first reward before finishing Story.
    State last{0x7FFFFF,0,{1,2,3,255},0,15};
    setup(last,0,1,5,3,63,15);tally(12); // Last column + sixth hybrid, not just one.
    last.trial=0;
    setup(last,0xFFFFFF,0,5,3,63,15);tally(12); // Access bit already set, save reconciliation still pending.
    setup(last,0xFFFFFF,0,5,3,63,63);tally(12); // All six already exposed: no double bonus.
    check(draws==previous_draws,"Result preview consumed native reward randomness");
    printf("Finish unlock bonus verified: %u native egg/column/death cases, missing black egg 600 -> 700, first Story and final Trial, repeat snapshots and progress preserved.\n",cases);
}
static void found_egg_paths(const std::filesystem::path& folder){
    namespace finds=workshop::finds;
    check(!std::filesystem::exists(folder),"Do not overwrite an existing find test profile");std::filesystem::create_directories(folder);
    const auto path=folder/"pc-hybrid-finds.json";finds::initialize(path);
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
    const auto at=gpr(int32_t(0x807F2000));
    State empty{0,255,{255,255,255,255},0,0};write(rdram,at,empty);
    finds::migrate(rdram,0);check(finds::view().migrated && finds::available(63)==0,"Fresh profile automatically acquired eligible Yoshis");
    State eligible{0xFFFFFF,0,{1,2,3,4},0,0};write(rdram,at,eligible);
    MEM_W(0,int32_t(0x800F8B88))=1;MEM_W(0,int32_t(0x800F8DF4))=63;
    recomp_context ctx{};ctx.r29=gpr(int32_t(0x80020000));
    for(unsigned c=8;c<14;c++){ctx.r4=at;ctx.r5=c;workshop_ys_ext_trial_color_unlocked(rdram,&ctx);check(ctx.r2==0,"Eligible but unfound Yoshi appeared in native selector");}
    func_800409F0(rdram,&ctx);check(MEM_W(0,int32_t(0x800F8B68))==6,"Objective alone granted a premature +100 bonus");
    finds::enter(0,43,true);check(!finds::reserve(63,0x46AA,1,2,true),"Egg replaced an occupied White Shy Guy spot");
    auto egg=finds::reserve(63,0x46AA,1,2,false);check(egg && *egg==0,"Eligible egg not offered");
    check(!finds::reserve(63,0x46AE,9,9,false),"A second egg appeared in another room/placement");
    finds::spawned(0x80030000,*egg);
    const auto object=gpr(int32_t(0x80030000)),player=gpr(int32_t(0x80031000));
    MEM_W(0,object)=MEM_W(0,player)=0x19971110;MEM_H(12,object)=0x401D;MEM_H(12,player)=0x4001;
    MEM_B(0x10,object)=1;
    for(auto a:{object,player}){MEM_W(0x3C,a)=std::bit_cast<uint32_t>(100.f);MEM_W(0x40,a)=std::bit_cast<uint32_t>(100.f);MEM_W(0x44,a)=std::bit_cast<uint32_t>(500.f);}
    MEM_W(0xB4,player)=std::bit_cast<uint32_t>(1.f);MEM_W(0x40,object)=std::bit_cast<uint32_t>(114.f);
    check(finds::touching(rdram,uint32_t(player))==uint32_t(object),"Scene egg could not be collected by body contact");
    MEM_W(0x3C,object)=std::bit_cast<uint32_t>(130.f);check(!finds::touching(rdram,uint32_t(player)),"Distant egg collected");
    MEM_W(0x3C,object)=std::bit_cast<uint32_t>(100.f);finds::enter(0,44,true);
    check(!finds::touching(rdram,uint32_t(player)),"An egg in a different room was collected");finds::enter(0,43,true);
    MEM_W(0,object)=0;check(!finds::touching(rdram,uint32_t(player)),"Freed egg actor collected");MEM_W(0,object)=0x19971110;
    finds::egg_state(0x80030000,4);
    check(finds::pending()==1 && finds::available(63)==0,"Pickup granted a permanent Yoshi before the finish");
    finds::spawned(0x80032000,0,false);finds::egg_state(0x80032000,4);finds::egg_state(0x80030000,0);
    check(finds::pending()==1 && !finds::scene_actor(0x80032000),"Removing the scene egg lost its native pool replacement");
    finds::enter(0,44,true);check(finds::pending()==1 && !finds::reserve(63,0x46AE,1,2,false),"Room change lost or duplicated the carried egg");
    func_800409F0(rdram,&ctx);check(MEM_W(0,int32_t(0x800F8B68))==7 && finds::view().found==1,"Native finish did not claim the egg and count its +100");
    func_800409F0(rdram,&ctx);check(MEM_W(0,int32_t(0x800F8B68))==7,"Found egg counted twice in results");
    ctx.r4=at;ctx.r5=8;workshop_ys_ext_trial_color_unlocked(rdram,&ctx);check(ctx.r2==1,"Found egg missing from native selection");
    finds::initialize(path);finds::migrate(rdram,0);check(finds::view().found==1 && finds::available(63)==1,"Reload auto-granted the remaining eligible colors");
    finds::reset_attempt();finds::enter(1,43,true);egg=finds::reserve(63,0x46AA,1,2,false);
    check(egg && *egg==1,"Next attempt did not offer one remaining egg");
    finds::spawned(0x80030000,1);finds::egg_state(0x80030000,4);finds::egg_state(0x80030000,2);
    check(!finds::pending(),"Dropped egg still counted as carried");
    finds::egg_state(0x80030000,5);finds::reset_attempt();check(!finds::pending() && finds::view().found==1,"Death/abandonment preserved an unconfirmed find");
    finds::enter(1,43,false);check(!finds::reserve(63,0x46AA,1,2,false),"Custom/Practice source created a find");
    auto legacy=folder/"legacy.json";finds::initialize(legacy);finds::migrate(rdram,0);
    check(finds::available(63)==63,"Migration removed previously earned Yoshis");
    finds::clear_save();check(finds::view().found==0 && finds::available(63)==0,"Erasing native progression retained old find ownership");
    auto broken=folder/"unsupported.json";{std::ofstream file(broken);file<<"{\"version\":99}";}
    finds::initialize(broken);finds::migrate(rdram,0);std::ifstream file(broken);std::string text((std::istreambuf_iterator<char>(file)),{});
    check(!finds::view().readable && text=="{\"version\":99}","Unsupported find ledger was overwritten");
    printf("Hybrid finds verified: migration, native selection gates, no automatic reward, one egg per level, occupied Story spot, room revisit, pickup/drop/death, finish +100 without duplication, persistence and unsupported file preservation.\n");
}
int main(int argc,char** argv){
    native_paths();
    score_paths();
    trial_access_paths();
    finish_unlock_paths();
    State old{0x101,3,{255,255,255,255},0,8};
    check(valid(old,0x777B77),"Legacy Story save rejected");
    auto p=old;unsigned added=reconcile(p,0x777B77,[](unsigned){return 0;});
    check(added==3 && p.first==3 && p.columns==std::array<uint8_t,4>{0,1,255,255},"Two Story columns did not award exactly two distinct Yoshis");
    check(p.trial==0x101 && p.eggs==0 && p.lives==11,"Story recovery changed Trial, eggs or previous life state");
    check(valid(p,0x777B77),"Recovered save rejected");
    check(!valid(p,0),"Unearned column accepted without Story or Trial");
    auto repeated=p;check(reconcile(p,0x777B77,[](unsigned){std::abort();return 0;})==0 && used(p)==used(repeated),"Reward duplicated on reload");
    State first{0,255,{255,255,255,255},0,63};check(reconcile(first,0x333333,[](unsigned){return 0;})==0,"Columns granted before the first Story reward");
    unsigned cases=0;
    for(unsigned c=0;c<4;c++)for(unsigned missing=0;missing<6;missing++){
        State q{0,3,{255,255,255,255},0,8};
        const uint32_t incomplete=column(c)&~(1u<<(missing*4+c));
        check(reconcile(q,incomplete,[](unsigned){return 0;})==0,"Incomplete column awarded a Yoshi");
        check(reconcile(q,column(c),[](unsigned){return 0;})==1,"Complete Story column not rewarded");
        check(q.columns[c]==0 && q.trial==0,"Story counted as Trial completion");cases++;
    }
    State all{0,3,{255,255,255,255},0,8};check(reconcile(all,0xFFFFFF,[](unsigned){return 0;})==0x37,"All Story access must award the final remaining hybrid too");
    check(used(all)==0x1F && unlocked(all,0xFFFFFF)==63 && all.lives==63 && all.eggs==0 && all.trial==0,"Access unlock changed Trial progress or omitted sixth life");
    all.lives&=~32u;check(reconcile(all,0xFFFFFF,[](unsigned){std::abort();return 0;})==0 && !(all.lives&32),"Reload revived a lost sixth Yoshi");
    auto bad=all;bad.columns[1]=bad.columns[0];check(!valid(bad,0xFFFFFF),"Duplicate colors accepted");
    bad=old;bad.eggs=1;check(!valid(bad,0xFFFFFF),"Story allowed a Trial egg before 24 Trial successes");
    printf("Story progression verified: legacy recovery to three extras, %u column boundaries, idempotence, independent Trial/egg requirements and unique rewards.\n",cases);
    check(argc==2,"Expected a disposable find-check folder");found_egg_paths(argv[1]);
}
