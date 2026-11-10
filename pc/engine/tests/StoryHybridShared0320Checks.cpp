#include "../achievements.hpp"
#include "../hybrid_finds.hpp"
#include "../metal_yoshis0300.hpp"
#include "../hybrid_completion0290.hpp"
#include "../game_modes0320.hpp"
#include "json/json.hpp"
#include <bit>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>
namespace fs=std::filesystem;
namespace a=workshop::achievements;
namespace f=workshop::finds;
namespace m=workshop::metal_yoshis0300;
namespace hc=workshop::hybrid_completion0290;
namespace p=workshop::progress;
static unsigned checks=0,achievement_cases=0,metal_cases=0,migration_cases=0;
static void need(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
static std::string bytes(const fs::path& file){std::ifstream in(file,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};}
static void put(const fs::path& file,const nlohmann::json& data){std::ofstream out(file,std::ios::binary);out<<data.dump(2)<<'\n';out.close();need(bool(out),"Fixture write failed");}
static void word(uint8_t* rdram,uint32_t address,unsigned value){MEM_W(0,int32_t(address))=int32_t(value);}
static unsigned word(uint8_t* rdram,uint32_t address){return uint32_t(MEM_W(0,int32_t(address)));}
static unsigned story_for(unsigned mask){return std::popcount(mask)==5?p::column(3):0;}
static p::State rights(unsigned mask){
 p::State result{0,255,{255,255,255,255},0,63};
 if(!mask)return result;
 unsigned left=mask;result.first=uint8_t(std::countr_zero(left));left&=left-1;
 for(unsigned c=0;c<4&&left;++c){result.columns[c]=uint8_t(std::countr_zero(left));left&=left-1;result.trial|=p::column(c);}
 if(mask==63)result.trial=0xFFFFFF;
 if(std::popcount(mask)==5)result.trial&=~p::column(3);
 need(p::valid(result,story_for(mask))&&p::unlocked(result,story_for(mask))==mask,"Synthetic authoritative rights do not match expected mask");return result;
}
static void completion(const fs::path& path,unsigned mask){put(path,{{"schema",1u},{"observedMask",mask},{"legacyMask",0u},{"legacyChecked",true},{"legacyChecksum",nullptr}});hc::initialize(path,{});}
int main(int argc,char** argv){try{
 need(argc==2,"Expected isolated fixture directory");const auto out=fs::absolute(argv[1]);need(!fs::exists(out),"Never overwrite existing fixtures");fs::create_directories(out);
 std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
 const unsigned modes[]={0,1,2,3,4,5,6,7,8,0xFFFFFFFF};
 // Production achievement frame fence, actual native mode bits stay0/1.
 for(unsigned mode:modes)for(unsigned native:{0u,1u,2u})for(bool allowed:{false,true}){
  const auto path=out/("ach-"+std::to_string(achievement_cases)+".json");a::initialize(path);a::start_session(mode);
  recomp_context ctx{};ctx.r4=int32_t(0x800F7EA0);const auto root=gpr(int32_t(0x800F7EA0));
  word(rdram,0x800F8B88,native);MEM_B(0,int32_t(0x800F8DFC))=1;MEM_B(0,int32_t(0x800F8DFD))=2;
  word(rdram,0x800F8828,0x80030000);MEM_H(12,int32_t(0x80030000))=0x4001;
  MEM_W(0xF10,root)=0;MEM_W(0xF14,root)=8;MEM_W(0xAB8,root)=3;
  word(rdram,0x800F89D0,0);word(rdram,0x800F8AC8,0);a::frame(rdram,&ctx,allowed);
  word(rdram,0x800F89D0,30);word(rdram,0x800F8AC8,1);for(unsigned i=0;i<30;++i)MEM_W(0xB34+i*4,root)=6;
  a::frame(rdram,&ctx,allowed);a::frame(rdram,&ctx,allowed);
  const bool accepted=allowed&&native<2&&(mode==0||mode==1||mode==3||mode==5);const auto view=a::view();
  need(view.levels[6].completions==unsigned(accepted),"Achievement admission/dedup changed");need(fs::exists(path)==accepted,"Excluded session wrote achievement file");
  if(accepted){need(view.levels[6].modes==(1u<<native),"PC mode leaked into native achievement bit format");need(view.levels[6].sameFruit==64&&view.levels[6].allHearts&&view.levels[6].fullHealth,"Actual hybrid fruit/health/heart finish was not retained");a::initialize(path);need(a::view().readable&&a::view().levels[6].modes==(1u<<native),"Native mode achievements do not reload");}
  ++achievement_cases;
 }
 const auto complete=out/"completion.json";completion(complete,0xFFFFFF);
 for(unsigned mode:modes)for(bool scope:{false,true})for(unsigned found=0;found<4;++found)for(unsigned alive=0;alive<4;++alive){
  if(alive&~found)continue;
  const auto path=out/("metal-"+std::to_string(metal_cases)+".json");put(path,{{"schema",1u},{"foundMask",found},{"storyAliveMask",alive}});m::initialize(path);m::start_session(mode,scope);const auto before=bytes(path);
  const bool playable=scope&&(mode==0||mode==1||mode==2||mode==3||mode==5);const bool search=playable&&mode!=2;
  need(m::unlocked_extra()==(playable?found<<6:0),"Metal availability opened Boss/unknown or excluded StoryHybrid");
  need(m::search_mask()==(search?(~found)&3u:0),"Metal search changed existing modes or omitted StoryHybrid");
  need(m::save_story_alive(alive)==(scope&&mode==0),"StoryHybrid wrote normal Story metal lives");
  need(bytes(path)==before,"Read/identical normal lives changed journal bytes");
  for(unsigned color:{14u,15u}){const auto bit=1u<<(color-14);const bool restore=playable&&(found&bit);need(m::restore_life(color)==restore,"Restore admission differs");if(mode!=0)need(bytes(path)==before,"Non-normal Story rescue changed normal Story journal");}
  ++metal_cases;
 }
 // New StoryHybrid acquisitions share ownership without reviving normal Story.
 const auto metal_hybrid=out/"metal-story-hybrid-acquire.json";put(metal_hybrid,{{"schema",1u},{"foundMask",0u},{"storyAliveMask",0u}});m::initialize(metal_hybrid);m::start_session(5,true);
 need(m::acquire(14)&&m::view().found==1&&m::view().story_alive==0,"StoryHybrid Gold acquisition changed normal Story life");
 need(m::acquire(15)&&m::view().found==3&&m::view().story_alive==0,"StoryHybrid Silver acquisition changed normal Story life");
 const auto metal_hybrid_saved=bytes(metal_hybrid);completion(complete,0);need(m::acquire(14)&&m::restore_life(15)&&!m::save_story_alive(3)&&bytes(metal_hybrid)==metal_hybrid_saved,"StoryHybrid owned acquisition/rescue/save changed normal Story journal");
 completion(complete,0xFFFFFF);
 // Missing hybrid ownership seeds only from canonical source, never ALT RAM.
 for(unsigned mask=0;mask<64;++mask){
  const auto authoritative=rights(mask);const auto child=rights((mask+17)%64);p::write(rdram,int32_t(0x807F2000),child);word(rdram,0x807FC144,0x12345678);
  const auto before=memory;auto expected=before;auto* expected_ram=expected.data();word(expected_ram,0x807FC144,0xF17D0000|mask);
  const auto file=out/("canonical-"+std::to_string(mask)+".json");f::initialize(file);need(!fs::exists(file),"Find initialize created ownership");f::migrate(rdram,authoritative,story_for(mask),true);
  need(f::view().migrated&&f::view().saved&&f::view().found==mask&&f::available(63)==mask,"Canonical rights did not seed exact shared ownership");need(memory==expected,"Authoritative migration changed ALT run payload or other RAM");
  const auto saved=bytes(file);f::migrate(rdram,rights((mask+31)%64),0,false);need(f::view().found==mask&&bytes(file)==saved,"Untrusted later source erased prior ownership");
  f::initialize(file);need(f::view().found==mask&&f::view().migrated,"Seeded ownership failed reload");++migration_cases;
  const auto old_file=out/("legacy-"+std::to_string(mask)+".json");p::write(rdram,int32_t(0x807F2000),authoritative);f::initialize(old_file);f::migrate(rdram,story_for(mask));need(f::view().found==mask&&f::view().migrated,"Existing RAM-source overload changed");++migration_cases;
 }
 // Both proof flag and semantic progression validation are required for a seed.
 const auto good=rights(21);std::vector<p::State> invalid;
 auto bad=good;bad.trial=0x1000000;invalid.push_back(bad);bad=good;bad.first=6;invalid.push_back(bad);bad=good;bad.lives=64;invalid.push_back(bad);bad=good;bad.eggs=4;invalid.push_back(bad);bad=good;bad.columns[0]=bad.first;invalid.push_back(bad);bad=good;bad.columns[3]=2;invalid.push_back(bad);
 unsigned invalid_i=0;for(const auto& state:invalid){need(!p::valid(state,0),"Invalid fixture became semantically valid");const auto file=out/("invalid-"+std::to_string(invalid_i++)+".json");f::initialize(file);const auto before=memory;f::migrate(rdram,state,0,true);need(!f::view().migrated&&!fs::exists(file)&&memory==before,"Invalid canonical state created ownership or RAM projection");++migration_cases;}
 const auto absent=out/"untrusted.json";f::initialize(absent);f::migrate(rdram,good,0,false);need(!f::view().migrated&&!fs::exists(absent),"Unvalidated canonical source created shared ownership");++migration_cases;
 const auto corrupt=out/"corrupt.json";{std::ofstream(corrupt)<<"broken original shared file";}f::initialize(corrupt);f::migrate(rdram,good,0,true);need(!f::view().readable&&bytes(corrupt)=="broken original shared file","Corrupt shared ledger overwritten");++migration_cases;
 // Calls share the production mutex; first valid seed wins, no subsequent union.
 const auto concurrent=out/"concurrent.json";f::initialize(concurrent);std::vector<std::thread> workers;
 std::array<p::State,6> candidates{};for(unsigned i=0;i<6;++i)candidates[i]=rights(1u<<i);
 for(unsigned i=0;i<12;++i)workers.emplace_back([&,i]{for(unsigned repeat=0;repeat<40;++repeat)f::migrate(rdram,candidates[i%6],0,true);});
 for(auto& worker:workers)worker.join();const auto won=f::view().found;need(std::has_single_bit(won)&&won<64&&word(rdram,0x807FC144)==(0xF17D0000|won),"Concurrent authoritative seed merged/replaced ownership");f::initialize(concurrent);need(f::view().found==won,"Concurrent journal is not reloadable");
 nlohmann::json result{{"passed",true},{"checks",checks},{"achievementCases",achievement_cases},{"metalPolicyCases",metal_cases},{"migrationCases",migration_cases},{"concurrentMigrationCalls",480},{"actualProductionModulesLinked",true},{"nativeRAMIsSynthetic",true},{"EEPROMUsed",false},{"gameLaunched",false},{"GPUUsed",false}};put(out/"execution.json",result);std::cout<<result.dump()<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
