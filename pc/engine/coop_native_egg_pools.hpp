#pragma once
#include "recomp.h"
#include <array>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <optional>

extern "C" void func_80064DD0(uint8_t*,recomp_context*);
extern "C" void func_80064E84(uint8_t*,recomp_context*);
extern "C" void func_8004661C(uint8_t*,recomp_context*);
extern "C" void func_8004667C(uint8_t*,recomp_context*);
extern "C" void func_80039ABC(uint8_t*,recomp_context*);
extern "C" void func_80057BAC(uint8_t*,recomp_context*);
extern "C" void func_80046790(uint8_t*,recomp_context*);
extern "C" void func_80046324(uint8_t*,recomp_context*);

namespace workshop::coop_egg_pools {
// Only call ensure/release at the completed675F0 fence, outside owner scopes.
// P1 keeps its native pools. P2 gets native descriptor/slot initialization,
// separate actor addresses, and embedded private blocks; no active actor moves.
inline constexpr std::array<uint32_t,2> globals{0x800C426C,0x800C4270};
inline constexpr std::array<uint32_t,2> stock_descriptors{0x800C5690,0x800C6AA0};
inline constexpr std::array<uint32_t,2> stock_records{0x800C56A0,0x800C6AB0};
inline constexpr std::array<uint32_t,2> capacities{8,2};
inline constexpr std::array<uint32_t,2> descriptor_offsets{0,0x10};
inline constexpr std::array<uint32_t,2> record_offsets{0x20,0x1420};
inline constexpr uint32_t manager_global=0x800F887C,stride=0x280,block_size=0x1920;
inline constexpr uint32_t guard=0xE6640000;
struct State {
    bool enabled=false,installed=false,invalidated=false;
    uint32_t allocation=0,manager=0,table=0;
    unsigned manager_capacity=0,depth=0;
    std::array<uint32_t,2> primary{};
    std::array<bool,2> initialized{};
    uint64_t selections=0;
};
inline State state{};
struct RoomFIFO {uint32_t actor=0,data=0;unsigned head=0,count=0;std::array<uint8_t,8*24> bytes{};bool operator==(const RoomFIFO&)const=default;};
struct RoomLease {
 uint64_t epoch=0;uint32_t allocation=0,manager=0,table=0;unsigned capacity=0;
 std::array<bool,4> empty{};std::array<std::array<uint8_t,8*stride>,4> records{};
 std::array<RoomFIFO,2> fifo{};
};
inline std::optional<RoomLease> room_lease;
inline bool rejected_scope_logged=false;
inline const State& snapshot(){return state;}
inline gpr address(uint32_t p){return gpr(int32_t(p));}
inline bool range(uint32_t p,uint32_t bytes,uint32_t align=4){return p>=0x80000400 && p<0x80800000 && p%align==0 && bytes<=0x80800000-p;}
inline uint32_t word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,address(p)));}
inline void word(uint8_t* rdram,uint32_t p,uint32_t v){MEM_W(0,address(p))=v;}
inline unsigned byte(uint8_t* rdram,uint32_t p){return MEM_BU(0,address(p));}
inline void byte(uint8_t* rdram,uint32_t p,unsigned v){MEM_B(0,address(p))=uint8_t(v);}
inline unsigned half(uint8_t* rdram,uint32_t p){return MEM_HU(0,address(p));}
inline void half(uint8_t* rdram,uint32_t p,unsigned v){MEM_H(0,address(p))=uint16_t(v);}
inline uint32_t pool(unsigned owner,unsigned index){
    if(index>=2 || owner>=2)return 0;
    return owner==0?state.primary[index]:state.allocation?state.allocation+descriptor_offsets[index]:0;
}
inline bool heap_block(uint8_t* r){
    const auto p=state.allocation;
    return r && p>=0x80000410 && range(p-0x10,block_size+0x10,16) && half(r,p-0x10)==0x913 && half(r,p-0xE)!=0 && word(r,p-4)>=block_size+0x10;
}
inline bool manager_matches(uint8_t* r){
    return r && range(state.manager,0x18) && word(r,manager_global)==state.manager &&
        half(r,state.manager+8)==state.manager_capacity && state.manager_capacity>=2 && state.manager_capacity<=150 &&
        word(r,state.manager+0x10)==state.table && range(state.table,state.manager_capacity*4) && half(r,state.manager+6)<=state.manager_capacity;
}
inline int registration(uint8_t* r,uint32_t descriptor){
    if(!manager_matches(r))return -2;
    int result=-1;
    for(unsigned n=0;n<state.manager_capacity;n++)if(word(r,state.table+n*4)==descriptor){if(result>=0)return -2;result=int(n);}
    return result;
}
inline bool layout(uint8_t* r,unsigned index){
    const auto d=pool(1,index);
    return index<2 && heap_block(r) && range(d,16) && word(r,d+4)==capacities[index] &&
        word(r,d+8)==state.allocation+record_offsets[index] && word(r,d+12)==(guard|uint32_t(0x4004+index));
}
inline unsigned free_slots(uint8_t* r,uint32_t descriptor){
    if(!r || !range(descriptor,12))return 0;
    const auto count=word(r,descriptor+4),raw=word(r,descriptor+8);
    if(count<1 || count>16 || !range(raw,count*stride,16))return 0;
    unsigned count_free=0;
    for(unsigned n=0;n<count;n++)if(byte(r,raw+n*stride)==0)++count_free;
    return count_free;
}
inline bool matches(uint8_t* r){
    if(!state.enabled || !state.installed || state.invalidated || !heap_block(r) || !manager_matches(r))return false;
    for(unsigned k=0;k<2;k++)if(!state.initialized[k] || !layout(r,k) || registration(r,pool(1,k))<0 ||
        state.primary[k]!=stock_descriptors[k] || word(r,state.primary[k]+4)!=capacities[k] || word(r,state.primary[k]+8)!=stock_records[k])return false;
    return true;
}
inline void invalidate()noexcept{state.invalidated=true;}
inline void configure(bool enabled){
    if(state.allocation || state.depth)throw std::logic_error("Live native egg pools must be released before configuration");
    state={};state.enabled=enabled;room_lease.reset();rejected_scope_logged=false;
}
struct Registers {
    recomp_context* context;recomp_context saved;
    explicit Registers(recomp_context* c):context(c),saved(*c){context->r29=ADD32(saved.r29,-0x80);}
    ~Registers(){*context=saved;}
};
inline bool outside_primary(uint8_t* r){return state.depth==0 && word(r,globals[0])==state.primary[0] && word(r,globals[1])==state.primary[1];}
inline bool inside_block(uint32_t p){return p>=state.allocation && p<state.allocation+block_size;}
inline bool actor_graph_references(uint8_t* r){
    const uint32_t manager=word(r,0x800F8810);
    if(!range(manager,0x18))return true;
    const unsigned count=half(r,manager+8);const uint32_t table=word(r,manager+0x10);
    if(count<1 || count>150 || !range(table,count*4))return true;
    for(unsigned n=0;n<count;n++){
        const uint32_t actor=word(r,table+n*4);if(inside_block(actor))return true;
        if(!range(actor,0x1FC) || word(r,actor)!=0x19971110 || byte(r,actor+6)==3)continue;
        if(inside_block(word(r,actor+0x180)))return true;
        //47F9C stores in20 fixed slots;48060 leaves holes when detaching.
        //184 is the occupied count, never an upper bound for slot indices.
        const unsigned children=half(r,actor+0x184);if(children>20)return true;
        unsigned occupied=0;
        for(unsigned child=0;child<20;child++){
            const uint32_t value=word(r,actor+0x188+child*4);
            if(value)++occupied;
            if(inside_block(value))return true;
        }
        if(occupied!=children)return true;
    }
    return false;
}
inline bool graphics_references(uint8_t* r){
    // Native45414 frees its graphics flag. Confirm no still-active record in
    // either native frame contains an address into the allocation being freed.
    for(unsigned type=0;type<7;type++){
        const uint32_t d=0x800A6344+type*20,count=word(r,d),flags=word(r,d+4),step=word(r,d+8);
        if(!count)continue;
        if(count>1024 || step<4 || step>0x400 || step%4 || !range(flags,count,1))return true;
        for(unsigned frame=0;frame<2;frame++){
            const uint32_t records=word(r,d+12+frame*4);
            if(!range(records,count*step))return true;
            for(unsigned n=0;n<count;n++)if(byte(r,flags+n))
                for(unsigned offset=0;offset<step;offset+=4)if(inside_block(word(r,records+n*step+offset)))return true;
        }
    }
    return false;
}

// Unregister only our empty descriptors, then release their sole allocation.
//57BAC removes one indexed entry; like46560, its caller must decrement H+6.
//57B14 resets a WHOLE manager and is deliberately never used here.
inline bool release(uint8_t* r,recomp_context* c){
    if(!state.allocation)return true;
    if(!c || !heap_block(r) || !manager_matches(r) || !outside_primary(r))return false;
    for(unsigned k=0;k<2;k++){
        const auto d=pool(1,k);const int slot=registration(r,d);
        if(slot==-2 || (state.initialized[k] && (!layout(r,k) || free_slots(r,d)!=capacities[k])))return false;
        if(slot>=0 && !state.initialized[k])return false;
    }
    // A corrupted unrelated registration into the block must also prevent free.
    for(unsigned n=0;n<state.manager_capacity;n++){
        const auto value=word(r,state.table+n*4);
        if(value>=state.allocation && value<state.allocation+block_size && value!=pool(1,0) && value!=pool(1,1))return false;
    }
    if(actor_graph_references(r))return false;
    for(unsigned k=0;k<2;k++)if(state.initialized[k]){
        const uint32_t raw=state.allocation+record_offsets[k];
        for(unsigned n=0;n<capacities[k];n++){
            const uint32_t actor=raw+n*stride+4;
            if(word(r,actor+0x180) || half(r,actor+0x184))return false;
        }
    }
    Registers registers(c);
    // Resource teardown is irreversible; a deferred release must never make
    // these now-destroyed cached slots available to a new owner scope.
    invalidate();
    for(unsigned k=0;k<2;k++)if(state.initialized[k]){
        const auto d=pool(1,k);
        //46790 ->462E0 marks cached actors for destruction (flag+4=0).
        //46730 would instead retain them.46324 performs45B64 resource cleanup.
        c->r4=address(d);func_80046790(r,c);
        c->r4=address(d);func_80046324(r,c);
        if(uint32_t(c->r2)!=1)return false;
        const uint32_t raw=state.allocation+record_offsets[k];
        for(unsigned n=0;n<capacities[k];n++){
            const uint32_t actor=raw+n*stride+4;
            if(word(r,actor+0x16C) || word(r,actor+0x174) || word(r,actor+0x1F4) ||
               word(r,actor+0x180) || half(r,actor+0x184))return false;
        }
    }
    if(actor_graph_references(r) || graphics_references(r))return false;
    for(unsigned k=0;k<2;k++){
        const auto d=pool(1,k);const int slot=registration(r,d);
        if(slot<0)continue;
        const unsigned used=half(r,state.manager+6);if(!used)return false;
        c->r4=address(state.manager);c->r5=unsigned(slot);func_80057BAC(r,c);
        if(uint32_t(c->r2)!=d || word(r,state.table+unsigned(slot)*4)!=0 || half(r,state.manager+6)!=used){invalidate();return false;}
        half(r,state.manager+6,used-1);
    }
    // Never free any memory still referenced by the native manager or globals.
    if(!manager_matches(r) || !outside_primary(r))return false;
    for(unsigned n=0;n<state.manager_capacity;n++){
        const auto value=word(r,state.table+n*4);
        if(value>=state.allocation && value<state.allocation+block_size)return false;
    }
    const auto allocation=state.allocation;
    c->r4=address(allocation);func_80064E84(r,c);
    const bool enabled=state.enabled;state={};state.enabled=enabled;
    fprintf(stderr,"[CoopEggPools] released block=%08X registrations=0\n",allocation);
    return true;
}
inline uint32_t hash_bytes(uint8_t* r,uint32_t p,unsigned count){
    uint32_t h=2166136261u;for(unsigned n=0;n<count;n++)h=(h^byte(r,p+n))*16777619u;return h;
}
inline bool ensure(uint8_t* r,recomp_context* c){
    if(!state.enabled)return true;
    if(state.installed)return matches(r);
    if(!r || !c || state.invalidated || state.allocation || state.depth)return false;
    for(unsigned k=0;k<2;k++)if(word(r,globals[k])!=stock_descriptors[k] || word(r,stock_descriptors[k]+4)!=capacities[k] || word(r,stock_descriptors[k]+8)!=stock_records[k])return false;
    state.primary=stock_descriptors;state.manager=word(r,manager_global);
    if(!range(state.manager,0x18))return false;
    state.table=word(r,state.manager+0x10);state.manager_capacity=half(r,state.manager+8);
    if(!manager_matches(r) || registration(r,state.primary[0])<0 || registration(r,state.primary[1])<0)return false;
    unsigned free=0;for(unsigned n=0;n<state.manager_capacity;n++)if(word(r,state.table+n*4)==0)++free;
    if(free<2 || half(r,state.manager+6)>state.manager_capacity-2)return false;
    const std::array<uint32_t,2> primary_hashes{hash_bytes(r,stock_descriptors[0],0x1410),hash_bytes(r,stock_descriptors[1],0x510)};
    Registers registers(c);
    c->r4=block_size;func_80064DD0(r,c);state.allocation=uint32_t(c->r2);
    if(!heap_block(r)){state.invalidated=true;return false;}
    for(unsigned n=0;n<block_size;n++)byte(r,state.allocation+n,0);
    const unsigned manager_used=half(r,state.manager+6);
    for(unsigned k=0;k<2;k++){
        const auto d=pool(1,k),raw=state.allocation+record_offsets[k];
        c->r4=address(d);c->r5=address(raw);c->r6=capacities[k];func_8004667C(r,c);
        // Mark as initialized only after the complete layout is known. This
        // also permits safe rollback after a later initialization failure.
        if(word(r,d+4)!=capacities[k] || word(r,d+8)!=raw || registration(r,d)<0){invalidate();return false;}
        word(r,d+12,guard|uint32_t(0x4004+k));state.initialized[k]=true;
        c->r4=address(raw);c->r5=capacities[k];func_80039ABC(r,c);
        for(unsigned n=0;n<capacities[k];n++){
            const auto actor=raw+n*stride+4;
            if(byte(r,actor-4)!=0 || byte(r,actor+5)!=0xFF || word(r,actor+0x1F4)!=actor+0x24C || byte(r,actor+0x1F2)!=1){invalidate();return false;}
        }
    }
    if(!manager_matches(r) || half(r,state.manager+6)!=manager_used+2 ||
       primary_hashes!=std::array<uint32_t,2>{hash_bytes(r,stock_descriptors[0],0x1410),hash_bytes(r,stock_descriptors[1],0x510)} || !outside_primary(r)){invalidate();return false;}
    state.installed=true;
    if(!matches(r)){invalidate();return false;}
    fprintf(stderr,"[CoopEggPools] installed block=%08X bytes=%u manager=%08X P1=%08X,%08X P2=%08X,%08X capacity=8,2 primaryByteExact=1\n",state.allocation,block_size,state.manager,state.primary[0],state.primary[1],pool(1,0),pool(1,1));
    return true;
}
inline bool read_room_fifo(uint8_t* r,uint32_t actor,RoomFIFO& out){
 if(!actor){out={};return true;}
 if(!range(actor,0x1FC)||word(r,actor)!=0x19971110||half(r,actor+12)!=0x4001||byte(r,actor+6)>2)return false;
 const auto data=word(r,actor+0x1F4);if(!range(data,0x1550))return false;
 RoomFIFO value;value.actor=actor;value.data=data;value.head=half(r,data+0x12BE);value.count=byte(r,data+0x12C8);if(value.head>=8||value.count>6)return false;
 for(unsigned i=0;i<value.bytes.size();i++)value.bytes[i]=uint8_t(byte(r,data+0x11FC+i));out=value;return true;
}
inline bool fifo_matches(uint8_t* r,const RoomLease& lease){
 for(const auto& captured:lease.fifo){RoomFIFO now;if(!read_room_fifo(r,captured.actor,now)||now!=captured)return false;}return true;
}
inline bool empty_provider(uint8_t* r,uint32_t descriptor,unsigned count){
 const auto storage=word(r,descriptor+8);if(!range(storage,count*stride)||free_slots(r,descriptor)!=count)return false;
 const auto manager=word(r,0x800F8810);if(!range(manager,0x18))return false;const auto cap=half(r,manager+8),table=word(r,manager+0x10);if(!cap||cap>150||!range(table,cap*4))return false;
 for(unsigned i=0;i<count;i++){
  const auto a=storage+i*stride+4;if(word(r,a+0x180)||half(r,a+0x184)||byte(r,a+0x17D)||word(r,a+0x1D8))return false;
  for(unsigned j=0;j<20;j++)if(word(r,a+0x188+j*4))return false;
  for(unsigned j=0;j<cap;j++)if(word(r,table+j*4)==a)return false;
 }
 return true;
}
inline bool capture_room_lease(uint8_t* r,uint64_t epoch,bool empty,const std::array<uint32_t,2>& roots={}){
 if(!state.enabled)return true;if(!epoch||room_lease||!matches(r)||registration(r,state.primary[0])<0||registration(r,state.primary[1])<0)return false;
 RoomLease lease;lease.epoch=epoch;lease.allocation=state.allocation;lease.manager=state.manager;lease.table=state.table;lease.capacity=state.manager_capacity;
 for(unsigned p=0;p<2;p++)if(!read_room_fifo(r,roots[p],lease.fifo[p]))return false;
 const std::array<uint32_t,4> descriptors{state.primary[0],state.primary[1],pool(1,0),pool(1,1)};
 for(unsigned d=0;d<descriptors.size();d++){
  const unsigned owner=d/2,k=d%2,count=capacities[k];const auto storage=word(r,descriptors[d]+8);lease.empty[d]=free_slots(r,descriptors[d])==count;
  if(lease.empty[d]){if(!empty_provider(r,descriptors[d],count))return false;}else if(!roots[owner]||k!=0)return false;
  unsigned busy=0;
  for(unsigned i=0;i<count;i++){
   const auto a=storage+i*stride+4;const unsigned marker=byte(r,a-4);if(marker>1)return false;if(!marker)continue;++busy;
   const auto index=half(r,a+0x17E);const auto& fifo=lease.fifo[owner];
   if(word(r,a)!=0x19971110||half(r,a+12)!=0x4004||byte(r,a+6)!=2||byte(r,a+4)!=1||byte(r,a+5)!=i+1||word(r,a+8)!=descriptors[d]||byte(r,a+0x17D)!=0||word(r,a+0x180)!=fifo.actor||index>=20||word(r,fifo.actor+0x188+index*4)!=a||word(r,a+0x1F4)!=a+0x24C)return false;
   unsigned matches_fifo=0;for(unsigned j=0;j<fifo.count;j++)matches_fifo+=word(r,fifo.data+0x11FC+((fifo.head+j)&7)*24)==a;if(matches_fifo!=1)return false;
  }
  if(k==0&&busy!=lease.fifo[owner].count)return false;
  if(!roots[owner]&&!empty)return false;
  for(unsigned i=0;i<count*stride;i++)lease.records[d][i]=uint8_t(byte(r,storage+i));
 }
 room_lease=std::move(lease);return true;
}
// The coordinator has proved two real carries and destinationReady. Only a
// provider empty BOTH at departure and now may be appended. Retained nonempty
// providers must still be registered with every slot and nativeFIFO unchanged.
inline bool rejoin_empty_room_lease(uint8_t* r,recomp_context* c,uint64_t epoch){
 if(!state.enabled)return true;if(!room_lease||room_lease->epoch!=epoch||!c)return false;
 const auto& lease=*room_lease;
 if(!state.installed||state.invalidated||state.depth||!heap_block(r)||!manager_matches(r)||!outside_primary(r)||word(r,0x800F828C)!=0||state.manager!=0x800F8880||state.table!=0x800D9800||state.manager_capacity!=150||lease.allocation!=state.allocation||lease.manager!=state.manager||lease.table!=state.table||lease.capacity!=state.manager_capacity||!fifo_matches(r,lease))return false;
 for(unsigned k=0;k<2;k++)if(!state.initialized[k]||!layout(r,k)||state.primary[k]!=stock_descriptors[k]||word(r,state.primary[k]+4)!=capacities[k]||word(r,state.primary[k]+8)!=stock_records[k])return false;
 const std::array<uint32_t,4> descriptors{state.primary[0],state.primary[1],pool(1,0),pool(1,1)};std::array<bool,4> missing{};unsigned expected_added=0,preserved_nonempty=0;
 for(unsigned i=0;i<descriptors.size();i++){
  const auto existing=registration(r,descriptors[i]);if(existing==-2)return false;missing[i]=existing==-1;expected_added+=missing[i];const auto count=capacities[i%2];
  if(lease.empty[i]){if(!empty_provider(r,descriptors[i],count))return false;}
  else {
   if(missing[i])return false;++preserved_nonempty;const auto storage=word(r,descriptors[i]+8);
   for(unsigned j=0;j<count*stride;j++)if(lease.records[i][j]!=byte(r,storage+j))return false;
  }
 }
 const unsigned used=half(r,state.manager+6);unsigned free=0,occupied=0;
 for(unsigned i=0;i<state.manager_capacity;i++){
  const auto descriptor=word(r,state.table+i*4);if(!descriptor){++free;continue;}++occupied;if(!range(descriptor,12))return false;
  bool owned=false;for(auto known:descriptors)owned=owned||descriptor==known;if(owned)continue;if(inside_block(descriptor))return false;
  const auto count=word(r,descriptor+4),records=word(r,descriptor+8);const uint64_t end=uint64_t(records)+uint64_t(count)*stride;
  for(const auto span:std::array<std::array<uint32_t,2>,3>{{{state.allocation,block_size},{stock_descriptors[0],0x1410},{stock_descriptors[1],0x510}}})if(count&&uint64_t(records)<uint64_t(span[0])+span[1]&&end>span[0])return false;
 }
 if(free<expected_added||occupied!=used||used>state.manager_capacity-expected_added)return false;
 if(!expected_added){room_lease.reset();return true;}
 std::array<uint8_t,block_size> slab{},primary_bytes{};for(unsigned i=0;i<block_size;i++)slab[i]=uint8_t(byte(r,state.allocation+i));for(unsigned i=0;i<0x1410;i++)primary_bytes[i]=uint8_t(byte(r,stock_descriptors[0]+i));for(unsigned i=0;i<0x510;i++)primary_bytes[0x1410+i]=uint8_t(byte(r,stock_descriptors[1]+i));
 const auto byte_exact=[&](){if(!fifo_matches(r,lease))return false;for(unsigned i=0;i<block_size;i++)if(slab[i]!=byte(r,state.allocation+i))return false;for(unsigned i=0;i<0x1410;i++)if(primary_bytes[i]!=byte(r,stock_descriptors[0]+i))return false;for(unsigned i=0;i<0x510;i++)if(primary_bytes[0x1410+i]!=byte(r,stock_descriptors[1]+i))return false;return true;};
 Registers registers(c);unsigned added=0;
 for(unsigned k=0;k<descriptors.size();k++)if(missing[k]){
  c->r4=address(descriptors[k]);func_8004661C(r,c);++added;
  if(uint32_t(c->r2)!=1||registration(r,descriptors[k])<0||half(r,state.manager+6)!=used+added||!byte_exact())return false;
 }
 if(added!=expected_added||!matches(r)||registration(r,state.primary[0])<0||registration(r,state.primary[1])<0)return false;
 std::fprintf(stderr,"[CoopEggPools] room-rejoined epoch=%llu allocation=%08X manager=%08X registrations=%d,%d primaryRegistrations=%d,%d usedBefore=%u usedAfter=%u added=%u freeP1=%u,%u freeP2=%u,%u slabByteExact=1 primaryByteExact=1 nativeRegistration=8004661C preservedNonempty=%u missingEmpty=%u fifoByteExact=1\n",(unsigned long long)epoch,state.allocation,state.manager,registration(r,pool(1,0)),registration(r,pool(1,1)),registration(r,state.primary[0]),registration(r,state.primary[1]),used,half(r,state.manager+6),added,free_slots(r,state.primary[0]),free_slots(r,state.primary[1]),free_slots(r,pool(1,0)),free_slots(r,pool(1,1)),preserved_nonempty,added);room_lease.reset();return true;
}
class Scope final {
    uint8_t* ram_=nullptr;std::array<uint32_t,2> saved_{};bool active_=false;
public:
    Scope(uint8_t* r,unsigned owner):ram_(r){
        if(!state.enabled)return;
        if(owner>1 || !matches(r) || state.depth>=32){
         if(!rejected_scope_logged){rejected_scope_logged=true;std::fprintf(stderr,"[CoopEggPools] rejected-scope owner=%u enabled=%u installed=%u invalidated=%u depth=%u allocation=%08X heap=%u managerMatch=%u registrations=%d,%d primaryRegistrations=%d,%d layouts=%u,%u globals=%08X,%08X\n",owner,unsigned(state.enabled),unsigned(state.installed),unsigned(state.invalidated),state.depth,state.allocation,unsigned(heap_block(r)),unsigned(manager_matches(r)),registration(r,pool(1,0)),registration(r,pool(1,1)),registration(r,state.primary[0]),registration(r,state.primary[1]),unsigned(layout(r,0)),unsigned(layout(r,1)),word(r,globals[0]),word(r,globals[1]));}
         throw std::runtime_error("Invalid native egg-pool owner scope");
        }
        saved_={word(r,globals[0]),word(r,globals[1])};
        const bool primary=saved_==state.primary;
        const bool secondary=saved_==std::array<uint32_t,2>{pool(1,0),pool(1,1)};
        if(!primary && !secondary)throw std::runtime_error("Unexpected native egg-pool globals");
        for(unsigned k=0;k<2;k++)word(r,globals[k],pool(owner,k));
        ++state.depth;++state.selections;active_=true;
    }
    ~Scope(){
        if(!active_)return;
        // If native scene/heap reset already replaced the epoch, do not write
        // old pointers back into its newly initialized globals.
        if(heap_block(ram_) && manager_matches(ram_))for(unsigned k=0;k<2;k++)word(ram_,globals[k],saved_[k]);
        else invalidate();
        --state.depth;
    }
    Scope(const Scope&)=delete;Scope& operator=(const Scope&)=delete;
};
}
