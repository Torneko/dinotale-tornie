#pragma once
#include "metal_native0300.hpp"
namespace workshop::boss_metals0354 {
// Separate from the canonical acquisition/Story service scope. Boss play may
// project already-owned colors, but cannot find, buy, restore or persist them.
constexpr bool scope(bool native_boss,bool cooperative,bool has_payload,bool custom,bool tally){return native_boss&&!cooperative&&!has_payload&&!custom&&!tally;}
inline unsigned available_extra(const metal_yoshis0300::View& found){return found.initialized&&found.readable&&found.saved&&found.found<=3?(found.found<<6):0;}
inline unsigned saved_lives(unsigned legacy,const metal_yoshis0300::View& found){return (legacy&0x3Fu)|available_extra(found);}
inline bool death(uint8_t* rdram,unsigned color){
 if(!rdram||!yoshi_colors0300::metal(color))return false;
 // Gold/Silver are extension bits6/7, never native White/Black bits6/7.
 metal_native0300::extras(rdram,metal_native0300::extras(rdram)&~(1u<<(color-8)));return true;
}
} // namespace workshop::boss_metals0354
