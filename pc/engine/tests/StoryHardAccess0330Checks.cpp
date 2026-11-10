#include "../story_hard_access0330.hpp"
#include "../story_rights0320.hpp"
#include "../hybrid_finds.hpp"
#include "../metal_yoshis0300.hpp"
#include "../boss_challenges0260.hpp"
#include "json/json.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
namespace fs=std::filesystem;namespace access=workshop::story_hard_access0330;
static unsigned checks=0,pool_cases=0,gate_cases=0,valid_files=0,invalid_files=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
static std::string bytes(const fs::path& p){std::ifstream in(p,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};}
static void put(const fs::path& p,const std::string& b){std::ofstream out(p,std::ios::binary);out<<b;out.close();need(bool(out),"Fixture write failed");}
static workshop::story_rights0320::View canonical(){workshop::story_rights0320::View r;r.present=r.readable=true;r.trial=0xFFFFFF;r.progression={0xFFFFFF,0,{1,2,3,4},3,63};r.bw_earned=3;return r;}
static workshop::finds::View hybrids(){workshop::finds::View v;v.found=63;v.initialized=v.migrated=v.readable=v.saved=true;return v;}
static workshop::metal_yoshis0300::View metals(){workshop::metal_yoshis0300::View v;v.found=3;v.story_alive=0;v.initialized=v.readable=v.saved=true;return v;}
static std::string token(unsigned n){std::ostringstream out;out<<std::hex<<std::setfill('0')<<std::setw(32)<<n;return out.str();}
static nlohmann::json ledger(unsigned mask){std::array<unsigned,14> counts{};auto runs=nlohmann::json::array();for(unsigned n=0;n<14;++n)if(mask&(1u<<n)){counts[n]=1;runs.push_back({{"token",token(n+1)},{"color",n}});}return {{"schema",1u},{"mask14",mask},{"counts14",counts},{"runs",runs}};}
int main(int argc,char**argv){try{
 need(argc==2,"Expected NEW disposable fixture directory");const auto out=fs::absolute(argv[1]);need(!fs::exists(out),"Never overwrite old fixtures");fs::create_directories(out);
 auto c=canonical();auto h=hybrids();auto m=metals();
 for(unsigned bw=0;bw<4;++bw)for(unsigned hybrid=0;hybrid<64;++hybrid)for(unsigned metal=0;metal<4;++metal){
  c.bw_earned=c.progression.eggs=uint8_t(bw);c.raw_flags=255;c.legacy_owned=63;h.found=hybrid;m.found=metal;m.story_alive=0;
  const auto owned=access::owned(c,h,m);need(owned&&*owned==(0x3Fu|(bw<<6)|(hybrid<<8)|(metal<<14)),"Owned pool used eligibility/Storyalive or lost genuine Trial colors");++pool_cases;
 }
 c=canonical();h=hybrids();m=metals();
 auto invalid_owned=[&](const auto& bad_c,const auto& bad_h,const auto& bad_m){need(!access::owned(bad_c,bad_h,bad_m),"Invalid ownership authority was accepted");};
 {auto b=c;b.present=false;invalid_owned(b,h,m);b=c;b.readable=false;invalid_owned(b,h,m);b=c;b.bw_earned=4;invalid_owned(b,h,m);b=c;b.bw_earned=0;invalid_owned(b,h,m);b=c;b.progression.lives=64;invalid_owned(b,h,m);b=c;b.trial=0;invalid_owned(b,h,m);}
 for(unsigned bit=0;bit<4;++bit){auto b=h;if(bit==0)b.initialized=false;if(bit==1)b.migrated=false;if(bit==2)b.readable=false;if(bit==3)b.saved=false;invalid_owned(c,b,m);}
 {auto b=h;b.found=64;invalid_owned(c,b,m);}
 for(unsigned bit=0;bit<3;++bit){auto b=m;if(bit==0)b.initialized=false;if(bit==1)b.readable=false;if(bit==2)b.saved=false;invalid_owned(c,h,b);}
 {auto b=m;b.found=4;invalid_owned(c,h,b);}
 // Pure future adapter policy; these typed synthetic vectors are NOT native wins.
 for(unsigned mask=0;mask<=0xFFFF;++mask){
  access::Evidence e{mask,0xFFFF,16,true,true,{}};need(access::entitled(e)==(mask==0xFFFF),"Future pure ownership gate accepts missing colors");++gate_cases;
  e={0xFFFF,mask,16,true,true,{}};need(access::entitled(e)==(mask==0xFFFF),"Future pure win gate accepts missing colors");++gate_cases;
  e={0xFFFF,mask,14,true,true,{}};need(!access::entitled(e),"Fourteen-color evidence opened sixteen-color gate");++gate_cases;
 }
 for(unsigned width:{0u,1u,13u,14u,15u,17u,32u,0xFFFFFFFF})need(!access::entitled({0xFFFF,0xFFFF,width,true,true,{}}),"Unknown evidence width accepted");
 need(!access::entitled({0xFFFF,0xFFFF,16,false,true,{}})&&!access::entitled({0xFFFF,0xFFFF,16,true,false,{}}),"Unreadable/unsaved evidence accepted");
 need(!access::entitled({0x1FFFF,0xFFFF,16,true,true,{}})&&!access::entitled({0xFFFF,0x1FFFF,16,true,true,{}}),"High evidence bits accepted");
 const auto missing=out/"never-created"/"pc-boss-challenges.json";const auto absent=access::inspect(c,h,m,missing);need(absent.readable&&absent.saved&&absent.bossWidth==0&&!access::entitled(absent)&&!fs::exists(missing.parent_path()),"Missing read created a profile or granted access");
 for(unsigned mask:{0u,1u,2u,0x1555u,0x2AAAu,0x3FFFu}){
  const auto dir=out/("valid-"+std::to_string(valid_files++));fs::create_directory(dir);const auto file=dir/"pc-boss-challenges.json";put(file,ledger(mask).dump(2));const auto before=bytes(file);const auto timestamp=fs::last_write_time(file);const auto e=access::inspect(c,h,m,file);
  need(e.readable&&e.saved&&e.owned16==0xFFFF&&e.bossWidth==14&&e.bossWins16==mask&&!access::entitled(e),"Valid legacy ledger treated as incomplete parse or sixteen-color proof");
  need(workshop::boss_challenges0260::load(dir)&&workshop::boss_challenges0260::snapshot().mask14==mask,"Existing legacy reader disagrees with accepted fixture");
  need(bytes(file)==before&&fs::last_write_time(file)==timestamp&&std::distance(fs::directory_iterator(dir),fs::directory_iterator{})==1,"Read-only inspect or legacy load wrote a file");
 }
 std::vector<std::string> invalid{"broken",std::string(16*1024*1024+1,'x')};auto j=ledger(0x3FFF);
 auto add=[&](const nlohmann::json& value){invalid.push_back(value.dump());};
 {auto b=j;b["schema"]=2;add(b);b=j;b["mask14"]=0xFFFF;add(b);b=j;b["mask14"]=0;add(b);b=j;b["counts14"]=std::array<unsigned,16>{};add(b);b=j;b["counts14"][0]=2;add(b);b=j;b["counts14"][0]=-1;add(b);b=j;b["counts14"][0]=1.0;add(b);b=j;b["runs"][0]["color"]=14;add(b);b=j;b["runs"][0]["token"]=std::string(32,'A');add(b);b=j;b["runs"][1]["token"]=b["runs"][0]["token"];add(b);b=j;b["runs"][0]["unexpected"]=true;add(b);b=j;b["extra"]=true;add(b);b=j;b.erase("runs");add(b);}
 invalid.push_back("{\"schema\":1,"+j.dump().substr(1));
 add({{"schema",2u},{"mask16",0xFFFFu},{"counts16",std::array<unsigned,16>{}},{"runs",nlohmann::json::array()}});
 for(const auto& value:invalid){const auto file=out/("invalid-"+std::to_string(invalid_files++)+".json");put(file,value);const auto before=bytes(file);const auto e=access::inspect(c,h,m,file);need(!e.readable&&!e.saved&&!access::entitled(e)&&e.bossWidth!=16&&bytes(file)==before,"Malformed/future evidence accepted, overwritten or upgraded to16");}
 const auto directory=out/"not-a-file";fs::create_directory(directory);need(!access::inspect(c,h,m,directory).readable,"Directory accepted as evidence file");
 auto locked_h=h;locked_h.saved=false;const auto protected_file=out/"protected.json";put(protected_file,j.dump());const auto old=bytes(protected_file);need(!access::inspect(c,locked_h,m,protected_file).readable&&bytes(protected_file)==old,"Invalid authority caused a mutation or admitted access");
 for(const auto& file:fs::recursive_directory_iterator(out))need(file.path().extension()!=".lock"&&file.path().extension()!=".bak"&&file.path().filename().string().find(".tmp")==std::string::npos,"Read-only access created auxiliary writable state");
 nlohmann::json result{{"passed",true},{"checks",checks},{"ownedPoolCases",pool_cases},{"pureGateCases",gate_cases},{"validLegacyFiles",valid_files},{"invalidOrFutureFiles",invalid_files},{"actualLegacyReaderCompared",true},{"productionReaderCanEmit16",false},{"pure16VectorsAreSyntheticNotVictories",true},{"existingProductionFilesEdited",false},{"gameLaunched",false},{"GPUUsed",false},{"ROMOrPersonalFilesUsed",false}};put(out/"execution.json",result.dump(2)+'\n');std::cout<<result.dump()<<'\n';return 0;
}catch(const std::exception&e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
