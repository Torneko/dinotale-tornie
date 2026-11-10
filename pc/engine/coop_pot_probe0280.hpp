#pragma once
#include "coop_rooms_native0280.hpp"
#include <array>
#include <cstdio>
#include <cstdint>
#include <map>
#include <tuple>

namespace workshop::coop_pot_probe0280 {
// QA ONLY. Loader calls these hooks solely under its exact bounded room-fixture
// flag. Guest RAM is read-only; no input function, scope, or native call is made.
// Dumps are relative to that isolated profile, at the native game-thread fence.
namespace n=coop_native;namespace ids=coop_collision;namespace rn=coop_rooms_native0280;
namespace b=coop_bridge;namespace life=coop_lifecycle0280;
struct NativeGuard {unsigned primary=0;int axis=0;uint32_t request=0;unsigned air=0,mouth154=0,mouth155=0,action1f8=0;};
// Original AF8C ordering: primary before the interposed AB004 axis read, then
// AB014/AB020/AB02C/AB038/AB044. "pass" means these guards only, not admission.
inline const char* native_first(const NativeGuard& g){
 if(g.primary!=1)return "AFFC_primary";
 if(g.axis>=-39)return "AB008_axis";
 if(g.request)return "AB014_request12EC";
 if(g.air)return "AB020_airF4";
 if(g.mouth154)return "AB02C_154";
 if(g.mouth155)return "AB038_155";
 if(g.action1f8)return "AB044_1F8";
 return "pass";
}
struct Readiness {bool ready=false;unsigned owner=2;const char* first="none";};
// Mirrors RN::readiness in Source order for diagnosis, never a permission.
inline Readiness explain_readiness(uint8_t* r,uint32_t pot){
 if(!rn::enabled)return {false,2,"disabled"};
 if(rn::failed)return {false,2,"room_failed"};
 if(!b::context)return {false,2,"no_context"};
 if(b::invalidated)return {false,2,"bridge_invalidated"};
 if(!rn::allowed(rn::current_room))return {false,2,"room_not_allowed"};
 if(rn::busy())return {false,2,"room_busy"};
 if(coop_fruit0264::registry.pending())return {false,2,"fruit_pending_HOST"};
 const auto& state=life::lifecycle.snapshot();
 if(!state.bound)return {false,2,"life_unbound"};
 if(state.failed)return {false,2,"life_failed"};
 if(state.ending)return {false,2,"life_ending"};
 if(state.room!=rn::current_room)return {false,2,"life_room"};
 for(unsigned p=0;p<2;p++){
  const auto& id=state.players[p].root;const auto actual=ids::observed_identity(r,id.address);
  if(state.players[p].phase!=life::Phase::alive)return {false,p,"owner_phase"};
  if(!actual||*actual!=id)return {false,p,"owner_identity"};
  if(life::fatal(r,id.address))return {false,p,"owner_fatal"};
  const auto parent=n::word(r,id.address+0x180);
  if(parent){
   if(parent!=pot)return {false,p,"foreign_parent"};
   if(n::byte(r,id.address+0x17D)!=0)return {false,p,"child_queue"};
   if(!rn::reciprocal(r,id.address,parent))return {false,p,"parent_reciprocal"};
  }else if(n::byte(r,id.address+0x17D)!=2)return {false,p,"root_queue"};
  const auto data=id.private_data;
  if(!n::range(data,0x1550))return {false,p,"private_range_diagnostic"};
  if(n::word(r,data+0x164))return {false,p,"mouth164"};
  if(n::byte(r,data+0x155))return {false,p,"mouth155"};
  if(n::byte(r,data+0x154))return {false,p,"mouth154"};
  if(n::byte(r,data+0x1F8))return {false,p,"action1F8"};
  if(n::byte(r,id.address+0x165))return {false,p,"hidden"};
  const auto graph=n::tree(r,id.address);
  if(graph.empty())return {false,p,"graph"};
  if(!ids::registry.eggs_for_owner(r,p).empty())return {false,p,"flying_egg4002_HOST"};
  for(auto a:graph){const auto k=n::kind(r,a);if(k==0x4016||k==0x401C||k==0x401D||k==0x406C||k==0x4144||k==0x4005)return {false,p,"special_child"};}
 }
 return {true,2,"none"};
}
struct Parent {uint32_t pot=0,actor=0,private_data=0;unsigned owner=2;uint64_t pot_birth=0,actor_birth=0;};
inline bool parent(uint8_t* r,uint32_t actor,uint32_t pot,unsigned room,Parent& out){
 if(!r||room!=81||!n::actor(r,pot,0x4209)||!n::actor(r,actor,0x4001)||n::byte(r,actor+0x17D)!=0||n::word(r,actor+0x180)!=pot||!rn::reciprocal(r,actor,pot))return false;
 const auto data=n::word(r,actor+0x1F4);if(!n::range(data,0x1550))return false;
 const auto a=ids::observed_identity(r,actor),p=ids::observed_identity(r,pot);const auto owner=rn::owner(r,actor);
 // Unknown HOST birth is logged, not hidden; the reciprocal native graph is
 // sufficient for a probe, but never creates an admission/ownership receipt.
 out={pot,actor,data,owner?*owner:2,p?p->generation:0,a?a->generation:0};return true;
}
inline unsigned records=0,errors=0;inline bool parent_dumped=false,axis_dumped=false;
using Key=std::pair<unsigned,uint32_t>;
using Tuple=std::array<uint64_t,32>;
inline std::map<Key,Tuple> previous;
inline std::FILE* open_file(const char* name,const char* mode)noexcept{
 std::FILE* file=nullptr;
#ifdef _WIN32
 if(fopen_s(&file,name,mode)!=0)return nullptr;
#else
 file=std::fopen(name,mode);
#endif
 return file;
}
inline void dump(uint8_t* r,const char* name,unsigned vi,const Parent& p,bool& attempted)noexcept{
 if(attempted)return;attempted=true;
 // Never overwrite even an existing diagnostic in a reused QA profile.
 if(auto* old=open_file(name,"rb")){std::fclose(old);std::fprintf(stderr,"[CoopPotProbe] dump file=%s vi=%u actor=%08X skippedExisting=1\n",name,vi,p.actor);return;}
 auto* file=open_file(name,"wb");if(!file){std::fprintf(stderr,"[CoopPotProbe] dump file=%s vi=%u actor=%08X openFailed=1\n",name,vi,p.actor);return;}
 const auto written=std::fwrite(r,1,0x800000,file);const bool closed=std::fclose(file)==0;
 std::fprintf(stderr,"[CoopPotProbe] dump file=%s vi=%u pot=%08X actor=%08X bytes=%zu complete=%u readOnly=1\n",name,vi,p.pot,p.actor,written,unsigned(written==0x800000&&closed));
}
inline void observe(uint8_t* r,const recomp_context* c,const Parent& p,unsigned edge,int native_y,int returned_y,unsigned vi,unsigned room){
 const auto diagnostic=explain_readiness(r,p.pot);rn::m::Route route;const bool source=rn::source(r,p.pot,route);
 const auto& state=life::lifecycle.snapshot();const auto data=p.private_data;
 const NativeGuard g{n::byte(r,p.actor+0x18),returned_y,n::word(r,data+0x12EC),n::byte(r,data+0xF4),n::byte(r,data+0x154),n::byte(r,data+0x155),n::byte(r,data+0x1F8)};
 // AAFD4 skips empty entries BEFORE AAFE0 decrements remaining count.
 // Record the exact pre62684 sparse list; do not confuse count with prefix.
 const auto pot_count=ids::half(r,p.pot+0x184),slot=ids::half(r,p.actor+0x17E);unsigned used20=0,remaining=pot_count;bool native_visit=false;std::array<char,181> slots{};
 for(unsigned i=0;i<20;i++){const auto child=n::word(r,p.pot+0x188+i*4);used20+=child!=0;if(remaining&&child){--remaining;native_visit|=child==p.actor;}std::snprintf(slots.data()+i*9,slots.size()-i*9,"%08X%s",child,i==19?"":",");}
 const auto first=edge?native_first(g):!native_visit?"AFD0_not_scanned":g.primary!=1?"AFFC_primary":"axis_not_observed";
 const bool native_rejected=edge&&first!=std::string_view("pass");
 if(!edge)dump(r,"coop-pot-first-parent.rdram",vi,p,parent_dumped);
 if(native_rejected)dump(r,"coop-pot-first-axis-rejected.rdram",vi,p,axis_dumped);
 const bool scope=rn::enabled&&b::context&&state.bound;
 const bool actual_ready=diagnostic.ready&&rn::readiness(r,p.pot);
 const char* decision=!scope||!source?"original":(!actual_ready||rn::active_pot)&&!edge?"blocked":"candidate";
 const Tuple tuple{p.pot,p.actor,p.pot_birth,p.actor_birth,uint64_t(source),uint64_t(actual_ready),uint64_t(coop_fruit0264::registry.pending()),uint64_t(rn::busy()),uint64_t(rn::active_pot!=0),g.primary,uint64_t(n::byte(r,p.actor+0x30)),uint32_t(native_y),uint32_t(returned_y),g.request,g.air,g.mouth154,g.mouth155,g.action1f8,diagnostic.owner,uint64_t(state.failed),uint64_t(state.ending),uint64_t(b::invalidated),uint64_t(b::owner_stack.size()),uint64_t(state.players[0].phase),uint64_t(state.players[1].phase),uint64_t(n::byte(r,p.actor+0x165)),uint64_t(reinterpret_cast<uintptr_t>(diagnostic.first)),uint64_t(scope),pot_count,used20,slot,uint64_t(native_visit)};
 const Key key{edge,p.actor};const auto old=previous.find(key);if(old!=previous.end()&&old->second==tuple)return;
 if(records>=64)return;previous[key]=tuple;++records;
 std::fprintf(stderr,"[CoopPotProbe] edge=%s vi=%u room=%u rnRoom=%u ra=%08X pot=%08X potBirth=%llu actor=%08X birth=%llu owner=%u private=%08X parent=%08X queue=%u slot=%u source=%u route=%08X target=%u readiness=%u rnFirst=%s rnFirstOwner=%u rnDecision=%s pendingHOST=%u busy=%u activePot=%llu primary=%u sub=%u nativeY=%d returnedY=%d request12EC=%08X F4=%u b154=%u b155=%u b1F8=%u nativeFirst=%s lifePhase=%u,%u ownerDepth=%zu contextDepth=%u inputDepth=%u potCount=%u potUsed20=%u nativeVisit=%u potSlots20=%s readOnly=1\n",
  edge?"axis-return":"before-begin",vi,room,rn::current_room,c?uint32_t(c->r31):0,p.pot,(unsigned long long)p.pot_birth,p.actor,(unsigned long long)p.actor_birth,p.owner,data,n::word(r,p.actor+0x180),n::byte(r,p.actor+0x17D),ids::half(r,p.actor+0x17E),unsigned(source),source?route.address:0,source?route.destination():0,unsigned(actual_ready),diagnostic.first,diagnostic.owner,decision,unsigned(coop_fruit0264::registry.pending()),unsigned(rn::busy()),(unsigned long long)rn::active_pot,g.primary,n::byte(r,p.actor+0x30),native_y,returned_y,g.request,g.air,g.mouth154,g.mouth155,g.action1f8,first,unsigned(state.players[0].phase),unsigned(state.players[1].phase),b::owner_stack.size(),b::context?b::context->depth():0,coop_native_input::depth,pot_count,used20,unsigned(native_visit),slots.data());
}
inline void before_begin(uint8_t* r,recomp_context* c,uint32_t fn,uint32_t a,unsigned vi,unsigned room)noexcept{
 if(!r||!c||room!=81||fn!=0x805AAF8C||!n::actor(r,a,0x4209))return;
 try{for(unsigned i=0;i<20;i++){const auto child=n::word(r,a+0x188+i*4);Parent p;if(parent(r,child,a,room,p))observe(r,c,p,0,0,0,vi,room);}}
 catch(...){if(errors++<2)std::fprintf(stderr,"[CoopPotProbe] host-error edge=before-begin vi=%u guestUnchanged=1\n",vi);}
}
inline void after_axis(uint8_t* r,recomp_context* c,uint32_t candidate,int32_t native_y,int32_t returned_y,unsigned vi,unsigned room)noexcept{
 if(!r||!c||room!=81||!n::actor(r,candidate,0x4001))return;
 try{Parent p;if(parent(r,candidate,n::word(r,candidate+0x180),room,p))observe(r,c,p,1,native_y,returned_y,vi,room);}
 catch(...){if(errors++<2)std::fprintf(stderr,"[CoopPotProbe] host-error edge=axis-return vi=%u guestUnchanged=1\n",vi);}
}
} // namespace workshop::coop_pot_probe0280
