#pragma once
#include "recomp.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <map>
#include <set>
#include <vector>

extern "C" void func_8004613C(uint8_t*,recomp_context*);
extern "C" void func_800440E4(uint8_t*,recomp_context*);
extern "C" void func_80048A30(uint8_t*,recomp_context*);
extern "C" void func_8005FDC4(uint8_t*,recomp_context*);
extern "C" void func_80043A88(uint8_t*,recomp_context*);
extern "C" void func_80043C0C(uint8_t*,recomp_context*);
extern "C" void func_800445A4(uint8_t*,recomp_context*);
extern "C" void func_80045BD8(uint8_t*,recomp_context*);

namespace workshop::coop_native {
// Isolated factory spike only. This is not a playable-coop capability.
// The caller must provide enclosing owner scopes before enabling native updates.
inline constexpr uint32_t player_global=0x800F8828,color_global=0x800F8DB0,
    custom_color_global=0x807F0008,health_global=0x800F8DB4,pool_header=0x800F87FC;
inline bool enabled=false,attempted=false,constructing=false,cleaning=false;
inline uint32_t primary=0,secondary=0,spawn_vi=0;
inline unsigned color=5;
inline uint32_t wait_mask=0;
inline unsigned constructor_trace_count=0;
enum class WaitReason:unsigned{constructing,cleaning,before_vi,room,player,phase,hidden,controls,update,private_data,coordinates};
inline std::vector<uint32_t> constructed;
struct Identity{uint16_t kind;uint32_t private_data;};
inline std::map<uint32_t,Identity> owned;
// Separate lifetime leases never attribute P1 children to player2. They are
// installed before native retirement and removed by genuine free/create events.
inline std::map<uint32_t,Identity> cleanup_leases;
inline bool range(uint32_t p,size_t bytes=4){return p>=0x80000400u && p<0x80800000u && bytes<=0x80800000u-p;}
inline gpr base(uint32_t p){return gpr(int32_t(p));}
inline uint32_t word(uint8_t* rdram,uint32_t p){const gpr b=base(p);return uint32_t(MEM_W(0,b));}
inline void word(uint8_t* rdram,uint32_t p,uint32_t value){const gpr b=base(p);MEM_W(0,b)=int32_t(value);}
inline unsigned byte(uint8_t* rdram,uint32_t p){const gpr b=base(p);return MEM_BU(0,b);}
inline void byte(uint8_t* rdram,uint32_t p,unsigned value){const gpr b=base(p);MEM_B(0,b)=uint8_t(value);}
inline bool actor(uint8_t* rdram,uint32_t a,uint16_t kind=0){
    if(!range(a,0x1FC) || word(rdram,a)!=0x19971110 || byte(rdram,a+6)==3)return false;
    const gpr b=base(a);return !kind || MEM_HU(12,b)==kind;
}
inline uint16_t kind(uint8_t* rdram,uint32_t a){const gpr b=base(a);return MEM_HU(12,b);}
inline uint32_t primary_actor(){return primary;}
inline uint32_t owned_actor(){return secondary;}
inline unsigned secondary_color(){return color;}
inline bool owns(uint8_t* rdram,uint32_t a){
    auto it=owned.find(a);return it!=owned.end() && actor(rdram,a,it->second.kind) && word(rdram,a+0x1F4)==it->second.private_data;
}
inline bool cleanup_owned(uint8_t* r,uint32_t a){
 if(owns(r,a))return true;const auto it=cleanup_leases.find(a);
 return it!=cleanup_leases.end()&&actor(r,a,it->second.kind)&&word(r,a+0x1F4)==it->second.private_data;
}
inline bool cleanup_lease(uint8_t* r,const std::vector<uint32_t>& actors){
 if(actors.empty()||actors.size()>48)return false;
 for(auto a:actors)if(!actor(r,a))return false;
 for(auto a:actors)cleanup_leases[a]={kind(r,a),word(r,a+0x1F4)};
 return true;
}
inline void wait_once(uint8_t* rdram,unsigned vi,unsigned room,WaitReason code,const char* reason,uint32_t a=0){
    const uint32_t bit=uint32_t(1)<<unsigned(code);if(wait_mask&bit)return;wait_mask|=bit;
    const bool valid=range(a,0x1FC);
    fprintf(stderr,"[CoopFactory] WAIT vi=%u reason=%s roomKey=%08X roomLow=%u player=%08X life=%u phase17D=%u visibility=%u callback=%08X update=%08X private=%08X\n",
        vi,reason,room,room&255,a,valid?byte(rdram,a+6):0,valid?byte(rdram,a+0x17D):0,valid?byte(rdram,a+0x165):0,
        valid?word(rdram,a+0x130):0,valid?word(rdram,a+0x20):0,valid?word(rdram,a+0x1F4):0);
}
inline void forget(uint32_t a){
    owned.erase(a);cleanup_leases.erase(a);
    if(a==primary)primary=0;
    if(a==secondary)secondary=0;
}
inline void configure(bool qa_enabled,unsigned secondary_color=5){
    // Call before a fresh game/profile begins, never to abandon live actors.
    enabled=qa_enabled && secondary_color<14;attempted=constructing=cleaning=false;
    primary=secondary=spawn_vi=0;wait_mask=0;constructor_trace_count=0;color=secondary_color;owned.clear();cleanup_leases.clear();constructed.clear();
}
// A native Trial retry already owns its new canonical P1. Only rearm host
// allocation locks after every previous owned actor/cleanup lease is gone.
inline bool rearm_after_teardown(unsigned secondary_color){
    if(secondary_color>=14||constructing||cleaning||!owned.empty()||!cleanup_leases.empty())return false;
    enabled=true;attempted=false;primary=secondary=spawn_vi=0;wait_mask=0;
    constructor_trace_count=0;color=secondary_color;constructed.clear();return true;
}
struct Globals {
    uint32_t player=0,native_color=0,custom_color=0,button_word=0,mode_word=0,audio_mode=0;
    std::array<uint32_t,8> health{};
    uint32_t camera=0,camera_callback=0,primary_data=0;
    unsigned scale_byte=0,primary_tail=0;
    static Globals capture(uint8_t* rdram){
        Globals s;s.player=word(rdram,player_global);s.native_color=word(rdram,color_global);s.custom_color=word(rdram,custom_color_global);
        for(unsigned i=0;i<8;i++)s.health[i]=word(rdram,health_global+i*4);
        // Proven direct constructor effects, each restored at its original width.
        s.button_word=word(rdram,0x800F897C);s.mode_word=word(rdram,0x800F89BC);s.scale_byte=byte(rdram,0x800F89B4);
        s.audio_mode=word(rdram,0x800992D0);s.camera=word(rdram,0x800F8318);
        if(range(s.camera,0xF8))s.camera_callback=word(rdram,s.camera+0xF4);
        if(actor(rdram,s.player,0x4001)){
            s.primary_data=word(rdram,s.player+0x1F4);
            if(range(s.primary_data,0x1550))s.primary_tail=byte(rdram,s.primary_data+0x1404);
        }
        return s;
    }
    void restore(uint8_t* rdram)const{
        for(unsigned i=0;i<8;i++)word(rdram,health_global+i*4,health[i]);
        word(rdram,color_global,native_color);word(rdram,custom_color_global,custom_color);
        word(rdram,0x800F897C,button_word);word(rdram,0x800F89BC,mode_word);
        byte(rdram,0x800F89B4,scale_byte);word(rdram,0x800992D0,audio_mode);
        if(range(camera,0xF8) && word(rdram,0x800F8318)==camera)word(rdram,camera+0xF4,camera_callback);
        if(actor(rdram,player,0x4001) && word(rdram,player+0x1F4)==primary_data && range(primary_data,0x1550))
            byte(rdram,primary_data+0x1404,primary_tail);
        // Never resurrect a freed actor or preserve a pointer to the destroyed secondary.
        word(rdram,player_global,actor(rdram,player,0x4001)?player:actor(rdram,primary,0x4001)?primary:actor(rdram,secondary,0x4001)?secondary:0);
    }
};
struct ScopedGlobals {
    uint8_t* rdram;Globals saved;
    explicit ScopedGlobals(uint8_t* ram):rdram(ram),saved(Globals::capture(ram)){}
    ~ScopedGlobals(){saved.restore(rdram);}
};
struct ScopedConstruction {
    ScopedConstruction(){constructing=true;}
    ~ScopedConstruction(){constructing=false;}
};
inline void construction_trace(const char* stage,recomp_context* c){
    if(!constructing || constructor_trace_count++>=64)return;
    fprintf(stderr,"[CoopFactory] constructing stage=%s a0=%08X a1=%08X a2=%08X a3=%08X sp=%08X\n",stage,uint32_t(c->r4),uint32_t(c->r5),uint32_t(c->r6),uint32_t(c->r7),uint32_t(c->r29));
}
struct ScopedContext {
    recomp_context* context;recomp_context saved;
    explicit ScopedContext(recomp_context* c):context(c),saved(*c){context->r29=ADD32(saved.r29,-0x40);}
    ~ScopedContext(){*context=saved;}
};
inline unsigned free_actor_slots(uint8_t* rdram){
    const gpr h=base(pool_header);const int count=MEM_H(8,h);const uint32_t table=word(rdram,pool_header+0x10);
    if(count<1 || count>150 || !range(table,size_t(count)*4))return 0;
    unsigned free=0;const gpr t=base(table);
    for(int n=0;n<count;n++){const uint32_t a=uint32_t(MEM_W(n*4,t));if(range(a,0x1FC) && byte(rdram,a+6)==3)free++;}
    return free;
}
//5D96C faults instead of returning a usable slot when a graphics class
//is full. The stock4001 type3 pool has only one slot, occupied by P1.
inline unsigned free_graphics_slots(uint8_t* rdram,unsigned type){
    if(type>6)return 0;const uint32_t header=0x800A6344+type*20;
    const uint32_t count=word(rdram,header),flags=word(rdram,header+4);
    if(!count || count>1024 || !range(flags,count))return 0;
    unsigned free=0;const gpr f=base(flags);
    for(unsigned n=0;n<count;n++)if(MEM_BU(n,f)==0)free++;
    return free;
}
//4001 normally owns manager slot5.43A88 reserves that slot regardless of
//occupancy, so a second4001 must request a genuinely free ordinary slot.
inline int free_main_slot(uint8_t* rdram,uint32_t first){
    const uint32_t manager=word(rdram,0x800F8810);if(!range(manager,0x14))return -1;
    const gpr m=base(manager);const int count=MEM_H(8,m);const uint32_t table=word(rdram,manager+0x10);
    if(count<7 || count>150 || !range(table,size_t(count)*4) || word(rdram,table+5*4)!=first)return -1;
    for(int n=6;n<std::min(count,124);n++)if(word(rdram,table+unsigned(n)*4)==0)return n;
    return -1;
}
inline bool primary_ready(uint8_t* rdram,uint32_t p){
    //443EC writes pending17D=4 at44448.4445C drains that queue through
    //440E4, which registers the actor and writes normal17D=2 at4417C.
    return actor(rdram,p,0x4001) && byte(rdram,p+0x17D)==2 && byte(rdram,p+0x165)==0
        && word(rdram,p+0x130)==0x800233B4 && word(rdram,p+0x20)==0x80043450
        && range(word(rdram,p+0x1F4),0x1550);
}
inline bool register_secondary(uint8_t* rdram,recomp_context* c,uint32_t a){
    const int expected=free_main_slot(rdram,primary);if(expected<0)return false;
    const uint32_t manager=word(rdram,0x800F8810),table=word(rdram,manager+0x10),packed=word(rdram,a+12);
    //Same resident sequence as440E4, with a generic free-slot query (a1=0).
    //The actual actor keeps its native4001 packed resource and native model.
    c->r4=base(a);func_8005FDC4(rdram,c);
    c->r4=base(manager);c->r5=0;func_80043A88(rdram,c);
    const int slot=int(int16_t(c->r2));
    if(slot!=expected || word(rdram,table+unsigned(slot)*4)!=0 || word(rdram,table+5*4)!=primary)return false;
    c->r4=base(manager);c->r5=slot;c->r6=base(a);func_80043C0C(rdram,c);
    if(uint32_t(c->r2)!=1 || word(rdram,table+unsigned(slot)*4)!=a || word(rdram,table+5*4)!=primary || word(rdram,a+12)!=packed)return false;
    byte(rdram,a+0x17D,2);
    fprintf(stderr,"[CoopFactory] registration actor=%08X manager=%08X primarySlot5=%08X secondarySlot=%u phase17D=2 packed=%08X\n",a,manager,primary,unsigned(slot),packed);
    return true;
}
inline std::vector<uint32_t> tree(uint8_t* rdram,uint32_t root){
    std::vector<uint32_t> result;if(!actor(rdram,root))return result;result.push_back(root);
    for(size_t i=0;i<result.size() && i<16;i++){
        const uint32_t a=result[i];const gpr b=base(a);const unsigned count=MEM_HU(0x184,b);
        if(count>20)return {};
        unsigned occupied=0;
        //47F9C searches all20 slots;48060 removes without compacting them.
        for(unsigned n=0;n<20;n++){
            const uint32_t child=uint32_t(MEM_W(0x188+n*4,b));
            if(!child)continue;
            ++occupied;
            if(!actor(rdram,child) || word(rdram,child+0x180)!=a)return {};
            if(std::find(result.begin(),result.end(),child)!=result.end())return {};
            result.push_back(child);if(result.size()>16)return {};
        }
        if(occupied!=count)return {};
    }return result;
}
inline void remember(uint8_t* rdram,uint32_t root){
    //Keep the root incarnation guarded even if a failed init has an incomplete
    //child list. Native deferred cleanup must still restore the primary global.
    if(actor(rdram,root))owned[root]={kind(rdram,root),word(rdram,root+0x1F4)};
    for(uint32_t a:tree(rdram,root))owned[a]={kind(rdram,a),word(rdram,a+0x1F4)};
}
inline void created(recomp_context* c){
    cleanup_leases.erase(uint32_t(c->r4));
    if(constructing && range(uint32_t(c->r4),0x1FC)){constructed.push_back(uint32_t(c->r4));construction_trace("45CAC-created",c);}
}
inline void log_actor(uint8_t* rdram,uint32_t a,const char* label,unsigned vi){
    if(!actor(rdram,a))return;
    const gpr b=base(a);const uint32_t data=word(rdram,a+0x1F4),drawing=word(rdram,a+0x16C),collision=word(rdram,a+0x174);
    fprintf(stderr,"[CoopFactory] %s vi=%u actor=%08X kind=%04X private=%08X draw=%08X collision=%08X children=%u parent=%08X active=%u visible=%u callback=%08X update=%08X xy=(%.3f,%.3f) global=%08X\n",
        label,vi,a,unsigned(kind(rdram,a)),data,drawing,collision,unsigned(MEM_HU(0x184,b)),word(rdram,a+0x180),
        byte(rdram,a+0x17D),byte(rdram,a+0x165),word(rdram,a+0x130),word(rdram,a+0x20),
        std::bit_cast<float>(word(rdram,a+0x84)),std::bit_cast<float>(word(rdram,a+0x88)),word(rdram,player_global));
    if(range(data,0x164) && kind(rdram,a)==0x4001)fprintf(stderr,"[CoopFactory] children vi=%u player=%08X nativeB=%08X nativeC=%08X\n",vi,a,word(rdram,data+0x160),word(rdram,data+0x15C));
}
inline void log_globals(uint8_t* rdram,const char* label,unsigned vi){
    const auto s=Globals::capture(rdram);
    fprintf(stderr,"[CoopFactory] globals label=%s vi=%u player=%08X nativeColor=%u customColor=%u health=%u,%u,%u,%u,%u,%u,%u,%u pad=%08X form=%u mode=%08X camera=%08X cameraCallback=%08X privateTail=%u audioMode=%08X\n",
        label,vi,s.player,s.native_color,s.custom_color,s.health[0],s.health[1],s.health[2],s.health[3],s.health[4],s.health[5],s.health[6],s.health[7],
        s.button_word,s.scale_byte,s.mode_word,s.camera,s.camera_callback,s.primary_tail,s.audio_mode);
}
inline bool independent_graph(uint8_t* rdram,uint32_t first,uint32_t second){
    const auto p=tree(rdram,first),q=tree(rdram,second);if(p.empty() || q.size()<4 || p.size()<4)return false;
    std::set<uint32_t> allocations;
    for(uint32_t a:p){allocations.insert(a);for(unsigned offset:{0x1F4u,0x16Cu,0x174u}){auto address=word(rdram,a+offset);if(address)allocations.insert(address);}}
    bool b=false,c=false,shadow=false;
    for(uint32_t a:q){
        if(allocations.contains(a))return false;
        for(unsigned offset:{0x1F4u,0x16Cu,0x174u}){auto address=word(rdram,a+offset);if(address && allocations.contains(address))return false;}
        b|=kind(rdram,a)==0x400B;c|=kind(rdram,a)==0x400C;shadow|=kind(rdram,a)==0x4007;
    }return b && c && shadow && range(word(rdram,second+0x1F4),0x1550) && range(word(rdram,second+0x16C),0x50) && range(word(rdram,second+0x174),0x18);
}
inline bool cleanup(uint8_t* rdram,recomp_context* c){
    if(cleaning || !owns(rdram,secondary))return false;
    if(byte(rdram,secondary+0x17D)==3)return true; //Already natively queued for disposal.
    ScopedGlobals global_scope(rdram);ScopedContext context_scope(c);cleaning=true;
    const uint32_t a=secondary;
    // Native recursive unlink + deferred disposal. The genuine45BD8 callback
    // must be wrapped by native_cleanup below until all identities are forgotten.
    c->r4=base(a);func_800445A4(rdram,c);cleaning=false;
    fprintf(stderr,"[CoopFactory] cleanup-request actor=%08X primary=%08X global=%08X\n",a,primary,word(rdram,player_global));
    return true;
}
inline void native_cleanup(uint8_t* rdram,recomp_context* c){
    if(!cleanup_owned(rdram,uint32_t(c->r4))){func_80045BD8(rdram,c);return;}
    ScopedGlobals global_scope(rdram);
    func_80045BD8(rdram,c);
}
// Rebuild exactly one retired logical owner. P1 uses the actual native
// canonical registration path440E4; P2 keeps a generic free manager slot.
// Call only at675F0 after the old graph/resource retirement is attested.
inline uint32_t respawn(uint8_t* r,recomp_context* c,unsigned slot,uint32_t anchor,uint32_t packed,unsigned requested_color){
 if(!enabled||!r||!c||slot>1||requested_color>=14||(packed>>16)!=0x4001||!primary_ready(r,anchor)||constructing||cleaning)return 0;
 const auto data=word(r,anchor+0x1F4);if(byte(r,data+0xF4)!=0)return 0;
 const float x=std::bit_cast<float>(word(r,anchor+0x84)),y=std::bit_cast<float>(word(r,anchor+0x88)),z=std::bit_cast<float>(word(r,anchor+0x8C));
 const float vx=std::bit_cast<float>(word(r,anchor+0xC4));
 if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(z)||!std::isfinite(vx)||x<0||x>100000||y<0||y>100000||std::abs(vx)>.25f)return 0;
 const auto manager=word(r,0x800F8810);if(!range(manager,0x18))return 0;
 const auto table=word(r,manager+0x10);const auto capacity=uint16_t(word(r,manager+8)>>16);
 if(capacity<7||capacity>150||!range(table,capacity*4))return 0;
 if(slot==0){if(word(r,table+20)!=0)return 0;}else if(free_main_slot(r,primary)<0)return 0;
 if(!free_graphics_slots(r,3)||free_actor_slots(r)<4)return 0;
 ScopedGlobals globals(r);ScopedContext registers(c);ScopedConstruction construction;
 word(r,color_global,requested_color<8?requested_color:0);word(r,custom_color_global,requested_color);
 c->r4=base(packed);c->r5=std::bit_cast<uint32_t>(x);c->r6=std::bit_cast<uint32_t>(y);c->r7=std::bit_cast<uint32_t>(z);
 func_8004613C(r,c);const auto a=uint32_t(c->r2);
 if(!actor(r,a,0x4001)||a==anchor||word(r,a+0x20)!=0x800436E0||word(r,a+0x1F4))return 0;
 cleanup_leases[a]={0x4001,0};
 const auto refuse=[&](){const auto graph=tree(r,a);if(!graph.empty())cleanup_lease(r,graph);c->r4=base(a);func_800445A4(r,c);return uint32_t(0);};
 if(slot==0){c->r4=base(a);func_800440E4(r,c);if(word(r,table+20)!=a||byte(r,a+0x17D)!=2)return refuse();}
 else if(!register_secondary(r,c,a))return refuse();
 *c=registers.saved;c->r29=ADD32(c->r29,-0x80);c->r4=base(a);func_80048A30(r,c);
 if(!primary_ready(r,a)||!independent_graph(r,anchor,a))return refuse();
 if(slot==0)primary=a;else {secondary=a;remember(r,a);}
 cleanup_leases.erase(a);
 return a;
}
// Only from675F0 outside actor iteration, with the actual current room.
// Root integration enables this flag only for an isolated bounded QA profile.
inline void frame(uint8_t* rdram,recomp_context* c,unsigned vi,unsigned room){
    if(!enabled)return;
    if(constructing){wait_once(rdram,vi,room,WaitReason::constructing,"constructor-active");return;}
    if(cleaning){wait_once(rdram,vi,room,WaitReason::cleaning,"cleanup-active");return;}
    if(secondary){
        if(room!=43){cleanup(rdram,c);enabled=false;return;}
        if(vi%120==0){log_actor(rdram,primary,"primary-live",vi);log_actor(rdram,secondary,"secondary-live",vi);}
        return;
    }
    if(attempted)return;
    if(vi<2400){wait_once(rdram,vi,room,WaitReason::before_vi,"before-spawn-vi");return;}
    if(room!=43){wait_once(rdram,vi,room,WaitReason::room,"room-is-not-43");return;}
    const uint32_t p=word(rdram,player_global);
    if(!primary_ready(rdram,p)){
        if(!actor(rdram,p,0x4001))wait_once(rdram,vi,room,WaitReason::player,"no-live-4001",p);
        else if(byte(rdram,p+0x17D)!=2)wait_once(rdram,vi,room,WaitReason::phase,"primary-not-registered-2",p);
        else if(byte(rdram,p+0x165)!=0)wait_once(rdram,vi,room,WaitReason::hidden,"primary-hidden",p);
        else if(word(rdram,p+0x130)!=0x800233B4)wait_once(rdram,vi,room,WaitReason::controls,"primary-controls-not-normal",p);
        else if(word(rdram,p+0x20)!=0x80043450)wait_once(rdram,vi,room,WaitReason::update,"primary-update-not-normal",p);
        else wait_once(rdram,vi,room,WaitReason::private_data,"primary-private-invalid",p);
        return;
    }
    const float x=std::bit_cast<float>(word(rdram,p+0x84)),y=std::bit_cast<float>(word(rdram,p+0x88)),z=std::bit_cast<float>(word(rdram,p+0x8C));
    if(!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) || x<0 || x>100000 || y<0 || y>100000){wait_once(rdram,vi,room,WaitReason::coordinates,"primary-coordinates-invalid",p);return;}
    //The real type3 drawing allocation precedes436E0. An exhausted pool
    //enters Fault_HungUp in5D96C, so reject before any actor allocation.
    const unsigned graphics_type=byte(rdram,p+0x164),graphics_free=free_graphics_slots(rdram,graphics_type);
    if(graphics_type!=3 || !graphics_free){
        attempted=true;
        fprintf(stderr,"[CoopFactory] rejected vi=%u reason=graphics-type-3-capacity type=%u free=%u count=%u flags=%08X\n",vi,graphics_type,graphics_free,word(rdram,0x800A6380),word(rdram,0x800A6384));
        return;
    }
    //436E0 allocates400B+400C via3CA5C and4007 via57978: four slots.
    const unsigned free=free_actor_slots(rdram);
    if(free<4 || free_main_slot(rdram,p)<0){attempted=true;fprintf(stderr,"[CoopFactory] rejected vi=%u reason=capacity free=%u\n",vi,free);return;}
    attempted=true;primary=p;spawn_vi=vi;constructed.clear();
    const uint32_t packed=word(rdram,p+12);
    log_actor(rdram,p,"before-primary",vi);log_globals(rdram,"before",vi);
    {
        ScopedGlobals global_scope(rdram);ScopedContext context_scope(c);
        word(rdram,color_global,color<8?color:0);word(rdram,custom_color_global,color);
        c->r4=base(packed);c->r5=std::bit_cast<uint32_t>(x+48.0f);c->r6=std::bit_cast<uint32_t>(y);c->r7=std::bit_cast<uint32_t>(z);
        ScopedConstruction construction_scope;
        construction_trace("4613C-basic-enter",c);func_8004613C(rdram,c);construction_trace("4613C-basic-return",c);
        const uint32_t candidate=uint32_t(c->r2);
        if(actor(rdram,candidate,0x4001) && candidate!=p){
            secondary=candidate;remember(rdram,candidate);
            //4613C creates a basic actor whose20 callback is the genuine
            //436E0 initializer. It does NOT invoke that initializer itself.
            if(word(rdram,candidate+0x20)!=0x800436E0 || word(rdram,candidate+0x1F4)!=0){
                fprintf(stderr,"[CoopFactory] rejected vi=%u reason=unexpected-basic-state candidate=%08X\n",vi,candidate);
                cleanup(rdram,c);secondary=0;return;
            }
            *c=context_scope.saved;c->r29=ADD32(c->r29,-0x40);
            construction_trace("register-enter",c);
            if(!register_secondary(rdram,c,candidate)){
                fprintf(stderr,"[CoopFactory] rejected vi=%u reason=registration candidate=%08X\n",vi,candidate);
                cleanup(rdram,c);secondary=0;return;
            }
            construction_trace("register-return",c);
            //Follow the first native registered update:48A30 runs4A028 then
            //dispatches the still-original actor20=436E0 exactly once.
            //3CA5C synchronously creates/attaches400B and400C;57978 attaches
            //4007. The shadow keeps its own later native initialization.
            *c=context_scope.saved;c->r29=ADD32(c->r29,-0x40);c->r4=base(candidate);
            construction_trace("48A30-init-enter",c);func_80048A30(rdram,c);construction_trace("48A30-init-return",c);
            remember(rdram,candidate);
            if(!primary_ready(rdram,candidate) || !independent_graph(rdram,p,candidate)){
                fprintf(stderr,"[CoopFactory] rejected vi=%u reason=initialization-or-graph candidate=%08X owned=%zu\n",vi,candidate,owned.size());
                cleanup(rdram,c);secondary=0;return;
            }
            fprintf(stderr,"[CoopFactory] spawned vi=%u primary=%08X secondary=%08X packed=%08X freeBefore=%u owned=%zu independentGraph=1\n",vi,p,candidate,packed,free,owned.size());
        }else fprintf(stderr,"[CoopFactory] rejected vi=%u reason=native-constructor candidate=%08X created=%zu\n",vi,candidate,constructed.size());
    }
    log_actor(rdram,p,"after-primary",vi);log_actor(rdram,secondary,"after-secondary",vi);log_globals(rdram,"after",vi);
}
}
