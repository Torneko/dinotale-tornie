#include "../metal_custom0311.hpp"
#include "../custom_session.hpp"
#include "../workshop_menu_game.hpp"
#include "../vanilla_sources.hpp"
#include "json/json.hpp"
#include <array>
#include <bit>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
namespace fs=std::filesystem;
namespace custom=workshop::custom_session;
namespace projection=workshop::metal_custom0311;
static unsigned checks=0,scope_cases=0,projection_cases=0,death_cases=0,configure_cases=0,finish_cases=0,navigation_cases=0,timing_cases=0,context_cases=0,mask_cases=0;
static workshop::book::Snapshot fixture_book;
namespace workshop::book {Snapshot snapshot(){return fixture_book;}}
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static void word(uint8_t* rdram,uint32_t address,uint32_t value){MEM_W(0,int32_t(address))=int32_t(value);}
static unsigned word(uint8_t* rdram,uint32_t address){return uint32_t(MEM_W(0,int32_t(address)));}
static std::string bytes(const fs::path& path){std::ifstream file(path,std::ios::binary);return {std::istreambuf_iterator<char>(file),{}};}
static nlohmann::json result(){std::ifstream in("custom-run-result.json");nlohmann::json value;in>>value;return value;}
static std::string token(unsigned id){const char hex[]="0123456789abcdef";std::string value(32,'a');value[30]=hex[(id>>4)&15];value[31]=hex[id&15];return value;}
template<class Action> static void reject(Action action,const char* why){bool rejected=false;try{action();}catch(const std::exception&){rejected=true;}need(rejected,why);}
static void fresh_case(const fs::path& output,const std::string& name){const auto path=output/fs::u8path(name);need(fs::create_directory(path),"Fixture directory unexpectedly exists");fs::current_path(path);}
static void check_navigation(unsigned scene,bool paused,unsigned stage,bool automatic,int preferred,bool release,bool ready,uint16_t expected,float expected_x,float expected_y,uint8_t* rdram,unsigned frame=120){
 custom::configure(token(100+navigation_cases),automatic,preferred,43);
 if(release)custom::release_preferred_color();
 if(ready){word(rdram,0x800F89D0,30);word(rdram,0x800F8AC8,1);word(rdram,0x807F0008,14);custom::finish(rdram);}
 fixture_book={};fixture_book.scene=scene;fixture_book.stage=stage;fixture_book.paused=paused;
 uint16_t buttons=0x0400;float x=.25f,y=-.5f;custom::navigation(frame,&buttons,&x,&y);
 need(buttons==expected,"Navigation sent unwanted automatic buttons");
 need(x==expected_x&&y==expected_y,"Navigation changed unexpected user stick input");
 if(release){need(custom::preferred_color()==-1,"Unavailable preferred color was retained");custom::release_preferred_color();need(custom::preferred_color()==-1,"Preferred release is not idempotent");}
 ++navigation_cases;
}
int main(int argc,char**argv){
 try{
  need(argc==2,"Expected disposable output directory");
  const auto output=fs::absolute(argv[1]);fs::create_directories(output);fs::current_path(output);
  const auto native_save=output/"saves-untouched.fixture";{std::ofstream(native_save,std::ios::binary)<<"synthetic-user-like-save-sentinel";}
  const auto untouched=bytes(native_save);
  // All combinations: neither custom input, either input, both, with independent tally/deferred exclusion.
  for(unsigned bits=0;bits<16;++bits){
   const bool payload=bits&1,session=bits&2,tally=bits&4,deferred=bits&8;
   const bool expected=(bits==1||bits==2||bits==3);
   need(projection::scope(payload,session,tally,deferred)==expected,"Custom scope bypassed tally/deferred or admitted ordinary solo");
   ++scope_cases;
  }
  // Identity projection is independent of Story losses, current save-write status and invalid high legacy bits.
  for(bool initialized:{false,true})for(bool readable:{false,true})for(bool saved:{false,true})for(unsigned found:{0u,1u,2u,3u,4u,UINT32_MAX})for(unsigned alive=0;alive<4;++alive){
   workshop::metal_yoshis0300::View view;view.initialized=initialized;view.readable=readable;view.saved=saved;view.found=found;view.story_alive=alive;
   unsigned expected=0;if(initialized&&readable){if(found==1)expected=0x40;else if(found==2)expected=0x80;else if(found==3)expected=0xC0;}
   need(projection::owned_extra(view)==expected,"Custom owned projection used Story lives, uninitialized/unreadable or invalid rights");
   const auto snapshot=view;
   for(unsigned legacy:{0u,1u,0x15u,0x3Fu,0xFFu,0x5A93FFFFu}){
    const auto selected=projection::saved_lives(legacy,view);
    need((selected&0x3F)==(legacy&0x3F),"Custom saved-life projection damaged legacy six bits");
    need((selected&0xC0)==expected&&selected<=0xFF,"Custom saved-life projection invented metal bits or retained high payload bits");
   }
   need(view.found==snapshot.found&&view.story_alive==snapshot.story_alive&&view.saved==snapshot.saved,"Read-only projection changed ownership view");
   ++projection_cases;
  }
  std::vector<uint8_t> memory(0x800000,0xA5);auto* rdram=memory.data();
  // Full memory comparison proves local death touches only the chosen extra-life word, not native White/Black or old codec.
  for(unsigned color:{14u,15u})for(unsigned extras:{0u,1u,0x20u,0x3Fu,0x40u,0x80u,0xC0u,0xFFu})for(unsigned flags:{0u,0xC0u,0x5A93FFFFu})for(unsigned native_mode:{0u,1u}){
   word(rdram,0x807F1000,extras);word(rdram,0x800F8DF4,flags);word(rdram,0x807F200A,0x13579BDF);word(rdram,0x800F8B88,native_mode);
   auto expected=memory;auto* expected_ram=expected.data();word(expected_ram,0x807F1000,extras&~(color==14?0x40u:0x80u));
   need(projection::death(rdram,color),"Custom metal death declined Gold/Silver");
   need(memory==expected,"Custom death mutated beyond its own transient extra-life bit");
   need(word(rdram,0x800F8DF4)==flags&&word(rdram,0x807F200A)==0x13579BDF,"Custom death changed nativeBW/legacy codec sentinel");
   ++death_cases;
  }
  for(unsigned color:{0u,5u,6u,7u,8u,13u,16u,UINT32_MAX}){
   const auto before=memory;need(!projection::death(rdram,color),"Custom death accepted a non-metal or invalid color");need(memory==before,"Rejected color changed RAM");
  }
  // Real session implementation, never a copy of its logic.
  fresh_case(output,"initial");word(rdram,0x800F89D0,30);word(rdram,0x800F8AC8,1);
  need(!custom::active()&&!custom::ready()&&custom::saved(),"Initial session state differs");
  custom::finish(rdram);need(!fs::exists("custom-run-result.json")&&!custom::close(true),"Inactive session wrote or advanced results");
  for(unsigned index=0;index<workshop::vanilla_sources.size();++index)for(int preferred=-1;preferred<16;++preferred){
   const auto identity=token(index);custom::configure(identity,index%2,preferred,workshop::vanilla_sources[index]);
   need(custom::active()&&!custom::ready()&&custom::saved(),"Valid configure did not reset unfinished state");
   need(custom::preferred_color()==preferred&&custom::source_stage()==int(index),"Preferred color or source stage changed");
   need(!custom::close(true),"Unfinished session can advance");++configure_cases;
  }
  const auto selected_before=custom::preferred_color(),source_before=custom::source_stage();
  for(int invalid:{-2,16,17,std::numeric_limits<int>::max()}){
   reject([&]{custom::configure(token(1),true,invalid,43);},"Invalid preferred color accepted");
   need(custom::preferred_color()==selected_before&&custom::source_stage()==source_before&&!custom::ready(),"Invalid configure damaged previous session");
  }
  for(const std::string& invalid:std::vector<std::string>{"",std::string(31,'a'),std::string(33,'a'),std::string(32,'g'),std::string(32,'/')}){
   reject([&]{custom::configure(invalid,true,14,43);},"Malformed result token accepted");
  }
  for(unsigned invalid:{177u,999u,UINT32_MAX})reject([&]{custom::configure(token(1),true,14,invalid);},"Invalid vanilla source accepted");
  // Gates, each selected color0..15, unchanged result schema/token/advance and idempotent successful finish.
  for(unsigned color=0;color<16;++color){
   fresh_case(output,"result-"+std::to_string(color));const auto identity=token(color);custom::configure(identity,true,int(color),43);
   for(const auto invalid:std::array<std::array<unsigned,2>,5>{{{{0,0}},{{29,1}},{{30,0}},{{31,1}},{{30,2}}}}){
    word(rdram,0x800F89D0,invalid[0]);word(rdram,0x800F8AC8,invalid[1]);word(rdram,0x807F0008,color);
    custom::finish(rdram);need(!custom::ready()&&!fs::exists("custom-run-result.json"),"Non-successful attempt wrote custom result");
   }
   word(rdram,0x800F89D0,30);word(rdram,0x800F8AC8,1);word(rdram,0x807F0008,color);custom::finish(rdram);
   need(custom::ready()&&custom::saved(),"Successful real custom finish was not persisted");
   auto value=result();need(value.size()==6&&value["schema"]==1&&value["token"]==identity&&value["completed"]==true&&value["fruits"]==30&&value["advance"]==true,"Custom finish result schema/token/completion changed");
   need(value["selectedColor"]==color,"Gold/Silver or legacy selected color silently fell back to Green");
   const auto first=bytes("custom-run-result.json");word(rdram,0x807F0008,0);custom::finish(rdram);
   need(bytes("custom-run-result.json")==first,"Repeated finish rewrote the result or duplicated completion");
   need(custom::close(false),"Close retaining progress failed");value=result();need(value["advance"]==false&&value["token"]==identity&&value["selectedColor"]==color,"Stop/keep-progress changed identity or selected color");
   need(custom::close(true),"Close advancing failed");value=result();need(value["advance"]==true&&value["selectedColor"]==color,"Advance lost selected color");
   need(!fs::exists("custom-run-result.json.tmp"),"Result temporary left after success");
   ++finish_cases;
  }
  fresh_case(output,"io-blocked");custom::configure(token(99),true,15,43);fs::create_directory("custom-run-result.json.tmp");
  word(rdram,0x800F89D0,30);word(rdram,0x800F8AC8,1);word(rdram,0x807F0008,15);custom::finish(rdram);
  need(custom::ready()&&!custom::saved()&&!custom::close(true)&&!fs::exists("custom-run-result.json"),"Failed result I/O forged a saved/advance result");
  // Stage0 scene5 is Lucky Fruit; Stage6 scene5 is Yoshi selection. Release disables automatic Stage0 confirmations.
  for(int color:{14,15}){
   fresh_case(output,"navigation-"+std::to_string(color));
   check_navigation(4,false,0,true,color,false,false,0x1400,0,0,rdram);
   check_navigation(5,false,0,true,color,false,false,0x8400,0,0,rdram);
   check_navigation(4,false,0,true,color,true,false,0x0400,.25f,-.5f,rdram);
   check_navigation(5,false,0,true,color,true,false,0x0400,.25f,-.5f,rdram);
   check_navigation(41,false,0,true,color,true,false,0x8400,0,0,rdram);
   check_navigation(5,true,0,true,color,false,false,0x0400,.25f,-.5f,rdram);
   check_navigation(5,false,1,true,color,false,false,0x0400,.25f,-.5f,rdram);
   check_navigation(5,false,0,true,color,false,true,0x0400,.25f,-.5f,rdram);
  }
  // Preserve Lucky Fruit Stage0 autoA; actual Yoshi selection Stage6 never receives CustomSession navigation autoA.
  fresh_case(output,"preferred-controller");
  for(int color=0;color<16;++color){
   for(unsigned frame:{0u,120u,1320u,1680u,1800u,2400u}){
    check_navigation(5,false,0,true,color,false,false,0x8400,0,0,rdram,frame);
    check_navigation(5,false,6,true,color,false,false,0x0400,.25f,-.5f,rdram,frame);
    check_navigation(41,false,0,true,color,false,false,0x8400,0,0,rdram,frame);
   }
   check_navigation(4,false,0,true,color,true,false,0x0400,.25f,-.5f,rdram);
   check_navigation(5,false,0,true,color,true,false,0x0400,.25f,-.5f,rdram);
  }
  check_navigation(5,false,0,true,-1,false,false,0x8400,0,0,rdram);
  check_navigation(4,false,0,true,14,false,false,0x1400,0,0,rdram);
  // Independent native-context and frame-policy boundaries, shared with the Loader.
  const std::array<std::pair<unsigned,bool>,10> frame_policy{{{0,false},{1,false},{1199,false},{1200,false},{1343,false},{1799,false},{1800,true},{1801,true},{2400,true},{UINT32_MAX,true}}};
  for(const auto& [frame,diagnostic_ready]:frame_policy)for(int diagnostic:{-1,0,14,15})for(int preferred=-1;preferred<16;++preferred){
   const bool expected=diagnostic==-1?(preferred!=-1):diagnostic_ready;
   need(projection::selection_ready(frame,diagnostic,preferred)==expected,"Custom preference delayed or diagnostic legacy timing changed");
   ++timing_cases;
  }
  for(unsigned stage=0;stage<9;++stage)for(unsigned scene=0;scene<64;++scene){
   const bool expected=stage==6&&scene==5;
   need(projection::selection_context(stage,scene)==expected,"Lucky Fruit/Trial/title admitted as real Yoshi-selection context");
   ++context_cases;
  }
  need(!projection::selection_context(UINT32_MAX,5)&&!projection::selection_context(6,UINT32_MAX),"Invalid native context admitted");
  for(unsigned bits=0;bits<=0xFFFF;++bits){
   const auto original=uint16_t(bits),filtered=projection::pending_buttons(original);
   need((filtered&0x9000)==0,"Pending native selection retained A/Start and can confirm Green early");
   need((filtered&0x4000)==(original&0x4000),"Pending selection swallowed B/back");
   need((filtered&0x2FFF)==(original&0x2FFF),"Pending selection changed unrelated controller buttons");
   need(projection::pending_buttons(filtered)==filtered,"Pending button filter is not idempotent");
   ++mask_cases;
  }
  need(bytes(native_save)==untouched,"Test changed unrelated sentinel file");
  nlohmann::json report{{"passed",true},{"checks",checks},{"scopeCases",scope_cases},{"ownedProjectionCases",projection_cases},{"ramDeathCases",death_cases},{"configureCases",configure_cases},{"selectedColorFinishCases",finish_cases},{"navigationCases",navigation_cases},{"selectionTimingCases",timing_cases},{"selectionContextCases",context_cases},{"pendingButtonMaskCases",mask_cases},{"realCustomSessionCppLinked",true},{"realMetalCustomHeaderUsed",true},{"onlyBookSnapshotIsStub",true},{"nativeRamSynthetic",true},{"canonicalMetalServiceNotLinkedOrWritten",true},{"gameLaunched",false},{"gpuUsed",false},{"personalProfileUsed",false},{"EEPROMCodecUnchanged",true}};
  std::ofstream summary(output/"execution.json");summary<<report.dump(2)<<'\n';summary.flush();if(!summary)throw std::runtime_error("Cannot write execution report");
  std::cout<<report.dump()<<'\n';return 0;
 }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';return 1;}
}
