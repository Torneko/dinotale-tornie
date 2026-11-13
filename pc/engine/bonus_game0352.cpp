#include "bonus_game0352.hpp"
#include "bonus_transport0352.hpp"
#include "bonus_goal0352.hpp"
#include "bonus_retry0353.hpp"
#include "bonus_team0353.hpp"
#include "bonus_factory0353.hpp"
#include "bonus_exit_policy0353.hpp"
#include "bonus_finale_bridge0353.hpp"
#include "bonus_finale_entries0353.hpp"
#include "bonus_fish0354.hpp"
#include "boss_candy_pool0260.hpp"
#include <chrono>
#include "workshop_menu.hpp"
#include "workshop_menu_game.hpp"
#include "workshop_data.hpp"
#include <atomic>
#include <fstream>
#include <cstdio>
#include <string_view>
extern "C" void func_800447C8(uint8_t*,recomp_context*);
namespace workshop::bonus_game0352 {
namespace {
namespace rt=bonus_runtime0352;
namespace tx=bonus_transport0352;
rt::Service service;
tx::Transport transport;
bonus_factory0353::Factory portal_factory;
bonus_exit_policy0353::Policy exit_policy;
bonus_finale_bridge0353::Bridge finale_bridge;
// Release authority remains the genuine native cleanup/unregistration, never a retry reset.
boss_candy_pool0260::Guard bonus_candy_pool0354;
std::optional<bonus_exit_policy0353::RetireAction> retiring_exit;
unsigned native_vi=0;const int32_t* native_overlays=nullptr;size_t native_overlay_count=0;
bonus_finale_bridge0353::Frame finale_frame(){return {native_vi,{native_overlays,native_overlay_count}};}
bool finale_ticket(const rt::Ticket& t){return t.entry==bonus_routes0351::EntryRequirement::FinaleSetupPending;}
uint64_t room_epoch=0;unsigned spawned_portals=0;
std::atomic_bool route_preview{false},complete_preview{false},completed{false},game_over{false};
bool force_new_attempt=false;
std::atomic_uint configured_bonus{1};uint64_t configured_seed=24;
std::optional<Launch> requested_launch;
std::optional<bonus_finale_bridge0353::Completion> pending_completion;
// Public availability stays closed until the grouped native gameplay test.
constexpr bool public_bonus_ready=false;
bonus_retry0353::Adapter retry_adapter;
bonus_team0353::Team team;
bool retrying=false,prefer_backward=false;
std::mutex profile_mutex;
std::filesystem::path profile;
std::atomic_bool private_preview{false};
std::atomic_bool interrupted{false};
unsigned room=177;
uint64_t attempt_epoch=0;
bool admitted=false,ready=false;
std::optional<rt::Ticket> pending_admission;
thread_local bool dispatching=false;
rt::LaunchPolicy policy=rt::LaunchPolicy::PrivateTwoRoomBridge;
bool actor(uint8_t* rdram,uint32_t p,unsigned kind){
 return rdram&&p>=0x80000000u&&p<=0x80800000u-0x1FC&&
 uint32_t(MEM_W(0,int32_t(p)))==0x19971110&&MEM_HU(12,int32_t(p))==kind;
}
unsigned course(uint8_t* rdram){
 const unsigned page=MEM_BU(0,int32_t(0x800F8DFC)),column=MEM_BU(0,int32_t(0x800F8DFD));
 return page<6&&column<4?page*4+column:24;
}
void set_course(uint8_t* rdram,unsigned value){
 workshop::require(value<24,"Bonus course outside original arrays");
 MEM_B(0,int32_t(0x800F8DFC))=value/4;MEM_B(0,int32_t(0x800F8DFD))=value%4;
}
tx::Environment environment(uint8_t* rdram){
 tx::Environment e;const auto ticket=service.current_ticket();
 e.pc_mode=active()?7:-1;e.bonus=ticket?ticket->bonus:0;e.seed=ticket?ticket->seed:0;
 e.attempt_epoch=attempt_epoch;e.native_course=rdram?course(rdram):(ticket?ticket->course:24);
 e.canonical=false;e.custom_active=false;e.deferred=false;e.prefer_backward=prefer_backward;return e;
}
bonus_retry0353::Environment retry_environment(){
 bonus_retry0353::Environment e;const auto ticket=service.current_ticket();
 e.pc_mode=active()?7:-1;e.bonus=ticket?ticket->bonus:0;e.seed=ticket?ticket->seed:0;
 e.attempt_epoch=attempt_epoch;e.owned_mask=team.ownership();e.independent_bonus_save=admitted;
 e.canonical=false;e.custom_active=false;e.deferred=false;return e;
}
rt::Observation observed(uint8_t* rdram,const rt::Ticket& ticket){
 const auto p=uint32_t(MEM_W(0,int32_t(0x800F8828)));
 return {room,course(rdram),ticket.root,actor(rdram,p,0x4001)&&MEM_BU(2,int32_t(0x800F7EA0))==7};
}
void fail(const char* reason){
 if(!interrupted.exchange(true))fprintf(stderr,"[Bonus0352] interrupted reason=%s noCanonicalWrites=1\n",reason);
 book::request_open();book::pause();
}
void accept_ready(uint8_t* rdram){
 const auto ticket=service.current_ticket();if(!ticket||ready)return;
 const auto observation=observed(rdram,*ticket);if(!observation.player_ready)return;
 const auto result=service.room_ready(*ticket,observation);
 if(result.status!=rt::Status::Ready||!result.ticket){fail("room-ready-fence");return;}
 if(!retry_adapter.bind_room(*result.ticket,service.session(),retry_environment())){fail("retry-room-ready-binding");return;}
 ready=true;
 fprintf(stderr,"[Bonus0352] ready epoch=%llu revision=%llu ordinal=%u room=%u course=%u\n",
 (unsigned long long)result.ticket->epoch,(unsigned long long)result.ticket->revision,
 result.ticket->ordinal,result.ticket->room,result.ticket->course);
}
}
bool active(){return workshop_menu_enabled()&&workshop_menu_session_mode()==7;}
void refresh(const std::filesystem::path& next_profile){
 std::lock_guard guard(profile_mutex);
 const auto absolute=std::filesystem::canonical(std::filesystem::absolute(next_profile));
 if(profile!=absolute){
  workshop::require(!admitted,"Cannot replace an admitted Bonus profile");
  if(!service.load(absolute)){profile.clear();return;}profile=absolute;
 }
 service.refresh_access();
}
View view(){
 return {service.access(),service.session(),public_bonus_ready||complete_preview,private_preview,interrupted.load(),completed,game_over,configured_bonus.load()};
}
void configure_private_preview(const std::filesystem::path& next_profile,bool requested){
 private_preview=false;route_preview=false;complete_preview=false;force_new_attempt=false;configured_bonus=1;configured_seed=24;policy=rt::LaunchPolicy::PrivateTwoRoomBridge;
 if(!requested)return;
 const auto absolute=std::filesystem::canonical(std::filesystem::absolute(next_profile));
 workshop::require(absolute.filename().u8string().starts_with(u8"bonus0352-qa-"),"Bonus preview requires a separate QA profile");
 const auto marker=absolute/"bonus-bridge0352.txt";
 const auto state=std::filesystem::symlink_status(marker);
 workshop::require(std::filesystem::is_regular_file(state)&&!std::filesystem::is_symlink(state)&&std::filesystem::file_size(marker)<=128,"Bonus preview marker missing");
 std::ifstream file(marker,std::ios::binary);
 const std::string text{std::istreambuf_iterator<char>(file),{}};
 workshop::require(text=="DinoTale Bonus0352 two-room preview only\n","Invalid Bonus preview marker");
 refresh(absolute);private_preview=true;service.enable_private_bridge(true);
}
void configure_private_route(const std::filesystem::path& next_profile,bool requested,unsigned bonus,uint64_t seed){
 if(!requested)return;
 workshop::require(!private_preview&&bonus>=1&&bonus<=6,"Conflicting Bonus QA modes or page");
 const auto absolute=std::filesystem::canonical(std::filesystem::absolute(next_profile));
 workshop::require(absolute.filename().u8string().starts_with(u8"bonus0353-qa-"),"Authored Bonus preview needs a separate0353 QA profile");
 const auto marker=absolute/"bonus-route0353.txt";const auto state=std::filesystem::symlink_status(marker);
 workshop::require(std::filesystem::is_regular_file(state)&&!std::filesystem::is_symlink(state)&&std::filesystem::file_size(marker)<=128,"Authored Bonus QA marker missing");
 std::ifstream file(marker,std::ios::binary);const std::string text{std::istreambuf_iterator<char>(file),{}};
 workshop::require(text=="DinoTale Bonus0353 authored ordinary preview only\n","Invalid authored Bonus QA marker");
 refresh(absolute);private_preview=route_preview=true;configured_bonus=bonus;configured_seed=seed;policy=rt::LaunchPolicy::PrivateAuthoredOrdinary;
}
void configure_private_complete(const std::filesystem::path& next_profile,bool requested,unsigned bonus,uint64_t seed){
 if(!requested)return;workshop::require(!private_preview&&bonus>=1&&bonus<=6,"Conflicting complete Bonus QA modes or page");
 const auto absolute=std::filesystem::canonical(std::filesystem::absolute(next_profile));
 workshop::require(absolute.filename().u8string().starts_with(u8"bonus0353-qa-"),"Complete Bonus QA needs a separate0353 profile");
 const auto marker=absolute/"bonus-complete0353.txt";const auto state=std::filesystem::symlink_status(marker);
 workshop::require(std::filesystem::is_regular_file(state)&&!std::filesystem::is_symlink(state)&&std::filesystem::file_size(marker)<=128,"Complete Bonus QA marker missing");
 std::ifstream file(marker,std::ios::binary);const std::string text{std::istreambuf_iterator<char>(file),{}};
 workshop::require(text=="DinoTale Bonus0353 complete route preview only\n","Invalid complete Bonus QA marker");
 refresh(absolute);private_preview=route_preview=complete_preview=true;configured_bonus=bonus;configured_seed=seed;policy=rt::LaunchPolicy::PrivateAuthoredComplete;
}
void configure_bonus_selection(const std::filesystem::path& next_profile,unsigned bonus,uint64_t seed,bool reset){
 // The public menu cannot schedule this until the native test is validated.
 // The separate complete-QA marker also permits its menu to hand off a page.
 workshop::require(!private_preview&&bonus>=1&&bonus<=6,"Conflicting Bonus selection or page");
 if(!public_bonus_ready)configure_private_complete(next_profile,true,bonus,seed);
 else{refresh(next_profile);route_preview=complete_preview=true;configured_bonus=bonus;configured_seed=seed;policy=rt::LaunchPolicy::PrivateAuthoredComplete;std::lock_guard guard(profile_mutex);requested_launch=Launch{bonus,seed,reset};}
 force_new_attempt=reset;
}
bool request_bonus(unsigned bonus,bool reset){
 if(!(public_bonus_ready||complete_preview)||bonus<1||bonus>6)return false;
 const auto access=service.access();const auto saved=service.session();
 if(!(access.unlocked_mask&(1u<<(bonus-1)))||!saved.readable||!saved.last_write_succeeded)return false;
 const auto& slot=saved.attempts[bonus-1];const bool fresh=reset||!slot||slot->phase==bonus_session0351::Phase::Completed;
 static std::atomic_uint64_t nonce{0};const auto seed=fresh?(uint64_t(std::chrono::steady_clock::now().time_since_epoch().count())^(++nonce<<8)^bonus):slot->seed;
 std::lock_guard guard(profile_mutex);requested_launch=Launch{bonus,seed,fresh};return true;
}
std::optional<Launch> pending_launch(){std::lock_guard guard(profile_mutex);return requested_launch;}
bool launch_requested(){return pending_launch().has_value();}
bool preview_requested(){return private_preview;}
bool prepare_native(std::span<const uint8_t> original,uint8_t* rdram,unsigned owned_mask,std::vector<uint8_t>* prepared_content){
 if(!active())return false;
 {
  std::lock_guard guard(profile_mutex);
  if(requested_launch){configured_bonus=requested_launch->bonus;configured_seed=requested_launch->seed;force_new_attempt=requested_launch->new_attempt;route_preview=complete_preview=true;policy=rt::LaunchPolicy::PrivateAuthoredComplete;}
 }
 workshop::require(private_preview||(public_bonus_ready&&launch_requested()),"Full Bonus routes have not been qualified yet");
 workshop::require(rdram,"Bonus reset requires valid old native memory");
 transport.reset(rdram);retry_adapter.reset(rdram);portal_factory.reset();exit_policy.reset();finale_bridge.reset();retiring_exit.reset();team.reset();retrying=false;spawned_portals=0;
 interrupted=false;completed=false;game_over=false;pending_completion.reset();ready=false;admitted=false;pending_admission.reset();room=177;
 const auto route=bonus_routes0351::make_route(configured_bonus,configured_seed);const auto& first=route.steps.front().key;
 workshop::require(first.course&&first.root&&first.room==*first.root,"Bonus first room needs its genuine native source root");
 std::optional<bonus_finale_entries0353::EntrySpec> finale_entry;
 if(complete_preview){const auto last=unsigned(route.steps.size()-2);finale_entry=bonus_finale_entries0353::prepare(original,route,last,route.steps[last].key);}
 ++attempt_epoch;tx::Environment env;env.pc_mode=7;env.bonus=configured_bonus;env.seed=configured_seed;env.attempt_epoch=attempt_epoch;env.native_course=*first.course;env.canonical=env.custom_active=env.deferred=false;
 if(route_preview){
  workshop::require(prepared_content,"Authored Bonus preview needs prepared native content");
  auto content=bonus_authoring0353::clone_preloads(original,*prepared_content,route);
  const bool transport_ok=finale_entry?transport.configure_authored_finale(original,route,*finale_entry,env):transport.configure_authored(original,route,env);
  workshop::require(transport_ok&&portal_factory.configure(original,route,env)&&exit_policy.configure(original,route,env),"Authored Bonus transport/factory refused");
  workshop::require(finale_bridge.configure(original,route,{7,configured_bonus,configured_seed,attempt_epoch,true,false,false,false},bonus_fish0354::colors(route.seed)),"Bonus finale preparation refused");
  *prepared_content=finale_bridge.prepare_content(std::move(content.rom));
  workshop::require(service.enable_private_authored(route,true),"Authored Bonus attestation refused");
  if(finale_entry)workshop::require(service.enable_private_finale(original,route,*finale_entry,attempt_epoch,true),"Bonus finale admission attestation refused");
 }else{
  const std::array<tx::Link,2> links{{{0,1,119},{1,0,117}}};
  workshop::require(configured_bonus==1&&configured_seed==24&&first.room==43&&route.steps[1].key.room==85,"Private bridge route changed");
  workshop::require(transport.configure(original,route,links,env),"Private Bonus transport refused");
 }
 const auto snapshot=service.session();const auto& slot=snapshot.attempts[configured_bonus-1];rt::Admission selected;
 if(!slot||force_new_attempt||(complete_preview&&slot->phase==bonus_session0351::Phase::Completed))selected=service.new_attempt(configured_bonus,configured_seed);
 else{workshop::require(slot->seed==configured_seed,"Bonus route differs; preserve its journal");selected=service.resume(configured_bonus);}
 workshop::require(selected.status==rt::Status::Accepted,"Bonus page requirements or journal unavailable");
 auto admission=service.prepare(policy);workshop::require(admission.ticket&&admission.status==rt::Status::Ready,"Bonus native admission unavailable");if(complete_preview&&admission.ticket->ordinal!=0){
  admission=service.retry(*admission.ticket,0,policy);
  workshop::require(admission.ticket&&admission.status==rt::Status::Ready,"Cold Bonus continuation needs its visited first root");
 }
 const auto& ticket=*admission.ticket;
 workshop::require(ticket.ordinal==0&&ticket.room==first.room&&ticket.root==*first.root&&ticket.course==*first.course,"Cold Bonus admission needs the native first root");
 workshop::require(team.start(owned_mask),"Bonus ownership projection invalid");admitted=true;
 workshop::require(retry_adapter.configure(original,route,retry_environment())&&retry_adapter.bind_room(ticket,service.session(),retry_environment()),"Bonus retry configuration refused");
 if(finale_entry)workshop::require(retry_adapter.attest_finale(original,*finale_entry,retry_environment()),"Bonus finale retry attestation refused");
 {std::lock_guard guard(profile_mutex);requested_launch.reset();}force_new_attempt=false;
 fprintf(stderr,"[Bonus0353] admitted bonus=%u seed=%llu ordinaryLinks=%u independentEEP=1 fullRoutePlayable=0\n",configured_bonus.load(),(unsigned long long)configured_seed,unsigned(transport.links().size()));return true;
}
unsigned team_available(uint8_t* rdram){if(!active()||!admitted)return 0;team.project(rdram);return team.available();}
unsigned team_owned(){return active()&&admitted?team.ownership():0;}
void native_course_access(uint8_t* rdram){
 // Isolated Bonus RAM only. Genuine page eligibility is read separately from
 // the canonical files; the vanilla course picker must reach the chosen root.
 if(active()&&admitted&&rdram)MEM_W(0,int32_t(0x800F8E5C))=0xFFFFFF;
}
int team_death(uint8_t* rdram,recomp_context* ctx){return active()&&admitted&&team.death(rdram,ctx);}
std::u8string save_stem(){
 const auto ticket=service.current_ticket();
 workshop::require(active()&&admitted&&!interrupted&&ticket&&ticket->save_stem==rt::bonus_save,"Bonus save admission missing");
 return ticket->save_stem;
}
void commit_completion(){
 if(!active()||!admitted||!complete_preview||completed)return;
 if(!pending_completion)pending_completion=finale_bridge.take_completion();
 if(!pending_completion)return;
 const auto ticket=service.current_ticket();const auto proof=*pending_completion;
 if(!ticket||proof.attempt_epoch!=attempt_epoch){fail("finale-completion-attempt-mismatch");return;}
 const rt::FinaleReceipt receipt{proof.bonus,proof.room,proof.course,proof.defeated_count,proof.seed,proof.attempt_epoch,proof.ticket_epoch,proof.arena_generation};
 const auto result=service.complete_finale(*ticket,receipt);
 if(result.status==rt::Status::Completed){completed=true;ready=false;pending_completion.reset();fprintf(stderr,"[Bonus0353] completed bonus=%u seed=%llu independentJournal=1 canonicalAwards=0\n",proof.bonus,(unsigned long long)proof.seed);book::request_open();book::pause();return;}
 if(result.status==rt::Status::PersistenceFailed||result.status==rt::Status::PersistenceUnavailable){book::request_open();book::pause();return;}
 fail("finale-completion-receipt-refused");
}
void frame_context(unsigned vi,const int32_t* overlays,size_t count){native_vi=vi;native_overlays=overlays;native_overlay_count=overlays?count:0;}
void wrapped_entry(uint8_t* r,const recomp_context* c,const char* n){if(active()&&admitted&&route_preview&&!completed&&!interrupted)finale_bridge.wrapped_entry(r,c,n,finale_frame());}
void wrapped_leave(uint8_t* r,const recomp_context* c,const char* n){if(active()&&admitted&&route_preview&&!completed&&!interrupted)finale_bridge.wrapped_leave(r,c,n,finale_frame());}
bool shared_completion(uint8_t* r,const recomp_context* c){if(!active()||!admitted||!route_preview||interrupted)return false;if(completed)return true;const bool handled=finale_bridge.shared_completion(r,c,finale_frame());commit_completion();return handled;}
void bowser_arm(uint8_t* r,const recomp_context* c){if(active()&&admitted&&route_preview&&!completed&&!interrupted)finale_bridge.bowser_arm(r,c,finale_frame());}
void bowser_commit(uint8_t* r,const recomp_context* c){if(active()&&admitted&&route_preview&&!completed&&!interrupted){finale_bridge.bowser_commit(r,c,finale_frame());commit_completion();}}
bool bowser_due(uint8_t* r,const recomp_context* c){return active()&&admitted&&route_preview&&!completed&&!interrupted&&finale_bridge.bowser_midpoint_due(r,c,finale_frame());}
bool bowser_keep(uint8_t* r,const recomp_context* c){return active()&&admitted&&route_preview&&!completed&&!interrupted&&finale_bridge.bowser_preserve_midpoint(r,c,finale_frame());}
void navigation(uint8_t* rdram,unsigned frame,uint16_t* buttons,float* x,float* y){
 if(!active()||!admitted||interrupted||!rdram)return;
 native_course_access(rdram);native_vi=frame;prefer_backward=buttons&&(*buttons&0x0020u);
 const auto ticket=service.current_ticket();if(!ticket)return;
 const auto snapshot=book::snapshot();
 if(snapshot.stage==0&&(snapshot.scene==5||snapshot.scene==41)){
  if(ready&&!retrying){fail("unexpected-native-selection-return");return;}
  set_course(rdram,ticket->course);
  if(snapshot.scene==41&&frame%120<8){*buttons|=0x8000;*x=*y=0;}
 }
}
int departing(uint8_t* rdram,recomp_context* ctx){
 if(!active())return 0;
 if(!admitted||interrupted){fail("departure-without-admission");return 1;}
 transport.departing(rdram,ctx,environment(rdram));
 const auto move=transport.pending_ticket(environment(rdram));
 const auto current=service.current_ticket();
 if(!move||!current||!ready){fail("departure-not-an-authored-bridge");return 1;}
 const auto observation=observed(rdram,*current);
 const auto next=move->to_ordinal<current->ordinal?
  service.backtrack(*current,observation,move->to_ordinal,policy):
  service.depart(*current,observation,true,policy);
 if(!next.ticket||next.status!=rt::Status::Ready||next.ticket->ordinal!=move->to_ordinal||
 next.ticket->room!=move->destination_room||next.ticket->course!=move->destination_course){
  fail("departure-journal-fence");return 1;
 }
 pending_admission=next.ticket;ready=false;return 0;
}
bool paint_begin(uint8_t* r,uint32_t actor,bool outer){return active()&&admitted&&!interrupted&&!completed&&finale_bridge.paint_begin(r,actor,outer,finale_frame());}
void paint_finish(uint8_t* r){finale_bridge.paint_finish(r);}
int candy_pool_enter(uint8_t* r,recomp_context* c){
 const auto ticket=service.current_ticket();const auto overlay=native_overlays&&native_overlay_count>168?uint32_t(native_overlays[168]):0;
 const boss_candy_pool0260::Frame frame{active()&&admitted&&!interrupted&&!completed&&ticket&&ticket->bonus==3&&finale_ticket(*ticket),162,room,0,2,overlay};
 const auto choice=bonus_candy_pool0354.enter(r,c,frame);
 if(choice==boss_candy_pool0260::Choice::Refused)fail("bonus_candy_pool_live_or_unknown");return int(choice);
}
void candy_pool_leave(uint8_t* r,const recomp_context* c,int ticket){if(ticket==1&&!bonus_candy_pool0354.leave(r,c))fail("bonus_candy_pool_native_initialization_rejected");}
void candy_pool_cleanup(uint8_t* r,const recomp_context* entry,const recomp_context* result){bonus_candy_pool0354.cleanup_return(r,entry,result);}
bool observes_trace(const char* name){
 if(!name)return false;
 constexpr std::array<std::string_view,33> names{
  "func_800483B8","func_80048F84","func_80048E30","func_8006E0FC","func_8006DA0C","func_80045CAC","func_800439F4","func_80045B64","func_8006DD34","func_800675F0","func_805A5628","workshop_807F1100","func_8006D7E8","func_805AB914",
  "func_80664E18","func_806657A0","func_80666ACC","func_80665C80","func_8042D33C","func_8042C428","func_8042C3A0","func_8042D090","func_8055863C","func_80558794","func_80559458","func_80558EA0","func_80557D44","func_8055484C","func_80554928","func_805C9D10","func_806F3A1C","func_80792064","func_8079BD84"};
 return std::find(names.begin(),names.end(),std::string_view(name))!=names.end();
}
void trace(uint8_t* rdram,recomp_context* ctx,const char* name){
 if(!ctx||!active()||!observes_trace(name)||!admitted||completed||interrupted||!rdram)return;
 if(dispatching){
  const std::string_view nested(name);
  if(nested=="func_80045CAC"){transport.created(ctx,room,environment(rdram));retry_adapter.created(ctx,retry_environment());if(route_preview){portal_factory.created(ctx,room,environment(rdram));exit_policy.created(ctx,room,environment(rdram));finale_bridge.trace(rdram,ctx,name,finale_frame());}}
  if(nested=="func_800439F4"||nested=="func_80045B64"){transport.destroy(rdram,uint32_t(ctx->r4),environment(rdram));retry_adapter.forget(uint32_t(ctx->r4),retry_environment());if(route_preview){portal_factory.destroyed(uint32_t(ctx->r4),environment(rdram));if(!retiring_exit||retiring_exit->actor!=uint32_t(ctx->r4))exit_policy.destroyed(uint32_t(ctx->r4),environment(rdram));finale_bridge.trace(rdram,ctx,name,finale_frame());}}
  return;
 }
 struct Recursion {Recursion(){dispatching=true;}~Recursion(){dispatching=false;}} recursion;
 const std::string_view fn(name);
 if(route_preview&&fn!="func_80045CAC"&&fn!="func_800439F4"&&fn!="func_80045B64"&&fn!="func_8006DA0C")finale_bridge.trace(rdram,ctx,name,finale_frame());
 if(pending_completion){commit_completion();if(completed||pending_completion)return;}
 if(fn=="workshop_807F1100"){
  if(!retry_adapter.death_begin(rdram,ctx,retry_environment())){fail("native-death-evidence-refused");return;}
 }
 if(fn=="func_8006E0FC"&&uint32_t(ctx->r31)==0x807F1198u&&ctx->r5==22){
  const auto death=retry_adapter.death_scene(rdram,ctx,retry_environment());
  if(!death){fail("native-death-scene-refused");return;}
  if(!death->post_death_team){game_over=true;fail("native-bonus-game-over");return;}
  const auto next=service.retry(death->previous,death->target_ordinal,policy);
  if(!retry_adapter.bind_admission(next,retry_environment())){fail("native-retry-admission-refused");return;}
  if(!transport.retry_reset(rdram,environment(rdram))){fail("native-retry-transport-reset");return;}
  ready=false;retrying=true;pending_admission.reset();
  set_course(rdram,next.ticket->course);
 }
 if(fn=="func_8006D7E8"&&retrying){
  const auto ticket=service.current_ticket();if(!ticket){fail("native-retry-ticket-missing");return;}
  set_course(rdram,ticket->course);
  if(!retry_adapter.before_room_selection(rdram,retry_environment())){fail("native-checkpoint-install-refused");return;}
 }
 if(fn=="func_805A5628")retry_adapter.wake_begin(rdram,ctx,retry_environment());
 if(fn=="func_8006E0FC"&&uint32_t(ctx->r5)==7){
  const auto ticket=service.current_ticket();
  if(!ticket||uint32_t(ctx->r6)!=ticket->room){fail("unexpected-native-room");return;}
  set_course(rdram,ticket->course);
 }
 if(fn=="func_8006DA0C"){
  const auto ticket=service.current_ticket();const unsigned next=unsigned(ctx->r5)&255u;
  if(!ticket||ticket->room!=next){fail("room-binding-fence");return;}
  set_course(rdram,ticket->course);room=next;spawned_portals=0;
  if(route_preview){
   ++room_epoch;
   if(finale_ticket(*ticket)){
    if(!finale_bridge.enter_room(next,ticket->course,ticket->epoch)){fail("native-finale-room-generation-refused");return;}
   }else if(!portal_factory.enter_room(ticket->ordinal,room_epoch,environment(rdram))||!exit_policy.enter_room(ticket->ordinal,room_epoch,environment(rdram))){fail("authored-factory-room-generation-refused");return;}
  }
  if(!retry_adapter.bind_room(*ticket,service.session(),retry_environment())){fail("native-retry-room-binding-refused");return;}
  transport.enter_room(room,environment(rdram));
 }
 if(fn=="func_80045CAC"){transport.created(ctx,room,environment(rdram));retry_adapter.created(ctx,retry_environment());if(route_preview){portal_factory.created(ctx,room,environment(rdram));exit_policy.created(ctx,room,environment(rdram));finale_bridge.trace(rdram,ctx,name,finale_frame());}}
 if(fn=="func_805AB914")transport.prepare(rdram,uint32_t(ctx->r4),environment(rdram),uint32_t(ctx->r5)==1&&uint32_t(ctx->r6)==3);
 if(fn=="func_800439F4"||fn=="func_80045B64"){transport.destroy(rdram,uint32_t(ctx->r4),environment(rdram));retry_adapter.forget(uint32_t(ctx->r4),retry_environment());if(route_preview){portal_factory.destroyed(uint32_t(ctx->r4),environment(rdram));if(!retiring_exit||retiring_exit->actor!=uint32_t(ctx->r4))exit_policy.destroyed(uint32_t(ctx->r4),environment(rdram));finale_bridge.trace(rdram,ctx,name,finale_frame());}}
 if(fn=="func_8006DD34"){
  transport.room_ready(rdram,ctx,environment(rdram));
  if(route_preview&&service.current_ticket()&&!finale_ticket(*service.current_ticket())&&(!portal_factory.room_ready(rdram,uint32_t(ctx->r4),environment(rdram))||!exit_policy.room_ready(rdram,uint32_t(ctx->r4),environment(rdram)))){fail("authored-factory-room-ready-refused");return;}
 }
 if(fn=="func_800675F0"&&uint32_t(ctx->r4)==0x800F7EA0u){
  retry_adapter.wake_finish(rdram,retry_environment());
  if(retrying){
   if(const auto receipt=retry_adapter.arrival(rdram,room,retry_environment())){
    if(!service.current_ticket()||receipt->admission!=*service.current_ticket()){fail("native-retry-arrival-ticket");return;}
    retrying=false;
   }
   if(retrying)return;
  }
  transport.frame(rdram,ctx,environment(rdram));
  if(const auto receipt=transport.take_arrival(environment(rdram))){
   const auto ticket=service.current_ticket();
   if(!pending_admission||!ticket||*ticket!=*pending_admission||
      receipt->ticket.destination_room!=ticket->room||receipt->ticket.to_ordinal!=ticket->ordinal){
    fail("arrival-journal-fence");return;
   }
   pending_admission.reset();
  }
  if(transport.view().phase==tx::Phase::Rejected){fail("native-transport-rejected");return;}
  if(retry_adapter.view().phase==bonus_retry0353::Phase::Rejected){fail("native-retry-rejected");return;}
  accept_ready(rdram);
  if(route_preview&&ready){
   const auto ticket=service.current_ticket();if(!ticket)return;
   if(finale_ticket(*ticket)){
    finale_bridge.frame(rdram,ctx,finale_frame(),[](uint32_t function,uint8_t* ram,recomp_context* context){workshop::require(function==0x800447C8,"Unowned Bonus finale native invoke");func_800447C8(ram,context);});
    if(finale_bridge.view().failed){fail("native-finale-evidence-refused");return;}
    commit_completion();return;
   }
   for(const auto& action:exit_policy.pending(rdram,environment(rdram))){
    if(!exit_policy.validate(action,rdram,environment(rdram)))continue;
    const auto saved=*ctx;retiring_exit=action;
    try{ctx->r4=gpr(int32_t(action.actor));func_800447C8(rdram,ctx);}catch(...){*ctx=saved;retiring_exit.reset();fail("native-exit-retire-exception");return;}
    *ctx=saved;
    if(!exit_policy.acknowledge(action,rdram,environment(rdram))){retiring_exit.reset();fail("native-exit-retire-receipt-refused");return;}
    retiring_exit.reset();
   }
   portal_factory.preload(rdram,ctx,environment(rdram));
   for(unsigned direction=0;direction<2;++direction){
    const bool back=direction==0;
    const bool wanted=std::any_of(transport.links().begin(),transport.links().end(),[&](const auto& link){return link.ordinal.from_ordinal==ticket->ordinal&&(back?link.ordinal.to_ordinal<ticket->ordinal:link.ordinal.to_ordinal>ticket->ordinal);});
    if(!wanted)continue;
    auto choice=back?bonus_factory0353::Direction::Backward:bonus_factory0353::Direction::Forward;
    const auto* site=portal_factory.site(choice);
    if(!site&&back){choice=bonus_factory0353::Direction::Forward;site=portal_factory.site(choice);}
    if(!site)continue;
    const unsigned portal_slot=choice==bonus_factory0353::Direction::Backward?0u:1u;
    if((spawned_portals&(1u<<portal_slot))&&site->needs_factory&&!portal_factory.receipt(choice,environment(rdram)))spawned_portals&=~(1u<<portal_slot);
    if(spawned_portals&(1u<<portal_slot))continue;
    const auto result=portal_factory.spawn(choice,rdram,ctx,environment(rdram));
    if(result.status==bonus_factory0353::Status::ExistingNativeSource){spawned_portals|=1u<<portal_slot;continue;}
    if(result.status==bonus_factory0353::Status::Failed){fail("authored-native-pot-factory-failed");return;}
    if(result.status!=bonus_factory0353::Status::Created||!result.receipt)continue;
    const auto& born=*result.receipt;
    if(!transport.bind_factory(rdram,{born.room,born.actor,born.packed,born.point,born.private_data,born.native_record},environment(rdram))){fail("authored-pot-factory-witness-refused");return;}
    transport.prepare(rdram,born.actor,environment(rdram),false);spawned_portals|=1u<<portal_slot;
   }
  }
 }
}
} // namespace workshop::bonus_game0352
extern "C" int workshop_bonus_active0352(){return workshop::bonus_game0352::active();}
extern "C" void workshop_bonus_fruit_frame0352(uint8_t* rdram){
 if(!workshop::bonus_game0352::active()||!rdram)return;
 const auto count=uint32_t(MEM_W(0,int32_t(0x800F89D0)));
 MEM_W(0,int32_t(0x800F89D0))=workshop::bonus_goal0352::before_collect(count,true);
}
extern "C" int workshop_bonus_depart0352(uint8_t* rdram,recomp_context* ctx){return workshop::bonus_game0352::departing(rdram,ctx);}

extern "C" int workshop_bonus_bowser_midpoint0353(uint8_t* r,recomp_context* c){return workshop::bonus_game0352::bowser_due(r,c);}
extern "C" int workshop_bonus_bowser_preserve0353(uint8_t* r,recomp_context* c){return workshop::bonus_game0352::bowser_keep(r,c);}
