#pragma once
#include "boss_fish_exit0260.hpp"
#include <string>
#include <string_view>
namespace workshop::boss_fish_water_recovery0260 {
namespace n=boss_native;namespace m=boss_monster_aim0260;namespace d=boss_driver0260;
enum class Stage:unsigned {Idle,Column,Ascend,Return,Settle,Blocked};
struct Frame {bool admitted=false;unsigned source=0,epoch=0;std::string_view token;float anchor_y=0;};
struct Body {float left=0,right=0,top=0,bottom=0;};
// Definition30/34 are native world terrain probes; the object A/C extents
// instead use definition70 scales. Observe both, never resize the native body.
inline std::optional<Body> body(uint8_t* ram,uint32_t player){
 auto* rdram=ram;
 const auto meta=n::word(ram,player+0x174);if(!m::pointer(meta,8)||n::word(ram,meta+4))return {};
 const auto def=n::word(ram,meta);if(!m::pointer(def,0x74)||n::word(ram,def+0x2C)!=12)return {};
 const auto xs=n::word(ram,def+0x30),ys=n::word(ram,def+0x34),scale=n::word(ram,def+0x70);
 if(!m::pointer(xs,48)||!m::pointer(ys,48)||!m::pointer(scale,12))return {};
 const float sx=m::scalar(ram,scale),sy=m::scalar(ram,scale+4);
 if(!std::isfinite(sx)||!std::isfinite(sy)||sx<=0||sy<=0||sx>4||sy>4||MEM_H(4,n::base(def))!=0)return {};
 const float cy=float(MEM_H(6,n::base(def)))*sy,hx=float(MEM_HU(0xA,n::base(def)))*sx,hy=float(MEM_HU(0xC,n::base(def)))*sy;
 Body b{-hx,hx,cy-hy,cy+hy};
 for(unsigned i=0;i<12;++i){const float x=m::scalar(ram,xs+4*i),y=m::scalar(ram,ys+4*i);if(!d::point(x,y)||std::abs(x)>64||std::abs(y)>64)return {};b.left=std::min(b.left,x);b.right=std::max(b.right,x);b.top=std::min(b.top,y);b.bottom=std::max(b.bottom,y);}
 if(b.left>=0||b.right<=0||b.top>=0||b.bottom<=2||b.left< -64||b.right>64||b.top< -64||b.bottom>64)return {};return b;
}
inline bool clear(uint8_t* ram,const m::Map& map,const Body& b,float x,float y,float tx,float ty,bool floor_contact=true){
 // Reuse the victory-exit's conservative observer without changing that pilot.
 if(!boss_fish_exit0260::body_clear(ram,map,x,y,tx,ty))return false;
 const float bottom=b.bottom-(floor_contact?2.f:0.f);
 const unsigned steps=std::max(1u,unsigned(std::ceil(std::hypot(tx-x,ty-y)/2)));
 for(unsigned i=0;i<=steps;++i){const float t=float(i)/steps;
  const unsigned nx=std::max(1u,unsigned(std::ceil((b.right-b.left)/4))),ny=std::max(1u,unsigned(std::ceil((bottom-b.top)/4)));
  for(unsigned ix=0;ix<=nx;++ix)for(unsigned iy=0;iy<=ny;++iy){const auto solid=map.occupied(ram,x+(tx-x)*t+b.left+(b.right-b.left)*float(ix)/nx,y+(ty-y)*t+b.top+(bottom-b.top)*float(iy)/ny);if(!solid||*solid)return false;}
 }return true;
}
struct Output {bool handled=false,completed=false;Stage stage=Stage::Idle;uint16_t buttons=0;float x=0,y=0,lane=0,rise_y=0;};
class Controller {
 uint32_t player_=0,private_=0;unsigned room_=177,index_=10,epoch_=0,start_=0,last_=0,settled_=0,settled_vi_=0;std::string token_;float anchor_=0,lane_=0,rise_=0;Stage stage_=Stage::Idle;
public:
 void reset(){*this={};} Stage stage()const{return stage_;}
 Output step(uint8_t* ram,const Frame& f,const d::Input& in){
  const bool scope=ram&&f.admitted&&f.source==97&&!f.token.empty()&&in.fish_navigation&&!in.qa_bird&&in.combat&&in.controls&&in.alive&&in.encounter<9&&in.room==167+in.encounter&&in.fish_retry_epoch==f.epoch;
  if(!scope){reset();return {};}
  if(!n::actor(ram,in.player,0x4001)||n::word(ram,0x800F8828)!=in.player){reset();return {};}
  const auto p=n::word(ram,in.player+0x1F4);
  if(player_!=in.player||private_!=p||room_!=in.room||index_!=in.encounter||epoch_!=f.epoch||token_!=f.token||in.vi<last_){reset();player_=in.player;private_=p;room_=in.room;index_=in.encounter;epoch_=f.epoch;token_=std::string(f.token);}
  last_=in.vi;
  // Native damage, attachments, held egg/reticle and mouth gestures win. They
  // are not interrupted, neutralised, or classified as completed navigation.
  if(!m::pointer(p,0x12F0)||n::byte(ram,in.player+0x17D)!=2||n::byte(ram,in.player+0x165)||n::word(ram,in.player+0x180)||n::byte(ram,0x800F7EA5)||n::byte(ram,0x800F7EA6)!=1||(in.primary_state!=1&&in.primary_state!=3&&in.primary_state!=4)||in.aiming_actor||in.reticle_actor||in.mouth_full||!in.native_tongue_known||!in.native_tongue_observed||in.native_tongue_live||in.native_tongue_callback_pending||n::word(ram,p+0x164)||n::word(ram,p+0xC0)||n::word(ram,p+0x1F4)){reset();return {};}
  const auto sub=n::byte(ram,in.player+0x30),callback=n::word(ram,in.player+0x20);
  const bool water=in.primary_state==1&&(sub==77||sub==78)&&(callback==0x80043450||callback==0x800434EC);
  if(stage_==Stage::Idle&&!water)return {};
  Output out;out.handled=true;out.stage=Stage::Blocked;
  const auto map=m::Map::read(ram);const auto shape=body(ram,in.player);
  const auto support=boss_fish_support0260::observe(ram,in.player,in.room,in.encounter,in.stable_stands,in.fish_island_observed?std::optional<float>(in.fish_anchor_x):std::nullopt);
  if(!map||!shape||!support||!in.fish_island_observed||!std::isfinite(f.anchor_y)||!in.horizontal_speed_observed||!in.native_stick_observed)return out;
  const float x=in.x,y=in.y,vx=in.horizontal_speed,anchor=support->x;
  // The prior actual standing observation plus native foot probes locate the
  // surface. A current bitmap transition, not a hardcoded ascent height,
  // determines the room's platform top and the complete return corridor.
  float surface=0;for(int dy=-2;dy<=2;++dy){const float at=f.anchor_y+shape->bottom+float(dy);const auto top=map->tile(ram,anchor,at),above=map->tile(ram,anchor,at-1);if(top&&*top==0x5F01&&above&&!*above){surface=at;break;}}
  if(!surface||!clear(ram,*map,*shape,anchor,f.anchor_y,anchor,f.anchor_y))return out;
  if(stage_==Stage::Idle){
   anchor_=anchor;
   const float distance=std::ceil((24+std::max(-shape->left,shape->right)+16)/8)*8;
   bool found=false;for(float sign:{x>=anchor?1.f:-1.f,x>=anchor?-1.f:1.f}){const float lane=anchor+sign*distance;
    if(lane+shape->left<in.min_x||lane+shape->right>in.max_x)continue;
    // Find the first real crossing corridor above the observed floor. Full
    // foot probes are required in transit; floor-contact slack is only for
    // the later native landing, never authority to pass through a wall.
    if(!clear(ram,*map,*shape,x,y,lane,y))continue;
    for(unsigned rise=0;rise<=64;++rise){const float at=f.anchor_y-float(rise);
     if(clear(ram,*map,*shape,lane,y,lane,at)&&clear(ram,*map,*shape,lane,at,anchor,at,false)&&clear(ram,*map,*shape,anchor,at,anchor,f.anchor_y)){lane_=lane;rise_=at;found=true;break;}}
    if(found)break;}
   if(!found)return out;start_=in.vi;stage_=Stage::Column;
  }
  out.lane=lane_;out.rise_y=rise_;
  if(anchor_!=anchor||in.vi<start_||in.vi-start_>1200){stage_=Stage::Blocked;return out;}
  // A current native landing owns the whole braking/recentring phase,
  // including drift beyond the final8px aiming window. Never re-issue A on
  // that actual support merely because inertia carried P1 outside8px.
  if(support->on_island){
   stage_=Stage::Settle;
   if(std::abs(x-anchor)<=8&&std::abs(vx)<=.25f){
    if(in.vi!=settled_vi_){settled_=settled_&&in.vi==settled_vi_+1?settled_+1:1;settled_vi_=in.vi;}
    if(settled_>=2){reset();out.completed=true;out.stage=Stage::Settle;return out;}
   }else {settled_=0;
    const bool braking=std::abs(vx)>.25f;const float direction=braking?(vx>0?-1.f:1.f):(x<anchor?1.f:-1.f);
    if(const auto axes=boss_stick_aim0260::toward(direction,0,in.native_stick,braking?54:4)){out.x=axes->host.x;out.y=axes->host.y;}
   }
   out.stage=Stage::Settle;return out;
  }
  settled_=0;if(water&&stage_==Stage::Return&&y>f.anchor_y+1)stage_=Stage::Column;
  // Keep A throughout native aerial14/19:3431C admits its own flutter only
  // after C8>2.7 and F7=1. A fixed45/15 duty cycle can release just there.
  const uint16_t stroke=!water||((in.vi-start_)%60<45)?d::jump_button:0;
  if(stage_==Stage::Ascend&&!clear(ram,*map,*shape,x,y,lane_,rise_)&&clear(ram,*map,*shape,x,y,lane_,y))stage_=Stage::Column;
  if(stage_==Stage::Column){if(!clear(ram,*map,*shape,x,y,lane_,y))return out;
   if(std::abs(x-lane_)>2||!clear(ram,*map,*shape,x,y,x,rise_)){out.x=x<lane_?.55f:-.55f;out.buttons=water?0:stroke;out.stage=stage_;return out;}stage_=Stage::Ascend;}
  if(stage_==Stage::Ascend){
   if(y<=f.anchor_y&&clear(ram,*map,*shape,x,y,anchor,y,false))stage_=Stage::Return;
   else {if(!clear(ram,*map,*shape,x,y,lane_,rise_))return out;out.x=std::abs(x-lane_)>2?(x<lane_?.55f:-.55f):0;out.y=1;out.buttons=stroke;out.stage=stage_;return out;}
  }
  if(stage_==Stage::Return||stage_==Stage::Settle){
   // Landing slack is licensed by current native support, never by the
   // previous airborne corridor. If it closes, keep ordinary aerial A for
   // native flutter but do not continue driving feet into the platform side.
   if(!clear(ram,*map,*shape,x,y,anchor,y,support->on_island)){if(!water)out.buttons=stroke;return out;}
   if(std::abs(x-anchor)>6)out.x=x<anchor?1.f:-1.f;
   else if(std::abs(vx)>.25f){if(const auto counter=boss_stick_aim0260::toward(vx>0?-1.f:1.f,0,in.native_stick,54)){out.x=counter->host.x;out.y=counter->host.y;}}
   out.buttons=stroke;
  }
  out.stage=stage_;return out;
 }
};
} // namespace workshop::boss_fish_water_recovery0260
