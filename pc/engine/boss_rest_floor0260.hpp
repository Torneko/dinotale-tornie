#pragma once
#include "boss_floor_persistence0260.hpp"
#include "boss_rest_data0240.hpp"

namespace workshop::boss_rest_floor0260 {
namespace n=boss_native;namespace data=boss_rest_data0240;namespace floor=boss_floor_persistence0260;
inline constexpr unsigned overlay_section=floor::overlay_section;
// A real native wake changes its cached room's player source across normal
// returns too. Runtime must attest that wake's payload/pose/player/mask and the
// current visit mode/epoch. DefaultNone never authorizes changed coordinates.
enum class SourceMode:uint8_t {None,Normal,Retry};
struct SourceProof {uint32_t player=0;unsigned checkpoint=0,epoch=0,completed=0;SourceMode mode=SourceMode::None;};
using RetrySource=SourceProof;
inline bool checkpoint_player_record(uint8_t* ram,uint32_t source_packed,uint32_t x,uint32_t y,unsigned room,unsigned index,const SourceProof& proof){
 // The caller has already checked raw packed==source orFFFFFFFF. This type is
 // the immutable source record, so a generic consumed row grants no permission.
 // Native5628 reads the character source X160/Y448 and the native player
 // terrain probe16, then48C2C rewrites only4001 to X160/Y432 (5704..5740).
 const bool mode=proof.mode==SourceMode::Retry?(proof.epoch&&proof.completed==proof.checkpoint):
  proof.mode==SourceMode::Normal&&proof.checkpoint<=proof.completed;
 return ram&&mode&&room==83&&index<9&&index+1==proof.completed&&
  (proof.checkpoint==3||proof.checkpoint==6||proof.checkpoint==9)&&
  n::actor(ram,proof.player,0x4001)&&n::word(ram,0x800F8828)==proof.player&&source_packed==0x40010000&&
  x==std::bit_cast<uint32_t>(data::checkpoint_x)&&y==std::bit_cast<uint32_t>(data::checkpoint_y-16.f);
}
inline bool retry_player_record(uint8_t* ram,uint32_t source_packed,uint32_t x,uint32_t y,unsigned room,unsigned index,const RetrySource& proof){return checkpoint_player_record(ram,source_packed,x,y,room,index,proof);}
struct Binding {uint32_t actor=0,record=0,packed=0,resource=0;bool pinned=false;};
inline bool pointer(uint32_t p,size_t bytes){return floor::pointer(p,bytes);}
class Guard {
 std::array<Binding,data::object_count> slots_{};
 unsigned room_=177,index_=10;uint32_t table_=0;
 bool scope(unsigned room,unsigned index)const{return room==83&&room_==room&&index<10&&index_==index;}
 static bool table(uint8_t* ram,uint32_t at,bool proved,unsigned room,unsigned index,const std::optional<SourceProof>& source_proof){
  if(!pointer(at,data::object_count*16))return false;
  for(unsigned i=0;i<data::object_count;++i){const auto p=at+i*16,source=i*12,packed=n::word(ram,p);
   if(packed!=0xFFFFFFFF&&packed!=u32(data::rest_objects,source))return false;
   // Native checkpoint5628 may update only the player's source XY.
   if(proved&&u16(data::rest_objects,source)==0x4001)continue;
   const auto x=n::word(ram,p+4),y=n::word(ram,p+8);
   if(x==u32(data::rest_objects,source+4)&&y==u32(data::rest_objects,source+8))continue;
   if(u16(data::rest_objects,source)==0x4001&&source_proof&&checkpoint_player_record(ram,u32(data::rest_objects,source),x,y,room,index,*source_proof))continue;
   return false;
  }return true;
 }
 static bool resource(uint8_t* ram,uint32_t actor,uint32_t r,uint32_t overlay){
  return pointer(r,0x48)&&pointer(overlay,floor::overlay_extent)&&n::word(ram,actor+0x168)==r&&
   n::word(ram,r+0x40)==overlay&&n::word(ram,r+0x44)==overlay+floor::init_offset;
 }
 static bool shape(uint8_t* ram,uint32_t at){
  if(!pointer(at,50))return false;auto* rdram=ram;constexpr std::array<unsigned,5> head{5,2,32,0,20};
  for(unsigned i=0;i<head.size();++i)if(MEM_HU(i*2,n::base(at))!=head[i])return false;
  for(unsigned i=0;i<10;++i)if(MEM_HU(10+i*4,n::base(at))!=(((i%5)<<12)|((i/5)<<8)|i)||MEM_HU(12+i*4,n::base(at))!=(i<5?0x0601:0x0401))return false;
  return true;
 }
 static bool checkpoint_tiles(uint8_t* ram){
  uint32_t p=n::word(ram,0x800F8810);
  for(unsigned offset:{0x10u,4u,0x16Cu,0x1B0u,0x20u}){if(!pointer(p,offset+4))return false;p=n::word(ram,p+offset);}
  if(!pointer(p,0x28))return false;auto* rdram=ram;
  const unsigned columns=MEM_HU(0x12,n::base(p)),rows=MEM_HU(0x14,n::base(p)),stride=MEM_HU(0x16,n::base(p));
  const auto pages=n::word(ram,p+0x18),tiles=n::word(ram,p+0x24);
  if(columns!=3||rows!=2||!stride||stride>1024||stride%16||!n::range(pages,columns*rows)||!pointer(tiles,32*stride))return false;
  const unsigned tx=unsigned(data::checkpoint_x)/16,ty=data::floor_tile_row;
  auto tile=[&](unsigned y)->unsigned{const unsigned page=n::byte(ram,pages+(y/16)*columns+tx/16);if(page==255)return 0;
   const unsigned column=page*16+tx%16;if(column>=stride)return 0;const auto at=tiles+2*((y%16)*stride+column);if(!n::range(at,2))return 0;return MEM_HU(0,n::base(at));};
  return tile(ty)==0x0601&&tile(ty+1)==0x0401;
 }
public:
 void reset(unsigned room=177,unsigned index=10){slots_={};room_=room;index_=index;table_=0;}
 const auto& snapshot()const{return slots_;}
 bool owned(uint32_t actor)const{for(const auto& b:slots_)if(actor&&b.actor==actor)return true;return false;}
 void forget(uint32_t actor){for(auto& b:slots_)if(b.actor==actor)b={};}
 // Read-only witness for the exact existing168/472 source which stamps the
 // checkpoint160/448 support. A present tile alone cannot attest native D05C.
 bool checkpoint_support_ready(uint8_t* ram,unsigned room,unsigned index,uint32_t overlay)const{
  if(!ram||!scope(room,index))return false;auto* rdram=ram;
  for(unsigned slot=0;slot<slots_.size();++slot){const unsigned source=slot*12;
   if(u16(data::rest_objects,source)!=0x4592||u32(data::rest_objects,source+4)!=std::bit_cast<uint32_t>(data::checkpoint_x+8.f)||u32(data::rest_objects,source+8)!=std::bit_cast<uint32_t>(data::floor_actor_y))continue;
   const auto& b=slots_[slot];const auto a=b.actor;
   if(!b.pinned||!n::actor(ram,a,0x4592)||!resource(ram,a,b.resource,overlay)||n::byte(ram,a+0x17D)!=2||n::byte(ram,a+0x18)!=1||n::byte(ram,a+0x30)||
      n::word(ram,a+0x180)||n::word(ram,a+0x174)||n::word(ram,a+0x20)!=overlay+floor::idle_offset||n::word(ram,a+0x2C)!=overlay+floor::dtor_offset||
      MEM_HU(0xC0,n::base(a))!=32767||MEM_HU(0xC2,n::base(a))!=32767)return false;
   if(!pointer(b.record,16)||n::word(ram,a+0x1D8)!=b.record||n::word(ram,a+0x1DC)!=b.packed||n::word(ram,b.record)!=0xFFFFFFFF||
      n::word(ram,b.record+4)!=u32(data::rest_objects,source+4)||n::word(ram,b.record+8)!=u32(data::rest_objects,source+8)||
      n::word(ram,a+0x3C)!=u32(data::rest_objects,source+4)||n::word(ram,a+0x40)!=u32(data::rest_objects,source+8))return false;
   const auto priv=n::word(ram,a+0x1F4);return pointer(priv,12)&&n::word(ram,priv+8)==overlay+floor::table_offset&&shape(ram,overlay+floor::table_offset)&&checkpoint_tiles(ram);
  }
  return false;
 }
 bool capture(uint8_t* ram,const recomp_context* c,unsigned room,unsigned index,uint32_t overlay,std::optional<SourceProof> source_proof={}){
  if(!ram||!c||!scope(room,index)||uint32_t(c->r31)!=0x8005A050)return false;
  const auto actor=uint32_t(c->r4),record=uint32_t(c->r5);if(!n::actor(ram,actor,0x4592)||!pointer(record,16))return false;
  const auto packed=n::word(ram,record);if(packed>>16!=0x4592)return false;
  unsigned slot=data::object_count,found=0;
  // Rest retains original platforms plus an added floor row. Native ordinals
  // repeat, so ownership is the exact immutable source-record index.
  for(unsigned i=0;i<data::object_count;++i){const auto p=i*12;if(u32(data::rest_objects,p)==packed&&
     n::word(ram,record+4)==u32(data::rest_objects,p+4)&&n::word(ram,record+8)==u32(data::rest_objects,p+8)){slot=i;++found;}}
  if(found!=1||record<slot*16)return false;const auto base=record-slot*16;
  if((table_&&table_!=base)||!table(ram,base,table_!=0,room,index,source_proof))return false;
  const auto r=n::word(ram,actor+0x168);if(!resource(ram,actor,r,overlay))return false;
  auto& b=slots_[slot];if(b.actor&&n::actor(ram,b.actor,0x4592)&&n::byte(ram,b.actor+0x17D)==2&&n::byte(ram,b.actor+0x18)!=0xFF)return false;
  for(unsigned i=0;i<slots_.size();++i)if(i!=slot&&(slots_[i].actor==actor||slots_[i].record==record))return false;
  table_=base;b={actor,record,packed,r,false};return true;
 }
 bool pin(uint8_t* ram,const recomp_context* entry,unsigned room,unsigned index,uint32_t overlay){
  if(!ram||!entry||!scope(room,index))return false;auto* rdram=ram;const auto actor=uint32_t(entry->r4);
  for(unsigned slot=0;slot<slots_.size();++slot){auto& b=slots_[slot];if(!actor||b.actor!=actor)continue;const auto source=slot*12;
   if(u16(data::rest_objects,source)!=0x4592||!n::actor(ram,actor,0x4592)||!resource(ram,actor,b.resource,overlay)||
      n::byte(ram,actor+0x17D)!=2||n::byte(ram,actor+0x18)!=1||n::byte(ram,actor+0x30)||n::word(ram,actor+0x180)||n::word(ram,actor+0x174)||
      n::word(ram,actor+0x20)!=overlay+floor::idle_offset||n::word(ram,actor+0x2C)!=overlay+floor::dtor_offset)return false;
   if(!pointer(b.record,16)||n::word(ram,actor+0x1D8)!=b.record||n::word(ram,actor+0x1DC)!=b.packed||n::word(ram,b.record)!=0xFFFFFFFF||
      n::word(ram,b.record+4)!=u32(data::rest_objects,source+4)||n::word(ram,b.record+8)!=u32(data::rest_objects,source+8)||
      n::word(ram,actor+0x3C)!=u32(data::rest_objects,source+4)||n::word(ram,actor+0x40)!=u32(data::rest_objects,source+8))return false;
   const auto priv=n::word(ram,actor+0x1F4);if(!pointer(priv,12)||n::word(ram,priv+8)!=overlay+floor::table_offset||!shape(ram,overlay+floor::table_offset))return false;
   if(b.pinned)return MEM_HU(0xC0,n::base(actor))==32767&&MEM_HU(0xC2,n::base(actor))==32767;
   MEM_H(0xC0,n::base(actor))=32767;MEM_H(0xC2,n::base(actor))=32767;b.pinned=true;return true;
  }
  return false;
 }
};
} // namespace workshop::boss_rest_floor0260
