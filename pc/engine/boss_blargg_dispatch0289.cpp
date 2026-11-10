#include "recomp.h"
#if defined(WORKSHOP_NATIVE_COOP)
extern "C" void workshop_battle_hook_leave(uint8_t*,const recomp_context*,const char*);
extern "C" void blargg_original_func_800447C8(uint8_t*,recomp_context*);
extern "C" void func_800447C8(uint8_t* r,recomp_context* c){
 const auto entry=*c;blargg_original_func_800447C8(r,c);
 workshop_battle_hook_leave(r,&entry,"func_800447C8");
}
extern "C" void blargg_original_func_804F42B8(uint8_t*,recomp_context*);
extern "C" void func_804F42B8(uint8_t* r,recomp_context* c){
 const auto entry=*c;blargg_original_func_804F42B8(r,c);
 workshop_battle_hook_leave(r,&entry,"func_804F42B8");
}
extern "C" void blargg_original_func_804F46E4(uint8_t*,recomp_context*);
extern "C" void func_804F46E4(uint8_t* r,recomp_context* c){
 const auto entry=*c;blargg_original_func_804F46E4(r,c);
 workshop_battle_hook_leave(r,&entry,"func_804F46E4");
}
extern "C" void blargg_original_func_804F57E4(uint8_t*,recomp_context*);
extern "C" void func_804F57E4(uint8_t* r,recomp_context* c){
 const auto entry=*c;blargg_original_func_804F57E4(r,c);
 workshop_battle_hook_leave(r,&entry,"func_804F57E4");
}
extern "C" void blargg_original_func_804F571C(uint8_t*,recomp_context*);
extern "C" void func_804F571C(uint8_t* r,recomp_context* c){
 const auto entry=*c;blargg_original_func_804F571C(r,c);
 workshop_battle_hook_leave(r,&entry,"func_804F571C");
}
extern "C" void blargg_original_func_804F5CE8(uint8_t*,recomp_context*);
extern "C" void func_804F5CE8(uint8_t* r,recomp_context* c){
 const auto entry=*c;blargg_original_func_804F5CE8(r,c);
 workshop_battle_hook_leave(r,&entry,"func_804F5CE8");
}
#endif
