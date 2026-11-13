#pragma once
#include <cstdint>
#include <optional>

// Policy decisions only. Native adapters must establish causal provenance before
// assigning a FoodFamily or Hazard; an address or a damage value is not proof.
// This header does not grant ownership, mutate RAM, force a player state, make
// all contacts harmless, or claim the pending lava/edibility hooks are complete.
namespace workshop::elemental_abilities0355 {
inline constexpr unsigned fire=16, water=17;
constexpr bool elemental(unsigned color){return color==fire||color==water;}
constexpr unsigned native_alias(unsigned color){return color==fire?7:color==water?6:8;}
struct Scope {
 bool solo=false; unsigned color=18,alias=8;
 constexpr bool admitted() const{return solo&&elemental(color)&&alias==native_alias(color);}
};
// Reused native slots must carry the same nonzero generation as the observation.
struct Birth {
 uint32_t address=0; uint64_t generation=0;
 constexpr bool valid()const{return address>=0x80000400&&address<=0x807FFE00&&generation!=0;}
 constexpr bool same(const Birth& other)const{return valid()&&other.valid()&&address==other.address&&generation==other.generation;}
};
constexpr bool fruit(uint16_t kind){return (kind>=0x4030&&kind<=0x4034)||(kind>=0x4F00&&kind<0x4F06);}
// Existing food adapters clamp health, handle fatal poison and generate sounds.
// Lucky-fruit class6 stays8/8; melon keeps its universal native bonus.
// No fruit is a color-specific favorite.
constexpr std::optional<int> food(unsigned color,uint16_t kind,bool score,unsigned native_class=8){
 if(!elemental(color))return {};
 if(kind==0x45B6)return score?std::optional<int>{}:std::optional<int>{color==fire?8:-3};
 if(fruit(kind)){if(native_class==6)return 8;if(kind==0x4034)return {};return 1;}
 return {};
}
// Native poison continuation has no lower clamp. Apply this at the health hook,
// using the currently selected native HP, before the real native addition.
constexpr int clamp_health_delta(int delta,int current){return delta<0&&current>=0&&delta < -current?-current:delta;}
enum class FoodFamily {Unknown,Aquatic,Penguin};
struct FoodObservation {
 Birth current{},observed{};FoodFamily family=FoodFamily::Unknown;
 // A native source/property classifier must prove family. No guessed ID list.
 bool classified=false;
};
constexpr std::optional<int> enemy_food(const Scope& scope,const FoodObservation& source,bool score,int native_amount){
 if(!scope.admitted()||scope.color!=water||!source.classified||!source.current.same(source.observed))return {};
 if(source.family==FoodFamily::Penguin)return 2;
 if(source.family==FoodFamily::Aquatic&&!score&&native_amount>0&&native_amount<=8)return native_amount*3;
 return {};
}
enum class Hazard {Unknown,FireAttack,Lava,Snowball};
struct Contact {
 Birth current{},observed{};Hazard hazard=Hazard::Unknown;bool classified=false;
};
constexpr std::optional<unsigned> damage(const Scope& scope,const Contact& source){
 if(!scope.admitted()||!source.classified||!source.current.same(source.observed))return {};
 if(scope.color==fire&&source.hazard==Hazard::FireAttack)return 1;
 if(scope.color==fire&&source.hazard==Hazard::Lava)return 0;
 if(scope.color==water&&source.hazard==Hazard::Snowball)return 0;
 return {};
}
constexpr bool can_eat_flame(const Scope& scope,const Contact& source){
 return scope.admitted()&&scope.color==fire&&source.classified&&source.hazard==Hazard::FireAttack&&source.current.same(source.observed);
}
} // namespace workshop::elemental_abilities0355
