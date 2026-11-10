#pragma once
#include "boss_battle0250.hpp"
#include "boss_native.hpp"
#include <array>
namespace workshop::boss_blargg_boot0289 {
namespace core=boss_battle0250;namespace n=boss_native;
inline constexpr uint32_t cold_controller=0x800F841C,scene_table_slot=0x800F8434;
struct Frame {
 bool marker=false,diagnostic=false;
 core::Phase phase=core::Phase::Idle;
 unsigned cold_scene=177,index=10,physical_room=177,logical_room=177,retry_epoch=0,completed=0,vi=0;
 uint32_t player=0;
};
struct Result {bool redirected=false,rejected=false;const char* reason="disabled";};
struct Snapshot {
 bool armed=false,used=false;unsigned index=10,target=177,vi=0;
 std::array<uint32_t,2> descriptor{};uint32_t table=0;
};
// QA cold-start bridge only. The real6DDCC parent stores raw52 in the global
// scene slot before6DA0C. That raw scene remains52; only6DA0C's cache key and
// selected descriptor become the actual prepared encounter source.
// No cache/tag/global/actor/gameplay memory write occurs in this adapter.
class Router final {
 Snapshot state_{};
 static bool aligned(uint32_t a,size_t bytes){return !(a&3)&&n::range(a,bytes);}
 static Result reject(const char* why){return {false,true,why};}
public:
 bool arm(bool marker,const core::Plan& plan,unsigned index,unsigned target,std::array<uint32_t,2> descriptor){
  state_={};
  if(!marker||!plan.config.blargg||!core::valid(plan)||index<1||index>8||target!=167+index||descriptor[0]!=0x01000000||(descriptor[1]>>24)!=4||(descriptor[1]&0xFFFFFF)<0x1000)return false;
  unsigned first=10;for(unsigned i=1;i<9;++i)if(plan.fights[i].family==core::Family::Blargg){first=i;break;}
  if(index!=first||!core::valid_blargg(plan.fights[index],plan.config))return false;
  state_.armed=true;state_.index=index;state_.target=target;state_.descriptor=descriptor;return true;
 }
 void disable(){state_={};}
 const Snapshot& snapshot()const{return state_;}
 // Call exactly after Runtime.enter_room(52), before ORIGINAL6DA0C.
 Result redirect(uint8_t* r,recomp_context* c,const Frame& f){
  if(!state_.armed||state_.used||!f.marker)return {};
  if(!r||!c)return reject("null_native_context");
  if(!f.diagnostic||f.phase!=core::Phase::Armed||f.cold_scene!=52||f.index!=state_.index||f.physical_room!=state_.target||f.logical_room!=19||f.retry_epoch||f.completed||f.player||!f.vi)return reject("cold_session_scope");
  // Actual687B8 passes mainstate+57C to6DDCC. The literal parent passes it
  // unchanged to6DA0C at6DDF4; its real return address is6DDFC.
  if(uint32_t(c->r4)!=cold_controller||uint32_t(c->r5)!=52||uint32_t(c->r31)!=0x8006DDFC||uint32_t(c->r29)<0x80000800||!aligned(uint32_t(c->r29)-0x200,0x240))return reject("native_parent");
  if(n::byte(r,0x800F7EA2)!=7||n::word(r,0x800F828C)||n::word(r,0x800F8290)||n::byte(r,cold_controller+0x1C)!=52||n::word(r,0x800F84CC))return reject("native_cold_scene");
  const auto level=n::word(r,0x800F8770);
  if(!aligned(level,0x84)||n::byte(r,level+0x7C)||n::byte(r,level+0x7D))return reject("native_warm_route");
  const auto table=n::word(r,scene_table_slot);
  if(!aligned(table,177*8))return reject("native_scene_table");
  for(unsigned j=0;j<2;++j)if(n::word(r,table+52*8+j*4)!=state_.descriptor[j]||n::word(r,table+state_.target*8+j*4)!=state_.descriptor[j])return reject("descriptor_mirror");
  // The source-specific cache must not preexist in this genuinely cold boot.
  auto* rdram=r;
  if(MEM_HU(8,n::base(0x800F8440))!=11)return reject("native_cache_registry");
  const auto registry=n::word(r,0x800F8450);if(!aligned(registry,44))return reject("native_cache_registry");
  for(unsigned i=0;i<11;++i){const auto entry=n::word(r,registry+i*4);if(!entry)continue;if(!aligned(entry,0xA94))return reject("native_cache_entry");const auto tag=n::byte(r,entry+1);if(tag==52||tag==state_.target)return reject("cache_already_loaded");}
  // This single owned argument is consumed by the unmodified6DA0C body:
  // getter6D6F8, descriptor index and native6DAD4 cache tag use its saved A1.
  c->r5=state_.target;state_.used=true;state_.table=table;state_.vi=f.vi;
  return {true,false,"redirected"};
 }
};
} // namespace workshop::boss_blargg_boot0289
