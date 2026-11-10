#include "../boss_battle_runtime0250.hpp"
#include "../boss_driver_bridge0260.hpp"
#define main monster_aim_fixture_main_not_executed
#include "BossMonsterAim0260Checks.cpp"
#undef main
#include "../boss_monster_burst0260.hpp"
namespace b=workshop::boss_monster_burst0260;
[[maybe_unused]] static b::Scene scene(){b::Scene s;s.x=100;s.y=200;s.z=500.339f;s.burst_x=s.burst_y=36;s.burst_z=30;s.egg.half_x=11.2f;s.egg.half_y=12.8f;s.egg.half_z=16;s.count=1;s.eyes[0]={1,10,200,200,500.3f,8,8,20,0,0};return s;}
struct Reader:Fixture {
 static constexpr uint32_t player=0x80300000,pp=0x80301000,reserve=0x80310000,ep=0x80311000,bp=0x80311100,overlay=0x80350000;
 m::History history;std::array<b::Root,1> roots{{{root,priv,7}}};
 Reader(){memory.resize(workshop::boss_egg_shape0260::required_capacity);rdram=memory.data();
  actor(player,0x4001);w(0x800F8828,player);w(player+0x1F4,pp);f(player+0x84,100);f(player+0x88,200);f(player+0x8C,500.34f);f(player+0x100,1);f(player+0x104,1);
  for(unsigned axis=0;axis<3;++axis){f(player+0x6C+4*axis,.8f);f(reserve+0x78+4*axis,.8f);h(ep+6+2*axis,uint16_t(axis==0?14:axis==1?16:20));h(bp+6+2*axis,uint16_t(axis==2?20:24));f(child+0x6C+4*axis,1);}
  actor(reserve,0x4004);w(reserve+0x180,player);b(pp+0x12C8,1);h(pp+0x12BE,0);w(pp+0x11FC,reserve);
  const auto er=workshop::boss_egg_shape0260::egg_row,br=workshop::boss_egg_shape0260::table+(0x4018-0x4001)*24;w(er,1);w(er+4,ep);w(br,1);w(br+4,bp);
  f(0x800B02D0,.7f);f(0x800B02E0,-.001f);w(0x800F8D18,overlay+0x7C0);f(overlay+0x908,.75f);f(overlay+0x90C,1.5f);f(overlay+0x910,2.25f);w(0x800F8DB0,0);w(0x800F8DB4,8);
  w(root+0x130,0x800483B8);w(definition+0x70,child+0x6C);h(definition+10,8);f(child+0x84,200);f(child+0x88,200);f(child+0x8C,500.3f);
  constexpr uint32_t second=0x80120200,meta2=0x80111200,def2=0x80111300;actor(second,0x46F9);b(second+0x17D,0);w(second+0x180,root);w(second+0x168,child_resource);w(second+0x174,meta2);w(meta2,def2);w(def2+0x28,factory+0x3E08);w(def2+0x70,second+0x6C);h(def2+10,8);h(def2+12,8);h(def2+14,20);f(second+0x84,300);f(second+0x88,200);f(second+0x8C,500.3f);for(unsigned axis=0;axis<3;++axis)f(second+0x6C+4*axis,1);w(root+0x188+4,second);
  for(unsigned vi:{1u,2u,3u})history.native_frame(rdram,root,priv,7,169,2,vi);
 }
 std::optional<b::Scene> read(){return b::observe(memory,player,roots,history,169,2,4);}
};
static d::Input input(std::array<d::Target,1>& targets){d::Input in;in.player=2;in.room=169;in.encounter=2;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.reserves=6;in.x=100;in.y=200;in.min_x=0;in.max_x=768;in.has_preferred_x=true;in.preferred_x=100;in.targets=targets;return in;}
#ifndef MONSTER_BURST_FIXTURE_ONLY
int main(){checks=0;
 {const auto s=scene();const auto good=b::evaluate(s,10,174,200);need(good.observed&&good.ready&&!good.passed&&good.expiry>0&&good.x<180,"A real reticle endpoint before direct contact yields an explicit native-puff window");
  const auto short_tip=b::evaluate(s,10,140,200);need(short_tip.observed&&!short_tip.ready&&!short_tip.passed,"A shorter real reticle remains outside actual4018 range");
  const auto far=b::evaluate(s,10,250,200);need(far.observed&&!far.ready&&far.passed&&far.reason==b::Reason::DirectEggContact,"A long aligned reticle is refused when4002 reaches an eye before expiry");
  need(!b::evaluate(s,11,174,200).ready,"A neighbouring foreign root is never the requested victim");
 }
 {auto s=scene();s.count=2;s.eyes[1]={2,20,150,200,500.3f,8,8,20,0,0};need(b::evaluate(s,10,174,200).passed,"Another owned root's eye also blocks direct4002 flight");s.eyes[1].y=300;need(b::evaluate(s,10,174,200).ready,"An off-path second eye does not invent an obstruction");s.eyes[1].y=230;s.eyes[1].vy=-2;need(b::evaluate(s,10,174,200).passed,"An eye entering the future path is checked at every discrete native move");}
 {auto s=scene();s.eyes[0].vx=3;need(!b::evaluate(s,10,174,200).ready,"Puff prediction includes future eye motion during flight and initialization");s=scene();s.mx=9;need(!b::evaluate(s,10,174,200).ready,"Release-animation uncertainty may invalidate the earlier safe gap");s=scene();s.eyes[0].z=600;need(!b::evaluate(s,10,174,200).ready,"Actual fixed primary4018 depth participates in puff overlap");}
 {auto s=scene();s.eyes[0].vx=1;need(!b::evaluate(s,10,174,200).ready,"Even with static P1, eight possible prelaunch native eye updates can invalidate the puff window");}
 for(unsigned colour:{0u,6u,7u})for(unsigned hp:{0u,7u,8u}){Reader r;r.w(0x800F8DB0,colour);r.w(0x800F8DB4+4*colour,hp);const auto before=r.memory;const auto s=r.read();const unsigned parameter=(colour==6||colour==7)?(hp==8?2u:1u):(hp==8?1u:0u);need(s&&s->param==parameter&&s->burst_x==24*(parameter==0?.75f:parameter==1?1.5f:2.25f),"Reader derives the exact nativeAE4C parameter and private14 collision scale");need(before==r.memory,"All native-profile reads preserve the complete extended memory fixture");}
 for(unsigned fault=0;fault<10;++fault){Reader r;if(fault==0)r.w(0x800F8828,0);if(fault==1)r.b(Reader::pp+0xF4,1);if(fault==2)r.w(0x800F8D18,0);if(fault==3)r.f(Reader::overlay+0x90C,1.6f);if(fault==4)r.h(Reader::bp+6,23);if(fault==5)r.w(0x800F8DB0,8);if(fault==6)r.w(0x800F8DB4,9);if(fault==7)r.w(Fixture::child+0x180,0);if(fault==8)r.f(Fixture::child+0x6C,2);if(fault==9)r.w(Reader::player+0x180,Fixture::root);const auto saved=r.memory;need(!r.read()&&saved==r.memory,"Unproved player/ground/profile/callback/colour/HP/eye/parent refuses a burst observation without writes");}
 for(bool passed:{false,true}){d::Controller c;need(c.configure(true,true,{},false),"Configure ordinary QA controller");std::array<d::Target,1> targets{{{10,0x41C6,200,200,0,0,true,true,false,false,true,6.6f,2}}};targets[0].monster_burst_observed=true;auto in=input(targets);c.step(in);in.vi=4;c.step(in);in.vi=5;in.aiming_actor=3;in.reticle_actor=4;in.reticle_x=174;in.reticle_y=200;targets[0].monster_burst_ready=!passed;targets[0].monster_burst_passed=passed;const auto out=c.step(in);
  need(passed?(c.snapshot().phase==d::Phase::CancelAim&&(out.buttons&d::tongue_button)):(c.snapshot().phase==d::Phase::Release&&!out.buttons),"Only the scoped actual burst window releases short of eye centre; a passed window cancels through ordinaryB");}
 // Actual captured b22 shot2: its fourfold direct-contact failure must not
 // become a shootable airburst merely because range>=eye centre.
 {auto s=scene();s.x=316.755768f;s.y=397.999969f;s.eyes[0].x=425;s.eyes[0].y=441;s.eyes[0].vx=-.25f;const auto r=b::evaluate(s,10,521.816772f,481.707825f);need(r.passed&&!r.ready,"Actual b22 second long-reticle trajectory is rejected before its proven direct-eye contact");}
 std::printf("BossMonsterBurst0260Checks: %u PASS (read-only native-shaped profiles and explicit control/trajectory fixtures; no game damage/victory claim)\n",checks);
 return 0;
}
#endif
