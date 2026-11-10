#include "../boss_checkpoint_inventory0260.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <stdexcept>
using namespace workshop;
namespace cp=boss_checkpoint_inventory0260;
constexpr uint32_t player=0x80010000,data=0x80020000,stack=0x80080000,palette=0x80500000;
static unsigned checks=0,allocations=0,allocation_limit=6,music_updates=0;
static void check(bool b,const char* reason){++checks;if(!b){std::fprintf(stderr,"checkpoint inventory check %u: %s\n",checks,reason);std::exit(1);}}
static void word(uint8_t* rdram,uint32_t a,uint32_t v){MEM_W(0,cp::n::base(a))=v;}
static void half(uint8_t* rdram,uint32_t a,unsigned v){MEM_H(0,cp::n::base(a))=uint16_t(v);}
static void byte(uint8_t* rdram,uint32_t a,unsigned v){MEM_B(0,cp::n::base(a))=uint8_t(v);}
extern "C" void func_8003E5EC(uint8_t*,recomp_context*);
extern "C" void func_80043E88(uint8_t*,recomp_context*);
extern "C" void func_80044658(uint8_t*,recomp_context*);
extern "C" recomp_func_t* get_function(int32_t vram){
 if(uint32_t(vram)==0x8003E5EC)return func_8003E5EC;
 if(uint32_t(vram)==0x80043E88)return func_80043E88;
 if(uint32_t(vram)==0x80044658)return func_80044658;
 std::abort();
}
extern "C" void Fault_HungUp(uint8_t*,recomp_context*){std::abort();}
extern "C" void func_80047090(uint8_t*,recomp_context* c){c->r2=0;}
// Explicit synthetic allocation/resource boundary. The real native40860/3BB20/
// 3BA90/47F9C/3E658/3E5EC/3B8B0/422CC bodies are extracted without modification.
extern "C" void func_80046804(uint8_t* rdram,recomp_context* c){
 if(allocations>=allocation_limit){c->r2=0;return;}
 const uint32_t a=0x80200000+allocations++*0x400,resource=a+0x200,drawing=a+0x300;
 word(rdram,a,0x19971110);half(rdram,a+12,unsigned(c->r5>>16));word(rdram,a+0x168,resource);word(rdram,a+0x16C,drawing);
 word(rdram,resource+0x88,palette);word(rdram,drawing,1);c->r2=cp::n::base(a);
}
extern "C" void func_8004590C(uint8_t* rdram,recomp_context* c){word(rdram,cp::n::word(rdram,uint32_t(c->r4)+0x16C)+8,uint32_t(c->r5));}
extern "C" void func_8003B890(uint8_t*,recomp_context*){std::abort();}
extern "C" void func_80065490(uint8_t* rdram,recomp_context* c){for(unsigned i=0;i<3;i++)word(rdram,uint32_t(c->r4)+i*4,cp::n::word(rdram,uint32_t(c->r5)+i*4));}
extern "C" void func_8001D770(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_80067E98(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_80067818(uint8_t*,recomp_context*){}
extern "C" void func_800679F0(uint8_t*,recomp_context*){++music_updates;}
// Explicit retirement resource/manager boundary; actual graph traversal,
// parent detach and retirement flags run in the extracted native functions.
extern "C" void func_8003E8C0(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_8003EC84(uint8_t*,recomp_context*){std::abort();}
extern "C" void func_8003EA94(uint8_t*,recomp_context*){std::abort();}
extern "C" void func_8003EAE0(uint8_t*,recomp_context*){std::abort();}
extern "C" void func_8003EB90(uint8_t*,recomp_context*){std::abort();}
extern "C" void func_8004457C(uint8_t*,recomp_context*){std::abort();}
extern "C" void func_8005FE48(uint8_t*,recomp_context*){}
extern "C" void func_80043D34(uint8_t*,recomp_context* c){c->r2=1;}

static void setup(uint8_t* rdram,unsigned color,unsigned hp,unsigned count,unsigned head,unsigned selected){
 std::memset(rdram,0,0x800000);allocations=0;allocation_limit=6;music_updates=0;
 word(rdram,player,0x19971110);half(rdram,player+12,0x4001);word(rdram,player+0x1F4,data);
 word(rdram,0x800F8828,player);word(rdram,0x807F0008,color);word(rdram,0x800F8DB0,color<8?color:0);word(rdram,0x800F8DB4+(color<8?color:0)*4,hp);
 word(rdram,player+0x84,0x43000000);word(rdram,player+0x88,0x43800000);word(rdram,player+0x8C,0x43FA0000);
 for(unsigned i=0;i<3;i++)word(rdram,player+0x78+i*4,0x3F800000);
 word(rdram,data+0x64,0x3F800000);half(rdram,data+0x12BE,head);byte(rdram,data+0x12C8,count);half(rdram,player+0x184,count);
 for(unsigned i=0;i<count;i++){
  const uint32_t a=0x80100000+i*0x400;word(rdram,a,0x19971110);half(rdram,a+12,0x4004);word(rdram,a+0x180,player);half(rdram,a+0x17E,i);word(rdram,a+0x1EC,(selected+i)%14);
  for(unsigned axis=0;axis<3;++axis)word(rdram,a+0x78+4*axis,std::bit_cast<uint32_t>(.50f+.015625f*float(i)+.125f*float(axis)));
  word(rdram,player+0x188+i*4,a);word(rdram,data+0x11FC+((head+i)&7)*24,a);
 }
 // Persisted bonuses are deliberately outside the checkpoint payload.
 word(rdram,0x800F8958,71);word(rdram,0x800F89D0,29);word(rdram,0x807B0000,0xC0FFEE);
}
static void native_death_fixture(uint8_t* rdram){
 // Synthetic death input, not a claim that the native death path was exercised.
 const unsigned color=cp::n::word(rdram,0x800F8DB0);word(rdram,0x800F8DB4+color*4,0);
 byte(rdram,data+0x12C8,0);half(rdram,player+0x184,0);
 for(unsigned i=0;i<20;i++)word(rdram,player+0x188+i*4,0);
 for(unsigned i=0;i<6;i++)word(rdram,0x80100000+i*0x400,0xDEADF00D);
 // Consumed later than activation: these must not roll back to capture values.
 word(rdram,0x800F8958,72);word(rdram,0x807B0000,0xD00D);
}
static recomp_context context(){recomp_context c{};c.r29=cp::n::base(stack);c.r31=0x12345678;c.r4=11;c.r5=22;c.r6=33;c.r7=44;return c;}
int main(){
 std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
 for(unsigned color=0;color<14;color++)for(unsigned hp=0;hp<=8;hp++)for(unsigned count=0;count<=6;count++)for(unsigned selected=0;selected<14;selected++){
  const unsigned head=(color+hp+count+selected)%8;setup(rdram,color,hp,count,head,selected);
  cp::Payload payload;check(cp::capture(rdram,player,100,payload),"capture valid activation values");check(payload.hp==hp&&payload.count==count&&payload.pc_color==color,"capture exact values");
  cp::Restore restore;auto c=context();auto saved=c;
  const cp::RetryProof proof{player,100,200,300,1,83,3};if(hp)check(!restore.apply(rdram,&c,payload,proof),"cannot heal live Yoshi");
  native_death_fixture(rdram);check(restore.apply(rdram,&c,payload,proof),"native restoration succeeds");check(std::memcmp(&c,&saved,sizeof c)==0,"all emulated registers preserved");check(cp::matches(rdram,payload),"restored exact activation hp count ordered palettes and saved egg scales");
  check(allocations==count&&music_updates==1,"exact native factory and health effects");
  cp::Inventory inventory;check(cp::read(rdram,player,inventory),"native recreated graph valid");
  for(unsigned i=0;i<count;i++){
   const auto a=inventory.actors[i];check(a>=0x80200000&&a<0x80202000,"no activation egg pointer reused");
   const auto drawing=cp::n::word(rdram,a+0x16C);check(cp::n::word(rdram,drawing+8)==palette+payload.eggs[i].palette*512,"display palette equals explicit inventory palette");
   check(cp::n::word(rdram,a+0x20)==0x8003B2B0&&cp::n::byte(rdram,a+0x17C)==1,"native reserve callback and parent visibility");
  }
  check(cp::n::word(rdram,0x800F8958)==72&&cp::n::word(rdram,0x807B0000)==0xD00D&&cp::n::word(rdram,0x800F89D0)==29,"later consumed bonuses and fruit state survive retry");
  check(!restore.apply(rdram,&c,payload,proof),"same retry cannot replay");
 }
 setup(rdram,13,5,6,7,8);cp::Payload payload;check(cp::capture(rdram,player,100,payload),"rejection fixture capture");native_death_fixture(rdram);
 const cp::RetryProof proof{player,100,200,300,1,83,3};
 auto reject=[&](cp::Payload p,cp::RetryProof r,const char* why){auto before=memory;cp::Restore restore;auto c=context();check(!restore.apply(rdram,&c,p,r),why);check(before==memory,"rejection is mutation free");};
 auto r=proof;r.room=162;reject(payload,r,"reject retry inside arena");r=proof;r.retry_epoch=0;reject(payload,r,"reject unproved epoch");r=proof;r.death_vi=100;reject(payload,r,"reject death before wake");r=proof;r.retry_vi=200;reject(payload,r,"reject retry before death");r=proof;r.player+=0x400;reject(payload,r,"reject different Yoshi");r=proof;r.activation_vi=99;reject(payload,r,"reject different activation");
 r=proof;r.checkpoint_completed=0;reject(payload,r,"reject CP0 to rest83");r=proof;r.checkpoint_completed=4;reject(payload,r,"reject noncheckpoint milestone");
 auto p=payload;p.hp=8+1;reject(p,proof,"reject impossible health");p=payload;p.eggs[0].palette=14;reject(p,proof,"reject impossible palette");p=payload;p.eggs[0].kind=0x406C;reject(p,proof,"reject portable object recreation before mutation");p=payload;p.eggs[0].kind=0x4144;reject(p,proof,"reject second portable object recreation before mutation");
 // Ring and graph reads are strict, including wraparound, duplicate references,
 // missing parent slots, and portable-type identification without conversion.
 for(auto kind:{0x406Cu,0x4144u}){setup(rdram,1,4,1,7,0);half(rdram,0x8010000C,kind);cp::Inventory inv;check(cp::read(rdram,player,inv)&&inv.eggs[0].kind==kind,"identify native portable kind exactly");cp::Payload old;check(!cp::capture(rdram,player,100,old),"cannot activate unsupported payload");}
 setup(rdram,1,4,2,7,0);word(rdram,data+0x11FC,0x80100000);cp::Inventory inv;check(!cp::read(rdram,player,inv),"reject duplicate FIFO actor");
 setup(rdram,1,4,1,7,0);word(rdram,player+0x188,0);check(!cp::read(rdram,player,inv),"reject missing graph parent slot");
 setup(rdram,1,4,0,7,0);half(rdram,data+0x12BE,8);check(!cp::read(rdram,player,inv),"reject malformed ring head");
 setup(rdram,1,4,0,7,0);byte(rdram,data+0x12C8,7);check(!cp::read(rdram,player,inv),"reject excess reserves");
 setup(rdram,8,3,3,6,8);check(cp::capture(rdram,player,100,payload),"partial failure fixture capture");native_death_fixture(rdram);allocation_limit=1;cp::Restore partial;auto c=context();auto saved=c;check(!partial.apply(rdram,&c,payload,proof),"partial native allocation failure is fatal");check(partial.last_epoch()==1&&std::memcmp(&c,&saved,sizeof c)==0,"failed retry consumed and registers preserved");check(cp::n::word(rdram,0x800F8DB4)==0,"partial failed restore does not heal");allocation_limit=6;check(!partial.apply(rdram,&c,payload,proof),"failed attempt cannot replay");
 // Native bulk disposal before transport, with any surviving count/head.
 for(unsigned count=0;count<=6;count++)for(unsigned head=0;head<8;head++)for(unsigned source_room:{83u,165u}){
  setup(rdram,9,0,count,head,8);check(cp::capture(rdram,player,100,payload),"HP0 activation remains valid");
  auto c=context();auto saved=c;byte(rdram,player+0x18,0xA);byte(rdram,player+0x30,0x11);
  check(cp::discard_after_death(rdram,&c,payload,200,source_room),"clear surviving reserves in old arena");
  check(cp::read(rdram,player,inv)&&inv.count==0,"native clear FIFO empty");
  check(std::memcmp(&c,&saved,sizeof c)==0,"clear preserves all emulated registers");
  for(unsigned i=0;i<count;i++)check(cp::n::word(rdram,0x80100000+i*0x400+0x180)==0&&cp::n::byte(rdram,0x80100000+i*0x400+0x17D)==3,"native old reserve detached and retired");
  cp::Restore restore;const cp::RetryProof first{player,100,200,300,1,162,0};
  check(restore.apply(rdram,&c,payload,first)&&cp::matches(rdram,payload),"CP0 returns exact HP0 stock to first arena162");
 }
 setup(rdram,1,0,2,7,0);check(cp::capture(rdram,player,100,payload),"clear refusal fixture capture");
 auto before=memory;c=context();check(!cp::discard_after_death(rdram,&c,payload,200,83)&&before==memory,"refuse clear in a living rest arrival");
 check(!cp::discard_after_death(rdram,&c,payload,100,165)&&before==memory,"cannot discard before activation death");
 for(unsigned sub:{0u,1u,2u,3u,9u,14u,15u}){
  byte(rdram,player+0x18,0xA);byte(rdram,player+0x30,sub);before=memory;
  check(!cp::discard_after_death(rdram,&c,payload,200,83)&&before==memory,"refuse nonfatal mainA branch including potA/2");
 }
 for(unsigned sub:{10u,11u,12u,13u,16u,17u}){
  setup(rdram,1,0,1,7,0);check(cp::capture(rdram,player,100,payload),"fatal branch fixture capture");
  byte(rdram,player+0x18,0xA);byte(rdram,player+0x30,sub);c=context();
  check(cp::discard_after_death(rdram,&c,payload,200,83),"all six native fatal states allow before-transport83 clear");
 }

 // A strictly attested death can retain native HP. Transport must preserve
 // that observed value; restoring any activation HP0..8 is then exact.
 for(unsigned activation_hp=0;activation_hp<=8;activation_hp++)for(unsigned death_hp=0;death_hp<=8;death_hp++)for(unsigned count=0;count<=6;count++)for(unsigned milestone:{0u,3u,6u,9u}){
  setup(rdram,12,activation_hp,count,7,9);check(cp::capture(rdram,player,100,payload),"death-HP matrix activation capture");
  native_death_fixture(rdram);word(rdram,0x800F8DB4,death_hp);cp::Restore restore;auto c=context();
  const cp::RetryProof death_proof{player,100,200,300,1,milestone?83u:162u,milestone,death_hp};
  check(restore.apply(rdram,&c,payload,death_proof)&&cp::matches(rdram,payload),"attested positive or zero death HP restores different activation HP exactly");
 }
 setup(rdram,12,3,2,6,8);check(cp::capture(rdram,player,100,payload),"positive HP clear activation capture");
 setup(rdram,12,7,6,7,2);byte(rdram,player+0x18,0xA);byte(rdram,player+0x30,0x11);c=context();
 before=memory;check(!cp::discard_after_death(rdram,&c,payload,200,83,6)&&before==memory,"reject mismatched observed death HP before clear");
 check(cp::discard_after_death(rdram,&c,payload,200,83,7),"positive death HP native83 clear preserves health");
 check(cp::read(rdram,player,inv)&&inv.count==0&&cp::n::word(rdram,0x800F8DB4)==7,"native clear retains exact observed death HP");
 byte(rdram,player+0x18,1);byte(rdram,player+0x30,0);
 cp::Restore retained;const cp::RetryProof retained_proof{player,100,200,300,1,83,3,7};
 check(retained.apply(rdram,&c,payload,retained_proof)&&cp::matches(rdram,payload),"six remaining eggs replaced by activation two eggs and HP7 restored toHP3");
 setup(rdram,12,7,0,7,2);const cp::RetryProof bad_hp{player,100,200,300,2,83,3,6};before=memory;
 check(!retained.apply(rdram,&c,payload,bad_hp)&&before==memory,"reject transport changing death HP before restoration");
 const cp::RetryProof impossible_hp{player,100,200,300,2,83,3,9};
 check(!retained.apply(rdram,&c,payload,impossible_hp)&&before==memory,"reject invalid observed death HP9");
 std::printf("BossCheckpointInventory0260Checks: %u checks PASS; real native factories/graph/FIFO/palette/health; synthetic allocator and death input\n",checks);
 return 0;
}
