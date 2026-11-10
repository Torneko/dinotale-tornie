#pragma once
#include "coop_native_factory.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string_view>

extern "C" void func_80047090(uint8_t*,recomp_context*);
extern "C" void func_80044194(uint8_t*,recomp_context*);
extern "C" void func_8004667C(uint8_t*,recomp_context*);
extern "C" void func_80064DD0(uint8_t*,recomp_context*);
extern "C" void func_8001D5B4(uint8_t*,recomp_context*);

namespace workshop::coop_classic_flower0250 {
namespace native=coop_native;
inline constexpr uint16_t centre_kind=0x4010,petal_kind=0x4011;
inline constexpr uint32_t native_petal_pool=0x800C1540,native_petal_storage=0x800C1550;
inline constexpr uint32_t pool_manager_global=0x800F887C,mask_global=0x800F8DA8;
inline constexpr unsigned petal_capacity=16,slot_stride=0x280,pool_bytes=0x2810;
inline constexpr uint32_t petal_packed=0x40110200,centre_z_bits=0x43FA07AE;
inline constexpr float secondary_offset_x=200.0f;
inline constexpr std::array<uint32_t,4> integration_functions{0x800485B8,0x8001CC20,0x80046804,0x8005F700};
struct State {
 bool enabled=false,failed=false,attempted=false,retiring=false,ready_seen=false,rebind_requested=false;
 uint64_t generation=0;unsigned last_vi=0,deferred_inits=0,position_calls=0,petal_pool_redirects=0;
 uint32_t primary=0,secondary=0,primary_private=0,secondary_private=0;
 uint32_t centre=0,centre_packed=0,centre_private=0;
 uint32_t allocation=0,storage=0,manager=0;unsigned allocations=0,rebinds=0;
 std::string_view failure;unsigned last_wait=~0u;
};
inline State state;
inline unsigned creator_depth=0;
inline uint32_t creator_centre=0;
inline uint64_t creator_generation=0;
inline uint32_t word(uint8_t* r,uint32_t p){return native::word(r,p);}
inline unsigned half(uint8_t* rdram,uint32_t p){return MEM_HU(0,native::base(p));}
inline bool range(uint32_t p,unsigned n){return p%4==0&&native::range(p,n);}
inline bool actor(uint8_t* r,uint32_t p,uint16_t kind){return r&&native::actor(r,p,kind);}
inline bool fail(const char* why){state.failed=true;state.failure=why;fprintf(stderr,"[CoopFlower] failed generation=%llu reason=%s\n",(unsigned long long)state.generation,why);return false;}
// Configure only before a fresh native runtime starts. Never discard a live
// registered pool: one10KiB allocation persists for the process lifetime.
inline void configure(bool enabled){
 if(state.allocation||state.centre){if(state.enabled!=enabled)fail("configure_with_live_native_flower");return;}
 state={};state.enabled=enabled;creator_depth=0;creator_centre=0;creator_generation=0;
}
inline void wait(unsigned code,const char* why,unsigned vi){if(state.last_wait!=code){state.last_wait=code;fprintf(stderr,"[CoopFlower] wait vi=%u reason=%s\n",vi,why);}}
inline bool manager_shape(uint8_t* r,uint32_t manager){
 if(!range(manager,0x14))return false;const unsigned count=half(r,manager+6),capacity=half(r,manager+8);const uint32_t table=word(r,manager+0x10);
 return capacity>0&&capacity<=150&&count<=capacity&&range(table,capacity*4);
}
inline int registered_slot(uint8_t* r,uint32_t manager,uint32_t pool){
 if(!manager_shape(r,manager))return -1;const auto table=word(r,manager+0x10);const auto capacity=half(r,manager+8);int found=-1;
 for(unsigned i=0;i<capacity;i++)if(word(r,table+i*4)==pool){if(found>=0)return -1;found=int(i);}return found;
}
inline bool manager_has_space(uint8_t* r,uint32_t manager){
 if(!manager_shape(r,manager)||half(r,manager+6)>=half(r,manager+8))return false;
 const auto table=word(r,manager+0x10);for(unsigned i=0;i<half(r,manager+8);i++)if(!word(r,table+i*4))return true;return false;
}
inline bool allocation_valid(uint8_t* r){
 const auto at=state.allocation;
 return at&&range(at-0x10,pool_bytes+0x10)&&!(at&15)&&half(r,at-0x10)==0x913&&half(r,at-0xE)!=0&&word(r,at-4)>=pool_bytes+0x10;
}
// Native46560 may unregister an empty pool at context closure. Reuse the
// SAME allocation only after all slots are quiescent, the old centre is gone,
// and the old manager no longer references this header (or is proven freed).
inline bool pool_storage_valid(uint8_t* r){return allocation_valid(r)&&state.storage==state.allocation+0x10&&word(r,state.allocation+4)==petal_capacity&&word(r,state.allocation+8)==state.storage;}
inline bool pool_valid(uint8_t* r){return pool_storage_valid(r)&&word(r,pool_manager_global)==state.manager&&registered_slot(r,state.manager,state.allocation)>=0;}
inline unsigned busy_petals_raw(uint8_t* r){
 if(!pool_storage_valid(r))return ~0u;unsigned busy=0;for(unsigned i=0;i<petal_capacity;i++){const auto value=native::byte(r,state.storage+i*slot_stride);if(value>1)return ~0u;busy+=value!=0;}return busy;
}
inline unsigned busy_petals(uint8_t* r){return pool_valid(r)?busy_petals_raw(r):~0u;}
inline bool old_manager_released(uint8_t* r,uint32_t current){
 if(!state.manager||state.manager==current)return registered_slot(r,current,state.allocation)<0;
 const bool old_freed=range(state.manager-0x10,0x24)&&half(r,state.manager-0x10)==0x913&&half(r,state.manager-0xE)==0;
 return old_freed||(manager_shape(r,state.manager)&&registered_slot(r,state.manager,state.allocation)<0);
}
inline bool secondary_valid(uint8_t* r){return actor(r,state.secondary,0x4001)&&word(r,state.secondary+0x1F4)==state.secondary_private;}
inline bool centre_valid(uint8_t* r){
 return actor(r,state.centre,centre_kind)&&word(r,state.centre+12)==state.centre_packed&&secondary_valid(r)&&(!state.centre_private||word(r,state.centre+0x1F4)==state.centre_private);
}
// Pure read: an independent native4010 graph is deliberately not attached to
// the Yoshi graph (which also owns six eggs and has existing bounded readers).
// NativeCC20 attaches4011 to its4010; lost petals retain private+0 afterdetach.
inline std::optional<unsigned> owner(uint8_t* r,uint32_t a){
 if(!state.enabled||!state.centre||!centre_valid(r))return {};
 if(a==state.centre)return 1;
 if(!actor(r,a,petal_kind))return {};
 const auto data=word(r,a+0x1F4);if(!range(data,0x10)||word(r,data)!=state.centre)return {};
 const uint32_t parent=word(r,a+0x180);
 if(parent&&parent!=state.centre)return {};
 // Every private-P2 petal must carry the real nativepoolback-pointer.
 if(!state.allocation||word(r,a+8)!=state.allocation)return {};
 return 1;
}
inline void forget(uint32_t a){
 if(a==state.centre){state.centre=state.centre_private=state.centre_packed=0;state.retiring=false;state.rebind_requested=true;}
 if(a==state.primary||a==state.secondary){state.ready_seen=false;state.retiring=true;state.rebind_requested=true;}
}
inline bool ensure_pool(uint8_t* r,recomp_context* c){
 if(state.allocation){
  if(pool_valid(r))return true;
  if(!pool_storage_valid(r))return fail("private_pool_heap_or_storage_changed");
  if(creator_depth||(state.centre&&actor(r,state.centre,centre_kind))){wait(5,"old_flower_or_creator_not_quiescent",state.last_vi);return false;}
  const unsigned busy=busy_petals_raw(r);if(busy==~0u)return fail("private_pool_busy_flags_invalid");if(busy){wait(6,"old_private_petals_still_live",state.last_vi);return false;}
  const auto manager=word(r,pool_manager_global);
  if(!manager_has_space(r,manager)){wait(7,"new_native_pool_manager_not_ready",state.last_vi);return false;}
  if(registered_slot(r,manager,state.allocation)>=0||!old_manager_released(r,manager)){wait(8,"old_manager_still_references_private_pool",state.last_vi);return false;}
  native::ScopedContext registers(c);state.manager=manager;c->r4=native::base(state.allocation);c->r5=native::base(state.storage);c->r6=petal_capacity;func_8004667C(r,c);
  if(!pool_valid(r)||busy_petals(r)!=0)return fail("private_pool_native_rebind_guard");state.rebinds++;
  fprintf(stderr,"[CoopFlower] pool-rebound header=%08X manager=%08X rebinds=%u sameAllocation=1 quiescent=1\n",state.allocation,manager,state.rebinds);return true;
 }
 if(state.allocations)return fail("private_pool_reallocation_refused");
 // The loaded native4011 pool is checked, never initialized or mutated here.
 if(word(r,native_petal_pool+4)!=16||word(r,native_petal_pool+8)!=native_petal_storage)return fail("native_petal_pool_not_loaded");
 const auto manager=word(r,pool_manager_global);if(!manager_has_space(r,manager))return fail("no_native_pool_manager_slot");
 native::ScopedContext registers(c);c->r4=pool_bytes;func_80064DD0(r,c);const uint32_t block=uint32_t(c->r2);
 if(!block)return fail("private_pool_allocation_failed");state.allocation=block;state.storage=block+0x10;state.manager=manager;state.allocations++;
 if(!allocation_valid(r))return fail("private_pool_allocation_guard");
 c->r4=native::base(block);c->r5=native::base(state.storage);c->r6=petal_capacity;func_8004667C(r,c);
 if(!pool_valid(r)||busy_petals(r)!=0)return fail("private_pool_native_initialization_guard");
 fprintf(stderr,"[CoopFlower] private-pool header=%08X storage=%08X capacity=%u bytes=%u manager=%08X persistent=1\n",block,state.storage,petal_capacity,pool_bytes,manager);return true;
}
inline void retire(uint8_t* r,recomp_context* c){
 if(!state.centre||!actor(r,state.centre,centre_kind))return;
 const unsigned phase=native::byte(r,state.centre+0x17D);
 // Pending native FIFO entries must never be retired through445A4 directly.
 // Keep identity until the native queue activates or the context teardown
 // forgets this incarnation. Root may add its proven FIFO removal helper.
 if(phase==1||phase==4){state.retiring=true;wait(4,"pending_flower_native_teardown_required",state.last_vi);return;}
 if(phase!=3){native::ScopedContext registers(c);c->r4=native::base(state.centre);func_800445A4(r,c);}
 state.retiring=true;
}
// Only the675F0 fence, after bridge binding and outside all owner callbacks.
// This duplicates the verified D6FC native creation sequence and captures V0
// BEFORE44194 (whose V0 is a boolean). Native init runs later through48A30,
// with the explicit owner resolver already able to scope this basic actor.
inline void frame(uint8_t* r,recomp_context* c,unsigned vi,uint32_t primary,uint32_t secondary,bool world_ready){
 if(!state.enabled||!r||!c)return;state.last_vi=vi;
 if(!world_ready){if(state.ready_seen){retire(r,c);state.ready_seen=false;state.rebind_requested=true;}return;}
 if(!actor(r,primary,0x4001)||!actor(r,secondary,0x4001)||primary==secondary)return;
 const auto p=word(r,primary+0x1F4),q=word(r,secondary+0x1F4);if(!range(p,0x1550)||!range(q,0x1550))return;
 const bool changed=state.rebind_requested||primary!=state.primary||secondary!=state.secondary||p!=state.primary_private||q!=state.secondary_private;
 if(changed){
  if(state.centre){if(!actor(r,state.centre,centre_kind))forget(state.centre);else {retire(r,c);wait(1,"old_flower_native_retirement",vi);return;}}
  if(state.allocation){if(busy_petals_raw(r)>petal_capacity){fail("private_pool_storage_changed_on_owner_rebind");return;}if(busy_petals_raw(r)){wait(6,"old_private_petals_still_live",vi);return;}if(!ensure_pool(r,c))return;}
  state.primary=primary;state.secondary=secondary;state.primary_private=p;state.secondary_private=q;state.generation++;state.attempted=false;state.retiring=false;state.rebind_requested=false;
 }
 state.ready_seen=true;
 if(state.failed)return;
 if(state.centre){
  if(!centre_valid(r)){fail("native_flower_identity_changed");return;}
  const auto data=word(r,state.centre+0x1F4);
  if(data){if(!range(data,0x3C)){fail("native_flower_private_invalid");return;}if(!state.centre_private){state.centre_private=data;fprintf(stderr,"[CoopFlower] initialized vi=%u centre=%08X private=%08X owner=1 generation=%llu\n",vi,state.centre,data,(unsigned long long)state.generation);}}
  return;
 }
 if(state.attempted)return;
 if(word(r,mask_global)!=0){wait(2,"native_flower_hidden",vi);return;}
 if(native::free_actor_slots(r)<1||native::free_graphics_slots(r,0)<9){wait(3,"native_actor_or_sprite_capacity",vi);return;}
 if(!ensure_pool(r,c))return;
 if(word(r,0x800AFBAC)!=centre_z_bits){fail("native_flower_depth_constant_differs");return;}
 state.attempted=true;native::ScopedContext registers(c);
 c->r4=centre_kind;func_80047090(r,c);const uint32_t instance=uint32_t(c->r2);
 if(instance>0x3FF){fail("native_flower_instance_invalid");return;}
 const uint32_t packed=uint32_t(centre_kind)<<16|0x2000|instance;
 c->r4=native::base(packed);c->r5=0;c->r6=0;c->r7=0;func_8004613C(r,c);const uint32_t centre=uint32_t(c->r2);
 if(!actor(r,centre,centre_kind)||word(r,centre+12)!=packed||word(r,centre+0x1F4)!=0||word(r,centre+0x20)!=0x8001D5B4){fail("native_flower_basic_creation_guard");return;}
 state.centre=centre;state.centre_packed=packed;state.centre_private=0;
 c->r4=native::base(centre);func_80044194(r,c);
 if(uint32_t(c->r2)!=1||!centre_valid(r)||native::byte(r,centre+0x17D)!=1){retire(r,c);fail("native_flower_registration_guard");return;}
 native::word(r,centre+0x44,centre_z_bits);
 fprintf(stderr,"[CoopFlower] created vi=%u centre=%08X packed=%08X secondary=%08X nativePending=1 independentGraph=1\n",vi,centre,packed,secondary);
}
// Lookup override for indirect actor+20==8001D5B4 ONLY. Native48A30 is already
// under the proper owner scope. A request to hide the HUD must remain intact.
inline void deferred_init(uint8_t* r,recomp_context* c){
 const auto a=uint32_t(c->r4);
 if(a==state.centre&&owner(r,a)&&word(r,mask_global)!=0){state.deferred_inits++;return;}
 func_8001D5B4(r,c);
}
// A public native485B8 wrapper must call this before its original body. Native
// CF70,C648,C960 andCA40 call485B8 directly, so lookup redirection alone would
// miss them. Translate incoming screen coordinates, never previous positions.
inline bool position(uint8_t* r,recomp_context* c){
 if(!c||!owner(r,uint32_t(c->r4)))return false;
 float x=std::bit_cast<float>(uint32_t(c->r5));if(!std::isfinite(x))return false;
 c->r5=std::bit_cast<uint32_t>(x+secondary_offset_x);state.position_calls++;return true;
}
struct PetalCreationScope {
 unsigned depth;uint32_t centre;uint64_t generation;
 PetalCreationScope(uint8_t* r,uint32_t a):depth(creator_depth),centre(creator_centre),generation(creator_generation){
  const bool selected=a==state.centre&&owner(r,a).has_value();creator_depth=selected?depth+1:0;creator_centre=selected?a:0;creator_generation=selected?state.generation:0;
 }
 ~PetalCreationScope(){creator_depth=depth;creator_centre=centre;creator_generation=generation;}
 PetalCreationScope(const PetalCreationScope&)=delete;PetalCreationScope& operator=(const PetalCreationScope&)=delete;
};
//46804 wrapper: -1 means return the native exhausted-pool result V0=0 without
//calling original; 0 leaves unrelated native calls unchanged; 1 redirects A0.
//A1, float args A2/A3, stackZ and all native return registers remain untouched.
inline int petal_pool(uint8_t* r,recomp_context* c){
 if(!c||!creator_depth||creator_centre!=state.centre||creator_generation!=state.generation)return 0;
 if(uint32_t(c->r4)!=native_petal_pool||uint32_t(c->r5)!=petal_packed)return 0;
 if(!owner(r,state.centre)||!pool_valid(r)){fail("private_petal_creation_guard");return -1;}
 c->r4=native::base(state.allocation);state.petal_pool_redirects++;return 1;
}
}
