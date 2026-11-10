#include "../boss_native.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
namespace b=workshop::boss_native;
static unsigned checks=0;
static void require(bool value,const char* what){++checks;if(!value)throw std::runtime_error(what);}
struct Fixture {
    std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);
    uint8_t* rdram=memory.data();
    static constexpr uint32_t root=0x80010000,priv=0x80011000,head=0x80012000,hp=0x80013000,player=0x80014000,transition=0x80016000,tp=0x80017000;
    recomp_context c{};
    b::Frame f{162,100,0x80400000,0x80500000};
    void w(uint32_t p,uint32_t v){MEM_W(0,b::base(p))=int32_t(v);}
    void by(uint32_t p,unsigned v){MEM_B(0,b::base(p))=uint8_t(v);}
    void actor(uint32_t a,uint16_t kind){w(a,0x19971110);MEM_H(12,b::base(a))=kind;}
    Fixture(){
        actor(root,0x455A);w(root+0x1F4,priv);w(priv,head);actor(head,0x454F);w(head+0x1F4,hp);w(head+0x180,root);w(hp+0x84,12);
        actor(player,0x4001);actor(transition,0x46EE);w(transition+0x1F4,tp);w(0x800F8D78,f.scene_overlay+0x2418);
        require(b::configure(true,"qa-first-boss",true),"configure valid token");b::enter_room(162);
    }
    void entry(const char* name,uint32_t a,uint32_t ra=0,uint32_t a1=0,uint32_t a2=0){c.r4=b::base(a);c.r5=a1;c.r6=a2;c.r31=b::base(ra);b::observe_entry(rdram,&c,name,f);}
    void init(){entry("func_80664E18",root);b::frame(rdram,f);}
    void active(){by(root+0x18,1);by(root+0x30,1);entry("func_806657A0",root);}
    void terminal(){w(hp+0x84,0);w(priv+0x20,1);by(root+0x18,1);by(root+0x30,8);w(root+0x1E0,0);++f.vi;entry("func_80665C80",root);}
    void call(){by(root+0x18,0xFF);by(root+0x17D,3);entry("func_807AEA78",player,f.candy_overlay+0x109C);}
    void ready(){entry("func_807AEA24",transition,f.scene_overlay+0x23AC,1,0);}
    void full(){init();active();terminal();call();ready();}
};
int main(){try{
    {
        Fixture x;x.full();std::vector<uint8_t> before;
        // Freeze RAM after the fixture has simulated native state. Observer
        // calls below must remain read-only, including extraction and cleanup.
        before=x.memory;b::frame(x.rdram,x.f);auto proof=b::take_defeat_observation();
        require(bool(proof),"full native call-chain can produce one observation");require(proof->diagnostic && proof->run_token=="qa-first-boss" && proof->source_room==162 && proof->stage==8,"proof scope/provenance");
        require(proof->initial_segments==12 && proof->remaining_segments==0 && proof->transition_actor==Fixture::transition,"actual source segment and transition identity");
        require(!b::take_defeat_observation(),"proof cannot be consumed twice");b::forget(Fixture::root);require(!b::snapshot().invalidated,"completed observation survives deferred native cleanup");require(x.memory==before,"all observer/extraction/forget operations preserve RAM exactly");
    }
    {Fixture x;b::configure(false,"qa");auto before=x.memory;x.entry("func_80664E18",Fixture::root);b::frame(x.rdram,x.f);require(!b::snapshot().initialized && x.memory==before,"disabled path inert");}
    {Fixture x;require(!b::configure(true,""),"empty token cannot arm");}
    {Fixture x;x.f.source_room=52;x.full();require(!b::take_defeat_observation(),"unremapped root52 cannot impersonate arena162");}
    {Fixture x;x.init();x.terminal();x.call();x.ready();require(!b::take_defeat_observation(),"initialization failure cannot count as active victory");}
    {Fixture x;x.init();x.active();x.entry("func_807AEA78",Fixture::player,x.f.candy_overlay+0x109C);x.ready();require(!b::take_defeat_observation(),"ordinary callback without terminal flag rejected");}
    {Fixture x;x.init();x.active();x.terminal();x.entry("func_807AEA78",Fixture::player,x.f.candy_overlay+0x100);x.ready();require(!b::take_defeat_observation(),"same handler from initialization failure rejected by caller");}
    {Fixture x;x.init();x.active();x.terminal();x.w(Fixture::root+0x1E0,1);x.call();x.ready();require(!b::take_defeat_observation(),"terminal countdown must really complete");}
    for(unsigned offset:{0x18u,0x30u}){Fixture x;x.init();x.active();x.terminal();x.by(Fixture::player+offset,offset==0x18?0xA:0x11);x.call();x.ready();require(!b::take_defeat_observation(),"native death guards respected");}
    {Fixture x;x.init();x.active();x.terminal();x.w(0x800F8D78,0);x.call();x.ready();require(!b::take_defeat_observation(),"unregistered completion handler rejected");}
    {Fixture x;x.init();x.active();x.terminal();x.call();x.entry("func_807AEA24",Fixture::transition,x.f.scene_overlay+0x2400,1,0);require(!b::take_defeat_observation(),"unrelated transition setter rejected");}
    {Fixture x;x.init();x.active();x.terminal();x.call();++x.f.vi;x.ready();require(!b::take_defeat_observation(),"unrelated later-frame transition rejected");}
    {Fixture x;x.init();x.active();x.terminal();x.call();x.w(Fixture::transition+0x1F4,0);x.ready();require(!b::take_defeat_observation(),"transition allocation failure rejected");}
    {Fixture x;x.init();x.active();x.terminal();x.call();b::forget(Fixture::root);x.ready();require(!b::take_defeat_observation(),"address reuse/full cleanup cancels incomplete evidence");}
    {Fixture x;x.init();x.active();b::enter_room(52);b::enter_room(162);x.terminal();x.call();x.ready();require(!b::take_defeat_observation(),"room roundtrip cannot reuse stale boss");}
    {Fixture x;x.init();x.active();x.w(Fixture::root+0x1F4,Fixture::priv+4);b::frame(x.rdram,x.f);require(b::snapshot().invalidated,"private allocation identity change detected");}
    {Fixture x;x.init();x.active();x.w(Fixture::hp+0x84,13);b::frame(x.rdram,x.f);require(b::snapshot().invalidated,"impossible native segment count rejected");}
    {Fixture x;x.w(Fixture::hp+0x84,6);x.full();require(!b::take_defeat_observation(),"unproven smaller/custom candy cannot count as standard");}
    {Fixture x;x.init();x.entry("func_80664E18",Fixture::root);require(b::snapshot().invalidated,"more than one candy initialization rejected");}
    {Fixture x;x.f.candy_overlay=0xFFFFFF00;x.init();x.active();x.terminal();x.call();x.ready();require(!b::take_defeat_observation(),"invalid relocated overlay rejected");}
    {Fixture x;const auto before=x.memory;for(uint32_t a:{0u,0x7FFFFFFFu,0x807FFFF0u,0xFFFFFFFFu}){b::configure(true,"qa");x.entry("func_80664E18",a);b::frame(x.rdram,x.f);require(!b::snapshot().initialized,"out-of-range actor rejected safely");}require(x.memory==before,"malformed actors cannot write RAM");}
    std::cout<<"Boss native observer: "<<checks<<" assertions passed; synthetic RAM only; no native gameplay proof.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
