#include "../boss_fish_water_recovery0260.hpp"
#include "../boss_native_mouth0260.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
namespace q=workshop::boss_fish_water_recovery0260;namespace n=workshop::boss_native;namespace d=workshop::boss_driver0260;namespace m=workshop::boss_monster_aim0260;
static unsigned checks=0;void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
// Only native SDK trigonometry services are supplied by the existing precise
// quantized native-sine model. The complete24188 velocity writer is unchanged.
extern "C" void func_80065954(uint8_t*,recomp_context*);
extern "C" void func_80066DE4(uint8_t*,recomp_context* c){c->f0.fl=workshop::boss_stick_aim0260::native_trig(c->f12.fl,true);}
extern "C" void func_80066E88(uint8_t*,recomp_context* c){c->f0.fl=workshop::boss_stick_aim0260::native_trig(c->f12.fl,false);}
static unsigned flutter_requests=0;
extern "C" void func_800340F8(uint8_t*,recomp_context* c){c->r2=0;} // Explicit unrelated action service: no interception.
extern "C" void func_8002DE68(uint8_t*,recomp_context*){++flutter_requests;} // Observe native dispatch only; do not emulate flutter.
extern "C" void func_8002DF30(uint8_t*,recomp_context*){++flutter_requests;}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-label"
#pragma clang diagnostic ignored "-Wunused-parameter"
extern "C" {
#include "native-water-vector.inc"
#include "native-flutter-request.inc"
}
#pragma clang diagnostic pop
struct Fixture {
 std::vector<uint8_t> memory;uint8_t* rdram=nullptr;uint32_t player=0,p=0;std::array<float,3> centers{152,408,632};d::Input in;q::Frame frame{true,97,0,"C217WaterReturn",273};
 explicit Fixture(const char* file){std::ifstream f(file,std::ios::binary);memory.assign(std::istreambuf_iterator<char>(f),{});need(memory.size()==0x800000,"Actual C217 source snapshot is8MiB");rdram=memory.data();player=n::word(rdram,0x800F8828);p=n::word(rdram,player+0x1F4);in.vi=3600;in.room=169;in.encounter=2;in.player=player;in.combat=in.controls=in.alive=in.fish_navigation=in.fish_island_observed=true;in.fish_anchor_x=408;in.stable_stands=centers;in.native_stick_observed=true;refresh();}
 void word(uint32_t at,uint32_t v){MEM_W(0,n::base(at))=int32_t(v);}void byte(uint32_t at,unsigned v){MEM_B(0,n::base(at))=uint8_t(v);}void fl(uint32_t at,float v){word(at,std::bit_cast<uint32_t>(v));}
 void refresh(){in.primary_state=n::byte(rdram,player+0x18);in.x=m::scalar(rdram,player+0x84);in.y=m::scalar(rdram,player+0x88);in.horizontal_speed=m::scalar(rdram,player+0xC4);in.horizontal_speed_observed=std::isfinite(in.horizontal_speed);in.native_stick={int(MEM_H(0,n::base(0x800F897E))),int(MEM_H(0,n::base(0x800F8980)))};in.grounded=n::byte(rdram,p+0xF4)==0;const auto t=workshop::boss_native_mouth0260::observe_ordinary_tongue(rdram,player);in.native_tongue_known=in.native_tongue_observed=t.observed;in.native_tongue_live=t.live;in.native_tongue_callback_pending=t.callback_pending;const auto s=workshop::boss_fish_support0260::observe(rdram,player,169,2,centers,408);in.fish_on_island=s&&s->on_island;}
 void pose(float x,float y,unsigned sub=78){fl(player+0x84,x);fl(player+0x88,y);byte(player+0x18,1);byte(player+0x30,sub);refresh();++in.vi;}
 q::Output step(q::Controller& c){const auto before=memory;const auto out=c.step(rdram,frame,in);need(memory==before,"Controller observation leaves every8MiB byte unchanged");return out;}
};
int main(int argc,char**argv){try{
 need(argc==4,"Two actual C217 water captures and actual60c8 airborne capture required");
 for(int capture=1;capture<=2;++capture){Fixture f(argv[capture]);q::Controller c;need(f.in.primary_state==1&&(n::byte(f.rdram,f.player+0x30)==77||n::byte(f.rdram,f.player+0x30)==78),"Actual failed native77/78 swim state");const auto b=q::body(f.rdram,f.player);need(b&&b->left< -11&&b->right>11&&b->bottom==16,"Actual collider and native terrain offsets bound the current body");const auto map=m::Map::read(f.rdram);need(map&&q::clear(f.rdram,*map,*b,f.in.x,f.in.y,352,f.in.y),"Actual water body route to left free column is clear");need(!q::clear(f.rdram,*map,*b,408,f.in.y,408,248),"Actual bitmap refuses a direct ascent through the island underside");auto out=f.step(c);need(out.handled&&out.stage==q::Stage::Column&&out.x<0&&!out.y&&!out.buttons&&out.lane==352&&out.rise_y==271,"Actual capture selects a laterally free whole-body route with geometry-derived height");
 const auto lane=out.lane,rise=out.rise_y;f.pose(lane,f.in.y);out=f.step(c);need(out.stage==q::Stage::Ascend&&out.y==1&&(out.buttons&d::jump_button),"At real column frontier issue UP plus ordinary A");const auto axes=workshop::boss_stick_aim0260::native_axes(workshop::boss_stick_aim0260::convert({out.x,out.y}),{});need(axes.y>0,"Requested UP survives the actual input conversion model");
 // HOST position frontiers exercise outputs, not native swimming or landing.
 f.pose(lane,280,0);out=f.step(c);need(out.handled&&out.stage==q::Stage::Ascend&&out.y==1&&!out.x,"Leaving77/78 does not abandon return below the native support");f.pose(lane,rise,0);out=f.step(c);need(out.stage==q::Stage::Return&&out.x>0,"Above measured rim return toward observed island");f.pose(408,273,0);f.fl(f.player+0xC4,.8f);f.refresh();out=f.step(c);need(out.stage==q::Stage::Settle&&out.x<0&&!out.completed,"Native ground support brakes actual positive inertia");f.fl(f.player+0xC4,0);f.refresh();++f.in.vi;need(!f.step(c).completed,"One stopped observation alone does not finish");++f.in.vi;out=f.step(c);need(out.completed&&c.stage()==q::Stage::Idle,"Two actual supported pose observations complete only host navigation");
 d::Controller driver;driver.configure(true,true,{},false);driver.step(f.in);auto& saved=const_cast<d::Snapshot&>(driver.snapshot());saved.attempts=9;saved.launches=7;saved.last_projectile=0x80020100;saved.phase=d::Phase::TrackAim;saved.fish_cooldown_vi=100;saved.fish_release_not_before=160;saved.fish_shot_authorized=true;const auto serial=driver.release_trace().serial;need(driver.resume_fish_navigation(f.in),"Observed landing refreshes diagnostic navigation");need(saved.phase==d::Phase::Approach&&saved.phase_vi==f.in.vi&&!saved.fish_shot_authorized&&saved.attempts==9&&saved.launches==7&&saved.last_projectile==0x80020100&&saved.fish_cooldown_vi==100&&saved.fish_release_not_before==160&&driver.release_trace().serial==serial,"Refresh preserves shot counters, cooldown and provenance serial");f.in.aiming_actor=0x80030000;need(!driver.resume_fish_navigation(f.in),"Real4005 retains precedence over refresh");
 }
 for(unsigned fault=0;fault<22;++fault){Fixture f(argv[2]);q::Controller c;switch(fault){case 0:f.frame.admitted=false;break;case 1:f.frame.source=96;break;case 2:f.frame.token={};break;case 3:++f.frame.epoch;break;case 4:f.in.room=168;break;case 5:f.in.qa_bird=true;break;case 6:f.in.combat=false;break;case 7:f.in.controls=false;break;case 8:f.in.alive=false;break;case 9:f.in.aiming_actor=0x80020000;break;case 10:f.in.reticle_actor=0x80020000;break;case 11:f.in.native_tongue_live=true;break;case 12:f.in.native_tongue_known=false;break;case 13:f.byte(f.player+0x165,1);break;case 14:f.word(f.player+0x180,0x80020000);break;case 15:f.byte(0x800F7EA5,1);break;case 16:f.word(f.p+0x164,0x80020000);break;case 17:f.word(f.p+0xC0,0x80020000);break;case 18:f.word(f.p+0x1F4,0x80020000);break;case 19:f.pose(f.in.x,f.in.y,0);break;case 20:f.in.primary_state=9;break;case 21:f.word(0x800F8828,0);break;}const auto out=f.step(c);need(!out.handled,"Foreign/busy/native-cancelled scope never owns controls");}

 {Fixture f(argv[2]);q::Controller c;need(f.step(c).handled,"Budget test starts from actual swim");f.in.vi+=1201;const auto out=f.step(c);need(out.handled&&out.stage==q::Stage::Blocked&&!out.buttons&&!out.x&&!out.y,"1200VI budget expires neutral without writing defeat or inventory");f.frame.admitted=false;need(!f.step(c).handled&&c.stage()==q::Stage::Idle,"Pause/context cancellation resets only host navigation");}
 {Fixture f(argv[2]);const auto map=m::Map::read(f.rdram);need(bool(map),"Unknown-map fixture starts from actual map");f.word(map->descriptor+0x18,0);q::Controller c;const auto out=f.step(c);need(out.handled&&out.stage==q::Stage::Blocked&&!out.x&&!out.y&&!out.buttons,"Unknown map cannot license a recovery path");}
 {Fixture f(argv[2]);recomp_context ctx{};ctx.r29=n::base(0x800FF800);ctx.r4=n::base(f.player);ctx.r5=std::bit_cast<uint32_t>(2.f);f.fl(f.player+0x50,90);f.fl(f.p+0x118,2);func_80024188(f.rdram,&ctx);need(std::abs(m::scalar(f.rdram,f.player+0xC4))<.01f&&m::scalar(f.rdram,f.player+0xC8)<-1.9f,"Unchanged native24188 writes upward negative world velocity for the UP heading");}

 {Fixture f(argv[2]);q::Controller c;f.in.vi=2950;auto out=f.step(c);f.pose(out.lane,f.in.y);need(f.step(c).stage==q::Stage::Ascend,"Real old swim capture starts observed return controller");
  Fixture air(argv[3]);need(air.in.primary_state==1&&n::byte(air.rdram,air.player+0x30)==14&&n::byte(air.rdram,air.p+0xF7)==1,"Actual60c8 snapshot is eligible native aerial14 with F7=1");air.in.vi=3000;out=air.step(c);need(out.handled&&out.stage==q::Stage::Ascend&&(out.buttons&d::jump_button),"A remains held at exact old45ON/15OFF gap during real airborne posture");
  const auto map=m::Map::read(air.rdram);const auto b=q::body(air.rdram,air.player);need(map&&b&&!q::clear(air.rdram,*map,*b,air.in.x,air.in.y,408,air.in.y,false),"Current real apex does not fake a full-foot crossing corridor");
  air.fl(air.player+0xC8,4.8f);air.byte(0x800F8995,0);recomp_context ctx{};ctx.r29=n::base(0x800FF800);ctx.r4=n::base(air.player);flutter_requests=0;func_8003431C(air.rdram,&ctx);need(flutter_requests==0,"Exact native3431C refuses flutter when old pilot releases A despite F7=1");
  air.byte(0x800F8995,1);ctx.r4=n::base(air.player);func_8003431C(air.rdram,&ctx);need(flutter_requests==1,"Exact native3431C dispatches ordinary flutter request with heldA/C8>2.7/F7=1");
  air.pose(352,271,14);out=air.step(c);need(out.handled&&out.stage==q::Stage::Return&&out.x>0&&(out.buttons&d::jump_button),"First full-foot-clear corridor returns immediately, without waiting for248");
  air.pose(360,273,14);out=air.step(c);need(out.handled&&out.stage==q::Stage::Blocked&&!out.x&&(out.buttons&d::jump_button),"Airborne Return rechecks full-foot corridor and retains native flutter A without driving into platform side");
  air.pose(392,273,0);air.byte(air.p+0xF4,0);air.refresh();out=air.step(c);need(out.handled&&out.stage==q::Stage::Settle&&!out.buttons,"Actual supported landing enters dedicated no-A braking/recentring");
 }
 {Fixture f(argv[2]);q::Controller c;auto out=f.step(c);f.pose(out.lane,f.in.y);f.step(c);f.pose(346.69214f,330.68076f,14);const auto drift_map=m::Map::read(f.rdram);const auto drift_body=q::body(f.rdram,f.player);need(drift_map&&drift_body&&!q::clear(f.rdram,*drift_map,*drift_body,f.in.x,f.in.y,352,248)&&q::clear(f.rdram,*drift_map,*drift_body,f.in.x,f.in.y,352,271),"Actual0401 pixel blocks the old tall diagonal but not the new first crossing corridor");out=f.step(c);need(out.handled&&out.stage==q::Stage::Ascend&&out.x>0&&out.y==1&&(out.buttons&d::jump_button),"Actual0401 drift uses the proved shorter corridor and steers back toward the free column with aerial A");
  f.pose(352,330.68076f,14);out=f.step(c);need(out.stage==q::Stage::Ascend&&out.y==1&&(out.buttons&d::jump_button),"Realigned whole-body vertical column re-admits ordinary ascent");
 }

 {Fixture f(argv[2]);q::Controller c;auto out=f.step(c);f.pose(out.lane,f.in.y);f.step(c);
  f.pose(412.37375f,273,0);f.byte(f.p+0xF4,0);f.fl(f.player+0xC4,1.5f);f.refresh();out=f.step(c);need(out.stage==q::Stage::Settle&&!out.buttons&&out.x<0,"Observed landing412.37/273 brakes real positive velocity without A");
  f.pose(416.71371f,273,0);f.fl(f.player+0xC4,.6f);f.refresh();out=f.step(c);need(out.stage==q::Stage::Settle&&!out.buttons&&out.x<0,"Actual-support drift416.71 beyond8px never returns to aerial A/Return");
  f.fl(f.player+0xC4,0);f.refresh();++f.in.vi;out=f.step(c);need(out.stage==q::Stage::Settle&&!out.buttons&&out.x<0&&!out.completed,"Stopped supported drift recentres using small native4 input with A released");
  f.pose(411.9f,273,0);out=f.step(c);need(!out.completed&&!out.buttons,"First centred stopped current-support observation waits");out=f.step(c);need(!out.completed,"Repeated sameVI is not a second native support observation");++f.in.vi;need(f.step(c).completed,"Next distinct stable native observation completes and returns to Driver");
 }
 std::cout<<"FishWaterRecovery0260Checks: "<<checks<<" PASS; real C217 map/body snapshots, unchanged native24188 velocity writer and HOST navigation boundaries; GPU swim/landing/victory remain unverified.\n";return 0;
 }catch(const std::exception&ex){std::cerr<<ex.what()<<'\n';return 1;}}
