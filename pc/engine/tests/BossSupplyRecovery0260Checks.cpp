#include "../boss_driver0260.hpp"
#include "../boss_native_mouth0260.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace d=workshop::boss_driver0260;namespace n=workshop::boss_native;
static unsigned checks=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
static bool same(const d::Output& a,const d::Output& b){return a.handled==b.handled&&a.buttons==b.buttons&&a.x==b.x&&a.y==b.y;}
struct Fixture {
 d::Controller c;d::Input in;std::array<d::Target,1> targets{};std::array<d::Supply,2> supplies{};std::array<float,80> xs{},ys{};
 Fixture(){
  need(c.configure(true,true,{},false),"Supply recovery is diagnostic input only");targets[0]={0x80110000,0x40A3,120,143,0,0,true,true,false};
  in.player=0x80100000;in.room=168;in.encounter=1;in.combat=in.controls=in.alive=true;in.primary_state=1;
  in.x=72.90283f;in.y=101.02122f;in.min_x=40;in.max_x=724;in.targets=targets;in.supplies=supplies;
  in.native_stick_observed=in.native_tongue_observed=in.native_tongue_known=in.native_tongue_ready=true;
  xs.fill(14);ys.fill(-18);in.native_mouth={true,true,0,1,1,xs,ys};in.native_tongue_max_reach=*workshop::boss_tongue_geometry0260::max_centre_reach(1.f,.001f,false);
 }
 d::Output at(unsigned vi){in.vi=vi;return c.step(in);}
 void bird(float x=364.36f,float y=53.04f,uint16_t kind=0x4526){supplies[0]={0x80140000,kind,x,y,true,false,0xC1};}
};
int main(int argc,char** argv){try{
 for(uint16_t kind:{uint16_t(0x4526),uint16_t(0x46DA)})for(bool right:{false,true}){
  Fixture f;if(!right){f.in.x=691.0972f;f.bird(399.64f,53.04f,kind);}else f.bird(364.36f,53.04f,kind);
  for(unsigned vi:{100u,101u,160u}){const auto out=f.at(vi);need(out.handled&&!out.buttons&&!out.y&&(right?out.x>0:out.x<0),"A real remote in-bounds bird gets horizontal travel only, never A/B or vertical steering");need(f.c.snapshot().supply_actor==f.supplies[0].actor&&!f.c.snapshot().jump_active,"Walking toward a source is not a tongue/jump attempt");need(same(out,f.at(vi)),"Remote travel is same-VI idempotent");}
 }
 for(unsigned fault=0;fault<7;++fault){Fixture f;f.in.x=382;f.bird(700,53);if(fault==0)f.supplies[0].x=800;if(fault==1)f.supplies[0].x=0;if(fault==2)f.supplies[0].y=f.in.y-201;if(fault==3)f.supplies[0].y=f.in.y+81;if(fault==4)f.supplies[0].active=false;if(fault==5)f.supplies[0].food_flags=0;if(fault==6)f.supplies[0].kind=0x41DD;
  const auto out=f.at(100);need(!out.buttons&&!out.x&&!out.y&&!f.c.snapshot().supply_actor,"Out-of-bounds/high/low/non-food/non-live sources cannot be chased from the safe centre");
 }
 {Fixture f;f.bird();auto out=f.at(100);need(out.x>0&&!out.buttons,"The61cb distant bird first receives walking input");f.in.x=260;out=f.at(101);need(!(out.buttons&d::jump_button)&&!f.c.snapshot().supply_jump_grounded,"Entering bird range while airborne cannot synthesize a ground jump");f.in.x=324;f.supplies[0].x=364;out=f.at(102);need(out.buttons==d::tongue_button&&!(out.buttons&d::jump_button),"A genuinely ready/reachable airborne bird can be eaten without a new flutter request");}
 for(unsigned start:{0u,100u,13740u}){Fixture f;f.in.x=324;f.bird(364,53);f.in.native_tongue_ready=false;for(unsigned dt=0;dt<75;++dt){const auto out=f.at(start+dt);need(!(out.buttons&d::jump_button),"New supply observations already in air never press A, including repeated reappearance epochs");}f.supplies[0].active=false;f.at(start+75);f.supplies[0].active=true;const auto out=f.at(start+76);need(!(out.buttons&d::jump_button),"A disappearing/reappearing bird cannot reset an airborne A hold");f.in.grounded=true;need(f.at(start+77).buttons==d::jump_button&&f.c.snapshot().supply_jump_grounded,"A new native grounded observation alone permits a fresh normal jump");}
 {Fixture f;f.in.x=324;f.bird(364,1);f.in.grounded=true;need(f.at(100).buttons==d::jump_button,"A real ground-origin supply jump retains its initial A edge");f.in.grounded=false;f.in.native_tongue_ready=false;for(unsigned age=1;age<12;++age)need(f.at(100+age).buttons==d::jump_button,"The existing twelve-VI preparation survives a genuine ground-origin jump");need(!(f.at(165).buttons&d::jump_button),"Even a real ground jump eventually releases A while its bird remains unreachable");}
 for(bool right:{false,true}){Fixture f;if(!right)f.in.x=691.0972f;auto out=f.at(100);need(out.handled&&!out.buttons&&!out.y&&(right?out.x>0:out.x<0),"No-source edge recovery moves inward while releasing every action button");need(f.c.snapshot().supply_relocating&&f.c.snapshot().supply_relocation_vi==100,"Recovery gets one bounded observation epoch");need(same(out,f.at(100))&&f.c.snapshot().supply_relocation_vi==100,"Duplicate VI cannot restart or consume recovery");out=f.at(219);need(!out.buttons&&f.c.snapshot().supply_relocating,"Inward recovery stays action-free within its120VI budget");out=f.at(220);need(!f.c.snapshot().supply_relocating&&!out.buttons&&!out.x,"A stationary unsuccessful recovery expires instead of asserting success");need(!f.c.snapshot().launches&&!f.c.snapshot().attempts,"Requested recovery movement creates no native projectile or attack receipt");}
 {Fixture f;f.in.x=300;need(!f.at(100).x,"A non-edge no-source player initially waits");need(!f.at(279).x,"A short stationary observation does not trigger fallback roaming");need(f.at(280).x>0&&f.c.snapshot().supply_relocating,"Only180VI of actual stationary position starts the bounded centre recovery");f.in.x=380;const auto out=f.at(281);need(!out.x&&!out.buttons&&!f.c.snapshot().supply_relocating,"Observed arrival near the centre ends recovery without a manufactured landing");need(!f.at(500).x,"Centre waiting never becomes perpetual arbitrary roaming");}
 {Fixture f;f.in.x=300;f.at(100);f.in.x=310;need(!f.at(279).x&&!f.c.snapshot().supply_relocating,"Observed displacement resets the stall clock");need(!f.at(280).x,"Old elapsed time cannot override recent physical progress");}
 {Fixture f;f.at(100);f.bird(112,53);const auto out=f.at(101);need(!f.c.snapshot().supply_relocating&&!(out.buttons&d::jump_button),"A reachable actual source preempts empty-source recovery without an airborne A reset");}
 for(bool island:{false,true}){Fixture f;std::array<float,3> stands{152,384,616};f.in.x=384;f.in.y=248;f.bird(616,148);if(island){f.targets[0].kind=0x4511;f.in.stable_stands=stands;}else{f.in.has_preferred_x=true;f.in.preferred_x=384;}const auto out=f.at(100);need(!out.x&&!out.buttons&&!f.c.snapshot().supply_relocating,"Fish island and explicit preferred anchor never chase a remote source or start centre recovery");}
 {Fixture f;f.supplies[0]={0x80140000,0x456E,88,101,true,false,0,0x4528,1,0};f.supplies[1]={0x80141000,0x4526,364,53,true,false,0xC1};const auto out=f.at(100);need(out.x>0&&!out.buttons&&!out.y&&f.c.snapshot().supply_actor==f.supplies[0].actor&&!f.c.snapshot().supply_relocating,"A native feeder retains priority over remote bird travel/recovery");}
 {Fixture f;f.in.native_tongue_live=true;f.in.native_tongue_phase=2;f.in.native_tongue_ready=false;const auto out=f.at(100);need(out.buttons==d::tongue_button&&!out.x&&!out.y&&!f.c.snapshot().supply_relocating,"An actual live tongue remains prioritary over edge recovery");}
 {Fixture f;f.in.native_tongue_known=false;const auto out=f.at(100);need(!out.buttons&&!out.x&&!out.y&&!f.c.snapshot().supply_relocating,"Unknown tongue graph remains neutral and cannot license new movement");}
 for(bool grounded:{false,true}){Fixture f;f.in.mouth_full=true;f.in.grounded=grounded;const auto out=f.at(100);need(!out.buttons&&!out.x&&(grounded?out.y<0:out.y==0)&&!f.c.snapshot().supply_relocating,"Held food keeps its native airborne/grounded ingestion priority");}
 for(unsigned fault=0;fault<5;++fault){Fixture f;if(fault==0)f.in.controls=false;if(fault==1)f.in.alive=false;if(fault==2)f.in.combat=false;if(fault==3)f.targets[0].active=false;if(fault==4)f.in.player=0;const auto out=f.at(100);need(!out.buttons&&!out.x&&!out.y&&!f.c.snapshot().supply_relocating,"Recovery cannot bypass native controls, liveness, ownership or combat scope");}
 {Fixture f;f.in.aiming_actor=0x80150000;f.in.reticle_actor=0x80151000;const auto out=f.at(100);need(f.c.snapshot().phase==d::Phase::TrackAim&&(out.buttons&d::egg_button)&&!f.c.snapshot().supply_relocating,"An actual4005 aiming graph preempts all supply recovery");}
 if(argc==2){
  std::ifstream stream(argv[1],std::ios::binary);std::vector<uint8_t> ram((std::istreambuf_iterator<char>(stream)),{});need(ram.size()==0x800000,"The diagnostic LIVE UNFENCED61cb capture is complete8MiB");auto* rdram=ram.data();
  const auto player=n::word(rdram,0x800F8828),p=n::word(rdram,player+0x1F4);auto fl=[&](uint32_t addr){return std::bit_cast<float>(n::word(rdram,addr));};
  need(n::actor(rdram,player,0x4001)&&n::byte(rdram,player+0x18)==1&&n::byte(rdram,player+0x30)==14&&n::word(rdram,player+0x20)==0x80043450,"Real captured P1 is living state1/14d with native43450, not a synthesized dead actor");
  need(n::byte(rdram,p+0x12C8)==0&&n::byte(rdram,p+0xF4)!=0&&!n::word(rdram,player+0x180),"Actual captured source has empty reserves, is airborne and has no parent");
  Fixture f;f.in.player=player;f.in.x=fl(player+0x84);f.in.y=fl(player+0x88);f.in.grounded=n::byte(rdram,p+0xF4)==0;f.in.controls=!n::byte(rdram,0x800F7EA5)&&n::byte(rdram,0x800F7EA6)==1;f.targets[0].actor=0x800C92D0;
  need(n::actor(rdram,f.targets[0].actor,0x40A3),"The sole host-supplied owned target retains the actual captured40A3 identity");
  const auto query=workshop::boss_native_mouth0260::observe_ordinary_tongue(rdram,player);f.in.native_tongue_known=query.observed;f.in.native_tongue_ready=query.ready;f.in.native_tongue_phase=query.phase;f.in.native_tongue_live=query.live;f.in.native_tongue_cancelled_idle=query.cancelled_idle;f.in.native_tongue_callback_pending=query.callback_pending;
  need(query.observed&&!query.live&&!query.callback_pending,"Real idle mouth graph permits movement while its tongue gate stays native");
  const auto before=ram;const auto out=f.at(14040);need(out.x>0&&!out.buttons&&!out.y&&f.c.snapshot().supply_relocating,"The exact unfenced61cb P1 observations now produce inward movement with A released");need(ram==before,"Reconstructing/reading captured observations and emitting controls preserves all8MiB");
  // This source was captured asynchronously by Root: no fence, physics step,
  // ingestion, recovery arrival, native checkpoint or victory is certified.
  std::cout<<"source=61cb diagnosticLiveUnfenced=1 capturedX="<<f.in.x<<" capturedY="<<f.in.y<<" outputX="<<out.x<<" outputA=0\n";
 }
 std::cout<<"BossSupplyRecovery0260Checks: "<<checks<<" PASS (ordinary diagnostic input decisions only; no native physics/ingestion/GPU/victory claim)\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
