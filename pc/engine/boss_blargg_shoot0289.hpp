#pragma once
// Isolated QA controller: ordinary pad outputs only. No guest writes, native
// calls, public family support, HP/reserve creation or forced hit/victory.
#include "boss_blargg0289.hpp"
#include "boss_battle0250.hpp"
#include "boss_solo_runtime0240.hpp"
#include "boss_native_egg_aim0284.hpp"
#include "boss_stick_aim0260.hpp"
#include "boss_monster_aim0260.hpp"
#include "boss_egg_shape0260.hpp"
#include "boss_lips_corridor0260.hpp"
#include <limits>
namespace workshop::boss_blargg_shoot0289 {
namespace n=boss_native; namespace core=boss_battle0250;
namespace ledger=boss_blargg0289; namespace aim=boss_stick_aim0260;
enum class Stage:unsigned {Disabled,WaitAcquire,WaitTarget,Face,RequestAim,Aiming,Flight,Retraction,Pot,Travel,Done,Failed};
struct Frame {
 bool marker=false,diagnostic=false,acquisition_done=false,fight_ready=false,generic_driver=false;
 core::Phase phase=core::Phase::Idle;
 unsigned physical_room=177,logical_room=177,index=10,retry_epoch=0,vi=0,completed=0;
 uint32_t player=0,exit_pot=0;
 std::string_view token{}; uint64_t seed=0;
 std::size_t observation_bytes=0;
};
struct Output {bool handled=false;uint16_t buttons=0;float x=0,y=0;};
struct Snapshot {
 bool enabled=false,started=false,done=false,failed=false,receipts_latched=false;
 Stage stage=Stage::Disabled;
 unsigned index=10,room=177,end_vi=0,first_vi=0,last_vi=0,stage_vi=0;
 unsigned stock=0,initial_stock=0,target_member=2,attempts=0,launches=0,hits=0,misses=0;
 std::array<unsigned,2> member_hits{}; std::array<bool,2> retired{},receipts{};
 uint32_t player=0,private_data=0,target=0,held=0,reticle=0,projectile=0,pot=0;
 uint64_t target_birth=0,projectile_birth=0;
 float target_x=0,target_y=0,origin_x=0,origin_y=0,ray_x=0,ray_y=0,flight_vi=0;
 const char* reason="disabled";
};
inline bool pointer(uint32_t a,std::size_t size){return !(a&3)&&n::range(a,size);}
inline float scalar(uint8_t* r,uint32_t a){return std::bit_cast<float>(n::word(r,a));}
inline bool point(float x,float y){return std::isfinite(x)&&std::isfinite(y)&&std::abs(x)<100000&&std::abs(y)<100000;}
// 47AB8 copies LOCAL3C/40/44 when parent180==0.46E4 then applies scales
// 6C/70/74, private60 onY, and the actual facing sign. Its three boxes are a
// UNION, not a top invulnerability. This QA deliberately chooses box number2.
struct Target {
 unsigned member=2,hits=0,open_remaining=0;
 ledger::Identity identity{};
 float x=0,y=0,z=0,half_x=0,half_y=0,half_z=0;
 float excluded_x=0,excluded_y=0,excluded_half_x=0,excluded_half_y=0;
};
inline std::optional<Target> observe_target(uint8_t* r,const ledger::Observer& observer,unsigned member){
 const auto& s=observer.snapshot();
 if(!r||member>=2||!s.enabled||s.invalidated||!observer.initialized_member(r,member))return {};
 const auto& m=s.members[member];const auto a=m.identity.actor,p=m.identity.private_data,o=m.overlay;
 if(m.hits>=3||m.retired||n::word(r,a+0x180)||n::byte(r,a+0x18)!=1||n::byte(r,a+0x30)!=5||n::word(r,a+0x20)!=o+(0x804F5928-ledger::original_overlay)||!pointer(o+0x20764,72))return {};
 constexpr std::array<float,18> boxes{0,-20,0,40,20,20,15,-61,0,25,21,20,0,-102,0,40,10,20};
 for(unsigned i=0;i<boxes.size();++i)if(scalar(r,o+0x20764+4*i)!=boxes[i])return {};
 const float x=scalar(r,a+0x3C),y=scalar(r,a+0x40),z=scalar(r,a+0x44),sx=scalar(r,a+0x6C),sy=scalar(r,a+0x70)*scalar(r,p+0x60),sz=scalar(r,a+0x74);
 const auto loops=n::word(r,p+0x34),tick=n::word(r,p+0x10);
 if(!point(x,y)||!std::isfinite(z)||!std::isfinite(sx)||!std::isfinite(sy)||!std::isfinite(sz)||sx<=0||sx>4||sy<=0||sy>4||sz<=0||sz>4||loops<1||loops>3||tick>=50)return {};
 const float sign=n::word(r,a+0x138)?-1.f:1.f;
 return Target{member,m.hits,50*loops-tick,m.identity,x+15*sx*sign,y-61*sy,z,25*sx,21*sy,20*sz,x,y-102*sy,40*sx,10*sy};
}
struct Flight {
 bool ready=false;unsigned ticks=0;
 float vx=0,vy=0,x=0,y=0;
 const char* reason="unobserved";
};
// Original405C8 uses P1 WORLD84/88 and8C+RAM[800B02E0] when creating4002
// at405F4..40600;4031C is another path and is not licensed here.404C8 derives
// its angle from4003 WORLD-P1 WORLD, then40548/570 load both speed6.6.
// The immutable PC table is read only through the existing capacity-aware
// Shape reader. This prediction is a PAD planning bound, never a hit proof.
inline Flight evaluate(uint8_t* r,const Frame& f,const Target& t,const boss_native_egg_aim0284::Observation& egg){
 Flight out;out.reason="native_aim_geometry";
 if(!r||!f.player||!egg.held||!egg.reticle||f.observation_bytes<boss_egg_shape0260::required_capacity)return out;
 const auto current=boss_native_egg_aim0284::observe(r,f.player);
 if(!current.player_observed||current.held!=egg.held||current.reticle!=egg.reticle||current.reticle_x!=egg.reticle_x||current.reticle_y!=egg.reticle_y)return out;
 const auto shape=boss_egg_shape0260::observe_aimed(std::span<const uint8_t>(r,f.observation_bytes),f.player);
 const auto map=boss_monster_aim0260::Map::read(r);
 if(!shape||!map||map->columns!=2||map->rows!=1)return out;
 const float x=scalar(r,f.player+0x84),y=scalar(r,f.player+0x88),z=scalar(r,f.player+0x8C)+scalar(r,0x800B02E0);
 const float dx=egg.reticle_x-x,dy=egg.reticle_y-y,length=std::hypot(dx,dy),distance=std::hypot(t.x-x,t.y-y);
 const float speedx=scalar(r,0x800B02D8),speedy=scalar(r,0x800B02DC),pi=scalar(r,0x800B02D4);
 if(!point(x,y)||!std::isfinite(z)||!std::isfinite(length)||length<distance+8||length>600||distance<8||std::abs(speedx-6.6f)>.00001f||std::abs(speedy-6.6f)>.00001f||std::abs(pi-3.1415927410125732f)>.000001f||scalar(r,0x800B02E0)!=-.001f){out.reason="native_reticle_range_or_constants";return out;}
 const float degrees=aim::native_atan2(dy,dx)*180.f/pi;
 out.vx=aim::native_trig(degrees,true)*speedx;out.vy=aim::native_trig(degrees,false)*speedy;
 // Flight is checked at native discrete updates. An8-update launch envelope
 // covers the observed deferred animation; leave an additional4-call margin.
 for(unsigned tick=1;tick<=96&&float(tick)*6.6f<length-2;++tick){
  const float tx=x+out.vx*float(tick),ty=y+out.vy*float(tick);
  if(std::abs(tx-t.x)>=t.half_x+shape->half_x-2||std::abs(ty-t.y)>=t.half_y+shape->half_y-2||std::abs(z-t.z)>=t.half_z+shape->half_z-2)continue;
  out.ticks=tick;out.x=tx;out.y=ty;
  if(t.open_remaining<=tick+12){out.reason="native_open_window_too_short";return out;}
  const auto corridor=boss_lips_corridor0260::observe(r,*map,x,y,tx,ty,shape->half_x,shape->half_y);
  if(!corridor||!corridor->clear){out.reason="native_egg_terrain_corridor";return out;}
  out.ready=true;out.reason="source_box_and_terrain_ray";return out;
 }
 out.reason="native_reticle_ray_misses_head_box";return out;
}
class Controller final {
 Snapshot state_{};std::string token_;uint64_t seed_=0;
 std::array<ledger::Identity,2> identities_{};
 Output cached_{};bool have_vi_=false,logging_=true;
 unsigned release_vi_=0,hits_at_release_=0,logs_=0,aim_matches_=0;
 uint16_t previous_buttons_=0;
 uint32_t launch_actor_=0;uint64_t launch_birth_=0;unsigned launch_vi_=0;
 uint64_t pot_birth_=0;uint32_t pot_resource_=0,pot_overlay_=0,pot_private_=0,pot_route_=0;
 unsigned pot_first_vi_=0;bool pot_active_=false,pot_mounted_=false,pot_entry_=false;
 void stage(Stage next,unsigned vi,const char* why){if(state_.stage!=next){state_.stage=next;state_.stage_vi=vi;}state_.reason=why;}
 Output fail(unsigned vi,const char* why){state_.failed=true;stage(Stage::Failed,vi,why);return cached_={true,0,0,0};}
 Output finish(uint8_t* r,const Frame& f,Output out){
  (void)r;
  if(!state_.receipts_latched)out.buttons&=uint16_t(0x2000);
  if(!std::isfinite(out.x)||!std::isfinite(out.y)||std::hypot(out.x,out.y)>1.001f)return fail(f.vi,"invalid_host_output");
  if(logging_&&logs_<240&&(out.buttons!=previous_buttons_||f.vi%60==0||state_.done||state_.failed)){
   ++logs_;
   std::fprintf(stderr,"[BossBlarggShootQA] vi=%u stage=%u phase=%u player=%08X stock=%u initial=%u member=%u target=%08X birth=%llu head=%.4f,%.4f aim=%08X reticle=%08X origin=%.4f,%.4f ray=%.4f,%.4f flight=%.1f attempts=%u launches=%u projectile=%08X projectileBirth=%llu hits=%u/%u/%u retired=%u/%u receipts=%u/%u latched=%u pot=%08X completed=%u output=%04X/%.5f/%.5f done=%u failed=%u reason=%s controlsOnly=1 ramWrites=0\n",f.vi,unsigned(state_.stage),unsigned(f.phase),state_.player,state_.stock,state_.initial_stock,state_.target_member,state_.target,(unsigned long long)state_.target_birth,state_.target_x,state_.target_y,state_.held,state_.reticle,state_.origin_x,state_.origin_y,state_.ray_x,state_.ray_y,state_.flight_vi,state_.attempts,state_.launches,state_.projectile,(unsigned long long)state_.projectile_birth,state_.hits,state_.member_hits[0],state_.member_hits[1],unsigned(state_.retired[0]),unsigned(state_.retired[1]),unsigned(state_.receipts[0]),unsigned(state_.receipts[1]),unsigned(state_.receipts_latched),state_.pot,f.completed,unsigned(out.buttons),out.x,out.y,unsigned(state_.done),unsigned(state_.failed),state_.reason);
  }
  previous_buttons_=out.buttons;cached_=out;return out;
 }
 //3BE44 uses the native pressed-Z edge only for admission. Once admitted,
 //3BD60/3BE00 keeps hold-mode aiming alive by reading HELD Z, and native
 //3/BF88 can temporarily close the initial request gate. A boss closing is
 //not permission to release an already requested egg before its graph exists.
 Output hold_request(uint8_t* r,const Frame& f,Output out){
  if(f.vi-state_.stage_vi>90)return fail(f.vi,"native4005_not_created");
  state_.reason="native_Z_pending_until_owned_graph";out.buttons=0x2000;
  return finish(r,f,neutral_axes(r,out));
 }
 static bool safe_stand(uint8_t* r,uint32_t player,uint32_t p){
  const float x=scalar(r,player+0x3C),y=scalar(r,player+0x40),wx=scalar(r,player+0x84),wy=scalar(r,player+0x88),vx=scalar(r,player+0xC4);
  if(!point(x,y)||!point(wx,wy)||!std::isfinite(vx)||x<228||x>284||wx<228||wx>284||y<148||y>168||wy<145||wy>174||std::abs(vx)>2||n::byte(r,p+0xF4)||n::word(r,player+0x180))return false;
  const auto map=boss_monster_aim0260::Map::read(r);if(!map||map->columns!=2||map->rows!=1||map->stride!=32)return false;
  for(float xx:{x-20,x,x+20}){for(float yy:{176.f,178.f}){const auto solid=map->occupied(r,xx,yy);if(!solid||!*solid)return false;}const auto air=map->occupied(r,xx,175);if(!air||*air)return false;}return true;
 }
 bool player(uint8_t* r,const Frame& f,bool bind){
  if(!pointer(f.player,0x27C)||!n::actor(r,f.player,0x4001)||n::word(r,0x800F8828)!=f.player)return false;
  const auto p=n::word(r,f.player+0x1F4);if(!pointer(p,0x12F0))return false;
  if(bind){state_.player=f.player;state_.private_data=p;}
  return f.player==state_.player&&p==state_.private_data;
 }
 static aim::Axes native_axes(uint8_t* r){auto* rdram=r;return {int(MEM_H(0,n::base(0x800F897E))),int(MEM_H(0,n::base(0x800F8980)))};}
 static Output neutral_axes(uint8_t* r,Output out){
  const auto old=native_axes(r);if(!aim::valid(old))return out;
  for(const auto& c:aim::legal_candidates()){const auto at=aim::native_axes(c.raw,old);if(!at.x&&!at.y){out.x=c.host.x;out.y=c.host.y;return out;}}return out;
 }
 static Output move_reticle(uint8_t* r,float ex,float ey,Output out){
  const auto old=native_axes(r);if(!aim::valid(old))return out;
  if(std::hypot(ex,ey)<.35f)ex=ey=0;
  const aim::Candidate* best=nullptr;float error=std::numeric_limits<float>::infinity(),size=error;
  //39338/390F0 moves the actual4003 by nativeAxes/9, Y sign reversed.
  // Reuse the existing legal host vectors and native filter, never its memory.
  for(const auto& c:aim::legal_candidates()){
   const auto a=aim::native_axes(c.raw,old);const float dx=ex-float(a.x)/9.f,dy=ey+float(a.y)/9.f,e=dx*dx+dy*dy,s=float(a.x*a.x+a.y*a.y);
   if(e<error-.00001f||(std::abs(e-error)<=.00001f&&s<size)){best=&c;error=e;size=s;}
  }if(best){out.x=best->host.x;out.y=best->host.y;}return out;
 }
 bool scope(const Frame& f)const{return f.marker&&f.diagnostic&&!f.generic_driver&&f.token==token_&&f.seed==seed_&&f.index==state_.index&&!f.retry_epoch&&f.vi&&f.vi<=state_.end_vi;}
 bool ledger_scope(const ledger::Observer& observer)const{
  const auto& s=observer.snapshot();return s.enabled&&!s.invalidated&&s.config.diagnostic&&s.config.token==token_&&s.config.encounter==state_.index&&s.config.physical_room==state_.room&&!s.config.retry_epoch;
 }
 bool update_ledger(const ledger::Observer& observer,unsigned vi){
  const auto& s=observer.snapshot();
  for(unsigned i=0;i<2;++i){const auto& m=s.members[i];
   if(m.identity!=identities_[i]||!m.initialized||m.hits<state_.member_hits[i]||m.hits>3||(state_.retired[i]&&!m.retired)||(state_.receipts[i]&&!m.receipt_sent))return false;
   if(m.hits>state_.member_hits[i]&&(state_.stage!=Stage::Flight||state_.target_member!=i||m.hits!=state_.member_hits[i]+1||!launch_actor_||!launch_birth_||launch_vi_<release_vi_))return false;
   state_.member_hits[i]=m.hits;state_.retired[i]=m.retired;state_.receipts[i]=m.receipt_sent;
   if((m.retired||m.receipt_sent)&&(!m.retracted||m.hits!=3||!m.retired||!m.retirement_vi||!m.retraction_vi||m.retraction_vi>m.retirement_vi||m.retirement_vi>vi))return false;
  }
  state_.hits=state_.member_hits[0]+state_.member_hits[1];return true;
 }
 //44194 queues an actor with primary0/queue1 before its resource ctor runs.
 //45F78/45F88 install resource44 as callback; the true4273 ctor is overlay144
 //+960. AB778 creates27C private bytes and AB914 selects the five1/x callbacks.
 //This bounded HOST wait never initializes, moves or otherwise writes the pot.
 bool observe_pot(uint8_t* r,const Frame& f,const ledger::Observer& observer,bool& pending){
  pending=false;const auto a=f.exit_pot;
  if(!pointer(a,0x1FC)||!n::actor(r,a,0x4273)){state_.reason="native_victory_pot_missing";return false;}
  const auto resource=n::word(r,a+0x168);const auto birth=observer.birth(a);
  if(!birth||!pointer(resource,0x48)||(n::word(r,resource+0x3C)>>16)!=0x4273){state_.reason="native_victory_pot_resource";return false;}
  const auto overlay=n::word(r,resource+0x40),route=n::word(r,resource+0x34);
  if(!pointer(overlay,0xB28)||!pointer(route,24)||n::word(r,resource+0x44)!=overlay+0x960||n::word(r,a+0x2C)!=0x80045BD8||n::word(r,a+0x180)){state_.reason="native_victory_pot_metadata";return false;}
  if(!state_.pot){state_.pot=a;pot_birth_=birth;pot_resource_=resource;pot_overlay_=overlay;pot_route_=route;pot_first_vi_=f.vi;}
  if(state_.pot!=a||pot_birth_!=birth||pot_resource_!=resource||pot_overlay_!=overlay||pot_route_!=route){state_.reason="native_victory_pot_generation";return false;}
  //The absolute budget precedes admission, including a first late queue2 step.
  if(!pot_active_&&f.vi-pot_first_vi_>=120){state_.reason="native_victory_pot_init_timeout";return false;}
  const auto queue=n::byte(r,a+0x17D),primary=n::byte(r,a+0x18),sub=n::byte(r,a+0x30);
  const auto p=n::word(r,a+0x1F4),callback=n::word(r,a+0x20);
  if(queue==1){
   if(pot_active_||primary||sub||p||n::word(r,a+0x1F8)||callback!=overlay+0x960){state_.reason="native_victory_pot_init_state";return false;}
   pending=true;return true;
  }
  //AB914 only selects primary1/sub0..4 for this pot; immutable table at+AD4.
  constexpr std::array<uint32_t,5> callbacks{0x33C,0x3C8,0x714,0x714,0x754};
  if(queue!=2||primary!=1||sub>=callbacks.size()||callback!=overlay+callbacks[sub]||n::word(r,overlay+0xAD4+20+4*sub)!=callback||!pointer(p,0x27C)||n::word(r,a+0x1F8)!=overlay+0x7B0||n::word(r,p+0x260)!=route){state_.reason="native_victory_pot_active_state";return false;}
  if(!pot_active_&&sub){state_.reason="native_victory_pot_first_active_state";return false;}
  if(pot_active_&&p!=pot_private_){state_.reason="native_victory_pot_private_generation";return false;}
  pot_private_=p;pot_active_=true;return true;
 }
 //The genuine rim is an ordinary4001 child:48114 removes it from the main
 //queue and47F9C installs the reciprocal parent slot. Down then61170/4816C
 //detaches it,443EC queues4, and440E4 returns2 before the deferred2C3B4.
 //Only this owned native path may pass the stricter combat player guards.
 int observe_pot_player(uint8_t* r){
  auto* rdram=r;
  const auto a=state_.player,p=state_.private_data;
  const auto queue=n::byte(r,a+0x17D),primary=n::byte(r,a+0x18),sub=n::byte(r,a+0x30),hidden=n::byte(r,a+0x165);
  const auto callback=n::word(r,a+0x20),parent=n::word(r,a+0x180),pending=n::word(r,p+0xC0);
  if(n::word(r,p+0x12EC)||n::word(r,p+0x1E0)){state_.reason="native_pot_player_gates";return -1;}
  const auto children=unsigned(MEM_HU(0x184,n::base(state_.pot)));
  unsigned populated=0,matches=0;for(unsigned i=0;i<20;++i){const auto child=n::word(r,state_.pot+0x188+4*i);populated+=child!=0;matches+=child==a;}
  if(children>20||populated!=children){state_.reason="native_pot_child_population";return -1;}
  if(primary==1&&callback==0x80043450&&!hidden){
   if(!pending&&parent==state_.pot&&queue==0&&pot_active_&&!pot_entry_){
    const auto slot=unsigned(MEM_HU(0x17E,n::base(a)));
    if(!children||matches!=1||slot>=20||n::word(r,state_.pot+0x188+4*slot)!=a||n::byte(r,a+0x17C)){state_.reason="native_pot_rim_reciprocal_slot";return -1;}
    pot_mounted_=true;return 0;
   }
   if(!parent&&!matches&&!children){
    if(pending==0x8002C3B4&&pot_active_&&pot_mounted_&&(queue==4||queue==2)&&n::word(r,p+0xD0)==state_.pot&&n::word(r,p+0xD8)==4){pot_entry_=true;return 1;}
    if(!pending&&queue==2&&!pot_entry_)return 0;
   }
  }
  //2C3B4 sets A/0+3112C, D0/1C0 to this pot and clears C0.30BDC
  //hides the actor during entry.30B7C selects A/2+31230 before5D320,
  //the exact non-death passenger already authenticated by the Runtime.
  if(pot_active_&&pot_mounted_&&!parent&&!matches&&!children&&!pending&&primary==0xA&&n::word(r,p+0xD0)==state_.pot&&n::word(r,p+0x1C0)==state_.pot&&n::word(r,p+0xD8)==4){
   if((sub==0&&callback==0x8003112C&&queue==2&&hidden<=1)||(sub==2&&callback==0x80031230&&!queue&&hidden==1)){pot_entry_=true;return 1;}
  }
  state_.reason="native_pot_player_path";return -1;
 }
 Output victory(uint8_t* r,const Frame& f,const ledger::Observer& observer,Output out){
  if(!state_.started||!update_ledger(observer,f.vi))return fail(f.vi,"causal_ledger_or_generation_mismatch");
  if(state_.hits!=6||!state_.retired[0]||!state_.retired[1]||!state_.receipts[0]||!state_.receipts[1]||f.completed!=1)return fail(f.vi,"victory_without_two_native_receipts");
  state_.receipts_latched=true;
  bool pot_pending=false;if(!observe_pot(r,f,observer,pot_pending))return fail(f.vi,state_.reason);
  const int path=observe_pot_player(r);if(path<0)return fail(f.vi,state_.reason);
  if(pot_pending){stage(Stage::Pot,f.vi,"waiting_same_native4273_constructor");return finish(r,f,neutral_axes(r,out));}
  if(path){stage(Stage::Pot,f.vi,"waiting_owned_native_pot_entry_animation");return finish(r,f,neutral_axes(r,out));}
  std::array<uint32_t,6> reserves{};if(!boss_solo0240::read_reserves(r,state_.player,reserves))return fail(f.vi,"native_reserve_graph");
  state_.stock=unsigned(std::count_if(reserves.begin(),reserves.end(),[](auto x){return x!=0;}));
  const auto egg=boss_native_egg_aim0284::observe(r,state_.player);if(!egg.player_observed)return fail(f.vi,"native_aim_identity");
  state_.held=egg.held;state_.reticle=egg.reticle;
  if(egg.held||egg.reticle||egg.held_flag){stage(Stage::Pot,f.vi,"ordinary_B_cancel_before_pot");out.buttons=0x2000|(f.vi%30<3?0x4000:0);return finish(r,f,out);}
  const float x=scalar(r,f.exit_pot+0x84),y=scalar(r,f.exit_pot+0x88);
  if(!point(x,y)||x<228||x>284||y<120||y>190)return fail(f.vi,"native_victory_pot_position");
  stage(Stage::Pot,f.vi,"ordinary_native_pot_input");boss_solo0240::drive_pot_input(r,state_.player,f.vi,x,0x4273,f.exit_pot,&out.buttons,&out.x,&out.y);return finish(r,f,out);
 }
public:
 bool configure(bool enabled,std::string token,uint64_t seed,unsigned index,unsigned physical,unsigned end_vi){
  *this=Controller{};if(!enabled||token.empty()||token.size()>128||index<1||index>8||physical!=167+index||end_vi<6000||end_vi>12000)return false;
  for(unsigned char c:token)if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_'))return false;
  token_=std::move(token);seed_=seed;state_.enabled=true;state_.index=index;state_.room=physical;state_.end_vi=end_vi;state_.stage=Stage::WaitAcquire;state_.reason="waiting_acquisition_done";return true;
 }
 void disable(){*this=Controller{};}void set_logging(bool enabled){logging_=enabled;}
 const Snapshot& snapshot()const{return state_;}
 void native_entry(uint8_t* r,const recomp_context* c,std::string_view name,const Frame& f,const ledger::Observer& observer){
  if(!r||!c||name!="func_800404C8"||!state_.enabled||!state_.started||state_.failed||state_.done||state_.receipts_latched||!scope(f)||f.phase!=core::Phase::Fighting||f.physical_room!=state_.room||f.logical_room!=19||state_.stage!=Stage::Flight||f.vi<release_vi_||f.vi-release_vi_>20||uint32_t(c->r5)!=state_.player||uint32_t(c->r31)!=0x80040630||!ledger_scope(observer))return;
  const auto a=uint32_t(c->r4);if(!pointer(a,0x27C)||!n::actor(r,a,0x4002))return;
  const auto resource=n::word(r,a+0x168),p=n::word(r,a+0x1F4);const auto birth=observer.birth(a);
  if(!birth||p!=a+0x24C||!pointer(resource,0x48)||n::byte(r,a+0x1F2)!=1||(n::word(r,resource+0x3C)>>16)!=0x4002||n::word(r,resource+0x40)||n::word(r,resource+0x44)!=0x8003B0D0||n::word(r,a+0x180))return;
  const float x=scalar(r,a+0x3C),y=scalar(r,a+0x40),px=scalar(r,state_.player+0x84),py=scalar(r,state_.player+0x88);
  if(!point(x,y)||!point(px,py)||x!=px||y!=py)return;
  if(launch_actor_){if(launch_actor_!=a||launch_birth_!=birth)fail(f.vi,"multiple_native_launches_for_release");return;}
  launch_actor_=state_.projectile=a;launch_birth_=state_.projectile_birth=birth;launch_vi_=f.vi;++state_.launches;
  if(logging_)std::fprintf(stderr,"[BossBlarggShootLaunchQA] vi=%u actor=%08X birth=%llu player=%08X member=%u origin=%.6f,%.6f actual404C8=1 ramWrites=0\n",f.vi,a,(unsigned long long)birth,state_.player,state_.target_member,x,y);
 }
 Output step(uint8_t* r,const Frame& f,const ledger::Observer& observer){
  if(!state_.enabled)return {};
  if(state_.failed||state_.done)return {true,0,0,0};
  if(!r||!scope(f)){if(state_.started)return fail(f.vi,"shoot_scope_lost");return {};}
  if(!f.acquisition_done){if(state_.started)return fail(f.vi,"acquisition_done_lost");return {};}
  if(have_vi_&&f.vi<state_.last_vi)return fail(f.vi,"VI_reversed");
  if(have_vi_&&f.vi==state_.last_vi)return cached_;
  have_vi_=true;state_.last_vi=f.vi;
  Output out{true,0,0,0};
  // This branch deliberately precedes ALL Observer, retired actor and old
  // private reads. enter_room(83) disables Observer and native cleanup can
  // release its memory. The two terminal receipts must already be latched.
  if(state_.receipts_latched&&(f.phase==core::Phase::ToRest||f.phase==core::Phase::Rest)){
   if(!pot_entry_||f.completed!=1||f.player!=state_.player||(f.physical_room!=state_.room&&f.physical_room!=83))return fail(f.vi,"latched_native_rest_scope");
   if(f.phase==core::Phase::ToRest){stage(Stage::Travel,f.vi,"ordinary_pot_transport");return finish(r,f,out);}
   if(f.physical_room!=83||f.logical_room!=83||!player(r,f,false))return fail(f.vi,"native_rest_player_mismatch");
   state_.done=true;stage(Stage::Done,f.vi,"six_causal_hits_two_receipts_native_rest83");return finish(r,f,out);
  }
  if(f.physical_room!=state_.room||f.logical_room!=19||!f.fight_ready||(f.phase!=core::Phase::Fighting&&f.phase!=core::Phase::Victory)||!ledger_scope(observer))return fail(f.vi,"native_fight_scope");
  if(!player(r,f,!state_.started))return fail(f.vi,"native_player_identity");
  const auto p=state_.private_data,a=state_.player;
  if(f.phase==core::Phase::Victory)return victory(r,f,observer,out);
  if(n::byte(r,a+0x17D)!=2||n::byte(r,a+0x165)||n::byte(r,a+0x18)==0||n::byte(r,a+0x18)==0xFF||n::byte(r,a+0x18)==0xA||n::word(r,p+0x12EC)||n::word(r,p+0x1E0)!=0)return fail(f.vi,"native_player_or_throw_mode");
  std::array<uint32_t,6> reserves{};if(!boss_solo0240::read_reserves(r,a,reserves))return fail(f.vi,"native_reserve_graph");
  state_.stock=unsigned(std::count_if(reserves.begin(),reserves.end(),[](auto x){return x!=0;}));
  const auto egg=boss_native_egg_aim0284::observe(r,a);if(!egg.player_observed)return fail(f.vi,"native_aim_identity");
  state_.held=egg.held;state_.reticle=egg.reticle;
  if(!state_.started){
   if(f.phase!=core::Phase::Fighting||f.completed||state_.stock!=6||egg.held||egg.reticle||egg.held_flag||!safe_stand(r,a,p))return fail(f.vi,"six_acquired_reserves_required");
   for(unsigned i=0;i<2;++i){const auto& m=observer.snapshot().members[i];if(!observer.initialized_member(r,i)||m.hits||m.retired||m.receipt_sent)return fail(f.vi,"initial_native_member_ledger");identities_[i]=m.identity;}
   state_.started=true;state_.first_vi=f.vi;state_.initial_stock=state_.stock;stage(Stage::WaitTarget,f.vi,"six_native4004_admitted");
  }
  if(!update_ledger(observer,f.vi))return fail(f.vi,"causal_ledger_or_generation_mismatch");
  if(!safe_stand(r,a,p))return fail(f.vi,"native_safe_centre_lost");
  if(f.vi==state_.end_vi)return fail(f.vi,"shoot_deadline");
  if(state_.stage==Stage::Flight){
   const unsigned age=f.vi-release_vi_;
   if(state_.hits>hits_at_release_){
    stage(Stage::Retraction,f.vi,"causal_hit_observed_wait_native_return");return finish(r,f,neutral_axes(r,out));
   }
   if(!launch_actor_&&age>20)return fail(f.vi,"native404C8_launch_unobserved");
   if(age>=180){++state_.misses;return fail(f.vi,"launched_egg_without_causal_hit");}
   return finish(r,f,neutral_axes(r,out));
  }
  if(state_.stage==Stage::Retraction){
   if(egg.held||egg.reticle||egg.held_flag)return finish(r,f,neutral_axes(r,out));
   if(state_.target_member<2&&state_.member_hits[state_.target_member]==3&&!state_.retired[state_.target_member])return finish(r,f,neutral_axes(r,out));
   state_.target=0;state_.target_member=2;state_.target_birth=0;stage(Stage::WaitTarget,f.vi,"waiting_next_native_open_member");
  }
  std::optional<Target> target;
  if(state_.target_member<2)target=observe_target(r,observer,state_.target_member);
  if(!target&&state_.target_member==2){
   for(unsigned i=0;i<2;++i)if(auto t=observe_target(r,observer,i);t&&(!target||t->hits<target->hits))target=t;
   if(target){state_.target_member=target->member;state_.target=target->identity.actor;state_.target_birth=target->identity.birth;stage(Stage::WaitTarget,f.vi,"real_open41EC_selected_wait_normal_dispatch");}
  }
  if(target){state_.target_x=target->x;state_.target_y=target->y;}
  if(egg.held||egg.reticle||egg.held_flag){
   if(state_.target_member>=2)return fail(f.vi,"unowned_aim_without_target");
   out.buttons=0x2000;
   if(!egg.held||!egg.reticle){if(f.vi-state_.stage_vi>120)return fail(f.vi,"native_aim_graph_timeout");return finish(r,f,neutral_axes(r,out));}
   if(state_.stage!=Stage::Aiming){aim_matches_=0;stage(Stage::Aiming,f.vi,"real4005_and4003_observed");}
   if(!target){aim_matches_=0;state_.reason="waiting_same_member_open_with_native_Z_held";return finish(r,f,neutral_axes(r,out));}
   // Primary3/BF88 has created the children before its native reticle update
   // resumes. Keep axes neutral through that genuine pending animation.
   if(n::byte(r,a+0x18)!=1)return finish(r,f,neutral_axes(r,out));
   const float px=scalar(r,a+0x84),py=scalar(r,a+0x88),dx=target->x-px,dy=target->y-py,distance=std::hypot(dx,dy);
   if(!std::isfinite(distance)||distance<8)return fail(f.vi,"native_target_distance");
   const float tx=target->x+16*dx/distance,ty=target->y+16*dy/distance;
   out=move_reticle(r,tx-egg.reticle_x,ty-egg.reticle_y,out);
   const auto flight=evaluate(r,f,*target,egg);state_.origin_x=px;state_.origin_y=py;state_.ray_x=egg.reticle_x;state_.ray_y=egg.reticle_y;state_.flight_vi=float(flight.ticks);
   const bool stopped=scalar(r,a+0xC4)==0.f;aim_matches_=flight.ready&&stopped?aim_matches_+1:0;
   state_.reason=flight.reason;
   if(aim_matches_>=3){
    ++state_.attempts;release_vi_=f.vi;hits_at_release_=state_.hits;launch_actor_=0;launch_birth_=0;launch_vi_=0;state_.projectile=0;state_.projectile_birth=0;stage(Stage::Flight,f.vi,"ordinary_hold_mode_Z_release");out.buttons=0;out=neutral_axes(r,out);
   }
   return finish(r,f,out);
  }
  if(state_.stage==Stage::Aiming)return fail(f.vi,"native_aim_ended_without_release");
  if(state_.stage==Stage::RequestAim)return hold_request(r,f,out);
  if(!target){if(state_.stage==Stage::Face)stage(Stage::WaitTarget,f.vi,"face_window_closed_before_request");state_.reason="waiting_real41EC_open";return finish(r,f,neutral_axes(r,out));}
  if(!state_.stock)return fail(f.vi,"native_reserves_exhausted_before_six_hits");
  // A genuine1/22 can still dispatch32BA0 after swallowing. Probe5893 saw
  // pressedZ there, but32BA0 never calls415FC/3BEC8. The next5894 frame
  // dispatched36298 and reached3BE44 only after the edge had disappeared.
  // License ONLY the two normal callbacks actually closed by source/trace:
  //36298->21958->415FC and364DC->21958->415FC.43450 owns this AC slot;
  // C0 is a native pending action. No other callback is guessed admissible.
  const auto input_callback=n::word(r,p+0xAC);
  const bool normal_dispatch=n::word(r,a+0x20)==0x80043450&&!n::word(r,p+0xC0)&&(input_callback==0x80036298||input_callback==0x800364DC);
  const bool request_ready=normal_dispatch&&n::byte(r,a+0x18)==1&&n::byte(r,a+0x30)!=0x14&&!n::byte(r,p+0x12C9)&&!n::byte(r,p+0x1F8)&&!n::word(r,p+0x164);
  if(state_.stage==Stage::WaitTarget){
   if(!request_ready){state_.reason="waiting_proved_normal_control_dispatch";return finish(r,f,neutral_axes(r,out));}
   stage(Stage::Face,f.vi,"normal_native_dispatch_begin_facing");
  }
  if(state_.stage==Stage::Face){
   // The admitted four ordinary native4 inputs can themselves enter the
   // movement dispatcher (actual66ffe shows1/3). Do not restart that same
   // preparation whenever it changes AC. Complete the bounded4VI, then
   // neutralize and wait for the PROVED normal dispatcher before pressedZ.
   if(f.vi-state_.stage_vi<4){if(auto v=aim::toward(target->x-scalar(r,a+0x84),0,native_axes(r),4)){out.x=v->host.x;out.y=v->host.y;}return finish(r,f,out);}
   if(!request_ready||scalar(r,a+0xC4)!=0.f){state_.reason="post_face_wait_normal_dispatch_and_stop";return finish(r,f,neutral_axes(r,out));}
   stage(Stage::RequestAim,f.vi,"first_native_Z_edge");
  }
  if(state_.stage==Stage::RequestAim)return hold_request(r,f,out);
  return finish(r,f,neutral_axes(r,out));
 }
};
} // namespace workshop::boss_blargg_shoot0289
