#pragma once
#include "recomp.h"
#include <string>
namespace workshop::custom_session {
void configure(const std::string& token,bool automatic,int preferred,unsigned source);
bool active();
bool ready();
bool saved();
int preferred_color();
int source_stage();
void navigation(unsigned frame,uint16_t* buttons,float* x,float* y);
void finish(uint8_t* rdram);
bool close(bool advance);
}
