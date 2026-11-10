#include "../boss_checkpoint_runtime0260.hpp"
#include "../boss_fish_cp6_bridge0284.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
namespace c=workshop::boss_campaign0250;namespace b=workshop::boss_battle0250;namespace n=workshop::boss_native;
namespace bridge=workshop::boss_fish_cp6_bridge0284;namespace intro=workshop::boss_fish_companion_intro0260;
static unsigned checks=0;static void need(bool yes,const char* why){++checks;if(!yes)throw std::runtime_error(why);}
struct Fixture {
 static constexpr uint32_t player=0x800C9CC0,fish=0x800C9A44,genie=0x800C9F3C;
 static constexpr unsigned vi=2841;std::vector<uint8_t> bytes;uint8_t* ram;
 explicit Fixture(const std::vector<uint8_t>& capture):bytes(capture),ram(bytes.data()){
  need(n::word(ram,0x800F8828)==player&&n::actor(ram,player,0x4001)&&n::actor(ram,fish,0x4511)&&n::actor(ram,genie,0x4072),"Actual95e4 RAM contains the exact singleton and two separately typed members");
  need(n::byte(ram,genie+0x18)==1&&n::byte(ram,genie+0x30)==0&&n::word(ram,genie+0x1E4)==2,"Actual real Genie is still in native intro1/0 with its configured2HP");
  c::enabled=c::driver=true;c::failed=false;c::finished=false;c::restore_pending=false;c::physical_room=173;c::logical_room=97;c::current_vi=vi;
  // EXPLICIT HOST boundary setup only. No earlier victory, real native init,
  // checkpoint wake, death or retry is claimed by these value transitions.
  need(c::session.begin("actual95e4-pair-host-fixture",b::make_plan(37784098137700ull),true)&&c::session.diagnostic_entry(5)&&c::session.diagnostic_cp6_rest6_fixture()&&c::session.diagnostic_bind_initial_rest6(player)&&c::session.use_rest_pot()&&c::session.enter_fight(97,player),"Explicit HOST fixture reaches the exact seventh-fight Session without inventing a wake");
  c::history={};c::bound={};c::observers={};c::observer_epochs={};c::variant_epoch=0;c::overlays={};
  c::checkpoints::configure(true,true);c::checkpoints::allow_retries(true);
  const auto& plan=c::session.plan();need(c::variants::begin(plan.fights[6],6,plan.config,plan.seed),"Mutable variant HOST context uses the actual current planner encounter");
  need(c::variants::bind(ram,0,fish)&&c::variants::bind(ram,1,genie)&&c::session.bind(0,fish)&&c::session.bind(1,genie),"Bindings refer to the actual immutable RAM actor identities");
  c::bound[0]=fish;c::bound[1]=genie;
  // Only the mutable Fish's HOST initialization is reconstructed. A real
  // native init body is NOT executed here; actual RAM private is read back.
  auto& f=c::variants::state.members[0];f.initialized=true;f.private_data=n::word(ram,fish+0x1F4);
  c::overlays[148]=n::word(ram,n::word(ram,fish+0x168)+0x40);
  c::overlays[16]=n::word(ram,n::word(ram,genie+0x168)+0x40);
  c::fight_ready={vi,6,173,97,0,player,n::word(ram,player+0x1F4)};
  configure_observer(c::session.snapshot().token,97,"func_8042D33C",genie);
  no_fake_genie();
 }
 void no_fake_genie()const{const auto& g=c::variants::snapshot().members[1];need(!g.initialized&&!g.private_data,"Genie mutable binding stays uninitialized/private0; no fabricated variant lifetime");}
 void configure_observer(std::string token,unsigned source,std::string_view init,uint32_t root){
  c::observers[1]={};need(c::observers[1].configure(c::session.current(),1,std::move(token),source,true)&&c::observers[1].bind(root),"Companion MemberObserver HOST context binds its explicit typed encounter");
  recomp_context ctx{};ctx.r4=n::base(root);const auto regs=ctx;const auto before=bytes;
  c::witnesses::Frame frame{source,vi,c::overlays[16],0};
  // Synthetic call boundary around an unchanged actual snapshot. This does
  // not executeD33C or prove that a native constructor callback just ran.
  c::observers[1].observe_entry(ram,&ctx,init,frame);c::observers[1].frame(ram,frame);
  need(before==bytes&&!std::memcmp(&ctx,&regs,sizeof(ctx)),"HOST observer reconstruction is read-only across8MiB and all guest registers");
 }
 void word(uint32_t a,uint32_t v){auto* rdram=ram;MEM_W(0,n::base(a))=v;}
 void half(uint32_t a,uint16_t v){auto* rdram=ram;MEM_H(0,n::base(a))=v;}
 auto sample(){const auto before=bytes;std::array<workshop::boss_route_checkpoint_test0260::Target,4> targets{};const auto out=bridge::make_frame(ram,vi,targets);need(before==bytes,"Actual bridge.make_frame preserves every byte of its8MiB input");no_fake_genie();return out;}
};
int main(int argc,char** argv){try{
 need(argc==2,"Provide the immutable actual95e4 final capture");std::ifstream in(argv[1],std::ios::binary);std::vector<uint8_t> captured((std::istreambuf_iterator<char>(in)),{});need(captured.size()==0x800000,"Actual captured input is exactly8MiB");
 {Fixture f(captured);const auto before=f.bytes;const auto pair=c::fish_companion_intro_frame(f.ram);need(f.bytes==before&&pair.active&&pair.index==6&&pair.room==173&&pair.source==97&&pair.player==Fixture::player,"Production pair context recognizes the two actual roots in the declared HOST encounter");
  need(intro::member(f.ram,pair.fish,0,0x188,0x3FF0,0x35A8)&&intro::member(f.ram,pair.companion,0,0x28,0x3200,0x2A9C),"Actual Fish and Genie resources prove their independent native factories and private extents");
  const auto& proof=c::observers[1].snapshot();need(proof.actors[0].initialized&&!proof.actors[0].active_seen&&proof.actors[0].actor==Fixture::genie&&proof.actors[0].private_data==n::word(f.ram,Fixture::genie+0x1F4),"Only the MemberObserver owns the native-shaped Genie initialization; active callback remains unseen");
  need(c::variants::live(f.ram,0)&&!c::variants::live(f.ram,1),"Actual mixed roster reproduces old bug: Fish live=true and mutable Genie live=false");
  const auto read=f.sample();need(read.members_live&&read.members[0]==Fixture::fish&&read.members[1]==Fixture::genie&&read.births[0]&&read.births[1]&&read.births[0]!=read.births[1],"Shared QA reader accepts the real mixed pair without requiring false Genie variant initialization");
  need(f.bytes==captured,"Positive actual snapshot remains byte-identical through all observations");
 }
 // HOST context negatives preserve the same actual RAM, and never manufacture
 // a native field, active callback, initialization or checkpoint receipt.
 for(unsigned fault=0;fault<12;++fault){Fixture f(captured);switch(fault){
  case 0:f.configure_observer("foreign-token",97,"func_8042D33C",Fixture::genie);break;
  case 1:f.configure_observer(c::session.snapshot().token,163,"func_8042D33C",Fixture::genie);break;
  case 2:c::observer_epochs[1]=1;break;case 3:c::variant_epoch=1;break;
  case 4:f.configure_observer(c::session.snapshot().token,97,"func_8042D338",Fixture::genie);break;
  case 5:c::bound[1]=Fixture::fish;break;case 6:c::variants::state.members[1].birth=0;break;
  case 7:c::variants::state.members[1].receipt_sent=true;break;case 8:c::variants::state.members[1].retire_queued=true;break;
  case 9:c::variants::state.members[0].initialized=false;break;case 10:c::variants::state.members[0].private_data=0;break;
  case 11:c::overlays[16]+=4;break;
 }need(!f.sample().members_live,"Foreign HOST token/source/epoch/init label/root/birth/receipt/private/factory context cannot certify the pair");need(f.bytes==captured,"HOST context negative leaves the actual snapshot untouched");}
 // Explicit HOST fault copies model corrupted native metadata. The original
 // capture/file remains immutable; each call must preserve the modified copy.
 for(unsigned fault=0;fault<7;++fault){Fixture f(captured);const auto resource=n::word(f.ram,Fixture::genie+0x168);switch(fault){
  case 0:f.word(Fixture::genie+0x1F4,n::word(f.ram,Fixture::genie+0x1F4)+4);break;
  case 1:f.word(resource+0x44,n::word(f.ram,resource+0x44)+4);break;
  case 2:f.word(Fixture::genie+0x168,n::word(f.ram,Fixture::fish+0x168));break;
  case 3:f.half(resource+0x3C,0x4073);break;
  case 4:f.half(Fixture::genie+12,0x4511);break;
  case 5:f.word(resource+0x40,n::word(f.ram,resource+0x40)+4);break;
  case 6:f.word(Fixture::fish+0x1F4,n::word(f.ram,Fixture::fish+0x1F4)+4);break;
 }need(!f.sample().members_live,"Wrong native private/resource/factory/kind in a labelled HOST fault copy fails closed");}
 c::enabled=c::driver=false;
 std::printf("BossFishPairObserver0284Checks PASS %u checks; actual95e4 RAM plus declared HOST runtime/call-boundary and fault-copy contexts; nativeBodiesExecuted=0; no native-init, first-pair event, wake, defeat, restore or GPU claim\n",checks);return 0;
 }catch(const std::exception& e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
