// STAGED ONLY: not compiled or run while the current GPU source is frozen.
#include "boss_rest_aim0260.hpp"
#include "boss_native_mouth0260.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
namespace r=workshop::boss_rest_aim0260;namespace s=workshop::boss_stick_aim0260;namespace n=workshop::boss_native;
static unsigned checks=0;static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static bool neutral(r::Output o){return o.handled&&!o.buttons&&!o.x&&!o.y;}
static bool extending(r::Output o){return o.handled&&o.buttons==r::B&&std::isfinite(o.x)&&std::isfinite(o.y);}
int main(int argc,char**argv){try{
 need(argc==2,"Failed2ef829 native snapshot path required");std::ifstream file(argv[1],std::ios::binary);need(bool(file),"Read existing native snapshot");std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),{});need(bytes.size()==0x800000,"Exact8MiB native snapshot");const auto before=bytes;auto* rdram=bytes.data();
 const auto player=n::word(rdram,0x800F8828);std::array<float,80> xs{},ys{};s::Mouth mouth;need(workshop::boss_native_mouth0260::observe_native_mouth(rdram,player,xs,ys,mouth),"Exact existing native mouth observer accepts captured player");
 r::Input base;base.vi=100;base.room=83;base.visit=2;base.player=player;base.fruit=0x803C7B14;base.active=base.controls=base.alive=base.grounded=base.fruit_available=base.stick_observed=true;base.health=5;base.tongue_observed=base.tongue_live=true;base.max_reach=*workshop::boss_tongue_geometry0260::max_centre_reach(mouth.scale_x,.001f,false);base.heading_degrees=0;base.x=281.005f;base.y=433;base.fruit_x=313;base.fruit_y=368;base.min_x=40;base.max_x=724;base.mouth=mouth;
 const auto ray=s::from_mouth(mouth,base.x,base.y,base.fruit_x,base.fruit_y);need(ray&&ray->distance<=90&&ray->up>0,"Captured banana is reachable upward from actual native mouth, no jump needed");
 need(s::native_axes(s::convert({.35f,0}),{0,0}).x==0,"Regression fixture proves old raw.35 trap stays inside native deadzone");
 {r::Controller c;need(c.configure(true,true),"Diagnostic-only rest controller");auto in=base;in.mouth_phase=2;
  for(unsigned i=0;i<240;i++){in.vi=100+i;need(extending(c.step(in)),"Observed159=2 holds B with neutral axes until native extension ends");}need(c.snapshot().phase==r::Phase::Tongue,"Active tongue never counted as held food");
  in.vi++;in.mouth_phase=0;need(neutral(c.step(in)),"Actual tongue end gets a clean release/prepare frame");
  unsigned b_edges=0;for(unsigned i=0;i<12&&!b_edges;i++){in.vi++;auto out=c.step(in);need(out.handled&&!(out.buttons&r::A),"Reachable captured banana never requires jumping");const auto filtered=s::native_axes(s::convert({out.x,out.y}),in.old_stick);if(out.buttons&r::B){++b_edges;need(filtered.y>0,"Actual B edge has upward native direction, never old horizontal miss");const float cosine=(filtered.x*ray->forward+filtered.y*ray->up)/(std::hypot(float(filtered.x),float(filtered.y))*ray->distance);need(cosine>=.9995f,"True native-filtered tongue angle reaches real fruit ray");}in.old_stick=filtered;}
  need(b_edges==1,"One normal B press, not a periodic tongue loop");in.mouth_phase=2;for(unsigned i=0;i<120;i++){in.vi++;need(extending(c.step(in)),"After accepted B, observed159=2 maintains its native extension without axes or swallowing");}
 }
 {r::Controller c;c.configure(true,true);auto in=base;in.mouth.right=!mouth.right;auto out=c.step(in);need(!out.buttons,"Wrong facing receives only a small real native turn");const auto f=s::native_axes(s::convert({out.x,out.y}),in.old_stick);need(f.x>0&&f.x<=8,"Facing uses nonzero filtered native axis4 rather than raw.35");}
 {r::Controller c;c.configure(true,true);auto in=base;in.held_valid=true;auto out=c.step(in);need(!out.buttons&&out.y<0,"Only a real held actor permits Down swallowing");in.vi++;in.grounded=false;need(neutral(c.step(in)),"Airborne held actor is never forcibly swallowed");}
 for(unsigned reset=0;reset<4;reset++){r::Controller c;c.configure(true,true);auto in=base;c.step(in);in.vi=101;c.step(in);if(reset==0)in.visit++;if(reset==1)in.player+=0x280;if(reset==2)in.fruit+=0x280;if(reset==3)in.vi=2;else in.vi++;const auto out=c.step(in);need(!out.buttons,"Visit/player/fruit/clock changes discard old prepared B edge");}
 {r::Controller c;c.configure(true,true);auto in=base;const auto out=c.step(in);const auto again=c.step(in);need(out.buttons==again.buttons&&out.x==again.x&&out.y==again.y,"SameVI result is idempotent");}
 {r::Controller c;c.configure(true,true);auto in=base;in.health=7;need(!c.step(in).handled,"Sufficient native health returns control to existing egg/pot pilot");in.vi++;in.health=5;in.fruit_available=false;need(!c.step(in).handled,"No genuine available tree fruit cannot manufacture a target");}
 {r::Controller c;need(!c.configure(true,false),"Production sessions cannot enable staged QA controller");need(!c.step(base).handled,"Manual input remains untouched outside diagnostic mode");}
 // Reproduce real left-facing movement feedback: without an accepted B,
 // the filtered horizontal axis changes facing. Positive relative tongue
 // axes must never be emitted on those preparation/movement frames.
 for(int old_x:{-54,0,54})for(int old_y:{-54,0,54}){
  r::Controller c;c.configure(true,true);auto in=base;in.mouth.right=false;in.fruit_x=in.x-32;in.old_stick={old_x,old_y};unsigned edge=0;
  for(unsigned frame=0;frame<16&&!edge;frame++){
   in.vi=100+frame;const bool left_before=!in.mouth.right;const auto before_ray=s::from_mouth(in.mouth,in.x,in.y,in.fruit_x,in.fruit_y);const auto out=c.step(in);const auto filtered=s::native_axes(s::convert({out.x,out.y}),in.old_stick);
   need(!(out.buttons&r::A),"Reachable left banana never uses a jump to hide bad facing");
   if(out.buttons&r::B){edge=in.vi;need(left_before&&before_ray.has_value(),"One accepted B is emitted only while the observed native mouth still faces left");need(filtered.x>0,"Left tongue uses a positive relative-forward vector on B edge only");const auto length=std::hypot(float(filtered.x),float(filtered.y));need(length>0&&(filtered.x*before_ray->forward+filtered.y*before_ray->up)/(length*before_ray->distance)>=.9995f,"Actual filtered B direction matches left mouth ray");}
   else{
    if(left_before)need(out.x==0&&out.y==0,"Correct left facing receives two neutral prepare VI, never positive movement axes");
    if(filtered.x)in.mouth.right=filtered.x>0;
   }
   in.old_stick=filtered;
  }
  need(edge!=0,"Left-facing native feedback converges to B instead of Face/Prepare flip loop");
 }
 // Native history can start at any already-filtered stick vector. A new
 // B edge is forbidden until the inverse actually reaches the fruit angle.
 for(int old_x:{-54,-27,0,27,54})for(int old_y:{-54,-27,0,27,54}){
  r::Controller c;c.configure(true,true);auto in=base;in.old_stick={old_x,old_y};unsigned edges=0;
  for(unsigned frame=0;frame<8&&!edges;frame++){in.vi=100+frame;const auto out=c.step(in);need(!(out.buttons&r::A),"Captured reachable fruit needs no A for any native input history");const auto filtered=s::native_axes(s::convert({out.x,out.y}),in.old_stick);if(out.buttons&r::B){++edges;const float length=std::hypot(float(filtered.x),float(filtered.y));need(length>0&&float(filtered.x*ray->forward+filtered.y*ray->up)/(length*ray->distance)>=.9995f,"B requires actual filtered direction even with opposing native history");}in.old_stick=filtered;}
  need(edges==1,"Every legal old-stick fixture reaches one normal B edge without forcing native state");
 }
 {r::Controller c;c.configure(true,true);auto in=base;in.fruit_x=in.x+40;in.fruit_y=in.y-130;
  auto out=c.step(in);need(!out.buttons&&out.x>0,"Far high fruit first approaches24px horizontal offset instead of retreating to48px trap");
  in.x+=16;in.vi++;out=c.step(in);need(!out.buttons&&c.snapshot().phase==r::Phase::Jump,"Observed closer position requests an ordinary jump only when mouth ray is still too far");
  in.vi++;out=c.step(in);need(out.buttons==r::A&&!(out.buttons&r::B),"Out-of-reach jump starts with A alone");
  in.vi=110;out=c.step(in);need(out.buttons==r::A&&!c.snapshot().air_seen,"Elapsed A input cannot fake an airborne observation");
  in.grounded=false;in.y=base.y-65;in.vi=114;out=c.step(in);need(!out.buttons&&c.snapshot().phase==r::Phase::Prepare&&c.snapshot().air_seen,"Actual airborne posture and reachable ray release A before preparing B");
  for(unsigned vi=115;vi<=116;vi++){in.vi=vi;out=c.step(in);need(!(out.buttons&r::A),"A never overlaps a later tongue edge");in.old_stick=s::native_axes(s::convert({out.x,out.y}),in.old_stick);}
 }
 {r::Controller c;c.configure(true,true);auto in=base;unsigned requested=0;for(unsigned frame=0;frame<8&&!requested;frame++){in.vi=100+frame;auto out=c.step(in);in.old_stick=s::native_axes(s::convert({out.x,out.y}),in.old_stick);if(out.buttons&r::B)requested=in.vi;}need(requested!=0,"Blocked-B fixture first requests one real edge");
  in.vi=requested+1;need(neutral(c.step(in))&&c.snapshot().phase==r::Phase::Prepare,"Proven native idle after rejected B releases and prepares immediately");
  in.vi=requested+2;need(neutral(c.step(in)),"Proven idle keeps the second neutral prepare VI");
  in.vi=requested+3;need(c.step(in).buttons==r::B,"Proven idle retries a ready request without a90VI guessed wait");
 }
 // Explicit goal keeps real HP intact. Default Rest retains both its
 // original room83 scope and stop-at7 behavior; Candy is opt-in only.
 static_assert(r::Input{}.goal==r::Goal::HealToSeven);
 static_assert(r::Input::Goal::ConsumeOwnedCandy==r::Goal::ConsumeOwnedCandy);
 {r::Controller c;c.configure(true,true);auto in=base;in.health=8;need(!c.step(in).handled,"Default Rest atHP8 still passes to native egg/pot input");in.vi++;in.room=162;in.health=5;need(!c.step(in).handled,"Default healing goal cannot control Candy arena");}
 for(bool left:{false,true})for(unsigned room:{162u,168u,176u}){
  r::Controller c;c.configure(true,true);auto in=base;in.goal=r::Goal::ConsumeOwnedCandy;in.room=room;in.health=8;in.fruit=0x802F2E24;in.x=40.40f;in.y=449;in.fruit_x=left?14.03f:66.77f;in.fruit_y=385.f;in.mouth.right=!left;unsigned edge=0;
  const auto candy_ray=s::from_mouth(in.mouth,in.x,in.y,in.fruit_x,in.fruit_y);need(candy_ray&&candy_ray->distance<=90,"In-range Candy ground ray is reachable under observed native centre bound");
  for(unsigned frame=0;frame<12&&!edge;frame++){
   in.vi=100+frame;const bool facing=in.mouth.right;const auto out=c.step(in);const auto filtered=s::native_axes(s::convert({out.x,out.y}),in.old_stick);need(out.handled,"Explicit owned Candy goal remains active at realHP8");need(!(out.buttons&r::A),"Reachable Candy does not need fake jump or health manipulation");
   if(out.buttons&r::B){edge=in.vi;need(facing==!left,"Candy B edge keeps observed right/left facing");const auto size=std::hypot(float(filtered.x),float(filtered.y));need(size>0&&(filtered.x*candy_ray->forward+filtered.y*candy_ray->up)/(size*candy_ray->distance)>=.9995f,"Candy uses same real filtered mouth angle as Rest");}
   else if(filtered.x)in.mouth.right=filtered.x>0;
   in.old_stick=filtered;need(in.health==8,"Actual full health is never falsified to obtain Candy controls");
  }
  need(edge!=0,"Explicit Candy goal emits one real B atHP8");in.mouth_phase=2;for(unsigned frame=0;frame<30;frame++){in.vi++;need(extending(c.step(in)),"Candy observed extension holds B with neutral axes even atHP8");}
  in.vi++;in.held_valid=true;in.grounded=true;const auto out=c.step(in);need(!out.buttons&&out.y<0,"Genuine held Candy alone permits native Down swallowing atHP8");
 }
 // Only the extending native phase licenses B maintenance. Retraction,
 // impact/capture and a callback without extension never keep B pressed.
 for(unsigned phase:{1u,3u,4u,255u}){r::Controller c;c.configure(true,true);auto in=base;in.mouth_phase=phase;need(neutral(c.step(in)),"Only observed159=2 may hold B; every other native phase releases");}
 {r::Controller c;c.configure(true,true);auto in=base;in.callback_pending=true;need(neutral(c.step(in)),"Callback alone never extends a tongue or manufactures a held actor");in.vi++;in.mouth_phase=2;in.held_valid=true;const auto o=c.step(in);need(!o.buttons&&o.y<0,"Real held food overrides extension with native swallowing input");}
 static_assert(r::Input{}.request_ready);
 {r::Controller c;c.configure(true,true);auto in=base;in.request_ready=false;
  for(unsigned vi=100;vi<112;vi++){in.vi=vi;const auto o=c.step(in);need(neutral(o),"Unready native request never emits relative axes or new B");need(c.snapshot().phase!=r::Phase::Tongue&&c.snapshot().request_vi==0,"Unready B never starts a Tongue state or90VI retry timer");in.old_stick=s::native_axes(s::convert({o.x,o.y}),in.old_stick);}
  in.vi++;in.request_ready=true;need(c.step(in).buttons==r::B,"New native readiness permits the already prepared true B edge");}
 for(auto goal:{r::Goal::HealToSeven,r::Goal::ConsumeOwnedCandy}){r::Controller c;c.configure(true,true);auto in=base;in.goal=goal;if(goal==r::Goal::ConsumeOwnedCandy){in.room=162;in.health=8;}in.request_ready=false;in.mouth_phase=2;need(extending(c.step(in)),"Unready new request cannot cut an already extending native tongue");in.vi++;in.held_valid=true;const auto o=c.step(in);need(!o.buttons&&o.y<0,"Unready new request cannot block swallowing a real held actor");in.vi++;in.held_valid=false;in.mouth_phase=0;in.callback_pending=true;need(neutral(c.step(in)),"Unready callback does not fabricate a B request");}
 {r::Controller c;c.configure(true,true);auto in=base;in.tongue_observed=false;in.mouth_phase=2;need(neutral(c.step(in)),"Unknown tongue graph cannot maintain stale raw phase2");in.vi++;in.mouth_phase=0;need(neutral(c.step(in)),"Unknown tongue graph cannot admit a new B even with raw phase0");}
 {r::Controller c;c.configure(true,true);auto in=base;in.mouth_phase=2;need(extending(c.step(in)),"Live graph extends before native damage");in.vi++;in.tongue_live=false;in.tongue_cancelled_idle=true;need(neutral(c.step(in)),"Proven native cancellation releases B immediately without rewriting raw2");need(c.snapshot().phase==r::Phase::Prepare&&c.snapshot().request_vi==0,"Cancelled native tongue discards its old retry wait");in.vi++;need(neutral(c.step(in)),"Cancellation prepares on a clean release frame");in.vi++;need(c.step(in).buttons==r::B,"Known idle cancellation may request fresh B while raw159 remains2");need(in.mouth_phase==2,"Helper preserves the raw native phase");}
 for(bool left:{false,true}){r::Controller c;c.configure(true,true);auto in=base;in.goal=r::Goal::ConsumeOwnedCandy;in.room=162;in.health=8;in.mouth.right=!left;in.fruit_x=in.x+(left?-160.f:160.f);in.fruit_y=in.y;const auto o=c.step(in);need(o.handled&&!o.buttons&&o.x==(left?-.65f:.65f)&&o.y==0,"Only far Candy approach uses ordinary global .65 movement");}
 {r::Controller c;c.configure(true,true);auto in=base;in.goal=r::Goal::ConsumeOwnedCandy;in.room=162;in.health=8;in.x=40.4f;in.y=449;in.fruit_x=66.77f;in.fruit_y=359.89f;in.mouth.right=true;const auto ray=s::from_mouth(in.mouth,in.x,in.y,in.fruit_x,in.fruit_y);need(ray&&ray->distance>in.max_reach&&ray->distance<90,"Observed88px target is beyond actual71px tongue centre reach");need(!c.step(in).buttons&&c.snapshot().phase==r::Phase::Jump,"Out-of-range Candy prepares a real jump instead of an impossible B");}
 for(float bad:{0.f,-1.f,std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity()}){r::Controller c;c.configure(true,true);auto in=base;in.max_reach=bad;need(neutral(c.step(in)),"Invalid or absent observed reach fails closed");}
 for(bool left:{false,true}){r::Controller c;c.configure(true,true);auto in=base;in.mouth.right=!left;in.fruit_available=false;in.fruit=0;in.mouth_phase=2;in.heading_degrees=65;in.heading_active=true;const auto o=c.step(in);need(o.buttons==r::B&&o.y>0&&(left?o.x<0:o.x>0),"Live tongue fallback uses observed heading in WORLD axes on both sides");in.vi++;in.mouth_phase=1;const auto retract=c.step(in);need(!retract.buttons&&retract.y>0&&(left?retract.x<0:retract.x>0),"Retraction retains live EA4 heading without holding B");}
 for(unsigned invalid:{2u,3u,255u}){r::Controller c;c.configure(true,true);auto in=base;in.goal=static_cast<r::Goal>(invalid);in.room=162;in.health=8;in.held_valid=true;need(neutral(c.step(in)),"Invalid goal enum cannot enable eating, aiming or swallowing");}
 for(unsigned room:{83u,161u,177u,999u}){r::Controller c;c.configure(true,true);auto in=base;in.goal=r::Goal::ConsumeOwnedCandy;in.room=room;in.health=8;need(neutral(c.step(in)),"Candy goal is rejected outside proven native Boss arena range");}
 {r::Controller c;c.configure(true,true);auto in=base;c.step(in);in.vi=101;c.step(in);in.vi=102;in.goal=r::Goal::ConsumeOwnedCandy;in.room=162;in.health=8;const auto out=c.step(in);need(!out.buttons&&c.snapshot().goal==r::Goal::ConsumeOwnedCandy,"Goal change discards prepared Rest B edge rather than inheriting it in Candy");}
 need(bytes==before,"Observer/controller never writes native RAM");std::cout<<"BossRestAim0260 "<<checks<<" staged CPU fixtures passed; no native physical ingestion proof\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
