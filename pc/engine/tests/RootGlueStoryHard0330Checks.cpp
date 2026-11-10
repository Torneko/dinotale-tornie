#include "../story_hard_bridge0330.hpp"
#include "../game_modes0320.hpp"
#include "../story_observer0320.hpp"
static workshop::story_observer0320::NativeObserver story_observer0320;
static unsigned story_pending_worlds0320=0;
#include <atomic>
#include <vector>
#include <bit>
namespace hard_boundary0330 {
inline unsigned armworld=0,commitworld=0,armbowser=0,commitbowser=0;
inline unsigned calls=0,init=0,fresh=0,resume=0,saves=0,loss=0,rescue=0,clear=0,capacity=0,picker=0,available_calls=0,tally=0,death_enter=0,rescue_enter=0,open=0;
inline bool success=true,tally_success=true;inline std::optional<unsigned> alive=0x8101u;
}
namespace workshop::story_hard_bridge0330 {
void arm_world(uint8_t*,recomp_context*){++hard_boundary0330::calls;++hard_boundary0330::armworld;}
void commit_world(uint8_t*,recomp_context*){++hard_boundary0330::calls;++hard_boundary0330::commitworld;}
void arm_bowser(uint8_t*,recomp_context*){++hard_boundary0330::calls;++hard_boundary0330::armbowser;}
void commit_bowser(uint8_t*,recomp_context*){++hard_boundary0330::calls;++hard_boundary0330::commitbowser;}
bool init_projection(uint8_t*,recomp_context*){++hard_boundary0330::calls;++hard_boundary0330::init;return hard_boundary0330::success;}
bool new_game(uint8_t*,recomp_context*){++hard_boundary0330::calls;++hard_boundary0330::fresh;return hard_boundary0330::success;}
bool continue_game(uint8_t*,recomp_context*){++hard_boundary0330::calls;++hard_boundary0330::resume;return hard_boundary0330::success;}
void save_finished(uint8_t*,recomp_context*){++hard_boundary0330::calls;++hard_boundary0330::saves;}
void death_finish(uint8_t*,recomp_context*){++hard_boundary0330::calls;++hard_boundary0330::loss;}
void rescue_finish(uint8_t*,recomp_context*){++hard_boundary0330::calls;++hard_boundary0330::rescue;}
bool capacity(uint8_t*,recomp_context* c){++hard_boundary0330::calls;++hard_boundary0330::capacity;c->r2=123;return hard_boundary0330::success;}
bool picker(uint8_t*,recomp_context* c){++hard_boundary0330::calls;++hard_boundary0330::picker;c->r2=15;return hard_boundary0330::success;}
std::optional<unsigned> available(){++hard_boundary0330::calls;++hard_boundary0330::available_calls;return hard_boundary0330::alive;}
unsigned unlocked_extras(){++hard_boundary0330::calls;return 0xA5;}
void clear_local(){++hard_boundary0330::calls;++hard_boundary0330::clear;}
bool saved_extras(uint8_t*,recomp_context* c){++hard_boundary0330::calls;c->r2=0x81;return true;}
bool death_enter(uint8_t*,recomp_context* c){++hard_boundary0330::calls;++hard_boundary0330::death_enter;c->r10=0x7F;return hard_boundary0330::success;}
bool rescue_enter(uint8_t*,recomp_context*){++hard_boundary0330::calls;++hard_boundary0330::rescue_enter;return hard_boundary0330::success;}
bool end_tally(uint8_t*,recomp_context* c){++hard_boundary0330::calls;++hard_boundary0330::tally;if(hard_boundary0330::tally_success)c->r5=77;return hard_boundary0330::tally_success;}
}
namespace workshop::book {void request_open(){++hard_boundary0330::open;}}
static std::atomic_bool hard_continue_refused0330{false};
static std::vector<unsigned char> payload;
static bool test_tally=false,test_coop_native=false;
namespace workshop::custom_session {bool active();}
namespace workshop::boss_solo0240 {inline bool enabled=false;}
namespace workshop::boss_campaign0250 {inline bool enabled=false;}
namespace workshop::coop_bridge {unsigned available_mask(unsigned v){return v;}}
#define main old_root_glue_main
#include "legacy-root-fixture0330.inc"
#undef main
namespace workshop::custom_session {bool active(){return custom_scope;}}
int main(int argc,char** argv){try {
 int result=old_root_glue_main(argc,argv);if(result)return result;
 using namespace hard_boundary0330;
 const auto legacy_checks=checks;const auto globals=fs::path(argv[1])/"hard-global-control";const auto canonicalFind=globals/"finds.json",canonicalMetal=globals/"metals.json";put(canonicalFind,{{"version",1u},{"foundMask",63u},{"legacyMigrated",true}});put(canonicalMetal,{{"schema",1u},{"foundMask",3u},{"storyAliveMask",2u}});f::initialize(canonicalFind);m::initialize(canonicalMetal);m::start_session(0,true);const auto canonicalFindBefore=bytes(canonicalFind),canonicalMetalBefore=bytes(canonicalMetal);const unsigned noHard=calls;need(noHard==0,"Legacy0..5 reached Hard bridge");
 std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();recomp_context c{};
 for(int mode=0;mode<6;++mode){selected_mode=mode;menu_enabled=true;const auto before=memory;const auto beforectx=c;workshop_hard_init0330(rdram,&c);workshop_hard_new0330(rdram,&c,1);workshop_hard_save0330(rdram,&c);workshop_hard_death0330(rdram,&c);workshop_hard_rescue0330(rdram,&c);need(!workshop_hard_continue0330(rdram,&c)&&!workshop_hard_capacity0330(rdram,&c)&&!workshop_hard_picker0330(rdram,&c),"Old native hook not no-op");workshop_story_world_arm0320(rdram,&c);workshop_story_world_commit0320(rdram,&c);workshop_story_bowser_arm0320(rdram,&c);workshop_story_bowser_commit0320(rdram,&c);need(memory==before&&std::memcmp(&c,&beforectx,sizeof(c))==0&&calls==noHard,"Old native hooks touched RAM/context/Hard");need(workshop_story_event0320()==(mode==5),"Story event scope changed");}
 selected_mode=6;menu_enabled=true;need(story_hard_session0330()&&workshop_story_event0320(),"Hard session guard absent");
 payload.clear();test_tally=false;test_coop_native=false;custom_scope=false;need(!metal_solo_scope(),"Hard opened canonical metal scope");
 for(int mode=0;mode<6;++mode){selected_mode=mode;for(unsigned flags=0;flags<64;++flags){payload.assign(flags&1,0);custom_scope=flags&2;test_tally=flags&4;test_coop_native=flags&8;workshop::boss_solo0240::enabled=flags&16;workshop::boss_campaign0250::enabled=flags&32;need(metal_solo_scope()==(flags==0),"Old canonical scope changed");}}
 selected_mode=6;payload.clear();custom_scope=test_tally=test_coop_native=false;workshop::boss_solo0240::enabled=workshop::boss_campaign0250::enabled=false;
 // All Hard wrappers must dispatch only locally; failedContinue changes only
 // the native UI confirmation and choice, retaining checkpoint/progress data.
 word(rdram,0x800F8E00,1);MEM_B(0,int32_t(0x8010E1E8))=0xF7;const auto beforeContinue=memory;success=false;c.r2=19;need(workshop_hard_continue0330(rdram,&c)==1&&workshop_hard_continue_refused()&&open==1,"RejectedContinue did not retain a NewGame recovery path");auto expectedRam=beforeContinue;auto* original=rdram;rdram=expectedRam.data();MEM_B(0,int32_t(0x8010E1E8))=0xF6;word(rdram,0x800F8E00,2);rdram=original;need(memory==expectedRam&&c.r2==19,"RejectedContinue reset more than native UI fields");workshop_hard_new0330(rdram,&c,1);need(workshop_hard_continue_refused(),"FailedNew cleared error fence");success=true;workshop_hard_new0330(rdram,&c,1);need(!workshop_hard_continue_refused(),"SuccessfulNew did not clear Continue refusal");need(!workshop_hard_continue0330(rdram,&c),"ValidContinue blocked");workshop_hard_init0330(rdram,&c);workshop_hard_save0330(rdram,&c);workshop_hard_death0330(rdram,&c);workshop_hard_rescue0330(rdram,&c);need(workshop_hard_capacity0330(rdram,&c)&&c.r2==123&&workshop_hard_picker0330(rdram,&c)&&c.r2==15,"Native local picker/capacity not dispatched");
 const auto beforeStartup=memory;const auto freshBefore=fresh;MEM_H(0,int32_t(0x800F9254))=1;MEM_B(0,int32_t(0x800F9253))=8;const auto withCp=memory;workshop_hard_new0330(rdram,&c,0);need(fresh==freshBefore&&memory==withCp,"Startup default choice reset an existing continuation");workshop_hard_new0330(rdram,&c,1);need(fresh==freshBefore+1,"ExplicitNew could not replace an existing continuation");MEM_H(0,int32_t(0x800F9254))=0;workshop_hard_new0330(rdram,&c,0);need(fresh==freshBefore+2,"Fresh startup could not establish local run");memory=beforeStartup;
 need(init==1&&fresh==4&&resume==2&&saves==1&&loss==1&&rescue==1,"Hard hook dispatch multiplicity incorrect");
 workshop_story_world_arm0320(rdram,&c);workshop_story_world_commit0320(rdram,&c);workshop_story_bowser_arm0320(rdram,&c);workshop_story_bowser_commit0320(rdram,&c);need(armworld==1&&commitworld==1&&armbowser==1&&commitbowser==1,"Hard world/Bowser wrappers used ordinary observer");
 const auto globalFind=f::view();const auto globalMetal=m::view();const auto tallyBefore=native_tally_calls,customBefore=custom_finish_calls;workshop_find_reset();workshop_find_clear();need(clear==1&&f::view().found==globalFind.found&&m::view().found==globalMetal.found,"Hard clear/reset changed global rights");need(workshop_find_unlocked(rdram)==0xA5,"Hard unavailable selection uses canonical rights");
 for(unsigned color=0;color<18;++color){c.r5=color;c.r2=0xBAD;need(workshop_metal_trial_color(rdram,&c)==1&&c.r2==(color<16?((*alive>>color)&1):0),"Hard picker not local16");}
 c.r2=9;workshop_metal_saved_lives(rdram,&c);need(c.r2==0x81,"Hard saved extras leaked canonical lives");need(workshop_metal_death(rdram,&c)==1&&c.r10==0x7F,"Hard local loss not dispatched");workshop_metal_rescued(rdram,&c);need(rescue_enter==1&&death_enter==1,"Hard rescue/loss dispatch count");
 const auto beforeReconcile=memory;workshop_progress_reconcile(rdram,&c);progress_payload=0x80120000;progress_reading=true;c.r2=0;workshop_progress_boot(rdram,&c);need(memory==beforeReconcile&&progress_payload==0&&!progress_reading,"Hard reconciled canonical progression");
 tally_success=false;c.r5=999;workshop_end_score_lives(rdram,&c);need(c.r5==unsigned(std::popcount(*alive))&&native_tally_calls==tallyBefore&&custom_finish_calls==customBefore,"FailedHard tally fell through to ordinary/canonical scoring");tally_success=true;workshop_end_score_lives(rdram,&c);need(c.r5==77&&native_tally_calls==tallyBefore&&custom_finish_calls==customBefore,"SuccessfulHard tally fell through");alive.reset();tally_success=false;workshop_end_score_lives(rdram,&c);need(c.r5==0&&workshop_coop_filter_available(0xFFFF)==0,"Missing local Hard team exposed canonical actors");
 need(bytes(canonicalFind)==canonicalFindBefore&&bytes(canonicalMetal)==canonicalMetalBefore&&f::view().found==63&&m::view().found==3,"Hard wrapper changed nonempty canonical ownership/lives");
 menu_enabled=false;need(!story_hard_session0330()&&workshop_hard_capacity0330(rdram,&c)==0,"Disabled menu opened Hard");
 for(int mode=-2;mode<9;++mode){using namespace workshop::game_modes0320;need(valid(mode)==(mode>=0&&mode<=6),"Mode validity");need(native_choice(mode)==(mode<0||mode>6?-1:mode==0||mode==5||mode==6?0:mode==2?3:1),"Native route changed");need(solo_playable(mode)==(mode==0||mode==1||mode==2||mode==3||mode==5),"Canonical solo scope widened");}
 std::cout<<Json({{"passed",true},{"checks",checks},{"legacyChecks",legacy_checks},{"legacyModes",Json::array({0,1,2,3,4,5})},{"hardBoundaryCalls",calls},{"hardBoundaryIsStub",true},{"scopeMatrixCases",384},{"game",false},{"gpu",false}}).dump()<<'\n';return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL HardRoot: "<<e.what()<<'\n';return 1;}}
