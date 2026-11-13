#pragma once
#include "fruit_mix.hpp"
#include "special_content_rules.hpp"
#include "metal_rewards0300.hpp"
#include "elemental_abilities0355.hpp"
#include "elemental_food_sources0355.hpp"
#include "hybrid_maskass_data.hpp"
#include <optional>

namespace workshop::rewards {
inline std::optional<unsigned>(*custom_enemy_variant)(uint32_t)=nullptr;
inline unsigned(*lucky_fruit_type)()=nullptr;
constexpr bool hybrid(unsigned color){return color>=8 && color<14;}
constexpr int custom_maskass_gain(unsigned variant,unsigned color){
    const auto pair=special::components[variant];
    auto standard=[&](unsigned c){const unsigned family=c==4?3:c==5?1:c;return family==pair[0] || family==pair[1]?3:1;};
    return hybrid(color)?(standard(special::components[color-8][0])+standard(special::components[color-8][1])+1)/2:standard(color);
}
inline std::optional<int> custom_gain(uint32_t actor,unsigned color){
    if(!custom_enemy_variant || color>=16)return {};
    auto variant=custom_enemy_variant(actor);if(!variant || *variant>=6)return {};
    if(color==6 || color==7)return 2;
    if(yoshi_colors0300::metal(color))return 3;
    return custom_maskass_gain(*variant,color);
}
constexpr bool native_maskass(uint16_t id){
    for(const auto kind:hybrid_mode::maskass_ids)if(kind==id)return true;
    return false;
}
// Only these four colors gain a new Maskass policy. Read the CURRENT native
// actor identity and current hybrid variant at the reward fence, including
// after a ground-pound recolor; never retain a spawn-time color here.
inline std::optional<int> custom_gain(uint8_t* rdram,uint32_t actor,unsigned color){
    if(color!=6 && color!=7 && !yoshi_colors0300::metal(color))return custom_gain(actor,color);
    if(!rdram || !fruit_mix::range(actor,0x10) || !native_maskass(MEM_HU(12,int32_t(actor))))return {};
    if(auto value=custom_gain(actor,color))return value;
    if(yoshi_colors0300::metal(color))return 2;
    return {}; // Ordinary White/Black Maskass retain genuine native3/3.
}
constexpr bool enemy_food(uint16_t id){
    return !(id>=0x4030 && id<=0x4035) && special::fruit_index(id)<0
        && id!=0x45B6 && id!=0x45BC && id!=0x45BF && id!=0x45F3;
}
constexpr int mean_up(int a,int b){const int sum=a+b;return sum/2+(sum>0 && sum%2);}
inline unsigned food_class(uint32_t properties,unsigned color){return properties>>(28-4*color)&7;}
inline int gain(uint8_t* rdram,unsigned food,bool score){return MEM_W(int(food*4),int32_t(score?0x8009DF24:0x8009DF84));}
inline std::optional<uint32_t> properties(uint8_t* rdram,uint32_t actor){
    if(!fruit_mix::range(actor,0x178))return {};
    uint32_t data=MEM_W(0x174,int32_t(actor));if(!fruit_mix::range(data,4))return {};
    data=MEM_W(0,int32_t(data));if(!fruit_mix::range(data,0x1C))return {};
    return uint32_t(MEM_W(0x18,int32_t(data)));
}
inline bool no_enemy_score(uint8_t* rdram,uint32_t actor){
    if(!fruit_mix::range(actor,0x178) || !enemy_food(MEM_HU(12,int32_t(actor))))return false;
    const auto props=properties(rdram,actor);if(!props)return false;
    // Native classes 1 and 5 carry no points for any Yoshi. Repeated enemy
    // sources use this profile; class 5 may still heal and show an empty heart.
    for(unsigned color=0;color<8;color++)if(gain(rdram,food_class(*props,color),true)!=0)return false;
    return true;
}
inline int enemy_mean(uint8_t* rdram,uint32_t props,unsigned color,bool score,bool killed=false){
    const auto pair=special::components[color-8];
    int values[2];
    for(unsigned i=0;i<2;i++){
        const auto kind=food_class(props,pair[i]);values[i]=gain(rdram,kind,score);
        // A life-only bonus is converted to points on a kill. The native
        // continuation applies x2 (and its existing combo multipliers).
        if(killed){const int life=gain(rdram,kind,false);if(life>values[i])values[i]=life;}
    }
    return mean_up(values[0],values[1]);
}
inline std::optional<int> food_gain(uint8_t* rdram,uint32_t actor,unsigned color,bool score,unsigned native_class=8){
    if(!fruit_mix::range(actor,0x178))return {};
    const auto id=MEM_HU(12,int32_t(actor));
    const int fruit=special::fruit_index(id);
    if(fruit>=0 && lucky_fruit_type && unsigned(6+fruit)==lucky_fruit_type())return 8;
    // Water chili enters the poison continuation for health, but its score
    // still comes from this food's genuine native color property, not class0.
    if(elemental_abilities0355::elemental(color)&&id==0x45B6&&score){
        if(const auto props=properties(rdram,actor))return gain(rdram,food_class(*props,elemental_abilities0355::native_alias(color)),true);
        return {};
    }
    if(auto elemental=elemental_abilities0355::food(color,id,score,native_class))return elemental;
    if(auto metal=metal_rewards0300::food(color,id,score,native_class))return metal;
    // A real413A fire creature has an original food definition, but the
    // native tongue refuses its flags. The separate causal tongue gate admits
    // only Fire; never turn transient flame meshes into fake food actors.
    if(color==elemental_abilities0355::fire&&id==0x413A&&properties(rdram,actor))return score&&no_enemy_score(rdram,actor)?0:1;
    if(color==elemental_abilities0355::water){
        const auto family=elemental_food_sources0355::food_family(id);
        if(family!=elemental_abilities0355::FoodFamily::Unknown){
            if(const auto props=properties(rdram,actor)){
                if(family==elemental_abilities0355::FoodFamily::Penguin)return score&&no_enemy_score(rdram,actor)?0:2;
                if(!score){const auto life=gain(rdram,food_class(*props,6),false);if(life>0&&life<=8)return life*3;}
            }
        }
    }
    if(no_enemy_score(rdram,actor)){
        if(score)return 0;
        if(hybrid(color))return enemy_mean(rdram,*properties(rdram,actor),color,false);
        return {};
    }
    if(auto gain=custom_gain(rdram,actor,color))return gain;
    if(!hybrid(color))return {};
    if(fruit>=0)return color==8+unsigned(fruit)?std::optional<int>(4):std::nullopt;
    if(!enemy_food(id))return {};
    const auto props=properties(rdram,actor);if(!props)return {};
    return enemy_mean(rdram,*props,color,score);
}
// Existing classes retain their original animations. Additional integer gains
// travel in the popup queue as literal amounts, never as table indices.
constexpr unsigned popup(int points){
    switch(points){case -1:return 0;case 0:return 1;case 1:return 2;case 2:return 3;case 3:return 4;case 4:return 8;case 8:return 6;default:return 0x80u|unsigned(points);}
}
}
