#include "recomp.h"
extern "C" void boss_original_func_807AEA78(uint8_t*,recomp_context*);
extern "C" void boss_original_func_8005D320(uint8_t*,recomp_context*);
extern "C" int workshop_battle_shared_completion(uint8_t*,recomp_context*);
extern "C" void workshop_boss_defeat_begin(uint8_t*,recomp_context*);
extern "C" void workshop_boss_defeat_complete(uint8_t*,recomp_context*);
extern "C" int workshop_boss_pot_complete(uint8_t*,recomp_context*);
extern "C" void func_807AEA78(uint8_t* r,recomp_context* c){
    if(workshop_battle_shared_completion(r,c))return;
    workshop_boss_defeat_begin(r,c);boss_original_func_807AEA78(r,c);workshop_boss_defeat_complete(r,c);
}
extern "C" void func_8005D320(uint8_t* r,recomp_context* c){
    if(!workshop_boss_pot_complete(r,c))boss_original_func_8005D320(r,c);
}
