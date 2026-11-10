#pragma once
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace workshop::coop_rooms0280 {
// Pure coordinator: receipts below are HOST inputs supplied by the native
// adapter. No RAM is read/written, no native function is called, and no receipt
// proves itself. In particular this header alone does not implement room travel.
inline constexpr uint32_t depart_caller=0x80030BC4,route_caller=0x80030B98;
inline constexpr unsigned warm_section=145,warm_size=0x5A0,registration_offset=0x7C;
inline constexpr unsigned maximum_duration=2400;
inline bool pointer(uint32_t p,size_t bytes){return p>=0x80000400u&&p<0x80800000u&&!(p&3)&&bytes<=0x80800000u-p;}
struct Identity {
 uint32_t address=0,private_data=0;uint64_t birth=0;uint16_t kind=0;
 bool operator==(const Identity&)const=default;
};
inline bool identity(const Identity& p){return pointer(p.address,0x1FC)&&p.birth&&p.kind&&(!p.private_data||pointer(p.private_data,4));}
inline bool root(const Identity& p){return identity(p)&&p.kind==0x4001&&pointer(p.private_data,0x1550);}
struct Reserve {
 Identity actor{};unsigned palette=0;std::array<uint32_t,3> saved_scale{};
 bool operator==(const Reserve&)const=default;
};
struct Player {
 Identity actor{};unsigned color=14,hp=9,count=0;
 // Sparse native child slots preserve both ownership and FIFO actor identity.
 std::array<Identity,20> children{};std::array<Reserve,6> fifo{};
 bool operator==(const Player&)const=default;
};
inline bool valid(const Player& p){
 if(!root(p.actor)||p.color>=14||p.hp>8||p.count>6)return false;
 for(size_t i=0;i<p.children.size();++i){
  const auto& a=p.children[i];if(a==Identity{})continue;
  if(!identity(a)||a.kind==0x4001||a.address==p.actor.address)return false;
  for(size_t j=0;j<i;++j)if(p.children[j].address==a.address)return false;
 }
 for(unsigned i=0;i<p.fifo.size();++i){
  const auto& egg=p.fifo[i];if(i>=p.count){if(egg!=Reserve{})return false;continue;}
  // Portable406C/4144 are refused before travel; they are never discarded or
  // silently reclassified as normal4004 reserves by this first adapter.
  if(!identity(egg.actor)||egg.actor.kind!=0x4004||egg.palette>=14)return false;
  unsigned references=0;for(const auto& child:p.children)if(child==egg.actor)++references;
  if(references!=1)return false;
  for(unsigned j=0;j<i;++j)if(p.fifo[j].actor.address==egg.actor.address)return false;
  for(auto bits:egg.saved_scale){const float value=std::bit_cast<float>(bits);if(!std::isfinite(value)||value<=0||value>4)return false;}
 }
 return true;
}
inline bool fruit(uint32_t p){return p<=4||(p>=6&&p<=11);}
struct Shared {
 std::array<uint32_t,10> scores{};unsigned count=0,backup_count=0,ending=0;
 std::array<uint32_t,30> fruits{},backup{};std::array<uint16_t,30> half_fruits{};
 bool operator==(const Shared&)const=default;
};
inline bool valid(const Shared& p){
 if(p.count>30||p.backup_count>p.count||p.ending)return false;
 for(unsigned i=0;i<p.count;++i)if(!fruit(p.fruits[i])||p.half_fruits[i]!=p.fruits[i])return false;
 for(unsigned i=0;i<p.backup_count;++i)if(!fruit(p.backup[i]))return false;
 return true;
}
struct Snapshot {
 std::array<Player,2> players{};Shared shared{};
 bool operator==(const Snapshot&)const=default;
};
inline bool valid(const Snapshot& p){
 if(!valid(p.players[0])||!valid(p.players[1])||!valid(p.shared))return false;
 const auto& a=p.players[0];const auto& b=p.players[1];
 if(a.actor.address==b.actor.address||a.actor.private_data==b.actor.private_data)return false;
 for(const auto& child:a.children)if(child.address){
  if(child.address==b.actor.address)return false;
  for(const auto& other:b.children)if(child.address==other.address)return false;
 }
 for(const auto& child:b.children)if(child.address==a.actor.address)return false;
 // Equal native aliases are intentional: each HP is an independent OwnerContext
 // value, never inferred twice from the shared eight-entry native health array.
 return true;
}
struct Route {
 uint32_t address=0;std::array<uint8_t,24> bytes{};
 bool operator==(const Route&)const=default;
 unsigned destination()const{return bytes[4];}
 uint32_t target()const{return uint32_t(bytes[0])<<24|uint32_t(bytes[1])<<16|uint32_t(bytes[2])<<8|bytes[3];}
};
inline bool rooms(unsigned from,unsigned to){return (from==43&&to==81)||(from==81&&to==43);}
inline bool valid(const Route& r,unsigned from){
 //5D320 copies24 bytes; stage is byte20, destination byte4.30B7C
 // passes byte22 to2C95C. Restrict this first helper to the real1-1 warm route.
 return pointer(r.address,24)&&rooms(from,r.destination())&&r.bytes[20]==7&&r.bytes[22]==1&&r.target()==0x44CA2000;
}
struct Readiness {
 std::array<uint32_t,2> held{};std::array<bool,2> mouth_idle{false,false},death_transition{};
 bool food_pending=false,other_scope=false;
};
inline bool ready(const Readiness& p){return p.held==std::array<uint32_t,2>{}&&p.mouth_idle[0]&&p.mouth_idle[1]&&!p.death_transition[0]&&!p.death_transition[1]&&!p.food_pending&&!p.other_scope;}
struct Departure {
 unsigned room=0,leader=2,vi=0;uint32_t caller=0,scoped_player=0,canonical_player=0;
 Identity source{};uint32_t source_from_player_1c0=0,route_from_source_260=0;
 Route route{};Snapshot payload{};Readiness readiness{};
};
struct Overlay {
 unsigned section=0;uint32_t base=0;uint64_t load=0;
 bool operator==(const Overlay&)const=default;
};
inline bool valid(const Overlay& p){return p.section==warm_section&&p.load&&pointer(p.base,warm_size);}
struct CarryProof {
 uint64_t epoch=0;Identity actor{};uint32_t list=0,parent=1,function=0;bool linked=false,unregistered=false;
};
struct Registration {
 uint64_t epoch=0;unsigned room=0,vi=0;Identity actor{},pot{};Overlay overlay{};
 uint32_t caller=0,current_in_pot=0,pot_target=0,canonical_player=0;unsigned cooldown=1,free_slot=0;
};
enum class Method {canonical,generic};
struct Permit {
 uint64_t epoch=0,ticket=0;unsigned owner=2,slot=0;Method method=Method::canonical;
 Identity actor{},pot{};Overlay overlay{};
 bool operator==(const Permit&)const=default;
};
struct Arrival {
 Permit permit{};unsigned room=0,vi=0,slot=0,queue=0,hidden=1;
 uint32_t parent=1,callback=0,registered_actor=0,canonical_player=0;
 bool original_completed=false;
 // NativeABBE8 retains responsibility for coordinates, depth, action and graph.
 // These bit patterns are observed after it; they are not requested positions.
 std::array<uint32_t,3> local{},world{};Player player{};
};
enum class Phase {disabled,idle,prepared,departed,loading,arriving,complete,failed};
enum class Decision {accepted,deferred,rejected};
class Coordinator final {
 Phase phase_=Phase::disabled;uint64_t serial_=0,ticket_=0,epoch_=0;
 Departure departure_{};unsigned carried_=0,arrived_=0,last_vi_=0;uint32_t carry_list_=0,level_info_=0;
 std::array<std::optional<Permit>,2> pending_{};std::optional<Identity> destination_pot_{};
 std::optional<Overlay> destination_overlay_{};
 bool time(unsigned vi){if(vi<last_vi_||vi<departure_.vi||vi-departure_.vi>maximum_duration){phase_=Phase::failed;return false;}last_vi_=vi;return true;}
 std::optional<unsigned> owner(const Identity& p)const{for(unsigned i=0;i<2;++i)if(departure_.payload.players[i].actor==p)return i;return {};}
public:
 Phase phase()const{return phase_;}uint64_t epoch()const{return epoch_;}
 unsigned arrived()const{return arrived_;}unsigned carried()const{return carried_;}
 const Departure& departure()const{return departure_;}
 // Failed transactions stay busy/fail-closed until the adapter explicitly
 // disables the entire session. A missing manager slot is NOT a death signal.
 bool busy()const{return phase_==Phase::prepared||phase_==Phase::departed||phase_==Phase::loading||phase_==Phase::arriving||phase_==Phase::failed;}
 void configure(bool enabled){phase_=enabled?Phase::idle:Phase::disabled;departure_={};carried_=arrived_=last_vi_=0;carry_list_=level_info_=0;pending_={};destination_pot_.reset();destination_overlay_.reset();epoch_=0;}
 Decision begin(const Departure& p){
  if(phase_==Phase::disabled||busy()||p.leader>=2||!p.vi||p.caller!=depart_caller||!valid(p.route,p.room)||!valid(p.payload)||!identity(p.source)||p.source.kind==0x4001||p.source.address==p.payload.players[0].actor.address||p.source.address==p.payload.players[1].actor.address||p.source_from_player_1c0!=p.source.address||p.route_from_source_260!=p.route.address||p.scoped_player!=p.payload.players[p.leader].actor.address||p.canonical_player!=p.payload.players[0].actor.address||serial_==UINT64_MAX)return Decision::rejected;
  if(!ready(p.readiness))return Decision::deferred;
  //2C95C calls2C8E8, which natively retires4016 children. Wait for that
  // transient to finish before taking a persistent graph snapshot; never erase
  // it here or weaken the later identity/FIFO conservation check.
  for(const auto& player:p.payload.players)for(const auto& child:player.children)if(child.kind==0x4016)return Decision::deferred;
  departure_=p;epoch_=++serial_;phase_=Phase::prepared;last_vi_=p.vi;carried_=arrived_=0;carry_list_=level_info_=0;pending_={};destination_pot_.reset();destination_overlay_.reset();return Decision::accepted;
 }
 bool cancel_unstarted(uint64_t epoch){if(epoch!=epoch_||phase_!=Phase::prepared||carried_)return false;phase_=Phase::idle;return true;}
 // Adapter records actual5D1C8/queue receipts for BOTH players. It enqueues
 // the follower with2BE60 in its own owner scope; only the leader runs30B7C.
 bool carried(const CarryProof& p){
  if(phase_!=Phase::prepared||p.epoch!=epoch_||p.function!=0x8005D1C8||p.parent||!p.linked||!p.unregistered||!pointer(p.list,0x84))return false;
  const auto slot=owner(p.actor);if(!slot||(carried_&(1u<<*slot))||(carry_list_&&carry_list_!=p.list))return false;
  carry_list_=p.list;carried_|=1u<<*slot;return true;
 }
 bool depart(uint64_t epoch,uint32_t caller,const Route& copied,unsigned vi){
  if(phase_!=Phase::prepared||epoch!=epoch_||caller!=route_caller||copied!=departure_.route||carried_!=3)return false;
  if(!time(vi))return false;phase_=Phase::departed;return true;
 }
 bool enter(uint64_t epoch,unsigned room,unsigned stage,unsigned vi){
  if(phase_!=Phase::departed||epoch!=epoch_||room!=departure_.route.destination()||stage!=7)return false;
  if(!time(vi))return false;phase_=Phase::loading;return true;
 }
 bool room_ready(uint64_t epoch,unsigned room,uint32_t level_info,unsigned vi){
  if(phase_!=Phase::loading||epoch!=epoch_||room!=departure_.route.destination()||!pointer(level_info,0x128))return false;
  if(!time(vi))return false;level_info_=level_info;phase_=Phase::arriving;return true;
 }
 std::optional<Permit> registration(const Registration& p){
  if(phase_!=Phase::arriving||p.epoch!=epoch_||p.room!=departure_.route.destination()||!valid(p.overlay)||p.caller!=p.overlay.base+registration_offset||!identity(p.pot)||p.pot.kind!=0x44CA||!pointer(p.pot.private_data,0x26C)||p.pot_target!=departure_.route.target()||p.current_in_pot!=p.actor.address||p.cooldown||ticket_==UINT64_MAX)return {};
  const auto slot=owner(p.actor);if(!slot||pending_[*slot]||(arrived_&(1u<<*slot)))return {};
  if(p.canonical_player>1&&p.canonical_player!=departure_.payload.players[0].actor.address)return {};
  if(*slot==1&&(p.free_slot<6||p.free_slot>=124))return {};
  if((destination_pot_&&*destination_pot_!=p.pot)||(destination_overlay_&&*destination_overlay_!=p.overlay)||!time(p.vi))return {};
  destination_pot_=p.pot;destination_overlay_=p.overlay;
  Permit permit{epoch_,++ticket_,*slot,*slot==0?5:p.free_slot,*slot==0?Method::canonical:Method::generic,p.actor,p.pot,p.overlay};pending_[*slot]=permit;return permit;
 }
 bool finish_arrival(const Arrival& p){
  if(phase_!=Phase::arriving||p.permit.owner>=2||p.permit.epoch!=epoch_||!pending_[p.permit.owner]||*pending_[p.permit.owner]!=p.permit||p.room!=departure_.route.destination()||!p.original_completed||p.slot!=p.permit.slot||p.registered_actor!=p.permit.actor.address||p.queue!=2||p.hidden||p.parent||p.callback!=0x800233B4||p.player!=departure_.payload.players[p.permit.owner])return false;
  if(p.permit.owner==0&&p.canonical_player!=departure_.payload.players[0].actor.address)return false;
  if(p.permit.owner==1&&p.canonical_player>1&&p.canonical_player!=departure_.payload.players[0].actor.address)return false;
  for(auto bits:p.local)if(!std::isfinite(std::bit_cast<float>(bits)))return false;
  for(auto bits:p.world)if(!std::isfinite(std::bit_cast<float>(bits)))return false;
  if(!time(p.vi))return false;arrived_|=1u<<p.permit.owner;pending_[p.permit.owner].reset();return true;
 }
 // Check at a completed outer frame, with no owner scope/food transaction.
 // Busy inputs/lifecycle must remain paused until this succeeds. If a genuine
 // scene reset changes shared counters, a separate strictly witnessed native
 // reset adapter is needed; this model never manufactures a shared restore.
 bool complete(uint64_t epoch,unsigned room,unsigned vi,uint32_t canonical_player,const Snapshot& observed,bool outer_idle){
  if(phase_!=Phase::arriving||epoch!=epoch_||room!=departure_.route.destination()||arrived_!=3||pending_[0]||pending_[1]||!outer_idle||canonical_player!=departure_.payload.players[0].actor.address||!valid(observed)||observed!=departure_.payload)return false;
  if(!time(vi))return false;phase_=Phase::complete;return true;
 }
 bool clock(unsigned vi){return !busy()||time(vi);}
};
} // namespace workshop::coop_rooms0280
