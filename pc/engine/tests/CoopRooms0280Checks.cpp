// HOST-only coordinator checks. No native game body/scheduler/physics is run.
#include "../coop_rooms0280.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
namespace c=workshop::coop_rooms0280;
static unsigned checks=0,journeys=0;
static void need(bool p,const char* why){++checks;if(!p)throw std::runtime_error(why);}
static uint32_t bits(float value){return std::bit_cast<uint32_t>(value);}
static c::Identity actor(uint32_t p,uint16_t kind,uint64_t birth,uint32_t data=0){return {p,data,birth,kind};}
static c::Departure source(unsigned leader=0,unsigned room=43,unsigned hp=8,unsigned eggs=2,unsigned color=8){
 c::Departure d;d.room=room;d.leader=leader;d.vi=2600;d.caller=c::depart_caller;
 for(unsigned p=0;p<2;++p){
  auto& player=d.payload.players[p];const uint32_t address=0x80100000+p*0x1000;
  player.actor=actor(address,0x4001,100+p,0x80200000+p*0x2000);player.color=(color+p*5)%14;player.hp=p?8-hp:hp;player.count=eggs;
  player.children[0]=actor(0x80104000+p*0x1000,0x400B,110+p);
  player.children[3]=actor(0x80106000+p*0x1000,0x400C,120+p);
  for(unsigned i=0;i<eggs;++i){
   const auto egg=actor(0x80300000+p*0x10000+i*0x400,0x4004,200+p*6+i);
   player.children[5+i]=egg;player.fifo[i]={egg,(color+i)%14,{bits(.75f),bits(1.f),bits(.5f)}};
  }
 }
 auto& shared=d.payload.shared;for(unsigned i=0;i<10;++i)shared.scores[i]=i?i*1024:UINT32_MAX;
 shared.count=6;shared.backup_count=3;shared.fruits={0,6,11,4,8,2};shared.backup={0,6,11};
 for(unsigned i=0;i<6;++i)shared.half_fruits[i]=uint16_t(shared.fruits[i]);
 d.canonical_player=d.payload.players[0].actor.address;d.scoped_player=d.payload.players[leader].actor.address;
 d.source=actor(0x80109000,room==43?0x4208:0x4209,35,0x80208000);d.source_from_player_1c0=d.source.address;
 d.route.address=0x803E0000;d.route.bytes[0]=0x44;d.route.bytes[1]=0xCA;d.route.bytes[2]=0x20;d.route.bytes[4]=uint8_t(room==43?81:43);d.route.bytes[20]=7;d.route.bytes[22]=1;d.route_from_source_260=d.route.address;
 d.readiness.mouth_idle={true,true};return d;
}
struct Run {
 c::Coordinator model; c::Departure d;unsigned vi=2600; c::Overlay overlay{145,0x80400000,22};c::Identity pot=actor(0x8010A000,0x44CA,302,0x80209000);
 explicit Run(c::Departure departure=source()):d(departure){model.configure(true);need(model.begin(d)==c::Decision::accepted,"valid begin");need(model.busy()&&model.epoch()!=0,"transaction busy before any native mutation");}
 c::CarryProof carry(unsigned p)const{return {model.epoch(),d.payload.players[p].actor,0x800F9000,0,0x8005D1C8,true,true};}
 void prepare(){need(model.carried(carry(1-d.leader)),"follower native carry receipt");need(model.carried(carry(d.leader)),"leader native carry receipt");need(model.depart(model.epoch(),c::route_caller,d.route,++vi),"single leader route departure");need(model.enter(model.epoch(),d.route.destination(),7,++vi),"native destination entry");need(model.room_ready(model.epoch(),d.route.destination(),0x8020C000,++vi),"native room ready remains busy");}
 c::Registration registration(unsigned p)const{
  return {model.epoch(),d.route.destination(),vi,d.payload.players[p].actor,pot,overlay,overlay.base+c::registration_offset,d.payload.players[p].actor.address,d.route.target(),(model.arrived()&1)?d.canonical_player:0,0,7};
 }
 c::Arrival arrival(const c::Permit& permit)const{
  c::Arrival a;a.permit=permit;a.room=d.route.destination();a.vi=vi;a.slot=permit.slot;a.queue=2;a.hidden=0;a.parent=0;a.callback=0x800233B4;a.registered_actor=permit.actor.address;a.canonical_player=permit.owner==0||(model.arrived()&1)?d.canonical_player:0;a.original_completed=true;a.local={bits(128.f),bits(152.f),bits(500.f)};a.world=a.local;a.player=d.payload.players[permit.owner];return a;
 }
 void arrive(unsigned p){
  auto proof=registration(p);auto permit=model.registration(proof);need(bool(permit),"exact dynamic warm registration authorized");need(permit->owner==p&&permit->slot==(p==0?5:7)&&permit->method==(p==0?c::Method::canonical:c::Method::generic),"P1 canonical / P2 generic independent of leader");
  need(model.finish_arrival(arrival(*permit)),"native arrival receipt matches payload");need(model.busy(),"first and second arrivals are not a completion fence");
 }
 void finish(){need(model.complete(model.epoch(),d.route.destination(),++vi,d.canonical_player,d.payload,true),"complete exact independent pair + shared payload");need(!model.busy()&&model.phase()==c::Phase::complete,"lifecycle resumes only after pair");++journeys;}
};
static void positive(){
 for(unsigned leader=0;leader<2;++leader)for(unsigned from:{43u,81u})for(unsigned hp=0;hp<=8;++hp)for(unsigned eggs=0;eggs<=6;++eggs)for(unsigned color:{0u,8u,13u})for(unsigned first=0;first<2;++first){
  Run r(source(leader,from,hp,eggs,color));r.prepare();need(!r.model.complete(r.model.epoch(),r.d.route.destination(),r.vi,r.d.canonical_player,r.d.payload,true),"no arrival is not completion");r.arrive(first);
  need(!r.model.complete(r.model.epoch(),r.d.route.destination(),r.vi,r.d.canonical_player,r.d.payload,true),"one arrival is not completion");r.vi+=192;r.arrive(1-first);r.finish();
 }
 // Explicit equal aliases/equal colours, independent HP0/8 and zero shared state.
 for(unsigned color=0;color<14;++color){auto d=source(1,43,0,6,color);d.payload.players[1].color=color;d.payload.shared={};Run r(d);r.prepare();r.arrive(1);r.vi+=192;r.arrive(0);r.finish();}
 Run r;r.prepare();r.arrive(0);r.vi+=192;r.arrive(1);r.finish();const auto old=r.model.epoch();
 auto d=source(1,81,0,0);d.vi=3100;need(r.model.begin(d)==c::Decision::accepted&&r.model.epoch()>old,"fresh reverse trip issues a new epoch");
 need(!r.model.carried({old,d.payload.players[0].actor,0x800F9000,0,0x8005D1C8,true,true}),"previous trip receipt rejected");
 r.model.configure(false);r.model.configure(true);need(r.model.begin(d)==c::Decision::accepted&&r.model.epoch()>old+1,"reconfigure never reuses transaction epoch");
}
static void departure_negatives(){
 auto rejected=[](c::Departure d){c::Coordinator m;m.configure(true);need(m.begin(d)==c::Decision::rejected&&!m.busy(),"bad departure rejected before mutation");};
 auto d=source();c::Coordinator disabled;need(disabled.begin(d)==c::Decision::rejected,"disabled by default");
 d.leader=2;rejected(d);d=source();d.vi=0;rejected(d);d=source();d.caller+=4;rejected(d);d=source();d.scoped_player=d.payload.players[1].actor.address;rejected(d);d=source();d.canonical_player=d.scoped_player+0x1000;rejected(d);
 d=source();d.room=44;rejected(d);d=source();d.route.bytes[4]=82;rejected(d);d=source();d.route.bytes[20]=1;rejected(d);d=source();d.route.bytes[22]=0;rejected(d);d=source();d.route.bytes[1]=0xCB;rejected(d);d=source();d.route.address=0x807FFFF0;d.route_from_source_260=d.route.address;rejected(d);d=source();d.route_from_source_260+=4;rejected(d);d=source();d.source_from_player_1c0+=4;rejected(d);d=source();d.source.birth=0;rejected(d);
 for(unsigned p=0;p<2;++p){
  d=source();d.payload.players[p].hp=9;rejected(d);d=source();d.payload.players[p].color=14;rejected(d);d=source();d.payload.players[p].actor.birth=0;rejected(d);d=source();d.payload.players[p].actor.private_data=0;rejected(d);
  d=source();d.payload.players[p].fifo[0].actor.kind=0x406C;d.payload.players[p].children[5].kind=0x406C;rejected(d);d=source();d.payload.players[p].fifo[0].actor.kind=0x4144;d.payload.players[p].children[5].kind=0x4144;rejected(d);
  d=source();d.payload.players[p].fifo[0].palette=14;rejected(d);d=source();d.payload.players[p].fifo[0].saved_scale[0]=bits(std::numeric_limits<float>::quiet_NaN());rejected(d);d=source();d.payload.players[p].fifo[0].saved_scale[0]=bits(0.f);rejected(d);
  d=source();d.payload.players[p].children[5]={};rejected(d);d=source();d.payload.players[p].children[10]=d.payload.players[p].children[5];rejected(d);d=source();d.payload.players[p].fifo[5]=d.payload.players[p].fifo[0];rejected(d);
 }
 d=source();d.payload.players[1].actor.private_data=d.payload.players[0].actor.private_data;rejected(d);d=source();d.payload.players[1].actor=d.payload.players[0].actor;rejected(d);d=source();d.payload.players[1].children[0]=d.payload.players[0].children[0];rejected(d);
 d=source();d.payload.shared.count=31;rejected(d);d=source();d.payload.shared.backup_count=7;rejected(d);d=source();d.payload.shared.ending=1;rejected(d);d=source();d.payload.shared.fruits[0]=5;rejected(d);d=source();d.payload.shared.half_fruits[0]=3;rejected(d);
 for(unsigned risk=0;risk<8;++risk){
  d=source();if(risk<2)d.readiness.held[risk]=0x80109800;else if(risk<4)d.readiness.mouth_idle[risk-2]=false;else if(risk<6)d.readiness.death_transition[risk-4]=true;else if(risk==6)d.readiness.food_pending=true;else d.readiness.other_scope=true;
  c::Coordinator m;m.configure(true);need(m.begin(d)==c::Decision::deferred&&!m.busy(),"mouth/food/death/scope defers without destructive reset");d.readiness={};d.readiness.mouth_idle={true,true};need(m.begin(d)==c::Decision::accepted&&m.departure().payload==d.payload,"retry retains every fruit/HP/FIFO");
 }
 for(unsigned p=0;p<2;++p){auto transient=source();transient.payload.players[p].children[15]=actor(0x803F0000,0x4016,401);c::Coordinator model;model.configure(true);need(model.begin(transient)==c::Decision::deferred&&!model.busy(),"native4016 cleanup pending defers without erasure");need(transient.payload.players[p].children[15].kind==0x4016,"transient is not retired by HOST coordinator");transient.payload.players[p].children[15]={};need(model.begin(transient)==c::Decision::accepted,"native transient completion allows retry");}
 Run r;auto before=r.model.departure();need(r.model.begin(source(1))==c::Decision::rejected&&r.model.departure().payload==before.payload,"concurrent leader cannot replace transaction");need(r.model.cancel_unstarted(r.model.epoch()),"unmutated transaction can cancel");need(!r.model.busy(),"cancel releases lifecycle pause only before carry");
}
static void transport_negatives(){
 Run r;auto proof=r.carry(0);auto bad=proof;bad.epoch++;need(!r.model.carried(bad),"stale carry epoch");bad=proof;bad.actor.birth++;need(!r.model.carried(bad),"reused root address rejected");bad=proof;bad.actor.private_data+=4;need(!r.model.carried(bad),"private-data drift rejected");bad=proof;bad.function+=4;need(!r.model.carried(bad),"wrong native carry function");bad=proof;bad.parent=1;need(!r.model.carried(bad),"parented root refused");bad=proof;bad.unregistered=false;need(!r.model.carried(bad),"live manager reference is not carry proof");bad=proof;bad.linked=false;need(!r.model.carried(bad),"missing native carry link");
 need(r.model.carried(proof),"first root carry");need(!r.model.carried(proof),"duplicate carry");need(!r.model.cancel_unstarted(r.model.epoch()),"cannot cancel once native mutation begins");need(!r.model.depart(r.model.epoch(),c::route_caller,r.d.route,r.vi),"missing follower stops transition");bad=r.carry(1);bad.list+=0x100;need(!r.model.carried(bad),"different native carry list");need(r.model.carried(r.carry(1)),"second root same carry list");need(!r.model.depart(r.model.epoch(),c::route_caller+4,r.d.route,r.vi),"wrong5D320 caller");auto route=r.d.route;route.bytes[5]=1;need(!r.model.depart(r.model.epoch(),c::route_caller,route,r.vi),"route bytes changed after begin");need(r.model.depart(r.model.epoch(),c::route_caller,r.d.route,++r.vi),"strict departure");need(!r.model.depart(r.model.epoch(),c::route_caller,r.d.route,r.vi),"second leader30B7C forbidden");need(!r.model.enter(r.model.epoch(),43,7,r.vi),"wrong destination");need(!r.model.enter(r.model.epoch(),81,6,r.vi),"wrong native stage");need(r.model.enter(r.model.epoch(),81,7,++r.vi),"native enter");need(!r.model.room_ready(r.model.epoch(),81,0x807FFFF0,r.vi),"out-of-bounds level-info");need(r.model.room_ready(r.model.epoch(),81,0x8020C000,++r.vi),"native ready");
 Run timeout;need(!timeout.model.clock(timeout.vi+c::maximum_duration+1)&&timeout.model.phase()==c::Phase::failed&&timeout.model.busy(),"bounded timeout retains fail-closed busy and payload");need(timeout.model.departure().payload==timeout.d.payload,"failed timeout does not erase carry snapshot");need(timeout.model.begin(source())==c::Decision::rejected,"failed transaction cannot restart over live carry");
 Run rollback;need(!rollback.model.clock(rollback.vi-1)&&rollback.model.busy(),"time reversal fails closed");
}
static void arrival_negatives(){
 need(c::warm_size==0x5A0,"warm bound includes native tables/constants through offset594");
 need(c::valid(c::Overlay{145,0x807FFA60,22}),"full5A0 bytes may end exactly at8MiB boundary");
 need(!c::valid(c::Overlay{145,0x807FFA64,22}),"four-byte data-bound overflow rejected");
 need(!c::valid(c::Overlay{145,0x807FFAC0,22}),"old540-only upper edge must not admit out-of-bounds native constants");
 Run r;r.prepare();auto proof=r.registration(1);auto reject=[&](const c::Registration& bad){need(!r.model.registration(bad),"invalid warm caller/actor/manager proof rejected");};
 auto bad=proof;bad.caller=0x805ABC0C;reject(bad);bad=proof;bad.overlay.section=144;reject(bad);bad=proof;bad.overlay.load=0;reject(bad);bad=proof;bad.overlay.base=0x807FFFFC;bad.caller=bad.overlay.base+c::registration_offset;reject(bad);bad=proof;bad.actor.birth++;reject(bad);bad=proof;bad.actor.private_data+=4;reject(bad);bad=proof;bad.room=43;reject(bad);bad=proof;bad.pot.kind=0x4208;reject(bad);bad=proof;bad.pot.private_data=0;reject(bad);bad=proof;bad.current_in_pot=r.d.payload.players[0].actor.address;reject(bad);bad=proof;bad.cooldown=1;reject(bad);bad=proof;bad.canonical_player=r.d.payload.players[1].actor.address;reject(bad);bad=proof;bad.free_slot=5;reject(bad);bad=proof;bad.free_slot=124;reject(bad);bad=proof;bad.pot_target++;reject(bad);
 auto permit=r.model.registration(proof);need(bool(permit),"pending valid registration");need(!r.model.registration(proof),"pending registration cannot duplicate");auto a=r.arrival(*permit);auto refused=[&](const c::Arrival& p){need(!r.model.finish_arrival(p)&&r.model.arrived()==0&&r.model.busy(),"bad arrival never consumes receipt or ends transaction");};
 auto b=a;b.permit.ticket++;refused(b);b=a;b.permit.overlay.load++;refused(b);b=a;b.original_completed=false;refused(b);b=a;b.slot=5;refused(b);b=a;b.queue=0;refused(b);b=a;b.hidden=1;refused(b);b=a;b.parent=0x80109000;refused(b);b=a;b.callback=0x800233B8;refused(b);b=a;b.registered_actor++;refused(b);b=a;b.player.hp--;refused(b);b=a;b.player.fifo[0].palette++;refused(b);b=a;std::swap(b.player.fifo[0],b.player.fifo[1]);refused(b);b=a;b.player.actor.birth++;refused(b);b=a;b.local[0]=bits(std::numeric_limits<float>::infinity());refused(b);
 need(r.model.finish_arrival(a),"valid arrival after non-consuming rejects");need(!r.model.finish_arrival(a),"arrival receipt single use");r.vi+=192;auto other=r.registration(0);bad=other;bad.pot.birth++;reject(bad);bad=other;bad.overlay.load++;reject(bad);auto next=r.model.registration(other);need(bool(next),"second root same pot and overlay");need(r.model.finish_arrival(r.arrival(*next)),"canonical root arrival");
 auto observed=r.d.payload;observed.shared.scores[0]=0;need(!r.model.complete(r.model.epoch(),81,r.vi,r.d.canonical_player,observed,true),"zero original score is protected too");observed=r.d.payload;observed.shared.fruits[0]=4;observed.shared.half_fruits[0]=4;need(!r.model.complete(r.model.epoch(),81,r.vi,r.d.canonical_player,observed,true),"shared fruit identity must survive");observed=r.d.payload;observed.shared.count=0;observed.shared.backup_count=0;need(!r.model.complete(r.model.epoch(),81,r.vi,r.d.canonical_player,observed,true),"no silent reset accepted");need(!r.model.complete(r.model.epoch(),81,r.vi,r.d.canonical_player,r.d.payload,false),"outer owner/food scopes must finish");need(!r.model.complete(r.model.epoch(),81,r.vi,r.d.payload.players[1].actor.address,r.d.payload,true),"P2 never becomes canonicalslot5");r.finish();
}
int main(){try{positive();departure_negatives();transport_negatives();arrival_negatives();std::cout<<"PASS "<<checks<<" checks / "<<journeys<<" HOST coordinator journeys; no native execution\n";return 0;}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<": "<<e.what()<<"\n";return 1;}}
