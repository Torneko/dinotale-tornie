#pragma once
#include "recomp.h"
#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>

namespace workshop::achievements {
constexpr unsigned fruit_mask=0xFDF; // 0..4 classics, 6..11 hybrids; no smiling heart
struct Entry {unsigned completions=0,modes=0,bestHealth=0,sameFruit=0;bool fullHealth=false;unsigned bestHearts=0;bool allHearts=false;};
struct View {std::array<Entry,24> levels{};bool readable=true,saved=true;};
struct Sample {
    bool playable=false;unsigned level=24,mode=2,fruits=0,ending=0,health=0,hearts=0;
    std::array<unsigned,30> ring{};
};
struct Finish {unsigned level,mode,health,sameFruit,hearts;};
class Tracker {
    unsigned level=24,mode=2;bool armed=false,finished=false;
public:
    void reset(){level=24;mode=2;armed=finished=false;}
    std::optional<Finish> observe(const Sample& s){
        if(!s.playable || s.level>=24 || s.mode>1)return {};
        if(level!=s.level || mode!=s.mode){reset();level=s.level;mode=s.mode;}
        if(s.fruits==0 && s.ending==0){armed=true;finished=false;}
        if(!armed || finished || s.fruits!=30 || s.ending!=1 || s.health>8)return {};
        unsigned mask=0;
        for(auto fruit:s.ring){if(fruit>=12 || !(fruit_mask&(1u<<fruit)))return {};mask|=1u<<fruit;}
        finished=true;
        return Finish{s.level,s.mode,s.health,(mask && !(mask&(mask-1)))?mask:0,s.hearts>3?3:s.hearts};
    }
};
void initialize(const std::filesystem::path& path="pc-achievements.json");
void start_session(unsigned mode);
void frame(uint8_t* rdram,recomp_context* ctx,bool allowed);
View view();
}
