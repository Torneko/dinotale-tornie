#pragma once
#include "metal_yoshis0300.hpp"
#include "recomp.h"

namespace workshop::metal_native0300 {
inline bool story_ready=false;
inline unsigned extras(uint8_t* rdram){return uint32_t(MEM_W(0,int32_t(0x807F1000)));}
inline void extras(uint8_t* rdram,unsigned value){MEM_W(0,int32_t(0x807F1000))=value;}
inline bool story(uint8_t* rdram){return MEM_W(0,int32_t(0x800F8B88))==0;}
inline bool continuing(uint8_t* rdram){return (MEM_HU(0,int32_t(0x800F9254))&7)&&(MEM_BU(0,int32_t(0x800F9253))&8);}
inline unsigned saved_lives(uint8_t* rdram,unsigned legacy){
 const unsigned metal=story(rdram)?metal_yoshis0300::story_extra(continuing(rdram)):metal_yoshis0300::unlocked_extra();
 if(story(rdram)){
  story_ready=true;
  if(!continuing(rdram))metal_yoshis0300::save_story_alive((metal>>6)&3);
 }
 return (legacy&0x3F)|(metal&0xC0);
}
// Entered before the native 1128 hybrid/native split. Preserve its counter,
// cargo identity and subsequent scene/audio callbacks, but never clear the
// WHITE/BLACK life bit for a Gold/Silver alias.
inline bool death(uint8_t* rdram,unsigned color){
 if(!yoshi_colors0300::metal(color))return false;
 const unsigned live=extras(rdram)&~(1u<<(color-8));extras(rdram,live);
 if(story(rdram)){story_ready=true;metal_yoshis0300::save_story_alive((live>>6)&3);}
 return true;
}
inline void save_story(uint8_t* rdram){
 if(story_ready&&story(rdram))metal_yoshis0300::save_story_alive((extras(rdram)>>6)&3);
}
// Native rescue still owns the White Shy Guy consumption/counters/animations.
// Mirror only the new color's life in its separate journal.
inline void rescued(unsigned color){if(yoshi_colors0300::metal(color))metal_yoshis0300::restore_life(color);}
} // namespace workshop::metal_native0300
