#pragma once
#include "boss_monster_aim0260.hpp"
#include <algorithm>
#include <cmath>
#include <optional>
namespace workshop::boss_lips_corridor0260 {
struct Result {bool clear=false;unsigned pixels_tested=0;float first_solid_x=0,first_solid_y=0;};
// Conservative continuous sweep of an axis-aligned native egg box through the
// actual pixel bimap. A solid pixel occupies its full unit square. This is
// diagnostic input planning, never a native collision/damage replacement.
inline bool intersects(float x,float y,float tx,float ty,float left,float top,float right,float bottom){
 float first=0,last=1;const float dx=tx-x,dy=ty-y;
 auto slab=[&](float position,float delta,float low,float high){if(delta==0)return position>=low&&position<=high;float a=(low-position)/delta,b=(high-position)/delta;if(a>b)std::swap(a,b);first=std::max(first,a);last=std::min(last,b);return first<=last;};
 return slab(x,dx,left,right)&&slab(y,dy,top,bottom);
}
inline std::optional<Result> observe(uint8_t* rdram,const boss_monster_aim0260::Map& map,float x,float y,float tx,float ty,float half_x,float half_y){
 if(!rdram||map.columns!=2||map.rows!=1||!boss_driver0260::point(x,y)||!boss_driver0260::point(tx,ty)||!std::isfinite(half_x)||!std::isfinite(half_y)||half_x<=0||half_y<=0||half_x>64||half_y>64||std::hypot(tx-x,ty-y)>1024)return {};
 const float lower_x=std::min(x,tx)-half_x,upper_x=std::max(x,tx)+half_x,lower_y=std::min(y,ty)-half_y,upper_y=std::max(y,ty)+half_y;
 if(lower_x<0||lower_y<0||upper_x>=512||upper_y>=256)return {};
 const int left=std::max(0,int(std::floor(lower_x))-1),right=int(std::floor(upper_x)),top=std::max(0,int(std::floor(lower_y))-1),bottom=int(std::floor(upper_y));
 const unsigned cells=unsigned(right-left+1)*unsigned(bottom-top+1);if(!cells||cells>512u*256u)return {};
 Result result;result.clear=true;
 for(int py=top;py<=bottom;++py)for(int px=left;px<=right;++px){
  if(!intersects(x,y,tx,ty,float(px)-half_x,float(py)-half_y,float(px+1)+half_x,float(py+1)+half_y))continue;
  const auto solid=map.occupied(rdram,float(px),float(py));if(!solid)return {};++result.pixels_tested;
  if(*solid){result.clear=false;result.first_solid_x=float(px);result.first_solid_y=float(py);return result;}
 }
 return result;
}
}
