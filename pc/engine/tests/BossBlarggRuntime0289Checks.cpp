// HOST integration checks reuse the literal native helper corpus; constructor allocation,
// Session admission and copying an observed ledger are declared fixtures, not gameplay proof.
#define main helper_checks_not_run
#include "BossBlargg0289Checks.cpp"
#undef main
#undef LOOKUP_FUNC
#define LOOKUP_FUNC(v) get_function(int32_t(v))
#include "../boss_checkpoint_runtime0260.hpp"
#include RUNTIME_NATIVE_STUBS
namespace rt=workshop::boss_campaign0250;
namespace bc=workshop::boss_battle0250;
static unsigned runtime_checks=0;
static void check(bool v,const char* message){++runtime_checks;need(v,message);}
static uint64_t seed_for_pair(){bc::Config cfg;cfg.blargg=true;for(uint64_t seed=0;seed<1000;++seed)if(bc::make_plan(seed,cfg).fights[2].family==bc::Family::Blargg)return seed;throw std::runtime_error("No Blargg seed at model index2");}
static void setup(Fixture& f){
 check(rt::configure("blargg-0289",seed_for_pair(),true,false,2,false,false,true),"Explicit opt-in Runtime model config");
 rt::physical_room=169;rt::logical_room=19;rt::current_vi=f.frame.vi;rt::overlays[90]=Fixture::overlay;
 check(rt::session.enter_fight(19,Fixture::player),"HOST Session admitted at exact source19");
 rt::blargg_observer=f.observer; // Explicit HOST import of separately observed native-shaped constructors.
 check(rt::variants::begin(rt::session.current(),2,rt::session.plan().config,1),"Real variants accepts bounded two-member plan");
 rt::variants::serial=99;
 for(unsigned i=0;i<2;++i){rt::bound[i]=Fixture::root(i);check(rt::session.bind(i,Fixture::root(i)),"HOST Session member binding");check(rt::variants::bind(f.r,i,Fixture::root(i))&&rt::variants::initialize_blargg(f.r,i,rt::blargg_observer),"HOST-only variants initializer imports authenticated ledger birth");}
 rt::observer_epochs.fill(0);rt::variant_epoch=0;
 f.b(Fixture::player+0x18,1);f.b(Fixture::player+0x17D,2);f.b(Fixture::player+0x165,0);f.w(Fixture::player+0x1F4,0x80072000);f.w(0x80072000+0x12EC,0);
 f.b(0x800F7EA5,0);f.b(0x800F7EA6,0);
 rt::fight_ready={f.frame.vi,2,169,19,0,Fixture::player,0x80072000};
}
int main(){try{
 {check(rt::configure("legacy-runtime",10,true,false),"Legacy Runtime configuration");check(!rt::session.plan().config.blargg&&!rt::blargg_owned()&&!rt::blargg_observer.snapshot().enabled,"Default Runtime keeps new boss disabled");}
 {Fixture f;setup(f);const auto memory=f.memory;const auto context=f.c;
  auto frame=rt::blargg_frame(f.r,true);check(frame.combat_active&&frame.quiescent&&frame.room==169&&frame.logical==19&&frame.encounter==2&&frame.overlay==Fixture::overlay&&frame.player==Fixture::player,"Frame binds physical room, logical source, plan index, overlay and true player");
  check(!rt::witnessed_zero_controls(f.r,Fixture::player),"Bare constructors cannot supply native activity or zero-controls permission");
  for(unsigned i=0;i<2;++i){f.c.r4=n::base(Fixture::root(i));rt::trace(f.r,&f.c,"func_804F5384",f.frame.vi,reinterpret_cast<uint32_t*>(sections));}
  check(rt::blargg_active_member(f.r,0)&&rt::blargg_active_member(f.r,1)&&rt::witnessed_zero_controls(f.r,Fixture::player),"Exact native5384 entry authenticates initial activity before first hit");
  f.c=context;check(f.memory==memory&&std::memcmp(&f.c,&context,sizeof(context))==0,"HOST Runtime activity checks preserve full RAM and caller registers");
  rt::scale_scopes.emplace_back(1,0);check(!rt::blargg_frame(f.r,true).quiescent,"Open scale scope rejects retirement fence");rt::scale_scopes.clear();
  rt::variants::paint_stack.push_back({});check(!rt::blargg_frame(f.r,true).quiescent,"Open paint scope rejects retirement fence");rt::variants::paint_stack.clear();
  rt::variants::damage_stack.push_back({});check(!rt::blargg_frame(f.r,true).quiescent,"Open collision scope rejects retirement fence");rt::variants::damage_stack.clear();
  check(!rt::blargg_frame(f.r,false).quiescent,"Entry hooks never supply a completed-frame fence");
  rt::physical_room=19;check(!rt::blargg_owned()&&!rt::blargg_frame(f.r,true).combat_active,"Vanilla source room cannot arm new encounter");rt::physical_room=169;
  rt::logical_room=97;check(!rt::blargg_owned(),"Wrong logical source cannot arm new encounter");rt::logical_room=19;
  rt::fight_ready.epoch=1;check(!rt::blargg_frame(f.r,true).combat_active,"Stale ready epoch cannot admit combat");rt::fight_ready.epoch=0;
  f.w(0x800F8828,Fixture::root(0));check(!rt::blargg_frame(f.r,true).combat_active&&!rt::witnessed_zero_controls(f.r,Fixture::player),"Wrong canonical native player rejects frame and input");f.w(0x800F8828,Fixture::player);
  f.w(Fixture::root(0)+0x1F8,0x80045BD8);check(!rt::blargg_active_member(f.r,0),"Common root destructor cannot substitute for native private callback");f.w(Fixture::root(0)+0x1F8,Fixture::overlay+0x128);
  rt::blargg_forgotten(Fixture::egg);check(!rt::failed,"Unbound actor destruction is harmless");
  constexpr uint32_t extra=0x80025000;check(!rt::blargg_observer.birth(extra),"No birth from constructor-shaped memory alone");rt::blargg_created(extra);check(rt::blargg_observer.birth(extra)!=0,"Loader create API uses refreshed current frame");rt::blargg_forgotten(extra);check(!rt::blargg_observer.birth(extra),"Loader full destruction forgets only observed birth");
 }
 {Fixture f;setup(f);
  for(unsigned member=0;member<2;++member){for(unsigned hit=0;hit<3;++hit)f.hit(member);f.retreat(member);check(f.retire(member),"Literal native hit/retract/retire produces true helper receipt");
   rt::current_vi=f.frame.vi;rt::blargg_observer=f.observer; // Declared HOST routing boundary: no claim of Loader pre/post hook dispatch here.
   const auto memory=f.memory;rt::drain_receipts(f.r);check(f.memory==memory,"Runtime draining authentic native receipts is guest-readonly");
   check(!rt::failed&&rt::session.snapshot().defeated==((1u<<(member+1))-1),"Runtime delivers unique exact-member causal receipt");
   check(!rt::restore_pending,"No cinematic restore is injected for native Blargg retirement");
   check(rt::session.snapshot().phase==(member?bc::Phase::Victory:bc::Phase::Fighting),"One receipt cannot win; second native receipt unlocks Session Victory");
   check(rt::history[2].members[member].actor==Fixture::root(member),"History records the correct independently born member");
   f.observer.take_receipts();rt::drain_receipts(f.r);check(!rt::failed&&rt::session.snapshot().defeated==((1u<<(member+1))-1),"Empty drain cannot duplicate a terminal receipt");
  }
  check(!rt::exit_pot,"HOST receipt proof does not pretend native exit allocation occurred");
 }
 std::cout<<"BossBlarggRuntime0289Checks: "<<runtime_checks<<" Runtime checks PASS; "<<body_calls<<" literal native body calls; "<<readonly_hooks<<" readonly helper hooks; HOST Session/ledger routing only, no scene or retry proof.\n";
 std::cout<<"{\"runtimeChecks\":"<<runtime_checks<<",\"totalAssertions\":"<<checks<<",\"nativeBodyCalls\":"<<body_calls<<",\"readonlyHelperHooks\":"<<readonly_hooks<<"}\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
