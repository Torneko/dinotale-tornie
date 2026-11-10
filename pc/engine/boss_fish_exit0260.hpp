#pragma once
#include "boss_fish_support0260.hpp"
#include "boss_stick_aim0260.hpp"
namespace workshop::boss_fish_exit0260 {
namespace n=boss_native;namespace m=boss_monster_aim0260;
inline int signed_axis(uint8_t* ram,uint32_t at){return int(int16_t((n::byte(ram,at)<<8)|n::byte(ram,at+1)));}
enum class Stage : unsigned {Idle,Column,Ascend,Return,Settle,Blocked};
struct Output {bool handled=false;Stage stage=Stage::Idle;uint16_t buttons=0;float x=0,y=0;};
// Read-only geometry for ordinary controller travel. The lower body probe is
// two pixels above native floor contact, so a supported body is not rejected
// merely for touching its current floor. No native collision writer is called.
inline bool body_clear(uint8_t* ram,const m::Map& map,float x,float y,float tx,float ty){
 if(!boss_driver0260::point(x,y)||!boss_driver0260::point(tx,ty))return false;
 const float length=std::hypot(tx-x,ty-y);if(length>768)return false;
 const unsigned steps=std::max(1u,unsigned(std::ceil(length/2)));
 for(unsigned i=0;i<=steps;++i){const float t=float(i)/steps;
  for(float dx:{-12.f,0.f,12.f})for(float dy:{-24.f,0.f,14.f}){
   const auto solid=map.occupied(ram,x+(tx-x)*t+dx,y+(ty-y)*t+dy);if(!solid||*solid)return false;
  }
 }return true;
}
class Controller {
 uint32_t player_=0,pot_=0;unsigned room_=177,encounter_=10,epoch_=0,start_=0;float lane_=0;Stage stage_=Stage::Idle;
public:
 void reset(){*this={};}
 Stage stage()const{return stage_;}
 Output step(uint8_t* ram,bool admitted,uint32_t player,uint32_t pot,unsigned room,unsigned encounter,unsigned epoch,unsigned vi,std::span<const float> centers){
  if(!admitted||!ram){reset();return {};}
  if(player_!=player||pot_!=pot||room_!=room||encounter_!=encounter||epoch_!=epoch){reset();player_=player;pot_=pot;room_=room;encounter_=encounter;epoch_=epoch;}
  if(!n::actor(ram,player,0x4001)||n::word(ram,0x800F8828)!=player||!n::actor(ram,pot,0x4273))return {};
  if(n::word(ram,player+0x180)==pot){stage_=Stage::Idle;return {};}
  const auto p=n::word(ram,player+0x1F4);if(!n::range(p,0x12D0))return {};
  const auto x=m::scalar(ram,player+0x84),y=m::scalar(ram,player+0x88),anchor=m::scalar(ram,pot+0x84),pot_y=m::scalar(ram,pot+0x88),vx=m::scalar(ram,player+0xC4);
  const auto support=boss_fish_support0260::observe(ram,player,room,encounter,centers,anchor);const auto map=m::Map::read(ram);
  const auto sub=n::byte(ram,player+0x30),callback=n::word(ram,player+0x20);
  const bool water=n::byte(ram,player+0x18)==1&&(sub==77||sub==78)&&(callback==0x80043450||callback==0x800434EC);
  if(stage_==Stage::Idle&&!water)return {};
  Output out;out.handled=true;
  if(!support||!map||!std::isfinite(vx)||(!std::isfinite(pot_y)||pot_y<224||pot_y>288)||n::byte(ram,0x800F7EA5)||n::byte(ram,0x800F7EA6)!=1||n::word(ram,p+0x164)||n::word(ram,p+0xC0)||n::word(ram,p+0x1F4)){out.stage=Stage::Blocked;return out;}
  if(stage_==Stage::Idle){stage_=Stage::Column;lane_=anchor+(x>=anchor?56.f:-56.f);start_=vi;}
  if(vi<start_||vi-start_>1200){out.stage=Stage::Blocked;return out;}
  if(support->on_island&&std::abs(x-anchor)<=10){
   stage_=Stage::Settle;
   if(std::abs(vx)<=.25f){stage_=Stage::Idle;return {};}
   const boss_stick_aim0260::Axes old{signed_axis(ram,0x800F897E),signed_axis(ram,0x800F8980)};
   if(const auto counter=boss_stick_aim0260::toward(vx>0?-1.f:1.f,0,old,54)){out.x=counter->host.x;out.y=counter->host.y;}
   out.stage=stage_;return out;
  }
  // Native37094/36C54 admits swimming through a real stick vector. Native
  // 36DE4/24188 makes an UP vector negative world C8. A is an ordinary stroke
  // and surface-jump edge; A alone with zero stick cannot leave water78.
  if(water&&stage_==Stage::Return&&y>274)stage_=Stage::Column;
  if(stage_==Stage::Column){
   if(!body_clear(ram,*map,x,y,lane_,y)){out.stage=Stage::Blocked;return out;}
   if(std::abs(x-lane_)>6){out.x=x<lane_?.85f:-.85f;out.stage=stage_;return out;}
   stage_=Stage::Ascend;
  }
  if(stage_==Stage::Ascend){
   if(y<=248)stage_=Stage::Return;
   else{
    if(!body_clear(ram,*map,x,y,lane_,248)){out.stage=Stage::Blocked;return out;}
    out.x=std::abs(x-lane_)>6?(x<lane_?.55f:-.55f):0;out.y=1;out.buttons=(vi-start_)%60<45?0x8000:0;out.stage=stage_;return out;
   }
  }
  if(stage_==Stage::Return||stage_==Stage::Settle){
   if(!body_clear(ram,*map,x,y,anchor,y)){out.stage=Stage::Blocked;return out;}
   if(std::abs(x-anchor)>6)out.x=x<anchor?1.f:-1.f;
   else if(std::abs(vx)>.25f){const boss_stick_aim0260::Axes old{signed_axis(ram,0x800F897E),signed_axis(ram,0x800F8980)};if(const auto counter=boss_stick_aim0260::toward(vx>0?-1.f:1.f,0,old,54)){out.x=counter->host.x;out.y=counter->host.y;}}
   // Native flutter keeps a real landing possible; entry Down is delegated
   // to the existing pot controller only after actual island support settles.
   out.buttons=(vi-start_)%60<45?0x8000:0;
  }
  out.stage=stage_;return out;
 }
};
} // namespace workshop::boss_fish_exit0260
