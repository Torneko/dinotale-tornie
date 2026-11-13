#pragma once
#include "elemental_abilities0355.hpp"
#include <cstdint>
namespace workshop::elemental_food_sources0355 {
using FoodFamily=elemental_abilities0355::FoodFamily;
// Exact native creature identities, verified against original sprite resources
// and overlay recipes; see pc/build/elementals-0355/native-food-identities0355.json.
// Caller supplies the CURRENT live actor kind at the synchronous food fence.
// Resource-B sharing or presence in a water level alone is never a classifier.
constexpr FoodFamily food_family(uint16_t kind){
 switch(kind){
 case 0x409E:return FoodFamily::Penguin;
 case 0x4195:case 0x45EC:case 0x45ED:case 0x45F3: // Native orange fish, including edible form.
 case 0x455D:case 0x4567:case 0x4568:case 0x4576:case 0x4577: // Blue aquatic spine family.
 case 0x4573:case 0x4574:return FoodFamily::Aquatic; // Pink spiny fish.
 default:return FoodFamily::Unknown;
 }
}
}
