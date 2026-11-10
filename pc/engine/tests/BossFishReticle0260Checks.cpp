// Narrow read-only Bridge/Controller composition plus original4003 cursor code.
// Poses2509/2525/2541 are explicit HOST reconstructions from the7821 journal
// on its unchanged8MiB diagnostic capture. No scheduler, physics or hit claim.
#define FISH_RECOVERY_FIXTURE_ONLY
#include "BossFishRecovery0260Checks.cpp"
#undef FISH_RECOVERY_FIXTURE_ONLY
static unsigned cursor_entries=0,math_entries=0;
void do_break(uint32_t){std::abort();}
namespace cursor_literal {
void func_800390F0(uint8_t*,recomp_context*);void func_80039338(uint8_t*,recomp_context*);
// Explicit HOST math services only. They do not select/control any actor.
void fatan2(uint8_t*,recomp_context* c){++math_entries;c->f0.fl=std::atan2(c->f12.fl,c->f14.fl);}
void Math_FFloorF(uint8_t*,recomp_context* c){++math_entries;c->f0.fl=std::floor(c->f12.fl);}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#include "native-reticle.inc"
#pragma clang diagnostic pop
}
struct ReticleCase {
 std::vector<uint8_t> memory;uint8_t* ram;uint32_t player=0,priv=0,reticle=0,data=0,red=0,blue=0;
 explicit ReticleCase(const std::vector<uint8_t>& source):memory(source),ram(memory.data()){
  player=n::word(ram,0x800F8828);priv=n::word(ram,player+0x1F4);reticle=n::word(ram,priv+0x1F4);data=n::word(ram,reticle+0x1F4);
  need(n::actor(ram,player,0x4001)&&n::actor(ram,reticle,0x4003)&&n::word(ram,reticle+0x180)==player,"Actual7821 singleton and its real reciprocal4003 parent are preserved");
  need(n::actor(ram,n::word(ram,priv+0x1F0),0x4005)&&fl(priv+0x1EC)==1,"Actual7821 owns native4005 and the cursor Y factor is exactly1");
  const auto queue=0x800F8814u,table=n::word(ram,queue+0x10);const auto cap=inv::half(ram,queue+8);need(cap<=150,"Actual native queue is bounded before locating the two live Fish");
  for(unsigned i=0;i<cap;++i){const auto a=n::word(ram,table+4*i);if(n::actor(ram,a,0x4511)&&n::byte(ram,a+0x17D)==2)red=a;if(n::actor(ram,a,0x4631)&&n::byte(ram,a+0x17D)==2)blue=a;}
  need(red&&blue,"Actual7821 contains both original live Fish identities");
 }
 float fl(uint32_t a)const{return std::bit_cast<float>(n::word(ram,a));}
 void set(uint32_t a,float v){word(ram,a,std::bit_cast<uint32_t>(v));}
 void publish(){
  set(reticle+0x84,fl(player+0x84)+fl(reticle+0x3C));set(reticle+0x88,fl(player+0x88)+fl(reticle+0x40));
  const auto camera=n::word(ram,0x800F8294),view=n::word(ram,0x800F8318);const float a=120.f/fl(camera+4),projection=a/(std::floor(fl(reticle+0x8C))-fl(camera+0x10)+a);
  word(ram,reticle+0x34,uint32_t(int32_t(std::floor(160.f+(fl(reticle+0x84)-fl(view+0x30))*projection))));word(ram,reticle+0x38,uint32_t(int32_t(std::floor(120.f+(fl(reticle+0x88)-fl(view+0x34))*projection))));
 }
 void pose(float py,float rx,float ry,bool air){
  // HOST reconstruction of logged coordinates only, not a game operation.
  set(player+0x84,356.70654f);set(player+0x88,py);set(player+0xC4,0);byte(ram,player+0x18,1);byte(ram,priv+0xF4,air);word(ram,priv+0xC0,0);
  set(red+0x84,397);set(red+0x88,338);byte(ram,red+0x18,1);
  set(reticle+0x3C,rx-fl(player+0x84));set(reticle+0x40,ry-py);publish();
 }
 void axes(stick::Axes a){half(ram,0x800F897E,uint16_t(a.x));half(ram,0x800F8980,uint16_t(a.y));}
 void native_cursor(){
  const auto before=memory;recomp_context context{};context.r29=n::base(0x807EF000);context.r4=n::base(priv);
  cursor_literal::func_80039338(ram,&context);++cursor_entries;
  need(uint32_t(context.r29)==0x807EF000,"Original39338/390F0 restores the native stack");
  auto masked=memory;
  auto allow=[&](uint32_t a,unsigned count){const auto start=a&0x7FFFFFu;std::copy_n(before.begin()+start,count,masked.begin()+start);};
  allow(0x807EEE00,0x300);allow(reticle+0x3C,8);allow(reticle+0x50,4);allow(data+0xC,8);
  need(masked==before,"Original cursor writes only its localXY/angle, private previousXY and scratch; HP/FIFO/player/fish are untouched");
  // HOST publication boundary only: actual4003 is parented to this same P1.
  // Original47C38/scheduler is NOT executed or claimed in this compact oracle.
  publish();
 }
};
static unsigned begin_tracking(Control& f,uint32_t red){
 f.targets[0]={red,0x4511,397,338,0,0,true,true,false};const auto epoch=f.cooldown();
 f.in.primary_state=1;f.in.aiming_actor=0x800C6AB4;f.in.reticle_actor=0x800C3D6C;f.in.reticle_x=f.in.x-90;f.in.reticle_y=f.in.y;
 f.at(epoch+66);f.at(epoch+67);need(f.controller.snapshot().phase==d::Phase::TrackAim,"HOST prior control history reaches real TrackAim without fabricating a release");return epoch+68;
}
int main(int argc,char** argv){try{
 checks=0;need(argc==2,"Supply the actual7821 unfenced diagnostic capture");const auto source=load(argv[1]);
 for(float scale:{1.f,.5f}){ReticleCase f(source);f.pose(273,360,330,false);f.set(f.priv+0x1EC,scale);f.axes({18,-27});const float x=f.fl(f.reticle+0x3C),y=f.fl(f.reticle+0x40);f.native_cursor();need(std::abs(f.fl(f.reticle+0x3C)-x-2)<.0001f&&std::abs(f.fl(f.reticle+0x40)-y-3*scale)<.0001f,"True39338/390F0 translates actual4003 by axes/9 and its native private1EC factor");}
 struct Pose{const char* name;float py,rx,ry;bool air;};
 for(const auto pose:{Pose{"2509",273,318.92877f,422.38f,false},Pose{"2525",220.50002f,339.151f,369.88998f,true},Pose{"2541",259.13995f,356.03989f,408.24399f,true}}){
  ReticleCase f(source);f.pose(pose.py,pose.rx,pose.ry,pose.air);Control prior(f.player);unsigned vi=begin_tracking(prior,f.red);
  need(r::configure("FishReticleComposite",1,true,true,4),"Explicit HOST isolated diagnostic Runtime");r::rooms::configure(r::session.plan());r::enter_room(52);need(r::session.enter_fight(97,f.player),"Real Session owns source97 and the captured same player");r::bound={f.red,f.blue,0,0};bridge::configure(false);bridge::controller=prior.controller;
  // Initial old native axes are the actual logged values, not zeroed filters.
  stick::Axes old=pose.air?stick::Axes{8,-27}:stick::Axes{16,-30};if(pose.py>250&&pose.air)old={11,-26};f.axes(old);
  bool released=false;unsigned updates=0;
  for(;updates<90;++updates,++vi){const auto before=f.memory;uint16_t buttons=0;float x=0,y=0;bridge::drive(f.ram,vi,&buttons,&x,&y);
   need(f.memory==before,"ACTUAL Bridge target/reticle observation and real Controller emit only ordinary input, preserving all8MiB");
   uint16_t again_buttons=0;float again_x=0,again_y=0;bridge::drive(f.ram,vi,&again_buttons,&again_x,&again_y);need(equal({true,buttons,x,y},{true,again_buttons,again_x,again_y}),"Repeated sameVI actual Bridge observation preserves the ordinary command without fabricating another matched sample");
   need(bridge::controller.snapshot().target==f.red,"Composition retains the actual selected red Fish identity");
   if(updates==0)need(y>0&&x>0,"Logged below-left cursor commands ordinary RIGHT/UP feedback, instead of the old down-right absolute ray");
   if(bridge::controller.snapshot().phase==d::Phase::Release){released=true;const float px=f.fl(f.player+0x84),py=f.fl(f.player+0x88),rx=f.fl(f.reticle+0x84)-px,ry=f.fl(f.reticle+0x88)-py,tx=397-px,ty=338-py;
    need(std::hypot(rx,ry)>=std::hypot(tx,ty)&&(rx*tx+ry*ty)/(std::hypot(rx,ry)*std::hypot(tx,ty))>=.996f,"Existing release is licensed only after the actual native cursor has sufficient range and original angular tolerance");break;}
   need(bridge::controller.snapshot().phase==d::Phase::TrackAim,"The bounded native-cursor exercise cannot hide a cancellation/timeout as successful alignment");
   const auto raw=stick::convert({x,y});old={native_x({true,buttons,x,y},old),stick::filter(raw.y,old.y)};f.axes(old);f.native_cursor();
  }
  need(released&&updates<90,"Each real logged cursor pose reaches existing release through ordinary feedback within its original120VI timeout");
  std::cout<<"pose="<<pose.name<<" nativeCursorUpdates="<<updates<<" finalXY="<<f.fl(f.reticle+0x84)<<','<<f.fl(f.reticle+0x88)<<'\n';
 }
 std::cout<<"BossFishReticle0260Checks: "<<checks<<" PASS; "<<cursor_entries<<" original39338/390F0 entries, "<<math_entries<<" explicit HOST math services; no projectile, hit, landing or victory claim.\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
