#pragma once
#include "custom_editor_objects.hpp"
#include <array>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <vector>
namespace workshop::story_hard_eggs0330 {
struct Placement {unsigned level=24,room=177;uint32_t packed=0,x=0,y=0;};
struct Carried {unsigned level=24,color=18;uint32_t actor=0,player=0;};
// Registry for the additional Hard bubbles only. It never reads or writes a
// canonical finds, metallic, collection or progression journal.
class Registry {
 using Key=std::array<uint32_t,4>;
 struct Source {unsigned level=24,room=177,color=18;};
 struct Egg {unsigned level=24,color=18;bool sounded=false,committed=false;};
 std::map<Key,Placement> placements_;
 std::map<uint32_t,Source> sources_,children_;
 std::map<uint32_t,Egg> eggs_;
 std::set<unsigned> released_;
 unsigned level_=24,room_=177;std::optional<unsigned> color_;
 uint32_t creating_=0;Carried carried_{};
 static bool range(uint32_t p,unsigned bytes){return fruit_mix::range(p,bytes);}
 static gpr at(uint32_t p){return gpr(int32_t(p));}
 static uint32_t word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,at(p)));}
 static bool actor(uint8_t* rdram,uint32_t p,unsigned kind){return range(p,0x1FC)&&word(rdram,p)==0x19971110&&MEM_HU(12,at(p))==kind;}
public:
 void configure(std::span<const Placement> placements){
  reset();for(const auto& p:placements){
   require(p.level<24&&p.room<177&&(p.packed>>16)==0x4692,"Invalid Hard egg placement");
   require(placements_.emplace(Key{p.room,p.packed,p.x,p.y},p).second,"Conflicting Hard egg placement");
  }
 }
 void reset(){placements_.clear();sources_.clear();children_.clear();eggs_.clear();released_.clear();level_=24;room_=177;color_.reset();creating_=0;carried_={};}
 void enter(unsigned level,unsigned room,std::optional<unsigned> color){
  if(level!=level_){carried_={};released_.clear();}
  level_=level<24?level:24;room_=room<177?room:177;color_=color&&*color<18?color:std::nullopt;creating_=0;
 }
 void attempt_reset(){carried_={};released_.erase(level_);for(auto& [actor,egg]:eggs_)if(egg.level==level_)egg.committed=true;}
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
   sources_.emplace(address,Source{level_,room_,color_.value_or(18)});
  if(kind==0x41CE&&creating_){
   auto source=sources_.find(creating_);if(source!=sources_.end())children_.emplace(address,source->second);creating_=0;
  }
 }
 std::optional<bool> hidden(uint32_t source)const{
  const auto found=sources_.find(source);if(found==sources_.end()||found->second.level!=level_||found->second.room!=room_)return {};
  return found->second.color>=18||released_.contains(level_);
 }
 bool owns_wrapper(uint32_t wrapper)const{return children_.contains(wrapper);}
 std::optional<unsigned> release_color(uint32_t wrapper)const{
  const auto found=children_.find(wrapper);
  if(found==children_.end()||found->second.level!=level_||found->second.room!=room_||found->second.color>=18||released_.contains(level_))return {};
  return found->second.color;
 }
 void spawned(uint32_t egg,unsigned color){
  if(egg&&color<18&&level_<24){eggs_[egg]={level_,color,false};released_.insert(level_);}
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
    carried_={level_,egg.color,address,player};break;
   }
  }
 }
 std::optional<Carried> carried()const{return carried_.color<18?std::optional<Carried>{carried_}:std::nullopt;}
 void committed(){if(auto egg=eggs_.find(carried_.actor);egg!=eggs_.end())egg->second.committed=true;carried_={};}
 // Borrow the already loaded sixteen-color401D sprite. Restore it before the
 // native cleanup compares texture descriptors, exactly like editor bubbles.
 void restore(uint8_t* rdram,uint32_t address)const{
  if(!children_.contains(address)||!range(address,0x170))return;
  const auto resource=word(rdram,address+0x168),drawing=word(rdram,address+0x16C);
  if(!range(resource,0x90)||!range(drawing,0x44))return;
  const auto s=at(resource),d=at(drawing);
  if(MEM_HU(0x3C,s)!=0x41CE||MEM_W(0x30,s)||MEM_W(0x38,d)||MEM_W(0x3C,d))return;
  MEM_W(8,d)=MEM_W(0x88,s);MEM_B(4,d)=0;MEM_W(0x30,d)=MEM_W(0x84,s);MEM_W(0x40,d)=resource+0x74;
 }
 void dispose(uint8_t* rdram,uint32_t actor){restore(rdram,actor);forget(actor);}
 bool paint(uint8_t* rdram,uint32_t address)const{
  std::optional<unsigned> color;
  if(auto child=children_.find(address);child!=children_.end()&&child->second.level==level_&&child->second.room==room_)color=child->second.color;
  else if(auto egg=eggs_.find(address);egg!=eggs_.end()&&egg->second.level==level_)color=egg->second.color;
  if(!color||*color>=18||!range(address,0x170))return false;
  const auto a=at(address);const auto resource=word(rdram,address+0x168),drawing=word(rdram,address+0x16C);
  if(MEM_W(0,a)!=0x19971110||MEM_BU(0x164,a)>1||!range(resource,0x90)||!range(drawing,0x44))return false;
  const auto egg_resource=editor_objects::native_egg_resource(rdram);if(!egg_resource)return false;
  const auto e=at(egg_resource),d=at(drawing);
  if(MEM_W(0x38,d)||MEM_W(0x3C,d))return false;
  MEM_W(8,d)=MEM_W(0x88,e)+*color*512;MEM_B(4,d)=0;MEM_W(0x30,d)=MEM_W(0x84,e);MEM_W(0x40,d)=egg_resource+0x74;
  return eggs_.contains(address);
 }
};
} // namespace workshop::story_hard_eggs0330
