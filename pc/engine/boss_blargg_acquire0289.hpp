#pragma once
// Diagnostic acquisition only; no native function call, RAM write or public
// driver support change. Source causes of the old fixed pad remain unproved.
#include "boss_variants0250.hpp"
#include "boss_solo_runtime0240.hpp"
#include "boss_monster_forage0284.hpp"
#include "boss_native_egg_aim0284.hpp"
namespace workshop::boss_blargg_acquire0289 {
namespace n=boss_native;namespace core=boss_battle0250;namespace v=boss_variants0250;
namespace mouth=boss_native_mouth0260;namespace aim=boss_stick_aim0260;namespace food=boss_monster_forage0284;
enum class Stage:unsigned {Disabled,WaitReady,WaitBird,Prepare,AwaitNative,Extend,Swallow,Cooldown,Done,Failed};
struct Frame {
 bool marker=false,diagnostic=false,fighting=false,fight_ready=false,generic_driver=false;
 unsigned physical_room=177,logical_room=177,index=10,retry_epoch=0,vi=0;
 uint32_t player=0;std::string_view token{};uint64_t seed=0;
};
struct Output {bool handled=false;uint16_t buttons=0;float x=0,y=0;};
struct Snapshot {
 bool enabled=false,started=false,done=false,failed=false;
 Stage stage=Stage::Disabled;unsigned index=10,room=177,last_vi=0,first_vi=0,arrival_vi=0,stage_vi=0,end_vi=0,stock=0,initial_stock=0,attempts=0,gains=0,misses=0;
 uint32_t player=0,private_data=0,target=0,held=0;uint64_t target_birth=0;
 bool native_b=false,live_seen=false;unsigned observed_birds=0,predicted_birds=0;
 float food_x=0,food_y=0,food_vx=0,food_vy=0,ray_forward=0,ray_up=0,ray_distance=0,max_reach=0;const char* reason="disabled";
};
struct Candidate {food::Food observed{};float predicted_x=0,predicted_y=0,distance=0;};
// Vertical-only first batch: from_mouth uses the true native animation table.
// The bird must cross the real near-vertical tongue corridor; no walking/A.
inline std::optional<Candidate> vertical_candidate(const food::Food& bird,float vx,float vy,const aim::Mouth& m,float px,float py,float max_reach){
 if(!std::isfinite(vx)||!std::isfinite(vy)||std::abs(vx)>3||std::abs(vy)>3||!std::isfinite(max_reach)||max_reach<=0)return {};
 const float tx=bird.x+vx*8.f,ty=bird.y+vy*8.f;
 const auto ray=aim::from_mouth(m,px,py,tx,ty);
 if(!ray||ray->up<12||std::abs(ray->forward)>7||ray->distance>max_reach-4)return {};
 const auto later=aim::from_mouth(m,px,py,bird.x+vx*12.f,bird.y+vy*12.f);
 if(!later||later->up<8||std::abs(later->forward)>11||later->distance>max_reach)return {};
 return Candidate{bird,tx,ty,ray->distance};
}
class Controller final {
 struct History {food::Bird binding{};unsigned vi=0;float x=0,y=0,vx=0,vy=0;bool velocity=false;};
 Snapshot state_{};std::string token_;uint64_t seed_=0;std::array<History,2> history_{};food::Bird target_{};Output cached_{};
 bool have_vi_=false,logging_=true,b_released_=false;unsigned logs_=0,shot_vi_=0;uint16_t previous_buttons_=0;
 static bool pointer(uint32_t a,unsigned size){return !(a&3)&&n::range(a,size);}
 static float fl(uint8_t* r,uint32_t a){return std::bit_cast<float>(n::word(r,a));}
 void stage(Stage next,unsigned vi,const char* why){if(state_.stage!=next){state_.stage=next;state_.stage_vi=vi;}state_.reason=why;}
 Output fail(unsigned vi,const char* why){state_.failed=true;stage(Stage::Failed,vi,why);return cached_={true,0,0,0};}
 static bool held_actor(uint8_t* r,uint32_t a){return pointer(a,0x1FC)&&n::word(r,a)==0x19971110&&n::byte(r,a+6)!=3&&n::byte(r,a+0x18)!=0xFF;}
 static bool safe_stand(uint8_t* r,uint32_t player,bool require_ground=true){
  const auto p=n::word(r,player+0x1F4);const float x=fl(r,player+0x3C),y=fl(r,player+0x40),world_x=fl(r,player+0x84),world_y=fl(r,player+0x88),vx=fl(r,player+0xC4);
  // Contact/local Y161 and animated world Y163.94 are distinct real fields.
  if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(world_x)||!std::isfinite(world_y)||!std::isfinite(vx)||x<228||x>284||world_x<228||world_x>284||y<148||y>168||world_y<145||world_y>174||std::abs(vx)>.5f||(require_ground&&n::byte(r,p+0xF4)!=0)||n::word(r,player+0x180))return false;
  const auto map=boss_monster_aim0260::Map::read(r);if(!map||map->columns!=2||map->rows!=1||map->stride!=32)return false;
  for(float xx:{x-20,x,x+20})for(float yy:{176.f,178.f}){const auto solid=map->occupied(r,xx,yy);if(!solid||!*solid)return false;}
  for(float xx:{x-20,x,x+20}){const auto air=map->occupied(r,xx,175.f);if(!air||*air)return false;}
  return true;
 }
 static bool idle(const mouth::TongueRequest& t,uint32_t held){return !held&&t.observed&&!t.live&&!t.callback_pending&&(!t.phase||t.cancelled_idle);}
 Output finish(uint8_t* r,const Frame& f,const mouth::TongueRequest& t,Output out){
  out.x=0;out.buttons&=uint16_t(0x4000); // No A, Z, facing walk or horizontal chase exists in this batch.
  if(logging_&&logs_<240&&(out.buttons!=previous_buttons_||f.vi%60==0||state_.stage==Stage::Done||state_.stage==Stage::Failed)){
   ++logs_;const auto packet=n::word(r,0x800F7EA8);auto* rdram=r;
   const unsigned pressed=pointer(packet,0x18)?MEM_HU(12,n::base(packet)):0,held_buttons=pointer(packet,0x18)?MEM_HU(0,n::base(packet)):0;
   std::fprintf(stderr,"[BossBlarggAcquireQA] vi=%u stage=%u player=%08X localXY=%.3f,%.3f worldXY=%.3f,%.3f ground=%u verticalVelocity=%.4f arrivalVI=%u stock=%u initial=%u target=%08X birth=%llu held=%08X tongue=%u/%u/%u/%u nativePressedB=%u packetHeld=%04X packetPressed=%04X starts=%u gains=%u misses=%u birds=%u/%u foodXY=%.3f,%.3f foodV=%.4f,%.4f ray=%.3f,%.3f/%.3f/%.3f output=%04X/%.3f/%.3f done=%u failed=%u reason=%s inputsOnly=1 ramWrites=0\n",f.vi,unsigned(state_.stage),state_.player,fl(r,state_.player+0x3C),fl(r,state_.player+0x40),fl(r,state_.player+0x84),fl(r,state_.player+0x88),n::byte(r,state_.private_data+0xF4),fl(r,state_.player+0xC8),state_.arrival_vi,state_.stock,state_.initial_stock,state_.target,(unsigned long long)state_.target_birth,state_.held,t.phase,unsigned(t.live),unsigned(t.callback_pending),unsigned(t.ready),n::byte(r,0x800F8998),held_buttons,pressed,state_.attempts,state_.gains,state_.misses,state_.observed_birds,state_.predicted_birds,state_.food_x,state_.food_y,state_.food_vx,state_.food_vy,state_.ray_forward,state_.ray_up,state_.ray_distance,state_.max_reach,unsigned(out.buttons),out.x,out.y,unsigned(state_.done),unsigned(state_.failed),state_.reason);
  }
  previous_buttons_=out.buttons;cached_=out;return out;
 }
 std::optional<Candidate> candidate(uint8_t* r,const Frame& f,const aim::Mouth& m,float px,float py,float max_reach){
  std::optional<Candidate> best;state_.observed_birds=state_.predicted_birds=0;state_.max_reach=max_reach;float nearest=1e9f;
  for(unsigned i=0;i<2;++i){const auto& binding=v::birds[i];food::Bird b{binding.actor,binding.private_data,binding.resource,binding.factory,binding.birth};
   const auto observed=food::observe_food(r,b);if(!observed){history_[i]={};continue;}
   auto& old=history_[i];bool velocity=false;float vx=0,vy=0;
   const auto& bb=observed->binding;++state_.observed_birds;
   if(old.binding.actor==bb.actor&&old.binding.birth==bb.birth&&old.binding.private_data==bb.private_data&&old.binding.resource==bb.resource&&f.vi>old.vi&&f.vi-old.vi<=4){
    const float dt=float(f.vi-old.vi);vx=(observed->x-old.x)/dt;vy=(observed->y-old.y)/dt;velocity=std::isfinite(vx)&&std::isfinite(vy)&&std::abs(vx)<=3&&std::abs(vy)<=3;
   }
   old={bb,f.vi,observed->x,observed->y,vx,vy,velocity};
   if(!velocity)continue;++state_.predicted_birds;
   if(const auto ray=aim::from_mouth(m,px,py,observed->x+vx*8.f,observed->y+vy*8.f);ray&&ray->distance<nearest){nearest=ray->distance;state_.food_x=observed->x;state_.food_y=observed->y;state_.food_vx=vx;state_.food_vy=vy;state_.ray_forward=ray->forward;state_.ray_up=ray->up;state_.ray_distance=ray->distance;}
   const auto test=vertical_candidate(*observed,vx,vy,m,px,py,max_reach);if(!test)continue;
   const auto map=boss_monster_aim0260::Map::read(r);
   if(!map||!map->clear(r,px,py-18,test->predicted_x,test->predicted_y,2))continue;
   if(!best||test->distance<best->distance)best=test;
  }return best;
 }
public:
 bool configure(bool enabled,std::string token,uint64_t seed,unsigned index,unsigned physical,unsigned end_vi){
  *this=Controller{};
  if(!enabled||token.empty()||token.size()>128||index<1||index>8||physical!=167+index||end_vi<6000||end_vi>12000)return false;
  for(unsigned char c:token)if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_'))return false;
  token_=std::move(token);seed_=seed;state_.enabled=true;state_.index=index;state_.room=physical;state_.end_vi=end_vi;state_.stage=Stage::WaitReady;state_.reason="waiting_fight_ready";return true;
 }
 void disable(){*this=Controller{};}void set_logging(bool enabled){logging_=enabled;}
 const Snapshot& snapshot()const{return state_;}
 Output step(uint8_t* r,const Frame& f){
  if(!state_.enabled)return {};
  if(state_.failed)return {true,0,0,0};
  const bool scope=r&&f.marker&&f.diagnostic&&!f.generic_driver&&f.token==token_&&f.seed==seed_&&f.physical_room==state_.room&&f.logical_room==19&&f.index==state_.index&&!f.retry_epoch&&f.vi&&f.vi<=state_.end_vi;
  if(!scope){if(state_.started||state_.arrival_vi)return fail(f.vi,"acquisition_scope_lost");return {};}
  if(!f.fighting||!f.fight_ready){if(state_.started||state_.arrival_vi)return fail(f.vi,"fight_ready_lost");return {};}
  if(have_vi_&&f.vi<state_.last_vi)return fail(f.vi,"VI_reversed");
  if(have_vi_&&f.vi==state_.last_vi)return cached_;
  have_vi_=true;state_.last_vi=f.vi;
  const auto& vs=v::snapshot();
  if(!vs.enabled||vs.invalidated||vs.encounter!=f.index||vs.room!=19||v::encounter.family!=core::Family::Blargg||v::encounter.birds!=2||!v::config.blargg||!core::valid_blargg(v::encounter,v::config))return fail(f.vi,"native_variant_scope");
  const auto player=f.player;
  if(!pointer(player,0x27C)||!n::actor(r,player,0x4001))return fail(f.vi,"native_player_address");
  const auto p=n::word(r,player+0x1F4);
  if(n::word(r,0x800F8828)!=player||!pointer(p,0x12F0)||n::byte(r,player+0x17D)!=2||n::byte(r,player+0x165)||n::byte(r,player+0x18)==0||n::byte(r,player+0x18)==0xFF||n::byte(r,player+0x18)==0xA||n::word(r,p+0x12EC))return fail(f.vi,"native_player_identity");
  if(state_.arrival_vi&&(player!=state_.player||p!=state_.private_data))return fail(f.vi,"native_player_generation_changed");
  std::array<uint32_t,6> reserves{};if(!boss_solo0240::read_reserves(r,player,reserves))return fail(f.vi,"native_reserve_graph");
  const unsigned stock=unsigned(std::count_if(reserves.begin(),reserves.end(),[](auto a){return a!=0;}));
  // fight_ready proves the room/P1 identity, not that native gravity has
  // already landed. Real1946 has local/world256/152, F4=1, VY+.8;
  // all map/support probes are already valid. Wait only for that native F4,
  // binding the same player/private before admission and bounding the wait.
  if(!state_.arrival_vi){state_.arrival_vi=f.vi;state_.player=player;state_.private_data=p;}
  if(!state_.started){
   if(stock)return fail(f.vi,"initial_stock_not_zero");
   if(!safe_stand(r,player,false))return fail(f.vi,"native_initial_centre_rejected");
   if(f.vi-state_.arrival_vi>=180)return fail(f.vi,"native_initial_ground_timeout");
   if(n::byte(r,p+0xF4)!=0){
    stage(Stage::WaitReady,f.vi,"waiting_native_ground");return finish(r,f,{},Output{true,0,0,0});
   }
   state_.started=true;state_.first_vi=f.vi;stage(Stage::WaitBird,f.vi,"waiting_owned_bird");
  }
  if(stock<state_.stock)return fail(f.vi,"reserve_lost_without_throw");
  if(stock>state_.stock){state_.gains+=stock-state_.stock;state_.stock=stock;}
  const auto t=mouth::observe_ordinary_tongue(r,player);const auto held=n::word(r,p+0x164);state_.held=held;
  if(!safe_stand(r,player))return fail(f.vi,"native_safe_centre_lost");
  const auto egg=boss_native_egg_aim0284::observe(r,player);
  if(!egg.player_observed||egg.held||egg.reticle||egg.held_flag)return fail(f.vi,"unexpected_native_aim");
  if(!t.observed&&!held)return fail(f.vi,"unknown_native_tongue_graph");
  Output out{true,0,0,0};
  if(state_.done)return finish(r,f,t,out);
  if(stock==6&&idle(t,held)){state_.done=true;stage(Stage::Done,f.vi,"six_real4004_reserves");return finish(r,f,t,out);}
  if(f.vi==state_.end_vi){fail(f.vi,"acquisition_deadline");return finish(r,f,t,out);}
  auto* rdram=r;const auto packet=n::word(r,0x800F7EA8);
  const bool pressed_b=n::byte(r,0x800F8998)||(pointer(packet,0x18)&&(MEM_HU(12,n::base(packet))&0x4000));
  if(state_.stage==Stage::AwaitNative||state_.stage==Stage::Extend)state_.native_b|=pressed_b;
  if(held){
   if(!held_actor(r,held)||held!=target_.actor||n::word(r,held+0x1F4)!=target_.private_data||n::word(r,held+0x168)!=target_.resource)return fail(f.vi,"held_food_identity");
   stage(Stage::Swallow,f.vi,"native_held_owned_bird");out.y=-.85f;return finish(r,f,t,out);
  }
  if(state_.stage==Stage::Swallow){if(idle(t,held)){target_={};state_.target=0;state_.target_birth=0;stage(Stage::Cooldown,f.vi,"native_swallow_returned");}return finish(r,f,t,out);}
  if(state_.stage==Stage::AwaitNative||state_.stage==Stage::Extend){
   const unsigned age=f.vi-shot_vi_;
   if(t.live||t.callback_pending){state_.live_seen=true;stage(Stage::Extend,f.vi,"native_tongue_motion");}
   if(!b_released_&&age<60&&(!state_.live_seen||(t.live&&t.phase<=2)))out.buttons=0x4000;else b_released_=true;
   if(t.heading_active){const auto heading=boss_tongue_geometry0260::hold_heading_world(t.heading_degrees,n::word(r,player+0x138)!=0,{int(MEM_H(0,n::base(0x800F897E))),int(MEM_H(0,n::base(0x800F8980)))});if(heading)out.y=heading->host.y;}
   else out.y=.85f;
   if(!state_.live_seen&&age>=20){++state_.misses;stage(Stage::Cooldown,f.vi,state_.native_b?"B_seen_without_native_extension":"native_B_front_unobserved");out={true,0,0,0};}
   else if(state_.live_seen&&idle(t,held)&&age>=4){++state_.misses;stage(Stage::Cooldown,f.vi,"native_extension_no_held_food");out={true,0,0,0};}
   if(age>150){fail(f.vi,"native_tongue_timeout");out={true,0,0,0};}
   return finish(r,f,t,out);
  }
  if(state_.stage==Stage::Cooldown){if(f.vi-state_.stage_vi<32)return finish(r,f,t,out);stage(Stage::WaitBird,f.vi,"waiting_owned_bird");}
  if(!idle(t,held)||!t.ready||n::byte(r,player+0x18)!=1)return finish(r,f,t,out);
  std::array<float,80> xs{},ys{};aim::Mouth m;
  if(!mouth::observe_native_mouth(r,player,xs,ys,m))return fail(f.vi,"native_mouth_geometry");
  const auto best=candidate(r,f,m,fl(r,player+0x84),fl(r,player+0x88),t.max_reach);
  if(!best){if(state_.stage==Stage::Prepare)stage(Stage::WaitBird,f.vi,"predicted_bird_left_vertical_ray");return finish(r,f,t,out);}
  if(state_.stage!=Stage::Prepare||target_.actor!=best->observed.binding.actor||target_.birth!=best->observed.binding.birth){
   target_=best->observed.binding;state_.target=target_.actor;state_.target_birth=target_.birth;stage(Stage::Prepare,f.vi,"owned_bird_in_native_vertical_ray");
  }
  out.y=.85f;
  // private174 is an OUTPUT of the B-triggered2ED94 initializer, then
  // maintained by25EA4. Requiring it before B would prevent its own birth.
  // Admit only the already prepared native stick direction instead; the
  // real tongue's heading is used later by the existing live-graph reader.
  const aim::Axes native_axes{int(MEM_H(0,n::base(0x800F897E))),int(MEM_H(0,n::base(0x800F8980)))};
  const float magnitude=std::hypot(float(native_axes.x),float(native_axes.y));
  const auto vertical=aim::toward(0,1,native_axes);
  const bool prepared_up=aim::valid(native_axes)&&native_axes.y>=24&&magnitude>0&&float(native_axes.y)/magnitude>=.9995f&&vertical&&vertical->cosine>=.9995f;
  if(f.vi-state_.stage_vi>=4&&prepared_up&&!(previous_buttons_&0x4000)){
   ++state_.attempts;shot_vi_=f.vi;state_.native_b=state_.live_seen=false;b_released_=false;stage(Stage::AwaitNative,f.vi,"single_B_edge");out.buttons=0x4000;
  }
  return finish(r,f,t,out);
 }
};
} // namespace workshop::boss_blargg_acquire0289
