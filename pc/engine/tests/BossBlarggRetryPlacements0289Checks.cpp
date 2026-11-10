// Reuse the actual native cache/getter/setter/consumer oracle.
#define main legacy_retry_fixture_main
#include "BossRetryPlacements0281Checks.cpp"
#undef main
namespace actual {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#include BOSS_FISH_RETRY_NATIVE_EXTRA
#pragma clang diagnostic pop
}
struct BlarggFixture:Fixture {
 std::vector<uint8_t> own_objects;
 BlarggFixture(){b::Config c;c.blargg=true;b::Random random{42};encounter=b::encounter(index+1,b::Family::Blargg,0,random,c);arena.logical_source=19;arena.family="Double Blargg";arena.boss_kind=0x41EC;own_objects.assign(arena.objects.begin(),arena.objects.end());for(unsigned m=0;m<2;++m){const auto at=size_t(arena.boss_indices[m])*12;const uint32_t packed=0x41EC0000u|m;for(unsigned j=0;j<4;++j)own_objects[at+j]=uint8_t(packed>>(24-j*8));half(actor(m)+12,0x41EC);}arena.objects=own_objects;for(size_t p=0;p<arena.objects.size();p+=12)for(unsigned j=0;j<3;++j)word(table()+unsigned(p/12)*16+j*4,workshop::u32(arena.objects,p+j*4));need(g::owned(arena,encounter,index),"Explicit declared logical19 arena owns two exact three-hit41EC records");}
 void prepare(unsigned checkpoint=6,bool reset_first=false){for(unsigned m=0;m<2;++m){capture(m);consume(m);}if(reset_first){c.r4=n::base(actor());actual::func_80048CA4(r,&c);}retire();retire(1);need(guard.arm(1,checkpoint,index),"Genuine captured retry epoch arms this source19 pair");fence_arguments();}
 void restored(unsigned expected){const auto before=bytes;const auto regs=c;g::Receipt proof;need(restore(fence(),proof)&&proof.count==expected&&proof.room==173&&proof.index==6&&proof.epoch==1,"Only original consumed Blargg packed words are restored");auto after=before;for(unsigned m=0;m<2;++m){const uint32_t packed=0x41EC0000u|m;std::memcpy(after.data()+(record(m)&0x7FFFFF),&packed,4);}same_except_stack(after,bytes);need(!std::memcmp(&regs,&c,sizeof(c)),"Blargg replay preserves caller integer and FP registers");const auto unchanged=bytes;const auto calls=setters;need(restore(fence(),proof)&&!proof.epoch&&setters==calls&&bytes==unchanged,"Duplicate scene entry cannot restore a Blargg twice");}
};
int main(){try{
 for(unsigned checkpoint:{0u,3u,6u}){BlarggFixture f;f.prepare(checkpoint);auto call=f.c;actual::func_8006DA0C(f.r,&call);need(n::word(f.r,f.record())==0xFFFFFFFF&&n::word(f.r,f.record(1))==0xFFFFFFFF,"Actual cached loader leaves both consumed Blarggs absent before replay guard");f.restored(2);}
 {BlarggFixture f;f.prepare(6,true);f.restored(1);}
 {BlarggFixture f;f.capture();f.consume();f.retire();f.retire(1);need(f.guard.arm(1,6,6),"One streamed Blargg can arm both captured pristine identities");f.fence_arguments();f.restored(1);}
 {BlarggFixture f;f.prepare();need(f.guard.arm(2,9,9),"Checkpoint9 banks the earlier pair");const auto before=f.bytes;g::Receipt proof;need(f.restore(f.fence(2),proof)&&!proof.epoch&&!f.setters&&f.bytes==before,"Already banked Blargg placements are not replayed");}
 for(unsigned fault=0;fault<19;++fault){BlarggFixture f;f.prepare();auto fence=f.fence();
  if(fault==0)f.arena.logical_source=97;if(fault==1)f.encounter.count=1;if(fault==2)f.encounter.members[0].kind=0x4594;if(fault==3)f.encounter.members[0].hits=4;if(fault==4)f.encounter.variant=1;if(fault==5)f.encounter.members[0].red=true;if(fault==6)f.encounter.members[1].scale=.9f;if(fault==7)f.encounter.no_flowers=false;if(fault==8)f.encounter.no_egg_blocks=false;if(fault==9)f.encounter.genie_companion=true;if(fault==10)f.encounter.purple_mask=1;
  if(fault==11)f.c.r31+=4;if(fault==12)fence.epoch=2;if(fault==13)fence.phase=b::Phase::Fighting;if(fault==14)f.word(f.record(1),0x41EC0017);if(fault==15)f.word(f.record(1)+4,0);if(fault==16)f.byte(f.actor(1)+0x17D,2);if(fault==17)f.word(Fixture::cache+0x130,f.table()+16);if(fault==18){const uint32_t a=0x80210000;f.word(a,0x19971110);f.half(a+12,0x4002);f.byte(a+0x17D,1);f.word(a+0x1D8,f.record(1));}
  const auto before=f.bytes;const auto regs=f.c;g::Receipt proof;need(!f.restore(fence,proof)&&!f.setters&&!proof.epoch,"Malformed Blargg shape/caller/cache/remaining live link rejects before any setter");same_except_stack(before,f.bytes);need(!std::memcmp(&regs,&f.c,sizeof(regs)),"Rejected replay preserves caller registers");
 }
 {BlarggFixture f;f.arguments();f.half(f.actor()+12,0x4594);const auto before=f.bytes;need(!f.guard.capture(f.r,&f.c,f.arena,f.encounter,f.index,0)&&f.bytes==before,"Native alias4594 cannot capture an exact41EC boss row");}
 std::printf("BossBlarggRetryPlacements0289Checks: %u PASS; actual native consume/cache/getter/setter/reset, HOST arena/teardown/epoch fixture; no game\n",checks);return 0;
}catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}}
