#pragma once
#include <cmath>
#include <cstdint>
namespace workshop::boss_ground_roundtrip0260 {
struct Frame {bool enabled=false,diagnostic=false,driver=false,victory=false;unsigned index=0,vi=0;float x=0,y=0;};
// Disposable QA controller only. Never receives RDRAM, actors or dispatchers.
// Exercise real camera streaming: walk to the far side, return to the exit,
// stand still, then let the ordinary native-pot input driver continue.
class Walk {
 unsigned stage_=0,since_=0,settled_=0,last_=0;bool failed_=false;
public:
 unsigned stage()const{return stage_;}bool done()const{return stage_==3;}bool failed()const{return failed_;}
 bool apply(const Frame& f,uint16_t* buttons,float* x,float* y){
  if(!buttons||!x||!y||!f.enabled||!f.diagnostic||!f.driver||!f.victory||f.index||!f.vi||!std::isfinite(f.x)||!std::isfinite(f.y)||f.x<0||f.x>768||f.y<0||f.y>512||failed_||done())return false;
  if(last_&&f.vi<last_)return false;
  last_=f.vi;if(!since_)since_=f.vi;
  *buttons=0;*x=*y=0;
  if(f.vi-since_>3600){failed_=true;return true;}
  if(f.vi-since_<30)return true;
  if(stage_==2){if(f.vi-settled_>=60)stage_=3;return true;}
  const float target=stage_==0?700.0f:160.0f,d=target-f.x;
  if(std::abs(d)>3){*x=std::copysign(std::abs(d)>16?.65f:.5f,d);settled_=0;return true;}
  if(!settled_)settled_=f.vi;
  if(f.vi-settled_<60)return true;
  ++stage_;settled_=0;if(stage_==2)settled_=f.vi;
  return true;
 }
};
}
