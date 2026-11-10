#pragma once
#include "boss_solo_fight0240.hpp"
#include "boss_rest_data0240.hpp"
#include "boss_tree0240.hpp"
#include <array>
#include <atomic>
#include <cstdio>
#include <cmath>
extern "C" void func_800447C8(uint8_t*,recomp_context*);
extern "C" void func_800445A4(uint8_t*,recomp_context*);
extern "C" void func_80043DD4(uint8_t*,recomp_context*);
extern "C" void func_80057D1C(uint8_t*,recomp_context*);
extern "C" void func_80043C94(uint8_t*,recomp_context*);
extern "C" void func_80030B7C(uint8_t*,recomp_context*);
extern "C" void func_80047090(uint8_t*,recomp_context*);
extern "C" void func_8004613C(uint8_t*,recomp_context*);
extern "C" void func_80044194(uint8_t*,recomp_context*);
extern "C" void func_8005D29C(uint8_t*,recomp_context*);
extern "C" void func_8002BEB0(uint8_t*,recomp_context*);
extern "C" void func_800443EC(uint8_t*,recomp_context*);
extern "C" void func_80040C0C(uint8_t*,recomp_context*);
extern "C" void func_80047AB8(uint8_t*,recomp_context*);
namespace workshop::boss_solo0240 {
inline bool enabled=false,diagnostic=false,driver=false;
inline boss_solo_fight::Session session;
inline unsigned room=177;
inline uint32_t ready_info=0;
inline unsigned carried_health=0,carried_fruits=0,carried_color=0,rest_vi=0,arrival_wait_vi=0;
inline uint32_t victory_pot=0;
inline float victory_pot_x=0;
inline unsigned running_before_victory=1;
inline bool running_captured=false;
inline std::array<uint32_t,6> carried_reserves{};
inline std::array<uint32_t,20> carried_children{};
inline std::atomic_bool finished=false;
inline constexpr uint32_t route_address=0x807DFF00;
inline unsigned effective_room(unsigned raw){return enabled && raw==52?162:raw;}
inline bool configure(std::string token,bool qa,bool drive){
    enabled=session.begin(token,qa,boss_rest_data0240::source_room);diagnostic=qa;driver=drive;
    room=177;ready_info=0;victory_pot=0;victory_pot_x=0;running_captured=false;rest_vi=0;arrival_wait_vi=0;finished=false;boss_tree0240::configure(enabled);return enabled;
}
inline void fail(const char* reason){session.cancel(reason);fprintf(stderr,"[BossSolo] failed reason=%s\n",reason);}
inline void enter_room(unsigned raw){
    if(!enabled || (session.snapshot().phase==boss_solo_fight::Phase::armed && raw!=52))return;room=effective_room(raw);ready_info=0;
    if(!session.enter_room(room))fprintf(stderr,"[BossSolo] unexpected-room source=%u\n",room);
    boss_tree0240::enter_room(room);
    fprintf(stderr,"[BossSolo] room=%u phase=%u\n",room,unsigned(session.snapshot().phase));
}
inline bool check_children(uint8_t* rdram,uint32_t player,std::array<uint32_t,20>& slots){
    if(!boss_native::actor(rdram,player,0x4001))return false;
    slots.fill(0);std::array<uint32_t,20> raw{};unsigned occupied=0;
    for(unsigned n=0;n<20;n++){
        const auto a=boss_native::word(rdram,player+0x188+n*4);raw[n]=a;if(!a)continue;
        for(unsigned j=0;j<n;j++)if(raw[j]==a)return false;
        if(!boss_native::range(a,0x1FC) || boss_native::word(rdram,a)!=0x19971110 || boss_native::word(rdram,a+0x180)!=player)return false;
        ++occupied;
        // Native2C8E8 deliberately removes4016 score popups before carrying.
        if(MEM_HU(12,boss_native::base(a))!=0x4016)slots[n]=a;
    }
    std::sort(slots.begin(),slots.end());
    return occupied==MEM_HU(0x184,boss_native::base(player));
}
inline bool read_reserves(uint8_t* rdram,uint32_t player,std::array<uint32_t,6>& eggs){
    eggs.fill(0);const auto data=boss_native::word(rdram,player+0x1F4);
    if(!boss_native::range(data,0x12D0))return false;
    const unsigned count=MEM_BU(0x12C8,boss_native::base(data)),head=MEM_HU(0x12BE,boss_native::base(data));
    if(count>6 || head>=8)return false;
    for(unsigned n=0;n<count;n++){
        const auto a=boss_native::word(rdram,data+0x11FC+((head+n)&7)*24);
        if(!boss_native::actor(rdram,a,0x4004) || boss_native::word(rdram,a+0x180)!=player)return false;
        for(unsigned j=0;j<n;j++)if(eggs[j]==a)return false;eggs[n]=a;
    }
    return true;
}
inline std::optional<unsigned> health(uint8_t* rdram){
    const auto color=boss_native::word(rdram,0x800F8DB0);if(color>=8)return std::nullopt;
    return boss_native::word(rdram,0x800F8DB4+color*4);
}
inline bool pot_passenger(uint8_t* rdram,uint32_t player){
    if(player!=session.snapshot().player || !boss_native::actor(rdram,player,0x4001) || !victory_pot || boss_native::word(rdram,0x800F8828)!=player)return false;
    const auto p=boss_native::word(rdram,player+0x1F4);const auto hp=health(rdram);
    // Native pot completion is the non-death A/2 transport state. Bind it to
    // this exact pot and callback; do not relax the native defeat observer.
    return hp && *hp>0 && boss_native::range(p,0x12D0) && MEM_BU(0x18,boss_native::base(player))==0xA && MEM_BU(0x30,boss_native::base(player))==2 && boss_native::word(rdram,player+0x20)==0x80031230 && boss_native::word(rdram,p+0xD0)==victory_pot && boss_native::word(rdram,p+0xC0)==0 && boss_native::word(rdram,player+0x180)==0 && MEM_BU(0x165,boss_native::base(player))==1 && MEM_BU(0x17D,boss_native::base(player))==0;
}
inline bool capture_carried(uint8_t* rdram){
    const auto player=session.snapshot().player;const auto hp=health(rdram);
    if(!hp || !pot_passenger(rdram,player) || !check_children(rdram,player,carried_children) || !read_reserves(rdram,player,carried_reserves))return false;
    carried_health=*hp;carried_color=boss_native::word(rdram,0x800F8DB0);carried_fruits=boss_native::word(rdram,0x800F89D0);return true;
}
inline std::optional<float> nearby_pot_x(uint8_t* rdram,uint32_t player){
    if(!boss_native::alive_player(rdram,player))return std::nullopt;
    const float x=std::bit_cast<float>(boss_native::word(rdram,player+0x84));
    if(!std::isfinite(x))return std::nullopt;
    return std::clamp(x+boss_rest_data0240::arena_pot_offset_x,boss_rest_data0240::arena_pot_min_x,boss_rest_data0240::arena_pot_max_x);
}
inline void make_rest_route(uint8_t* rdram){
    const gpr route=boss_native::base(route_address);
    for(unsigned n=0;n<24;n++)MEM_B(n,route)=boss_rest_data0240::native_arena_pot_route[n];
    MEM_B(4,route)=boss_rest_data0240::source_room; // Preserve44CA2000: native arrival-marker identity
    MEM_W(8,route)=u32(boss_rest_data0240::native_descriptor,0x28);MEM_W(12,route)=u32(boss_rest_data0240::native_descriptor,0x2C);
}
inline void defeat_begin(uint8_t* rdram){
    if(enabled && session.snapshot().phase==boss_solo_fight::Phase::fighting){running_before_victory=MEM_BU(0,int32_t(0x800F7EA6));running_captured=true;}
}
inline bool retire_cinematic(uint8_t* rdram,recomp_context* c,uint32_t a){
    if(!boss_native::actor(rdram,a,0x46EE))return false;
    const auto saved=*c;const unsigned phase=MEM_BU(0x17D,boss_native::base(a));
    if(phase==1){
        // Native44250 cannot skip holes in this FIFO. Remove only a known
        // endpoint through native pop operations, keeping both cursors and
        // count consistent, without executing cinematic callback48968.
        constexpr uint32_t queue=0x800F882C;
        const int count=MEM_H(6,boss_native::base(queue)),capacity=MEM_H(8,boss_native::base(queue)),index=MEM_H(0x17E,boss_native::base(a)),head=MEM_H(2,boss_native::base(queue)),tail=MEM_H(4,boss_native::base(queue));
        const auto table=boss_native::word(rdram,queue+0x10);
        if(count<=0 || capacity<=0 || capacity>150 || count>capacity || head<0 || head>=capacity || tail<0 || tail>=capacity || index<0 || index>=capacity || !boss_native::range(table,unsigned(capacity)*4) || boss_native::word(rdram,table+unsigned(index)*4)!=a)return false;
        const bool at_head=index==head,at_tail=index==(tail+capacity-1)%capacity;
        if(!at_head && !at_tail)return false;
        for(unsigned n=0;n<20;n++)if(boss_native::word(rdram,a+0x188+n*4))return false;
        c->r29=ADD32(saved.r29,-0x80);c->r4=boss_native::base(queue);
        if(at_head)func_80043DD4(rdram,c);else func_80057D1C(rdram,c);
        const bool popped=uint32_t(c->r2)==a && !boss_native::word(rdram,table+unsigned(index)*4) && MEM_H(6,boss_native::base(queue))==count-1 && MEM_H(2,boss_native::base(queue))==(at_head?(head+1)%capacity:head) && MEM_H(4,boss_native::base(queue))==(at_head?tail:index);
        *c=saved;if(!popped)return false;
        if(!at_head){c->r29=ADD32(saved.r29,-0x80);c->r4=boss_native::base(a);func_80043C94(rdram,c);const bool detached=c->r2!=0;*c=saved;if(!detached)return false;}
        if(MEM_HU(0x17E,boss_native::base(a))!=0x96 || MEM_BU(0x17D,boss_native::base(a))!=0)return false;
        c->r29=ADD32(saved.r29,-0x80);c->r4=boss_native::base(a);func_800445A4(rdram,c);const bool ok=c->r2!=0;*c=saved;return ok;
    }
    if(phase==2){c->r29=ADD32(saved.r29,-0x80);c->r4=boss_native::base(a);func_800447C8(rdram,c);*c=saved;return true;}
    return false;
}
inline void defeat_complete(uint8_t* rdram,recomp_context* c){
    if(!enabled)return;
    fprintf(stderr,"[BossSolo] defeat-hook phase=%u ready=%u delivered=%u\n",unsigned(session.snapshot().phase),unsigned(boss_native::snapshot().completion_ready),unsigned(boss_native::delivered));
    auto proof=boss_native::take_defeat_observation();if(!proof)return;
    fprintf(stderr,"[BossSolo] proof token=%s diagnostic=%u stage=%u room=%u vi=%u actor=%08X player=%08X transition=%08X initial=%d remaining=%d\n",proof->run_token.c_str(),unsigned(proof->diagnostic),proof->stage,proof->source_room,proof->vi,proof->actor,proof->player,proof->transition_actor,proof->initial_segments,proof->remaining_segments);
    if(!session.accept_defeat(*proof)){fprintf(stderr,"[BossSolo] proof-rejected phase=%u reason=%s\n",unsigned(session.snapshot().phase),session.snapshot().failure.c_str());return;}
    fprintf(stderr,"[BossSolo] accepted retiring-cinematic=%08X\n",proof->transition_actor);
    if(!retire_cinematic(rdram,c,proof->transition_actor)){fail("cinematic-retirement-rejected");return;}
    if(!running_captured){fail("missing-pre-victory-controls");return;}
    const auto saved=*c;c->r29=ADD32(saved.r29,-0x80);c->r4=boss_native::base(proof->player);func_80040C0C(rdram,c);*c=saved;
    MEM_B(0,int32_t(0x800F7EA6))=running_before_victory;
    fprintf(stderr,"[BossSolo] native-victory vi=%u player=%08X candy=%08X cinematic=%08X segments=%d diagnostic=%u\n",proof->vi,proof->player,proof->actor,proof->transition_actor,proof->remaining_segments,unsigned(diagnostic));
}
inline void room_ready(uint8_t* rdram,recomp_context* c){
    if(enabled && session.snapshot().phase==boss_solo_fight::Phase::departing && room==boss_rest_data0240::source_room && boss_native::range(uint32_t(c->r4),0x128) && MEM_BU(1,c->r4)==room)ready_info=uint32_t(c->r4);
}
inline bool pot_complete(uint8_t* rdram,recomp_context* c){
    if(!enabled || !boss_native::range(uint32_t(c->r4),24))return false;
    if(room==boss_native::first_room && session.snapshot().phase==boss_solo_fight::Phase::victory_ready){
        if(!boss_native::actor(rdram,victory_pot,boss_rest_data0240::arena_pot_kind))return false;
        const auto data=boss_native::word(rdram,victory_pot+0x1F4);
        if(!boss_native::range(data,0x264) || boss_native::word(rdram,data+0x260)!=uint32_t(c->r4))return false;
        for(unsigned n=0;n<24;n++)if(MEM_BU(n,c->r4)!=boss_rest_data0240::native_arena_pot_route[n])return false;
        if(!capture_carried(rdram) || !session.begin_departure(boss_rest_data0240::source_room)){fail("invalid-pot-carry");return true;}
        make_rest_route(rdram);c->r4=boss_native::base(route_address);
        fprintf(stderr,"[BossSolo] arena-pot-entered player=%08X health=%u fruits=%u target=%u\n",session.snapshot().player,carried_health,carried_fruits,boss_rest_data0240::source_room);
        return false; // original5D320 executes with this owned warm route
    }
    if(room!=boss_rest_data0240::source_room || session.snapshot().phase!=boss_solo_fight::Phase::rest)return false;
    for(unsigned n=0;n<24;n++)if(MEM_BU(n,c->r4)!=boss_rest_data0240::native_pot_route[n])return false;
    if(session.request_next()){finished=true;fprintf(stderr,"[BossSolo] onward-pot first-slice-finished tenBossComplete=0\n");return true;}
    return false;
}
inline void frame(uint8_t* rdram,recomp_context* c,unsigned vi){
    if(!enabled)return;boss_tree0240::frame(rdram);
    const auto& tree=boss_tree0240::snapshot();
    static std::array<uint32_t,5> previousTree{~0u,~0u,~0u,~0u,~0u};
    const std::array<uint32_t,5> nowTree{tree.tree,tree.consumedmask,tree.regenerations,tree.totalConsumed,unsigned(tree.invalidated)};
    if(tree.initialized && nowTree!=previousTree){previousTree=nowTree;fprintf(stderr,"[BossTree] vi=%u tree=%08X room=%u mask=%u regens=%u total=%u heart=%u invalid=%u\n",vi,tree.tree,tree.room,tree.consumedmask,tree.regenerations,tree.totalConsumed,unsigned(MEM_HU(0x1C,boss_native::base(tree.privateData))),unsigned(tree.invalidated));}

    const auto phase=session.snapshot().phase;
    if(phase==boss_solo_fight::Phase::victory_ready){
        if(victory_pot)return;
        const auto pot_position=nearby_pot_x(rdram,session.snapshot().player);if(!pot_position){fail("invalid-victory-pot-position");return;}victory_pot_x=*pot_position;
        const auto saved=*c;c->r29=ADD32(saved.r29,-0x80);c->r4=boss_rest_data0240::arena_pot_kind;func_80047090(rdram,c);
        const auto resource=uint32_t(c->r2);if(resource==0x3FF){*c=saved;fail("victory-pot-resource-unavailable");return;}
        *c=saved;c->r29=ADD32(saved.r29,-0x80);c->r4=(uint32_t(boss_rest_data0240::arena_pot_kind)<<16)|0x2000|(resource&0x3FF);
        c->r5=std::bit_cast<uint32_t>(victory_pot_x);c->r6=std::bit_cast<uint32_t>(boss_rest_data0240::arena_pot_y);c->r7=boss_rest_data0240::arena_pot_z_bits;func_8004613C(rdram,c);
        victory_pot=uint32_t(c->r2);
        if(!boss_native::actor(rdram,victory_pot,boss_rest_data0240::arena_pot_kind)){*c=saved;fail("victory-pot-allocation-failed");return;}
        // Native45CAC uses these visibility bounds for uncullable resources.
        // Keep only this owned exit pot alive if the player walks away; normal
        // scene cleanup is unchanged, as are its collision, model and route.
        MEM_H(0xC0,boss_native::base(victory_pot))=0x7FFF;
        MEM_H(0xC2,boss_native::base(victory_pot))=0x7FFF;
        *c=saved;c->r29=ADD32(saved.r29,-0x80);c->r4=boss_native::base(victory_pot);func_80044194(rdram,c);const bool registered=c->r2!=0;*c=saved;
        if(!registered){fail("victory-pot-registration-failed");return;}
        fprintf(stderr,"[BossSolo] victory-pot-ready vi=%u actor=%08X kind=%04X voluntary=1 x=%.2f y=%.2f\n",vi,victory_pot,boss_rest_data0240::arena_pot_kind,victory_pot_x,boss_rest_data0240::arena_pot_y);return;
    }
    if(phase!=boss_solo_fight::Phase::departing || !ready_info)return;
    if(!arrival_wait_vi)arrival_wait_vi=vi;
    const auto player=session.snapshot().player;
    if(!boss_native::actor(rdram,player,0x4001)){fail("carried-player-identity-lost");return;}
    // Native44CA matches the preserved route identity, drains the carry queue,
    // and releases the same Yoshi. Observe that release after its animation;
    // never dequeue it ourselves or overwrite its state/callback/position.
    const bool released=boss_native::word(rdram,0x800F8828)==player && MEM_BU(0x17D,boss_native::base(player))==2 && MEM_BU(0x165,boss_native::base(player))==0 && boss_native::alive_player(rdram,player);
    if(!released){if(vi-arrival_wait_vi>1800)fail("native-rest-arrival-timeout");return;}
    std::array<uint32_t,20> after{};std::array<uint32_t,6> eggs{};
    const auto hp=health(rdram);const bool graph_ok=check_children(rdram,player,after),eggs_ok=read_reserves(rdram,player,eggs);
    if(!graph_ok || after!=carried_children || !eggs_ok || eggs!=carried_reserves || !hp || *hp!=carried_health || boss_native::word(rdram,0x800F8DB0)!=carried_color || boss_native::word(rdram,0x800F89D0)!=carried_fruits){
        fprintf(stderr,"[BossSolo] carry-check graph=%u childrenSame=%u eggs=%u eggsSame=%u health=%u->%u color=%u->%u fruits=%u->%u\n",unsigned(graph_ok),unsigned(after==carried_children),unsigned(eggs_ok),unsigned(eggs==carried_reserves),carried_health,hp.value_or(~0u),carried_color,boss_native::word(rdram,0x800F8DB0),carried_fruits,boss_native::word(rdram,0x800F89D0));fail("carried-state-changed");return;}
    ready_info=0;
    rest_vi=vi;
    if(!session.arrive_rest(rdram,room,player)){fail("rest-arrival-rejected");return;}
    fprintf(stderr,"[BossSolo] rest-ready vi=%u player=%08X samePlayer=1 health=%u fruits=%u inventoryPreserved=1 tree=4708 maxRegenerations=2 tenBossComplete=0\n",vi,player,carried_health,carried_fruits);
}
// Diagnostic controller only: the native pot is entered from its rim.
inline void drive_pot_input(uint8_t* rdram,uint32_t player,unsigned vi,float target,uint16_t kind,uint32_t exact_pot,uint16_t* buttons,float* x,float* y){
    const float px=std::bit_cast<float>(boss_native::word(rdram,player+0x84)),dx=target-px;
    *x=std::abs(dx)>6?(dx>0?0.65f:-0.65f):0;*buttons=0;*y=0;
    const auto parent=boss_native::word(rdram,player+0x180);
    if(boss_native::actor(rdram,parent,kind) && (!exact_pot || parent==exact_pot)){
        *x=0;*y=-0.9f;return;
    }
    if(std::abs(dx)<=24 && vi%120<30)*buttons=0x8000;
}
inline void drive_input(uint8_t* rdram,unsigned vi,uint16_t* buttons,float* x,float* y){
    if(!enabled || !driver || !rdram)return;
    const auto phase=session.snapshot().phase;
    if(phase==boss_solo_fight::Phase::victory_ready || phase==boss_solo_fight::Phase::rest){
        const auto player=boss_native::word(rdram,0x800F8828);if(!boss_native::alive_player(rdram,player))return;
        const float px=std::bit_cast<float>(boss_native::word(rdram,player+0x84));*buttons=0;*x=*y=0;
        if(phase==boss_solo_fight::Phase::victory_ready){if(!victory_pot)return;drive_pot_input(rdram,player,vi,victory_pot_x,boss_rest_data0240::arena_pot_kind,victory_pot,buttons,x,y);return;}
        if(vi<rest_vi+300)return;
        std::array<uint32_t,6> eggs{};const bool got_egg=read_reserves(rdram,player,eggs) && eggs[0]!=0;
        if(got_egg){drive_pot_input(rdram,player,vi,boss_rest_data0240::pot_x,0x4274,0,buttons,x,y);return;}
        const float dx=boss_rest_data0240::egg_block_x-px;*x=std::abs(dx)>8?(dx>0?0.65f:-0.65f):0;
        if(std::abs(dx)<=18)*buttons=vi%80<20?0x8000:0;return;
    }
    if(phase!=boss_solo_fight::Phase::fighting)return;
    const auto& b=boss_native::snapshot();if(!b.active_seen || !b.head || !boss_native::actor(rdram,b.head,0x454F))return;
    const auto p=boss_native::word(rdram,0x800F8828);if(!boss_native::alive_player(rdram,p))return;
    const auto hp=boss_native::word(rdram,b.head+0x1F4);if(!boss_native::range(hp,0x88))return;
    const auto px=std::bit_cast<float>(boss_native::word(rdram,p+0x84)),py=std::bit_cast<float>(boss_native::word(rdram,p+0x88));
    float target=std::bit_cast<float>(boss_native::word(rdram,b.head+0x84)),lowest=-100000;
    for(unsigned n=0;n<12;n++){auto a=boss_native::word(rdram,hp+0x54+n*4);if(!boss_native::actor(rdram,a,0x4550) || MEM_BU(0x18,boss_native::base(a))!=1)continue;float ay=std::bit_cast<float>(boss_native::word(rdram,a+0x88));if(ay>lowest){lowest=ay;target=std::bit_cast<float>(boss_native::word(rdram,a+0x84));}}
    // A fresh jump edge after a tongue-free window, then hold for flutter.
    // The last segments rise: aim the tongue upward near the jump apex.
    const float desired=std::max(8.0f,target-38.0f),dx=desired-px;
    *x=dx>4?0.65f:dx< -4?-0.65f:0.16f;*y=0;
    const unsigned cycle=vi%120;*buttons=cycle<70?0x8000:0;
    if(cycle>=18 && cycle<100 && vi%20<5){*buttons|=0x4000;if(lowest<py-70)*y=0.8f;}
    static int lastSegments=-1;
    const int segments=int32_t(boss_native::word(rdram,hp+0x84));
    if(segments!=lastSegments){lastSegments=segments;fprintf(stderr,"[BossDriver] vi=%u segments=%d player=(%.2f,%.2f) target=(%.2f,%.2f)\n",vi,segments,px,py,target,lowest);}
}
} // namespace workshop::boss_solo0240
