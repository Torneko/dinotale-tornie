#include "collections.hpp"
#include "collection_frustration0283.hpp"
#include "json/json.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
using namespace workshop::collections;
namespace f=workshop::collections::frustration0283;
extern "C" void func_80048E9C(uint8_t*,recomp_context*);
extern "C" void func_80048CA4(uint8_t*,recomp_context*);
static unsigned checks=0;
static void need(bool ok,const char* text){++checks;if(!ok){std::cerr<<text<<'\n';std::exit(1);}}
struct Fixture {
 static constexpr uint32_t metadata=0x80010000,objects=0x80011000,actor=0x80020000,
     resource=0x80030000,drawing=0x80031000,palette=0x80032000,root=0x800F7EA0;
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000,0xA5);uint8_t* rdram=memory.data();
 static gpr a(uint32_t value){return gpr(int32_t(value));}
 uint32_t w(uint32_t at){return uint32_t(MEM_W(0,a(at)));}
 unsigned h(uint32_t at){return MEM_HU(0,a(at));}
 void w(uint32_t at,uint32_t value){MEM_W(0,a(at))=value;}
 void h(uint32_t at,unsigned value){MEM_H(0,a(at))=uint16_t(value);}
 void mode(unsigned page=2,unsigned column=3,unsigned mode=1){MEM_B(0xF5C,a(root))=uint8_t(page);MEM_B(0xF5D,a(root))=uint8_t(column);w(root+0xCE8,mode);}
 void load(const std::filesystem::path& directory,unsigned room=14){
  std::ifstream in(directory/("room-"+std::to_string(room)+".bin"),std::ios::binary);
  std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)),{});need(data.size()>=4,"Native room fixture exists");
  auto u=[&](size_t at){return uint32_t(data.at(at))<<24|uint32_t(data.at(at+1))<<16|uint32_t(data.at(at+2))<<8|data.at(at+3);};
  const unsigned count=u(0);need(data.size()==4+count*12,"Actual ROM placement fixture length");
  for(unsigned i=0;i<f::native_capacity*4;i++)w(objects+i*4,0);
  h(metadata,count);w(metadata+4,objects);
  for(unsigned i=0;i<count;i++)for(unsigned j=0;j<3;j++)w(objects+i*16+j*4,u(4+i*12+j*4));
 }
 void birth(unsigned index){recomp_context context{};context.r4=a(actor);context.r5=w(objects+index*16);context.r6=w(objects+index*16+4);context.r7=w(objects+index*16+8);auto before=context;created(rdram,&context);need(!std::memcmp(&context,&before,sizeof(context)),"Native creation registers unchanged");}
 void art(){h(actor+12,0x406F);w(actor+0x168,resource);w(actor+0x16C,drawing);w(resource+0x88,palette);for(unsigned i=0;i<256;i++)h(palette+i*2,i*257);}
 void color(uint32_t expected){draw(rdram,actor);need(w(drawing+8)==expected,"Correct turquoise/white native palette");for(unsigned i=0;i<256;i++)need(h(palette+i*2)==i*257,"Native borrowed palette preserved");}
 void pickup(unsigned index){birth(index);w(root+0xABC,w(root+0xABC)+1);collect(rdram,actor);const unsigned score=w(root+0xAB4);collect(rdram,actor);need(w(root+0xAB4)==score,"Duplicate native callback cannot add points");}
};
static nlohmann::json json(const std::filesystem::path& p){nlohmann::json j;std::ifstream(p)>>j;return j;}
static nlohmann::json legacy(){
 auto ps=nlohmann::json::array();for(unsigned i=0;i<f::originals.size();i++)ps.push_back({14,14+i,std::bit_cast<uint32_t>(f::originals[i].x),std::bit_cast<uint32_t>(f::originals[i].y)});
 return {{"version",1},{"purchases",0},{"legacyEggs",2},{"remainderAchievement",false},{"levels",{{"3-4",{{"places",ps},{"claimedMask",0xFFF}}},{"1-1",{{"places",{{43,20,1128792064,1136918528}}},{"claimedMask",1}}}}}};
}
int main(int argc,char** argv){
 need(argc==2,"Expected disposable fixture/output directory");const auto directory=std::filesystem::path(argv[1]);
 auto file=directory/"migration.json";need(!std::filesystem::exists(file),"Never overwrite an existing test ledger");const auto old=legacy();{std::ofstream out(file);out<<old;}
 Fixture x;x.mode();x.load(directory);x.art();initialize(file);start_session(1);
 const auto before=x.memory;enter(x.rdram,14,Fixture::metadata,true);
 need(x.h(Fixture::metadata)==123 && x.w(Fixture::metadata+4)==Fixture::objects,"Native table grows115 to123 in its existing allocation");
 for(size_t i=0;i<before.size();i++)if(before[i]!=x.memory[i])need(i==((Fixture::metadata&0x7FFFFF)^2)||i==(((Fixture::metadata&0x7FFFFF)^2)+1)||(i>=(Fixture::objects&0x7FFFFF)+115*16 && i<(Fixture::objects&0x7FFFFF)+123*16),"No RAM writes outside metadata count and eight unused native records");
 const auto migrated=view();need(migrated.readable && migrated.saved && migrated.state.valid(),"Migrated ledger remains valid");
 need(migrated.state.levels[11].places.size()==20 && migrated.state.levels[11].claimed==0xFFF && migrated.state.legacyEggs==2,"Twelve claims and legacy eggs survive append migration");
 for(unsigned i=0;i<12;i++)need(migrated.state.levels[11].places[i]==Place{14,14+i,std::bit_cast<uint32_t>(f::originals[i].x),std::bit_cast<uint32_t>(f::originals[i].y)},"Old slot identities and coordinates never move");
 need(json(file)["levels"]["1-1"]==old["levels"]["1-1"],"Other level ledger remains unchanged");
 for(unsigned i=0;i<8;i++){const auto& p=migrated.state.levels[11].places[12+i];need(p.room==14 && p.index==115+i && p.x==std::bit_cast<uint32_t>(f::additions[i].x) && p.y==std::bit_cast<uint32_t>(f::additions[i].y),"Eight deterministic appended native identities");need(x.w(Fixture::objects+(115+i)*16)==0x406F000C+i && x.w(Fixture::objects+(115+i)*16+12)==0,"Standard406F record with unique native layer and empty backlink");}
 x.birth(14);x.color(0x807FD000);x.birth(115);x.color(0x807FC200);
 const auto after=x.memory;enter(x.rdram,14,Fixture::metadata,true);need(x.memory==after,"Repeated already123 entry is a complete RAM no-op");
 x.w(Fixture::root+0xAB4,0);x.w(Fixture::root+0xABC,0);
 for(unsigned i=0;i<8;i++){x.pickup(115+i);need(json(file)["levels"]["3-4"]["claimedMask"]==((1u<<(13+i))-1),"Every unique new coin is persisted immediately");}
 need(view().state.earned()==21 && view().state.levels[11].claimed==0xFFFFF && x.w(Fixture::root+0xAB4)+x.w(Fixture::root+0xABC)==24,"Exactly eight new currency and three score points per native pickup");
 const auto saved=json(file);initialize(file);start_session(1);enter(x.rdram,14,Fixture::metadata,true);
 x.w(Fixture::root+0xAB4,0);x.w(Fixture::root+0xABC,0);
 for(unsigned i=0;i<8;i++){x.birth(115+i);x.color(0x807FD000);x.pickup(115+i);}
 need(view().state.earned()==21 && json(file)==saved && x.w(Fixture::root+0xAB4)+x.w(Fixture::root+0xABC)==24,"Relaunch/replay coins are white, score normally, and cannot farm currency");
 for(unsigned room:{105u,165u,79u}){x.load(directory,room);const auto snap=x.memory;enter(x.rdram,room,Fixture::metadata,true);need(x.memory==snap && view().state.levels[11].places.size()==20 && view().state.earned()==21,"Native other-room visits neither append nor duplicate currency");}
 x.load(directory);enter(x.rdram,14,Fixture::metadata,true);need(x.h(Fixture::metadata)==123,"Reloaded native room appends its same eight records");
 const auto snapshot=x.memory;start_session(1);enter(x.rdram,14,Fixture::metadata,true);need(x.memory==snapshot && json(file)==saved,"Attempt reset/revisit preserves exact20 unique slots");
 // Exact native placement consume/restore bodies, including realFFFFFFFF
 // source words and retained actor record ownership, exercise active revisits.
 for(unsigned index:{14u,115u,112u}){
  recomp_context c{};c.r4=Fixture::a(Fixture::actor);c.r5=Fixture::a(Fixture::objects+index*16);c.r29=Fixture::a(0x807B0000);c.r31=Fixture::a(0x80000300);
  const auto packed=x.w(Fixture::objects+index*16);func_80048E9C(x.rdram,&c);
  need(x.w(Fixture::objects+index*16)==0xFFFFFFFFu && x.w(Fixture::actor+0x1D8)==Fixture::objects+index*16 && x.w(Fixture::actor+0x1DC)==packed,"Exact48E9C/48BD8/6D340 consumes native source and retains backlink");
  auto active=x.memory;enter(x.rdram,14,Fixture::metadata,true);need(x.memory==active && x.h(Fixture::metadata)==123,"Active consumed original/added/terrain record revisit does not overwrite actor ownership");
  // Restore exactly as native cleanup does, then consume again for mode exit.
  c.r4=Fixture::a(Fixture::actor);func_80048CA4(x.rdram,&c);need(x.w(Fixture::objects+index*16)==packed,"Exact48CA4 restores original native packed flags");
  c.r4=Fixture::a(Fixture::actor);c.r5=Fixture::a(Fixture::objects+index*16);func_80048E9C(x.rdram,&c);
 }
 x.mode(2,3,0);const auto consumed=x.memory;enter(x.rdram,14,Fixture::metadata,true);need(x.h(Fixture::metadata)==115 && x.w(Fixture::metadata+4)==Fixture::objects,"Cached active native room returns to115 vanilla records inStory");
 for(size_t i=0;i<consumed.size();i++)if(x.memory[i]!=consumed[i])need(i==((Fixture::metadata&0x7FFFFF)^2)||i==(((Fixture::metadata&0x7FFFFF)^2)+1),"Mode exit changes count only, preserving consumed native words and backlinks");
 x.load(directory);
 for(unsigned i=0;i<115*16;i++)need(x.memory[(Fixture::objects&0x7FFFFF)+i]==before[(Fixture::objects&0x7FFFFF)+i],"All115 original record bytes survive mode switches");
 for(unsigned test=0;test<5;test++){x.load(directory);x.mode(2,3,test==0?0:1);start_session(test==2?2:test==3?3:1);const auto snap=x.memory;enter(x.rdram,test==4?105:14,Fixture::metadata,test!=1);need(x.memory==snap && x.h(Fixture::metadata)==115,"Story/custom/Practice/Hybrid/other-room scope cannot add coins");}
 for(unsigned level=0;level<24;level++)if(level!=11){x.load(directory);x.mode(level/4,level%4);start_session(1);const auto snap=x.memory;enter(x.rdram,14,Fixture::metadata,true);need(x.memory==snap,"All23 other level selectors retain native placement bytes");}
 for(unsigned fault=0;fault<9;fault++){x.load(directory);if(fault==0)x.h(Fixture::metadata,150);if(fault==1)x.h(Fixture::metadata,114);if(fault==2)x.h(Fixture::objects+14*16,0x4070);if(fault==3)x.w(Fixture::objects+14*16+4,0);if(fault==4)x.w(Fixture::objects+82*16+8,0);if(fault==5)x.h(Fixture::objects+112*16,0x80BF);if(fault==6)x.w(Fixture::metadata+4,0x807FFF00);if(fault==7)x.w(Fixture::metadata+4,Fixture::objects+2);if(fault==8){need(f::prepare(x.rdram,Fixture::metadata,true),"Valid supplement for corrupt appended fixture");x.w(Fixture::objects+115*16+4,0);}const auto snap=x.memory;need(!f::prepare(x.rdram,Fixture::metadata,true)&&x.memory==snap,"Malformed/foreign/capacity/duplicate suffix is rejected without RAM writes");}
 // A brand-new normal profile receives the same12+8, with no numbering holes.
 const auto fresh=directory/"fresh.json";initialize(fresh);start_session(1);x.load(directory);x.mode();x.w(Fixture::root+0xAB4,0);x.w(Fixture::root+0xABC,0);enter(x.rdram,14,Fixture::metadata,true);
 for(unsigned i=0;i<12;i++)x.pickup(14+i);for(unsigned i=0;i<8;i++)x.pickup(115+i);
 need(view().state.valid()&&view().state.earned()==20&&view().state.levels[11].claimed==0xFFFFF&&x.w(Fixture::root+0xAB4)+x.w(Fixture::root+0xABC)==60,"New profile has20 unique native coins and60 pickup points");
 std::cout<<"Frustration0283: "<<checks<<" checks passed; actual ROM fixtures, native115->123 capacity, append migration,20 unique slots, immediate saves, white replay, score and mode boundaries.\n";
}
