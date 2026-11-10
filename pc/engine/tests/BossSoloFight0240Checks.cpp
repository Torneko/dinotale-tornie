#include "../boss_solo_fight0240.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
namespace b=workshop::boss_native;
namespace s=workshop::boss_solo_fight;
static unsigned checks=0;
static void require(bool value,const char* what){++checks;if(!value)throw std::runtime_error(what);}
struct Fixture {
    std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);
    uint8_t* rdram=memory.data();
    static constexpr uint32_t root=0x80010000,priv=0x80011000,head=0x80012000,hp=0x80013000,player=0x80014000,transition=0x80016000,tp=0x80017000;
    recomp_context c{};b::Frame f{162,100,0x80400000,0x80500000};
    void w(uint32_t p,uint32_t v){MEM_W(0,b::base(p))=int32_t(v);}
    void by(uint32_t p,unsigned v){MEM_B(0,b::base(p))=uint8_t(v);}
    void actor(uint32_t a,uint16_t kind){w(a,0x19971110);MEM_H(12,b::base(a))=kind;}
    Fixture(bool diagnostic=false){
        actor(root,0x455A);w(root+0x1F4,priv);w(priv,head);actor(head,0x454F);w(head+0x1F4,hp);w(head+0x180,root);w(hp+0x84,12);
        actor(player,0x4001);actor(transition,0x46EE);w(transition+0x1F4,tp);w(0x800F8D78,f.scene_overlay+0x2418);
        require(b::configure(true,"solo-first-0240",diagnostic),"observer configuration");b::enter_room(162);
    }
    void entry(const char* name,uint32_t a,uint32_t ra=0,uint32_t a1=0,uint32_t a2=0){c.r4=b::base(a);c.r5=a1;c.r6=a2;c.r31=b::base(ra);b::observe_entry(rdram,&c,name,f);}
    void native_chain(){
        entry("func_80664E18",root);b::frame(rdram,f);
        by(root+0x18,1);by(root+0x30,1);entry("func_806657A0",root);
        w(hp+0x84,0);w(priv+0x20,1);by(root+0x18,1);by(root+0x30,8);w(root+0x1E0,0);++f.vi;entry("func_80665C80",root);
        by(root+0x18,0xFF);by(root+0x17D,3);entry("func_807AEA78",player,f.candy_overlay+0x109C);
        entry("func_807AEA24",transition,f.scene_overlay+0x23AC,1,0);
    }
    b::DefeatObservation proof(){native_chain();const auto receipt=b::take_defeat_observation();require(bool(receipt),"strict existing observer produces proof in complete synthetic chain");return *receipt;}
};
static s::Session fighting(bool diagnostic=false){s::Session session;require(session.begin("solo-first-0240",diagnostic),"begin solo milestone");require(session.enter_room(162),"actual arena entered");return session;}
int main(){try{
    {
        Fixture x;auto proof=x.proof();auto session=fighting();const auto before=x.memory;
        require(session.accept_defeat(proof),"existing complete native proof accepted");require(!session.accept_defeat(proof),"same proof cannot dispatch twice");
        require(!session.arrive_rest(x.rdram,43,Fixture::player),"arrival cannot precede requested transition");
        require(session.begin_departure(43),"only configured rest route admitted");require(session.enter_room(43),"expected room load is not rest-ready proof");require(session.snapshot().phase==s::Phase::departing,"warm restoration still required");
        require(session.arrive_rest(x.rdram,43,Fixture::player),"same living native carried player enters rest");require(!session.arrive_rest(x.rdram,43,Fixture::player),"arrival consumed once");
        require(session.request_next() && !session.request_next(),"onward pot finishes only this milestone once");require(!session.active(),"milestone does not pretend second fight is running");require(x.memory==before,"session never writes native health, progression or any RDRAM");
    }
    {Fixture x;auto session=fighting();x.entry("func_80664E18",Fixture::root);require(!b::take_defeat_observation(),"initialization alone gives no victory");require(!session.begin_departure(43) && !session.request_next(),"no rest or onward route without actual completion");}
    {Fixture x(true);auto proof=x.proof();auto session=fighting(false);require(!session.accept_defeat(proof) && session.snapshot().phase==s::Phase::failed,"QA proof cannot be upgraded to live victory");}
    {Fixture x(false);auto proof=x.proof();auto session=fighting(true);require(!session.accept_defeat(proof),"diagnostic provenance must match both directions");}
    {Fixture x;auto proof=x.proof();auto session=fighting();session.reset();require(!session.accept_defeat(proof),"cancelled session cannot resume from old receipt");require(session.begin("different-run",false) && session.enter_room(162),"new independent run begins");require(!session.accept_defeat(proof),"old token cannot complete a later run");}
    {Fixture x;auto proof=x.proof();proof.initial_segments=6;auto session=fighting();require(!session.accept_defeat(proof),"smaller candy cannot pass as first standard encounter");}
    for(unsigned field=0;field<4;++field){Fixture x;auto proof=x.proof();if(field==0)proof.stage=0;if(field==1)proof.source_room=52;if(field==2)proof.remaining_segments=13;if(field==3)proof.vi=0;auto session=fighting();require(!session.accept_defeat(proof),"wrong native source/count/timing rejected");}
    for(uint32_t pointer:{0u,0x7FFFFFFFu,0x807FFFF0u,0xFFFFFFFFu}){Fixture x;auto proof=x.proof();proof.transition_actor=pointer;auto session=fighting();require(!session.accept_defeat(proof),"malformed transition pointer rejected");}
    {Fixture x;auto proof=x.proof();proof.transition_actor=proof.player;auto session=fighting();require(!session.accept_defeat(proof),"transition cannot alias carried Yoshi");}
    {Fixture x;auto proof=x.proof();auto session=fighting();require(session.accept_defeat(proof) && !session.begin_departure(162),"no accidental route back into defeated first boss");}
    {Fixture x;auto proof=x.proof();auto session=fighting();require(session.accept_defeat(proof) && session.begin_departure(43),"prepare carry failure case");require(!session.arrive_rest(x.rdram,43,Fixture::head),"a recreated or different actor cannot impersonate carried player");}
    for(unsigned offset:{0x18u,0x30u}){Fixture x;auto proof=x.proof();auto session=fighting();require(session.accept_defeat(proof) && session.begin_departure(43),"prepare native death guard");x.by(Fixture::player+offset,offset==0x18?0xA:0x11);require(!session.arrive_rest(x.rdram,43,Fixture::player),"dead player cannot activate rest controls");}
    {Fixture x;auto proof=x.proof();auto session=fighting();require(session.accept_defeat(proof) && session.begin_departure(43),"prepare parent guard");x.w(Fixture::player+0x180,Fixture::head);require(!session.arrive_rest(x.rdram,43,Fixture::player),"attached player cannot impersonate free carried player");}
    {s::Session session;require(!session.begin("invalid token",false),"log-safe unique token required");require(!session.begin("valid",false,162),"rest cannot equal arena");require(!session.begin("valid",false,256),"rest room fits native byte");}
    {auto session=fighting();require(!session.enter_room(52) && !session.active(),"unexpected stage return cancels partial fight");}
    std::cout<<"Boss solo first milestone: "<<checks<<" assertions passed; synthetic native observer chain only, no gameplay claim.\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
