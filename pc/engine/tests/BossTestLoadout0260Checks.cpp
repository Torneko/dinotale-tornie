#include "../boss_test_loadout0260.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace l=workshop::boss_test_loadout0260;namespace i=l::inventory;namespace n=l::n;
static unsigned checks=0,births=0,palettes=0,heals=0,limit=6;static bool bad_palette=false,bad_heal=false,bad_scale=false;
static void need(bool yes,const char* why){++checks;if(!yes)throw std::runtime_error(why);}
static void word(uint8_t* rdram,uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}
static void half(uint8_t* rdram,uint32_t a,unsigned v){MEM_H(0,n::base(a))=uint16_t(v);}
static void byte(uint8_t* rdram,uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}
static void actor(uint8_t* ram,uint32_t a,unsigned kind){word(ram,a,0x19971110);half(ram,a+12,kind);byte(ram,a+0x17D,2);byte(ram,a+0x18,1);}
// Explicit synthetic native boundaries verify argument routing and readback.
// Existing checkpoint inventory oracles cover the actual native bodies.
extern "C" void func_80040860(uint8_t* ram,recomp_context* c){
 const auto p=uint32_t(c->r4),data=n::word(ram,p+0x1F4);need(uint32_t(c->r5)==p+0x84&&i::pointer(uint32_t(c->r6),12),"Native reserve birth uses the actual player position and a validated scale triple");
 const unsigned call=births++;if(call>=limit){c->r2=0;return;}
 const auto a=0x80200000+call*0x400;const auto count=n::byte(ram,data+0x12C8),head=i::half(ram,data+0x12BE);unsigned slot=20;for(unsigned s=0;s<20;++s)if(!n::word(ram,p+0x188+s*4)){slot=s;break;}
 need(count<6&&slot<20,"Native reserve birth is requested only with validated capacity");actor(ram,a,0x4004);word(ram,a+0x180,p);half(ram,a+0x17E,slot);word(ram,a+0x1EC,0);for(unsigned axis=0;axis<3;++axis){const auto scale=n::word(ram,uint32_t(c->r6)+4*axis);word(ram,a+0x6C+4*axis,scale);word(ram,a+0x78+4*axis,bad_scale?std::bit_cast<uint32_t>(.8f):scale);}word(ram,p+0x188+slot*4,a);half(ram,p+0x184,i::half(ram,p+0x184)+1);word(ram,data+0x11FC+((head+count)&7)*24,a);byte(ram,data+0x12C8,count+1);c->r2=n::base(a);c->r16=0xDEADBEEF;
}
extern "C" void func_8003B8B0(uint8_t* ram,recomp_context* c){++palettes;need(n::actor(ram,uint32_t(c->r4),0x4004)&&unsigned(c->r5)<14,"Native palette call targets only the newly-created normal egg");word(ram,uint32_t(c->r4)+0x1EC,bad_palette?14:unsigned(c->r5));c->r16=0xBAD00001;}
extern "C" void func_800422CC(uint8_t* ram,recomp_context* c){++heals;need(c->r4==8,"Explicit QA request heals to native maximum8");word(ram,0x800F8DB4+n::word(ram,0x800F8DB0)*4,bad_heal?7:8);c->r16=0xBAD00002;}
struct Fixture {
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* ram=memory.data();recomp_context context{};l::Controller controller;
 static constexpr uint32_t player=0x80010000,data=0x80011000,body=0x80030000,shadow=0x80030400;
 l::Frame frame{true,true,true,true,4,4,171,97,2408,true,player};
 Fixture(unsigned color=0,unsigned hp=5,unsigned count=0,unsigned head=7){
  births=palettes=heals=0;limit=6;bad_palette=bad_heal=bad_scale=false;actor(ram,player,0x4001);word(ram,player+0x1F4,data);word(ram,0x800F8828,player);word(ram,0x807F0008,color);word(ram,0x800F8DB0,color<8?color:0);word(ram,0x800F8DB4+(color<8?color:0)*4,hp);byte(ram,0x800F7EA6,1);half(ram,data+0x12BE,head);byte(ram,data+0x12C8,count);
  for(unsigned axis=0;axis<3;++axis){word(ram,player+0x6C+4*axis,std::bit_cast<uint32_t>(.8f));word(ram,player+0x78+4*axis,std::bit_cast<uint32_t>(.8f));}word(ram,0x800AFFE8,std::bit_cast<uint32_t>(.7f));
  actor(ram,body,0x400B);actor(ram,shadow,0x4007);word(ram,body+0x180,player);word(ram,shadow+0x180,player);word(ram,player+0x188,body);word(ram,player+0x18C,shadow);half(ram,player+0x184,count+2);
  for(unsigned k=0;k<count;++k){const auto a=0x80100000+k*0x400;actor(ram,a,0x4004);word(ram,a+0x180,player);word(ram,a+0x1EC,(color+k+2)%14);for(unsigned axis=0;axis<3;++axis)word(ram,a+0x78+4*axis,std::bit_cast<uint32_t>(.40f+.03f*float(k)+.01f*float(axis)));word(ram,player+0x188+(k+2)*4,a);word(ram,data+0x11FC+((head+k)&7)*24,a);}
  word(ram,0x800F8958,0x12345678);word(ram,0x800F89D0,0x23456789);context.r29=n::base(0x80780000);context.r16=0x34567890;context.r31=n::base(0x80012340);
 }
};
int main(){try{
 for(unsigned color=0;color<14;++color)for(unsigned count=0;count<=6;++count){Fixture f(color,count%9,count);i::Inventory before;need(i::read(f.ram,Fixture::player,before),"Original wrapped native FIFO is valid");const auto context=f.context;const auto body=std::vector<uint8_t>(f.ram+(Fixture::body&0x7FFFFF),f.ram+(Fixture::body&0x7FFFFF)+0x1FC);need(f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().applied&&f.controller.snapshot().attempted&&!f.controller.snapshot().failed,"Only the explicit manual visible diagnostic fish request applies");i::Inventory after;need(i::read(f.ram,Fixture::player,after)&&after.count==6&&n::word(f.ram,0x800F8DB4+(color<8?color:0)*4)==8,"Native readback confirms full8HP and6eggs for all14Yoshi colours");need(births==6-count&&palettes==6-count&&heals==1,"Native calls only top up the deficit and heal once");
  for(unsigned k=0;k<count;++k)need(after.actors[k]==before.actors[k]&&after.eggs[k]==before.eggs[k],"Existing reserve identities, FIFO order and palettes remain intact");for(unsigned k=count;k<6;++k)need(after.eggs[k]==i::Egg{0x4004,color,{std::bit_cast<uint32_t>(.8f*.7f),std::bit_cast<uint32_t>(.8f*.7f),std::bit_cast<uint32_t>(.8f*.7f)}},"New normal eggs receive the current14-way Yoshi palette through the native setter");
  need(!std::memcmp(&context,&f.context,sizeof(context))&&!std::memcmp(body.data(),f.ram+(Fixture::body&0x7FFFFF),body.size())&&n::word(f.ram,0x800F8958)==0x12345678&&n::word(f.ram,0x800F89D0)==0x23456789,"Guest registers, body, heart statistic and fruit statistic are preserved");const auto once=f.memory;need(f.controller.frame(f.ram,&f.context,f.frame)&&f.memory==once&&births==6-count&&heals==1,"Repeated frame never refills or heals again");word(f.ram,0x800F8DB4+(color<8?color:0)*4,1);const auto consumed=f.memory;need(f.controller.frame(f.ram,&f.context,f.frame)&&f.memory==consumed,"Later player damage does not reapply the initial requested loadout");
 }
 {Fixture f(0,8,6);need(f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().applied&&!births&&!palettes&&!heals,"An already-full loadout is confirmed once without redundant native mutations");}
 for(unsigned fault=0;fault<19;++fault){Fixture f;bool waiting=false;
  if(fault==0)f.frame.requested=false;
  if(fault==1)f.frame.manual=false;
  if(fault==2)f.frame.diagnostic=false;
  if(fault==3)f.frame.visible=false;
  if(fault==4)f.frame.case_index=3;
  if(fault==5){f.frame.vi=2407;waiting=true;}
  if(fault==6){f.frame.fighting=false;waiting=true;}
  if(fault==7)f.frame.index=3;
  if(fault==8)f.frame.room=97;
  if(fault==9)f.frame.source_room=83;
  if(fault==10)word(f.ram,0x800F8828,Fixture::body);
  if(fault==11){byte(f.ram,Fixture::player+0x18,0xA);waiting=true;}
  if(fault==12){byte(f.ram,Fixture::player+0x17D,0);waiting=true;}
  if(fault==13){byte(f.ram,Fixture::player+0x165,1);waiting=true;}
  if(fault==14){word(f.ram,Fixture::player+0x180,Fixture::body);waiting=true;}
  if(fault==15){word(f.ram,Fixture::data+0x12EC,1);waiting=true;}
  if(fault==16){byte(f.ram,0x800F7EA5,1);waiting=true;}
  if(fault==17){byte(f.ram,0x800F7EA6,0);waiting=true;}
  if(fault==18)f.context.r29=n::base(0x807FFFF0);
  const auto before=f.memory;const auto context=f.context;const bool ok=f.controller.frame(f.ram,&f.context,f.frame);need(f.memory==before&&!std::memcmp(&context,&f.context,sizeof(context))&&!births&&!palettes&&!heals&&!f.controller.snapshot().applied&&!f.controller.snapshot().attempted,"Unrequested, foreign or unreleased contexts never mutate native RAM or call constructors");need(ok==(waiting||fault==0)&&f.controller.snapshot().failed==(!waiting&&fault!=0),"Pending native release waits; invalid explicit scope fails closed");
 }
 {Fixture f;for(unsigned k=2;k<17;++k){const auto a=0x80180000+k*0x400;actor(f.ram,a,0x4010);word(f.ram,a+0x180,Fixture::player);word(f.ram,Fixture::player+0x188+k*4,a);}half(f.ram,Fixture::player+0x184,17);const auto before=f.memory;need(!f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().failed&&f.memory==before&&!births&&!heals,"Child capacity is validated before any native reserve allocation");}
 for(unsigned fault=0;fault<3;++fault){Fixture f(10);limit=fault==0?1:6;bad_palette=fault==1;bad_heal=fault==2;const auto context=f.context;need(!f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().attempted&&f.controller.snapshot().failed&&!f.controller.snapshot().applied&&!std::memcmp(&context,&f.context,sizeof(context)),"Partial native allocation/palette/heal failure consumes the request and restores all guest registers");const auto before=f.memory;const auto count=births;need(!f.controller.frame(f.ram,&f.context,f.frame)&&f.memory==before&&births==count,"Failed partial native provisioning cannot be replayed to manufacture further resources");}
 {Fixture f;f.frame.vi=2407;need(f.controller.frame(f.ram,&f.context,f.frame)&&!f.controller.snapshot().applied,"Handoff fence waits before2408");f.frame.vi=2408;need(f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().applied&&f.controller.snapshot().vi==2408,"The same request applies exactly at the authorized live handoff fence");}
 for(unsigned count:{0u,3u}){Fixture f(10,2,count);f.frame.manual=false;f.frame.automatic=true;i::Inventory before;need(i::read(f.ram,Fixture::player,before),"Automatic fixture starts with validated nativeFIFO");const auto saved=f.context;need(f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().applied&&births==6-count&&palettes==6-count&&heals==1,"Explicit automatic visible diagnosticFish uses the same one-shot native8HP6eggs path");i::Inventory after;need(i::read(f.ram,Fixture::player,after)&&after.count==6&&n::word(f.ram,0x800F8DB4)==8,"Automatic native readback confirms expected8/6");for(unsigned k=0;k<count;++k)need(after.actors[k]==before.actors[k]&&after.eggs[k]==before.eggs[k],"Automatic mode preserves existing FIFO order and palettes");const auto once=f.memory;need(f.controller.frame(f.ram,&f.context,f.frame)&&f.memory==once&&births==6-count&&heals==1&&!std::memcmp(&saved,&f.context,sizeof(saved)),"Repeated automatic frames cannot refill or mutate registers again");}
 for(bool both:{false,true}){Fixture f;f.frame.manual=both;f.frame.automatic=both;const auto before=f.memory;need(!f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().failed&&!f.controller.snapshot().attempted&&f.memory==before&&!births&&!heals,"Automatic/manual are strictXOR; neither or both must fail before any native mutation");}
 {Fixture f;f.frame.manual=false;f.frame.automatic=true;f.frame.vi=2407;need(f.controller.frame(f.ram,&f.context,f.frame)&&!f.controller.snapshot().applied&&!births,"Automatic request waits at the same pre2408 native handoff fence");f.frame.vi=2408;need(f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().applied&&f.controller.snapshot().vi==2408,"Automatic readiness is native readback at the same2408 fence");}
 // Explicit targeted Lips diagnostics share the same native top-up path;
 // the unobserved0/0 pair is still closed, including in every Fish case.
 for(bool zero:{false,true}){Fixture f(10,2,3);f.frame.manual=false;f.frame.automatic=true;f.frame.lips_target=true;f.frame.case_index=f.frame.index=1;f.frame.room=168;f.frame.source_room=165;f.frame.proved_zero_controls=zero;byte(f.ram,0x800F7EA6,zero?0:1);
  i::Inventory before;need(i::read(f.ram,Fixture::player,before),"Targeted Lips starts with real validated reserve FIFO");const auto registers=f.context;
  need(f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().applied&&births==3&&palettes==3&&heals==1,"Visible automatic Lips admits0/1 or the exact externally witnessed0/0 and native8/6 once");
  i::Inventory after;need(i::read(f.ram,Fixture::player,after)&&after.count==6&&n::word(f.ram,0x800F8DB4)==8,"Targeted Lips reads native8HP6eggs back");
  for(unsigned k=0;k<3;++k)need(after.actors[k]==before.actors[k]&&after.eggs[k]==before.eggs[k],"Lips top-up preserves existing FIFO identities and palettes");
  const auto once=f.memory;need(f.controller.frame(f.ram,&f.context,f.frame)&&f.memory==once&&births==3&&heals==1&&!std::memcmp(&registers,&f.context,sizeof(registers)),"Lips loadout remains one-shot and preserves guest registers");
 }
 for(unsigned fault=0;fault<10;++fault){Fixture f;f.frame.manual=false;f.frame.automatic=true;f.frame.lips_target=true;f.frame.case_index=f.frame.index=1;f.frame.room=168;f.frame.source_room=165;bool waiting=false;
  if(fault==0)f.frame.case_index=4;
  if(fault==1)f.frame.index=4;
  if(fault==2)f.frame.room=171;
  if(fault==3)f.frame.source_room=97;
  if(fault==4){f.frame.manual=true;f.frame.automatic=false;}
  if(fault==5){byte(f.ram,0x800F7EA6,0);waiting=true;}
  if(fault==6){byte(f.ram,0x800F7EA6,0);f.frame.proved_zero_controls=true;byte(f.ram,0x800F7EA5,1);waiting=true;}
  if(fault==7){byte(f.ram,0x800F7EA6,2);f.frame.proved_zero_controls=true;waiting=true;}
  if(fault==8){f.frame.vi=2407;waiting=true;}
  if(fault==9){f.frame.lips_target=false;}
  const auto before=f.memory;const auto registers=f.context;const bool ok=f.controller.frame(f.ram,&f.context,f.frame);
  need(ok==waiting&&!f.controller.snapshot().attempted&&!f.controller.snapshot().applied&&f.memory==before&&!births&&!heals&&!std::memcmp(&registers,&f.context,sizeof(registers)),"Lips foreign/missing/unproved native scopes never provision resources");
 }
 {Fixture f;f.frame.manual=false;f.frame.automatic=true;f.frame.proved_zero_controls=true;byte(f.ram,0x800F7EA6,0);const auto before=f.memory;need(f.controller.frame(f.ram,&f.context,f.frame)&&!f.controller.snapshot().applied&&f.memory==before&&!births&&!heals,"Fish ignores the new Lips-only0/0 proof field and retains its original0/1 guard");}
 {Fixture f;f.frame.manual=false;f.frame.automatic=true;f.frame.lips_target=true;f.frame.case_index=f.frame.index=1;f.frame.room=168;f.frame.source_room=165;byte(f.ram,0x800F7EA6,0);need(f.controller.frame(f.ram,&f.context,f.frame)&&!f.controller.snapshot().attempted,"Unproved Lips0/0 waits without consuming its request");f.frame.proved_zero_controls=true;need(f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().applied,"The same Lips request may apply only when the Runtime supplies exact native zero-controls proof");}
 // Genie is an appended, exclusive diagnostic target. The original Fish/Lips
 // fixtures above exercise their unchanged defaults and native call routing.
 for(bool zero:{false,true}){Fixture f(9,2,3);f.frame.manual=false;f.frame.automatic=true;f.frame.genie_target=true;f.frame.case_index=f.frame.index=2;f.frame.room=169;f.frame.source_room=163;f.frame.proved_zero_controls=zero;byte(f.ram,0x800F7EA6,zero?0:1);
  i::Inventory before;need(i::read(f.ram,Fixture::player,before),"Genie starts with an actual validated FIFO");const auto registers=f.context;
  need(f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().applied&&births==3&&palettes==3&&heals==1,"Genie admits normal0/1 or witnessed0/0 and applies the native8/6 request once");
  i::Inventory after;need(i::read(f.ram,Fixture::player,after)&&after.count==6&&n::word(f.ram,0x800F8DB4)==8,"Genie native inventory and health read back8/6");
  for(unsigned k=0;k<3;++k)need(after.actors[k]==before.actors[k]&&after.eggs[k]==before.eggs[k],"Genie preserves every existing FIFO identity and palette");
  const auto once=f.memory;need(f.controller.frame(f.ram,&f.context,f.frame)&&f.memory==once&&births==3&&heals==1&&!std::memcmp(&registers,&f.context,sizeof(registers)),"Repeated Genie requests cannot manufacture resources or alter guest registers");
 }
 for(unsigned fault=0;fault<15;++fault){Fixture f;f.frame.manual=false;f.frame.automatic=true;f.frame.genie_target=true;f.frame.case_index=f.frame.index=2;f.frame.room=169;f.frame.source_room=163;bool waiting=false;
  if(fault==0)f.frame.case_index=1;
  if(fault==1)f.frame.index=1;
  if(fault==2)f.frame.room=168;
  if(fault==3)f.frame.source_room=165;
  if(fault==4){f.frame.manual=true;f.frame.automatic=false;}
  if(fault==5){byte(f.ram,0x800F7EA6,0);waiting=true;}
  if(fault==6){byte(f.ram,0x800F7EA6,0);f.frame.proved_zero_controls=true;byte(f.ram,0x800F7EA5,1);waiting=true;}
  if(fault==7){byte(f.ram,0x800F7EA6,2);f.frame.proved_zero_controls=true;waiting=true;}
  if(fault==8){f.frame.vi=2407;waiting=true;}
  if(fault==9)f.frame.genie_target=false;
  if(fault==10)f.frame.lips_target=true;
  if(fault==11)f.frame.automatic=false;
  if(fault==12)f.frame.manual=true;
  if(fault==13){word(f.ram,Fixture::player+0x180,Fixture::body);waiting=true;}
  if(fault==14){word(f.ram,Fixture::data+0x12EC,1);waiting=true;}
  const auto before=f.memory;const auto registers=f.context;const bool ok=f.controller.frame(f.ram,&f.context,f.frame);
  need(ok==waiting&&!f.controller.snapshot().attempted&&!f.controller.snapshot().applied&&f.memory==before&&!births&&!heals&&!std::memcmp(&registers,&f.context,sizeof(registers)),"Foreign/ambiguous/unreleased Genie requests cannot provision native resources");
 }
 {Fixture f;f.frame.manual=false;f.frame.automatic=true;f.frame.genie_target=true;f.frame.case_index=f.frame.index=2;f.frame.room=169;f.frame.source_room=163;byte(f.ram,0x800F7EA6,0);need(f.controller.frame(f.ram,&f.context,f.frame)&&!f.controller.snapshot().attempted,"Genie0/0 without the Runtime witness waits without consuming the request");f.frame.proved_zero_controls=true;need(f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().applied,"The same Genie request applies when the exact native controls witness arrives");}
 // Empty reserves are a Genie-only diagnostic request. Nothing is removed:
 // the ordinary fresh FIFO must already be empty before native health8.
 for(bool zero:{false,true}){Fixture f(11,2,0);f.frame.manual=false;f.frame.automatic=true;f.frame.genie_target=true;f.frame.case_index=f.frame.index=2;f.frame.room=169;f.frame.source_room=163;f.frame.target_eggs=0;f.frame.proved_zero_controls=zero;byte(f.ram,0x800F7EA6,zero?0:1);const auto registers=f.context;
  need(f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().applied&&f.controller.snapshot().added==0&&!births&&!palettes&&heals==1,"Explicit Genie8/0 heals once and never creates or removes an egg");i::Inventory empty;need(i::read(f.ram,Fixture::player,empty)&&empty.count==0&&n::word(f.ram,0x800F8DB4)==8&&!std::memcmp(&registers,&f.context,sizeof(registers)),"Genie8/0 reads back native empty FIFO and full health with guest registers preserved");
  f.context.r4=n::base(Fixture::player);f.context.r5=n::base(Fixture::player+0x84);f.context.r6=n::base(Fixture::player+0x78);func_80040860(f.ram,&f.context);f.context=registers;const auto refilled=f.memory;
  need(f.controller.frame(f.ram,&f.context,f.frame)&&f.memory==refilled&&births==1&&!palettes&&heals==1,"A later native-shaped refill is never cleared or replaced by the one-shot empty-reserve diagnostic");
 }
 for(unsigned count:{1u,6u}){Fixture f(11,2,count);f.frame.manual=false;f.frame.automatic=true;f.frame.genie_target=true;f.frame.case_index=f.frame.index=2;f.frame.room=169;f.frame.source_room=163;f.frame.target_eggs=0;const auto before=f.memory;const auto registers=f.context;
  need(!f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().failed&&!f.controller.snapshot().attempted&&f.memory==before&&!births&&!palettes&&!heals&&!std::memcmp(&registers,&f.context,sizeof(registers)),"Genie8/0 rejects any existing reserve before healing, allocation or clearing native inventory");
 }
 for(unsigned target:{1u,5u,7u,0xFFFFFFFFu}){Fixture f;f.frame.manual=false;f.frame.automatic=true;f.frame.genie_target=true;f.frame.case_index=f.frame.index=2;f.frame.room=169;f.frame.source_room=163;f.frame.target_eggs=target;const auto before=f.memory;need(!f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().failed&&f.memory==before&&!births&&!heals,"Only the exact Genie diagnostic counts0 and6 are admitted");}
 for(bool lips:{false,true}){Fixture f;f.frame.manual=false;f.frame.automatic=true;f.frame.target_eggs=0;if(lips){f.frame.lips_target=true;f.frame.case_index=f.frame.index=1;f.frame.room=168;f.frame.source_room=165;}const auto before=f.memory;need(!f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().failed&&f.memory==before&&!births&&!heals,"Fish and Lips cannot request empty reserves; their six-egg paths remain unchanged");}
 // Appended Monsters target keeps both original diagnostic branches unchanged.
 for(unsigned count:{0u,3u,6u}){Fixture f(12,2,count);f.frame.manual=false;f.frame.automatic=true;f.frame.monster_target=true;f.frame.case_index=f.frame.index=3;f.frame.room=170;f.frame.source_room=151;
  i::Inventory before;need(i::read(f.ram,Fixture::player,before),"Monsters starts with validated native reserve FIFO");const auto registers=f.context;
  need(f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().applied&&births==6-count&&palettes==6-count&&heals==1,"Visible automatic Monsters receives exactly one native8HP6eggs top-up");
  i::Inventory after;need(i::read(f.ram,Fixture::player,after)&&after.count==6&&n::word(f.ram,0x800F8DB4)==8,"Monsters native readback confirms8/6");
  for(unsigned k=0;k<count;++k)need(after.actors[k]==before.actors[k]&&after.eggs[k]==before.eggs[k],"Monsters preserves existing egg identities, FIFO order and palettes");
  const auto once=f.memory;need(f.controller.frame(f.ram,&f.context,f.frame)&&f.memory==once&&births==6-count&&heals==1&&!std::memcmp(&registers,&f.context,sizeof(registers)),"Monsters provisioning is one-shot with all guest registers preserved");
 }
 for(unsigned fault=0;fault<18;++fault){Fixture f;f.frame.manual=false;f.frame.automatic=true;f.frame.monster_target=true;f.frame.case_index=f.frame.index=3;f.frame.room=170;f.frame.source_room=151;bool waiting=false;
  if(fault==0)f.frame.case_index=2;
  if(fault==1)f.frame.index=2;
  if(fault==2)f.frame.room=169;
  if(fault==3)f.frame.source_room=163;
  if(fault==4){f.frame.manual=true;f.frame.automatic=false;}
  if(fault==5){byte(f.ram,0x800F7EA6,0);f.frame.proved_zero_controls=true;waiting=true;}
  if(fault==6){f.frame.vi=2407;waiting=true;}
  if(fault==7){f.frame.fighting=false;waiting=true;}
  if(fault==8)f.frame.lips_target=true;
  if(fault==9)f.frame.genie_target=true;
  if(fault==10)f.frame.target_eggs=0;
  if(fault==11)f.frame.target_eggs=7;
  if(fault==12)f.frame.visible=false;
  if(fault==13)f.frame.diagnostic=false;
  if(fault==14){word(f.ram,Fixture::player+0x180,Fixture::body);waiting=true;}
  if(fault==15){word(f.ram,Fixture::data+0x12EC,1);waiting=true;}
  if(fault==16){byte(f.ram,0x800F7EA5,1);waiting=true;}
  if(fault==17)f.frame.monster_target=false;
  const auto before=f.memory;const auto registers=f.context;const bool ok=f.controller.frame(f.ram,&f.context,f.frame);
  need(ok==waiting&&!f.controller.snapshot().attempted&&!f.controller.snapshot().applied&&f.memory==before&&!births&&!heals&&!std::memcmp(&registers,&f.context,sizeof(registers)),"Foreign, ambiguous, empty-reserve or unreleased Monsters diagnostics never provision resources");
 }
 for(unsigned fault=0;fault<6;++fault){Fixture f;const auto registers=f.context;
  if(fault==0)word(f.ram,0x800AFFE8,0);
  if(fault==1)word(f.ram,0x800AFFE8,0x7FC00000);
  if(fault==2)word(f.ram,Fixture::player+0x6C,std::bit_cast<uint32_t>(-1.f));
  if(fault==3)word(f.ram,Fixture::player+0x70,0x7F800000);
  if(fault==4)word(f.ram,Fixture::player+0x74,std::bit_cast<uint32_t>(5.f));
  if(fault==5)word(f.ram,0x800AFFE8,std::bit_cast<uint32_t>(5.f));
  const auto invalid=f.memory;need(!f.controller.frame(f.ram,&f.context,f.frame)&&f.memory==invalid&&!births&&!palettes&&!heals&&!std::memcmp(&registers,&f.context,sizeof(registers)),"Invalid native scale/factor refuses all provisioning before mutation");
 }
 {Fixture f;bad_scale=true;need(!f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().failed&&f.controller.snapshot().attempted&&births==1&&!heals,"Native egg readback rejects enlarged saved scale before healing");const auto partial=f.memory;need(!f.controller.frame(f.ram,&f.context,f.frame)&&f.memory==partial&&births==1,"Failed native scale readback cannot replay the one-shot request");}
 // Only the appended fish_target path knows these three exact tuples.
 for(uint64_t seed=0;seed<21;++seed)for(unsigned index=0;index<10;++index)need(l::fish_tuple(seed,index)==((seed==1&&index==4)||(seed==7&&index==3)||(seed==17&&index==2)),"Fish diagnostic tuple has exactly the three admitted seed/case pairs");
 auto mixed_fish=[](Fixture& f,uint64_t seed){const unsigned index=seed==7?3:2;f.frame.manual=false;f.frame.automatic=true;f.frame.fish_target=true;f.frame.seed=seed;f.frame.case_index=f.frame.index=index;f.frame.room=167+index;f.frame.source_room=97;f.frame.vi=1946;f.frame.ready={1945,index,167+index,97,0,Fixture::player,Fixture::data};f.frame.native_request_recent=true;f.frame.native_request_vi=1946;f.frame.fish_roots={0x80050000,0x80050400};actor(f.ram,f.frame.fish_roots[0],0x4511);actor(f.ram,f.frame.fish_roots[1],seed==7?0x4599:0x4072);word(f.ram,f.frame.fish_roots[0]+0x1F4,0x80051000);word(f.ram,f.frame.fish_roots[1]+0x1F4,0x80051400);};
 for(uint64_t seed:{7ull,17ull})for(unsigned count:{0u,3u,6u}){Fixture f(0,1,count);mixed_fish(f,seed);const auto registers=f.context;i::Inventory before;need(i::read(f.ram,Fixture::player,before),"Mixed diagnostic begins with a valid native-format FIFO");need(f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().applied&&f.controller.snapshot().vi==1946&&births==6-count&&heals==1,"New mixed scope applies one8/6 only after actual ready/admission witnesses, without arbitrary2408 delay");i::Inventory after;need(i::read(f.ram,Fixture::player,after)&&after.count==6,"Mixed native-shaped readback confirms six reserves");for(unsigned k=0;k<count;++k)need(before.actors[k]==after.actors[k]&&before.eggs[k]==after.eggs[k],"Mixed top-up preserves every preexisting egg identity/palette/scale");for(unsigned k=count;k<6;++k)need(after.eggs[k].saved_scale==i::Scale{std::bit_cast<uint32_t>(.8f*.7f),std::bit_cast<uint32_t>(.8f*.7f),std::bit_cast<uint32_t>(.8f*.7f)},"Mixed newly born eggs use the normal native scale recipe");const auto once=f.memory;need(f.controller.frame(f.ram,&f.context,f.frame)&&f.memory==once&&births==6-count&&heals==1&&!std::memcmp(&registers,&f.context,sizeof(registers)),"Mixed request is one-shot with complete register preservation");}
 for(unsigned fault=0;fault<31;++fault){Fixture f;mixed_fish(f,7);bool waiting=false;switch(fault){
 case 0:f.frame.seed=1;break;case 1:f.frame.case_index=4;break;case 2:f.frame.index=4;break;case 3:f.frame.room=171;break;case 4:f.frame.source_room=163;break;case 5:f.frame.ready.vi=0;waiting=true;break;case 6:f.frame.vi=f.frame.ready.vi;waiting=true;break;
 case 7:f.frame.ready.player=Fixture::body;break;case 8:f.frame.ready.private_data=Fixture::data+4;break;case 9:f.frame.ready.index=2;break;case 10:f.frame.ready.room=169;break;case 11:f.frame.ready.source=163;break;case 12:f.frame.ready.epoch=1;break;case 13:f.frame.epoch=f.frame.ready.epoch=1;break;
 case 14:f.frame.native_request_recent=false;waiting=true;break;case 15:f.frame.native_request_vi=f.frame.ready.vi;waiting=true;break;case 16:f.frame.native_request_vi=f.frame.vi+1;waiting=true;break;case 17:f.frame.vi+=3;waiting=true;break;
 case 18:byte(f.ram,0x800F7EA6,0);f.frame.proved_zero_controls=true;waiting=true;break;case 19:word(f.ram,Fixture::data+0x12EC,1);waiting=true;break;case 20:byte(f.ram,Fixture::data+0x12C9,1);waiting=true;break;case 21:byte(f.ram,Fixture::data+0x1F8,1);waiting=true;break;
 case 22:half(f.ram,f.frame.fish_roots[1]+12,0x4072);break;case 23:f.frame.fish_roots[1]=f.frame.fish_roots[0];break;case 24:byte(f.ram,f.frame.fish_roots[1]+0x17D,1);waiting=true;break;case 25:word(f.ram,f.frame.fish_roots[0]+0x1F4,0);waiting=true;break;case 26:f.frame.manual=true;f.frame.automatic=false;break;case 27:f.frame.diagnostic=false;break;case 28:f.frame.visible=false;break;case 29:f.frame.genie_target=true;break;case 30:f.frame.fish_target=false;break;}
 const auto before=f.memory,regs=std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(&f.context),reinterpret_cast<const uint8_t*>(&f.context)+sizeof(f.context));const auto ok=f.controller.frame(f.ram,&f.context,f.frame);need(ok==waiting&&!f.controller.snapshot().applied&&!f.controller.snapshot().attempted&&f.memory==before&&!births&&!palettes&&!heals&&!std::memcmp(regs.data(),&f.context,sizeof(f.context)),"Missing/foreign/stale readiness, native request, typed roots or controls never provision mixed resources");}
 {Fixture f;f.frame.manual=false;f.frame.automatic=true;f.frame.fish_target=true;f.frame.seed=1;f.frame.vi=2407;need(f.controller.frame(f.ram,&f.context,f.frame)&&!f.controller.snapshot().attempted&&!births,"Existing standalone Fish tuple still waits before2408 despite appended fields");f.frame.vi=2408;need(f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().applied,"Existing standalone Fish tuple retains its original2408 boundary without a new witness");}
 {Fixture f(0,1,6);mixed_fish(f,17);word(f.ram,0x800AFFE8,0x7FC00000);const auto before=f.memory;need(!f.controller.frame(f.ram,&f.context,f.frame)&&f.memory==before&&!births&&!heals,"Mixed added0 still rejects an invalid normal-scale recipe before any heal/logging");}
 // Distinct focused cold-Rest6 one-time request. No fight is asserted.
 auto focused=[](Fixture& f){f.frame.manual=false;f.frame.automatic=true;f.frame.cp6_focused_target=f.frame.rest_ready=true;f.frame.fighting=false;f.frame.case_index=f.frame.index=5;f.frame.room=f.frame.source_room=83;f.frame.seed=37784098137700ull;f.frame.vi=1946;f.frame.ready={1945,5,83,83,0,Fixture::player,Fixture::data};};
 for(unsigned count:{0u,3u,6u}){Fixture f(13,2,count);focused(f);const auto regs=f.context;need(f.controller.frame(f.ram,&f.context,f.frame)&&f.controller.snapshot().applied&&births==6-count&&heals==1,"FocusedRest6 requests exactly one native8/6 initialization after real room-ready, without waiting arbitrary2408");const auto once=f.memory;need(f.controller.frame(f.ram,&f.context,f.frame)&&f.memory==once&&births==6-count&&heals==1&&!std::memcmp(&regs,&f.context,sizeof(regs)),"FocusedRest6 never reapplies loadout after initial confirmation");}
 for(unsigned fault=0;fault<17;++fault){Fixture f;focused(f);bool wait=false;switch(fault){case 0:f.frame.seed=1;break;case 1:f.frame.case_index=0;break;case 2:f.frame.index=6;break;case 3:f.frame.room=172;break;case 4:f.frame.source_room=164;break;case 5:f.frame.epoch=1;break;case 6:f.frame.ready.vi=0;wait=true;break;case 7:f.frame.ready.player=Fixture::body;break;case 8:f.frame.ready.private_data=Fixture::body;break;case 9:f.frame.ready.epoch=1;break;case 10:f.frame.rest_ready=false;wait=true;break;case 11:f.frame.ready.source=164;break;case 12:f.frame.fish_target=true;break;case 13:f.frame.target_eggs=0;break;case 14:f.frame.visible=false;break;case 15:f.frame.manual=true;break;case 16:f.frame.cp6_focused_target=false;break;}const auto before=f.memory;const auto regs=f.context;const bool ok=f.controller.frame(f.ram,&f.context,f.frame);need(ok==wait&&!f.controller.snapshot().applied&&!f.controller.snapshot().attempted&&before==f.memory&&!births&&!heals&&!std::memcmp(&regs,&f.context,sizeof(regs)),"Focused foreign seed/epoch/scene/ready/target cannot allocate or heal");}
 std::cout<<"BossTestLoadout0260Checks: "<<checks<<" PASS; explicit synthetic native boundaries only; no game/GPU/receipt/save.\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
