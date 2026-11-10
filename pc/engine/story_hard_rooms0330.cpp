#include "story_hard_rooms0330.hpp"
#include "workshop_data.hpp"
#include <bit>
#include <cmath>
#include <optional>
namespace workshop::story_hard_rooms0330 {
namespace {
constexpr std::array<unsigned,24> target_rooms{81,85,119,92,120,93,106,110,82,103,130,105,117,97,86,118,21,121,123,101,89,157,134,100};
size_t align16(size_t v){return (v+15)&~size_t(15);}
size_t bank(std::span<const uint8_t> rom,uint32_t pointer){return terrain::bank(rom,pointer);}
void half(std::vector<uint8_t>& rom,size_t at,uint16_t value){terrain::half(rom,at,value);}
void word(std::vector<uint8_t>& rom,size_t at,uint32_t value){terrain::word(rom,at,value);}
struct Room {size_t header=0,meta=0,objects=0;unsigned count=0;};
Room room(std::span<const uint8_t> rom,unsigned id){
 require(id<177,"Hard room index invalid");Room r;
 r.header=bank(rom,u32(rom,0xA84A0+size_t(id)*8+4));require(r.header<=rom.size()&&0x118<=rom.size()-r.header,"Hard room header truncated");
 r.meta=bank(rom,u32(rom,bank(rom,u32(rom,r.header+0x5C))));r.count=u16(rom,r.meta);r.objects=bank(rom,u32(rom,r.meta+4));
 require(r.count>0&&r.count<=149&&r.objects<=rom.size()&&size_t(r.count)*12<=rom.size()-r.objects,"Hard original object list invalid");return r;
}
Anchor placement(std::span<const uint8_t> rom,unsigned id,const Room& r,unsigned index,AnchorKind kind){
 require(index<r.count,"Hard anchor index invalid");const size_t at=r.objects+size_t(index)*12;
 return {kind,u32(rom,at),u32(rom,at+4),u32(rom,at+8),id,177,0,at};
}
Anchor final_entry(std::span<const uint8_t> rom,const navigation::Source& native){
 const Room dest=room(rom,123);std::optional<Anchor> found;
 for(unsigned i=0;i<dest.count;++i){const auto a=placement(rom,123,dest,i,AnchorKind::NativeEntry);
  if(a.packed>>16==0x4001&&(std::bit_cast<float>(a.x)!=0||std::bit_cast<float>(a.y)!=0)){require(!found,"Hard final-room entry ambiguous");found=a;}
 }
 if(found)return *found;
 // Room123's original4001 is(0,0), a transport placeholder. Resolve its real
 // destination object through unchanged native passage records instead.
 for(unsigned origin:vanilla_rooms[18])if(origin<177){const auto source=room(rom,origin);
  for(unsigned i=0;i<source.count;++i){const auto from=placement(rom,origin,source,i,AnchorKind::NativeEntry);const auto route=native.passage(uint16_t(from.packed>>16));
   if(!route||route->destination!=123||route->target==UINT32_MAX)continue;
   std::optional<Anchor> arrival;
   for(unsigned n=0;n<dest.count;++n){auto a=placement(rom,123,dest,n,AnchorKind::NativeEntry);
    if((a.packed&0xFFFF03FFu)!=(route->target&0xFFFF03FFu))continue;
    require(!arrival,"Hard native passage destination is ambiguous");
    a.via_room=origin;a.via_packed=from.packed;arrival=a;
   }
   // Match Source::effective_entry: choose the first proven inbound route in
   // native placement order. The second real entrance remains untouched.
   if(arrival)return *arrival;
  }
 }
 require(found.has_value(),"Hard final room has no proven native arrival");return *found;
}
void preload(std::span<const uint8_t> original,std::vector<uint8_t>& output,size_t header){
 size_t end=0;unsigned pool=0,bubble=0;
 for(size_t off=0x60;off<0x100;off+=2){const auto kind=u16(original,off);if(kind==0xFFFF){end=off;break;}pool+=kind==0x401D;bubble+=kind==0x4692;}
 require(end&&pool==1&&bubble<=1,"Hard native preload list incompatible");
 if(bubble)return;
 require(end+4<0x100&&u16(original,end+2)==0,"Hard preload padding unavailable");
 // The copied header is byte-identical apart from this appended resource and
 // its new terminator.401D is already listed once and remains unchanged.
 half(output,header+end,0x4692);half(output,header+end+2,0xFFFF);
}
}
Applied apply(std::span<const uint8_t> original){
 require(original.size()==allocation_limit&&u32(original,0)==0x80371240,"Hard requires normalized USA-sized ROM");
 require(std::all_of(original.begin()+allocation_start,original.end(),[](uint8_t b){return b==255;}),"Hard private ROM arena occupied");
 const auto bank4=u32(original,0xA79D4+16);require(bank4>0&&bank4<allocation_start,"Hard bank4 base invalid");
 auto pointer=[&](size_t at){require(at>=bank4&&at<allocation_limit&&at-bank4<=0xFFFFFF,"Hard bank4 pointer overflow");return 0x04000000u|uint32_t(at-bank4);};
 navigation::Source native(original);Applied result;result.rom.assign(original.begin(),original.end());size_t cursor=allocation_start;
 for(unsigned level=0;level<24;++level){
  std::optional<Anchor> anchor;unsigned warp_count=0;
  for(unsigned id:vanilla_rooms[level])if(id<177){const auto r=room(original,id);for(unsigned i=0;i<r.count;++i)if(u16(original,r.objects+size_t(i)*12)==0x450A){++warp_count;anchor=placement(original,id,r,i,AnchorKind::Warp4);}}
  if(level==18){require(warp_count==0,"Hard5-3 source unexpectedly contains Warp4");anchor=final_entry(original,native);}else require(warp_count==1,"Hard course requires its unique native Warp4");
  require(anchor&&anchor->room==target_rooms[level],"Hard source room identity changed");
  require(navigation::coordinate(anchor->x)&&navigation::coordinate(anchor->y),"Hard anchor coordinate invalid");
  const auto source=room(original,anchor->room);require(source.count<149,"Hard appended bubble exceeds native149-object cap");
  const float ax=std::bit_cast<float>(anchor->x),ay=std::bit_cast<float>(anchor->y);
  //5-3 enters atY504 along the top of its existing page. Put its bubble
  //40units below that entrance; all23 Warp4 anchors use the40unit rise.
  uint32_t x=std::bit_cast<uint32_t>(ax+32.0f),y=std::bit_cast<uint32_t>(ay+(level==18?-40.0f:40.0f));
  if(!native.within_terrain(anchor->room,{x,y}))x=std::bit_cast<uint32_t>(ax-32.0f);
  require(navigation::coordinate(x)&&navigation::coordinate(y)&&native.within_terrain(anchor->room,{x,y}),"Hard bubble must remain inside an existing terrain page");
  const size_t objects=align16(cursor),meta=align16(objects+size_t(source.count+1)*12),header=align16(meta+12),end=align16(header+0x118);
  require(end<=allocation_limit,"Hard private ROM arena overflow");
  std::copy_n(original.begin()+source.objects,size_t(source.count)*12,result.rom.begin()+objects);
  const size_t added=objects+size_t(source.count)*12;word(result.rom,added,bubble_packed);word(result.rom,added+4,x);word(result.rom,added+8,y);
  // Native metadata is count,padding,list pointer, then one indirection word.
  // Preserve its original two padding bytes rather than assuming their value.
  std::copy_n(original.begin()+source.meta,8,result.rom.begin()+meta);half(result.rom,meta,uint16_t(source.count+1));word(result.rom,meta+4,pointer(objects));word(result.rom,meta+8,pointer(meta));
  std::copy_n(original.begin()+source.header,0x118,result.rom.begin()+header);preload(original.subspan(source.header,0x118),result.rom,header);word(result.rom,header+0x5C,pointer(meta+8));
  word(result.rom,0xA84A0+size_t(anchor->room)*8+4,pointer(header));
  result.plans[level]={level,anchor->room,source.count,source.count,bubble_packed,x,y,*anchor,source.header,source.meta,source.objects,objects,meta,header,added,true,false};cursor=end;
 }
 result.allocation_end=cursor;return result;
}
} // namespace workshop::story_hard_rooms0330
