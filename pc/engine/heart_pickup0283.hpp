#pragma once
#include "recomp.h"
#include "heart_sources0283.hpp"
#include "vanilla_sources.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace workshop::heart_pickup0283 {
namespace sources=heart_sources0283;
struct Frame {uint32_t bubble_overlay=0,heart_overlay=0,hidden_overlay=0,player=0;};
struct Receipt {unsigned level=24,slot=3,room=177;uint32_t actor=0;uint64_t birth=0;};
inline bool range(uint32_t p,size_t n=4){return !(p&3)&&p>=0x80000400u&&p<0x80800000u&&n<=0x80800000u-p;}
inline gpr base(uint32_t p){return gpr(int32_t(p));}
inline uint32_t word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,base(p)));}
inline uint16_t half(uint8_t* rdram,uint32_t p){return MEM_HU(0,base(p));}
inline unsigned byte(uint8_t* rdram,uint32_t p){return MEM_BU(0,base(p));}
inline uint16_t kind(uint8_t* rdram,uint32_t p){return MEM_HU(12,base(p));}
inline bool actor(uint8_t* rdram,uint32_t p){return rdram&&range(p,0x1FC)&&word(rdram,p)==0x19971110u;}
// A read-only witness. It never cancels a native callback and never writes RAM,
// changes color/flags, calls the wallet, or supplies health/eggs.
class Observer {
 struct Binding {const sources::Source* source=nullptr;uint64_t birth=0;uint16_t kind=0;bool collected=false;unsigned contact=0;};
 struct Origin {const sources::Source* source=nullptr;uint32_t actor=0,sp=0;uint64_t birth=0,until=0;};
 struct Spawn {Origin origin{};uint32_t sp=0,heart_overlay=0;std::array<uint32_t,3> position{};uint64_t after_birth=0,until=0;};
 struct Collection {Binding binding{};uint32_t actor=0,sp=0,overlay=0,player=0,color=0,hp=0,counter=0;uint64_t until=0;unsigned phase=0;bool retired=false;};
 unsigned level_=24,room_=177;uint32_t table_=0;bool allowed_=false;
 uint64_t serial_=0,event_=0;
 std::unordered_map<uint32_t,uint64_t> births_;
 std::unordered_map<uint32_t,Binding> bindings_;
 Origin release_{},hidden_{};Spawn spawn_{};Collection collection_{};
 std::vector<Receipt> receipts_;
 bool same_birth(uint32_t p,uint64_t b)const{const auto i=births_.find(p);return b&&i!=births_.end()&&i->second==b;}
 uint64_t birth(uint32_t p)const{const auto i=births_.find(p);return i==births_.end()?0:i->second;}
 bool scope(uint8_t* r)const{return allowed_&&r&&word(r,0x800F8B88)==1&&byte(r,0x800F8DFC)*4+byte(r,0x800F8DFD)==level_;}
 bool valid(uint8_t* r,uint32_t p,const Binding& b)const{return b.source&&same_birth(p,b.birth)&&actor(r,p)&&kind(r,p)==b.kind&&b.source->level==level_&&b.source->room==room_;}
 bool heart_table(uint8_t* r,const Frame& f)const{return range(f.heart_overlay,0xF10)&&word(r,f.heart_overlay+0xEC8)==f.heart_overlay+0x4E8;}
 const sources::Source* placed(uint8_t* r,uint32_t p)const{
  if(!actor(r,p)||!table_)return nullptr;
  const auto record=word(r,p+0x1D8);if(record<table_||(record-table_)%16)return nullptr;
  const auto* s=sources::find(level_,room_,(record-table_)/16);
  if(!s||!range(record,16)||word(r,p+0x1DC)!=s->packed||kind(r,p)!=s->kind)return nullptr;
  const auto packed=word(r,record);
  return (packed==s->packed||packed==0xFFFFFFFFu)&&word(r,record+4)==s->x&&word(r,record+8)==s->y?s:nullptr;
 }
 bool origin(uint8_t* r,const Origin& o)const{return o.source&&event_<=o.until&&same_birth(o.actor,o.birth)&&actor(r,o.actor)&&o.source->level==level_&&o.source->room==room_;}
 bool retired(uint8_t* r)const{return actor(r,collection_.actor)&&byte(r,collection_.actor+0x18)==0xFF&&byte(r,collection_.actor+0x28)==3&&byte(r,collection_.actor+0x17D)==3;}
 void clear_pending(){release_={};hidden_={};spawn_={};collection_={};}
public:
 // Call once at native80045CAC entry for EVERY actor address reuse, before its
 // initialization. Forget on complete cleanup439F4/45B64, never447C8 retirement.
 void created(uint32_t p){if(!range(p,0x1FC))return;forget(p);births_[p]=++serial_;}
 void forget(uint32_t p){bindings_.erase(p);births_.erase(p);if(collection_.actor==p)collection_={};if(release_.actor==p)release_={};if(hidden_.actor==p)hidden_={};if(spawn_.origin.actor==p)spawn_={};}
 // level is zero-based Trial index. Root supplies allowed only for vanilla
 // Trial (ordinary/hybrid), excluding Story, Practice, editor and Boss Battle.
 // Existing bindings keep their fixed source across ordinary room revisits.
 bool enter(uint8_t* r,unsigned level,unsigned room,uint32_t metadata,bool allowed){
  if(level!=level_)bindings_.clear();level_=level;room_=room;table_=0;allowed_=false;clear_pending();
  if(!allowed||level>=24||!r||!range(metadata,8))return false;
  const unsigned expected=sources::count(room),n=half(r,metadata);
  if(!expected||!workshop::vanilla_room(workshop::vanilla_sources[level],room)||(n!=expected&&!(room==14&&n==expected+8)))return false;
  const auto table=word(r,metadata+4);if(!range(table,size_t(n)*16))return false;
  bool belongs=false;
  for(const auto& s:sources::sources)if(s.room==room){if(s.level!=level)return false;belongs=true;
   const auto record=table+s.index*16,packed=word(r,record);
   if((packed!=s.packed&&packed!=0xFFFFFFFFu)||word(r,record+4)!=s.x||word(r,record+8)!=s.y)return false;
  }
  // Some native rooms contain no hearts. Their table is valid, but cannot
  // resolve a receipt until a room with one of the72 sources is entered.
  if(!belongs)for(auto root:sources::sources)if(root.level==level){belongs=true;break;}
  if(!belongs)return false;table_=table;allowed_=true;if(!scope(r)){allowed_=false;table_=0;return false;}return true;
 }
 void trace(uint8_t* r,const recomp_context* c,std::string_view name,const Frame& f){
  if(!r||!c)return;++event_;
  if(!scope(r)){clear_pending();return;}
  if(collection_.binding.source&&event_>collection_.until)collection_={};
  if(spawn_.origin.source&&event_>spawn_.until)spawn_={};
  const uint32_t a=uint32_t(c->r4),sp=uint32_t(c->r29),ra=uint32_t(c->r31);
  if(name=="func_8059EA78"){
   if(!actor(r,a)||kind(r,a)!=0x41CE||byte(r,a+0x30)!=7||!birth(a)||!range(f.bubble_overlay,0x850)||half(r,f.bubble_overlay+0x816)!=0x40C5)return;
   const auto parent=word(r,a+0x180);const auto* s=placed(r,parent);
   if(!s||!sources::bubble(s->kind)||!birth(parent))return;
   const auto index=half(r,a+0x17E);
   if(index>=20||word(r,parent+0x188+index*4)!=a)return;
   bindings_[a]={s,birth(a),0x41CE,false};return;
  }
  if(name=="func_8059EC8C"){
   release_={};auto i=bindings_.find(a);
   if(i==bindings_.end()||!valid(r,a,i->second)||kind(r,a)!=0x41CE||word(r,a+0x180)!=0)return;
   const auto data=word(r,a+0x1F4);if(!range(data,4)||half(r,data)!=0x40C5)return;
   release_={i->second.source,a,sp,birth(a),event_+256};return;
  }
  if(name=="func_806F55E8"){
   hidden_={};const auto* s=placed(r,a);if(s&&s->kind==0x464C&&birth(a))hidden_={s,a,sp,birth(a),event_+1024};return;
  }
  if(name=="func_806E7E70"){
   spawn_={};if(!heart_table(r,f)||word(r,0x800F8D38)!=f.heart_overlay+0xC30)return;
   Origin o{};
   if(range(f.bubble_overlay,0x850)&&ra==f.bubble_overlay+0x340&&origin(r,release_)&&a==release_.actor+0x3C&&sp==release_.sp-0x40)o=release_;
   else if(range(f.hidden_overlay,0x1200)&&ra==f.hidden_overlay+0x530&&origin(r,hidden_)&&a==hidden_.actor+0x84&&sp==hidden_.sp-0x70)o=hidden_;
   if(!o.source||!range(a,12))return;
   spawn_={o,sp,f.heart_overlay,{word(r,a),word(r,a+4),word(r,a+8)},serial_,event_+256};return;
  }
  if(name=="func_806E7468"){
   if(!heart_table(r,f)||!actor(r,a)||!sources::direct(kind(r,a))||!birth(a))return;
   if(const auto* s=placed(r,a);s&&sources::direct(s->kind)){bindings_[a]={s,birth(a),s->kind,false};return;}
   if(!spawn_.origin.source||!origin(r,spawn_.origin)||kind(r,a)!=0x40C5||f.heart_overlay!=spawn_.heart_overlay||ra!=f.heart_overlay+0xC88||sp!=spawn_.sp-0x20||birth(a)<=spawn_.after_birth||word(r,a+0x1D8)!=0)return;
   for(unsigned n=0;n<3;++n)if(word(r,a+0x3C+n*4)!=spawn_.position[n])return;
   bindings_[a]={spawn_.origin.source,birth(a),0x40C5,false};spawn_={};return;
  }
  if(name=="func_806E7574"){
   const auto i=bindings_.find(a);if(i==bindings_.end()||!valid(r,a,i->second)||!heart_table(r,f)||byte(r,a+0x17D)!=2)return;
   i->second.contact=0;const auto other=uint32_t(c->r5);
   if(actor(r,other)&&((other==f.player&&other==word(r,0x800F8828)&&kind(r,other)==0x4001)||(kind(r,other)==0x4002&&byte(r,other+0x18)==0x68)))i->second.contact=1;
   return;
  }
  if(name=="func_806E7E4C"&&uint32_t(c->r5)==3&&ra==f.heart_overlay+0x388){
   const auto i=bindings_.find(a);if(i!=bindings_.end()&&valid(r,a,i->second)&&i->second.contact==1)i->second.contact=2;return;
  }
  if(name=="func_806E7728"){
   collection_={};const auto i=bindings_.find(a);
   if(i==bindings_.end()||i->second.collected||i->second.contact!=2||byte(r,a+0x17D)!=2||!valid(r,a,i->second)||!heart_table(r,f)||byte(r,a+0x18)!=3||word(r,a+0x20)!=f.heart_overlay+0x4E8)return;
   const auto player=word(r,0x800F8828),color=word(r,0x800F8DB0);
   if(player!=f.player||!actor(r,player)||kind(r,player)!=0x4001||byte(r,player+0x17D)!=2||byte(r,player+0x18)==0xA||byte(r,player+0x30)==0x11||color>=8)return;
   const auto hp=word(r,0x800F8DB4+color*4),counter=word(r,0x800F8958);
   if(hp>8||counter>=3)return;
   collection_={i->second,a,sp,f.heart_overlay,player,color,hp,counter,event_+1024,0,false};return;
  }
  if(!collection_.binding.source)return;
  auto& q=collection_;
  if(!same_birth(q.actor,q.binding.birth)||f.heart_overlay!=q.overlay||f.player!=q.player||word(r,0x800F8828)!=q.player){collection_={};return;}
  if(name=="func_800447C8"&&a==q.actor&&ra==q.overlay+0x560&&sp==q.sp-0x30){q.phase=1;return;}
  if(name=="func_80044658"&&a==q.actor&&q.phase==1){q.retired=true;return;}
  if(name=="func_80049624"){
   if(q.phase!=1||!q.retired||ra!=q.overlay+0x578||sp!=q.sp-0x30||!retired(r)||word(r,0x800F8958)!=q.counter+1){collection_={};return;}q.phase=2;return;
  }
  if(name=="func_8002B428"){
   if(q.phase!=2||a!=8||ra!=q.overlay+0x580||sp!=q.sp-0x30||word(r,0x800F8DB0)!=q.color){collection_={};return;}q.phase=3;return;
  }
  if(name=="func_8002B3A0"){
   if(q.phase==3&&ra==0x8002B460&&sp==q.sp-0x48&&retired(r)&&word(r,0x800F8958)==q.counter+1&&word(r,0x800F8DB0)==q.color&&word(r,0x800F8DB4+q.color*4)==8&&actor(r,q.player)&&kind(r,q.player)==0x4001&&receipts_.size()<72){
    auto i=bindings_.find(q.actor);if(i!=bindings_.end()&&i->second.birth==q.binding.birth){i->second.collected=true;receipts_.push_back({q.binding.source->level,q.binding.source->slot,q.binding.source->room,q.actor,q.binding.birth});}
   }
   collection_={};
  }
 }
 std::vector<Receipt> drain(){auto result=std::move(receipts_);receipts_.clear();return result;}
};
inline Observer observer{};
inline bool enter(uint8_t* r,unsigned level,unsigned room,uint32_t metadata,bool allowed){return observer.enter(r,level,room,metadata,allowed);}
inline void created(uint32_t actor){observer.created(actor);}
inline void forget(uint32_t actor){observer.forget(actor);}
inline void trace(uint8_t* r,const recomp_context* c,std::string_view name,const Frame& f){observer.trace(r,c,name,f);}
inline std::vector<Receipt> drain(){return observer.drain();}
}
