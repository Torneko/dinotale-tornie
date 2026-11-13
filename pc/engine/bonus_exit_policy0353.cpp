#include "bonus_exit_policy0353.hpp"
#include "workshop_data.hpp"
#include <algorithm>
#include <limits>
namespace workshop::bonus_exit_policy0353 {
namespace {
bool permitted(const Environment& e){return e.pc_mode==7&&e.bonus>=1&&e.bonus<=6&&e.attempt_epoch&&!e.canonical&&!e.custom_active&&!e.deferred;}
bool range(uint32_t at,size_t bytes){return at>=0x80000400&&at<0x80400000&&bytes<=0x80400000-at;}
uint32_t word(uint8_t* rdram,uint32_t at){return uint32_t(MEM_W(0,int32_t(at)));}
bool identity(const RetireAction& a,uint8_t* rdram){return rdram&&range(a.actor,0x1FC)&&word(rdram,a.actor)==0x19971110&&(word(rdram,a.actor+12)&0xFFFF03FFu)==(a.packed&0xFFFF03FFu)&&word(rdram,a.actor+0x3C)==a.point[0]&&word(rdram,a.actor+0x40)==a.point[1];}
bool dynamic_initialized(const RetireAction& a,uint8_t* rdram){
 if(a.reason!=RetireReason::OriginalDynamicPot)return true;
 const auto data=word(rdram,a.actor+0x1F4);if(!range(data,0x264))return false;
 const auto payload=word(rdram,data+0x260);if(payload<0x80000400||payload>=0x80800000||24>0x80800000-payload)return false;
 for(unsigned n=0;n<24;++n)if(MEM_BU(int(n),int32_t(payload))!=a.expected_native_payload[n])return false;return true;
}
bool detached_leaf(const RetireAction& a,uint8_t* rdram){return !word(rdram,a.actor+0x180)&&!MEM_HU(0x184,int32_t(a.actor));}
}
struct Policy::Impl {
 struct Witness{RetireAction action;bool queued=false;};
 struct DynamicPot{uint16_t kind;std::array<uint8_t,24> payload;};
 std::array<std::optional<DynamicPot>,24> dynamic_pots;
 bonus_authoring0353::Plan plan;Environment env;const bonus_authoring0353::AuthoredRoom* room=nullptr;
 std::vector<Witness> witnessed;uint64_t serial=0,room_epoch=0;bool configured=false,ready=false;
 bool same(const Environment& e)const{return configured&&permitted(e)&&e.bonus==env.bonus&&e.seed==env.seed&&e.attempt_epoch==env.attempt_epoch;}
 bool context(const Environment& e)const{return same(e)&&room&&room->key.course==e.native_course&&room_epoch;}
 const Witness* find(const RetireAction& a)const{auto it=std::find_if(witnessed.begin(),witnessed.end(),[&](const auto& w){return w.action==a;});return it==witnessed.end()?nullptr:&*it;}
};
Policy::Policy():p(std::make_unique<Impl>()){}Policy::~Policy()=default;
void Policy::reset(){const auto serial=p->serial;p=std::make_unique<Impl>();p->serial=serial;}
bool Policy::configure(std::span<const uint8_t> original,const bonus_routes0351::Route& route,const Environment& e){
 if(!permitted(e)||route.bonus!=e.bonus||route.seed!=e.seed){reset();return false;}
 try{auto plan=bonus_authoring0353::build(original,route);
  // Only the five original unplaced pot resources are eligible. Their
  // exact native course table, resource family and stage7 record are read
  // from the strictly verified original ROM. Reward/hidden creators survive.
  auto catalog=bonus_rooms0351::read(original);navigation::Source source(original);std::array<std::optional<Impl::DynamicPot>,24> dynamic;
  for(unsigned course=0;course<24;++course){const auto kind=u16(original,0xA6F0C+course*2);
   if(kind!=0x426B&&kind!=0x4269&&kind!=0x428C&&kind!=0x420C&&kind!=0x4278)continue;
   if(std::any_of(catalog.rooms.begin(),catalog.rooms.end(),[&](const auto& room){return std::any_of(room.placements.begin(),room.placements.end(),[&](const auto& item){return item.packed>>16==kind;});}))continue;
   const auto passage=source.passage(kind,true);if(!passage)continue;const auto* destination=catalog.find(passage->destination);
   if(!destination||destination->course!=course||!std::any_of(destination->candidates.begin(),destination->candidates.end(),[&](const auto& candidate){return candidate.native.kind==kind&&candidate.native.record_rom==passage->record;}))continue;
   dynamic[course]=Impl::DynamicPot{kind,source.native_payload(*passage)};
  }
  reset();p->plan=std::move(plan);p->dynamic_pots=std::move(dynamic);p->env=e;p->configured=true;return true;
 }catch(...){reset();return false;}
}
bool Policy::enter_room(unsigned ordinal,uint64_t epoch,const Environment& e){
 if(!p->same(e)||!epoch||epoch<=p->room_epoch)return false;auto found=std::find_if(p->plan.rooms.begin(),p->plan.rooms.end(),[&](const auto& r){return r.ordinal==ordinal;});
 if(found==p->plan.rooms.end()||found->key.course!=e.native_course||found->role!=bonus_routes0351::StepRole::Ordinary)return false;
 p->room=&*found;p->room_epoch=epoch;p->ready=false;p->witnessed.clear();return true;
}
bool Policy::room_ready(uint8_t* rdram,uint32_t info,const Environment& e){if(!p->context(e)||!rdram||!range(info,0x128)||MEM_BU(1,int32_t(info))!=p->room->key.room)return false;p->ready=true;return true;}
void Policy::created(recomp_context* c,unsigned room,const Environment& e){
 if(!p->same(e)||!c)return;const auto actor=uint32_t(c->r4);std::erase_if(p->witnessed,[&](const auto& w){return w.action.actor==actor;});if(p->serial==std::numeric_limits<uint64_t>::max())return;const auto birth=++p->serial;
 if(!p->context(e)||room!=p->room->key.room||!range(actor,0x1FC))return;
 for(const auto& exit:p->room->unselected_native_exits)if(exit.source.packed==uint32_t(c->r5)&&exit.source.point[0]==uint32_t(c->r6)&&exit.source.point[1]==uint32_t(c->r7)){
  // Two original identities with the same exact constructor key would be
  // ambiguous. Do not allow either to receive retirement authority.
  const auto duplicates=std::count_if(p->room->unselected_native_exits.begin(),p->room->unselected_native_exits.end(),[&](const auto& other){return other.source.packed==exit.source.packed&&other.source.point==exit.source.point;});
  if(duplicates!=1)return;p->witnessed.push_back({{p->room->ordinal,room,exit.source.index,actor,exit.source.packed,exit.source.point,birth,e.attempt_epoch,p->room_epoch},false});return;
 }
 if(e.native_course>=p->dynamic_pots.size()||!p->dynamic_pots[e.native_course])return;const auto& dynamic=*p->dynamic_pots[e.native_course];const auto packed=uint32_t(c->r5),id=packed&0x3FFu;
 if((packed&0xFFFFFC00u)!=(uint32_t(dynamic.kind)<<16|0x2000u)||id<0x200||id>=0x3FF||!navigation::coordinate(uint32_t(c->r6))||!navigation::coordinate(uint32_t(c->r7)))return;
 RetireAction action{p->room->ordinal,room,std::numeric_limits<unsigned>::max(),actor,packed,{uint32_t(c->r6),uint32_t(c->r7)},birth,e.attempt_epoch,p->room_epoch};action.reason=RetireReason::OriginalDynamicPot;action.expected_native_payload=dynamic.payload;p->witnessed.push_back({action,false});

}
void Policy::destroyed(uint32_t actor,const Environment& e){if(p->same(e))std::erase_if(p->witnessed,[&](const auto& w){return w.action.actor==actor;});}
bool Policy::validate(const RetireAction& a,uint8_t* rdram,const Environment& e)const{
 if(!p->context(e)||!p->ready)return false;const auto* witness=p->find(a);return witness&&!witness->queued&&identity(a,rdram)&&dynamic_initialized(a,rdram)&&detached_leaf(a,rdram)&&MEM_BU(0x17D,int32_t(a.actor))==4&&MEM_BU(4,int32_t(a.actor))!=1;
}
std::vector<RetireAction> Policy::pending(uint8_t* rdram,const Environment& e)const{std::vector<RetireAction> out;if(!p->context(e)||!p->ready)return out;for(const auto& w:p->witnessed)if(validate(w.action,rdram,e))out.push_back(w.action);return out;}
bool Policy::acknowledge(const RetireAction& a,uint8_t* rdram,const Environment& e){
 if(!p->context(e)||!p->ready)return false;const auto* w=p->find(a);if(!w||w->queued||!identity(a,rdram)||!dynamic_initialized(a,rdram)||!detached_leaf(a,rdram)||MEM_BU(0x17D,int32_t(a.actor))!=3||MEM_BU(0x28,int32_t(a.actor))!=3||MEM_BU(0x18,int32_t(a.actor))!=0xFF)return false;
 for(auto& witness:p->witnessed)if(witness.action==a){witness.queued=true;return true;}return false;
}
View Policy::view(uint8_t* rdram,const Environment& e)const{View out;if(!p->context(e))return out;out.configured=true;out.ready=p->ready;out.expected=unsigned(p->room->unselected_native_exits.size()+std::count_if(p->witnessed.begin(),p->witnessed.end(),[](const auto& witness){return witness.action.reason==RetireReason::OriginalDynamicPot;}));out.observed=unsigned(p->witnessed.size());for(const auto& w:p->witnessed){if(w.queued)++out.queued;else if(identity(w.action,rdram)&&MEM_HU(0x184,int32_t(w.action.actor)))++out.blocked_children;}return out;}
}
