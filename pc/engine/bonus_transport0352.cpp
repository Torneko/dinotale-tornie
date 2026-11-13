#include "bonus_transport0352.hpp"
#include "bonus_finale_links0353.hpp"
#include "workshop_data.hpp"
#include <map>
#include <limits>
extern "C" void func_8005D29C(uint8_t*,recomp_context*);
extern "C" void func_8002BEB0(uint8_t*,recomp_context*);
extern "C" void func_800443EC(uint8_t*,recomp_context*);
extern "C" void func_80040C0C(uint8_t*,recomp_context*);
extern "C" void func_80047AB8(uint8_t*,recomp_context*);
namespace workshop::bonus_transport0352 {
namespace {
bool permitted(const Environment& e){return e.pc_mode==7&&e.bonus>=1&&e.bonus<=6&&e.attempt_epoch&&!e.canonical&&!e.custom_active&&!e.deferred;}
bool range(uint32_t a,size_t bytes){return a>=0x80000400&&a<0x80800000&&bytes<=0x80800000-a;}
bool heap(uint32_t a,size_t bytes){return a>=0x80000400&&a<0x80400000&&bytes<=0x80400000-a;}
uint32_t word(uint8_t* rdram,uint32_t at){return uint32_t(MEM_W(0,int32_t(at)));}
bool actor(uint8_t* rdram,uint32_t at,unsigned kind=0){return heap(at,0x1FC)&&word(rdram,at)==0x19971110&&(!kind||MEM_HU(12,int32_t(at))==kind)&&MEM_BU(0x17D,int32_t(at))!=3;}
std::array<uint32_t,3> scale(uint8_t* rdram,uint32_t a){return {word(rdram,a+0xB4),word(rdram,a+0xB8),word(rdram,a+0xBC)};}
bonus_rooms0351::Point arrival_point(const PreparedLink& route){return route.authored_arrival?route.authored_arrival->point:route.arrival.placement.point;}
using Key=std::array<uint32_t,4>;
struct Birth {uint64_t serial=0;unsigned room=177;uint32_t packed=0;bonus_rooms0351::Point point{};};
struct Child {uint32_t address=0,parent=0;uint64_t birth=0;unsigned kind=0,relation=0;std::array<uint32_t,3> scale{};bool operator==(const Child&)const=default;};
struct Binding {unsigned link=0;uint64_t birth=0;uint32_t original=0,private_data=0;};
struct Pending {Ticket ticket;Phase phase=Phase::Departed;uint32_t level_info=0;unsigned health=0,fruits=0;std::array<uint32_t,3> scale{};std::vector<Child> children;};
}
struct Transport::Impl {
    Environment env;View state;std::vector<PreparedLink> routes;std::map<Key,unsigned> keys,backward_keys;
    std::map<uint32_t,Birth> births;std::map<uint32_t,Binding> bindings;std::optional<Pending> pending;std::optional<Receipt> arrival;
    uint64_t serial=0,transition=0;bool restoring=false;
    bool same(const Environment& e)const{return state.configured&&permitted(e)&&e.bonus==env.bonus&&e.seed==env.seed&&e.attempt_epoch==env.attempt_epoch;}
    void cancel(const char* why){pending.reset();arrival.reset();state.phase=Phase::Rejected;state.reason=why;}
    bool child_snapshot(uint8_t* rdram,uint32_t player,std::vector<Child>& result)const{
        if(!actor(rdram,player,0x4001))return false;const unsigned count=MEM_HU(0x184,int32_t(player));if(count>20||!heap(player+0x188,size_t(count)*4))return false;
        result.clear();std::set<uint32_t> seen;
        for(unsigned n=0;n<count;++n){const uint32_t a=word(rdram,player+0x188+n*4);const auto found=births.find(a);if(!actor(rdram,a)||found==births.end()||!seen.insert(a).second)return false;
            result.push_back({a,word(rdram,a+0x180),found->second.serial,MEM_HU(12,int32_t(a)),MEM_BU(0x17C,int32_t(a)),bonus_transport0352::scale(rdram,a)});
        }return true;
    }
    bool player_matches(uint8_t* rdram,const Pending& w)const{
        const auto found=births.find(w.ticket.player);std::vector<Child> now;
        return found!=births.end()&&found->second.serial==w.ticket.player_birth&&actor(rdram,w.ticket.player,0x4001)
            &&bonus_transport0352::scale(rdram,w.ticket.player)==w.scale&&child_snapshot(rdram,w.ticket.player,now)&&now==w.children
            &&MEM_HU(0,int32_t(0x800F897E))==w.health&&word(rdram,0x800F89D0)==w.fruits;
    }
    void release(uint8_t* rdram,uint32_t a,const Binding& binding){
        const auto birth=births.find(a);if(!rdram||binding.link>=routes.size()||birth==births.end()||birth->second.serial!=binding.birth||!binding.original||!actor(rdram,a,uint16_t(routes[binding.link].source.packed>>16)))return;
        const auto data=word(rdram,a+0x1F4);if(data==binding.private_data&&heap(data,0x264)&&word(rdram,data+0x260)==routes[binding.link].record_address)MEM_W(0x260,int32_t(data))=int32_t(binding.original);
    }
};
Transport::Transport():p(std::make_unique<Impl>()){}Transport::~Transport()=default;
View Transport::view()const{return p->state;}
std::span<const PreparedLink> Transport::links()const{return p->routes;}
unsigned Transport::shared_sources()const{return unsigned(p->backward_keys.size());}
void Transport::reset(uint8_t* rdram){for(const auto& [a,binding]:p->bindings)p->release(rdram,a,binding);const auto serial=p->serial,transition=p->transition;p=std::make_unique<Impl>();p->serial=serial;p->transition=transition;}
bool Transport::retry_reset(uint8_t* rdram,const Environment& env){
 if(!rdram||!p->same(env))return false;
 auto routes=p->routes;auto keys=p->keys;auto backward=p->backward_keys;const auto bound=p->env;
 reset(rdram);p->env=bound;p->routes=std::move(routes);p->keys=std::move(keys);p->backward_keys=std::move(backward);
 p->state={true,true,Phase::Idle,177,unsigned(p->routes.size()),0,{}};return true;
}
bool Transport::configure(std::span<const uint8_t> original,const bonus_routes0351::Route& route,std::span<const Link> requested,const Environment& env){
    if(!permitted(env)){p->state.configured=false;p->cancel("environment-denied");return false;}
    if(std::any_of(p->bindings.begin(),p->bindings.end(),[](const auto& row){return row.second.original!=0;})){p->state.configured=false;p->cancel("reset-required-before-reconfigure");return false;}
    try{
        require(bonus_routes0351::validate(route)&&route.bonus==env.bonus&&route.seed==env.seed,"Bonus transport route/attempt mismatch");
        require(!requested.empty()&&requested.size()<=max_links,"Bonus transport link count invalid");const auto catalog=bonus_rooms0351::read(original);std::vector<PreparedLink> verified;std::map<Key,unsigned> keys;
        for(const auto& link:requested){
            require(link.from_ordinal<route.steps.size()&&link.to_ordinal<route.steps.size()&&((link.from_ordinal+1==link.to_ordinal)||(link.to_ordinal+1==link.from_ordinal)),"Bonus transport requires adjacent route steps");
            const auto& from=route.steps[link.from_ordinal];const auto& to=route.steps[link.to_ordinal];require(from.role==bonus_routes0351::StepRole::Ordinary&&to.role==bonus_routes0351::StepRole::Ordinary,"Bonus transport finale requires its own protocol");
            const auto* source=catalog.find(from.key.room);const auto* dest=catalog.find(to.key.room);require(source&&dest&&source->world==env.bonus&&dest->world==env.bonus,"Bonus transport source world mismatch");
            require(from.key.course==source->course&&from.key.root==source->root&&to.key.course==dest->course&&to.key.root==dest->root,"Bonus transport room provenance mismatch");
            require(dest->entry.has_value()&&dest->entry->kind!=bonus_rooms0351::AnchorKind::UnplacedNativeTarget,"Bonus transport destination needs native creation proof");
            const auto found=std::find_if(source->passages.begin(),source->passages.end(),[&](const auto& edge){return edge.source.index==link.source_pot_index&&edge.native.editable_pot;});require(found!=source->passages.end(),"Bonus transport needs a placed native source pot");
            const Key key{source->id,found->source.packed,found->source.point[0],found->source.point[1]};require(keys.emplace(key,unsigned(verified.size())).second,"Bonus transport source pot is ambiguous");
            verified.push_back({link,source->id,dest->id,source->course,dest->course,dest->root,found->source,*dest->entry,found->native.native_payload,slab+uint32_t(verified.size())*slot_bytes,{},false});
        }
        const auto serial=p->serial,transition=p->transition;p=std::make_unique<Impl>();p->serial=serial;p->transition=transition;p->env=env;p->routes=std::move(verified);p->keys=std::move(keys);p->state={true,true,Phase::Idle,177,unsigned(p->routes.size()),0,{}};return true;
    }catch(const std::exception& error){p->state.configured=false;p->cancel(error.what());return false;}
}
bool Transport::configure_authored(std::span<const uint8_t> original,const bonus_routes0351::Route& route,const Environment& env){
 if(!permitted(env)){p->state.configured=false;p->cancel("authored-environment-denied");return false;}
 if(std::any_of(p->bindings.begin(),p->bindings.end(),[](const auto& row){return row.second.original!=0;})){p->state.configured=false;p->cancel("reset-required-before-authored-reconfigure");return false;}
 try{
  require(bonus_routes0351::validate(route)&&route.bonus==env.bonus&&route.seed==env.seed,"Authored transport route/attempt mismatch");
  const auto plan=bonus_authoring0353::build(original,route);
  const auto catalog=bonus_rooms0351::read(original);const auto* native_source=catalog.find(43);
  require(native_source,"Authored transport pot calibration room missing");
  const auto prototype=std::find_if(native_source->passages.begin(),native_source->passages.end(),[](const auto& e){return e.source.index==119&&e.native.kind==0x4208&&e.native.editable_pot;});
  require(prototype!=native_source->passages.end(),"Authored transport native4208 payload missing");
  std::vector<PreparedLink> verified;std::map<Key,unsigned> keys,backward_keys;
  auto room_for=[&](unsigned ordinal)->const bonus_authoring0353::AuthoredRoom&{
   const auto found=std::find_if(plan.rooms.begin(),plan.rooms.end(),[&](const auto& r){return r.ordinal==ordinal;});
   require(found!=plan.rooms.end()&&found->role==bonus_routes0351::StepRole::Ordinary,"Authored transport needs ordinary room provenance");return *found;
  };
  for(unsigned from=0;from<route.steps.size();++from){
   if(route.steps[from].role!=bonus_routes0351::StepRole::Ordinary)continue;
   for(int delta:{-1,1}){
    const int target=int(from)+delta;if(target<0||target>=int(route.steps.size())||route.steps[target].role!=bonus_routes0351::StepRole::Ordinary)continue;
    const auto& source=room_for(from);const auto& dest=room_for(unsigned(target));
    const auto& preferred_site=delta<0?source.backward:source.forward;
    const auto& site=preferred_site?preferred_site:source.forward;
    const auto& preferred_contact=delta<0?dest.forward:dest.backward;
    const auto& contact=preferred_contact?preferred_contact:dest.forward;
    require(site&&contact&&source.key.course&&source.key.root&&dest.key.course&&dest.key.root,"Authored transport site/course absent");
    PreparedLink link;link.ordinal={from,unsigned(target),site->source_placement_index.value_or(UINT32_MAX)};
    link.source_room=source.key.room;link.destination_room=dest.key.room;link.source_course=*source.key.course;link.destination_course=*dest.key.course;link.destination_root=*dest.key.root;
    link.native_payload=prototype->native.native_payload;link.requires_factory=site->needs_factory;link.authored_arrival=*contact;
    if(site->reuse_native_pot){const auto* sr=catalog.find(source.key.room);require(sr,"Authored native source missing");const auto found=std::find_if(sr->passages.begin(),sr->passages.end(),[&](const auto& e){return e.native.editable_pot&&e.source.index==site->source_placement_index;});
     require(found!=sr->passages.end()&&found->source.point==site->point&&found->source.packed==site->source_packed,"Authored native source mismatch");link.source=found->source;link.native_payload=found->native.native_payload;
    }else{
     require(site->template_kind==0x4208,"Authored factory must use the proven4208 template");
     link.source={UINT32_MAX,0,0x42082000u,site->point}; //8003BA90 dynamic-factory bit.
    }
    require(navigation::coordinate(contact->point[0])&&navigation::coordinate(contact->point[1])&&(contact->point[0]||contact->point[1]),"Authored contact invalid");
    const Key source_key{link.source_room,link.source.packed,link.source.point[0],link.source.point[1]};
    const auto [key,inserted]=keys.emplace(source_key,unsigned(verified.size()));
    if(!inserted){
     const auto& prior=verified.at(key->second);
     require(source.needs_second_site&&source.forward&&!source.backward&&delta==1&&prior.ordinal.from_ordinal==from&&prior.ordinal.to_ordinal+1==from&&link.ordinal.to_ordinal==from+1&&prior.native_payload==link.native_payload&&!backward_keys.contains(source_key),"Authored source pot collision");
     backward_keys.emplace(source_key,key->second);key->second=unsigned(verified.size());
    }
    link.record_address=slab+uint32_t(verified.size())*slot_bytes;verified.push_back(std::move(link));
   }
  }
  require(!verified.empty()&&verified.size()<=max_links,"Authored transport capacity invalid");
  const auto serial=p->serial,transition=p->transition;p=std::make_unique<Impl>();p->serial=serial;p->transition=transition;p->env=env;p->routes=std::move(verified);p->keys=std::move(keys);p->backward_keys=std::move(backward_keys);p->state={true,true,Phase::Idle,177,unsigned(p->routes.size()),0,{}};return true;
 }catch(const std::exception& error){p->state.configured=false;p->cancel(error.what());return false;}
}
bool Transport::configure_authored_finale(std::span<const uint8_t> original,const bonus_routes0351::Route& route,const bonus_finale_entries0353::EntrySpec& entry,const Environment& env){
 if(!configure_authored(original,route,env))return false;
 try{
  const auto link=bonus_finale_links0353::make_link(original,route,env,entry,unsigned(p->routes.size()));
  const Key key{link.source_room,link.source.packed,link.source.point[0],link.source.point[1]};
  const auto found=p->keys.find(key);
  if(found!=p->keys.end()){
   const auto& prior=p->routes.at(found->second);const auto plan=bonus_authoring0353::build(original,route);
   const auto source=std::find_if(plan.rooms.begin(),plan.rooms.end(),[&](const auto& r){return r.ordinal==link.ordinal.from_ordinal;});
   require(source!=plan.rooms.end()&&source->needs_second_site&&source->forward&&!source->backward&&prior.ordinal.from_ordinal==link.ordinal.from_ordinal&&prior.ordinal.to_ordinal+1==prior.ordinal.from_ordinal&&link.ordinal.to_ordinal==link.ordinal.from_ordinal+1&&prior.native_payload==link.native_payload&&!p->backward_keys.contains(key),"Finale append source key collides outside explicit shared-site policy");
   p->backward_keys.emplace(key,found->second);found->second=unsigned(p->routes.size());
  }else p->keys.emplace(key,unsigned(p->routes.size()));
  p->routes.push_back(link);p->state.links=unsigned(p->routes.size());return true;
 }catch(const std::exception& error){p->state.configured=false;p->cancel(error.what());return false;}
}
void Transport::created(recomp_context* c,unsigned room,const Environment& env){
    if(!p->same(env)||p->restoring||!c)return;const uint32_t a=uint32_t(c->r4);if(!heap(a,0x1FC))return;
    if(p->serial==std::numeric_limits<uint64_t>::max()){p->cancel("birth-epoch-exhausted");return;}
    if(p->pending&&(p->pending->ticket.player==a||std::any_of(p->pending->children.begin(),p->pending->children.end(),[a](const Child& child){return child.address==a;})))p->cancel("carried-actor-reused");
    const auto birth=++p->serial;p->bindings.erase(a);p->births[a]={birth,room,uint32_t(c->r5),{uint32_t(c->r6),uint32_t(c->r7)}};
    const auto found=p->keys.find({room,uint32_t(c->r5),uint32_t(c->r6),uint32_t(c->r7)});
    if(found!=p->keys.end()&&!p->routes[found->second].requires_factory&&p->state.current_room==room&&env.native_course==p->routes[found->second].source_course)p->bindings.emplace(a,Binding{found->second,birth,0,0});
}
bool Transport::bind_factory(uint8_t* rdram,const FactoryWitness& witness,const Environment& env){
 if(!rdram||!p->same(env)||p->restoring||p->pending||p->state.current_room!=witness.room)return false;
 const auto birth=p->births.find(witness.actor);
 if(birth==p->births.end()||birth->second.room!=witness.room||birth->second.packed!=witness.packed||birth->second.point!=witness.point||!actor(rdram,witness.actor,0x4208)||
    word(rdram,witness.actor+0x1F4)!=witness.private_data||!heap(witness.private_data,0x264)||word(rdram,witness.private_data+0x260)!=witness.native_record||!range(witness.native_record,24))return false;
 if((witness.packed&0xFFFFFC00u)!=0x42082000u||(witness.packed&0x3FFu)<0x200u||(witness.packed&0x3FFu)==0x3FFu)return false;
 const auto found=p->keys.find({witness.room,0x42082000u,witness.point[0],witness.point[1]});
 if(found==p->keys.end())return false;const auto& link=p->routes[found->second];
 if(!link.requires_factory||env.native_course!=link.source_course||p->bindings.contains(witness.actor))return false;
 const gpr record=gpr(int32_t(witness.native_record));
 for(unsigned n=0;n<24;++n)if(MEM_BU(n,record)!=link.native_payload[n])return false;
 p->bindings.emplace(witness.actor,Binding{found->second,birth->second.serial,0,0});return true;
}
void Transport::destroy(uint8_t* rdram,uint32_t a,const Environment& env){
    if(!p->same(env)||p->restoring)return;auto found=p->bindings.find(a);if(found!=p->bindings.end()){p->release(rdram,a,found->second);p->bindings.erase(found);}
    if(p->pending&&(p->pending->ticket.player==a||std::any_of(p->pending->children.begin(),p->pending->children.end(),[a](const Child& child){return child.address==a;})))p->cancel("carried-actor-destroyed");p->births.erase(a);
}
void Transport::prepare(uint8_t* rdram,uint32_t a,const Environment& env,bool select_direction){
    if(!p->same(env)||p->restoring||!rdram)return;auto found=p->bindings.find(a);if(found==p->bindings.end())return;auto& binding=found->second;
    const auto birth_record=p->births.find(a);if(birth_record==p->births.end()||birth_record->second.serial!=binding.birth)return;
    const auto& born=birth_record->second;const uint32_t packed=p->routes[binding.link].requires_factory?(born.packed&0xFFFFFC00u):born.packed;
    const Key source_key{born.room,packed,born.point[0],born.point[1]};const auto back=p->backward_keys.find(source_key);
    if(select_direction&&back!=p->backward_keys.end()){
     const unsigned selected=env.prefer_backward?back->second:p->keys.at(source_key);
     if(binding.link!=selected){
      if(binding.original){const auto data=word(rdram,a+0x1F4);if(data!=binding.private_data||!heap(data,0x264)||word(rdram,data+0x260)!=p->routes[binding.link].record_address)return;MEM_W(0x260,int32_t(data))=int32_t(binding.original);}
      binding.link=selected;
     }
    }
    const auto& route=p->routes[binding.link];
    const auto birth=p->births.find(a);if(birth==p->births.end()||birth->second.serial!=binding.birth||p->state.current_room!=route.source_room||env.native_course!=route.source_course||!actor(rdram,a,uint16_t(route.source.packed>>16)))return;
    const uint32_t data=word(rdram,a+0x1F4);if(!heap(data,0x264))return;const auto original=word(rdram,data+0x260);if(original==route.record_address&&binding.private_data==data)return;
    if(!range(original,24)||(original>=slab&&original<slab_end))return;
    const gpr original_base=gpr(int32_t(original)),copy_base=gpr(int32_t(route.record_address));
    for(unsigned n=0;n<24;++n)if(MEM_BU(n,original_base)!=route.native_payload[n])return;
    const auto camera=navigation::camera_origin(arrival_point(route));for(unsigned n=0;n<24;++n)MEM_B(n,copy_base)=route.native_payload[n];
    MEM_W(0,int32_t(route.record_address))=-1;MEM_B(4,int32_t(route.record_address))=uint8_t(route.destination_room);MEM_W(8,int32_t(route.record_address))=int32_t(camera[0]);MEM_W(12,int32_t(route.record_address))=int32_t(camera[1]);
    binding.original=original;binding.private_data=data;MEM_W(0x260,int32_t(data))=int32_t(route.record_address);
}
void Transport::departing(uint8_t* rdram,recomp_context* c,const Environment& env){
    if(!p->same(env)||p->restoring||!rdram||!c)return;const uint32_t at=uint32_t(c->r4);
    if(at<slab||at>=slab_end||(at-slab)%slot_bytes){p->cancel("unowned-native-route");return;}
    const unsigned index=(at-slab)/slot_bytes;if(index>=p->routes.size()){p->cancel("unowned-slot");return;}const auto& route=p->routes[index];
    if(p->state.current_room!=route.source_room||env.native_course!=route.source_course){p->cancel("wrong-source-course-room");return;}
    bool live=false;for(const auto& [a,binding]:p->bindings){const auto birth=p->births.find(a);if(binding.link==index&&birth!=p->births.end()&&birth->second.serial==binding.birth&&actor(rdram,a,uint16_t(route.source.packed>>16))&&word(rdram,a+0x1F4)==binding.private_data&&heap(binding.private_data,0x264)&&word(rdram,binding.private_data+0x260)==at)live=true;}
    const uint32_t player=word(rdram,0x800F8828);const auto birth=p->births.find(player);if(!live||birth==p->births.end()||!actor(rdram,player,0x4001)){p->cancel("unknown-source-player");return;}
    if(p->pending&&p->pending->ticket.link==index&&p->pending->phase==Phase::Departed)return;
    if(p->transition==std::numeric_limits<uint64_t>::max()){p->cancel("transition-epoch-exhausted");return;}
    Pending next;next.ticket={env.attempt_epoch,++p->transition,index,route.ordinal.from_ordinal,route.ordinal.to_ordinal,route.source_room,route.destination_room,route.source_course,route.destination_course,route.destination_root,player,birth->second.serial};next.health=MEM_HU(0,int32_t(0x800F897E));next.fruits=word(rdram,0x800F89D0);next.scale=scale(rdram,player);
    if(!p->child_snapshot(rdram,player,next.children)){p->cancel("unknown-carried-child");return;}
    p->arrival.reset();p->pending=std::move(next);p->state.phase=Phase::Departed;p->state.reason.clear();
}
std::optional<Ticket> Transport::pending_ticket(const Environment& env)const{if(!p->same(env)||!p->pending)return {};return p->pending->ticket;}
void Transport::enter_room(unsigned room,const Environment& env){
    if(!p->same(env)||p->restoring)return;p->state.current_room=room;
    if(!p->pending){p->state.phase=Phase::Idle;return;}
    auto& next=*p->pending;if(next.phase!=Phase::Departed||room!=next.ticket.destination_room||env.native_course!=next.ticket.destination_course){p->cancel("destination-course-room-or-epoch-mismatch");return;}next.phase=Phase::Entered;p->state.phase=Phase::Entered;
}
void Transport::room_ready(uint8_t* rdram,recomp_context* c,const Environment& env){
    if(!p->same(env)||p->restoring||!p->pending||p->pending->phase!=Phase::Entered||!rdram||!c)return;auto& next=*p->pending;const auto info=uint32_t(c->r4);
    if(env.native_course!=next.ticket.destination_course){p->cancel("destination-course-not-installed");return;}
    if(range(info,0x128)&&MEM_BU(1,int32_t(info))==next.ticket.destination_room){next.level_info=info;next.phase=Phase::Ready;p->state.phase=Phase::Ready;}
}
void Transport::frame(uint8_t* rdram,recomp_context* c,const Environment& env){
    if(!p->same(env)||p->restoring||!p->pending||p->pending->phase!=Phase::Ready||!rdram||!c)return;const Pending work=*p->pending;const auto& route=p->routes[work.ticket.link];
    if(env.native_course!=work.ticket.destination_course||p->state.current_room!=work.ticket.destination_room||!range(work.level_info,0x128)||MEM_BU(1,int32_t(work.level_info))!=work.ticket.destination_room||!p->player_matches(rdram,work)){p->cancel("invalid-carried-arrival-state");return;}
    const auto queue=word(rdram,0x800F8770);if(!heap(queue,0x84)){p->cancel("invalid-native-carry-queue");return;}
    p->pending.reset();p->restoring=true;struct Restore{recomp_context* c;recomp_context saved;bool& restoring;~Restore(){*c=saved;restoring=false;}} restore{c,*c,p->restoring};uint32_t restored=0;bool drained=false;
    for(unsigned n=0;n<150;++n){*c=restore.saved;c->r4=gpr(int32_t(queue));func_8005D29C(rdram,c);const auto item=uint32_t(c->r2);if(!item){drained=true;break;}if(!actor(rdram,item)){p->cancel("invalid-queued-actor");return;}if(MEM_HU(12,int32_t(item))!=0x4001)continue;if(item!=work.ticket.player||restored){p->cancel("different-queued-player");return;}
        *c=restore.saved;c->r4=gpr(int32_t(item));func_8002BEB0(rdram,c);*c=restore.saved;c->r4=gpr(int32_t(item));func_800443EC(rdram,c);if(!c->r2){p->cancel("native-activation-refused");return;}
        MEM_W(0x3C,int32_t(item))=int32_t(arrival_point(route)[0]);MEM_W(0x40,int32_t(item))=int32_t(arrival_point(route)[1]);const float z=std::bit_cast<float>(u32(route.native_payload,16));MEM_W(0x44,int32_t(item))=int32_t(z<500.5f?0x43FA2B85u:0x43FA547Bu);
        *c=restore.saved;c->r4=gpr(int32_t(item));func_80040C0C(rdram,c);MEM_B(0x165,int32_t(item))=0;*c=restore.saved;c->r4=gpr(int32_t(item+0x84));c->r5=gpr(int32_t(item));func_80047AB8(rdram,c);MEM_W(0x130,int32_t(item))=int32_t(0x800233B4);restored=item;
    }
    const auto camera=navigation::camera_origin(arrival_point(route));
    if(!drained||!restored||!p->player_matches(rdram,work)||word(rdram,work.level_info+0x11C)!=camera[0]||word(rdram,work.level_info+0x120)!=camera[1]){p->cancel("arrival-preservation-or-camera-failed");return;}
    p->arrival=Receipt{work.ticket,work.health,work.fruits,unsigned(work.children.size())};p->state.phase=Phase::Arrived;p->state.completed_epoch=work.ticket.transition_epoch;p->state.reason.clear();
}
std::optional<Receipt> Transport::take_arrival(const Environment& env){if(!p->same(env)||!p->arrival)return {};auto result=p->arrival;p->arrival.reset();return result;}
}
