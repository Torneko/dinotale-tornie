#define MONSTER_BURST_FIXTURE_ONLY
#include "BossMonsterBurst0260Checks.cpp"
#undef MONSTER_BURST_FIXTURE_ONLY
#ifndef MONSTER_NATIVE_AIM_ENTRY
#error Supply the immutable real2EF78/3BE44 extraction.
#endif
static void (*construct_held)(uint8_t*,recomp_context*)=nullptr;
static unsigned constructions=0,animation_calls=0,native_calls=0;
namespace literal {
// Explicit HOST boundaries only: no native allocator/animation is simulated.
void func_800403E4(uint8_t* ram,recomp_context* c){++constructions;construct_held(ram,c);c->r2=1;}
void func_800280F4(uint8_t*,recomp_context*){++animation_calls;}
void func_8002EF78(uint8_t*,recomp_context*);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include MONSTER_NATIVE_AIM_ENTRY
#pragma clang diagnostic pop
}
struct Aim:Reader {
 static constexpr uint32_t held=0x80312000,reticle=0x80312400,pad=0x80313000;
 void attach(uint32_t a,uint16_t kind,unsigned slot){actor(a,kind);b(a+0x17D,0);w(a+0x180,player);h(a+0x17E,uint16_t(slot));w(player+0x188+4*slot,a);h(player+0x184,uint16_t(slot+1));}
 void egg(){attach(held,0x4005,0);w(pp+0x1F0,held);for(unsigned i=0;i<3;++i)f(held+0x78+4*i,.8f);}
 void tip(){attach(reticle,0x4003,1);w(pp+0x1F4,reticle);f(reticle+0x84,174);f(reticle+0x88,200);}
 void platform(){constexpr uint32_t p=0x80314000;actor(p,0x415F);w(p+0x130,0x800483B8);f(p+0x100,1);f(p+0x104,1);f(p+0xC8,-1);w(player+0x180,p);b(player+0x17D,0);h(player+0x17E,19);h(p+0x184,1);w(p+0x188+19*4,player);}
 void enter(){w(0x800F7EA8,pad);h(pad+12,0x2000);construct_held=[](uint8_t* ram,recomp_context* c){auto* rdram=ram;const auto p=uint32_t(c->r4);MEM_W(0,n::base(held))=0x19971110;MEM_H(12,n::base(held))=0x4005;MEM_B(0x18,n::base(held))=1;MEM_B(0x17D,n::base(held))=0;MEM_W(0x180,n::base(held))=p;MEM_H(0x17E,n::base(held))=0;MEM_H(0x184,n::base(p))=1;MEM_W(0x188,n::base(p))=held;const auto priv=n::word(rdram,p+0x1F4);MEM_W(0x1F0,n::base(priv))=held;for(unsigned i=0;i<3;++i)MEM_W(0x78+4*i,n::base(held))=std::bit_cast<uint32_t>(.8f);};
  recomp_context c{};c.r4=n::base(player);c.r29=n::base(0x807F0000);c.r31=n::base(0x80012340);const auto sp=c.r29,ra=c.r31;literal::func_8003BE44(rdram,&c);native_calls+=2;need(c.r29==sp&&c.r31==ra&&c.r2==1,"RealBE44/EF78 preserve their stack/link and report native admission");
 }
 std::optional<b::Scene> at(unsigned vi){history.native_frame(rdram,root,priv,7,169,2,vi);return b::observe(memory,player,roots,history,169,2,vi);}
};
int main(){checks=0;
 {Aim a;a.platform();need(bool(a.read()),"Ordinary primary1 spatial scene on a real-shaped reciprocal415F is valid before Z");a.enter();need(n::byte(a.rdram,Aim::player+0x18)==3&&n::word(a.rdram,Aim::player+0x20)==0x8003BF88&&n::byte(a.rdram,Aim::pp)==1,"REAL EF78 creates primary3/BF88 with saved native primary1");need(!n::word(a.rdram,Aim::pp+0x1F4),"Explicit constructor boundary models the source-allowed absent4003 case");
  const auto saved=a.memory;const auto pending=a.at(5);need(pending&&saved==a.memory,"Strict true4005 plus real primary3 is admitted as a read-only waiting scene");
  d::Controller c;need(c.configure(true,true,{},false),"Configure the real control-only controller");std::array<d::Target,1> targets{{{Fixture::root,0x41C6,200,200,0,0,true,true,false,false,true,6.6f,2}}};auto in=input(targets);in.player=Aim::player;in.x=100;in.y=200;in.vi=0;c.step(in);in.vi=4;c.step(in);in.vi=5;in.primary_state=3;in.aiming_actor=Aim::held;in.reticle_actor=0;
  b::Result result;workshop::boss_driver_bridge0260::plan_monster_burst(a.rdram,*pending,in,targets[0],result);const auto wait=c.step(in);need(targets[0].clear_path&&!targets[0].monster_burst_ready&&c.snapshot().phase==d::Phase::TrackAim&&wait.buttons==d::egg_button,"Actual Reader→Bridge→Driver holds Z without B or release while4003 is absent");
  a.tip();const auto ready=a.at(6);need(bool(ready),"A subsequent exact reciprocal4003 completes the real native aiming graph");in.vi=6;in.reticle_actor=Aim::reticle;in.reticle_x=174;in.reticle_y=200;workshop::boss_driver_bridge0260::plan_monster_burst(a.rdram,*ready,in,targets[0],result);need(targets[0].monster_burst_ready&&!c.step(in).buttons&&c.snapshot().phase==d::Phase::Release,"Strict reticle completion can release through the actual composed pipeline");
  for(unsigned vi=7;vi<=14;++vi){in.vi=vi;const auto pending_throw=a.at(vi);need(bool(pending_throw),"Native primary3 and4005 remain observable during the explicit eight-VI deferred-throw boundary");workshop::boss_driver_bridge0260::plan_monster_burst(a.rdram,*pending_throw,in,targets[0],result);const auto out=c.step(in);need(!out.buttons&&c.snapshot().phase!=d::Phase::CancelAim,"Release/Cooldown does not inject B or a new Z while405C8 has not yet consumed4005");}
  // HOST post405C8 boundary: native406C0 retires4005 before BCDC calls
  // EFF8(primary4). This fixture does not simulate projectile construction.
  a.b(Aim::held+6,3);a.w(Aim::pp+0x1F0,0);a.w(Aim::pp+0x1F4,0);a.w(Aim::player+0x188,0);a.w(Aim::player+0x188+4,0);a.h(Aim::player+0x184,0);a.b(Aim::player+0x18,4);a.w(Aim::player+0x20,0x8003C0DC);in.vi=15;in.primary_state=4;in.aiming_actor=in.reticle_actor=0;targets[0].clear_path=false;need(!a.at(15),"Unrelated primary4 is not admitted by the Reader");need(!c.step(in).buttons&&c.snapshot().phase!=d::Phase::CancelAim,"No false B cancellation after the already-consumed4005 disappears");
 }
 {Aim a;a.enter();d::Controller c;need(c.configure(true,true,{},false),"Configure bounded no-reticle timeout");std::array<d::Target,1> targets{{{Fixture::root,0x41C6,200,200,0,0,true,true,false,false,true,6.6f,2}}};auto in=input(targets);in.vi=0;c.step(in);in.vi=4;c.step(in);in.primary_state=3;in.aiming_actor=Aim::held;
  for(unsigned vi=5;vi<=125;++vi){in.vi=vi;const auto scene=a.at(vi);need(bool(scene),"A native4003 allocation failure may retain a strictly proved spatial scene");b::Result result;workshop::boss_driver_bridge0260::plan_monster_burst(a.rdram,*scene,in,targets[0],result);const auto out=c.step(in);need(!targets[0].monster_burst_ready,"No update can certify a burst without a real4003");if(vi<125)need(out.buttons==d::egg_button&&c.snapshot().phase==d::Phase::TrackAim,"Waiting retains native Z, never invents a blockage or shot");else need((out.buttons&d::tongue_button)&&c.snapshot().phase==d::Phase::CancelAim,"The unchanged120VI aim timeout cancels the absent-reticle attempt through native B");}
 }
 for(unsigned fault=0;fault<13;++fault){Aim a;a.enter();a.tip();if(fault==0)a.b(Aim::player+0x18,2);if(fault==1)a.w(Aim::player+0x20,0x80043450);if(fault==2)a.b(Aim::pp,2);if(fault==3)a.w(Aim::pp+0x1F0,0);if(fault==4)a.w(Aim::held+0x180,Fixture::root);if(fault==5)a.w(Aim::player+0x188+8,Aim::held);if(fault==6)a.w(Aim::pp+0x1F4,0x80312800);if(fault==7)a.h(Aim::reticle+12,0x4002);if(fault==8)a.w(Aim::reticle+0x180,Fixture::root);if(fault==9)a.h(Aim::reticle+0x17E,19);if(fault==10)a.b(Aim::reticle+6,3);if(fault==11)a.b(Aim::pp+0xF4,1);if(fault==12)a.w(0x800F8828,Fixture::root);const auto saved=a.memory;need(!a.at(5)&&saved==a.memory,"Foreign posture/callback/saved state/held/reticle/parent/slot/lifetime/ground/player fails without writes");}
 need(constructions==15&&animation_calls==15,"Every tested transition crossed the two explicit constructor/animation boundaries exactly once");
 std::printf("BossMonsterAimTransition0260Checks: %u PASS (%u real BE44/EF78 calls; HOST constructor/animation boundaries explicit, no game shot/victory claim)\n",checks,native_calls);
}
