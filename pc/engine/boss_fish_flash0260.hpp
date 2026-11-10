#pragma once
#include <cstdint>
namespace workshop::boss_fish_flash0260 {
// Native41C6: accepted4018 hit writes DE4=50 (80554A08); updates use
// FFC=float((remaining>>2)&1) (80554204/54634). No Fish state is written.
inline constexpr unsigned duration_ticks=50;
inline constexpr bool alternate(unsigned age){
 return age<duration_ticks && (((duration_ticks-age)>>2)&1u)!=0;
}
// Native41C6 palette at res88+200 is exactly RGB->BRG, alpha unchanged.
// Apply after the member's colour tint so every identity returns unchanged
// between flashes. Native514AC/B74 selects this palette when FFC==1.
inline constexpr uint16_t color(uint16_t rgba5551,bool phase){
 if(!phase)return rgba5551;
 const unsigned r=(rgba5551>>11)&31,g=(rgba5551>>6)&31,b=(rgba5551>>1)&31;
 return uint16_t((b<<11)|(r<<6)|(g<<1)|(rgba5551&1));
}
}
