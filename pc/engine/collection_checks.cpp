#include "collections.hpp"
#include "json/json.hpp"
#include <fstream>
#include <iostream>
#include <bit>
#include <cstdlib>
using namespace workshop::collections;
static void check(bool b,const char* m){if(!b){std::cerr<<m<<'\n';std::exit(1);}}
int main(int argc,char** argv){
    check(argc==2,"Expected disposable folder");auto folder=std::filesystem::path(argv[1]);std::filesystem::create_directories(folder);
    State all;
    for(unsigned l=0;l<24;l++)for(unsigned i=0;i<20;i++){all.levels[l].places.push_back({43+l,i,0,0});check(all.claim(l,i),"Initial coin missing");check(!all.claim(l,i),"Duplicate currency credited");}
    check(all.valid() && all.earned()==480 && all.balance()==480,"480 unique currency budget changed");
    check(all.buy(Item::White) && all.buy(Item::Black) && all.balance()==240,"Egg prices changed");
    check(!all.buy(Item::White) && !all.buy(Item::Hybrid) && !all.buy(Item::Editor),"Duplicate or unavailable item sold");
    check(all.buy(Item::Hybrid,true) && all.balance()==40 && all.remainder,"Forty remaining achievement lost");
    check(all.buy(Item::Editor,true) && all.balance()==0 && all.remainder && all.valid(),"Editor price or permanent achievement changed");
    for(unsigned c=0;c<65536;c++){
        check((turquoise(uint16_t(c))&1)==(c&1),"Palette alpha changed");
        const auto white=collected_white(uint16_t(c));
        check((white&1)==(c&1) && ((white>>11)&31)==((white>>6)&31) && ((white>>6)&31)==((white>>1)&31),"Collected coin lost neutral shading or transparency");
    }
    check(turquoise(0xFFFF)==0xFFFF && turquoise(0)==0,"Neutral palette colors changed");
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
    constexpr auto root=gpr(int32_t(0x800F7EA0)),meta=gpr(int32_t(0x80010000)),objects=gpr(int32_t(0x80011000)),actor=gpr(int32_t(0x80020000));
    auto path=folder/"collection.json";check(!std::filesystem::exists(path),"Never overwrite test files");initialize(path);start_session(1);
    MEM_H(0,meta)=26;MEM_W(4,meta)=uint32_t(objects);MEM_H(0,objects)=0x4001;
    for(unsigned i=1;i<=25;i++){MEM_H(i*16,objects)=0x406F;MEM_W(i*16+4,objects)=std::bit_cast<uint32_t>(float(i*20));MEM_W(i*16+8,objects)=std::bit_cast<uint32_t>(400.f);}
    MEM_W(0xCE8,root)=0;enter(rdram,43,uint32_t(meta),true);check(view().state.levels[0].places.empty(),"Story allocated turquoise coins");
    MEM_W(0xCE8,root)=1;enter(rdram,43,uint32_t(meta),false);check(view().state.levels[0].places.empty(),"Editor allocated currency");
    start_session(2);enter(rdram,43,uint32_t(meta),true);check(view().state.levels[0].places.empty(),"Practice allocated currency");
    start_session(1);enter(rdram,43,uint32_t(meta),true);check(view().state.levels[0].places.size()==20,"Placement quota is not twenty");
    recomp_context ctx{};ctx.r4=actor;ctx.r5=0x406F1800;ctx.r6=MEM_W(20,objects);ctx.r7=MEM_W(24,objects);
    const auto registers=ctx;created(rdram,&ctx);check(!memcmp(&ctx,&registers,sizeof(ctx)),"Birth tracking changed registers");
    const auto extra=gpr(int32_t(0x80012000));MEM_H(0,extra)=0;
    enter(rdram,43,uint32_t(extra),true);
    MEM_H(12,actor)=0x406F;MEM_W(0x168,actor)=0x80030000;MEM_W(0x16C,actor)=0x80031000;
    const auto resource=gpr(int32_t(0x80030000)),drawing=gpr(int32_t(0x80031000)),pal=gpr(int32_t(0x80032000));
    MEM_W(0x88,resource)=uint32_t(pal);MEM_W(8,drawing)=uint32_t(pal);
    for(unsigned i=0;i<256;i++)MEM_H(i*2,pal)=i*257;
    const auto before=memory;draw(rdram,uint32_t(actor));check(uint32_t(MEM_W(8,drawing))==0x807FC200,"Selected coin is not turquoise");
    for(unsigned i=0;i<256;i++)check(MEM_HU(i*2,pal)==i*257,"Shared native palette modified");
    collect(rdram,uint32_t(actor));collect(rdram,uint32_t(actor));
    check(view().state.earned()==1 && MEM_W(0xAB4,root)==2,"Pickup credited twice or missing two supplementary points");
    initialize(path);start_session(1);enter(rdram,43,uint32_t(meta),true);created(rdram,&ctx);
    draw(rdram,uint32_t(actor));check(uint32_t(MEM_W(8,drawing))==0x807FD000,"Previously collected coin did not turn white on replay");
    for(unsigned i=0;i<256;i++)check(MEM_HU(i*2,gpr(int32_t(0x807FD000)))==collected_white(uint16_t(i*257)),"White coin palette mismatch");
    for(unsigned i=0;i<256;i++)check(MEM_HU(i*2,gpr(int32_t(0x807FC200)))==turquoise(uint16_t(i*257)),"White coin changed the palette of uncollected turquoise coins");
    collect(rdram,uint32_t(actor));
    check(view().state.earned()==1 && MEM_W(0xAB4,root)==4,"Revisit or relaunch farmed currency, or replay points missing");
    ctx.r6=0;created(rdram,&ctx);draw(rdram,uint32_t(actor));check(uint32_t(MEM_W(8,drawing))==uint32_t(pal),"Pooled ordinary coin kept turquoise palette");
    check(!purchase(Item::White),"Insufficient balance allowed purchase");
    // A funded, disposable ledger exercises atomic spending and egg search gates.
    nlohmann::json levels=nlohmann::json::object();
    for(unsigned l=0;l<6;l++){auto ps=nlohmann::json::array();for(unsigned i=0;i<20;i++)ps.push_back({43+l,i,0,0});levels[std::to_string(l/4+1)+"-"+std::to_string(l%4+1)]={{"places",ps},{"claimedMask",0xFFFFF}};}
    auto funded=folder/"funded.json";{std::ofstream f(funded);f<<nlohmann::json{{"version",1},{"levels",levels},{"purchases",0},{"remainderAchievement",false}};}
    initialize(funded);check(purchase(Item::White) && view().state.balance()==0,"Atomic egg purchase failed");
    initialize(funded);check(view().state.owns(Item::White) && view().state.balance()==0,"Purchase did not survive restart");
    const auto p=gpr(int32_t(0x807F2000));MEM_W(0,p)=0;MEM_B(4,p)=-1;for(unsigned i=0;i<4;i++)MEM_B(5+i,p)=-1;
    check(!egg_allowed(rdram,6),"Bought egg bypassed 24 Trial requirement");
    MEM_W(0,p)=0xFFFFFF;MEM_B(4,p)=0;for(unsigned i=0;i<4;i++)MEM_B(5+i,p)=i+1;
    check(egg_allowed(rdram,6) && !egg_allowed(rdram,7),"Egg purchase colors or Trial gate changed");
    for(unsigned mode:{0u,1u}){
        MEM_W(0xCE8,root)=mode;MEM_W(0xF54,root)=0;
        MEM_H(12,actor)=0x4692;check(egg_bubble_hidden(rdram,uint32_t(actor))==false,"Bought white egg source hidden in Story/Trial/Hybrid");
        MEM_W(0xF54,root)=0x400;check(egg_bubble_hidden(rdram,uint32_t(actor))==true,"Carried white egg source duplicated");
        MEM_W(0xF54,root)=0;MEM_H(12,actor)=0x4693;check(egg_bubble_hidden(rdram,uint32_t(actor))==true,"Unbought black egg source shown");
        MEM_H(12,actor)=0x41D1;check(!egg_bubble_hidden(rdram,uint32_t(actor)),"Ordinary fruit bubble gate changed");
    }
    MEM_B(9,p)=2;check(egg_allowed(rdram,7),"Existing earned egg was revoked");
    MEM_H(12,actor)=0x4693;check(egg_bubble_hidden(rdram,uint32_t(actor))==false,"Earned black egg source was revoked");
    MEM_B(9,p)=0;MEM_W(0,p)=0x7FFFFF;MEM_H(12,actor)=0x4692;
    check(egg_bubble_hidden(rdram,uint32_t(actor))==true,"Egg bubble bypassed incomplete Trial");
    // Both purchased eggs obey the same gate in Story and both Trial modes.
    auto budget=levels;
    for(unsigned l=6;l<12;l++){auto ps=nlohmann::json::array();for(unsigned i=0;i<20;i++)ps.push_back({43+l,i,0,0});budget[std::to_string(l/4+1)+"-"+std::to_string(l%4+1)]={{"places",ps},{"claimedMask",0xFFFFF}};}
    unsigned eggCases=0;
    for(unsigned owned=0;owned<4;owned++){
        auto gateFile=folder/("egg-gates-"+std::to_string(owned)+".json");
        {std::ofstream f(gateFile);f<<nlohmann::json{{"version",1},{"levels",budget},{"purchases",owned},{"remainderAchievement",false}};}
        initialize(gateFile);check(view().readable,"Purchased egg gate fixture rejected");
        for(unsigned trial:{0u,166147u,0xFFFFFFu})for(unsigned mode:{0u,1u})for(unsigned color:{6u,7u}){
            MEM_W(0,p)=trial;MEM_B(9,p)=0;MEM_W(0xCE8,root)=mode;MEM_W(0xF54,root)=0;
            MEM_H(12,actor)=color==6?0x4692:0x4693;
            const bool expected=trial==0xFFFFFFu && (owned&(1u<<(color-6)));
            check(egg_allowed(rdram,color)==expected,"White/black purchase and Trial gates diverged");
            check(egg_bubble_hidden(rdram,uint32_t(actor))==!expected,"White/black source visibility diverged from eligibility");eggCases++;
        }
    }
    std::cout<<"White/black sources: "<<eggCases<<" purchase/completion/mode cases passed.\n";
    auto invalid=folder/"future.json";{std::ofstream f(invalid);f<<"{\"version\":99}";}
    initialize(invalid);check(!view().readable && !purchase(Item::White),"Unsupported ledger was spent");
    nlohmann::json preserved;std::ifstream(invalid)>>preserved;check(preserved["version"]==99,"Unsupported file overwritten");
    std::cout<<"Collection: 480 unique slots, prices 120/120/200/40, replay protection, immediate persistence, atomic spending, disabled future purchases, palette alpha/pooling, native score supplement, Trial/custom/Practice gates and earned-egg migration passed.\n";
}
