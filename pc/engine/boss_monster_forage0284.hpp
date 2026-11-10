#pragma once
#include "boss_monster_forage_geometry0284.hpp"
#include "boss_monster_upper0260.hpp"
#include "boss_bird_test0260.hpp"
#include "boss_native_mouth0260.hpp"
namespace workshop::boss_monster_forage0284 {
namespace d=boss_driver0260;namespace m=boss_monster_aim0260;namespace n=boss_native;namespace g=boss_monster_forage_geometry0284;
using Bird=boss_bird_test0260::Bird;
struct Food {Bird binding{};float x=0,y=0;};
inline std::optional<Food> observe_food(uint8_t* ram,const Bird& b){
 auto* rdram=ram;const auto a=b.actor;
 if(!ram||!b.birth||!n::actor(ram,a,0x4526)||n::byte(ram,a+0x17D)!=2||n::byte(ram,a+0x18)!=1||n::word(ram,a+0x180)||n::byte(ram,a+0x165))return {};
 const auto sub=n::byte(ram,a+0x30),p=n::word(ram,a+0x1F4),res=n::word(ram,a+0x168),meta=n::word(ram,a+0x174);
 if((sub!=5&&sub!=6)||!m::pointer(p,12)||!m::pointer(res,0x48)||!m::pointer(meta,4))return {};
 const auto factory=n::word(ram,res+0x40),def=n::word(ram,meta);
 if(!m::pointer(factory,0x4000)||!m::pointer(def,0x75)||MEM_HU(0x3C,n::base(res))!=0x4526||n::word(ram,res+0x44)!=factory+0x1B1C||n::word(ram,def+0x28)!=factory+0x1A14||!(MEM_HU(0x1C,n::base(def))&1)||!n::byte(ram,def+0x74)||n::word(ram,a+0x20)!=factory+(sub==5?0x9BC:0x884))return {};
 if((b.private_data&&b.private_data!=p)||(b.resource&&b.resource!=res)||(b.factory&&b.factory!=factory))return {};
 const float x=m::scalar(ram,a+0x84),y=m::scalar(ram,a+0x88);if(!d::point(x,y))return {};
 Food out{b,x,y};out.binding.private_data=p;out.binding.resource=res;out.binding.factory=factory;return out;
}
enum class Stage:unsigned {Idle,Release,Flight,Tongue,Return,Settle,Blocked,LandElevator,LowRecovery};
struct Output {bool handled=false,completed=false,gained=false;Stage stage=Stage::Idle;uint16_t buttons=0;float x=0,y=0;uint32_t food=0;unsigned stock_before=0;};
class Controller {
 Stage stage_=Stage::Idle;uint32_t player_=0,private_=0;unsigned last_=0,start_=0,stage_vi_=0,stock_=0,settle_=0,settle_vi_=0;std::string token_;Bird target_{};uint16_t previous_=0;unsigned samples_=0;bool have_vi_=false,on_rider_=false;unsigned rider_vi_=0,low_vi_=0;Output cached_{};
 void stage(Stage next,unsigned vi){if(stage_!=next){stage_=next;stage_vi_=vi;}}
 static bool idle_mouth(const d::Input& in){return in.native_tongue_observed&&in.native_tongue_known&&!in.native_tongue_live&&!in.native_tongue_callback_pending&&!in.mouth_full&&(!in.native_tongue_phase||in.native_tongue_cancelled_idle);}
 static void horizontal(const d::Input& in,Output& out,float direction,float magnitude=54){if(auto axes=boss_stick_aim0260::toward(direction,0,in.native_stick,magnitude)){out.x=axes->host.x;out.y=axes->host.y;}}
 static void hold_tongue(const d::Input& in,Output& out){
  out.buttons=in.native_tongue_phase==2?d::tongue_button:0;
  if(in.native_tongue_heading_active&&in.native_mouth.valid())if(auto axes=boss_tongue_geometry0260::hold_heading_world(in.native_tongue_heading_degrees,in.native_mouth.right,in.native_stick)){out.x=axes->host.x;out.y=axes->host.y;}
 }
 std::optional<Food> current_food(uint8_t* ram,std::span<const Bird> birds)const{
  for(const auto& b:birds)if(b.actor==target_.actor&&b.birth==target_.birth){if(auto food=observe_food(ram,b);food&&food->binding.private_data==target_.private_data&&food->binding.resource==target_.resource&&food->binding.factory==target_.factory)return food;}
  return {};
 }
public:
 void reset(){*this={};}Stage current_stage()const{return stage_;}
 Output step(uint8_t* ram,bool admitted,bool refill_debt,std::string_view token,const d::Input& in,std::span<const Bird> birds){
  if(!admitted||!ram||!in.monster_side_navigation||!in.monster_upper_navigation||!in.monster_navigation||in.room!=168||in.encounter!=1||token.empty()||!in.combat||!in.controls||!in.alive||!in.has_supply_stand||in.supply_stand_x!=248||in.supply_stand_y!=310||in.reserves>6){reset();return {};}
  if(!n::actor(ram,in.player,0x4001)||n::word(ram,0x800F8828)!=in.player){reset();return {};}
  const auto p=n::word(ram,in.player+0x1F4);if(!m::pointer(p,0x12F0)){reset();return {};}
  if(player_!=in.player||private_!=p||token_!=token||(have_vi_&&in.vi<last_)){reset();player_=in.player;private_=p;token_=std::string(token);}
  if(in.aiming_actor||in.reticle_actor||!refill_debt){reset();return {};}
  if(have_vi_&&in.vi==last_)return cached_;have_vi_=true;last_=in.vi;
  if(stage_==Stage::Idle&&in.reserves>=6){cached_={};return {};}
  Output out;out.handled=true;out.stock_before=stock_;out.food=target_.actor;
  auto finish=[&](){out.stage=stage_;previous_=out.buttons;if(samples_<256&&(in.vi%60==0||out.completed)){++samples_;std::fprintf(stderr,"[BossMonsterForageQA] vi=%u phase=%u xy=%.5f,%.5f ground=%u upper=%u eggs=%u startEggs=%u food=%08X birth=%llu mouth=%u/%u live=%u ready=%u parent=%08X c4=%.5f output=%04X/%.5f/%.5f completed=%u gained=%u actualInputsOnly=1 nativeObservation=1\n",in.vi,unsigned(stage_),in.x,in.y,unsigned(in.grounded),unsigned(in.monster_upper_supported),in.reserves,stock_,target_.actor,static_cast<unsigned long long>(target_.birth),unsigned(in.mouth_full),in.native_tongue_phase,unsigned(in.native_tongue_live),unsigned(in.native_tongue_ready),n::word(ram,in.player+0x180),in.horizontal_speed,unsigned(out.buttons),out.x,out.y,unsigned(out.completed),unsigned(out.gained));}cached_=out;return out;};
  const auto held=n::word(ram,p+0x164);const bool held_food=n::range(held,0x1FC)&&n::word(ram,held)==0x19971110&&n::byte(ram,held+6)!=3;
  if(in.mouth_full!=held_food){stage(Stage::Blocked,in.vi);return finish();}
  if(!d::point(in.x,in.y)||in.x<224||in.x>336||in.y<64||in.y>480||!in.native_stick_observed||!in.horizontal_speed_observed||!std::isfinite(in.horizontal_speed)||std::abs(in.horizontal_speed)>64||!in.native_tongue_observed||(!in.native_tongue_known&&!held_food)||!n::actor(ram,in.player,0x4001)||n::word(ram,0x800F8828)!=in.player){stage(Stage::Blocked,in.vi);return finish();}
  const auto geometry=g::observe(ram,in.player);const auto lift=g::elevator(ram);
  if(!geometry||!lift){stage(Stage::Blocked,in.vi);return finish();}
  if(stage_==Stage::Blocked){
   if(in.monster_upper_supported&&in.grounded&&idle_mouth(in)){stage(Stage::Idle,in.vi);start_=in.vi;target_={};stock_=in.reserves;settle_=0;return finish();}
   // A recovered geometry witness allows only return recovery after timeout,
   // never a new outbound attempt or a claimed reserve/landing completion.
   stage(Stage::Return,in.vi);start_=in.vi;
  }
  if(stage_==Stage::Idle){
   if(!in.monster_upper_supported||!in.grounded||std::abs(in.x-248)>3||in.horizontal_speed!=0||!idle_mouth(in)){
    stage(Stage::Return,in.vi);start_=in.vi;stock_=in.reserves;
   }else{
    std::optional<Food> best;for(const auto& b:birds)if(auto food=observe_food(ram,b);food&&food->x>=300&&food->x<=392&&food->y>=128&&food->y<=240&&(!best||food->x<best->x))best=food;
    if(!best)return finish();
    // Nominal departure/rising corridors are current native geometry only;
    // no trajectory or landing is inferred from these clear observations.
    if(!g::clear(ram,*geometry,in.x,in.y,lift->x,in.y,true)||!g::clear(ram,*geometry,in.x,in.y-2,lift->x,210,false))return finish();
    target_=best->binding;stock_=in.reserves;start_=in.vi;settle_=0;stage(Stage::Release,in.vi);return finish();
   }
  }
  if(in.vi<start_||in.vi-start_>480){stage(Stage::Blocked,in.vi);return finish();}
  if(geometry->on_elevator){
   stage(Stage::Return,in.vi);settle_=0;
   if(!on_rider_){on_rider_=true;rider_vi_=in.vi;return finish();}
   const unsigned age=(in.vi-rider_vi_)%60;
   if(age>=2&&age<47){out.buttons=d::jump_button;horizontal(in,out,-1,32);}
   return finish();
  }
  on_rider_=false;
  if(stage_==Stage::Release){if(in.vi-stage_vi_<2)return finish();stage(Stage::Flight,in.vi);out.buttons=d::jump_button;return finish();}
  const auto food=current_food(ram,birds);
  if(in.mouth_full&&(stage_==Stage::Flight||stage_==Stage::Tongue))stage(Stage::LandElevator,in.vi);
  if(stage_==Stage::Flight){
   if(!food||in.x>=330||in.x<232||in.y>337||in.vi-stage_vi_>=75){stage(Stage::LandElevator,in.vi);}
   else if(in.native_tongue_live||in.native_tongue_callback_pending||!idle_mouth(in)){stage(Stage::Tongue,in.vi);}
   else{
    // B is separate from A, and has no hypothetical range/heading admission.
    if(!in.grounded&&std::abs(in.x-lift->x)<=6&&std::abs(in.horizontal_speed)<=.5f&&in.native_mouth.valid()&&in.native_tongue_ready&&!(previous_&d::tongue_button)){
     if(const auto ray=boss_stick_aim0260::from_mouth(in.native_mouth,in.x,in.y,food->x,food->y);ray&&std::isfinite(in.native_tongue_max_reach)&&in.native_tongue_max_reach>0&&ray->distance<=in.native_tongue_max_reach){
      if(const auto axes=boss_stick_aim0260::toward(ray->forward,ray->up,in.native_stick);axes&&axes->cosine>=.9995f){out.x=axes->host.x;out.y=axes->host.y;out.buttons=d::tongue_button;stage(Stage::Tongue,in.vi);return finish();}
     }
    }
    // First get over the OBSERVED312 elevator and brake real C4. The native
    // eating animation can ignore movement; B from298 used to fall through
    // its left edge. The column is not a fabricated support or landing.
    out.buttons=d::jump_button;
    if(std::abs(in.x-lift->x)<=6){
     if(std::abs(in.horizontal_speed)>.5f)horizontal(in,out,in.horizontal_speed>0?-1.f:1.f,54);
     else if(!in.native_mouth.right)horizontal(in,out,1,4);
    }else if(g::clear(ram,*geometry,in.x,in.y,in.x+(in.x<lift->x?4.f:-4.f),in.y,in.grounded))horizontal(in,out,in.x<lift->x?1.f:-1.f,54);
    return finish();
   }
  }
  if(stage_==Stage::Tongue){
   if(in.native_tongue_live&&(in.native_tongue_phase==1||in.native_tongue_phase==2)){hold_tongue(in,out);return finish();}
   if(in.native_tongue_callback_pending||!idle_mouth(in))return finish();
   stage(Stage::LandElevator,in.vi);
  }
  // Land over the real moving platform before attempting the left jump.
  // Mouth/callback ownership remains native; actual parent attachment above
  // is the only route into the rider A+left recovery.
  if(stage_==Stage::LandElevator){
   if(in.monster_upper_supported&&in.grounded)stage(Stage::Return,in.vi);
   else if(in.y>337){stage(Stage::LowRecovery,in.vi);low_vi_=in.vi;}
   else {
    if(in.native_tongue_live&&!in.mouth_full&&(in.native_tongue_phase==1||in.native_tongue_phase==2)){hold_tongue(in,out);return finish();}
    if(in.native_tongue_callback_pending&&!in.mouth_full)return finish();
    if(std::abs(in.x-lift->x)>4)horizontal(in,out,in.x<lift->x?1.f:-1.f,54);
    else if(std::abs(in.horizontal_speed)>.5f)horizontal(in,out,in.horizontal_speed>0?-1.f:1.f,54);
    return finish();
   }
  }
  if((stage_==Stage::Return||stage_==Stage::Settle)&&in.y>337&&!in.monster_upper_supported){stage(Stage::LowRecovery,in.vi);low_vi_=in.vi;}
  if(stage_==Stage::LowRecovery&&in.monster_upper_supported&&in.grounded)stage(Stage::Return,in.vi);
  if(stage_==Stage::LowRecovery){
   // The real sloping floor at267/460 is valid native ground even though a
   // conservative enclosing rectangle clips a slope pixel. Do not call that
   // corridor clear. Request bounded ordinary movement/A toward the real
   // elevator; native terrain and object collisions decide every outcome.
   if(in.native_tongue_live&&!in.mouth_full&&(in.native_tongue_phase==1||in.native_tongue_phase==2)){hold_tongue(in,out);return finish();}
   if(in.native_tongue_callback_pending&&!in.mouth_full)return finish();
   if(!low_vi_)low_vi_=in.vi;const unsigned age=(in.vi-low_vi_)%90;
   if(std::abs(in.x-lift->x)>4)horizontal(in,out,in.x<lift->x?1.f:-1.f,54);
   else if(std::abs(in.horizontal_speed)>.5f)horizontal(in,out,in.horizontal_speed>0?-1.f:1.f,54);
   if(age>=2&&age<77)out.buttons=d::jump_button;
   return finish();
  }
  if(stage_==Stage::Return||stage_==Stage::Settle){
   if(in.native_tongue_live&&!in.mouth_full&&(in.native_tongue_phase==1||in.native_tongue_phase==2)){hold_tongue(in,out);return finish();}
   if(in.native_tongue_callback_pending&&!in.mouth_full)return finish();
   if(in.monster_upper_supported&&in.grounded){
    stage(Stage::Settle,in.vi);
    if(std::abs(in.x-248)>3){horizontal(in,out,in.x<248?1.f:-1.f,4);settle_=0;return finish();}
    if(in.horizontal_speed!=0){settle_=0;return finish();}
    if(in.mouth_full){out.y=-.85f;settle_=0;return finish();}
    if(!idle_mouth(in)){settle_=0;return finish();}
    settle_=settle_&&in.vi==settle_vi_+1?settle_+1:1;settle_vi_=in.vi;
    if(settle_>=2){out.completed=true;out.gained=in.reserves>stock_;auto result=finish();stage_=Stage::Idle;target_={};previous_=0;settle_=0;return result;}
    return finish();
   }
   settle_=0;
   if(g::clear(ram,*geometry,in.x,in.y,248,in.y,in.grounded)){
    if(std::abs(in.x-248)>3)horizontal(in,out,in.x<248?1.f:-1.f,54);
    else if(in.horizontal_speed!=0)horizontal(in,out,in.horizontal_speed>0?-1.f:1.f,54);
    if(!in.grounded)out.buttons=d::jump_button;
   }
   // A blocked high return (realY190) must descend into the actual clear
   // Y210 corridor. Releasing A here is not a claimed physical descent.
   return finish();
  }
  return finish();
 }
};
}
