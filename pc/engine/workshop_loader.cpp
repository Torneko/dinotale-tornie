#include "workshop_loader.hpp"
#include "boss_manual_handoff0260.hpp"
#include "custom_session.hpp"
extern "C" void workshop_request_native_history_test(unsigned);
#include "workshop_menu.hpp"
#include "menu_music_session0310.hpp"
#include "workshop_menu_game.hpp"
#include "achievements.hpp"
#include "collections.hpp"
#include "hybrid_mode.hpp"
#include "game_modes0320.hpp"
#include "bonus_game0352.hpp"
#include "bonus_team0353.hpp"
#include "story_hybrid0320.hpp"
#include "story_rights0320.hpp"
#include "known_yoshis_runtime0355.hpp"
#include "locked_selection_native0355.hpp"
#include "story_observer0320.hpp"
#include "story_hard_bridge0330.hpp"
#include "hybrid_completion0290.hpp"
#include "metal_yoshis0300.hpp"
#include "metal_native0300.hpp"
#include "metal_custom0311.hpp"
#include "boss_metals0354.hpp"
#include "gold_silver_resources0300.hpp"
#include "elemental_resources0355.hpp"
#include "unknown_yoshi_resources0355.hpp"
#include "elemental_yoshis0355.hpp"
#include "elemental_abilities0355.hpp"
#include "elemental_native0355.hpp"
#include "elemental_contact0355.hpp"
#include "elemental_tongue0355.hpp"
#include "elemental_egg_draw0363.hpp"
#include "enemy_egg_provenance0366.hpp"
#include "elemental_flame_food0355.hpp"
#include "elemental_lava0355.hpp"
#include "elemental_selection_capacity0355.hpp"
#include "hybrid_finds.hpp"
#include "hybrid_pickup_sound.hpp"
#include "workshop_data.hpp"
#include "librecomp/game.hpp"
#include "librecomp/addresses.hpp"
#include "librecomp/overlays.hpp"
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <string_view>
#ifdef WORKSHOP_CONTENT_BUILD
#include "SDL.h"
#include "content_config.hpp"
#include "recompinput/profiles.h"
#include <array>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <mutex>
#include <sstream>
#include <bit>
#include <cmath>
#ifdef WORKSHOP_SPECIAL_CONTENT
#include "special_content_rules.hpp"
#include "hybrid_rewards.hpp"
#include "resource_cache.hpp"
#include "fruit_mix.hpp"
#include "fruit_visits.hpp"
#include "fruit_resources.hpp"
#include "special_bubbles.hpp"
#include "custom_editor_objects.hpp"
#include "custom_black_flyguy.hpp"
#include "custom_passages.hpp"
#include "heart_shop0283.hpp"
#include "heart_pickup0283.hpp"
#include "trial_special_lives0350.hpp"
#include "trial_special_native0350.hpp"
#include <map>
#ifdef WORKSHOP_NATIVE_COOP
#include "coop_native_bridge.hpp"
#include "coop_room_route_driver0280.hpp"
#include "coop_room_return0286.hpp"
#include "coop_room_reentry0287.hpp"
#include "coop_secondary_return0288.hpp"
#include "coop_pot_probe0280.hpp"
#include "coop_native_probe.hpp"
#include "coop_native_egg_stress.hpp"
#include "coop_manual_input.hpp"
#include "boss_native.hpp"
#include "starting_bonus0283.hpp"
#include "boss_solo_runtime0240.hpp"
#include "boss_battle_runtime0250.hpp"
#include "boss_blargg_arena0289.hpp"
#include "boss_blargg_boot0289.hpp"
#include "boss_blargg_acquire0289.hpp"
#include "boss_blargg_shoot0289.hpp"
#include "boss_intro0260.hpp"
#include "boss_driver_bridge0260.hpp"
#include "boss_route_checkpoint_bridge0260.hpp"
#include "boss_swallow_retry_test0263.hpp"
#include "boss_fish_cp6_bridge0284.hpp"
#include "boss_followup_input0260.hpp"
#include "boss_ground_roundtrip0260.hpp"
#include "boss_diagnostic_watch0260.hpp"
#include "boss_test_loadout0260.hpp"
#include "boss_fish_probe0260.hpp"
#include "boss_lips_probe0260.hpp"
#include "boss_checkpoint_probe0260.hpp"
#include "boss_challenges0260.hpp"
#include "boss_selection0260.hpp"
#include "coop_health_hud0250.hpp"
#endif
extern "C" void func_806ED088(uint8_t*,recomp_context*);
static void black_controller_no_collision(uint8_t*,recomp_context* c){c->r2=0;}
#include "special_victory_draw.hpp"
#include "story_progress.hpp"
#include "pause_favorites.hpp"
#include "reward_popup.hpp"
#include <random>
#endif
#endif

namespace { std::vector<uint8_t> payload; int find_spawning=-1;bool find_spawning_scene=true; }

#ifdef WORKSHOP_CONTENT_BUILD
namespace {
struct LookupRecord { uint32_t address,ra,sp,a0,a1;const char* caller; };
std::array<LookupRecord,128> lookup_records{};
size_t lookup_count=0;
std::mutex lookup_mutex;
struct TestInput {unsigned first,last;uint16_t buttons;float x,y;};
std::vector<TestInput> test_inputs;
#ifdef WORKSHOP_NATIVE_COOP
std::vector<TestInput> test_inputs_p2;
bool test_coop_room_fixture0280=false;
bool test_coop_room_route_driver0280=false;unsigned test_coop_room_route_leader0280=1;
bool test_coop_room_death0280=false;
bool test_coop_room_return0286=false;
bool test_coop_room_reentry0287=false;
bool test_coop_secondary_return0288=false;
bool test_coop_native=false,test_coop_interactions=false,test_coop_egg_probe=false,test_coop_manual=false,test_coop_egg_stress=false,test_coop_shared_egg_pools=false;
unsigned test_coop_color=5;
std::string test_first_boss,boss_solo_token,boss_battle_token;
uint64_t boss_battle_seed=20261031;
bool boss_test_swallow_checkpoint=false;
bool boss_test_blargg0289=false,boss_test_blargg_acquire0289=false,boss_test_blargg_shoot0289=false;
workshop::boss_blargg_boot0289::Router boss_blargg_boot_router;
workshop::boss_blargg_acquire0289::Controller boss_blargg_acquire;
workshop::boss_blargg_shoot0289::Controller boss_blargg_shoot;
unsigned boss_blargg_request_probe_count=0;bool boss_blargg_request_probe_dumped=false;
unsigned boss_blargg_flame_probe_count=0,boss_blargg_flame_probe_vi=0;
uint32_t boss_blargg_flame_probe_actor=0,boss_blargg_flame_probe_callback=0,boss_blargg_flame_probe_step=0;
static workshop::boss_blargg_shoot0289::Frame boss_blargg_shoot_frame(unsigned vi){
    namespace campaign=workshop::boss_campaign0250;
    const auto& state=campaign::session.snapshot();const auto& ready=campaign::fight_ready;
    const bool fight_ready=ready.vi && ready.index==state.index && ready.room==campaign::physical_room && ready.source==19 && ready.epoch==state.retry_epoch && ready.player==state.player;
    return {boss_test_blargg0289,state.diagnostic,boss_blargg_acquire.snapshot().done,fight_ready,campaign::driver,state.phase,campaign::physical_room,campaign::logical_room,state.index,state.retry_epoch,vi,state.completed,state.player,campaign::exit_pot,state.token,campaign::session.plan().seed,recomp::mem_size};
}
bool boss_test_fish_cp6=false,boss_test_fish_cp6_focused=false;
int boss_test_case=-1;std::string boss_test_goal="route";bool boss_test_menu=false;
bool boss_solo_driver=false,boss_production_menu=false,boss_manual_handoff=false,boss_test_checkpoint=false,boss_test_retry_replay=false,boss_test_full_loadout=false,boss_test_special_fish=false,boss_test_automatic=false,boss_test_route_checkpoints=false,boss_test_lips_loadout=false,boss_test_genie_loadout=false,boss_test_genie_empty_reserves=false,boss_test_genie_refill_logged=false,boss_test_monsters_loadout=false,boss_test_fish_loadout=false,boss_test_final_loadout=false;
workshop::boss_test_loadout0260::Controller boss_loadout;
workshop::boss_route_checkpoint_bridge0260::NativeEggWitness boss_fish_loadout_witness;
static workshop::boss_route_checkpoint_bridge0260::EggCallScope boss_fish_loadout_scope(uint8_t* ram,unsigned vi){
    namespace c=workshop::boss_campaign0250;namespace n=workshop::boss_native;namespace l=workshop::boss_test_loadout0260;
    workshop::boss_route_checkpoint_bridge0260::EggCallScope out;const auto& s=c::session.snapshot();const auto& ready=c::fight_ready;
    if(!boss_test_fish_loadout||!l::mixed_fish_tuple(boss_battle_seed,unsigned(boss_test_case))||!ram||!c::active()||!c::driver||!s.diagnostic||s.phase!=workshop::boss_battle0250::Phase::Fighting||c::session.plan().seed!=boss_battle_seed||s.index!=unsigned(boss_test_case)||c::logical_room!=97||c::physical_room!=167+s.index||!ready.vi||vi<=ready.vi||ready.player!=s.player||ready.index!=s.index||ready.room!=c::physical_room||ready.source!=97||ready.epoch!=s.retry_epoch||!n::actor(ram,s.player,0x4001)||n::word(ram,0x800F8828)!=s.player||n::word(ram,s.player+0x1F4)!=ready.private_data)return out;
    out={true,s.player,ready.private_data,vi,c::physical_room,c::logical_room,s.index,s.retry_epoch,c::session.plan().seed,s.token,true};return out;
}
bool checkpoint_probe_enabled(){const auto& s=workshop::boss_campaign0250::session.snapshot();return boss_test_route_checkpoints&&workshop::boss_campaign0250::active()&&workshop::boss_campaign0250::physical_room==83&&s.completed>=3&&s.completed<=9&&s.completed%3==0;}
std::vector<workshop::boss_followup_input0260::Event> boss_followup_inputs;
workshop::boss_followup_input0260::Anchor boss_followup_anchor;
workshop::boss_ground_roundtrip0260::Walk boss_ground_walk;
std::atomic_bool boss_award_saved=false;
#endif
std::atomic<unsigned> test_vi{0};
uint8_t* diagnostic_rdram=nullptr;
int test_color=-1;
bool background_test=false,trace_startup=false;
bool test_software_gpu0284=false;
bool test_book_resume=false;
bool test_book_shop=false;
bool bonus_preview0352=false,bonus_route0353=false,bonus_complete0353=false,bonus_selection0353=false,bonus_new0353=false;
unsigned bonus_page0353=1;uint64_t bonus_seed0353=24;
bool hard_preview0330=false;int hard_test_egg0330=-1;bool hard_qa_room_routed0330=false;std::atomic_bool hard_continue_refused0330{false};
bool test_find_reveal=false;
bool test_editor_eggs=false,test_editor_picked=false,test_editor_probe_logged=false;
uint32_t test_editor_pool=0,test_editor_flags=0;
unsigned test_editor_release_vi=0,test_editor_pending=0,test_editor_found=0;
uint32_t test_find_source=0,test_find_egg=0;
unsigned test_find_spawn_vi=0;
bool test_find_finish_fruit=false;
bool test_find_finish_committed=false;
uint32_t test_find_finish_actor=0;
bool fruit_audit=false;
bool special_hud=true;
bool test_tally=false;
int test_special_victory=-1;
bool test_hybrid=false;
bool relative_play=false;
unsigned test_player_vi=0;
int test_stage=-1;
unsigned test_native_history=0;
int test_zone=-1;
bool test_spawn=false;float test_spawn_x=0,test_spawn_y=0;
bool test_release_spawn=false;
bool test_warp=false;float test_warp_x=0,test_warp_y=0;unsigned test_warp_at=0,test_warp_hold_until=0;
unsigned end_test_vi=0;
uint32_t fruit_seed=0;
bool hybrid_level_active=false;
#ifdef WORKSHOP_SPECIAL_CONTENT
workshop::fruit_mix::SpawnChoices hybrid_spawn_choices;
workshop::fruit_mix::Visits hybrid_visits;
uint32_t hybrid_room=0;
std::unordered_map<uint32_t,std::array<uint32_t,12>> audited_fruit_draws;
struct FindSource {uint32_t record,kind,x,y;unsigned color;};
std::optional<FindSource> find_binding;
std::unordered_map<uint32_t,unsigned> find_sources;
#endif
#ifdef WORKSHOP_NATIVE_COOP
workshop::starting_bonus0283::Controller starting_bonus;
static workshop::starting_bonus0283::Environment starting_bonus_environment(uint8_t* ram){
    namespace sb=workshop::starting_bonus0283;
    sb::Environment e;e.vi=test_vi.load();
    e.custom=workshop::bonus_game0352::active() || !payload.empty() || workshop::custom_session::active() || test_tally || test_coop_native || workshop::boss_solo0240::enabled || hard_preview0330;
    if(e.custom){e.mode=sb::Mode::Custom;return e;}
    if(workshop::boss_campaign0250::enabled){
        const auto& state=workshop::boss_campaign0250::session.snapshot();
        e.mode=sb::Mode::BossBattle;e.boss_armed=state.phase==workshop::boss_battle0250::Phase::Armed;
        e.boss_index=state.index;e.retry_epoch=state.retry_epoch;return e;
    }
    const int selected=workshop_menu_session_mode();
    if(workshop_menu_enabled()){
        e.mode=workshop::game_modes0320::story(selected)?sb::Mode::Story:selected==1?sb::Mode::Trial:selected==3?sb::Mode::Hybrid:selected==2?sb::Mode::Practice:sb::Mode::None;
    }else if(ram){
        const unsigned native=workshop::boss_native::word(ram,0x800F8B88);
        e.mode=workshop::hybrid_mode::active()?sb::Mode::Hybrid:native==0?sb::Mode::Story:native==1?sb::Mode::Trial:sb::Mode::Practice;
    }
    return e;
}
static void starting_bonus_frame(uint8_t* ram,recomp_context* context){
    const auto environment=starting_bonus_environment(ram);
    starting_bonus.frame(ram,context,environment,workshop::heart_shop0283::wallet());
    const auto& state=starting_bonus.snapshot();
    static unsigned phase=~0u;static std::string token,reason;
    if(phase!=unsigned(state.phase) || token!=state.attempt || reason!=state.reason){
        phase=unsigned(state.phase);token=state.attempt;reason=state.reason;
        fprintf(stderr,"[HeartBonus] phase=%u mode=%u level=%u source=%u scene6=%u player=%08X born=%llu vi=%u life=%u eggs=%u added=%u token=%s reason=%s committed=%u nativeOnly=1\n",
            phase,unsigned(state.mode),state.level,state.source,state.scene6,state.player,(unsigned long long)state.player_birth,
            environment.vi,unsigned(state.requested.life),unsigned(state.requested.eggs),state.added,token.c_str(),reason.c_str(),unsigned(state.phase==workshop::starting_bonus0283::Phase::Complete && !state.requested.empty()));
    }
}
#endif
uint32_t progress_payload=0,progress_boot_story=0;
bool progress_reading=false;
workshop::story_rights0320::View canonical_rights0320;
bool story_lives_ready0320=false;
workshop::story_observer0320::NativeObserver story_observer0320;
unsigned story_pending_worlds0320=0;
static bool story_hybrid_session0320(){return workshop_menu_enabled()&&workshop_menu_session_mode()==workshop::game_modes0320::StoryHybrid;}
static bool story_hard_session0330(){return workshop_menu_enabled()&&workshop_menu_session_mode()==workshop::game_modes0320::StoryHard;}
unsigned auto_cursor=0,auto_next=0,auto_end=0;
float auto_x=0,auto_y=0;
uint16_t auto_buttons=0;
bool ram_pointer(uint32_t a){return a>=0x80000400 && a<0x80800000;}
void select_test_color(unsigned frame,uint16_t* buttons,float* x,float* y){
    workshop::bonus_game0352::navigation(diagnostic_rdram,frame,buttons,x,y);
    if(test_stage>=0 && diagnostic_rdram && frame>=1700 && frame<2400){
        auto* rdram=diagnostic_rdram;
        MEM_B(0,gpr(int32_t(0x800F8DFC)))=test_stage/4;
        MEM_B(0,gpr(int32_t(0x800F8DFD)))=test_stage%4;
    }
#ifdef WORKSHOP_NATIVE_COOP
    if(diagnostic_rdram && (workshop::boss_solo0240::enabled || workshop::boss_campaign0250::enabled)){
        const auto s=workshop::book::snapshot();
        if(s.stage==0 && (s.scene==5 || s.scene==41)){
            auto* rdram=diagnostic_rdram;MEM_B(0,int32_t(0x800F8DFC))=2;MEM_B(0,int32_t(0x800F8DFD))=0;
            if(s.scene==41 && frame%120<8){*buttons|=0x8000;*x=*y=0;}
        }
    }
#endif
    if(diagnostic_rdram && workshop::custom_session::active()){
        const auto snapshot=workshop::book::snapshot();
        if(snapshot.stage==0 && (snapshot.scene==41 || snapshot.scene==5)){
            auto* rdram=diagnostic_rdram;const int stage=workshop::custom_session::source_stage();
            MEM_B(0,gpr(int32_t(0x800F8DFC)))=stage/4;MEM_B(0,gpr(int32_t(0x800F8DFD)))=stage%4;
        }
    }
    const int preferred=workshop::custom_session::preferred_color();
    const int selected_color=test_color>=0?test_color:preferred;
    if(!diagnostic_rdram || !workshop::metal_custom0311::selection_ready(frame,test_color,preferred))return;
    if(test_color<0 && workshop::custom_session::active()){
        const auto context=workshop::book::snapshot();
        if(!workshop::metal_custom0311::selection_context(context.stage,context.scene))return;
        // A user confirmation during the native transition must not pick the
        // default Green before the saved color's cursor is ready. B stays live.
        *buttons=workshop::metal_custom0311::pending_buttons(*buttons);
        if(*buttons&0x4000)return; // Let B cancel before any queued automatic input.
    }
    auto* rdram=diagnostic_rdram;
    auto read=[&](uint32_t a){return uint32_t(MEM_W(0,int32_t(a)));};
    auto is_yoshi=[&](uint32_t a){auto id=read(a+12)>>16;return (id>=0x41F1 && id<=0x41F8)||(id>=0x4EF1 && id<=0x4EFA);};
    auto player=read(0x800F8828);if(ram_pointer(player) && read(player+12)>>16==0x4001)return;
    if(frame<=auto_end){*buttons=auto_buttons;*x=auto_x;*y=auto_y;return;}
    if(frame<auto_next)return;auto_next=frame+90;
    if(auto_cursor && (read(auto_cursor)!=0x19971110 || read(auto_cursor+12)>>16!=0x41FA))auto_cursor=0;
    if(!auto_cursor)for(uint32_t at=0x80000400;at<0x80400000-0x200;at+=4){
        if(read(at)==0x19971110 && read(at+12)>>16==0x41FA){auto parent=read(at+0x180);if(ram_pointer(parent) && is_yoshi(parent)){auto_cursor=at;break;}}
    }
    if(!auto_cursor || read(auto_cursor+24)>>24!=1 || read(auto_cursor+48)>>24!=2)return;
    auto current=read(auto_cursor+0x180);if(!ram_pointer(current)||!is_yoshi(current))return;
    int current_color=read(current+12)&255;auto_buttons=0;auto_x=auto_y=0;
    if(current_color==selected_color)auto_buttons=0x8000;
    else {
        auto background=read(current+0x180);if(!ram_pointer(background))return;
        unsigned count=read(background+0x184)>>16;if(count>26)return;
        std::array<float,18> px{},py{};int size=0,source=-1,target=-1;
        for(unsigned i=0;i<count && size<18;i++){
            auto object=read(background+0x188+i*4);if(!ram_pointer(object)||!is_yoshi(object))continue;
            px[size]=std::bit_cast<float>(read(object+60));py[size]=std::bit_cast<float>(read(object+64));
            if(object==current)source=size;if(int(read(object+12)&255)==selected_color)target=size;size++;
        }
        if(source<0 || target<0){
            if(target<0 && selected_color>=14 && test_color<0 && workshop::custom_session::active())workshop::custom_session::release_preferred_color();
            return;
        }
        std::array<int,18> previous{},first{},queue{};previous.fill(-1);previous[source]=source;first[source]=-1;
        int head=0,tail=0;queue[tail++]=source;
        while(head<tail && previous[target]<0){
            int node=queue[head++];
            for(int direction=0;direction<4;direction++){
                int nearest=-1;float best=400;
                for(int next=0;next<size;next++){
                    float dx=px[next]-px[node],dy=py[next]-py[node],distance=std::abs(dx)+std::abs(dy);
                    bool eligible=direction==0?dy<0:direction==1?dy>0:direction==2?dx<0:dx>0;
                    if(eligible && distance<=best){best=distance;nearest=next;}
                }
                if(nearest>=0 && previous[nearest]<0){previous[nearest]=node;first[nearest]=node==source?direction:first[node];queue[tail++]=nearest;}
            }
        }
        if(previous[target]<0)return;int d=first[target];auto_x=d==2?-1.f:d==3?1.f:0.f;auto_y=d==0?1.f:d==1?-1.f:0.f;
    }
    fprintf(stderr,"[Workshop] Controller selection VI %u color %d target %d stick %.0f,%.0f buttons %04X\n",frame,current_color,selected_color,auto_x,auto_y,auto_buttons);
    auto_end=frame+8;*buttons=auto_buttons;*x=auto_x;*y=auto_y;
}
std::string diagnostic_utf8_path0311(const std::filesystem::path& path){
    const auto bytes=path.u8string();return std::string(bytes.begin(),bytes.end());
}
void configure_diagnostics(){
    const auto stamp=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto directory=std::filesystem::current_path()/"logs";std::filesystem::create_directories(directory);
    const auto prefix=directory/("engine-"+std::to_string(stamp));
#ifdef _WIN32
    // CRT narrow fopen cannot reliably open an absolute Unicode profile path.
    auto error_log=prefix;error_log+=L".stderr.log";
    auto output_log=prefix;output_log+=L".stdout.log";
    _wfreopen(error_log.c_str(),L"w",stderr);
    _wfreopen(output_log.c_str(),L"w",stdout);
#else
    std::freopen((prefix.string()+".stderr.log").c_str(),"w",stderr);
    std::freopen((prefix.string()+".stdout.log").c_str(),"w",stdout);
#endif
    setvbuf(stderr,nullptr,_IONBF,0);setvbuf(stdout,nullptr,_IONBF,0);
    fprintf(stderr,"[Workshop] Profile %s\n",diagnostic_utf8_path0311(std::filesystem::current_path()).c_str());
    std::atexit(workshop_dump_trace);
}
}
extern "C" void workshop_dump_trace(){
    std::lock_guard lock(lookup_mutex);
    fprintf(stderr,"[Workshop] Last indirect calls (%zu total):\n",lookup_count);
    const auto first=lookup_count>lookup_records.size()?lookup_count-lookup_records.size():0;
    for(size_t i=first;i<lookup_count;i++){
        const auto& r=lookup_records[i%lookup_records.size()];
        fprintf(stderr,"  %08X from %s ra=%08X sp=%08X a0=%08X a1=%08X\n",r.address,r.caller,r.ra,r.sp,r.a0,r.a1);
    }
    fflush(stderr);
}
#ifdef WORKSHOP_SPECIAL_CONTENT
static bool audit_fruit_id(uint32_t id){return (id>=0x4030 && id<=0x4034)||(id>=0x4F00 && id<=0x4F05);}
static uint32_t audit_bytes(uint8_t* rdram,uint32_t address,unsigned size){
    if(!workshop::fruit_mix::range(address,size))return 0;
    uint32_t hash=2166136261u;
    for(unsigned i=0;i<size;i++)hash=(hash^MEM_BU(int(i),int32_t(address)))*16777619u;
    return hash;
}
static void audit_fruit_actor(uint8_t* rdram,gpr actor,const char* event){
    if(!ram_pointer(uint32_t(actor)) || uint32_t(MEM_W(0,actor))!=0x19971110)return;
    const uint32_t id=MEM_HU(12,actor);if(!audit_fruit_id(id))return;
    const uint32_t resource=MEM_W(0x168,actor),parent=MEM_W(0x180,actor),frame=MEM_HU(0x144,actor);
    uint32_t kind=0,pixels=0,palette=0,pixelHash=0,paletteHash=0;
    if(workshop::fruit_mix::range(resource,0x90)){
        const auto r=gpr(int32_t(resource));kind=MEM_HU(0x3C,r);
        const uint32_t stride=MEM_HU(0x7C,r),count=MEM_HU(0x64,r),mapping=MEM_W(0x68,r);
        if(frame<count && stride && stride<=4096 && workshop::fruit_mix::range(mapping,(frame+1)*2)){
            const unsigned mapped=MEM_HU(int(frame*2),int32_t(mapping));
            if(mapped<count){pixels=uint32_t(MEM_W(0x80,r))+mapped*stride;pixelHash=audit_bytes(rdram,pixels,stride);}
        }
        palette=MEM_W(0x88,r);paletteHash=audit_bytes(rdram,palette,512);
    }
    const uint32_t parentKind=ram_pointer(parent)?MEM_HU(12,int32_t(parent)):0;
    const uint32_t drawing=MEM_W(0x16C,actor);
    uint32_t drawPalette=0,drawPaletteHash=0,drawTexture=0;
    if(MEM_BU(0x164,actor)<=1 && workshop::fruit_mix::range(drawing,0x44)){
        drawPalette=MEM_W(8,int32_t(drawing));drawPaletteHash=audit_bytes(rdram,drawPalette,512);
        drawTexture=MEM_W(0x30,int32_t(drawing));
    }
    const std::array<uint32_t,12> signature{id,kind,resource,frame,pixels,palette,pixelHash,paletteHash,parent,drawPalette,drawPaletteHash,drawTexture};
    if(std::string_view(event)=="draw"){
        if(MEM_BU(0x165,actor)!=0)return;
        auto found=audited_fruit_draws.find(uint32_t(actor));
        if(found!=audited_fruit_draws.end() && found->second==signature)return;
        audited_fruit_draws[uint32_t(actor)]=signature;
    }
    fprintf(stderr,"[FruitAudit] %s vi=%u actor=%08X id=%04X resourceId=%04X frame=%u pixelPtr=%08X pixelHash=%08X paletteHash=%08X drawPalettePtr=%08X drawPaletteHash=%08X drawTexture=%08X parent=%04X@%08X xy=%.2f,%.2f\n",event,test_vi.load(),uint32_t(actor),id,kind,frame,pixels,pixelHash,paletteHash,drawPalette,drawPaletteHash,drawTexture,parentKind,parent,std::bit_cast<float>(uint32_t(MEM_W(0x84,actor))),std::bit_cast<float>(uint32_t(MEM_W(0x88,actor))));
    fflush(stderr);
}
static void audit_fruit_ring(uint8_t* rdram){
    static std::array<uint32_t,31> previous{};std::array<uint32_t,31> current{};
    current[0]=uint32_t(MEM_W(0,int32_t(0x800F89D0)));if(current[0]>30)return;
    for(unsigned i=0;i<current[0];i++)current[i+1]=MEM_W(int(i*4),int32_t(0x800F89D4));
    if(current==previous)return;previous=current;
    std::ostringstream line;line<<"[FruitAudit] ring vi="<<test_vi.load()<<" count="<<current[0]<<" types=";
    for(unsigned i=0;i<current[0];i++)line<<(i?",":"")<<current[i+1];
    fprintf(stderr,"%s\n",line.str().c_str());fflush(stderr);
}
#endif
void workshop_tick_vi(){
    const unsigned frame=++test_vi;
#ifdef WORKSHOP_SPECIAL_CONTENT
    if(fruit_audit && diagnostic_rdram)audit_fruit_ring(diagnostic_rdram);
#endif
    if(!test_inputs.empty() && diagnostic_rdram && frame%300==0){
        auto* rdram=diagnostic_rdram;
        fprintf(stderr,"[Workshop] VI %u state=%u mode=%u page=%u level=%u display=%u player=%08X\n",frame,
            uint32_t(MEM_W(0,int32_t(0x800AAC14))),uint32_t(MEM_W(0,int32_t(0x800F8B88))),
            MEM_BU(0,int32_t(0x800F8DFC)),MEM_BU(0,int32_t(0x800F8DFD)),
            uint32_t(MEM_W(0,int32_t(0x807F0008))),uint32_t(MEM_W(0,int32_t(0x800F8828))));
        const auto player=uint32_t(MEM_W(0,int32_t(0x800F8828)));
        if(ram_pointer(player))fprintf(stderr,"[Workshop] Player XY %.2f,%.2f score %u fruits %u\n",
            std::bit_cast<float>(uint32_t(MEM_W(0,int32_t(player+0x3C)))),std::bit_cast<float>(uint32_t(MEM_W(0,int32_t(player+0x40)))),
            uint32_t(MEM_W(0,int32_t(0x800F8954))),uint32_t(MEM_W(0,int32_t(0x800F89D0))));
        if(frame==3000 || frame==3600){
            const std::string path="controller-test-"+std::to_string(frame)+".rdram";
            std::ofstream snapshot(path,std::ios::binary);snapshot.write(reinterpret_cast<char*>(rdram),0x800000);
            workshop_dump_trace();
        }
    }
    if(end_test_vi && frame>=end_test_vi){
#ifdef WORKSHOP_NATIVE_COOP
        // The VI clock may interrupt an owner scope. Capture/exit only after
        // the next completed game-frame wait, with the primary restored.
        if(test_coop_native || (workshop::boss_campaign0250::enabled && !workshop::boss_campaign0250::failed.load()))return;
#endif
#ifdef WORKSHOP_NATIVE_COOP
        if(!test_first_boss.empty() || workshop::boss_solo0240::enabled){
            const auto& b=workshop::boss_native::snapshot();
            if(workshop::boss_solo0240::enabled)fprintf(stderr,"[BossSolo] final phase=%u delivered=%u reason=%s\n",unsigned(workshop::boss_solo0240::session.snapshot().phase),unsigned(workshop::boss_native::delivered),workshop::boss_solo0240::session.snapshot().failure.c_str());
            fprintf(stderr,"[BossNative] final initialized=%d active=%d terminal=%d completion=%d ready=%d invalid=%d segments=%d min=%d max=%d actor=%08X reason=%s\n",b.initialized,b.active_seen,b.terminal_seen,b.completion_called,b.completion_ready,b.invalidated,b.segments,b.min_segments,b.max_segments,b.actor,b.failure.c_str());
        }
#endif
        fprintf(stderr,"[Workshop] Automatic test reached VI %u; terminating isolated test.\n",frame);
        if(diagnostic_rdram){std::ofstream final("controller-test-final.rdram",std::ios::binary);final.write(reinterpret_cast<char*>(diagnostic_rdram),0x800000);}
        workshop_dump_trace();workshop::menu_music_session0310::shutdown();std::fflush(nullptr);std::_Exit(0);
    }
}
bool workshop_background_test(){return background_test;}
bool workshop_test_book_resume(){return test_book_resume && end_test_vi;}
bool workshop_test_book_shop(){return test_book_shop && background_test && end_test_vi;}
bool workshop_get_input(int player,uint16_t* buttons,float* x,float* y){
#ifdef WORKSHOP_NATIVE_COOP
    if(test_coop_native && player>=0 && player<2 && workshop::coop_rooms_native0280::input_blocked(unsigned(player))){*buttons=0;*x=0;*y=0;return true;}
    if(test_coop_room_fixture0280 && test_coop_room_route_driver0280 && diagnostic_rdram && player>=0 && player<2){
        namespace n=workshop::coop_native;namespace rooms=workshop::coop_rooms_native0280;
        const auto& state=workshop::coop_lifecycle0280::lifecycle.snapshot();
        if(state.bound){
            const auto actor=state.players[unsigned(player)].root.address;
            if(n::range(actor,0x1F8)){
                const auto parent=n::word(diagnostic_rdram,actor+0x180);
                if(n::actor(diagnostic_rdram,parent,0x4209) && rooms::reciprocal(diagnostic_rdram,actor,parent)){
                    const auto reason=workshop::coop_room_route_driver0280::rejection_reason(diagnostic_rdram,test_vi.load(),hybrid_room&255,test_coop_room_route_leader0280,unsigned(player));
                    static std::array<std::string,2> previous;static std::array<unsigned,2> counts{};
                    if(counts[unsigned(player)]<16 && previous[unsigned(player)]!=reason){
                        previous[unsigned(player)]=reason;++counts[unsigned(player)];
                        fprintf(stderr,"[CoopPotInput] vi=%u player=%u actor=%08X parent=%08X reason=%s queue=%u hidden=%u callback=%08X ownerDepth=%zu ordinaryInput=1 forcedGameState=0\n",test_vi.load(),unsigned(player),actor,parent,reason,n::byte(diagnostic_rdram,actor+0x17D),n::byte(diagnostic_rdram,actor+0x165),n::word(diagnostic_rdram,actor+0x130),workshop::coop_bridge::owner_stack.size());
                    }
                }
            }
        }
    }
    if(test_coop_secondary_return0288 && player>=0 && player<2 && workshop::coop_secondary_return0288::input(diagnostic_rdram,test_vi.load(),hybrid_room&255,unsigned(player),buttons,x,y)){
        namespace secondary=workshop::coop_secondary_return0288;
        const auto proof=secondary::witness(diagnostic_rdram,test_vi.load(),hybrid_room&255);
        struct Previous {bool seen=false;secondary::Phase phase{};uint16_t buttons=0;float x=0,y=0;};static std::array<Previous,2> previous{};
        auto& old=previous[unsigned(player)];
        if(!old.seen || proof.phase!=old.phase || *buttons!=old.buttons || *x!=old.x || *y!=old.y){old={true,proof.phase,*buttons,*x,*y};fprintf(stderr,"[CoopSecondaryReturnInput] vi=%u player=%u phase=%s buttons=%04X x=%.3f y=%.3f ordinaryInput=1 forcedGameState=0\n",test_vi.load(),unsigned(player),secondary::phase_name(proof.phase),unsigned(*buttons),*x,*y);}
        return true;
    }
    if(test_coop_room_reentry0287 && player>=0 && player<2 && workshop::coop_room_reentry0287::input(diagnostic_rdram,test_vi.load(),hybrid_room&255,unsigned(player),buttons,x,y)){
        namespace reentry=workshop::coop_room_reentry0287;
        const auto proof=reentry::witness(diagnostic_rdram,test_vi.load(),hybrid_room&255);
        struct Previous {bool seen=false;reentry::Phase phase{};uint16_t buttons=0;float x=0,y=0;};static std::array<Previous,2> previous{};
        auto& old=previous[unsigned(player)];
        if(!old.seen || proof.phase!=old.phase || *buttons!=old.buttons || *x!=old.x || *y!=old.y){old={true,proof.phase,*buttons,*x,*y};fprintf(stderr,"[CoopRoomReentryInput] vi=%u player=%u phase=%s buttons=%04X x=%.3f y=%.3f ordinaryInput=1 forcedGameState=0\n",test_vi.load(),unsigned(player),reentry::phase_name(proof.phase),unsigned(*buttons),*x,*y);}
        return true;
    }
    if(test_coop_room_return0286 && player>=0 && player<2 && workshop::coop_room_return0286::input(diagnostic_rdram,test_vi.load(),hybrid_room&255,unsigned(player),buttons,x,y)){
        namespace returns=workshop::coop_room_return0286;
        const auto proof=returns::witness(diagnostic_rdram,test_vi.load(),hybrid_room&255);
        struct Previous {bool seen=false;returns::Phase phase{};uint16_t buttons=0;float x=0,y=0;};static std::array<Previous,2> previous{};
        auto& old=previous[unsigned(player)];
        if(!old.seen || proof.phase!=old.phase || *buttons!=old.buttons || *x!=old.x || *y!=old.y){old={true,proof.phase,*buttons,*x,*y};fprintf(stderr,"[CoopRoomReturnInput] vi=%u player=%u phase=%s buttons=%04X x=%.3f y=%.3f ordinaryInput=1 forcedGameState=0\n",test_vi.load(),unsigned(player),returns::phase_name(proof.phase),unsigned(*buttons),*x,*y);}
        return true;
    }
    if(test_coop_room_route_driver0280 && player>=0 && player<2 && workshop::coop_room_route_driver0280::input(diagnostic_rdram,test_vi.load(),hybrid_room&255,test_coop_room_route_leader0280,unsigned(player),buttons,x,y)){
        namespace route=workshop::coop_room_route_driver0280;
        struct Previous {bool seen=false;route::Mode mode{};uint16_t buttons=0;float x=0,y=0;};static std::array<Previous,2> previous{};
        const auto mode=route::last_mode[unsigned(player)];auto& old=previous[unsigned(player)];
        if(!old.seen || mode!=old.mode || *buttons!=old.buttons || *x!=old.x || *y!=old.y){old={true,mode,*buttons,*x,*y};fprintf(stderr,"[CoopRoomInput] vi=%u player=%u mode=%s buttons=%04X x=%.3f y=%.3f ordinaryInput=1 forcedGameState=0\n",test_vi.load(),unsigned(player),route::names[unsigned(mode)],unsigned(*buttons),*x,*y);}
        return true;
    }
    // This explicit test cannot hand off before its requested loadout is read back.
    const bool mixed_fish_wait=boss_test_fish_loadout&&workshop::boss_test_loadout0260::mixed_fish_tuple(boss_battle_seed,unsigned(boss_test_case))&&workshop::boss_campaign0250::session.snapshot().phase==workshop::boss_battle0250::Phase::Fighting&&workshop::boss_campaign0250::fight_ready.vi;
    if((boss_test_full_loadout || boss_test_lips_loadout || boss_test_genie_loadout || boss_test_monsters_loadout || boss_test_fish_loadout || boss_test_final_loadout) && (test_vi.load()>=workshop::boss_manual_handoff0260::handoff_vi||mixed_fish_wait) && !boss_loadout.snapshot().applied){*buttons=mixed_fish_wait?uint16_t(*buttons&0x8000):0;*x=0;*y=0;return player==0;}
#endif
    if(test_inputs.empty() || workshop::boss_manual_handoff0260::live(boss_manual_handoff,test_vi.load())){
        #ifdef WORKSHOP_NATIVE_COOP
        const bool connected=test_coop_manual?workshop::coop_manual_input::read(player,buttons,x,y):recompinput::profiles::get_n64_input(player,buttons,x,y);
#else
        const bool connected=recompinput::profiles::get_n64_input(player,buttons,x,y);
#endif
        if(!connected){*buttons=0;*x=0;*y=0;}
        if(player==0){
            const auto frame=test_vi.load();
            workshop::custom_session::navigation(frame,buttons,x,y);
            if(workshop::custom_session::active()
#ifdef WORKSHOP_NATIVE_COOP
                || workshop::boss_solo0240::enabled || workshop::boss_campaign0250::enabled
#endif
            )select_test_color(frame,buttons,x,y);
            workshop::book::filter_input(buttons,x,y);
        }
        if(boss_manual_handoff && player==0){
            static unsigned samples=0,last_vi=0;static bool have=false,last_connected=false;static uint16_t last_buttons=0;static float last_x=0,last_y=0;
            const unsigned vi=test_vi.load();const bool change=!have||connected!=last_connected||*buttons!=last_buttons||*x!=last_x||*y!=last_y;
            if(samples<4000 && ((change && (!have||vi-last_vi>=2)) || (!connected && vi-last_vi>=120))){
                fprintf(stderr,"[BossManualInput] vi=%u connected=%u buttons=%04X stick=%.5f,%.5f liveInput=1 scriptedCombat=0\n",vi,unsigned(connected),unsigned(*buttons),*x,*y);
                ++samples;last_vi=vi;have=true;last_connected=connected;last_buttons=*buttons;last_x=*x;last_y=*y;
            }
        }
        return connected;
    }
    *buttons=0;*x=0;*y=0;
#ifdef WORKSHOP_NATIVE_COOP
    if(player==1 && test_coop_native){
        const unsigned frame=test_vi.load();
        const unsigned playback=relative_play && frame>=2408?(test_player_vi && frame>=test_player_vi+180?3000+frame-test_player_vi-180:2999):frame;
        for(const auto& e:test_inputs_p2)if(playback>=e.first && playback<e.last){*buttons=e.buttons;*x=e.x;*y=e.y;break;}
        return true;
    }
#endif
    if(player!=0)return false;
    const unsigned frame=test_vi.load();
    unsigned playback=frame;
    if(relative_play && frame>=2408){
        if(diagnostic_rdram && !test_player_vi){
            auto* rdram=diagnostic_rdram;auto actor=MEM_W(0,gpr(int32_t(0x800F8828)));
            if(ram_pointer(uint32_t(actor)) && MEM_HU(12,static_cast<gpr>(static_cast<int32_t>(actor)))==0x4001){test_player_vi=frame;fprintf(stderr,"[Workshop] Gameplay input begins at VI %u.\n",frame+180);}
        }
        playback=test_player_vi && frame>=test_player_vi+180?3000+frame-test_player_vi-180:2999;
        if(test_warp && playback>=test_warp_at && diagnostic_rdram){
            auto* rdram=diagnostic_rdram;const auto actor=MEM_W(0,gpr(int32_t(0x800F8828)));
            if(ram_pointer(uint32_t(actor)) && MEM_HU(12,actor)==0x4001){
                const float dx=test_warp_x-std::bit_cast<float>(uint32_t(MEM_W(0x3C,actor))),dy=test_warp_y-std::bit_cast<float>(uint32_t(MEM_W(0x40,actor)));
                const auto camera=MEM_W(0,gpr(int32_t(0x800F8318)));
                if(ram_pointer(uint32_t(camera)))for(int offset:{0x3C,0x84}){
                    MEM_W(offset,camera)=std::bit_cast<uint32_t>(std::bit_cast<float>(uint32_t(MEM_W(offset,camera)))+dx);
                    MEM_W(offset+4,camera)=std::bit_cast<uint32_t>(std::bit_cast<float>(uint32_t(MEM_W(offset+4,camera)))+dy);
                }
                MEM_W(0x3C,actor)=std::bit_cast<uint32_t>(test_warp_x);MEM_W(0x40,actor)=std::bit_cast<uint32_t>(test_warp_y);
                MEM_W(0x84,actor)=std::bit_cast<uint32_t>(test_warp_x);MEM_W(0x88,actor)=std::bit_cast<uint32_t>(test_warp_y);
                MEM_W(0xA8,actor)=std::bit_cast<uint32_t>(test_warp_x);MEM_W(0xAC,actor)=std::bit_cast<uint32_t>(test_warp_y);
                test_warp=false;test_warp_hold_until=playback+30;fprintf(stderr,"[Workshop] One-shot test warp vi=%u playback=%u xy=%.2f,%.2f; release after 30 frames.\n",frame,playback,test_warp_x,test_warp_y);
            }
        }
        if(test_warp_hold_until && playback<test_warp_hold_until && diagnostic_rdram){
            auto* rdram=diagnostic_rdram;const auto actor=MEM_W(0,gpr(int32_t(0x800F8828)));
            if(ram_pointer(uint32_t(actor)) && MEM_HU(12,actor)==0x4001)for(int offset:{0x3C,0x84,0xA8}){
                MEM_W(offset,actor)=std::bit_cast<uint32_t>(test_warp_x);MEM_W(offset+4,actor)=std::bit_cast<uint32_t>(test_warp_y);
            }
        }
        if(test_spawn && test_player_vi && diagnostic_rdram){
            auto* rdram=diagnostic_rdram;const auto actor=MEM_W(0,gpr(int32_t(0x800F8828)));
            if(ram_pointer(uint32_t(actor)) && MEM_W(0,int32_t(0x800F89D0))<30 && MEM_HU(12,actor)==0x4001){MEM_W(0x3C,actor)=std::bit_cast<uint32_t>(test_spawn_x);MEM_W(0x40,actor)=std::bit_cast<uint32_t>(test_spawn_y);if(test_release_spawn)test_spawn=false;}
        }
    }
    for(const auto& event:test_inputs)if(playback>=event.first && playback<event.last){*buttons=event.buttons;*x=event.x;*y=event.y;break;}
    workshop::custom_session::navigation(frame,buttons,x,y);
    select_test_color(frame,buttons,x,y);
#ifdef WORKSHOP_NATIVE_COOP
    workshop::boss_solo0240::drive_input(diagnostic_rdram,frame,buttons,x,y);
    const bool checkpoint_controls=workshop::boss_route_checkpoint_bridge0260::drive(diagnostic_rdram,frame,buttons,x,y);
    const bool swallow_controls=workshop::boss_swallow_retry_test0263::drive(diagnostic_rdram,frame,buttons,x,y);
    const bool fish_cp6_controls=(boss_test_fish_cp6 || boss_test_fish_cp6_focused) && workshop::boss_fish_cp6_bridge0284::drive(diagnostic_rdram,frame,buttons,x,y);
    if(!checkpoint_controls && !swallow_controls && !fish_cp6_controls && (!(boss_test_full_loadout || boss_test_lips_loadout || boss_test_genie_loadout || boss_test_monsters_loadout || boss_test_fish_loadout || boss_test_final_loadout) || boss_loadout.snapshot().applied))workshop::boss_campaign0250::drive_input(diagnostic_rdram,frame,buttons,x,y);
    if(!checkpoint_controls && !swallow_controls && !fish_cp6_controls && (!(boss_test_full_loadout || boss_test_lips_loadout || boss_test_genie_loadout || boss_test_monsters_loadout || boss_test_fish_loadout || boss_test_final_loadout) || boss_loadout.snapshot().applied))workshop::boss_driver_bridge0260::drive(diagnostic_rdram,frame,buttons,x,y,recomp::mem_size);
    if(!boss_followup_inputs.empty()){
        const auto& s=workshop::boss_campaign0250::session.snapshot();const auto before=boss_followup_anchor.anchor_vi();
        boss_followup_anchor.apply({workshop::boss_campaign0250::enabled,s.diagnostic,workshop::boss_campaign0250::driver,s.index,s.phase==workshop::boss_battle0250::Phase::Fighting,s.phase==workshop::boss_battle0250::Phase::Victory,frame},boss_followup_inputs,buttons,x,y);
        if(!before&&boss_followup_anchor.anchor_vi())fprintf(stderr,"[BossFollowupInput] handoff vi=%u encounter=2 events=%u inputOnly=1\n",boss_followup_anchor.anchor_vi(),unsigned(boss_followup_inputs.size()));
    }
#endif
    #ifdef WORKSHOP_NATIVE_COOP
    if(boss_test_goal=="ground-roundtrip"&&diagnostic_rdram){
        namespace n=workshop::boss_native;const auto& s=workshop::boss_campaign0250::session.snapshot();const auto p=s.player;
        if(n::actor(diagnostic_rdram,p,0x4001)&&!workshop::boss_campaign0250::fatal_player(diagnostic_rdram,p))boss_ground_walk.apply({workshop::boss_campaign0250::enabled,s.diagnostic,workshop::boss_campaign0250::driver,s.phase==workshop::boss_battle0250::Phase::Victory,s.index,frame,std::bit_cast<float>(n::word(diagnostic_rdram,p+0x84)),std::bit_cast<float>(n::word(diagnostic_rdram,p+0x88))},buttons,x,y);
    }
    #endif
#ifdef WORKSHOP_NATIVE_COOP
    if(boss_test_blargg_acquire0289){
        namespace campaign=workshop::boss_campaign0250;
        if(boss_test_blargg_shoot0289 && boss_blargg_acquire.snapshot().done){
            const auto chosen=boss_blargg_shoot.step(diagnostic_rdram,boss_blargg_shoot_frame(frame),campaign::blargg_observer);
            if(chosen.handled){*buttons=chosen.buttons;*x=chosen.x;*y=chosen.y;}
            if(boss_blargg_shoot.snapshot().failed && !campaign::failed.load()){
                fprintf(stderr,"[BossBlarggShootQA] rejected reason=%s vi=%u\n",boss_blargg_shoot.snapshot().reason,frame);
                campaign::fail("blargg_shooting_QA_rejected");
            }
        }else{
        const auto& state=campaign::session.snapshot();const auto& ready=campaign::fight_ready;
        const bool fight_ready=ready.vi && ready.index==state.index && ready.room==campaign::physical_room && ready.source==19 && ready.epoch==state.retry_epoch && ready.player==state.player;
        const workshop::boss_blargg_acquire0289::Frame observed{boss_test_blargg0289,state.diagnostic,state.phase==workshop::boss_battle0250::Phase::Fighting,fight_ready,campaign::driver,campaign::physical_room,campaign::logical_room,state.index,state.retry_epoch,frame,state.player,state.token,campaign::session.plan().seed};
        const auto chosen=boss_blargg_acquire.step(diagnostic_rdram,observed);
        if(chosen.handled){*buttons=chosen.buttons;*x=chosen.x;*y=chosen.y;}
        if(boss_blargg_acquire.snapshot().failed && !campaign::failed.load()){
            fprintf(stderr,"[BossBlarggAcquireQA] rejected reason=%s vi=%u\n",boss_blargg_acquire.snapshot().reason,frame);
            campaign::fail("blargg_acquisition_QA_rejected");
        }
        }
    }
#endif
    workshop::book::filter_input(buttons,x,y);
    if(fruit_audit){
        static unsigned samples=0,last=0;
        if(samples<3 || (*buttons && frame-last>=30)){
            fprintf(stderr,"[InputAudit] vi=%u player=%d buttons=%04X stick=%.2f,%.2f\n",frame,player,unsigned(*buttons),*x,*y);
            last=frame;samples++;
        }
    }
    return true;
}
#ifdef WORKSHOP_NATIVE_COOP
extern "C" void func_80045BD8(uint8_t*,recomp_context*);
#endif
extern "C" recomp_func_t* workshop_lookup(int32_t address,uint8_t* rdram,recomp_context* ctx,const char* caller){
    // Audio DMA runs continuously, even when the game is waiting for assets.
    // Exclude that high-frequency callback so the ring keeps gameplay calls.
    if(uint32_t(address)!=0x8008AB60u){std::lock_guard lock(lookup_mutex);lookup_records[lookup_count++%lookup_records.size()]={uint32_t(address),uint32_t(ctx->r31),uint32_t(ctx->r29),uint32_t(ctx->r4),uint32_t(ctx->r5),caller};}
    if(uint32_t(address)<0x80000000u || uint32_t(address)>=0x80800000u){
        fprintf(stderr,"[Workshop] Invalid target %08X from %s, ra=%08X sp=%08X a0=%08X a1=%08X\n",uint32_t(address),caller,uint32_t(ctx->r31),uint32_t(ctx->r29),uint32_t(ctx->r4),uint32_t(ctx->r5));fflush(stderr);
    }
    auto function=get_function(address);
#ifdef WORKSHOP_NATIVE_COOP
    if(function==func_80045BD8 && workshop::coop_native::cleanup_owned(rdram,uint32_t(ctx->r4)))return workshop::coop_native::native_cleanup;
    if(function==func_8001D5B4 && workshop::coop_classic_flower0250::owner(rdram,uint32_t(ctx->r4)))return workshop::coop_classic_flower0250::deferred_init;
#endif
#ifdef WORKSHOP_SPECIAL_CONTENT
    if(function==func_806ED088 && workshop::black_flyguy::skip_collision(rdram,uint32_t(ctx->r4)))return black_controller_no_collision;
#endif
    return function;
}
// The cartridge bootstrap used PI hardware registers directly. The PC port
// performs the same two ROM copies through the runtime's DMA interface.
// These two adapters are excluded from the generated game functions.
extern "C" void workshop_807F1600(uint8_t* rdram,recomp_context*) {
    recomp::do_rom_read(rdram,gpr(int32_t(0x807F0000)),0x10F10000,0x2000);
    recomp::do_rom_read(rdram,gpr(int32_t(0x807F2000)),0x10F60000,0x6000);
}
#ifdef WORKSHOP_SPECIAL_CONTENT
extern "C" void func_8006988C(uint8_t* rdram,recomp_context* ctx);
static bool metal_solo_scope(){
    bool allowed=!workshop::bonus_game0352::active()&&payload.empty()&&!workshop::custom_session::active()&&!test_tally;
#ifdef WORKSHOP_NATIVE_COOP
    allowed=allowed&&!test_coop_native&&!workshop::boss_solo0240::enabled&&!workshop::boss_campaign0250::enabled;
#endif
    return allowed&&!story_hard_session0330();
}
static bool metal_custom_scope0311(){
    bool deferred=false;
#ifdef WORKSHOP_NATIVE_COOP
    deferred=test_coop_native||workshop::boss_solo0240::enabled||workshop::boss_campaign0250::enabled;
#endif
    return workshop::metal_custom0311::scope(!payload.empty(),workshop::custom_session::active(),test_tally,deferred);
}
static bool metal_boss_scope0354(){
#ifdef WORKSHOP_NATIVE_COOP
 return workshop::boss_metals0354::scope(workshop::boss_solo0240::enabled||workshop::boss_campaign0250::enabled,test_coop_native,!payload.empty(),workshop::custom_session::active(),test_tally);
#else
 return false;
#endif
}
static bool metal_play_scope0311(){return workshop::bonus_game0352::active()||metal_boss_scope0354()||metal_solo_scope()||metal_custom_scope0311();}
// These two extra lives never share the aliased native White/Black life bits.
static bool elemental_solo_scope0355(){
 bool allowed=!test_tally;
#ifdef WORKSHOP_NATIVE_COOP
 allowed=allowed&&!test_coop_native;
#endif
 return allowed&&(story_hard_session0330()||metal_play_scope0311());
}
static unsigned elemental_owned_extra0355(){
 const auto v=workshop::elemental_yoshis0355::view();
 return elemental_solo_scope0355()&&v.initialized&&v.readable&&v.saved?(v.found&3u)<<8:0;
}
static void elemental_save_lives0355(uint8_t* rdram){
 if(rdram&&metal_solo_scope()&&workshop::metal_native0300::story(rdram))
  workshop::elemental_yoshis0355::save_story_alive((workshop::metal_native0300::extras(rdram)>>8)&3u);
}
static workshop::elemental_contact0355::Tracker elemental_contacts0355;
struct ElementalTongueDraw0355 {uint64_t token=0;workshop::elemental_tongue0355::Ticket ticket;};
static std::vector<ElementalTongueDraw0355> elemental_tongue_draws0355;
static uint64_t elemental_tongue_serial0355=0;
extern "C" uint64_t workshop_elemental_tongue_begin0355(uint8_t* rdram,uint32_t actor){
    if(!elemental_solo_scope0355()||elemental_tongue_draws0355.size()>=32)return 0;
    // Allocate the host slot before changing either guest palette pointer.
    if(++elemental_tongue_serial0355==0)++elemental_tongue_serial0355;
    elemental_tongue_draws0355.push_back({elemental_tongue_serial0355,{}});
    auto& draw=elemental_tongue_draws0355.back();draw.ticket=workshop::elemental_tongue0355::begin(rdram,actor,true);
    if(!draw.ticket.active){elemental_tongue_draws0355.pop_back();return 0;}
    return draw.token;
}
extern "C" void workshop_elemental_tongue_end0355(uint8_t* rdram,uint64_t token)noexcept{
    if(!token||elemental_tongue_draws0355.empty()||elemental_tongue_draws0355.back().token!=token)return;
    workshop::elemental_tongue0355::finish(rdram,elemental_tongue_draws0355.back().ticket);elemental_tongue_draws0355.pop_back();
}

extern "C" void workshop_elemental_hit_begin0355(uint8_t* rdram,recomp_context* ctx){elemental_contacts0355.begin(rdram,ctx,elemental_solo_scope0355(),uint32_t(section_addresses[19]),uint32_t(section_addresses[20]),uint32_t(section_addresses[130]),uint32_t(section_addresses[88]),uint32_t(section_addresses[207]));}
extern "C" void workshop_elemental_hit_end0355(uint8_t* rdram,recomp_context* ctx){elemental_contacts0355.end(rdram,ctx);}
extern "C" int workshop_elemental_hit_loss0355(uint8_t* rdram,recomp_context* ctx){return elemental_solo_scope0355()?elemental_contacts0355.loss(rdram,ctx):3;}
extern "C" void workshop_elemental_flame_food_gate0355(uint8_t* rdram,recomp_context* ctx){
    const auto player=uint32_t(MEM_W(0,int32_t(0x800F8828)));
    workshop::elemental_flame_food0355::gate(rdram,ctx,elemental_solo_scope0355(),uint32_t(section_addresses[88]),elemental_contacts0355.birth(player),elemental_contacts0355.birth(uint32_t(ctx->r16)));
}
extern "C" void func_804A1398(uint8_t*,recomp_context*);
extern "C" void workshop_locked_selection_partition0355(uint8_t* rdram,recomp_context* ctx){workshop::locked_selection_native0355::prepare_children(rdram,ctx);}
extern "C" int workshop_locked_selection_confirm0355(uint8_t* rdram,recomp_context* ctx){return workshop::locked_selection_native0355::confirm(rdram,ctx);}
extern "C" int workshop_locked_selection_static0355(uint8_t* rdram,recomp_context* ctx){
 const auto state=workshop::locked_selection_native0355::initialize(rdram,ctx,uint32_t(section_addresses[57]));
 if(state==workshop::locked_selection_native0355::Init::not_question)return 0;
 if(state==workshop::locked_selection_native0355::Init::ready){const auto saved=*ctx;func_804A1398(rdram,ctx);*ctx=saved;}
 return 1;
}
extern "C" int workshop_selection_unknown0355(unsigned color){
 return color<18&&!workshop::known_yoshis_runtime0355::snapshot().known(color);
}
extern "C" int workshop_locked_selection_node0355(uint8_t* rdram,uint32_t actor){return workshop::locked_selection_native0355::node(rdram,actor);}
extern "C" void workshop_selection_begin0355(uint8_t*,recomp_context*){
 workshop::known_yoshis_runtime0355::canonical(workshop::story_rights0320::read(recomp::get_config_path()/"saves"/"yoshisstory.n64.us.bin"));
}
extern "C" void workshop_elemental_selection_capacity0355(uint8_t* rdram,recomp_context* ctx){workshop::elemental_selection_capacity0355::clamp_white_icons(rdram,ctx);}
extern "C" uint32_t workshop_elemental_terrain_word(uint8_t* rdram,uint32_t actor,uint32_t raw){return workshop::elemental_lava0355::terrain_word(rdram,actor,raw,elemental_solo_scope0355());}
extern "C" void workshop_water_swim0355(uint8_t* rdram,recomp_context* ctx){workshop::elemental_native0355::swim_target(rdram,ctx,elemental_solo_scope0355());}
static workshop::trial_special_lives0350::Store trial_special_store0350;
struct TrialPendingLoss0350 {unsigned mask=0,last_retry_vi=0;};
static std::mutex trial_pending_mutex0350;
static std::map<std::filesystem::path,TrialPendingLoss0350> trial_pending_by_profile0350;
static std::filesystem::path trial_special_profile0350;
static TrialPendingLoss0350 trial_special_pending0350(){
    std::lock_guard guard(trial_pending_mutex0350);return trial_pending_by_profile0350[trial_special_profile0350];
}
static void trial_special_pending_update0350(unsigned color,bool pending){
    std::lock_guard guard(trial_pending_mutex0350);auto& state=trial_pending_by_profile0350[trial_special_profile0350];
    if(pending)state.mask|=1u<<color;else state.mask&=~(1u<<color);
    state.last_retry_vi=test_vi.load();
}
static bool trial_special_scope0350(uint8_t* rdram){
    if(!rdram)return false;
    const int native=MEM_W(0,int32_t(0x800F8B88));
    const int selected=workshop_menu_enabled()?workshop_menu_session_mode():end_test_vi?(test_hybrid?3:1):(native==1?1:-1);
    return workshop::trial_special_native0350::scope(metal_solo_scope(),selected,native);
}
static unsigned trial_special_blocked0350(uint8_t* rdram){
    return trial_special_scope0350(rdram)?trial_special_store0350.snapshot().blocked_mask()|trial_special_pending0350().mask:0;
}
static void trial_special_retry0350(uint8_t* rdram){
    if(!trial_special_scope0350(rdram))return;
    const auto pending=trial_special_pending0350();const unsigned frame=test_vi.load();
    if(!pending.mask||frame-pending.last_retry_vi<60)return;
    {std::lock_guard guard(trial_pending_mutex0350);trial_pending_by_profile0350[trial_special_profile0350].last_retry_vi=frame;}
    trial_special_store0350.refresh();
    for(unsigned color:{6u,7u,14u,15u})if(pending.mask&(1u<<color)){
        if(trial_special_store0350.lose(color).accepted())trial_special_pending_update0350(color,false);
    }
}
static void trial_special_project0350(uint8_t* rdram){
    if(!trial_special_scope0350(rdram))return;
    trial_special_retry0350(rdram);
    workshop::trial_special_native0350::project(rdram,trial_special_blocked0350(rdram));
}
static unsigned trial_special_recovery_metals0350(uint8_t* rdram){
    if(!trial_special_scope0350(rdram))return 0;
    const auto state=trial_special_store0350.snapshot();
    if(!state.readable||!state.last_write_succeeded)return 0;
    return workshop::trial_special_native0350::metal_extras(state.lost_mask)&(workshop::metal_yoshis0300::view().found<<6);
}
static bool trial_special_owned0350(uint8_t* rdram,unsigned color){
    if(color==14||color==15)return (workshop::metal_yoshis0300::view().found&(1u<<(color-14)))!=0;
    if(color!=6&&color!=7)return false;
    const auto p=workshop::progress::read(rdram,gpr(int32_t(0x807F2000)));
    const auto story=uint32_t(MEM_W(0,int32_t(0x800F8E5C)))&0xFFFFFFu;
    return workshop::progress::valid(p,story)&&p.trial==0xFFFFFFu&&(p.eggs&(1u<<(color-6)));
}
static void trial_special_death0350(uint8_t* rdram,unsigned color){
    if(!trial_special_scope0350(rdram)||!workshop::trial_special_lives0350::special(color)||!trial_special_owned0350(rdram,color))return;
    trial_special_pending_update0350(color,true);
    const auto result=trial_special_store0350.lose(color);
    if(result.accepted())trial_special_pending_update0350(color,false);
    trial_special_project0350(rdram);
    fprintf(stderr,"[TrialSpecial0350] death pc=%u lost=%04X saved=%u ownershipChanged=0\n",color,result.lost_mask,unsigned(result.accepted()));
}
static void trial_special_recover0350(uint8_t* rdram,unsigned pc_mask){
    if(!trial_special_scope0350(rdram))return;
    const auto state=trial_special_store0350.snapshot();
    if(!state.readable||!state.last_write_succeeded)return;
    for(unsigned color:{6u,7u,14u,15u})if((pc_mask&state.lost_mask&(1u<<color))&&trial_special_owned0350(rdram,color)){
        const auto result=trial_special_store0350.recover(color);
        if(result.accepted())trial_special_pending_update0350(color,false);
        fprintf(stderr,"[TrialSpecial0350] egg-finished pc=%u lost=%04X saved=%u ownershipChanged=0\n",color,result.lost_mask,unsigned(result.accepted()));
    }
}
static unsigned metal_available_extra0311(uint8_t* rdram){
    if(workshop::bonus_game0352::active())return (workshop::bonus_game0352::team_available(rdram)>>8)&0x3C0u;
    if(metal_boss_scope0354())return workshop::boss_metals0354::available_extra(workshop::metal_yoshis0300::view())|elemental_owned_extra0355();
    // Search/acquisition/Story writes still use ONLY the canonical gate.
    workshop::metal_yoshis0300::set_scope(metal_solo_scope());
    const unsigned owned=metal_custom_scope0311()?workshop::metal_custom0311::owned_extra(workshop::metal_yoshis0300::view()):workshop::metal_yoshis0300::unlocked_extra();
    return (owned&~workshop::trial_special_native0350::metal_extras(trial_special_blocked0350(rdram)))|elemental_owned_extra0355();
}
static uint32_t story_clears(uint8_t* rdram){
    if(progress_payload)return uint32_t(MEM_W(0,static_cast<gpr>(static_cast<int32_t>(progress_payload))))&0xFFFFFF;
    if(progress_reading)return progress_boot_story;
    return uint32_t(MEM_W(0,int32_t(0x800F8E5C)))&0xFFFFFF;
}
extern "C" int workshop_progress_valid(uint8_t* rdram,recomp_context* ctx){
    if(!ram_pointer(uint32_t(ctx->r4)))return 0;
    return workshop::progress::valid(workshop::progress::read(rdram,ctx->r4),story_clears(rdram));
}
extern "C" int workshop_progress_trial_access(uint8_t* rdram){return workshop::bonus_game0352::active()||story_clears(rdram)==0xFFFFFF;}
extern "C" int workshop_progress_trial(uint8_t* rdram,recomp_context* ctx){
    if(workshop::bonus_game0352::active()){ctx->r2=0;return 1;}
    // A Story reward belongs to this same column when Trial catches up.
    // Keep its color instead of drawing a second reward for the same column.
    if(!ram_pointer(uint32_t(ctx->r4)) || ctx->r5>=6 || ctx->r6>=4)return 0;
    auto p=workshop::progress::read(rdram,ctx->r4);const unsigned c=ctx->r6;
    const uint32_t bit=1u<<(ctx->r5*4+c),mask=workshop::progress::column(c);
    if(!workshop::progress::valid(p,story_clears(rdram)) || p.first==255 || p.columns[c]>=6 || (p.trial&bit) || ((p.trial|bit)&mask)!=mask)return 0;
    p.trial|=bit;workshop::progress::write(rdram,ctx->r4,p);
    const auto output=MEM_W(0x14,ctx->r29);
    if(output && ram_pointer(uint32_t(output)))MEM_B(0,output)=255;
    ctx->r2=p.trial==0xFFFFFF?4:1;return 1;
}
extern "C" void workshop_progress_read_begin(uint8_t*,recomp_context* ctx){progress_payload=uint32_t(ctx->r4);progress_reading=true;}
extern "C" void workshop_progress_read_end(uint8_t* rdram,recomp_context*){
    if(workshop::bonus_game0352::active()){workshop::bonus_game0352::native_course_access(rdram);progress_payload=0;progress_reading=false;return;}
    progress_boot_story=story_clears(rdram);
    if(story_hard_session0330()){progress_payload=0;return;}
    if(story_hybrid_session0320())workshop::finds::migrate(rdram,canonical_rights0320.progression,canonical_rights0320.story,canonical_rights0320.present&&canonical_rights0320.readable);
    else workshop::finds::migrate(rdram,progress_boot_story);
    workshop::hybrid_mode::records_loaded(rdram,progress_payload);progress_payload=0;
}
extern "C" int workshop_hybrid_records_save(uint8_t* rdram,recomp_context* ctx){return workshop::hybrid_mode::records_saving(rdram,uint32_t(MEM_W(0x20,ctx->r29)),0x800F97F8);}
extern "C" int workshop_hybrid_active(){return workshop::hybrid_mode::lucky_override_active();}
extern "C" unsigned workshop_hybrid_lucky_type(){return workshop::hybrid_mode::lucky_type();}
extern "C" unsigned workshop_hybrid_lucky_object(){return workshop::hybrid_mode::lucky_object();}
extern "C" void workshop_hybrid_recolor(uint8_t* rdram,recomp_context* ctx){if(ctx->r2==1)workshop::hybrid_mode::recolored(rdram,uint32_t(ctx->r16));}
static unsigned reconcile_story(uint8_t* rdram){
    const auto at=gpr(int32_t(0x807F2000));auto p=workshop::progress::read(rdram,at);
    static std::mt19937 random(std::random_device{}());
    const auto added=workshop::progress::reconcile(p,story_clears(rdram),[&](unsigned count){return std::uniform_int_distribution<unsigned>(0,count-1)(random);});
    if(!added)return 0;
    workshop::progress::write(rdram,at,p);
    const auto acquired=workshop::finds::available(added);
    MEM_W(0,int32_t(0x807F1000))|=acquired;
    if((MEM_HU(0,int32_t(0x800F9254))&7) && (MEM_BU(0,int32_t(0x800F9253))&8)){
        const unsigned saved=(((MEM_BU(0,int32_t(0x800F8E58))&3)<<3)|(MEM_BU(0,int32_t(0x800F9253))&7)|acquired)&31;
        MEM_B(0,int32_t(0x800F8E58))=(MEM_BU(0,int32_t(0x800F8E58))&252)|(saved>>3);
        MEM_B(0,int32_t(0x800F9253))=(MEM_BU(0,int32_t(0x800F9253))&248)|(saved&7);
    }
    fprintf(stderr,"[Workshop] Story/Trial column rewards: story=%06X trial=%06X added=%02X first=%u columns=%u,%u,%u,%u.\n",story_clears(rdram),p.trial,added,p.first,p.columns[0],p.columns[1],p.columns[2],p.columns[3]);
    return added;
}
extern "C" void workshop_progress_reconcile(uint8_t* rdram,recomp_context*){
    if(workshop::bonus_game0352::active())return;
    if(story_hard_session0330())return;
    if(story_hybrid_session0320()){
        if(story_lives_ready0320&&workshop::metal_native0300::story(rdram)){workshop::story_hybrid0320::save_lives((workshop::metal_native0300::extras(rdram)>>6)&3);elemental_save_lives0355(rdram);}return;
    }
    reconcile_story(rdram);if(metal_solo_scope()){workshop::metal_native0300::save_story(rdram);elemental_save_lives0355(rdram);}
}
extern "C" int workshop_story_event0320(){return story_hybrid_session0320()||story_hard_session0330();}
extern "C" void workshop_hard_init0330(uint8_t* r,recomp_context* c){if(story_hard_session0330())workshop::story_hard_bridge0330::init_projection(r,c);}
extern "C" void workshop_hard_new0330(uint8_t* rdram,recomp_context* c,int chosen){
    // The default title boot is not an explicit New Game choice. Its reset
    // preserves the native continuation, so preserve the matching Hard run.
    const bool continuing=rdram&&(MEM_HU(0,int32_t(0x800F9254))&7)&&(MEM_BU(0,int32_t(0x800F9253))&8);
    if(story_hard_session0330()&&(chosen||!continuing)&&workshop::story_hard_bridge0330::new_game(rdram,c))hard_continue_refused0330=false;
}
bool workshop_hard_continue_refused(){return hard_continue_refused0330.load();}
extern "C" int workshop_hard_continue0330(uint8_t* rdram,recomp_context* ctx){
    if(!story_hard_session0330()||workshop::story_hard_bridge0330::continue_game(rdram,ctx))return 0;
    // The native confirmation bit and New Game selection keep the existing
    // choice screen usable; no saved continuation or journal is reset here.
    hard_continue_refused0330=true;
    MEM_B(0,int32_t(0x8010E1E8))=MEM_BU(0,int32_t(0x8010E1E8))&~1u;
    MEM_W(0,int32_t(0x800F8E00))=2;
    workshop::book::request_open();return 1;
}
extern "C" void workshop_hard_save0330(uint8_t* r,recomp_context* c){if(story_hard_session0330())workshop::story_hard_bridge0330::save_finished(r,c);}
extern "C" void workshop_hard_death0330(uint8_t* r,recomp_context* c){if(story_hard_session0330())workshop::story_hard_bridge0330::death_finish(r,c);}
extern "C" void workshop_hard_rescue0330(uint8_t* r,recomp_context* c){if(story_hard_session0330())workshop::story_hard_bridge0330::rescue_finish(r,c);}
extern "C" int workshop_hard_capacity0330(uint8_t* r,recomp_context* c){return story_hard_session0330()&&workshop::story_hard_bridge0330::capacity(r,c);}
extern "C" int workshop_hard_picker0330(uint8_t* r,recomp_context* c){return story_hard_session0330()&&workshop::story_hard_bridge0330::picker(r,c);}

static void story_world_receipt0320(std::optional<unsigned> world){
    if(world&&*world<6&&story_hybrid_session0320())story_pending_worlds0320|=1u<<*world;
    for(unsigned page=0;page<6;page++)if(story_pending_worlds0320&(1u<<page)){
        if(workshop::story_hybrid0320::successful_world(page,true)){
            story_pending_worlds0320&=~(1u<<page);
            const auto state=workshop::story_hybrid0320::view();
            fprintf(stderr,"[StoryHybrid0320] native-world-commit world=%u worlds=%02X eligible=%02X automaticOwnership=0\n",page+1,state.worlds,state.eligible);
        }
    }
}
extern "C" void workshop_story_world_arm0320(uint8_t* rdram,recomp_context* context){if(story_hard_session0330()){workshop::story_hard_bridge0330::arm_world(rdram,context);return;}story_observer0320.arm_world_save(rdram,context);}
extern "C" void workshop_story_world_commit0320(uint8_t* rdram,recomp_context* context){if(story_hard_session0330()){workshop::story_hard_bridge0330::commit_world(rdram,context);return;}story_world_receipt0320(story_observer0320.finish_world_save(rdram,context));}
extern "C" void workshop_story_bowser_arm0320(uint8_t* rdram,recomp_context* context){if(workshop::bonus_game0352::active()){workshop::bonus_game0352::bowser_arm(rdram,context);return;}if(story_hard_session0330()){workshop::story_hard_bridge0330::arm_bowser(rdram,context);return;}story_observer0320.arm_bowser(rdram,context);}
extern "C" void workshop_story_bowser_commit0320(uint8_t* rdram,recomp_context* context){if(workshop::bonus_game0352::active()){workshop::bonus_game0352::bowser_commit(rdram,context);return;}if(story_hard_session0330()){workshop::story_hard_bridge0330::commit_bowser(rdram,context);return;}story_world_receipt0320(story_observer0320.finish_bowser(rdram,context));}
extern "C" unsigned workshop_find_unlocked(uint8_t* rdram){
    if(workshop::bonus_game0352::active())return workshop::bonus_game0352::team_available(rdram)>>8;
    if(story_hard_session0330())return workshop::story_hard_bridge0330::unlocked_extras();
    if(story_hybrid_session0320()){
        const auto found=workshop::finds::view();
        return found.initialized&&found.readable&&found.migrated&&found.saved?(found.found&63)|metal_available_extra0311(rdram):0;
    }
    const auto p=workshop::progress::read(rdram,gpr(int32_t(0x807F2000)));
    const auto story=story_clears(rdram);
    if(!workshop::progress::valid(p,story))return 0;
    workshop::finds::migrate(rdram,story);
    return workshop::finds::available(workshop::progress::unlocked(p,story)|workshop::story_hybrid0320::view().acquired)|metal_available_extra0311(rdram);
}
extern "C" void workshop_find_reset(){if(!workshop::bonus_game0352::active()&&!story_hard_session0330())workshop::finds::reset_attempt();}
extern "C" void workshop_find_clear(){
    if(workshop::bonus_game0352::active())return;
    if(story_hard_session0330()){workshop::story_hard_bridge0330::clear_local();return;}
    if(story_hybrid_session0320()){
        story_lives_ready0320=false;workshop::finds::reset_attempt();workshop::story_hybrid0320::reset_run(workshop::metal_yoshis0300::view().found);return;
    }
    workshop::finds::clear_save();if(!metal_custom_scope0311()){workshop::metal_yoshis0300::clear_save();workshop::elemental_yoshis0355::clear_save();}
}
extern "C" int workshop_metal_trial_color(uint8_t* rdram,recomp_context* ctx){
    if(workshop::bonus_game0352::active()){const unsigned color=uint32_t(ctx->r5);ctx->r2=color<18?(workshop::bonus_game0352::team_available(rdram)>>color)&1u:0u;return 1;}
    if(story_hard_session0330()){const unsigned color=uint32_t(ctx->r5);ctx->r2=color<18?(workshop::story_hard_bridge0330::available().value_or(0)>>color)&1:0;return 1;}
    const unsigned color=uint32_t(ctx->r5);
    if(metal_boss_scope0354()){
        if(!workshop::yoshi_colors0300::extended(color))return 0;
        ctx->r2=(metal_available_extra0311(rdram)>>(color-8))&1;return 1;
    }
    trial_special_project0350(rdram);
    if(color<18&&(trial_special_blocked0350(rdram)&(1u<<color))){ctx->r2=0;return 1;}
    if(!workshop::yoshi_colors0300::extended(color))return 0;
    ctx->r2=(metal_available_extra0311(rdram)>>(color-8))&1;return 1;
}
extern "C" void workshop_metal_saved_lives(uint8_t* rdram,recomp_context* ctx){
    if(workshop::bonus_game0352::active()){ctx->r2=workshop::bonus_game0352::team_available(rdram)>>8;return;}
    if(story_hard_session0330()){workshop::story_hard_bridge0330::saved_extras(rdram,ctx);return;}
    if(metal_boss_scope0354()){ctx->r2=workshop::boss_metals0354::saved_lives(uint32_t(ctx->r2),workshop::metal_yoshis0300::view())|elemental_owned_extra0355();return;}
    workshop::metal_yoshis0300::set_scope(metal_solo_scope());
    if(story_hybrid_session0320()){
        const auto owned=workshop::metal_yoshis0300::view().found;
        if(!workshop::metal_native0300::continuing(rdram))workshop::story_hybrid0320::new_run(owned);
        const auto elemental=workshop::elemental_yoshis0355::story_extra(workshop::metal_native0300::continuing(rdram));
        if(!workshop::metal_native0300::continuing(rdram))workshop::elemental_yoshis0355::save_story_alive((elemental>>8)&3u);
        story_lives_ready0320=true;ctx->r2=(uint32_t(ctx->r2)&63)|(workshop::story_hybrid0320::view().metal_alive<<6)|elemental;return;
    }
    if(metal_custom_scope0311())ctx->r2=workshop::metal_custom0311::saved_lives(uint32_t(ctx->r2),workshop::metal_yoshis0300::view());
    else ctx->r2=workshop::metal_native0300::saved_lives(rdram,uint32_t(ctx->r2));
    const bool story=metal_solo_scope()&&workshop::metal_native0300::story(rdram);
    const auto elemental=story?workshop::elemental_yoshis0355::story_extra(workshop::metal_native0300::continuing(rdram)):elemental_owned_extra0355();
    if(story&&!workshop::metal_native0300::continuing(rdram))workshop::elemental_yoshis0355::save_story_alive((elemental>>8)&3u);
    ctx->r2=uint32_t(ctx->r2)|elemental;
    if(trial_special_scope0350(rdram))ctx->r2=uint32_t(ctx->r2)&~workshop::trial_special_native0350::metal_extras(trial_special_blocked0350(rdram));
}
extern "C" int workshop_metal_death(uint8_t* rdram,recomp_context* ctx){
    if(workshop::bonus_game0352::active())return workshop::bonus_game0352::team_death(rdram,ctx);
    if(story_hard_session0330())return workshop::story_hard_bridge0330::death_enter(rdram,ctx);
    if(elemental_solo_scope0355()&&workshop::yoshi_colors0300::elemental(uint32_t(ctx->r9))){
        const auto live=workshop::metal_native0300::extras(rdram)&~(1u<<(uint32_t(ctx->r9)-8));
        workshop::metal_native0300::extras(rdram,live);elemental_save_lives0355(rdram);
        ctx->r10=live;ctx->r12=MEM_W(0,int32_t(0x800F8DF4));return 1;
    }
    if(metal_boss_scope0354()){
        if(!workshop::boss_metals0354::death(rdram,uint32_t(ctx->r9)))return 0;
        ctx->r10=workshop::metal_native0300::extras(rdram);ctx->r12=MEM_W(0,int32_t(0x800F8DF4));return 1;
    }
    trial_special_death0350(rdram,uint32_t(ctx->r9));
    trial_special_project0350(rdram);
    if(story_hard_session0330())return workshop::story_hard_bridge0330::death_enter(rdram,ctx);
    if(story_hybrid_session0320()&&workshop::yoshi_colors0300::metal(uint32_t(ctx->r9))){
        const unsigned live=workshop::metal_native0300::extras(rdram)&~(1u<<(uint32_t(ctx->r9)-8));
        story_lives_ready0320=true;workshop::metal_native0300::extras(rdram,live);workshop::story_hybrid0320::save_lives((live>>6)&3);
        ctx->r10=live;ctx->r12=MEM_W(0,int32_t(0x800F8DF4));return 1;
    }
    const bool handled=metal_custom_scope0311()?workshop::metal_custom0311::death(rdram,uint32_t(ctx->r9)):
        metal_solo_scope()&&workshop::metal_native0300::death(rdram,uint32_t(ctx->r9));
    if(!handled)return 0;
    ctx->r10=workshop::metal_native0300::extras(rdram);
    ctx->r12=MEM_W(0,int32_t(0x800F8DF4));return 1;
}
extern "C" void workshop_metal_remaining(uint8_t* rdram,recomp_context* ctx){
    if(workshop::bonus_game0352::active()){ctx->r12=workshop::bonus_game0352::team_available(rdram);return;}
    if(trial_special_scope0350(rdram)){
        trial_special_project0350(rdram);
        ctx->r12=(uint32_t(ctx->r12)&0x3Fu)|(uint32_t(MEM_W(0,int32_t(0x800F8DF4)))&0xC0u)|(workshop::metal_native0300::extras(rdram)&0x3C0u);return;
    }
    if(story_hard_session0330()||metal_play_scope0311())ctx->r12|=workshop::metal_native0300::extras(rdram)&0x3C0;
}
extern "C" void workshop_metal_rescued(uint8_t* rdram,recomp_context* ctx){
    if(story_hard_session0330()){workshop::story_hard_bridge0330::rescue_enter(rdram,ctx);return;}
    if(elemental_solo_scope0355()&&workshop::yoshi_colors0300::elemental(uint32_t(ctx->r2)))workshop::elemental_yoshis0355::restore_life(uint32_t(ctx->r2));
    if(story_hybrid_session0320()){
        if(workshop::yoshi_colors0300::metal(uint32_t(ctx->r2)))workshop::story_hybrid0320::save_lives(((workshop::metal_native0300::extras(rdram)>>6)|(1u<<(uint32_t(ctx->r2)-14)))&3);
        return;
    }
    if(metal_solo_scope())workshop::metal_native0300::rescued(uint32_t(ctx->r2));
}
extern "C" void workshop_progress_boot(uint8_t* rdram,recomp_context* ctx){
    progress_payload=0;progress_reading=false;if(workshop::bonus_game0352::active()||ctx->r2!=0||story_hybrid_session0320()||story_hard_session0330())return;
    // Preserve the last valid EEPROM before a first reconciliation writes it.
    const auto original=workshop::progress::read(rdram,gpr(int32_t(0x807F2000)));
    if(!workshop::progress::valid(original,story_clears(rdram)))return;
    bool missing=false;for(unsigned c=0;c<4;c++)missing|=original.first<6 && original.columns[c]==255 && ((original.trial|story_clears(rdram))&workshop::progress::column(c))==workshop::progress::column(c);
    if(!missing)return;
    const auto source=std::filesystem::current_path()/"saves/yoshisstory.n64.us.bin";
    try {
        if(std::filesystem::file_size(source)!=2048)throw std::runtime_error("Expected normal EEPROM");
        const auto stamp=std::chrono::system_clock::now().time_since_epoch().count();
        const auto folder=std::filesystem::current_path()/"save-backups";std::filesystem::create_directories(folder);
        const auto backup=folder/("before-story-rewards-"+std::to_string(stamp)+".bin");
        std::filesystem::copy_file(source,backup);
        if(std::filesystem::file_size(backup)!=2048)throw std::runtime_error("Incomplete save backup");
        if(reconcile_story(rdram)){
            const auto saved=*ctx;func_8006988C(rdram,ctx);
            fprintf(stderr,"[Workshop] Story rewards recovered; native save result=%u, backup=%s.\n",uint32_t(ctx->r2),diagnostic_utf8_path0311(backup).c_str());*ctx=saved;
        }
    }catch(const std::exception& error){fprintf(stderr,"[Workshop] Story recovery deferred: %s.\n",error.what());}
}
extern "C" void func_80051368(uint8_t* rdram,recomp_context* ctx);
extern "C" void func_80058A14(uint8_t*,recomp_context*);
static unsigned custom_editor_room(){
    if(test_zone>=0 && !payload.empty())for(const auto& room:workshop::payload_rooms(payload))if(room.source==unsigned(test_zone))return unsigned(test_zone);
    return hybrid_room&255;
}
static workshop::fruit_mix::Random& hybrid_random(){
    static const uint32_t seed=fruit_seed?fruit_seed:std::random_device{}();
    static workshop::fruit_mix::Random random(seed);
    static bool noted=false;if(fruit_audit && !noted){fprintf(stderr,"[FruitAudit] seed=%u manualInput=1\n",seed);noted=true;}
    return random;
}
static bool heart_collection_allowed(uint8_t* rdram){
    bool allowed=!workshop::bonus_game0352::active() && payload.empty() && !workshop::custom_session::active() && !test_tally
        && uint32_t(MEM_W(0,int32_t(0x800F8B88)))==1;
    const auto selected=workshop_menu_session_mode();
    if(workshop_menu_enabled())allowed=allowed && (selected==1 || selected==3);
#ifdef WORKSHOP_NATIVE_COOP
    allowed=allowed && !workshop::boss_solo0240::enabled && !workshop::boss_campaign0250::enabled;
#endif
    return allowed;
}
extern "C" void workshop_mix_level(uint8_t* rdram,recomp_context* ctx){
    bool custom_metadata=false;
    if(!payload.empty() && workshop::fruit_mix::range(uint32_t(ctx->r19),8)){
        const auto meta=ctx->r19;const unsigned count=MEM_HU(0,meta);const uint32_t objects=MEM_W(4,meta);
        for(const auto& room:workshop::payload_rooms(payload))if(room.source==custom_editor_room() && count==room.count && workshop::fruit_mix::range(objects,count*16)){
            bool matched=true;
            for(unsigned i=0;i<count && matched;i++){const auto object=gpr(int32_t(objects+i*16));const size_t p=room.offset+size_t(i)*room.stride;matched=uint32_t(MEM_W(0,object))==workshop::black_flyguy::packed(workshop::editor_objects::packed(room.source,workshop::special_bubbles::packed(room.source,workshop::u32(payload,p)))) && uint32_t(MEM_W(4,object))==workshop::u32(payload,p+4) && uint32_t(MEM_W(8,object))==workshop::u32(payload,p+8);}
            if(matched){custom_metadata=true;fprintf(stderr,"[CustomSource] payload-verified source=%u actualRoom=%u placements=%u\n",workshop::u32(payload,8),room.source,count);}
        }
    }
    if(!workshop::fruit_mix::playable(rdram,uint32_t(ctx->r19)) && !custom_metadata){
        // Ordinary resource preloads preserve valid room bindings, while an
        // actual switch to a non-Trial/custom/Boss scope closes the observer.
        if(!heart_collection_allowed(rdram))workshop::heart_pickup0283::enter(rdram,24,hybrid_room&255,0,false);
        return;
    }
    if(test_tally && test_zone==110 && workshop::fruit_mix::playable(rdram,uint32_t(ctx->r19))){
        const auto list=gpr(int32_t(MEM_W(4,ctx->r19)));const unsigned count=MEM_HU(0,ctx->r19);
        for(unsigned i=0;i<count;i++)if(MEM_HU(i*16,list)==0x46AE || MEM_HU(i*16,list)==0x46AA){
            MEM_W(i*16+4,list)=std::bit_cast<uint32_t>(200.f);MEM_W(i*16+8,list)=std::bit_cast<uint32_t>(120.f);
        }
        // Replace one existing coin record, retaining the native placement
        // loader and allocation size, to supply the thirtieth test fruit.
        for(unsigned i=0;i<count;i++)if(MEM_HU(i*16,list)==0x406F){
            MEM_H(i*16,list)=0x4030;
            MEM_W(i*16+4,list)=std::bit_cast<uint32_t>(240.f);MEM_W(i*16+8,list)=std::bit_cast<uint32_t>(132.f);
            break;
        }
    }
    hybrid_spawn_choices.clear();
    hybrid_visits.enter();
    hybrid_level_active=payload.empty() && workshop::fruit_mix::playable(rdram,uint32_t(ctx->r19));
#ifdef WORKSHOP_NATIVE_COOP
    if(workshop::boss_solo0240::enabled || workshop::boss_campaign0250::enabled)hybrid_level_active=false;
#endif
    workshop::hybrid_mode::enter(rdram,uint32_t(ctx->r19),hybrid_room);
    workshop::collections::enter(rdram,hybrid_room&255,uint32_t(ctx->r19),payload.empty()&&!workshop::bonus_game0352::active());
    {
        const bool allowed=heart_collection_allowed(rdram);
        const unsigned level=unsigned(MEM_BU(0,int32_t(0x800F8DFC)))*4+unsigned(MEM_BU(0,int32_t(0x800F8DFD)));
        workshop::heart_pickup0283::enter(rdram,level,hybrid_room&255,uint32_t(ctx->r19),allowed);
    }
    if(story_hard_session0330())workshop::story_hard_bridge0330::enter(rdram,hybrid_room&255);
    if(!workshop::bonus_game0352::active()&&!story_hard_session0330()&&workshop::fruit_mix::playable(rdram,uint32_t(ctx->r19)))workshop::finds::enter(
        unsigned(MEM_BU(0,int32_t(0x800F8DFC)))*4+unsigned(MEM_BU(0,int32_t(0x800F8DFD))),hybrid_room&255,
        payload.empty() && uint32_t(MEM_W(0,int32_t(0x800F8B88)))<2,trial_special_recovery_metals0350(rdram));
    find_binding.reset();find_sources.clear();
    if(!workshop::bonus_game0352::active()&&!story_hard_session0330()&&hybrid_level_active && uint32_t(MEM_W(0,int32_t(0x800F8B88)))<2){
        const auto p=workshop::progress::read(rdram,gpr(int32_t(0x807F2000)));
        const auto story=story_clears(rdram);
        if(workshop::progress::valid(p,story)){
            bool whitePresent=false;const auto saved=*ctx;
            if(MEM_W(0,int32_t(0x800F8B88))==0){func_80058A14(rdram,ctx);whitePresent=ctx->r2!=0;*ctx=saved;}
            const auto list=gpr(int32_t(MEM_W(4,ctx->r19)));const unsigned count=MEM_HU(0,ctx->r19);
            for(unsigned i=0;i<count;i++){
                const auto record=gpr(ADD32(list,i*16));const auto kind=MEM_HU(0,record);
                if(kind!=0x46AA && kind!=0x46AE)continue;
                const auto x=uint32_t(MEM_W(4,record)),y=uint32_t(MEM_W(8,record));
                if(auto color=workshop::finds::reserve(story_hybrid_session0320()?workshop::story_hybrid0320::view().eligible:workshop::progress::unlocked(p,story),kind,x,y,whitePresent)){
                    find_binding=FindSource{uint32_t(record),kind,x,y,*color};MEM_H(0,record)=0x4692;
                    ctx->r4=0x4692;func_80051368(rdram,ctx);*ctx=saved;break;
                }
            }
        }
    }
    if(!hybrid_level_active){
        bool preload_custom_fruits=custom_metadata;
#ifdef WORKSHOP_NATIVE_COOP
        preload_custom_fruits=preload_custom_fruits||workshop::boss_campaign0250::active();
#endif
        if(preload_custom_fruits){const auto saved=*ctx;for(unsigned i=0;i<6;i++){ctx->r4=0x4F00+i;func_80051368(rdram,ctx);}*ctx=saved;}
        return;
    }
    const unsigned threshold=workshop::hybrid_mode::fruit_threshold();
    const auto stats=hybrid_visits.apply(rdram,uint32_t(ctx->r19),hybrid_room,[](){return hybrid_random().ticket();},threshold);
    const auto saved=*ctx;
    for(unsigned i=0;i<6;i++){ctx->r4=0x4F00+i;func_80051368(rdram,ctx);}
    *ctx=saved;
    fprintf(stderr,"[Workshop] Hybrid fruit mix enabled (%u%% total); mode %u; six resources preloaded.\n",threshold*100/60,uint32_t(MEM_W(0,int32_t(0x800F8B88))));
    fprintf(stderr,"[Workshop] Placed fruit mix: %u eligible, %u changed; visit-key=%08X.\n",stats.eligible,stats.changed,hybrid_room);
}
extern "C" void workshop_mix_parent_created(uint32_t parent){hybrid_spawn_choices.created(parent);if(fruit_audit)audited_fruit_draws.erase(parent);}
extern "C" void workshop_prepare_fruit_resource(uint8_t* rdram,recomp_context* ctx){
    const auto actor=uint32_t(ctx->r4),requested=uint32_t(ctx->r5)>>16;
    if(workshop::fruit_resources::prepare(rdram,actor,uint16_t(requested)) && fruit_audit)
        fprintf(stderr,"[FruitAudit] rebind vi=%u actor=%08X requested=%04X\n",test_vi.load(),actor,requested);
}
extern "C" void workshop_finish_fruit_resource(uint8_t* rdram,recomp_context* ctx){
    const auto actor=uint32_t(MEM_W(0x28,ctx->r29));
    if(workshop::fruit_resources::finish(rdram,actor) && fruit_audit)
        fprintf(stderr,"[FruitAudit] render-rebind vi=%u actor=%08X\n",test_vi.load(),actor);
}
extern "C" uint32_t workshop_mix_spawn(uint8_t* rdram,recomp_context* ctx){
    const auto id=uint32_t(ctx->r5);
#ifdef WORKSHOP_NATIVE_COOP
    if(auto chosen=workshop::boss_tree_hybrid0260::mix_override(rdram,ctx,uint16_t(id)))return *chosen;
#endif
    const auto custom_parent=workshop::fruit_mix::range(uint32_t(ctx->r17),4)?uint32_t(MEM_W(0,ctx->r17)):0;
    if(workshop::fruit_resources::world(uint16_t(id)))if(auto chosen=workshop::special_bubbles::fruit(custom_parent)){
        if(fruit_audit)fprintf(stderr,"[SpecialBubble] parent=%08X requested=%04X actual=%04X\n",custom_parent,id,*chosen);
        return *chosen;
    }
    if(!hybrid_level_active || !workshop::fruit_mix::eligible(id))return id;
    const uint32_t parent=ram_pointer(uint32_t(ctx->r17))?uint32_t(MEM_W(0,ctx->r17)):0;
    const bool preview=ram_pointer(parent) && workshop::fruit_mix::preview_source(MEM_HU(12,int32_t(parent)));
    const auto result=preview?(hybrid_visits.actors.contains(parent)
        ?hybrid_visits.pick(id,parent,[](){return hybrid_random().ticket();},workshop::hybrid_mode::fruit_threshold())
        :hybrid_spawn_choices.pick(id,parent,[](){return hybrid_random().ticket();},workshop::hybrid_mode::fruit_threshold()))
        :workshop::fruit_mix::replacement(id,hybrid_random().ticket(),workshop::hybrid_mode::fruit_threshold());
    if(result!=id)fprintf(stderr,"[Workshop] Hybrid replacement %04X -> %04X.\n",id,result);
    return result;
}
extern "C" int workshop_use_special_hud(){return special_hud;}
extern "C" void workshop_807F1300(uint8_t*,recomp_context*);
extern "C" unsigned workshop_coop_filter_available(unsigned mask){
    if(workshop::bonus_game0352::active())return workshop::bonus_game0352::team_available(diagnostic_rdram);
    if(story_hard_session0330())return workshop::story_hard_bridge0330::available().value_or(0);
#ifdef WORKSHOP_NATIVE_COOP
    if(test_coop_native)return workshop::coop_bridge::available_mask(mask&0x3FFFu);
    if(metal_boss_scope0354())return mask&(0x3FFFu|(metal_available_extra0311(diagnostic_rdram)<<8));
    if(workshop::boss_solo0240::enabled||workshop::boss_campaign0250::enabled)return mask&0x3FFFu;
#endif
    trial_special_project0350(diagnostic_rdram);
    return mask&~trial_special_blocked0350(diagnostic_rdram);
}
extern "C" void workshop_pause_score_lives(uint8_t* rdram,recomp_context* ctx){
    const auto saved=*ctx;
    workshop_807F1300(rdram,ctx);
    const auto alive=uint32_t(ctx->r2)&((story_hard_session0330()||metal_play_scope0311())?0x3FFFFu:0x3FFFu);
    *ctx=saved;ctx->r2=std::popcount(alive);
    static unsigned previous=~0u;
    if(fruit_audit && previous!=ctx->r2){previous=unsigned(ctx->r2);fprintf(stderr,"[ScoreAudit] pause vi=%u livingYoshis=%u mask=%04X\n",test_vi.load(),previous,alive);}
}
extern "C" void workshop_end_score_lives(uint8_t* rdram,recomp_context* ctx){
    if(workshop::bonus_game0352::active()){ctx->r5=0;return;}
    if(story_hard_session0330()){if(!workshop::story_hard_bridge0330::end_tally(rdram,ctx))ctx->r5=std::popcount(workshop::story_hard_bridge0330::available().value_or(0));return;}
    const auto saved=*ctx;
    workshop_807F1300(rdram,ctx);
    const auto alive=uint32_t(ctx->r2)&((story_hard_session0330()||metal_play_scope0311())?0x3FFFFu:0x3FFFu);
    *ctx=saved;
    const auto p=workshop::progress::read(rdram,gpr(int32_t(0x807F2000)));
    const auto finds=workshop::finds::view();
    ctx->r5=workshop::progress::tally_lives(p,story_clears(rdram),MEM_W(0,int32_t(0x800F8B88))!=0,
        finds.initialized && finds.readable && finds.migrated?6:MEM_BU(0,int32_t(0x800F8DFC)),MEM_BU(0,int32_t(0x800F8DFD)),
        alive|(workshop::finds::pending()<<8),uint32_t(MEM_W(0,int32_t(0x800F8DF4))),metal_play_scope0311()?0x3FFFFu:0x3FFFu);
    if(story_hybrid_session0320())ctx->r5=std::popcount((alive|(workshop::finds::pending()<<8)|((uint32_t(MEM_W(0,int32_t(0x800F8DF4)))>>4)&192))&0x3FFFFu);
    const unsigned pending_metals0350=workshop::finds::pending_metals()<<14;
    const unsigned pending_bw0350=workshop::trial_special_native0350::carried_bw(uint32_t(MEM_W(0,int32_t(0x800F8DF4))));
    const bool find_finished0350=workshop::finds::finish(rdram);
    trial_special_recover0350(rdram,pending_bw0350|(find_finished0350?pending_metals0350:0u));
    if(find_finished0350&&story_hybrid_session0320()){
        workshop::story_hybrid0320::note_acquired(workshop::finds::view().found);
        workshop::story_hybrid0320::prepare(5,workshop::finds::view().found,workshop::metal_yoshis0300::view().found);
        workshop::story_hybrid0320::save_lives((workshop::metal_native0300::extras(rdram)>>6)&3);
    }
    workshop::custom_session::finish(rdram);
    if(fruit_audit)fprintf(stderr,"[ScoreAudit] end vi=%u livingYoshis=%u resultYoshis=%u bonus=%u\n",test_vi.load(),std::popcount(alive),unsigned(ctx->r5),unsigned(ctx->r5)*100);
}
extern "C" void func_807B634C(uint8_t*,recomp_context*);
extern "C" void func_8004613C(uint8_t*,recomp_context*);
extern "C" void func_80047F9C(uint8_t*,recomp_context*);
extern "C" void func_8003B540(uint8_t*,recomp_context*);
extern "C" void func_8003BB94(uint8_t*,recomp_context*);
extern "C" void func_80044194(uint8_t*,recomp_context*);
extern "C" void workshop_pause_create(uint8_t* rdram,recomp_context* ctx){
    const auto incoming=*ctx;const auto parent=ctx->r4;
    func_807B634C(rdram,ctx);const auto originalReturn=*ctx;
    const auto data=MEM_W(0x1F4,parent);
    if(workshop::favorites::pointer(uint32_t(data))){
        for(unsigned slot=1;slot<4;slot++)MEM_W(workshop::favorites::pointerOffset(slot),data)=0;
        const auto favorites=workshop::favorites::list(uint32_t(MEM_W(0,int32_t(0x807F0008))));
        for(unsigned slot=1;slot<favorites.count;slot++){
            *ctx=incoming;ctx->r4=0x402C0201+slot;ctx->r5=0x42D00000;ctx->r6=gpr(int32_t(0xC2000000));ctx->r7=gpr(int32_t(0xBB03126F));
            func_8004613C(rdram,ctx);const auto child=ctx->r2;
            if(!workshop::favorites::pointer(uint32_t(child)))continue;
            MEM_W(workshop::favorites::pointerOffset(slot),data)=child;MEM_B(0x31,child)=1;
            *ctx=incoming;ctx->r4=parent;ctx->r5=child;func_80047F9C(rdram,ctx);
        }
        fprintf(stderr,"[Workshop] Pause favorites: Yoshi %u, %u icons.\n",uint32_t(MEM_W(0,int32_t(0x807F0008))),favorites.count);
    }
    *ctx=originalReturn;
}
extern "C" void workshop_pause_position(uint8_t* rdram,recomp_context* ctx){workshop::favorites::layout(rdram,MEM_W(0x20,ctx->r29));}
extern "C" int workshop_pause_frame(uint8_t* rdram,recomp_context* ctx){return workshop::favorites::frame(rdram,ctx->r4);}
extern "C" void workshop_pause_hide(uint8_t* rdram,recomp_context* ctx){workshop::favorites::hide(rdram,ctx->r4);}
extern "C" void workshop_native_fault(uint8_t* rdram,recomp_context* ctx){
    std::string file;
    if(ram_pointer(uint32_t(ctx->r4)))for(unsigned i=0;i<255;i++){auto c=MEM_BU(i,ctx->r4);if(!c)break;file.push_back(char(c));}
    fprintf(stderr,"[Workshop] Native assertion: %s:%u caller=%08X sp=%08X\n",file.c_str(),uint32_t(ctx->r5),uint32_t(ctx->r31),uint32_t(ctx->r29));
    workshop_dump_trace();
#ifdef WORKSHOP_NATIVE_COOP
    if(test_coop_room_fixture0280 && !test_inputs.empty() && end_test_vi){
        // Bounded QA: retain the actual assertion RAM before the native
        // Fault_HungUp loop. The failure remains explicit and cannot pass.
        {std::ofstream dump("coop-room-assert.rdram",std::ios::binary);dump.write(reinterpret_cast<const char*>(rdram),0x800000);}
        fprintf(stderr,"[CoopRooms] assertion-captured vi=%u bytes=8388608 forcedGameState=0\n",test_vi.load());
        std::fflush(nullptr);std::_Exit(2);
    }
#endif
    if(background_test){
        {std::ofstream dump("native-fault.rdram",std::ios::binary);dump.write(reinterpret_cast<char*>(rdram),0x900000);}
        std::fflush(nullptr);std::_Exit(2);
    }
}
extern "C" int workshop_resource_slot(uint8_t* rdram,recomp_context* ctx,int match){return workshop::cache::find_slot(rdram,ctx,match!=0);}
#ifdef WORKSHOP_NATIVE_COOP
extern "C" void battle_original_func_80050C8C(uint8_t*,recomp_context*);
extern "C" void func_80050C8C(uint8_t* rdram,recomp_context* ctx){workshop::boss_lips_terrain_alias0260::runtime::dispatch(rdram,ctx,battle_original_func_80050C8C);}
#endif
extern "C" void workshop_test_level_start(uint8_t* rdram,recomp_context*){
    if(!test_tally)return;
    MEM_W(0,gpr(int32_t(0x800F89D0)))=29;
    for(unsigned i=0;i<30;i++){
        MEM_W(i*4,gpr(int32_t(0x800F89D4)))=test_special_victory>=0?6+test_special_victory:0;
        MEM_H(i*2,gpr(int32_t(0x800F8AD4)))=test_special_victory>=0?6+test_special_victory:0;
    }
    fprintf(stderr,"[Workshop] Tally fixture: 29 fruits seeded at VI %u; only disposable test profile.\n",test_vi.load());
}
extern "C" void workshop_palette_note(uint8_t* rdram,recomp_context* ctx,int kind){
    if(!test_tally)return;
    const auto actor=ctx->r4;
    if(!ram_pointer(uint32_t(actor)))return;
    const auto id=MEM_HU(12,actor);
    const auto palette=kind==0?gpr(int32_t(MEM_W(0x10,ctx->r29))):ctx->r5;
    if(!ram_pointer(uint32_t(palette)))return;
    static std::array<uint32_t,2> last{};
    uint32_t hash=2166136261;
    for(unsigned i=0;i<512;i++)hash=(hash^MEM_BU(i,palette))*16777619;
    if(last[kind]==hash)return;last[kind]=hash;
    const auto color=MEM_W(0,gpr(int32_t(0x807F0008)));
    fprintf(stderr,"[Workshop] Palette VI %u kind=%d actor=%04X display=%d pointer=%08X hash=%08X frame=%u ending=%u\n",test_vi.load(),kind,id,color,uint32_t(palette),hash,uint32_t(ctx->r7),uint32_t(MEM_W(0,gpr(int32_t(0x800F8AC8)))));
    const std::string name="palette-"+std::to_string(test_vi.load())+"-"+std::to_string(kind)+"-"+std::to_string(id)+".bin";
    std::array<uint8_t,512> bytes;for(unsigned i=0;i<512;i++)bytes[i]=MEM_BU(i,palette);
    std::ofstream file(name,std::ios::binary);file.write(reinterpret_cast<char*>(bytes.data()),bytes.size());
}
extern "C" void func_80574060(uint8_t*,recomp_context*);
extern "C" void func_804011D8(uint8_t*,recomp_context*);
static void custom_victory_fixture(uint8_t* rdram,recomp_context* ctx){
    // Explicit, finite diagnostics only: seed29 is supplied by test_level_start;
    // consume the real custom-world fruit through the native callback.
    static bool consumed=false;static unsigned popped=0;
    if(test_special_victory>=0 && !popped && test_vi.load()>2400 && test_vi.load()%30==0){
        for(auto [parent,variant]:workshop::special_bubbles::actors)if(variant==unsigned(test_special_victory) && workshop::fruit_mix::range(parent,0x200) && MEM_BU(0x18,int32_t(parent))==1){
            const auto saved=*ctx;ctx->r29=ADD32(ctx->r29,-0x40);ctx->r4=gpr(int32_t(parent));func_804011D8(rdram,ctx);*ctx=saved;popped=test_vi.load();
            fprintf(stderr,"[SpecialBubbleTest] native-pop parent=%08X fruit=%u vi=%u\n",parent,variant,popped);break;
        }
    }
    if(test_special_victory>=0)return; // The real scripted tongue now performs the collection.
    if(!test_tally || (!workshop::custom_session::active() && test_special_victory<0) || consumed || test_vi.load()<2500 || test_vi.load()%30 || MEM_W(0,int32_t(0x800F89D0))!=29)return;
    for(uint32_t p=0x80000400;p<0x80400000-0x200;p+=4){
        const auto a=gpr(int32_t(p));
        if(uint32_t(MEM_W(0,a))!=0x19971110 || MEM_HU(12,a)!=(test_special_victory>=0?0x4F00+test_special_victory:0x4030) || (test_special_victory<0?MEM_BU(0x18,a)!=1:(MEM_BU(0x18,a)!=2 || MEM_W(0x180,a)!=0)))continue;
        consumed=true;const auto saved=*ctx;ctx->r29=ADD32(ctx->r29,-0x30);ctx->r4=a;ctx->r5=0x66;func_80574060(rdram,ctx);*ctx=saved;
        fprintf(stderr,"[CustomSessionTest] native-thirtieth-fruit actor=%08X count=%u ending=%u\n",p,uint32_t(MEM_W(0,int32_t(0x800F89D0))),uint32_t(MEM_W(0,int32_t(0x800F8AC8))));break;
    }
}
static void test_find_frame(uint8_t*,recomp_context*);
static void test_editor_eggs_frame(uint8_t*,recomp_context*);
extern "C" void workshop_trace_entry(uint8_t* rdram,recomp_context* ctx,const char* name){
    if(std::string_view(name)=="func_80045CAC")elemental_contacts0355.created(uint32_t(ctx->r4));
    if(std::string_view(name)=="func_800439F4"||std::string_view(name)=="func_80045B64")elemental_contacts0355.forget(uint32_t(ctx->r4));
    if(std::string_view(name)=="func_8006DA0C")elemental_contacts0355.clear_pending();
    if(workshop::bonus_game0352::active()&&workshop::bonus_game0352::observes_trace(name)){
        workshop::bonus_game0352::frame_context(test_vi.load(),section_addresses,286);
        workshop::bonus_game0352::trace(rdram,ctx,name);
    }
#ifdef WORKSHOP_NATIVE_COOP
    starting_bonus.trace(rdram,ctx,name,starting_bonus_environment(rdram));
#endif
    // Observe native provenance before existing cleanup hooks forget bindings.
    workshop::heart_pickup0283::trace(rdram,ctx,name,{
        uint32_t(section_addresses[134]),uint32_t(section_addresses[220]),
        uint32_t(section_addresses[227]),uint32_t(MEM_W(0,int32_t(0x800F8828)))});
    for(const auto& receipt:workshop::heart_pickup0283::drain()){
        if(workshop::bonus_game0352::active())continue;
        const auto result=workshop::heart_shop0283::wallet().claim(receipt.level,receipt.slot);
        fprintf(stderr,"[HeartWallet] collect level=%u slot=%u room=%u actor=%08X birth=%llu status=%u balance=%u nativeReceipt=1\n",
            receipt.level,receipt.slot,receipt.room,receipt.actor,(unsigned long long)receipt.birth,unsigned(result.status),result.balance);
    }
    if(std::string_view(name)=="func_800439F4" || std::string_view(name)=="func_80045B64")
        workshop::heart_pickup0283::forget(uint32_t(ctx->r4));
    if(std::string_view(name)=="func_80045CAC")workshop::heart_pickup0283::created(uint32_t(ctx->r4));
#ifdef WORKSHOP_NATIVE_COOP
    // Observe the old room before the normal entry hook updates hybrid_room.
    if(test_coop_native){
        try{workshop::coop_rooms_native0280::trace(rdram,ctx,name,test_vi.load(),hybrid_room&255,uint32_t(section_addresses[144]),uint32_t(section_addresses[145]));}
        catch(...){workshop::coop_rooms_native0280::host_error();}
        if(test_coop_room_fixture0280 && end_test_vi && !test_inputs.empty() && !test_inputs_p2.empty()){
            namespace rooms=workshop::coop_rooms_native0280;
            // Bounded QA evidence only: preserve the native providers exactly
            // after both carries, and capture a rejected transition before any
            // subsequent owner scope. These dumps never change guest state.
            const auto epoch=rooms::coordinator.epoch();
            if(std::string_view(name)=="func_8005D320" && epoch>=1 && epoch<=2 && rooms::coordinator.carried()==3){
                const auto file=std::string("coop-room-departure-")+std::to_string(epoch)+".rdram";
                {std::ofstream dump(file,std::ios::binary);dump.write(reinterpret_cast<const char*>(rdram),0x800000);}
                fprintf(stderr,"[CoopRooms] departure-captured epoch=%llu vi=%u bytes=8388608 forcedGameState=0\n",(unsigned long long)epoch,test_vi.load());
            }
            if(rooms::failed){
                {std::ofstream dump("coop-room-rejected.rdram",std::ios::binary);dump.write(reinterpret_cast<const char*>(rdram),0x800000);}
                fprintf(stderr,"[CoopRooms] rejection-captured vi=%u bytes=8388608 forcedGameState=0\n",test_vi.load());
                std::fflush(nullptr);std::_Exit(2);
            }
        }
    }
#endif
    // Arena release can follow private/collision cleanup. Forget its logical
    // bindings before any preparation that would inspect the old controller.
    if(std::string_view(name)=="func_800439F4"){
        if(story_hard_session0330())workshop::story_hard_bridge0330::forget(uint32_t(ctx->r4));
        workshop::editor_objects::forget(uint32_t(ctx->r4));
        workshop::black_flyguy::forget(uint32_t(ctx->r4));
        workshop::passages::forget(uint32_t(ctx->r4));
#ifdef WORKSHOP_NATIVE_COOP
        workshop::coop_bridge::forget(uint32_t(ctx->r4));
        workshop::boss_native::forget(uint32_t(ctx->r4));
        workshop::boss_campaign0250::forget(uint32_t(ctx->r4));
        workshop::boss_tree0240::forget(uint32_t(ctx->r4));
#endif
    }else workshop::black_flyguy::prepare(rdram,uint32_t(ctx->r4));
    if(std::string_view(name)=="func_80047F9C")workshop::black_flyguy::attached(rdram,ctx);
    if(std::string_view(name)=="func_80045B64"){
#ifdef WORKSHOP_NATIVE_COOP
        workshop::coop_collision::forgotten(uint32_t(ctx->r4));
#endif
        if(story_hard_session0330())workshop::story_hard_bridge0330::dispose(rdram,uint32_t(ctx->r4));
        workshop::editor_objects::dispose(rdram,uint32_t(ctx->r4));
        workshop::black_flyguy::destroy(rdram,uint32_t(ctx->r4));
        workshop::passages::destroy(rdram,uint32_t(ctx->r4));
    }
    if(test_find_reveal && test_find_egg && uint32_t(ctx->r4)==test_find_egg){
        static unsigned calls=0;
        if(calls++<24)fprintf(stderr,"[HiddenEggTest] call=%s actor=%08X\n",name,test_find_egg);
    }
    if(fruit_audit && (std::string_view(name)=="func_806F616C" || std::string_view(name)=="func_8079AC9C"))
        fprintf(stderr,"[HybridFindAudit] gate %s actor=%08X\n",name,uint32_t(ctx->r4));
    // Private QA enters the actual requested room. Aliasing its descriptor
    // onto another scene would leave native room/physics identity inconsistent.
    if(hard_preview0330&&story_hard_session0330()&&workshop::story_hard_bridge0330::active()&&!hard_qa_room_routed0330&&test_zone>=0&&test_stage>=0&&std::string_view(name)=="func_8006E0FC"&&ctx->r4==0&&ctx->r5==7&&uint32_t(ctx->r6)==workshop::vanilla_sources[unsigned(test_stage)]){
        ctx->r6=unsigned(test_zone);hard_qa_room_routed0330=true;
        fprintf(stderr,"[StoryHardQA0330] actual-room route=%u stage=7 descriptorAlias=0\n",unsigned(test_zone));fflush(stderr);
    }
    if(end_test_vi && std::string_view(name)=="func_8006E0FC")
        fprintf(stderr,"[NavigationAudit] vi=%u transition=%u stage=%u scene=%u caller=%08X titleBase=%08X\n",test_vi.load(),unsigned(ctx->r4),unsigned(ctx->r5),unsigned(ctx->r6),uint32_t(ctx->r31),uint32_t(section_addresses[32]));
    if(fruit_audit && (std::string_view(name)=="func_80064920" || std::string_view(name)=="func_80064AD4" || std::string_view(name)=="func_80051DA0")){
        static unsigned last=0;
        if(test_vi.load()-last>=30){
            last=test_vi.load();
            fprintf(stderr,"[PadAudit] vi=%u function=%s boot=%u imem=%08X cur=%04X cache=%04X pressed=%04X main=%04X mode=%u\n",last,name,
                uint32_t(MEM_W(0,int32_t(0x800A5AD0))),uint32_t(MEM_W(0,int32_t(0x80108624))),
                MEM_HU(0,int32_t(0x8010DEE0)),MEM_HU(0,int32_t(0x8010E170)),MEM_HU(12,int32_t(0x8010E170)),MEM_HU(0,int32_t(0x800F9390)),MEM_BU(0,int32_t(0x800F7EB4)));
        }
    }
    if(std::string_view(name)=="func_8006DA0C"){
        hybrid_room=workshop::fruit_mix::visit_room(rdram,unsigned(ctx->r5));
#ifdef WORKSHOP_NATIVE_COOP
        workshop::boss_solo0240::enter_room(unsigned(ctx->r5)&255);
        const auto native_requested=unsigned(ctx->r5)&255;
        if(!workshop::boss_fish_cp6_fixture0284::enter_room(native_requested))workshop::boss_campaign0250::enter_room(native_requested);
        if(boss_test_blargg0289 && native_requested==52){
            namespace campaign=workshop::boss_campaign0250;
            const auto& state=campaign::session.snapshot();
            const workshop::boss_blargg_boot0289::Frame frame{boss_test_blargg0289,state.diagnostic,state.phase,native_requested,state.index,campaign::physical_room,campaign::logical_room,state.retry_epoch,state.completed,test_vi.load(),state.player};
            const auto result=boss_blargg_boot_router.redirect(rdram,ctx,frame);
            if(result.rejected){fprintf(stderr,"[BossBlarggQA] cold-cache-rejected reason=%s vi=%u\n",result.reason,frame.vi);campaign::fail("blargg_initial_cache_rejected");}
            else if(result.redirected)fprintf(stderr,"[BossBlarggQA] cold-cache-redirect from=%u to=%u caller=%08X vi=%u guestMemoryWrites=0\n",native_requested,unsigned(ctx->r5),uint32_t(ctx->r31),frame.vi);
        }
#endif
        workshop::editor_objects::enter_room(custom_editor_room());workshop::black_flyguy::enter_room(custom_editor_room());workshop::passages::enter_room(custom_editor_room());
        if(end_test_vi || workshop::custom_session::active())fprintf(stderr,"[CustomSource] requested=%u actualRoom=%u page=%u column=%u\n",payload.empty()?999:workshop::u32(payload,8),unsigned(ctx->r5)&255,unsigned(MEM_BU(0,int32_t(0x800F8DFC))),unsigned(MEM_BU(0,int32_t(0x800F8DFD))));
    }
    if(std::string_view(name)=="func_805AB914")workshop::passages::prepare(rdram,uint32_t(ctx->r4));
    if(std::string_view(name)=="func_8005D320")workshop::passages::departing(rdram,ctx);
    if(std::string_view(name)=="func_8006DD34"){
        workshop::passages::room_ready(rdram,ctx);
#ifdef WORKSHOP_NATIVE_COOP
        workshop::boss_solo0240::room_ready(rdram,ctx);
        if(!workshop::boss_fish_cp6_fixture0284::room_ready(rdram,ctx,test_vi.load()))workshop::boss_campaign0250::room_ready(rdram,ctx);
#endif
    }
    if(std::string_view(name)=="func_80400068"){workshop::editor_objects::begin_child(ctx);if(story_hard_session0330())workshop::story_hard_bridge0330::begin_child(ctx);}
    if(std::string_view(name)=="func_8059EC8C"){workshop::editor_objects::prepare_release(rdram,uint32_t(ctx->r4));if(story_hard_session0330())workshop::story_hard_bridge0330::prepare_release(rdram,uint32_t(ctx->r4));}
    if(std::string_view(name)=="func_8004535C"){workshop::editor_objects::restore(rdram,uint32_t(ctx->r4));if(story_hard_session0330())workshop::story_hard_bridge0330::restore(rdram,uint32_t(ctx->r4));}
    if(std::string_view(name)=="func_80045CAC"){
#ifdef WORKSHOP_NATIVE_COOP
        workshop::coop_collision::created(uint32_t(ctx->r4));
        workshop::coop_native::created(ctx);
        workshop::coop_classic_flower0250::forget(uint32_t(ctx->r4));
        workshop::boss_native::forget(uint32_t(ctx->r4));
        workshop::boss_campaign0250::forget(uint32_t(ctx->r4));
        workshop::boss_tree0240::forget(uint32_t(ctx->r4));
#endif
        workshop::special_bubbles::created(ctx,hybrid_room);
        if(story_hard_session0330())workshop::story_hard_bridge0330::created(rdram,ctx);
        workshop::editor_objects::created(rdram,ctx,custom_editor_room());workshop::black_flyguy::created(ctx,custom_editor_room());workshop::passages::created(ctx,custom_editor_room());
        workshop::reward_popup::created(uint32_t(ctx->r4));
        find_sources.erase(uint32_t(ctx->r4));
        if(find_binding && (uint32_t(ctx->r5)>>16)==0x4692 && uint32_t(ctx->r6)==find_binding->x && uint32_t(ctx->r7)==find_binding->y)
            find_sources[uint32_t(ctx->r4)]=find_binding->color;
        workshop::finds::created(uint32_t(ctx->r4));
        if(find_spawning>=0 && (uint32_t(ctx->r5)>>16)==0x401D)workshop::finds::spawned(uint32_t(ctx->r4),unsigned(find_spawning),find_spawning_scene);
        workshop::collections::created(rdram,ctx);
        workshop::hybrid_mode::created(rdram,ctx,hybrid_room);
        const auto actor=uint32_t(ctx->r4),kind=uint32_t(ctx->r5)>>16;
        hybrid_visits.actors.erase(actor);
        if(hybrid_level_active && workshop::fruit_mix::preview_source(uint16_t(kind))
            && workshop::fruit_mix::range(uint32_t(ctx->r29),0x14)){
            hybrid_visits.created(actor,{hybrid_room,kind,uint32_t(ctx->r6),uint32_t(ctx->r7),uint32_t(MEM_W(0x10,ctx->r29))});
        }
    }
#ifdef WORKSHOP_NATIVE_COOP
    const unsigned boss_source=workshop::boss_solo0240::enabled?workshop::boss_solo0240::effective_room(hybrid_room&255):(test_zone==162 && test_stage==8 && (hybrid_room&255)==workshop::vanilla_sources[8]?162:(hybrid_room&255));
    workshop::boss_tree0240::trace(rdram,ctx,name);
    if(workshop::boss_campaign0250::enabled){
        auto& sections=workshop::boss_campaign0250::overlays;for(unsigned n:{16u,90u,117u,120u,126u,138u,144u,148u,156u,167u,189u,197u,220u,285u,293u})sections[n]=uint32_t(section_addresses[n]);
        // Read-only snapshots scoped to the exact bounded Genie-pot QA.
        if(boss_test_goal=="transition-rest" && boss_test_case==7 && end_test_vi && !test_inputs.empty()){
            namespace n=workshop::boss_native;
            const auto phase=workshop::boss_campaign0250::session.snapshot().phase;
            const auto fn=std::string_view(name);
            const unsigned bit=fn=="func_8005D320"?1u:fn=="func_80039828"&&phase==workshop::boss_battle0250::Phase::ToRest?2u:0u;
            static unsigned observed=0;
            if(bit && !(observed&bit)){
                observed|=bit;
                const auto player=workshop::boss_campaign0250::session.snapshot().player;
                const auto priv=n::range(player,0x1FC)?n::word(rdram,player+0x1F4):0;
                const auto manager=n::word(rdram,0x800F8810);
                const auto table=n::range(manager,0x14)?n::word(rdram,manager+0x10):0;
                const auto slot=n::range(table,0x18)?n::word(rdram,table+0x14):0;
                const auto reticle=n::range(priv,0x1F8)?n::word(rdram,priv+0x1F4):0;
                const auto file=std::string("genie-pot-")+(bit==1?"departure":"reticle-update")+".rdram";
                {std::ofstream dump(file,std::ios::binary);dump.write(reinterpret_cast<const char*>(rdram),0x800000);}
                fprintf(stderr,"[BossPotAimQA] fn=%s phase=%u vi=%u player=%08X private=%08X registrySlot5=%08X reticle=%08X actor=%08X caller=%08X bytes=8388608 readOnly=1\n",name,unsigned(phase),test_vi.load(),player,priv,slot,reticle,uint32_t(ctx->r4),uint32_t(ctx->r31));
            }
        }
        workshop::boss_campaign0250::trace(rdram,ctx,name,test_vi.load(),sections.data());
        if(std::string_view(name)=="func_80045CAC")workshop::boss_campaign0250::blargg_created(uint32_t(ctx->r4));
        if(std::string_view(name)=="func_800439F4"||std::string_view(name)=="func_80045B64")workshop::boss_campaign0250::blargg_forgotten(uint32_t(ctx->r4));
        // Read-only native Flame Unit sampling for the isolated Blargg QA.
        // Actual callback entry proves execution; no native state is advanced here.
        if(boss_test_blargg0289 && end_test_vi && !test_inputs.empty() && boss_blargg_flame_probe_count<600){
            namespace n=workshop::boss_native;namespace campaign=workshop::boss_campaign0250;
            const auto fn=std::string_view(name);const auto& state=campaign::session.snapshot();
            const bool wanted=fn=="func_8057E620"||fn=="func_8057E7D4"||fn=="func_8057E888"||fn=="func_8057E934";
            const auto actor=uint32_t(ctx->r4);const auto vi=test_vi.load();
            if(wanted && state.diagnostic && state.phase==workshop::boss_battle0250::Phase::Fighting && campaign::logical_room==19 && campaign::physical_room==167+state.index && n::actor(rdram,actor,0x41D0)){
                const auto data=n::word(rdram,actor+0x1F4),resource=n::word(rdram,actor+0x168);
                if(n::range(data,0x2D0) && uint32_t(ctx->r5)==data && n::range(resource,0x48)){
                    const auto callback=n::word(rdram,actor+0x20),step=n::word(rdram,data+0x2C8);
                    if(actor!=boss_blargg_flame_probe_actor || callback!=boss_blargg_flame_probe_callback || step!=boss_blargg_flame_probe_step || vi>=boss_blargg_flame_probe_vi+30){
                        ++boss_blargg_flame_probe_count;boss_blargg_flame_probe_vi=vi;boss_blargg_flame_probe_actor=actor;boss_blargg_flame_probe_callback=callback;boss_blargg_flame_probe_step=step;
                        auto scalar=[&](uint32_t address){return std::bit_cast<float>(n::word(rdram,address));};
                        fprintf(stderr,"[BossBlarggFlameQA] vi=%u fn=%s count=%u fight=%u room=%u actor=%08X birth=%llu private=%08X resource=%08X overlay130=%08X factory=%08X caller=%08X callback=%08X primary=%u secondary=%u queue=%u hidden=%u step=%u timer=%u local=%.4f,%.4f world=%.4f,%.4f velocity=%.4f,%.4f anchor=%.4f,%.4f trail=%.4f,%.4f readOnly=1 nativeEntry=1\n",vi,name,boss_blargg_flame_probe_count,state.index+1,campaign::physical_room,actor,(unsigned long long)campaign::blargg_observer.birth(actor),data,resource,uint32_t(section_addresses[130]),n::word(rdram,resource+0x44),uint32_t(ctx->r31),callback,n::byte(rdram,actor+0x18),n::byte(rdram,actor+0x30),n::byte(rdram,actor+0x17D),n::byte(rdram,actor+0x165),step,n::word(rdram,data+0x2CC),scalar(actor+0x3C),scalar(actor+0x40),scalar(actor+0x84),scalar(actor+0x88),scalar(actor+0xC4),scalar(actor+0xC8),scalar(data+0x2C0),scalar(data+0x2C4),scalar(data+0x208),scalar(data+0x20C));
                    }
                }
            }
        }
        if(boss_test_blargg_shoot0289 && boss_blargg_shoot.snapshot().started && boss_blargg_request_probe_count<192){
            namespace n=workshop::boss_native;
            const auto& probe=boss_blargg_shoot.snapshot();const auto observed_vi=test_vi.load();
            const auto fn=std::string_view(name);const auto player=workshop::boss_campaign0250::session.snapshot().player;
            const bool wanted=(fn=="func_80043450" && uint32_t(ctx->r4)==player)||fn=="func_80020014"||fn=="func_80021958"||fn=="func_800415FC"||fn=="func_8003BEC8"||fn=="func_8003BE44"||fn=="func_800403E4"||fn=="func_800401D8"||fn=="func_8003BB58"||fn=="func_8003BA90";
            if(wanted && probe.stage==workshop::boss_blargg_shoot0289::Stage::RequestAim && observed_vi>=probe.stage_vi && observed_vi-probe.stage_vi<=120 && n::actor(rdram,player,0x4001)){
                const auto data=n::word(rdram,player+0x1F4),packet=n::word(rdram,0x800F7EA8);
                if(n::range(data,0x12F0) && n::range(packet,0x18)){
                    const auto held=MEM_HU(0,n::base(packet)),pressed=MEM_HU(12,n::base(packet));
                    ++boss_blargg_request_probe_count;
                    fprintf(stderr,"[BossBlarggRequestProbeQA] vi=%u fn=%s count=%u stage=%u A0=%08X A1=%08X caller=%08X P1=%08X primary=%u secondary=%u callback=%08X inputCallback=%08X pending=%08X gate12C9=%u heldFlag=%u heldEgg=%08X reticle=%08X packet=%08X held=%04X pressed=%04X axes=%d,%d controls=%u/%u stock=%u readOnly=1\n",observed_vi,name,boss_blargg_request_probe_count,unsigned(probe.stage),uint32_t(ctx->r4),uint32_t(ctx->r5),uint32_t(ctx->r31),player,n::byte(rdram,player+0x18),n::byte(rdram,player+0x30),n::word(rdram,player+0x20),n::word(rdram,data+0xAC),n::word(rdram,data+0xC0),n::byte(rdram,data+0x12C9),n::byte(rdram,data+0x1F8),n::word(rdram,data+0x1F0),n::word(rdram,data+0x1F4),packet,unsigned(held),unsigned(pressed),int(MEM_H(0,n::base(0x800F897E))),int(MEM_H(0,n::base(0x800F8980))),n::byte(rdram,0x800F7EA5),n::byte(rdram,0x800F7EA6),unsigned(n::byte(rdram,data+0x12C8)));
                    if(!boss_blargg_request_probe_dumped && fn=="func_8003BE44" && uint32_t(ctx->r4)==player && (pressed&0x200F)){
                        boss_blargg_request_probe_dumped=true;
                        {std::ofstream dump("blargg-request-front-native3BE44.rdram",std::ios::binary);dump.write(reinterpret_cast<const char*>(rdram),0x800000);}
                        fprintf(stderr,"[BossBlarggRequestProbeQA] front-snapshot vi=%u bytes=8388608 readOnly=1\n",observed_vi);
                    }
                }
            }
        }
        if(boss_test_blargg_shoot0289 && boss_blargg_acquire.snapshot().done && std::string_view(name)=="func_800404C8"){
            const auto before=boss_blargg_shoot.snapshot().launches;
            const auto observed_vi=test_vi.load();
            boss_blargg_shoot.native_entry(rdram,ctx,name,boss_blargg_shoot_frame(observed_vi),workshop::boss_campaign0250::blargg_observer);
            const auto after=boss_blargg_shoot.snapshot().launches;
            constexpr auto bytes=workshop::boss_egg_shape0260::required_capacity;
            if(after>before && after<=6 && recomp::mem_size>=bytes){
                const auto file=std::string("blargg-shot-")+std::to_string(after)+"-before-native404C8.rdram";
                {std::ofstream dump(file,std::ios::binary);dump.write(reinterpret_cast<const char*>(rdram),bytes);}
                fprintf(stderr,"[BossBlarggShootQA] launch-snapshot shot=%u vi=%u bytes=%zu phase=before-native404C8 readOnly=1\n",after,observed_vi,size_t(bytes));
            }
        }
        if(boss_test_full_loadout)workshop::boss_fish_probe0260::trace(rdram,ctx,name,test_vi.load(),workshop::boss_campaign0250::variant_frame(rdram));
        workshop::boss_lips_probe0260::trace(rdram,ctx,std::string_view(name),test_vi.load(),boss_test_lips_loadout || boss_test_route_checkpoints);
        if(std::string_view(name)=="func_800483B8")workshop::boss_checkpoint_probe0260::trace_entry(rdram,ctx,std::string_view(name),test_vi.load(),recomp::mem_size,checkpoint_probe_enabled());
        if(end_test_vi && workshop::boss_campaign0250::driver && workshop::boss_campaign0250::session.snapshot().diagnostic)workshop::boss_driver_bridge0260::trace_native_launch(rdram,ctx,std::string_view(name),test_vi.load());
        if(boss_test_route_checkpoints)workshop::boss_route_checkpoint_bridge0260::native_egg_request(rdram,ctx,std::string_view(name),test_vi.load());
        if(boss_test_fish_loadout&&(std::string_view(name)=="func_8003BEC8"||std::string_view(name)=="func_8003BE44"))boss_fish_loadout_witness.observe(rdram,ctx,std::string_view(name),boss_fish_loadout_scope(rdram,test_vi.load()));
    }
    if(!test_first_boss.empty() || workshop::boss_solo0240::enabled)workshop::boss_native::observe_entry(rdram,ctx,name,{boss_source,test_vi.load(),uint32_t(section_addresses[167]),uint32_t(section_addresses[285])});
#endif
    // Placed 406F coins use the F49C state machine (collected state 3).
    // The separate F070 state machine is used by another native coin form.
    if(std::string_view(name)=="func_8006F49C" && (uint32_t(ctx->r6)&255)==3)
        workshop::collections::collect(rdram,uint32_t(ctx->r4));
    if(std::string_view(name)=="workshop_8003B8C8")workshop::elemental_egg_draw0363::palette_offset(rdram,ctx,elemental_solo_scope0355());
    if(std::string_view(name)=="func_8006E7F0" || std::string_view(name)=="func_8005F700")workshop::collections::draw(rdram,uint32_t(ctx->r4));
    if(std::string_view(name)=="func_8006E7F0" || std::string_view(name)=="func_8005F700")workshop::reward_popup::paint(rdram,uint32_t(ctx->r4));
    if(fruit_audit && std::string_view(name)=="func_8005F700" && workshop::reward_popup::fours.contains(uint32_t(ctx->r4))){
        static uint32_t previous=0;
        if(previous!=uint32_t(ctx->r4)){previous=uint32_t(ctx->r4);fprintf(stderr,"[RewardPopup] draw single-four actor=%08X textureFrame=%u resource=%08X\n",previous,unsigned(MEM_HU(0x144,ctx->r4)),uint32_t(MEM_W(0x168,ctx->r4)));}
    }
    if(std::string_view(name)=="func_8005F700" || std::string_view(name)=="func_8006E7F0"){workshop::elemental_egg_draw0363::paint(rdram,uint32_t(ctx->r4),elemental_solo_scope0355());workshop::editor_objects::paint(rdram,uint32_t(ctx->r4));if(story_hard_session0330())workshop::story_hard_bridge0330::paint(rdram,uint32_t(ctx->r4));}
    if(std::string_view(name)=="func_8005F700")workshop::hybrid_mode::paint(rdram,uint32_t(ctx->r4));
    if(std::string_view(name)=="func_800675F0"){
        workshop::passages::frame(rdram,ctx);
        if(story_hard_session0330())workshop::story_hard_bridge0330::frame(rdram,ctx);
        story_observer0320.frame(rdram,ctx);
        if(story_pending_worlds0320&&uint32_t(ctx->r4)==0x800F7EA0&&test_vi.load()%300==0)story_world_receipt0320({});
        workshop::achievements::frame(rdram,ctx,payload.empty() && !test_tally && !workshop::bonus_game0352::active());
        bool hybrid_completion_allowed=!workshop::bonus_game0352::active() && payload.empty() && !workshop::custom_session::active() && !test_tally;
#ifdef WORKSHOP_NATIVE_COOP
        hybrid_completion_allowed=hybrid_completion_allowed && !test_coop_native && !workshop::boss_solo0240::enabled && !workshop::boss_campaign0250::enabled;
#endif
        workshop::hybrid_completion0290::frame(rdram,ctx,hybrid_completion_allowed);
        custom_victory_fixture(rdram,ctx);
        workshop::custom_session::finish(rdram);
        workshop::book::frame(rdram,ctx,test_vi.load());
        if(test_find_reveal)test_find_frame(rdram,ctx);
        if(test_editor_eggs)test_editor_eggs_frame(rdram,ctx);
    }
    if(std::string_view(name)=="func_80471368")workshop::book::request_open();
    if(fruit_audit){
        const std::string_view function(name);
        if(function=="func_8005F700")audit_fruit_actor(rdram,ctx->r4,"draw");
        if(function=="func_80573F94")audit_fruit_actor(rdram,ctx->r4,"collect");
    }
    if((end_test_vi || fruit_audit) && std::string_view(name)=="func_80573F94" && ram_pointer(uint32_t(ctx->r4))){
        const auto actor=ctx->r4;
        fprintf(stderr,"[Workshop] Collected fruit VI %u actor=%08X id=%04X frame=%u parent=%08X\n",test_vi.load(),uint32_t(actor),MEM_HU(12,actor),MEM_HU(0x144,actor),uint32_t(MEM_W(0x180,actor)));
    }
    if((end_test_vi || fruit_audit) && std::string_view(name)=="func_80574B44" && ram_pointer(uint32_t(ctx->r4))){
        fprintf(stderr,"[Workshop] Fruit factory VI %u id=%04X parent=%08X caller=%08X\n",test_vi.load(),MEM_HU(4,ctx->r4),uint32_t(MEM_W(0,ctx->r4)),uint32_t(ctx->r31));
    }
    const auto sp=uint32_t(ctx->r29);
    if(!trace_startup || sp<0x800F0000u || sp>=0x80100000u)return;
    std::lock_guard lock(lookup_mutex);
    lookup_records[lookup_count++%lookup_records.size()]={0,uint32_t(ctx->r31),sp,uint32_t(ctx->r4),uint32_t(ctx->r5),name};
}
extern "C" int workshop_egg_allowed(uint8_t* rdram,unsigned color){return !workshop::bonus_game0352::active()&&!story_hard_session0330()&&workshop::collections::egg_allowed(rdram,color);}
extern "C" void func_80058A14(uint8_t*,recomp_context*);
extern "C" void func_80047090(uint8_t*,recomp_context*);
extern "C" void func_8004613C(uint8_t*,recomp_context*);
extern "C" void func_8003B328(uint8_t*,recomp_context*);
extern "C" void func_8003B890(uint8_t*,recomp_context*);
extern "C" void func_8003B8B0(uint8_t*,recomp_context*);
static std::optional<unsigned> workshop_find_place(uint8_t* rdram,recomp_context* ctx,uint32_t actor,bool container){
    if(workshop::bonus_game0352::active())return {};
    if(story_hard_session0330())return {};
    if(!workshop::fruit_mix::range(actor,container?0x90:0x48))return {};
    const auto a=gpr(int32_t(actor));const auto id=MEM_HU(12,a);
    const auto p=workshop::progress::read(rdram,gpr(int32_t(0x807F2000)));const auto story=story_clears(rdram);
    if(!workshop::progress::valid(p,story))return {};
    const auto saved=*ctx;bool whitePresent=false;
    if(MEM_W(0,int32_t(0x800F8B88))==0){func_80058A14(rdram,ctx);whitePresent=ctx->r2!=0;*ctx=saved;}
    return workshop::finds::reserve(story_hybrid_session0320()?workshop::story_hybrid0320::view().eligible:workshop::progress::unlocked(p,story),id,uint32_t(MEM_W(container?0x84:0x3C,a)),uint32_t(MEM_W(container?0x88:0x40,a)),whitePresent);
}
static uint32_t workshop_find_pool(uint8_t* rdram,recomp_context* ctx,unsigned color,uint32_t x,uint32_t y,uint32_t z,bool revealed){
    const auto root=uint32_t(MEM_W(0,int32_t(0x800C4278)));if(!ram_pointer(root))return 0;
    const auto saved=*ctx;ctx->r29=ADD32(ctx->r29,-0x30);
    ctx->f12.u32l=x;ctx->f14.u32l=y;ctx->r6=gpr(int32_t(z));ctx->r7=gpr(int32_t(root));
    MEM_W(0x10,ctx->r29)=0x401D;MEM_W(0x14,ctx->r29)=8+color;
    find_spawning=int(color);find_spawning_scene=false;func_8003BB94(rdram,ctx);
    find_spawning=-1;find_spawning_scene=true;
    const auto egg=uint32_t(ctx->r2);
    if(ram_pointer(egg)){
        workshop::finds::spawned(egg,color,false);
        // Bubble owners enqueue their released child themselves. A hidden
        // item callback has no such owner: queue the pool actor through the
        // native initializer, or it remains invisible with no collision.
        if(revealed && !MEM_BU(0x17D,int32_t(egg))){
            *ctx=saved;ctx->r4=gpr(int32_t(egg));func_80044194(rdram,ctx);
        }
        if(revealed)for(int offset=0x6C;offset<=0x80;offset+=4)MEM_W(offset,int32_t(egg))=0x3F800000;
        if(test_find_reveal){test_find_egg=egg;test_find_spawn_vi=test_vi.load();}
        fprintf(stderr,"[HybridFind] Native egg color=%u actor=%08X revealed=%u.\n",color+8,egg,unsigned(revealed));
    }
    *ctx=saved;return egg;
}
static int workshop_find_source(uint8_t* rdram,recomp_context* ctx){
    auto found=find_sources.find(uint32_t(ctx->r4));if(found==find_sources.end())return 0;
    // The actor already owns its native 4692 resource. Restore the original
    // placement ID so cached room metadata remains reusable on later visits.
    if(find_binding && workshop::fruit_mix::range(find_binding->record,16)){
        const auto record=gpr(int32_t(find_binding->record));
        if(MEM_HU(0,record)==0x4692)MEM_H(0,record)=find_binding->kind;
    }
    ctx->r2=0;return 1;
}
static uint32_t workshop_editor_egg_pool(uint8_t* rdram,recomp_context* ctx,unsigned color){
    const auto root=uint32_t(MEM_W(0,int32_t(0x800C4278)));if(!ram_pointer(root))return 0;
    const auto saved=*ctx;ctx->r29=ADD32(ctx->r29,-0x30);
    ctx->r6=gpr(int32_t(saved.r6));ctx->r7=gpr(int32_t(root));
    MEM_W(0x10,ctx->r29)=0x401D;MEM_W(0x14,ctx->r29)=color;
    const auto previous=workshop::editor_objects::spawning_color;workshop::editor_objects::spawning_color=int(color);
    func_8003BB94(rdram,ctx);workshop::editor_objects::spawning_color=previous;
    const auto egg=uint32_t(ctx->r2);if(ram_pointer(egg))workshop::editor_objects::spawned(egg,color);
    if(test_editor_eggs && !test_editor_pool)test_editor_pool=egg;
    *ctx=saved;fprintf(stderr,"[EditorEgg] release color=%u actor=%08X canonicalUnlock=0\n",color,egg);return egg;
}
extern "C" int workshop_find_release(uint8_t* rdram,recomp_context* ctx){
    if(story_hard_session0330()&&workshop::story_hard_bridge0330::release(rdram,ctx))return 1;
    if(auto color=workshop::editor_objects::release_color(rdram,ctx)){ctx->r2=gpr(int32_t(workshop_editor_egg_pool(rdram,ctx,*color)));return 1;}
    for(auto [source,color]:find_sources){
        if(!workshop::fruit_mix::range(source,0x48))continue;
        const auto actor=gpr(int32_t(source));
        const auto x=uint32_t(MEM_W(0x3C,actor)),y=uint32_t(MEM_W(0x40,actor));
        if(std::abs(std::bit_cast<float>(x)-ctx->f12.fl)>4.f || std::abs(std::bit_cast<float>(y)-ctx->f14.fl)>4.f)continue;
        ctx->r2=gpr(int32_t(workshop_find_pool(rdram,ctx,color,ctx->f12.u32l,ctx->f14.u32l,uint32_t(ctx->r6),false)));
        return 1;
    }
    return 0;
}
extern "C" int workshop_find_hidden(uint8_t* rdram,recomp_context* ctx,uint32_t actor){
    auto color=workshop_find_place(rdram,ctx,actor,true);if(!color)return 0;
    const auto a=gpr(int32_t(actor));
    return workshop_find_pool(rdram,ctx,*color,uint32_t(MEM_W(0x84,a)),uint32_t(MEM_W(0x88,a)),uint32_t(MEM_W(0x8C,a)),true)!=0;
}
extern "C" void workshop_find_container_gate(uint8_t* rdram,recomp_context* ctx){
    const auto actor=uint32_t(MEM_W(0x18,ctx->r29));
    if(workshop_find_place(rdram,ctx,actor,true)){
        ctx->r2=1;
        if(test_find_reveal && !test_find_source)test_find_source=actor;
    }
}
extern "C" int workshop_special_victory(uint8_t* rdram,recomp_context* ctx){
    auto fruit=workshop::special_victory::paint(rdram,uint32_t(ctx->r4));if(!fruit)return 0;
    fprintf(stderr,"[SpecialVictory] fruit=%u frame=%u actor=%08X thirty-identical=1\n",6+*fruit,6+*fruit,uint32_t(ctx->r4));return 1;
}
extern "C" void func_8059EC8C(uint8_t*,recomp_context*);
extern "C" void func_80039F48(uint8_t*,recomp_context*);
static void test_editor_eggs_frame(uint8_t* rdram,recomp_context* ctx){
    const auto vi=test_vi.load();
    if(!test_editor_release_vi && vi>=3000){
        uint32_t child=0;unsigned desired=0,valid=0,active=0;
        for(auto [actor,binding]:workshop::editor_objects::children){
            if(binding.room!=workshop::editor_objects::current_room || (binding.color!=6 && binding.color!=8))continue;
            desired++;if(!workshop::fruit_mix::range(actor,0x1F8))continue;
            const auto a=gpr(int32_t(actor));
            if(MEM_W(0,a)!=0x19971110 || MEM_HU(12,a)!=0x41CE)continue;
            const auto item=uint32_t(MEM_W(0x1F4,a));if(!workshop::fruit_mix::range(item,2))continue;
            if(MEM_HU(0,int32_t(item))!=0x401C && MEM_HU(0,int32_t(item))!=0x401D)continue;
            valid++;if(!MEM_BU(0x18,a) || !workshop::fruit_mix::range(uint32_t(MEM_W(0x16C,a)),0x44))continue;
            active++;if(!child || actor<child)child=actor;
        }
        if(!test_editor_probe_logged){test_editor_probe_logged=true;fprintf(stderr,"[EditorEggTest] probe vi=%u frameHook=800675F0 room=%u children=%zu desired=%u valid=%u active=%u candidate=%08X\n",vi,workshop::editor_objects::current_room,workshop::editor_objects::children.size(),desired,valid,active,child);}
        if(child){
            test_editor_release_vi=vi;test_editor_flags=uint32_t(MEM_W(0,int32_t(0x800F8DF4)));
            test_editor_pending=workshop::finds::pending();test_editor_found=workshop::finds::view().found;
            // Explicit callback fixture, not a simulated manual tongue input.
            // Invoke the same native wrapper release and colored pool as play.
            workshop::editor_objects::paint(rdram,child);
            workshop::editor_objects::prepare_release(rdram,child);
            const auto saved=*ctx;ctx->r4=gpr(int32_t(child));func_8059EC8C(rdram,ctx);*ctx=saved;
            fprintf(stderr,"[EditorEggTest] native-release vi=%u wrapper=%08X pool=%08X F54before=%08X F54after=%08X pendingBefore=%u pendingAfter=%u foundBefore=%u foundAfter=%u diagnosticCallback=1\n",vi,child,test_editor_pool,test_editor_flags,uint32_t(MEM_W(0,int32_t(0x800F8DF4))),test_editor_pending,workshop::finds::pending(),test_editor_found,workshop::finds::view().found);
        }
    }
    if(test_editor_pool && test_editor_release_vi && !test_editor_picked && vi>=test_editor_release_vi+4){
        const auto egg=gpr(int32_t(test_editor_pool));
        if(!workshop::fruit_mix::range(test_editor_pool,0x1F8) || MEM_W(0,egg)!=0x19971110 || MEM_HU(12,egg)!=0x401D)return;
        const auto player=uint32_t(MEM_W(0,int32_t(0x800F8828)));const auto p=gpr(int32_t(player));
        if(!workshop::fruit_mix::range(player,0x1F8) || MEM_W(0,p)!=0x19971110 || MEM_HU(12,p)!=0x4001 || !workshop::fruit_mix::range(uint32_t(MEM_W(0x1F4,p)),0x12CC))return;
        if(!workshop::fruit_mix::range(uint32_t(MEM_W(0x174,egg)),0x18) || !workshop::fruit_mix::range(uint32_t(MEM_W(0x1F4,egg)),0x14))return;
        // Invoke the real egg/Yoshi collision callback: it sends event69, then
        // configures carrying and attaches the owner. State4 alone omits that
        // ownership setup and is not a complete native pickup transaction.
        test_editor_picked=true;const auto saved=*ctx;ctx->r4=egg;ctx->r5=p;func_80039F48(rdram,ctx);*ctx=saved;
        workshop::editor_objects::paint(rdram,test_editor_pool);
        const auto parent=uint32_t(MEM_W(0x180,egg));
        const auto parentId=workshop::fruit_mix::range(parent,0x1F8)?unsigned(MEM_HU(12,int32_t(parent))):0;
        fprintf(stderr,"[EditorEggTest] native-pickup vi=%u pool=%08X state=%u color=%u player=%08X parent=%08X parentId=%04X attachMode=%u callback=80039F48 F54before=%08X F54after=%08X pendingBefore=%u pendingAfter=%u foundBefore=%u foundAfter=%u diagnosticCallback=1\n",vi,test_editor_pool,unsigned(MEM_BU(0x30,egg)),uint32_t(MEM_W(0x1EC,egg)),player,parent,parentId,unsigned(MEM_BU(0x17C,egg)),test_editor_flags,uint32_t(MEM_W(0,int32_t(0x800F8DF4))),test_editor_pending,workshop::finds::pending(),test_editor_found,workshop::finds::view().found);
    }
}
extern "C" void func_80574060(uint8_t*,recomp_context*);
static void test_find_frame(uint8_t* rdram,recomp_context* ctx){
    if(test_find_source && !test_find_egg && test_vi.load()>=2700){
        const auto saved=*ctx;
        fprintf(stderr,"[HiddenEggTest] reveal source=%08X\n",test_find_source);
        workshop_find_hidden(rdram,ctx,test_find_source);*ctx=saved;
    }
    // A real thirtieth fruit verifies the carried egg's normal finish commit.
    // This fixture is available only with both explicit diagnostic switches.
    if(test_tally && !test_find_finish_fruit && workshop::finds::pending()){
        const auto player=MEM_W(0,int32_t(0x800F8828));
        if(ram_pointer(uint32_t(player)) && MEM_HU(12,player)==0x4001){
            test_find_finish_fruit=true;const auto saved=*ctx;ctx->r29=ADD32(ctx->r29,-0x30);
            ctx->r4=0x4030;func_80047090(rdram,ctx);
            ctx->r4=0x40302000|(uint32_t(ctx->r2)&0x3FF);
            ctx->r5=std::bit_cast<uint32_t>(std::bit_cast<float>(uint32_t(MEM_W(0x3C,player)))+24.f);
            ctx->r6=std::bit_cast<uint32_t>(std::bit_cast<float>(uint32_t(MEM_W(0x40,player)))-24.f);
            ctx->r7=MEM_W(0x44,player);func_8004613C(rdram,ctx);
            const auto fruit=uint32_t(ctx->r2);
            test_find_finish_actor=fruit;
            if(ram_pointer(fruit) && !MEM_BU(0x17D,int32_t(fruit))){ctx->r4=gpr(int32_t(fruit));func_80044194(rdram,ctx);}
            fprintf(stderr,"[HiddenEggTest] finish-fruit actor=%08X\n",fruit);*ctx=saved;
        }
    }
    if(test_tally && !test_find_finish_committed && ram_pointer(test_find_finish_actor)
        && test_vi.load()>=test_find_spawn_vi+60 && MEM_W(0,int32_t(0x800F89D0))==29){
        const auto fruit=gpr(int32_t(test_find_finish_actor));
        if(MEM_BU(0x18,fruit)==1){
            // Native egg contact was exercised above. Finish integration uses
            // the native fruit-consumption callback directly, independent of wind
            // and tongue controls; the result and save are left to the game.
            test_find_finish_committed=true;const auto saved=*ctx;
            ctx->r29=ADD32(ctx->r29,-0x30);ctx->r4=fruit;ctx->r5=0x66;func_80574060(rdram,ctx);*ctx=saved;
            fprintf(stderr,"[HiddenEggTest] native-thirtieth-fruit-tally\n");
        }
    }
    if(test_find_egg && test_vi.load()%30==0){
        const auto a=gpr(int32_t(test_find_egg));
        fprintf(stderr,"[HiddenEggTest] vi=%u actor=%08X id=%04X flags=%04X visible=%u active=%u state=%u xyz=%08X,%08X,%08X update=%08X collision=%08X parent=%08X color=%u pending=%u\n",test_vi.load(),test_find_egg,MEM_HU(12,a),MEM_HU(14,a),MEM_BU(0x10,a),MEM_BU(0x18,a),MEM_BU(0x30,a),uint32_t(MEM_W(0x3C,a)),uint32_t(MEM_W(0x40,a)),uint32_t(MEM_W(0x44,a)),uint32_t(MEM_W(0x20,a)),uint32_t(MEM_W(0x1C,a)),uint32_t(MEM_W(0x180,a)),uint32_t(MEM_W(0x1EC,a)),workshop::finds::pending());
    }
}
extern "C" void workshop_find_egg_state(uint8_t*,recomp_context*);
extern "C" void workshop_find_egg_state(uint8_t* rdram,recomp_context* ctx){
    if(workshop::bonus_game0352::active())return;
    if(story_hard_session0330()&&workshop::story_hard_bridge0330::egg_state(rdram,ctx))return;
    const auto state=MEM_BU(0x1F,ctx->r29);
    if(workshop::editor_objects::egg_state(uint32_t(ctx->r4),state)){
        workshop::editor_objects::pickup_sound(rdram,ctx,uint32_t(ctx->r4),state);
        ctx->r2=0x401E;return;
    }
    if(workshop::finds::egg_state(uint32_t(ctx->r4),state)){
        if(workshop::finds::pickup_sound(rdram,ctx,uint32_t(ctx->r4),state))
            fprintf(stderr,"[HybridFind] pickup-sound actor=%08X bank=6 cue=0251\n",uint32_t(ctx->r4));
        ctx->r2=0x401E;
        if(workshop::finds::scene_actor(uint32_t(ctx->r4))){
            MEM_B(0x10,ctx->r4)=(state==4 || state==5)?0:1;
            MEM_B(0x164,ctx->r4)=(state==4 || state==5)?0:3;
            const auto flags=MEM_HU(14,ctx->r4)&~0x400u;
            MEM_H(14,ctx->r4)=flags|((state==4 || state==5)?0:0x400);
        }
    }
}
extern "C" int workshop_egg_bubble_gate(uint8_t* rdram,recomp_context* ctx){
    if(workshop::bonus_game0352::active()){
        const auto actor=uint32_t(ctx->r4);
        if(!workshop::fruit_mix::range(actor,0x10))return 0;
        const gpr address=gpr(int32_t(actor));
        const auto kind=MEM_HU(12,address);
        ctx->r2=kind==0x4692||kind==0x4693||kind==0x46AA||kind==0x46AE;return 1;
    }
    if(story_hard_session0330()&&workshop::story_hard_bridge0330::egg_bubble_gate(rdram,ctx))return 1;
    if(workshop::editor_objects::source_color(uint32_t(ctx->r4))){ctx->r2=0;return 1;}
    if(workshop_find_source(rdram,ctx))return 1;
    const auto bubble=uint32_t(ctx->r4);
    if(trial_special_scope0350(rdram)&&workshop::fruit_mix::range(bubble,0x10)){
        const auto kind=MEM_HU(12,int32_t(bubble));
        if(kind==0x4692||kind==0x4693){
            const unsigned color=kind==0x4692?6:7;
            const auto state=trial_special_store0350.snapshot();
            if(state.readable&&state.last_write_succeeded&&(state.lost_mask&(1u<<color))&&trial_special_owned0350(rdram,color)){
                const unsigned carried_flag=1u<<(color+4);
                ctx->r2=!workshop::collections::egg_allowed(rdram,color)||(uint32_t(MEM_W(0,int32_t(0x800F8DF4)))&carried_flag)!=0;return 1;
            }
        }
    }
    if(auto hidden=workshop::collections::egg_bubble_hidden(rdram,bubble)){ctx->r2=*hidden;return 1;}
    return 0;
}
extern "C" void workshop_egg_enemy_color(uint8_t* rdram,recomp_context* ctx){
    workshop::enemy_egg_provenance0366::enemy_stack(rdram,ctx,workshop::rewards::custom_enemy_variant);
}
extern "C" void workshop_egg_default_color(uint8_t* rdram,recomp_context* ctx){
    if(!ctx->r2)return;
    const auto color=uint32_t(MEM_W(0,int32_t(0x807F0008)));
    if(workshop::yoshi_colors0300::elemental(color)){ctx->r7=workshop::yoshi_colors0300::native_alias(color);return;}
    if(workshop::enemy_egg_provenance0366::default_color(rdram,ctx,workshop::rewards::custom_enemy_variant))return;
    if(workshop::yoshi_colors0300::metal(color)){ctx->r7=color;return;}
    if(color>=8 && color<14)ctx->r7=color;
}
// Entered at the existing food bridge with the native parent stack intact.
// Its shared continuation performs health clamping, score, sound and logging.
extern "C" void workshop_special_lookup_note(uint32_t id){fprintf(stderr,"[Workshop] Resource lookup %04X\n",id);}
extern "C" int workshop_special_food(uint8_t* rdram,recomp_context* ctx){
    const auto actor=MEM_W(0x3C,ctx->r29);
    const auto id=MEM_HU(12,actor);
    const auto index=workshop::special::fruit_index(id);
    const auto color=uint32_t(MEM_W(0,int32_t(0x807F0008)));
    if(workshop::yoshi_colors0300::elemental(color)&&id==0x45B6){ctx->r8=color==17?0:3;ctx->r12=MEM_W(0x2C,ctx->r29);return 1;}
    if(index<0){
        if(!workshop::rewards::enemy_food(id))return 0;
        const auto life=workshop::rewards::food_gain(rdram,uint32_t(actor),color,false);
        if(!life)return 0;
        // Avoid inheriting the aliased green color's special class branches.
        ctx->r8=*life<0?0:3;
    }else ctx->r8=workshop::rewards::lucky_fruit_type && unsigned(6+index)==workshop::rewards::lucky_fruit_type()?6:workshop::special::food_class(index,color);
    ctx->r12=MEM_W(0x2C,ctx->r29);
    if(index>=0)fprintf(stderr,"[Workshop] Special fruit %04X Yoshi %u class %u\n",id,color,uint32_t(ctx->r8));
    return 1;
}
extern "C" void workshop_food_health(uint8_t* rdram,recomp_context* ctx){
    const auto actor=uint32_t(MEM_W(0x3C,ctx->r29)),color=uint32_t(MEM_W(0,int32_t(0x807F0008)));
#ifdef WORKSHOP_NATIVE_COOP
    if(workshop::boss_campaign0250::active())if(auto life=workshop::boss_variants0250::healing_override(rdram,actor)){ctx->r2=gpr(*life);return;}
#endif
    if(auto value=workshop::rewards::food_gain(rdram,actor,color,false,unsigned(ctx->r8))){
        int delta=*value;
        if(color==17&&delta<0){const auto hp=unsigned(MEM_W(0,int32_t(0x800F8DB4+6*4)));delta=-int(std::min(3u,hp<=8?hp:0u));}
        ctx->r2=gpr(delta);
    }
}
extern "C" void workshop_food_score(uint8_t* rdram,recomp_context* ctx){
    const auto color=uint32_t(MEM_W(0,int32_t(0x807F0008)));
    MEM_W(0x18,ctx->r29)=0;
#ifdef WORKSHOP_NATIVE_COOP
    if(workshop::boss_campaign0250::active()&&workshop::boss_variants0250::renewable_actor(rdram,uint32_t(ctx->r5))){ctx->r24=0;MEM_W(0x18,ctx->r29)=6;return;}
#endif
    if(auto value=workshop::rewards::food_gain(rdram,uint32_t(ctx->r5),color,true,unsigned(ctx->r6))){
        const auto life=workshop::rewards::food_gain(rdram,uint32_t(ctx->r5),color,false,unsigned(ctx->r6));
        const int healing=life?*life:workshop::rewards::gain(rdram,unsigned(ctx->r6)&7,false);
        const unsigned bubble=*value==0 && healing>0?5:workshop::rewards::popup(*value);
        ctx->r24=gpr(*value);MEM_W(0x18,ctx->r29)=1+bubble;
    }
}
extern "C" void workshop_food_popup(uint8_t* rdram,recomp_context* ctx){
    const auto encoded=uint32_t(MEM_W(0x18,ctx->r29));
    if(encoded)MEM_W(0x38,ctx->r29)=encoded-1;
}
extern "C" void workshop_enemy_kill_reward(uint8_t* rdram,recomp_context* ctx){
    const auto color=uint32_t(MEM_W(0,int32_t(0x807F0008)));
#ifdef WORKSHOP_NATIVE_COOP
    if(workshop::boss_campaign0250::active()&&workshop::boss_variants0250::renewable_actor(rdram,uint32_t(ctx->r5))){ctx->r12=0;ctx->r13=1;return;}
#endif
    if(workshop::rewards::no_enemy_score(rdram,uint32_t(ctx->r5))){ctx->r12=0;ctx->r13=1;return;}
    if(auto value=workshop::rewards::custom_gain(rdram,uint32_t(ctx->r5),color)){ctx->r12=gpr(*value);ctx->r13=workshop::rewards::popup(*value);return;}
    if(!workshop::rewards::hybrid(color) || !workshop::fruit_mix::range(uint32_t(ctx->r5),0x10)
        || !workshop::rewards::enemy_food(MEM_HU(12,ctx->r5)))return;
    const int value=workshop::rewards::enemy_mean(rdram,uint32_t(ctx->r8),color,true,true);
    ctx->r12=gpr(value);ctx->r13=workshop::rewards::popup(value);
}
extern "C" void func_8002B03C(uint8_t*,recomp_context*);
extern "C" int workshop_split_reward_popup(uint8_t* rdram,recomp_context* ctx){
    const unsigned encoded=uint32_t(ctx->r5);
    if(encoded<0x85 || encoded>0x87)return 0;
    const auto saved=*ctx;
    unsigned remaining=encoded&0x7F;
    while(remaining){
        const unsigned amount=remaining==4?4:std::min(remaining,3u);
        *ctx=saved;ctx->r5=workshop::rewards::popup(amount);func_8002B03C(rdram,ctx);remaining-=amount;
    }
    *ctx=saved;return 1;
}
extern "C" void workshop_reward_popup_created(uint8_t* rdram,recomp_context* ctx){
    const gpr queue=int32_t(MEM_W(0x2C,ctx->r29));
    if(workshop::fruit_mix::range(uint32_t(queue),0x1311) && MEM_BU(0x1310,queue)==8){
        workshop::reward_popup::mark(rdram,uint32_t(ctx->r2));
        if(fruit_audit)fprintf(stderr,"[RewardPopup] single-four actor=%08X nativeAnimation=6 textureFrame=7\n",uint32_t(ctx->r2));
    }
}
#endif
#ifdef WORKSHOP_NATIVE_COOP
extern "C" uint32_t workshop_coop_scoped_player(uint8_t* rdram){
    if(!workshop::coop_bridge::context || workshop::coop_bridge::owner_stack.empty() || workshop::coop_native::constructing || workshop::coop_native::cleaning)return 0;
    const auto player=workshop::coop_native::word(rdram,workshop::coop::native_player);
    return (player==workshop::coop_bridge::bound_primary || player==workshop::coop_bridge::bound_secondary) && workshop::coop_native::actor(rdram,player,0x4001)?player:0;
}
extern "C" uint64_t workshop_coop_rooms_begin(uint8_t* r,recomp_context* c,uint32_t fn,uint32_t a,uint32_t arg,uint32_t third)noexcept{
    const auto saved=*c;
    try{
        if(test_coop_room_fixture0280 && end_test_vi && !test_inputs.empty() && !test_inputs_p2.empty())
            workshop::coop_pot_probe0280::before_begin(r,c,fn,a,test_vi.load(),hybrid_room&255);
        const auto ticket=workshop::coop_rooms_native0280::begin(r,c,fn,a,arg,third,test_vi.load(),hybrid_room&255);*c=saved;return ticket;
    }
    catch(...){workshop::coop_rooms_native0280::host_error();*c=saved;return UINT64_MAX;}
}
extern "C" void workshop_coop_rooms_finish(uint8_t* r,recomp_context* c,uint64_t token,int completed)noexcept{
    const auto saved=*c;
    try{workshop::coop_rooms_native0280::finish(r,c,token,completed!=0);}
    catch(...){workshop::coop_rooms_native0280::host_error();}
    *c=saved;
}
extern "C" int32_t workshop_coop_pot_axis(uint8_t* r,recomp_context* c,uint32_t candidate,int32_t native_y)noexcept{
    const auto saved=*c;
    try{
        const auto axis=workshop::coop_rooms_native0280::pot_axis(r,c,candidate,native_y);
        if(test_coop_room_fixture0280 && end_test_vi && !test_inputs.empty() && !test_inputs_p2.empty())
            workshop::coop_pot_probe0280::after_axis(r,c,candidate,native_y,axis,test_vi.load(),hybrid_room&255);
        *c=saved;return axis;
    }
    catch(...){workshop::coop_rooms_native0280::host_error();*c=saved;return 0;}
}
extern "C" uint64_t workshop_coop_food_begin(uint8_t* rdram,recomp_context*,uint32_t fn,uint32_t actor,uint32_t arg)noexcept{
    try{return workshop::coop_fruit0264::registry.begin(rdram,fn,actor,arg,workshop_coop_scoped_player(rdram));}
    catch(...){workshop::coop_fruit0264::registry.host_error();workshop::coop_bridge::invalidated=true;return UINT64_MAX;}
}
extern "C" void workshop_coop_food_finish(uint8_t* rdram,recomp_context*,uint64_t token,int completed)noexcept{
    try{
        workshop::coop_fruit0264::registry.finish(rdram,token,completed!=0);
        if(workshop::coop_fruit0264::registry.failed())workshop::coop_bridge::invalidated=true;
    }catch(...){workshop::coop_fruit0264::registry.host_error();workshop::coop_bridge::invalidated=true;}
}
extern "C" int workshop_coop_skip_collision(uint8_t* rdram,recomp_context* ctx,uint32_t fn,uint32_t work){
    if(uint32_t(ctx->r4)!=work)return 0;
    return workshop::coop_collision::filter_geometry(rdram,ctx,fn)?1:0;
}
extern "C" uint32_t workshop_coop_capture_egg_owner(uint8_t* rdram,recomp_context*)noexcept{
    if(!test_coop_native || workshop::coop_native::constructing || workshop::coop_native::cleaning || workshop::coop_bridge::owner_stack.empty())return UINT32_MAX;
    const auto slot=workshop::coop_bridge::owner_stack.back();
    const auto actor=workshop_coop_scoped_player(rdram);
    const auto owner=workshop::coop_collision::owner_of_body(rdram,actor);
    return owner && *owner==slot?slot:UINT32_MAX;
}
extern "C" void workshop_coop_egg_created(uint8_t* rdram,recomp_context*,uint32_t egg,uint32_t slot)noexcept{
    try{
        if(slot<2 && workshop::coop_collision::egg_created(rdram,egg,slot))fprintf(stderr,"[CoopEgg] created actor=%08X owner=%u vi=%u\n",egg,slot,test_vi.load());
    }catch(...){workshop::coop_collision::configure(false);fprintf(stderr,"[CoopContacts] disabled reason=host-registry-error\n");}
}
extern "C" uint64_t workshop_coop_begin_egg_private_init(uint8_t* rdram,recomp_context*,uint32_t egg)noexcept{
    try{return workshop::coop_collision::begin_egg_private_init(rdram,egg);}
    catch(...){workshop::coop_collision::configure(false);fprintf(stderr,"[CoopContacts] disabled reason=host-registry-error\n");return 0;}
}
extern "C" void workshop_coop_finish_egg_private_init(uint8_t* rdram,recomp_context*,uint64_t token,int completed)noexcept{
    try{
        if(completed)workshop::coop_collision::finish_egg_private_init(rdram,token);
        else workshop::coop_collision::cancel_egg_private_init(token);
    }catch(...){workshop::coop_collision::configure(false);fprintf(stderr,"[CoopContacts] disabled reason=host-registry-error\n");}
}
extern "C" void workshop_boss_defeat_begin(uint8_t* rdram,recomp_context*){workshop::boss_solo0240::defeat_begin(rdram);}
extern "C" void workshop_boss_defeat_complete(uint8_t* rdram,recomp_context* ctx){workshop::boss_solo0240::defeat_complete(rdram,ctx);}
extern "C" int workshop_boss_pot_complete(uint8_t* rdram,recomp_context* ctx){return workshop::boss_campaign0250::enabled?workshop::boss_campaign0250::pot_complete(rdram,ctx):workshop::boss_solo0240::pot_complete(rdram,ctx);}
extern "C" int workshop_battle_selector(uint8_t* r,recomp_context* c){
 const bool handled=workshop::boss_selection0260::handle(r,c,{workshop::boss_campaign0250::enabled,workshop::boss_campaign0250::session.snapshot().phase,uint32_t(section_addresses[workshop::boss_selection0260::overlay_section]),true});
 if(handled)fprintf(stderr,"[BossSelection] direct-native-confirmation vi=%u accessUnchanged=%06X\n",test_vi.load(),workshop::boss_native::word(r,0x800F8E5C));return handled;
}
extern "C" int workshop_battle_shared_completion(uint8_t* r,recomp_context* c){if(workshop::bonus_game0352::active()){workshop::bonus_game0352::frame_context(test_vi.load(),section_addresses,286);return workshop::bonus_game0352::shared_completion(r,c);}return workshop::boss_campaign0250::shared_completion(r,c);}
extern "C" uintptr_t workshop_flower_creator_begin(uint8_t* r,recomp_context* c){return reinterpret_cast<uintptr_t>(new workshop::coop_classic_flower0250::PetalCreationScope(r,uint32_t(c->r4)));}
extern "C" void workshop_flower_creator_finish(uintptr_t ticket){delete reinterpret_cast<workshop::coop_classic_flower0250::PetalCreationScope*>(ticket);}
extern "C" uint64_t workshop_boss_tree_spawn_enter(uint8_t* r,recomp_context* context){
 return workshop::boss_tree_hybrid0260::spawn_enter(r,context,workshop::boss_campaign0250::hybrid_tree_frame());
}
extern "C" void workshop_boss_tree_spawn_leave(uint8_t* r,recomp_context* context,uint64_t ticket){
 if(const auto born=workshop::boss_tree_hybrid0260::spawn_leave(r,context,ticket))fprintf(stderr,"[BossTreeHybrid] spawned=%04X position=%u actor=%08X vi=%u\n",unsigned(born->kind),born->position,born->actor,test_vi.load());
}
extern "C" int workshop_boss_intro_popup(uint8_t* r,recomp_context* context){
 namespace c=workshop::boss_campaign0250;namespace intro=workshop::boss_intro0260;
 if(!r||!context||!c::active())return 0;const auto& state=c::session.snapshot();
 if(state.phase!=workshop::boss_battle0250::Phase::Fighting)return 0;
 const auto& bindings=c::variants::snapshot();if(!bindings.enabled||bindings.invalidated||bindings.encounter!=state.index)return 0;
 std::array<intro::Member,4> members{};const auto count=c::session.current().count;
 for(unsigned n=0;n<count;++n){const auto& member=bindings.members[n];const auto kind=c::session.current().members[n].kind;
  const auto& proof=kind==0x455A?c::candies[n].snapshot():c::observers[n].snapshot();const auto& actor=proof.actors[0];
  if(member.actor==c::bound[n]&&member.birth&&proof.enabled&&!proof.invalidated&&proof.registered_actors==1&&actor.initialized&&!actor.disposed&&!actor.completion_returned&&actor.actor==member.actor&&actor.private_data)members[n]={member.actor,actor.private_data,kind,c::witness_frame(n).family_overlay};
 }
 const intro::Frame frame{true,true,state.index,c::physical_room,c::logical_room,uint32_t(section_addresses[91]),{members.data(),count}};
 if(const auto request=intro::observe_dialog_creation(r,context,frame)){
  fprintf(stderr,"[BossIntro] dialogue-skipped fight=%u member=%u actor=%08X text=%d nativeNullBranch=1 animationsPreserved=1\n",state.index+1,request->member,request->actor,request->text_id);return 1;
 }
 return 0;
}
extern "C" int workshop_flower_petal_pool(uint8_t* r,recomp_context* c){return workshop::coop_classic_flower0250::petal_pool(r,c);}
extern "C" void workshop_flower_position(uint8_t* r,recomp_context* c){workshop::coop_classic_flower0250::position(r,c);}
static void record_native_battle(uint8_t* rdram){
 namespace c=workshop::boss_campaign0250;namespace a=workshop::boss_challenges0260;
 const auto& s=c::session.snapshot();a::CompletionEvidence e;e.token=s.token;
 e.provenance=boss_production_menu?a::Provenance::ProductionMenu:a::Provenance::ScriptedQa;
 e.phaseComplete=s.phase==workshop::boss_battle0250::Phase::Complete;e.diagnostic=s.diagnostic;e.skippedEntry=boss_test_case>=0;e.qaMarker=std::filesystem::exists("boss-battle-qa.txt")||std::filesystem::exists("manual-coop-qa.txt");
 e.completed=s.completed;e.index=s.index;e.player=s.player;e.initialPlayer=c::first_player;e.color=c::cargo.pc_color;e.initialColor=c::first_color;e.playerKind=workshop::boss_native::actor(rdram,s.player,0x4001)?0x4001:0;e.completionVi=test_vi.load();
 for(unsigned k=0;k<10;++k){auto& to=e.encounters[k];const auto& from=c::history[k];to.number=k+1;to.expectedMembers=c::session.plan().fights[k].count;to.player=from.player;to.color=from.color;
  for(unsigned m=0;m<to.expectedMembers;++m){const auto& r=from.members[m];if(r.actor&&r.encounter==k&&r.member==m&&r.vi&&r.initialized&&r.active&&r.terminal&&r.causal_hit){to.defeatedMask|=1u<<m;to.defeatVi=std::max(to.defeatVi,r.vi);}}
  to.initialized=to.active=to.terminal=to.causal=to.defeatedMask==((1u<<to.expectedMembers)-1);
 }
 const bool saved=a::record(e);boss_award_saved=saved;fprintf(stderr,"[BossChallenge] saved=%u diagnostic=%u provenance=%u completed=%u reason=%s\n",unsigned(saved),unsigned(e.diagnostic),unsigned(e.provenance),e.completed,a::snapshot().reason.c_str());
}
extern "C" unsigned workshop_battle_challenges(){return workshop::boss_challenges0260::snapshot().mask18;}
extern "C" int workshop_battle_awarded(){return boss_award_saved.load();}
extern "C" int workshop_battle_challenge_readable(){return workshop::boss_challenges0260::snapshot().readable;}
extern "C" int workshop_battle_diagnostic(){return workshop::boss_campaign0250::session.snapshot().diagnostic;}
// Called before the game thread starts, from the native book menu only.
extern "C" int workshop_battle_prepare(){
    if(!payload.empty() || test_coop_native || !test_first_boss.empty() || !boss_solo_token.empty())return 0;
    if(!boss_test_menu)boss_battle_seed=uint64_t(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    boss_battle_token.assign(32,'0');const uint64_t salt=uint64_t(std::chrono::steady_clock::now().time_since_epoch().count());
    for(unsigned n=0;n<32;++n)boss_battle_token[n]="0123456789abcdef"[((n<16?boss_battle_seed:salt)>>(60-4*(n%16)))&15];
    const bool qa=!test_inputs.empty() || end_test_vi || test_color>=0 || std::filesystem::exists("boss-battle-qa.txt") || std::filesystem::exists("manual-coop-qa.txt");
    if(!boss_test_menu){boss_test_case=-1;boss_test_goal="route";}boss_production_menu=true;boss_award_saved=false;
    const bool configured=workshop::boss_campaign0250::configure(boss_battle_token,boss_battle_seed,qa,false);
    if(configured)workshop::boss_campaign0250::checkpoints::allow_retries(test_inputs.empty() || boss_manual_handoff || boss_test_checkpoint || boss_test_retry_replay || boss_test_route_checkpoints);
    return configured;
}
// Only the native4550 getter's first pool allocation is expanded. The native
// header/provider and every12-segment per-head proof remain unchanged.
extern "C" int workshop_boss_candy_pool_enter(uint8_t* r,recomp_context* c){
 if(workshop::bonus_game0352::active()){workshop::bonus_game0352::frame_context(test_vi.load(),section_addresses,286);return workshop::bonus_game0352::candy_pool_enter(r,c);}
 namespace campaign=workshop::boss_campaign0250;namespace pool=workshop::boss_candy_pool0260;
 const auto choice=campaign::candy_pool.enter(r,c,campaign::candy_pool_frame(uint32_t(section_addresses[168])));
 if(choice==pool::Choice::Refused)campaign::fail("candy_segment_pool_live_or_unknown");return int(choice);
}
extern "C" void workshop_boss_candy_pool_leave(uint8_t* r,const recomp_context* c,int ticket){
 if(workshop::bonus_game0352::active()){workshop::bonus_game0352::candy_pool_leave(r,c,ticket);return;}
 namespace campaign=workshop::boss_campaign0250;
 if(ticket!=1)return;if(!campaign::candy_pool.leave(r,c)){campaign::fail("candy_segment_pool_native_initialization_rejected");return;}
 const auto& lease=campaign::candy_pool.snapshot().lease;
 std::fprintf(stderr,"[BossCandyPool] action=initialized header=%08X registry=%08X storage=%08X slots=%u candies=%u index=%u native4667C=1 nativeProviderUnchanged=1\n",lease.header,lease.registry,workshop::boss_candy_pool0260::storage,lease.slots,lease.slots/12,campaign::session.snapshot().index);
}
extern "C" void workshop_boss_candy_pool_cleanup(uint8_t* r,const recomp_context* entry,const recomp_context* result){
 if(workshop::bonus_game0352::active()){workshop::bonus_game0352::candy_pool_cleanup(r,entry,result);return;}
 namespace campaign=workshop::boss_campaign0250;
 if(campaign::candy_pool.cleanup_return(r,entry,result))std::fprintf(stderr,"[BossCandyPool] action=native-cleanup header=%08X registry=%08X native46324Returned=1 unregistrationStillRequired=1\n",campaign::candy_pool.snapshot().lease.header,campaign::candy_pool.snapshot().lease.registry);
}

extern "C" int workshop_battle_done(){return workshop::boss_campaign0250::finished.load();}
extern "C" int workshop_battle_failed(){return workshop::boss_campaign0250::failed.load();}
extern "C" int workshop_battle_hook_enter(uint8_t* r,recomp_context* c,const char* n){
 if(workshop::bonus_game0352::active()){workshop::bonus_game0352::frame_context(test_vi.load(),section_addresses,286);workshop::bonus_game0352::wrapped_entry(r,c,n);return 0;}
 if(workshop::boss_campaign0250::enabled&&std::string_view(n)=="func_8069D05C"){workshop::boss_campaign0250::current_vi=test_vi.load();workshop::boss_campaign0250::overlays[189]=uint32_t(section_addresses[189]);}
 if(std::string_view(n)=="func_8069D05C")workshop::boss_checkpoint_probe0260::trace_entry(r,c,std::string_view(n),test_vi.load(),recomp::mem_size,checkpoint_probe_enabled());
 return workshop::boss_campaign0250::hook_enter(r,c,n);
}
extern "C" void workshop_battle_hook_leave(uint8_t* r,const recomp_context* c,const char* n){if(workshop::bonus_game0352::active()){workshop::bonus_game0352::frame_context(test_vi.load(),section_addresses,286);workshop::bonus_game0352::wrapped_leave(r,c,n);return;}workshop::boss_campaign0250::hook_leave(r,c,n);if(std::string_view(n)=="func_8069D05C")workshop::boss_checkpoint_probe0260::trace_leave(r,c,std::string_view(n),test_vi.load(),recomp::mem_size,checkpoint_probe_enabled());}
extern "C" int workshop_battle_paint_begin(uint8_t* r,recomp_context* c,unsigned fn){if(workshop::bonus_game0352::active()){workshop::bonus_game0352::frame_context(test_vi.load(),section_addresses,286);return (fn==0x8005F700||fn==0x805C6500)&&workshop::bonus_game0352::paint_begin(r,uint32_t(fn==0x8005F700?c->r4:c->r5),fn==0x8005F700);}return workshop::boss_campaign0250::active() && (fn==0x8005F700 || fn==0x805C6500 || fn==0x8066664C) && workshop::boss_variants0250::palette_begin(r,uint32_t(fn==0x8005F700?c->r4:c->r5),fn==0x8005F700,workshop::boss_campaign0250::variant_frame(r));}
extern "C" void workshop_battle_paint_finish(uint8_t* r){if(workshop::bonus_game0352::active()){workshop::bonus_game0352::paint_finish(r);return;}workshop::boss_variants0250::palette_finish(r);}


// Scoped native checkpoint hooks. These are inert for ordinary Vanilla scenes.
static void checkpoint_overlay_ready(){
 workshop::boss_campaign0250::current_vi=test_vi.load();
 if(workshop::boss_campaign0250::enabled){workshop::boss_campaign0250::overlays[128]=uint32_t(section_addresses[128]);workshop::boss_campaign0250::overlays[138]=uint32_t(section_addresses[138]);}
}
extern "C" int workshop_boss_checkpoint_spawn(uint8_t* r,recomp_context* c){
 if(!r||!c||uint32_t(c->r4)>>16!=0x4507)return 0;
 checkpoint_overlay_ready();const auto choice=workshop::boss_campaign0250::checkpoints::select_spawn(r,c);
 if(!choice.owned)return 0;if(choice.suppress)return 1;c->r4=choice.packed;return 0;
}
extern "C" uint64_t workshop_boss_checkpoint_init_enter(uint8_t* r,const recomp_context* c){checkpoint_overlay_ready();const auto ticket=workshop::boss_campaign0250::checkpoints::init_enter(r,c);if(ticket!=workshop::boss_checkpoint_actor0260::deferred_initialization)workshop::boss_checkpoint_probe0260::trace_entry(r,c,"func_805A6600",test_vi.load(),recomp::mem_size,checkpoint_probe_enabled());return ticket;}
extern "C" void workshop_boss_checkpoint_init_leave(uint8_t* r,const recomp_context* c,uint64_t t){workshop::boss_campaign0250::checkpoints::init_leave(r,c,t);workshop::boss_checkpoint_probe0260::trace_leave(r,c,"func_805A6600",test_vi.load(),recomp::mem_size,checkpoint_probe_enabled());}
extern "C" uint64_t workshop_boss_checkpoint_wake_enter(uint8_t* r,const recomp_context* c){
 checkpoint_overlay_ready();
 return workshop::boss_campaign0250::checkpoints::wake_enter(r,c);
}
extern "C" void workshop_boss_checkpoint_wake_leave(uint8_t* r,const recomp_context* c,uint64_t t){workshop::boss_campaign0250::checkpoints::wake_leave(r,c,t);}
extern "C" int workshop_boss_checkpoint_block_warp(uint8_t* r,const recomp_context* c){checkpoint_overlay_ready();return workshop::boss_campaign0250::checkpoints::suppress_warp(r,c);}
extern "C" int workshop_boss_checkpoint_retry_death(uint8_t* r,recomp_context* c){
    const unsigned coop_callback_vi=test_vi.load();
    if(test_coop_native && workshop::coop_bridge::intercept_death(r,c,coop_callback_vi,hybrid_room&255)){
        if(test_coop_secondary_return0288 && r && c && (hybrid_room&255)==81){
            namespace n=workshop::coop_native;namespace life=workshop::coop_lifecycle0280;
            const auto& state=life::lifecycle.snapshot();const auto& player=state.players[1];
            const auto observed=workshop::coop_collision::observed_identity(r,player.root.address);
            static bool captured=false;
            if(!captured && state.bound && !state.failed && !state.ending && state.room==81 && player.phase==life::Phase::death_pending && player.death_vi==coop_callback_vi && uint32_t(c->r4)==player.root.address && observed && *observed==player.root && workshop::coop_rooms_native0280::coordinator.epoch()==1 && n::range(player.root.private_data,0x1550)){
                const char* file="coop-secondary-return-death.rdram";
                std::ofstream dump(file,std::ios::binary);dump.write(reinterpret_cast<const char*>(r),0x800000);dump.close();
                if(dump){captured=true;fprintf(stderr,"[CoopSecondaryReturnSnapshot] phase=death vi=%u file=%s bytes=8388608 readonly=1 room=81 actor=%08X private=%08X birth=%llu epoch=%llu ra=%08X fifo=%u beforeCleanup=1\n",coop_callback_vi,file,player.root.address,player.root.private_data,(unsigned long long)player.root.generation,(unsigned long long)state.epoch,uint32_t(c->r31),unsigned(n::byte(r,player.root.private_data+0x12C8)));}
            }
        }
        // QA observation at the genuine fatal callback. The bridge defers all
        // native retirement to the outer update fence, so reserve RAM is intact.
        if(test_coop_room_reentry0287 && r && c && (hybrid_room&255)==81){
            namespace n=workshop::coop_native;namespace life=workshop::coop_lifecycle0280;
            const auto& state=life::lifecycle.snapshot();const auto& player=state.players[0];
            const auto observed=workshop::coop_collision::observed_identity(r,player.root.address);
            static bool captured=false;
            if(!captured && state.bound && !state.failed && !state.ending && state.room==81 && player.phase==life::Phase::death_pending && player.death_vi==coop_callback_vi && uint32_t(c->r4)==player.root.address && observed && *observed==player.root && workshop::coop_rooms_native0280::coordinator.epoch()==1 && n::range(player.root.private_data,0x1550)){
                const char* file="coop-room-reentry-death.rdram";
                std::ofstream dump(file,std::ios::binary);dump.write(reinterpret_cast<const char*>(r),0x800000);dump.close();
                if(dump){captured=true;fprintf(stderr,"[CoopRoomReentrySnapshot] phase=death vi=%u file=%s bytes=8388608 readonly=1 room=81 actor=%08X private=%08X birth=%llu epoch=%llu ra=%08X fifo=%u beforeCleanup=1\n",coop_callback_vi,file,player.root.address,player.root.private_data,(unsigned long long)player.root.generation,(unsigned long long)state.epoch,uint32_t(c->r31),unsigned(n::byte(r,player.root.private_data+0x12C8)));}
            }
        }
        return 1;
    }
    checkpoint_overlay_ready();workshop::boss_route_checkpoint_bridge0260::native_death(r,c,test_vi.load());
    workshop::boss_swallow_retry_test0263::death_entry(r,c,test_vi.load());
    if(boss_test_fish_cp6 || boss_test_fish_cp6_focused)workshop::boss_fish_cp6_bridge0284::native_death(r,c,test_vi.load());
    return workshop::boss_campaign0250::checkpoints::intercept_death(r,c);
}
extern "C" int workshop_boss_solo_done(){return workshop::boss_solo0240::finished.load();}
extern "C" uintptr_t workshop_coop_dispatch_enter(uint8_t* rdram,recomp_context* ctx,uint32_t fn,uint32_t a,uint32_t arg){
    if(fn==0x800675F0)return (test_coop_native || !test_first_boss.empty() || workshop::boss_solo0240::enabled || workshop::boss_campaign0250::enabled
        || workshop::starting_bonus0283::allowed(starting_bonus_environment(rdram).mode) || starting_bonus.unsettled())?UINTPTR_MAX:0;
    if(fn==0x80048060 && workshop::coop_collision::before_detach(rdram,a))fprintf(stderr,"[CoopEgg] detach actor=%08X vi=%u\n",a,test_vi.load());
    return workshop::coop_bridge::enter(rdram,ctx,fn,a,arg);
}
extern "C" void workshop_coop_dispatch_leave(uint8_t* rdram,recomp_context* ctx,uint32_t fn,uint32_t a,uint32_t arg,uintptr_t ticket,int completed)noexcept{
    if(fn!=0x800675F0){
        if(completed && fn==0x8003C510 && test_coop_native){
            const auto parent=workshop::coop_native::actor(rdram,a)?workshop::coop_native::word(rdram,a+0x180):0;
            const auto kind=workshop::coop_native::range(arg,0x1FC)?workshop::coop_native::kind(rdram,arg):0;
            const auto owner=workshop::coop_bridge::owner_stack.empty()?UINT32_MAX:workshop::coop_bridge::owner_stack.back();
            fprintf(stderr,"[CoopTongue] contact vi=%u tongue=%08X parent=%08X target=%08X kind=%04X global=%08X owner=%u completed=1\n",test_vi.load(),a,parent,arg,kind,workshop::coop_native::word(rdram,workshop::coop::native_player),owner);
        }
        if(completed && (fn==0x80039F48 || fn==0x8003FFD8))workshop::coop_collision::attached(rdram,a,arg);
        workshop::coop_bridge::leave(ticket);return;
    }
    if(!completed)return;
    starting_bonus_frame(rdram,ctx);
    // Ordinary solo modes use this fence only for paid starting bonuses.
    // Preserve the previous campaign/coop-only scope of all subsequent hooks.
    if(!test_coop_native && test_first_boss.empty() && !workshop::boss_solo0240::enabled && !workshop::boss_campaign0250::enabled)return;
    if(workshop::boss_solo0240::enabled){static unsigned n=0;if(n++<3)fprintf(stderr,"[BossSolo] post-fence vi=%u phase=%u\n",test_vi.load(),unsigned(workshop::boss_solo0240::session.snapshot().phase));}
    workshop::boss_solo0240::frame(rdram,ctx,test_vi.load());
    const bool focused_initializing=boss_test_fish_cp6_focused&&workshop::boss_fish_cp6_bridge0284::fixture_pre_fence(rdram,ctx,test_vi.load());
    if(!focused_initializing)workshop::boss_campaign0250::frame(rdram,ctx,test_vi.load(),[](uint32_t fn,uint8_t* r,recomp_context* c){
        uint32_t address=fn;if(fn>=0x805C64F0 && fn<0x805CA4E0)address=uint32_t(section_addresses[148])+fn-0x805C64F0;
        if(fn==0x804326C4)address=uint32_t(section_addresses[19])+fn-0x804322E0;
        if(fn==0x805D3444)address=uint32_t(section_addresses[156])+fn-0x805D1A30;
        get_function(int32_t(address))(r,c);
    });
    workshop::boss_route_checkpoint_bridge0260::observe(rdram,test_vi.load());
    if(boss_test_fish_cp6 || boss_test_fish_cp6_focused)workshop::boss_fish_cp6_bridge0284::observe(rdram,test_vi.load());
    if(boss_test_full_loadout)workshop::boss_fish_probe0260::frame(rdram,test_vi.load());
    if((boss_test_full_loadout || boss_test_lips_loadout || boss_test_genie_loadout || boss_test_monsters_loadout || boss_test_fish_loadout || boss_test_final_loadout) && !boss_loadout.snapshot().applied){
        const auto& s=workshop::boss_campaign0250::session.snapshot();
        if(workshop::boss_campaign0250::enabled && s.phase==workshop::boss_battle0250::Phase::Fighting){
            workshop::boss_test_loadout0260::Frame f{true,boss_manual_handoff,s.diagnostic,!background_test,unsigned(boss_test_case),s.index,workshop::boss_campaign0250::physical_room,workshop::boss_campaign0250::logical_room,test_vi.load(),true,s.player,boss_test_automatic || boss_test_lips_loadout || boss_test_genie_loadout || boss_test_monsters_loadout || boss_test_fish_loadout || boss_test_final_loadout,boss_test_lips_loadout,(boss_test_lips_loadout || boss_test_genie_loadout) && workshop::boss_campaign0250::witnessed_zero_controls(rdram,s.player),boss_test_genie_loadout,boss_test_genie_empty_reserves?0u:6u,boss_test_monsters_loadout,boss_test_final_loadout};
            f.fish_target=boss_test_fish_loadout;f.seed=workshop::boss_campaign0250::session.plan().seed;
            if(f.fish_target&&workshop::boss_test_loadout0260::mixed_fish_tuple(f.seed,f.case_index)){
                const auto& r=workshop::boss_campaign0250::fight_ready;f.ready={r.vi,r.index,r.room,r.source,r.epoch,r.player,r.private_data};f.epoch=s.retry_epoch;f.fish_roots={workshop::boss_campaign0250::bound[0],workshop::boss_campaign0250::bound[1]};
                f.native_request_recent=boss_fish_loadout_witness.recent(rdram,boss_fish_loadout_scope(rdram,test_vi.load()));f.native_request_vi=boss_fish_loadout_witness.vi();
            }
            if(!boss_loadout.frame(rdram,ctx,f))workshop::boss_campaign0250::fail("requested_test_loadout_failed");
        }
    }
    if(boss_test_genie_empty_reserves && boss_loadout.snapshot().applied && !boss_test_genie_refill_logged){
        namespace iv=workshop::boss_checkpoint_inventory0260;namespace bn=workshop::boss_native;
        const auto& s=workshop::boss_campaign0250::session.snapshot();iv::Payload value;iv::Inventory stock;
        if(s.diagnostic && s.phase==workshop::boss_battle0250::Phase::Fighting && s.index==2 && workshop::boss_campaign0250::physical_room==169 && workshop::boss_campaign0250::logical_room==163 && s.player==boss_loadout.snapshot().player && iv::capture(rdram,s.player,test_vi.load(),value) && value.count>0 && iv::read(rdram,s.player,stock) && stock.count==value.count && bn::actor(rdram,stock.actors[0],0x4004) && bn::word(rdram,stock.actors[0]+0x180)==s.player){
            boss_test_genie_refill_logged=true;
            fprintf(stderr,"[BossTestRefill] case=2 index=2 room=169 player=%08X count=%u firstEgg=%08X palette=%u vi=%u nativeReadback=1 forcedEggs=0\n",s.player,value.count,stock.actors[0],unsigned(stock.eggs[0].palette),test_vi.load());
        }
    }
    const unsigned room_frame_vi0280=test_vi.load();
    if(test_coop_native){
        const unsigned stage=workshop::coop_native::byte(rdram,0x800F7EA2),scene=workshop::coop_native::byte(rdram,0x800F841D);
        unsigned alive_mask=0;
        if(room_frame_vi0280>=2400 && (stage==6 || stage==7) && scene==5){
            workshop::coop_native::ScopedContext registers(ctx);ctx->r29=ADD32(ctx->r29,-0x80);
            workshop_807F1300(rdram,ctx);alive_mask=unsigned(ctx->r2)&0x3FFF;
        }
        workshop::coop_bridge::attempt_observation(rdram,ctx,room_frame_vi0280,stage,scene,hybrid_room&255,alive_mask);
    }
    workshop::coop_bridge::frame(rdram,ctx,room_frame_vi0280,hybrid_room&255);
    if(test_coop_secondary_return0288){
        const auto proof=workshop::coop_secondary_return0288::witness(rdram,room_frame_vi0280,hybrid_room&255);
        static std::array<bool,3> captured{};
        const std::array<unsigned,3> anchors={proof.scope_vi,proof.return_vi,proof.arrival_vi};
        constexpr std::array<const char*,3> phases={"scope","return","arrival"};
        for(unsigned i=0;i<3;i++)if(!captured[i] && anchors[i] && !proof.rejected && workshop::coop_bridge::owner_stack.empty()){
            const auto file=std::string("coop-secondary-return-")+phases[i]+".rdram";
            std::ofstream dump(file,std::ios::binary);dump.write(reinterpret_cast<const char*>(rdram),0x800000);dump.close();
            if(dump){captured[i]=true;fprintf(stderr,"[CoopSecondaryReturnSnapshot] phase=%s vi=%u anchorVI=%u file=%s bytes=8388608 readonly=1 room=%u\n",phases[i],room_frame_vi0280,anchors[i],file.c_str(),hybrid_room&255);}
        }
    }
    if(test_coop_room_reentry0287){
        const auto proof=workshop::coop_room_reentry0287::witness(rdram,room_frame_vi0280,hybrid_room&255);
        static std::array<bool,3> captured{};
        const std::array<unsigned,3> anchors={proof.scope_vi,proof.return_vi,proof.arrival_vi};
        constexpr std::array<const char*,3> phases={"scope","return","arrival"};
        for(unsigned i=0;i<3;i++)if(!captured[i] && anchors[i] && !proof.rejected && workshop::coop_bridge::owner_stack.empty()){
            const auto file=std::string("coop-room-reentry-")+phases[i]+".rdram";
            std::ofstream dump(file,std::ios::binary);dump.write(reinterpret_cast<const char*>(rdram),0x800000);dump.close();
            if(dump){captured[i]=true;fprintf(stderr,"[CoopRoomReentrySnapshot] phase=%s vi=%u anchorVI=%u file=%s bytes=8388608 readonly=1 room=%u\n",phases[i],room_frame_vi0280,anchors[i],file.c_str(),hybrid_room&255);}
        }
    }
    // Read-only evidence for the reported manual1-1 fall. This threshold
    // merely requests one local snapshot; it never infers death or changes
    // position, life, collision, a room transition or the team-loss mask.
    if(test_coop_manual && (hybrid_room&255)==43 && !workshop::coop_bridge::invalidated && workshop::coop_bridge::context && workshop::coop_bridge::owner_stack.empty() && !workshop::coop_rooms_native0280::busy()){
        static bool fall_observed=false;
        const auto& life=workshop::coop_lifecycle0280::lifecycle.snapshot();
        if(!fall_observed && life.bound && !life.failed && !life.ending){
            for(unsigned owner=0;owner<2;owner++){
                const auto& id=life.players[owner].root;const auto current=workshop::coop_collision::observed_identity(rdram,id.address);
                if(!current || *current!=id || !workshop::coop_lifecycle0280::lifecycle.alive(rdram,owner))continue;
                const float y=std::bit_cast<float>(workshop::coop_native::word(rdram,id.address+0x88));
                if(!std::isfinite(y) || y<=1024.f)continue;
                fall_observed=true;const auto file=std::string("coop-fall-observation-")+std::to_string(room_frame_vi0280)+".rdram";
                if(!std::filesystem::exists(file)){
                    std::ofstream dump(file,std::ios::binary);dump.write(reinterpret_cast<const char*>(rdram),0x800000);
                    fprintf(stderr,"[CoopLife] fall-observation owner=%u actor=%08X birth=%llu vi=%u worldY=%.3f bytes=8388608 readonly=1 deathInferred=0 file=%s\n",owner,id.address,(unsigned long long)id.generation,room_frame_vi0280,y,file.c_str());
                }
                break;
            }
        }
    }
    if(test_coop_manual && workshop::coop_lifecycle0280::lifecycle.snapshot().failed){
        static bool preserved=false;
        if(!preserved){
            preserved=true;const auto file=std::string("coop-lifecycle-rejected-")+std::to_string(room_frame_vi0280)+".rdram";
            if(!std::filesystem::exists(file)){
                std::ofstream dump(file,std::ios::binary);dump.write(reinterpret_cast<const char*>(rdram),0x800000);
                fprintf(stderr,"[CoopLife] rejection-captured vi=%u bytes=8388608 readonly=1 file=%s\n",room_frame_vi0280,file.c_str());
            }
        }
    }
    if(test_coop_room_fixture0280 && workshop::coop_bridge::context && workshop::coop_bridge::owner_stack.empty()){
        static unsigned last_room_probe=0;const auto vi=test_vi.load();
        const auto return_proof=workshop::coop_room_return0286::witness(rdram,vi,hybrid_room&255);
        const bool return_probe=test_coop_room_return0286&&(return_proof.handled||return_proof.rejected);
        const auto reentry_proof=workshop::coop_room_reentry0287::witness(rdram,vi,hybrid_room&255);
        const bool reentry_probe=test_coop_room_reentry0287&&(reentry_proof.handled||reentry_proof.delegated||reentry_proof.rejected);
        const auto secondary_proof=workshop::coop_secondary_return0288::witness(rdram,vi,hybrid_room&255);
        const bool secondary_probe=test_coop_secondary_return0288&&(secondary_proof.handled||secondary_proof.delegated||secondary_proof.rejected);
        if(((vi>=2580 && vi<=2910) || vi%120==0 || ((return_probe||reentry_probe||secondary_probe)&&vi%30==0)) && vi!=last_room_probe){
            last_room_probe=vi;namespace native=workshop::coop_native;
            fprintf(stderr,"[CoopRoomProbe] vi=%u room=%u busy=%u",vi,hybrid_room&255,unsigned(workshop::coop_rooms_native0280::busy()));
            for(unsigned p=0;p<2;p++){
                const auto a=p?workshop::coop_bridge::bound_secondary:workshop::coop_bridge::bound_primary;
                const bool valid=native::actor(rdram,a,0x4001);const auto data=valid?native::word(rdram,a+0x1F4):0;
                fprintf(stderr," root%u=%08X valid%u=%u x%u=%.3f y%u=%.3f hp%u=%u fifo%u=%u queue%u=%u parent%u=%08X mode%u=%u sub%u=%u",p,a,p,unsigned(valid),p,valid?std::bit_cast<float>(native::word(rdram,a+0x84)):0.0f,p,valid?std::bit_cast<float>(native::word(rdram,a+0x88)):0.0f,p,workshop::coop_bridge::context->player(p).health,p,native::range(data,0x1550)?native::byte(rdram,data+0x12C8):255,p,valid?native::byte(rdram,a+0x17D):255,p,valid?native::word(rdram,a+0x180):0,p,valid?native::byte(rdram,a+0x18):255,p,valid?native::byte(rdram,a+0x30):255);
            }
            if((vi>=2580 && vi<=2910)||return_probe||reentry_probe||secondary_probe){
                for(unsigned p=0;p<2;p++){
                    const auto a=p?workshop::coop_bridge::bound_secondary:workshop::coop_bridge::bound_primary;
                    const bool valid=native::actor(rdram,a,0x4001);const auto data=valid?native::word(rdram,a+0x1F4):0;
                    fprintf(stderr," vx%u=%.3f vy%u=%.3f groundRaw%u=%u",p,valid?std::bit_cast<float>(native::word(rdram,a+0xC4)):0.0f,p,valid?std::bit_cast<float>(native::word(rdram,a+0xC8)):0.0f,p,native::range(data,0x1550)?native::byte(rdram,data+0xF4):255);
                }
                const auto pointer=native::word(rdram,workshop::coop_native_input::pad_pointer);
                const bool pad_valid=native::range(pointer,16);
                const auto pad=pad_valid?workshop::coop_native_input::Raw::capture(rdram,pointer):workshop::coop_native_input::Raw{};
                fprintf(stderr," p1RawValid=%u p1Raw=%04X p1Pressed=%04X p1X=%d p1Y=%d p1Aheld=%u p1Apressed=%u p1StickX=%d p1StickY=%d",unsigned(pad_valid),unsigned(pad.buttons),unsigned(pad.pressed),int(pad.x),int(pad.y),unsigned(native::byte(rdram,0x800F8995)),unsigned(native::byte(rdram,0x800F8996)),int(int16_t(workshop::coop_collision::half(rdram,0x800F897E))),int(int16_t(workshop::coop_collision::half(rdram,0x800F8980))));
            }
            if(return_probe)fprintf(stderr," returnPhase=%s returnAnchor=%u returnElapsed=%u returnRejected=%u",workshop::coop_room_return0286::phase_name(return_proof.phase),return_proof.return_vi,return_proof.elapsed,unsigned(return_proof.rejected));
            if(reentry_probe)fprintf(stderr," reentryPhase=%s reentryDeath=%u reentryReturn=%u reentryArrival=%u reentryAnchor=%u reentryElapsed=%u reentryRejected=%u",workshop::coop_room_reentry0287::phase_name(reentry_proof.phase),reentry_proof.death_vi,reentry_proof.return_vi,reentry_proof.arrival_vi,reentry_proof.arrival_vi,reentry_proof.elapsed,unsigned(reentry_proof.rejected));
            if(secondary_probe)fprintf(stderr," secondaryPhase=%s secondaryDeath=%u secondaryReturn=%u secondaryArrival=%u secondaryAnchor=%u secondaryElapsed=%u secondaryRejected=%u secondaryEscort=%u secondaryParked=%u secondaryLower=%u secondaryReason=%s",workshop::coop_secondary_return0288::phase_name(secondary_proof.phase),secondary_proof.death_vi,secondary_proof.return_vi,secondary_proof.arrival_vi,secondary_proof.arrival_vi,secondary_proof.elapsed,unsigned(secondary_proof.rejected),secondary_proof.escort_stage,unsigned(secondary_proof.survivor_parked),unsigned(secondary_proof.lower_route),secondary_proof.rejection_reason?secondary_proof.rejection_reason:"none");
            fprintf(stderr," nativeObservation=1 readonly=1\n");
        }
    }
    workshop::coop_classic_flower0250::frame(rdram,ctx,test_vi.load(),workshop::coop_bridge::bound_primary,workshop::coop_bridge::bound_secondary,test_coop_native&&!workshop::coop_rooms_native0280::busy()&&!workshop::coop_bridge::invalidated&&workshop::coop_bridge::collisions_ready&&workshop::coop_bridge::owner_stack.empty());
    if(test_coop_native){
        const auto& flower=workshop::coop_classic_flower0250::state;static uint32_t proved=0;
        if(flower.centre&&flower.centre!=proved&&workshop::coop_classic_flower0250::centre_valid(rdram)){
            unsigned count=0;bool valid=true;for(unsigned n=0;n<20;++n){const auto petal=workshop::coop_native::word(rdram,flower.centre+0x188+n*4);if(!petal)continue;
                valid=valid&&workshop::coop_native::actor(rdram,petal,0x4011)&&workshop::coop_native::word(rdram,petal+0x180)==flower.centre&&workshop::coop_native::word(rdram,petal+8)==flower.allocation;
                if(valid)++count;
            }
            if(valid&&count>0&&count<=8){proved=flower.centre;fprintf(stderr,"[CoopFlower] native-petals-ready vi=%u centre=%08X privatePool=%08X attached=%u independentGraph=1\n",test_vi.load(),flower.centre,flower.allocation,count);}
        }
    }
    {
        namespace bridge=workshop::coop_bridge;namespace life=workshop::coop_lifecycle0280;
        const auto& cycle=life::lifecycle.snapshot();const auto vi=test_vi.load();
        std::array<workshop::coop_health0250::Waiting,2> waiting{};
        bool ready=!bridge::invalidated && bridge::collisions_ready;
        if(test_coop_native && bridge::context && cycle.enabled && cycle.bound){
            ready=!bridge::invalidated && !cycle.failed && !cycle.ending && cycle.room==(hybrid_room&255);
            unsigned alive=0,awaiting=0;
            for(unsigned p=0;p<2;p++){
                const auto& player=cycle.players[p];
                if(player.phase==life::Phase::alive){
                    const auto& view=bridge::context->player(p);
                    ready=ready && life::lifecycle.alive(rdram,p) && view.actor.address==player.root.address && view.color==player.color;alive++;
                }else if(player.phase==life::Phase::death_pending || player.phase==life::Phase::retiring || player.phase==life::Phase::waiting){
                    ready=ready && player.death_vi && vi>=player.death_vi && player.due_vi>=player.death_vi;
                    waiting[p]={true,player.color,player.due_vi>vi?player.due_vi-vi:0};awaiting++;
                }else ready=false;
            }
            ready=ready && (alive==2 || (alive==1 && awaiting==1));
        }
        ready=ready && !workshop::coop_rooms_native0280::busy();
        workshop::coop_health0250::publish_waiting_frame(vi,test_coop_native,ready,bridge::context.get(),bridge::bound_primary,bridge::bound_secondary,bridge::owner_stack.empty(),waiting);
    }
    if(test_coop_native){const auto health=workshop::coop_health0250::snapshot();static std::array<unsigned,4> last{99,99,99,99};const std::array<unsigned,4> now{health.players[0].color,health.players[0].health,health.players[1].color,health.players[1].health};if(health.visible&&now!=last){last=now;fprintf(stderr,"[CoopHealth] vi=%u p1=%08X color=%u health=%u p2=%08X color=%u health=%u\n",test_vi.load(),health.players[0].actor,now[0],now[1],health.players[1].actor,now[2],now[3]);}}

    workshop::coop_probe::frame(rdram,ctx,test_vi.load());
    workshop::coop_egg_stress::frame(rdram,ctx,test_vi.load());
    const unsigned boss_source=workshop::boss_solo0240::enabled?workshop::boss_solo0240::effective_room(hybrid_room&255):(test_zone==162 && test_stage==8 && (hybrid_room&255)==workshop::vanilla_sources[8]?162:(hybrid_room&255));
    workshop::boss_native::frame(rdram,{boss_source,test_vi.load(),uint32_t(section_addresses[167]),uint32_t(section_addresses[285])});
    if(auto proof=workshop::boss_native::take_defeat_observation())fprintf(stderr,"[BossNative] real-defeat token=%s vi=%u actor=%08X player=%08X transition=%08X initial=%d remaining=%d diagnostic=1\n",proof->run_token.c_str(),proof->vi,proof->actor,proof->player,proof->transition_actor,proof->initial_segments,proof->remaining_segments);
    const auto& campaign=workshop::boss_campaign0250::session.snapshot();
    if(workshop::boss_campaign0250::enabled&&campaign.diagnostic&&end_test_vi&&test_vi.load()%120==0){
        namespace b=workshop::boss_native;const auto player=campaign.player,p=b::word(rdram,player+0x1F4);
        if(b::actor(rdram,player,0x4001)&&b::range(p,0x1300)){std::array<uint32_t,6> eggs{};const bool valid=workshop::boss_solo0240::read_reserves(rdram,player,eggs);const auto hp=workshop::boss_solo0240::health(rdram);
            fprintf(stderr,"[BossState] vi=%u phase=%u fight=%u hp=%u xy=%.2f,%.2f held=%08X mouthPhase=%u callbackC0=%08X eggs=%d fruits=%u treeConsumed=%u treeRegens=%u state=%u/%u parent=%08X controls=%u/%u padY=%d potGates=%u/%u/%u/%u/%u\n",test_vi.load(),unsigned(campaign.phase),campaign.index+1,hp?*hp:99,std::bit_cast<float>(b::word(rdram,player+0x84)),std::bit_cast<float>(b::word(rdram,player+0x88)),b::word(rdram,p+0x164),b::byte(rdram,p+0x159),b::word(rdram,p+0xC0),valid?int(std::count_if(eggs.begin(),eggs.end(),[](auto a){return a!=0;})):-1,b::word(rdram,0x800F89D0),workshop::boss_tree0240::snapshot().totalConsumed,workshop::boss_tree0240::snapshot().regenerations,b::byte(rdram,player+0x18),b::byte(rdram,player+0x30),b::word(rdram,player+0x180),b::byte(rdram,0x800F7EA5),b::byte(rdram,0x800F7EA6),int(MEM_H(0,b::base(0x800F8980))),b::word(rdram,p+0x12EC),b::byte(rdram,p+0xF4),b::byte(rdram,p+0x154),b::byte(rdram,p+0x155),b::byte(rdram,p+0x1F8));
        }
    }

    if(boss_test_goal=="ground-roundtrip"&&campaign.diagnostic&&end_test_vi){
        static unsigned recorded_stage=0;
        const auto stage=boss_ground_walk.stage();
        if(stage!=recorded_stage){recorded_stage=stage;namespace n=workshop::boss_native;const auto p=campaign.player,pot=workshop::boss_campaign0250::exit_pot;const auto& cp=workshop::boss_campaign0250::checkpoints::characters.snapshot();
            fprintf(stderr,"[BossGroundWalk] stage=%u vi=%u player=%08X xy=%.2f,%.2f pot=%08X potY=%.2f checkpoint=%08X checkpointBound=%u inputOnly=1\n",stage,test_vi.load(),p,std::bit_cast<float>(n::word(rdram,p+0x84)),std::bit_cast<float>(n::word(rdram,p+0x88)),pot,n::actor(rdram,pot,0x4273)?std::bit_cast<float>(n::word(rdram,pot+0x88)):-999.f,cp.binding.actor,unsigned(!cp.invalidated&&cp.binding.actor!=0));
            std::ofstream dump("ground-roundtrip-"+std::to_string(stage)+".rdram",std::ios::binary);dump.write(reinterpret_cast<char*>(rdram),0x800000);
        }
    }
    bool campaign_ready=campaign.phase==workshop::boss_battle0250::Phase::Fighting&&workshop::boss_campaign0250::flags_ready;
    if(campaign_ready)for(unsigned n=0;n<workshop::boss_campaign0250::session.current().count;++n){const auto a=workshop::boss_campaign0250::bound[n];const auto kind=workshop::boss_campaign0250::session.current().members[n].kind;
        const bool witnessed=kind==0x455A?workshop::boss_campaign0250::candies[n].snapshot().actors[0].active_seen:workshop::boss_family_native0250::family_for_kind(kind)?workshop::boss_campaign0250::observers[n].snapshot().actors[0].active_seen:workshop::boss_variants0250::snapshot().members[n].active;
        campaign_ready=campaign_ready&&witnessed&&workshop::boss_native::actor(rdram,a,kind)&&MEM_BU(0x17D,workshop::boss_native::base(a))==2;}
    const bool campaign_goal=workshop::boss_campaign0250::enabled&&campaign.diagnostic&&!test_inputs.empty()&&(
        (boss_test_goal=="swallow-checkpoint"&&boss_test_swallow_checkpoint&&workshop::boss_swallow_retry_test0263::complete(rdram))||
        (boss_test_goal=="fish-cp6-replay"&&boss_test_fish_cp6&&workshop::boss_fish_cp6_bridge0284::complete()&&campaign.index==6&&campaign.completed==7&&campaign.retry_epoch==1&&campaign.phase==workshop::boss_battle0250::Phase::Rest&&workshop::boss_campaign0250::physical_room==83)||
        (boss_test_goal=="fish-cp6-focused-replay"&&boss_test_fish_cp6_focused&&workshop::boss_fish_cp6_bridge0284::complete()&&campaign.index==6&&campaign.completed==7&&campaign.retry_epoch==1&&campaign.phase==workshop::boss_battle0250::Phase::Rest&&workshop::boss_campaign0250::physical_room==83)||
        (boss_test_goal=="fish-cp6-rebirth"&&boss_test_fish_cp6_focused&&workshop::boss_fish_cp6_bridge0284::snapshot().goal==workshop::boss_fish_cp6_test0284::Goal::Rebirth&&workshop::boss_fish_cp6_bridge0284::complete()&&campaign.index==6&&campaign.completed==6&&!campaign.defeated&&campaign.retry_epoch==1&&campaign.phase==workshop::boss_battle0250::Phase::Fighting&&workshop::boss_campaign0250::physical_room==173&&workshop::boss_campaign0250::logical_room==97)||
        (boss_test_goal=="retry-victory"&&boss_test_retry_replay&&campaign.retry_epoch>=2&&campaign.index==0&&campaign.completed==1&&campaign.phase==workshop::boss_battle0250::Phase::Victory&&!workshop::boss_campaign0250::restore_pending&&workshop::boss_native::actor(rdram,workshop::boss_campaign0250::exit_pot,0x4273))||
        (boss_test_goal=="ground-roundtrip"&&boss_ground_walk.done()&&campaign.phase==workshop::boss_battle0250::Phase::Rest&&campaign.completed==1)||
        (boss_test_goal=="transition-rest"&&workshop::boss_campaign0250::driver&&campaign.index==7&&campaign.completed==1&&campaign.phase==workshop::boss_battle0250::Phase::Rest&&workshop::boss_campaign0250::physical_room==83&&!workshop::boss_campaign0250::stats_reset_pending&&workshop::boss_campaign0250::cargo_epoch!=0&&workshop::boss_campaign0250::stats_restored_epoch==workshop::boss_campaign0250::cargo_epoch)||
        (boss_test_goal=="route"&&workshop::boss_campaign0250::driver&&campaign_ready&&campaign.index==1&&campaign.completed==1)||
        (boss_test_goal=="ready"&&campaign_ready)||
        (boss_test_goal=="second-victory"&&!boss_followup_inputs.empty()&&campaign.index==1&&campaign.completed==2&&campaign.phase==workshop::boss_battle0250::Phase::Victory&&!workshop::boss_campaign0250::restore_pending&&workshop::boss_native::actor(rdram,workshop::boss_campaign0250::exit_pot,0x4273))||
        (boss_test_goal=="victory"&&campaign.phase==workshop::boss_battle0250::Phase::Victory&&campaign.completed==1&&(!boss_test_automatic||workshop::boss_driver_bridge0260::qa_bird_complete()))||
        (boss_test_goal=="full"&&campaign.phase==workshop::boss_battle0250::Phase::Complete&&campaign.completed==10)||
        (boss_test_goal=="route-checkpoints"&&boss_test_route_checkpoints&&workshop::boss_route_checkpoint_bridge0260::complete()&&campaign.phase==workshop::boss_battle0250::Phase::Complete&&campaign.completed==10)||
        (boss_test_goal=="checkpoint"&&boss_test_checkpoint&&campaign.retry_epoch>0&&campaign.checkpoint_awake_mask&&workshop::boss_campaign0250::checkpoints::retry_restored_epoch()==campaign.retry_epoch&&(campaign_ready||campaign.phase==workshop::boss_battle0250::Phase::Rest)));
    // A bounded input-pilot transition gets a stable diagnostic dump even if
    // a spring keeps it moving in circles. This times out only disposable QA;
    // it never changes gameplay phase, health or a native victory receipt.
    bool campaign_stalled=false;
    {static unsigned watched_phase=99,watched_index=99,phase_since=0;
     const bool watch=workshop::boss_campaign0250::enabled&&workshop::boss_campaign0250::driver&&campaign.diagnostic&&end_test_vi&&!test_inputs.empty()&&(boss_test_goal=="full"||boss_test_goal=="route-checkpoints"||boss_test_fish_cp6||boss_test_fish_cp6_focused)&&(campaign.phase==workshop::boss_battle0250::Phase::Victory||campaign.phase==workshop::boss_battle0250::Phase::Rest);
     if(watch){const auto phase=unsigned(campaign.phase);
      if(watched_phase!=phase||watched_index!=campaign.index){watched_phase=phase;watched_index=campaign.index;phase_since=test_vi.load();}
      const unsigned budget=campaign.phase==workshop::boss_battle0250::Phase::Victory?3600:7200;
      campaign_stalled=test_vi.load()-phase_since>=budget;
     }else watched_phase=watched_index=99;
    }
    // Stop only an isolated Full input pilot that cannot obtain any egg.
    // No gameplay failure is recorded: the existing stable fence below
    // captures its unfinished native state and exits the disposable QA with2.
    {static workshop::boss_diagnostic_watch0260::AmmoWatch empty_ring_watch;
     namespace n=workshop::boss_native;
     workshop::boss_diagnostic_watch0260::Sample q;
     q.enabled=workshop::boss_campaign0250::enabled;q.diagnostic=campaign.diagnostic;q.driver=workshop::boss_campaign0250::driver;
     q.full=boss_test_goal=="full"||boss_test_fish_cp6||boss_test_fish_cp6_focused;q.scripted=!test_inputs.empty();q.bounded=end_test_vi!=0;q.fighting=campaign.phase==workshop::boss_battle0250::Phase::Fighting;
     q.vi=test_vi.load();q.encounter=campaign.index;q.player=campaign.player;
     if(q.enabled&&q.diagnostic&&q.driver&&q.scripted&&q.bounded&&q.fighting){
      q.live_player=n::actor(rdram,q.player,0x4001)&&!workshop::boss_campaign0250::fatal_player(rdram,q.player);
      if(q.live_player){std::array<uint32_t,6> ring{};q.inventory_valid=workshop::boss_solo0240::read_reserves(rdram,q.player,ring);
       q.reserves=unsigned(std::count_if(ring.begin(),ring.end(),[](auto egg){return egg!=0;}));
       for(unsigned slot=0;slot<20;++slot){const auto child=n::word(rdram,q.player+0x188+4*slot);if(n::actor(rdram,child,0x4005)&&n::word(rdram,child+0x180)==q.player)q.aiming=true;}
      }
     }
     if(empty_ring_watch.step(q)){campaign_stalled=true;fprintf(stderr,"[BossDriver] diagnostic-no-resupply vi=%u fight=%u emptyBudgetVI=3600 inputOnly=1 noGameplayFailure=1\n",q.vi,q.encounter+1);}
     static workshop::boss_diagnostic_watch0260::MonsterProgressWatch monster_progress_watch;
     workshop::boss_diagnostic_watch0260::ProgressSample progress;progress.gate=q;
     if(q.enabled&&q.diagnostic&&q.driver&&q.scripted&&q.bounded&&q.fighting&&q.live_player){
      progress.monsters=workshop::boss_campaign0250::session.current().family==workshop::boss_battle0250::Family::Monsters;
      if(progress.monsters){const auto& native=workshop::boss_variants0250::snapshot();const unsigned count=workshop::boss_campaign0250::session.current().count;
       progress.initialized=native.enabled&&!native.invalidated&&count>0&&count<=4;progress.defeated_mask=campaign.defeated;
       for(unsigned member=0;member<count&&member<4;++member){const auto& b=native.members[member];progress.causal_hits+=b.hits;
        if(!(campaign.defeated&(1u<<member))){const bool valid=b.initialized&&b.birth&&n::actor(rdram,b.actor,0x41C6)&&b.private_data%4==0&&n::range(b.private_data,0x1000)&&n::word(rdram,b.actor+0x1F4)==b.private_data;progress.initialized=progress.initialized&&valid;
         if(valid&&q.vi%120==0)fprintf(stderr,"[BossMonsterProgress] vi=%u fight=%u member=%u actor=%08X nativeHP=%u remaining=%u causalHits=%u nativeObservation=1 noForcedDamage=1\n",q.vi,q.encounter+1,member,b.actor,n::word(rdram,b.private_data+0xDE8),b.remaining,b.hits);
        }
       }
      }
     }
     if(monster_progress_watch.step(progress)){campaign_stalled=true;fprintf(stderr,"[BossDriver] diagnostic-no-monster-hit vi=%u fight=%u hitBudgetVI=18000 inputOnly=1 noGameplayFailure=1\n",q.vi,q.encounter+1);}

    }
    if(campaign_stalled)fprintf(stderr,"[BossDriver] diagnostic-stagnation vi=%u fight=%u phase=%u inputOnly=1 noGameplayFailure=1\n",test_vi.load(),campaign.index+1,unsigned(campaign.phase));
    // A rejected route protocol cannot complete a pending retry. Stop its
    // isolated QA at this same post-update fence and preserve the failure RAM.
    // Planned native deaths keep waiting normally while the controller is valid.
    const bool rejected_checkpoint_route=boss_test_route_checkpoints&&boss_test_goal=="route-checkpoints"&&workshop::boss_route_checkpoint_bridge0260::enabled&&workshop::boss_route_checkpoint_bridge0260::controller.failed();
    const bool campaign_abort=workshop::boss_campaign0250::enabled&&campaign.diagnostic&&!test_inputs.empty()&&((campaign.phase==workshop::boss_battle0250::Phase::Failed&&(!workshop::boss_campaign0250::checkpoints::waiting_for_retry()||rejected_checkpoint_route))||campaign_stalled||(boss_test_swallow_checkpoint&&workshop::boss_swallow_retry_test0263::failed)||((boss_test_fish_cp6 || boss_test_fish_cp6_focused)&&workshop::boss_fish_cp6_bridge0284::failed()));
    // QA-only early finish after the two real room transactions. Keep the
    // original finite deadline; observe120VI after the exact completed frame
    // instead of letting neutral players encounter unrelated enemies later.
    static unsigned room_complete_vi0280=0;bool room_goal0280=false,life_goal0280=false,room_return_goal0286=false,room_reentry_goal0287=false,secondary_return_goal0288=false,secondary_return_rejected0288=false;
    if(test_coop_room_fixture0280 && test_coop_room_route_driver0280 && end_test_vi && !test_inputs.empty() && !test_inputs_p2.empty()){
        namespace rooms=workshop::coop_rooms_native0280;const auto& coordinator=rooms::coordinator;
        namespace lifecycle=workshop::coop_lifecycle0280;const auto& life=lifecycle::lifecycle.snapshot();
        const bool complete=rooms::enabled && !rooms::failed && !workshop::coop_bridge::invalidated && workshop::coop_bridge::context && workshop::coop_bridge::owner_stack.empty() && !workshop::coop_bridge::context->depth() && !rooms::busy() && (hybrid_room&255)==43 && life.bound && !life.failed && !life.ending && life.room==43 && coordinator.epoch()==2 && coordinator.phase()==workshop::coop_rooms0280::Phase::complete && coordinator.arrived()==3 && rooms::arrival_reported[0] && rooms::arrival_reported[1] && coordinator.departure().room==81 && coordinator.departure().route.destination()==43;
        if(complete){
            if(!room_complete_vi0280)room_complete_vi0280=room_frame_vi0280;
            const auto now=test_vi.load();
            const bool alive=lifecycle::lifecycle.alive(rdram,0) && lifecycle::lifecycle.alive(rdram,1);
            if(test_coop_secondary_return0288){
                const auto proof=workshop::coop_secondary_return0288::witness(rdram,now,hybrid_room&255);
                const auto& victim=life.players[1];const auto& departure=coordinator.departure();
                const auto drawing=workshop::coop_native::word(rdram,0x800F8318);
                const bool canonical=alive && life.players[0].root.address==workshop::coop_bridge::bound_primary && victim.root.address==workshop::coop_bridge::bound_secondary && departure.payload.players[0].actor.birth==life.players[0].root.generation && departure.payload.players[1].actor.birth==victim.root.generation && workshop::coop_native::byte(rdram,0x800F7EA5)==0 && workshop::coop_native::byte(rdram,0x800F7EB4)==0 && workshop::coop_native::range(drawing,0xF8) && workshop::coop_native::word(rdram,drawing+0xE8)==life.players[0].root.address && workshop::coop_native::word(rdram,drawing+0xEC)==0x8002ABD0;
                secondary_return_goal0288=canonical && proof.handled && proof.complete && !proof.rejected && proof.death_vi==victim.death_vi && proof.return_vi==victim.respawn_vi && proof.return_vi>=proof.death_vi && proof.return_vi-proof.death_vi>=lifecycle::respawn_delay && proof.arrival_vi>=proof.return_vi && now>=proof.arrival_vi && now-proof.arrival_vi>=1020 && now<=end_test_vi;
            }else if(test_coop_room_reentry0287){
                const auto proof=workshop::coop_room_reentry0287::witness(rdram,now,hybrid_room&255);
                const auto& p0=life.players[0];const auto& departure=coordinator.departure();
                const bool canonical=alive && p0.root.address==workshop::coop_bridge::bound_primary && life.players[1].root.address==workshop::coop_bridge::bound_secondary && departure.payload.players[0].actor.birth==p0.root.generation && departure.payload.players[1].actor.birth==life.players[1].root.generation && workshop::coop_native::byte(rdram,0x800F7EA5)==0 && workshop::coop_native::byte(rdram,0x800F7EB4)==0;
                room_reentry_goal0287=canonical && proof.handled && proof.complete && !proof.rejected && proof.death_vi==p0.death_vi && proof.return_vi==p0.respawn_vi && proof.return_vi>=proof.death_vi && proof.return_vi-proof.death_vi>=lifecycle::respawn_delay && proof.arrival_vi>=proof.return_vi && now>=proof.arrival_vi && now-proof.arrival_vi>=1020 && now<=end_test_vi;
            }else if(test_coop_room_return0286){
                const auto& p0=life.players[0];const auto& original=coordinator.departure().payload.players[0];
                const auto proof=workshop::coop_room_return0286::witness(rdram,now,hybrid_room&255);
                const bool returned=alive && original.count<=6 && p0.death_vi>=room_complete_vi0280 && p0.respawn_vi>=p0.death_vi+lifecycle::respawn_delay && p0.root.generation!=original.actor.birth && workshop::coop_native::range(p0.root.private_data,0x1550) && workshop::coop_native::byte(rdram,p0.root.private_data+0x12C8)==0 && workshop::coop_native::byte(rdram,0x800F7EA5)==0 && workshop::coop_native::byte(rdram,0x800F7EB4)==0;
                room_return_goal0286=returned && proof.handled && proof.complete && !proof.rejected && proof.death_vi==p0.death_vi && proof.return_vi==p0.respawn_vi && now>=p0.respawn_vi && now-p0.respawn_vi>=1020 && now<=end_test_vi;
            }else if(test_coop_room_death0280){
                const auto& p0=life.players[0];const auto& original=coordinator.departure().payload.players[0];
                const bool returned=alive && original.count>0 && p0.death_vi>=room_complete_vi0280 && p0.respawn_vi>=p0.death_vi+lifecycle::respawn_delay && p0.root.generation!=original.actor.birth && workshop::coop_native::byte(rdram,p0.root.private_data+0x12C8)==0 && workshop::coop_native::byte(rdram,0x800F7EA5)==0 && workshop::coop_native::byte(rdram,0x800F7EB4)==0;
                life_goal0280=returned && now>=p0.respawn_vi && now-p0.respawn_vi>=120 && now<=end_test_vi;
            }else room_goal0280=alive && now>=room_complete_vi0280 && now-room_complete_vi0280>=120 && now<=end_test_vi;
        }else room_complete_vi0280=0;
    }
    if(test_coop_secondary_return0288)secondary_return_rejected0288=workshop::coop_secondary_return0288::witness(rdram,test_vi.load(),hybrid_room&255).rejected;
    if((test_coop_native || workshop::boss_campaign0250::enabled) && end_test_vi && (test_vi.load()>=end_test_vi || campaign_goal || campaign_abort || room_goal0280 || life_goal0280 || room_return_goal0286 || room_reentry_goal0287 || secondary_return_goal0288 || secondary_return_rejected0288) && workshop::coop_bridge::owner_stack.empty()){
        const unsigned final_test_vi0280=test_vi.load();
        if(secondary_return_goal0288){const auto proof=workshop::coop_secondary_return0288::witness(rdram,final_test_vi0280,hybrid_room&255);fprintf(stderr,"[CoopSecondaryReturnQA] goal deathVI=%u returnedVI=%u arrivalVI=%u actualVI=%u deadlineVI=%u p1StartVI=%u p1EndVI=%u p2StartVI=%u p2EndVI=%u settledVI=120 nativeObservation=1\n",proof.death_vi,proof.return_vi,proof.arrival_vi,final_test_vi0280,end_test_vi,proof.arrival_vi+180,proof.arrival_vi+420,proof.arrival_vi+660,proof.arrival_vi+900);}
        if(secondary_return_rejected0288){const auto proof=workshop::coop_secondary_return0288::witness(rdram,final_test_vi0280,hybrid_room&255);fprintf(stderr,"[CoopSecondaryReturnQA] rejected vi=%u reason=%s escortStage=%u ordinaryInputRouteRejected=1 forcedGameState=0 gameBugInferred=0\n",final_test_vi0280,proof.rejection_reason?proof.rejection_reason:"unspecified",proof.escort_stage);}
        if(room_reentry_goal0287){const auto proof=workshop::coop_room_reentry0287::witness(rdram,final_test_vi0280,hybrid_room&255);fprintf(stderr,"[CoopRoomReentryQA] goal deathVI=%u returnedVI=%u arrivalVI=%u actualVI=%u deadlineVI=%u p1StartVI=%u p1EndVI=%u p2StartVI=%u p2EndVI=%u settledVI=120 nativeObservation=1\n",proof.death_vi,proof.return_vi,proof.arrival_vi,final_test_vi0280,end_test_vi,proof.arrival_vi+180,proof.arrival_vi+420,proof.arrival_vi+660,proof.arrival_vi+900);}
        if(room_return_goal0286){const auto& player=workshop::coop_lifecycle0280::lifecycle.snapshot().players[0];fprintf(stderr,"[CoopRoomReturnQA] goal deathVI=%u returnedVI=%u actualVI=%u deadlineVI=%u p1StartVI=%u p1EndVI=%u p2StartVI=%u p2EndVI=%u settledVI=120 nativeObservation=1\n",player.death_vi,player.respawn_vi,final_test_vi0280,end_test_vi,player.respawn_vi+180,player.respawn_vi+420,player.respawn_vi+660,player.respawn_vi+900);}
        if(room_goal0280)fprintf(stderr,"[CoopRoomsQA] roundtrip-goal completeVI=%u actualVI=%u deadlineVI=%u settledVI=120 nativeObservation=1\n",room_complete_vi0280,final_test_vi0280,end_test_vi);
        if(life_goal0280){const auto& player=workshop::coop_lifecycle0280::lifecycle.snapshot().players[0];fprintf(stderr,"[CoopLifecycleQA] return-goal deathVI=%u returnedVI=%u actualVI=%u deadlineVI=%u settledVI=120 nativeObservation=1\n",player.death_vi,player.respawn_vi,final_test_vi0280,end_test_vi);}
        if(campaign_goal&&boss_test_retry_replay){
            const auto& a=workshop::boss_campaign0250::candies[0].snapshot().actors[0];
            fprintf(stderr,"[BossRetryVictory] vi=%u actor=%08X private=%08X initialCounter=%d counter=%d causalHits=%u active=%u terminal=%u completionReturned=%u retryEpoch=%u receiptEpoch=%u nativeObservation=1\n",test_vi.load(),a.actor,a.private_data,a.initial_counter,a.counter,a.causal_hits,unsigned(a.active_seen),unsigned(a.terminal_seen),unsigned(a.completion_returned),campaign.retry_epoch,workshop::boss_campaign0250::history[0].members[0].retry_epoch);
        }
        if(campaign_goal)fprintf(stderr,"[BossBattle] automatic-goal-reached goal=%s case=%d deadlineVI=%u actualVI=%u completed=%u\n",boss_test_goal.c_str(),boss_test_case,end_test_vi,test_vi.load(),campaign.completed);
        fprintf(stderr,"[CoopBridge] stable-final requestedVI=%u actualVI=%u global=%08X depth=0\n",end_test_vi,final_test_vi0280,workshop::coop_native::word(rdram,workshop::coop::native_player));
        if(workshop::boss_campaign0250::enabled){const auto& state=workshop::boss_campaign0250::session.snapshot();fprintf(stderr,"[BossBattle] final phase=%u fight=%u completed=%u reason=%s\n",unsigned(state.phase),state.index+1,state.completed,state.failure.c_str());}
        {std::ofstream final("controller-test-final.rdram",std::ios::binary);final.write(reinterpret_cast<char*>(rdram),0x800000);}
        if(!workshop::coop_egg_stress::flush_dumps()){
            fprintf(stderr,"[CoopEggStress] phase=failed reason=dump-flush\n");std::fflush(nullptr);std::_Exit(2);
        }
        if(secondary_return_goal0288)fprintf(stderr,"[Workshop] Automatic test reached its secondary-return goal at VI %u; deadline %u.\n",final_test_vi0280,end_test_vi);
        else if(room_reentry_goal0287)fprintf(stderr,"[Workshop] Automatic test reached its room-reentry goal at VI %u; deadline %u.\n",final_test_vi0280,end_test_vi);
        else if(room_return_goal0286)fprintf(stderr,"[Workshop] Automatic test reached its room-return goal at VI %u; deadline %u.\n",final_test_vi0280,end_test_vi);
        else if(life_goal0280)fprintf(stderr,"[Workshop] Automatic test reached its lifecycle goal at VI %u; deadline %u.\n",final_test_vi0280,end_test_vi);
        else if(room_goal0280)fprintf(stderr,"[Workshop] Automatic test reached its room goal at VI %u; deadline %u.\n",final_test_vi0280,end_test_vi);
        else if(campaign_goal)fprintf(stderr,"[Workshop] Automatic test reached its goal at VI %u; deadline %u.\n",test_vi.load(),end_test_vi);
        else fprintf(stderr,"[Workshop] Automatic test reached VI %u; terminating isolated test.\n",end_test_vi);
        workshop_dump_trace();std::fflush(nullptr);std::_Exit((campaign_abort||secondary_return_rejected0288)?2:0);
    }
}
#endif
extern "C" void workshop_80000438(uint8_t* rdram,recomp_context* ctx) {
    workshop_807F1600(rdram,ctx);
    ctx->r31=MEM_W(ctx->r29,0x14);
    ctx->r29=ADD32(ctx->r29,0x18);
}
#endif

bool workshop_configure(int& argc, char** argv) {
    try {
#ifdef WORKSHOP_CONTENT_BUILD
        configure_diagnostics();
#endif
        int output=1;std::string custom_token;bool custom_automatic=false;int custom_preferred=-1;
        for (int n=1;n<argc;n++) {
            if(std::string_view(argv[n])=="--workshop-wait-parent"){
                workshop::require(n+1<argc,"Expected previous game process");
                workshop::require(workshop_menu_wait_parent(std::stoul(argv[++n])),"Previous game did not finish closing; profile was not reopened");continue;
            }
            if(std::string_view(argv[n])=="--workshop-main-menu"){workshop_menu_enable();continue;}
            if(std::string_view(argv[n])=="--workshop-start-mode"){
                workshop::require(n+1<argc,"Expected book mode");int mode=std::stoi(argv[++n]);
                workshop::require(workshop::game_modes0320::valid(mode),"Invalid book mode");workshop_menu_start_mode(mode);continue;
            }
#ifdef WORKSHOP_CONTENT_BUILD
            if(std::string_view(argv[n])=="--workshop-bonus-bridge0352"){bonus_preview0352=true;continue;}
            if(std::string_view(argv[n])=="--workshop-bonus-route0353"){
                workshop::require(n+2<argc,"Expected Bonus page and seed");bonus_page0353=std::stoul(argv[++n]);bonus_seed0353=std::stoull(argv[++n]);
                workshop::require(bonus_page0353>=1&&bonus_page0353<=6,"Expected Bonus page1..6");bonus_route0353=true;continue;
            }
            if(std::string_view(argv[n])=="--workshop-bonus-finales0353"){
                workshop::require(n+2<argc,"Expected complete Bonus page and seed");bonus_page0353=std::stoul(argv[++n]);bonus_seed0353=std::stoull(argv[++n]);
                workshop::require(bonus_page0353>=1&&bonus_page0353<=6,"Expected Bonus page1..6");bonus_complete0353=true;continue;
            }
            if(std::string_view(argv[n])=="--workshop-bonus-selection0353"){
                workshop::require(n+3<argc,"Expected Bonus page, seed and restart choice");bonus_page0353=std::stoul(argv[++n]);bonus_seed0353=std::stoull(argv[++n]);const auto choice=std::stoul(argv[++n]);
                workshop::require(bonus_page0353>=1&&bonus_page0353<=6&&choice<=1,"Invalid Bonus selection");bonus_new0353=choice!=0;bonus_selection0353=true;continue;
            }
            if(std::string_view(argv[n])=="--workshop-hard-preview"){hard_preview0330=true;continue;}
            if(std::string_view(argv[n])=="--workshop-hard-test-egg"){
                workshop::require(n+1<argc,"Expected Hard preview egg color");hard_test_egg0330=std::stoi(argv[++n]);workshop::require(hard_test_egg0330>=1&&hard_test_egg0330<18,"Invalid Hard preview egg color");continue;
            }
#ifdef WORKSHOP_NATIVE_COOP
            if(std::string_view(argv[n])=="--workshop-test-coop-native"){test_coop_native=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-coop-room-fixture0280"){test_coop_room_fixture0280=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-coop-room-death0280"){test_coop_room_death0280=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-coop-room-return0286"){test_coop_room_return0286=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-coop-room-reentry0287"){test_coop_room_reentry0287=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-coop-secondary-return0288"){test_coop_secondary_return0288=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-coop-room-route-driver0280"){
                workshop::require(n+1<argc,"Expected room route leader0/1");test_coop_room_route_driver0280=true;test_coop_room_route_leader0280=std::stoul(argv[++n]);workshop::require(test_coop_room_route_leader0280<2,"Expected room route leader0/1");continue;
            }
            if(std::string_view(argv[n])=="--workshop-test-coop-manual"){test_coop_manual=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-coop-interactions"){test_coop_interactions=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-coop-egg-probe"){test_coop_egg_probe=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-coop-egg-stress"){test_coop_egg_stress=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-shared-egg-pools"){test_coop_shared_egg_pools=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-coop-color"){
                workshop::require(n+1<argc,"Expected secondary Yoshi colour");test_coop_color=std::stoul(argv[++n]);workshop::require(test_coop_color<14,"Invalid secondary Yoshi colour");continue;
            }
            if(std::string_view(argv[n])=="--workshop-boss-battle"){
                workshop::require(n+1<argc && boss_battle_token.empty(),"Expected one Boss Battle token");boss_battle_token=argv[++n];continue;
            }
            if(std::string_view(argv[n])=="--workshop-test-boss-menu"){boss_test_menu=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-case"){
                workshop::require(n+1<argc,"Expected diagnostic encounter index");boss_test_case=std::stoi(argv[++n]);workshop::require(boss_test_case>=0 && boss_test_case<10,"Invalid diagnostic encounter index");continue;
            }
            if(std::string_view(argv[n])=="--workshop-test-boss-goal"){
                workshop::require(n+1<argc,"Expected diagnostic Boss Battle goal");boss_test_goal=argv[++n];workshop::require(boss_test_goal=="ready" || boss_test_goal=="victory" || boss_test_goal=="full" || boss_test_goal=="route-checkpoints" || boss_test_goal=="route" || boss_test_goal=="checkpoint" || boss_test_goal=="second-victory" || boss_test_goal=="ground-roundtrip" || boss_test_goal=="retry-victory" || boss_test_goal=="swallow-checkpoint" || boss_test_goal=="transition-rest" || boss_test_goal=="fish-cp6-replay" || boss_test_goal=="fish-cp6-focused-replay" || boss_test_goal=="fish-cp6-rebirth","Invalid diagnostic Boss Battle goal");continue;
            }
            if(std::string_view(argv[n])=="--workshop-boss-seed"){
                workshop::require(n+1<argc,"Expected Boss Battle seed");boss_battle_seed=std::stoull(argv[++n]);continue;
            }
            if(std::string_view(argv[n])=="--workshop-boss-solo"){
                workshop::require(n+1<argc && boss_solo_token.empty(),"Expected one solo boss session token");boss_solo_token=argv[++n];continue;
            }
            if(std::string_view(argv[n])=="--workshop-test-boss-followup-input"){
                workshop::require(n+1<argc&&boss_followup_inputs.empty(),"Expected one followup controller file");std::ifstream stream(std::filesystem::u8path(argv[++n]));workshop::require(bool(stream),"Cannot read followup controller file");
                workshop::boss_followup_input0260::Event event{};unsigned buttons;int x,y;
                while(stream>>event.first>>event.last>>std::hex>>buttons>>std::dec>>x>>y){workshop::require(buttons<=65535&&x>=-80&&x<=80&&y>=-80&&y<=80,"Invalid followup controller units");event.buttons=uint16_t(buttons);event.x=float(x)/80;event.y=float(y)/80;boss_followup_inputs.push_back(event);}
                workshop::require(stream.eof()&&workshop::boss_followup_input0260::valid_events(boss_followup_inputs),"Invalid bounded followup controller file");continue;
            }
            if(std::string_view(argv[n])=="--workshop-test-boss-retry-replay"){boss_test_retry_replay=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-checkpoint"){boss_test_checkpoint=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-swallow-checkpoint"){boss_test_swallow_checkpoint=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-manual"){boss_manual_handoff=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-full-loadout"){boss_test_full_loadout=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-automatic"){boss_test_automatic=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-route-checkpoints"){boss_test_route_checkpoints=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-blargg-shoot0289"){boss_test_blargg_shoot0289=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-blargg-acquire0289"){boss_test_blargg_acquire0289=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-blargg0289"){boss_test_blargg0289=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-fish-cp6"){boss_test_fish_cp6=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-fish-cp6-focused"){boss_test_fish_cp6_focused=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-lips-loadout"){boss_test_lips_loadout=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-genie-loadout"){boss_test_genie_loadout=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-genie-empty-reserves"){boss_test_genie_empty_reserves=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-monsters-loadout"){boss_test_monsters_loadout=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-fish-loadout"){boss_test_fish_loadout=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-final-loadout"){boss_test_final_loadout=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-special-fish"){boss_test_special_fish=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-boss-driver"){boss_solo_driver=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-first-boss"){
                workshop::require(n+1<argc,"Expected isolated boss token");test_first_boss=argv[++n];workshop::require(!test_first_boss.empty() && test_first_boss.size()<=128,"Invalid boss token");continue;
            }
            if(std::string_view(argv[n])=="--workshop-input-player2"){
                workshop::require(n+1<argc && test_inputs_p2.empty(),"Expected one secondary controller script");
                std::ifstream script(std::filesystem::u8path(argv[++n]));workshop::require(bool(script),"Cannot read secondary controller script");
                TestInput e{};unsigned b;int x,y;
                while(script>>e.first>>e.last>>std::hex>>b>>std::dec>>x>>y){workshop::require(e.first<e.last && b<=65535 && x>=-80 && x<=80 && y>=-80 && y<=80,"Invalid secondary input");e.buttons=uint16_t(b);e.x=float(x)/80;e.y=float(y)/80;test_inputs_p2.push_back(e);}
                workshop::require(script.eof() && !test_inputs_p2.empty(),"Invalid secondary script");continue;
            }
#endif
            if(std::string_view(argv[n])=="--workshop-background-test"){background_test=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-book-resume"){test_book_resume=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-book-shop"){test_book_shop=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-find-reveal"){test_find_reveal=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-editor-eggs"){test_editor_eggs=true;continue;}
            if(std::string_view(argv[n])=="--workshop-release-test-spawn"){test_release_spawn=true;continue;}
            if(std::string_view(argv[n])=="--workshop-fruit-audit"){fruit_audit=true;fprintf(stderr,"[FruitAudit] Passive capture enabled; controls and quitting remain manual.\n");continue;}
            if(std::string_view(argv[n])=="--workshop-native-hud"){special_hud=false;continue;}
            if(std::string_view(argv[n])=="--workshop-trace-startup"){trace_startup=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-special-victory"){
                workshop::require(n+1<argc,"Expected special fruit index");test_special_victory=std::stoi(argv[++n]);workshop::require(test_special_victory>=0 && test_special_victory<6,"Invalid special fruit index");test_tally=true;continue;
            }
            if(std::string_view(argv[n])=="--workshop-test-tally"){test_tally=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-hybrid"){test_hybrid=true;continue;}
            if(std::string_view(argv[n])=="--workshop-relative-play"){relative_play=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-software-gpu"){test_software_gpu0284=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-native-history"){
                workshop::require(n+1<argc,"Expected native history stress count");test_native_history=std::stoul(argv[++n]);
                workshop::require(test_native_history>=64 && test_native_history<=24000,"Invalid native history stress count");continue;
            }
            if(std::string_view(argv[n])=="--workshop-custom-token"){
                workshop::require(n+1<argc,"Expected custom token");custom_token=argv[++n];continue;
            }
            if(std::string_view(argv[n])=="--workshop-custom-continue"){custom_automatic=true;continue;}
            if(std::string_view(argv[n])=="--workshop-preferred-color"){
                workshop::require(n+1<argc,"Expected preferred Yoshi");custom_preferred=std::stoi(argv[++n]);continue;
            }
            if(std::string_view(argv[n])=="--workshop-test-stage"){
                workshop::require(n+1<argc,"Expected test stage index");test_stage=std::stoi(argv[++n]);
                workshop::require(test_stage>=0 && test_stage<24,"Invalid test stage");continue;
            }
            if(std::string_view(argv[n])=="--workshop-fruit-seed"){
                workshop::require(n+1<argc,"Expected diagnostic fruit seed");fruit_seed=std::stoul(argv[++n]);workshop::require(fruit_seed!=0,"Invalid fruit seed");continue;
            }
            if(std::string_view(argv[n])=="--workshop-test-zone"){
                workshop::require(n+1<argc,"Expected diagnostic zone");test_zone=std::stoi(argv[++n]);workshop::require(test_zone>=0 && test_zone<177,"Invalid test zone");continue;
            }
            if(std::string_view(argv[n])=="--workshop-test-spawn"){
                workshop::require(n+2<argc,"Expected diagnostic spawn X Y");test_spawn=true;test_spawn_x=std::stof(argv[++n]);test_spawn_y=std::stof(argv[++n]);
                workshop::require(std::isfinite(test_spawn_x) && std::isfinite(test_spawn_y) && std::abs(test_spawn_x)<100000 && std::abs(test_spawn_y)<100000,"Invalid diagnostic spawn");continue;
            }
            if(std::string_view(argv[n])=="--workshop-test-warp"){
                workshop::require(n+3<argc,"Expected diagnostic warp X Y playback frame");test_warp=true;test_warp_x=std::stof(argv[++n]);test_warp_y=std::stof(argv[++n]);test_warp_at=std::stoul(argv[++n]);
                workshop::require(std::isfinite(test_warp_x) && std::isfinite(test_warp_y) && std::abs(test_warp_x)<100000 && std::abs(test_warp_y)<100000 && test_warp_at>=3000 && test_warp_at<=12000,"Invalid diagnostic warp");continue;
            }
            if(std::string_view(argv[n])=="--workshop-end-vi"){
                workshop::require(n+1<argc,"Expected test duration");end_test_vi=std::stoul(argv[++n]);
                workshop::require(end_test_vi>=600 && end_test_vi<=108000,"Invalid test duration");continue;
            }
            if(std::string_view(argv[n])=="--workshop-auto-color"){
                workshop::require(n+1<argc,"Expected test Yoshi color");test_color=std::stoi(argv[++n]);workshop::require(test_color>=0 && test_color<18,"Invalid test Yoshi color");continue;
            }
            if(std::string_view(argv[n])=="--workshop-input"){
                workshop::require(n+1<argc,"Expected one controller test script");
                std::ifstream script(std::filesystem::u8path(argv[++n]));workshop::require(bool(script),"Cannot read controller test script");
                TestInput event{};unsigned buttons;int x,y;
                while(script>>event.first>>event.last>>std::hex>>buttons>>std::dec>>x>>y){
                    workshop::require(event.first<event.last && buttons<=65535 && abs(x)<=80 && abs(y)<=80,"Invalid controller test event");
                    event.buttons=uint16_t(buttons);event.x=float(x)/80;event.y=float(y)/80;test_inputs.push_back(event);
                }
                workshop::require(script.eof() && !test_inputs.empty(),"Invalid controller test script");
                continue;
            }
#endif
            if (std::string_view(argv[n])!="--workshop-level") {argv[output++]=argv[n];continue;}
            workshop::require(payload.empty() && n+1<argc, "Expected one --workshop-level file");
            std::ifstream stream(std::filesystem::u8path(argv[++n]),std::ios::binary|std::ios::ate);
            workshop::require(bool(stream), "Cannot open custom placement file");
            auto size=stream.tellg();
            workshop::require(size>=16 && size<=2097152, "Invalid placement file size");
            payload.resize(size_t(size));stream.seekg(0);
            workshop::require(bool(stream.read(reinterpret_cast<char*>(payload.data()),payload.size())), "Cannot read custom placements");
            workshop::validate_payload(payload);
        }
        argc=output;argv[output]=nullptr;
        workshop::require((!custom_automatic && custom_preferred<0) || !custom_token.empty(),"Custom continuation requires a run token");
        if(!custom_token.empty()){
            workshop::require(!payload.empty(),"Custom session requires a level payload");
            workshop::custom_session::configure(custom_token,custom_automatic,custom_preferred,workshop::u32(payload,8));
        }
#ifdef WORKSHOP_CONTENT_BUILD
        workshop::require((!background_test && !trace_startup && !end_test_vi && !test_tally && !test_hybrid && !test_book_resume && !test_book_shop && !test_find_reveal && !test_editor_eggs && !test_release_spawn && !relative_play && test_stage<0 && test_zone<0 && !test_spawn && !test_warp && !fruit_seed && !test_native_history) || (!test_inputs.empty() && end_test_vi),"Automatic diagnostics require scripted input and a finite duration");
#endif
#ifdef WORKSHOP_CONTENT_BUILD
#ifdef _WIN32
        const char* inherited_software_gpu=std::getenv("DINOTALE_QA_WARP0284");
        workshop::require(!inherited_software_gpu || std::string_view(inherited_software_gpu)!="1" || test_software_gpu0284,"Software GPU QA requires its explicit bounded diagnostic flag");
        if(test_software_gpu0284){
            std::ifstream marker("software-gpu-qa0284.txt");std::string label;std::getline(marker,label);
            workshop::require(label=="DinoTale software GPU QA 0.28.4" && std::filesystem::is_regular_file("portable.txt") && !background_test && !test_inputs.empty() && end_test_vi>=3600 && end_test_vi<=72000,"Software GPU QA requires an isolated visible portable profile, scripted input and a finite duration");
            workshop::require(_putenv_s("DINOTALE_QA_WARP0284","1")==0,"Cannot enable software GPU for this QA process");
            fprintf(stderr,"[SoftwareGPUQA] requested=WARP rendererOnly=1 visible=1 deadlineVI=%u\n",end_test_vi);
        }
#else
        workshop::require(!test_software_gpu0284,"Software GPU QA is available only on Windows");
#endif
        workshop::require(!test_editor_eggs || (!payload.empty() && end_test_vi>=3000),"Editor egg diagnostics require a custom payload and at least 3000 VI");
        if(test_native_history)workshop_request_native_history_test(test_native_history);
#ifdef WORKSHOP_NATIVE_COOP
        if(test_coop_manual){
            std::ifstream marker("manual-coop-qa.txt");std::string label;std::getline(marker,label);
            workshop::require(test_coop_native && test_inputs.empty() && test_inputs_p2.empty() && !end_test_vi && !background_test && !trace_startup && !relative_play && test_first_boss.empty(),"Manual co-op requires live input in its isolated QA profile");
            workshop::require(label=="DinoTale manual co-op 0.23.2" && std::filesystem::is_regular_file("portable.txt"),"Manual co-op requires its dedicated portable QA profile");
            fprintf(stderr,"[CoopManual] live keyboard=P1 gamepad=P2 profile=isolated-qa automaticExit=0\n");
        }
        workshop::require((!test_coop_native && test_inputs_p2.empty() && test_first_boss.empty()) || test_coop_manual || (!test_inputs.empty() && end_test_vi>=3000),"Native prototypes require bounded scripted diagnostics or the dedicated manual QA profile");
        workshop::require(test_inputs_p2.empty() || test_coop_native,"Secondary script requires native co-op prototype");
        bool room_fixture_scope0280=false;
        if(test_coop_room_fixture0280){
            std::ifstream marker("coop-room-qa0280.txt");std::string label;std::getline(marker,label);
            uint64_t fingerprint=14695981039346656037ull;for(const auto value:payload)fingerprint=(fingerprint^value)*1099511628211ull;
            room_fixture_scope0280=test_coop_native && !test_coop_manual && !test_coop_egg_probe && !test_coop_egg_stress &&
                !background_test && relative_play && !test_inputs.empty() && !test_inputs_p2.empty() &&
                end_test_vi>=4200 && end_test_vi<=12000 && custom_token.empty() && boss_solo_token.empty() && boss_battle_token.empty() &&
                test_first_boss.empty() && test_stage<0 && test_zone<0 && !test_spawn && !test_warp &&
                payload.size()==2016 && fingerprint==0x64A2218ADDDFCF30ull &&
                label=="DinoTale coop room QA 0.28.0" && std::filesystem::is_regular_file("portable.txt");
            workshop::require(room_fixture_scope0280,"Co-op room QA requires its exact isolated declared-entry fixture and bounded visible controller scripts");
            // Dedicated bounded QA only. Preserve abort behavior while recording
            // uncaught host exceptions that otherwise terminate before the native
            // crash callback can produce evidence. No guest state is changed.
            std::set_terminate([]{
                const auto error=std::current_exception();
                if(error){try{std::rethrow_exception(error);}catch(const std::exception& e){fprintf(stderr,"[CoopRooms] host-terminate vi=%u exception=%s\n",test_vi.load(),e.what());}catch(...){fprintf(stderr,"[CoopRooms] host-terminate vi=%u exception=non-standard\n",test_vi.load());}}
                else fprintf(stderr,"[CoopRooms] host-terminate vi=%u exception=none\n",test_vi.load());
                fflush(stderr);
                try{if(diagnostic_rdram){std::ofstream dump("coop-room-host-terminate.rdram",std::ios::binary);dump.write(reinterpret_cast<const char*>(diagnostic_rdram),0x800000);}}catch(...){}
                std::abort();
            });
        }
        workshop::require(!test_coop_room_route_driver0280 || (room_fixture_scope0280 && end_test_vi>=7200 && end_test_vi<=12000),"Room route input driver requires the exact isolated fixture and7200..12000VI");
        workshop::require(!test_coop_room_death0280 || (room_fixture_scope0280 && test_coop_room_route_driver0280 && test_coop_room_route_leader0280==1 && end_test_vi>=9000 && end_test_vi<=12000),"After-room lifecycle QA requires the exact isolated fixture, leader1 route driver and9000..12000VI");
        if(test_coop_room_return0286){
            std::ifstream marker("coop-room-return-qa0286.txt");std::string label;std::getline(marker,label);
            workshop::require(room_fixture_scope0280 && test_coop_room_route_driver0280 && test_coop_room_route_leader0280==0 && !test_coop_room_death0280 && end_test_vi>=10000 && end_test_vi<=12000 && label=="DinoTale coop room return QA 0.28.6","Room-return QA requires its dedicated marker, exact isolated fixture, leader0 route driver and10000..12000VI");
        }
        if(test_coop_room_reentry0287){
            std::ifstream marker("coop-room-reentry-qa0287.txt");std::string label;std::getline(marker,label);
            workshop::require(room_fixture_scope0280 && test_coop_room_route_driver0280 && test_coop_room_route_leader0280==0 && !test_coop_room_death0280 && !test_coop_room_return0286 && end_test_vi>=10000 && end_test_vi<=12000 && label=="DinoTale coop room reentry QA 0.28.7","Room-reentry QA requires its dedicated marker, exact isolated fixture, leader0 route driver and10000..12000VI");
        }
        if(test_coop_secondary_return0288){
            std::ifstream marker("coop-secondary-return-qa0288.txt");std::string label;std::getline(marker,label);
            workshop::require(room_fixture_scope0280 && test_coop_room_route_driver0280 && test_coop_room_route_leader0280==0 && !test_coop_room_death0280 && !test_coop_room_return0286 && !test_coop_room_reentry0287 && end_test_vi>=10000 && end_test_vi<=12000 && label=="DinoTale secondary return QA 0.28.8","Secondary-return QA requires its dedicated marker, exact isolated fixture, leader0 and10000..12000VI");
        }
        workshop::coop_secondary_return0288::configure(test_coop_secondary_return0288);
        workshop::coop_room_reentry0287::configure(test_coop_room_reentry0287);
        workshop::coop_room_return0286::configure(test_coop_room_return0286);
        workshop::coop_room_route_driver0280::configure(test_coop_room_route_driver0280,test_coop_room_death0280);
        workshop::require(!test_coop_native || (test_first_boss.empty() && test_stage<0 && test_zone<0 && !test_spawn && !test_warp && (payload.empty() || room_fixture_scope0280)),"Co-op prototype requires native Trial 1-1 or its exact isolated room QA fixture");
        workshop::require(test_first_boss.empty() || (test_stage==8 && test_zone==162 && end_test_vi>=3600 && !test_coop_native),"First boss diagnostic requires stage8 room162 and a finite duration");
        workshop::require(!test_coop_interactions || test_coop_native,"Player interactions require the bounded native co-op prototype");
        workshop::require(!test_coop_egg_probe || test_coop_native,"Egg provisioning requires the bounded native co-op prototype");
        workshop::require(!test_coop_shared_egg_pools || (test_coop_egg_stress && !test_coop_manual && end_test_vi>=3000),"Stock egg pool comparison requires the bounded egg stress diagnostic");
        workshop::coop_bridge::configure(test_coop_native,test_coop_color,workshop_get_input,test_coop_interactions,!test_coop_shared_egg_pools);
        workshop::require(!test_coop_egg_stress || (test_coop_native && !test_coop_egg_probe && !test_coop_manual && end_test_vi>=3000),"Egg stress requires the bounded scripted co-op QA profile");
        workshop::coop_probe::configure(test_coop_egg_probe);
        workshop::coop_egg_stress::configure(test_coop_egg_stress,test_coop_shared_egg_pools);
        workshop::require(boss_solo_token.empty() || (!test_coop_native && test_first_boss.empty() && payload.empty() && test_stage<0 && test_zone<0 && !test_spawn && !test_warp),"Solo boss milestone requires its own native scene, without co-op/custom/diagnostic room overrides");
        if(boss_test_blargg0289){
            std::ifstream marker("boss-blargg-qa0289.txt");std::string label;std::getline(marker,label);
            workshop::require(label=="DinoTale Blargg prototype 0.28.9"&&std::filesystem::is_regular_file("portable.txt"),"Blargg prototype requires its separate portable profile");
            workshop::require(!boss_battle_token.empty()&&!test_coop_native&&!background_test&&!boss_solo_driver&&!test_inputs.empty()&&end_test_vi>=3600&&end_test_vi<=12000&&boss_test_goal=="route"&&!boss_test_automatic&&!boss_test_full_loadout&&!boss_test_special_fish&&!boss_test_lips_loadout&&!boss_test_genie_loadout&&!boss_test_genie_empty_reserves&&!boss_test_monsters_loadout&&!boss_test_fish_loadout&&!boss_test_final_loadout&&!boss_test_route_checkpoints&&!boss_test_checkpoint&&!boss_test_retry_replay&&!boss_test_fish_cp6&&!boss_test_fish_cp6_focused&&!boss_test_swallow_checkpoint,"Blargg prototype requires an isolated bounded solo observation with no unrelated test fixtures");
            workshop::boss_battle0250::Config cfg;cfg.blargg=true;const auto plan=workshop::boss_battle0250::make_plan(boss_battle_seed,cfg);
            workshop::require(workshop::boss_battle0250::valid(plan),"Invalid opt-in Blargg plan");
            if(boss_test_case<0)for(unsigned i=1;i<=8;++i)if(plan.fights[i].family==workshop::boss_battle0250::Family::Blargg){boss_test_case=int(i);break;}
            workshop::require(boss_test_case>=1&&boss_test_case<=8&&plan.fights[unsigned(boss_test_case)].family==workshop::boss_battle0250::Family::Blargg,"Blargg prototype requires a real Blargg encounter in its opt-in plan");
            fprintf(stderr,"[BossBlarggQA] configured seed=%llu case=%d initialDiagnosticEntry=1 skippedPrefixNotVerified=1 forcedVictory=0\n",(unsigned long long)boss_battle_seed,boss_test_case);
        }
        workshop::require(end_test_vi<=12000 || (!boss_battle_token.empty() && (boss_test_goal=="full" || (boss_test_goal=="route-checkpoints"&&boss_test_route_checkpoints) || (boss_test_goal=="fish-cp6-replay"&&boss_test_fish_cp6&&end_test_vi<=72000) || ((boss_test_goal=="fish-cp6-focused-replay"||boss_test_goal=="fish-cp6-rebirth")&&boss_test_fish_cp6_focused&&end_test_vi<=36000) || (boss_test_goal=="swallow-checkpoint"&&boss_test_swallow_checkpoint&&end_test_vi<=24000)) && !test_inputs.empty()) || (boss_test_goal=="second-victory"&&!boss_followup_inputs.empty()&&end_test_vi<=24000),"Long diagnostics require a bounded full Boss Battle run");
        unsigned boss_last_script_vi=0;for(const auto& event:test_inputs)boss_last_script_vi=std::max(boss_last_script_vi,event.last);
        workshop::require(!boss_test_blargg_acquire0289 || (boss_test_blargg0289 && !boss_manual_handoff && end_test_vi>=6000 && end_test_vi<=12000 && boss_last_script_vi<=1900),"Blargg acquisition QA requires its own prototype flag, bounded duration and startup-only controller file");
        workshop::require(!boss_test_blargg_shoot0289 || (boss_test_blargg_acquire0289 && end_test_vi>=10000 && end_test_vi<=12000),"Blargg shooting QA requires the isolated acquisition pilot and10000..12000VI");
        workshop::require(workshop::boss_manual_handoff0260::valid(boss_manual_handoff,!boss_battle_token.empty(),boss_solo_driver,background_test,boss_test_case,end_test_vi,boss_last_script_vi),"Manual Boss Battle requires an isolated visible case, startup-only inputs, and no automatic driver");
        workshop::require(boss_test_goal!="transition-rest" || (!boss_battle_token.empty() && boss_test_case==7 && boss_battle_seed==249908710788600ull && boss_solo_driver && !boss_manual_handoff && !background_test && !test_inputs.empty() && end_test_vi>=3600 && end_test_vi<=12000 && !boss_test_automatic && !boss_test_full_loadout && !boss_test_special_fish && !boss_test_route_checkpoints && !boss_test_lips_loadout && boss_test_genie_loadout && !boss_test_genie_empty_reserves && !boss_test_monsters_loadout && !boss_test_checkpoint && !boss_test_retry_replay && !boss_test_final_loadout && boss_followup_inputs.empty()),"Genie pot QA requires the exact isolated visible seed/case7 and ordinary bounded controller driver");
        workshop::require(!boss_test_automatic || (boss_test_full_loadout && !boss_manual_handoff && boss_solo_driver && !boss_battle_token.empty() && boss_test_case==4 && boss_battle_seed==1 && !background_test && !test_inputs.empty() && boss_test_goal=="victory" && end_test_vi>=3600 && end_test_vi<=12000),"Automatic full-loadout Fish test requires its explicit isolated visible bounded driver case");
        workshop::require(!boss_test_full_loadout || (!boss_battle_token.empty() && boss_test_case==4 && !background_test && !test_inputs.empty() && end_test_vi>=3600 && ((boss_manual_handoff && !boss_solo_driver && !boss_test_automatic) || (boss_test_automatic && boss_solo_driver && !boss_manual_handoff))),"Full test loadout requires an explicit isolated visible manual or automatic Fish case");
        workshop::require(!boss_test_special_fish || (boss_test_full_loadout && (boss_manual_handoff != boss_test_automatic) && boss_test_case==4 && boss_battle_seed==1),"Special Fish preview requires an explicit full-loadout case4 and seed1");
        workshop::require(!boss_test_route_checkpoints || (!boss_battle_token.empty() && boss_test_case==0 && boss_battle_seed==1 && boss_test_goal=="route-checkpoints" && boss_solo_driver && !boss_manual_handoff && !background_test && !test_inputs.empty() && end_test_vi>=12000 && end_test_vi<=108000 && !boss_test_automatic && !boss_test_full_loadout && !boss_test_special_fish && !boss_test_lips_loadout && !boss_test_genie_loadout && !boss_test_genie_empty_reserves && !boss_test_monsters_loadout && !boss_test_checkpoint && !boss_test_retry_replay && !boss_test_final_loadout && boss_followup_inputs.empty()),"Route checkpoint QA requires its isolated visible seed1 full campaign and ordinary bounded driver");
        workshop::require(boss_test_goal!="route-checkpoints" || boss_test_route_checkpoints,"Route checkpoint goal requires its explicit diagnostic flag");
        workshop::require(!boss_test_lips_loadout || (!boss_battle_token.empty() && boss_test_case==1 && boss_battle_seed==1 && boss_test_goal=="victory" && boss_solo_driver && !boss_manual_handoff && !background_test && !test_inputs.empty() && end_test_vi>=3600 && end_test_vi<=9000 && !boss_test_automatic && !boss_test_full_loadout && !boss_test_special_fish && !boss_test_route_checkpoints && !boss_test_genie_loadout && !boss_test_genie_empty_reserves && !boss_test_monsters_loadout && !boss_test_checkpoint && !boss_test_retry_replay && !boss_test_final_loadout && boss_followup_inputs.empty()),"Lips loadout QA requires its isolated visible bounded case1 ordinary driver");
        workshop::require(!boss_test_genie_loadout || (!boss_battle_token.empty() && ((boss_test_case==2 && boss_battle_seed==1 && boss_test_goal=="victory") || (boss_test_case==7 && boss_battle_seed==249908710788600ull && boss_test_goal=="transition-rest" && !boss_test_genie_empty_reserves)) && boss_solo_driver && !boss_manual_handoff && !background_test && !test_inputs.empty() && end_test_vi>=3600 && end_test_vi<=(boss_test_case==7?12000u:9000u) && !boss_test_automatic && !boss_test_full_loadout && !boss_test_special_fish && !boss_test_route_checkpoints && !boss_test_lips_loadout && !boss_test_monsters_loadout && !boss_test_checkpoint && !boss_test_retry_replay && !boss_test_final_loadout && boss_followup_inputs.empty()),"Genie loadout QA requires its isolated visible bounded case2 ordinary driver");
        workshop::require(!boss_test_fish_loadout || (!boss_battle_token.empty() && workshop::boss_test_loadout0260::fish_tuple(boss_battle_seed,unsigned(boss_test_case)) && boss_test_goal=="victory" && boss_solo_driver && !boss_manual_handoff && !background_test && !test_inputs.empty() && end_test_vi>=3600 && end_test_vi<=12000 && !boss_test_menu && !boss_test_automatic && !boss_test_full_loadout && !boss_test_special_fish && !boss_test_route_checkpoints && !boss_test_lips_loadout && !boss_test_genie_loadout && !boss_test_genie_empty_reserves && !boss_test_monsters_loadout && !boss_test_checkpoint && !boss_test_retry_replay && !boss_test_final_loadout && boss_followup_inputs.empty()),"Fish loadout QA requires exact seed/case1/4,7/3,17/2 and its isolated visible bounded victory driver");
        workshop::require(!boss_test_genie_empty_reserves || boss_test_genie_loadout,"Empty Genie reserves require the explicit isolated Genie loadout case");
        workshop::require(!boss_test_monsters_loadout || (!boss_battle_token.empty() && boss_test_case==3 && boss_battle_seed==1 && boss_test_goal=="victory" && boss_solo_driver && !boss_manual_handoff && !background_test && !test_inputs.empty() && end_test_vi>=3600 && end_test_vi<=12000 && !boss_test_automatic && !boss_test_full_loadout && !boss_test_special_fish && !boss_test_route_checkpoints && !boss_test_lips_loadout && !boss_test_genie_loadout && !boss_test_genie_empty_reserves && !boss_test_checkpoint && !boss_test_retry_replay && !boss_test_final_loadout && boss_followup_inputs.empty()),"Monsters loadout QA requires its isolated visible bounded case3 ordinary driver");
        workshop::require(!boss_test_final_loadout || (!boss_battle_token.empty() && boss_test_case==9 && boss_battle_seed==1 && boss_test_goal=="victory" && boss_solo_driver && !boss_manual_handoff && !background_test && !test_inputs.empty() && end_test_vi>=3600 && end_test_vi<=12000 && !boss_test_menu && !boss_test_automatic && !boss_test_full_loadout && !boss_test_special_fish && !boss_test_route_checkpoints && !boss_test_lips_loadout && !boss_test_genie_loadout && !boss_test_genie_empty_reserves && !boss_test_monsters_loadout && !boss_test_fish_loadout && !boss_test_checkpoint && !boss_test_retry_replay && boss_followup_inputs.empty()),"Final loadout QA requires its isolated visible seed1 bounded case9 ordinary driver");
        workshop::require(boss_test_goal!="full" || !boss_test_final_loadout,"Full campaign QA always keeps natural resources");
        workshop::require(boss_test_goal!="full" || !boss_test_fish_loadout,"Full campaign QA never injects a Fish loadout");
        workshop::require(!boss_test_swallow_checkpoint || (!boss_battle_token.empty() && boss_test_case==0 && boss_battle_seed==163184534930300ull && boss_test_goal=="swallow-checkpoint" && boss_solo_driver && !boss_manual_handoff && !background_test && !boss_test_menu && !test_inputs.empty() && end_test_vi>=12000 && end_test_vi<=24000 && !test_coop_native && !test_coop_manual && !boss_test_route_checkpoints && !boss_test_full_loadout && !boss_test_special_fish && !boss_test_automatic && !boss_test_lips_loadout && !boss_test_genie_loadout && !boss_test_genie_empty_reserves && !boss_test_monsters_loadout && !boss_test_checkpoint && !boss_test_retry_replay && !boss_test_fish_loadout && !boss_test_final_loadout && boss_followup_inputs.empty()),"Swallow checkpoint QA requires the exact isolated visible seed/case0 and ordinary controller-only driver");
        workshop::require(boss_test_goal!="swallow-checkpoint" || boss_test_swallow_checkpoint,"Swallow checkpoint goal requires its explicit isolated flag");
        workshop::require(!boss_test_fish_cp6 || (!boss_battle_token.empty() && boss_test_case==0 && boss_battle_seed==37784098137700ull && boss_test_goal=="fish-cp6-replay" && boss_solo_driver && !boss_manual_handoff && !background_test && !boss_test_menu && !test_inputs.empty() && end_test_vi>=18000 && end_test_vi<=72000 && !test_coop_native && !boss_test_route_checkpoints && !boss_test_swallow_checkpoint && !boss_test_full_loadout && !boss_test_special_fish && !boss_test_automatic && !boss_test_lips_loadout && !boss_test_genie_loadout && !boss_test_genie_empty_reserves && !boss_test_monsters_loadout && !boss_test_checkpoint && !boss_test_retry_replay && !boss_test_fish_loadout && !boss_test_final_loadout && boss_followup_inputs.empty()),"Fish CP6 replay QA requires its exact isolated visible case0/seed, controller-only prefix and no forced loadout");
        workshop::require(boss_test_goal!="fish-cp6-replay" || boss_test_fish_cp6,"Fish CP6 replay goal requires its explicit isolated flag");
        if(boss_test_fish_cp6){std::ifstream marker("fish-cp6-qa0284.txt");std::string label;std::getline(marker,label);workshop::require(label=="DinoTale Fish CP6 retry 0.28.4","Fish CP6 QA marker required");}
        workshop::require(!boss_test_fish_cp6_focused || (!boss_battle_token.empty() && relative_play && test_color==0 && !trace_startup && !test_tally && test_special_victory<0 && !test_hybrid && !fruit_seed && !test_native_history && !test_book_resume && !test_book_shop && !test_find_reveal && !test_editor_eggs && !test_release_spawn && !test_coop_interactions && !test_coop_egg_probe && !test_coop_egg_stress && !test_coop_shared_egg_pools && !test_coop_room_fixture0280 && !test_coop_room_route_driver0280 && !test_coop_room_death0280 && test_inputs_p2.empty() && custom_token.empty() && payload.empty() && boss_solo_token.empty() && test_first_boss.empty() && test_stage<0 && test_zone<0 && !test_spawn && !test_warp && boss_test_case==5 && boss_battle_seed==37784098137700ull && (boss_test_goal=="fish-cp6-focused-replay" || boss_test_goal=="fish-cp6-rebirth") && boss_solo_driver && !boss_manual_handoff && !background_test && !boss_test_menu && !test_inputs.empty() && end_test_vi>=12000 && end_test_vi<=36000 && !test_coop_native && !test_coop_manual && !boss_test_fish_cp6 && !boss_test_route_checkpoints && !boss_test_swallow_checkpoint && !boss_test_full_loadout && !boss_test_special_fish && !boss_test_automatic && !boss_test_lips_loadout && !boss_test_genie_loadout && !boss_test_genie_empty_reserves && !boss_test_monsters_loadout && !boss_test_checkpoint && !boss_test_retry_replay && !boss_test_fish_loadout && !boss_test_final_loadout && boss_followup_inputs.empty()),"Focused Fish CP6 requires isolated visible case5 and declared HOST completed6/cold Rest83/initial native8HP6eggs fixtures");
        workshop::require((boss_test_goal!="fish-cp6-focused-replay" && boss_test_goal!="fish-cp6-rebirth") || boss_test_fish_cp6_focused,"Focused Fish CP6 goal requires its distinct explicit flag");
        if(boss_test_fish_cp6_focused){std::ifstream marker("fish-cp6-focused-qa0284.txt");std::string label;std::getline(marker,label);workshop::require(label=="DinoTale focused Fish CP6 rest fixture 0.28.4","Focused Fish CP6 fixture marker required");}
        if(boss_test_goal=="fish-cp6-rebirth"){std::ifstream marker("fish-cp6-rebirth-qa0284.txt");std::string label;std::getline(marker,label);workshop::require(label=="DinoTale Fish CP6 live rebirth 0.28.4","Live rebirth QA marker required");}
        workshop::boss_fish_cp6_bridge0284::configure(boss_test_fish_cp6 || boss_test_fish_cp6_focused,boss_test_fish_cp6_focused?workshop::boss_fish_cp6_test0284::StartingScope::FixtureRest6:workshop::boss_fish_cp6_test0284::StartingScope::Full,boss_test_goal=="fish-cp6-rebirth"?workshop::boss_fish_cp6_test0284::Goal::Rebirth:workshop::boss_fish_cp6_test0284::Goal::ConsumedReplay);
        workshop::boss_swallow_retry_test0263::configure(boss_test_swallow_checkpoint);
        boss_loadout.reset();boss_fish_loadout_witness.reset();workshop::boss_fish_probe0260::reset();
        boss_test_genie_refill_logged=false;
        workshop::require(!boss_test_checkpoint || (!boss_battle_token.empty() && boss_test_case==0 && boss_test_goal=="checkpoint" && !boss_solo_driver && !background_test && !test_inputs.empty() && end_test_vi>=3600),"Checkpoint QA requires a bounded visible first encounter without the automatic pilot");
        workshop::require(boss_test_goal!="checkpoint" || boss_test_checkpoint,"Checkpoint goal requires its explicit isolated test");
        workshop::require(boss_followup_inputs.empty() || (!boss_battle_token.empty()&&boss_test_case==0&&boss_test_goal=="second-victory"&&boss_solo_driver&&!boss_manual_handoff&&!boss_test_checkpoint&&!background_test&&!test_inputs.empty()&&end_test_vi>=8000&&end_test_vi<=24000),"Followup inputs require a bounded visible two-victory driver test");
        workshop::require(boss_test_goal!="second-victory" || !boss_followup_inputs.empty(),"Second-victory goal requires isolated followup inputs");
        workshop::require(boss_test_goal!="ground-roundtrip" || (!boss_battle_token.empty()&&boss_test_case==0&&boss_solo_driver&&!boss_manual_handoff&&!boss_test_checkpoint&&!background_test&&!test_inputs.empty()&&end_test_vi>=6000&&end_test_vi<=12000),"Ground roundtrip requires a bounded visible first-victory input driver test");
        workshop::require(!boss_test_retry_replay || (!boss_battle_token.empty()&&boss_test_case==0&&boss_test_goal=="retry-victory"&&!boss_solo_driver&&!boss_manual_handoff&&!boss_test_checkpoint&&!background_test&&!test_inputs.empty()&&boss_followup_inputs.empty()&&end_test_vi>=6000&&end_test_vi<=12000),"Retry replay requires a bounded visible scripted first-encounter test without a driver");
        workshop::require(boss_test_goal!="retry-victory" || boss_test_retry_replay,"Retry-victory requires its isolated replay flag");
        workshop::require(!boss_solo_driver || ((!boss_solo_token.empty() || !boss_battle_token.empty()) && !test_inputs.empty() && end_test_vi>=3600),"Boss driver requires a bounded solo diagnostic");
        if(!boss_solo_token.empty()){
            std::ifstream marker("solo-boss-qa.txt");std::string label;std::getline(marker,label);
            workshop::require(label=="DinoTale solo boss 0.24.0" && std::filesystem::is_regular_file("portable.txt"),"Solo boss milestone requires its separate portable test profile");
            workshop::require(workshop::boss_solo0240::configure(boss_solo_token,!test_inputs.empty(),boss_solo_driver),"Invalid solo boss token");
        }
        workshop::require((!boss_test_menu && boss_test_case<0 && boss_test_goal=="route") || ((!boss_battle_token.empty() || boss_test_menu) && !test_inputs.empty() && end_test_vi>=3600),"Boss diagnostic cases/goals require a bounded isolated controller test");
        workshop::require(boss_battle_token.empty() || (boss_solo_token.empty() && test_first_boss.empty() && !test_coop_native && payload.empty() && test_stage<0 && test_zone<0 && !test_spawn && !test_warp),"Boss Battle needs an isolated native campaign");
        if(!boss_battle_token.empty()){
            std::ifstream marker("boss-battle-qa.txt");std::string label;std::getline(marker,label);
            workshop::require(label=="DinoTale boss battle 0.25.0" && std::filesystem::is_regular_file("portable.txt"),"Boss Battle preview requires its separate portable profile");
            workshop::require(workshop::boss_campaign0250::configure(boss_battle_token,boss_battle_seed,true,boss_solo_driver,boss_test_case,boss_test_special_fish,boss_test_automatic,boss_test_blargg0289),"Invalid Boss Battle token");
            if(boss_test_fish_cp6_focused)workshop::require(workshop::boss_fish_cp6_bridge0284::start_focused_fixture(),"Focused HOST cold-Rest6 fixture rejected");
            workshop::boss_campaign0250::checkpoints::allow_retries(boss_manual_handoff || boss_test_checkpoint || boss_test_retry_replay || boss_test_route_checkpoints || boss_test_swallow_checkpoint || boss_test_fish_cp6 || boss_test_fish_cp6_focused || (!boss_solo_driver && !end_test_vi));
        }
        workshop::boss_challenges0260::load(std::filesystem::current_path());
        workshop::boss_native::configure(!test_first_boss.empty() || !boss_solo_token.empty(),boss_solo_token.empty()?test_first_boss:boss_solo_token,boss_solo_token.empty() || !test_inputs.empty());
#endif
#endif
#ifdef WORKSHOP_SPECIAL_CONTENT
        if(bonus_preview0352||bonus_route0353){
            workshop::require(!(bonus_preview0352&&bonus_route0353),"Conflicting Bonus diagnostic paths");
            workshop::require(workshop_menu_enabled()&&end_test_vi&&!test_inputs.empty()&&payload.empty()&&!workshop::custom_session::active()&&!hard_preview0330&&!test_tally&&!test_spawn&&!test_warp&&test_stage<0&&test_zone<0,"Bonus bridge requires a bounded independent input test");
#ifdef WORKSHOP_NATIVE_COOP
            workshop::require(!test_coop_native&&!workshop::boss_solo0240::enabled&&!workshop::boss_campaign0250::enabled,"Bonus bridge cannot use deferred modes");
#endif
        }
        if(bonus_complete0353||bonus_selection0353){
            workshop::require(!bonus_preview0352&&!bonus_route0353&&!(bonus_complete0353&&bonus_selection0353)&&workshop_menu_enabled()&&payload.empty()&&!workshop::custom_session::active()&&!hard_preview0330&&!test_tally&&!test_spawn&&!test_warp&&test_stage<0&&test_zone<0,"Complete Bonus requires its independent native menu session");
#ifdef WORKSHOP_NATIVE_COOP
            workshop::require(!test_coop_native&&!workshop::boss_solo0240::enabled&&!workshop::boss_campaign0250::enabled,"Complete Bonus cannot enter deferred modes");
#endif
        }
        workshop::bonus_game0352::configure_private_preview(std::filesystem::current_path(),bonus_preview0352);
        workshop::bonus_game0352::configure_private_route(std::filesystem::current_path(),bonus_route0353,bonus_page0353,bonus_seed0353);
        workshop::bonus_game0352::configure_private_complete(std::filesystem::current_path(),bonus_complete0353,bonus_page0353,bonus_seed0353);
        if(bonus_selection0353)workshop::bonus_game0352::configure_bonus_selection(std::filesystem::current_path(),bonus_page0353,bonus_seed0353,bonus_new0353);
        bool hard_verified=false;
        if(hard_preview0330){
            const auto directory=std::filesystem::current_path();const auto marker=directory/"story-hard-qa.txt";
            const auto status=std::filesystem::symlink_status(marker);
            workshop::require(workshop_menu_enabled()&&directory.filename().u8string().starts_with(u8"story-hard0330-qa-")&&std::filesystem::is_regular_file(status)&&!std::filesystem::is_symlink(status)&&std::filesystem::file_size(marker)<=128,"Hard preview requires its independent QA profile");
            std::ifstream input(marker,std::ios::binary);const std::string text{std::istreambuf_iterator<char>(input),{}};
            workshop::require(text=="DinoTale Story Hard0330 preview only\n","Invalid Hard preview marker");
            workshop::require(payload.empty()&&!workshop::custom_session::active(),"Hard preview cannot use custom payloads");
#ifdef WORKSHOP_NATIVE_COOP
            workshop::require(!test_coop_native&&!workshop::boss_solo0240::enabled&&!workshop::boss_campaign0250::enabled,"Hard preview cannot use deferred modes");
#endif
            hard_verified=true;
        }
        workshop::require(hard_test_egg0330<0||(hard_verified&&end_test_vi&&!test_inputs.empty()),"Hard egg diagnostic requires a bounded private preview");
        workshop::story_hard_bridge0330::Config hard_config;
        hard_config.preview=hard_preview0330;hard_config.preview_verified=hard_verified;hard_config.test_egg_color=hard_test_egg0330;
        bool hard_ordinary=payload.empty()&&!workshop::custom_session::active();
#ifdef WORKSHOP_NATIVE_COOP
        hard_ordinary=hard_ordinary&&!test_coop_native&&!workshop::boss_solo0240::enabled&&!workshop::boss_campaign0250::enabled;
#endif
        hard_config.genuine_events=hard_ordinary&&!test_tally&&!test_find_reveal&&!test_editor_eggs&&test_special_victory<0&&test_stage<0&&test_zone<0&&!test_spawn&&!test_warp&&!test_native_history;
        workshop::story_hard_bridge0330::configure(hard_config);
#endif
        return true;
    } catch (const std::exception& e) {
        fprintf(stderr,"[Workshop] Invalid project: %s\n",e.what());fflush(stderr);return false;
    }
}

extern "C" void recomp_entrypoint(uint8_t*,recomp_context*);
void workshop_game_entrypoint0320(uint8_t* rdram,recomp_context* context){
#ifdef WORKSHOP_CONTENT_BUILD
#ifdef WORKSHOP_SPECIAL_CONTENT
    if(workshop::bonus_game0352::active())ultramodern::change_save_file({},workshop::bonus_game0352::save_stem());
    if(story_hybrid_session0320()){
        canonical_rights0320=workshop::story_rights0320::read(recomp::get_config_path()/"saves"/"yoshisstory.n64.us.bin");
        workshop::require(canonical_rights0320.trial_complete(),"Story Hybrid requires all24 genuine Trial clears");
        workshop::require(workshop::collections::view().readable&&workshop::collections::view().state.owns(workshop::collections::Item::Hybrid),"Story Hybrid purchase missing");
        // Shared ownership is migrated from the ordinary EEPROM only; never
        // promote the independent Story Hybrid access/continuation as authority.
        workshop::finds::migrate(rdram,canonical_rights0320.progression,canonical_rights0320.story,canonical_rights0320.present&&canonical_rights0320.readable);
        const auto finds=workshop::finds::view();const auto metals=workshop::metal_yoshis0300::view();
        workshop::require(finds.initialized&&finds.readable&&finds.migrated&&finds.saved&&metals.initialized&&metals.readable&&metals.saved,"Shared Yoshi ownership unavailable");
        workshop::require(workshop::story_hybrid0320::prepare(5,workshop::finds::view().found,workshop::metal_yoshis0300::view().found),"Story Hybrid journal unavailable");
        ultramodern::change_save_file({},workshop::game_modes0320::hybrid_save);
        const bool genuine_scope=metal_solo_scope()&&!test_find_reveal&&!test_find_finish_fruit&&!test_editor_eggs&&test_special_victory<0&&test_stage<0&&!test_spawn&&!test_warp;
        story_observer0320.start(5,genuine_scope,uint64_t(std::chrono::steady_clock::now().time_since_epoch().count())|1ull);
        workshop::collections::start_session(5,canonical_rights0320.trial,canonical_rights0320.bw_earned,canonical_rights0320.readable);
        fprintf(stderr,"[StoryHybrid0320] independentEEP=1 canonicalTrial=%06X worlds=%02X owned=%02X nativeMode=0\n",canonical_rights0320.trial,workshop::story_hybrid0320::view().worlds,workshop::finds::view().found);
        fflush(stderr);
    }
    if(story_hard_session0330()){
        workshop::require(workshop::story_hard_bridge0330::prepare_entry(),"Story Hard unavailable: see independent journal");
        ultramodern::change_save_file({},workshop::story_hard_bridge0330::save_name());
        fprintf(stderr,"[StoryHard0330] independentEEP=1 preview=%u nativeMode=0\n",unsigned(workshop::story_hard_bridge0330::preview()));fflush(stderr);
    }
#endif
#endif
    recomp_entrypoint(rdram,context);
}
void workshop_on_init(uint8_t* rdram, recomp_context*) {
#ifdef WORKSHOP_CONTENT_BUILD
#ifndef WORKSHOP_DISABLE_HYBRID_FINDS
    workshop::finds::initialize();
    workshop::metal_yoshis0300::initialize();
    workshop::elemental_yoshis0355::initialize();
    workshop::known_yoshis_runtime0355::canonical(workshop::story_rights0320::read(recomp::get_config_path()/"saves"/"yoshisstory.n64.us.bin"));
    elemental_contacts0355.reset();
    elemental_tongue_draws0355.clear();
    {
        std::lock_guard guard(trial_pending_mutex0350);
        trial_special_profile0350=std::filesystem::current_path();
        // Preserve failed loss receipts across a same-process restart. Distinct
        // profile keys prevent a custom/preview profile from inheriting them.
        trial_pending_by_profile0350[trial_special_profile0350].last_retry_vi=0;
    }
    trial_special_store0350.load(std::filesystem::current_path());
#endif
    if(!workshop_menu_enabled() && end_test_vi){workshop::collections::initialize();workshop::collections::start_session(test_hybrid?3:1);workshop::hybrid_mode::initialize();workshop::hybrid_mode::activate(test_hybrid);workshop::hybrid_completion0290::initialize("pc-hybrid-completion.json","pc-hybrid-records.json");workshop::hybrid_completion0290::start_session(test_hybrid?3:1);}
    workshop::metal_yoshis0300::set_scope(metal_solo_scope());
    workshop::elemental_yoshis0355::start_session(workshop_menu_enabled()?unsigned(workshop_menu_session_mode()):end_test_vi?(test_hybrid?3:1):4,elemental_solo_scope0355());
    workshop::metal_native0300::story_ready=false;
    story_lives_ready0320=false;story_observer0320.reset();story_pending_worlds0320=0;
    if(workshop_menu_enabled())workshop::metal_yoshis0300::start_session(unsigned(workshop_menu_session_mode()),metal_solo_scope());
    else if(end_test_vi)workshop::metal_yoshis0300::start_session(test_hybrid?3:1,metal_solo_scope());
    if(!workshop::story_hybrid0320::view().initialized)workshop::story_hybrid0320::initialize();
    diagnostic_rdram=rdram;test_vi=0;
#ifdef WORKSHOP_NATIVE_COOP
    workshop::boss_campaign0250::completion_hook=record_native_battle;
    workshop::boss_lips_terrain_alias0260::runtime::reset(rdram);
    boss_fish_loadout_witness.reset();
    boss_blargg_boot_router.disable();
    boss_blargg_acquire.disable();
    boss_blargg_shoot.disable();
    boss_blargg_flame_probe_count=boss_blargg_flame_probe_vi=0;
    boss_blargg_flame_probe_actor=boss_blargg_flame_probe_callback=boss_blargg_flame_probe_step=0;
    boss_blargg_request_probe_count=0;boss_blargg_request_probe_dumped=false;
#endif
    char* base=SDL_GetBasePath();workshop::require(base!=nullptr,"Cannot locate content engine folder");
    auto path=std::filesystem::u8path(base)/"workshop-content.bin";SDL_free(base);
    std::ifstream input(path,std::ios::binary|std::ios::ate);
    workshop::require(bool(input) && input.tellg()==workshop::rom_size,"Missing built-in Yoshi content data");
    std::vector<uint8_t> rom(workshop::rom_size);input.seekg(0);input.read(reinterpret_cast<char*>(rom.data()),rom.size());
    uint64_t hash=14695981039346656037ULL;
    for(uint8_t value:rom){hash^=value;hash*=1099511628211ULL;}
    workshop::require(hash==workshop_content_hash,"Content data differs from the recompiled game");
#ifdef WORKSHOP_SPECIAL_CONTENT
    if(workshop::bonus_game0352::active()){
        const auto original_path=recomp::get_config_path()/"yoshisstory.n64.us.z64";
        std::ifstream original_file(original_path,std::ios::binary|std::ios::ate);
        workshop::require(bool(original_file)&&original_file.tellg()==workshop::rom_size,"Bonus original USA ROM unavailable");
        std::vector<uint8_t> original(workshop::rom_size);original_file.seekg(0);original_file.read(reinterpret_cast<char*>(original.data()),original.size());
        const auto rights=workshop::story_rights0320::read(recomp::get_config_path()/"saves"/"yoshisstory.n64.us.bin");
        const auto finds=workshop::finds::view();const auto metals=workshop::metal_yoshis0300::view();
        const workshop::bonus_team0353::Rights ownership{rights.present&&rights.readable,
            finds.initialized&&finds.readable&&finds.saved,metals.initialized&&metals.readable&&metals.saved,
            rights.bw_earned,finds.migrated?finds.found:((rights.present&&rights.readable)?rights.legacy_owned:0u),metals.found,workshop::elemental_yoshis0355::view().initialized&&workshop::elemental_yoshis0355::view().readable&&workshop::elemental_yoshis0355::view().saved,workshop::elemental_yoshis0355::view().found};
        rom=workshop::unknown_yoshi_resources0355::apply(workshop::elemental_resources0355::apply(workshop::gold_silver_resources0300::apply(std::move(rom))));
        workshop::require(bool(original_file)&&workshop::bonus_game0352::prepare_native(original,rdram,workshop::bonus_team0353::owned(ownership),&rom),"Bonus native preparation failed");
    }else rom=workshop::unknown_yoshi_resources0355::apply(workshop::elemental_resources0355::apply(workshop::gold_silver_resources0300::apply(std::move(rom))));
    if(story_hard_session0330())rom=workshop::story_hard_bridge0330::patch_rom(std::move(rom));
    workshop::special_bubbles::configure(rom,payload);
    workshop::editor_objects::configure(rom,payload);workshop::black_flyguy::configure(payload);workshop::passages::configure(rom,payload);
#endif
    if(!payload.empty())rom=workshop::apply(rom,payload,0xFF0000,
#ifdef WORKSHOP_SPECIAL_CONTENT
        true
#else
        false
#endif
    );
#ifdef WORKSHOP_NATIVE_COOP
    if(workshop::boss_solo0240::enabled){
        rom=workshop::boss_rest_data0240::apply(std::move(rom));
        std::copy_n(rom.begin()+0xA84A0+162*8,8,rom.begin()+0xA84A0+52*8);
        fprintf(stderr,"[BossSolo] first-arena=162 root=52 rest=83 tree=4708 solo=1 tenBossComplete=0\n");
    }
#endif
    if(workshop::boss_campaign0250::enabled){
        workshop::boss_driver_bridge0260::configure(boss_test_automatic && boss_test_full_loadout);
        workshop::boss_route_checkpoint_bridge0260::configure(boss_test_route_checkpoints);
        workshop::boss_checkpoint_probe0260::reset();
        if(workshop::boss_campaign0250::session.plan().config.blargg)workshop::boss_blargg_arena0289::prepare(rom);
        rom=workshop::boss_rest_data0240::apply(std::move(rom));
        rom=workshop::boss_rooms_data0250::apply(std::move(rom),workshop::boss_campaign0250::session.plan());
        if(workshop::boss_campaign0250::session.plan().config.blargg)workshop::boss_blargg_arena0289::install(rom);
        const auto initial_source=boss_test_fish_cp6_focused?workshop::boss_fish_cp6_fixture0284::initial_source(workshop::boss_campaign0250::source_for(workshop::boss_campaign0250::session.snapshot().index)):workshop::boss_campaign0250::source_for(workshop::boss_campaign0250::session.snapshot().index);
        std::copy_n(rom.begin()+0xA84A0+initial_source*8,8,rom.begin()+0xA84A0+52*8);
        if(boss_test_blargg0289){
            const auto& plan=workshop::boss_campaign0250::session.plan();
            const auto index=workshop::boss_campaign0250::session.snapshot().index;
            const std::array<uint32_t,2> descriptor{workshop::u32(rom,0xA84A0+52*8),workshop::u32(rom,0xA84A0+52*8+4)};
            workshop::require(boss_blargg_boot_router.arm(boss_test_blargg0289,plan,index,initial_source,descriptor),"Blargg cold-start QA requires the first real Blargg encounter and its exact ROM descriptor");
            if(boss_test_blargg_acquire0289)workshop::require(boss_blargg_acquire.configure(true,workshop::boss_campaign0250::session.snapshot().token,plan.seed,index,initial_source,end_test_vi),"Invalid isolated Blargg acquisition QA configuration");
            if(boss_test_blargg_shoot0289)workshop::require(boss_blargg_shoot.configure(true,workshop::boss_campaign0250::session.snapshot().token,plan.seed,index,initial_source,end_test_vi),"Invalid isolated Blargg shooting QA configuration");
        }
        fprintf(stderr,"[BossBattle] native-preview version=0.25.0 fights=10 rest=83 seed=%llu diagnostic=%u\n",(unsigned long long)boss_battle_seed,unsigned(workshop::boss_campaign0250::session.snapshot().diagnostic));
    }
    if(test_zone>=0){
        const size_t target=0xA84A0+size_t(workshop::vanilla_sources[test_stage>=0?test_stage:0])*8,source=0xA84A0+size_t(test_zone)*8;
        const bool hard_actual_room=hard_preview0330&&story_hard_session0330();
        if(!hard_actual_room)std::copy_n(rom.begin()+source,8,rom.begin()+target);
        fprintf(stderr,"[Workshop] Diagnostic 1-1 scene uses zone %d.\n",test_zone);
        if(test_spawn){
            auto bank=[&](uint32_t p){return size_t(workshop::u32(rom,0xA79D4+((p>>24)&15)*4))+(p&0xFFFFFF);};
            const auto header=bank(workshop::u32(rom,(hard_actual_room?source:target)+4));
            const auto meta=bank(workshop::u32(rom,bank(workshop::u32(rom,header+0x5C))));
            const auto objects=bank(workshop::u32(rom,meta+4));
            for(unsigned i=0;i<workshop::u16(rom,meta);i++)if(workshop::u16(rom,objects+i*12)==0x4001){
                workshop::put32(rom,objects+i*12+4,std::bit_cast<uint32_t>(test_spawn_x));workshop::put32(rom,objects+i*12+8,std::bit_cast<uint32_t>(test_spawn_y));
            }
            if(hard_actual_room)test_spawn=false; // Constructor owns native physics coordinates.
        }
    }
#ifdef WORKSHOP_SPECIAL_CONTENT
    rom=workshop::special_victory::extend(std::move(rom));
#endif
    recomp::set_rom_contents(std::move(rom));
    recomp::do_rom_read(rdram,gpr(int32_t(0x80000400)),0x10001000,0x100000);
#ifdef WORKSHOP_SPECIAL_CONTENT
    recomp::do_rom_read(rdram,gpr(int32_t(0x807FC000)),0x10FB0000,120);
    recomp::do_rom_read(rdram,gpr(int32_t(workshop::gold_silver_resources0300::metadata_ram)),0x10000000u+uint32_t(workshop::gold_silver_resources0300::metadata_rom),uint32_t(workshop::gold_silver_resources0300::metadata_size));
    recomp::do_rom_read(rdram,gpr(int32_t(workshop::elemental_resources0355::metadata_ram)),0x10000000u+uint32_t(workshop::elemental_resources0355::metadata_rom),uint32_t(workshop::elemental_resources0355::metadata_size));
#ifdef WORKSHOP_NATIVE_COOP
    if(workshop::boss_campaign0250::enabled){
        namespace aliases=workshop::boss_lips_terrain_alias0260;
        recomp::do_rom_read(rdram,gpr(int32_t(aliases::ram_table)),0x10000000u+aliases::allocation,aliases::table_size);
        workshop::require(aliases::runtime::publish(rdram),"Lips terrain alias table readback failed");
    }
#endif
    workshop::hybrid_mode::custom_placements(payload);
    workshop::hybrid_mode::initialize_ram(rdram);
#endif
    load_overlays(0xF10000,int32_t(0x807F0000),0x2000);
    load_overlays(0xF60000,int32_t(0x807F2000),0x6000);
    recomp::overlays::add_loaded_function(int32_t(0x80000438),workshop_80000438);
    recomp::overlays::add_loaded_function(int32_t(0x807F1600),workshop_807F1600);
    fprintf(stderr,"[Workshop] Built-in content: 16 solo Yoshis and v9.2 rules; custom placements %u.\n",payload.empty()?0:workshop::payload_count(payload));
    fflush(stderr);
#else
    if (payload.empty()) return;
    auto rom=workshop::apply(recomp::get_rom(),payload);
#ifdef WORKSHOP_SPECIAL_CONTENT
    rom=workshop::special_victory::extend(std::move(rom));
#endif
    recomp::set_rom_contents(std::move(rom));
    // init() has already copied the main 1 MiB into RDRAM. Refresh the one
    // descriptor table entry as well as the in-memory ROM used by future DMA.
    for(const auto& room:workshop::payload_rooms(payload)){const uint32_t entry=0xA84A0+room.source*8;recomp::do_rom_read(rdram,gpr(int32_t(0x80000400+entry-0x1000)),0x10000000+entry,8);}
    fprintf(stderr,"[Workshop] Loaded custom source %u: %u placements; original ROM verified; native code unchanged.\n",workshop::u32(payload,8),workshop::payload_count(payload));
    fflush(stderr);
#endif
}
