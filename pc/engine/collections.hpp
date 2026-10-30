#pragma once
#include "recomp.h"
#include <array>
#include <bit>
#include <filesystem>
#include <optional>
#include <vector>
namespace workshop::collections {
enum class Item:unsigned {White,Black,Hybrid,Editor};
constexpr std::array<unsigned,4> prices{120,120,200,40};
struct Place {unsigned room=0,index=0,x=0,y=0;bool operator==(const Place&)const=default;};
struct Level {std::vector<Place> places;unsigned claimed=0;};
struct State {
    std::array<Level,24> levels;unsigned purchases=0,legacyEggs=0;bool remainder=false;
    unsigned earned()const{unsigned n=0;for(auto& l:levels)n+=std::popcount(l.claimed);return n;}
    unsigned spent()const{unsigned n=0;for(unsigned i=0;i<4;i++)if(purchases&(1u<<i))n+=prices[i];return n;}
    unsigned balance()const{return earned()-spent();}
    bool owns(Item item)const{return purchases&(1u<<unsigned(item));}
    bool valid()const;
    bool claim(unsigned level,unsigned slot);
    bool buy(Item item,bool hybridReady=false);
};
struct View {State state;bool readable=true,saved=true;};
void initialize(const std::filesystem::path& path="pc-collection.json");
void start_session(unsigned mode);
View view();
bool purchase(Item item);
bool editor_unlocked();
bool egg_allowed(uint8_t* rdram,unsigned color);
std::optional<bool> egg_bubble_hidden(uint8_t* rdram,uint32_t actor);
void enter(uint8_t* rdram,unsigned room,uint32_t metadata,bool allowed);
void created(uint8_t* rdram,recomp_context* ctx);
void draw(uint8_t* rdram,uint32_t actor);
void collect(uint8_t* rdram,uint32_t actor);
constexpr uint16_t turquoise(uint16_t c){return uint16_t((c&0x3E)<<10|(c&0xF800)>>5|(c&0x7C0)>>5|(c&1));}
constexpr uint16_t collected_white(uint16_t c){
    const unsigned r=(c>>11)&31,g=(c>>6)&31,b=(c>>1)&31;
    const unsigned v=r>g?(r>b?r:b):(g>b?g:b);
    return uint16_t((v<<11)|(v<<6)|(v<<1)|(c&1));
}
}
