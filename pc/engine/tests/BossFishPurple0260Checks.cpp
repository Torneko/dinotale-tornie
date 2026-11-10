#include "../boss_fish_purple0260.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#ifndef PURPLE_NATIVE_WATER_SLICE
#error Supply literal original native water and speed-query bodies
#endif
extern "C" void do_break(uint32_t){std::abort();} // Any native division assertion fails the CPU oracle.
int32_t sections[512]{};int32_t* section_addresses=sections;
namespace actual_native {
void func_80052CDC(uint8_t*,recomp_context*){throw std::runtime_error("Actual capture already has a native geometry actor; no source initialization is allowed");}
void func_800483B8(uint8_t*,recomp_context*);void func_80048F84(uint8_t*,recomp_context*);void func_8006593C(uint8_t*,recomp_context*);void func_8006A99C(uint8_t*,recomp_context*);void func_8006AA74(uint8_t*,recomp_context*);void func_8006AB7C(uint8_t*,recomp_context*);void func_8006ABF0(uint8_t*,recomp_context*);void func_80053924(uint8_t*,recomp_context*);void func_805C6A70(uint8_t*,recomp_context*);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include PURPLE_NATIVE_WATER_SLICE
#pragma clang diagnostic pop
}
namespace p=workshop::boss_fish_purple0260;namespace n=workshop::boss_native;
unsigned checks=0,native_calls=0;void need(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
struct Fixture{
 std::vector<uint8_t> memory;uint8_t* rdram;static constexpr uint32_t a=0x800C9CC0;recomp_context c{};p::Frame frame;
 Fixture(const std::vector<uint8_t>& source):memory(source),rdram(memory.data()),frame{true,true,true,97,a,n::word(rdram,a+0x1F4),1}{byte(0x800F7EA5,0);/* Explicit live-combat reconstruction: captured final is paused1/1 afterP1death. */c.r29=n::base(0x807EF000);c.r4=n::base(a+0xC4);c.r5=0;}
 void word(uint32_t at,uint32_t v){MEM_W(0,n::base(at))=int32_t(v);}void byte(uint32_t at,unsigned v){MEM_B(0,n::base(at))=uint8_t(v);}void fl(uint32_t at,float v){word(at,std::bit_cast<uint32_t>(v));}float f(uint32_t at){return std::bit_cast<float>(n::word(rdram,at));}
 void speed(float x,float vx){fl(a+0x84,x);fl(a+0x3C,x);fl(a+0xC4,vx);fl(a+0xDC,vx);fl(a+0xE8,.3f);}
 bool water(float x,float y){fl(a+0x84,x);fl(a+0x88,y);c.r4=n::base(a);actual_native::func_805C6A70(rdram,&c);++native_calls;return c.r2!=0;}
};
int main(int argc,char** argv){try{
 need(argc==2,"Supply actual9714 finalRAM");std::ifstream input(argv[1],std::ios::binary);std::vector<uint8_t> source((std::istreambuf_iterator<char>(input)),{});need(source.size()==0x800000,"Actual8MiB native map required");Fixture raw(source);need(n::actor(raw.rdram,Fixture::a,0x4631)&&n::byte(raw.rdram,Fixture::a+0x30)==7,"Source is actual live runaway purple4631 in native1/7");const auto basin=p::basin(raw.rdram);need(basin&&basin->left==16&&basin->right==752,"Actual3x2nativepage map yields interior bounds");need(raw.water(400,362),"Literal native6A70/53924/6ABF0 finds actual water inside basin");need(!raw.water(1292,362),"Literal native water test fails after horizontal escape even at return depth");need(!raw.water(-184,2823),"Observed runaway location never gets a native water receipt");
 for(float vx:{-4.5f,-3.f,0.f,3.f,4.5f})for(float target:{-3.f,0.f,3.f}){Fixture x(source);x.fl(Fixture::a+0xC4,vx);x.fl(Fixture::a+0xDC,target);x.fl(Fixture::a+0xE8,.3f);const auto expected=p::next_speed(vx,target,.3f);x.c.r4=n::base(Fixture::a+0xC4);x.c.r5=0;actual_native::func_80048F84(x.rdram,&x.c);++native_calls;need(x.f(Fixture::a+0xC4)==expected,"Lookahead matches literal native48F84 including acceleration/overshoot");}
 for(bool right:{false,true}){Fixture x(source);x.speed(right?750.f:18.f,right?3.f:-3.f);x.fl(Fixture::a+0x88,300);const auto before=x.memory;const auto saved=x.c;need(p::before_axis(x.rdram,&x.c,x.frame),"Actual owned Purple callback reflects before leaving water-map interior");auto expected=before;auto* rdram=expected.data();const auto v=std::bit_cast<uint32_t>(right?-3.f:3.f);MEM_W(0xC4,n::base(Fixture::a))=v;MEM_W(0xDC,n::base(Fixture::a))=v;need(x.memory==expected&&!std::memcmp(&saved,&x.c,sizeof(saved)),"Only two horizontal floats change; complete context and other8MiB preserved");actual_native::func_80048F84(x.rdram,&x.c);++native_calls;const auto next=x.f(Fixture::a+0x84)+x.f(Fixture::a+0xC4);need(next>=16&&next<752,"Next literal native X step stays inside actual basin");need(x.water(next,362),"Actual native water return remains reachable after reflected movement");}
 {Fixture x(source);x.speed(400,3);const auto before=x.memory;need(!p::before_axis(x.rdram,&x.c,x.frame)&&x.memory==before,"Interior native movement is untouched");}
 for(unsigned fault=0;fault<12;++fault){Fixture x(source);x.speed(750,3);switch(fault){case 0:x.frame.purple=false;break;case 1:x.frame.birth=0;break;case 2:x.frame.room=171;break;case 3:x.frame.combat_active=false;break;case 4:x.word(x.frame.private_data+0x40,1);break;case 5:x.byte(Fixture::a+0x30,0xD);break;case 6:x.word(Fixture::a+0x20,0x80000000);break;case 7:x.byte(0x800F7EA5,1);break;case 8:x.c.r5=1;break;case 9:x.word(Fixture::a+0x1F4,x.frame.private_data+4);break;case 10:x.word(0x800A60D0,0);break;case 11:x.fl(Fixture::a+0xE8,-.3f);break;}const auto before=x.memory;need(!p::before_axis(x.rdram,&x.c,x.frame)&&x.memory==before,"Wrong owner/phase/birth/source/passenger/axis/map/physics is rejected unchanged");}
 // Explicit no-interpolation fixture: actual native483B8 performs every
 // position step; publishing local->world here is a named synthetic frame
 // fence, not a claim to execute the complete native rendering/AI graph.
 for(unsigned sub:{3u,5u,6u})for(bool right:{false,true}){Fixture x(source);const auto resource=n::word(x.rdram,Fixture::a+0x168),factory=n::word(x.rdram,resource+0x40);x.byte(Fixture::a+0x30,sub);x.word(Fixture::a+0x20,factory+(p::updates[sub]-0x805C64F0));x.speed(right?750.f:18.f,right?3.f:-3.f);x.fl(Fixture::a+0x40,300);x.fl(Fixture::a+0x88,300);x.fl(Fixture::a+0xC8,5);x.fl(Fixture::a+0xDC,right?3.f:-3.f);need(n::word(x.rdram,Fixture::a+0x130)==0x800483B8&&x.f(Fixture::a+0x100)==1&&x.f(Fixture::a+0x10C)==0,"Actual source installs483B8 and has native factor1/external0");bool reflected=false,water=false;for(unsigned tick=0;tick<20;++tick){x.c.r4=n::base(Fixture::a);const auto saved=x.c;const auto before=x.memory;const bool changed=p::before_integrate(x.rdram,&x.c,x.frame);reflected|=changed;need(!std::memcmp(&saved,&x.c,sizeof(saved)),"Physical boundary hook leaves all native registers untouched");if(changed){auto expected=before;auto* rdram=expected.data();MEM_W(0xC4,n::base(Fixture::a))=n::word(x.rdram,Fixture::a+0xC4);MEM_W(0xDC,n::base(Fixture::a))=n::word(x.rdram,Fixture::a+0xDC);need(x.memory==expected,"Physical hook changes onlyC4/DC, never any position or vertical data");}actual_native::func_800483B8(x.rdram,&x.c);++native_calls;const float xx=x.f(Fixture::a+0x3C),yy=x.f(Fixture::a+0x40);need(xx>=16&&xx<752&&yy==300+5.f*float(tick+1),"Every original483B8 step remains in basin while nativeY advances unchanged");x.fl(Fixture::a+0x84,xx);x.fl(Fixture::a+0x88,yy);if(yy>362){water=x.water(xx,yy);break;}}need(reflected&&water,"Previously unhooked native3/5/6 paths can reach the real6A70 water gate without48F84");}
 for(unsigned fault=0;fault<4;++fault){Fixture x(source);x.speed(750,3);x.c.r4=n::base(Fixture::a);if(fault==0)x.fl(Fixture::a+0x10C,1);if(fault==1)x.word(Fixture::a+0x130,0x80048378);if(fault==2)x.word(Fixture::a+0x180,0x80010000);if(fault==3){const auto meta=n::word(x.rdram,Fixture::a+0x174);x.word(meta+8,1);}const auto before=x.memory;need(!p::before_integrate(x.rdram,&x.c,x.frame)&&x.memory==before,"Unknown external motion/foreign physics/parent/native right blocker is never overridden");}
 std::cout<<"BossFishPurple0260Checks: "<<checks<<" PASS; nativeCalls="<<native_calls<<"; actual captured water map, reflected future steps, no GPU/full-arc claim.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
