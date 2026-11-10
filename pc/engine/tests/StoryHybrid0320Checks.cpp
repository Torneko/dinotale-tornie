#include "../story_hybrid0320.hpp"
#include "json/json.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif
namespace sh=workshop::story_hybrid0320;
namespace fs=std::filesystem;
using Json=nlohmann::json;
static unsigned checks=0,malformed=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static std::string bytes(const fs::path& path){std::ifstream input(path,std::ios::binary);return {std::istreambuf_iterator<char>(input),{}};}
static void write(const fs::path& path,const std::string& text){fs::create_directories(path.parent_path());std::ofstream f(path,std::ios::binary);f<<text;need(bool(f),"Cannot write fixture");}
static Json sample(sh::Order order={4,1,5,0,3,2},unsigned worlds=0,unsigned acquired=0,unsigned owned=0,unsigned alive=0){return {{"schema",1u},{"mode",5u},{"completedWorldMask",worlds},{"hybridOrder",order},{"acquiredMask",acquired},{"metalOwnedMask",owned},{"metalAliveMask",alive}};}
static unsigned expected(const sh::Order& order,unsigned worlds,unsigned acquired){unsigned result=acquired;for(unsigned i=0;i<unsigned(std::popcount(worlds));++i)result|=1u<<order[i];return result;}
static fs::path journal_path(const fs::path& out,const std::string& folder){return out/folder/sh::filename;}
int main(int argc,char** argv){try{
 need(argc==2,"Use a fresh isolated fixture directory");const auto out=fs::absolute(argv[1]);need(!fs::exists(out),"Old output must be preserved");fs::create_directories(out);
 const sh::Order order{4,1,5,0,3,2};const auto path=journal_path(out,"main");unsigned draws=0;sh::Journal journal([&]{++draws;return order;});journal.initialize(path);
 auto view=journal.view();need(view.initialized&&!view.present&&view.readable&&view.saved&&!view.active&&view.eligible==0&&!fs::exists(path.parent_path()),"Initialize/read missing wrote or invented progress");
 for(unsigned mode:{0u,1u,2u,3u,4u,6u,UINT32_MAX})need(!journal.prepare(mode,63,3)&&!fs::exists(path.parent_path()),"Other mode created/activated StoryHybrid data");
 need(journal.note_acquired(63)&&!fs::exists(path.parent_path()),"Cross-mode observation created a missing journal");
 need(!journal.successful_world(0,true)&&!journal.save_lives(0)&&!journal.new_run()&&!fs::exists(path.parent_path()),"Inactive operations created a journal");
 need(!journal.prepare(5,64,0)&&!journal.prepare(5,0,4)&&!fs::exists(path.parent_path()),"Invalid shared masks created a journal");
 need(journal.prepare(5,1,1)&&draws==1&&fs::is_regular_file(path),"First mode5 activation did not persist its sole permutation");
 view=journal.view();need(view.active&&view.present&&view.order==order&&view.worlds==0&&view.count==0&&view.eligible==1&&view.acquired==1&&view.metal_owned==1&&view.metal_alive==1,"Owned colors or first metal lives were not preserved");
 const auto initial=bytes(path);need(!journal.successful_world(0,false)&&!journal.successful_world(6,true)&&!journal.successful_world(UINT32_MAX,true)&&bytes(path)==initial,"Untrusted/out-of-range receipt altered progress");
 need(journal.successful_world(5,true),"Distinct valid world failed");view=journal.view();need(view.worlds==32&&view.count==1&&view.eligible==(1u<<4|1)&&view.acquired==1,"Eligibility used page maximum/access rather than distinct worlds");
 const auto one=bytes(path);const auto stamp=fs::last_write_time(path);for(unsigned i=0;i<20;++i)need(journal.successful_world(5,true)&&bytes(path)==one&&fs::last_write_time(path)==stamp,"Duplicate world changed order/count/file");
 need(journal.successful_world(2,true)&&journal.view().count==2&&journal.view().eligible==((1u<<4)|(1u<<1)|1u),"Second distinct world did not extend the prefix");
 need(journal.note_acquired(32)&&journal.view().acquired==33&&journal.view().eligible==51,"Observed existing ownership was dropped or counted as another world");
 need(journal.save_lives(0)&&journal.view().metal_alive==0&&!journal.save_lives(2),"Unowned metal life accepted or death not saved");
 need(journal.prepare(5,0,3)&&draws==1&&journal.view().metal_owned==3&&journal.view().metal_alive==2,"New owned metal revived an older dead one or rerolled");
 need(journal.new_run()&&journal.view().metal_alive==3&&journal.view().worlds==36&&journal.view().order==order&&journal.view().acquired==33,"New run erased durable progress");
 need(journal.save_lives(1)&&journal.reset_run(0)&&journal.view().metal_alive==3&&journal.view().count==2,"Reset erased progress or failed to restore owned local lives");
 journal.deactivate();const auto beforeCross=bytes(path);need(!journal.successful_world(1,true)&&!journal.save_lives(0)&&!journal.new_run()&&bytes(path)==beforeCross,"Inactive receipt/life event changed state");
 need(journal.note_acquired(2)&&journal.view().acquired==35&&!journal.view().active,"Cross-mode observation did not update existing journal safely");
 sh::Journal reloaded([]()->sh::Order{throw std::runtime_error("must not reroll");});reloaded.initialize(path);need(reloaded.view().order==order&&reloaded.view().worlds==36&&!reloaded.view().active,"Reload changed data/activated gameplay");need(reloaded.prepare(5,0,3)&&reloaded.view().order==order&&reloaded.view().metal_alive==3,"Continuing run rerolled or removed ownership");
 for(unsigned page:{4u,0u,1u,3u})need(reloaded.successful_world(page,true),"Later distinct world rejected");need(reloaded.view().count==6&&reloaded.view().eligible==63&&reloaded.view().worlds==63,"Six distinct worlds did not expose exactly six colors");
 need(reloaded.view().acquired==35,"Eligibility automatically granted ownership");
 // Exhaust all64 distinct-world subsets against an independently computed prefix.
 for(unsigned mask=0;mask<64;++mask){const auto p=journal_path(out,"subset-"+std::to_string(mask));write(p,sample(order,mask,8,3,1).dump());sh::Journal j;j.initialize(p);const auto v=j.view();need(v.readable&&v.count==unsigned(std::popcount(mask))&&v.eligible==expected(order,mask,8)&&v.acquired==8,"World subset/cardinality projection incorrect");const auto b=bytes(p);(void)j.view();need(bytes(p)==b,"Read view altered an existing journal");}
 // Every permutation is legal; its persisted prefix survives preparation.
 sh::Order permutation{0,1,2,3,4,5};unsigned permutations=0;
 do{const auto p=journal_path(out,"order-"+std::to_string(permutations));write(p,sample(permutation,0b10101,2).dump());sh::Journal j([]()->sh::Order{throw std::runtime_error("unexpected new order");});j.initialize(p);need(j.view().readable&&j.view().order==permutation&&j.view().eligible==expected(permutation,0b10101,2),"Valid persisted permutation or prefix rejected");need(j.prepare(5)&&j.view().order==permutation,"Prepare changed persisted permutation");++permutations;}while(std::next_permutation(permutation.begin(),permutation.end()));need(permutations==720,"Permutation coverage incomplete");
 for(unsigned fault=0;fault<24;++fault){auto bad=sample(order,3,9,3,2);std::string text;
  if(fault==0)bad["schema"]=2;if(fault==1)bad["mode"]=3;if(fault==2)bad["completedWorldMask"]=64;if(fault==3)bad["completedWorldMask"]=-1;if(fault==4)bad["completedWorldMask"]=1.0;if(fault==5)bad["acquiredMask"]=64;if(fault==6)bad["metalOwnedMask"]=4;if(fault==7)bad["metalAliveMask"]=4;if(fault==8){bad["metalOwnedMask"]=1;bad["metalAliveMask"]=2;}if(fault==9)bad["hybridOrder"]={0,1,2,3,4};if(fault==10)bad["hybridOrder"]={0,1,2,3,4,4};if(fault==11)bad["hybridOrder"]={0,1,2,3,4,6};if(fault==12)bad["hybridOrder"]={0,1,2,3,4,-1};if(fault==13)bad["hybridOrder"]={0,1,2,3,4,5.0};if(fault==14)bad["extra"]=1;if(fault==15)bad.erase("acquiredMask");if(fault==16)bad["hybridOrder"]="012345";if(fault==17)bad["metalOwnedMask"]="3";if(fault==18)bad["schema"]=true;if(fault==19)bad["mode"]=nullptr;
  text=bad.dump();if(fault==20)text="{";if(fault==21)text=std::string(4097,' ');if(fault==22)text="null";if(fault==23){text=sample().dump();text.insert(1,"\"schema\":1,");}
  const auto p=journal_path(out,"invalid-"+std::to_string(fault));write(p,text);sh::Journal j;j.initialize(p);need(!j.view().readable&&!j.view().saved&&!j.view().active&&j.view().eligible==0,"Invalid file remained eligible");need(!j.prepare(5,63,3)&&!j.successful_world(0,true)&&!j.note_acquired(63)&&!j.save_lives(0)&&!j.new_run()&&bytes(p)==text&&!fs::exists(fs::path(p.string()+".lock")),"Invalid journal replaced or modified");++malformed;
 }
 {sh::Journal j;j.initialize(out/"ordinary-metal.json");need(!j.view().readable&&!j.prepare(5)&&!fs::exists(out/"ordinary-metal.json"),"Unexpected journal filename accepted");}
 {const auto p=journal_path(out,"directory");fs::create_directories(p);sh::Journal j;j.initialize(p);need(!j.view().readable&&!j.prepare(5)&&fs::is_directory(p),"Directory journal followed or replaced");}
 {const auto p=journal_path(out,"disappeared");sh::Journal j([&]{return order;});j.initialize(p);need(j.prepare(5),"Disappeared fixture could not prepare");fs::remove(p);need(!j.successful_world(0,true)&&!j.view().readable&&j.view().eligible==0&&!fs::exists(p),"A disappeared existing journal was recreated silently");}
 {const auto p=journal_path(out,"bad-random");unsigned calls=0;sh::Journal j([&]{return ++calls==1?sh::Order{}:order;});j.initialize(p);need(!j.prepare(5)&&!j.view().active&&j.view().eligible==0&&!fs::exists(p),"Bad random factory persisted an invalid order");need(j.prepare(5)&&calls==2&&j.view().order==order,"Explicit recovery after failed first creation failed");}
 // Existing canonical-looking files are synthetic sentinels. This service must
 // touch only its own journal and lock, including during saves/reset/death.
 {const auto directory=out/"canonical-sentinels";fs::create_directories(directory);const std::array<const char*,5> names{"pc-metal-yoshis.json","pc-hybrid-finds.json","pc-collection.json","pc-heart-wallet.json","yoshisstory.n64.us.bin"};for(auto name:names)write(directory/name,std::string("original-")+name);sh::Journal j([&]{return order;});j.initialize(directory/sh::filename);need(j.prepare(5,63,3)&&j.successful_world(0,true)&&j.save_lives(0)&&j.reset_run(3),"Sentinel fixture operation failed");for(auto name:names)need(bytes(directory/name)==std::string("original-")+name,"Canonical ownership/economy/EEPROM was modified");}
 // Two stale instances re-read under an exclusive lock; first persisted order
 // wins, and independent completed-world bits are merged without lost updates.
 {const auto p=journal_path(out,"concurrent");sh::Journal a([&]{return order;}),b([]{return sh::Order{5,4,3,2,1,0};});a.initialize(p);b.initialize(p);need(a.prepare(5)&&b.prepare(5)&&b.view().order==order,"Competing creation rerolled a persisted order");need(a.successful_world(1,true)&&b.successful_world(4,true),"Stale writer did not merge world");sh::Journal final;final.initialize(p);need(final.view().worlds==18&&final.view().count==2,"World update lost a stale instance bit");
  std::array<std::unique_ptr<sh::Journal>,6> writers;for(auto& j:writers){j=std::make_unique<sh::Journal>();j->initialize(p);need(j->prepare(5),"Concurrent fixture prepare failed");}std::atomic_uint succeeded=0;std::array<std::thread,6> threads;
  for(unsigned i=0;i<6;++i)threads[i]=std::thread([&,i]{for(unsigned attempt=0;attempt<300;++attempt){if(writers[i]->successful_world(i,true)){++succeeded;return;}std::this_thread::sleep_for(std::chrono::milliseconds(2));}});for(auto& t:threads)t.join();final.initialize(p);need(succeeded==6&&final.view().worlds==63&&final.view().order==order,"Concurrent trusted worlds lost progress/order");
 }
#ifdef _WIN32
 {const auto p=journal_path(out,"blocked-publish");sh::Journal j([&]{return order;});j.initialize(p);need(j.prepare(5),"Blocked publish fixture failed");const auto original=bytes(p);HANDLE blocker=CreateFileW(p.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);need(blocker!=INVALID_HANDLE_VALUE,"Cannot create publication denial fixture");const bool accepted=j.successful_world(0,true);CloseHandle(blocker);need(!accepted&&!j.view().saved&&j.view().eligible==0&&bytes(p)==original,"Failed atomic publication changed old file or remained eligible");need(j.successful_world(0,true)&&j.view().saved&&j.view().worlds==1,"Publication retry did not recover");for(auto& entry:fs::directory_iterator(p.parent_path()))need(entry.path().filename()==sh::filename||entry.path().filename()==std::string(sh::filename)+".lock","Failed publication left an owned temporary");}
#endif
 std::ofstream report(out/"execution.json");report<<Json{{"passed",true},{"checks",checks},{"worldSubsets",64},{"permutations",permutations},{"malformedPreserved",malformed},{"concurrentWriters",6},{"trustedReceiptCallerSupplied",true},{"gameOrNativeRamUsed",false},{"eepromReadOrWritten",false},{"personalFilesAccessed",false},{"eligibilityGrantsOwnership",false}}.dump(2)<<'\n';std::cout<<"StoryHybrid0320 "<<checks<<" checks PASS\n";return 0;
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
