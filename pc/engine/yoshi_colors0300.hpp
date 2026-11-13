#pragma once
#include <cstdint>
namespace workshop::yoshi_colors0300 {
inline constexpr unsigned gold=14,silver=15,fire=16,water=17,total=18,legacy_total=14,hybrid_count=6;
inline constexpr uint32_t legacy_mask=0x3FFF,all_mask=0x3FFFF,metal_mask=0xC000;
inline constexpr unsigned legacy_extra_mask=0x3F,metal_extra_mask=0xC0;
constexpr bool metal(unsigned color){return color==gold||color==silver;}
constexpr bool elemental(unsigned color){return color==fire||color==water;}
constexpr bool extended(unsigned color){return metal(color)||elemental(color);}
constexpr bool hybrid(unsigned color){return color>=8&&color<legacy_total;}
// Display14/15 never index an original eight-entry native table.
constexpr unsigned native_alias(unsigned color){return color<8?color:hybrid(color)?0:metal(color)?color-8:color==fire?7:color==water?6:8;}
constexpr unsigned metal_bit(unsigned color){return metal(color)?1u<<(color-gold):0;}
constexpr unsigned extra_from_metals(unsigned mask){return (mask&3u)<<6;}
constexpr unsigned metals_from_extra(unsigned mask){return (mask>>6)&3u;}
}
