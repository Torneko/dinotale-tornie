#include "../boss_battle_runtime0250.hpp"
#include "../boss_driver_bridge0260.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <vector>
#include <limits>
namespace c=workshop::boss_campaign0250;namespace bridge=workshop::boss_driver_bridge0260;namespace n=workshop::boss_native;namespace v=workshop::boss_variants0250;namespace d=workshop::boss_driver0260;namespace aim=workshop::boss_fish_aim0260;
static unsigned checks=0;static void need(bool yes,const char* why){++checks;if(!yes){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
static float value(uint8_t* ram,uint32_t a){return std::bit_cast<float>(n::word(ram,a));}
static bool close(float a,float z){return std::isfinite(a)&&std::abs(a-z)<.001f;}
static void put(uint8_t* rdram,uint32_t a,uint32_t v){MEM_W(0,n::base(a))=int32_t(v);}
static void fl(uint8_t* rdram,uint32_t a,float v){put(rdram,a,std::bit_cast<uint32_t>(v));}
static void by(uint8_t* rdram,uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}
struct Case {
 std::vector<uint8_t> memory;uint32_t player=0;std::array<uint32_t,2> roots{};
 explicit Case(const char* path){std::ifstream input(path,std::ios::binary);memory.assign(std::istreambuf_iterator<char>(input),{});need(memory.size()==0x800000,"Actual Fish diagnostic capture contains8MiB");auto*rdram=memory.data();player=n::word(rdram,0x800F8828);need(n::actor(rdram,player,0x4001),"Actual capture retains the singleton native Yoshi");
  const auto q=0x800F8814u,table=n::word(rdram,q+0x10);const unsigned count=MEM_HU(8,n::base(q));need(count<=150&&n::range(table,count*4),"Locate Fish through the actual bounded native queue");
  for(unsigned i=0;i<count;++i){const auto a=n::word(rdram,table+4*i);if(n::actor(rdram,a,0x4511)&&n::byte(rdram,a+0x17D)==2)roots[0]=a;if(n::actor(rdram,a,0x4631)&&n::byte(rdram,a+0x17D)==2)roots[1]=a;}
  need(roots[0]&&roots[1],"Both live typed Fish actors come from the real source image");
 }
 void configure(){auto*rdram=memory.data();const auto before=memory;
  need(c::configure("qa-fish-motion-composite",1,true,true,4),"Real isolated diagnostic Runtime configure selects seed1/case4");c::rooms::configure(c::session.plan());c::enter_room(52);
  need(c::physical_room==171&&c::logical_room==97&&c::session.current().count==2&&c::session.current().family==workshop::boss_battle0250::Family::Fish,"Actual Runtime uses physical171/source97 and the two configured members");
  need(c::session.enter_fight(97,player),"Real Session enters Fighting with the captured Yoshi");
  for(unsigned member=0;member<2;++member){need(c::session.bind(member,roots[member]),"Real Session binds the exact member actor");c::bound[member]=roots[member];need(v::bind(rdram,member,roots[member]),"Real Variants bind supplies a new owned generation");need(v::snapshot().members[member].private_data==0&&!bridge::observe_fish_motion(rdram,member),"Initial host bind does not fabricate the Fish private pointer or authorize unobserved motion");
   // Explicit HOST init-entry observation boundary only. Original9D10/constructor
   // is NOT executed; trace consumes the already captured native private pointer.
   recomp_context context{};context.r4=n::base(roots[member]);context.r5=1;context.r6=1;v::trace(rdram,&context,"func_805C9D10",{97,2408,true,false});
   const auto& binding=v::snapshot().members[member];need(binding.birth&&binding.initialized&&binding.private_data==n::word(rdram,roots[member]+0x1F4),"Actual Variants init-observation path attests the captured private identity without snapshot mutation");
  }
  need(memory==before,"Runtime/Session/bind/init-observation preparation preserves all8MiB native bytes");
 }
 std::optional<aim::Motion> observe(unsigned member){const auto before=memory;const auto out=bridge::observe_fish_motion(memory.data(),member);need(memory==before,"Actual Bridge motion adapter is read-only across all8MiB");return out;}
};
static void actual(const char* path,bool mobile,bool full_drive){Case f(path);f.configure();auto*ram=f.memory.data();
 for(unsigned member=0;member<2;++member){const auto a=f.roots[member];const auto data=n::word(ram,a+0x1F4),resource=n::word(ram,a+0x168),meta=n::word(ram,a+0x174),def=n::word(ram,meta);const auto motion=f.observe(member);need(bool(motion),"True Bridge accepts the untouched current native Fish profile and generation");
  const float sx=value(ram,a+0x6C),sy=value(ram,a+0x70);const auto drawing=n::word(ram,a+0x16C),table=drawing?n::word(ram,drawing):0,index=n::word(ram,a+0x138);const bool mirrored=table&&n::byte(ram,a+0x164)!=7&&(n::word(ram,table+index*4)&1);
  need(motion->actor==a&&motion->birth==v::snapshot().members[member].birth,"Returned motion retains actual actor and current Variant generation");need(close(motion->x,value(ram,a+0x84)+(mirrored?30.f:-30.f)*sx)&&close(motion->y,value(ram,a+0x88)),"Returned coordinates are the native mirrored/scaled collision centre, not the spawn or raw actor origin");
  need(close(motion->vx,value(ram,a+0xC4)*value(ram,a+0x100)+value(ram,a+0x10C))&&close(motion->vy,value(ram,a+0xC8)*value(ram,a+0x104)+value(ram,a+0x110))&&motion->speed==6.6f,"Captured483B8 world motion and original4002 speed are published in the same per-update units");
  need(n::word(ram,def+0x70)==a+0x6C&&n::word(ram,resource+0x44)==n::word(ram,resource+0x40)+0x35A8&&data&&sy>0,"The positive path uses the real public Fish initializer and original scale pointer");
  if(mobile&&member==0){need(n::byte(ram,a+0x30)==15&&close(motion->vx,-2)&&close(motion->vy,-2.900002f),"Real b23 is already a mobile free-jump Fish; no synthetic jump is needed for the principal positive");d::Input in;in.x=value(ram,f.player+0x84);in.y=value(ram,f.player+0x88);d::Target target{a,0x4511,motion->x,motion->y,motion->vx,motion->vy,true,true,false,false,true,motion->speed};float x=0,y=0;d::predicted_target(in,target,x,y);const float t=(x-motion->x)/motion->vx;need(t>0&&t<60&&close(y,motion->y+motion->vy*t)&&std::abs(std::hypot(x-in.x,y-in.y)-motion->speed*t)<.01f,"Existing Fish-enabled intercept solves the real mobile centre against native6.6, rather than default14 or velocity0");}
 }
 need(!f.observe(2)&&!bridge::observe_fish_motion(nullptr,0),"Missing member and null RAM refuse motion");
 if(full_drive){bridge::configure();const auto before=f.memory;uint16_t buttons=0;float x=0,y=0;bridge::drive(ram,3001,&buttons,&x,&y);need(bridge::controller.snapshot().target==f.roots[0]&&f.memory==before,"Actual drive consumes real2c4005/4003 and selects the witnessed red centre without altering the capture");
  auto& bindings=const_cast<v::Snapshot&>(v::snapshot());const auto prior=bindings;for(auto& binding:bindings.members)binding.private_data=0;bridge::drive(ram,3002,&buttons,&x,&y);need((buttons&d::tongue_button)&&bridge::controller.snapshot().phase==d::Phase::CancelAim&&f.memory==before,"With actual4005 still held, the real Bridge failing every Fish motion witness reaches ordinary B cancellation instead of silently dropping held Z");bindings=prior;}
}
static void faults(const char* path){Case f(path);f.configure();auto*ram=f.memory.data();auto& host=const_cast<v::Snapshot&>(v::snapshot());auto& session=const_cast<workshop::boss_battle0250::Snapshot&>(c::session.snapshot());
 for(unsigned fault=0;fault<23;++fault){const auto saved_host=host;const auto saved_state=session;const auto bound=c::bound;const auto epoch=c::variant_epoch,physical=c::physical_room,logical=c::logical_room;const bool enabled=c::enabled,driver=c::driver,pending=c::restore_pending;
  switch(fault){case 0:c::enabled=false;break;case 1:c::driver=false;break;case 2:session.diagnostic=false;break;case 3:session.phase=workshop::boss_battle0250::Phase::Rest;break;case 4:c::restore_pending=true;break;case 5:c::logical_room=163;break;case 6:c::physical_room=172;break;case 7:session.index=5;break;case 8:session.defeated=1;break;case 9:host.enabled=false;break;case 10:host.invalidated=true;break;case 11:host.room=163;break;case 12:host.encounter=5;break;case 13:++c::variant_epoch;break;case 14:++session.retry_epoch;break;case 15:c::bound[0]=f.roots[1];break;case 16:host.members[0].actor=f.player;break;case 17:host.members[0].birth=0;break;case 18:host.members[0].receipt_sent=true;break;case 19:host.members[0].retire_queued=true;break;case 20:host.members[0].private_data+=4;break;case 21:session.index=9;break;case 22:c::bound[0]=0;break;}
  need(!f.observe(0),"Foreign or stale Runtime/source/member/epoch/private/generation rejects through the actual Bridge");host=saved_host;session=saved_state;c::bound=bound;c::variant_epoch=epoch;c::physical_room=physical;c::logical_room=logical;c::enabled=enabled;c::driver=driver;c::restore_pending=pending;
 }
 const auto original=f.memory;const auto a=f.roots[0],p=n::word(ram,a+0x1F4),resource=n::word(ram,a+0x168),factory=n::word(ram,resource+0x40),meta=n::word(ram,a+0x174),def=n::word(ram,meta),drawing=n::word(ram,a+0x16C),table=n::word(ram,drawing),index=n::word(ram,a+0x138);
 for(unsigned fault=0;fault<32;++fault){f.memory=original;ram=f.memory.data();switch(fault){
  case 0:put(ram,a+0x180,f.player);break;case 1:by(ram,a+0x17D,0);break;case 2:by(ram,a+0x18,0xFF);break;case 3:by(ram,a+0x165,1);break;case 4:put(ram,a+0x130,0x8003A5EC);break;case 5:put(ram,resource+0x44,factory+0x3804);break;case 6:put(ram,a+0x20,factory+4);break;case 7:put(ram,p+0x40,1);break;
  case 8:by(ram,a+0x30,13);break;case 9:by(ram,a+0x30,14);break;case 10:by(ram,a+0x30,16);break;case 11:by(ram,a+0x30,17);break;case 12:by(ram,a+0x30,18);break;case 13:put(ram,def+0x70,a+0x70);break;case 14:by(ram,def+0x74,0);break;case 15:fl(ram,a+0x6C,0);break;case 16:fl(ram,a+0x6C,5);break;case 17:fl(ram,a+0x70,std::numeric_limits<float>::quiet_NaN());break;case 18:fl(ram,a+0xC4,std::numeric_limits<float>::infinity());break;case 19:fl(ram,a+0x100,0);break;case 20:fl(ram,a+0x104,5);break;case 21:fl(ram,a+0x110,std::numeric_limits<float>::quiet_NaN());break;case 22:fl(ram,a+0xC4,17);break;case 23:fl(ram,0x800B02D8,14);break;case 24:fl(ram,0x800B02DC,7);break;case 25:put(ram,def+0x28,factory+0x414);break;case 26:put(ram,a+0x1F4,0);break;case 27:put(ram,resource+0x40,0);break;case 28:put(ram,meta,0);break;case 29:put(ram,def+4,0);break;case 30:put(ram,drawing,0x807FFFFC);put(ram,a+0x138,0xFFFFFFFF);break;case 31:fl(ram,a+0x84,std::numeric_limits<float>::quiet_NaN());break;
  }
  need(!f.observe(0),"Malformed native posture/passenger/profile/factory/shape/motion/scales/scene constant refuses prediction");
 }
 f.memory=original;ram=f.memory.data();const bool mirror=bool(n::word(ram,table+4*index)&1);put(ram,table+4*index,n::word(ram,table+4*index)^1);const auto other=f.observe(0);need(bool(other)&&close(other->x,value(ram,a+0x84)+(mirror?-30.f:30.f)*value(ram,a+0x6C)),"A real native drawing-table mirror change flips only the observed collision offset");
 f.memory=original;ram=f.memory.data();fl(ram,a+0xC4,-1.3f);fl(ram,a+0xC8,-3.7f);fl(ram,a+0x100,.7f);fl(ram,a+0x104,.8f);fl(ram,a+0x10C,.2f);fl(ram,a+0x110,-.1f);const auto scaled=f.observe(0);need(bool(scaled)&&close(scaled->vx,-.71f)&&close(scaled->vy,-3.06f),"Explicit HOST factor/external frontier exercises the original483B8 arithmetic without moving any game actor");
 need(!aim::observe(ram,a,0x4511,0)&&!aim::observe(ram,a,0x4072,1),"Missing birth and foreign actor kind cannot authorize the independent reader");
}
int main(int argc,char** argv){need(argc==5,"Supply real b2/2c/c9/b23 diagnostic captures");for(int i=1;i<argc;++i)actual(argv[i],i==4,i==2);faults(argv[4]);std::printf("BossFishAim0260Checks: %u PASS; four intact real capture pipelines including mobile b23; explicit HOST callback/fault/math frontiers only, no native game/hit/victory claim.\n",checks);}
