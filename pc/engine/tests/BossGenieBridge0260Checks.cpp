#include "../boss_battle_runtime0250.hpp"
#include "../boss_driver_bridge0260.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <vector>
#include <cmath>
namespace c=workshop::boss_campaign0250;namespace b=workshop::boss_driver_bridge0260;namespace n=workshop::boss_native;namespace v=workshop::boss_variants0250;namespace f=workshop::boss_family_native0250;
static unsigned checks=0;static void need(bool ok,const char*why){++checks;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
static bool close(float a,float z){return std::abs(a-z)<=.001f;}
static void motion_checks(const char* path){
 std::ifstream input(path,std::ios::binary);std::vector<uint8_t> memory((std::istreambuf_iterator<char>(input)),{});
 need(memory.size()==0x800000,"Actual39b motion capture is a complete8MiB read-only source");
 const auto captured=memory;auto* rdram=memory.data();constexpr uint32_t root=0x800CB31C;
 const auto player=n::word(rdram,0x800F8828),data=n::word(rdram,root+0x1F4),resource=n::word(rdram,root+0x168),factory=n::word(rdram,resource+0x40);
 const auto value=[&](uint32_t address){return std::bit_cast<float>(n::word(rdram,address));};
 need(n::actor(rdram,root,0x4072)&&n::actor(rdram,player,0x4001),"Actual39b has the native4072 and player identities");
 need(c::configure("qa-genie-motion-bridge",1,true,true,2),"Motion caller enters through real isolated Runtime configure");
 c::rooms::configure(c::session.plan());c::enter_room(52);
 need(c::physical_room==169&&c::logical_room==163&&c::session.current().members[0].kind==0x4072,"Real case2 configures the actual Genie source and member");
 need(c::session.enter_fight(163,player)&&c::session.bind(0,root),"Actual session enters Fighting and binds its native root");c::bound[0]=root;
 need(v::bind(rdram,0,root)&&c::observers[0].bind(root),"Real variant birth and family observer bind independently");
 need(v::snapshot().members[0].birth&&v::snapshot().members[0].private_data==0,"Ordinary4072 variant bind intentionally leaves private_data zero");
 c::overlays[16]=factory;c::overlays[285]=0x80400000;c::current_vi=3000;
 recomp_context context{};context.r4=n::base(root);const auto frame=c::witness_frame(0);
 // Explicit host callback boundaries, not execution of native initialization.
 // Captured counter3/private/resource remain untouched. Temporarily present the
 // source C428 active entry, then restore every captured guest byte before use.
 c::observers[0].observe_entry(rdram,&context,"func_8042D33C",frame);c::observers[0].frame(rdram,frame);
 const auto primary=n::byte(rdram,root+0x18),sub=n::byte(rdram,root+0x30),callback=n::word(rdram,root+0x20);
 MEM_B(0x18,n::base(root))=1;MEM_B(0x30,n::base(root))=6;MEM_W(0x20,n::base(root))=int32_t(factory+0x1B88);
 c::observers[0].observe_entry(rdram,&context,"func_8042C428",frame);
 MEM_B(0x18,n::base(root))=uint8_t(primary);MEM_B(0x30,n::base(root))=uint8_t(sub);MEM_W(0x20,n::base(root))=int32_t(callback);
 const auto& observed=c::observers[0].snapshot();
 need(observed.enabled&&!observed.invalidated&&observed.registered_actors==1&&observed.actors[0].initialized&&observed.actors[0].active_seen&&observed.actors[0].private_data==data,"Actual Observer owns the native private identity after init/frame/active boundaries");
 need(memory==captured,"Observer boundary fixture restores all8MiB to the actual39b capture");
 const auto motion=b::observe_genie_motion(rdram,0);
 need(bool(motion),"Actual Bridge motion caller accepts the Observer private while variants.private_data remains zero");
 need(close(motion->x,value(root+0x84))&&close(motion->y,value(root+0x88)-10*value(root+0x70)),"Bridge returns the captured native collider centre rather than an invented root point");
 need(close(motion->vx,value(root+0xC4)*value(root+0x100)+value(root+0x10C))&&close(motion->vy,value(root+0xC8)*value(root+0x104)+value(root+0x110)),"Bridge returns exact per-native-update4072 motion from the captured source");
 need(memory==captured&&!b::observe_genie_motion(rdram,1)&&!b::observe_genie_motion(rdram,4)&&!b::observe_genie_motion(nullptr,0),"Foreign member/null source rejects without altering actual RAM");
 c::remember_controls(rdram,workshop::boss_battle0250::Phase::Fighting);
 need(c::flags_ready&&b::observe_controls(rdram,player),"Runtime admits the actual released-player controls baseline from its real active witness");
 b::configure();uint16_t buttons=0;float x=0,y=0;b::drive(rdram,3001,&buttons,&x,&y);
 need(b::controller.snapshot().target==root,"Complete drive caller consumes witnessed motion and selects4072 despite variants.private_data zero");
 need(memory==captured,"Composed Runtime/Observer/Bridge drive emits only inputs and preserves all8MiB");

 // Host snapshot faults only. No forged native hit, progress or private pointer
 // is used to make the positive path pass; each rejection starts from that path.
 auto& proof=const_cast<f::Snapshot&>(c::observers[0].snapshot());auto& variant=const_cast<v::Snapshot&>(v::snapshot());
 auto& state=const_cast<workshop::boss_battle0250::Snapshot&>(c::session.snapshot());
 for(unsigned fault=0;fault<35;++fault){
  const auto saved_proof=proof;const auto saved_variant=variant;const auto saved_state=state;
  const auto saved_bound=c::bound;const auto saved_epochs=c::observer_epochs;const auto saved_variant_epoch=c::variant_epoch;
  const auto saved_overlay=c::overlays[16],saved_physical=c::physical_room,saved_logical=c::logical_room;const auto saved_enabled=c::enabled,saved_driver=c::driver,saved_pending=c::restore_pending;
  switch(fault){
   case 0:proof.enabled=false;break;case 1:proof.invalidated=true;break;
   case 2:proof.registered_actors=0;break;case 3:proof.registered_actors=2;break;
   case 4:proof.actors[0].initialized=false;break;case 5:proof.actors[0].disposed=true;break;
   case 6:proof.actors[0].completion_returned=true;break;case 7:proof.actors[0].actor=player;break;
   case 8:proof.actors[0].private_data=0;break;case 9:proof.actors[0].private_data+=4;break;
   case 10:proof.run_token="foreign";break;case 11:proof.source_room=165;break;
   case 12:proof.family=f::Family::giant_lips;break;case 13:variant.members[0].actor=player;break;
   case 14:variant.members[0].birth=0;break;case 15:variant.enabled=false;break;
   case 16:variant.invalidated=true;break;case 17:variant.encounter=1;break;
   case 18:variant.room=165;break;case 19:++c::observer_epochs[0];break;
   case 20:++c::variant_epoch;break;case 21:state.phase=workshop::boss_battle0250::Phase::Rest;break;
   case 22:state.defeated=1;break;case 23:c::bound[0]=player;break;
   case 24:c::driver=false;break;case 25:c::enabled=false;break;
   case 26:c::overlays[16]+=4;break;case 27:c::restore_pending=true;break;
   case 28:c::physical_room=168;break;case 29:variant.members[0].receipt_sent=true;break;
   case 30:variant.members[0].retire_queued=true;break;case 31:proof.actors[0].terminal_seen=true;break;
   case 32:proof.expected_actors=2;break;case 33:proof.expected_hits=1;break;
   case 34:c::logical_room=165;break;
  }
  need(!b::observe_genie_motion(rdram,0)&&memory==captured,"Foreign/stale source, phase, lifetime, token, generation or retry epoch cannot publish motion or write RAM");
  proof=saved_proof;variant=saved_variant;state=saved_state;c::bound=saved_bound;c::observer_epochs=saved_epochs;c::variant_epoch=saved_variant_epoch;c::overlays[16]=saved_overlay;c::enabled=saved_enabled;c::driver=saved_driver;c::restore_pending=saved_pending;c::physical_room=saved_physical;c::logical_room=saved_logical;
 }
 need(bool(b::observe_genie_motion(rdram,0))&&memory==captured,"Restored independent host bindings recover only the original read-only native observation");
}
int main(int argc,char**argv){need(argc==4,"Supply actual f706 final and39b3000/3600 Genie dumps");std::ifstream input(argv[1],std::ios::binary);std::vector<uint8_t> memory((std::istreambuf_iterator<char>(input)),{});need(memory.size()==0x800000,"Complete8MiB source capture");auto*rdram=memory.data();constexpr uint32_t root=0x800CC204;const auto player=n::word(rdram,0x800F8828),priv=n::word(rdram,root+0x1F4),factory=n::word(rdram,n::word(rdram,root+0x168)+0x40);
 need(n::actor(rdram,root,0x4072)&&n::actor(rdram,player,0x4001),"Source contains actual Genie and native player");
 need(c::configure("qa-genie-controls",1,true,true,2),"Actual configure case2");c::rooms::configure(c::session.plan());c::enter_room(52);need(c::logical_room==163&&c::session.current().family==workshop::boss_battle0250::Family::Genie&&c::session.current().count==1,"Real seed1 selects original single Genie in physical169");need(c::session.enter_fight(163,player)&&c::session.bind(0,root),"Real member/session bind");c::bound[0]=root;need(v::bind(rdram,0,root),"Actual variant generation bind");need(v::snapshot().members[0].private_data==0,"Ordinary Genie variant private0 uses family witness identity");need(c::observers[0].bind(root),"Actual family observer bind");c::overlays[16]=factory;
 // Explicit CPU source pose only: reconstruct init counter3 and callbackC428
 // from the final dump; no native function, damage, hit or game is invoked.
 MEM_W(0x1E4,n::base(root))=3;MEM_B(0x18,n::base(root))=1;MEM_B(0x30,n::base(root))=6;MEM_W(0x20,n::base(root))=int32_t(factory+0x1B88);
 recomp_context ctx{};ctx.r4=n::base(root);const f::Frame frame{163,9000,factory,0x80400000};c::observers[0].observe_entry(rdram,&ctx,"func_8042D33C",frame);c::observers[0].frame(rdram,frame);need(c::observers[0].snapshot().actors[0].private_data==priv,"Real family frame owns captured native private identity");
 MEM_B(0,n::base(0x800F7EA5))=0;MEM_B(0,n::base(0x800F7EA6))=0;need(!b::observe_controls(rdram,player),"Init/intro0/0 alone cannot drive");c::remember_controls(rdram,workshop::boss_battle0250::Phase::Fighting);need(!c::flags_ready,"Runtime rejects unproved active Genie intro baseline");
 c::observers[0].observe_entry(rdram,&ctx,"func_8042C428",frame);need(c::observers[0].snapshot().actors[0].active_seen,"Source-accurate native active callback accepted by actual observer");need(!b::observe_controls(rdram,player),"Active observer alone does not substitute saved baseline");c::remember_controls(rdram,workshop::boss_battle0250::Phase::Fighting);need(c::flags_ready&&!c::flags5&&!c::flags6,"Actual Runtime remembers proved active Genie0/0");
 const auto before=memory;need(b::observe_controls(rdram,player)&&memory==before,"Genie0/0 admission observes all8MiB without writes");b::configure();uint16_t buttons=0;float x=0,y=0;b::drive(rdram,9001,&buttons,&x,&y);need(b::controller.snapshot().have_vi&&b::controller.snapshot().last_vi==9001&&b::controller.snapshot().player==player&&memory==before,"Complete Bridge advances the exact saved0/0 player while retaining its independent support navigation guard");
 auto& proof=const_cast<f::Snapshot&>(c::observers[0].snapshot());auto& variant=const_cast<v::Snapshot&>(v::snapshot());for(unsigned fault=0;fault<13;++fault){const auto saved=proof;const auto variants=variant;const auto old_ready=c::flags_ready,old_pending=c::restore_pending;const auto old_overlay=c::overlays[16];switch(fault){case 0:proof.actors[0].active_seen=false;break;case 1:proof.run_token="foreign";break;case 2:proof.source_room=165;break;case 3:proof.family=f::Family::giant_lips;break;case 4:proof.actors[0].private_data+=4;break;case 5:proof.actors[0].disposed=true;break;case 6:proof.actors[0].completion_returned=true;break;case 7:variant.members[0].birth=0;break;case 8:variant.members[0].actor=player;break;case 9:variant.encounter=1;break;case 10:c::overlays[16]+=4;break;case 11:c::flags_ready=false;break;case 12:c::restore_pending=true;break;}need(!b::observe_controls(rdram,player)&&memory==before,"Foreign/init/unbound/generation/private/baseline refuses without writes");proof=saved;variant=variants;c::flags_ready=old_ready;c::restore_pending=old_pending;c::overlays[16]=old_overlay;}
 motion_checks(argv[2]);motion_checks(argv[3]);
 std::printf("BossGenieBridge0260Checks: %u PASS; real Runtime/variants/Observer/Bridge pipeline and actual f706/39b graphs; explicit host callback boundaries only, no native game/GPU/victory proof.\n",checks);
}
