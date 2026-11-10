#include "../boss_battle_runtime0250.hpp"
#include "../boss_driver_bridge0260.hpp"
#include <array>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>
namespace b=workshop::boss_driver_bridge0260;
namespace d=workshop::boss_driver0260;
namespace c=workshop::boss_campaign0250;
namespace core=workshop::boss_battle0250;
namespace n=workshop::boss_native;
static unsigned checks=0;
static void need(bool yes,const char* why){++checks;if(!yes){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
 std::vector<uint8_t> bytes=std::vector<uint8_t>(0x800000);uint8_t*rdram=bytes.data();recomp_context ctx{};
 static constexpr uint32_t player=0x80100000,priv=0x80110000,egg=0x80120000,egg_private=0x80121000,resource=0x80122000,reticle=0x80130000,held=0x80140000;
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}void h(uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}void by(uint32_t a,uint8_t v){MEM_B(0,n::base(a))=v;}void f(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}
 void actor(uint32_t a,uint16_t kind){w(a,0x19971110);h(a+12,kind);by(a+0x18,1);by(a+0x17D,2);}
 Fixture(){
  // Synthetic host-session admission only: no game function, damage or award.
  need(c::session.begin("cpu-egg-trace",core::make_plan(20261031),true)&&c::session.diagnostic_entry(2)&&c::session.enter_fight(core::room(c::session.current().family),player),"Explicit QA host session enters a synthetic fight");c::enabled=c::driver=true;c::physical_room=169;b::configure();
  actor(player,0x4001);w(player+0x1F4,priv);w(0x800F8828,player);f(player+0x84,384);f(player+0x88,248);
  actor(egg,0x4002);w(egg+0x1F4,egg_private);w(egg+0x168,resource);f(egg+0x3C,384);f(egg+0x40,248);f(egg+0x84,384);f(egg+0x88,248);
  actor(reticle,0x4003);w(reticle+0x180,player);f(reticle+0x84,534);f(reticle+0x88,248);w(priv+0x1F4,reticle);
  actor(held,0x4005);w(held+0x180,player);w(priv+0x1F0,held);
  ctx.r4=n::base(egg);ctx.r5=n::base(player);ctx.r29=n::base(0x800FF000);ctx.r31=n::base(0x80040630);ctx.r6=0x12345678;ctx.f2.u32l=0xABCDEF01;
 }
 void release(){
  std::array<d::Target,1> targets{{{0x80150000,0x41C6,534,248,0,0,true,true,false,false,true,6.6f,2.f,0x80150100}}};d::Input in;
  in.player=player;in.room=169;in.encounter=2;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.reserves=6;in.x=384;in.y=248;in.min_x=40;in.max_x=724;in.targets=targets;
  in.vi=100;b::controller.step(in);in.vi=104;b::controller.step(in);in.aiming_actor=held;in.reticle_actor=reticle;in.reticle_x=534;in.reticle_y=248;
  for(unsigned vi=105;vi<=107;++vi){in.vi=vi;b::controller.step(in);}const auto& r=b::controller.release_trace();need(r.serial==1&&r.vi==107&&r.root==targets[0].actor&&r.eye==targets[0].observation_actor&&r.aim==held&&r.reticle==reticle,"Release journal captures the exact target/eye/4005/reticle identity at its front");need(r.x==384&&r.y==248&&r.target_x==534&&r.target_y==248&&r.predicted_x==534&&r.predicted_y==248&&!r.vx&&!r.vy,"Release journal captures the same intercept computation without modifying it");
  const auto before=r.serial;in.vi=107;b::controller.step(in);need(b::controller.release_trace().serial==before,"Repeated same VI cannot duplicate the release journal");
 }
};
int main(){
 {Fixture f;f.release();const auto memory=f.bytes;const auto context=f.ctx;b::trace_native_launch(f.rdram,&f.ctx,"func_800404C8",107);need(b::native_launch_logs==1&&b::native_launch_traces[0].pending&&b::native_launch_traces[0].release_serial==1,"Actual-shaped404C8 a0=4002/a1=P1 is associated with the recent observed Release");need(f.bytes==memory&&std::memcmp(&context,&f.ctx,sizeof(context))==0,"Native entry trace cannot alter any RDRAM byte or register");b::trace_native_launch(f.rdram,&f.ctx,"func_800404C8",107);need(b::native_launch_logs==1,"Repeated identical native entry within one VI is deduplicated");b::sample_native_launches(f.rdram,107);need(!b::native_launch_first_logs&&b::native_launch_traces[0].pending,"Before-velocity entry is not mislabeled as a post-call sample on the same VI");f.f(Fixture::egg+0xC4,6.6f);f.f(Fixture::egg+0xC8,-1);f.f(Fixture::egg+0x3C,390.6f);const auto after=f.bytes;b::sample_native_launches(f.rdram,108);need(b::native_launch_first_logs==1&&!b::native_launch_traces[0].pending&&f.bytes==after,"Later read-only sample records actual native output fields exactly once");b::sample_native_launches(f.rdram,109);need(b::native_launch_first_logs==1,"First4002 observation is never reported twice");}
 for(unsigned fault=0;fault<13;++fault){Fixture f;std::string_view name="func_800404C8";
  if(fault==0)c::enabled=false;if(fault==1)c::driver=false;if(fault==2)c::session.cancel("synthetic-inactive");if(fault==3)name="func_800406E0";if(fault==4)f.ctx.r5=n::base(Fixture::egg);if(fault==5)f.w(0x800F8828,Fixture::egg);if(fault==6)f.h(Fixture::egg+12,0x4005);if(fault==7)f.by(Fixture::egg+6,3);if(fault==8)f.w(Fixture::player+0x1F4,0x807FFFF0);if(fault==9)f.w(Fixture::egg+0x1F4,0);if(fault==10)f.w(Fixture::egg+0x168,0x807FFFF0);if(fault==11)f.h(Fixture::player+12,0x4004);if(fault==12){need(c::session.begin("production-trace-denied",core::make_plan(20261031),false)&&c::session.enter_fight(core::room(c::session.current().family),Fixture::player),"Synthetic production host session constructed without awards");}
  const auto before=f.bytes;const auto context=f.ctx;b::trace_native_launch(f.rdram,&f.ctx,name,100);need(!b::native_launch_logs&&f.bytes==before&&std::memcmp(&context,&f.ctx,sizeof(context))==0,"Wrong provenance/name/a0/a1/identity/ranges fail silently and read-only");
 }
 for(unsigned fault=0;fault<6;++fault){Fixture f;b::trace_native_launch(f.rdram,&f.ctx,"func_800404C8",100);if(fault==0)f.h(Fixture::egg+12,0x4005);if(fault==1)f.w(Fixture::egg+0x1F4,Fixture::egg_private+4);if(fault==2)f.w(Fixture::egg+0x168,Fixture::resource+4);if(fault==3)c::physical_room=170;const auto before=f.bytes;b::sample_native_launches(f.rdram,fault==4?99:fault==5?109:101);need(!b::native_launch_first_logs&&!b::native_launch_traces[0].pending&&f.bytes==before,"Reuse, room changes, clock rollback or expired sample never fabricate a native output observation");}
 {Fixture f;const auto before=f.bytes;for(unsigned vi=0;vi<160;++vi){b::trace_native_launch(f.rdram,&f.ctx,"func_800404C8",vi);b::sample_native_launches(f.rdram,vi+1);}need(b::native_launch_logs==128&&b::native_launch_first_logs==128&&f.bytes==before,"Both native trace streams are bounded to128 records per configured diagnostic session");b::configure();need(!b::native_launch_logs&&!b::native_launch_first_logs,"New diagnostic configuration clears the bounded journal");}
 {Fixture f;b::monster_return_motion_logs=384;b::configure();need(!b::monster_return_motion_logs,"Starting a new diagnostic session resets the bounded return-motion trace");}
 {Fixture f;const auto before=f.bytes;b::trace_native_launch(nullptr,&f.ctx,"func_800404C8",100);b::trace_native_launch(f.rdram,nullptr,"func_800404C8",100);need(!b::native_launch_logs&&f.bytes==before,"Null RAM/context are refused without touching observations");}
 std::printf("BossEggTrace0260Checks: %u PASS (synthetic QA journal admission/identity/cap/read-only8MiB and registers; no aiming changes, native gameplay, damage, ingestion or victory proof)\n",checks);
}
