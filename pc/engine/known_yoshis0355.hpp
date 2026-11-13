#pragma once
#include "story_rights0320.hpp"
#include "hybrid_finds.hpp"
#include "metal_yoshis0300.hpp"
#include "elemental_yoshis0355.hpp"
#include "yoshi_colors0300.hpp"
#include <cstdint>
#include <string>
#include <string_view>
namespace workshop::known_yoshis0355 {
inline constexpr uint32_t ordinary_mask=0x3Fu,all_mask=0x3FFFFu;
inline constexpr unsigned unknown_color=18;
// Presentation only. Known permanently is independent of alive/available now.
// Never use this snapshot to grant selection, eggs, achievements or ownership.
struct Snapshot {
 uint32_t knownMask18=ordinary_mask,trustedMask18=ordinary_mask;
 bool known(unsigned color)const{return color<18&&(knownMask18&(1u<<color));}
 unsigned display_color(unsigned color)const{return known(color)?color:unknown_color;}
 std::string_view label(unsigned color,std::string_view name)const{return known(color)?name:std::string_view("???");}
 bool operator==(const Snapshot&)const=default;
};
inline Snapshot project(const story_rights0320::View& canonical,const finds::View& hybrids,
 const metal_yoshis0300::View& metals,const elemental_yoshis0355::View& elementals){
 Snapshot out;
 // The caller must pass the validated CANONICAL EEPROM, including in Hard or
 // Hybrid Story. A local run roster, raw life flags or a Trial loss is not proof.
 if(canonical.present&&canonical.readable&&canonical.story<=0xFFFFFFu
   &&canonical.trial==canonical.progression.trial&&canonical.bw_earned<=3u
   &&canonical.bw_earned==canonical.progression.eggs
   &&progress::valid(canonical.progression,canonical.story)){
  out.knownMask18|=canonical.bw_earned<<6;out.trustedMask18|=0xC0u;
 }
 // Legacy unlock eligibility is deliberately not a discovery fallback.
 if(hybrids.initialized&&hybrids.readable&&hybrids.saved&&hybrids.migrated&&hybrids.found<=63u){
  out.knownMask18|=hybrids.found<<8;out.trustedMask18|=0x3F00u;
 }
 if(metals.initialized&&metals.readable&&metals.saved&&metals.found<=3u){
  out.knownMask18|=metals.found<<14;out.trustedMask18|=0xC000u;
 }
 if(elementals.initialized&&elementals.readable&&elementals.saved&&elementals.found<=3u){
  out.knownMask18|=elementals.found<<16;out.trustedMask18|=0x30000u;
 }
 return out;
}
// Optional host-only cache. The explicit canonical profile key prevents a menu
// from reusing another profile's identities. Caller supplies synchronization if
// sharing this object across threads; no file, service or game state is written.
class Cache {
 std::string profile_;Snapshot state_;
public:
 void clear(){profile_.clear();state_={};}
 void refresh(std::string_view profile,const story_rights0320::View& canonical,
  const finds::View& hybrids,const metal_yoshis0300::View& metals,const elemental_yoshis0355::View& elementals){
  if(profile.empty()){clear();return;}profile_=profile;state_=project(canonical,hybrids,metals,elementals);
 }
 Snapshot snapshot(std::string_view profile)const{return !profile.empty()&&profile==profile_?state_:Snapshot{};}
 bool known(std::string_view profile,unsigned color)const{return snapshot(profile).known(color);}
};
} // namespace workshop::known_yoshis0355
