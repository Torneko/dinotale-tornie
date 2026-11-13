#include "recomp.h"
extern "C" uintptr_t workshop_coop_dispatch_enter(uint8_t*,recomp_context*,uint32_t,uint32_t,uint32_t);
extern "C" void workshop_coop_dispatch_leave(uint8_t*,recomp_context*,uint32_t,uint32_t,uint32_t,uintptr_t,int)noexcept;

extern "C" int workshop_battle_hook_enter(uint8_t*,recomp_context*,const char*);
extern "C" void workshop_battle_hook_leave(uint8_t*,const recomp_context*,const char*);
extern "C" int workshop_battle_paint_begin(uint8_t*,recomp_context*,unsigned);
extern "C" void workshop_battle_paint_finish(uint8_t*);
extern "C" void battle_original_func_8042D33C(uint8_t*,recomp_context*);
extern "C" void func_8042D33C(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x8042D33C);
 if(!workshop_battle_hook_enter(r,c,"func_8042D33C"))battle_original_func_8042D33C(r,c);
 workshop_battle_hook_leave(r,&entry,"func_8042D33C");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_806AF580(uint8_t*,recomp_context*);
extern "C" void func_806AF580(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x806AF580);
 if(!workshop_battle_hook_enter(r,c,"func_806AF580"))battle_original_func_806AF580(r,c);
 workshop_battle_hook_leave(r,&entry,"func_806AF580");if(paint)workshop_battle_paint_finish(r);
}
extern "C" uint64_t workshop_boss_tree_spawn_enter(uint8_t*,recomp_context*);
extern "C" void workshop_boss_tree_spawn_leave(uint8_t*,recomp_context*,uint64_t);
extern "C" void battle_original_func_80574B44(uint8_t*,recomp_context*);
extern "C" void func_80574B44(uint8_t* r,recomp_context* c){
 const auto ticket=workshop_boss_tree_spawn_enter(r,c);
 if(ticket==UINT64_MAX){c->r2=0;return;}
 battle_original_func_80574B44(r,c);
 workshop_boss_tree_spawn_leave(r,c,ticket);
}
extern "C" void battle_original_func_8055863C(uint8_t*,recomp_context*);
extern "C" void func_8055863C(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x8055863C);
 if(!workshop_battle_hook_enter(r,c,"func_8055863C"))battle_original_func_8055863C(r,c);
 workshop_battle_hook_leave(r,&entry,"func_8055863C");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_8042D090(uint8_t*,recomp_context*);
extern "C" void func_8042D090(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x8042D090);
 if(!workshop_battle_hook_enter(r,c,"func_8042D090"))battle_original_func_8042D090(r,c);
 workshop_battle_hook_leave(r,&entry,"func_8042D090");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_806AEFDC(uint8_t*,recomp_context*);
extern "C" void func_806AEFDC(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x806AEFDC);
 if(!workshop_battle_hook_enter(r,c,"func_806AEFDC"))battle_original_func_806AEFDC(r,c);
 workshop_battle_hook_leave(r,&entry,"func_806AEFDC");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_80557D44(uint8_t*,recomp_context*);
extern "C" void func_80557D44(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x80557D44);
 if(!workshop_battle_hook_enter(r,c,"func_80557D44"))battle_original_func_80557D44(r,c);
 workshop_battle_hook_leave(r,&entry,"func_80557D44");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_80666ACC(uint8_t*,recomp_context*);
extern "C" void func_80666ACC(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x80666ACC);
 if(!workshop_battle_hook_enter(r,c,"func_80666ACC"))battle_original_func_80666ACC(r,c);
 workshop_battle_hook_leave(r,&entry,"func_80666ACC");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_806F3A1C(uint8_t*,recomp_context*);
extern "C" void func_806F3A1C(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x806F3A1C);
 if(!workshop_battle_hook_enter(r,c,"func_806F3A1C"))battle_original_func_806F3A1C(r,c);
 workshop_battle_hook_leave(r,&entry,"func_806F3A1C");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_80554928(uint8_t*,recomp_context*);
extern "C" void func_80554928(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x80554928);
 if(!workshop_battle_hook_enter(r,c,"func_80554928"))battle_original_func_80554928(r,c);
 workshop_battle_hook_leave(r,&entry,"func_80554928");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_806F3498(uint8_t*,recomp_context*);
extern "C" void func_806F3498(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x806F3498);
 if(!workshop_battle_hook_enter(r,c,"func_806F3498"))battle_original_func_806F3498(r,c);
 workshop_battle_hook_leave(r,&entry,"func_806F3498");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_805C6900(uint8_t*,recomp_context*);
extern "C" void func_805C6900(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x805C6900);
 if(!workshop_battle_hook_enter(r,c,"func_805C6900"))battle_original_func_805C6900(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C6900");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_805C74C8(uint8_t*,recomp_context*);
extern "C" void func_805C74C8(uint8_t* r,recomp_context* c){
 const auto entry=*c;
 if(!workshop_battle_hook_enter(r,c,"func_805C74C8"))battle_original_func_805C74C8(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C74C8");
}
extern "C" void battle_original_func_805D3444(uint8_t*,recomp_context*);
extern "C" void func_805D3444(uint8_t* r,recomp_context* c){
 const auto entry=*c;
 if(!workshop_battle_hook_enter(r,c,"func_805D3444"))battle_original_func_805D3444(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805D3444");
}
extern "C" void battle_original_func_805C7C04(uint8_t*,recomp_context*);
extern "C" void func_805C7C04(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x805C7C04);
 if(!workshop_battle_hook_enter(r,c,"func_805C7C04"))battle_original_func_805C7C04(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C7C04");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_805C7C2C(uint8_t*,recomp_context*);
extern "C" void func_805C7C2C(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x805C7C2C);
 if(!workshop_battle_hook_enter(r,c,"func_805C7C2C"))battle_original_func_805C7C2C(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C7C2C");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_805C8140(uint8_t*,recomp_context*);
extern "C" void func_805C8140(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x805C8140);
 if(!workshop_battle_hook_enter(r,c,"func_805C8140"))battle_original_func_805C8140(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C8140");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_805C8240(uint8_t*,recomp_context*);
extern "C" void func_805C8240(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x805C8240);
 if(!workshop_battle_hook_enter(r,c,"func_805C8240"))battle_original_func_805C8240(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C8240");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_805C82C0(uint8_t*,recomp_context*);
extern "C" void func_805C82C0(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x805C82C0);
 if(!workshop_battle_hook_enter(r,c,"func_805C82C0"))battle_original_func_805C82C0(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C82C0");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_805C836C(uint8_t*,recomp_context*);
extern "C" void func_805C836C(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x805C836C);
 if(!workshop_battle_hook_enter(r,c,"func_805C836C"))battle_original_func_805C836C(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C836C");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_805C8904(uint8_t*,recomp_context*);
extern "C" void func_805C8904(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x805C8904);
 if(!workshop_battle_hook_enter(r,c,"func_805C8904"))battle_original_func_805C8904(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C8904");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_805C8988(uint8_t*,recomp_context*);
extern "C" void func_805C8988(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x805C8988);
 if(!workshop_battle_hook_enter(r,c,"func_805C8988"))battle_original_func_805C8988(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C8988");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_805C8A44(uint8_t*,recomp_context*);
extern "C" void func_805C8A44(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x805C8A44);
 if(!workshop_battle_hook_enter(r,c,"func_805C8A44"))battle_original_func_805C8A44(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C8A44");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_805C8D30(uint8_t*,recomp_context*);
extern "C" void func_805C8D30(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x805C8D30);
 if(!workshop_battle_hook_enter(r,c,"func_805C8D30"))battle_original_func_805C8D30(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C8D30");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_805C8E20(uint8_t*,recomp_context*);
extern "C" void func_805C8E20(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x805C8E20);
 if(!workshop_battle_hook_enter(r,c,"func_805C8E20"))battle_original_func_805C8E20(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C8E20");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_805C9080(uint8_t*,recomp_context*);
extern "C" void func_805C9080(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x805C9080);
 if(!workshop_battle_hook_enter(r,c,"func_805C9080"))battle_original_func_805C9080(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C9080");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_805C9154(uint8_t*,recomp_context*);
extern "C" void func_805C9154(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x805C9154);
 if(!workshop_battle_hook_enter(r,c,"func_805C9154"))battle_original_func_805C9154(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C9154");if(paint)workshop_battle_paint_finish(r);
}
extern "C" uint64_t workshop_elemental_tongue_begin0355(uint8_t*,uint32_t);
extern "C" void workshop_elemental_tongue_end0355(uint8_t*,uint64_t)noexcept;
extern "C" void battle_original_func_8005F700(uint8_t*,recomp_context*);
extern "C" void func_8005F700(uint8_t* r,recomp_context* c){
 const auto entry=*c;const auto owner=workshop_coop_dispatch_enter(r,c,0x8005F700,uint32_t(entry.r4),uint32_t(entry.r5));
 struct TongueDraw {uint8_t* ram;uint64_t token;~TongueDraw(){workshop_elemental_tongue_end0355(ram,token);}} tongue{r,workshop_elemental_tongue_begin0355(r,uint32_t(entry.r4))};
 const bool paint=workshop_battle_paint_begin(r,c,0x8005F700);
 if(!workshop_battle_hook_enter(r,c,"func_8005F700"))battle_original_func_8005F700(r,c);
 workshop_battle_hook_leave(r,&entry,"func_8005F700");if(paint)workshop_battle_paint_finish(r);workshop_coop_dispatch_leave(r,c,0x8005F700,uint32_t(entry.r4),uint32_t(entry.r5),owner,1);
}
extern "C" void battle_original_func_805C6500(uint8_t*,recomp_context*);
extern "C" void func_805C6500(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x805C6500);
 if(!workshop_battle_hook_enter(r,c,"func_805C6500"))battle_original_func_805C6500(r,c);
 workshop_battle_hook_leave(r,&entry,"func_805C6500");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_8066664C(uint8_t*,recomp_context*);
extern "C" void func_8066664C(uint8_t* r,recomp_context* c){
 const auto entry=*c;const bool paint=workshop_battle_paint_begin(r,c,0x8066664C);
 if(!workshop_battle_hook_enter(r,c,"func_8066664C"))battle_original_func_8066664C(r,c);
 workshop_battle_hook_leave(r,&entry,"func_8066664C");if(paint)workshop_battle_paint_finish(r);
}
extern "C" void battle_original_func_80666E34(uint8_t*,recomp_context*);
extern "C" void func_80666E34(uint8_t* r,recomp_context* c){const auto entry=*c;if(!workshop_battle_hook_enter(r,c,"func_80666E34"))battle_original_func_80666E34(r,c);workshop_battle_hook_leave(r,&entry,"func_80666E34");}

extern "C" uintptr_t workshop_flower_creator_begin(uint8_t*,recomp_context*);
extern "C" void workshop_flower_creator_finish(uintptr_t);
extern "C" int workshop_boss_intro_popup(uint8_t*,recomp_context*);
extern "C" int workshop_flower_petal_pool(uint8_t*,recomp_context*);
extern "C" void workshop_flower_position(uint8_t*,recomp_context*);
extern "C" void flower_original_func_8001CC20(uint8_t*,recomp_context*);
extern "C" void flower_original_func_80046804(uint8_t*,recomp_context*);
extern "C" void flower_original_func_800485B8(uint8_t*,recomp_context*);
extern "C" void func_8001CC20(uint8_t* r,recomp_context* c){const auto scope=workshop_flower_creator_begin(r,c);flower_original_func_8001CC20(r,c);workshop_flower_creator_finish(scope);}
extern "C" void func_80046804(uint8_t* r,recomp_context* c){if(workshop_boss_intro_popup(r,c)){c->r2=0;return;}if(workshop_flower_petal_pool(r,c)<0){c->r2=0;return;}flower_original_func_80046804(r,c);}
extern "C" void func_800485B8(uint8_t* r,recomp_context* c){workshop_flower_position(r,c);flower_original_func_800485B8(r,c);}

extern "C" int workshop_battle_selector(uint8_t*,recomp_context*);
extern "C" void selection_original_func_8079EDB8(uint8_t*,recomp_context*);
extern "C" void func_8079EDB8(uint8_t* r,recomp_context* c){if(!workshop_battle_selector(r,c))selection_original_func_8079EDB8(r,c);}

// Boss Battle checkpoints keep the original routines for Vanilla callers.
extern "C" int workshop_boss_checkpoint_spawn(uint8_t*,recomp_context*);
extern "C" uint64_t workshop_boss_checkpoint_init_enter(uint8_t*,const recomp_context*);
extern "C" void workshop_boss_checkpoint_init_leave(uint8_t*,const recomp_context*,uint64_t);
extern "C" uint64_t workshop_boss_checkpoint_wake_enter(uint8_t*,const recomp_context*);
extern "C" void workshop_boss_checkpoint_wake_leave(uint8_t*,const recomp_context*,uint64_t);
extern "C" int workshop_boss_checkpoint_block_warp(uint8_t*,const recomp_context*);
extern "C" int workshop_boss_checkpoint_retry_death(uint8_t*,recomp_context*);
extern "C" void battle_original_func_8004613C(uint8_t*,recomp_context*);
extern "C" void func_8004613C(uint8_t* r,recomp_context* c){
 if(workshop_boss_checkpoint_spawn(r,c)){c->r2=0;return;}
 battle_original_func_8004613C(r,c);
}
extern "C" void battle_original_func_805A6600(uint8_t*,recomp_context*);
extern "C" void func_805A6600(uint8_t* r,recomp_context* c){
 const auto entry=*c;const auto ticket=workshop_boss_checkpoint_init_enter(r,c);
 if(ticket==UINT64_MAX)return;
 battle_original_func_805A6600(r,c);workshop_boss_checkpoint_init_leave(r,&entry,ticket);
}
extern "C" void battle_original_func_805A5628(uint8_t*,recomp_context*);
extern "C" void func_805A5628(uint8_t* r,recomp_context* c){
 const auto entry=*c;const auto ticket=workshop_boss_checkpoint_wake_enter(r,c);
 battle_original_func_805A5628(r,c);workshop_boss_checkpoint_wake_leave(r,&entry,ticket);
}
extern "C" void battle_original_func_805A58AC(uint8_t*,recomp_context*);
extern "C" void func_805A58AC(uint8_t* r,recomp_context* c){
 if(workshop_boss_checkpoint_block_warp(r,c)){c->r2=1;return;}
 battle_original_func_805A58AC(r,c);
}
extern "C" void battle_original_func_8002F5F4(uint8_t*,recomp_context*);
extern "C" void func_8002F5F4(uint8_t* r,recomp_context* c){
 if(workshop_boss_checkpoint_retry_death(r,c))return;
 battle_original_func_8002F5F4(r,c);
}

// Keep the native floor stamp intact; only the proven encounter supports get
// persistent streaming margins after its actual native callback returns.
extern "C" void battle_original_func_8069D05C(uint8_t*,recomp_context*);
extern "C" void func_8069D05C(uint8_t* r,recomp_context* c){
 const auto entry=*c;
 if(!workshop_battle_hook_enter(r,c,"func_8069D05C"))battle_original_func_8069D05C(r,c);
 workshop_battle_hook_leave(r,&entry,"func_8069D05C");
}

extern "C" int workshop_boss_candy_pool_enter(uint8_t*,recomp_context*);
extern "C" void workshop_boss_candy_pool_leave(uint8_t*,const recomp_context*,int);
extern "C" void workshop_boss_candy_pool_cleanup(uint8_t*,const recomp_context*,const recomp_context*);
extern "C" void battle_original_func_8004667C(uint8_t*,recomp_context*);
extern "C" void func_8004667C(uint8_t* r,recomp_context* c){
 const int ticket=workshop_boss_candy_pool_enter(r,c);if(ticket<0)return;
 battle_original_func_8004667C(r,c);workshop_boss_candy_pool_leave(r,c,ticket);
}
extern "C" void battle_original_func_80046324(uint8_t*,recomp_context*);
extern "C" void func_80046324(uint8_t* r,recomp_context* c){
 const auto entry=*c;battle_original_func_80046324(r,c);workshop_boss_candy_pool_cleanup(r,&entry,c);
}

// Keep the original constructor intact, adapting only the owned Lips-companion entry after return.
extern "C" void battle_original_func_80664E18(uint8_t*,recomp_context*);
extern "C" void func_80664E18(uint8_t* r,recomp_context* c){
 const auto entry=*c;
 if(!workshop_battle_hook_enter(r,c,"func_80664E18"))battle_original_func_80664E18(r,c);
 workshop_battle_hook_leave(r,&entry,"func_80664E18");
}
