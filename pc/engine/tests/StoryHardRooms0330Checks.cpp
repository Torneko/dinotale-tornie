#include "../story_hard_rooms0330.hpp"
#include "../workshop_data.hpp"
#include "json/json.hpp"
#include <bit>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
namespace fs=std::filesystem;namespace rooms=workshop::story_hard_rooms0330;
static unsigned checks=0,rejections=0;
static void need(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
static size_t bank(std::span<const uint8_t> b,uint32_t p){return workshop::terrain::bank(b,p);}
static std::vector<uint16_t> preloads(std::span<const uint8_t>b,size_t h){std::vector<uint16_t> ids;for(size_t n=0x60;n<0x100;n+=2){auto k=workshop::u16(b,h+n);if(k==65535)return ids;ids.push_back(k);}throw std::runtime_error("Missing native preload terminator");}
static void word(std::vector<uint8_t>&b,size_t p,uint32_t v){workshop::terrain::word(b,p,v);}
static void half(std::vector<uint8_t>&b,size_t p,uint16_t v){workshop::terrain::half(b,p,v);}
template<class Change>static void reject(const std::vector<uint8_t>&original,Change change,const char* why){auto input=original;change(input);const auto before=input;bool failed=false;try{(void)rooms::apply(input);}catch(const std::exception&){failed=true;}need(failed,why);need(input==before,"Rejected transform changed its input ROM");++rejections;}
static void put(const fs::path&p,const nlohmann::json&data){std::ofstream f(p);f<<data.dump(2)<<'\n';f.close();need(bool(f),"Report write failed");}
int main(int argc,char**argv){try{
 need(argc==4,"Expected reference ROM, read-only placement audit and NEW output directory");
 std::ifstream file(argv[1],std::ios::binary);const std::vector<uint8_t> original((std::istreambuf_iterator<char>(file)),{});need(original.size()==0x1000000,"Reference USA ROM size differs");const auto untouched=original;
 nlohmann::json audit;std::ifstream(argv[2])>>audit;need(audit["totalWarp4Placements"]==23,"Independent source audit has wrong coverage");
 const auto out=fs::absolute(argv[3]);need(!fs::exists(out),"Never overwrite existing fixture report");fs::create_directories(out);
 const auto result=rooms::apply(original);need(original==untouched,"Pure transform modified source ROM");need(result.rom.size()==original.size()&&result.plans.size()==24,"Clone changed ROM size/course count");need(result.allocation_end==0xFF8410,"Conservative24-clone byte budget changed");
 std::vector<bool> allowed(original.size(),false);for(size_t i=rooms::allocation_start;i<result.allocation_end;++i)allowed[i]=true;
 std::set<unsigned> cloned;auto dataset=nlohmann::json::array();workshop::navigation::Source native(original),clones(result.rom);
 for(unsigned level=0;level<24;++level){const auto&p=result.plans[level];need(p.level==level&&p.room<177&&cloned.insert(p.room).second,"Plan identities duplicated or out of range");need(p.added_index==p.original_count&&p.original_count+1<=149,"Native object cap/index differs");
  const size_t entry=0xA84A0+size_t(p.room)*8+4;for(unsigned n=0;n<4;++n)allowed[entry+n]=true;
  need(bank(result.rom,workshop::u32(result.rom,entry))==p.header,"Cloned room header pointer invalid");
  need(bank(result.rom,workshop::u32(result.rom,p.header+0x5C))==p.meta+8&&bank(result.rom,workshop::u32(result.rom,p.meta+8))==p.meta,"Native metadata indirection invalid");
  need(workshop::u16(result.rom,p.meta)==p.original_count+1&&bank(result.rom,workshop::u32(result.rom,p.meta+4))==p.objects,"Cloned native count/list pointer invalid");
  need(std::equal(original.begin()+p.source_objects,original.begin()+p.source_objects+size_t(p.original_count)*12,result.rom.begin()+p.objects),"An original object/fruit/placement was modified or replaced");
  need(workshop::u32(result.rom,p.record)==0x46920000&&workshop::u32(result.rom,p.record+4)==p.x&&workshop::u32(result.rom,p.record+8)==p.y,"Native4692 appended record differs");
  const auto before=native.objects(p.room),after=clones.objects(p.room);need(after.size()==before.size()+1,"Native reader did not observe additive list");for(unsigned i=0;i<before.size();++i)need(before[i].packed==after[i].packed&&before[i].x==after[i].x&&before[i].y==after[i].y,"Native source prefix changed");
  const auto source_preload=preloads(original,p.source_header),new_preload=preloads(result.rom,p.header);need(std::count(source_preload.begin(),source_preload.end(),0x401D)==1&&std::count(new_preload.begin(),new_preload.end(),0x401D)==1,"401D preload dropped or duplicated");need(std::count(new_preload.begin(),new_preload.end(),0x4692)==1,"Question bubble preload absent/duplicated");auto restored=new_preload;restored.erase(std::find(restored.begin(),restored.end(),0x4692));need(restored==source_preload,"Other original preload resources changed");
  const size_t old_term=0x60+source_preload.size()*2;for(size_t off=0;off<0x118;++off){const bool changed=(off>=0x5C&&off<0x60)||(off>=old_term&&off<old_term+4);if(!changed)need(result.rom[p.header+off]==original[p.source_header+off],"Unrelated camera/music/descriptor field changed");}
  need(p.terrain_page_validated&&!p.floor_or_occlusion_certified&&native.within_terrain(p.room,{p.x,p.y}),"Plan lacks existing native terrain page or falsely certifies floor");
  const float dx=std::bit_cast<float>(p.x)-std::bit_cast<float>(p.anchor.x),dy=std::bit_cast<float>(p.y)-std::bit_cast<float>(p.anchor.y);need((dx==32||dx==-32)&&dy==(level==18?-40:40),"Bubble is not near its native anchor or5-3 top-page exception");
  if(level!=18){const auto& exact=audit["levels"][level]["warp4"][0];need(p.anchor.kind==rooms::AnchorKind::Warp4&&p.anchor.packed==0x450A0000&&p.room==exact["room"]&&std::bit_cast<float>(p.anchor.x)==exact["x"].get<float>()&&std::bit_cast<float>(p.anchor.y)==exact["y"].get<float>(),"Anchor differs from independent23-Warp4 source dataset");}
  else{need(p.room==123&&p.anchor.kind==rooms::AnchorKind::NativeEntry&&(p.anchor.x!=0||p.anchor.y!=0),"5-3 used missingWarp4 or zero placeholder entry");for(const auto&obj:before)need(obj.packed>>16!=0x450A,"5-3 unexpectedly acquired a Warp4");need(std::count_if(before.begin(),before.end(),[](const auto&o){return o.packed==0x40010000&&o.x==0&&o.y==0;})==1,"Original(0,0) transport4001 was changed");}
  dataset.push_back({{"level",level},{"room",p.room},{"originalCount",p.original_count},{"appendedIndex",p.added_index},{"packed",p.packed},{"x",std::bit_cast<float>(p.x)},{"y",std::bit_cast<float>(p.y)},{"anchor",{{"kind",p.anchor.kind==rooms::AnchorKind::Warp4?"Warp4":"NativeEntry"},{"packed",p.anchor.packed},{"x",std::bit_cast<float>(p.anchor.x)},{"y",std::bit_cast<float>(p.anchor.y)},{"room",p.anchor.room},{"viaRoom",p.anchor.via_room},{"viaPacked",p.anchor.via_packed},{"sourceRecord",p.anchor.record}}},{"cloneObjects",p.objects},{"cloneMeta",p.meta},{"cloneHeader",p.header},{"appendedRecord",p.record},{"terrainPageValidated",true},{"floorOrOcclusionCertified",false}});
 }
 for(size_t i=0;i<original.size();++i)if(!allowed[i])need(original[i]==result.rom[i],"ROM mutation escaped24 room pointers/private arena");
 for(unsigned id=0;id<177;++id)if(!cloned.contains(id)){need(workshop::u32(original,0xA84A0+id*8+4)==workshop::u32(result.rom,0xA84A0+id*8+4),"Unrelated room was redirected");}
 reject(original,[](auto&b){b.pop_back();},"Truncated ROM accepted");reject(original,[](auto&b){b[0]=0;},"Wrong byte order/header accepted");reject(original,[](auto&b){b[0xFFFFFF]=0;},"Occupied allocation arena accepted");
 const auto&p=result.plans[0];reject(original,[&](auto&b){half(b,p.source_meta,149);},"Appending beyond149 accepted");reject(original,[&](auto&b){word(b,p.anchor.record,0x45090000);},"Missing nativeWarp4 accepted");reject(original,[&](auto&b){word(b,p.source_objects,0x450A0000);},"Duplicate nativeWarp4 accepted");reject(original,[&](auto&b){word(b,p.anchor.record+4,0x7FC00000);},"NaN anchor accepted");
 const size_t term=0x60+preloads(original,p.source_header).size()*2;reject(original,[&](auto&b){half(b,p.source_header+term+2,0x1234);},"Occupied header resource padding overwritten");
 {auto b=original;half(b,p.source_header+term,0x4692);half(b,p.source_header+term+2,0xFFFF);const auto one=rooms::apply(b);const auto ids=preloads(one.rom,one.plans[0].header);need(std::count(ids.begin(),ids.end(),0x4692)==1,"Pre-existing4692 was duplicated");}
 bool twice=false;try{(void)rooms::apply(result.rom);}catch(const std::exception&){twice=true;}need(twice,"Reapplying cloned ROM created another bubble set");
 put(out/"plans.json",{{"plans",dataset},{"allocationStart",rooms::allocation_start},{"allocationEnd",result.allocation_end},{"allocationBytes",result.allocation_end-rooms::allocation_start},{"ROMExported",false},{"floorOrOcclusionCertified",false}});
 nlohmann::json report{{"passed",true},{"checks",checks},{"courses",24},{"nativeWarp4Anchors",23},{"lastRoom123EntryAnchors",1},{"rejectedMutations",rejections},{"cloneBytes",result.allocation_end-rooms::allocation_start},{"originalObjectsPreserved",true},{"originalROMUnchanged",true},{"patchedROMExported",false},{"floorOrOcclusionCertified",false},{"gameLaunched",false},{"GPUUsed",false}};put(out/"execution.json",report);std::cout<<report.dump()<<'\n';return 0;
}catch(const std::exception&e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
