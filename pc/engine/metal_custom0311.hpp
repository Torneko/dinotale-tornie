#pragma once
#include "metal_yoshis0300.hpp"
#include "recomp.h"
// Read-only projection for isolated custom profiles. This deliberately never
// enables the canonical search scope or writes the permanent metal journal.
namespace workshop::metal_custom0311 {
constexpr bool scope(bool payload,bool custom_session,bool tally,bool deferred){
 return (payload||custom_session)&&!tally&&!deferred;
}
// Diagnostic scripts retain their historical timing. A real custom
// continuation reacts as soon as the native Yoshi-selection context exists.
constexpr bool selection_ready(unsigned frame,int diagnostic,int preferred){
 return diagnostic>=0?frame>=1800:preferred>=0;
}
constexpr bool selection_context(unsigned stage,unsigned scene){return stage==6&&scene==5;}
constexpr uint16_t pending_buttons(uint16_t buttons){return buttons&uint16_t(~0x9000u);} // Keep B/back.
inline unsigned owned_extra(const metal_yoshis0300::View& view){
 return view.initialized&&view.readable&&view.found<=3?yoshi_colors0300::extra_from_metals(view.found):0;
}
inline unsigned saved_lives(unsigned legacy,const metal_yoshis0300::View& view){return (legacy&0x3F)|owned_extra(view);}
inline bool death(uint8_t* rdram,unsigned color){
 if(!yoshi_colors0300::metal(color))return false;
 MEM_W(0,int32_t(0x807F1000))=uint32_t(MEM_W(0,int32_t(0x807F1000)))&~(1u<<(color-8));
 return true;
}
}
