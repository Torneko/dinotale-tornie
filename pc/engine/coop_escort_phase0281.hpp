#pragma once
#include <array>
#include <cstdint>

namespace workshop::coop_escort_phase0281 {
// Prepared section128, native8057C6B0 reads this fourteen-entry table at+F90.
// C728 temporarily installs the index-specific initializer at+FE8 first.
inline constexpr std::array<uint32_t,14> updates{
    0x5AC,0x5AC,0x638,0x638,0x838,0x8A0,
    0x908,0x950,0xA80,0x998,0x9D0,0x9D0,0xA80,0xA80};
inline constexpr std::array<uint32_t,4> initializers{0xB74,0xB48,0xB48,0xB74};
inline constexpr bool matches(uint32_t overlay,uint32_t update,unsigned sub,unsigned index){
    if(index>=initializers.size())return false;
    if(sub<updates.size()&&update==overlay+updates[sub])return true;
    // Resource constructorC658 already selected sub0 beforeC728 replaces20.
    return sub==0&&update==overlay+initializers[index];
}
}
