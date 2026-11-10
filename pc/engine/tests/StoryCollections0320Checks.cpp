#include "../collections.hpp"
#include "../game_modes0320.hpp"
#include "../story_progress.hpp"
#include "json/json.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace fs=std::filesystem;namespace c=workshop::collections;namespace m=workshop::game_modes0320;
unsigned checks=0;
void need(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
void put(const fs::path& file,unsigned purchases){
 nlohmann::json levels=nlohmann::json::object();
 for(unsigned level=0;level<12;level++){auto places=nlohmann::json::array();for(unsigned i=0;i<20;i++)places.push_back({43u+level,i,i*10u,400u});levels[std::to_string(level/4+1)+"-"+std::to_string(level%4+1)]={{"places",places},{"claimedMask",0xFFFFF}};}
 std::ofstream out(file);out<<nlohmann::json{{"version",1},{"levels",levels},{"purchases",purchases},{"legacyEggs",0},{"remainderAchievement",false}}.dump(2);out.close();need(bool(out),"Fixture write failed");
}
std::string bytes(const fs::path& file){std::ifstream in(file,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};}
int main(int argc,char** argv){try{
 need(argc==2,"Disposable folder required");const auto dir=fs::path(argv[1]);need(fs::create_directories(dir),"Folder exists");
 std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
 workshop::progress::State empty{0,255,{255,255,255,255},0,63};workshop::progress::write(rdram,int32_t(0x807F2000),empty);
 const auto file=dir/"collection.json";put(file,3);const auto before=bytes(file);c::initialize(file);
 for(unsigned trial:{0u,1u,0xFFFFFEu,0xFFFFFFu})for(bool valid:{false,true}){
  c::start_session(5,trial,0,valid);
  need(c::egg_allowed(rdram,6)==(valid&&trial==0xFFFFFF),"White HybridStory search ignored canonical gate");
  need(c::egg_allowed(rdram,7)==(valid&&trial==0xFFFFFF),"Black HybridStory search ignored canonical gate");
 }
 // A forged child payload never substitutes for unreadable canonical rights.
 auto forged=empty;forged.trial=0xFFFFFF;forged.eggs=3;workshop::progress::write(rdram,int32_t(0x807F2000),forged);
 c::start_session(5,0,0,false);need(!c::egg_allowed(rdram,6)&&!c::egg_allowed(rdram,7),"Child payload became authority");
 for(unsigned mode:{0u,1u,2u,3u,4u}){c::start_session(mode);need(c::egg_allowed(rdram,6)&&c::egg_allowed(rdram,7),"Original native earned egg behavior changed");}
 workshop::progress::write(rdram,int32_t(0x807F2000),empty);
 for(unsigned mode:{0u,1u,2u,3u,4u}){c::start_session(mode,0xFFFFFF,3,true);need(!c::egg_allowed(rdram,6)&&!c::egg_allowed(rdram,7),"Canonical override leaked into an older mode");}
 need(bytes(file)==before,"Gate mutated shared collection");
 put(dir/"unbought.json",0);c::initialize(dir/"unbought.json");
 for(unsigned eggs=0;eggs<4;eggs++){c::start_session(5,0xFFFFFF,eggs,true);for(unsigned color=6;color<8;color++)need(c::egg_allowed(rdram,color)==bool(eggs&(1u<<(color-6))),"Earned legacy eggs no longer searchable");}
 for(unsigned color=0;color<20;color++){c::start_session(5,0xFFFFFF,3,true);if(color!=6&&color!=7)need(!c::egg_allowed(rdram,color),"Non-BW color admitted");}
 // Story Hybrid never allocates Trial currency, even if stale native mode1 is seen.
 c::initialize(dir/"none.json");c::start_session(5,0xFFFFFF,3,true);
 const gpr root=int32_t(0x800F7EA0),meta=int32_t(0x80010000),objects=int32_t(0x80011000);
 MEM_H(0,meta)=26;MEM_W(4,meta)=int32_t(objects);MEM_H(0,objects)=0x4001;
 for(unsigned i=1;i<=25;i++){MEM_H(i*16,objects)=0x406F;MEM_W(i*16+4,objects)=i*20;MEM_W(i*16+8,objects)=400;}
 for(unsigned native:{0u,1u}){MEM_W(0xCE8,root)=native;c::enter(rdram,43,uint32_t(meta),true);need(c::view().state.levels[0].places.empty(),"Hybrid Story allocated turquoise currency");}
 need(!fs::exists(dir/"none.json"),"Story wrote a collection file");
 for(int mode=-32;mode<=32;mode++){
  need(m::valid(mode)==(mode>=0&&mode<=5),"Invalid PC mode accepted");
  need(m::story(mode)==(mode==0||mode==5),"Story mapping changed");
  need(m::hybrid_content(mode)==(mode==3||mode==5),"Hybrid content scope changed");
  need(m::hybrid_trial_records(mode)==(mode==3),"Records bank scope changed");
  need(m::native_choice(mode)==(mode<0||mode>5?-1:mode==0||mode==5?0:mode==2?3:1),"Native menu routing changed");
  need(m::solo_playable(mode)==(mode==0||mode==1||mode==2||mode==3||mode==5),"Deferred combat scope opened");
 }
 nlohmann::json result={{"passed",true},{"checks",checks},{"actualCollectionsModuleExecuted",true},{"syntheticRam",true},{"canonicalBoundaryExplicit",true},{"gameLaunched",false},{"personalFilesAccessed",false}};
 std::ofstream(dir/"execution.json")<<result.dump(2)<<'\n';std::cout<<result.dump()<<'\n';return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
