#pragma once
#include <cmath>
#include <cstdint>
#include <span>
namespace workshop::boss_followup_input0260 {
struct Frame {bool enabled=false,diagnostic=false,driver=false;unsigned index=0;bool fighting=false,victory=false;unsigned vi=0;};
struct Event {unsigned first=0,last=0;uint16_t buttons=0;float x=0,y=0;};
inline constexpr unsigned max_relative_end=429;
inline bool valid_events(std::span<const Event> events){
 if(events.empty()||events.size()>512)return false;unsigned previous=0;
 for(const auto& e:events){if(e.first<previous||e.first>=e.last||e.last>max_relative_end||!std::isfinite(e.x)||!std::isfinite(e.y)||std::abs(e.x)>1||std::abs(e.y)>1)return false;previous=e.last;}
 return true;
}
// Host-only QA input handoff. No guest pointers, actors, gameplay or native calls.
// Call reset for a new isolated campaign. The preceding native Candy/rest input
// driver remains in charge until this exact second encounter naturally starts.
class Anchor {
 bool anchored_=false,finished_=false;unsigned anchor_=0,last_=0;
public:
 void reset(){anchored_=finished_=false;anchor_=last_=0;}
 bool anchored()const{return anchored_;}
 unsigned anchor_vi()const{return anchor_;}
 bool apply(const Frame& f,std::span<const Event> events,uint16_t* buttons,float* x,float* y){
  if(!buttons||!x||!y||!f.enabled||!f.diagnostic||!f.driver||f.index!=1||f.fighting==f.victory||!valid_events(events))return false;
  if(anchored_&&f.vi<last_)return false;
  if(!anchored_){if(!f.fighting)return false;anchored_=true;anchor_=f.vi;}
  last_=f.vi;uint16_t next_buttons=0;float next_x=0,next_y=0;
  if(f.victory)finished_=true;
  if(!finished_){const unsigned relative=f.vi-anchor_;if(relative>=events.back().last)return false;for(const auto& e:events)if(relative>=e.first&&relative<e.last){next_buttons=e.buttons;next_x=e.x;next_y=e.y;break;}}
  *buttons=next_buttons;*x=next_x;*y=next_y;return true;
 }
};
}
