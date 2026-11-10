#pragma once
#include "coop_runtime.hpp"
#include "coop_native_factory.hpp"
#include "coop_native_input.hpp"
#include "coop_native_graphics.hpp"
#include "coop_native_collision.hpp"
#include "coop_native_egg_pools.hpp"
#include "coop_fruit0264.hpp"
#include "coop_lifecycle0280.hpp"
#include "coop_camera_follow0287.hpp"
#include "coop_classic_flower0250.hpp"
#include "coop_escort_phase0281.hpp"
#include "coop_attempt0281.hpp"
#include <memory>
#include <optional>

extern "C" void func_8006CB3C(uint8_t*,recomp_context*);
extern "C" void func_800422CC(uint8_t*,recomp_context*);
extern "C" void func_8002AF8C(uint8_t*,recomp_context*);

namespace workshop::coop_rooms_native0280 {
inline void configure(bool);inline bool busy();inline bool allowed(unsigned);inline bool input_blocked(unsigned);inline bool retaining_carried();inline bool attempt_quiescent();inline void frame(uint8_t*,recomp_context*,unsigned,unsigned);
}
namespace workshop::coop_bridge {
// QA-only adapter. The actor factory and wrappers remain disabled by default.
// This deliberately does not advertise playable co-op or issue progression.
struct Memory final:coop::ScalarMemory{
    uint8_t* ram=nullptr;
    uint32_t read(uint32_t a)override{return coop_native::word(ram,a);}
    void write(uint32_t a,uint32_t v)override{coop_native::word(ram,a,v);}
};
inline Memory memory;
inline std::unique_ptr<coop::OwnerContext> context;
inline uint32_t bound_primary=0,bound_secondary=0;
inline unsigned p1_color=0;
inline unsigned scopes=0,secondary_updates=0,secondary_draws=0;
inline bool invalidated=false,collisions_ready=false;
using Input=bool(*)(int,uint16_t*,float*,float*);
inline Input input=nullptr;
struct Transient {uint32_t mode=0;unsigned form=0;};
inline std::array<Transient,2> transients{};
inline std::array<uint32_t,2> last_held{UINT32_MAX,UINT32_MAX};
inline std::vector<unsigned> owner_stack;
inline std::array<std::vector<coop_collision::Identity>,2> retiring_graphs{};
inline std::array<uint32_t,2> owner_packed{};
// Native4015 death escorts are cached, not deallocated by468E8. Keep the
// exact incarnation and reciprocal pool slot observed before retirement.
struct EscortCache {coop_collision::Identity id{};uint32_t pool=0,storage=0,resource=0,drawing=0,collision=0,overlay=0;unsigned slot=0;uint32_t update=0;unsigned sub=0,index=0;};
inline std::array<std::map<uint32_t,EscortCache>,2> retiring_escorts{};
inline unsigned escort_capture_diagnostics=0;
inline unsigned attempt_wait_diagnostics=0;
inline int attempt_last_wait=-1;
//4004 reserves use the same native468E8 cache return, with embedded private
//data and owner-specific eight-slot pools. A live-resource actor alone is
//never a retirement receipt: capture its exact FIFO incarnation beforehand.
struct ReserveCache {coop_collision::Identity id{};uint32_t pool=0,storage=0,resource=0,drawing=0,collision=0,allocation=0,manager=0,table=0;unsigned owner=2,slot=0,capacity=0;};
inline std::array<std::map<uint32_t,ReserveCache>,2> retiring_reserves{};
//2AF54 ->2AF3C pauses the global camera for a native fatal animation.
//The local-death interceptor skips the normal scene reset that would unpause.
struct DeathPause {coop_collision::Identity root{};uint64_t epoch=0;unsigned room=0;uint32_t camera=0,drawing=0,game_callback=0,camera_callback=0,draw_callback=0;bool deferred=false;};
inline std::array<std::optional<DeathPause>,2> death_pauses{};
inline std::array<std::optional<coop_camera_follow0287::Receipt>,2> drawing_returns{};
inline void lifecycle_frame(uint8_t*,recomp_context*,unsigned,unsigned);
inline bool intercept_death(uint8_t*,recomp_context*,unsigned,unsigned);
inline coop_attempt0281::Identity attempt_identity(const coop_collision::Identity& id){return {id.address,id.private_data,id.generation,id.kind};}
inline bool sync_attempt_binding(){
    const auto& s=coop_lifecycle0280::lifecycle.snapshot();auto& attempts=coop_attempt0281::attempts;
    if(!attempts.enabled()||!s.bound)return true;
    if(attempts.phase()==coop_attempt0281::Phase::awaiting_native_retry)return true;
    return attempts.bind(s.epoch,{attempt_identity(s.players[0].root),attempt_identity(s.players[1].root)},{s.players[0].color,s.players[1].color});
}
inline unsigned available_mask(unsigned mask){return coop_attempt0281::eligible(mask);}
inline void attempt_observation(uint8_t*,recomp_context*,unsigned,unsigned,unsigned,unsigned,unsigned);

inline Transient capture(uint8_t* r){return {coop_native::word(r,0x800F89BC),coop_native::byte(r,0x800F89B4)};}
inline void install(uint8_t* r,Transient t){coop_native::word(r,0x800F89BC,t.mode);coop_native::byte(r,0x800F89B4,t.form);}
inline std::optional<unsigned> owner(uint8_t* r,uint32_t a){
    if(auto flower=coop_classic_flower0250::owner(r,a))return flower;
    if(auto egg=coop_collision::owner_of_egg(r,a))return egg;
    for(unsigned depth=0;depth<16 && coop_native::actor(r,a);depth++){
        if(a==bound_primary)return 0;
        if(a==bound_secondary || coop_native::owns(r,a))return 1;
        const auto parent=coop_native::word(r,a+0x180);if(parent==a)break;a=parent;
    }return std::nullopt;
}
inline void forget(uint32_t a){
    coop_fruit0264::registry.forget(a,&coop_lifecycle0280::lifecycle.snapshot());
    coop_classic_flower0250::forget(a);
    if((a==bound_primary || a==bound_secondary)&&!coop_lifecycle0280::lifecycle.expected_forget(a))invalidated=true;
    coop_collision::forgotten(a);
    coop_native::forget(a);
}
inline void configure(bool enabled,unsigned color,Input provider,bool interactions=false,bool separate_egg_pools=true){
    coop_attempt0281::attempts.configure(enabled,color);
    attempt_wait_diagnostics=0;attempt_last_wait=-1;
    coop_classic_flower0250::configure(enabled);
    coop_fruit0264::registry.configure(enabled);
    coop_lifecycle0280::lifecycle.configure(enabled);coop_rooms_native0280::configure(enabled);retiring_graphs={};retiring_escorts={};retiring_reserves={};death_pauses={};drawing_returns={};owner_packed={};
    context.reset();owner_stack.clear();coop_native_input::reset();bound_primary=bound_secondary=0;
    scopes=secondary_updates=secondary_draws=0;invalidated=false;collisions_ready=false;input=provider;last_held={UINT32_MAX,UINT32_MAX};
    coop_collision::configure(enabled,interactions);
    coop_egg_pools::configure(enabled && separate_egg_pools);
    coop_native::configure(enabled,color);
}
inline void frame(uint8_t* r,recomp_context* c,unsigned vi,unsigned room){
    if(!owner_stack.empty())return;
    if(invalidated || (!coop_rooms_native0280::busy()&&!coop_rooms_native0280::allowed(room)))coop_fruit0264::registry.frame(r,vi,room,false,0,0);
    if(invalidated){
        // Typed native carry receipts keep both retained roots in native custody.
        // A rejected diagnostic must not destroy P2 during scene teardown.
        if(coop_rooms_native0280::retaining_carried())return;
        if(coop_native::owns(r,coop_native::owned_actor()))coop_native::cleanup(r,c);
        if(!coop_native::owns(r,coop_native::owned_actor())){
            context.reset();bound_primary=bound_secondary=0;coop_native::enabled=false;
            coop_egg_pools::release(r,c);
        }
        return;
    }
    if(!coop_native::enabled)return;
    if(context&&coop_rooms_native0280::busy()){coop_rooms_native0280::frame(r,c,vi,room);return;}
    if(context&&coop_lifecycle0280::lifecycle.snapshot().bound){lifecycle_frame(r,c,vi,room);if(coop_lifecycle0280::lifecycle.snapshot().failed){invalidated=true;return;}}
    if(vi>=2400 && room==43 && !coop_native::attempted){
        const auto p=coop_native::word(r,coop::native_player);
        if(coop_native::primary_ready(r,p) && !coop_graphics::ensure(r,c,p)){
            coop_native::attempted=true;coop_native::enabled=false;
            fprintf(stderr,"[CoopBridge] rejected vi=%u reason=graphics-extension\n",vi);return;
        }
        if(coop_native::primary_ready(r,p) && !coop_egg_pools::ensure(r,c)){
            coop_native::attempted=true;coop_native::enabled=false;invalidated=true;
            fprintf(stderr,"[CoopBridge] rejected vi=%u reason=egg-pool-extension\n",vi);return;
        }
    }
    if(!coop_lifecycle0280::lifecycle.snapshot().bound||room==43)coop_native::frame(r,c,vi,room);
    const auto p=coop_native::primary_actor(),q=coop_native::owned_actor();
    if(!context && coop_native::actor(r,p,0x4001) && coop_native::owns(r,q) && coop_native::independent_graph(r,p,q)){
        memory.ram=r;context=std::make_unique<coop::OwnerContext>(memory);
        bound_primary=p;bound_secondary=q;p1_color=coop_native::word(r,coop::custom_color);
        if(p1_color>=14){context.reset();invalidated=true;return;}
        const unsigned hp=coop_native::word(r,coop::native_health+coop::native_alias(p1_color)*4);
        if(hp<1 || hp>8){context.reset();invalidated=true;return;}
        context->bind(0,{{p,1},p1_color,hp});
        context->bind(1,{{q,2},coop_native::secondary_color(),hp});
        transients[0]=transients[1]=capture(r);
        fprintf(stderr,"[CoopBridge] bound vi=%u primary=%08X secondary=%08X colors=%u,%u health=%u,%u\n",vi,p,q,p1_color,coop_native::secondary_color(),hp,hp);
    }
    if(context && !invalidated){
        const auto data1=coop_native::actor(r,bound_primary,0x4001)?coop_native::word(r,bound_primary+0x1F4):0,data2=coop_native::actor(r,bound_secondary,0x4001)?coop_native::word(r,bound_secondary+0x1F4):0;
        if(coop_native::range(data1,0x1550) && coop_native::range(data2,0x1550)){
            const std::array<uint32_t,2> held{coop_native::word(r,data1+0x164),coop_native::word(r,data2+0x164)};
            if(held!=last_held){
                last_held=held;
                fprintf(stderr,"[CoopFood] held vi=%u root1=%08X root2=%08X p1=%08X p2=%08X phase1=%u phase2=%u\n",vi,bound_primary,bound_secondary,held[0],held[1],coop_native::byte(r,data1+0x159),coop_native::byte(r,data2+0x159));
            }
        }
        const bool roots_ready=coop_collision::bind_root(r,0,bound_primary) && coop_collision::bind_root(r,1,bound_secondary);
        if(roots_ready){
            for(auto root:{bound_primary,bound_secondary})for(auto child:coop_native::tree(r,root)){
                const auto kind=coop_native::kind(r,child);
                if(kind==0x400B || kind==0x400C)coop_collision::register_body(r,child);
                else if(kind==0x4002)coop_collision::attached(r,child,coop_native::word(r,child+0x180));
            }
        }
        if(roots_ready&&!coop_lifecycle0280::lifecycle.snapshot().bound){
            owner_packed={coop_native::word(r,bound_primary+12),coop_native::word(r,bound_secondary+12)};
            if(!coop_lifecycle0280::lifecycle.bind(r,bound_primary,bound_secondary,p1_color,coop_native::secondary_color(),room,vi)){invalidated=true;return;}
            if(!sync_attempt_binding()){coop_lifecycle0280::lifecycle.fail("attempt_binding_rejected");invalidated=true;return;}
        }
        const auto& life=coop_lifecycle0280::lifecycle.snapshot();
        if(life.bound&&!life.ending&&(life.players[0].phase!=coop_lifecycle0280::Phase::alive||life.players[1].phase!=coop_lifecycle0280::Phase::alive))coop_fruit0264::registry.single_frame(r,vi,room,life);
        else coop_fruit0264::registry.frame(r,vi,room,roots_ready,bound_primary,bound_secondary,&life);
        if(roots_ready!=collisions_ready){collisions_ready=roots_ready;fprintf(stderr,"[CoopContacts] bindings ready=%u primary=%08X secondary=%08X\n",unsigned(roots_ready),bound_primary,bound_secondary);}
        if(vi%120==0){
            const auto& stats=coop_collision::statistics();
            fprintf(stderr,"[CoopContacts] frame vi=%u primary=%08X secondary=%08X parent1=%08X parent2=%08X hp=%u,%u bodyReject=%llu eggReject=%llu ready=%u interactions=%u\n",vi,bound_primary,bound_secondary,coop_native::actor(r,bound_primary,0x4001)?coop_native::word(r,bound_primary+0x180):0,coop_native::actor(r,bound_secondary,0x4001)?coop_native::word(r,bound_secondary+0x180):0,context->player(0).health,context->player(1).health,(unsigned long long)stats.body_rejections,(unsigned long long)stats.egg_rejections,unsigned(roots_ready),unsigned(coop_collision::registry.interactions()));
        }
        uint16_t buttons=0;float x=0,y=0;if(input)input(1,&buttons,&x,&y);
        coop_native_input::update({buttons,x,y});
        if(vi%120==0)fprintf(stderr,"[CoopBridge] frame vi=%u updates=%u draws=%u scopes=%u health=%u,%u global=%08X depth=%u p2buttons=%04X p2stick=%.2f,%.2f\n",vi,secondary_updates,secondary_draws,scopes,context->player(0).health,context->player(1).health,coop_native::word(r,coop::native_player),context->depth(),buttons,x,y);
    }
}
struct Scope{
    uint8_t* ram;unsigned index;Transient saved;bool swapped;
    std::unique_ptr<coop_native_input::Scope> pad;
    std::unique_ptr<coop_egg_pools::Scope> egg_pools;
    Scope(uint8_t* r,recomp_context* c,unsigned p):ram(r),index(p),saved(capture(r)),swapped(owner_stack.empty() || owner_stack.back()!=p){
        context->enter(p);
        try{
            egg_pools=std::make_unique<coop_egg_pools::Scope>(r,p);
            if(swapped){transients[owner_stack.empty()?0:owner_stack.back()]=saved;install(r,transients[p]);pad=std::make_unique<coop_native_input::Scope>(r,c,p==1,coop_rooms_native0280::input_blocked(p));}
            owner_stack.push_back(p);scopes++;
        }catch(...){
            egg_pools.reset();pad.reset();install(r,saved);context->leave();
            const auto active=coop_native::word(r,coop::native_player);
            if(!coop_native::actor(r,active,0x4001))coop_native::word(r,coop::native_player,coop_native::actor(r,bound_primary,0x4001)?bound_primary:0);
            throw;
        }
    }
    ~Scope(){
        transients[index]=capture(ram);owner_stack.pop_back();egg_pools.reset();pad.reset();
        if(swapped)install(ram,owner_stack.empty()?transients[0]:transients[owner_stack.back()]);
        context->leave();
        const auto active=coop_native::word(ram,coop::native_player);
        if(!coop_native::actor(ram,active,0x4001))coop_native::word(ram,coop::native_player,coop_native::actor(ram,bound_primary,0x4001)?bound_primary:coop_native::actor(ram,bound_secondary,0x4001)?bound_secondary:0);
    }
};
inline uintptr_t enter(uint8_t* r,recomp_context* c,uint32_t fn,uint32_t a,uint32_t arg){
    if(!context || coop_native::constructing || coop_native::cleaning)return 0;
    // Lifetime callbacks are handled by the factory's native cleanup guard;
    // installing a soon-to-be-freed owner here would leave a dangling global.
    if(fn==0x800439F4 || fn==0x80045B64 || fn==0x8003B8F0)return 0;
    auto p=owner(r,(fn==0x80039F48 || fn==0x8003FFD8)?arg:a);if(!p || !coop_native::actor(r,*p==0?bound_primary:bound_secondary,0x4001))return 0;
    if(*p==1 && fn==0x80043450)secondary_updates++;
    if(*p==1 && fn==0x8005E884)secondary_draws++;
    return reinterpret_cast<uintptr_t>(new Scope(r,c,*p));
}
inline void leave(uintptr_t ticket)noexcept{delete reinterpret_cast<Scope*>(ticket);}
// These functions run only on the native game thread. The death hook never
// destroys actors on a dispatcher stack. Actual cleanup/reconstruction waits
// for the completed675F0 fence and every owner scope to close.
inline bool intercept_death(uint8_t* r,recomp_context* c,unsigned vi,unsigned room){
    if(!context||invalidated||owner_stack.empty()||coop_native::constructing||coop_native::cleaning)return false;
    const auto who=owner_stack.back();if(who>1||uint32_t(c->r4)!=(who?bound_secondary:bound_primary))return false;
    if(!sync_attempt_binding())return false;
    const bool intercepted=coop_lifecycle0280::lifecycle.intercept(r,c,vi,room);
    const auto& state=coop_lifecycle0280::lifecycle.snapshot();
    if(state.bound&&state.ending)coop_attempt0281::attempts.finish({state.epoch,{attempt_identity(state.players[0].root),attempt_identity(state.players[1].root)},true,false});
    if(intercepted&&!state.failed&&!state.ending&&state.players[who].phase==coop_lifecycle0280::Phase::death_pending&&state.players[who].death_vi==vi&&state.room==room&&!coop_rooms_native0280::busy()&&coop_native::byte(r,0x800F7EA5)==1&&coop_native::byte(r,0x800F7EB4)==1){
        const auto camera=coop_native::word(r,0x800F8294),drawing=coop_native::word(r,0x800F8318);
        if(coop_native::range(camera,0xB0)&&coop_native::range(drawing,0xF8)&&coop_native::word(r,camera+0x80)==drawing&&coop_native::word(r,0x800F7EB8)==0x80022E4C){
            death_pauses[who]=DeathPause{state.players[who].root,state.epoch,room,camera,drawing,coop_native::word(r,0x800F7EB8),coop_native::word(r,camera+0x78),coop_native::word(r,drawing+0xF4)};
            drawing_returns[who]=coop_camera_follow0287::capture(r,state,who,vi,room);
        }
    }
    return intercepted;
}
inline bool release_death_pause(uint8_t* r,recomp_context* c,unsigned owner,unsigned vi,unsigned room){
    namespace life=coop_lifecycle0280;if(owner>1||!death_pauses[owner])return true;
    const auto receipt=*death_pauses[owner];const auto& state=life::lifecycle.snapshot();
    if(!context||!c||invalidated||!owner_stack.empty()||context->depth()||coop_egg_pools::snapshot().depth||coop_native_input::depth||coop_native::constructing||coop_native::cleaning||coop_rooms_native0280::busy()||coop_rooms_native0280::retaining_carried()||state.failed||state.ending||!state.bound||state.epoch!=receipt.epoch||state.room!=room||room!=receipt.room||(room!=43&&room!=81)||state.players[owner].phase!=life::Phase::waiting||state.players[owner].root!=receipt.root||!state.players[owner].death_vi||vi<state.players[owner].death_vi)return false;
    if(!life::lifecycle.alive(r,1-owner)){
        const auto& other=state.players[1-owner];const auto now=coop_collision::observed_identity(r,other.root.address);
        if(other.phase!=life::Phase::alive||!now||*now!=other.root||!life::fatal(r,other.root.address))return false;
        if(!receipt.deferred)std::fprintf(stderr,"[CoopLife] camera-resume-deferred owner=%u partner=%08X vi=%u epoch=%llu reason=partner_native_fatal\n",owner,other.root.address,vi,(unsigned long long)receipt.epoch);
        death_pauses[owner]->deferred=true;return true; // Await its real fatal callback; never infer an ending.
    }
    if(coop_native::word(r,0x800F8294)!=receipt.camera||coop_native::word(r,0x800F8318)!=receipt.drawing||!coop_native::range(receipt.camera,0xB0)||!coop_native::range(receipt.drawing,0xF8)||coop_native::word(r,receipt.camera+0x80)!=receipt.drawing||coop_native::word(r,0x800F7EB8)!=receipt.game_callback||coop_native::word(r,receipt.camera+0x78)!=receipt.camera_callback||coop_native::word(r,receipt.drawing+0xF4)!=receipt.draw_callback)return false;
    const auto mode=coop_native::byte(r,0x800F7EA5),paused=coop_native::byte(r,0x800F7EB4);
    if(!mode&&!paused){death_pauses[owner].reset();return true;} // Already released by the native game.
    if(mode!=1||paused!=1)return false;
    {coop_native::ScopedContext registers(c);func_8002AF8C(r,c);}
    if(coop_native::byte(r,0x800F7EA5)||coop_native::byte(r,0x800F7EB4))return false;
    death_pauses[owner].reset();
    std::fprintf(stderr,"[CoopLife] camera-resumed owner=%u actor=%08X oldBirth=%llu anchor=%08X camera=%08X drawing=%08X vi=%u epoch=%llu native2AF8C=1 flags=0,0\n",owner,receipt.root.address,(unsigned long long)receipt.root.generation,state.players[1-owner].root.address,receipt.camera,receipt.drawing,vi,(unsigned long long)receipt.epoch);
    return true;
}
inline uint32_t progress_hash(uint8_t* r){
    uint32_t hash=2166136261u;
    // Shared native score/fruit/backup/half-fruit data, read only. Source
    //2D0B4 and6CFC0 establish these spans; no synthetic progress restore.
    for(unsigned i=0;i<10;i++)hash=(hash^coop_native::word(r,0x800F8950+i*4))*16777619u;
    for(unsigned i=0;i<0x13C;i++)hash=(hash^coop_native::byte(r,0x800F89D0+i))*16777619u;
    return hash;
}
inline void publish_anchor(uint8_t* r){
    const auto anchor=coop_lifecycle0280::lifecycle.anchor(r);if(!anchor||!context)return;
    const auto& view=context->player(*anchor);
    coop_native::word(r,coop::native_player,view.actor.address);coop_native::word(r,coop::native_color,coop::native_alias(view.color));coop_native::word(r,coop::custom_color,view.color);
    coop_native::word(r,coop::native_health+coop::native_alias(view.color)*4,view.health);
}
inline bool escort_cache_capture(uint8_t* r,const coop_collision::Identity& id,uint32_t root,EscortCache& result){
    const auto a=id.address;const auto now=coop_collision::observed_identity(r,a);
    const auto rejected=[&](const char* reason){
        if(escort_capture_diagnostics++<4){
            const bool valid=coop_native::range(a,0x1FC);const auto res=valid?coop_native::word(r,a+0x168):0;
            std::fprintf(stderr,"[CoopLife] escort-capture-rejected reason=%s actor=%08X birth=%llu root=%08X private=%08X life=%u flag=%u primary=%u sub=%u update=%08X index=%u pool=%08X slot=%u resource=%08X overlay=%08X parent=%08X queue=%u children=%u\n",reason,a,(unsigned long long)id.generation,root,id.private_data,valid?coop_native::byte(r,a+6):0,valid?coop_native::byte(r,a+4):0,valid?coop_native::byte(r,a+0x18):0,valid?coop_native::byte(r,a+0x30):0,valid?coop_native::word(r,a+0x20):0,valid?coop_native::word(r,a+0x1E0):0,valid?coop_native::word(r,a+8):0,valid?coop_native::byte(r,a+5):0,res,coop_native::range(res,0x48)?coop_native::word(r,res+0x40):0,valid?coop_native::word(r,a+0x180):0,valid?coop_native::byte(r,a+0x17D):0,valid?coop_collision::half(r,a+0x184):0);
        }return false;
    };
    if(!now||*now!=id||id.kind!=0x4015||id.private_data)return rejected("identity");
    if(coop_native::byte(r,a+4)||coop_native::byte(r,a+6)!=2||coop_native::byte(r,a+0x17D)||coop_native::byte(r,a+0x18)||coop_native::word(r,a+0x180)!=root||coop_collision::half(r,a+0x184)||!coop_lifecycle0280::fatal(r,root))return rejected("native-attached-state");
    const auto pool=coop_native::word(r,a+8),slot=coop_native::byte(r,a+5),resource=coop_native::word(r,a+0x168);
    if(!coop_native::range(pool,12)||slot<1||slot>4||coop_native::word(r,pool+4)!=4||!coop_native::range(resource,0x48)||coop_collision::half(r,resource+0x3C)!=0x4015)return rejected("pool-resource");
    const auto storage=coop_native::word(r,pool+8),overlay=coop_native::word(r,resource+0x40);
    if(!coop_native::range(storage,4*0x280)||storage+(slot-1)*0x280+4!=a||coop_native::byte(r,a-4)!=1||!coop_native::range(overlay,0x1220)||coop_native::word(r,resource+0x44)!=overlay+0xB48||coop_native::word(r,a+0x2C)!=overlay+0xAC0)return rejected("pool-slot-factory");
    const auto update=coop_native::word(r,a+0x20),sub=coop_native::byte(r,a+0x30),index=coop_native::word(r,a+0x1E0);
    if(!coop_escort_phase0281::matches(overlay,update,sub,index))return rejected("native-phase");
    for(unsigned i=0;i<20;i++)if(coop_native::word(r,a+0x188+i*4))return rejected("nonempty-graph");
    result={id,pool,storage,resource,coop_native::word(r,a+0x16C),coop_native::word(r,a+0x174),overlay,slot,update,sub,index};return true;
}
inline bool escort_cache_returned(uint8_t* r,const EscortCache& cache){
    const auto a=cache.id.address;const auto now=coop_collision::observed_identity(r,a);
    if(!now||*now!=cache.id||cache.id.kind!=0x4015||cache.id.private_data||cache.slot<1||cache.slot>4||!coop_native::range(cache.pool,12)||!coop_native::range(cache.storage,4*0x280)||cache.storage+(cache.slot-1)*0x280+4!=a)return false;
    if(coop_native::word(r,a+8)!=cache.pool||coop_native::word(r,cache.pool+4)!=4||coop_native::word(r,cache.pool+8)!=cache.storage||coop_native::byte(r,a-4)||coop_native::byte(r,a+4)||coop_native::byte(r,a+5)!=cache.slot||coop_native::byte(r,a+6)!=1||coop_native::byte(r,a+0x17D)||coop_collision::half(r,a+0x17E)!=150||coop_native::word(r,a+0x180)||coop_collision::half(r,a+0x184)||coop_native::word(r,a+0x1D8))return false;
    if(coop_native::word(r,a+0x168)!=cache.resource||coop_native::word(r,a+0x16C)!=cache.drawing||coop_native::word(r,a+0x174)!=cache.collision||!coop_native::range(cache.resource,0x48)||coop_collision::half(r,cache.resource+0x3C)!=0x4015||coop_native::word(r,cache.resource+0x40)!=cache.overlay||coop_native::word(r,cache.resource+0x44)!=cache.overlay+0xB48||coop_native::word(r,a+0x20)!=cache.update||coop_native::byte(r,a+0x30)!=cache.sub||coop_native::word(r,a+0x1E0)!=cache.index||!coop_escort_phase0281::matches(cache.overlay,cache.update,cache.sub,cache.index)||coop_native::word(r,a+0x2C)!=cache.overlay+0xAC0)return false;
    for(unsigned i=0;i<20;i++)if(coop_native::word(r,a+0x188+i*4))return false;
    return true;
}
inline bool reserve_pool_matches(uint8_t* r,const ReserveCache& cache){
    const auto& pools=coop_egg_pools::snapshot();
    return cache.owner<2&&coop_egg_pools::matches(r)&&cache.pool==coop_egg_pools::pool(cache.owner,0)&&
        pools.allocation==cache.allocation&&pools.manager==cache.manager&&pools.table==cache.table&&pools.manager_capacity==cache.capacity&&
        coop_native::range(cache.pool,12)&&coop_native::word(r,cache.pool+4)==8&&coop_native::word(r,cache.pool+8)==cache.storage&&
        coop_native::range(cache.storage,8*0x280)&&cache.slot>=1&&cache.slot<=8&&cache.storage+(cache.slot-1)*0x280+4==cache.id.address;
}
inline bool reserve_cache_capture(uint8_t* r,const coop_collision::Identity& id,unsigned owner,ReserveCache& result){
    const auto& life=coop_lifecycle0280::lifecycle.snapshot();if(owner>1)return false;
    const auto root=life.players[owner].root;const auto a=id.address;const auto now=coop_collision::observed_identity(r,a);
    if(life.players[owner].phase!=coop_lifecycle0280::Phase::death_pending||!coop_lifecycle0280::fatal(r,root.address)||!now||*now!=id||id.kind!=0x4004||id.private_data!=a+0x24C||!coop_native::range(id.private_data,0x30)||coop_native::byte(r,a+4)||coop_native::byte(r,a+6)!=2||coop_native::byte(r,a+0x17D)||coop_native::word(r,a+0x180)!=root.address||coop_collision::half(r,a+0x184))return false;
    const auto index=coop_collision::half(r,a+0x17E);if(index>=20||coop_native::word(r,root.address+0x188+index*4)!=a)return false;
    const auto count=coop_native::byte(r,root.private_data+0x12C8);const auto head=coop_collision::half(r,root.private_data+0x12BE);if(count<1||count>6||head>=8)return false;
    unsigned found=0;for(unsigned i=0;i<count;i++)found+=coop_native::word(r,root.private_data+0x11FC+((head+i)&7)*24)==a;if(found!=1)return false;
    const auto& pools=coop_egg_pools::snapshot();const auto pool=coop_native::word(r,a+8),resource=coop_native::word(r,a+0x168);
    if(!coop_native::range(pool,12)||!coop_native::range(resource,0x48))return false;
    const ReserveCache candidate{id,pool,coop_native::word(r,pool+8),resource,coop_native::word(r,a+0x16C),coop_native::word(r,a+0x174),pools.allocation,pools.manager,pools.table,owner,coop_native::byte(r,a+5),pools.manager_capacity};
    if(!reserve_pool_matches(r,candidate)||coop_native::byte(r,a-4)!=1||!coop_native::range(candidate.drawing,0x50)||!coop_native::range(candidate.collision,0x18)||coop_collision::half(r,resource+0x3C)!=0x4004||coop_native::word(r,resource+0x40)||coop_native::word(r,resource+0x44)!=0x8003B2B0||coop_native::word(r,a+0x20)!=0x8003AA10||coop_native::word(r,a+0x2C)!=0x80045BD8)return false;
    for(unsigned i=0;i<20;i++)if(coop_native::word(r,a+0x188+i*4))return false;
    result=candidate;return true;
}
inline bool reserve_cache_returned(uint8_t* r,const ReserveCache& cache){
    const auto a=cache.id.address;const auto now=coop_collision::observed_identity(r,a);
    if(!now||*now!=cache.id||cache.id.kind!=0x4004||cache.id.private_data!=a+0x24C||!reserve_pool_matches(r,cache))return false;
    if(coop_native::word(r,a+8)!=cache.pool||coop_native::byte(r,a-4)||coop_native::byte(r,a+4)||coop_native::byte(r,a+5)!=cache.slot||coop_native::byte(r,a+6)!=1||coop_native::byte(r,a+0x17D)||coop_collision::half(r,a+0x17E)!=150||coop_native::word(r,a+0x180)||coop_collision::half(r,a+0x184)||coop_native::word(r,a+0x1D8))return false;
    if(coop_native::word(r,a+0x168)!=cache.resource||coop_native::word(r,a+0x16C)!=cache.drawing||coop_native::word(r,a+0x174)!=cache.collision||!coop_native::range(cache.resource,0x48)||coop_collision::half(r,cache.resource+0x3C)!=0x4004||coop_native::word(r,cache.resource+0x40)||coop_native::word(r,cache.resource+0x44)!=0x8003B2B0||coop_native::word(r,a+0x20)!=0x8003AA10||coop_native::word(r,a+0x2C)!=0x80045BD8)return false;
    for(unsigned i=0;i<20;i++)if(coop_native::word(r,a+0x188+i*4))return false;
    return true;
}
inline bool retired_identity(uint8_t* r,const coop_collision::Identity& id,const EscortCache* cache=nullptr,const ReserveCache* reserve=nullptr){
    const auto now=coop_collision::observed_identity(r,id.address);
    if(now&&now->generation!=id.generation)return true; // Genuine native reuse.
    if(cache&&cache->id==id&&escort_cache_returned(r,*cache))return true;
    if(reserve&&reserve->id==id&&reserve_cache_returned(r,*reserve))return true;
    return !coop_native::actor(r,id.address)||(!coop_native::word(r,id.address+0x16C)&&!coop_native::word(r,id.address+0x174)&&!coop_native::word(r,id.address+0x1F4)&&!coop_native::word(r,id.address+0x180)&&!coop_collision::half(r,id.address+0x184));
}
inline bool retirement_plan(uint8_t* r,unsigned slot,std::vector<coop_collision::Identity>& out){
    const auto& life=coop_lifecycle0280::lifecycle.snapshot();if(slot>1)return false;
    const auto root=life.players[slot].root;const auto now=coop_collision::observed_identity(r,root.address);
    if(!now||*now!=root||coop_native::word(r,root.address+0x180))return false;
    const auto graph=coop_native::tree(r,root.address);if(graph.empty())return false;
    out.clear();
    for(auto a:graph){
        const auto id=coop_collision::observed_identity(r,a);if(!id)return false;
        if(a!=root.address){const auto parent=coop_native::word(r,a+0x180);const auto index=coop_collision::half(r,a+0x17E);if(!coop_native::actor(r,parent)||std::find(graph.begin(),graph.end(),parent)==graph.end()||index>=20||coop_native::word(r,parent+0x188+index*4)!=a)return false;}
        out.push_back(*id);
    }
    for(const auto& egg:coop_collision::registry.eggs_for_owner(r,slot)){
        bool seen=false;for(const auto& id:out)seen|=id.address==egg.address;
        if(!seen){if(coop_native::word(r,egg.address+0x180))return false;out.push_back(egg);}
    }
    return out.size()<=48;
}
inline bool empty_respawn_inventory(uint8_t* r,uint32_t actor){
    if(!coop_native::primary_ready(r,actor))return false;
    const auto data=coop_native::word(r,actor+0x1F4);
    if(coop_collision::half(r,data+0x12BE)||coop_native::byte(r,data+0x12C8)||coop_native::word(r,data+0x164)||coop_native::byte(r,data+0x155))return false;
    // Source436E0 initializes all eight native FIFO records (six words each).
    for(unsigned i=0;i<8*6;i++)if(coop_native::word(r,data+0x11FC+i*4))return false;
    for(auto child:coop_native::tree(r,actor))if(coop_native::kind(r,child)==0x4004||coop_native::kind(r,child)==0x4005)return false;
    return true;
}
inline void lifecycle_frame(uint8_t* r,recomp_context* c,unsigned vi,unsigned room){
    namespace life=coop_lifecycle0280;auto& cycle=life::lifecycle;
    cycle.clock(vi,room);if(cycle.snapshot().failed||cycle.snapshot().ending||(room!=43&&room!=81))return;
    publish_anchor(r);
    for(unsigned p=0;p<2;p++){
        const auto phase=cycle.snapshot().players[p].phase;
        if(phase==life::Phase::death_pending){
            if(!retirement_plan(r,p,retiring_graphs[p])){cycle.fail("native_retirement_graph_rejected");return;}
            retiring_escorts[p].clear();retiring_reserves[p].clear();
            for(const auto& id:retiring_graphs[p])if(id.kind==0x4004){ReserveCache cache;if(!reserve_cache_capture(r,id,p,cache)){cycle.fail("native_reserve_cache_capture_rejected");return;}retiring_reserves[p].emplace(id.address,cache);}
            for(const auto& id:retiring_graphs[p])if(id.kind==0x4015){EscortCache cache;if(!escort_cache_capture(r,id,cycle.snapshot().players[p].root.address,cache)){cycle.fail("native_escort_cache_capture_rejected");return;}retiring_escorts[p].emplace(id.address,cache);}
            std::vector<uint32_t> addresses;for(const auto& id:retiring_graphs[p])addresses.push_back(id.address);
            if(!coop_native::cleanup_lease(r,addresses)){cycle.fail("native_cleanup_lease_rejected");return;}
            const auto root=cycle.snapshot().players[p].root.address;
            const auto before=progress_hash(r);
            // Camera controller+68 is an optional second target. Clear it
            // only when it is exactly the retiring actor, via native6CB3C.
            const auto camera=coop_native::word(r,0x800F8294);
            if(coop_native::range(camera,0x6C)&&coop_native::word(r,camera+0x68)==root){
                coop_native::ScopedContext registers(c);func_8006CB3C(r,c);
            }
            cycle.retired(p); // Death receipt now permits expected free hooks.
            {
                Scope owner_scope(r,c,p);coop_native::ScopedGlobals globals(r);coop_native::ScopedContext registers(c);
                // Retire detached flying eggs first while their original root
                // still proves ownership. Native445A4 handles all resource work.
                for(const auto& id:retiring_graphs[p])if(id.kind==0x4002&&!coop_native::word(r,id.address+0x180)){
                    c->r4=coop_native::base(id.address);func_800445A4(r,c);
                }
                c->r4=coop_native::base(root);func_800445A4(r,c);
            }
            publish_anchor(r);
            if(progress_hash(r)!=before){cycle.fail("retirement_changed_shared_progress");return;}
            std::fprintf(stderr,"[CoopLife] cleanup owner=%u actor=%08X members=%zu vi=%u deferredNative=1 sharedProgressUnchanged=1\n",p,root,retiring_graphs[p].size(),vi);
        }
        if(cycle.snapshot().players[p].phase==life::Phase::retiring){
            bool ready=true;for(const auto& id:retiring_graphs[p]){const auto cache=retiring_escorts[p].find(id.address);const auto reserve=retiring_reserves[p].find(id.address);ready=ready&&retired_identity(r,id,cache==retiring_escorts[p].end()?nullptr:&cache->second,reserve==retiring_reserves[p].end()?nullptr:&reserve->second);}
            const auto manager=coop_native::word(r,0x800F8810);
            if(!coop_native::range(manager,0x18)){cycle.fail("native_manager_changed");return;}
            const auto table=coop_native::word(r,manager+0x10);const auto count=coop_collision::half(r,manager+8);
            if(!count||count>150||!coop_native::range(table,count*4)){cycle.fail("native_manager_layout_changed");return;}
            for(unsigned i=0;i<count;i++)for(const auto& id:retiring_graphs[p])if(coop_native::word(r,table+i*4)==id.address){const auto now=coop_collision::observed_identity(r,id.address);if(!now||now->generation==id.generation)ready=false;}
            if(ready){
                // Cached4015/4004 can be reused without resource deallocation.
                // Drop every completed lease before its slot becomes reusable.
                for(const auto& id:retiring_graphs[p])coop_native::cleanup_leases.erase(id.address);
                if(!retiring_escorts[p].empty())std::fprintf(stderr,"[CoopLife] cache-returned owner=%u escorts=%zu vi=%u native468E8Contract=1 resourcesRetained=1 leasesReleased=1\n",p,retiring_escorts[p].size(),vi);
                if(!retiring_reserves[p].empty())std::fprintf(stderr,"[CoopLife] reserve-cache-returned owner=%u eggs=%zu vi=%u native468E8Contract=1 resourcesRetained=1 leasesReleased=1\n",p,retiring_reserves[p].size(),vi);
                cycle.freed(p);if(p==0){bound_primary=0;coop_native::primary=0;}else {bound_secondary=0;coop_native::secondary=0;}
                std::fprintf(stderr,"[CoopLife] freed owner=%u oldBirth=%llu vi=%u dynamicResourcesReleased=1 cachedEscorts=%zu cachedReserveEggs=%zu\n",p,(unsigned long long)cycle.snapshot().players[p].root.generation,vi,retiring_escorts[p].size(),retiring_reserves[p].size());

            }
        }
        if(cycle.snapshot().players[p].phase==life::Phase::waiting&&death_pauses[p]&&!release_death_pause(r,c,p,vi,room)){cycle.fail("native_death_pause_release_rejected");return;}
        if(cycle.due(r,p,vi)){
            const unsigned anchor=1-p;const auto& survivor=context->player(anchor);
            if(!coop_native::primary_ready(r,survivor.actor.address))continue;
            const auto before=progress_hash(r),survivor_hp=survivor.health;
            const auto a=coop_native::respawn(r,c,p,survivor.actor.address,owner_packed[p],cycle.snapshot().players[p].color);
            if(!a)continue;
            const auto id=coop_collision::observed_identity(r,a);
            if(!id||!coop_collision::bind_root(r,p,a)){cycle.fail("new_native_birth_binding_rejected");return;}
            context->bind(p,{{a,uint32_t(id->generation)},cycle.snapshot().players[p].color,life::respawn_health});
            if(p==0)bound_primary=a;else bound_secondary=a;
            transients[p]={};
            unsigned readback_hp=9;
            {Scope owner_scope(r,c,p);coop_native::ScopedContext registers(c);c->r4=life::respawn_health;func_800422CC(r,c);readback_hp=coop_native::word(r,coop::native_health+coop::native_alias(cycle.snapshot().players[p].color)*4);}
            if(readback_hp!=life::respawn_health||context->player(p).health!=life::respawn_health||!empty_respawn_inventory(r,a)||context->player(anchor).health!=survivor_hp||progress_hash(r)!=before||!cycle.reborn(r,p,a,vi)){cycle.fail("native_respawn_readback_rejected");return;}
            if(!sync_attempt_binding()){cycle.fail("attempt_respawn_binding_rejected");return;}
            if(drawing_returns[p]){
                const auto& receipt=*drawing_returns[p];
                const bool quiescent=owner_stack.empty()&&!context->depth()&&!coop_egg_pools::snapshot().depth&&!coop_native_input::depth&&!coop_native::constructing&&!coop_native::cleaning&&!coop_rooms_native0280::busy()&&!coop_rooms_native0280::retaining_carried();
                if(!coop_camera_follow0287::rebind(r,c,receipt,cycle.snapshot(),bound_primary,bound_secondary,vi,room,quiescent)){cycle.fail("native_drawing_follow_rebind_rejected");return;}
                std::fprintf(stderr,"[CoopLife] drawing-follow-rebound owner=%u targetOwner=0 actor=%08X birth=%llu retired=%08X retiredBirth=%llu drawing=%08X vi=%u deathVI=%u epoch=%llu native5A994=1 nativeObservation=1\n",p,bound_primary,(unsigned long long)cycle.snapshot().players[0].root.generation,receipt.dead.address,(unsigned long long)receipt.dead.generation,receipt.drawing,vi,receipt.death_vi,(unsigned long long)receipt.epoch);
                drawing_returns[p].reset();
            }
            const auto root_now=cycle.snapshot().players[p].root.address,anchor_now=cycle.snapshot().players[anchor].root.address;
            std::fprintf(stderr,"[CoopLife] return-position owner=%u actor=%08X anchor=%08X xyz=%08X,%08X,%08X anchorXYZ=%08X,%08X,%08X vi=%u nativeObservation=1\n",p,root_now,anchor_now,coop_native::word(r,root_now+0x84),coop_native::word(r,root_now+0x88),coop_native::word(r,root_now+0x8C),coop_native::word(r,anchor_now+0x84),coop_native::word(r,anchor_now+0x88),coop_native::word(r,anchor_now+0x8C),vi);
            retiring_graphs[p].clear();retiring_escorts[p].clear();retiring_reserves[p].clear();publish_anchor(r);
        }
    }
}

}
#include "coop_rooms_native0280.hpp"

namespace workshop::coop_bridge {
inline bool old_attempt_roots_absent(uint8_t* r,const coop_lifecycle0280::State& life){
    const auto manager=coop_native::word(r,0x800F8810);
    if(!coop_native::range(manager,0x14))return false;
    const auto capacity=coop_collision::half(r,manager+8);
    const auto table=coop_native::word(r,manager+0x10);
    if(!capacity||capacity>150||!coop_native::range(table,capacity*4))return false;
    for(const auto& p:life.players){
        if(!p.root.address||!p.root.generation)return false;
        const auto now=coop_collision::observed_identity(r,p.root.address);
        if(now&&now->generation<=p.root.generation)return false;
        // An unobserved but still typed4001 is unknown, never a free receipt.
        if(!now&&coop_native::actor(r,p.root.address,0x4001))return false;
        for(unsigned i=0;i<capacity;i++)if(coop_native::word(r,table+i*4)==p.root.address&&(!now||now->generation<=p.root.generation))return false;
    }return true;
}
inline bool attempt_rooms_idle(){
    return coop_rooms_native0280::attempt_quiescent();
}
inline void attempt_observation(uint8_t* r,recomp_context* c,unsigned vi,unsigned stage,unsigned scene,unsigned room,unsigned mask){
    namespace a=coop_attempt0281;auto& attempts=a::attempts;
    if(!r||!c||!attempts.enabled())return;
    const auto life=coop_lifecycle0280::lifecycle.snapshot();
    if(life.bound&&!sync_attempt_binding())return;
    if(life.bound&&(life.ending||invalidated))attempts.finish({life.epoch,{attempt_identity(life.players[0].root),attempt_identity(life.players[1].root)},life.ending,invalidated});
    if(attempts.phase()!=a::Phase::awaiting_native_retry)return;
    const bool scopes_zero=owner_stack.empty()&&(!context||!context->depth())&&!coop_egg_pools::snapshot().depth&&!coop_native_input::depth&&!coop_fruit0264::registry.pending();
    const bool owned_empty=coop_native::owned.empty()&&coop_native::cleanup_leases.empty();
    const bool factory_idle=!coop_native::constructing&&!coop_native::cleaning,rooms_idle=attempt_rooms_idle();
    const bool old_absent=old_attempt_roots_absent(r,life);
    // Native scene teardown, not a missing/dead-looking actor, closes the old
    // views. Keep births observed while the replacement P1 was constructed.
    if(stage!=7&&scopes_zero&&owned_empty&&factory_idle&&rooms_idle&&old_absent){
        context.reset();bound_primary=bound_secondary=0;collisions_ready=false;
        coop_native::enabled=false;invalidated=true;
        if(coop_egg_pools::snapshot().allocation){coop_native::ScopedContext registers(c);coop_egg_pools::release(r,c);}
    }
    const auto& pools=coop_egg_pools::snapshot();
    a::Frame f;f.vi=vi;f.stage=stage;f.scene=scene;f.room=room;f.actual_alive_mask=mask;
    f.mask_observed=vi>=2400&&(stage==6||stage==7)&&scene==5;
    f.teardown={!context,owner_stack.empty(),scopes_zero,owned_empty,factory_idle,rooms_idle,!pools.installed&&!pools.allocation&&!pools.depth&&!coop_egg_pools::room_lease,old_absent};
    const auto p=coop_native::word(r,coop::native_player);
    const auto id=coop_collision::observed_identity(r,p);
    const auto manager=coop_native::word(r,0x800F8810);
    if(id)f.canonical_primary=attempt_identity(*id);
    f.primary_color=coop_native::word(r,coop::custom_color);
    f.canonical_ready=id&&coop_native::primary_ready(r,p)&&coop_native::range(manager,0x14)&&coop_collision::half(r,manager+8)>5&&coop_collision::half(r,manager+8)<=150;
    if(f.canonical_ready){
        const auto table=coop_native::word(r,manager+0x10),capacity=unsigned(coop_collision::half(r,manager+8));
        f.canonical_ready=coop_native::range(table,capacity*4)&&coop_native::word(r,table+20)==p&&coop_collision::half(r,p+0x17E)==5;
        if(f.canonical_ready){unsigned refs=0;for(unsigned i=0;i<capacity;i++)refs+=coop_native::word(r,table+i*4)==p;f.canonical_ready=refs==1;}
    }
    const auto decision=attempts.frame(f);
    if(!decision.ready){
        if(attempt_last_wait!=int(decision.reason)&&attempt_wait_diagnostics<16){
            attempt_last_wait=int(decision.reason);++attempt_wait_diagnostics;
            std::fprintf(stderr,"[CoopAttempt] wait reason=%s oldEpoch=%llu vi=%u stage=%u scene=%u room=%u primary=%08X birth=%llu oldRootsAbsent=%u contextGone=%u owned=%zu leases=%zu scopesZero=%u pools=%08X roomsIdle=%u maskObserved=%u available=%04X\n",a::name(decision.reason),(unsigned long long)decision.epoch,vi,stage,scene,room,p,(unsigned long long)f.canonical_primary.birth,unsigned(old_absent),unsigned(!context),coop_native::owned.size(),coop_native::cleanup_leases.size(),unsigned(scopes_zero),pools.allocation,unsigned(rooms_idle),unsigned(f.mask_observed),decision.eligible_mask);
        }return;
    }
    // All following changes are host metadata only; native P1, fruit births,
    // life flags, score and FIFO are untouched. No live pool is configured.
    if(!coop_collision::registry.clear_ownership()||!coop_native::rearm_after_teardown(decision.secondary_color))return;
    const bool pools_enabled=pools.enabled;
    coop_fruit0264::registry.configure(true);coop_lifecycle0280::lifecycle.configure(true);coop_rooms_native0280::configure(true);coop_egg_pools::configure(pools_enabled);
    context.reset();bound_primary=bound_secondary=0;invalidated=collisions_ready=false;
    retiring_graphs={};retiring_escorts={};retiring_reserves={};death_pauses={};drawing_returns={};owner_packed={};transients={};last_held={UINT32_MAX,UINT32_MAX};coop_native_input::reset();
    if(!attempts.commit(decision)){invalidated=true;coop_native::enabled=false;return;}
    std::fprintf(stderr,"[CoopAttempt] rearmed oldEpoch=%llu ticket=%llu vi=%u primary=%08X birth=%llu colors=%u,%u available=%04X lost=%04X stage=%u scene=%u room=%u oldRootsAbsent=1 poolsClosed=1 birthsPreserved=1 guestPayloadWrites=0\n",(unsigned long long)decision.epoch,(unsigned long long)decision.ticket,vi,decision.primary.address,(unsigned long long)decision.primary.birth,decision.primary_color,decision.secondary_color,decision.eligible_mask,attempts.lost_mask(),stage,scene,room);
}
}
