#pragma once
#include "recomp.h"
bool workshop_configure(int& argc, char** argv);
void workshop_on_init(uint8_t* rdram, recomp_context* context);
#ifdef WORKSHOP_CONTENT_BUILD
void workshop_tick_vi();
bool workshop_background_test();
bool workshop_test_book_resume();
bool workshop_test_book_shop();
bool workshop_get_input(int player,uint16_t* buttons,float* x,float* y);
extern "C" void workshop_dump_trace();
#endif
