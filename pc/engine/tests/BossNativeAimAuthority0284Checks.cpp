#include "../boss_native_egg_aim0284.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>
#ifndef NATIVE_AIM_CANCEL0284
#error Supply unchanged native40C48 and3BF08 bodies.
#endif
static unsigned checks=0,native_calls=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
namespace actual_cancel {
void func_8003BF08(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected nonempty40C48 native restore boundary");}
void func_8003FBE4(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected nonempty40C48 native egg-return boundary");}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#include NATIVE_AIM_CANCEL0284
#pragma clang diagnostic pop
}
namespace actual_pending {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#include NATIVE_AIM_PENDING0284
#pragma clang diagnostic pop
}
namespace n=workshop::boss_native;namespace aim=workshop::boss_native_egg_aim0284;
struct Memory {
 std::vector<uint8_t> bytes;uint8_t* rdram;uint32_t player,priv;
 explicit Memory(const std::vector<uint8_t>& source):bytes(source),rdram(bytes.data()),player(n::word(rdram,0x800F8828)),priv(n::word(rdram,player+0x1F4)){}
 void word(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}void half(uint32_t a,unsigned v){MEM_H(0,n::base(a))=uint16_t(v);}void byte(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}
 aim::Observation read(){const auto before=bytes;const auto out=aim::observe(rdram,player);need(before==bytes,"Observer preserves every byte of actual memory or explicit HOST copy");return out;}
};
int main(int argc,char** argv){try{
 need(argc==3,"Supply immutable nonfenced live Genie sample and authentic pending held4005 capture");std::ifstream stream(argv[1],std::ios::binary);const std::vector<uint8_t> capture((std::istreambuf_iterator<char>(stream)),{});need(capture.size()==0x800000,"Actual capture is exactly8MiB");constexpr uint32_t ghost=0x800C6AB4,stack=0x807A0000;
 {Memory f(capture);auto* rdram=f.rdram;need(n::actor(f.rdram,f.player,0x4001)&&n::actor(f.rdram,ghost,0x4005),"Actual P1 and returned4005 source identities");need(n::word(f.rdram,f.priv+0x1F0)==0&&n::word(f.rdram,f.priv+0x1F4)==0&&!n::byte(f.rdram,f.priv+0x1F8),"Actual private held/reticle/flag are zero");need(n::byte(f.rdram,ghost+0x18)==0x69&&n::byte(f.rdram,ghost+0x17D)==0&&n::word(f.rdram,ghost+0x180)==f.player&&MEM_HU(0x17E,n::base(ghost))==8&&MEM_HU(0x184,n::base(f.player))==8,"Actual returned egg occupies sparse slot8 while population is8");const auto out=f.read();need(out.player_observed&&!out.held&&!out.reticle&&!out.held_flag,"Actual sparse returned4005 is not native held aim");}
 {std::ifstream positive(argv[2],std::ios::binary);const std::vector<uint8_t> actualPositive((std::istreambuf_iterator<char>(positive)),{});need(actualPositive.size()==0x800000,"Authentic positive native pending capture is exactly8MiB");Memory f(actualPositive);const auto held=n::word(f.rdram,f.priv+0x1F0),reticle=n::word(f.rdram,f.priv+0x1F4);need(n::byte(f.rdram,f.player+0x18)==3&&n::actor(f.rdram,held,0x4005)&&n::actor(f.rdram,reticle,0x4003)&&!n::byte(f.rdram,f.priv+0x1F8),"Authentic native pending3 has held4005 and4003 before held flag1");const auto out=f.read();need(out.player_observed&&out.held==held&&out.reticle==reticle&&!out.held_flag,"Actual native pending held4005/reticle pair is observed read-only without HOST reconstruction");need(f.bytes==actualPositive,"Authentic positive native sample remains byte-exact");}
 for(unsigned queue=0;queue<=2;++queue){Memory f(capture);f.word(f.priv+0x1F0,ghost);f.byte(ghost+0x17D,queue);f.byte(f.player+0x18,3);f.word(f.player+0x20,0x8003BF88);f.byte(f.priv+0x1F8,0);const auto pending=f.read();need(pending.player_observed&&pending.held==ghost&&!pending.held_flag,"Explicit HOST pending held graph accepted before nativeBF08 even phase69 and sparse slot>=population");recomp_context c{};c.r4=n::base(f.player);c.r5=n::base(f.priv);actual_pending::func_8003BF08(f.rdram,&c);++native_calls;need(n::byte(f.rdram,f.player+0x18)==1&&n::word(f.rdram,f.player+0x20)==0x80043450&&n::byte(f.rdram,f.priv+0x1F8)==1,"Unchanged nativeBF08 restores saved primary1/callback43450 and flag1");const auto ready=f.read();need(ready.held==ghost&&ready.held_flag==1,"Restored native held identity survives flag0to1");
  // Explicit HOST boundary: native cancellation is now entered with no held
  // pointer, matching the actual sample; the returned sparse child remains.
  f.word(f.priv+0x1F0,0);const auto before=f.bytes;recomp_context cancel{};cancel.r4=n::base(f.player);cancel.r5=n::base(f.priv);cancel.r29=n::base(stack);actual_cancel::func_80040C48(f.rdram,&cancel);++native_calls;need(uint32_t(cancel.r29)==stack&&!n::byte(f.rdram,f.priv+0x1F8),"Unchanged40C48 zero-held branch clears flag and restores stack without calling forbidden services");need(std::equal(before.begin()+(ghost&0x7FFFFF),before.begin()+(ghost&0x7FFFFF)+0x1FC,f.bytes.begin()+(ghost&0x7FFFFF)),"Actual zero-held cancellation preserves the returned sparse4005 actor");const auto cancelled=f.read();need(!cancelled.held&&!cancelled.reticle&&!cancelled.held_flag&&n::word(f.rdram,f.player+0x188+8*4)==ghost,"Cancel result is idle despite still-reciprocal returned4005");}
 for(unsigned fault=0;fault<10;++fault){Memory f(capture);f.word(f.priv+0x1F0,ghost);if(fault==0)f.word(f.priv+0x1F0,0);if(fault==1)f.half(ghost+12,0x4004);if(fault==2)f.byte(ghost+0x17D,3);if(fault==3)f.byte(ghost+0x18,0xFF);if(fault==4)f.word(ghost+0x180,0);if(fault==5)f.half(ghost+0x17E,20);if(fault==6)f.half(f.player+0x184,0);if(fault==7)f.half(f.player+0x184,21);if(fault==8)f.word(f.player+0x188+3*4,ghost);if(fault==9)f.word(f.player+0x188+8*4,0);need(!f.read().held,"Wrong private/type/queue/disposed/parent/slot/count/duplicate reciprocal graph cannot manufacture held aim");}
 {Memory f(capture);f.half(ghost+12,0x4003);f.word(f.priv+0x1F4,ghost);const auto reticle=f.read();need(reticle.reticle==ghost&&!reticle.held,"Explicit HOST typed4003 uses current private1F4 authority");f.word(ghost+0x84,0x7FC00000);need(!f.read().reticle,"Unknown nonfinite4003 coordinates rejected");}
 {Memory f(capture);f.word(0x800F8828,0);need(!f.read().player_observed,"Foreign singleton rejected");}
 std::printf("BossNativeAimAuthority0284Checks: %u PASS; %u actual native entries; actual nonfenced GPU sample read-only + declared HOST frontiers; no constructor/GPU/gameplay claim\n",checks,native_calls);return 0;
 }catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}}
