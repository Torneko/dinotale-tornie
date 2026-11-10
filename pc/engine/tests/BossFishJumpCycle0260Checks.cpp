#include <cstdio>
#include "../boss_variants0250.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
namespace v=workshop::boss_variants0250;namespace b=workshop::boss_battle0250;namespace n=workshop::boss_native;
unsigned checks=0;void need(bool q,const char* why){++checks;if(!q)throw std::runtime_error(why);}
// Explicit HOST fixture: native actor/resource layout and native call boundary,
// not an executed native AI loop or evidence of a GPU victory.
struct Fixture {
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* rdram=memory.data();recomp_context c{};
 static constexpr uint32_t a=0x80010000,p=0x80030000,res=0x80040000,factory=0x80200000;
 v::Frame f{97,2,true};
 void w(uint32_t x,uint32_t y){MEM_W(0,n::base(x))=y;}void by(uint32_t x,unsigned y){MEM_B(0,n::base(x))=uint8_t(y);}void half(uint32_t x,unsigned y){MEM_H(0,n::base(x))=uint16_t(y);}void fl(uint32_t x,float y){w(x,std::bit_cast<uint32_t>(y));}
 void sub(unsigned s){by(a+0x30,s);w(a+0x20,factory+(s==1?0x173C:workshop::boss_fish_green0260::updates[s]-0x805C64F0));}
 Fixture(){auto e=b::make_plan(1).fights[4];need(e.family==b::Family::Fish&&e.count==3&&e.members[2].purple,"Seed1 is threeFish with actual purple slot2");need(v::begin(e,4),"Production variant descriptor accepted");w(a,0x19971110);half(a+12,0x4511);by(a+0x17D,2);by(a+0x18,1);w(a+0x168,res);half(res+0x3C,0x4511);w(res+0x40,factory);w(res+0x44,factory+0x35A8);w(a+0x1F4,p);w(a+0x130,0x800483B8);by(0x800F7EA6,1);need(v::bind(rdram,2,a),"Purple generation bound");c.r4=n::base(a);c.r5=1;c.r6=1;v::trace(rdram,&c,"func_805C9D10",f);sub(1);v::trace(rdram,&c,"func_805C7C2C",f);need(v::state.members[2].active&&v::state.members[2].initialized,"Native constructor/activity boundaries admitted");}
 void mark_jump(){sub(5);c.r4=n::base(a);c.r5=c.r6=0;v::trace(rdram,&c,"func_800483B8",f);}
 void finish(){sub(12);c.r4=n::base(a);c.r5=c.r6=1;c.r31=n::base(factory+0x2CFC);v::trace(rdram,&c,"func_805C9D10",f);}
};
int main(int argc,char**argv){try{
 need(argc==2,"Supply literal funcs146 native source");std::ifstream f(argv[1]);std::string source((std::istreambuf_iterator<char>(f)),{});const auto begin=source.find("RECOMP_FUNC void func_805C9154");need(begin!=std::string::npos&&source.find("// 0x805C91E4: jal         0x805C9D10",begin)!=std::string::npos&&source.find("// 0x805C91E8: addiu       $a2, $zero, 0x1",begin)!=std::string::npos,"Original9154 proves1/12 return call and91EC caller address");
 {Fixture x;const auto before=x.memory;x.c.r4=n::base(Fixture::a);x.c.r5=x.c.r6=1;v::trace(x.rdram,&x.c,"func_805C9D10",x.f);need(v::state.members[2].purple_phase==v::PurplePhase::Arc&&x.memory==before,"Repeated constructor1/1 never alternates or writesRAM");x.mark_jump();need(v::state.members[2].jump_started,"Actual free-jump physics callback marks current owned jump");const auto ram=x.memory;const auto ctx=x.c;x.finish();need(v::state.members[2].purple_phase==v::PurplePhase::Vertical&&!v::state.members[2].jump_started&&x.memory!=ram,"Native free return selects vertical next; fixture only changed sub/callback");const auto after=x.memory;v::trace(x.rdram,&x.c,"func_805C9D10",x.f);need(v::state.members[2].purple_phase==v::PurplePhase::Vertical&&x.memory==after,"Same return cannot toggle twice");(void)ctx;
  x.mark_jump();need(bool(v::green_frame(x.rdram,Fixture::a,x.f))&&!v::purple_frame(x.rdram,Fixture::a,x.f),"Purple vertical cycle selects exact Green motion adapter only");x.fl(Fixture::a+0xC4,3);x.fl(Fixture::a+0xDC,5);x.fl(Fixture::a+0xE8,.6f);x.fl(Fixture::a+0xC8,-5);x.fl(Fixture::a+0xE0,5);x.fl(Fixture::a+0xEC,.2f);x.c.r4=n::base(Fixture::a);x.c.r31=n::base(Fixture::factory+0x2378);v::leave(x.rdram,&x.c,"func_805C74C8",x.f);need(n::word(x.rdram,Fixture::a+0xC4)==0&&n::word(x.rdram,Fixture::a+0xC8)==std::bit_cast<uint32_t>(-5.5f),"Purple vertical cycle inherits zeroX and existing Green launch height");x.finish();need(v::state.members[2].purple_phase==v::PurplePhase::Arc,"Second completed native jump returns to arc");x.sub(5);need(bool(v::purple_frame(x.rdram,Fixture::a,x.f))&&!v::green_frame(x.rdram,Fixture::a,x.f),"Purple arc cycle keeps native red movement and basin guard");}
 for(unsigned fault=0;fault<9;++fault){Fixture x;x.mark_jump();x.sub(12);x.c.r4=n::base(Fixture::a);x.c.r5=x.c.r6=1;x.c.r31=n::base(Fixture::factory+0x2CFC);if(fault==0)x.c.r31+=4;if(fault==1)x.sub(1);if(fault==2)x.w(Fixture::p+0x40,1);if(fault==3)x.f.combat_active=false;if(fault==4)x.by(0x800F7EA6,0);if(fault==5)v::state.members[2].jump_started=false;if(fault==6)x.w(Fixture::a+0x1F4,Fixture::p+4);if(fault==7)x.c.r6=0;if(fault==8)x.f.room=171;const auto before=x.memory;const auto ctx=x.c;v::trace(x.rdram,&x.c,"func_805C9D10",x.f);need(v::state.members[2].purple_phase==v::PurplePhase::Arc&&x.memory==before&&!std::memcmp(&ctx,&x.c,sizeof(ctx)),"Foreign caller/passenger/intro/pause/stale owner cannot toggle or mutateRAM/registers");}
 {Fixture x;x.sub(1);for(unsigned vi=3;vi<500;vi+=17){x.f.vi=vi;v::frame(x.rdram,x.f);for(auto a:v::take_actions())need(a.operation!=v::Operation::Water&&a.operation!=v::Operation::Jump,"No timed water or forced jump action generated");}v::Action a{v::Operation::Water,4,2,500,Fixture::a,v::state.members[2].birth,0x805C72B0,{Fixture::a,0,0,0}};need(!v::action_valid(x.rdram,a),"Legacy Water native action explicitly rejected");a.operation=v::Operation::Jump;need(!v::action_valid(x.rdram,a),"Legacy forced Jump action explicitly rejected");need(!v::skip_native_update(x.rdram,&x.c,"func_805C836C",x.f),"Every original jump AI callback remains enabled");}
 std::cout<<"BossFishJumpCycle0260Checks: "<<checks<<" PASS; explicit HOST actor/call-boundary fixtures, no GPU victory claim.\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
