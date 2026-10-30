#pragma once
#include "recomp.h"
#include <cstdint>
namespace workshop::book {
struct Snapshot {
    bool ready=false,valid=false,paused=false,mainMenu=false;
    uint32_t story=0,trial=0,extras=0,mode=0,stage=0,scene=0;
    uint32_t frame=0,player=0,x=0,y=0,fruits=0,health=0;
};
void enable();
void initial_navigation_finished();
void pause();
void resume();
void filter_input(uint16_t* buttons,float* x,float* y);
Snapshot snapshot();
bool take_open_request();
void request_open();
void frame(uint8_t* rdram,recomp_context* ctx,unsigned vi);
}
