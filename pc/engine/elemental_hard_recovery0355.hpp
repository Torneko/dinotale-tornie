#pragma once
#include "story_hard0330.hpp"
#include <algorithm>
#include <array>
#include <optional>
namespace workshop::elemental_hard_recovery0355 {
// The Hard journal validated these exact durable carry-and-finish receipts.
// Mere reservation/selection, eligibility, preview or ownership never suffices.
inline std::optional<unsigned> durable_mask(const story_hard0330::View& v,
 const story_hard_access0330::Evidence& gate,bool genuine){
 if(!genuine||!v.initialized||!v.present||!v.active||!v.readable||!v.saved||
    v.scope!=story_hard0330::Scope::Production||!story_hard_access0330::entitled(gate))return {};
 unsigned mask=0;
 for(unsigned i=0;i<2;++i){const unsigned level=i?11:5,color=16+i,bit=1u<<color;
  if(!(v.run.claimed_levels&(1u<<level))||v.run.egg_choice[level]!=color)continue;
  if(!v.run.serial||v.run.token.size()!=32||!std::all_of(v.run.token.begin(),v.run.token.end(),[](char c){return (c>='0'&&c<='9')||(c>='a'&&c<='f');})||v.run.phase==story_hard0330::Phase::Empty||v.run.phase>story_hard0330::Phase::Abandoned||
     !(v.run.found&bit)||!(v.run.owned&bit))return {};
  mask|=1u<<i;
 }
 return mask;
}
template<class Acquire>bool reconcile(const story_hard0330::View& v,
 const story_hard_access0330::Evidence& gate,bool genuine,Acquire&& acquire){
 const auto bits=durable_mask(v,gate,genuine);if(!bits)return false;
 for(unsigned i=0;i<2;++i)if((*bits&(1u<<i))&&!acquire(16+i,i?11u:5u))return false;
 return true;
}
}
