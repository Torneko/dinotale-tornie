#pragma once
#include "yoshi_colors0300.hpp"
#include <optional>
namespace workshop::metal_rewards0300 {
// Native Lucky Fruit remains its own named8/8 bonus before the2/3 rules.
constexpr std::optional<int> food(unsigned color,unsigned id,bool score,unsigned native_class=8){
    if(!yoshi_colors0300::metal(color))return {};
    const bool fruit=(id>=0x4030&&id<=0x4034)||(id>=0x4F00&&id<0x4F06);
    if(fruit&&native_class==6)return 8;
    if(id>=0x4F00&&id<0x4F06)return 3;
    if(id>=0x4030&&id<=0x4034)return 2;
    if(id==0x45B6)return score?1:4;
    return {}; // Enemies and every other item retain the native White/Black path.
}
}
