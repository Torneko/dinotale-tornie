#pragma once
#include "coop_native_bridge.hpp"
#include "coop_rooms0280.hpp"
#include <map>
#include <memory>
#include <string_view>
extern "C" void func_8002BE88(uint8_t*,recomp_context*);
extern "C" void func_8004816C(uint8_t*,recomp_context*);
namespace workshop::coop_rooms_native0280 {
namespace n=coop_native;namespace ids=coop_collision;namespace m=coop_rooms0280;namespace b=coop_bridge;namespace life=coop_lifecycle0280;
inline constexpr uint64_t blocked=UINT64_MAX;
inline m::Coordinator coordinator;
inline bool enabled=false,failed=false,transporting=false;
inline unsigned current_vi=0,current_room=177;
struct Admission {unsigned owner=2,vi=0;uint32_t pot=0; m::Route route{}; m::Snapshot payload{};m::Identity source{};uint64_t life_epoch=0;unsigned room=177;};
inline std::optional<Admission> admission;
struct EntryRejoin {m::Identity actor{},pot{};uint64_t epoch=0,outer=0,detach=0;unsigned room=0,vi=0;bool queued=false;};
inline std::optional<EntryRejoin> entry_rejoin;
// A real4816C->48060 detach may also originate at an ordinary support. The
// pending scheduler still uses4001's canonical slot; only this known P2 gets
// a one-use generic rejoin after the original443EC and native queue drain.
struct SupportRejoin {m::Identity actor{},parent{};uint64_t epoch=0,load=0,pending_call=0;unsigned room=0,vi=0;uint32_t stack=0,outer_ra=0,manager=0,table=0;unsigned capacity=0;bool queued=false;unsigned admission_vi=0;};
inline std::optional<SupportRejoin> support_rejoin;
inline unsigned support_diagnostics=0;
inline std::array<bool,8> gate_diagnostics{};
inline unsigned transaction_diagnostics=0,fence_diagnostics=0,last_fence_diagnostic=0;
inline std::array<bool,3> pool_diagnostics{};
inline uint32_t entry_overlay=0,warm_overlay=0;inline uint64_t room_load=0,next_ticket=0,active_pot=0;
struct Call {uint32_t fn=0,a=0,arg=0,third=0,ra=0;uint64_t epoch=0;std::unique_ptr<b::Scope> scope;};
inline std::map<uint64_t,Call> calls;
inline std::array<std::optional<m::Permit>,2> permits{};
inline std::array<std::optional<m::Registration>,2> warm_gates{};inline uint32_t entering_manager=0;
inline std::array<bool,2> warm_returned{},registered{},arrival_reported{};
inline void fail(const char* why){if(!failed)std::fprintf(stderr,"[CoopRooms] failed reason=%s vi=%u epoch=%llu\n",why,current_vi,(unsigned long long)coordinator.epoch());failed=true;b::invalidated=true;}
inline void host_error(){
 fail("host_transaction_exception");
 // Tickets nest in increasing order; release native owner/pad scopes LIFO.
 while(!calls.empty()){auto it=std::prev(calls.end());it->second.scope.reset();calls.erase(it);}active_pot=0;
}
inline bool busy(){return enabled&&(failed||admission.has_value()||transporting||coordinator.busy());}
inline bool input_blocked(unsigned owner){return owner<2&&busy();}
inline bool retaining_carried(){return enabled&&coordinator.carried()!=0&&coordinator.phase()!=m::Phase::complete&&coordinator.phase()!=m::Phase::disabled;}
inline bool attempt_quiescent(){return calls.empty()&&!active_pot&&!busy()&&!retaining_carried();}
inline void configure(bool on){gate_diagnostics={};pool_diagnostics={};transaction_diagnostics=fence_diagnostics=last_fence_diagnostic=0;admission.reset();entry_rejoin.reset();support_rejoin.reset();support_diagnostics=0;enabled=on;failed=transporting=false;current_vi=0;current_room=177;entry_overlay=warm_overlay=0;room_load=active_pot=0;calls.clear();permits={};warm_gates={};entering_manager=0;warm_returned=registered=arrival_reported={};coordinator.configure(on);}
inline m::Identity identity(const ids::Identity& id){return {id.address,id.private_data,id.generation,id.kind};}
inline std::optional<m::Identity> observed(uint8_t* r,uint32_t a){const auto id=ids::observed_identity(r,a);return id?std::optional<m::Identity>(identity(*id)):std::nullopt;}
inline std::optional<unsigned> owner(uint8_t* r,uint32_t a){const auto& state=life::lifecycle.snapshot();for(unsigned p=0;p<2;p++){const auto now=ids::observed_identity(r,a);if(state.bound&&state.players[p].root.address==a&&now&&*now==state.players[p].root)return p;}return {};}
inline bool allowed(unsigned room){const auto& s=life::lifecycle.snapshot();return room==43||(room==81&&s.enabled&&s.bound&&!s.failed&&s.room==81);}
inline bool route(uint8_t* r,uint32_t address,unsigned from,m::Route& out){if(!n::range(address,24))return false;m::Route value;value.address=address;for(unsigned i=0;i<24;i++)value.bytes[i]=uint8_t(n::byte(r,address+i));if(!m::valid(value,from))return false;out=value;return true;}
inline bool source(uint8_t* r,uint32_t a,m::Route& out){
 const auto id=observed(r,a);if(!id||id->kind==0x4001||!n::range(id->private_data,0x278)||!n::range(entry_overlay,0xB30))return false;
 const auto resource=n::word(r,a+0x168);return n::range(resource,0x48)&&ids::half(r,resource+0x3C)==id->kind&&n::word(r,resource+0x40)==entry_overlay&&route(r,n::word(r,id->private_data+0x260),current_room,out);
}
// Read-only, bounded bootstrap diagnostics. No admission or input decision is
// changed by these observations; one line per hook/fence and bind phase.
inline void diagnose_gate(uint8_t* r,const recomp_context* c,uint32_t fn,uint32_t a,uint32_t arg,bool traced,uint32_t section144=0){
 if(!enabled||(fn!=0x805AAF8C&&fn!=0x8002F7AC))return;
 const bool bound=life::lifecycle.snapshot().bound;const unsigned slot=(fn==0x8002F7AC?4:0)+(bound?2:0)+(traced?1:0);if(gate_diagnostics[slot])return;gate_diagnostics[slot]=true;
 const auto pot=fn==0x805AAF8C?a:arg;const auto id=observed(r,pot);const auto data=n::actor(r,pot)?n::word(r,pot+0x1F4):0,res=n::actor(r,pot)?n::word(r,pot+0x168):0;
 const uint32_t base=n::range(res,0x48)?n::word(r,res+0x40):0,rt=n::range(data,0x278)?n::word(r,data+0x260):0;const unsigned resource_kind=n::range(res,0x48)?ids::half(r,res+0x3C):0;
 m::Route route_value;const bool route_ok=route(r,rt,current_room,route_value);const char* why=!id?"unknown_birth":id->kind==0x4001?"player_source":!n::range(id->private_data,0x278)?"source_private":!n::range(entry_overlay,0xB30)?"entry_overlay":!n::range(res,0x48)?"resource":resource_kind!=id->kind?"resource_kind":base!=entry_overlay?"resource_base":!route_ok?"route":"ok";
 std::fprintf(stderr,"[CoopRooms] gate fn=%08X edge=%s vi=%u room=%u ra=%08X context=%u lifeBound=%u pot=%08X birthKnown=%u private=%08X resource=%08X resourceKind=%04X resourceBase=%08X entryOverlay=%08X section144=%08X route=%08X routeOK=%u sourceWhy=%s activePot=%llu nativeY=%d\n",fn,traced?"trace":"begin",current_vi,current_room,uint32_t(c->r31),unsigned(bool(b::context)),unsigned(bound),pot,unsigned(bool(id)),data,res,resource_kind,base,entry_overlay,section144,rt,unsigned(route_ok),why,(unsigned long long)active_pot,int(int16_t(ids::half(r,0x800F8980))));
}
inline bool reciprocal(uint8_t* r,uint32_t a,uint32_t parent){
 if(!n::actor(r,parent))return false;const auto slot=ids::half(r,a+0x17E),count=ids::half(r,parent+0x184);if(slot>=20||count>20||n::word(r,parent+0x188+slot*4)!=a)return false;
 unsigned used=0,matches=0;for(unsigned i=0;i<20;i++){const auto child=n::word(r,parent+0x188+i*4);used+=child!=0;matches+=child==a;}return used==count&&matches==1;
}
inline bool player(uint8_t* r,unsigned p,m::Player& out){
 if(!b::context||p>1)return false;const auto& s=life::lifecycle.snapshot();const auto root=s.players[p].root;const auto now=ids::observed_identity(r,root.address);
 if(!now||*now!=root||root.kind!=0x4001||!n::range(root.private_data,0x1550))return false;
 const auto parent=n::word(r,root.address+0x180);if(parent&&!reciprocal(r,root.address,parent))return false;
 m::Player value;value.actor=identity(root);value.color=s.players[p].color;value.hp=b::context->player(p).health;
 if(!b::owner_stack.empty()&&b::owner_stack.back()==p)value.hp=n::word(r,coop::native_health+coop::native_alias(value.color)*4);
 unsigned occupied=0;
 for(unsigned i=0;i<20;i++){const auto a=n::word(r,root.address+0x188+i*4);if(!a)continue;const auto id=observed(r,a);if(!id||!reciprocal(r,a,root.address))return false;value.children[i]=*id;++occupied;}
 if(occupied!=ids::half(r,root.address+0x184))return false;
 value.count=n::byte(r,root.private_data+0x12C8);const auto head=ids::half(r,root.private_data+0x12BE);if(value.count>6||head>=8)return false;
 for(unsigned i=0;i<value.count;i++){auto& egg=value.fifo[i];const auto a=n::word(r,root.private_data+0x11FC+((head+i)&7)*24);const auto id=observed(r,a);if(!id)return false;egg.actor=*id;egg.palette=n::word(r,a+0x1EC);for(unsigned j=0;j<3;j++)egg.saved_scale[j]=n::word(r,a+0x78+j*4);}
 if(!m::valid(value))return false;out=value;return true;
}
inline bool shared(uint8_t* r,m::Shared& out){m::Shared v;for(unsigned i=0;i<10;i++)v.scores[i]=n::word(r,0x800F8950+i*4);v.count=n::word(r,0x800F89D0);v.backup_count=n::word(r,0x800F8A4C);v.ending=n::word(r,0x800F8AC8);for(unsigned i=0;i<30;i++){v.fruits[i]=n::word(r,0x800F89D4+i*4);v.backup[i]=n::word(r,0x800F8A50+i*4);v.half_fruits[i]=ids::half(r,0x800F8AD4+i*2);}if(!m::valid(v))return false;out=v;return true;}
inline bool snapshot(uint8_t* r,m::Snapshot& out){m::Snapshot value;if(!player(r,0,value.players[0])||!player(r,1,value.players[1])||!shared(r,value.shared)||!m::valid(value))return false;out=value;return true;}
inline uint64_t signature(const m::Snapshot& p){
 uint64_t h=1469598103934665603ull;auto mix=[&](uint64_t v){h=(h^v)*1099511628211ull;};auto id=[&](const m::Identity& a){mix(a.address);mix(a.private_data);mix(a.birth);mix(a.kind);};
 for(const auto& owner:p.players){id(owner.actor);mix(owner.color);mix(owner.hp);mix(owner.count);for(const auto& a:owner.children)id(a);for(const auto& egg:owner.fifo){id(egg.actor);mix(egg.palette);for(auto v:egg.saved_scale)mix(v);}}
 for(auto v:p.shared.scores)mix(v);mix(p.shared.count);mix(p.shared.backup_count);mix(p.shared.ending);for(auto v:p.shared.fruits)mix(v);for(auto v:p.shared.backup)mix(v);for(auto v:p.shared.half_fruits)mix(v);return h;
}
inline bool readiness(uint8_t* r,uint32_t source_pot){
 if(!enabled||failed||!b::context||b::invalidated||!allowed(current_room)||busy()||coop_fruit0264::registry.pending())return false;
 const auto& s=life::lifecycle.snapshot();if(!s.bound||s.failed||s.ending||s.room!=current_room)return false;
 for(unsigned p=0;p<2;p++){
  const auto& id=s.players[p].root;const auto actual=ids::observed_identity(r,id.address);if(s.players[p].phase!=life::Phase::alive||!actual||*actual!=id||life::fatal(r,id.address))return false;
  const auto parent=n::word(r,id.address+0x180);if(parent){if(parent!=source_pot||n::byte(r,id.address+0x17D)!=0||!reciprocal(r,id.address,parent))return false;}else if(n::byte(r,id.address+0x17D)!=2)return false;
  const auto data=id.private_data;if(n::word(r,data+0x164)||n::byte(r,data+0x155)||n::byte(r,data+0x154)||n::byte(r,data+0x1F8)||n::byte(r,s.players[p].root.address+0x165))return false;
  const auto graph=n::tree(r,s.players[p].root.address);if(graph.empty()||!ids::registry.eggs_for_owner(r,p).empty())return false;
  for(auto a:graph){const auto k=n::kind(r,a);if(k==0x4016||k==0x401C||k==0x401D||k==0x406C||k==0x4144||k==0x4005)return false;}
 }
 return true;
}
inline bool registered_as(uint8_t* r,uint32_t a,unsigned& slot){const auto manager=n::word(r,0x800F8810);if(!n::range(manager,0x14))return false;const auto table=n::word(r,manager+0x10);const auto cap=ids::half(r,manager+8);if(cap<7||cap>150||!n::range(table,cap*4))return false;unsigned count=0;for(unsigned i=0;i<cap;i++)if(n::word(r,table+i*4)==a){slot=i;++count;}return count==1;}
inline uint32_t canonical_player(uint8_t* r){const auto manager=n::word(r,0x800F8810);if(!n::range(manager,0x14)||ids::half(r,manager+8)<6)return UINT32_MAX;const auto table=n::word(r,manager+0x10);return n::range(table,24)?n::word(r,table+20):UINT32_MAX;}
inline int generic_slot(uint8_t* r,uint32_t canonical_expected=0){const auto manager=n::word(r,0x800F8810);if(!n::range(manager,0x14))return -1;const auto table=n::word(r,manager+0x10);const auto cap=ids::half(r,manager+8);if(cap<7||cap>150||!n::range(table,cap*4))return -1;const auto canonical=n::word(r,table+20),expected=canonical_expected?canonical_expected:coordinator.departure().payload.players[0].actor.address;if(canonical>1&&canonical!=expected)return -1;for(unsigned i=6;i<std::min<unsigned>(cap,124);i++)if(!n::word(r,table+i*4))return int(i);return -1;}
inline bool generic_register(uint8_t* r,recomp_context* c,uint32_t a,unsigned expected,uint32_t canonical_expected=0){
 if(generic_slot(r,canonical_expected)!=int(expected))return false;n::ScopedContext regs(c);c->r4=n::base(a);func_8005FDC4(r,c);c->r4=n::base(n::word(r,0x800F8810));c->r5=0;func_80043A88(r,c);if(uint32_t(c->r2)!=expected)return false;c->r4=n::base(n::word(r,0x800F8810));c->r5=expected;c->r6=n::base(a);func_80043C0C(r,c);unsigned slot=150;if(uint32_t(c->r2)!=1||!registered_as(r,a,slot)||slot!=expected)return false;n::byte(r,a+0x17D,2);return true;
}
inline bool pending_rejoin(uint8_t* r,uint32_t a){
 const uint32_t header=0x800F8840;const auto cap=ids::half(r,header+8),used=ids::half(r,header+6),slot=ids::half(r,a+0x17E);const auto table=n::word(r,header+0x10);
 if(!cap||cap>150||used>cap||slot>=cap||!n::range(table,cap*4)||n::word(r,table+slot*4)!=a||n::byte(r,a+0x17D)!=4||n::word(r,a+0x180))return false;
 unsigned found=0,count=0;for(unsigned j=0;j<cap;j++){const auto child=n::word(r,table+j*4);found+=child==a;count+=child!=0;}return found==1&&count==used;
}
inline bool entry_identity(uint8_t* r){
 if(!entry_rejoin)return false;const auto& e=*entry_rejoin;const auto& state=life::lifecycle.snapshot();const auto actor=observed(r,e.actor.address),pot=observed(r,e.pot.address);
 return state.bound&&!state.failed&&!state.ending&&state.epoch==e.epoch&&state.room==e.room&&current_room==e.room&&state.players[1].phase==life::Phase::alive&&identity(state.players[1].root)==e.actor&&actor&&*actor==e.actor&&pot&&*pot==e.pot&&current_vi>=e.vi&&current_vi-e.vi<=m::maximum_duration;
}
// The admitted P1 can still leave P2 standing on the closing entry pot.
// That real support detach precedes departure; it may rejoin only under the
// same exact admission, never while the room coordinator is transporting.
inline bool admission_follower_support(uint8_t* r,const m::Identity& parent,unsigned admission_vi){
 if(!admission_vi||!admission||admission->vi!=admission_vi||admission->owner!=0||transporting||coordinator.busy()||entry_rejoin||active_pot)return false;
 const auto& a=*admission;const auto& state=life::lifecycle.snapshot();const auto source_now=observed(r,parent.address);m::Route route_now;
 if(a.source!=parent||(parent.kind!=0x4208&&parent.kind!=0x4209)||a.pot!=parent.address||!source_now||*source_now!=parent||a.life_epoch!=state.epoch||a.room!=state.room||a.room!=current_room||(a.room!=43&&a.room!=81)||current_vi<a.vi||current_vi-a.vi>m::maximum_duration||!source(r,a.pot,route_now)||route_now!=a.route)return false;
 for(unsigned p=0;p<2;p++){
  const auto& id=state.players[p].root;const auto now=observed(r,id.address);
  if(state.players[p].phase!=life::Phase::alive||a.payload.players[p].actor!=identity(id)||!now||*now!=identity(id)||life::fatal(r,id.address))return false;
 }
 return true;
}
inline bool support_identity(uint8_t* r){
 if(!support_rejoin||!enabled||failed||!b::context||b::invalidated||n::constructing||n::cleaning||entry_rejoin)return false;
 const auto& e=*support_rejoin;const auto& state=life::lifecycle.snapshot();const auto actor=observed(r,e.actor.address);
 if(e.admission_vi?!admission_follower_support(r,e.parent,e.admission_vi):busy())return false;
 if(!state.enabled||!state.bound||state.failed||state.ending||state.epoch!=e.epoch||state.room!=e.room||current_room!=e.room||(e.room!=43&&e.room!=81)||room_load!=e.load||state.players[1].phase!=life::Phase::alive||identity(state.players[1].root)!=e.actor||!actor||*actor!=e.actor||e.actor.address!=b::bound_secondary||life::fatal(r,e.actor.address)||current_vi<e.vi||current_vi-e.vi>2)return false;
 return n::word(r,0x800F8810)==e.manager&&n::range(e.manager,0x14)&&n::word(r,e.manager+0x10)==e.table&&ids::half(r,e.manager+8)==e.capacity&&n::range(e.table,e.capacity*4);
}
inline bool main_absent(uint8_t* r,uint32_t a){
 const auto manager=n::word(r,0x800F8810);if(!n::range(manager,0x14))return false;const auto table=n::word(r,manager+0x10);const auto cap=ids::half(r,manager+8);if(cap<7||cap>150||!n::range(table,cap*4))return false;
 for(unsigned i=0;i<cap;i++)if(n::word(r,table+i*4)==a)return false;return true;
}
inline void observe_support_detach(uint8_t* r,const recomp_context* c){
 if(!enabled||failed||!b::context||b::invalidated||n::constructing||n::cleaning||entry_rejoin||active_pot||uint32_t(c->r31)!=0x8004819C)return;
 const auto a=uint32_t(c->r4),sp=uint32_t(c->r29);const auto p=owner(r,a);if(!p||*p!=1||a!=b::bound_secondary)return;
 const auto& state=life::lifecycle.snapshot();if(!state.enabled||!state.bound||state.failed||state.ending||state.room!=current_room||(current_room!=43&&current_room!=81)||state.players[1].phase!=life::Phase::alive||!life::lifecycle.alive(r,1)||!n::range(sp,0x1C)||n::word(r,sp+0x18)!=a)return;
 const auto parent=n::word(r,a+0x180);const auto parent_id=observed(r,parent),actor_id=observed(r,a);
 const auto manager=n::word(r,0x800F8810);if(!parent_id||!actor_id||parent==a||!reciprocal(r,a,parent)||n::byte(r,a+0x17D)!=0||!main_absent(r,a)||!n::range(manager,0x14))return;
 const auto table=n::word(r,manager+0x10);const unsigned cap=ids::half(r,manager+8);if(cap<7||cap>150||!n::range(table,cap*4))return;
 const unsigned admission_vi=admission?admission->vi:0;if(busy()&&!admission_follower_support(r,*parent_id,admission_vi))return;
 if(support_rejoin){fail("native_support_detach_reentrant");return;}
 support_rejoin=SupportRejoin{*actor_id,*parent_id,state.epoch,room_load,0,current_room,current_vi,sp,n::word(r,sp+0x14),manager,table,cap,false,admission_vi};
}
inline bool carry_linked(uint8_t* r,uint32_t list,uint32_t a){const auto header=list+4;if(!n::range(list,0x84))return false;const auto cap=ids::half(r,header+8),used=ids::half(r,header+6);const auto table=n::word(r,header+0x10);if(!cap||cap>150||used>cap||!n::range(table,cap*4)||n::word(r,list+0x78)!=list)return false;unsigned count=0;for(unsigned i=0;i<cap;i++)count+=n::word(r,table+i*4)==a;unsigned slot=150;return count==1&&!registered_as(r,a,slot)&&!n::word(r,a+0x180);}
inline uint64_t make_call(uint32_t fn,uint32_t a,uint32_t arg,uint32_t third,uint32_t ra){if(calls.size()>=32||next_ticket>=UINT64_MAX-1){fail("native_call_capacity");return blocked;}const auto ticket=++next_ticket;calls.emplace(ticket,Call{fn,a,arg,third,ra,coordinator.epoch(),{}});return ticket;}
inline void diagnose_transaction(uint32_t fn,uint64_t token,const char* edge){
 if(transaction_diagnostics>=16)return;++transaction_diagnostics;
 std::fprintf(stderr,"[CoopRooms] host-transaction fn=%08X token=%llu edge=%s vi=%u calls=%zu activePot=%llu ownerDepth=%zu owner=%u contextDepth=%u admission=%u\n",fn,(unsigned long long)token,edge,current_vi,calls.size(),(unsigned long long)active_pot,b::owner_stack.size(),b::owner_stack.empty()?2:b::owner_stack.back(),b::context?b::context->depth():0,unsigned(bool(admission)));
}
inline void diagnose_fence(){
 if(!enabled||!b::context||(!busy()&&calls.empty()&&b::owner_stack.empty())||fence_diagnostics>=12)return;
 if(fence_diagnostics>=4&&current_vi-last_fence_diagnostic<120)return;last_fence_diagnostic=current_vi;++fence_diagnostics;
 std::fprintf(stderr,"[CoopRooms] host-fence vi=%u calls=%zu activePot=%llu ownerDepth=%zu owner=%u contextDepth=%u admission=%u busy=%u",current_vi,calls.size(),(unsigned long long)active_pot,b::owner_stack.size(),b::owner_stack.empty()?2:b::owner_stack.back(),b::context->depth(),unsigned(bool(admission)),unsigned(busy()));
 unsigned count=0;for(const auto& [token,call]:calls){if(++count>4)break;std::fprintf(stderr," ticket%u=%llu fn%u=%08X actor%u=%08X scope%u=%u",count,(unsigned long long)token,count,call.fn,count,call.a,count,unsigned(bool(call.scope)));}std::fputc('\n',stderr);
}
inline void diagnose_pools(uint8_t* r,unsigned point,uint32_t actor){
 if(point>=pool_diagnostics.size()||pool_diagnostics[point])return;pool_diagnostics[point]=true;namespace e=coop_egg_pools;const auto& value=e::snapshot();const auto current=n::word(r,e::manager_global);const bool current_valid=n::range(current,0x18);
 std::fprintf(stderr,"[CoopRooms] pool-scope point=%s vi=%u room=%u actor=%08X enabled=%u installed=%u invalidated=%u depth=%u allocation=%08X heap=%u matches=%u managerMatch=%u oldManager=%08X oldTable=%08X oldCap=%u currentManager=%08X currentTable=%08X currentCap=%u layout=%u,%u registrations=%d,%d free=%u,%u globals=%08X,%08X\n",point==0?"6DD34":point==1?"2BEB0":"ABBE8",current_vi,current_room,actor,unsigned(value.enabled),unsigned(value.installed),unsigned(value.invalidated),value.depth,value.allocation,unsigned(e::heap_block(r)),unsigned(e::matches(r)),unsigned(e::manager_matches(r)),value.manager,value.table,value.manager_capacity,current,current_valid?n::word(r,current+0x10):0,current_valid?ids::half(r,current+8):0,unsigned(e::layout(r,0)),unsigned(e::layout(r,1)),e::registration(r,e::pool(1,0)),e::registration(r,e::pool(1,1)),e::free_slots(r,e::pool(1,0)),e::free_slots(r,e::pool(1,1)),n::word(r,e::globals[0]),n::word(r,e::globals[1]));
}
inline uint64_t begin(uint8_t* r,recomp_context* c,uint32_t fn,uint32_t a,uint32_t arg,uint32_t third,unsigned vi,unsigned room){
 current_vi=vi;current_room=room;diagnose_gate(r,c,fn,a,arg,false);if(!enabled||!b::context||!life::lifecycle.snapshot().bound)return 0;
 if(fn==0x805AAF8C){m::Route rt;if(!source(r,a,rt))return 0;if(!readiness(r,a)||active_pot)return blocked;const auto ticket=make_call(fn,a,arg,third,uint32_t(c->r31));active_pot=ticket;return ticket;}
 if((fn==0x805AB308||fn==0x805AB4D8)&&admission&&a==admission->pot){
  const auto& state=life::lifecycle.snapshot();const auto pot_id=observed(r,a);const auto p=admission->owner;const auto actor=p<2?observed(r,admission->payload.players[p].actor.address):std::nullopt;m::Route rt;
  const bool callback=fn==0x805AB308?n::word(r,a+0x20)==entry_overlay+0x3C8&&n::byte(r,a+0x18)==1&&n::byte(r,a+0x30)==1&&uint32_t(c->r31)==0x80048A74:n::word(r,a+0x20)==entry_overlay+0x714&&n::byte(r,a+0x18)==1&&(n::byte(r,a+0x30)==2||n::byte(r,a+0x30)==3)&&uint32_t(c->r31)==entry_overlay+0x744;
  if(!pot_id||*pot_id!=admission->source||!source(r,a,rt)||rt!=admission->route||state.epoch!=admission->life_epoch||state.room!=admission->room||room!=admission->room||state.failed||state.ending||p>=2||state.players[p].phase!=life::Phase::alive||!actor||*actor!=admission->payload.players[p].actor||!callback){fail("native_pot_owner_scope");return blocked;}
  const auto ticket=make_call(fn,a,arg,third,uint32_t(c->r31));calls.at(ticket).scope=std::make_unique<b::Scope>(r,c,p);return ticket;
 }
 if(fn==0x80061170){
  if(!active_pot||uint32_t(c->r31)!=entry_overlay+0x118)return 0;const auto outer=calls.find(active_pot);const auto p=owner(r,a);
  if(outer==calls.end()||outer->second.fn!=0x805AAF8C||!p||*p!=1)return 0;
  const auto pot=outer->second.a;const auto actor_id=observed(r,a),pot_id=observed(r,pot);m::Route rt;
  if(entry_rejoin||!actor_id||!pot_id||!source(r,pot,rt)||n::word(r,a+0x180)!=pot||!reciprocal(r,a,pot)||n::byte(r,a+0x17D)!=0){fail("native_entry_detach_identity");return blocked;}
  if(!outer->second.scope)outer->second.scope=std::make_unique<b::Scope>(r,c,1);
  const auto ticket=make_call(fn,a,arg,third,uint32_t(c->r31));entry_rejoin=EntryRejoin{*actor_id,*pot_id,life::lifecycle.snapshot().epoch,active_pot,ticket,room,vi,false};return ticket;
 }
 if(fn==0x8002F7AC){
  if(!active_pot)return 0;auto it=calls.find(active_pot);const auto p=owner(r,a);if(it==calls.end()||it->second.fn!=0x805AAF8C||it->second.a!=arg||!p){fail("pot_candidate_scope");return blocked;}
  diagnose_transaction(fn,active_pot,"before-owner");if(!it->second.scope)it->second.scope=std::make_unique<b::Scope>(r,c,*p);const auto ticket=make_call(fn,a,arg,third,uint32_t(c->r31));diagnose_transaction(fn,ticket,"begin-return");return ticket;
 }
 if(fn==0x80030B7C){
  const auto p=owner(r,a);if(!p||uint32_t(c->r31)!=m::depart_caller)return 0;
  m::Departure departure;departure.room=room;departure.leader=*p;departure.vi=vi;departure.caller=uint32_t(c->r31);departure.scoped_player=a;departure.canonical_player=b::bound_primary;
  const auto pot=n::word(r,life::lifecycle.snapshot().players[*p].root.private_data+0x1C0);const auto source_id=observed(r,pot);if(!source_id||!source(r,pot,departure.route)||departure.route.address!=arg||!snapshot(r,departure.payload)){fail("native_departure_payload");return blocked;}
  if(!admission||admission->owner!=*p||admission->pot!=pot||admission->route!=departure.route||admission->payload!=departure.payload){
   std::fprintf(stderr,"[CoopRooms] admission-mismatch vi=%u hasAdmission=%u actualOwner=%u expectedOwner=%u actualPot=%08X expectedPot=%08X ownerMatch=%u potMatch=%u routeMatch=%u payloadMatch=%u actualSig=%016llX expectedSig=%016llX\n",vi,unsigned(bool(admission)),*p,admission?admission->owner:2,pot,admission?admission->pot:0,unsigned(admission&&admission->owner==*p),unsigned(admission&&admission->pot==pot),unsigned(admission&&admission->route==departure.route),unsigned(admission&&admission->payload==departure.payload),(unsigned long long)signature(departure.payload),(unsigned long long)(admission?signature(admission->payload):0));
   fail(!admission?"native_admission_missing":admission->owner!=*p?"native_admission_owner_changed":admission->pot!=pot?"native_admission_pot_changed":admission->route!=departure.route?"native_admission_route_changed":"native_admission_payload_changed");return blocked;
  }
  departure.source=*source_id;departure.source_from_player_1c0=pot;departure.route_from_source_260=n::word(r,source_id->private_data+0x260);departure.readiness.mouth_idle={true,true};
  if(coordinator.begin(departure)!=m::Decision::accepted){fail("native_departure_not_preflighted");return blocked;}
  const auto ticket=make_call(fn,a,arg,third,uint32_t(c->r31));calls.at(ticket).scope=std::make_unique<b::Scope>(r,c,*p);permits={};warm_gates={};warm_returned=registered=arrival_reported={};
  std::fprintf(stderr,"[CoopRooms] begin epoch=%llu leader=%u room=%u target=%u p0=%08X private0=%08X birth0=%llu p1=%08X private1=%08X birth1=%llu hp=%u,%u fifo=%u,%u ring=%u payloadSig=%016llX vi=%u fn=80030B7C ra=80030BC4 nativeObservation=1\n",(unsigned long long)coordinator.epoch(),*p,room,departure.route.destination(),departure.payload.players[0].actor.address,departure.payload.players[0].actor.private_data,(unsigned long long)departure.payload.players[0].actor.birth,departure.payload.players[1].actor.address,departure.payload.players[1].actor.private_data,(unsigned long long)departure.payload.players[1].actor.birth,departure.payload.players[0].hp,departure.payload.players[1].hp,departure.payload.players[0].count,departure.payload.players[1].count,departure.payload.shared.count,(unsigned long long)signature(departure.payload),vi);
  const unsigned follower=1-*p;const auto f=departure.payload.players[follower].actor.address;
  {b::Scope follower_scope(r,c,follower);n::ScopedContext regs(c);const auto parent=n::word(r,f+0x180);if(parent){if(!reciprocal(r,f,parent))fail("follower_parent_changed");else {c->r4=n::base(f);func_8004816C(r,c);}}if(!failed){c->r4=n::base(f);func_8002BE88(r,c);}}
  if(failed||coordinator.carried()!=(1u<<follower)){fail("native_follower_not_carried");calls.erase(ticket);return blocked;}
  coop_fruit0264::registry.frame(r,vi,room,false,0,0);return ticket;
 }
 if(fn==0x800443EC&&entry_rejoin&&a==entry_rejoin->actor.address&&uint32_t(c->r31)==0x800481A4){
  const auto detach=calls.find(entry_rejoin->detach);
  if(!entry_identity(r)||entry_rejoin->queued||active_pot!=entry_rejoin->outer||detach==calls.end()||detach->second.fn!=0x80061170||detach->second.a!=a||n::word(r,a+0x180)){fail("native_entry_pending_chain");return blocked;}
  return make_call(fn,a,arg,third,uint32_t(c->r31));
 }
 if(fn==0x800440E4&&entry_rejoin&&a==entry_rejoin->actor.address&&uint32_t(c->r31)==0x800444B0){
  const auto& e=*entry_rejoin;unsigned old_slot=150;
  if(!entry_identity(r)||!e.queued||!admission||admission->owner!=1||admission->pot!=e.pot.address||admission->payload.players[1].actor!=e.actor||n::byte(r,a+0x17D)!=0||ids::half(r,a+0x17E)!=150||n::word(r,a+0x180)||registered_as(r,a,old_slot)){fail("native_entry_rejoin_permission");return blocked;}
  const auto slot=generic_slot(r,b::bound_primary);if(slot<0){fail("native_entry_generic_capacity");return blocked;}
  {b::Scope scope(r,c,1);if(!generic_register(r,c,a,unsigned(slot),b::bound_primary)){fail("native_entry_generic_registration");return blocked;}}
  std::fprintf(stderr,"[CoopRooms] entry-rejoined owner=1 actor=%08X birth=%llu slot=%u canonical=%08X vi=%u fn=800440E4 ra=800444B0 nativeObservation=1\n",a,(unsigned long long)e.actor.birth,unsigned(slot),canonical_player(r),vi);entry_rejoin.reset();return blocked;
 }
 if(fn==0x800443EC&&support_rejoin&&a==support_rejoin->actor.address&&uint32_t(c->r31)==0x800481A4){
  const auto& e=*support_rejoin;const auto sp=uint32_t(c->r29);
  if(!support_identity(r)||e.queued||e.pending_call||sp!=e.stack||!n::range(sp,0x1C)||n::word(r,sp+0x18)!=a||n::word(r,sp+0x14)!=e.outer_ra||n::word(r,a+0x180)||ids::half(r,a+0x17E)!=150||n::byte(r,a+0x17D)!=0||!main_absent(r,a)){fail("native_support_pending_chain");return blocked;}
  const auto ticket=make_call(fn,a,arg,third,uint32_t(c->r31));support_rejoin->pending_call=ticket;return ticket;
 }
 if(fn==0x800440E4&&support_rejoin&&a==support_rejoin->actor.address&&uint32_t(c->r31)==0x800444B0){
  const auto e=*support_rejoin;
  if(!support_identity(r)||!e.queued||e.pending_call||n::byte(r,a+0x17D)!=0||ids::half(r,a+0x17E)!=150||n::word(r,a+0x180)||!main_absent(r,a)){fail("native_support_rejoin_permission");return blocked;}
  const auto slot=generic_slot(r,b::bound_primary);if(slot<0){fail("native_support_generic_capacity");return blocked;}
  {b::Scope scope(r,c,1);if(!generic_register(r,c,a,unsigned(slot),b::bound_primary)){fail("native_support_generic_registration");return blocked;}}
  if(support_diagnostics++<8)std::fprintf(stderr,"[CoopRooms] support-rejoined owner=1 actor=%08X birth=%llu parent=%08X parentKind=%04X parentBirth=%llu epoch=%llu room=%u slot=%u canonical=%08X detachVI=%u vi=%u fn=800440E4 ra=800444B0 nativeObservation=1 admissionVI=%u\n",a,(unsigned long long)e.actor.birth,e.parent.address,e.parent.kind,(unsigned long long)e.parent.birth,(unsigned long long)e.epoch,room,unsigned(slot),canonical_player(r),e.vi,vi,e.admission_vi);
  support_rejoin.reset();return blocked;
 }
 if(!coordinator.busy()||failed)return 0;
 if(fn==0x8005D1C8){const auto p=owner(r,arg);if(!p)return 0;return make_call(fn,a,arg,third,uint32_t(c->r31));}
 if(fn==0x8002BEB0){const auto p=owner(r,a);if(!p)return 0;diagnose_pools(r,1,a);const auto ticket=make_call(fn,a,arg,third,uint32_t(c->r31));calls.at(ticket).scope=std::make_unique<b::Scope>(r,c,*p);return ticket;}
 if(fn==0x805ABBE8){
  const auto p=owner(r,third);if(!p)return 0;diagnose_pools(r,2,third);const auto ticket=make_call(fn,a,arg,third,uint32_t(c->r31));calls.at(ticket).scope=std::make_unique<b::Scope>(r,c,*p);return ticket;
 }
 if(fn==0x800443EC&&uint32_t(c->r31)==warm_overlay+0x7C){
  const auto p=owner(r,a);const auto pot=uint32_t(c->r7);if(!p)return 0;const auto pot_id=observed(r,pot);if(!pot_id||pot_id->kind!=0x44CA||!n::range(pot_id->private_data,0x26C)){fail("warm_pot_identity");return blocked;}
  const auto res=n::word(r,pot+0x168);if(!n::range(res,0x48)||n::word(r,res+0x40)!=warm_overlay||ids::half(r,res+0x3C)!=0x44CA){fail("warm_resource_identity");return blocked;}
  m::Registration gate{coordinator.epoch(),room,vi,identity(life::lifecycle.snapshot().players[*p].root),*pot_id,{145,warm_overlay,room_load},uint32_t(c->r31),n::word(r,pot_id->private_data+0x264),n::word(r,pot+12),canonical_player(r),n::word(r,pot_id->private_data+0x268),6};
  if(coordinator.phase()!=m::Phase::arriving||room!=coordinator.departure().route.destination()||!m::valid(gate.overlay)||gate.current_in_pot!=a||gate.cooldown||gate.pot_target!=coordinator.departure().route.target()||warm_gates[*p]){fail("warm_registration_gate");return blocked;}
  warm_gates[*p]=gate;return 0;
 }
 if(fn==0x800440E4&&uint32_t(c->r31)==0x800444B0){
  const auto p=owner(r,a);if(!p)return 0;if(!warm_gates[*p]||warm_gates[*p]->epoch!=coordinator.epoch()||permits[*p]){fail("warm_registration_without_gate");return blocked;}
  auto gate=*warm_gates[*p];const auto pot_now=observed(r,gate.pot.address);const auto slot=*p?generic_slot(r):5;
  if(!pot_now||*pot_now!=gate.pot||slot<0||gate.actor!=identity(life::lifecycle.snapshot().players[*p].root)){fail("warm_gate_identity_changed");return blocked;}
  gate.vi=vi;gate.free_slot=unsigned(slot);gate.canonical_player=canonical_player(r);const auto permit=coordinator.registration(gate);
  if(!permit){fail("warm_registration_permit");return blocked;}permits[*p]=permit;
  if(*p==1){b::Scope scope(r,c,1);if(!generic_register(r,c,a,permits[1]->slot))fail("native_generic_registration");else registered[1]=true;return blocked;}
  const auto ticket=make_call(fn,a,arg,third,uint32_t(c->r31));calls.at(ticket).scope=std::make_unique<b::Scope>(r,c,0);return ticket;
 }
 return 0;
}
inline int32_t pot_axis(uint8_t* r,recomp_context* c,uint32_t candidate,int32_t native_y){
 if(!enabled||failed||!active_pot)return native_y;const auto it=calls.find(active_pot);const auto p=owner(r,candidate);if(it==calls.end()||it->second.fn!=0x805AAF8C||!p)return 0;
 if(n::word(r,candidate+0x180)!=it->second.a||!reciprocal(r,candidate,it->second.a))return 0;
 b::Scope scope(r,c,*p);return int16_t(ids::half(r,0x800F8980));
}
inline void finish(uint8_t* r,recomp_context*,uint64_t token,bool completed){
 if(!token||token==blocked)return;const auto it=calls.find(token);if(it==calls.end())return;
 const bool diagnostic=it->second.fn==0x8002F7AC||(it->second.fn==0x805AAF8C&&bool(it->second.scope));const auto fn=it->second.fn;
 struct DiagnosticClose{bool active;uint32_t fn;uint64_t token;~DiagnosticClose(){if(active)diagnose_transaction(fn,token,"finish-leave");}} done{diagnostic,fn,token};
 if(diagnostic)diagnose_transaction(fn,token,"finish-enter");auto call=std::move(it->second);calls.erase(it);
 if(call.fn==0x805AAF8C){active_pot=0;if(!completed)fail("pot_callback_did_not_return");return;}
 if(!completed){fail("native_call_did_not_return");return;}
 if(call.fn==0x800443EC&&entry_rejoin&&call.a==entry_rejoin->actor.address&&call.ra==0x800481A4){if(!entry_identity(r)||!pending_rejoin(r,call.a)){fail("native_entry_pending_receipt");return;}entry_rejoin->queued=true;}
 if(call.fn==0x800443EC&&support_rejoin&&call.a==support_rejoin->actor.address&&call.ra==0x800481A4){
  if(!support_identity(r)||support_rejoin->pending_call!=token||support_rejoin->queued||!pending_rejoin(r,call.a)||!main_absent(r,call.a)){fail("native_support_pending_receipt");return;}
  support_rejoin->pending_call=0;support_rejoin->queued=true;
 }
 if(call.fn==0x80061170&&entry_rejoin&&call.a==entry_rejoin->actor.address){if(!entry_identity(r)||!entry_rejoin->queued||!pending_rejoin(r,call.a)){fail("native_entry_detach_receipt");return;}}
 if(call.fn==0x8002F7AC){
  const auto p=owner(r,call.a);m::Route rt;m::Snapshot payload;
  const auto data=n::word(r,call.a+0x1F4);
  if(!p||!source(r,call.arg,rt)||n::word(r,data+0xC0)!=0x8002C3B4||n::word(r,data+0xD0)!=call.arg||!snapshot(r,payload)){fail("native_pot_admission");return;}
  admission=Admission{*p,current_vi,call.arg,rt,payload,*observed(r,call.arg),life::lifecycle.snapshot().epoch,current_room};
 }
 if(call.fn==0x8005D1C8){const auto id=observed(r,call.arg);if(!id||!carry_linked(r,call.a,call.arg)||!coordinator.carried({coordinator.epoch(),*id,call.a,n::word(r,call.arg+0x180),call.fn,true,true})){fail("native_carry_receipt");return;}const auto p=owner(r,call.arg);std::fprintf(stderr,"[CoopRooms] carried epoch=%llu owner=%u actor=%08X private=%08X birth=%llu list=%08X vi=%u fn=8005D1C8 linked=1 unregistered=1 nativeObservation=1\n",(unsigned long long)coordinator.epoch(),*p,id->address,id->private_data,(unsigned long long)id->birth,call.a,current_vi);}
 if(call.fn==0x800440E4){unsigned slot=150;const auto p=owner(r,call.a);if(!p||*p!=0||!registered_as(r,call.a,slot)||slot!=5||n::byte(r,call.a+0x17D)!=2){fail("native_canonical_registration");return;}registered[0]=true;}
 if(call.fn==0x805ABBE8){const auto p=owner(r,call.third);if(!p||!warm_gates[*p]){fail("warm_return_without_gate");return;}warm_returned[*p]=true;}
}
inline void trace(uint8_t* r,recomp_context* c,const char* name,unsigned vi,unsigned room,uint32_t entry,uint32_t warm){
 current_vi=vi;current_room=room;entry_overlay=entry;warm_overlay=warm;const std::string_view fn(name);if(fn=="func_800675F0")diagnose_fence();if(fn=="func_805AAF8C"||fn=="func_8002F7AC")diagnose_gate(r,c,fn=="func_805AAF8C"?0x805AAF8C:0x8002F7AC,uint32_t(c->r4),uint32_t(c->r5),true,entry);if(fn=="func_80048060")observe_support_detach(r,c);if(!enabled||!coordinator.busy()||failed)return;
 if(fn=="func_8005D320"){m::Route rt;if(uint32_t(c->r31)!=m::route_caller||!route(r,uint32_t(c->r4),coordinator.departure().room,rt)||!coordinator.depart(coordinator.epoch(),uint32_t(c->r31),rt,vi)){fail("native_route_departure");return;}
  bool empty=coordinator.departure().payload.players[0].count==0&&coordinator.departure().payload.players[1].count==0;for(const auto& p:coordinator.departure().payload.players)for(auto a:n::tree(r,p.actor.address))if(n::kind(r,a)==0x4004||n::kind(r,a)==0x4005)empty=false;
  m::Snapshot preserved;if(!snapshot(r,preserved)||preserved!=coordinator.departure().payload){fail("native_departure_pool_payload");return;}
  if(!coop_egg_pools::capture_room_lease(r,coordinator.epoch(),empty,{preserved.players[0].actor.address,preserved.players[1].actor.address})){fail("native_room_pool_lease");return;}transporting=true;}
 if(fn=="func_8006DA0C"){const unsigned target=unsigned(c->r5)&255;const auto list=n::word(r,0x800F8770);
  if(uint32_t(c->r31)!=0x8006DDFC||!n::range(uint32_t(c->r4),0x24)||!n::range(list,0x84)||n::word(r,list+0x80)!=0x800E8090||ids::half(r,list+0x7C)!=1){
   const bool valid=n::range(list,0x84);std::fprintf(stderr,"[CoopRooms] entry-chain vi=%u ra=%08X manager=%08X managerValid=%u list=%08X listValid=%u flagWord=%08X flagHalf=%04X routePtr=%08X target=%u\n",vi,uint32_t(c->r31),uint32_t(c->r4),unsigned(n::range(uint32_t(c->r4),0x24)),list,unsigned(valid),valid?n::word(r,list+0x7C):0,valid?ids::half(r,list+0x7C):0,valid?n::word(r,list+0x80):0,target);fail("native_room_entry_chain");return;
  }
  for(unsigned i=0;i<24;i++)if(n::byte(r,0x800E8090+i)!=coordinator.departure().route.bytes[i]){fail("native_copied_route_changed");return;}
  entering_manager=uint32_t(c->r4);if(!coordinator.enter(coordinator.epoch(),target,n::byte(r,0x800F7EA2),vi)){fail("native_room_entry");return;}++room_load;current_room=target;std::fprintf(stderr,"[CoopRooms] enter epoch=%llu room=%u vi=%u nativeObservation=1\n",(unsigned long long)coordinator.epoch(),target,vi);}
 if(fn=="func_8006DD34"){diagnose_pools(r,0,uint32_t(c->r4));const auto level=uint32_t(c->r4);if(uint32_t(c->r31)!=0x8006DE08||!n::range(entering_manager,0x24)||n::word(r,entering_manager+0x20)!=level||!n::range(level,0x128)||n::byte(r,level+1)!=coordinator.departure().route.destination()||!coordinator.room_ready(coordinator.epoch(),coordinator.departure().route.destination(),level,vi)){fail("native_room_ready");return;}std::fprintf(stderr,"[CoopRooms] ready epoch=%llu room=%u vi=%u nativeObservation=1\n",(unsigned long long)coordinator.epoch(),coordinator.departure().route.destination(),vi);
  m::Snapshot preserved;if(!snapshot(r,preserved)||preserved!=coordinator.departure().payload){fail("native_ready_pool_payload");return;}
  if(!b::owner_stack.empty()||b::context->depth()||!coop_egg_pools::rejoin_empty_room_lease(r,c,coordinator.epoch())){fail("native_empty_pool_reregistration");return;}
 }
}
inline void frame(uint8_t* r,recomp_context*,unsigned vi,unsigned room){
 current_vi=vi;current_room=room;if(!enabled||!busy()||failed)return;if(admission&&vi-admission->vi>m::maximum_duration){fail("native_admission_timeout");return;}if(!coordinator.clock(vi)){fail("native_room_timeout");return;}
 if(coordinator.phase()!=m::Phase::arriving)return;
 for(unsigned p=0;p<2;p++)if(warm_returned[p]&&registered[p]&&!arrival_reported[p]){
  const auto a=life::lifecycle.snapshot().players[p].root.address;unsigned slot=150;m::Arrival event;event.permit=*permits[p];event.room=room;event.vi=vi;event.queue=n::byte(r,a+0x17D);event.hidden=n::byte(r,a+0x165);event.parent=n::word(r,a+0x180);event.callback=n::word(r,a+0x130);event.original_completed=true;
  if(!registered_as(r,a,slot)){fail("arrival_manager_identity");return;}event.slot=slot;event.registered_actor=a;const auto manager=n::word(r,0x800F8810);event.canonical_player=n::word(r,n::word(r,manager+0x10)+20);
  for(unsigned i=0;i<3;i++){event.local[i]=n::word(r,a+0x3C+i*4);event.world[i]=n::word(r,a+0x84+i*4);}if(!player(r,p,event.player)||!coordinator.finish_arrival(event)){fail("native_warm_arrival_payload");return;}arrival_reported[p]=true;
  std::fprintf(stderr,"[CoopRooms] arrival epoch=%llu owner=%u actor=%08X private=%08X birth=%llu room=%u slot=%u queue=%u hidden=%u parent=%08X callback=%08X hp=%u fifo=%u vi=%u fn=805ABBE8 payloadMatch=1 nativeObservation=1\n",(unsigned long long)coordinator.epoch(),p,a,event.player.actor.private_data,(unsigned long long)event.player.actor.birth,room,slot,event.queue,event.hidden,event.parent,event.callback,event.player.hp,event.player.count,vi);
 }
 if(coordinator.arrived()!=3||!b::owner_stack.empty()||coop_fruit0264::registry.pending())return;m::Snapshot value;if(!snapshot(r,value)){fail("pair_arrival_snapshot");return;}
 const auto canonical=n::word(r,n::word(r,n::word(r,0x800F8810)+0x10)+20);if(!coordinator.complete(coordinator.epoch(),room,vi,canonical,value,true)){fail("pair_arrival_payload");return;}
 if(!life::lifecycle.transferred(r,room)){fail("lifecycle_room_handoff");return;}transporting=false;admission.reset();b::publish_anchor(r);
 std::fprintf(stderr,"[CoopRooms] complete epoch=%llu room=%u vi=%u arrived=3 payloadMatch=1 payloadSig=%016llX nativeObservation=1\n",(unsigned long long)coordinator.epoch(),room,vi,(unsigned long long)signature(value));
}
} // namespace
