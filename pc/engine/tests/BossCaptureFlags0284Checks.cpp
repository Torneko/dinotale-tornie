#define main legacy_capture_main
#include "BossRetryPlacements0281Checks.cpp"
#undef main
#include <fstream>
namespace flags_actual {
void apply(uint8_t* rdram,recomp_context* ctx){
#include BOSS_CAPTURE_FLAGS_NATIVE
L_80045D78: return;
}
}
static uint32_t native_flags(Fixture& f,uint32_t actor,uint32_t resource,uint32_t packed,unsigned flags){
 f.word(actor+12,packed);f.word(actor+0x168,resource);f.byte(resource+2,flags);recomp_context c{};c.r3=n::base(resource);c.r6=n::base(actor);flags_actual::apply(f.r,&c);return n::word(f.r,actor+12);
}
struct FlagFixture:Fixture {
 explicit FlagFixture(unsigned at=1):Fixture(){
  plan=b::make_plan(37784098137700ull);index=at;rooms::configure(plan);arena=rooms::arena(rooms::source_for_encounter(index));encounter=plan.fights[index];std::fill(bytes.begin(),bytes.end(),0);guard.reset();c={};
  word(0x800F8450,registry);half(0x800F8448,11);word(registry+24,cache);byte(cache,1);byte(cache+1,arena.source);word(cache+0x60,cache+0x128);word(cache+0x128,cache+0x12C);half(cache+0x12C,unsigned(arena.objects.size()/12));word(cache+0x130,table());
  for(size_t p=0;p<arena.objects.size();p+=12)for(unsigned j=0;j<3;++j)word(table()+uint32_t(p/12)*16+j*4,workshop::u32(arena.objects,p+j*4));
  for(unsigned i=0;i<encounter.count;++i){word(actor(i),0x19971110);byte(actor(i)+0x17D,1);native_flags(*this,actor(i),resource+i*0x100,workshop::u32(arena.objects,size_t(arena.boss_indices[i])*12),9);}
  c.r29=n::base(stack);c.r31=n::base(0x8005A050);
 }
};
static void expect_capture(FlagFixture& f,unsigned member){
 f.arguments(member);const auto before=f.bytes;const auto regs=f.c;const bool captured=f.guard.capture(f.r,&f.c,f.arena,f.encounter,f.index,0);
#ifdef EXPECT_LEGACY_REJECT
 need(!captured&&std::string_view(f.guard.failure())=="capture_placement","Old full-word comparison rejects actual native flags");
#else
 need(captured,"Exact native flag transformation preserves pristine boss capture");
#endif
 need(before==f.bytes&&!std::memcmp(&regs,&f.c,sizeof(regs)),"Capture changes neither8MiB nor context");
 const auto packed=workshop::u32(f.arena.objects,size_t(f.arena.boss_indices[member])*12);actual::func_80048E9C(f.r,&f.c);need(n::word(f.r,f.record(member))==0xFFFFFFFF&&n::word(f.r,f.actor(member)+0x1D8)==f.record(member)&&n::word(f.r,f.actor(member)+0x1DC)==packed,"Actual48E9C attaches unmodified pristine record while actor flags differ");
}
int main(int argc,char** argv){try{
 need(argc==2,"Provide preserved isolated GPU final RAM dump");
 {FlagFixture f;need(f.index==1&&f.arena.source==168&&f.arena.logical_source==151&&f.encounter.family==b::Family::Monsters&&f.encounter.variant==0&&f.encounter.terrain==1,"Actual planner reproduces failed Monsters2 tuple");need(n::word(f.r,f.actor())+0u==0x19971110&&n::word(f.r,f.actor()+12)==0x41C62400&&n::word(f.r,f.actor(1)+12)==0x41C62401,"Unchanged native45D54..6C instructions reproduce exact two failing GPU words");for(unsigned i=0;i<2;++i)expect_capture(f,i);}
 {FlagFixture f;std::ifstream input(argv[1],std::ios::binary);input.read(reinterpret_cast<char*>(f.bytes.data()),std::streamsize(f.bytes.size()));need(input.good()&&input.gcount()==std::streamsize(f.bytes.size()),"Read exact8MiB preserved GPU dump");f.guard.reset();constexpr uint32_t actors[]{0x800C9CC0,0x800C9F3C};
  for(unsigned i=0;i<2;++i){const auto actor=actors[i],record=n::word(f.r,actor+0x1D8),packed=n::word(f.r,actor+0x1DC);need(packed==0x41C60000+i&&n::word(f.r,actor+12)==(packed|0x2400),"Actual GPU actor and pristine backlink differ only by real flags2400");f.c.r4=n::base(record);f.c.r5=n::base(packed);actual::func_8006D34C(f.r,&f.c);f.c.r4=n::base(actor);f.c.r5=n::base(record);f.c.r31=n::base(0x8005A050);const auto before=f.bytes;const auto regs=f.c;const bool captured=f.guard.capture(f.r,&f.c,f.arena,f.encounter,f.index,0);
#ifdef EXPECT_LEGACY_REJECT
   need(!captured&&std::string_view(f.guard.failure())=="capture_placement","Actual preserved GPU RAM reproduces old guard rejection at48E9C boundary");
#else
   need(captured,"Corrected guard admits exact preserved GPU RAM at pristine native48E9C boundary");
#endif
   need(before==f.bytes&&!std::memcmp(&regs,&f.c,sizeof(regs)),"Actual GPU-memory capture remains read-only");actual::func_80048E9C(f.r,&f.c);need(n::word(f.r,record)==0xFFFFFFFF&&n::word(f.r,actor+0x1D8)==record&&n::word(f.r,actor+0x1DC)==packed,"Actual48E9C preserves exact GPU record provenance after capture");
  }
 }
#ifndef EXPECT_LEGACY_REJECT
 for(unsigned index:{1u,6u})for(unsigned flags=0;flags<256;++flags){FlagFixture f(index);for(unsigned i=0;i<f.encounter.count;++i){const auto packed=workshop::u32(f.arena.objects,size_t(f.arena.boss_indices[i])*12);const auto observed=native_flags(f,f.actor(i),Fixture::resource+i*0x100,packed,flags);need(observed==(packed|(uint32_t(flags&63)<<10)),"Actual native45D54..6C projection matches exact retained role and six resource flag bits");expect_capture(f,i);}}
 for(unsigned fault=0;fault<8;++fault){FlagFixture f;f.arguments();if(fault==0)f.word(f.actor()+12,n::word(f.r,f.actor()+12)^1);if(fault==1)f.word(f.actor()+12,n::word(f.r,f.actor()+12)^0x100);if(fault==2)f.word(f.actor()+12,n::word(f.r,f.actor()+12)^0x400);if(fault==3)f.half(f.actor()+12,0x4511);if(fault==4)f.c.r5=n::base(f.record(1));if(fault==5)f.word(f.record(),0x41C62400);if(fault==6)f.c.r31+=4;if(fault==7)f.word(f.record()+4,0);const auto before=f.bytes;need(!f.guard.capture(f.r,&f.c,f.arena,f.encounter,f.index,0)&&before==f.bytes,"Wrong role/type/resource flags/boss record/packed ROM/caller/XY remains rejected");}
#endif
 std::printf("BossCaptureFlags0284Checks: %u PASS (unchanged native45D54..6C flags +48E9C/6D34C, exact preserved GPU dump; no GPU launch)\n",checks);return 0;
 }catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}}
