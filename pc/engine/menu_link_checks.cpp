#include "workshop_menu_game.hpp"
#include "story_progress.hpp"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>
std::atomic_bool exited{false};
static bool menu_enabled=true;
static int selected_mode=-1,confirmations=0;
static unsigned confirmed_choice=99;
bool workshop_menu_enabled(){return menu_enabled;}
int workshop_menu_take_boot_mode(){int mode=selected_mode;selected_mode=-1;return mode;}
extern "C" int workshop_menu_title(uint8_t*,recomp_context*);
extern "C" void func_80453F10(uint8_t* rdram,recomp_context* ctx){
    const gpr data=int32_t(MEM_W(0x1F4,ctx->r4));
    const gpr pad=int32_t(MEM_W(0,int32_t(0x800F7EA8)));
    if(MEM_HU(12,pad)!=0x1000)std::abort();
    confirmed_choice=MEM_W(0x16D10,data);confirmations++;
    ctx->r4=99;ctx->r29=0;ctx->r31=123;
}
static void check(bool ok,const char* message){if(!ok){fprintf(stderr,"Book regression: %s\n",message);std::exit(1);}}
template<class F> void until(F f){
    auto end=std::chrono::steady_clock::now()+std::chrono::seconds(2);
    while(!f()){check(std::chrono::steady_clock::now()<end,"native frame did not respond");std::this_thread::sleep_for(std::chrono::milliseconds(1));}
}
int main(){
    using namespace workshop::book;
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
    constexpr gpr root=gpr(int32_t(0x800F7EA0));
    recomp_context ctx{};ctx.r4=root;ctx.r29=gpr(int32_t(0x800F0000));ctx.r31=0x12345678;
    MEM_B(2,root)=5;MEM_B(0x57D,root)=5;
    MEM_W(0,gpr(int32_t(0x800F8E5C)))=0x111111;
    workshop::progress::State p{0,255,{255,255,255,255},0,0};
    workshop::progress::write(rdram,gpr(int32_t(0x807F2000)),p);
    enable();initial_navigation_finished();frame(rdram,&ctx,1);
    auto original=ctx;auto before=memory;
    pause();std::thread parked([&]{frame(rdram,&ctx,2);});
    until([]{return snapshot().paused;});
    check(snapshot().story==0x111111,"live Story progression missing");
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    check(memory==before,"pause changed game memory");
    resume();parked.join();
    check(!std::memcmp(&original,&ctx,sizeof(ctx)),"pause changed native registers");
    MEM_B(2,root)=0;MEM_B(0x57D,root)=4;
    for(unsigned vi=3;vi<15;vi++)frame(rdram,&ctx,vi);
    check(!take_open_request(),"Story page selection was mistaken for a mode exit");
    request_open();
    check(take_open_request(),"native menu return did not request the book");
    check(!take_open_request(),"menu return request repeated");
    check(!snapshot().paused,"game parked before UI presentation was ready");
    p={0x28903,3,{2,1,0,5},0,63};
    workshop::progress::write(rdram,gpr(int32_t(0x807F2000)),p);
    MEM_W(0,gpr(int32_t(0x800F8E5C)))=0xFFFFFF;frame(rdram,&ctx,15);
    check(snapshot().valid && snapshot().extras==63 && snapshot().trial==0x28903,"Book omitted sixth hybrid or changed Trial completion count");
    constexpr gpr pad=int32_t(0x8010E170),actor=int32_t(0x800C88E0),data=int32_t(0x8039B660);
    MEM_W(8,root)=int32_t(pad);MEM_W(0x1F4,actor)=int32_t(data);MEM_H(12,pad)=0x8000;
    auto route=ctx;route.r4=actor;const auto route_before=route;
    for(int mode=0;mode<4;mode++){
        selected_mode=mode;
        check(workshop_menu_title(rdram,&route)==1,"book mode did not replace native menu");
        check(confirmed_choice==unsigned(mode==0?0:mode==2?3:1),"wrong native mode selected");
        check(MEM_HU(12,pad)==0x8000,"title routing leaked a simulated Start into the next screen");
        check(!std::memcmp(&route_before,&route,sizeof(route)),"title routing changed caller registers");
    }
    menu_enabled=false;selected_mode=1;
    check(!workshop_menu_title(rdram,&route) && confirmations==4,"raw native diagnostics were rerouted");menu_enabled=true;
    // B in a level remains gameplay input. B in selection is consumed before
    // parking so resuming cannot accidentally open the old native menu.
    MEM_B(2,root)=7;MEM_B(0x57D,root)=43;MEM_H(12,pad)=0x4000;frame(rdram,&ctx,16);
    check(!take_open_request() && MEM_HU(12,pad)==0x4000,"gameplay B was intercepted");
    MEM_B(2,root)=0;MEM_B(0x57D,root)=6;frame(rdram,&ctx,16);
    check(!take_open_request() && MEM_HU(12,pad)==0x4000,"name-entry B was intercepted");
    MEM_B(2,root)=0;MEM_B(0x57D,root)=41;
    MEM_H(12,pad)=0;frame(rdram,&ctx,17);
    uint16_t buttons=0;float stick_x=0,stick_y=0;
    filter_input(&buttons,&stick_x,&stick_y);
    buttons=0x4000;filter_input(&buttons,&stick_x,&stick_y);
    check(buttons==0 && take_open_request(),"selection B reached the native pad before opening the book");
    std::thread back_frame([&]{frame(rdram,&ctx,18);});until([]{return snapshot().paused;});
    resume();back_frame.join();
    buttons=0x8000;filter_input(&buttons,&stick_x,&stick_y);
    check(buttons==0,"Resume confirmation leaked into native selection");
    buttons=0;filter_input(&buttons,&stick_x,&stick_y);
    MEM_B(2,root)=7;MEM_B(0x57D,root)=43;frame(rdram,&ctx,19);
    buttons=0x4000;filter_input(&buttons,&stick_x,&stick_y);
    check(buttons==0x4000 && !take_open_request(),"gameplay tongue B was filtered");
    MEM_B(2,root)=5;pause();std::thread shutdown([&]{frame(rdram,&ctx,20);});
    until([]{return snapshot().paused;});exited=true;shutdown.join();
    puts("Book: pause/resume RAM and registers, 4 direct mode routes, input restoration, B selection/gameplay separation and shutdown passed.");
}

