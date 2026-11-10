#pragma once
#include "boss_lips_aim0260.hpp"
#include "boss_driver0260.hpp"
#include <array>

namespace workshop::boss_lips_motion0260 {
namespace l=boss_lips_aim0260;namespace d=boss_driver0260;
struct Identity {
 uint32_t root=0,private_data=0;uint64_t birth=0;unsigned room=177,encounter=10,retry_epoch=0;
 bool operator==(const Identity&)const=default;
};
struct Motion {bool observed=false;float vx=0,vy=0;unsigned samples=0,vi=0;bool body_observed=false;float body_vx=0;};
// Samples are taken only at the actual parent's800483B8 entry. Each new
// sample is ONE native movement update, not an assumed number of VI.4002's
// native8003A5EC likewise adds its6.6-speed vector once per native update.
class History {
 struct Sample {Identity id{};l::WeakPoint weak{};unsigned vi=0,count=0;float root_y=0;std::array<float,5> x{},root_ys{},root_xs{};bool body=false;Motion motion{};};
 std::array<Sample,4> samples_{};unsigned cursor_=0;
 static bool graph(const l::WeakPoint& a,const l::WeakPoint& b){return a.actor==b.actor&&a.metadata==b.metadata&&a.definition==b.definition&&a.resource==b.resource;}
 static bool valid(const Identity& id,const l::WeakPoint& weak){return id.birth&&l::pointer(id.root,0x1FC)&&l::pointer(id.private_data,0x34)&&id.encounter<10&&id.room==(id.encounter?167+id.encounter:162)&&l::pointer(weak.actor,0x1FC)&&l::pointer(weak.metadata,4)&&l::pointer(weak.definition,0x2C)&&l::pointer(weak.resource,0x44)&&d::point(weak.x,weak.y);}
public:
 void reset(){samples_={};cursor_=0;}
 void native_frame(const Identity& id,const l::WeakPoint& weak,float root_world_y,unsigned vi,float root_world_x=std::numeric_limits<float>::quiet_NaN()){
  if(!valid(id,weak)||!std::isfinite(root_world_y)||std::abs(root_world_y)>=100000||std::abs(weak.y-root_world_y)>256)return;Sample* found=nullptr;for(auto& s:samples_)if(s.id.root==id.root){found=&s;break;}if(!found)found=&samples_[cursor_++%samples_.size()];
  auto& s=*found;const bool same=s.id==id&&graph(s.weak,weak)&&s.count;
  if(same&&vi==s.vi)return; // Duplicate observations cannot fabricate speed.
  const float dx=weak.x-s.weak.x,dy=root_world_y-s.root_y;
  const bool body=std::isfinite(root_world_x)&&std::abs(root_world_x)<100000;
  if(!same||vi<s.vi||vi-s.vi>12||!std::isfinite(dx)||!std::isfinite(dy)||std::abs(dx)>8||std::abs(dy)>8||body!=s.body||(body&&std::abs(root_world_x-s.root_xs[s.count-1])>8)){s={};s.id=id;s.x[0]=weak.x;s.root_ys[0]=root_world_y;s.root_xs[0]=root_world_x;s.body=body;s.count=1;}
  else{
   if(s.count==s.x.size()){for(unsigned i=1;i<s.count;++i){s.x[i-1]=s.x[i];s.root_ys[i-1]=s.root_ys[i];s.root_xs[i-1]=s.root_xs[i];}--s.count;}
   s.x[s.count]=weak.x;s.root_ys[s.count]=root_world_y;s.root_xs[s.count]=root_world_x;++s.count;
  }
  s.weak=weak;s.root_y=root_world_y;s.vi=vi;s.motion={false,0,0,s.count,vi};
  if(s.count>=3){
   const float intervals=float(s.count-1);float vx=(s.x[s.count-1]-s.x[0])/intervals,vy=(s.root_ys[s.count-1]-s.root_ys[0])/intervals;
   // Native turns reverse X abruptly; never extrapolate the previous heading
   // after an observed reversal. Only the parent's resolved WORLD Y supplies
   // vertical motion: the attached collider's current animated offset stays
   // in Target.y, but its oscillation is never extrapolated into the future.
   const float latest_x=s.x[s.count-1]-s.x[s.count-2];if(latest_x*vx<0)vx=latest_x;
   if(std::isfinite(vx)&&std::isfinite(vy)&&std::abs(vx)<=4&&std::abs(vy)<=4){s.motion={true,vx,vy,s.count,vi};if(body){float bx=(s.root_xs[s.count-1]-s.root_xs[0])/intervals;const float latest=s.root_xs[s.count-1]-s.root_xs[s.count-2];if(latest*bx<0)bx=latest;if(std::isfinite(bx)&&std::abs(bx)<=4){s.motion.body_observed=true;s.motion.body_vx=bx;}}}
  }
 }
 Motion observe(const Identity& id,const l::WeakPoint& weak,unsigned vi)const{
  if(!valid(id,weak))return {};
  for(const auto& s:samples_)if(s.id==id&&graph(s.weak,weak)&&s.motion.observed&&vi>=s.vi&&vi-s.vi<=8&&std::abs(weak.x-s.weak.x)<=8&&std::abs(weak.y-s.root_y)<=256)return s.motion;
  return {};
 }
};
}
