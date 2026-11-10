// CPU-only oracle: exact generated native food/bridge functions and existing
// hooks are extracted into COOP_BIRD_NATIVE_SLICE (see provenance artifact).
// It covers health accounting after ingestion, not tongue/physics or netplay.
#include "../coop_runtime.hpp"
#include "../hybrid_rewards.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#ifndef COOP_BIRD_NATIVE_SLICE
#error Generate the exact native food slice before compiling this isolated test.
#endif
extern "C" int workshop_hybrid_active();
extern "C" unsigned workshop_hybrid_lucky_type();
#include COOP_BIRD_NATIVE_SLICE
namespace coop=workshop::coop;
namespace rewards=workshop::rewards;
static unsigned checks=0,cases=0,score_calls=0,invincibility_calls=0;
extern "C" int workshop_hybrid_active(){return 0;}
extern "C" unsigned workshop_hybrid_lucky_type(){return 6;}
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
// External presentation/score calls are stubs. Health selection, clamping,
// native scalar writes and production owner/reward hooks are the real code.
extern "C" void switch_error(const char*,uint32_t,uint32_t){std::abort();}
extern "C" void func_8001D770(uint8_t*,recomp_context* c){++invincibility_calls;c->r2=0;}
extern "C" void func_80067BF8(uint8_t*,recomp_context* c){++invincibility_calls;c->r2=0;}
extern "C" void func_80067818(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_80067D28(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_8002B200(uint8_t*,recomp_context*){++score_calls;}
extern "C" void func_8002B3A0(uint8_t*,recomp_context*){}
struct Memory final:coop::ScalarMemory{
 uint8_t* ram;explicit Memory(uint8_t* p):ram(p){}
 uint32_t read(uint32_t a)override{need(a>=0x80000000&&a<=0x807FFFFC&&!(a&3),"Scalar read in guest RAM");auto* rdram=ram;return uint32_t(MEM_W(0,int32_t(a)));}
 void write(uint32_t a,uint32_t v)override{need(a>=0x80000000&&a<=0x807FFFFC&&!(a&3),"Scalar write in guest RAM");auto* rdram=ram;MEM_W(0,int32_t(a))=int32_t(v);}
};
static uint32_t be32(const std::vector<uint8_t>& b,size_t p){need(p+4<=b.size(),"Native ROM read bounded");return(uint32_t(b[p])<<24)|(uint32_t(b[p+1])<<16)|(uint32_t(b[p+2])<<8)|b[p+3];}
static uint16_t be16(const std::vector<uint8_t>& b,size_t p){need(p+2<=b.size(),"Native ROM half read bounded");return uint16_t((unsigned(b[p])<<8)|b[p+1]);}
static size_t bank(const std::vector<uint8_t>& rom,uint32_t p){return be32(rom,0xA79D4+((p>>24)&15)*4)+(p&0xFFFFFF);}
int main(int argc,char**argv){try{
 need(argc==2,"Original USA ROM path required");std::ifstream input(argv[1],std::ios::binary);need(bool(input),"Original ROM open");std::vector<uint8_t> rom((std::istreambuf_iterator<char>(input)),{});need(rom.size()==0x1000000,"Original ROM size");
 std::array<uint32_t,2> properties{};std::array<uint16_t,2> flags{};std::array<size_t,2> records{};
 for(size_t q=0xA1C90;q<0xA1C90+1582*12;q+=12){const auto kind=be16(rom,q);if(kind!=0x4526&&kind!=0x46DA)continue;const unsigned index=kind==0x46DA;const auto resource=bank(rom,be32(rom,q+4)),descriptor=bank(rom,be32(rom,resource+0x40));need(be32(rom,descriptor+4)==26,"Native bird collision/food record length");records[index]=bank(rom,be32(rom,descriptor+8));properties[index]=be32(rom,records[index]+20);flags[index]=be16(rom,records[index]+24);}
 need(records[0]==0x5332BC&&records[1]==0x5332E4,"Distinct native healthy and damaged bird records");need(properties[0]==0x11111111&&properties[1]==0x55555555,"Native healthy class1 versus damaged class5 for every color");need(flags[0]==0xC1&&flags[1]==0xC1,"Both native forms keep existing edible/egg flags");
 std::vector<uint8_t> bytes(0x800000);auto* rdram=bytes.data();Memory memory(rdram);
 auto write=[&](uint32_t a,uint32_t v){MEM_W(0,int32_t(a))=int32_t(v);};auto word=[&](uint32_t a){return uint32_t(MEM_W(0,int32_t(a)));};
 // Real main-ROM health/score/mask/shift tables, not copied expectations.
 for(unsigned i=0;i<8;i++)for(uint32_t table:{0x8009DF24u,0x8009DF44u,0x8009DF64u,0x8009DF84u})write(table+i*4,be32(rom,table-0x80000400+0x1000+i*4));
 need(int32_t(word(0x8009DF84+4))==0&&word(0x8009DF84+5*4)==2&&word(0x8009DF24+5*4)==0,"Native damaged bird already heals2 with zero points");
 const std::array<uint32_t,2> players{0x80100000,0x80101000},privates{0x80110000,0x80112000},birds{0x80120000,0x80120300},metadata{0x80121000,0x80121100},definitions{0x80122000,0x80122100};
 for(unsigned index=0;index<2;index++){write(birds[index],0x19971110);MEM_H(12,int32_t(birds[index]))=uint16_t(index?0x46DA:0x4526);MEM_B(0x17D,int32_t(birds[index]))=2;MEM_B(0x18,int32_t(birds[index]))=1;MEM_B(0x30,int32_t(birds[index]))=uint8_t(index?7:5);write(birds[index]+0x174,metadata[index]);write(metadata[index],definitions[index]);write(definitions[index]+0x18,properties[index]);MEM_H(0x1C,int32_t(definitions[index]))=flags[index];}
 for(unsigned c0=0;c0<14;c0++)for(unsigned c1=0;c1<14;c1++)for(unsigned eater=0;eater<2;eater++)for(unsigned old=0;old<=8;old++)for(unsigned mate:{0u,1u,7u,8u})for(unsigned damaged=0;damaged<2;damaged++){
  ++cases;const std::array<unsigned,2> colors{c0,c1};for(unsigned k=0;k<8;k++)write(coop::native_health+k*4,8);write(coop::native_player,players[0]);write(coop::native_color,coop::native_alias(c0));write(coop::custom_color,c0);write(0x800F8940,0);write(0x807F0020,0);
  coop::OwnerContext owner(memory);owner.bind(0,{{players[0],1},c0,8});owner.bind(1,{{players[1],2},c1,8});
  for(unsigned p=0;p<2;p++){coop::ScopedOwner scope(owner,p);write(coop::native_health+coop::native_alias(colors[p])*4,p==eater?old:mate);}
  const unsigned previous0=owner.player(0).health,previous1=owner.player(1).health;const auto before_scores=score_calls;
  std::array<uint32_t,8> globals{};for(unsigned k=0;k<8;k++)globals[k]=word(coop::native_health+k*4);
  write(privates[1-eater]+0x178,0xCAFE1234);write(privates[eater]+0x164,birds[damaged]);
  {coop::ScopedOwner scope(owner,eater);need(word(coop::native_player)==players[eater]&&word(coop::custom_color)==colors[eater],"Ingestion executes in actual eater owner context");need(word(coop::native_health+coop::native_alias(colors[eater])*4)==old,"Zero HP is retained on context entry");
   recomp_context context{};context.r4=int32_t(players[eater]);context.r5=int32_t(privates[eater]);context.r29=int32_t(0x807B0000);context.r31=int32_t(0x80000300);context.r16=0x12345678;
   func_8002B470(rdram,&context);
   need(uint32_t(context.r29)==0x807B0000&&uint32_t(context.r16)==0x12345678,"Exact native ingestion/bridge restores its parent stack andS0");
   const unsigned expected=damaged?std::min(8u,old+2):old;need(word(coop::native_health+coop::native_alias(colors[eater])*4)==expected,"Native ingestion heals damaged bird2 even from0 and clamps8");need(word(privates[eater]+0x178)==(damaged?2u:0u),"Production reward hooks retain original native bird healing value");need(word(privates[1-eater]+0x178)==0xCAFE1234,"Partner private healing field unchanged");
  }
  need(owner.depth()==0,"Ingestion leaves no owner scope open");need(owner.player(eater).health==(damaged?std::min(8u,old+2):old),"Actual eater health captured after native continuation");need(owner.player(1-eater).health==(eater?previous0:previous1),"Partner health stays independent even with equal or aliased colors");need(word(coop::native_player)==players[0]&&word(coop::native_color)==coop::native_alias(c0)&&word(coop::custom_color)==c0,"Primary native singleton restored");
  for(unsigned k=0;k<8;k++){const unsigned expected=eater==0&&k==coop::native_alias(c0)?owner.player(0).health:globals[k];need(word(coop::native_health+k*4)==expected,"Only primary active view changes externally; other health-table slots restored");}
  need(score_calls==before_scores+1,"Native post-health score continuation reached exactly once");
 }
 need(invincibility_calls==0,"Damaged birds never grant automatic invincibility");
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"ingestions\":"<<cases<<",\"colorsPerPlayer\":14,\"healthRange\":[0,8],\"healthyHealing\":0,\"damagedHealing\":2,\"nativeFoodFunctionsExecuted\":true,\"engineSourcesChanged\":false,\"gameLaunched\":false,\"nativeCoopPhysicsVerified\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
