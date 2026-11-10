#include "../hybrid_completion0290.hpp"
#include "json/json.hpp"
#include <array>
#include <bit>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
namespace h=workshop::hybrid_completion0290;
namespace fs=std::filesystem;
static unsigned checks=0,native_calls=0;
static void need(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
static std::string bytes(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
static void put(const fs::path& p,const std::string& value){std::ofstream f(p,std::ios::binary|std::ios::trunc);f<<value;f.flush();need(bool(f),"Fixture write failed");}
static uint32_t checksum(const std::array<uint8_t,1000>& records){uint32_t sum=2166136261u;for(auto n:records)sum=(sum^n)*16777619u;return sum;}
static nlohmann::json legacy(const std::array<uint8_t,1000>& records){return {{"version",1u},{"records",records},{"checksum",checksum(records)}};}
static nlohmann::json journal(uint32_t observed,uint32_t imported=0,bool checked=true){return {{"schema",1u},{"observedMask",observed},{"legacyMask",imported},{"legacyChecked",checked},{"legacyChecksum",imported?nlohmann::json(123u):nlohmann::json(nullptr)}};}
static void no_temps(const fs::path& dir){for(const auto& e:fs::directory_iterator(dir))need(e.path().filename().string().find(".tmp-")==std::string::npos,"Failed atomic save left a temporary file");}
struct Fixture {
    std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);
    uint8_t* rdram=memory.data();recomp_context context{};
    static constexpr auto root=gpr(int32_t(0x800F7EA0)),player=gpr(int32_t(0x80010000)),priv=gpr(int32_t(0x80012000));
    Fixture(){context.r4=root;context.r29=gpr(int32_t(0x80030000));context.r31=0x12345678;MEM_W(0,player)=0x19971110;MEM_H(12,player)=0x4001;MEM_W(0x1F4,player)=uint32_t(priv);MEM_W(0x988,root)=uint32_t(player);MEM_W(0xCE8,root)=1;}
    void select(unsigned level){MEM_B(0xF5C,root)=uint8_t(level/4);MEM_B(0xF5D,root)=uint8_t(level%4);}
    void step(unsigned fruits,unsigned ending,bool allowed=true){MEM_W(0xB30,root)=fruits;MEM_W(0xC28,root)=ending;const auto before=memory;const auto registers=context;h::frame(rdram,&context,allowed);need(memory==before&&!std::memcmp(&context,&registers,sizeof(context)),"Completion observer mutated native RAM/registers");}
    void run(unsigned level){select(level);step(0,0);step(29,0);step(30,1);step(30,1);}
};
extern "C" void func_80069644(uint8_t*,recomp_context*);
extern "C" void func_800696D4(uint8_t*,recomp_context*);
static void native_layout(Fixture& f){
    auto* rdram=f.rdram;
    constexpr auto destination=gpr(int32_t(0x80020000));
    for(unsigned level=0;level<24;++level)for(unsigned rank=0;rank<5;++rank){
        const unsigned at=0xFE8+level*40+rank*8;for(unsigned n=0;n<6;++n)MEM_B(int(at+2+n),Fixture::root)=uint8_t(16+(level+rank+n)%32);
        recomp_context c{};c.r4=level;c.r5=rank;c.r6=destination;c.r29=gpr(int32_t(0x80040000));func_800696D4(f.rdram,&c);++native_calls;
        for(unsigned n=0;n<6;++n)need(MEM_HU(int(n*2),destination)==16+(level+rank+n)%32,"Actual native Trial getter disagrees with24x5x8 layout");
    }
    for(unsigned rank=0;rank<5;++rank){for(unsigned n=0;n<6;++n)MEM_B(int(0xFC2+rank*8+n),Fixture::root)=uint8_t(80+rank+n);recomp_context c{};c.r4=rank;c.r5=destination;c.r29=gpr(int32_t(0x80040000));func_80069644(f.rdram,&c);++native_calls;for(unsigned n=0;n<6;++n)need(MEM_HU(int(n*2),destination)==80+rank+n,"Actual native Story getter disagrees with leading5x8 records");}
}
int main(int argc,char** argv){try{
    need(argc==2,"Supply a NEW disposable fixture directory");const fs::path out=argv[1];need(!fs::exists(out),"Never overwrite existing test data");fs::create_directories(out);
    Fixture f;auto* rdram=f.rdram;native_layout(f);
    const auto path=out/"pc-hybrid-completion.json",source=out/"pc-hybrid-records.json";
    put(out/"pc-achievements.json","merged ordinary and hybrid achievements must remain unread");put(out/"menu-progress.json","ordinary Trial completion must remain unread");put(out/"native-eeprom.bin","untouched EEPROM sentinel");
    const auto ordinary=bytes(out/"pc-achievements.json"),menu=bytes(out/"menu-progress.json"),eep=bytes(out/"native-eeprom.bin");
    h::initialize(path,source);need(h::view().readable&&h::view().saved&&h::view().count==0&&!fs::exists(path),"Missing Hybrid records invented progress or eagerly created a journal");
    h::start_session(3);f.select(0);f.step(30,1);need(h::view().mask==0&&!fs::exists(path),"Historical result earned a new completion at startup");
    for(unsigned mode:{0u,1u,2u,4u,5u}){h::start_session(mode);f.run(0);need(h::view().mask==0,"Another session mode earned Hybrid completion");}
    h::start_session(3);for(unsigned native_mode:{0u,2u,3u}){MEM_W(0xCE8,Fixture::root)=native_mode;f.run(0);need(h::view().mask==0,"Non-Trial native mode earned Hybrid completion");}MEM_W(0xCE8,Fixture::root)=1;
    f.select(0);f.step(0,0);f.step(30,1,false);f.step(30,1);need(h::view().mask==0,"Disallowed/custom interval retained a stale attempt");
    f.step(0,0);f.step(30,0);f.step(29,1);f.step(31,1);need(h::view().mask==0,"Non-success fruit/ending states earned Hybrid completion");
    h::start_session(3);f.select(0);f.step(0,0);f.select(1);f.step(30,1);need(h::view().mask==0,"Changing level inherited the previous attempt");
    h::start_session(3);f.select(0);f.step(0,0);MEM_W(0x1F4,Fixture::player)=uint32_t(Fixture::priv)+4;f.step(30,1);need(h::view().mask==0,"Changing player incarnation inherited the previous attempt");MEM_W(0x1F4,Fixture::player)=uint32_t(Fixture::priv);
    f.step(0,0);MEM_W(0,Fixture::player)=0;f.step(30,1);MEM_W(0,Fixture::player)=0x19971110;f.step(30,1);need(h::view().mask==0,"Invalid native player retained a stale attempt");
    f.select(24);f.step(0,0);f.step(30,1);MEM_B(0xF5C,Fixture::root)=0;MEM_B(0xF5D,Fixture::root)=4;f.step(0,0);f.step(30,1);need(h::view().mask==0,"Out-of-range page/column earned Hybrid completion");
    f.select(0);f.step(0,0);h::start_session(3);f.step(30,1);need(h::view().mask==0,"Starting a session retained an armed prior result");
    for(unsigned level=0;level<24;++level){f.run(level);need(h::view().mask==((1u<<(level+1))-1)&&h::view().count==level+1,"Real synthetic Hybrid finish lost its proper level");const auto before=bytes(path);f.step(30,1);need(bytes(path)==before,"Repeated ending rewrote/credited the same attempt");}
    need(h::view().mask==h::all_levels&&h::view().legacy_mask==0,"Observed completions were mislabeled as legacy records");h::initialize(path,source);need(h::view().mask==h::all_levels&&h::view().count==24,"Observed completions did not survive restart");
    need(bytes(out/"pc-achievements.json")==ordinary&&bytes(out/"menu-progress.json")==menu&&bytes(out/"native-eeprom.bin")==eep,"Observer touched ordinary progress/achievements/EEPROM");
    const auto migration=out/"migration";fs::create_directory(migration);
    for(unsigned level=0;level<24;++level)for(unsigned rank=0;rank<5;++rank){
        const auto dir=migration/(std::to_string(level)+"-"+std::to_string(rank));fs::create_directory(dir);std::array<uint8_t,1000> records{};
        // Also put records in every Story slot: they must never leak into Trial.
        for(unsigned story=0;story<5;++story){records[story*8]=0x7F;records[story*8+1]=0xFF;}
        const auto at=(5+level*5+rank)*8;records[at]=uint8_t(rank?0:1);records[at+1]=uint8_t(rank?rank:0);
        const auto old=dir/"pc-hybrid-records.json",ledger=dir/"journal.json";put(old,legacy(records).dump());const auto before=bytes(old);h::initialize(ledger,old);
        need(h::view().readable&&h::view().saved&&h::view().legacy_checked&&h::view().mask==(1u<<level)&&h::view().legacy_mask==(1u<<level),"Legacy migration misread a level/rank or native big-endian score");need(bytes(old)==before,"Legacy migration modified Hybrid records");
        const auto committed=bytes(ledger);h::initialize(ledger,old);need(bytes(ledger)==committed&&h::view().mask==(1u<<level),"Legacy migration was not idempotent");
    }
    const auto story_only=out/"story-only";fs::create_directory(story_only);std::array<uint8_t,1000> records{};for(unsigned row=0;row<125;++row)for(unsigned n=0;n<6;++n)records[row*8+2+n]=80;
    for(unsigned row=0;row<5;++row)records[row*8+1]=42;put(story_only/"old.json",legacy(records).dump());h::initialize(story_only/"journal.json",story_only/"old.json");need(h::view().mask==0&&h::view().legacy_checked,"Story scores or names without scores were treated as Trial wins");
    const auto all=out/"all-legacy";fs::create_directory(all);records.fill(0);for(unsigned level=0;level<24;++level)records[(5+level*5+4)*8+1]=1;put(all/"old.json",legacy(records).dump());const auto all_before=bytes(all/"old.json");h::initialize(all/"journal.json",all/"old.json");need(h::view().count==24&&h::view().legacy_mask==h::all_levels,"All24 legacy Hybrid records were not retained");const auto imported=bytes(all/"journal.json");put(all/"old.json","externally corrupted after completed migration");h::initialize(all/"journal.json",all/"old.json");need(h::view().mask==h::all_levels&&bytes(all/"journal.json")==imported,"Completed migration depended on or overwrote a later changed legacy file");need(all_before!=bytes(all/"old.json"),"Invalid legacy fixture was not applied");
    std::vector<std::string> bad_journals={"{","{\"schema\":99}",journal(0x1000000).dump(),"{\"schema\":1,\"schema\":1,\"observedMask\":0,\"legacyMask\":0,\"legacyChecked\":true,\"legacyChecksum\":null}"};
    auto bad=journal(0);bad["observedMask"]=-1;bad_journals.push_back(bad.dump());bad=journal(0);bad["observedMask"]=1.0;bad_journals.push_back(bad.dump());bad=journal(0);bad["legacyMask"]=1;bad_journals.push_back(bad.dump());bad=journal(0);bad["unknown"]=1;bad_journals.push_back(bad.dump());bad=journal(0);bad["legacyChecked"]=false;bad["legacyChecksum"]=1u;bad_journals.push_back(bad.dump());bad_journals.push_back(std::string(4097,'x'));
    for(unsigned i=0;i<bad_journals.size();++i){const auto dir=out/("bad-journal-"+std::to_string(i));fs::create_directory(dir);const auto file=dir/"journal.json";put(file,bad_journals[i]);h::initialize(file,dir/"absent.json");h::start_session(3);f.run(0);need(!h::view().readable&&!h::view().saved&&bytes(file)==bad_journals[i],"Corrupt/unknown journal was credited or overwritten");}
    records.fill(0);records[40+1]=1;const auto valid_old=legacy(records);std::vector<std::string> bad_sources={"{",std::string(32769,'x')};
    bad=valid_old;bad["version"]=2u;bad_sources.push_back(bad.dump());bad=valid_old;bad["checksum"]=0u;bad_sources.push_back(bad.dump());bad=valid_old;bad["records"][0]=256u;bad_sources.push_back(bad.dump());bad=valid_old;bad["records"][0]=-1;bad_sources.push_back(bad.dump());bad=valid_old;bad["records"][0]=1.0;bad_sources.push_back(bad.dump());bad=valid_old;bad["records"].erase(bad["records"].end()-1);bad_sources.push_back(bad.dump());bad=valid_old;bad["unknown"]=1;bad_sources.push_back(bad.dump());records[42]=1;bad_sources.push_back(legacy(records).dump());bad_sources.push_back("{\"version\":1,\"version\":1,\"records\":[],\"checksum\":0}");
    for(unsigned i=0;i<bad_sources.size();++i){const auto dir=out/("bad-legacy-"+std::to_string(i));fs::create_directory(dir);const auto file=dir/"old.json",ledger=dir/"journal.json";put(file,bad_sources[i]);h::initialize(ledger,file);h::start_session(3);f.run(0);need(!h::view().readable&&!h::view().legacy_readable&&h::view().mask==0&&!fs::exists(ledger)&&bytes(file)==bad_sources[i],"Corrupt/unknown legacy records earned credit or were overwritten");}
    const auto external=out/"external";fs::create_directory(external);put(external/"journal.json",journal(1).dump());h::initialize(external/"journal.json",external/"none.json");put(external/"journal.json",journal(3).dump());h::start_session(3);f.run(2);need(h::view().mask==7,"External valid journal update was lost during a completion");
    const auto preserved=bytes(external/"journal.json");put(external/"journal.json","external corruption");f.run(3);need(!h::view().readable&&bytes(external/"journal.json")=="external corruption"&&h::view().mask==7,"Completion overwrote a newly corrupted journal");need(preserved!="external corruption","Corruption fixture invalid");
#ifdef _WIN32
    const auto failure=out/"save-failure";fs::create_directory(failure);const auto target=failure/"journal.json";put(target,journal(1).dump());h::initialize(target,failure/"none.json");const auto before=bytes(target);
    HANDLE held=CreateFileW(target.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);need(held!=INVALID_HANDLE_VALUE,"Unable to prepare replacement-denial fixture");h::start_session(3);f.run(1);need(!h::view().saved&&h::view().readable&&h::view().mask==1&&bytes(target)==before,"Failed atomic replacement published uncommitted progress");CloseHandle(held);no_temps(failure);f.step(30,1);need(h::view().saved&&h::view().mask==3,"Proven completion was not retryable after an IO failure");
#endif
    no_temps(out);
    nlohmann::json report{{"passed",true},{"checks",checks},{"actualNativeGetterCalls",native_calls},{"legacyLevelRankCases",120},{"nativeLayout","5 Story records then24x5 Trial records;8bytes/record;BE16score+6glyphs"},{"corruptJournalCases",bad_journals.size()},{"corruptLegacyCases",bad_sources.size()},{"nativeFramesSynthetic",true},{"migrationSourcesSynthetic",true},{"gameLaunched",false},{"EEPROMReadOrWritten",false},{"personalFilesReadOrWritten",false}};
    put(out/"execution.json",report.dump(2)+'\n');std::cout<<report.dump()<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
