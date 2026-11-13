#include "bonus_factory0353.hpp"
#include "workshop_data.hpp"
#include <limits>
extern "C" void func_80051200(uint8_t*,recomp_context*);
extern "C" void func_80051368(uint8_t*,recomp_context*);
extern "C" void func_805AB970(uint8_t*,recomp_context*);
namespace workshop::bonus_factory0353 {
namespace {
bool permitted(const Environment& e){return e.pc_mode==7&&e.bonus>=1&&e.bonus<=6&&e.attempt_epoch&&!e.canonical&&!e.custom_active&&!e.deferred;}
bool range(uint32_t at,size_t bytes){return at>=0x80000400&&at<0x80800000&&bytes<=0x80800000-at;}
bool heap(uint32_t at,size_t bytes){return range(at,bytes)&&at<0x80400000&&bytes<=0x80400000-at;}
uint32_t word(uint8_t* rdram,uint32_t at){return uint32_t(MEM_W(0,int32_t(at)));}
bool actor(uint8_t* rdram,uint32_t at,unsigned kind){return heap(at,0x1FC)&&word(rdram,at)==0x19971110&&MEM_HU(12,int32_t(at))==kind&&MEM_BU(0x17D,int32_t(at))!=3;}
struct Allocator {uint32_t table=0;unsigned capacity=0;std::vector<uint32_t> slots;bool operator==(const Allocator&)const=default;};
// This is the real general actor list used by43A24/57A28. 800C4278 is
// a separate one-slot finds pool and must never host Bonus portals.
std::optional<Allocator> general_allocator(uint8_t* rdram){
    constexpr uint32_t root=0x800F87FC;const unsigned count=MEM_HU(8,int32_t(root));const auto table=word(rdram,root+0x10);
    if(!count||count>100||!heap(table,size_t(count)*4))return {};Allocator result{table,count,{}};
    for(unsigned n=0;n<count;++n){const auto slot=word(rdram,table+n*4);if(slot&&!heap(slot,0x27C))return {};for(auto earlier:result.slots)if(slot&&earlier&&slot<earlier+0x27C&&earlier<slot+0x27C)return {};result.slots.push_back(slot);}return result;
}
bool activation_queue(uint8_t* rdram,bool require_free){
    constexpr uint32_t root=0x800F882C;const unsigned capacity=MEM_HU(8,int32_t(root)),cursor=MEM_HU(4,int32_t(root)),count=MEM_HU(6,int32_t(root));const auto table=word(rdram,root+0x10),counter=word(rdram,0x800F8808);
    return capacity&&capacity<=150&&cursor<capacity&&count<=capacity&&heap(table,size_t(capacity)*4)&&heap(counter,2)&&word(rdram,0x800F8810)!=root&&(!require_free||!word(rdram,table+cursor*4));
}
struct Context {
    recomp_context* ctx;recomp_context saved;
    explicit Context(recomp_context* c):ctx(c),saved(*c){ctx->r29=ADD32(ctx->r29,-0x40);}
    ~Context(){*ctx=saved;}
};
bool stack(const recomp_context* c){return c&&uint32_t(c->r29)>=0x80002400&&uint32_t(c->r29)<0x80400000;}
bool resource_state(uint8_t* rdram){const auto table=word(rdram,0x800F84D4);if(!range(table,4))return false;const unsigned count=MEM_BU(0,int32_t(table));if(!range(table+4,size_t(count)*4))return false;for(unsigned n=0;n<count;++n){const auto entry=word(rdram,table+4+n*4);if(entry&&!range(entry,0x3E))return false;}return true;}
bool pending_state(uint8_t* rdram){const auto count=word(rdram,0x800DA830),base=word(rdram,0x800DA834),next=word(rdram,0x800DA838);return count<256&&range(base,size_t(count+1)*2)&&next==base+count*2;}
std::optional<std::vector<uint32_t>> protected_player(uint8_t* rdram){const auto player=word(rdram,0x800F8828);if(!actor(rdram,player,0x4001))return {};const auto count=MEM_HU(0x184,int32_t(player));if(count>20||!heap(player+0x188,size_t(count)*4))return {};
    std::vector<uint32_t> values{player,MEM_HU(0,int32_t(0x800F897E)),word(rdram,0x800F89D0),word(rdram,0x807F0008),count};for(unsigned off:{0xB4u,0xB8u,0xBCu})values.push_back(word(rdram,player+off));
    for(unsigned n=0;n<count;++n){const auto child=word(rdram,player+0x188+n*4);if(!heap(child,0x1FC)||word(rdram,child)!=0x19971110)return {};values.push_back(child);values.push_back(word(rdram,child+0x180));for(unsigned off:{0xB4u,0xB8u,0xBCu})values.push_back(word(rdram,child+off));}return values;
}
}
struct Factory::Impl {
    bonus_authoring0353::Plan plan;Environment env;const bonus_authoring0353::AuthoredRoom* room=nullptr;uint64_t room_epoch=0,serial=0;bool configured=false,ready=false,busy=false;
    std::optional<Allocator> allocator;uint32_t z=0;std::array<uint8_t,24> native_payload{};std::optional<Receipt> active;std::array<std::optional<Receipt>,2> receipts;bool ambiguous=false;
    bool same(const Environment& e)const{return configured&&permitted(e)&&e.bonus==env.bonus&&e.seed==env.seed&&e.attempt_epoch==env.attempt_epoch;}
    bool context(const Environment& e)const{return same(e)&&room&&room->key.course==e.native_course&&room_epoch;}
};
Factory::Factory():p(std::make_unique<Impl>()){}Factory::~Factory()=default;
void Factory::reset(){const auto serial=p->serial;p=std::make_unique<Impl>();p->serial=serial;}
bool Factory::configure(std::span<const uint8_t> original,const bonus_routes0351::Route& route,const Environment& env){
    if(!permitted(env)||route.bonus!=env.bonus||route.seed!=env.seed||p->busy)return false;
    try{auto plan=bonus_authoring0353::build(original,route);const auto res=[&](uint16_t kind){for(unsigned n=0;n<1582;++n){size_t at=0xA1C90+size_t(n)*12;if(u16(original,at)==kind)return terrain::bank(original,u32(original,at+8));}throw std::runtime_error("Bonus factory template resource absent");};
        const auto b=res(0x4208);require(b+0x38<=original.size(),"Bonus factory template truncated");const auto record=terrain::bank(original,u32(original,b+0x34));require(record+32<=original.size()&&u32(original,record)==32,"Bonus factory native route invalid");const uint32_t z=u32(original,b+4);require(navigation::coordinate(z),"Bonus factory native depth invalid");
        reset();p->plan=std::move(plan);p->env=env;p->z=z;std::copy_n(original.begin()+record+8,24,p->native_payload.begin());p->configured=true;return true;
    }catch(...){reset();return false;}
}
bool Factory::enter_room(unsigned ordinal,uint64_t epoch,const Environment& env){if(!p->same(env)||!epoch||p->busy)return false;auto found=std::find_if(p->plan.rooms.begin(),p->plan.rooms.end(),[ordinal](const auto& room){return room.ordinal==ordinal;});if(found==p->plan.rooms.end()||found->key.course!=env.native_course||found->role!=bonus_routes0351::StepRole::Ordinary)return false;
    p->room=&*found;p->room_epoch=epoch;p->ready=false;p->allocator.reset();p->receipts={};p->active.reset();return true;
}
bool Factory::room_ready(uint8_t* rdram,uint32_t info,const Environment& env){if(!p->context(env)||!rdram||!range(info,0x128)||MEM_BU(1,int32_t(info))!=p->room->key.room)return false;const auto allocator=general_allocator(rdram);if(!allocator||!activation_queue(rdram,false))return false;p->allocator=allocator;p->ready=true;return true;}
const bonus_authoring0353::Site* Factory::site(Direction direction)const{if(!p->room)return nullptr;const auto& slot=direction==Direction::Backward?p->room->backward:p->room->forward;return slot?&*slot:nullptr;}
Reply Factory::preload(uint8_t* rdram,recomp_context* ctx,const Environment& env){
    if(!p->context(env)||!rdram||!stack(ctx)||p->busy)return {Status::Denied,{},"factory-scope-or-stack"};if(!resource_state(rdram))return {Status::Unavailable,{},"resource-cache-not-ready"};
    Context save(ctx);ctx->r4=0x4208;func_80051200(rdram,ctx);if(ctx->r2)return {Status::Cached,{},"native-template-cached"};if(!pending_state(rdram))return {Status::Unavailable,{},"native-preload-queue-not-ready"};*ctx=save.saved;ctx->r29=ADD32(ctx->r29,-0x40);ctx->r4=0x4208;func_80051368(rdram,ctx);return {Status::Scheduled,{},"native-preload-requested-not-yet-certified-loaded"};
}
Reply Factory::spawn(Direction direction,uint8_t* rdram,recomp_context* ctx,const Environment& env){
    if(!p->context(env)||!p->ready||!rdram||!stack(ctx)||p->busy)return {Status::Denied,{},"factory-not-ready"};const auto allocator=general_allocator(rdram);if(!allocator||!p->allocator||*allocator!=*p->allocator)return {Status::Denied,{},"stale-general-native-allocator"};const auto* chosen=site(direction);if(!chosen)return {Status::Unavailable,{},"second-site-unproven"};if(chosen->reuse_native_pot)return {Status::ExistingNativeSource,{},"reuse-placed-native-pot-no-duplicate"};
    const unsigned index=direction==Direction::Backward?0:1;if(p->receipts[index])return {Status::Created,p->receipts[index],"existing-fresh-receipt"};if(chosen->template_kind!=0x4208||!resource_state(rdram))return {Status::Unavailable,{},"template-unavailable"};const auto player=word(rdram,0x800F8828);if(MEM_BU(2,int32_t(0x800F7EA0))!=7||!actor(rdram,player,0x4001)||MEM_BU(0x17D,int32_t(player))!=4)return {Status::Unavailable,{},"player-not-active-in-native-room"};const auto before=protected_player(rdram);if(!before)return {Status::Unavailable,{},"player-not-ready"};
    Context save(ctx);ctx->r4=0x4208;func_80051200(rdram,ctx);if(!ctx->r2)return {Status::Unavailable,{},"preload-not-complete"};*ctx=save.saved;ctx->r29=ADD32(ctx->r29,-0x40);
    p->busy=true;p->ambiguous=false;p->active=Receipt{p->room->ordinal,p->room->key.room,direction,env.attempt_epoch,p->room_epoch,0,0,0,chosen->point,0,0};struct Busy{Impl& p;~Busy(){p.busy=false;p.active.reset();}} busy{*p};
    bool available=false;for(auto slot:allocator->slots)if(slot&&MEM_BU(6,int32_t(slot))==3)available=true;
    if(!available||!activation_queue(rdram,true))return {Status::Unavailable,{},"native-general-actor-or-activation-list-full"};
    const auto xyz=uint32_t(ctx->r29)+0x20;MEM_W(0,int32_t(xyz))=chosen->point[0];MEM_W(4,int32_t(xyz))=chosen->point[1];MEM_W(8,int32_t(xyz))=p->z;
    ctx->r4=0x4208;ctx->r5=gpr(int32_t(xyz));func_805AB970(rdram,ctx);const auto actor_address=uint32_t(ctx->r2);
    if(!actor_address&&!p->ambiguous&&p->active&&!p->active->birth)return {Status::Unavailable,{},"native-general-pool-or-dynamic-id-full"};
    if(p->ambiguous||!p->active||!p->active->birth||p->active->actor!=actor_address||!actor(rdram,actor_address,0x4208)||std::find(allocator->slots.begin(),allocator->slots.end(),actor_address)==allocator->slots.end()||MEM_BU(5,int32_t(actor_address))||general_allocator(rdram)!=allocator)return {Status::Failed,{},"fresh-constructor-proof-missing"};
    auto result=*p->active;const auto data=word(rdram,actor_address+0x1F4);if(!heap(data,0x264))return {Status::Failed,{},"native-pot-data-invalid"};const auto record=word(rdram,data+0x260);if(!range(record,24))return {Status::Failed,{},"native-pot-route-invalid"};const gpr base=gpr(int32_t(record));for(unsigned n=0;n<24;++n)if(MEM_BU(n,base)!=p->native_payload[n])return {Status::Failed,{},"native-pot-route-template-mismatch"};
    if(word(rdram,actor_address+0x3C)!=chosen->point[0]||word(rdram,actor_address+0x40)!=chosen->point[1]||word(rdram,actor_address+0x180)||protected_player(rdram)!=before)return {Status::Failed,{},"native-factory-altered-player-or-pot-pose"};
    result.private_data=data;result.native_record=record;result.allocator_list=0x800F87FC;result.allocator_table=allocator->table;result.allocator_capacity=allocator->capacity;p->receipts[index]=result;return {Status::Created,result,"native-fresh-pot-owned"};
}
void Factory::created(recomp_context* ctx,unsigned room,const Environment& env){
    if(!p->same(env)||!ctx)return;const auto address=uint32_t(ctx->r4);for(auto& receipt:p->receipts)if(receipt&&receipt->actor==address)receipt.reset();if(p->serial==std::numeric_limits<uint64_t>::max())return;const auto birth=++p->serial;
    if(!p->busy||!p->active||room!=p->active->room||(uint32_t(ctx->r5)&0xFFFFFC00u)!=0x42082000u||(uint32_t(ctx->r5)&0x3FFu)<0x200u||uint32_t(ctx->r6)!=p->active->point[0]||uint32_t(ctx->r7)!=p->active->point[1])return;
    if(p->active->birth){p->ambiguous=true;return;}p->active->actor=address;p->active->packed=uint32_t(ctx->r5);p->active->birth=birth;
}
void Factory::destroyed(uint32_t actor,const Environment& env){if(!p->same(env))return;for(auto& receipt:p->receipts)if(receipt&&receipt->actor==actor)receipt.reset();if(p->active&&p->active->actor==actor)p->ambiguous=true;}
std::optional<Receipt> Factory::receipt(Direction direction,const Environment& env)const{if(!p->context(env))return {};return p->receipts[direction==Direction::Backward?0:1];}
}
