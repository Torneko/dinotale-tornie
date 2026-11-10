#pragma once
#include "workshop_data.hpp"
#include "boss_battle0250.hpp"
#include <array>
namespace workshop::boss_lips_terrain_alias0260 {
inline constexpr uint16_t first_id=0x8F10,last_id=0x8F1F;
inline constexpr unsigned slots=16,record_size=12,table_size=slots*record_size;
inline constexpr uint32_t allocation=0x00FFC000,end=0x01000000,ram_table=0x807FC080;
static_assert(table_size==192 && ram_table+table_size==0x807FC140);
static_assert((first_id>>14)==2 && (last_id>>14)==2);
inline std::array<uint8_t,table_size> prepared_table{};
inline bool prepared=false;
inline constexpr bool alias_id(unsigned id){return id>=first_id&&id<=last_id;}
inline std::array<uint16_t,2> pair(unsigned index){require(index>=1&&index<=8,"Lips geometry encounter outside1..8");const auto id=uint16_t(first_id+2*(index-1));return {id,uint16_t(id+1)};}
inline size_t lookup(std::span<const uint8_t> rom,uint16_t id){for(size_t p=0xA1C90;p<0xA1C90+1582*12;p+=12)if(u16(rom,p)==id)return p;return 0;}
inline size_t cell(unsigned x,unsigned y){require(x<32&&y<16,"Lips terrain cell outside native512x256");return (y*32+x)*2;}
inline void validate_source(std::span<const uint8_t> rom,const std::array<size_t,2>& a,const std::array<terrain::Bytes,2>& visual,const terrain::Bytes& collision){
 for(unsigned layer=0;layer<2;++layer){require(u16(rom,a[layer])==0&&u32(rom,a[layer]+8)==0x8006E548&&u16(rom,a[layer]+0x18)==0x8110+layer,"Unknown Lips geometry resource class");const auto d=terrain::bank(rom,u32(rom,a[layer]+0x20));for(unsigned i=0;i<6;++i)require(u16(rom,d+2*i)==std::array<unsigned,6>{256,256,16,16,512,256}[i],"Lips dimensions changed");const auto pages=terrain::block(rom,u32(rom,a[layer]+0x40));require(pages==terrain::Bytes{0,1}&&visual[layer].size()==1024,"Lips native two-page layout changed");}
 require(collision.size()==1024&&u32(rom,a[1]+0x50)==0,"Lips collision/background separation changed");
 for(unsigned y=0;y<16;++y)for(unsigned x=0;x<32;++x){const bool wall=y==0||y>=13||x<4||x>=28;const bool appui=(y==7&&x>=14&&x<18)||(y==9&&((x>=8&&x<10)||(x>=22&&x<24)));require(u16(collision,cell(x,y))==(wall?0x0701:appui?0x0601:0),"Lips source collision fixture changed");}
 for(unsigned x=14;x<18;++x){require(u16(visual[1],cell(x,7))==0x026B+(x%2)&&u16(visual[1],cell(x,8))==0x026D+(x%2),"Lips central platform art changed");}
 for(unsigned x:{8u,22u}){require(u16(visual[1],cell(x,9))==0x026F&&u16(visual[1],cell(x+1,9))==0x0270&&u16(visual[1],cell(x,10))==0x0275&&u16(visual[1],cell(x+1,10))==0x0276,"Lips side platform art changed");}
}
inline void edit(unsigned terrain_index,terrain::Bytes& collision,terrain::Bytes& background){
 require(terrain_index==1||terrain_index==2,"Only Lips terrains1/2 have aliases");
 auto erase=[&](unsigned x,unsigned y,unsigned width,unsigned source_x){for(unsigned dx=0;dx<width;++dx){terrain::half(collision,cell(x+dx,y),0);for(unsigned dy=0;dy<2;++dy)terrain::half(background,cell(x+dx,y+dy),u16(background,cell(source_x+dx,y+dy)));}};
 if(terrain_index==1)erase(14,7,4,10);else {erase(8,9,2,12);erase(22,9,2,10);}
}
// Immutable records for one entire boot. Unique IDs belong to physical fight
// indices, never to the mutable current-room state or a shared live cache.
inline void install(std::vector<uint8_t>& rom,const boss_battle0250::Plan& plan){
 prepared=false;prepared_table.fill(0);require(rom.size()==rom_size&&boss_battle0250::valid(plan),"Invalid Lips terrain installation");require(std::all_of(rom.begin()+allocation,rom.begin()+end,[](uint8_t v){return v==255;}),"Lips geometry slab occupied");
 for(unsigned id=first_id;id<=last_id;++id)require(!lookup(rom,uint16_t(id)),"Lips alias collides with native resource ID");
 const std::array<size_t,2> entries{lookup(rom,0x8110),lookup(rom,0x8111)};require(entries[0]&&entries[1],"Missing Lips geometry source");
 const std::array<size_t,2> a{terrain::bank(rom,u32(rom,entries[0]+4)),terrain::bank(rom,u32(rom,entries[1]+4))};for(unsigned layer=0;layer<2;++layer)require(u32(rom,terrain::bank(rom,u32(rom,entries[layer]+8)))==0x05000100,"Lips native geometry B header changed");
 const std::array<terrain::Bytes,2> visual{terrain::block(rom,u32(rom,a[0]+0x38)),terrain::block(rom,u32(rom,a[1]+0x38))};const auto collision=terrain::block(rom,u32(rom,a[0]+0x50));validate_source(rom,a,visual,collision);
 const auto bank4=u32(rom,0xA79D4+16);auto pointer=[&](size_t p){require(p>=bank4&&p-bank4<=0xFFFFFF,"Lips clone bank overflow");return 0x04000000u|uint32_t(p-bank4);};size_t cursor=allocation+table_size;
 auto append=[&](std::span<const uint8_t> bytes){require(cursor<=end&&bytes.size()<=end-cursor,"Lips geometry clone slab overflow");const auto at=cursor;std::copy(bytes.begin(),bytes.end(),rom.begin()+at);cursor=(cursor+bytes.size()+15)&~size_t(15);return at;};
 auto store=[&](const terrain::Bytes& bytes){const auto at=append(bytes);std::vector<uint8_t> info(12);put32(info,0,uint32_t(bytes.size()));put32(info,4,uint32_t(bytes.size()));put32(info,8,pointer(at));return append(info);};unsigned occurrences=0;
 for(unsigned index=1;index<=8;++index){const auto& encounter=plan.fights[index];if(encounter.family!=boss_battle0250::Family::Lips||!encounter.terrain)continue;require(++occurrences<=4,"Too many Lips encounters for reserved slab");const auto ids=pair(index);auto col=collision,bg=visual[1];edit(encounter.terrain,col,bg);const std::array<size_t,2> blocks{store(col),store(bg)};
  for(unsigned layer=0;layer<2;++layer){std::vector<uint8_t> descriptor(rom.begin()+a[layer],rom.begin()+a[layer]+0x64);terrain::half(descriptor,0x18,ids[layer]);put32(descriptor,layer?0x38:0x50,pointer(blocks[layer]));const auto cloned=append(descriptor);const unsigned slot=unsigned(ids[layer]-first_id)*record_size;terrain::Bytes record(record_size);terrain::half(record,0,ids[layer]);put32(record,4,pointer(cloned));put32(record,8,u32(rom,entries[layer]+8));std::copy(record.begin(),record.end(),prepared_table.begin()+slot);}
 }
 require(cursor<=allocation+0x24C0,"Lips maximum four-pair allocation bound exceeded");std::copy(prepared_table.begin(),prepared_table.end(),rom.begin()+allocation);prepared=true;
}
}
#ifdef WORKSHOP_NATIVE_COOP
#include "recomp.h"
namespace workshop::boss_lips_terrain_alias0260::runtime {
inline bool ready=false,used=false;
inline void reset(uint8_t* rdram=nullptr){ready=false;used=false;if(rdram)for(unsigned i=0;i<table_size;++i)MEM_B(i,static_cast<gpr>(int32_t(ram_table)))=0;}
inline bool publish(uint8_t* rdram){
 if(!rdram||ready||used||!prepared)return false;
 for(unsigned i=0;i<table_size;++i)if(MEM_BU(i,static_cast<gpr>(int32_t(ram_table)))!=prepared_table[i])return false;
 ready=true;return true;
}
inline bool try_lookup(uint8_t* rdram,recomp_context* context){
 if(!rdram||!context)return false;const auto id=uint16_t(context->r4);if(!alias_id(id)||!ready)return false;const unsigned slot=unsigned(id-first_id)*record_size;
 if(u16(prepared_table,slot)!=id||u16(prepared_table,slot+2)||!u32(prepared_table,slot+4)||!u32(prepared_table,slot+8))return false;
 for(unsigned i=0;i<record_size;++i)if(MEM_BU(slot+i,static_cast<gpr>(int32_t(ram_table)))!=prepared_table[slot+i])return false;
 used=true;context->r5=id;context->r2=static_cast<gpr>(int32_t(ram_table+slot));return true;
}
template<class Original> inline void dispatch(uint8_t* rdram,recomp_context* context,Original original){if(try_lookup(rdram,context))return;require(context&&!alias_id(uint16_t(context->r4)),"Lips terrain alias unavailable or corrupted");original(rdram,context);}
}
#endif
