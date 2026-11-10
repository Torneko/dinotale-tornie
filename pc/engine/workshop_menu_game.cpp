#include "workshop_menu_game.hpp"
#include "workshop_menu.hpp"
#include "story_progress.hpp"
#include "game_modes0320.hpp"
#include "hybrid_finds.hpp"
#include "story_hard_bridge0330.hpp"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <mutex>
#include <thread>
extern std::atomic_bool exited;
namespace workshop::book {
namespace {
constexpr gpr manager=gpr(int32_t(0x800F7EA0));
std::atomic_bool enabled{false},paused{false},initial_done{false},open_requested{false};
std::atomic_bool discard_input{false};
std::mutex snapshot_mutex;
Snapshot current;
unsigned last_scene=~0u;
Snapshot read_snapshot(uint8_t* rdram,unsigned vi){
    Snapshot s;s.ready=true;s.paused=paused.load();s.frame=vi;
    s.stage=MEM_BU(2,manager);s.scene=MEM_BU(0x57D,manager);
    s.mainMenu=s.stage==0 && s.scene<=4;
    s.mode=MEM_W(0,gpr(int32_t(0x800F8B88)));
    s.story=uint32_t(MEM_W(0,gpr(int32_t(0x800F8E5C))))&0xFFFFFF;
    auto p=workshop::progress::read(rdram,gpr(int32_t(0x807F2000)));
    s.valid=workshop::progress::valid(p,s.story);
    if(s.valid){s.trial=p.trial;s.extras=workshop::progress::unlocked(p,s.story);}
    const auto finds=uint32_t(MEM_W(0,int32_t(0x807FC144)));
    if((finds&0xFFFFFFC0)==0xF17D0000)s.extras&=finds&63;
    if(workshop_menu_session_mode()==game_modes0320::StoryHybrid){
        const auto owned=workshop::finds::view();
        s.extras=owned.initialized&&owned.readable&&owned.migrated&&owned.saved?owned.found&63:0;
    }
    if(workshop_menu_session_mode()==game_modes0320::StoryHard)s.extras=(workshop::story_hard_bridge0330::available().value_or(0)>>8)&63;
    s.player=MEM_W(0,gpr(int32_t(0x800F8828)));
    s.fruits=MEM_W(0,gpr(int32_t(0x800F89D0)));
    if(s.player>=0x80000000 && s.player<0x807FF000){
        auto actor=gpr(int32_t(s.player));s.x=MEM_W(0x3C,actor);s.y=MEM_W(0x40,actor);
    }
    return s;
}
void publish(const Snapshot& s){std::lock_guard lock(snapshot_mutex);current=s;}
void note(const char* event,const Snapshot& s){
    fprintf(stderr,"[BookLink] %s vi=%u stage=%u scene=%u mode=%u player=%08X xy=%08X,%08X fruits=%u story=%06X trial=%06X extras=%02X\n",
        event,s.frame,s.stage,s.scene,s.mode,s.player,s.x,s.y,s.fruits,s.story,s.trial,s.extras);fflush(stderr);
}
}
void enable(){enabled=true;}
void initial_navigation_finished(){initial_done=true;}
void pause(){discard_input=true;paused=true;}
void resume(){paused=false;}
void filter_input(uint16_t* buttons,float* x,float* y){
    if(!enabled)return;
    const auto s=snapshot();
    // Intercept before the pad update: at the end-of-frame pause hook the
    // selection actors have already handled B and scheduled a native Back.
    if(initial_done && !paused && s.stage==0 && (s.scene==5 || s.scene==9 || s.scene==41) && (*buttons&0x4000)){
        request_open();pause();
    }
    if(paused || discard_input){
        if(!paused && !*buttons && !*x && !*y)discard_input=false;
        *buttons=0;*x=0;*y=0;
    }
}
Snapshot snapshot(){std::lock_guard lock(snapshot_mutex);return current;}
bool take_open_request(){return open_requested.exchange(false);}
void request_open(){if(enabled && initial_done)open_requested=true;}
void frame(uint8_t* rdram,recomp_context* ctx,unsigned vi){
    if(!enabled || uint32_t(ctx->r4)!=uint32_t(manager))return;
    auto s=read_snapshot(rdram,vi);
    unsigned key=(s.stage<<8)|s.scene;
    if(last_scene!=key){last_scene=key;note("scene",s);}
    publish(s);
    // Selection screens handle B here; gameplay retains its tongue action.
    if(initial_done && s.stage==0 && (s.scene==5 || s.scene==9 || s.scene==41)){
        const gpr pad=int32_t(MEM_W(8,manager));
        if(uint32_t(pad)>=0x80000000 && uint32_t(pad)<0x807FFFF0 && (MEM_HU(12,pad)&0x4000)){
            MEM_H(12,pad)=MEM_HU(12,pad)&~0x4000;
            note("back-to-book",s);request_open();pause();
        }
    }
    if(paused){
        s=read_snapshot(rdram,vi);s.paused=true;publish(s);note("paused",s);
        while(paused.load() && !exited.load())std::this_thread::sleep_for(std::chrono::milliseconds(5));
        fprintf(stderr,"[BookLink] wake vi=%u exited=%d\n",vi,int(exited.load()));fflush(stderr);
        note("resumed",read_snapshot(rdram,vi));
    }
}
}

extern "C" void func_80453F10(uint8_t*,recomp_context*);
extern "C" int workshop_menu_title(uint8_t* rdram,recomp_context* ctx){
    if(!workshop_menu_enabled())return 0;
    const gpr actor=ctx->r4;
    if(uint32_t(actor)<0x80000000 || uint32_t(actor)>=0x807FFE00)return 0;
    const gpr data=int32_t(MEM_W(0x1F4,actor));
    if(uint32_t(data)<0x80000000 || uint32_t(data)>0x807E92EC)return 0;
    const gpr pad=int32_t(MEM_W(0,int32_t(0x800F7EA8)));
    if(uint32_t(pad)<0x80000000 || uint32_t(pad)>=0x807FFFF0)return 0;
    const int mode=workshop_menu_take_boot_mode();
    if(mode<0){workshop::book::request_open();workshop::book::pause();return 1;}
    // Reached only after the real title Start. Retain native mode setup and
    // cleanup, before the four-choice menu is constructed.
    const int choice=workshop::game_modes0320::native_choice(mode);
    if(choice<0)return 0;
    const auto pressed=MEM_HU(12,pad);
    MEM_W(0x16D10,data)=choice;MEM_H(12,pad)=0x1000;
    const auto saved=*ctx;
    func_80453F10(rdram,ctx);
    *ctx=saved;MEM_H(12,pad)=pressed;
    workshop::book::initial_navigation_finished();
    fprintf(stderr,"[BookLink] title-Start direct mode=%d nativeChoice=%u\n",mode,choice);fflush(stderr);
    return 1;
}

