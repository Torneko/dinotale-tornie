#pragma once
#include "recomp.h"
#include <array>
#include <bit>
#include <cstdint>
namespace workshop::progress {
struct State {uint32_t trial;uint8_t first;std::array<uint8_t,4> columns;uint8_t eggs,lives;};
constexpr uint32_t column(unsigned c){return 0x111111u<<c;}
inline bool valid(const State& p,uint32_t story){
    if(p.trial>0xFFFFFF || p.eggs>3 || p.lives>63 || (p.eggs && p.trial!=0xFFFFFF))return false;
    unsigned used=0;
    if(p.first!=255){if(p.first>=6)return false;used=1u<<p.first;}
    for(unsigned c=0;c<4;c++){
        const auto color=p.columns[c];
        const bool trial=(p.trial&column(c))==column(c);
        const bool earned=((p.trial|story)&column(c))==column(c);
        if(p.first!=255 && earned){
            // An older save may have completed a Story column without a reward.
            // It is accepted and reconciled after the native payload is read.
            if(color==255 && !trial)continue;
            if(color>=6 || (used&(1u<<color)))return false;used|=1u<<color;
        }else if(color!=255)return false;
    }
    return true;
}
inline unsigned used(const State& p){unsigned m=p.first<6?1u<<p.first:0;for(auto c:p.columns)if(c<6)m|=1u<<c;return m;}
inline bool final_ready(const State& p,uint32_t story){return p.first<6 && (p.trial==0xFFFFFF || (story&0xFFFFFF)==0xFFFFFF);}
inline unsigned unlocked(const State& p,uint32_t story){return final_ready(p,story)?63:used(p);}

// The native result snapshot precedes egg promotion and the progression save.
// Preview only their count here: do not draw a color, revive an older loss or
// write progression before the normal successful-level commit.
inline unsigned tally_lives(const State& p,uint32_t story,bool trial,unsigned row,unsigned col,unsigned alive,unsigned flags,unsigned allowed_mask=0x3FFF){
    unsigned mask=alive&(allowed_mask==0xFFFF?0xFFFFu:0x3FFFu);
    const bool accepted=valid(p,story);
    if(!trial || (accepted && p.trial==0xFFFFFF))mask|=(flags>>4)&192;
    unsigned count=std::popcount(mask);
    if(!accepted || row>=6 || col>=4)return count;
    const unsigned known=used(p);
    const uint32_t bit=1u<<(row*4+col);
    const auto stages=p.trial|(trial?bit:0);
    const auto access=story|(trial?0:bit);
    const bool first=p.first<6 || (!trial && row==5);
    if(!first)return count;
    unsigned added=p.first==255?1:0;
    for(unsigned c=0;c<4;c++)if(p.columns[c]==255 && ((stages|access)&column(c))==column(c))added++;
    if(!added)return count;
    if(stages==0xFFFFFF || access==0xFFFFFF)added=6-std::popcount(known);
    // Full Story access can expose the remaining color before reconciliation.
    // Any such lives already present in the base mask must count only once.
    const unsigned covered=std::popcount((mask>>8)&63&~known);
    return count+(added>covered?added-covered:0);
}
template<class Choose> unsigned reconcile(State& p,uint32_t story,Choose&& choose){
    if(!valid(p,story) || p.first==255)return 0;
    unsigned added=0;
    for(unsigned c=0;c<4;c++)if(((p.trial|story)&column(c))==column(c) && p.columns[c]==255){
        std::array<unsigned,6> choices{};unsigned count=0,mask=used(p);
        for(unsigned i=0;i<6;i++)if(!(mask&(1u<<i)))choices[count++]=i;
        if(!count)break;
        const auto color=choices[choose(count)];p.columns[c]=uint8_t(color);added|=1u<<color;
    }
    // A newly completed final column also gives the remaining sixth color
    // its first life. Reloading cannot revive a color lost later in Story.
    if(added && final_ready(p,story))added|=63u&~used(p);
    p.lives|=added;return added;
}
inline State read(uint8_t* rdram,gpr at){
    State p{};p.trial=MEM_W(0,at);p.first=MEM_BU(4,at);
    for(unsigned c=0;c<4;c++)p.columns[c]=MEM_BU(5+c,at);
    p.eggs=MEM_BU(9,at);p.lives=MEM_BU(10,at);return p;
}
inline void write(uint8_t* rdram,gpr at,const State& p){
    MEM_W(0,at)=p.trial;MEM_B(4,at)=p.first;
    for(unsigned c=0;c<4;c++)MEM_B(5+c,at)=p.columns[c];
    MEM_B(9,at)=p.eggs;MEM_B(10,at)=p.lives;
}
}
