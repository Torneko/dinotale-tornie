// Reuse the existing Runtime fixture's explicit synthetic native boundaries.
// Its broad regression entry point is compiled but is NEVER executed here.
#define main checkpoint_runtime_regression_entry_not_executed
#include "BossCheckpointRuntime0260Checks.cpp"
#undef main
#include <fstream>
#include <iterator>

namespace rf=workshop::boss_rest_floor0260;
namespace data=workshop::boss_rest_data0240;
static constexpr uint32_t retry_table=0x80710000,retry_floor=0x80700000,retry_resource=0x80701000;
static constexpr uint32_t captured_table=0x80101D80;
static std::vector<uint8_t> captured;
static uint32_t captured_word(uint32_t a){uint32_t v=0;std::memcpy(&v,captured.data()+(a&0x7FFFFF),4);return v;}
static unsigned player_slot(){for(unsigned i=0;i<data::object_count;++i)if(workshop::u16(data::rest_objects,i*12)==0x4001)return i;throw std::runtime_error("No4001 source");}
static unsigned floor_slot(){for(unsigned i=0;i<data::object_count;++i)if(workshop::u16(data::rest_objects,i*12)==0x4592)return i;throw std::runtime_error("No4592 source");}
static uint32_t record(unsigned i){return retry_table+i*16;}

struct RetryFixture:Fixture {
 unsigned cp;uint32_t floor_overlay=0x802A13B0;
 explicit RetryFixture(unsigned checkpoint=3,bool consumed=false,bool perform_retry=true,bool witnessed_wake=true):Fixture(0,8,6),cp(checkpoint){
  reach(cp);health(8);set_eggs(6);
  if(witnessed_wake)need(inv::capture(ram,player,150,q::saved_inventory[cp/3])&&
   q::pose::capture(ram,q::saved_inventory[cp/3],q::saved_pose[cp/3])&&
   r::session.awaken_checkpoint(cp),"Explicit host wake receipt stores checkpoint payload and pose through real Session APIs");
  if(perform_retry){
   need(r::session.use_rest_pot(),"The losing attempt starts only after the real Session checkpoint prefix");
   const unsigned source=r::source_for(r::session.snapshot().index);r::enter_room(source);
   need(r::session.enter_fight(r::logical_room,player),"The losing attempt uses its configured physical/logical room");
   set_eggs(5);fatal(0,200,source);leave();
   need(r::session.snapshot().phase==b::Phase::RetryToRest&&q::transport_pending&&
    r::session.snapshot().retry_epoch==1,"Actual Runtime death interception creates a pending checkpoint transport epoch");
   r::enter_room(83);
  }else r::physical_room=r::logical_room=83; // Explicit already-arrived native Rest boundary.
  r::current_vi=201;install(consumed);
 }
 void install(bool consumed){
  for(unsigned i=0;i<data::object_count;++i)for(unsigned j=0;j<4;++j)
   word(ram,record(i)+j*4,captured_word(captured_table+i*16+j*4));
  word(ram,record(player_slot()),consumed?0xFFFFFFFF:0x40010000);
  word(ram,record(floor_slot()),workshop::u32(data::rest_objects,floor_slot()*12));
  word(ram,record(data::checkpoint_index),data::checkpoint_packed);
  actor(ram,retry_floor,0x4592);word(ram,retry_floor+0x168,retry_resource);
  word(ram,retry_resource+0x40,floor_overlay);
  word(ram,retry_resource+0x44,floor_overlay+workshop::boss_floor_persistence0260::init_offset);
  r::overlays[rf::overlay_section]=floor_overlay;
 }
 bool checkpoint_proof(){
  recomp_context context{};context.r4=n::base(data::checkpoint_packed);
  context.r16=n::base(record(data::checkpoint_index));context.r31=n::base(0x8005A010);
  const auto before=memory;const bool ok=q::prove_record(ram,&context);
  need(memory==before,"Checkpoint source proof preserves all8MiB");return ok;
 }
 bool floor_proof(){
  recomp_context context{};context.r4=n::base(retry_floor);context.r5=n::base(record(floor_slot()));
  context.r31=n::base(0x8005A050);std::array<uint32_t,300> sections=r::overlays;
  const auto before=memory;
  // Real Runtime trace adapter calls the actual floor Guard with rest_source.
  r::trace(ram,&context,"func_80048E9C",r::current_vi,sections.data());
  need(memory==before,"Runtime floor trace and Guard capture preserve all8MiB");
  return r::rest_floor_guard.owned(retry_floor);
 }
};

struct NormalFixture:RetryFixture {
 explicit NormalFixture(unsigned completed=4,unsigned epoch=0,bool consumed=false,bool witnessed_wake=true)
  :RetryFixture(3,consumed,epoch!=0,witnessed_wake){
  if(epoch){arrive(83,300);need(!r::failed&&!q::transport_pending&&q::retry_restored_epoch()==1,
   "Actual Runtime arrival performs its synthetic native inventory restoration before a later normal return");}
  while(r::session.snapshot().completed<completed){
   need(r::session.use_rest_pot(),"Normal onward pot advances from the actual Session rest");
   r::enter_room(r::source_for(r::session.snapshot().index));
   need(r::session.enter_fight(r::logical_room,player),"Normal post-checkpoint fight uses its real configured owner room");
   const unsigned index=r::session.snapshot().index;
   for(unsigned member=0;member<r::session.current().count;++member){
    const uint32_t boss=0x80380000+index*0x1000+member*0x200;
    need(r::session.bind(member,boss)&&r::session.defeat({index,member,400+index,boss,true,true,true,true,r::session.snapshot().retry_epoch}),
     "Explicit HOST causal boundary earns every member before the normal return");
   }
   need(r::session.use_victory_pot(),"Only the genuine Session victory permits the normal Rest route");
   r::make_route(ram,83);r::enter_room(83);
   if(r::session.snapshot().completed<completed)need(r::session.arrive_rest(player),"Intermediate normal Rest is admitted before the next earned fight");
  }
  r::current_vi=500;health(5);set_eggs(2);install(consumed);
 }
};

int main(int argc,char** argv){try{
 need(argc==2,"Actual f41 live diagnostic source capture required");
 std::ifstream input(argv[1],std::ios::binary);captured.assign(std::istreambuf_iterator<char>(input),{});
 need(captured.size()==0x800000,"Actual source capture is8MiB");const auto original=captured;
 unsigned changed=0;
 for(unsigned i=0;i<data::object_count;++i){const auto at=captured_table+i*16,source=i*12;
  const auto packed=captured_word(at);need(packed==0xFFFFFFFF||packed==workshop::u32(data::rest_objects,source),
   "Every captured native row retains its immutable type or consumed marker");
  const bool different=captured_word(at+4)!=workshop::u32(data::rest_objects,source+4)||
   captured_word(at+8)!=workshop::u32(data::rest_objects,source+8);
  if(different){++changed;need(i==player_slot()&&captured_word(at+4)==std::bit_cast<uint32_t>(160.f)&&
   captured_word(at+8)==std::bit_cast<uint32_t>(432.f),"The one actual difference is native4001 X160/Y432");}
 }
 need(changed==1,"Actual f41 capture matches30of31 source coordinates exactly");
 for(unsigned cp:{3u,6u,9u})for(bool consumed:{false,true}){
  RetryFixture f(cp,consumed);const auto before=f.memory;const auto proof=q::rest_source(f.ram);
  need(proof&&proof->mode==rf::SourceMode::Retry&&proof->player==Fixture::player&&proof->checkpoint==cp&&proof->epoch==1&&f.memory==before,
   "Runtime authenticates the exact pending checkpoint epoch before arrival restoration");
  inv::Inventory current;need(inv::read(f.ram,Fixture::player,current)&&current.count==0&&
   n::word(f.ram,0x800F8DB4)==0&&!inv::matches(f.ram,q::selected_inventory())&&
   q::inventory_restore.last_epoch()==0,"Current RAM isHP0/R0 while saved payload is8/6; no completed restoration is fabricated");
  need(f.checkpoint_proof(),"Actual checkpoint prove_record accepts native rewritten4001, including rawFFFFFFFF");
  need(f.floor_proof()&&!r::failed,"Real Runtime floor trace accepts the same source proof before native constructor");
 }
 for(unsigned fault=0;fault<19;++fault){RetryFixture f;
  if(fault==0)q::transport_pending=false;
  if(fault==1)q::death_pending=true;
  if(fault==2)q::retries_allowed=false;
  if(fault==3)q::baseline_ready=false;
  if(fault==4)q::pose_restore.reset();
  if(fault==5)word(f.ram,r::route_address,0x44CA0000);
  if(fault==6)byte(f.ram,r::route_address+4,162);
  if(fault==7)q::saved_inventory[1].player+=4;
  if(fault==8)q::saved_pose[1].activation_vi++;
  if(fault==9)q::death_vi=q::selected_inventory().activation_vi;
  if(fault==10)r::current_vi=q::death_vi-1;
  if(fault==11)word(f.ram,0x800F8828,0);
  if(fault==12)word(f.ram,0x807F0008,1);
  if(fault==13)r::logical_room=162;
  if(fault==14)r::physical_room=162;
  if(fault==15)need(r::session.arrive_retry(83,Fixture::player),"Negative normal Rest arrival ends pending source authorization");
  if(fault==16)r::enabled=false;
  if(fault==17)half(f.ram,Fixture::player+12,0x4004);
  if(fault==18)q::saved_inventory[1].activation_vi=0;
  const auto before=f.memory;need(!q::rest_source(f.ram)&&f.memory==before,
   "Foreign, stale, normal-entry or unauthenticated Runtime cannot authorize the checkpoint coordinate");
  need(!f.checkpoint_proof(),"Without authenticated retry, first checkpoint table proof remains strict");
  if(fault!=14&&fault!=16)need(!f.floor_proof(),"Runtime floor caller also refuses the unauthenticated changed player source");
 }
 for(unsigned fault=0;fault<5;++fault){RetryFixture f;
  if(fault==0)word(f.ram,record(player_slot())+4,std::bit_cast<uint32_t>(161.f));
  if(fault==1)word(f.ram,record(player_slot())+8,std::bit_cast<uint32_t>(433.f));
  if(fault==2)word(f.ram,record(player_slot()),0x40012000);
  if(fault==3)word(f.ram,record(floor_slot()+1)+4,std::bit_cast<uint32_t>(999.f));
  if(fault==4)word(f.ram,record(floor_slot()),0x45930000);
  need(bool(q::rest_source(f.ram)),"Negative table fixture retains genuine host transport authorization");
  need(!f.checkpoint_proof()&&!f.floor_proof(),"Even authenticated retry cannot weaken arbitraryXY, type, or other floor rows");
 }
 {NormalFixture f(4,0,false,false);
  word(f.ram,record(player_slot())+4,std::bit_cast<uint32_t>(data::entry_x));
  word(f.ram,record(player_slot())+8,std::bit_cast<uint32_t>(data::entry_y));
  need(!q::rest_source(f.ram)&&f.checkpoint_proof()&&f.floor_proof(),
   "Default normal entry120/432 remains accepted without any retry exception");
 }
 for(unsigned completed:{4u,5u})for(unsigned epoch:{0u,1u})for(bool consumed:{false,true}){
  NormalFixture f(completed,epoch,consumed);
  if(!epoch){q::retries_allowed=false;q::baseline_ready=false;}
  const auto before=f.memory;const auto proof=q::rest_source(f.ram);
  need(proof&&proof->mode==rf::SourceMode::Normal&&proof->checkpoint==3&&proof->completed==completed&&
   proof->epoch==epoch&&f.memory==before,"Normal return retains the exact witnessed source after victories4/5 with no defeat or with one earlier retry");
  need(f.checkpoint_proof()&&f.floor_proof()&&!r::failed,
   "Both actual Runtime callers accept cached160/432 at their first proof, raw4001 or consumedFFFFFFFF");
 }
 {NormalFixture f(4,0,false,false);need(!q::rest_source(f.ram)&&!f.checkpoint_proof()&&!f.floor_proof(),
   "Normal progress without any checkpoint wake never authorizes a copied160/432 record");}
 {NormalFixture f;need(r::session.arrive_rest(Fixture::player)&&r::session.use_rest_pot(),"Negative source fixture really enters ToFight");need(!q::rest_source(f.ram),
   "A wrong phase cannot reuse normal cached-coordinate authorization");}
 {NormalFixture f;need(!rf::checkpoint_player_record(f.ram,0x40010000,std::bit_cast<uint32_t>(160.f),
   std::bit_cast<uint32_t>(432.f),83,3,rf::SourceProof{}),"Default SourceModeNone is entirely inert");}
 {NormalFixture f(4,0,true);need(r::session.arrive_rest(Fixture::player),"Actual Session normal arrival enters Rest");
  const auto proof=q::rest_source(f.ram);need(proof&&proof->mode==rf::SourceMode::Normal&&
   f.checkpoint_proof()&&f.floor_proof(),"Normal Rest phase also admits both first proofs of the cached consumed4001 source");}
 need(captured==original,"Original live diagnostic capture is never modified");
 std::cout<<"BossCheckpointRetrySource0260Checks: "<<checks<<" PASS; real Runtime/Session/getter/trace/prove_record composition,31 actual native source rows; explicit synthetic native transport boundaries, no GPU arrival-restore or gameplay claim.\n";
 return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
