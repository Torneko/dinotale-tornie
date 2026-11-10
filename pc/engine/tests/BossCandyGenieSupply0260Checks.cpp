#include "../boss_variants0250.hpp"
#include <bit>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace b=workshop::boss_battle0250;
namespace v=workshop::boss_variants0250;
namespace n=workshop::boss_native;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static b::Encounter companion(const b::Config& c={}){
 for(unsigned seed=0;seed<4096;seed++){b::Random r{seed};auto e=b::encounter(7,b::Family::Candy,0,r,c);if(e.genie_companion)return e;}
 throw std::runtime_error("No deterministic companion fixture");
}
int main(){try{
 // Captured from the unmodified header before this fix. The fingerprint covers
 // all40,960 encounters and every original field except the intentional bird
 // quota. This golden predates the explicit Fish3->2HP request too. Only
 // active Fish HP2->3 and Fish delay3000->8000 are normalized here; the exact
 // current HP/timing/quota and every green flag are checked separately.
 uint64_t hash=14695981039346656037ull;unsigned companions=0,ordinary_candy=0;
 auto put=[&](uint32_t value){for(unsigned i=0;i<4;i++){hash^=uint8_t(value>>(8*i));hash*=1099511628211ull;}};
 for(unsigned seed=0;seed<4096;seed++){auto p=b::make_plan(seed);need(b::valid(p),"Existing ten-fight configuration remains valid");put(seed);
  for(const auto&e:p.fights){put(e.number);put(unsigned(e.family));put(e.variant);put(e.terrain);put(e.count);put(e.genie_companion);put(e.purple_mask);put(e.family==b::Family::Fish?8000:e.bird_respawn_ms);put(e.no_flowers);put(e.no_egg_blocks);
   for(const auto&m:e.members){const bool fish=e.family==b::Family::Fish&&(m.kind==0x4511||m.kind==0x4631);put(m.kind);put(fish&&m.hits==2?3:m.hits);put(std::bit_cast<uint32_t>(m.scale));put(m.red);put(m.purple);if(fish)need(m.hits==2,"Every actual campaign Fish preserves the explicit two-HP request");need(!m.green,"Ordinary campaign RNG never silently selects the diagnostic green variant");}
   if(e.family==b::Family::Candy){if(e.genie_companion){++companions;need(e.number!=1&&e.variant==0&&e.count==2,"Only later single Candy can have Genie");need(e.members[0].kind==0x455A&&e.members[0].hits==12&&e.members[1].kind==0x4072&&e.members[1].hits==2,"Real Candy and two-hit Genie remain unchanged");need(e.birds==p.config.reference_birds,"Companion encounter now has renewable native egg supply");}else{++ordinary_candy;need(e.birds==0,"Candy without Genie retains its original supply");}}
   else if(e.family==b::Family::Lips)need(e.birds==p.config.reference_birds,"Every egg-dependent Lips variant now has renewable supply");
   else if(e.family==b::Family::Fish)need(e.birds==p.config.fish_birds&&e.birds==3&&e.bird_respawn_ms==p.config.fish_respawn_ms&&e.bird_respawn_ms==3000,"Fish has exactly three renewable birds and three-second regeneration");
   else if(e.family==b::Family::RedCandy)need(e.birds==p.config.reference_birds&&e.bird_respawn_ms==p.config.bird_respawn_ms,"Final RedCandy reference quota and existing timing remain unchanged");
   else if(e.family==b::Family::Monsters)need(e.birds==(e.variant==0?p.config.reference_birds:e.variant==1?p.config.more_birds:p.config.fewer_birds),"Monster variant supply unchanged");
   else need(e.birds==0,"Other families retain native supply without new quota");
  }
  need(p.fights[0].count==1&&!p.fights[0].genie_companion&&p.fights[0].birds==0,"First Candy remains strictly alone without birds");
 }
 if(hash!=11843694986324822537ull)std::cerr<<"Actual historical fingerprint="<<hash<<" companions="<<companions<<'\n';
 need(hash==11843694986324822537ull&&companions==66,"Original RNG, outcomes, members, scales and terrain unchanged; only requested Fish HP/delay normalized");need(ordinary_candy>4096,"Ordinary Candy variants exercised");
 for(unsigned quota:{2u,3u,7u}){b::Config c;c.reference_birds=quota;c.more_birds=quota+1;need(c.valid(),"Custom reference quota fixture valid");auto e=companion(c);need(e.birds==quota&&e.bird_respawn_ms==c.bird_respawn_ms,"Companion uses configured reference quota and existing respawn timing");for(unsigned variant=0;variant<3;variant++){b::Random r{26};const auto lips=b::encounter(4,b::Family::Lips,variant,r,c);need(lips.birds==quota&&lips.bird_respawn_ms==c.bird_respawn_ms,"All Lips variants use configured reference quota without new timing");}}
 // Exercise the actual existing bird quota adapter with a generated encounter,
 // rather than assigning encounter.birds manually or injecting any eggs.
 const auto e=companion();need(e.birds==2&&v::begin(e,6,{},26),"Generated Candy+Genie begins existing native quota adapter");
 std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();MEM_B(0,n::base(0x800F7EA6))=1;recomp_context context{};context.r29=n::base(0x807C0000);
 v::Frame frame{162,1,true};for(unsigned slot=0;slot<e.birds;slot++)need(v::set_bird_spawn(slot,240.f+240.f*slot,300,500),"Existing router bird spawn points accepted");need(!v::set_bird_spawn(2,240,300,500),"No extra birds beyond quota");
 v::frame(rdram,frame);auto actions=v::take_actions();need(actions.size()==2,"Two native yellow birds requested in companion arena");v::frame(rdram,frame);need(v::take_actions().empty(),"Same-VI fence cannot duplicate requests");
 unsigned calls=0;for(unsigned slot=0;slot<actions.size();slot++){const auto& a=actions[slot];need(a.operation==v::Operation::BirdSpawn&&a.member==slot&&a.arguments[0]==0x45262000,"Quota requests real4526 rather than fabricated egg objects");const auto bird=0x80060000u+slot*0x300u;const auto saved=context;
  need(v::execute(rdram,&context,a,[&](uint32_t fn,uint8_t*,recomp_context* c){++calls;if(fn==0x80047090){need(uint32_t(c->r4)==0x4526,"Native resource lookup uses yellow bird");c->r2=9+slot;}else if(fn==0x8004613C){need(uint32_t(c->r4)==(0x45262009u+slot),"Native factory receives allocated instance");MEM_W(0,n::base(bird))=int32_t(0x19971110);MEM_H(12,n::base(bird))=0x4526;MEM_B(0x17D,n::base(bird))=1;c->r2=n::base(bird);}else if(fn==0x80044194){need(uint32_t(c->r4)==bird,"Actual spawned bird enters native queue");c->r2=1;}else need(false,"Unexpected native operation");}),"Existing spawn/register adapter accepts companion quota");
  need(std::memcmp(&saved,&context,sizeof(context))==0,"Native quota action preserves all guest registers");need(v::renewable_actor(rdram,bird)&&v::renewable_actor(bird),"Spawned renewable bird has existing zero-score provenance");
 }
 need(calls==6,"Each bird uses lookup, native factory, native registration");const uint32_t foreign=0x80061000;MEM_W(0,n::base(foreign))=int32_t(0x19971110);MEM_H(12,n::base(foreign))=0x4526;need(!v::renewable_actor(rdram,foreign),"Unrelated native bird is not relabeled renewable");
 v::forget(0x80060000);need(!v::renewable_actor(0x80060000),"Consumed/disposed bird drops stale score provenance");
 for(unsigned i=0;i<479;i++){++frame.vi;v::frame(rdram,frame);}need(v::take_actions().empty(),"Missing bird waits full existing eight active seconds");++frame.vi;v::frame(rdram,frame);actions=v::take_actions();need(actions.size()==1&&actions[0].operation==v::Operation::BirdSpawn&&actions[0].member==0,"Only depleted bird slot regenerates after eight seconds");
 need(v::take_receipts().empty()&&!v::snapshot().invalidated,"Supply creates neither defeat receipts nor artificial damage");
 // Lips' collision also requires thrown eggs; all three generated variants
 // must reach the same existing native supply adapter, not just set a UI flag.
 for(unsigned variant=0;variant<3;variant++){
  b::Random r{26};const auto lips=b::encounter(4,b::Family::Lips,variant,r,{});
  need(lips.members[0].kind==0x40A3&&lips.members[0].hits==3,"Lips primary and hit count unchanged");
  need(v::begin(lips,3,{},26),"Each generated Lips variant begins native quota adapter");
  for(unsigned slot=0;slot<2;slot++)need(v::set_bird_spawn(slot,128.f+192.f*slot,96,500),"Lips router can provide two arena-relative bird points");
  v::frame(rdram,{165,1,true});auto spawned=v::take_actions();need(spawned.size()==2,"Each Lips variant requests two real yellow birds");
  for(const auto&a:spawned)need(a.operation==v::Operation::BirdSpawn&&a.arguments[0]==0x45262000,"Lips source is native renewable4526, no injected eggs");
  need(v::take_receipts().empty()&&!v::snapshot().invalidated,"Lips supply cannot manufacture damage or completion");
 }
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"plansChecked\":4096,\"companionEncounters\":"<<companions<<",\"unchangedFieldsFNV64\":"<<hash<<",\"gameLaunched\":false,\"nativeGameplayVerified\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
