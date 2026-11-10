#pragma once
#include "boss_native.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstdio>
#include <string_view>

namespace workshop::boss_checkpoint_probe0260 {
namespace n=boss_native;
// Temporary, bounded observations of the native numbered point and its floor.
// capacity is the actual recomp memory allocation, including the PC catalogue.
inline bool range(uint32_t p,size_t bytes,size_t capacity){
 return p>=0x80000400u&&uint64_t(p)-0x80000000u<=capacity&&bytes<=capacity-(uint64_t(p)-0x80000000u);
}
inline bool pointer(uint32_t p,size_t bytes,size_t capacity){return !(p&3)&&range(p,bytes,capacity);}
inline uint32_t word(uint8_t* ram,uint32_t p){return n::word(ram,p);}
inline float scalar(uint8_t* ram,uint32_t p){return std::bit_cast<float>(word(ram,p));}
struct Observation {
 uint32_t actor=0,record=0,packed=0,resource=0,overlay=0,metadata=0,definition=0,private_data=0,parent=0,callback=0,movement=0;
 unsigned kind=0,life=0,queue=0,primary=0,sub=0,def74=0,def75=0;
 std::array<float,3> local{},world{},velocity{};
 unsigned contacts=0,previous_contacts=0,probe_count=0,terrain_flags=0;
 uint32_t xs=0,ys=0,query=0,terrain=0,previous_terrain=0;
 bool probe_known=false,catalogue_known=false;
 float probe_x=0,probe_y=0,correction_y=0;
 unsigned query_tile=0,query_contact=0;
 uint32_t row=0;std::array<uint32_t,6> catalogue{};
 uint32_t map=0;unsigned columns=0,rows=0,stride=0;
 std::array<unsigned,4> tiles{0x10000,0x10000,0x10000,0x10000};
};
inline std::optional<Observation> observe(uint8_t* ram,uint32_t a,size_t capacity){
 if(!ram||!pointer(a,0x1FC,capacity)||word(ram,a)!=0x19971110)return {};
 auto* rdram=ram;const unsigned kind=MEM_HU(12,n::base(a));if(kind<0x4508||kind>0x450A)return {};
 Observation o;o.actor=a;o.kind=kind;o.record=word(ram,a+0x1D8);o.packed=word(ram,a+0x1DC);o.resource=word(ram,a+0x168);
 o.metadata=word(ram,a+0x174);o.private_data=word(ram,a+0x1F4);o.parent=word(ram,a+0x180);o.callback=word(ram,a+0x20);o.movement=word(ram,a+0x130);
 o.life=n::byte(ram,a+6);o.queue=n::byte(ram,a+0x17D);o.primary=n::byte(ram,a+0x18);o.sub=n::byte(ram,a+0x30);
 for(unsigned i=0;i<3;++i){o.local[i]=scalar(ram,a+0x3C+i*4);o.world[i]=scalar(ram,a+0x84+i*4);o.velocity[i]=scalar(ram,a+0xC4+i*4);}
 if(pointer(o.resource,0x48,capacity))o.overlay=word(ram,o.resource+0x40);
 if(pointer(o.metadata,0x10,capacity)){o.definition=word(ram,o.metadata);o.contacts=word(ram,o.metadata+8);o.previous_contacts=word(ram,o.metadata+12);}
 if(pointer(o.definition,0x7C,capacity)){
  const auto d=o.definition;o.probe_count=word(ram,d+0x2C);o.xs=word(ram,d+0x30);o.ys=word(ram,d+0x34);o.query=word(ram,d+0x40);
  o.terrain=word(ram,d+0x44);o.previous_terrain=word(ram,d+0x48);o.terrain_flags=MEM_HU(0x4C,n::base(d));o.def74=n::byte(ram,d+0x74);o.def75=n::byte(ram,d+0x75);
  if(o.probe_count&&pointer(o.xs,4,capacity)&&pointer(o.ys,4,capacity)){o.probe_x=scalar(ram,o.xs);o.probe_y=scalar(ram,o.ys);o.probe_known=std::isfinite(o.probe_x)&&std::isfinite(o.probe_y)&&std::abs(o.probe_x)<=128&&std::abs(o.probe_y)<=128;}
  if(pointer(o.query,24,capacity)){o.query_tile=MEM_HU(0,n::base(o.query));o.query_contact=n::byte(ram,o.query+0x14);o.correction_y=scalar(ram,o.query+8);}
 }
 // Native5BDA8/5BDF8 read WORD counts and pointers from this per-kind row.
 o.row=0x80810000+(kind-0x4001)*24;
 if(pointer(o.row,24,capacity)){o.catalogue_known=true;for(unsigned i=0;i<6;++i)o.catalogue[i]=word(ram,o.row+i*4);}
 uint32_t p=range(0x800F8810,4,capacity)?word(ram,0x800F8810):0;
 for(unsigned offset:{0x10u,4u,0x16Cu,0x1B0u,0x20u}){if(!pointer(p,offset+4,capacity)){p=0;break;}p=word(ram,p+offset);}
 if(pointer(p,0x28,capacity)){
  o.map=p;o.columns=MEM_HU(0x12,n::base(p));o.rows=MEM_HU(0x14,n::base(p));o.stride=MEM_HU(0x16,n::base(p));const auto pages=word(ram,p+0x18),data=word(ram,p+0x24);
  if(o.columns&&o.columns<=8&&o.rows&&o.rows<=8&&o.stride&&o.stride<=1024&&o.stride%16==0&&range(pages,o.columns*o.rows,capacity)){
   auto tile=[&](float x,float y)->unsigned{
    if(!std::isfinite(x)||!std::isfinite(y)||x<0||y<0||x>=o.columns*256.f||y>=o.rows*256.f)return 0x10000;
    const unsigned tx=unsigned(x)/16,ty=unsigned(y)/16,page=n::byte(ram,pages+(ty/16)*o.columns+tx/16);if(page==255)return 0;
    const unsigned column=page*16+tx%16;if(column>=o.stride)return 0x10000;const uint64_t wide=uint64_t(data)+2*((ty%16)*o.stride+column);
    if(wide>UINT32_MAX||!range(data,2,capacity))return 0x10000;const auto at=uint32_t(wide);
    return range(at,2,capacity)?MEM_HU(0,n::base(at)):0x10000;
   };
   o.tiles[0]=tile(160,464);o.tiles[1]=tile(160,480);
   if(o.probe_known){o.tiles[2]=tile(o.local[0]+o.probe_x,o.local[1]+o.probe_y);o.tiles[3]=tile(o.world[0]+o.probe_x,o.world[1]+o.probe_y);}
  }
 }
 return o;
}
struct State {uint32_t actor=0,record=0;unsigned kind=0,lines=0,total_lines=0,updates=0,near_updates=0,points=0;};
inline State state;
inline void reset(){state={};}
inline void emit(const char* stage,const Observation& o,unsigned vi){
 if(state.lines>=64||state.total_lines>=192)return;++state.lines;++state.total_lines;
 std::fprintf(stderr,"[BossCheckpointProbe] stage=%s vi=%u actor=%08X kind=%04X record=%08X packed=%08X life=%u queue=%u state=%u/%u callback=%08X movement=%08X parent=%08X local=%.6f,%.6f,%.6f world=%.6f,%.6f,%.6f velocity=%.6f,%.6f,%.6f resource=%08X overlay=%08X private=%08X metadata=%08X definition=%08X contacts=%08X previousContacts=%08X probes=%u xs=%08X ys=%08X query=%08X terrain=%08X previousTerrain=%08X terrainFlags=%04X def74=%u def75=%u probeKnown=%u probe=%.6f,%.6f queryTile=%04X queryContact=%u correctionY=%.6f catalogueKnown=%u row=%08X catalogue=%08X,%08X,%08X,%08X,%08X,%08X map=%08X mapSize=%u,%u,%u tiles=%05X,%05X,%05X,%05X update=%u\n",
  stage,vi,o.actor,o.kind,o.record,o.packed,o.life,o.queue,o.primary,o.sub,o.callback,o.movement,o.parent,o.local[0],o.local[1],o.local[2],o.world[0],o.world[1],o.world[2],o.velocity[0],o.velocity[1],o.velocity[2],o.resource,o.overlay,o.private_data,o.metadata,o.definition,o.contacts,o.previous_contacts,o.probe_count,o.xs,o.ys,o.query,o.terrain,o.previous_terrain,o.terrain_flags,o.def74,o.def75,unsigned(o.probe_known),o.probe_x,o.probe_y,o.query_tile,o.query_contact,o.correction_y,unsigned(o.catalogue_known),o.row,o.catalogue[0],o.catalogue[1],o.catalogue[2],o.catalogue[3],o.catalogue[4],o.catalogue[5],o.map,o.columns,o.rows,o.stride,o.tiles[0],o.tiles[1],o.tiles[2],o.tiles[3],state.updates);
}
inline bool source(uint8_t* ram,const Observation& o,size_t capacity){
 return o.life!=3&&o.queue!=3&&o.packed==0x45070000&&pointer(o.record,16,capacity)&&word(ram,o.record)==0xFFFFFFFF&&
  word(ram,o.record+4)==std::bit_cast<uint32_t>(160.f)&&word(ram,o.record+8)==std::bit_cast<uint32_t>(448.f)&&
  pointer(o.resource,0x48,capacity)&&pointer(o.overlay,0x1720,capacity)&&word(ram,o.resource+0x44)==o.overlay+0x1340;
}
inline void stamp(uint8_t* ram,const recomp_context* c,unsigned vi,size_t capacity,const char* stage){
 if(!ram||!c||state.lines>=64||state.total_lines>=192)return;auto* rdram=ram;const auto a=uint32_t(c->r4);
 if(!pointer(a,0x1FC,capacity)||word(ram,a)!=0x19971110||MEM_HU(12,n::base(a))!=0x4592)return;
 const float x=scalar(ram,a+0x84),y=scalar(ram,a+0x88);if(!std::isfinite(x)||!std::isfinite(y)||x<104||x>216)return;
 const auto record=word(ram,a+0x1D8),packed=word(ram,a+0x1DC);if(!pointer(record,16,capacity)||packed>>16!=0x4592||word(ram,record)!=0xFFFFFFFF)return;
 if(word(ram,record+4)!=std::bit_cast<uint32_t>(x)||word(ram,record+8)!=std::bit_cast<uint32_t>(y))return;
 ++state.lines;++state.total_lines;std::fprintf(stderr,"[BossCheckpointProbe] stage=%s vi=%u floor=%08X record=%08X packed=%08X world=%.6f,%.6f local=%.6f,%.6f callback=%08X\n",stage,vi,a,record,packed,x,y,scalar(ram,a+0x3C),scalar(ram,a+0x40),word(ram,a+0x20));
}
inline void trace_entry(uint8_t* ram,const recomp_context* c,std::string_view name,unsigned vi,size_t capacity,bool enabled){
 if(!enabled||!ram||!c)return;
 if(name=="func_8069D05C"){stamp(ram,c,vi,capacity,"stamp-enter");return;}
 if(name!="func_805A6600"&&name!="func_800483B8")return;
 const auto o=observe(ram,uint32_t(c->r4),capacity);if(!o)return;
 if(name=="func_805A6600"){
  if(!source(ram,*o,capacity)||o->callback!=o->overlay+0x1340||o->local[0]!=160.f||o->local[1]!=448.f)return;
  if(state.actor!=o->actor||state.record!=o->record||state.kind!=o->kind){if(state.points>=3)return;state.actor=o->actor;state.record=o->record;state.kind=o->kind;state.lines=state.updates=state.near_updates=0;++state.points;}
  emit("init-enter",*o,vi);return;
 }
 if(o->actor!=state.actor||o->record!=state.record||o->kind!=state.kind||!source(ram,*o,capacity))return;
 ++state.updates;const float y=o->local[1]+(o->probe_known?o->probe_y:0.f);const bool near_surface=std::isfinite(y)&&y>=455&&y<=485;
 if(state.updates<=32||(near_surface&&state.near_updates++<12))emit("integrate-enter",*o,vi);
}
inline void trace_leave(uint8_t* ram,const recomp_context* c,std::string_view name,unsigned vi,size_t capacity,bool enabled){
 if(!enabled||!ram||!c)return;
 if(name=="func_8069D05C"){stamp(ram,c,vi,capacity,"stamp-leave");return;}
 if(name!="func_805A6600"||uint32_t(c->r4)!=state.actor)return;
 const auto o=observe(ram,state.actor,capacity);if(o&&o->record==state.record&&o->kind==state.kind)emit("init-leave",*o,vi);
}
} // namespace workshop::boss_checkpoint_probe0260
