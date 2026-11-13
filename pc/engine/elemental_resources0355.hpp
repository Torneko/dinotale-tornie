#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
namespace workshop::elemental_resources0355 {
inline constexpr unsigned fire=16,water=17,legacy_display_count=16,display_count=18;
inline constexpr unsigned logical_neutral_palette=18,raw_extra_neutral_index=0;
inline constexpr unsigned raw_extra_fire_index=17,raw_extra_water_index=18;
inline constexpr uint16_t fire_selection_id=0x4EF9,water_selection_id=0x4EFA;
inline constexpr size_t slab_begin=0xFD0000,slab_end=0xFE0000;
inline constexpr size_t player_at=0xFD0100,egg_at=0xFD2500,egg_extra_at=0xFD4900,transition_at=0xFD6F00;
inline constexpr size_t tongue_at=0xFD9700,tongue_palette_count=3;
inline constexpr uint32_t original_tongue_fnv=0x33F56928;
constexpr uint16_t tongue_color(uint16_t c,unsigned color){
 if(!(c&1)||(color!=fire&&color!=water))return c;const unsigned r=c>>11,g=(c>>6)&31,b=(c>>1)&31,v=r>g?(r>b?r:b):(g>b?g:b);
 const unsigned red=color==fire?(v*4+15)/31:0,green=color==fire?red:(v*24+15)/31,blue=color==fire?red:v;return uint16_t(red<<11|green<<6|blue<<1|1);
}
// Visual spots are independent of the Black/White native power aliases.
constexpr uint16_t egg_color(uint16_t c,unsigned color){
 if(!(c&1)||(color!=fire&&color!=water))return c;
 const unsigned r=c>>11,g=(c>>6)&31,b=(c>>1)&31;
 if(!(g>r&&g>b&&g-(r<b?r:b)>=4))return c;
 const unsigned v=r>g?(r>b?r:b):(g>b?g:b);
 const unsigned rr=color==fire?31:3,gg=color==fire?13:20,bb=color==fire?2:31;
 return uint16_t(((v*rr+15)/31)<<11|((v*gg+15)/31)<<6|((v*bb+15)/31)<<1|(c&1));
}
inline constexpr size_t metadata_rom=0xFD9500,metadata_size=0x120;
inline constexpr uint32_t metadata_ram=0x807FD300;
inline constexpr uint32_t selection_ids_ram=metadata_ram,selection_x_ram=metadata_ram+0x30,selection_y_ram=metadata_ram+0x80,rescue_offsets_ram=metadata_ram+0xD0;
constexpr bool elemental(unsigned color){return color==fire||color==water;}
// Native powers remain Black for Fire and White for Water; visuals use their elemental row.
// Return8 for non-elemental input so a caller cannot silently alias a legacy slot.
constexpr unsigned native_alias(unsigned color){return color==fire?7:color==water?6:8;}
// Apply after gold_silver_resources0300::apply and BEFORE any appended terrain.
// No disk ROM or six-byte progression codec is modified. All16 previous player,
// egg, transition and rescue palettes remain exact prefixes. Metadata uses a
// separate18-entry layout; the old FC metallic slab stays byte-for-byte intact.
std::vector<uint8_t> apply(std::vector<uint8_t> private_rom);
}
