#pragma once
#include <array>
#include <cstdint>
namespace workshop::special {
inline constexpr std::array<std::array<unsigned,2>,6> components{{{1,2},{1,3},{3,0},{2,0},{1,0},{3,2}}};
constexpr int fruit_index(uint16_t id){return id>=0x4F00 && id<0x4F06?int(id-0x4F00):-1;}
constexpr bool favorite(unsigned fruit,unsigned yoshi){
    const unsigned family=yoshi==4?3:yoshi==5?1:yoshi;
    return fruit<6 && yoshi<14 && (yoshi==6 || yoshi==7 || yoshi==8+fruit || family==components[fruit][0] || family==components[fruit][1]);
}
constexpr unsigned food_class(unsigned fruit,unsigned yoshi){return favorite(fruit,yoshi)?3:2;}
}
