#pragma once
#include "custom_editor_objects.hpp"
#include <array>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <vector>
namespace workshop::story_hard_eggs0330 {
struct Placement {unsigned level=24,room=177;uint32_t packed=0,x=0,y=0;unsigned extra_slot=255;};
struct Carried {unsigned level=24,color=18;uint32_t actor=0,player=0;unsigned extra_slot=255;};
// Registry for the additional Hard bubbles only. It never reads or writes a
// canonical finds, metallic, collection or progression journal.
class Registry {
 using Key=std::array<uint32_t,4>;
 struct Source {unsigned level=24,room=177,color=18,extra_slot=255;};
 struct Egg {unsigned level=24,color=18;bool sounded=false,committed=false;unsigned extra_slot=255;};
 std::map<Key,Placement> placements_;
 std::map<uint32_t,Source> sources_,children_;
 std::map<uint32_t,Egg> eggs_;
 std::set<unsigned> released_;
 unsigned level_=24,room_=177;std::optional<unsigned> color_;std::array<std::optional<unsigned>,4> extra_colors_{};
 static unsigned release_key(unsigned level,unsigned slot){return slot<4?24+slot:level;}
 uint32_t creating_=0;Carried carried_{};
 static bool range(uint32_t p,unsigned bytes){return fruit_mix::range(p,bytes);}
 static gpr at(uint32_t p){return gpr(int32_t(p));}
 static uint32_t word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,at(p)));}
 static bool actor(uint8_t* rdram,uint32_t p,unsigned kind){return range(p,0x1FC)&&word(rdram,p)==0x19971110&&MEM_HU(12,at(p))==kind;}
public:
 void configure(std::span<const Placement> placements){
  reset();for(const auto& p:placements){
   require(p.level<24&&p.room<177&&((p.packed>>16)==0x4692||(p.packed>>16)==0x4693),"Invalid Hard egg placement");
   require(p.extra_slot==255||p.extra_slot<4,"Invalid Hard extra slot");
   require(placements_.emplace(Key{p.room,p.packed,p.x,p.y},p).second,"Conflicting Hard egg placement");
  }
 }
 void reset(){placements_.clear();sources_.clear();children_.clear();eggs_.clear();released_.clear();level_=24;room_=177;color_.reset();creating_=0;carried_={};}
 void enter(unsigned level,unsigned room,std::optional<unsigned> color,std::array<std::optional<unsigned>,4> extras={}){
  if(level!=level_){carried_={};released_.clear();}
  extra_colors_=extras;level_=level<24?level:24;room_=room<177?room:177;color_=color&&*color<18?color:std::nullopt;creating_=0;
 }
 void attempt_reset(){carried_={};released_.erase(level_);for(unsigned slot=0;slot<4;++slot)released_.erase(24+slot);for(auto& [actor,egg]:eggs_)if(egg.level==level_)egg.committed=true;}
 void begin_child(const recomp_context* c){
  creating_=0;if(!c||uint32_t(c->r5)!=0x41CE)return;
  auto source=sources_.find(uint32_t(c->r4));
  if(source!=sources_.end()&&source->second.level==level_&&source->second.room==room_&&source->second.color<18)creating_=source->first;
 }
 void forget(uint32_t address){
  sources_.erase(address);children_.erase(address);eggs_.erase(address);
  if(creating_==address)creating_=0;if(carried_.actor==address||carried_.player==address)carried_={};
 }
 void created(const recomp_context* c){
  if(!c)return;const auto address=uint32_t(c->r4),kind=uint32_t(c->r5)>>16;
  forget(address);
  auto found=placements_.find(Key{room_,uint32_t(c->r5),uint32_t(c->r6),uint32_t(c->r7)});
  if(found!=placements_.end()&&found->second.level==level_)
   sources_.emplace(address,Source{level_,room_,found->second.extra_slot<4?extra_colors_[found->second.extra_slot].value_or(18):color_.value_or(18),found->second.extra_slot});
  if(kind==0x41CE&&creating_){
   auto source=sources_.find(creating_);if(source!=sources_.end())children_.emplace(address,source->second);creating_=0;
  }
 }
 std::optional<bool> hidden(uint32_t source)const{
  const auto found=sources_.find(source);if(found==sources_.end()||found->second.level!=level_||found->second.room!=room_)return {};
  return found->second.color>=18||released_.contains(release_key(level_,found->second.extra_slot));
 }
 bool owns_wrapper(uint32_t wrapper)const{return children_.contains(wrapper);}
 std::optional<unsigned> release_color(uint32_t wrapper)const{
  const auto found=children_.find(wrapper);
  if(found==children_.end()||found->second.level!=level_||found->second.room!=room_||found->second.color>=18||released_.contains(release_key(level_,found->second.extra_slot)))return {};
  return found->second.color;
 }
 std::optional<unsigned> release_slot(uint32_t wrapper)const{auto f=children_.find(wrapper);if(f==children_.end())return {};return f->second.extra_slot;}
 void spawned(uint32_t egg,unsigned color,unsigned slot=255){
  if(egg&&color<18&&level_<24){eggs_[egg]={level_,color,false,false,slot};released_.insert(release_key(level_,slot));}
 }
 void prepare_release(uint8_t* rdram,uint32_t wrapper)const{
  if(!release_color(wrapper)||!actor(rdram,wrapper,0x41CE))return;
  const auto item=word(rdram,wrapper+0x1F4);if(!range(item,2))return;
  const auto kind=MEM_HU(0,at(item));if(kind==0x401C||kind==0x401D)MEM_H(0,at(item))=0x401D;
 }
 // Suppress the native White-egg promotion for these exact tracked401D only.
 // Real carrying is proved later, after native80039F48 has attached the parent.
 bool egg_state(uint32_t egg,unsigned state){
  auto found=eggs_.find(egg);if(found==eggs_.end())return false;
  if(state!=4&&state!=5&&carried_.actor==egg)carried_={};return true;
 }
 bool pickup_sound(uint32_t egg,unsigned state){
  auto found=eggs_.find(egg);if(found==eggs_.end()||found->second.sounded||(state!=4&&state!=5))return false;
  found->second.sounded=true;return true;
 }
 void frame(uint8_t* rdram){
  carried_={};if(!rdram||level_>=24)return;
  const auto player=word(rdram,0x800F8828);if(!actor(rdram,player,0x4001))return;
  const auto data=word(rdram,player+0x1F4);if(!range(data,0x1300))return;
  for(const auto& [address,egg]:eggs_){
   if(egg.committed||egg.level!=level_||egg.color>=18||!actor(rdram,address,0x401D))continue;
   const auto state=MEM_BU(0x30,at(address));
   if((state==4||state==5)&&word(rdram,address+0x180)==player&&MEM_BU(0x17C,at(address))<=1){
    carried_={level_,egg.color,address,player,egg.extra_slot};break;
   }
  }
 }
 std::optional<Carried> carried()const{return carried_.color<18?std::optional<Carried>{carried_}:std::nullopt;}
 void committed(){if(auto egg=eggs_.find(carried_.actor);egg!=eggs_.end())egg->second.committed=true;carried_={};}
 // Closed41CE wrappers retain their native mystery artwork. Nothing is
 // borrowed from401D, so their original cleanup needs no descriptor repair.
 void restore(uint8_t*,uint32_t)const{}
 void dispose(uint8_t* rdram,uint32_t actor){restore(rdram,actor);forget(actor);}
 bool paint(uint8_t* rdram,uint32_t address)const{
  const auto egg=eggs_.find(address);
  if(egg==eggs_.end()||egg->second.level!=level_||egg->second.color>=18||!range(address,0x170))return false;
  const unsigned color=egg->second.color;
  const auto a=at(address);const auto resource=word(rdram,address+0x168),drawing=word(rdram,address+0x16C);
  if(MEM_W(0,a)!=0x19971110||MEM_HU(12,a)!=0x401D||MEM_BU(0x164,a)>1||!range(resource,0x90)||!range(drawing,0x44))return false;
  const auto egg_resource=editor_objects::native_egg_resource(rdram);if(!egg_resource)return false;
  const auto e=at(egg_resource),d=at(drawing);
  if(MEM_W(0x38,d)||MEM_W(0x3C,d))return false;
  MEM_W(8,d)=MEM_W(0x88,e)+color*512;MEM_B(4,d)=0;MEM_W(0x30,d)=MEM_W(0x84,e);MEM_W(0x40,d)=egg_resource+0x74;
  return true;
 }
};
} // namespace workshop::story_hard_eggs0330
