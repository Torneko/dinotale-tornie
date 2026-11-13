#include "story_hard_bridge0330.hpp"
#include "story_hard0330.hpp"
#include "story_hard_native0330.hpp"
#include "story_hard_eggs0330.hpp"
#include "story_hard_rooms0330.hpp"
#include "story_rights0320.hpp"
#include "hybrid_finds.hpp"
#include "metal_yoshis0300.hpp"
#include "elemental_yoshis0355.hpp"
#include "elemental_hard_recovery0355.hpp"
#include <algorithm>
#include <bit>
#include <cstdio>
#include <cstdarg>
#include <filesystem>
#include <iomanip>
#include <mutex>
#include <random>
#include <sstream>
extern "C" void func_8003BB94(uint8_t*,recomp_context*);
extern "C" void func_800653D8(uint8_t*,recomp_context*);
extern "C" void func_80067E68(uint8_t*,recomp_context*);
extern "C" void func_80067818(uint8_t*,recomp_context*);
// Exact public declaration from librecomp/game.hpp; this module uses no runtime
// operation other than locating the existing separate EEPROM directory.
namespace recomp {std::filesystem::path get_config_path();}
namespace workshop::story_hard_bridge0330 {
namespace {
namespace journal=story_hard0330;namespace native=story_hard_native0330;namespace eggs=story_hard_eggs0330;
struct PendingEgg {unsigned level=24,color=18;};
struct State {
 std::recursive_mutex mutex;Config config{};bool configured=false,prepared=false,ready=false,tally_seen=false,mutation_blocked=false;
 unsigned entered_level=24,entered_room=177,qa_logs=0;int spawning_color=-1;std::string problem,token;story_hard_access0330::Evidence evidence{};story_rights0320::View disk_eeprom;
 native::Adapter adapter;eggs::Registry registry;std::vector<eggs::Placement> placements;
 std::optional<PendingEgg> pending_egg;std::optional<unsigned> pending_world,pending_bowser;
};
State& state(){static State s;return s;}
#define GUARD auto& s=state();std::lock_guard lock(s.mutex)
static gpr at(uint32_t p){return gpr(int32_t(p));}
static uint32_t word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,at(p)));}
static bool range(uint32_t p,unsigned n){return !(p&3)&&p>=0x80000400&&p<0x80800000&&n<=0x80800000-p;}
static unsigned level(uint8_t* rdram){if(!rdram)return 24;const unsigned w=MEM_BU(0,at(0x800F8DFC)),c=MEM_BU(0,at(0x800F8DFD));return w<6&&c<4?w*4+c:24;}
static void failure(State& s,const std::string& why){s.problem=why.empty()?"story-hard-operation-failed":why;fprintf(stderr,"[StoryHard0330] refused reason=%s\n",s.problem.c_str());fflush(stderr);}
static void trace(State& s,const char* format,...){if(s.qa_logs>=96)return;++s.qa_logs;fprintf(stderr,"[StoryHard0330] ");va_list args;va_start(args,format);vfprintf(stderr,format,args);va_end(args);fputc('\n',stderr);fflush(stderr);}
static bool accepted(State& s,bool value){if(!value){s.mutation_blocked=true;const auto v=journal::view();failure(s,v.reason.empty()?"story-hard-journal-refused":v.reason);}return value;}
static native::Team team(const State& s){const auto v=journal::view();if(s.ready){if(v.readable&&v.run.token==s.token&&native::valid({v.run.found,v.run.alive}))return {v.run.found,v.run.alive};return {1,0};}return {1,1};}
static void reset_attempt(State& s){s.ready=false;s.tally_seen=false;s.mutation_blocked=false;s.entered_level=24;s.entered_room=177;s.spawning_color=-1;s.token.clear();s.adapter.reset();s.registry.configure(s.placements);s.pending_egg.reset();s.pending_world.reset();s.pending_bowser.reset();}
static std::string fresh_token(){std::random_device random;std::ostringstream out;out<<std::hex<<std::setfill('0');for(unsigned i=0;i<4;++i)out<<std::setw(8)<<uint32_t(random());return out.str();}
static bool live(State& s,uint8_t* rdram){return s.prepared&&s.ready&&!s.mutation_blocked&&s.adapter.enabled(rdram);}
static void start_adapter(State& s){const auto v=journal::view();s.adapter.start(6,s.config.genuine_events||(s.config.preview&&s.config.preview_verified),v.run.serial?v.run.serial:1);}
static bool reconcile_ownership(State& s){
 if(s.config.preview)return true;
 const bool ok=elemental_hard_recovery0355::reconcile(journal::view(),s.evidence,s.config.genuine_events,
  [](unsigned color,unsigned course){return elemental_yoshis0355::acquire(color,course,true);});
 if(!ok){failure(s,"elemental-durable-recovery-refused");return false;}return true;
}
static bool flush_egg(State& s,uint8_t* rdram){
 if(!s.pending_egg)return true;const auto egg=*s.pending_egg;
 if(!accepted(s,journal::found_egg(egg.level,egg.color,s.token,true)))return false;
 if(egg.color>=16&&!s.config.preview&&!elemental_yoshis0355::acquire(egg.color,egg.level,true)){failure(s,"elemental-ownership-commit-failed");return false;}
 s.registry.committed();s.pending_egg.reset();s.adapter.project(rdram,team(s));
 fprintf(stderr,"[StoryHard0330] egg-committed level=%u color=%u sharedOwnership=%u\n",egg.level,egg.color,unsigned(egg.color>=16&&!s.config.preview));fflush(stderr);return true;
}
static bool flush_world(State& s,uint8_t* rdram){
 if(!flush_egg(s,rdram))return false;
 if(s.pending_world){if(!accepted(s,journal::world_finished(*s.pending_world,s.token,true)))return false;s.pending_world.reset();}
 if(s.pending_bowser&&s.tally_seen){if(!accepted(s,journal::world_finished(*s.pending_bowser,s.token,true)))return false;if(!accepted(s,journal::complete_run(s.token,journal::CompletionPolicy::SurvivingRecruits,true)))return false;s.pending_bowser.reset();}
 return true;
}
static void arm_pending_egg(State& s,uint8_t* rdram){s.registry.frame(rdram);if(const auto carry=s.registry.carried())s.pending_egg=PendingEgg{carry->level,carry->color};}
}
void configure(Config c){GUARD;s.config=c;s.configured=true;s.prepared=false;s.qa_logs=0;s.problem.clear();s.evidence={};s.disk_eeprom={};s.placements.clear();reset_attempt(s);journal::initialize(s.config.preview?journal::preview_filename:journal::production_filename,s.config.preview?journal::Scope::Preview:journal::Scope::Production);journal::deactivate();}
bool active(){GUARD;return s.prepared;}bool preview(){GUARD;return s.config.preview;}std::string error(){GUARD;return s.problem;}story_hard_access0330::Evidence access(){GUARD;if(!s.prepared){const auto canonical=story_rights0320::read(recomp::get_config_path()/"saves"/"yoshisstory.n64.us.bin");s.evidence=story_hard_access0330::inspect(canonical,finds::view(),metal_yoshis0300::view());}return s.evidence;}
std::u8string save_name(){GUARD;return s.config.preview?u8"yoshisstory.n64.us.story-hard0330-preview":u8"yoshisstory.n64.us.story-hard0330";}
bool prepare_entry(){GUARD;s.prepared=false;reset_attempt(s);s.problem.clear();
 if(!s.configured||(!s.config.genuine_events&&!(s.config.preview&&s.config.preview_verified))){failure(s,"story-hard-native-scope-untrusted");return false;}
 const auto canonical=story_rights0320::read(recomp::get_config_path()/"saves"/"yoshisstory.n64.us.bin");
 s.evidence=story_hard_access0330::inspect(canonical,finds::view(),metal_yoshis0300::view());
 if(s.config.preview){const auto owned=story_hard_access0330::owned(canonical,finds::view(),metal_yoshis0300::view());if(!s.config.preview_verified||!owned){failure(s,"story-hard-preview-not-verified");return false;}s.evidence.owned16=*owned;s.evidence.readable=s.evidence.saved=true;s.evidence.reason="private-preview-owned-pool-only";
  if(s.config.test_egg_color!=-1){if(s.config.test_egg_color<1||s.config.test_egg_color>15||!(*owned&(1u<<unsigned(s.config.test_egg_color)))){failure(s,"story-hard-preview-color-unowned");return false;}s.evidence.owned16=1u|(1u<<unsigned(s.config.test_egg_color));}
 }else if(s.config.test_egg_color!=-1){failure(s,"story-hard-test-color-requires-preview");return false;}
 journal::initialize(s.config.preview?journal::preview_filename:journal::production_filename,s.config.preview?journal::Scope::Preview:journal::Scope::Production);
 if(!accepted(s,journal::prepare(6,s.evidence,s.config.preview&&s.config.preview_verified)))return false;
 if(!s.config.preview){elemental_yoshis0355::initialize();elemental_yoshis0355::start_session(6,true);if(!reconcile_ownership(s))return false;}
 s.disk_eeprom=story_rights0320::read(recomp::get_config_path()/"saves"/std::filesystem::path(save_name()+u8".bin"));s.prepared=true;
 fprintf(stderr,"[StoryHard0330] prepared preview=%u pool=%04X nativeChoicePending=1 sharedWrites=0\n",unsigned(s.config.preview),s.evidence.owned16);fflush(stderr);return true;
}
std::vector<uint8_t> patch_rom(std::vector<uint8_t> rom){GUARD;if(!s.configured)return rom;auto applied=story_hard_rooms0330::apply(rom);s.placements.clear();for(const auto& p:applied.plans)s.placements.push_back({p.level,p.room,p.packed,p.x,p.y});s.registry.configure(s.placements);return std::move(applied.rom);}
bool init_projection(uint8_t* rdram,recomp_context* ctx){GUARD;if(!s.prepared)return false;start_adapter(s);auto provisional=native::Team{1,1};const auto v=journal::view();const auto bytes=native::read_continuation(rdram);
 if(v.run.phase==journal::Phase::Active&&v.run.checkpoint_bound&&s.disk_eeprom.present&&s.disk_eeprom.readable&&s.disk_eeprom.continuing&&bytes&&*bytes==v.run.checkpoint&&*bytes==s.disk_eeprom.continuation)provisional={v.run.found,v.run.alive};
 return s.adapter.init_projection(rdram,ctx,provisional);
}
bool new_game(uint8_t* rdram,recomp_context* ctx){GUARD;if(!s.prepared||!rdram||!ctx||!reconcile_ownership(s))return false;reset_attempt(s);std::string token;for(unsigned tries=0;tries<8;++tries){token=fresh_token();if(token!=journal::view().run.token)break;}
 if(!accepted(s,journal::new_run(token,true)))return false;s.token=token;s.ready=true;s.problem.clear();start_adapter(s);for(unsigned i=0;i<14;++i)MEM_B(i,at(0x800F9248))=0;const bool projected=s.adapter.project(rdram,team(s));
 fprintf(stderr,"[StoryHard0330] new-run serial=%llu alive=%04X checkpointCleared14=1 preview=%u\n",static_cast<unsigned long long>(journal::view().run.serial),team(s).alive,unsigned(s.config.preview));fflush(stderr);return projected;
}
bool continue_game(uint8_t* rdram,recomp_context*){GUARD;if(!s.prepared||!rdram||!reconcile_ownership(s))return false;reset_attempt(s);const auto v=journal::view();const auto current=native::read_continuation(rdram);
 const bool coherent=s.disk_eeprom.present&&s.disk_eeprom.readable&&s.disk_eeprom.continuing&&native::Adapter::continuing(rdram)&&current&&*current==s.disk_eeprom.continuation&&v.run.checkpoint_bound&&v.run.checkpoint==*current&&unsigned(std::popcount(v.run.worlds))==unsigned((*current)[13]&7);
 if(!coherent){failure(s,"story-hard-continuation-mismatch");return false;}
 if(!accepted(s,journal::resume_run(v.run.token,*current,true)))return false;s.token=v.run.token;s.ready=true;s.problem.clear();start_adapter(s);return s.adapter.project(rdram,team(s));
}
void save_finished(uint8_t* rdram,recomp_context* ctx){GUARD;if(!live(s,rdram))return;const auto current=s.adapter.saved_continuation(rdram,ctx);if(!current)return;if(accepted(s,journal::bind_checkpoint(s.token,*current,true))){s.disk_eeprom.present=s.disk_eeprom.readable=true;s.disk_eeprom.continuation=*current;s.disk_eeprom.continuing=native::Adapter::continuing(rdram);}}
void clear_local(){GUARD;reset_attempt(s);}
void frame(uint8_t* rdram,recomp_context* ctx){GUARD;if(!live(s,rdram))return;s.registry.frame(rdram);s.adapter.frame(rdram,ctx);if(s.pending_egg||s.pending_world||s.pending_bowser)(void)flush_world(s,rdram);}
std::optional<unsigned> available(){GUARD;if(!s.prepared)return {};return team(s).alive;}
unsigned unlocked_extras(){GUARD;return s.prepared?(team(s).found>>8)&1023:0;}
bool saved_extras(uint8_t* rdram,recomp_context* ctx){GUARD;if(!s.prepared||!ctx||!rdram)return false;const auto owned=team(s);ctx->r2=(owned.alive>>8)&1023;return true;}
bool death_enter(uint8_t* rdram,recomp_context* ctx){GUARD;return live(s,rdram)&&s.adapter.loss_enter(rdram,ctx,team(s));}
void death_finish(uint8_t* rdram,recomp_context* ctx){GUARD;if(!live(s,rdram))return;if(auto color=s.adapter.loss_finish(rdram,ctx)){if(accepted(s,journal::lose_yoshi(*color,s.token,true))){s.registry.attempt_reset();s.adapter.project(rdram,team(s));}}}
bool rescue_enter(uint8_t* rdram,recomp_context* ctx){GUARD;return live(s,rdram)&&s.adapter.rescue_enter(rdram,ctx,team(s));}
void rescue_finish(uint8_t* rdram,recomp_context* ctx){GUARD;if(!live(s,rdram))return;if(auto color=s.adapter.rescue_finish(rdram,ctx))if(accepted(s,journal::restore_yoshi(*color,s.token,true)))s.adapter.project(rdram,team(s));}
bool capacity(uint8_t* rdram,recomp_context* ctx){GUARD;if(!live(s,rdram)||!ctx)return false;const auto value=s.adapter.capacity(rdram,team(s));if(!value)return false;ctx->r2=*value;return true;}
bool picker(uint8_t* rdram,recomp_context* ctx){GUARD;if(!live(s,rdram)||!ctx)return false;const auto local=team(s);const unsigned count=std::popcount(native::dead(local));if(!count){ctx->r2=18;return true;}const auto saved=*ctx;func_800653D8(rdram,ctx);const auto random=uint32_t(ctx->r2);*ctx=saved;ctx->r2=native::rescue_pick(local,random%count);return true;}
void arm_world(uint8_t* rdram,recomp_context* ctx){GUARD;if(live(s,rdram)&&s.config.genuine_events)s.adapter.arm_world_save(rdram,ctx);}
void commit_world(uint8_t* rdram,recomp_context* ctx){GUARD;if(!live(s,rdram)||!s.config.genuine_events)return;if(auto world=s.adapter.finish_world_save(rdram,ctx)){const auto witness=s.adapter.witness();s.pending_world=*world*4+witness.column;(void)flush_world(s,rdram);}}
void arm_bowser(uint8_t* rdram,recomp_context* ctx){GUARD;if(live(s,rdram)&&s.config.genuine_events)s.adapter.arm_bowser(rdram,ctx);}
void commit_bowser(uint8_t* rdram,recomp_context* ctx){GUARD;if(!live(s,rdram)||!s.config.genuine_events)return;if(auto world=s.adapter.finish_bowser(rdram,ctx)){const auto witness=s.adapter.witness();s.pending_bowser=*world*4+witness.column;(void)flush_world(s,rdram);}}
bool end_tally(uint8_t* rdram,recomp_context* ctx){GUARD;if(!live(s,rdram)||!ctx||!s.config.genuine_events)return false;const auto witness=s.adapter.witness();if(!witness.armed||witness.world>=6||witness.column>=4||witness.world*4+witness.column!=level(rdram))return false;
 if(witness.world<5&&!witness.finished)return false;arm_pending_egg(s,rdram);s.tally_seen=true;(void)flush_world(s,rdram);s.adapter.project(rdram,team(s));ctx->r5=std::popcount(team(s).alive);return true;
}
void enter(uint8_t* rdram,unsigned room){GUARD;if(!live(s,rdram))return;const unsigned course=level(rdram);if(course!=s.entered_level){s.tally_seen=false;s.entered_level=course;}if(const auto elemental=elemental_yoshis0355::search_color(course)) (void)journal::reserve_elemental(course,*elemental,s.token,true);const auto chosen=journal::egg_for(course,s.token,true);s.entered_room=room;s.registry.enter(course,room,chosen);trace(s,"enter course=%u room=%u color=%d",course,room,chosen?int(*chosen):-1);}
void begin_child(recomp_context* ctx){GUARD;if(s.prepared)s.registry.begin_child(ctx);}
void prepare_release(uint8_t* rdram,uint32_t actor){GUARD;if(s.prepared)s.registry.prepare_release(rdram,actor);}
void created(uint8_t*,recomp_context* ctx){GUARD;if(!s.prepared)return;s.registry.created(ctx);if(ctx){const auto address=uint32_t(ctx->r4);if(s.registry.hidden(address))trace(s,"source actor=%08X packed=%08X x=%08X y=%08X course=%u room=%u",address,uint32_t(ctx->r5),uint32_t(ctx->r6),uint32_t(ctx->r7),s.entered_level,s.entered_room);else if(s.registry.owns_wrapper(address))trace(s,"wrapper actor=%08X course=%u room=%u",address,s.entered_level,s.entered_room);}if(ctx&&s.spawning_color>=0&&uint32_t(ctx->r5)>>16==0x401D)s.registry.spawned(uint32_t(ctx->r4),unsigned(s.spawning_color));}
void restore(uint8_t* rdram,uint32_t actor){GUARD;if(s.prepared)s.registry.restore(rdram,actor);}
void dispose(uint8_t* rdram,uint32_t actor){GUARD;if(s.prepared)s.registry.dispose(rdram,actor);}
void forget(uint32_t actor){GUARD;if(s.prepared)s.registry.forget(actor);}
bool paint(uint8_t* rdram,uint32_t actor){GUARD;return s.prepared&&s.registry.paint(rdram,actor);}
bool egg_bubble_gate(uint8_t*,recomp_context* ctx){GUARD;if(!s.prepared||!ctx)return false;const auto hidden=s.registry.hidden(uint32_t(ctx->r4));if(!hidden)return false;ctx->r2=*hidden;return true;}
bool release(uint8_t* rdram,recomp_context* ctx){GUARD;if(!live(s,rdram)||!ctx)return false;const unsigned wrapper=uint32_t(ctx->r7);if(!s.registry.owns_wrapper(wrapper))return false;const auto color=s.registry.release_color(wrapper);if(!color){ctx->r2=0;return true;}const auto root=word(rdram,0x800C4278);if(!range(root,4)||!range(uint32_t(ctx->r29),0x20)||uint32_t(ctx->r29)<0x80000430){ctx->r2=0;return true;}
 const auto saved=*ctx;ctx->r29=ADD32(ctx->r29,-0x30);ctx->r6=gpr(int32_t(saved.r6));ctx->r7=at(root);MEM_W(0x10,ctx->r29)=0x401D;MEM_W(0x14,ctx->r29)=*color;const int previous=s.spawning_color;s.spawning_color=int(*color);func_8003BB94(rdram,ctx);s.spawning_color=previous;const auto egg=uint32_t(ctx->r2);*ctx=saved;
 const bool valid_egg=range(egg,0x1FC)&&word(rdram,egg)==0x19971110&&MEM_HU(12,at(egg))==0x401D;if(valid_egg)s.registry.spawned(egg,*color);ctx->r2=valid_egg?at(egg):0;trace(s,"release color=%u egg=%08X valid=%u",*color,egg,unsigned(valid_egg));return true;
}
bool egg_state(uint8_t* rdram,recomp_context* ctx){GUARD;if(!s.prepared||!rdram||!ctx||!range(uint32_t(ctx->r29),0x20))return false;const auto actor=uint32_t(ctx->r4),value=unsigned(MEM_BU(0x1F,ctx->r29));if(!s.registry.egg_state(actor,value))return false;if(s.registry.pickup_sound(actor,value)){const auto saved=*ctx;ctx->r4=6;func_80067E68(rdram,ctx);ctx->r5=ctx->r2;ctx->r4=0x251;func_80067818(rdram,ctx);*ctx=saved;}ctx->r2=0x401E;return true;}
#undef GUARD
} // namespace workshop::story_hard_bridge0330
