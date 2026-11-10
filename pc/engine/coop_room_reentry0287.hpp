#pragma once
#ifdef COOP_ROOM_REENTRY0287_PURE
#ifndef COOP_ROOM_RETURN0286_PURE
#define COOP_ROOM_RETURN0286_PURE 1
#define COOP_ROOM_REENTRY0287_LOCAL_PURE 1
#endif
#endif
#include "coop_room_return0286.hpp"
#ifdef COOP_ROOM_REENTRY0287_LOCAL_PURE
#undef COOP_ROOM_RETURN0286_PURE
#undef COOP_ROOM_REENTRY0287_LOCAL_PURE
#endif
namespace workshop::coop_room_reentry0287 {
namespace q=coop_room_return0286;
using Root=q::Root;using Player=q::Player;using Output=q::Output;
// QA only: all native reads are observations. Commands are ordinary pads.
// The first completed43->81 transaction and the genuine death/return precede
// delegation to the unchanged native4209 driver. No receipt is manufactured.
enum class Phase {inactive,entry_align,entry_release,entry_jump,entry_land,entry_on_pot,approach_hazard,seek,await_return,return_settle,reenter,transport,settle,p1_probe,gap,p2_probe,final_settle,complete,rejected};
inline const char* phase_name(Phase p){switch(p){case Phase::inactive:return "inactive";case Phase::entry_align:return "entry-align";case Phase::entry_release:return "entry-release";case Phase::entry_jump:return "entry-jump";case Phase::entry_land:return "entry-land";case Phase::entry_on_pot:return "entry-on-pot";case Phase::approach_hazard:return "approach-hazard";case Phase::seek:return "seek";case Phase::await_return:return "await-return";case Phase::return_settle:return "return-settle";case Phase::reenter:return "reenter";case Phase::transport:return "transport";case Phase::settle:return "settle";case Phase::p1_probe:return "p1-probe";case Phase::gap:return "gap";case Phase::p2_probe:return "p2-probe";case Phase::final_settle:return "final-settle";case Phase::complete:return "complete";case Phase::rejected:return "rejected";}return "rejected";}
// The historical scope has only a lower403A behind P1. Select a real4181
// from the accessible plateau instead; no same-height chase on the slope.
// Shape and native callback identify a contact candidate, not a promised hit.
struct PlateauHazard {uint32_t address=0,private_data=0;uint64_t birth=0;float x=0,y=0;};
template<class Memory> inline std::optional<PlateauHazard> plateau_hazard(const Memory& m,uint32_t a,uint32_t private_data,uint64_t birth){
 if(!birth||!m.range(a,0x1FC)||m.word(a)!=0x19971110u||(m.word(a+0xC)>>16)!=0x4181||m.byte(a+6)>2||m.byte(a+4)||m.byte(a+0x18)!=1||m.byte(a+0x17D)!=2||m.word(a+0x180)||m.word(a+0x1F4)!=private_data||!m.range(private_data,0x80))return std::nullopt;
 const auto state=m.byte(a+0x30);const auto resource=m.word(a+0x168),meta=m.word(a+0x174);
 if(!m.range(resource,0x48)||!m.range(meta,0x18)||m.word(resource+0x3C)!=0x41810100u||m.word(a+0x130)!=0x800483B8u)return std::nullopt;
 const auto base=m.word(resource+0x40),shape=m.word(meta);
 if(!m.range(base,0x2000)||!m.range(shape,0x78)||m.word(resource+0x44)!=base+0x10||m.word(a+0x2C)!=base+0x9A0||m.word(meta+4)||m.word(a+0x134)!=state||!((state==4&&m.word(a+0x20)==base+0xD68)||(state==5&&m.word(a+0x20)==base+0xF14)))return std::nullopt;
 if(m.word(shape+0x1C)!=0x00C10000u||m.word(shape+0x28)!=base+0x6B8||m.half(shape+4)!=6||m.half(shape+6)||m.half(shape+8)||m.half(shape+0xA)!=8||m.half(shape+0xC)!=14||m.half(shape+0xE)!=10)return std::nullopt;
 const float x=std::bit_cast<float>(m.word(a+0x84)),y=std::bit_cast<float>(m.word(a+0x88));
 if(!q::finite(x)||!q::finite(y)||x<4624||x>4912||y<224||y>460)return std::nullopt;
 return PlateauHazard{a,private_data,birth,x,y};
}
struct Frame:q::Frame {unsigned lifecycle_room=0,carried_mask=0;std::array<bool,2> phase_alive{};bool admission_valid=false;std::array<Root,2> admission_roots{};bool entry_pot_valid=false,entry_parent=false,entry_grounded=false,entry_a_held=false,entry_admission_valid=false;Root entry_pot{};float entry_pot_x=0,entry_pot_y=0,entry_vx=0;std::array<Root,2> entry_admission_roots{};};
struct Witness {
 bool handled=false,complete=false,rejected=false,delegated=false,inputs_current=false;
 Phase phase=Phase::inactive;unsigned vi=0,scope_vi=0,death_vi=0,return_vi=0,arrival_vi=0,elapsed=0,input_vi=0,entry_vi=0,entry_attempt=0;
 uint64_t lifecycle_epoch=0;std::array<Root,2> initial{},returned{},roots{};std::array<Output,2> inputs{};std::array<bool,2> control{};
};
inline bool first_room(const Frame& f){return f.room_epoch==1&&f.from==43&&f.to==81&&f.room==81&&f.lifecycle_room==81;}
inline bool second_room(const Frame& f){return f.room_epoch==2&&f.from==81&&f.to==43&&(f.room==81||f.room==43)&&(f.lifecycle_room==81||f.lifecycle_room==43);}
inline bool session(const Frame& f){return f.enabled&&f.bound&&f.bridge_valid&&f.leader==0&&f.lifecycle_epoch&&(first_room(f)||second_room(f));}
inline bool identity_present(const Player& p,const Root& expected){return p.observed&&!p.pending&&!p.native_fatal&&p.root==expected&&q::valid_root(p.root);}
// Initial movement remains native/scripted until the factory has published
// BOTH roots. In particular, the initial pot child must first leave queue0 so
// primary_ready can construct P2. This controller never forges that binding.
class EntryController final {
 enum class Stage {align,release,jump,land};
 bool seen_=false,failed_=false;Stage stage_=Stage::align;unsigned start_=0,last_vi_=0,stage_vi_=0,stable_=0,attempts_=0;uint64_t epoch_=0;float last_y_=0;
 Root pot_{};std::array<Root,2> roots_{};Witness last_{};
 bool context(const Frame& f)const{
  if(!f.enabled||!f.bound||!f.bridge_valid||f.room!=43||f.lifecycle_room!=43||f.room_epoch!=0||!f.entry_pot_valid||f.entry_pot!=pot_||f.lifecycle_epoch!=epoch_||!q::finite(f.entry_vx)||!q::finite(f.entry_pot_x)||!q::finite(f.entry_pot_y))return false;
  for(unsigned p=0;p<2;p++)if(!f.phase_alive[p]||!identity_present(f.players[p],roots_[p]))return false;
  if(f.busy)return f.entry_admission_valid&&f.entry_admission_roots==roots_;
  return q::live(f.players[0])&&q::live(f.players[1]);
 }
 Phase phase(const Frame& f)const{if(f.entry_parent||f.busy)return Phase::entry_on_pot;switch(stage_){case Stage::align:return Phase::entry_align;case Stage::release:return Phase::entry_release;case Stage::jump:return Phase::entry_jump;case Stage::land:return Phase::entry_land;}return Phase::rejected;}
 bool ready(const Frame& f)const{return f.entry_grounded&&!f.entry_a_held&&std::abs(f.players[0].x-f.entry_pot_x)<=6&&std::abs(f.entry_vx)<=.25f;}
public:
 Witness inspect(const Frame& f)const{
  Witness w;w.vi=f.vi;for(unsigned p=0;p<2;p++)w.roots[p]=f.players[p].root;
  if(!seen_||f.room_epoch>0)return w;
  w.entry_vi=start_;w.entry_attempt=attempts_;w.lifecycle_epoch=epoch_;w.initial=roots_;
  w.rejected=failed_||!context(f)||f.vi<start_||f.vi-start_>900;
  if(w.rejected){w.phase=Phase::rejected;return w;}w.handled=true;w.phase=phase(f);
  if(f.vi==last_vi_){w.inputs=last_.inputs;w.control=last_.control;w.inputs_current=true;w.input_vi=last_vi_;w.delegated=last_.delegated;}
  return w;
 }
 Witness step(const Frame& f){
  if(seen_&&f.vi==last_vi_)return inspect(f);
  if(!seen_){
   if(!f.enabled||!f.bound||!f.bridge_valid||f.room!=43||f.lifecycle_room!=43||f.room_epoch||f.busy||!f.entry_pot_valid||!q::live(f.players[0])||!q::live(f.players[1])||!q::finite(f.entry_vx)||!q::finite(f.entry_pot_x)||!q::finite(f.entry_pot_y))return {};
   seen_=true;start_=f.vi;epoch_=f.lifecycle_epoch;pot_=f.entry_pot;roots_={f.players[0].root,f.players[1].root};last_y_=f.players[0].y;
  }
  auto w=inspect(f);if(!w.handled)return w;last_vi_=f.vi;w.input_vi=f.vi;w.inputs_current=true;
  if(f.busy){w.delegated=true;last_=w;return w;}
  w.control={true,true};
  if(f.entry_parent){w.phase=Phase::entry_on_pot;w.inputs[0]={0x0400,0,-1};last_=w;return w;}
  const float dx=f.entry_pot_x-f.players[0].x;
  if(stage_==Stage::align){
   if(ready(f)&&std::abs(f.players[0].y-last_y_)<=.05f){if(stable_<4)++stable_;}else stable_=0;
   if(stable_>=4){stage_=Stage::release;stage_vi_=f.vi;}
   else if(std::abs(dx)>6){if(dx*f.entry_vx<0)w.inputs[0].x=dx>0?.5f:-.5f;else if(std::abs(dx)<12&&std::abs(f.entry_vx)>.75f)w.inputs[0].x=f.entry_vx>0?-.5f:.5f;else w.inputs[0].x=dx>0?.55f:-.55f;}
   else if(std::abs(f.entry_vx)>.25f)w.inputs[0].x=f.entry_vx>0?-.55f:.55f;
  }else if(stage_==Stage::release){
   if(!ready(f)){stage_=Stage::align;stable_=0;}
   else if(f.vi-stage_vi_>=4){if(attempts_>=3)failed_=true;else {++attempts_;stage_=Stage::jump;stage_vi_=f.vi;w.inputs[0].buttons=0x8000;}}
  }else if(stage_==Stage::jump){
   if(f.vi-stage_vi_<32)w.inputs[0].buttons=0x8000;else {stage_=Stage::land;stage_vi_=f.vi;}
  }else if(f.entry_grounded&&std::abs(f.players[0].y-last_y_)<=.05f){stage_=Stage::align;stable_=0;}
  last_y_=f.players[0].y;w.entry_attempt=attempts_;w.phase=failed_?Phase::rejected:phase(f);w.rejected=failed_;if(failed_)w.handled=false;last_=w;return w;
 }
};
class Controller final {
 EntryController entry_;
 bool started_=false,rejected_=false;unsigned scope_vi_=0,death_vi_=0,return_vi_=0,arrival_vi_=0,last_vi_=0;uint64_t epoch_=0;
 std::array<Root,2> initial_{},returned_{};std::array<float,2> directions_{1,1};Witness last_{};
 bool same_session(const Frame& f)const{
  if(!session(f)||f.lifecycle_epoch!=epoch_)return false;
  if(first_room(f))return f.original==initial_;
  return return_vi_&&f.original==returned_;
 }
 bool transit_identity(const Frame& f,unsigned p,const Root& expected)const{
  if(!f.phase_alive[p]||!identity_present(f.players[p],expected))return false;
  if(f.players[p].alive)return true;
  // Unregistered carried roots are not runnable, but genuine coordinator
  // cargo (or the exact fresh4209 admission before begin) retains custody.
  return second_room(f)?f.original==returned_&&bool(f.carried_mask&(1u<<p)):first_room(f)&&f.admission_valid&&f.admission_roots==returned_;
 }
 bool survivor(const Frame& f)const{return f.busy?transit_identity(f,1,initial_[1]):q::live(f.players[1])&&f.players[1].root==initial_[1];}
 bool fresh_pair(const Frame& f)const{return return_vi_&&survivor(f)&&(f.busy?transit_identity(f,0,returned_[0]):q::live(f.players[0])&&f.players[0].root==returned_[0])&&f.players[0].death_vi==death_vi_&&f.players[0].respawn_vi==return_vi_;}
 static Phase arrival_phase(unsigned t){return t<180?Phase::settle:t<420?Phase::p1_probe:t<660?Phase::gap:t<900?Phase::p2_probe:t<1020?Phase::final_settle:Phase::complete;}
 float patrol(unsigned p,float x){if(x<=15)directions_[p]=1;else if(x>=90)directions_[p]=-1;return directions_[p];}
 Witness base(const Frame& f)const{Witness w;w.vi=f.vi;w.scope_vi=scope_vi_;w.death_vi=death_vi_;w.return_vi=return_vi_;w.arrival_vi=arrival_vi_;w.lifecycle_epoch=epoch_;w.initial=initial_;w.returned=returned_;for(unsigned p=0;p<2;p++)w.roots[p]=f.players[p].root;return w;}
public:
 void reset(){*this=Controller{};}
 Witness inspect(const Frame& f)const{
  Witness w=base(f);if(!started_)return entry_.inspect(f);
  w.rejected=rejected_||!same_session(f)||!survivor(f)||f.vi<scope_vi_;
  if(return_vi_&&(f.vi<return_vi_||!fresh_pair(f)))w.rejected=true;
  if(arrival_vi_&&(f.vi<arrival_vi_||!second_room(f)||f.room!=43||!f.room_complete||f.busy||f.lifecycle_room!=43))w.rejected=true;
  if(w.rejected){w.phase=Phase::rejected;return w;}w.handled=true;
  if(arrival_vi_){w.elapsed=f.vi-arrival_vi_;w.phase=arrival_phase(w.elapsed);w.complete=w.phase==Phase::complete;}
  else if(return_vi_){w.elapsed=f.vi-return_vi_;w.phase=f.busy||second_room(f)?Phase::transport:w.elapsed<180?Phase::return_settle:Phase::reenter;}
  else w.phase=death_vi_||f.players[0].native_fatal?Phase::await_return:f.enemy_x?Phase::seek:Phase::approach_hazard;
  if(last_vi_==f.vi){w.inputs=last_.inputs;w.control=last_.control;w.input_vi=last_vi_;w.inputs_current=true;w.delegated=last_.delegated;}
  return w;
 }
 Witness step(const Frame& f){
  if(!started_&&f.room_epoch==0)return entry_.step(f);
  if(started_&&f.vi==last_vi_)return inspect(f);
  if(!started_){
   if(!session(f)||!first_room(f)||!f.room_complete||f.busy||f.players[0].x<4600||f.players[0].x>5000||!q::live(f.players[0])||!q::live(f.players[1])||f.players[0].root!=f.original[0]||f.players[1].root!=f.original[1])return {};
   started_=true;scope_vi_=f.vi;epoch_=f.lifecycle_epoch;initial_=f.original;
  }
  if(f.vi<last_vi_||!same_session(f)||!survivor(f)){rejected_=true;return inspect(f);}
  const auto& victim=f.players[0];
  if(!return_vi_){
   if(!first_room(f)||f.busy)rejected_=true;
   if(victim.pending&&victim.death_vi){if(victim.root!=initial_[0]||victim.death_vi<scope_vi_||victim.death_vi>f.vi||(death_vi_&&victim.death_vi!=death_vi_))rejected_=true;else death_vi_=victim.death_vi;}
   if(death_vi_&&q::live(victim)&&victim.root==initial_[0])rejected_=true;
   if(q::live(victim)&&victim.root!=initial_[0]){
    if(!death_vi_||victim.root.birth==initial_[0].birth||victim.root.color!=initial_[0].color||victim.death_vi!=death_vi_||victim.respawn_vi>f.vi||victim.respawn_vi<death_vi_||victim.respawn_vi-death_vi_<300)rejected_=true;
    else {return_vi_=victim.respawn_vi;returned_={victim.root,initial_[1]};}
   }else if(victim.root!=initial_[0]||(!victim.pending&&!victim.native_fatal&&!q::live(victim)))rejected_=true;
  }
  if(return_vi_&&!arrival_vi_&&second_room(f)&&f.room==43&&f.lifecycle_room==43&&f.room_complete&&!f.busy&&fresh_pair(f))arrival_vi_=f.vi;
  Witness w=inspect(f);last_vi_=f.vi;w.input_vi=f.vi;w.inputs_current=w.handled;
  if(w.handled&&!w.complete){
   if(!return_vi_){
    w.control={true,true};
    // Native map proof: the descending corridor reaches floor416/root401.
    // A typed4181 on that plateau is a bounded experimental contact target.
    // Its real native callback/physics alone decide any damage or death.
    if(!death_vi_&&!victim.native_fatal&&q::live(victim)){
     if(f.enemy_x&&q::finite(*f.enemy_x)){const float dx=*f.enemy_x-victim.x;w.inputs[0].x=std::abs(dx)>2?(dx>0?1.f:-1.f):0.f;}
     else if(victim.x<4912)w.inputs[0].x=1;
    }
    // P2's existing upper platform is native. Release every action and
    // movement so it lands/stops naturally and supplies the factory's real
    // ground/low-velocity anchor. Never jump onto the contact target.
   }else if(!arrival_vi_){
    if(w.phase==Phase::return_settle)w.control={true,true};
    else if(w.phase==Phase::reenter){
     for(unsigned p=0;p<2;p++){
      if(f.players[p].x<6000){w.control[p]=true;w.inputs[p]={uint16_t((f.vi%96)<64?0x8000:0),1,0};}
      else w.delegated=true; // The unchanged old driver owns the real4209.
     }
    }else w.delegated=true; // Busy native admission/transport: do not reject.
   }else {
    w.control={true,true};
    if(w.phase==Phase::p1_probe||w.phase==Phase::p2_probe){
     const unsigned p=w.phase==Phase::p1_probe?0:1;
     // Finish P1's active probe walking left without jumping. The actual
     // native boundary parks it away from the later P2-probe enemy contact;
     // no pose, protection or relaxed idle-motion criterion is supplied.
     if(p==0&&w.elapsed>=330)w.inputs[p]={0,-1,0};
     else w.inputs[p]={uint16_t((f.vi%96)<64?0x8000:0),patrol(p,f.players[p].x),0};
    }
   }
  }else if(w.complete)w.control={true,true};
  last_=w;return w;
 }
};
#ifndef COOP_ROOM_REENTRY0287_PURE
namespace n=coop_native;namespace ids=coop_collision;namespace rn=coop_rooms_native0280;namespace life=coop_lifecycle0280;namespace bridge=coop_bridge;namespace old=coop_room_route_driver0280;
struct HazardMemory {uint8_t* r;bool range(uint32_t a,size_t bytes)const{return n::range(a,bytes);}uint32_t word(uint32_t a)const{return n::word(r,a);}uint8_t byte(uint32_t a)const{return n::byte(r,a);}uint16_t half(uint32_t a)const{return ids::half(r,a);}};
inline std::optional<float> plateau_enemy_x(uint8_t* r){
 const auto manager=n::word(r,0x800F8810);if(!n::range(manager,0x14))return std::nullopt;
 const auto capacity=ids::half(r,manager+8);const auto table=n::word(r,manager+0x10);if(!capacity||capacity>150||!n::range(table,size_t(capacity)*4))return std::nullopt;
 std::optional<PlateauHazard> selected;const HazardMemory memory{r};
 for(unsigned slot=0;slot<capacity;slot++){
  const auto a=n::word(r,table+slot*4);if(!n::actor(r,a,0x4181))continue;const auto id=rn::observed(r,a);if(!id||id->kind!=0x4181)continue;
  const auto candidate=plateau_hazard(memory,a,id->private_data,id->birth);if(candidate&&(!selected||candidate->x<selected->x||(candidate->x==selected->x&&candidate->address<selected->address)))selected=candidate;
 }
 return selected?std::optional<float>{selected->x}:std::nullopt;
}
inline bool enabled=false;inline Controller controller;
inline void configure(bool on){enabled=on;controller.reset();}
inline Frame observe(uint8_t* r,unsigned vi,unsigned room){
 Frame f;f.vi=vi;f.room=room;f.enabled=enabled;if(!r||!enabled)return f;
 const auto& state=life::lifecycle.snapshot();const auto& c=rn::coordinator;const auto& d=c.departure();
 f.from=d.room;f.to=d.route.destination();f.leader=d.leader;f.room_epoch=c.epoch();f.lifecycle_epoch=state.epoch;f.lifecycle_room=state.room;f.room_complete=c.phase()==coop_rooms0280::Phase::complete;f.carried_mask=c.carried();
 f.bound=state.enabled&&state.bound&&!state.failed&&!state.ending;f.bridge_valid=bool(bridge::context)&&!bridge::invalidated&&!n::constructing&&!n::cleaning;f.busy=rn::busy();
 for(unsigned p=0;p<2;p++){
  f.original[p]=q::root(d.payload.players[p].actor,d.payload.players[p].color);const auto& source=state.players[p];auto& target=f.players[p];const auto& id=source.root;
  f.phase_alive[p]=source.phase==life::Phase::alive;target.root={id.address,id.private_data,id.generation,source.color,id.kind};target.death_vi=source.death_vi;target.respawn_vi=source.respawn_vi;target.pending=source.phase==life::Phase::death_pending||source.phase==life::Phase::retiring||source.phase==life::Phase::waiting;
  const auto now=ids::observed_identity(r,id.address);target.observed=now&&*now==id;
  if(target.observed){target.native_fatal=life::fatal(r,id.address);target.alive=life::lifecycle.alive(r,p);target.controllable=old::root_observed(r,p)&&id.address==(p?bridge::bound_secondary:bridge::bound_primary);target.x=std::bit_cast<float>(n::word(r,id.address+0x84));target.y=std::bit_cast<float>(n::word(r,id.address+0x88));}
 }
 if(rn::admission){
  const auto& a=*rn::admission;const auto pot=rn::observed(r,a.pot);coop_rooms0280::Route route;
  if(a.owner==0&&a.life_epoch==state.epoch&&a.room==81&&room==81&&a.source.kind==0x4209&&a.source.address==a.pot&&pot&&*pot==a.source&&rn::source(r,a.pot,route)&&route==a.route&&route.destination()==43){f.admission_valid=true;for(unsigned p=0;p<2;p++)f.admission_roots[p]=q::root(a.payload.players[p].actor,a.payload.players[p].color);}
 }
 if(f.room_epoch==0&&room==43&&state.room==43&&f.bound&&f.bridge_valid){
  const auto manager=n::word(r,0x800F8810);if(n::range(manager,0x14)){
   const auto capacity=ids::half(r,manager+8);const auto table=n::word(r,manager+0x10);
   if(capacity&&capacity<=150&&n::range(table,size_t(capacity)*4))for(unsigned slot=0;slot<capacity;slot++){
    const auto a=n::word(r,table+slot*4);if(!n::actor(r,a,0x4208))continue;const auto id=rn::observed(r,a);coop_rooms0280::Route route;
    if(!id||id->kind!=0x4208||!rn::source(r,a,route)||route.destination()!=81)continue;
    const float x=std::bit_cast<float>(n::word(r,a+0x84)),y=std::bit_cast<float>(n::word(r,a+0x88));if(!q::finite(x)||!q::finite(y)||std::abs(x-6536)>.5f||std::abs(y-296)>16)continue;
    if(f.entry_pot_valid){f.entry_pot_valid=false;break;}f.entry_pot_valid=true;f.entry_pot=q::root(*id,14);f.entry_pot_x=x;f.entry_pot_y=y;
   }
  }
  const auto& player=state.players[0];const auto& id=player.root;
  if(f.entry_pot_valid&&f.players[0].observed&&q::valid_root(f.players[0].root)){
   f.entry_parent=n::word(r,id.address+0x180)==f.entry_pot.address&&n::byte(r,id.address+0x17D)==0&&rn::reciprocal(r,id.address,f.entry_pot.address);
   f.entry_grounded=n::byte(r,id.private_data+0xF4)==0;f.entry_vx=std::bit_cast<float>(n::word(r,id.address+0xC4));f.entry_a_held=n::byte(r,0x800F8995)!=0;
  }
  if(rn::admission){const auto& a=*rn::admission;const auto pot=rn::observed(r,a.pot);coop_rooms0280::Route route;
   if(a.owner==0&&a.life_epoch==state.epoch&&a.room==43&&a.source.kind==0x4208&&a.source.address==a.pot&&pot&&*pot==a.source&&rn::source(r,a.pot,route)&&route==a.route&&route.destination()==81){f.entry_admission_valid=true;for(unsigned p=0;p<2;p++)f.entry_admission_roots[p]=q::root(a.payload.players[p].actor,a.payload.players[p].color);}
  }
 }
 if(first_room(f)&&q::live(f.players[0]))f.enemy_x=plateau_enemy_x(r);
 return f;
}
inline bool input(uint8_t* r,unsigned vi,unsigned room,unsigned player,uint16_t* buttons,float* x,float* y){if(player>1||!buttons||!x||!y)return false;const auto w=controller.step(observe(r,vi,room));if(!w.handled||!w.control[player])return false;*buttons=w.inputs[player].buttons;*x=w.inputs[player].x;*y=w.inputs[player].y;return true;}
inline Witness witness(uint8_t* r,unsigned vi,unsigned room){return controller.inspect(observe(r,vi,room));}
#endif
} // namespace
