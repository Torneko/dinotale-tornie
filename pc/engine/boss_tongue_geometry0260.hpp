#pragma once
// Diagnostic arithmetic only, reproduced from native3D54C/48F84/3D0B0/3CE84.
// No guest-memory access, food/collision override or gameplay time limit.
#include "boss_stick_aim0260.hpp"
#include <limits>
namespace workshop::boss_tongue_geometry0260 {
namespace stick=boss_stick_aim0260;
inline std::optional<float> max_centre_reach(float owner_scale,float body_base,bool extended){
 if(!std::isfinite(owner_scale)||owner_scale<=0||owner_scale>2||!std::isfinite(body_base)||body_base<0||body_base>2)return {};
 float velocity=owner_scale*1.9f;const float target=-velocity,deceleration=owner_scale*(extended?.10f:.15f);
 if(!std::isfinite(velocity)||!std::isfinite(deceleration)||deceleration<=0)return {};
 float scale=owner_scale*body_base,peak=scale;
 // This derived bound evaluates only the native outgoing velocity portion.
 const unsigned updates=unsigned(std::ceil(velocity/deceleration))+2;
 for(unsigned i=0;i<updates;++i){
  if(velocity>target)velocity=std::max(target,velocity-deceleration);
  else if(velocity<target)velocity=std::min(target,velocity+deceleration);
  scale+=velocity*.5f;peak=std::max(peak,scale);if(velocity<0)break;
 }
 const float result=peak*16.f;if(!std::isfinite(result)||result<=0)return {};return result;
}
inline std::optional<stick::Result> hold_heading_world(float relative_heading_degrees,bool right,stick::Axes old,float magnitude=32){
 if(!std::isfinite(relative_heading_degrees)||relative_heading_degrees< -90||relative_heading_degrees>90)return {};
 const float radians=relative_heading_degrees*stick::pi/180.f;
 return stick::toward(std::cos(radians)*(right?1.f:-1.f),std::sin(radians),old,magnitude);
}
}
