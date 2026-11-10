#pragma once
#include "boss_rooms_data0250.hpp"
#include "boss_native.hpp"
#include <array>
#include <bit>

namespace workshop::boss_floor_persistence0260 {
namespace n=boss_native;namespace rooms=boss_rooms_data0250;
inline constexpr unsigned overlay_section=189;
inline constexpr uint32_t original_overlay=0x8069CCF0,overlay_extent=0x77C;
inline constexpr uint32_t stamp_offset=0x36C,idle_offset=0x2C0,init_offset=0x5A4,dtor_offset=0x43C,table_offset=0x714;
struct Binding {uint32_t actor=0,record=0,packed=0,resource=0;bool pinned=false;};
inline bool pointer(uint32_t p,size_t bytes){return !(p&3)&&n::range(p,bytes);}
class Guard {
 std::array<Binding,12> slots_{};unsigned room_=177,encounter_=10;uint32_t table_=0;
 bool scope(const rooms::Arena& a,unsigned room,unsigned encounter)const{
  return encounter<10&&room==room_&&encounter==encounter_&&room==rooms::source_for_encounter(encounter)&&a.source==room&&a.logical_source==162&&!a.objects.empty()&&a.objects.size()%12==0&&a.objects.size()/12<=149;
 }
 static bool source(const rooms::Arena& a,unsigned instance,size_t& offset){
  unsigned seen=0,mask=0;for(size_t p=0;p<a.objects.size();p+=12)if(u16(a.objects,p)==0x4592){const auto index=u16(a.objects,p+2);if(index>=12||(mask&(1u<<index))||u32(a.objects,p+4)!=std::bit_cast<uint32_t>(40.f+64.f*index)||u32(a.objects,p+8)!=std::bit_cast<uint32_t>(472.f))return false;mask|=1u<<index;++seen;if(index==instance)offset=p;}
  return seen==12&&mask==4095;
 }
 static bool native_table(uint8_t* ram,uint32_t table,const rooms::Arena& a,bool proved){
  if(!pointer(table,a.objects.size()/12*16))return false;
  for(size_t p=0;p<a.objects.size();p+=12){const auto at=table+uint32_t(p/12)*16,packed=n::word(ram,at);if(packed!=0xFFFFFFFF&&packed!=u32(a.objects,p))return false;
   // Native checkpoint wake48C2C moves only the original player record.
   if(proved&&u16(a.objects,p)==0x4001)continue;
   if(n::word(ram,at+4)!=u32(a.objects,p+4)||n::word(ram,at+8)!=u32(a.objects,p+8))return false;
  }return true;
 }
 static bool resource(uint8_t* ram,uint32_t actor,uint32_t r,uint32_t overlay){return pointer(r,0x48)&&pointer(overlay,overlay_extent)&&n::word(ram,actor+0x168)==r&&n::word(ram,r+0x40)==overlay&&n::word(ram,r+0x44)==overlay+init_offset;}
 static bool shape(uint8_t* ram,uint32_t table){
  if(!pointer(table,50))return false;auto* rdram=ram;constexpr std::array<unsigned,5> head{5,2,32,0,20};
  for(unsigned i=0;i<head.size();++i)if(MEM_HU(i*2,n::base(table))!=head[i])return false;
  for(unsigned i=0;i<10;++i)if(MEM_HU(10+i*4,n::base(table))!=(((i%5)<<12)|((i/5)<<8)|i)||MEM_HU(12+i*4,n::base(table))!=(i<5?0x0601:0x0401))return false;return true;
 }
public:
 void reset(unsigned room=177,unsigned encounter=10){slots_={};room_=room;encounter_=encounter;table_=0;}
 const auto& snapshot()const{return slots_;}
 bool owned(uint32_t actor)const{for(const auto& b:slots_)if(b.actor==actor&&actor)return true;return false;}
 void forget(uint32_t actor){for(auto& b:slots_)if(b.actor==actor)b={};}
 bool capture(uint8_t* ram,const recomp_context* c,const rooms::Arena& a,unsigned room,unsigned encounter,uint32_t overlay){
  if(!ram||!c||!scope(a,room,encounter)||uint32_t(c->r31)!=0x8005A050)return false;
  const auto actor=uint32_t(c->r4),record=uint32_t(c->r5);if(!n::actor(ram,actor,0x4592)||!pointer(record,16))return false;
  const auto packed=n::word(ram,record),instance=packed&65535;if(packed>>16!=0x4592||instance>=12)return false;size_t offset=0;if(!source(a,instance,offset))return false;
  if(packed!=u32(a.objects,offset)||n::word(ram,record+4)!=u32(a.objects,offset+4)||n::word(ram,record+8)!=u32(a.objects,offset+8))return false;
  const auto native_offset=uint32_t(offset/12)*16;if(record<native_offset)return false;const auto table=record-native_offset;
  if((table_&&table_!=table)||!native_table(ram,table,a,table_!=0))return false;
  const auto r=n::word(ram,actor+0x168);if(!resource(ram,actor,r,overlay))return false;
  auto& b=slots_[instance];if(b.actor&&n::actor(ram,b.actor,0x4592)&&n::byte(ram,b.actor+0x17D)==2&&n::byte(ram,b.actor+0x18)!=0xFF)return false;
  for(unsigned i=0;i<12;++i)if(i!=instance&&(slots_[i].actor==actor||slots_[i].record==record))return false;
  table_=table;b={actor,record,packed,r,false};return true;
 }
 bool pin(uint8_t* ram,const recomp_context* entry,const rooms::Arena& a,unsigned room,unsigned encounter,uint32_t overlay){
  if(!ram||!entry||!scope(a,room,encounter))return false;auto* rdram=ram;const auto actor=uint32_t(entry->r4);
  for(auto& b:slots_){if(!actor||b.actor!=actor)continue;const auto instance=b.packed&65535;size_t offset=0;if(!source(a,instance,offset)||!n::actor(ram,actor,0x4592)||!resource(ram,actor,b.resource,overlay)||n::byte(ram,actor+0x17D)!=2||n::byte(ram,actor+0x18)!=1||n::byte(ram,actor+0x30)||n::word(ram,actor+0x180)||n::word(ram,actor+0x174)||n::word(ram,actor+0x20)!=overlay+idle_offset||n::word(ram,actor+0x2C)!=overlay+dtor_offset)return false;
   if(!pointer(b.record,16)||n::word(ram,actor+0x1D8)!=b.record||n::word(ram,actor+0x1DC)!=b.packed||n::word(ram,b.record)!=0xFFFFFFFF||n::word(ram,b.record+4)!=u32(a.objects,offset+4)||n::word(ram,b.record+8)!=u32(a.objects,offset+8)||n::word(ram,actor+0x3C)!=u32(a.objects,offset+4)||n::word(ram,actor+0x40)!=u32(a.objects,offset+8))return false;
   const auto priv=n::word(ram,actor+0x1F4);if(!pointer(priv,12)||n::word(ram,priv+8)!=overlay+table_offset||!shape(ram,overlay+table_offset))return false;
   if(b.pinned)return MEM_HU(0xC0,n::base(actor))==32767&&MEM_HU(0xC2,n::base(actor))==32767;
   MEM_H(0xC0,n::base(actor))=32767;MEM_H(0xC2,n::base(actor))=32767;b.pinned=true;return true;
  }
  return false;
 }
};
} // namespace workshop::boss_floor_persistence0260
