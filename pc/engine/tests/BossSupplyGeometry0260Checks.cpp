#include "../boss_battle_runtime0250.hpp"
#include "../boss_driver_bridge0260.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <vector>
namespace d=workshop::boss_driver0260;
namespace s=workshop::boss_stick_aim0260;
namespace g=workshop::boss_tongue_geometry0260;
namespace n=workshop::boss_native;
static unsigned checks=0;
static void need(bool yes,const char* why){++checks;if(!yes){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
static bool same(const d::Output&a,const d::Output&b){return a.handled==b.handled&&a.buttons==b.buttons&&a.x==b.x&&a.y==b.y;}
static bool neutral(const d::Output&o){return o.handled&&!o.buttons&&!o.x&&!o.y;}
struct Fixture {
 d::Controller controller;d::Input in;std::array<d::Target,1> targets{};std::array<d::Supply,2> supplies{};std::array<float,80> xs{},ys{};
 Fixture(bool right=true,uint16_t kind=0x41C6){
  need(controller.configure(true,true,{},false),"Controller fixture is explicitly diagnostic");
  targets[0]={0x80150000,kind,534,248,0,0,true,true,false,kind==0x455A,true,6.6f,2};
  supplies[0]={0x80160000,0x4526,right?424.f:344.f,203,true,false,0xC1};
  supplies[1]={0x80161000,0x46DA,right?410.f:358.f,218,true,false,0xC1};supplies[1].active=false;
  in.player=0x80100000;in.room=169;in.encounter=2;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.x=384;in.y=248;in.min_x=40;in.max_x=724;in.targets=targets;in.supplies=supplies;
  in.native_stick_observed=in.native_tongue_observed=in.native_tongue_known=in.native_tongue_ready=true;
  xs.fill(14);ys.fill(-18);in.native_mouth={true,right,0,.8f,.8f,xs,ys};
  in.native_tongue_max_reach=*g::max_centre_reach(.8f,.001f,false);in.native_tongue_heading_degrees=48;in.native_tongue_heading_active=true;
 }
 d::Output at(unsigned vi){in.vi=vi;return controller.step(in);}
 void sample(const d::Output&o){in.native_stick=s::native_axes(s::convert({o.x,o.y}),in.native_stick);}
 d::Output begin(){for(unsigned vi=0;vi<12;++vi){in.grounded=vi==0;const auto o=at(vi);need(o.buttons==d::jump_button,"Preparation remains A-only before the native airborne tongue request");sample(o);}in.grounded=false;const auto o=at(12);need(o.buttons==d::tongue_button,"Observed admission and physical reach permit the first real B request");need(o.x>0&&o.y>0,"First-B capture stays relative and forward even when facing LEFT");sample(o);return o;}
 void live(unsigned phase=2){in.native_tongue_live=true;in.native_tongue_phase=phase;in.native_tongue_ready=false;}
 float desired()const{const auto ray=s::from_mouth(in.native_mouth,in.x,in.y,supplies[0].x,supplies[0].y);need(bool(ray),"Owned supply remains in native mouth coordinates");return std::clamp(std::atan2(ray->up,ray->forward)*180.f/s::pi,-90.f,90.f);}
 void expected_heading(const d::Output&o,float heading){const auto expected=g::hold_heading_world(heading,in.native_mouth.right,in.native_stick);need(bool(expected),"Shared geometry provides an exact inverse world-input candidate");need(o.x==expected->host.x&&o.y==expected->host.y,"Controller uses the shared native-world inverse with actual previous filtered axes");need(s::convert({o.x,o.y}).x==expected->raw.x&&s::convert({o.x,o.y}).y==expected->raw.y,"Returned host axes reproduce the candidate raw pad");}
};
struct Memory {
 std::vector<uint8_t> bytes=std::vector<uint8_t>(0x800000);uint8_t*rdram=bytes.data();
 static constexpr uint32_t player=0x80100000,priv=0x80110000,body=0x80120000,tip=0x80121000;
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}void h(uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}void b(uint32_t a,uint8_t v){MEM_B(0,n::base(a))=v;}void f(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}
 Memory(){
  w(player,0x19971110);h(player+12,0x4001);b(player+0x18,1);b(player+0x17D,2);w(player+0x1F4,priv);w(0x800F8828,player);f(player+0x6C,.8f);f(player+0x70,.8f);w(player+0x160,0x80025EA4);f(priv+0x174,65);f(body+0x78,.001f);
  for(const auto row:std::array<std::array<uint32_t,4>,2>{{{body,0x400B,0x80122000,0x80124000},{tip,0x400C,0x80123000,0x80125000}}}){w(row[0],0x19971110);h(row[0]+12,uint16_t(row[1]));b(row[0]+0x18,1);w(row[0]+0x1F4,row[2]);w(row[0]+0x168,row[3]);w(row[0]+0x20,0x8003C9B0);}
  w(priv+0x160,body);w(priv+0x15C,tip);w(body+0x180,player);w(tip+0x180,body);w(player+0x188+5*4,body);w(body+0x188+2*4,tip);w(tip+0x174,0x80126000);w(0x80126000,0x80126100);
 }
};
int main(){
 // This suite consumes the common pure geometry verified separately against
 // real22B20/25EA4/3CE84/48F84 bodies. It does not simulate native ingestion.
 need(std::abs(*g::max_centre_reach(.8f,.001f,false)-71.0528f)<.002f,"Observed normal .8-scale/base.001 geometry replaces the unsupported90px constant");
 for(bool right:{false,true})for(uint16_t kind:std::array<uint16_t,7>{0x4599,0x4072,0x40A3,0x4511,0x4631,0x455A,0x41C6}){
  Fixture f(right,kind);f.begin();f.live();f.supplies[1].active=true;
  for(unsigned vi=13;vi<40;++vi){f.supplies[0].x=f.in.x+(right?1.f:-1.f)*(40.f+float(vi%5));f.supplies[0].y=203.f-float(vi%7);const auto out=f.at(vi);need(out.buttons==d::tongue_button,"Live extension2 keeps B until the native phase changes");f.expected_heading(out,f.desired());need(right?out.x>0:out.x<0,"Accepted live tongue receives WORLD-facing X, including negative LEFT");need(same(out,f.at(vi)),"World inverse is same-VI idempotent");need(f.controller.snapshot().supply_actor==f.supplies[0].actor,"Live tongue does not retarget the admitted supply");f.sample(out);}
  f.live(1);const auto retract=f.at(40);need(!retract.buttons,"Native retraction1 releases B while maintaining its observed heading");f.expected_heading(retract,f.desired());
  f.in.native_tongue_phase=3;need(neutral(f.at(41)),"Native phase3 remains neutral");
 }
 for(bool right:{false,true})for(unsigned mutation=0;mutation<5;++mutation){
  Fixture f(right);f.begin();f.live();const auto owned=f.supplies[0].actor;
  f.supplies[1].active=true; // A closer different bird must not become the target.
  if(mutation==0)f.supplies[0].active=false;if(mutation==1)f.supplies[0].actor+=4;if(mutation==2)f.supplies[0].kind=0x41DD;if(mutation==3)f.supplies[0].food_flags=0;if(mutation==4)f.supplies[0].x=std::numeric_limits<float>::quiet_NaN();
  const auto out=f.at(13);need(out.buttons==d::tongue_button,"Source disappearance cannot shorten an already accepted native extension");f.expected_heading(out,48);need(f.controller.snapshot().supply_actor==owned,"Invalidated source uses captured native174 instead of changing supply actor");
 }
 for(bool right:{false,true})for(unsigned phase:{1u,2u}){Fixture f(right);f.begin();f.live(phase);f.in.native_tongue_heading_active=false;const auto out=f.at(13);need(out.buttons==(phase==2?d::tongue_button:0)&&!out.x&&!out.y,"No continuous input steering is invented if native25EA4 is not active");}
 for(float margin:{-.1f,.1f})for(bool right:{false,true}){
  Fixture f(right);const float reach=f.in.native_tongue_max_reach;f.supplies[0].x=f.in.x+(right?11.2f:-11.2f);f.supplies[0].y=f.in.y-14.4f-reach-margin;f.at(0);f.in.grounded=false;const auto out=f.at(12);need(bool(out.buttons&d::tongue_button)==(margin<0),"First B is gated at observed native max reach, on both sides");
 }
 for(unsigned mutation=0;mutation<5;++mutation){Fixture f;f.at(0);f.in.grounded=false;if(mutation==0)f.in.native_tongue_max_reach=0;if(mutation==1)f.in.native_tongue_max_reach=-1;if(mutation==2)f.in.native_tongue_max_reach=std::numeric_limits<float>::quiet_NaN();if(mutation==3)f.in.native_tongue_observed=false;if(mutation==4){f.in.native_tongue_known=false;f.in.native_tongue_ready=false;}need(!(f.at(12).buttons&d::tongue_button),"Native-stick input without complete admission/reach evidence cannot send B");}
 for(bool right:{false,true}){Fixture f(right);f.begin();f.live();f.in.native_tongue_cancelled_idle=true;f.in.native_tongue_live=false;f.in.native_tongue_ready=true;need(neutral(f.at(13)),"Known cancellation releases B with neutral axes before a fresh request");f.in.native_tongue_known=false;need(neutral(f.at(14)),"Unknown tongue graph remains fail-closed");}
 for(auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress}){Fixture f;f.begin();f.live();f.in.throw_mode=mode;f.in.aiming_actor=0x80200000;f.in.reticle_actor=0x80201000;f.in.reticle_x=0;f.in.reticle_y=0;const auto out=f.at(13);need(f.controller.snapshot().phase==d::Phase::TrackAim&&out.buttons==(mode==d::ThrowMode::HoldAndRelease?d::egg_button:0),"True4005 remains prioritary over live Supply angle steering in both modes");}
 {Fixture f;f.begin();f.live();f.in.mouth_full=true;f.in.grounded=true;const auto out=f.at(13);need(!out.buttons&&out.y<0,"Actual held food retains grounded swallowing priority");}
 {Memory f;const auto before=f.bytes;const auto q=workshop::boss_native_mouth0260::observe_ordinary_tongue(f.rdram,Memory::player);need(q.observed&&q.ready&&q.heading_active&&q.heading_degrees==65&&std::abs(q.max_reach-71.0528f)<.002f,"Typed native query publishes actual heading/update callback and physical reach");need(f.bytes==before,"Geometry and heading observation are read-only across all8MiB");}
 {Memory f;f.w(0x800F8944,1);const auto before=f.bytes;const auto q=workshop::boss_native_mouth0260::observe_ordinary_tongue(f.rdram,Memory::player);need(q.observed&&std::abs(q.max_reach-*g::max_centre_reach(.8f,.001f,true))<.001f&&q.max_reach>109&&q.max_reach<110,"Real extended native mode publishes its separately derived longer reach");need(f.bytes==before,"Extended geometry observation is also read-only");}
 for(unsigned mutation=0;mutation<4;++mutation){Memory f;if(mutation==0)f.f(Memory::player+0x6C,0);if(mutation==1)f.f(Memory::body+0x78,-.001f);if(mutation==2)f.f(Memory::player+0x6C,std::numeric_limits<float>::quiet_NaN());if(mutation==3)f.f(Memory::body+0x78,std::numeric_limits<float>::quiet_NaN());const auto before=f.bytes;const auto q=workshop::boss_native_mouth0260::observe_ordinary_tongue(f.rdram,Memory::player);need(!q.observed&&!q.ready&&f.bytes==before,"Invalid native geometry cannot license a fresh tongue or modify memory");}
 std::printf("BossSupplyGeometry0260Checks: %u PASS (synthetic diagnostic controls/shared geometry and read-only native fixtures; no native ingestion, game, GPU, save, ledger or victory proof)\n",checks);
}
