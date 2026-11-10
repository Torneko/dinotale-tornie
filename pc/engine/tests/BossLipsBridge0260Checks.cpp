// Integration of actual host bind APIs and captured native RAM. No game/GPU.
#include "../boss_battle_runtime0250.hpp"
#include "../boss_driver_bridge0260.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <vector>
namespace c=workshop::boss_campaign0250;namespace b=workshop::boss_driver_bridge0260;namespace n=workshop::boss_native;namespace v=workshop::boss_variants0250;namespace f=workshop::boss_family_native0250;
static unsigned checks=0;static void need(bool ok,const char*why){++checks;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
struct Fixture{
 std::vector<uint8_t> memory;uint8_t*rdram;static constexpr uint32_t root=0x800C97C8;uint32_t player=0,priv=0;
 explicit Fixture(const std::vector<uint8_t>& captured):memory(captured),rdram(memory.data()){
  player=n::word(rdram,0x800F8828);priv=n::word(rdram,root+0x1F4);
  need(c::configure("qa-lips-bridge",1,true,true,1),"fresh isolated diagnostic session uses real configure API");c::rooms::configure(c::session.plan());c::enter_room(52);
  need(c::session.current().members[0].kind==0x40A3&&c::session.current().count==1&&c::logical_room==165,"seed1/case1 really selects single Lips");
  need(c::session.enter_fight(165,player)&&c::session.bind(0,root),"real session enters and binds this root");c::bound[0]=root;
  need(v::bind(rdram,0,root),"real variants bind accepts captured root");
  need(v::snapshot().members[0].actor==root&&v::snapshot().members[0].birth&&v::snapshot().members[0].private_data==0,"ordinary Lips actual variants bind leaves private0 intentionally");
  need(c::observers[0].bind(root),"real family member wrapper binds root");recomp_context ctx{};ctx.r4=n::base(root);
  const f::Frame frame{165,100,n::word(rdram,n::word(rdram,root+0x168)+0x40),0x80400000};c::overlays[120]=frame.family_overlay;
  c::observers[0].observe_entry(rdram,&ctx,"func_8055863C",frame);c::observers[0].frame(rdram,frame);
  const auto& proof=c::observers[0].snapshot();need(proof.enabled&&!proof.invalidated&&proof.registered_actors==1&&proof.actors[0].initialized&&proof.actors[0].private_data==priv,"real witness init/frame owns the captured private identity");
 }
};
int main(int argc,char**argv){need(argc==2,"cold Lips3000 capture required");std::ifstream file(argv[1],std::ios::binary);need(bool(file),"capture opens read only");const std::vector<uint8_t> captured((std::istreambuf_iterator<char>(file)),{});need(captured.size()==0x800000,"full8MiB capture");
 {Fixture x(captured);auto*rdram=x.rdram;const auto before=x.memory;const auto point=b::observe_lips_member(x.rdram,0);need(point&&point->actor==0x800CAE24&&point->y==146,"Bridge observes real40A7 using witness private despite actual variant private0");need(x.memory==before,"complete bound-member observation leaves all8MiB unchanged");b::lips_jump_logs=512;b::configure();need(b::lips_jump_logs==0,"new diagnostic session resets bounded Lips jump journal");uint16_t buttons=0;float stickx=0,sticky=0;b::drive(x.rdram,3001,&buttons,&stickx,&sticky);need(b::controller.snapshot().target==Fixture::root,"complete drive adapter selects bound Lips with actual variant private0");need(x.memory==before,"complete Bridge drive emits only input and preserves all8MiB");need(!b::observe_lips_member(x.rdram,1)&&!b::observe_lips_member(x.rdram,4),"foreign/out-of-range member rejects");
 // Explicit synthetic pose in the known left wall makes LOS false and tests
 // bounded log emission only, never a physical navigation claim.
 MEM_W(0x84,n::base(x.player))=int32_t(std::bit_cast<uint32_t>(48.f));const auto log_fixture=x.memory;b::lips_jump_logs=511;b::drive(x.rdram,3060,&buttons,&stickx,&sticky);need(b::lips_jump_logs==512&&x.memory==log_fixture,"one eligible native observation emits last bounded journal row without guest writes");b::drive(x.rdram,3120,&buttons,&stickx,&sticky);need(b::lips_jump_logs==512&&x.memory==log_fixture,"journal cap512 stops later eligible rows and preserves all8MiB");}
 // Synthetic HOST observer/binding snapshots below test rejection gates only;
 // they never manufacture guest memory, a receipt, hit, damage or victory.
 for(unsigned fault=0;fault<17;++fault){Fixture x(captured);auto& proof=const_cast<f::Snapshot&>(c::observers[0].snapshot());auto& variant=const_cast<v::Snapshot&>(v::snapshot());switch(fault){case 0:proof.enabled=false;break;case 1:proof.invalidated=true;break;case 2:proof.registered_actors=0;break;case 3:proof.registered_actors=2;break;case 4:proof.actors[0].initialized=false;break;case 5:proof.actors[0].disposed=true;break;case 6:proof.actors[0].actor=x.player;break;case 7:proof.actors[0].private_data=0;break;case 8:proof.actors[0].private_data+=4;break;case 9:proof.actors[0].completion_returned=true;break;case 10:proof.run_token="foreign";break;case 11:proof.source_room=164;break;case 12:variant.members[0].actor=x.player;break;case 13:variant.members[0].birth=0;break;case 14:variant.enabled=false;break;case 15:variant.invalidated=true;break;case 16:variant.encounter=2;break;}const auto before=x.memory;need(!b::observe_lips_member(x.rdram,0)&&x.memory==before,"unproved witness/generation/session cannot publish native weak point and never writes RAM");}
 // Explicit CPU copy: a source-accurate active callback establishes the real
 // host witness, then the existing Runtime remembers a playable0/0 baseline.
 // It does not invoke game code, synthesize a hit, or change the original dump.
 {Fixture x(captured);auto*rdram=x.rdram;const auto p=n::word(rdram,x.player+0x1F4);
  need(b::observe_controls(rdram,x.player),"legacy context6=1 keeps its prior admission unchanged");
  MEM_B(0,n::base(0x800F7EA5))=0;MEM_B(0,n::base(0x800F7EA6))=0;
  need(!b::observe_controls(rdram,x.player),"context0/0 without witnessed baseline cannot drive a native intro");
  c::remember_controls(rdram,workshop::boss_battle0250::Phase::Fighting);need(!c::flags_ready,"Runtime refuses an initialized but never-active Lips intro baseline");
  const auto old_sub=n::byte(rdram,Fixture::root+0x30),old_callback=n::word(rdram,Fixture::root+0x20),factory=n::word(rdram,n::word(rdram,Fixture::root+0x168)+0x40);
  MEM_B(0x30,n::base(Fixture::root))=1;MEM_W(0x20,n::base(Fixture::root))=int32_t(factory+0xC44);
  recomp_context ctx{};ctx.r4=n::base(Fixture::root);c::observers[0].observe_entry(rdram,&ctx,"func_80558794",{165,3000,factory,0x80400000});
  MEM_B(0x30,n::base(Fixture::root))=int8_t(old_sub);MEM_W(0x20,n::base(Fixture::root))=int32_t(old_callback);
  need(c::observers[0].snapshot().actors[0].active_seen,"source-accurate58794 active callback establishes real bound observer activity");
  need(!b::observe_controls(rdram,x.player),"active witness alone cannot bypass missing saved baseline");
  c::remember_controls(rdram,workshop::boss_battle0250::Phase::Fighting);need(c::flags_ready&&!c::flags5&&!c::flags6,"existing Runtime captures the actual witnessed playable0/0 baseline");
  const auto before=x.memory;need(b::observe_controls(rdram,x.player)&&x.memory==before,"exact released active Lips0/0 controls observation is read only");
  b::configure();uint16_t buttons=0x1234;float sx=.5f,sy=.5f;b::drive(rdram,3001,&buttons,&sx,&sy);need(b::controller.snapshot().target==Fixture::root&&x.memory==before,"complete Bridge advances actual Lips pilot with saved0/0 and preserves all8MiB");
  for(unsigned fault=0;fault<14;++fault){const auto memory=x.memory;const auto old_ready=c::flags_ready,old_pending=c::restore_pending,old_drive=c::driver;const auto old5=c::flags5,old6=c::flags6;auto& proof=const_cast<f::Snapshot&>(c::observers[0].snapshot());const auto saved_proof=proof;
   switch(fault){case 0:c::flags_ready=false;break;case 1:c::flags5=1;break;case 2:c::flags6=1;break;case 3:c::restore_pending=true;break;case 4:proof.actors[0].active_seen=false;break;case 5:proof.invalidated=true;break;case 6:proof.run_token="foreign";break;case 7:MEM_B(0x165,n::base(x.player))=1;break;case 8:MEM_B(0x17D,n::base(x.player))=0;break;case 9:MEM_B(0x18,n::base(x.player))=0xA;break;case 10:MEM_W(0x12EC,n::base(p))=1;break;case 11:MEM_B(0,n::base(0x800F7EA5))=1;break;case 12:MEM_W(0,n::base(0x800F8828))=int32_t(Fixture::root);break;case 13:c::driver=false;break;}
   const auto invalid=x.memory;need(!b::observe_controls(rdram,x.player)&&x.memory==invalid,"missing/foreign/locked/unreleased baseline rejects without guest writes");x.memory=memory;rdram=x.rdram=x.memory.data();c::flags_ready=old_ready;c::restore_pending=old_pending;c::driver=old_drive;c::flags5=old5;c::flags6=old6;proof=saved_proof;
  }
 }
 std::printf("BossLipsBridge0260Checks: %u PASS; actual variants/member bind APIs with private0, read-only captured ABI and diagnostic inputs only.\n",checks);
}
