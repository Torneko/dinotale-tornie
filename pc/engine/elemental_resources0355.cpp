#include "elemental_resources0355.hpp"
#include "gold_silver_resources0300.hpp"
#include "terrain_data.hpp"
#include <array>
#include <bit>
namespace workshop::elemental_resources0355 {
namespace {
using namespace workshop::terrain;
constexpr size_t bank3_base=0x526EC0,fire_info=0xFD9400,water_info=0xFD940C,fire_object=0xFD9420,water_object=0xFD9490;
constexpr uint32_t ptr(size_t address){return 0x03000000u|uint32_t(address-bank3_base);}
struct Object{size_t a,row;uint32_t behavior;};
Object object(const Bytes& rom,unsigned id){for(unsigned i=0;i<1582;++i){const size_t row=0xA1C90+i*12;if(be16(rom,row)==id)return{bank(rom,be32(rom,row+4)),row,be32(rom,row+8)};}throw std::runtime_error("Elemental template missing");}
void put(Bytes& rom,size_t at,const Bytes& bytes){need(at>=slab_begin&&at+bytes.size()<=slab_end,"Elemental slab overflow");std::copy(bytes.begin(),bytes.end(),rom.begin()+at);}
void descriptor(Bytes& rom,size_t info,size_t at,size_t length){word(rom,info,uint32_t(length));word(rom,info+4,uint32_t(length));word(rom,info+8,ptr(at));}
bool green_body(uint16_t color){const unsigned r=color>>11,g=(color>>6)&31,b=(color>>1)&31;return (color&1)&&g>r&&g>b&&g-std::min(r,b)>=4;}
Bytes recolor(const Bytes& palette,bool aquatic){
 need(palette.size()==512,"Unexpected elemental player palette");Bytes result=palette;
 for(unsigned i=0;i<256;++i){const auto c=be16(palette,i*2);if(!(c&1))continue;const unsigned r=c>>11,g=(c>>6)&31,b=(c>>1)&31,v=std::max({r,g,b});
  const bool boots=i>=37&&i<100,belly=i>=16&&i<=36,extra=(i>=5&&i<=9)||i==102||i==103;
  if(!boots&&!belly&&!extra&&!green_body(c))continue;
  const std::array<unsigned,3> target=boots?(aquatic?std::array<unsigned,3>{25,26,28}:std::array<unsigned,3>{31,29,2}):belly?(aquatic?std::array<unsigned,3>{18,29,31}:std::array<unsigned,3>{8,7,8}):aquatic?std::array<unsigned,3>{3,20,31}:std::array<unsigned,3>{31,13,2};
  const auto scale=[&](unsigned channel){return (v*channel+15)/31;};half(result,i*2,uint16_t((scale(target[0])<<11)|(scale(target[1])<<6)|(scale(target[2])<<1)|(c&1)));
 }
 // Oral red texels share this row with dorsal spikes; both intentionally follow
 // the elemental tongue. Warm boot highlights188/195/206 are outside this span.
 for(unsigned i=212;i<=243;++i)half(result,i*2,tongue_color(be16(result,i*2),aquatic?water:fire));
 return result;
}
Bytes append_players(const Bytes& original){need(original.size()==16*512,"Elemental pack needs sixteen player palettes");Bytes result=original;const Bytes green(original.begin(),original.begin()+512);for(bool aquatic:{false,true}){auto palette=recolor(green,aquatic);result.insert(result.end(),palette.begin(),palette.end());}return result;}
Bytes append_eggs(const Bytes& original,bool extra){const unsigned count=extra?17:16;need(original.size()==count*512,"Elemental pack needs known egg palette count");Bytes result=original;for(unsigned color:{fire,water}){const unsigned index=extra?1:0;Bytes row(original.begin()+index*512,original.begin()+(index+1)*512);for(unsigned i=0;i<256;++i)half(row,i*2,egg_color(be16(row,i*2),color));result.insert(result.end(),row.begin(),row.end());}return result;}
}
std::vector<uint8_t> apply(std::vector<uint8_t> rom){
 need(rom.size()==0x1000000,"Elemental resources must precede appended terrain");need(be32(rom,0xA79D4+12)==bank3_base,"Unexpected elemental bank");
 constexpr std::array<uint8_t,8> metallic_magic{'Y','S','G','S','0','3','0','0'};need(std::equal(metallic_magic.begin(),metallic_magic.end(),rom.begin()+gold_silver_resources0300::slab_begin)&&be32(rom,gold_silver_resources0300::slab_begin+8)==16,"Apply metallic resources before elementals");
 need(std::all_of(rom.begin()+slab_begin,rom.begin()+slab_end,[](uint8_t c){return c==255;}),"Elemental slab occupied");
 for(unsigned id:{fire_selection_id,water_selection_id})for(unsigned i=0;i<1582;++i)need(be16(rom,0xA1C90+i*12)!=id,"Elemental selection ID occupied");
 need(std::all_of(rom.begin()+0xF100A0,rom.begin()+0xF100B8,[](uint8_t c){return c==0;}),"Elemental resident lookup occupied");
 need(be16(rom,0xB47D4E)==0xFFFF&&be32(rom,0xB47D50)==0,"Unexpected extended selection preload");
 const auto player=object(rom,0x4001),egg=object(rom,0x4002),extra=object(rom,0x4524),castle=object(rom,0x4656),rescue=object(rom,0x4727),green=object(rom,0x41F1),tongue=object(rom,0x400C),tongue_body=object(rom,0x400B);
 const size_t source=bank(rom,be32(rom,player.a+0x28));need(source==gold_silver_resources0300::slab_begin+0x100,"Unexpected16-color player resource");
 const Bytes player_old(rom.begin()+source,rom.begin()+source+16*512);const auto player_new=append_players(player_old),egg_new=append_eggs(block(rom,be32(rom,egg.a+0x30)),false),extra_new=append_eggs(block(rom,be32(rom,extra.a+0x30)),true);
 size_t transition=0;for(unsigned i=0;i<10;++i){const size_t row=0xFB0000+i*12;if(be16(rom,row)==0x4006)transition=bank(rom,be32(rom,row+4));}need(transition,"Extended transition missing");
 const auto transition_new=append_players(block(rom,be32(rom,transition+0x30)));
 need(block(rom,be32(rom,castle.a+0x30))==player_old&&block(rom,be32(rom,rescue.a+0x30))==player_old,"Castle/rescue palette prefixes differ");
 const auto tongue_pointer=be32(rom,tongue.a+0x30);need(tongue_pointer==be32(rom,tongue_body.a+0x30),"Tongue body/tip palettes unexpectedly differ");const auto tongue_old=block(rom,tongue_pointer);need(tongue_old.size()==512,"Unexpected original tongue palette count");uint32_t tongue_hash=2166136261u;for(auto v:tongue_old)tongue_hash=(tongue_hash^v)*16777619u;need(tongue_hash==original_tongue_fnv,"Original classic red tongue palette changed");Bytes tongue_new=tongue_old;for(unsigned color:{fire,water}){Bytes row=tongue_old;for(unsigned i=0;i<256;++i)half(row,i*2,tongue_color(be16(row,i*2),color));tongue_new.insert(tongue_new.end(),row.begin(),row.end());}put(rom,tongue_at,tongue_new);descriptor(rom,bank(rom,tongue_pointer),tongue_at,tongue_new.size());
 // Castle factories use native White/Black actor aliases for extended colours.
 // Their original single-row resources must retain row0 for classic actors while
 // exposing the full display-index range used by the existing palette wrapper.
 for(unsigned alias:{6u,7u}){const auto a=object(rom,0x4656+alias);const auto info=bank(rom,be32(rom,a.a+0x30));const auto original=block(rom,be32(rom,a.a+0x30));
  need(original.size()==512&&std::equal(original.begin(),original.end(),player_old.begin()+alias*512),"Unexpected native castle alias palette");
  Bytes rows=player_new;std::copy(original.begin(),original.end(),rows.begin());const size_t at=0xFDAB00+(alias-6)*18*512;put(rom,at,rows);descriptor(rom,info,at,rows.size());
 }
 put(rom,player_at,player_new);put(rom,egg_at,egg_new);put(rom,egg_extra_at,extra_new);put(rom,transition_at,transition_new);word(rom,player.a+0x28,ptr(player_at));
 for(const auto& item:std::array<std::array<size_t,3>,5>{{{egg.a,egg_at,18*512},{extra.a,egg_extra_at,19*512},{castle.a,player_at,18*512},{rescue.a,player_at,18*512},{transition,transition_at,18*512}}})descriptor(rom,bank(rom,be32(rom,item[0]+0x30)),item[1],item[2]);
 for(unsigned n=0;n<2;++n){const size_t a=n?water_object:fire_object,info=n?water_info:fire_info;const uint16_t id=n?water_selection_id:fire_selection_id;
  put(rom,a,Bytes(rom.begin()+green.a,rom.begin()+green.a+0x64));half(rom,a+0x18,id);word(rom,a+0x30,ptr(info));descriptor(rom,info,player_at+(16+n)*512,512);
  const size_t row=0xF100A0+n*12;half(rom,row,id);half(rom,row+2,0);word(rom,row+4,ptr(a));word(rom,row+8,green.behavior);half(rom,0xB47D4E+n*2,id);
 }
 half(rom,0xB47D52,0xFFFF);
 constexpr std::array<float,6> xs{47,92,137,182,227,272};constexpr std::array<float,3> ys{110,145,180};
 for(unsigned i=0;i<18;++i){half(rom,metadata_rom+i*2,uint16_t(i<8?0x41F1+i:0x4EF1+i-8));word(rom,metadata_rom+0x30+i*4,std::bit_cast<uint32_t>(xs[i%6]));word(rom,metadata_rom+0x80+i*4,std::bit_cast<uint32_t>(ys[i/6]));word(rom,metadata_rom+0xD0+i*4,i*256);}
 word(rom,metadata_rom+0x118,0);constexpr std::array<uint8_t,8> magic{'Y','S','E','L','0','3','5','5'};std::copy(magic.begin(),magic.end(),rom.begin()+slab_begin);word(rom,slab_begin+8,18);word(rom,slab_begin+12,uint32_t(metadata_rom));word(rom,slab_begin+16,metadata_ram);word(rom,slab_begin+20,uint32_t(metadata_size));
 return rom;
}
} // namespace workshop::elemental_resources0355
