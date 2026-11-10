#include "../boss_fish_shot_trace0260.hpp"
#include <algorithm>
#include <bit>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <vector>
namespace t=workshop::boss_fish_shot_trace0260;namespace n=workshop::boss_native;
static unsigned checks=0;
static void need(bool yes,const char* why){++checks;if(!yes){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
static void put(uint8_t* rdram,uint32_t a,uint32_t value){MEM_W(0,n::base(a))=int32_t(value);}
static void byte(uint8_t* rdram,uint32_t a,unsigned value){MEM_B(0,n::base(a))=uint8_t(value);}
static void half(uint8_t* rdram,uint32_t a,unsigned value){MEM_H(0,n::base(a))=uint16_t(value);}
static void actual(uint8_t* ram,const std::vector<uint8_t>& original,uint32_t egg,const t::Frame& f){
 t::Trace trace;auto* shot=trace.begin(ram,f,3700,3,3,egg,100);
 need(shot&&trace.count==1&&trace.rows==0,"Actual captured4002 creates a diagnostic host ticket without advancing a row");
 need(shot->egg==egg&&shot->private_data==n::word(ram,egg+0x1F4)&&shot->resource==n::word(ram,egg+0x168)&&shot->serial==3&&shot->release_serial==3&&shot->pulse_floor==100,"Ticket retains exact native identity and launch/release provenance");
 need(t::same(shot->context,f)&&t::egg_identity(ram,*shot,egg),"Captured launch context and native tuple remain exact");
 need(!trace.sample(ram,f,*shot,3700)&&!trace.sample(ram,f,*shot,3699),"Same-VI and backwards samples are inert");
 need(trace.sample(ram,f,*shot,3701)&&trace.rows==1,"First subsequent VI is observed");
 need(!trace.sample(ram,f,*shot,3701)&&!trace.sample(ram,f,*shot,3702)&&trace.rows==1,"Duplicate and adjacent VI respect the2VI cadence");
 need(trace.sample(ram,f,*shot,3703),"Second ordinary sample is admitted after2VI");
 need(trace.finish(ram,f,3703,egg)==shot&&shot->ended&&trace.rows==3,"Native disposal-entry observation records the exact tuple once");
 need(!trace.finish(ram,f,3703,egg)&&trace.rows==3,"Repeated disposal event cannot duplicate its row");
 need(trace.sample(ram,f,*shot,3705),"Sampling may continue after disposal-entry to observe later proven4018");
 need(!trace.begin(ram,f,3706,3,4,egg,101)&&!trace.begin(ram,f,3700,4,4,egg,101),"Serial and same-VI/native-tuple duplicates cannot create another ticket");
 need(std::equal(original.begin(),original.end(),ram),"All actual capture observation paths preserve every one of8MiB");
}
static void scope(std::vector<uint8_t>& memory,uint32_t egg,const t::Frame& f){auto* ram=memory.data();const auto original=memory;
 for(unsigned fault=0;fault<10;++fault){auto bad=f;switch(fault){case 0:bad.enabled=false;break;case 1:bad.player=egg;break;case 2:bad.source=151;break;case 3:++bad.room;break;case 4:bad.encounter=9;break;case 5:bad.token.clear();break;case 6:bad.player=0;break;case 7:bad.source=177;break;case 8:bad.encounter=0;break;case 9:bad.room=83;break;}
  t::Trace trace;need(!trace.begin(ram,bad,100,1,1,egg,0)&&trace.count==0&&trace.rows==0,"Wrong scene/source/player/token cannot begin a trace");
 }
 for(unsigned fault=0;fault<8;++fault){t::Trace trace;auto* s=trace.begin(ram,f,100,1,1,egg,0);need(s!=nullptr,"Create exact ticket before context rejection");auto bad=f;switch(fault){case 0:bad.enabled=false;break;case 1:++bad.epoch;break;case 2:bad.token+="-foreign";break;case 3:bad.player=egg;break;case 4:++bad.room;break;case 5:bad.source=162;break;case 6:bad.encounter=9;break;case 7:bad.token.clear();break;}
  need(!trace.sample(ram,bad,*s,101)&&!trace.finish(ram,bad,101,egg)&&trace.rows==0&&!s->ended,"Existing ticket cannot cross epoch/token/source/player/room boundaries");
 }
 t::Trace reset;need(reset.begin(ram,f,100,1,1,egg,0)!=nullptr,"Begin prior context before reconfiguration");auto next=f;++next.epoch;need(reset.begin(ram,next,200,1,1,egg,0)&&reset.count==1&&reset.rows==0&&reset.shots[0].context.epoch==next.epoch,"New authenticated context clears prior host launch tickets");reset.reset();need(reset.count==0&&reset.rows==0&&!reset.shots[0].serial,"Explicit configuration reset removes every diagnostic ticket");
 need(!reset.begin(nullptr,f,100,1,1,egg,0)&&!reset.begin(ram,f,100,0,1,egg,0)&&!reset.begin(ram,f,100,1,1,f.player,0),"Null RAM, zero serial and foreign native kind stay inert");
 need(memory==original,"Context and reset checks preserve all captured RDRAM");
}
static void reuse(std::vector<uint8_t>& memory,uint32_t egg,const t::Frame& f){const auto original=memory;auto* ram=memory.data();
 for(unsigned fault=0;fault<5;++fault){memory=original;ram=memory.data();t::Trace trace;auto* s=trace.begin(ram,f,100,1,1,egg,0);need(s!=nullptr,"Create exact original native ticket before HOST retirement/reuse frontier");
  switch(fault){case 0:put(ram,egg+0x1F4,s->private_data+4);break;case 1:put(ram,egg+0x168,s->resource+4);break;case 2:put(ram,egg,0);break;case 3:half(ram,egg+12,0x4018);break;case 4:put(ram,egg+0x1F4,0x807FFFFC);break;}
  const auto changed=memory;need(!t::egg_identity(ram,*s,egg)&&!trace.finish(ram,f,101,egg),"Reused/foreign/private/resource tuple cannot certify disposal of the old egg");
  need(trace.sample(ram,f,*s,101)&&trace.rows==1,"The original launch window remains available for its separately attested pulse after egg reuse");need(memory==changed,"Even malformed native tuples are read-only");
 }
 memory=original;ram=memory.data();t::Trace trace;auto* s=trace.begin(ram,f,100,1,1,egg,0);need(s!=nullptr,"Create native identity before retired-phase readback");byte(ram,egg+0x18,0xFF);byte(ram,egg+0x17D,0);const auto retired=memory;
 need(t::egg_identity(ram,*s,egg)&&trace.sample(ram,f,*s,101),"Retired phase is recorded separately from stable original address/private/resource identity");need(memory==retired,"Retired tuple observation preserves8MiB");memory=original;
}
static void bounds(uint8_t* ram,uint32_t egg,const t::Frame& f){t::Trace trace;
 for(unsigned i=0;i<t::Trace::max_shots;++i)need(trace.begin(ram,f,100+100*i,i+1,i+1,egg,i)!=nullptr,"Eight distinct launch events fit the fixed ticket array");
 need(trace.count==8&&!trace.begin(ram,f,1000,9,9,egg,0)&&trace.count==8,"Ninth launch cannot exceed the diagnostic ticket bound");
 for(auto& s:trace.shots){need(!trace.sample(ram,f,s,s.vi)&&!trace.sample(ram,f,s,s.vi+61),"Each launch has a strict future60VI window");for(unsigned age=1;age<=59;age+=2)need(trace.sample(ram,f,s,s.vi+age),"Cadenced observations fit the bounded postshot window");need(trace.finish(ram,f,s.vi+60,egg)==&s,"Exact60VI disposal remains inside the diagnostic window");need(!trace.finish(ram,f,s.vi+61,egg),"Disposal outside the window is not certified");}
 need(trace.rows==248,"Eight full natural windows plus eight disposal rows remain below256rows");
 // Explicit HOST counter frontier exercises the cap without inventing more
 // game actors or pulse events than the fixed8-shot trace can naturally hold.
 t::Trace capped;auto* s=capped.begin(ram,f,100,1,1,egg,0);need(s!=nullptr,"Create ticket at explicit HOST row-counter boundary");capped.rows=255;need(capped.sample(ram,f,*s,101)&&capped.rows==256,"Last permitted diagnostic row reaches256 exactly");need(!capped.sample(ram,f,*s,103)&&!capped.finish(ram,f,103,egg)&&capped.rows==256,"Neither sample nor end event exceeds the shared row cap");
 t::Trace edge;auto* e=edge.begin(ram,f,100,1,1,egg,0);need(e&&edge.sample(ram,f,*e,160)&&!edge.sample(ram,f,*e,161),"Age60 admitted and61 refused independently of cadence history");
}
int main(int argc,char** argv){need(argc==2,"Supply one actual c1de Fish diagnostic8MiB capture");std::ifstream input(argv[1],std::ios::binary);std::vector<uint8_t> memory{std::istreambuf_iterator<char>(input),{}};need(memory.size()==0x800000,"Actual source image is exactly8MiB");auto* ram=memory.data();const uint32_t player=n::word(ram,0x800F8828),egg=0x800C4294;need(n::actor(ram,player,0x4001)&&n::actor(ram,egg,0x4002),"Capture contains the true singleton and native4002 allocation");const t::Frame frame{true,player,97,171,4,0,"qa-fish-shot-c1de"};const auto original=memory;actual(ram,original,egg,frame);scope(memory,egg,frame);reuse(memory,egg,frame);bounds(memory.data(),egg,frame);need(memory==original,"Complete trace suite preserves the actual8MiB source capture");std::printf("BossFishShotTrace0260Checks: %u PASS; capture-backed trace identity/caps/read-only checks; HOST counter/reuse frontiers explicit; no puff synthesis, fish_hit, native gameplay or victory claim.\n",checks);}
