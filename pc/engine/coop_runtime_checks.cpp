#include "coop_runtime.hpp"
#include <iostream>
#include <map>
using namespace workshop::coop;
void need(bool v,const char* why){if(!v)throw std::runtime_error(why);}
struct Memory final:ScalarMemory {
    std::map<uint32_t,uint32_t> values;
    uint32_t read(uint32_t address)override{return values.at(address);}
    void write(uint32_t address,uint32_t value)override{if(!values.contains(address))throw std::runtime_error("Context wrote outside proven scalars");values[address]=value;}
};
int main(){
 try{
    need(!capabilities.native_second_actor_proven && !capabilities.local_coop_playable && !capabilities.host_join_playable,"Prepared module falsely claims playability");
    Session s;s.start(11);s.bind(0,{0x800C9000,1},8,8);s.bind(1,{0x800CA000,2},9,4);
    need(s.collect(0,{11,1}) && !s.collect(1,{11,1}) && s.fruit_count()==1,"Duplicate fruit credited twice");
    need(!s.collect(0,{10,2}) && !s.collect(0,{11,0}),"Stale or invalid world event admitted");
    need(s.defeat(1,{11,4}) && !s.defeat(0,{11,4}) && s.defeated_count()==1,"Enemy defeat credited twice");
    need(!s.players_collide(0,1),"Player interactions are enabled by default");
    ActorHandle egg{0x800CB000,3};s.projectile(egg,0);
    need(!s.egg_may_affect_teammate(egg,s.player(1).actor),"Egg affects teammate");
    s.forget(egg);need(s.egg_may_affect_teammate({egg.address,4},s.player(1).actor),"Reused actor inherited old egg ownership");
    need(s.input(1,{2,100,0x8000,80,-80},100) && !s.input(1,{1,101,0,0,0},101),"Input sequence replay admitted");
    need(!s.input(1,{3,500,0,0,0},100) && !s.input(1,{3,1,0,0,0},500),"Out-of-window input admitted");
    need(s.die(1,100) && !s.die(1,101) && s.advance(399).empty(),"Respawn timer admitted early/repeated death");
    auto pending=s.advance(400);need(pending.size()==1 && pending[0].player==1 && pending[0].anchor_player==0 && s.advance(401).empty(),"Respawn request not exactly once");
    need(s.defer_respawn(1) && s.advance(402).size()==1,"Unsafe respawn location cannot be deferred");
    need(s.confirm_respawn(1,{0x800CC000,4},4) && s.player(1).alive,"Partner-alive respawn rejected");
    need(s.die(0,500) && s.die(1,501) && s.both_down() && s.advance(1000).empty(),"Both-dead team self-resurrected");
    s.start(12,{Role::host,true,2,1});s.bind(0,{0x800C9000,10},0,8);s.bind(1,{0x800CA000,11},1,8);s.projectile(egg,0);
    need(s.players_collide(0,1) && !s.egg_may_affect_teammate(egg,s.player(1).actor),"Interaction option enabled friendly egg hit");
    need(s.collect(0,{12,8}) && s.collect(1,{12,9}) && s.fruit_target_reached() && !s.collect(1,{12,10}),"Shared fruit target overflowed");
    Session guest;guest.start(12,{Role::guest});guest.bind(0,{0x800C9000,10},0,8);
    need(!guest.collect(0,{12,8}) && guest.apply_host_collection({12,8}) && !guest.apply_host_collection({12,8}) && !guest.die(0,20),"Guest became collection/death authority");
    Memory memory;memory.values[native_player]=0x800C9000;memory.values[native_color]=0;memory.values[custom_color]=8;
    for(unsigned i=0;i<8;i++)memory.values[native_health+i*4]=8-i;
    auto before=memory.values;OwnerContext owner(memory);owner.bind(0,{{0x800C9000,10},8,8});owner.bind(1,{{0x800CA000,11},9,3});
    {
      ScopedOwner first(owner,0);memory.write(native_health,5);
      {ScopedOwner second(owner,1);need(memory.read(native_player)==0x800CA000 && memory.read(native_color)==0 && memory.read(custom_color)==9 && memory.read(native_health)==3,"Hybrid aliases share context");
       memory.write(native_health,1);
       {ScopedOwner nested(owner,0);need(memory.read(native_health)==5,"Nested context lost primary health");memory.write(native_health,6);}
       need(memory.read(native_player)==0x800CA000 && memory.read(native_health)==1,"Nested context lost secondary health");
      }
      need(memory.read(native_player)==0x800C9000 && memory.read(native_health)==6,"Primary view not restored");
    }
    need(owner.depth()==0 && owner.player(0).health==6 && owner.player(1).health==1,"Independent health state not captured");
    before[native_health]=6;need(memory.values==before,"Context changed unrelated world/global fields");
    try{ScopedOwner guarded(owner,1);throw std::runtime_error("synthetic callback failure");}catch(const std::runtime_error&){}
    need(owner.depth()==0 && memory.read(native_player)==0x800C9000 && memory.read(native_health)==6,"Exception leaked a secondary global context");
    std::cout<<"Co-op prepared rules/context PASS: authoritative dedup, eggs ignore teammate, optional body collisions, delayed partner-alive respawn, both-down, replay rejection, aliased-colour nested context and restoration. Native playability remains FALSE.\n";
    return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
