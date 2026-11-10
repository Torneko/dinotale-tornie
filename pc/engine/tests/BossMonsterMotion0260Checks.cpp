// Reuse only the native-shaped memory fixture. Its legacy test main is never
// called here, and no synthetic input is claimed as an actual game hit.
#define main monster_aim_legacy_entry_not_executed
#include "BossMonsterAim0260Checks.cpp"
#undef main
#ifndef MONSTER_NATIVE_MOTION_SLICE
#error Supply the immutable literal483B8/A5EC extraction.
#endif
namespace literal {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include MONSTER_NATIVE_MOTION_SLICE
#pragma clang diagnostic pop
}
static unsigned native_calls=0;
static bool close(float a,float b){return std::abs(a-b)<.001f;}
static void frame(Fixture& f,m::History& history,unsigned vi,float x,float y=200,uint64_t birth=7){
 f.f(Fixture::child+0x84,x);f.f(Fixture::child+0x88,y);
 const auto saved=f.memory;history.native_frame(f.rdram,Fixture::root,Fixture::priv,birth,169,2,vi);
 need(f.memory==saved,"Actual-update observation does not write any native byte");
}
static std::optional<m::Eye> eye(Fixture& f,const m::History& history,unsigned vi,uint64_t birth=7){
 m::Eye current;need(m::observe_eye(f.rdram,Fixture::root,Fixture::priv,birth,Fixture::child,current),"Fixture supplies the exact native46F9 parent/callback graph");
 const auto saved=f.memory;auto value=history.observe(Fixture::root,Fixture::priv,birth,169,2,vi,current);
 need(f.memory==saved,"Sampling an already witnessed native clock remains read-only");return value;
}
static float flight(float vx,bool lead){
 Fixture f;auto* rdram=f.rdram;constexpr uint32_t egg=0x80300000,moving=0x80300400;
 d::Input in;in.x=312;in.y=302.7f;d::Target target{1,0x41C6,195.839f,267.732f,vx,0,true,true,false,false,true,6.6f,2};
 float tx=target.x,ty=target.y;if(lead)d::predicted_target(in,target,tx,ty);const float length=std::hypot(tx-in.x,ty-in.y);
 f.f(egg+0x3C,in.x);f.f(egg+0x40,in.y);f.f(egg+0xC4,6.6f*(tx-in.x)/length);f.f(egg+0xC8,6.6f*(ty-in.y)/length);
 f.f(moving+0x3C,target.x);f.f(moving+0x40,target.y);f.f(moving+0xC4,vx);f.f(moving+0xC8,0);
 auto value=[&](uint32_t a){return std::bit_cast<float>(n::word(rdram,a));};float best=1e9f;recomp_context c{};
 for(unsigned update=0;update<60;++update){best=std::min(best,std::hypot(value(egg+0x3C)-value(moving+0x3C),value(egg+0x40)-value(moving+0x40)));
  for(auto a:{egg,moving}){c.r4=n::base(a);literal::func_8003A5EC(rdram,&c);++native_calls;}
 }
 return best;
}
int main(){
 checks=0;
 {Fixture f;m::History history;f.w(Fixture::root+0x130,0x800483B8);
  frame(f,history,100,100);need(!eye(f,history,100),"One native point cannot invent motion");
  frame(f,history,102,102);need(!eye(f,history,102),"Two native points do not satisfy the three-update witness");
  frame(f,history,103,104);const auto first=eye(f,history,103);need(first&&close(first->vx,2)&&close(first->vy,0),"Intervening VI are not denominators of native projectile-step velocity");
  for(unsigned vi:{104u,105u,106u}){const auto paused=eye(f,history,vi);need(paused&&close(paused->vx,2),"Controller observations without483B8 never overwrite motion with zero");}
  frame(f,history,103,106);const auto repeated=eye(f,history,104);need(repeated&&close(repeated->vx,2),"Duplicate same-VI native observation cannot add a fake movement sample");
  frame(f,history,105,106);frame(f,history,106,108);frame(f,history,107,106);const auto reversed=eye(f,history,107);need(reversed&&close(reversed->vx,-2),"A native reversal supersedes a positive historical average");
  need(!eye(f,history,116),"A motion witness older than eight VI is not a current target");
  need(!eye(f,history,107,8),"A new actor birth cannot consume an old native movement history");
  frame(f,history,108,108,200,8);need(!eye(f,history,108,8),"New birth starts a fresh three-update witness");
 }
 for(unsigned fault=0;fault<6;++fault){Fixture f;m::History history;f.w(Fixture::root+0x130,0x800483B8);frame(f,history,1,100);frame(f,history,2,102);frame(f,history,3,104);
  if(fault==0)f.w(Fixture::root+0x130,0x8003A5EC);if(fault==1)f.w(Fixture::child+0x180,0);if(fault==2)f.w(Fixture::root+0x1F4,0);if(fault==3)f.b(Fixture::root+0x18,0xFF);
  if(fault<4){frame(f,history,12,106);m::Eye e{Fixture::child,Fixture::metadata,Fixture::definition,Fixture::child_resource,106,200};need(!history.observe(Fixture::root,Fixture::priv,7,169,2,12,e),"Invalid native callback/graph cannot refresh stale movement proof");}
  if(fault==4){frame(f,history,20,106);need(!eye(f,history,20),"Sparse actual native updates restart the clock witness");}
  if(fault==5){frame(f,history,4,200);need(!eye(f,history,4),"A teleport is not admitted as an eye velocity");}
 }
 {Fixture f;m::History history;f.w(Fixture::root+0x130,0x800483B8);d::Input input;input.x=312;input.y=302.7f;
  frame(f,history,1,195.839f,267.732f);auto target=m::choose(f.rdram,history,Fixture::root,Fixture::priv,7,169,2,1,input,true);need(target&&!target->vulnerable,"Before a native motion witness the real eye remains a navigation target, not a shot");
  frame(f,history,2,193.839f,267.732f);frame(f,history,4,191.839f,267.732f);target=m::choose(f.rdram,history,Fixture::root,Fixture::priv,7,169,2,5,input,true);need(target&&target->vulnerable&&close(target->vx,-2)&&target->native_speed==6.6f,"Actual native-step motion reaches the real target used by the existing6.6 interception solver");
 }
 // Captured shot9 origin/target, with an explicit synthetic constant motion.
 // The actual native projectile integrator proves the timing unit, not a hit
 // or an assertion about the unrecorded future Monster trajectory in1e98.
 const float led=flight(-3.5f,true),static_aim=flight(-3.5f,false);std::printf("LiteralA5EC closest distances: led=%.6f static=%.6f\n",led,static_aim);
 need(led<4&&static_aim>30,"LiteralA5EC demonstrates why native-update lead matters for an explicit moving-eye fixture");
 std::printf("BossMonsterMotion0260Checks: %u PASS (%u literal native A5EC calls; explicit motion fixtures, no game hit/victory claim)\n",checks,native_calls);
}
