#include "../boss_challenges0260.hpp"
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

namespace c=workshop::boss_challenges0260;
namespace fs=std::filesystem;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static std::string token(unsigned id){std::ostringstream s;s<<std::hex<<std::setfill('0')<<std::setw(32)<<id;return s.str();}
static c::CompletionEvidence evidence(unsigned color,unsigned id){
    c::CompletionEvidence e;e.token=token(id);e.provenance=c::Provenance::ProductionMenu;e.phaseComplete=true;e.completed=10;e.index=9;e.color=e.initialColor=color;e.player=e.initialPlayer=0x80010000;e.playerKind=0x4001;e.completionVi=1200;
    for(unsigned n=0;n<10;++n){auto& r=e.encounters[n];r.number=n+1;r.expectedMembers=1+n%4;r.defeatedMask=(1u<<r.expectedMembers)-1;r.defeatVi=100+n*100;r.color=color;r.player=e.player;r.initialized=r.active=r.terminal=r.causal=true;}return e;
}
static std::string bytes(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
static void put(const fs::path& p,std::string_view value){std::ofstream f(p,std::ios::binary);f<<value;if(!f)throw std::runtime_error("QA fixture write failed");}
int main(int argc,char** argv){try{
    if(argc!=2)throw std::runtime_error("Explicit NEW isolated QA output directory required");
    const fs::path out=fs::absolute(argv[1]);if(fs::exists(out))throw std::runtime_error("Refusing to overwrite a previous test directory");fs::create_directories(out);
    const auto normal=out/"normal";fs::create_directory(normal);const auto ledger=normal/"pc-boss-challenges.json";
    need(c::load(normal),"Empty explicit profile loads");need(c::snapshot().readable&&c::snapshot().saved&&c::snapshot().mask14==0&&!fs::exists(ledger),"Load must not create a ledger");
    need(!c::record({})&&!fs::exists(ledger),"Default evidence must never award");
    auto reject=[&](auto change){auto e=evidence(0,1);change(e);const auto before=c::snapshot();need(!c::record(e),"Invalid completion evidence admitted");const auto after=c::snapshot();need(before.mask14==after.mask14&&before.counts14==after.counts14&&!fs::exists(ledger),"Rejected evidence mutated progression or wrote a file");};
    reject([](auto& e){e.token="yes";});reject([](auto& e){e.provenance=c::Provenance::Unknown;});reject([](auto& e){e.provenance=c::Provenance::ScriptedQa;});reject([](auto& e){e.provenance=c::Provenance::ManualQa;});reject([](auto& e){e.provenance=c::Provenance::Simulation;});
    reject([](auto& e){e.phaseComplete=false;});reject([](auto& e){e.completed=9;});reject([](auto& e){e.completed=11;});reject([](auto& e){e.index=8;});reject([](auto& e){e.index=10;});reject([](auto& e){e.diagnostic=true;});reject([](auto& e){e.qaMarker=true;});reject([](auto& e){e.skippedEntry=true;});
    reject([](auto& e){e.initialPlayer+=4;});reject([](auto& e){e.player=e.initialPlayer=0;});reject([](auto& e){e.player=e.initialPlayer=0x80010001;});reject([](auto& e){e.playerKind=0;});reject([](auto& e){e.color=14;});reject([](auto& e){e.initialColor=1;});reject([](auto& e){e.completionVi=999;});
    for(unsigned n=0;n<10;++n){
        reject([&](auto& e){e.encounters[n].number=0;});reject([&](auto& e){e.encounters[n].expectedMembers=0;});reject([&](auto& e){e.encounters[n].expectedMembers=5;});reject([&](auto& e){e.encounters[n].defeatedMask=0;});reject([&](auto& e){e.encounters[n].defeatedMask|=16;});
        reject([&](auto& e){e.encounters[n].initialized=false;});reject([&](auto& e){e.encounters[n].active=false;});reject([&](auto& e){e.encounters[n].terminal=false;});reject([&](auto& e){e.encounters[n].causal=false;});reject([&](auto& e){e.encounters[n].player+=4;});reject([&](auto& e){e.encounters[n].color=1;});reject([&](auto& e){e.encounters[n].defeatVi=0;});
        if(n)reject([&](auto& e){e.encounters[n].defeatVi=e.encounters[n-1].defeatVi;});
    }
    for(unsigned color=0;color<14;++color){need(c::record(evidence(color,color+1)),"Qualified synthetic guard fixture rejected");need(c::snapshot().counts14[color]==1,"A native color challenge is missing");}
    need(c::snapshot().mask14==0x3FFF,"All fourteen challenge bits are represented");const auto original=bytes(ledger);need(!c::record(evidence(0,1))&&bytes(ledger)==original,"Duplicate completion altered ledger");
    need(c::load(normal)&&c::snapshot().mask14==0x3FFF&&!c::record(evidence(0,1)),"Reload lost once-only token or challenge state");
    auto letter=evidence(13,99);letter.token="abcdefabcdefabcdefabcdefabcdefab";need(c::record(letter),"Hex-letter token rejected");std::transform(letter.token.begin(),letter.token.end(),letter.token.begin(),[](char a){return a>='a'&&a<='f'?char(a-'a'+'A'):a;});need(!c::record(letter)&&c::snapshot().counts14[13]==2,"Uppercase replay bypassed canonical once-only tokens");
    const auto valid=bytes(ledger);
    for(const char* marker:{"boss-battle-qa.txt","solo-boss-qa.txt","manual-coop-qa.txt","native-coop-qa.txt","coop-qa.txt","manual-preview.txt"}){
        const auto profile=out/(std::string("marker-")+marker);fs::create_directory(profile);put(profile/marker,"QA");need(c::load(profile)&&!c::record(evidence(0,200))&&!fs::exists(profile/"pc-boss-challenges.json"),"Manual QA marker was allowed to grant permanent progression");
    }
    const auto portable=out/"portable-normal";fs::create_directory(portable);put(portable/"portable.txt","");need(c::load(portable)&&c::record(evidence(6,201)),"A normal portable profile was mistaken for QA");
    for(unsigned kind=0;kind<5;++kind){auto profile=out/("invalid-"+std::to_string(kind));fs::create_directory(profile);auto file=profile/"pc-boss-challenges.json";auto data=nlohmann::json::parse(valid);
        if(kind==0)put(file,"broken");else {if(kind==1)data["schema"]=2u;if(kind==2)data["mask14"]=0u;if(kind==3)data["counts14"][0]=10u;if(kind==4)data["runs"].push_back(data["runs"][0]);put(file,data.dump());}
        const auto before=bytes(file);need(!c::load(profile)&&!c::snapshot().readable&&!c::snapshot().saved,"Malformed ledger admitted");need(!c::record(evidence(0,300))&&bytes(file)==before,"Unreadable ledger overwritten or reset");
    }
    need(c::load(normal),"Normal ledger reloaded");put(ledger,"external-corruption-after-load");need(!c::record(evidence(0,301))&&!c::snapshot().readable&&bytes(ledger)=="external-corruption-after-load","Commit overwrote newly corrupted data");put(ledger,valid);
    need(c::load(normal),"Restored isolated fixture loads");put(normal/"boss-battle-qa.txt","added after load");need(!c::record(evidence(0,302))&&bytes(ledger)==valid,"A marker added after load was ignored");fs::remove(normal/"boss-battle-qa.txt");
#ifdef _WIN32
    HANDLE held=CreateFileW(ledger.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);need(held!=INVALID_HANDLE_VALUE,"QA destination lock failed");const auto before=c::snapshot();const bool saveAccepted=c::record(evidence(0,303));CloseHandle(held);
    need(!saveAccepted&&!c::snapshot().saved&&c::snapshot().counts14==before.counts14&&bytes(ledger)==valid,"Failed atomic replace changed the old file or counters");
    need(c::record(evidence(0,303))&&c::snapshot().saved&&c::snapshot().counts14[0]==2,"A failed save was not retryable exactly once");
#endif
    const auto concurrent=out/"concurrent";fs::create_directory(concurrent);need(c::load(concurrent),"Concurrent fixture loads");std::atomic_uint accepted{0};std::vector<std::thread> threads;for(unsigned n=0;n<12;++n)threads.emplace_back([&]{if(c::record(evidence(7,400)))++accepted;});for(auto& thread:threads)thread.join();need(accepted==1&&c::snapshot().counts14[7]==1&&c::snapshot().mask14==(1u<<7),"Concurrent duplicate callbacks awarded more than once");
    need(!c::load(out/"missing-profile")&&!c::record(evidence(0,500)),"An implicit/missing profile was created");
    nlohmann::json result{{"passed",true},{"checks",checks},{"scope","CPU evidence guards and isolated JSON persistence only; every qualified receipt is synthetic, not native gameplay proof"},{"fourteenColorsRepresented",true},{"defaultEvidenceRejected",true},{"tenStrictEncounterProofsRequired",true},{"qaManualSimulationExcluded",true},{"atomicReplacementFailurePreservesOldFile",true},{"duplicateTokensIdempotentAcrossReloadAndThreads",true},{"invalidFilesPreserved",true},{"gameLaunched",false},{"personalProfileUsed",false},{"nativeTenFightRunVerified",false}};put(out/"verification.json",result.dump(2)+'\n');std::cout<<result.dump(2)<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
