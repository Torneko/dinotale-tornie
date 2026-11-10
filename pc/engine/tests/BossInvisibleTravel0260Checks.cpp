#include "../boss_driver0260.hpp"
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
namespace d=workshop::boss_driver0260;
static unsigned checks=0;
static void need(bool yes,const char* why){++checks;if(!yes)throw std::runtime_error(why);}
static bool same(d::Output a,d::Output b){return a.handled==b.handled&&a.buttons==b.buttons&&a.x==b.x&&a.y==b.y;}
struct Fixture {
 d::Controller c;std::array<d::Target,2> targets{{{0x800CB0A0,0x4599,213.862717f,339.044220f,0,0,true,true,false},{0x800CB31C,0x4599,375.489716f,293.011780f,0,0,true,true,false}}};d::Input in;
 Fixture(){need(c.configure(true,true,{},false),"HOST diagnostic input-only fixture");in.vi=31391;in.room=172;in.encounter=5;in.player=0x800CA6B0;in.combat=in.controls=in.alive=in.grounded=true;in.primary_state=1;in.reserves=6;in.x=56;in.y=449;in.min_x=40;in.max_x=724;in.has_preferred_x=in.invisible_navigation=true;in.preferred_x=324;in.targets=targets;}
 d::Output at(unsigned vi){in.vi=vi;return c.step(in);}
};
int main(int argc,char** argv){try{
 need(argc==2,"Supply immutable failing Full7e09 log");std::ifstream f(argv[1]);const std::string log((std::istreambuf_iterator<char>(f)),{});
 need(log.find("vi=31571 room=172 encounter=5 phase=3")!=std::string::npos&&log.find("vi=31680 player=800CA6B0 state=1 reserves=5 aim=800C6AB4 reticle=800C3D6C position=140.0,449.0")!=std::string::npos,"Actual failure establishes premature Face at180VI and low aiming origin; fixture never claims native motion simulation");
 {Fixture q;need(q.at(31391).x>0,"Travel starts toward the published anchor");q.in.x=140.01f;for(unsigned vi:{31570u,31571u,31575u,31680u}){const auto out=q.at(vi);need(out.x>0&&!(out.buttons&(d::egg_button|d::tongue_button))&&q.c.snapshot().phase==d::Phase::Approach,"Actual180VI boundary must keep ordinary travel rather than start an egg aim atX140");}need(q.c.snapshot().attempts==0&&q.c.snapshot().launches==0,"Travel input never certifies a shot or native launch");}
 {Fixture q;q.at(31391);q.targets[0].vulnerable=q.targets[1].vulnerable=false;q.in.x=140;const auto out=q.at(31571);need(out.x>0&&q.c.snapshot().invisible_travel_vi==31391,"Native visibility changes cannot restart or suspend the standing-anchor trip");}
 {Fixture q;q.at(31391);q.in.x=160;q.in.spring_attached=true;need(!q.at(31600).buttons&&q.c.snapshot().phase==d::Phase::SpringRelease,"True spring attachment first releases previous buttons");need(q.at(31602).buttons==d::jump_button,"Existing native spring release sends its fresh A edge");q.in.spring_attached=q.in.grounded=false;q.in.x=224;q.in.y=365;const auto out=q.at(31603);need(out.x>0&&out.buttons==d::jump_button&&q.c.snapshot().phase==d::Phase::SpringRelease,"Native detachment keeps the existing bounded escape to the anchor");q.in.x=324;q.in.y=329;q.in.grounded=true;const auto arrived=q.at(31664);need(!q.c.snapshot().invisible_travelling&&q.c.snapshot().phase==d::Phase::Face&&!(arrived.buttons&d::egg_button),"Observed anchor arrival restores facing rather than inventing a ready shot");need(q.at(31668).buttons==d::egg_button&&q.c.snapshot().attempts==1,"Only arrival plus ordinary facing interval permits the real Z request");}
 for(float x:{321.f,327.f,320.99f,327.01f}){Fixture q;q.at(31391);q.in.x=x;const auto out=q.at(31571);const bool arrived=std::abs(x-324.f)<=3;need(arrived?!q.c.snapshot().invisible_travelling:q.c.snapshot().invisible_travelling,"Arrival tolerance uses the observed coordinate, not elapsed time");if(!arrived)need(!(out.buttons&d::egg_button)&&out.x*(324.f-x)>0,"A position outside arrival tolerance still walks in the correct direction");}
 {Fixture q;q.at(31391);q.in.x=140;need(q.at(33190).x>0,"Bounded trip retains its last admitted native travel frame");for(unsigned vi:{33191u,33192u,33500u}){const auto out=q.at(vi);need(!out.buttons&&!out.x&&!out.y&&q.c.snapshot().invisible_travel_expired,"Unreachable anchor times out neutral without falling through to an egg shot");}need(q.c.snapshot().attempts==0,"Timeout never records a forced aim or relaxed victory");}
 for(unsigned fault=0;fault<9;++fault){Fixture q;q.at(31391);q.in.x=140;if(fault==0)q.in.controls=false;if(fault==1)q.in.alive=false;if(fault==2)q.in.combat=false;if(fault==3)q.in.player=0;if(fault==4)q.in.has_preferred_x=false;if(fault==5)q.in.preferred_x=900;if(fault==6)for(auto& t:q.targets)t.active=false;if(fault==7)for(auto& t:q.targets)t.defeated=true;if(fault==8)for(auto& t:q.targets)t.kind=0x4072;const auto out=q.at(31571);need(!out.buttons&&!out.x&&!out.y,"Invalid native input, anchor or owned Invisible cannot license travel or an aim");}
 {Fixture q;q.in.invisible_navigation=false;q.at(31391);q.in.x=140;q.at(31571);need(q.c.snapshot().phase==d::Phase::Face,"Default-off opt-in preserves the old generic180VI branch");}
 for(auto mode:{d::ThrowMode::HoldAndRelease,d::ThrowMode::SecondPress}){Fixture q,old;q.in.throw_mode=old.in.throw_mode=mode;old.in.invisible_navigation=false;q.in.x=old.in.x=140;q.in.aiming_actor=old.in.aiming_actor=0x800C6AB4;q.in.reticle_actor=old.in.reticle_actor=0x800C3D6C;q.in.reticle_x=old.in.reticle_x=214;q.in.reticle_y=old.in.reticle_y=339;for(unsigned vi=31391;vi<31399;++vi)need(same(q.at(vi),old.at(vi)),"A real4005 keeps exactly the existing aim/release controls in both throw modes");}
 std::cout<<"BossInvisibleTravel0260Checks: "<<checks<<" PASS; actual-log regression/180VI, observed arrival, native spring controls,1800VI neutral timeout and negative/default scopes. HOST inputs only; no native travel or win is claimed.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
