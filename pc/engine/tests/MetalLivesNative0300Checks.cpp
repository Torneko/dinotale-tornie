#include "../metal_native0300.hpp"
#include "../hybrid_completion0290.hpp"
#include "json/json.hpp"
#include <array>
#include <cstdlib>
#include <bit>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace fs=std::filesystem;
namespace metal=workshop::metal_yoshis0300;
namespace life=workshop::metal_native0300;
static unsigned checks=0,native_calls=0,death_cases=0,saved_cases=0,rescue_cases=0,trial_cases=0;
static bool ordinary=true;static unsigned legacy_unlocked=0x3F,random_value=0,reset_calls=0,transition_calls=0,scene_after_death=0,cleanup_calls=0,audio_calls=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
static bool metal_solo_scope(){return ordinary;}
static int32_t sections[300]{};int32_t* section_addresses=sections;
static void word(uint8_t* rdram,uint32_t address,uint32_t value){MEM_W(0,int32_t(address))=int32_t(value);}
static unsigned word(uint8_t* rdram,uint32_t address){return uint32_t(MEM_W(0,int32_t(address)));}
static void byte(uint8_t* rdram,uint32_t address,unsigned value){MEM_B(0,int32_t(address))=uint8_t(value);}
static void half(uint8_t* rdram,uint32_t address,unsigned value){MEM_H(0,int32_t(address))=uint16_t(value);}
extern "C" void workshop_807F1100(uint8_t*,recomp_context*);
extern "C" void workshop_ext_saved_lives(uint8_t*,recomp_context*);
extern "C" void workshop_807F1C00(uint8_t*,recomp_context*);
extern "C" void workshop_ext_rescue_picker(uint8_t*,recomp_context*);
extern "C" void workshop_ext_rescue_capacity(uint8_t*,recomp_context*);
extern "C" void workshop_ys_ext_trial_color_unlocked(uint8_t*,recomp_context*);
extern "C" void workshop_trace_entry(uint8_t*,recomp_context*,const char*){++native_calls;}
extern "C" unsigned workshop_find_unlocked(uint8_t*){metal::set_scope(ordinary);return legacy_unlocked|metal::unlocked_extra();}
extern "C" void workshop_find_reset(){++reset_calls;}
extern "C" void workshop_ys_ext_valid(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void workshop_ys_ext_unlocked_mask(uint8_t*,recomp_context* c){c->r2=legacy_unlocked;}
extern "C" void func_8006E0FC(uint8_t*,recomp_context* c){++transition_calls;need(c->r4==0&&c->r5==0x16,"Death transition lost native a0/a1");scene_after_death=unsigned(c->r6);}
extern "C" void func_8002AEE4(uint8_t*,recomp_context*){++cleanup_calls;}
extern "C" void func_80067878(uint8_t*,recomp_context* c){++audio_calls;need(c->r4==0x15&&c->r5==0,"Death sound path changed");}
extern "C" void fixture_random(uint8_t*,recomp_context* c){c->r2=random_value;}
extern "C" recomp_func_t* get_function(int32_t address){if(uint32_t(address)==0x800653D8)return fixture_random;std::abort();}
namespace workshop::hybrid_completion0290 {View view(){View v;v.mask=all_levels;v.count=24;return v;}}
#include METAL_NATIVE_GLUE
struct Fixture {
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* ram=memory.data();recomp_context context{};
 Fixture(){context.r29=int32_t(0x80770000);context.r31=0x12345678;ordinary=true;legacy_unlocked=0x3F;reset_calls=transition_calls=cleanup_calls=audio_calls=scene_after_death=0;for(unsigned i=0;i<8;++i)word(ram,0x8009E7F8+4*i,~(1u<<i));life::story_ready=false;}
 void mode(unsigned value,bool continuing=false){word(ram,0x800F8B88,value);half(ram,0x800F9254,continuing?1:0);byte(ram,0x800F9253,continuing?8:0);}
};
static std::string bytes(const fs::path& file){std::ifstream in(file,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};}
static void initialize(const fs::path& file,unsigned found,unsigned alive,unsigned session=0){
 fs::create_directories(file.parent_path());std::ofstream out(file,std::ios::binary);out<<nlohmann::json{{"schema",1u},{"foundMask",found},{"storyAliveMask",alive}}.dump();out.close();need(bool(out),"Fixture journal write failed");metal::initialize(file);metal::start_session(session,ordinary);need(metal::view().readable,"Fixture journal did not initialize");
}
int main(int argc,char** argv){
 try{need(argc==2,"Expected isolated output path");const auto out=fs::absolute(argv[1]);fs::create_directories(out);const auto journal=out/"pc-metal-yoshis.json";
 // All sixteen native death branches with independent native/extras masks.
 initialize(journal,3,3,1);
 for(unsigned color=0;color<16;++color)for(unsigned native:{0u,1u,0x40u,0x80u,0xFFu})for(unsigned extra:{0u,1u,0x20u,0x40u,0x80u,0xC0u,0xFFu})for(unsigned counter:{0u,9u,31u}){
  Fixture f;f.mode(1);metal::start_session(1,true);word(f.ram,0x807F0008,color);word(f.ram,0x800F8DB0,color<8?color:color<14?0:color-8);word(f.ram,0x800F8DF4,0x5A930000|native);word(f.ram,0x807F1000,extra);word(f.ram,0x807F1004,counter);
  const auto sp=f.context.r29,ra=f.context.r31;workshop_807F1100(f.ram,&f.context);
  const unsigned expected_native=color<8?native&~(1u<<color):native;const unsigned expected_extra=color>=8?extra&~(1u<<(color-8)):extra;
  need(word(f.ram,0x800F8DF4)==(0x5A930000|expected_native),"Death changed native White/Black or reserved bits");need(word(f.ram,0x807F1000)==expected_extra,"Death changed wrong extra life bit");
  need(word(f.ram,0x807F1004)==counter+1&&word(f.ram,0x807F1008)==color,"Death counter/cargo identity changed");need(scene_after_death==((expected_native||expected_extra)?8u:0x1Du),"All-lost transition ignored native or metallic remaining life");
  need(transition_calls==1&&cleanup_calls==1&&audio_calls==1&&reset_calls==1,"Native death callbacks skipped or duplicated");need(f.context.r29==sp&&f.context.r31==ra,"Death stack/return address damaged");++death_cases;
 }
 // Real saved-life getter with separate journal fresh/continue semantics.
 for(unsigned mode:{0u,1u})for(bool cont:{false,true})for(unsigned alive=0;alive<4;++alive)for(unsigned unlocked:{0u,1u,0x15u,0x3Fu})for(unsigned legacy_alive:{0u,0x12u,0x3Fu}){
  Fixture f;f.mode(mode,cont);legacy_unlocked=unlocked;byte(f.ram,0x807F200A,legacy_alive);initialize(journal,3,alive,mode);const auto before=f.memory;const auto sp=f.context.r29,ra=f.context.r31;workshop_ext_saved_lives(f.ram,&f.context);
  const unsigned base=mode==0&&cont?(unlocked&legacy_alive):unlocked;const unsigned bits=mode==0&&cont?alive:3;
  need(unsigned(f.context.r2)==(base|(bits<<6)),"Saved-life projection lost old mask or metallic fresh/continue semantics");need(metal::view().story_alive==(mode==0&&!cont?3u:alive),"Native saved getter changed wrong Story journal life");
  need(std::equal(before.begin()+0xF8E58,before.begin()+0xF8E60,f.memory.begin()+0xF8E58)&&std::equal(before.begin()+0xF9250,before.begin()+0xF9258,f.memory.begin()+0xF9250),"Saved getter modified native EEPROM fields");need(f.context.r29==sp&&f.context.r31==ra,"Saved getter stack/return damaged");++saved_cases;
 }
 // Genuine Story deaths persist only their own metal bit, retaining nativeBW.
 for(unsigned color:{14u,15u}){
  Fixture f;f.mode(0,true);initialize(journal,3,3);word(f.ram,0x807F0008,color);word(f.ram,0x800F8DB0,color-8);word(f.ram,0x800F8DF4,0x5A93C0C0);word(f.ram,0x807F1000,0xC0);workshop_807F1100(f.ram,&f.context);
  need(metal::view().story_alive==(3u&~(1u<<(color-14))),"Story death failed durable separate life update");need(word(f.ram,0x800F8DF4)==0x5A93C0C0,"Metal death cleared nativeBW alias");metal::initialize(journal);metal::start_session(0,true);need(metal::story_extra(true)==(0xC0u&~(1u<<(color-8))),"Story continue revived dead metal");
 }
 // Real picker, register-preserving wrapper and rescue consumer for all colors.
 for(unsigned color=0;color<16;++color){
  Fixture f;f.mode(0,true);initialize(journal,3,0);unsigned native=0xFF,extra=0xFF;if(color<8)native&=~(1u<<color);else extra&=~(1u<<(color-8));const unsigned prior=0x5A900000|0x3000|(3u<<14)|native;
  word(f.ram,0x800F8DF4,prior);word(f.ram,0x807F1000,extra);word(f.ram,0x807F1E00,7);word(f.ram,0x807F1E04,99);const auto sp=f.context.r29,ra=f.context.r31;workshop_807F1C00(f.ram,&f.context);
  need(word(f.ram,0x800F8E20)==color+1,"Rescue picker failed exact missing color");need(word(f.ram,0x807F1000)==0xFF,"Rescue did not restore correct extra life");need(word(f.ram,0x800F8DF4)==((prior&~(7u<<14))|(2u<<14)|(color<8?1u<<color:0u)),"Rescue changed nativeBW/reserved bits or WhiteShyGuy count");
  need(word(f.ram,0x807F1E00)==8&&word(f.ram,0x807F1E04)==color,"Rescue statistics lost display color");need(metal::view().story_alive==(color>=14?1u<<(color-14):0u),"Rescue journal restored wrong or legacy color");need(f.context.r29==sp&&f.context.r31==ra,"Rescue damaged stack/return");++rescue_cases;
 }
 {Fixture f;f.mode(0,true);initialize(journal,3,3);word(f.ram,0x800F8DF4,0x5A93FFFF);word(f.ram,0x807F1000,0xFF);word(f.ram,0x807F1E00,7);workshop_807F1C00(f.ram,&f.context);need(word(f.ram,0x800F8E20)==0&&word(f.ram,0x807F1E00)==7&&word(f.ram,0x800F8DF4)==0x5A93FFFF,"No missing rescue consumed a WhiteShyGuy or accepted sentinel16");}
 // Capacity includes both metals without inflating historical native inputs.
 for(unsigned extra:{0u,0x3Fu,0x7Fu,0xBFu,0xFFu})for(unsigned native_count:{5u,6u}){
  Fixture f;f.mode(0);initialize(journal,3,3);word(f.ram,0x807F1000,extra);f.context.r4=native_count;f.context.r5=6;workshop_ext_rescue_capacity(f.ram,&f.context);need(unsigned(f.context.r2)==unsigned(native_count+std::popcount(extra)<14),"Rescue capacity failed eight extra colors");
 }
 // All simultaneously missing ordinary/extras candidates exercise the enlarged stack array.
 for(unsigned index=0;index<14;++index){
  Fixture f;f.mode(0,true);initialize(journal,3,0);word(f.ram,0x800F8DF4,0x3000);word(f.ram,0x807F1000,0);random_value=index;const auto sp=f.context.r29,ra=f.context.r31;word(f.ram,uint32_t(sp)-0x74,0xA55AC33C);workshop_ext_rescue_picker(f.ram,&f.context);
  need(unsigned(f.context.r2)==(index<6?index:index+2),"Full rescue candidate array skipped/overwrote a color");need(f.context.r29==sp&&f.context.r31==ra&&word(f.ram,uint32_t(sp)-0x74)==0xA55AC33C,"Full rescue array damaged stack guard/return");
 }
 // Trial early override must not touch native tables/stack for14/15.
 for(unsigned session=0;session<=4;++session)for(bool scope:{false,true})for(unsigned found=0;found<4;++found)for(unsigned color:{14u,15u}){
  Fixture f;ordinary=scope;f.mode(session?1:0);initialize(journal,found,found,session);const auto sp=f.context.r29,ra=f.context.r31;f.context.r5=color;workshop_ys_ext_trial_color_unlocked(f.ram,&f.context);need(unsigned(f.context.r2)==unsigned(scope&&session<=3&&((found>>(color-14))&1)),"Metal Trial selection ignored owned/scope/session");need(f.context.r29==sp&&f.context.r31==ra,"Metal early override touched native stack");++trial_cases;
 }
 // Production scope suppresses metallic remaining bits in deferred modes.
 {Fixture f;ordinary=false;f.mode(1);word(f.ram,0x807F0008,0);word(f.ram,0x800F8DB0,0);word(f.ram,0x800F8DF4,1);word(f.ram,0x807F1000,0xC0);workshop_807F1100(f.ram,&f.context);need(scene_after_death==0x1D,"Deferred scope counted metallic lives");}
 // Journal lock failure retains RAM death; a later genuineStory fence retries.
 {Fixture f;f.mode(0,true);initialize(journal,3,3);word(f.ram,0x807F0008,14);word(f.ram,0x800F8DB0,6);word(f.ram,0x800F8DF4,0xC0);word(f.ram,0x807F1000,0xC0);const auto saved=bytes(journal);const auto lock=fs::path(journal.string()+".lock");fs::remove(lock);fs::create_directory(lock);workshop_807F1100(f.ram,&f.context);need(!metal::view().saved&&bytes(journal)==saved&&word(f.ram,0x807F1000)==0x80,"IO failure forged save or restored dead RAM life");fs::remove(lock);life::save_story(f.ram);need(metal::view().saved&&metal::view().story_alive==2,"Story save fence did not retry failed journal");}
 nlohmann::json report{{"passed",true},{"checks",checks},{"actualNativeCalls",native_calls},{"deathMatrixCases",death_cases},{"savedLifeCases",saved_cases},{"rescueCases",rescue_cases},{"trialEarlyCases",trial_cases},{"realMetalJournalLinked",true},{"hybrid24ReadyIsHostStub",true},{"nativeInputsSynthetic",true},{"legacyUnlockedIsHostBoundary",true},{"sceneCleanupAudioAreExplicitStubs",true},{"nativePickerAndConsumerExecuted",true},{"nativeEEPROMUnchanged",true},{"gameLaunched",false},{"gpuUsed",false},{"personalProfileUsed",false}};
 std::ofstream summary(out/"native-verification.json");summary<<report.dump(2)<<'\n';need(bool(summary),"Could not save CPU report");std::cout<<report.dump(2)<<'\n';return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}