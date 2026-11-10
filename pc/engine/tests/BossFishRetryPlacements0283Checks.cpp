// Reuse the unchanged legacy262-control oracle, without running its main here.
#define main legacy_retry_main
#include "BossRetryPlacements0281Checks.cpp"
#undef main
namespace actual {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#include BOSS_FISH_RETRY_NATIVE_EXTRA
#pragma clang diagnostic pop
}
struct FishFixture:Fixture {
 explicit FishFixture(int variant=-1,bool genie=true){
  plan=b::make_plan(37784098137700ULL);index=6;
  need(plan.fights[index].family==b::Family::Fish&&plan.fights[index].variant==2&&plan.fights[index].terrain==1&&plan.fights[index].members[0].hits==3&&plan.fights[index].members[1].kind==0x4072,"Exact logged seed has three-hit Fish7 plus4072 on terrain1");
  if(variant>=0){b::Random random{41};plan.fights[index]=b::encounter(index+1,b::Family::Fish,unsigned(variant),random,plan.config);if(variant==2){plan.fights[index].members[1].kind=genie?0x4072:0x4599;plan.fights[index].genie_companion=genie;}}
  rooms::configure(plan);arena=rooms::arena(rooms::source_for_encounter(index));encounter=plan.fights[index];
  std::fill(bytes.begin(),bytes.end(),0);guard.reset();c={};setters=getters=0;
  word(0x800F8450,registry);half(0x800F8448,11);word(registry+24,cache);byte(cache,1);byte(cache+1,arena.source);word(cache+0x60,cache+0x128);word(cache+0x128,cache+0x12C);half(cache+0x12C,unsigned(arena.objects.size()/12));word(cache+0x130,table());
  for(size_t p=0;p<arena.objects.size();p+=12)for(unsigned j=0;j<3;++j)word(table()+unsigned(p/12)*16+j*4,workshop::u32(arena.objects,p+j*4));
  for(unsigned i=0;i<encounter.count;++i){word(actor(i),0x19971110);half(actor(i)+12,encounter.members[i].kind);half(actor(i)+14,i);byte(actor(i)+0x17D,2);word(actor(i)+0x168,resource+i*0x100);}
  word(0x800F8DB4,5);word(0x800F89D0,12345);word(0x807F1000,0x15);c.r29=n::base(stack);c.r31=0x8005A050;c.r16=0x88776655;c.f2.u32l=0xAABBCCDD;
  need(arena.source==173&&arena.logical_source==97&&arena.native_init==0x01000000&&g::owned(arena,encounter,index),"Fish uses the same native phase-clone loader as Monsters, with exact2/3 original packed identities");
 }
 void detach(unsigned i){c.r4=n::base(actor(i));c.r5=c.r6=0;actual::func_80048BE8(r,&c);need(!n::word(r,actor(i)+0x1D8)&&!n::word(r,actor(i)+0x1DC)&&n::word(r,record(i))==0xFFFFFFFF,"Actual48BE8 leaves defeated companion placement permanently consumed");}
 void reset_survivor(unsigned i){c.r4=n::base(actor(i));actual::func_80048CA4(r,&c);need(n::word(r,record(i))==workshop::u32(arena.objects,size_t(arena.boss_indices[i])*12),"Actual48CA4 restores surviving4511 placement through its original link");}
 void prepare(unsigned epoch=1,unsigned checkpoint=6,bool reset_first=false){for(unsigned i=0;i<encounter.count;++i){capture(i);consume(i);}if(reset_first)reset_survivor(0);for(unsigned i=0;i<encounter.count;++i)retire(i);need(guard.arm(epoch,checkpoint,index),"Only genuine replay epoch arms captured Fish placements");fence_arguments();}
 void expect_restored(unsigned epoch,unsigned expected_count){const auto before=bytes;const auto regs=c;g::Receipt p;need(restore(fence(epoch),p),"Fish replay dispatches actual native getter and setter before the cache-hit loader");need(p.count==expected_count&&p.epoch==epoch&&p.room==173&&p.index==6&&p.table==table(),"Only consumed original Fish/companion packed words restored");auto expected=before;for(unsigned i=0;i<encounter.count;++i){const uint32_t v=workshop::u32(arena.objects,size_t(arena.boss_indices[i])*12);std::memcpy(expected.data()+(record(i)&0x7FFFFF),&v,4);}same_except_stack(expected,bytes);need(!std::memcmp(&regs,&c,sizeof(c)),"Fish replay preserves all caller integer/FP registers");const auto count=setters;const auto after=bytes;need(restore(fence(epoch),p)&&setters==count&&!p.epoch&&bytes==after,"Duplicate native entry cannot rearm a row twice or spawn an extra actor");}
};
int main(){try{
 {FishFixture f;need(f.guard.arm(1,3,3),"The earlier CP3 replay advances epoch1 before Fish7 is first visited");for(unsigned i=0;i<2;++i){f.capture(i,1);f.consume(i);}f.reset_survivor(0);f.detach(1);f.retire();f.retire(1);need(f.guard.arm(2,6,6),"Exact logged CP6 retry advances epoch2");f.fence_arguments();auto call=f.c;actual::func_8006DA0C(f.r,&call);need(n::word(f.r,Fixture::info+0x20)==Fixture::cache&&n::word(f.r,f.record(1))==0xFFFFFFFF&&n::word(f.r,f.record())!=0xFFFFFFFF,"Before fix, actual cached6DA0C keeps only companion4072 missing while surviving4511 was reset");f.expect_restored(2,1);need(f.setters==1,"Logged failure re-arms exactly one4072 word, with no4511 write");
  // The genuine native consumer sees both pristine rows after restoration.
  for(unsigned i=0;i<2;++i){f.byte(f.actor(i)+0x17D,2);f.capture(i,2);f.consume(i);f.retire(i);}need(f.guard.arm(3,6,6),"A later real retry has a fresh epoch");f.fence_arguments();f.expect_restored(3,2);
 }
 for(unsigned variant=0;variant<3;++variant)for(bool genie:{false,true})for(bool reset_first:{false,true}){FishFixture f(int(variant),genie);f.prepare(1,6,reset_first);f.expect_restored(1,f.encounter.count-unsigned(reset_first));}
 {FishFixture f;for(unsigned i=0;i<f.encounter.count;++i){f.capture(i);f.consume(i);f.reset_survivor(i);f.retire(i);}need(f.guard.arm(1,6,6),"A retry may find all native rows already restored");f.fence_arguments();f.expect_restored(1,0);need(!f.setters,"Already restored rows dispatch no setter and cannot double-spawn");}
 {FishFixture f;f.prepare(1,0);f.expect_restored(1,2);}
 {FishFixture f;f.prepare();need(f.guard.arm(2,9,9),"A banked checkpoint9 excludes Fish7");g::Receipt p;const auto before=f.bytes;need(f.restore(f.fence(2),p)&&!f.setters&&f.bytes==before,"Banked Fish records are never rewritten");}
 for(unsigned fault=0;fault<15;++fault){FishFixture f;f.prepare();auto fence=f.fence();
  if(fault==0)f.c.r31+=4;if(fault==1)fence.epoch=2;if(fault==2)fence.phase=b::Phase::Fighting;if(fault==3)f.word(f.record(1),0x40720017);if(fault==4)f.word(f.record(1)+4,0);if(fault==5)f.byte(f.actor(1)+0x17D,2);
  if(fault==6){const uint32_t a=0x80210000;f.word(a,0x19971110);f.half(a+12,0x4002);f.byte(a+0x17D,1);f.word(a+0x1D8,f.record(1));}
  if(fault==7)f.word(Fixture::cache+0x130,f.table()+16);if(fault==8)f.arena.native_init=0x02000000;if(fault==9)f.arena.logical_source=163;if(fault==10)f.encounter.members[1].kind=0x40A3;if(fault==11)f.encounter.count=3;
  const std::array<uint16_t,2> duplicate{f.arena.boss_indices[0],f.arena.boss_indices[0]};if(fault==12)f.arena.boss_indices=duplicate;if(fault==13)f.word(f.table()+12,0xDEADBEEF),f.word(f.record(1)+8,0);if(fault==14)f.word(Fixture::registry+28,Fixture::cache);
  const auto before=f.bytes;const auto regs=f.c;g::Receipt p;need(!f.restore(fence,p)&&!f.setters&&!p.epoch,"Corrupt source/second member/fence/cache or any retained live link rejects before setter1");same_except_stack(before,f.bytes);need(!std::memcmp(&regs,&f.c,sizeof(f.c)),"Rejected Fish batch preserves context");
 }
 for(auto family:{b::Family::Candy,b::Family::Invisible,b::Family::Genie,b::Family::Lips,b::Family::RedCandy}){FishFixture f;f.encounter.family=family;const auto before=f.bytes;f.arguments();need(!g::owned(f.arena,f.encounter,f.index)&&!f.guard.capture(f.r,&f.c,f.arena,f.encounter,f.index,0)&&f.bytes==before,"Other ordinary families remain outside this bounded guard");}
 std::printf("BossFishRetryPlacements0283Checks: %u PASS (exact seed Fish7/CP3-to-CP6, actual prepared consume/cache/getter/setter/reset/unlink; HOST retirement/epoch fixture; no GPU claim)\n",checks);return 0;
 }catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}}
