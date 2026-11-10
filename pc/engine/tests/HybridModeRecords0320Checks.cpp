#include "../hybrid_mode.hpp"
#include "../hybrid_rewards.hpp"
#include "json/json.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace hm=workshop::hybrid_mode;
namespace fs=std::filesystem;
using Json=nlohmann::json;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static std::string bytes(const fs::path& path){std::ifstream file(path,std::ios::binary);return {std::istreambuf_iterator<char>(file),{}};}
static unsigned checksum(const std::array<uint8_t,1000>& values){unsigned h=2166136261u;for(auto b:values)h=(h^b)*16777619u;return h;}
int main(int argc,char** argv){try{
 need(argc==2,"Use a new isolated fixture output");const auto out=fs::absolute(argv[1]);need(!fs::exists(out),"Old test output must remain preserved");fs::create_directories(out);
 const auto file=out/"pc-hybrid-records.json";std::array<uint8_t,1000> normal{},hybrid{},changed{},story{};for(unsigned i=0;i<1000;++i){normal[i]=uint8_t(i*17+31);hybrid[i]=uint8_t(i*29+83);changed[i]=uint8_t(i*47+11);story[i]=uint8_t(i*61+97);}
 auto sidecar=[&](const std::array<uint8_t,1000>& records){std::ofstream output(file,std::ios::binary);output<<Json{{"version",1},{"records",records},{"checksum",checksum(records)}}.dump()<<'\n';};sidecar(hybrid);const auto initial=bytes(file);
 std::vector<uint8_t> memory(0x800000,0xA5);auto* rdram=memory.data();constexpr uint32_t payload=0x80110000,buffer=0x80112000,metadata=0x80114000,objects=0x80114100,actor=0x80115000,stack=0x80116000;
 auto fill=[&](uint32_t at,const std::array<uint8_t,1000>& records){for(unsigned i=0;i<1018;++i)MEM_B(int(i),int32_t(at))=0xA5;for(unsigned i=0;i<1000;++i)MEM_B(int(i+4),int32_t(at))=records[i];};
 auto equal=[&](uint32_t at,const std::array<uint8_t,1000>& records){for(unsigned i=0;i<1000;++i)need(MEM_BU(int(i+4),int32_t(at))==records[i],"Record payload differs");for(unsigned i=0;i<4;++i)need(MEM_BU(int(i),int32_t(at))==0xA5,"Record prefix changed");for(unsigned i=1004;i<1018;++i)need(MEM_BU(int(i),int32_t(at))==0xA5,"Record suffix changed");};
 for(unsigned i=0;i<256;++i)MEM_B(int(i),int32_t(0x807F2100))=0; // Legal synthetic glyph lookup.
 hm::initialize(file);need(hm::readable()&&hm::saved()&&!hm::active(),"Valid existing Trial sidecar failed to initialize");fill(payload,normal);fill(buffer,changed);
 hm::activate(false);hm::records_loaded(rdram,payload);need(hm::records_saving(rdram,payload,buffer),"Ordinary disabled mode blocked native save");equal(payload,normal);equal(buffer,changed);need(bytes(file)==initial,"Ordinary mode wrote Trial records");
 // Legacy single-argument activation retains exactly the old Trial behavior.
 hm::activate(true);need(hm::active(),"Legacy content activation failed");hm::records_loaded(rdram,payload);equal(payload,hybrid);fill(payload,changed);need(hm::records_saving(rdram,payload,buffer),"Legacy Hybrid Trial record save failed");equal(buffer,normal);
 auto persisted=Json::parse(bytes(file));need(persisted["records"]==Json(changed)&&persisted["checksum"]==checksum(changed),"Legacy Trial records/checksum changed semantics");need(bytes(fs::path(file.string()+".bak"))==initial,"Legacy Trial backup no longer retains old scores");
 // New Story scope uses all hybrid content with no read/write score swapping.
 hm::activate(true,false);need(hm::active()&&workshop::rewards::custom_enemy_variant&&workshop::rewards::lucky_fruit_type,"Story content lost hybrid enemy/lucky hooks");hm::initialize_ram(rdram);need(MEM_W(0,int32_t(0x807FC140))>=6&&MEM_W(0,int32_t(0x807FC140))<=11,"Story hybrid Lucky Fruit not installed");
 MEM_H(0,int32_t(metadata))=1;MEM_W(4,int32_t(metadata))=objects;MEM_H(0,int32_t(objects))=0x4001;MEM_B(0,int32_t(0x800F8DFC))=0;MEM_B(0,int32_t(0x800F8DFD))=0;hm::enter(rdram,metadata,43);
 recomp_context ctx{};ctx.r4=int32_t(actor);ctx.r5=int32_t(0x403A0000);ctx.r6=100;ctx.r7=200;ctx.r29=int32_t(stack);MEM_W(0x10,ctx.r29)=0;hm::created(rdram,&ctx,43);const auto enemy=hm::enemy_variant(actor);need(enemy&&*enemy<6,"Story hybrid enemy variant missing");hm::recolored(rdram,actor);need(hm::enemy_variant(actor)==((*enemy+1)%6),"Story hybrid enemy recolor no longer works");
 const auto trialBefore=bytes(file),backupBefore=bytes(fs::path(file.string()+".bak"));fill(payload,story);fill(buffer,normal);hm::records_loaded(rdram,payload);equal(payload,story);need(hm::records_saving(rdram,payload,buffer),"Story native save blocked by unused Trial score bank");equal(buffer,normal);need(bytes(file)==trialBefore&&bytes(fs::path(file.string()+".bak"))==backupBefore,"Story Hybrid modified Trial score bank or backup");
 // A new activation must not reuse a capture from the earlier Trial session.
 hm::activate(true);const auto beforeMissingCapture=bytes(file);need(!hm::records_saving(rdram,payload,buffer)&&bytes(file)==beforeMissingCapture,"Activation reused stale ordinary score capture");hm::records_loaded(rdram,payload);equal(payload,changed);fill(payload,hybrid);need(hm::records_saving(rdram,payload,buffer),"Fresh Trial capture did not recover");equal(buffer,story);
 hm::activate(false,false);need(!hm::active()&&!workshop::rewards::lucky_fruit_type,"Disabled content retained its Lucky hook");fill(payload,story);fill(buffer,normal);hm::records_loaded(rdram,payload);need(hm::records_saving(rdram,payload,buffer),"Both-disabled scope blocked save");equal(payload,story);equal(buffer,normal);
 // Invalid Trial records remain preserved but cannot disable native Story saves.
 {std::ofstream bad(file,std::ios::binary);bad<<"{invalid trial record fixture";}const auto corrupt=bytes(file);hm::initialize(file);need(!hm::readable(),"Corrupt Trial fixture unexpectedly readable");hm::activate(true,false);fill(payload,story);fill(buffer,normal);hm::records_loaded(rdram,payload);need(hm::records_saving(rdram,payload,buffer)&&hm::active(),"Corrupt unrelated Trial file blocked Story content/save");equal(payload,story);equal(buffer,normal);need(bytes(file)==corrupt,"Story overwrote corrupt Trial record file");hm::activate(true);need(!hm::records_saving(rdram,payload,buffer)&&bytes(file)==corrupt,"Legacy Trial accepted corrupt records");
 // A missing Trial record sidecar is not manufactured by Story Hybrid.
 const auto missing=out/"missing-pc-hybrid-records.json";hm::initialize(missing);hm::activate(true,false);fill(payload,story);fill(buffer,normal);hm::records_loaded(rdram,payload);need(hm::records_saving(rdram,payload,buffer)&&!fs::exists(missing),"Story created a Trial record bank");equal(payload,story);equal(buffer,normal);
 hm::activate(false);std::ofstream report(out/"execution.json");report<<Json{{"passed",true},{"checks",checks},{"legacySingleArgumentBehavior",true},{"hybridContentWithoutTrialRecords",true},{"freshCaptureRequiredAfterActivation",true},{"corruptOrMissingTrialFilePreservedInStory",true},{"actualHybridModeModuleExecuted",true},{"syntheticRam",true},{"gameLaunched",false},{"realEepromAccessed",false},{"personalFilesAccessed",false}}.dump(2)<<'\n';std::cout<<"HybridModeRecords0320 "<<checks<<" checks PASS\n";return 0;
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
