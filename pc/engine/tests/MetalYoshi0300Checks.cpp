#include "../metal_yoshis0300.hpp"
#include "../metal_rewards0300.hpp"
#include "../hybrid_completion0290.hpp"
#include "../hybrid_finds.hpp"
#include "../hybrid_rewards.hpp"
#include "../pause_favorites.hpp"
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
namespace fs=std::filesystem;
namespace m=workshop::metal_yoshis0300;
namespace c=workshop::yoshi_colors0300;
namespace hc=workshop::hybrid_completion0290;
namespace finds=workshop::finds;
static unsigned checks=0;
static void need(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
static std::string bytes(const fs::path& path){std::ifstream f(path,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
static void put(const fs::path& path,const std::string& text){std::ofstream f(path,std::ios::binary|std::ios::trunc);f<<text;f.flush();need(bool(f),"Fixture write failed");}
static nlohmann::json journal(unsigned found,unsigned alive){return {{"schema",1u},{"foundMask",found},{"storyAliveMask",alive}};}
static void completion(const fs::path& dir,uint32_t mask){put(dir/"complete.json",nlohmann::json{{"schema",1u},{"observedMask",mask},{"legacyMask",0u},{"legacyChecked",true},{"legacyChecksum",nullptr}}.dump());hc::initialize(dir/"complete.json",dir/"no-old-records.json");}
static void legacy_finds(const fs::path& path,unsigned mask){put(path,nlohmann::json{{"version",1u},{"legacyMigrated",true},{"foundMask",mask}}.dump());finds::initialize(path);}
static void no_temps(const fs::path& dir){for(const auto& entry:fs::recursive_directory_iterator(dir))if(entry.is_regular_file())need(entry.path().filename().string().find(".tmp-")==std::string::npos,"Atomic replacement left a temporary file");}
static void setup_progress(uint8_t* rdram){
    workshop::progress::State p{0xFFFFFF,0,{1,2,3,4},3,63};workshop::progress::write(rdram,gpr(int32_t(0x807F2000)),p);
    MEM_W(0,int32_t(0x800F8DF4))=0x5A5A3FFF;MEM_W(0,int32_t(0x807F1000))=63;MEM_W(0,int32_t(0x807F2024))=0x11223344;
}
static unsigned reserve(uint8_t* rdram,unsigned level,unsigned expected){
    finds::reset_attempt();finds::enter(level,43,true);finds::migrate(rdram,0);
    need(!finds::reserve(63,0x46AA,1,2,true),"Occupied native White Shy Guy slot was replaced");
    const auto slot=finds::reserve(63,0x46AA,1,2,false);need(slot&&*slot==expected,"Queue did not retain six legacy colors before metals");
    need(!finds::reserve(63,0x46AE,3,4,false),"A second source offered another egg in the same attempt");return *slot;
}
static void carry(unsigned slot,uint32_t actor=0x80030000){finds::spawned(actor,slot,false);need(finds::egg_state(actor,4),"Typed synthetic egg was not recognized");}
int main(int argc,char** argv){try{
    need(argc==2,"Supply a NEW disposable fixture directory");const fs::path out=argv[1];need(!fs::exists(out),"Never overwrite existing fixtures");fs::create_directories(out);
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();setup_progress(rdram);
    for(unsigned color=0;color<16;++color){need(c::native_alias(color)<8,"PC color escaped native eight-entry table");need(c::native_alias(color)==(color<8?color:color<14?0:color-8),"Legacy/native alias changed");}
    need(c::native_alias(16)==8&&c::metal_bit(14)==1&&c::metal_bit(15)==2&&!c::metal_bit(13),"Metal bounds/bit identity changed");
    need(workshop::favorites::list(14).count==1&&workshop::favorites::list(15).count==1,"Metal favorite HUD did not inherit White/Black indicator");
    for(unsigned color=0;color<14;++color)for(unsigned id:{0x4030u,0x4034u,0x4F00u,0x45B6u})need(!workshop::metal_rewards0300::food(color,id,false),"Metal policy changed an established color");
    for(unsigned color:{14u,15u})for(bool score:{false,true}){
        for(unsigned id=0x4030;id<=0x4034;++id){need(workshop::metal_rewards0300::food(color,id,score)==2,"Classic fruit/melon did not give2");need(workshop::metal_rewards0300::food(color,id,score,6)==8,"Native Lucky fruit lost8 priority");}
        for(unsigned id=0x4F00;id<0x4F06;++id){need(workshop::metal_rewards0300::food(color,id,score)==3,"Hybrid fruit did not give3");need(workshop::metal_rewards0300::food(color,id,score,6)==8,"Hybrid Lucky fruit lost8 priority");}
        need(workshop::metal_rewards0300::food(color,0x45B6,score)==(score?1:4),"Pepper did not keep1point and4health");need(!workshop::metal_rewards0300::food(color,0x403A,score),"Enemy reward did not retain native White/Black path");
    }
    const auto file=out/"pc-metal-yoshis.json";m::initialize(file);need(m::view().initialized&&m::view().readable&&!fs::exists(file),"Missing journal created an entitlement");
    completion(out,0x7FFFFF);for(unsigned mode:{0u,1u,2u,3u,4u}){m::start_session(mode);need(!m::search_mask()&&!m::acquire(14),"Incomplete Hybrid journal allowed metal search/acquisition");}
    completion(out,0xFFFFFF);for(unsigned mode:{0u,1u,3u}){m::start_session(mode);need(m::search_mask()==3,"Story/Trial/Hybrid search was not admitted after24Hybrid clears");}m::start_session(2);need(!m::search_mask()&&!m::acquire(14),"Practice granted a permanent metal Yoshi");m::start_session(4);need(!m::search_mask()&&!m::unlocked_extra(),"Boss mode enabled metal colors");
    m::start_session(3,false);need(!m::search_mask()&&!m::unlocked_extra()&&!m::acquire(14),"Excluded cooperative/custom/Boss scope enabled metals");m::set_scope(true);need(m::search_mask()==3,"Ordinary solo scope failed to recover");
    need(!m::acquire(13)&&!m::acquire(16)&&!fs::exists(file),"Invalid color wrote metal ownership");
    const auto legacy_path=out/"pc-hybrid-finds.json";legacy_finds(legacy_path,0);finds::migrate(rdram,0);
    for(unsigned color=0;color<6;++color){const auto slot=reserve(rdram,color,color);carry(slot);need(finds::pending_hybrids()==(1u<<color)&&finds::pending_metals()==0,"Legacy egg was reported as a metal");need(finds::finish(rdram),"Established hybrid queue stopped working");need(!m::view().found&&!fs::exists(file),"Established hybrid finish granted metal ownership");}
    const auto legacy_before=bytes(legacy_path);const auto native_progress=workshop::progress::read(rdram,gpr(int32_t(0x807F2000)));const auto flags=MEM_W(0,int32_t(0x800F8DF4));const auto dirty=MEM_W(0,int32_t(0x807F2024));
    auto slot=reserve(rdram,10,6);carry(slot);need(finds::pending()==64&&finds::pending_hybrids()==0&&finds::pending_metals()==1&&!m::view().found,"Gold pickup granted ownership before finishing");finds::reset_attempt();need(!finds::pending()&&!m::view().found&&!fs::exists(file),"Death/abandonment granted an unconfirmed metal");
    slot=reserve(rdram,10,6);carry(slot);finds::spawned(0x80032000,slot,false);finds::egg_state(0x80032000,4);finds::egg_state(0x80030000,0);need(finds::pending()==64,"Scene/pool handoff lost the exact carried metal");
    finds::enter(10,44,true);need(finds::pending_metals()==1&&!finds::reserve(63,0x46AE,9,9,false),"Room change lost or duplicated the metal egg");
    const auto before=memory;auto expected=before;auto* saved=rdram;rdram=expected.data();MEM_W(0,int32_t(0x807F1000))|=64;rdram=saved;
    need(finds::finish(rdram)&&memory==expected,"Gold finish changed native state outside the extra-life projection");need(m::view().found==1&&m::view().story_alive==1&&finds::view().found==63&&bytes(legacy_path)==legacy_before,"Gold finish changed historical ownership/ledger");
    need(workshop::progress::read(rdram,gpr(int32_t(0x807F2000))).lives==native_progress.lives&&MEM_W(0,int32_t(0x800F8DF4))==flags&&MEM_W(0,int32_t(0x807F2024))==dirty,"Gold leaked into six-bit EEPROM lives/native White/Black flags");
    need(m::search_mask()==2&&m::unlocked_extra()==64,"Gold did not reveal only the remaining Silver search");
    const auto first=bytes(file);need(m::acquire(14)&&bytes(file)==first,"Repeated Gold ownership rewrote the journal");m::initialize(file);m::start_session(1);need(m::view().found==1&&m::search_mask()==2,"Ownership/search did not survive restart");
    slot=reserve(rdram,11,7);carry(slot);need(finds::pending_metals()==2&&finds::pending_hybrids()==0,"Silver pending field was not distinct");
#ifdef _WIN32
    HANDLE held=CreateFileW(file.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);need(held!=INVALID_HANDLE_VALUE,"Could not prepare atomic replacement failure");need(!finds::finish(rdram)&&m::view().found==1&&!m::view().saved&&finds::pending_metals()==2&&bytes(file)==first,"Failed replacement granted/discarded Silver");CloseHandle(held);need(finds::finish(rdram),"Silver failed to retry after IO recovered");
#else
    need(finds::finish(rdram),"Silver finish failed");
#endif
    need(m::view().found==3&&m::view().story_alive==3&&m::view().saved&&m::search_mask()==0&&m::unlocked_extra()==192,"Two metal rights did not persist distinctly");need(bytes(legacy_path)==legacy_before,"Silver modified the historical six-bit ledger");
    m::start_session(0);need(m::save_story_alive(2)&&m::story_extra(true)==128&&m::story_extra(false)==192,"Independent Story life checkpoint did not preserve a Gold loss");const auto lost=bytes(file);need(m::acquire(14)&&bytes(file)==lost&&m::view().story_alive==2,"Repeated ownership revived a lost Story color");need(!m::save_story_alive(4)&&bytes(file)==lost,"Invalid Story live bits overwrote state");m::start_session(1);need(m::restore_life(14)&&m::view().story_alive==2&&m::unlocked_extra()==192,"Trial rescue/fresh availability changed the saved Story life");m::start_session(0);need(m::restore_life(14)&&m::view().story_alive==3,"Native Story rescue could not restore its earned metal life");need(m::save_story_alive(0)&&m::story_extra(true)==0&&m::story_extra(false)==192,"Losing both metals revoked ownership or fresh Story availability");
    m::initialize(file);m::start_session(2);need(m::view().found==3&&m::view().story_alive==0&&m::unlocked_extra()==192&&!m::search_mask(),"Reload/Practice projected unearned colors or reset losses");m::set_scope(false);need(!m::unlocked_extra()&&!m::story_extra(false)&&m::view().found==3,"Delayed-mode filter erased ownership instead of hiding availability");
    completion(out,0);m::start_session(3);need(m::unlocked_extra()==192&&!m::search_mask(),"Losing completion evidence erased already acquired metal rights");
    const auto proof=workshop::progress::State{0xFFFFFF,0,{1,2,3,4},3,63};
    need(workshop::progress::tally_lives(proof,0xFFFFFF,true,6,0,0xFFFF,0)==14,"Default tally changed the fourteen-color compatibility contract");need(workshop::progress::tally_lives(proof,0xFFFFFF,true,6,0,0xFFFF,0,0xFFFF)==16,"Explicit solo tally dropped metal lives");need(workshop::progress::valid(proof,0xFFFFFF),"Six-byte legacy progression became invalid");
    std::vector<std::string> invalid={"{","{\"schema\":2}",journal(4,0).dump(),journal(0,1).dump(),std::string(4097,'x'),"{\"schema\":1,\"schema\":1,\"foundMask\":0,\"storyAliveMask\":0}"};auto bad=journal(0,0);bad["foundMask"]=-1;invalid.push_back(bad.dump());bad=journal(0,0);bad["foundMask"]=1.0;invalid.push_back(bad.dump());bad=journal(0,0);bad["unknown"]=1;invalid.push_back(bad.dump());
    for(unsigned i=0;i<invalid.size();++i){const auto dir=out/("bad-"+std::to_string(i));fs::create_directory(dir);const auto target=dir/"metal.json";put(target,invalid[i]);m::initialize(target);m::start_session(3);need(!m::view().readable&&!m::search_mask()&&!m::unlocked_extra()&&!m::acquire(14)&&!m::clear_save()&&bytes(target)==invalid[i],"Invalid/future metal journal was overwritten or credited");}
    const auto fallback=out/"legacy-with-bad-metal.json";legacy_finds(fallback,0);finds::migrate(rdram,0);finds::reset_attempt();finds::enter(0,43,true);need(finds::reserve(63,0x46AA,1,2,false)==0,"Unreadable metal file broke established hybrid acquisition");
    const auto external=out/"external.json";put(external,journal(1,1).dump());m::initialize(external);m::start_session(0);put(external,journal(3,3).dump());need(m::save_story_alive(1)&&m::view().found==3&&m::view().story_alive==1,"A valid external acquisition was lost on Story save");put(external,"external corruption");need(!m::save_story_alive(0)&&!m::view().readable&&bytes(external)=="external corruption","New corruption was overwritten by a Story checkpoint");
    const auto vanished=out/"disappeared.json";put(vanished,journal(1,1).dump());m::initialize(vanished);m::start_session(0);fs::remove(vanished);need(!m::save_story_alive(0)&&!m::view().readable&&!fs::exists(vanished),"A vanished existing journal was silently recreated");
    m::initialize(file);m::start_session(0);need(m::clear_save()&&m::view().found==0&&m::view().story_alive==0,"Explicit progression erase retained metal ownership");need(bytes(legacy_path)==legacy_before,"Metal erase touched the old six-color file");no_temps(out);
    nlohmann::json report{{"passed",true},{"checks",checks},{"corruptMetalCases",invalid.size()},{"allAdmissionsSynthetic",true},{"nativeRAMMutationScope","Gold/Silver finish onlyORstheir807F1000bits; no legacy EEPROM state or native White/Black flags"},{"legacySixBitCodecRetained",true},{"gameLaunched",false},{"EEPROMReadOrWritten",false},{"personalFilesReadOrWritten",false}};put(out/"execution.json",report.dump(2)+'\n');std::cout<<report.dump()<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
