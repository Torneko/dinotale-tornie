#include "workshop_loader.hpp"
#include "custom_session.hpp"
extern "C" void workshop_request_native_history_test(unsigned);
#include "workshop_menu.hpp"
#include "workshop_menu_game.hpp"
#include "achievements.hpp"
#include "collections.hpp"
#include "hybrid_mode.hpp"
#include "hybrid_finds.hpp"
#include "hybrid_pickup_sound.hpp"
#include "workshop_data.hpp"
#include "librecomp/game.hpp"
#include "librecomp/overlays.hpp"
#include <cstdio>
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
std::atomic<unsigned> test_vi{0};
uint8_t* diagnostic_rdram=nullptr;
int test_color=-1;
bool background_test=false,trace_startup=false;
bool test_book_resume=false;
bool test_book_shop=false;
bool test_find_reveal=false;
uint32_t test_find_source=0,test_find_egg=0;
unsigned test_find_spawn_vi=0;
bool test_find_finish_fruit=false;
bool test_find_finish_committed=false;
uint32_t test_find_finish_actor=0;
bool fruit_audit=false;
bool special_hud=true;
bool test_tally=false;
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
uint32_t progress_payload=0,progress_boot_story=0;
bool progress_reading=false;
unsigned auto_cursor=0,auto_next=0,auto_end=0;
float auto_x=0,auto_y=0;
uint16_t auto_buttons=0;
bool ram_pointer(uint32_t a){return a>=0x80000400 && a<0x80800000;}
void select_test_color(unsigned frame,uint16_t* buttons,float* x,float* y){
    if(test_stage>=0 && diagnostic_rdram && frame>=1700 && frame<2400){
        auto* rdram=diagnostic_rdram;
        MEM_B(0,gpr(int32_t(0x800F8DFC)))=test_stage/4;
        MEM_B(0,gpr(int32_t(0x800F8DFD)))=test_stage%4;
    }
    if(diagnostic_rdram && workshop::custom_session::active()){
        const auto snapshot=workshop::book::snapshot();
        if(snapshot.stage==0 && (snapshot.scene==41 || snapshot.scene==5)){
            auto* rdram=diagnostic_rdram;const int stage=workshop::custom_session::source_stage();
            MEM_B(0,gpr(int32_t(0x800F8DFC)))=stage/4;MEM_B(0,gpr(int32_t(0x800F8DFD)))=stage%4;
        }
    }
    const int selected_color=test_color>=0?test_color:workshop::custom_session::preferred_color();
    if(selected_color<0 || !diagnostic_rdram || frame<1800)return;
    auto* rdram=diagnostic_rdram;
    auto read=[&](uint32_t a){return uint32_t(MEM_W(0,int32_t(a)));};
    auto is_yoshi=[&](uint32_t a){auto id=read(a+12)>>16;return (id>=0x41F1 && id<=0x41F8)||(id>=0x4EF1 && id<=0x4EF6);};
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
        std::array<float,16> px{},py{};int size=0,source=-1,target=-1;
        for(unsigned i=0;i<count && size<16;i++){
            auto object=read(background+0x188+i*4);if(!ram_pointer(object)||!is_yoshi(object))continue;
            px[size]=std::bit_cast<float>(read(object+60));py[size]=std::bit_cast<float>(read(object+64));
            if(object==current)source=size;if(int(read(object+12)&255)==selected_color)target=size;size++;
        }
        if(source<0 || target<0)return;
        std::array<int,16> previous{},first{},queue{};previous.fill(-1);previous[source]=source;first[source]=-1;
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
void configure_diagnostics(){
    const auto stamp=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto directory=std::filesystem::current_path()/"logs";std::filesystem::create_directories(directory);
    std::string prefix=(directory/("engine-"+std::to_string(stamp))).string();
    std::freopen((prefix+".stderr.log").c_str(),"w",stderr);setvbuf(stderr,nullptr,_IONBF,0);
    std::freopen((prefix+".stdout.log").c_str(),"w",stdout);setvbuf(stdout,nullptr,_IONBF,0);
    fprintf(stderr,"[Workshop] Profile %s\n",std::filesystem::current_path().string().c_str());
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
        fprintf(stderr,"[Workshop] Automatic test reached VI %u; terminating isolated test.\n",frame);
        if(diagnostic_rdram){std::ofstream final("controller-test-final.rdram",std::ios::binary);final.write(reinterpret_cast<char*>(diagnostic_rdram),0x800000);}
        workshop_dump_trace();std::fflush(nullptr);std::_Exit(0);
    }
}
bool workshop_background_test(){return background_test;}
bool workshop_test_book_resume(){return test_book_resume && background_test && end_test_vi;}
bool workshop_test_book_shop(){return test_book_shop && background_test && end_test_vi;}
bool workshop_get_input(int player,uint16_t* buttons,float* x,float* y){
    if(test_inputs.empty()){
        const bool connected=recompinput::profiles::get_n64_input(player,buttons,x,y);
        if(!connected){*buttons=0;*x=0;*y=0;}
        if(player==0){
            const auto frame=test_vi.load();
            workshop::custom_session::navigation(frame,buttons,x,y);
            if(workshop::custom_session::active())select_test_color(frame,buttons,x,y);
            workshop::book::filter_input(buttons,x,y);
        }
        return connected;
    }
    *buttons=0;*x=0;*y=0;if(player!=0)return false;
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
extern "C" recomp_func_t* workshop_lookup(int32_t address,uint8_t*,recomp_context* ctx,const char* caller){
    // Audio DMA runs continuously, even when the game is waiting for assets.
    // Exclude that high-frequency callback so the ring keeps gameplay calls.
    if(uint32_t(address)!=0x8008AB60u){std::lock_guard lock(lookup_mutex);lookup_records[lookup_count++%lookup_records.size()]={uint32_t(address),uint32_t(ctx->r31),uint32_t(ctx->r29),uint32_t(ctx->r4),uint32_t(ctx->r5),caller};}
    if(uint32_t(address)<0x80000000u || uint32_t(address)>=0x80800000u){
        fprintf(stderr,"[Workshop] Invalid target %08X from %s, ra=%08X sp=%08X a0=%08X a1=%08X\n",uint32_t(address),caller,uint32_t(ctx->r31),uint32_t(ctx->r29),uint32_t(ctx->r4),uint32_t(ctx->r5));fflush(stderr);
    }
    return get_function(address);
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
static uint32_t story_clears(uint8_t* rdram){
    if(progress_payload)return uint32_t(MEM_W(0,static_cast<gpr>(static_cast<int32_t>(progress_payload))))&0xFFFFFF;
    if(progress_reading)return progress_boot_story;
    return uint32_t(MEM_W(0,int32_t(0x800F8E5C)))&0xFFFFFF;
}
extern "C" int workshop_progress_valid(uint8_t* rdram,recomp_context* ctx){
    if(!ram_pointer(uint32_t(ctx->r4)))return 0;
    return workshop::progress::valid(workshop::progress::read(rdram,ctx->r4),story_clears(rdram));
}
extern "C" int workshop_progress_trial_access(uint8_t* rdram){return story_clears(rdram)==0xFFFFFF;}
extern "C" int workshop_progress_trial(uint8_t* rdram,recomp_context* ctx){
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
extern "C" void workshop_progress_read_end(uint8_t* rdram,recomp_context*){progress_boot_story=story_clears(rdram);workshop::finds::migrate(rdram,progress_boot_story);workshop::hybrid_mode::records_loaded(rdram,progress_payload);progress_payload=0;}
extern "C" int workshop_hybrid_records_save(uint8_t* rdram,recomp_context* ctx){return workshop::hybrid_mode::records_saving(rdram,uint32_t(MEM_W(0x20,ctx->r29)),0x800F97F8);}
extern "C" int workshop_hybrid_active(){return workshop::hybrid_mode::active();}
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
extern "C" void workshop_progress_reconcile(uint8_t* rdram,recomp_context*){reconcile_story(rdram);}
extern "C" unsigned workshop_find_unlocked(uint8_t* rdram){
    const auto p=workshop::progress::read(rdram,gpr(int32_t(0x807F2000)));
    const auto story=story_clears(rdram);
    if(!workshop::progress::valid(p,story))return 0;
    workshop::finds::migrate(rdram,story);
    return workshop::finds::available(workshop::progress::unlocked(p,story));
}
extern "C" void workshop_find_reset(){workshop::finds::reset_attempt();}
extern "C" void workshop_find_clear(){workshop::finds::clear_save();}
extern "C" void workshop_progress_boot(uint8_t* rdram,recomp_context* ctx){
    progress_payload=0;progress_reading=false;if(ctx->r2!=0)return;
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
            fprintf(stderr,"[Workshop] Story rewards recovered; native save result=%u, backup=%s.\n",uint32_t(ctx->r2),backup.string().c_str());*ctx=saved;
        }
    }catch(const std::exception& error){fprintf(stderr,"[Workshop] Story recovery deferred: %s.\n",error.what());}
}
extern "C" void func_80051368(uint8_t* rdram,recomp_context* ctx);
extern "C" void func_80058A14(uint8_t*,recomp_context*);
static workshop::fruit_mix::Random& hybrid_random(){
    static const uint32_t seed=fruit_seed?fruit_seed:std::random_device{}();
    static workshop::fruit_mix::Random random(seed);
    static bool noted=false;if(fruit_audit && !noted){fprintf(stderr,"[FruitAudit] seed=%u manualInput=1\n",seed);noted=true;}
    return random;
}
extern "C" void workshop_mix_level(uint8_t* rdram,recomp_context* ctx){
    bool custom_metadata=false;
    if(!payload.empty() && workshop::fruit_mix::range(uint32_t(ctx->r19),8)){
        const auto meta=ctx->r19;const unsigned count=MEM_HU(0,meta);const uint32_t objects=MEM_W(4,meta);
        for(const auto& room:workshop::payload_rooms(payload))if(room.source==(hybrid_room&255) && count==room.count && workshop::fruit_mix::range(objects,count*16)){
            bool matched=true;
            for(unsigned i=0;i<count && matched;i++){const auto object=gpr(int32_t(objects+i*16));const size_t p=room.offset+size_t(i)*room.stride;matched=uint32_t(MEM_W(0,object))==workshop::u32(payload,p) && uint32_t(MEM_W(4,object))==workshop::u32(payload,p+4) && uint32_t(MEM_W(8,object))==workshop::u32(payload,p+8);}
            if(matched){custom_metadata=true;fprintf(stderr,"[CustomSource] payload-verified source=%u actualRoom=%u placements=%u\n",workshop::u32(payload,8),room.source,count);}
        }
    }
    if(!workshop::fruit_mix::playable(rdram,uint32_t(ctx->r19)) && !custom_metadata)return;
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
    workshop::hybrid_mode::enter(rdram,uint32_t(ctx->r19),hybrid_room);
    workshop::collections::enter(rdram,hybrid_room&255,uint32_t(ctx->r19),payload.empty());
    if(workshop::fruit_mix::playable(rdram,uint32_t(ctx->r19)))workshop::finds::enter(
        unsigned(MEM_BU(0,int32_t(0x800F8DFC)))*4+unsigned(MEM_BU(0,int32_t(0x800F8DFD))),hybrid_room&255,
        payload.empty() && uint32_t(MEM_W(0,int32_t(0x800F8B88)))<2);
    find_binding.reset();find_sources.clear();
    if(hybrid_level_active && uint32_t(MEM_W(0,int32_t(0x800F8B88)))<2){
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
                if(auto color=workshop::finds::reserve(workshop::progress::unlocked(p,story),kind,x,y,whitePresent)){
                    find_binding=FindSource{uint32_t(record),kind,x,y,*color};MEM_H(0,record)=0x4692;
                    ctx->r4=0x4692;func_80051368(rdram,ctx);*ctx=saved;break;
                }
            }
        }
    }
    if(!hybrid_level_active)return;
    const unsigned threshold=workshop::hybrid_mode::active()?45:15;
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
    if(!hybrid_level_active || !workshop::fruit_mix::eligible(id))return id;
    const uint32_t parent=ram_pointer(uint32_t(ctx->r17))?uint32_t(MEM_W(0,ctx->r17)):0;
    const bool preview=ram_pointer(parent) && workshop::fruit_mix::preview_source(MEM_HU(12,int32_t(parent)));
    const auto result=preview?(hybrid_visits.actors.contains(parent)
        ?hybrid_visits.pick(id,parent,[](){return hybrid_random().ticket();},workshop::hybrid_mode::active()?45:15)
        :hybrid_spawn_choices.pick(id,parent,[](){return hybrid_random().ticket();},workshop::hybrid_mode::active()?45:15))
        :workshop::fruit_mix::replacement(id,hybrid_random().ticket(),workshop::hybrid_mode::active()?45:15);
    if(result!=id)fprintf(stderr,"[Workshop] Hybrid replacement %04X -> %04X.\n",id,result);
    return result;
}
extern "C" int workshop_use_special_hud(){return special_hud;}
extern "C" void workshop_807F1300(uint8_t*,recomp_context*);
extern "C" void workshop_pause_score_lives(uint8_t* rdram,recomp_context* ctx){
    const auto saved=*ctx;
    workshop_807F1300(rdram,ctx);
    const auto alive=uint32_t(ctx->r2)&0x3FFFu;
    *ctx=saved;ctx->r2=std::popcount(alive);
    static unsigned previous=~0u;
    if(fruit_audit && previous!=ctx->r2){previous=unsigned(ctx->r2);fprintf(stderr,"[ScoreAudit] pause vi=%u livingYoshis=%u mask=%04X\n",test_vi.load(),previous,alive);}
}
extern "C" void workshop_end_score_lives(uint8_t* rdram,recomp_context* ctx){
    const auto saved=*ctx;
    workshop_807F1300(rdram,ctx);
    const auto alive=uint32_t(ctx->r2)&0x3FFFu;
    *ctx=saved;
    const auto p=workshop::progress::read(rdram,gpr(int32_t(0x807F2000)));
    const auto finds=workshop::finds::view();
    ctx->r5=workshop::progress::tally_lives(p,story_clears(rdram),MEM_W(0,int32_t(0x800F8B88))!=0,
        finds.initialized && finds.readable && finds.migrated?6:MEM_BU(0,int32_t(0x800F8DFC)),MEM_BU(0,int32_t(0x800F8DFD)),
        alive|(workshop::finds::pending()<<8),uint32_t(MEM_W(0,int32_t(0x800F8DF4))));
    workshop::finds::finish(rdram);
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
    if(background_test){
        {std::ofstream dump("native-fault.rdram",std::ios::binary);dump.write(reinterpret_cast<char*>(rdram),0x900000);}
        std::fflush(nullptr);std::_Exit(2);
    }
}
extern "C" int workshop_resource_slot(uint8_t* rdram,recomp_context* ctx,int match){return workshop::cache::find_slot(rdram,ctx,match!=0);}
extern "C" void workshop_test_level_start(uint8_t* rdram,recomp_context*){
    if(!test_tally)return;
    MEM_W(0,gpr(int32_t(0x800F89D0)))=29;
    for(unsigned i=0;i<30;i++){
        MEM_W(i*4,gpr(int32_t(0x800F89D4)))=0;
        MEM_H(i*2,gpr(int32_t(0x800F8AD4)))=0;
    }
    fprintf(stderr,"[Workshop] Tally fixture: 29 apples seeded at VI %u; only disposable test profile.\n",test_vi.load());
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
static void custom_victory_fixture(uint8_t* rdram,recomp_context* ctx){
    // Explicit, finite diagnostics only: seed29 is supplied by test_level_start;
    // consume the real custom-world fruit through the native callback.
    static bool consumed=false;
    if(!test_tally || !workshop::custom_session::active() || consumed || test_vi.load()<2500 || test_vi.load()%30 || MEM_W(0,int32_t(0x800F89D0))!=29)return;
    for(uint32_t p=0x80000400;p<0x80400000-0x200;p+=4){
        const auto a=gpr(int32_t(p));
        if(uint32_t(MEM_W(0,a))!=0x19971110 || MEM_HU(12,a)!=0x4030 || MEM_BU(0x18,a)!=1)continue;
        consumed=true;const auto saved=*ctx;ctx->r29=ADD32(ctx->r29,-0x30);ctx->r4=a;ctx->r5=0x66;func_80574060(rdram,ctx);*ctx=saved;
        fprintf(stderr,"[CustomSessionTest] native-thirtieth-fruit actor=%08X count=%u ending=%u\n",p,uint32_t(MEM_W(0,int32_t(0x800F89D0))),uint32_t(MEM_W(0,int32_t(0x800F8AC8))));break;
    }
}
static void test_find_frame(uint8_t*,recomp_context*);
extern "C" void workshop_trace_entry(uint8_t* rdram,recomp_context* ctx,const char* name){
    if(test_find_reveal && test_find_egg && uint32_t(ctx->r4)==test_find_egg){
        static unsigned calls=0;
        if(calls++<24)fprintf(stderr,"[HiddenEggTest] call=%s actor=%08X\n",name,test_find_egg);
    }
    if(fruit_audit && (std::string_view(name)=="func_806F616C" || std::string_view(name)=="func_8079AC9C"))
        fprintf(stderr,"[HybridFindAudit] gate %s actor=%08X\n",name,uint32_t(ctx->r4));
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
        if(end_test_vi || workshop::custom_session::active())fprintf(stderr,"[CustomSource] requested=%u actualRoom=%u page=%u column=%u\n",payload.empty()?999:workshop::u32(payload,8),unsigned(ctx->r5)&255,unsigned(MEM_BU(0,int32_t(0x800F8DFC))),unsigned(MEM_BU(0,int32_t(0x800F8DFD))));
    }
    if(std::string_view(name)=="func_80045CAC"){
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
    // Placed 406F coins use the F49C state machine (collected state 3).
    // The separate F070 state machine is used by another native coin form.
    if(std::string_view(name)=="func_8006F49C" && (uint32_t(ctx->r6)&255)==3)
        workshop::collections::collect(rdram,uint32_t(ctx->r4));
    if(std::string_view(name)=="func_8006E7F0" || std::string_view(name)=="func_8005F700")workshop::collections::draw(rdram,uint32_t(ctx->r4));
    if(std::string_view(name)=="func_8006E7F0" || std::string_view(name)=="func_8005F700")workshop::reward_popup::paint(rdram,uint32_t(ctx->r4));
    if(fruit_audit && std::string_view(name)=="func_8005F700" && workshop::reward_popup::fours.contains(uint32_t(ctx->r4))){
        static uint32_t previous=0;
        if(previous!=uint32_t(ctx->r4)){previous=uint32_t(ctx->r4);fprintf(stderr,"[RewardPopup] draw single-four actor=%08X textureFrame=%u resource=%08X\n",previous,unsigned(MEM_HU(0x144,ctx->r4)),uint32_t(MEM_W(0x168,ctx->r4)));}
    }
    if(std::string_view(name)=="func_8005F700")workshop::hybrid_mode::paint(rdram,uint32_t(ctx->r4));
    if(std::string_view(name)=="func_800675F0"){
        workshop::achievements::frame(rdram,ctx,payload.empty() && !test_tally);
        custom_victory_fixture(rdram,ctx);
        workshop::custom_session::finish(rdram);
        workshop::book::frame(rdram,ctx,test_vi.load());
        if(test_find_reveal)test_find_frame(rdram,ctx);
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
extern "C" int workshop_egg_allowed(uint8_t* rdram,unsigned color){return workshop::collections::egg_allowed(rdram,color);}
extern "C" void func_80058A14(uint8_t*,recomp_context*);
extern "C" void func_80047090(uint8_t*,recomp_context*);
extern "C" void func_8004613C(uint8_t*,recomp_context*);
extern "C" void func_8003B328(uint8_t*,recomp_context*);
extern "C" void func_8003B890(uint8_t*,recomp_context*);
extern "C" void func_8003B8B0(uint8_t*,recomp_context*);
static std::optional<unsigned> workshop_find_place(uint8_t* rdram,recomp_context* ctx,uint32_t actor,bool container){
    if(!workshop::fruit_mix::range(actor,container?0x90:0x48))return {};
    const auto a=gpr(int32_t(actor));const auto id=MEM_HU(12,a);
    const auto p=workshop::progress::read(rdram,gpr(int32_t(0x807F2000)));const auto story=story_clears(rdram);
    if(!workshop::progress::valid(p,story))return {};
    const auto saved=*ctx;bool whitePresent=false;
    if(MEM_W(0,int32_t(0x800F8B88))==0){func_80058A14(rdram,ctx);whitePresent=ctx->r2!=0;*ctx=saved;}
    return workshop::finds::reserve(workshop::progress::unlocked(p,story),id,uint32_t(MEM_W(container?0x84:0x3C,a)),uint32_t(MEM_W(container?0x88:0x40,a)),whitePresent);
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
extern "C" int workshop_find_release(uint8_t* rdram,recomp_context* ctx){
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
    const auto state=MEM_BU(0x1F,ctx->r29);
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
    if(workshop_find_source(rdram,ctx))return 1;
    if(auto hidden=workshop::collections::egg_bubble_hidden(rdram,uint32_t(ctx->r4))){ctx->r2=*hidden;return 1;}
    return 0;
}
extern "C" void workshop_egg_enemy_color(uint8_t* rdram,recomp_context* ctx){
    const auto color=uint32_t(MEM_W(0,int32_t(0x807F0008)));
    if(color==6 || color==7 || !workshop::rewards::custom_enemy_variant)return;
    if(auto variant=workshop::rewards::custom_enemy_variant(uint32_t(ctx->r5));variant && *variant<6)
        MEM_W(0x10,ctx->r29)=8+*variant;
}
extern "C" void workshop_egg_default_color(uint8_t* rdram,recomp_context* ctx){
    if(!ctx->r2)return;
    const auto color=uint32_t(MEM_W(0,int32_t(0x807F0008)));
    if(color!=6 && color!=7 && workshop::rewards::custom_enemy_variant){
        const auto source=uint32_t(MEM_W(0x24,ctx->r29));
        if(auto variant=workshop::rewards::custom_enemy_variant(source);variant && *variant<6){ctx->r7=8+*variant;return;}
    }
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
    if(auto value=workshop::rewards::food_gain(rdram,actor,color,false))ctx->r2=gpr(*value);
}
extern "C" void workshop_food_score(uint8_t* rdram,recomp_context* ctx){
    const auto color=uint32_t(MEM_W(0,int32_t(0x807F0008)));
    MEM_W(0x18,ctx->r29)=0;
    if(auto value=workshop::rewards::food_gain(rdram,uint32_t(ctx->r5),color,true)){
        const auto life=workshop::rewards::food_gain(rdram,uint32_t(ctx->r5),color,false);
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
    if(workshop::rewards::no_enemy_score(rdram,uint32_t(ctx->r5))){ctx->r12=0;ctx->r13=1;return;}
    if(auto value=workshop::rewards::custom_gain(uint32_t(ctx->r5),color)){ctx->r12=gpr(*value);ctx->r13=workshop::rewards::popup(*value);return;}
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
                workshop::require(mode>=0 && mode<=3,"Invalid book mode");workshop_menu_start_mode(mode);continue;
            }
#ifdef WORKSHOP_CONTENT_BUILD
            if(std::string_view(argv[n])=="--workshop-background-test"){background_test=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-book-resume"){test_book_resume=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-book-shop"){test_book_shop=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-find-reveal"){test_find_reveal=true;continue;}
            if(std::string_view(argv[n])=="--workshop-release-test-spawn"){test_release_spawn=true;continue;}
            if(std::string_view(argv[n])=="--workshop-fruit-audit"){fruit_audit=true;fprintf(stderr,"[FruitAudit] Passive capture enabled; controls and quitting remain manual.\n");continue;}
            if(std::string_view(argv[n])=="--workshop-native-hud"){special_hud=false;continue;}
            if(std::string_view(argv[n])=="--workshop-trace-startup"){trace_startup=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-tally"){test_tally=true;continue;}
            if(std::string_view(argv[n])=="--workshop-test-hybrid"){test_hybrid=true;continue;}
            if(std::string_view(argv[n])=="--workshop-relative-play"){relative_play=true;continue;}
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
                workshop::require(end_test_vi>=600 && end_test_vi<=12000,"Invalid test duration");continue;
            }
            if(std::string_view(argv[n])=="--workshop-auto-color"){
                workshop::require(n+1<argc,"Expected test Yoshi color");test_color=std::stoi(argv[++n]);workshop::require(test_color>=0 && test_color<14,"Invalid test Yoshi color");continue;
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
            workshop::require(size>=16 && size<=96096, "Invalid placement file size");
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
        workshop::require((!background_test && !trace_startup && !end_test_vi && !test_tally && !test_hybrid && !test_book_resume && !test_book_shop && !test_find_reveal && !test_release_spawn && !relative_play && test_stage<0 && test_zone<0 && !test_spawn && !test_warp && !fruit_seed && !test_native_history) || (!test_inputs.empty() && end_test_vi),"Automatic diagnostics require scripted input and a finite duration");
#endif
#ifdef WORKSHOP_CONTENT_BUILD
        if(test_native_history)workshop_request_native_history_test(test_native_history);
#endif
        return true;
    } catch (const std::exception& e) {
        fprintf(stderr,"[Workshop] Invalid project: %s\n",e.what());fflush(stderr);return false;
    }
}

void workshop_on_init(uint8_t* rdram, recomp_context*) {
#ifdef WORKSHOP_CONTENT_BUILD
#ifndef WORKSHOP_DISABLE_HYBRID_FINDS
    workshop::finds::initialize();
#endif
    if(!workshop_menu_enabled() && end_test_vi){workshop::collections::initialize();workshop::collections::start_session(test_hybrid?3:1);workshop::hybrid_mode::initialize();workshop::hybrid_mode::activate(test_hybrid);}
    diagnostic_rdram=rdram;test_vi=0;
    char* base=SDL_GetBasePath();workshop::require(base!=nullptr,"Cannot locate content engine folder");
    auto path=std::filesystem::u8path(base)/"workshop-content.bin";SDL_free(base);
    std::ifstream input(path,std::ios::binary|std::ios::ate);
    workshop::require(bool(input) && input.tellg()==workshop::rom_size,"Missing built-in Yoshi content data");
    std::vector<uint8_t> rom(workshop::rom_size);input.seekg(0);input.read(reinterpret_cast<char*>(rom.data()),rom.size());
    uint64_t hash=14695981039346656037ULL;
    for(uint8_t value:rom){hash^=value;hash*=1099511628211ULL;}
    workshop::require(hash==workshop_content_hash,"Content data differs from the recompiled game");
    if(!payload.empty())rom=workshop::apply(rom,payload,0xFF0000,
#ifdef WORKSHOP_SPECIAL_CONTENT
        true
#else
        false
#endif
    );
    if(test_zone>=0){
        const size_t target=0xA84A0+size_t(workshop::vanilla_sources[test_stage>=0?test_stage:0])*8,source=0xA84A0+size_t(test_zone)*8;
        std::copy_n(rom.begin()+source,8,rom.begin()+target);
        fprintf(stderr,"[Workshop] Diagnostic 1-1 scene uses zone %d.\n",test_zone);
        if(test_spawn){
            auto bank=[&](uint32_t p){return size_t(workshop::u32(rom,0xA79D4+((p>>24)&15)*4))+(p&0xFFFFFF);};
            const auto header=bank(workshop::u32(rom,target+4));
            const auto meta=bank(workshop::u32(rom,bank(workshop::u32(rom,header+0x5C))));
            const auto objects=bank(workshop::u32(rom,meta+4));
            for(unsigned i=0;i<workshop::u16(rom,meta);i++)if(workshop::u16(rom,objects+i*12)==0x4001){
                workshop::put32(rom,objects+i*12+4,std::bit_cast<uint32_t>(test_spawn_x));workshop::put32(rom,objects+i*12+8,std::bit_cast<uint32_t>(test_spawn_y));
            }
        }
    }
    recomp::set_rom_contents(std::move(rom));
    recomp::do_rom_read(rdram,gpr(int32_t(0x80000400)),0x10001000,0x100000);
#ifdef WORKSHOP_SPECIAL_CONTENT
    recomp::do_rom_read(rdram,gpr(int32_t(0x807FC000)),0x10FB0000,120);
    workshop::hybrid_mode::custom_placements(payload);
    workshop::hybrid_mode::initialize_ram(rdram);
#endif
    load_overlays(0xF10000,int32_t(0x807F0000),0x2000);
    load_overlays(0xF60000,int32_t(0x807F2000),0x6000);
    recomp::overlays::add_loaded_function(int32_t(0x80000438),workshop_80000438);
    recomp::overlays::add_loaded_function(int32_t(0x807F1600),workshop_807F1600);
    fprintf(stderr,"[Workshop] Built-in content: 14 Yoshis and v9.2 rules; custom placements %u.\n",payload.empty()?0:workshop::payload_count(payload));
    fflush(stderr);
#else
    if (payload.empty()) return;
    auto rom=workshop::apply(recomp::get_rom(),payload);
    recomp::set_rom_contents(std::move(rom));
    // init() has already copied the main 1 MiB into RDRAM. Refresh the one
    // descriptor table entry as well as the in-memory ROM used by future DMA.
    for(const auto& room:workshop::payload_rooms(payload)){const uint32_t entry=0xA84A0+room.source*8;recomp::do_rom_read(rdram,gpr(int32_t(0x80000400+entry-0x1000)),0x10000000+entry,8);}
    fprintf(stderr,"[Workshop] Loaded custom source %u: %u placements; original ROM verified; native code unchanged.\n",workshop::u32(payload,8),workshop::payload_count(payload));
    fflush(stderr);
#endif
}
