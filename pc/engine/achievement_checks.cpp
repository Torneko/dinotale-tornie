#include "achievements.hpp"
#include "json/json.hpp"
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdlib>
using namespace workshop::achievements;
static void check(bool b,const char* message){if(!b){std::cerr<<message<<'\n';std::exit(1);}}
int main(int argc,char** argv){
    check(argc==2,"Expected disposable test directory");
    const auto folder=std::filesystem::path(argv[1]);std::filesystem::create_directories(folder);
    auto path=folder/"achievements.json";check(!std::filesystem::exists(path),"Never overwrite a previous test ledger");
    Tracker tracker;Sample s;s.playable=true;s.level=0;s.mode=0;s.health=8;
    s.fruits=30;s.ending=1;s.ring.fill(0);
    check(!tracker.observe(s),"A historical result earned a new achievement");
    for(unsigned fruit=0;fruit<12;fruit++){
        tracker.reset();s.fruits=0;s.ending=0;tracker.observe(s);
        s.fruits=30;s.ending=1;s.ring.fill(fruit);
        auto got=tracker.observe(s);
        if(fruit==5)check(!got,"Smiling hearts counted as fruit");
        else check(got && got->sameFruit==(1u<<fruit),"A classic or hybrid fruit challenge was lost");
        check(!tracker.observe(s),"One finish counted twice");
    }
    tracker.reset();s.fruits=0;s.ending=0;tracker.observe(s);
    s.fruits=30;s.ending=1;s.ring.fill(0);s.ring[29]=6;
    check(tracker.observe(s)->sameFruit==0,"Mixed fruit earned the identical-fruit challenge");
    tracker.reset();s.fruits=0;s.ending=0;s.hearts=0;tracker.observe(s);
    s.hearts=3;check(!tracker.observe(s),"Three hearts without a finished level earned the challenge");
    s.fruits=30;s.ending=1;check(tracker.observe(s)->hearts==3,"Three hearts were not captured at the finish");
    s.fruits=0;s.ending=0;s.hearts=0;tracker.observe(s);s.fruits=30;s.ending=1;s.hearts=1;
    check(tracker.observe(s)->hearts==1,"Heart totals accumulated between attempts");
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
    constexpr auto root=gpr(int32_t(0x800F7EA0)),player=gpr(int32_t(0x80010000));
    recomp_context ctx{};ctx.r4=root;ctx.r29=gpr(int32_t(0x80020000));ctx.r31=0x1234;
    MEM_W(0x988,root)=uint32_t(player);MEM_H(12,player)=0x4001;MEM_W(0xF14,root)=8;
    MEM_W(0xCE8,root)=0; // native Story mode
    auto begin=[&]{MEM_W(0xB30,root)=0;MEM_W(0xC28,root)=0;frame(rdram,&ctx,true);};
    auto finish=[&](unsigned fruit){MEM_W(0xB30,root)=30;MEM_W(0xC28,root)=1;for(unsigned i=0;i<30;i++)MEM_W(0xB34+i*4,root)=fruit;frame(rdram,&ctx,true);};
    initialize(path);start_session(0);begin();
    const auto registers=ctx;
    MEM_W(0xB30,root)=30;MEM_W(0xC28,root)=1;for(unsigned i=0;i<30;i++)MEM_W(0xB34+i*4,root)=11;
    MEM_W(0xAB8,root)=3;
    const auto before=memory;frame(rdram,&ctx,true);frame(rdram,&ctx,true);
    check(memory==before && !memcmp(&ctx,&registers,sizeof(ctx)),"Tracking changed native RAM or registers");
    check(view().levels[0].completions==1 && view().levels[0].sameFruit==(1u<<11) && view().levels[0].fullHealth,"Native snapshot did not persist the kiwi/full-health finish");
    check(view().levels[0].allHearts && view().levels[0].bestHearts==3,"Native red heart finish was not saved");
    initialize(path);check(view().levels[0].completions==1,"Achievements did not survive restart");
    start_session(2);begin();finish(0);check(view().levels[0].completions==1,"Practice earned achievements");
    start_session(1);begin();MEM_W(0xB30,root)=30;frame(rdram,&ctx,false);
    check(view().levels[0].completions==1,"Editor mode earned achievements");
    MEM_W(0xCE8,root)=1;MEM_W(0xF14,root)=7;begin();finish(0);
    check(view().levels[0].completions==2 && view().levels[0].modes==3 && view().levels[0].sameFruit==((1u<<11)|1),"Story and Trial records were not merged");
    check(view().levels[0].fullHealth && view().levels[0].bestHealth==8,"A later run erased a better result");
    nlohmann::json backup;std::ifstream(path.string()+".bak")>>backup;
    check(backup["levels"]["1-1"]["completions"]==1,"Atomic save backup did not retain the previous ledger");
    start_session(3);begin();finish(6);
    check(view().levels[0].completions==3 && view().levels[0].sameFruit==((1u<<11)|1|(1u<<6)),"Hybrid mode did not contribute to the same level achievements");
    start_session(1);begin();finish(6);
    check(view().levels[0].completions==4 && view().levels[0].sameFruit==((1u<<11)|1|(1u<<6)),"Classic Trial did not share the already earned hybrid fruit challenge");
    const auto corrupt=folder/"future.json";{std::ofstream file(corrupt);file<<"{\"version\":99}";}
    initialize(corrupt);start_session(0);begin();finish(0);
    std::string content;std::getline(std::ifstream(corrupt),content);
    check(!view().readable && content=="{\"version\":99}","Unsupported ledger was overwritten");
    auto legacy=folder/"legacy-v1.json";{std::ofstream file(legacy);file<<nlohmann::json{{"version",1},{"levels",{{"1-1",{{"completions",1},{"modes",1},{"bestHealth",8},{"fullHealth",true},{"sameFruitMask",1}}}}}};}
    initialize(legacy);check(view().readable && view().levels[0].fullHealth && !view().levels[0].allHearts,"0.9 achievements were lost or new hearts awarded retroactively");
    std::cout<<"Achievements: 11 fruit types, mixed/heart rejection, no retroactive awards, duplicate protection, native RAM/register preservation, persistence/backup, Story/Trial merge, Practice/custom exclusions and unsupported-file preservation passed.\n";
}
