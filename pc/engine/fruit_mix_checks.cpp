#include "fruit_mix.hpp"
#include "fruit_visits.hpp"
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
using namespace workshop::fruit_mix;
extern "C" void func_80051418(uint8_t*,recomp_context*);
static std::vector<unsigned> loaded;
static bool enabled=true;
static unsigned draws=0;
static Visits nativeVisits;
extern "C" void func_80051348(uint8_t*,recomp_context*){loaded.clear();}
extern "C" void func_80051368(uint8_t*,recomp_context* ctx){loaded.push_back(uint32_t(ctx->r4));}
extern "C" void func_80057F74(uint8_t*,recomp_context*){std::abort();}
extern "C" void workshop_mix_level(uint8_t* rdram,recomp_context* ctx){if(enabled && playable(rdram,uint32_t(ctx->r19))){nativeVisits.apply(rdram,uint32_t(ctx->r19),43,[](){return draws++%3;});for(unsigned i=0;i<6;i++)loaded.push_back(0x4F00+i);}}
static void require(bool b,const char* s){if(!b){fprintf(stderr,"%s\n",s);std::exit(1);}}
int main(int argc,char** argv){
    require(argc==2,"Expected original ROM");
    constexpr uint16_t expected[4][3]={{0x4F00,0x4F01,0x4F04},{0x4F00,0x4F03,0x4F05},{0x4F02,0x4F03,0x4F04},{0x4F01,0x4F02,0x4F05}};
    unsigned probabilityCases=0;
    for(unsigned f=0;f<4;f++){
        unsigned unchanged=0,changed=0;
        for(unsigned t=0;t<60;t++){
            const auto got=replacement(0x4030+f,t);
            require(got==(t<15?expected[f][t%3]:0x4030+f),"Hybrid component mapping changed");
            if(got==0x4030+f)unchanged++;else changed++;probabilityCases++;
        }
        require(changed==15 && unchanged==45,"Replacement is not exactly 25 percent");
    }
    for(uint16_t id:{0x4001,0x4034,0x4035,0x45B6,0x41D1,0x406F,0x4F00,0x4F05})for(unsigned t=0;t<60;t++)require(replacement(id,t)==id,"Protected object was randomized");
    SpawnChoices sources;
    for(uint16_t id=0x4030;id<=0x4033;id++)for(unsigned ticket=0;ticket<60;ticket++){
        sources.clear();unsigned draws=0;
        const auto first=sources.pick(id,0x80020000,[&](){draws++;return ticket;});
        for(unsigned repeat=0;repeat<4;repeat++)require(sources.pick(id,0x80020000,[&](){draws++;return ticket<15?59u:0u;})==first,"A visible container fruit was drawn again");
        require(draws==1,"Recreating one fruit consumed another random ticket");
        const unsigned next=ticket<15?59:0;
        require(sources.pick(id,0x80020400,[&](){draws++;return next;})==replacement(id,next),"Independent fruit sources shared their choice");
        sources.created(0x80020000);
        require(sources.pick(id,0x80020000,[&](){draws++;return next;})==replacement(id,next) && draws==3,"A pooled actor kept the previous source's choice");
    }
    sources.clear();require(sources.parents.empty(),"Room change kept old fruit choices");
    for(uint16_t kind:{0x41D1,0x41D2,0x41D5,0x452A,0x4530,0x46A8,0x4199,0x419C})require(preview_source(kind),"A visible fruit source may reroll its preview");
    for(uint16_t kind:{0x4001,0x4086,0x403A,0x4684})require(!preview_source(kind),"A multi-fruit generator shares one choice");
    Random a(41),b(41);for(unsigned i=0;i<10000;i++)require(a.ticket()==b.ticket(),"Diagnostic random seed is not reproducible");
    std::vector<uint8_t> memory(0x800000,0);auto* rdram=memory.data();
    const auto header=gpr(int32_t(0x80010000)),metaRef=gpr(int32_t(0x80011000)),meta=gpr(int32_t(0x80012000)),objects=gpr(int32_t(0x80013000));
    const uint16_t ids[]={0x4001,0x4030,0x4031,0x4032,0x4033,0x4035,0x45B6,0x41D1,0x406F,0x4F00};
    auto setup=[&](){
        MEM_W(0x5C,header)=uint32_t(metaRef);MEM_H(0x60,header)=0xFFFF;
        MEM_W(0,metaRef)=uint32_t(meta);MEM_H(0,meta)=std::size(ids);MEM_W(4,meta)=uint32_t(objects);
        for(unsigned i=0;i<std::size(ids);i++){
            MEM_H(i*16,objects)=ids[i];MEM_H(i*16+2,objects)=0x1234+i;
            for(unsigned j=4;j<16;j+=4)MEM_W(i*16+j,objects)=0x12345678+i+j;
        }
        draws=0;
    };
    for(unsigned ticket=0;ticket<60;ticket++){
        Visits visits;setup();unsigned rolls=0;
        auto first=visits.apply(rdram,uint32_t(meta),43,[&]{rolls++;return ticket;});
        require(rolls==4,"Initial placements did not get exactly one draw");
        for(unsigned visit=0;visit<50;visit++){
            // Revisit either the same mutated template or freshly loaded ROM.
            if(visit%2)setup();
            visits.enter();
            auto again=visits.apply(rdram,uint32_t(meta),43,[&]{rolls++;return ticket<15?59u:0u;});
            require(rolls==4 && again.changed==first.changed,"Room revisit applied the 25 percent again");
            for(unsigned f=0;f<4;f++)require(MEM_HU((f+1)*16,objects)==replacement(0x4030+f,ticket),"Room revisit changed a fruit");
        }
        MEM_H(16,objects)=0xFFFF;
        visits.apply(rdram,uint32_t(meta),43,[&]{rolls++;return 0u;});
        require(MEM_HU(16,objects)==0xFFFF && rolls==4,"Room revisit resurrected a collected fruit");
        setup();visits.apply(rdram,uint32_t(meta),44,[&]{rolls++;return 0u;});
        require(rolls==8,"Independent rooms shared placement rolls");
        for(unsigned f=0;f<4;f++){
            const Visits::Source key{43,0x4198,0x43000000,0x43800000,0};
            visits.created(0x80020000,key);
            auto selected=visits.pick(0x4030+f,0x80020000,[&]{rolls++;return ticket;});
            const unsigned after=rolls;
            visits.enter();visits.created(0x80024000,key);
            require(visits.pick(0x4030+f,0x80024000,[&]{rolls++;return ticket<15?59u:0u;})==selected && rolls==after,"A recreated carrier rerolled after moving to another RAM address");
            visits.created(0x80024000,{44,0x4198,0x43000000,0x43800000,0});
            visits.pick(0x4030+f,0x80024000,[&]{rolls++;return ticket;});
            require(rolls==after+1,"Pooled address leaked another room's fruit choice");
        }
    }
    for(bool on:{false,true}){
        setup();nativeVisits={};enabled=on;recomp_context ctx{};ctx.r29=gpr(int32_t(0x80020000));ctx.r4=0;ctx.r5=header;ctx.r6=0;
        const auto stack=ctx.r29;func_80051418(rdram,&ctx);
        require(ctx.r29==stack && loaded.size()==std::size(ids)+(on?6:0),"Native resource preload contract changed");
        require(draws==(on?4:0),"Custom placement consumed random draws");
        if(on)for(unsigned i=0;i<6;i++)require(loaded[i]==0x4F00+i,"Hybrid resources were not preloaded");
        for(unsigned i=0;i<std::size(ids);i++){
            const auto want=on && i>=1 && i<=4?expected[i-1][(i-1)%3]:ids[i];
            require(loaded[i+(on?6:0)]==want && MEM_HU(i*16,objects)==want,"Replacement was not loaded before spawning");
            require(MEM_HU(i*16+2,objects)==0x1234+i,"Layer/placement parameter changed");
            for(unsigned j=4;j<16;j+=4)require(uint32_t(MEM_W(i*16+j,objects))==0x12345678+i+j,"Coordinates or other placement data changed");
        }
        for(unsigned visit=0;visit<50;visit++){
            if(visit%2)for(unsigned i=0;i<std::size(ids);i++)MEM_H(i*16,objects)=ids[i];
            ctx={};ctx.r29=stack;ctx.r5=header;
            func_80051418(rdram,&ctx);
            require(draws==(on?4:0),"Native room preload rerolled previously visited placements");
            require(loaded.size()==std::size(ids)+(on?6:0),"Revisited room failed to preload all required resources");
        }
    }
    setup();MEM_H(0,objects)=0x4006;
    require(!playable(rdram,uint32_t(meta)),"Menu was randomized");
    std::ifstream file(argv[1],std::ios::binary);std::vector<uint8_t> rom((std::istreambuf_iterator<char>(file)),{});
    auto nativeWord=[&](unsigned va){unsigned p=va-0x80000400+0x1000;return unsigned(rom.at(p))<<24|unsigned(rom.at(p+1))<<16|unsigned(rom.at(p+2))<<8|rom.at(p+3);};
    require(nativeWord(0x8009DF24+3*4)==2 && nativeWord(0x8009DF84+3*4)==2,"Native class 3 must grant 2 base score and 2 health");
    auto romWord=[&](unsigned p){return unsigned(rom.at(p))<<24|unsigned(rom.at(p+1))<<16|unsigned(rom.at(p+2))<<8|rom.at(p+3);};
    auto romHalf=[&](unsigned p){return unsigned(rom.at(p))<<8|rom.at(p+1);};
    for(auto root:{0x41D1u,0x4198u}){
        unsigned descriptor=0,variants=0;
        for(unsigned i=0;i<1582;i++){unsigned at=0xA1C90+12*i;if(romHalf(at)==root){descriptor=romWord(at+4);break;}}
        require(descriptor!=0,"Native fruit source descriptor missing");
        for(unsigned i=0;i<1582;i++){
            const unsigned at=0xA1C90+12*i;
            if(romWord(at+4)!=descriptor)continue;
            const auto id=uint16_t(romHalf(at));
            require(preview_source(id),"A native bubble/carrier variant may reroll its visible fruit");
            sources.clear();unsigned rolls=0;
            const auto first=sources.pick(0x4033,0x80020000,[&](){rolls++;return 2u;});
            require(first==0x4F05,"Grape carrier should select kiwi for ticket 2");
            for(unsigned n=0;n<3;n++)require(sources.pick(0x4033,0x80020000,[&](){rolls++;return 59u;})==first,"Carrier kiwi preview changed when recreated");
            require(rolls==1,"Carrier recreation consumed an extra ticket");variants++;
        }
        require(variants==(root==0x41D1?33u:8u),"Native bubble/carrier variant coverage changed");
    }
    printf("Hybrid mix verified: %u exhaustive probability cases (25%% total), 50 repeated visits of fresh/reused templates, stable recreated carriers, compatible choices, melons protected, deterministic seeds, actual native preload ordering, custom/menu exclusion and native 2-health/2-score class.\n",probabilityCases);
}
