#include "../story_hybrid0320.hpp"
#include "../hybrid_finds.hpp"
#include "../metal_yoshis0300.hpp"
#include "../metal_native0300.hpp"
#include "../metal_custom0311.hpp"
#include "../hybrid_completion0290.hpp"
#include "../story_progress.hpp"
#include "../game_modes0320.hpp"
#include "json/json.hpp"
#include <array>
#include <atomic>
#include <bit>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>
namespace fs=std::filesystem;
namespace sh=workshop::story_hybrid0320;
namespace f=workshop::finds;
namespace m=workshop::metal_yoshis0300;
namespace mn=workshop::metal_native0300;
namespace hc=workshop::hybrid_completion0290;
namespace progress=workshop::progress;
using Json=nlohmann::json;
// Boundary fixtures only. The extracted Loader wrappers below are byte-exact.
// Actual scene/controller/EEPROM services are not linked or represented as real.
static int selected_mode=5;static bool menu_enabled=true,ordinary_scope=true,custom_scope=false;
bool workshop_menu_enabled(){return menu_enabled;}int workshop_menu_session_mode(){return selected_mode;}
static bool metal_solo_scope(){return ordinary_scope;}static bool metal_custom_scope0311(){return custom_scope;}static bool metal_play_scope0311(){return ordinary_scope||custom_scope;}
static uint32_t progress_payload=0,progress_boot_story=0;static bool progress_reading=false,story_lives_ready0320=false;
static bool fruit_audit=false;static std::atomic_uint test_vi=0;static unsigned native_tally_mask=0,native_tally_calls=0,native_save_calls=0,custom_finish_calls=0;
extern "C" void workshop_807F1300(uint8_t*,recomp_context* context){++native_tally_calls;context->r2=native_tally_mask;}
extern "C" void func_8006988C(uint8_t*,recomp_context* context){++native_save_calls;context->r2=0;}
namespace workshop::custom_session {void finish(uint8_t*){++custom_finish_calls;}} // Inactive custom boundary.
static std::string diagnostic_utf8_path0311(const fs::path& path){const auto text=path.u8string();return {text.begin(),text.end()};}
#include "root-glue-extracted0320.inc"
static unsigned checks=0,contexts=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static std::string bytes(const fs::path& path){std::ifstream input(path,std::ios::binary);return {std::istreambuf_iterator<char>(input),{}};}
static void put(const fs::path& path,const Json& value){fs::create_directories(path.parent_path());std::ofstream output(path,std::ios::binary);output<<value.dump(2)<<'\n';need(bool(output),"Fixture write failed");}
static void word(uint8_t* rdram,uint32_t at,unsigned value){MEM_W(0,int32_t(at))=value;}
int main(int argc,char** argv){try{
 need(argc==2,"Pass an isolated fresh fixture directory");const auto out=fs::absolute(argv[1]);need(!fs::exists(out),"Old reports must not be overwritten");fs::create_directories(out);fs::current_path(out);
 std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();unsigned fixture=0;fs::path metal_file,find_file,journal_file;
 auto setup=[&](int mode,unsigned found=21,unsigned metalFound=3,unsigned normalAlive=1){
  fs::path directory=out/("case-"+std::to_string(fixture++));fs::create_directories(directory);selected_mode=mode;menu_enabled=true;ordinary_scope=true;custom_scope=false;story_lives_ready0320=false;progress_payload=progress_boot_story=0;progress_reading=false;
  std::fill(memory.begin(),memory.end(),0);word(rdram,0x800F8B88,mode==0||mode==5?0:mode==2?3:1);progress::write(rdram,int32_t(0x807F2000),{0xFFFFFF,0,{1,2,3,4},3,63});word(rdram,0x800F8E5C,0xFFFFFF);word(rdram,0x800F8DF4,0);
  find_file=directory/"pc-hybrid-finds.json";put(find_file,{{"version",1u},{"foundMask",found},{"legacyMigrated",true}});f::initialize(find_file);
  metal_file=directory/"pc-metal-yoshis.json";put(metal_file,{{"schema",1u},{"foundMask",metalFound},{"storyAliveMask",normalAlive}});m::initialize(metal_file);m::start_session(unsigned(mode),true);
  const auto completion_file=directory/"pc-hybrid-completion.json";put(completion_file,{{"schema",1u},{"observedMask",0xFFFFFFu},{"legacyMask",0u},{"legacyChecked",true},{"legacyChecksum",nullptr}});hc::initialize(completion_file,{});
  journal_file=directory/sh::filename;put(journal_file,{{"schema",1u},{"mode",5u},{"completedWorldMask",9u},{"hybridOrder",std::array<unsigned,6>{5,1,4,0,2,3}},{"acquiredMask",found},{"metalOwnedMask",metalFound},{"metalAliveMask",metalFound}});sh::initialize(journal_file);if(mode==5)need(sh::prepare(5,found,metalFound),"Mode5 fixture preparation failed");++contexts;
 };
 recomp_context context{};
 // Mode5 clear must reset only its local run, retaining global journals.
 setup(5);const auto sharedMetal=bytes(metal_file),sharedFind=bytes(find_file);const auto durable=sh::view();need(sh::save_lives(0),"Cannot seed local dead metals");f::enter(0,43,true);f::spawned(0x80040000,1);f::egg_state(0x80040000,4);need(f::pending()!=0,"Pending fixture not established");story_lives_ready0320=true;workshop_find_clear();
 need(bytes(metal_file)==sharedMetal&&bytes(find_file)==sharedFind&&f::pending()==0,"Mode5 clear changed global rights or kept carried egg");need(sh::view().worlds==durable.worlds&&sh::view().order==durable.order&&sh::view().acquired==durable.acquired&&sh::view().metal_alive==3&&!story_lives_ready0320,"Mode5 clear lost durable rewards or retained stale hydration fence");
 // Before the native saved-lives getter, RAM extras are not authoritative.
 need(sh::save_lives(3),"Cannot seed pre-restore lives");mn::extras(rdram,0);story_lives_ready0320=false;context.r2=0;workshop_progress_reconcile(rdram,&context);need(sh::view().metal_alive==3&&bytes(metal_file)==sharedMetal,"Boot reconcile cleared local lives before hydration");
 for(bool continuing:{false,true})for(unsigned alive=0;alive<4;++alive){
  setup(5);need(sh::save_lives(alive),"Local life fixture failed");const auto global=bytes(metal_file);MEM_H(0,int32_t(0x800F9254))=continuing?1:0;MEM_B(0,int32_t(0x800F9253))=continuing?8:0;context={};context.r2=0x2Au;workshop_metal_saved_lives(rdram,&context);
  const unsigned expected=continuing?alive:3;need(uint32_t(context.r2)==(0x2Au|(expected<<6))&&sh::view().metal_alive==expected&&story_lives_ready0320,"Saved lives did not distinguish continuation/new run");need(bytes(metal_file)==global,"Mode5 getter changed ordinary Story lives");
 }
 for(unsigned color:{14u,15u})for(unsigned local=0;local<4;++local){
  setup(5);need(sh::save_lives(local),"Death fixture lives failed");const auto global=bytes(metal_file);const auto worlds=sh::view().worlds;mn::extras(rdram,0x15u|(local<<6));word(rdram,0x800F8DF4,0x3456);context={};context.r9=color;need(workshop_metal_death(rdram,&context)==1,"Mode5 metal death was not handled");const unsigned after=(0x15u|(local<<6))&~(1u<<(color-8));need(mn::extras(rdram)==after&&uint32_t(context.r10)==after&&uint32_t(context.r12)==0x3456&&sh::view().metal_alive==((after>>6)&3)&&story_lives_ready0320,"Mode5 death changed alias/lower lives or failed local persistence");need(bytes(metal_file)==global&&sh::view().worlds==worlds,"Mode5 death changed ordinary/global/durable state");
  // The actual rescue hook runs BEFORE the native OR store. RAM is still dead.
  const unsigned beforeRescue=mn::extras(rdram);context.r2=color;workshop_metal_rescued(rdram,&context);need(sh::view().metal_alive==(((beforeRescue>>6)|(1u<<(color-14)))&3)&&mn::extras(rdram)==beforeRescue,"Rescue did not persist the pre-native OR bit or wrote native RAM");need(bytes(metal_file)==global,"Mode5 rescue revived ordinary Story life");
 }
 // Eligibility is not ownership. Corrupt/missing ownership fails closed.
 setup(5,0);for(unsigned page=0;page<6;++page)need(sh::successful_world(page,true),"Cannot seed six distinct successful world receipts");need(sh::view().eligible==63&&workshop_find_unlocked(rdram)==0xC0,"Unlocked projection counted eligibility as acquired hybrids");
 {std::ofstream bad(find_file);bad<<"corrupt shared fixture";}f::initialize(find_file);need(workshop_find_unlocked(rdram)==0&&bytes(find_file)=="corrupt shared fixture","Corrupt shared Finds exposed all six fallback colors");
 const auto missing=out/"missing-finds.json";f::initialize(missing);need(workshop_find_unlocked(rdram)==0&&!fs::exists(missing),"Unmigrated shared Finds exposed colors or created ownership");
 // A journal's eligible order alone adds no +100 Yoshi. Only the carried
 // native-egg boundary supplied to real Finds may grow its ownership/count.
 setup(5,0);for(unsigned page=0;page<6;++page)need(sh::successful_world(page,true),"Eligibility fixture world failed");native_tally_mask=0x3F;context={};workshop_end_score_lives(rdram,&context);need(context.r5==6&&f::view().found==0,"Uncollected eligibility earned a score/ownership bonus");
 const auto globalBefore=bytes(metal_file);f::enter(0,43,true);need(f::reserve(1,0x46AA,100,200,false)==0,"Pending hybrid slot fixture failed");f::spawned(0x80040000,0);f::egg_state(0x80040000,4);context={};workshop_end_score_lives(rdram,&context);need(context.r5==7&&(context.r5-6)*100==100&&f::view().found==1&&sh::view().acquired==1&&f::pending()==0,"Actual pending hybrid did not add exactly one tally Yoshi");need(bytes(metal_file)==globalBefore,"Hybrid tally changed normal metal state");
 native_tally_mask=0x13F;f::enter(0,43,true);f::spawned(0x80040000,0);f::egg_state(0x80040000,4);context={};workshop_end_score_lives(rdram,&context);need(context.r5==7&&f::view().found==1,"Already counted hybrid was counted again");
 setup(5,63,0,0);const auto globalNoMetals=bytes(metal_file);native_tally_mask=0x3F;f::enter(0,43,true);need(f::reserve(63,0x46AA,100,200,false)==6,"Pending Gold slot fixture failed");f::spawned(0x80040000,6);f::egg_state(0x80040000,4);context={};workshop_end_score_lives(rdram,&context);need(context.r5==7&&m::view().found==1&&m::view().story_alive==0&&sh::view().metal_alive==1,"Mode5 pending Gold tally corrupted ordinary lives or omitted local life");need(bytes(metal_file)!=globalNoMetals,"Pending Gold did not save real shared ownership");
 // Early mode5 boot excludes all legacy reconciliation/EEPROM backup paths.
 setup(5);const auto before=memory;progress_payload=0x80100000;progress_reading=true;context={};context.r2=0;const auto nativeCalls=native_save_calls;workshop_progress_boot(rdram,&context);need(memory==before&&progress_payload==0&&!progress_reading&&native_save_calls==nativeCalls&&!fs::exists(out/"save-backups"),"Mode5 boot used the canonical save repair path");
 // Existing0..3 retain their owned projection and local/native life paths.
 for(int mode:{0,1,2,3}){
  setup(mode,63,3,1);need(workshop_find_unlocked(rdram)==0xFF,"Old mode owned selection changed");const auto findBefore=bytes(find_file);context={};context.r2=0x15;MEM_H(0,int32_t(0x800F9254))=1;MEM_B(0,int32_t(0x800F9253))=8;workshop_metal_saved_lives(rdram,&context);need(uint32_t(context.r2)==(0x15u|(mode==0?0x40u:0xC0u)),"Old saved lives behavior changed");
  mn::extras(rdram,0xFF);context={};context.r9=14;need(workshop_metal_death(rdram,&context)==1&&mn::extras(rdram)==0xBF,"Old native death behavior changed");need(m::view().story_alive==(mode==0?2u:1u),"Non-Story death wrote ordinary Story lives");context.r2=14;workshop_metal_rescued(rdram,&context);need(m::view().story_alive==(mode==0?3u:1u),"Old rescue behavior changed");need(bytes(find_file)==findBefore,"Metal wrapper changed old hybrid ownership");
  const progress::State empty{0,255,{255,255,255,255},0,63};progress::write(rdram,int32_t(0x807F2000),empty);word(rdram,0x800F8E5C,0);progress_payload=0;progress_reading=false;workshop_progress_reconcile(rdram,&context);workshop_progress_boot(rdram,&context);need(native_save_calls==nativeCalls,"Old no-repair branch called stub EEPROM save");
  workshop_find_clear();need(f::view().found==0&&m::view().found==0&&m::view().story_alive==0,"Old explicit clear semantics changed");
 }
 Json execution{{"passed",true},{"checks",checks},{"contexts",contexts},{"exactLoaderFunctionBodiesExecuted",true},{"realFindsMetalsStoryJournalLinked",true},{"nativeRamSynthetic",true},{"nativeTallyBoundaryStub",true},{"nativeEepromSaveBoundaryStub",true},{"inactiveCustomFinishBoundaryStub",true},{"gameOrGpuLaunched",false},{"actualEepromAccessed",false},{"personalFilesAccessed",false},{"naturalPickupOrWorldClearProven",false}};std::ofstream report(out/"execution.json");report<<execution.dump(2)<<'\n';std::cout<<execution.dump()<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
