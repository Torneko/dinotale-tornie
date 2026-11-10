#pragma once

#include <cstdint>
#include "recomp.h"

// The generated C definitions are renamed only under WORKSHOP_NATIVE_COOP.
// Public func_* references, direct calls and overlay tables retain these names.
// Implement the adapter hooks before linking this opt-in prototype.
extern "C" uintptr_t workshop_coop_dispatch_enter(
    uint8_t* rdram, recomp_context* ctx, uint32_t function_vram,
    uint32_t actor_a0, uint32_t argument_a1);
extern "C" void workshop_coop_dispatch_leave(
    uint8_t* rdram, recomp_context* ctx, uint32_t function_vram,
    uint32_t actor_a0, uint32_t argument_a1, uintptr_t ticket,
    int completed) noexcept;
// Return only an active scoped player's validated native actor, or0. This is
// separate from8828 because4700C otherwise searches the shared actor registry.
extern "C" uint32_t workshop_coop_scoped_player(uint8_t* rdram);
// Pair-work arguments are native collision work objects, not presumed actors.
extern "C" int workshop_coop_skip_collision(
    uint8_t* rdram, recomp_context* ctx, uint32_t function_vram, uint32_t work);
//3B8F0 has float coordinates and colour arguments, no player argument. Capture
// an enclosing validated owner before native code can mutate registers/scopes.
//0/1 denote valid slots; UINT32_MAX means no known owner and must not attribute.
extern "C" uint32_t workshop_coop_capture_egg_owner(
    uint8_t* rdram, recomp_context* ctx) noexcept;
extern "C" void workshop_coop_egg_created(
    uint8_t* rdram, recomp_context* ctx, uint32_t native_v0, uint32_t owner_slot) noexcept;
// Native3AE00 may initialize an already-owned4002 private block. The opaque
// one-shot token binds the proven birth/owner before that native transition.
extern "C" uint64_t workshop_coop_begin_egg_private_init(
    uint8_t* rdram, recomp_context* ctx, uint32_t actor) noexcept;
extern "C" void workshop_coop_finish_egg_private_init(
    uint8_t* rdram, recomp_context* ctx, uint64_t token, int completed) noexcept;


// Food hooks are inert unless a live two-player room43 bridge is active.
// UINT64_MAX suppresses only an unproved/contended fruit call; zero bypasses.
extern "C" uint64_t workshop_coop_food_begin(uint8_t*,recomp_context*,uint32_t,uint32_t,uint32_t) noexcept;
extern "C" void workshop_coop_food_finish(uint8_t*,recomp_context*,uint64_t,int) noexcept;

extern "C" uint64_t workshop_coop_rooms_begin(uint8_t*,recomp_context*,uint32_t,uint32_t,uint32_t,uint32_t) noexcept;
extern "C" void workshop_coop_rooms_finish(uint8_t*,recomp_context*,uint64_t,int) noexcept;

namespace workshop::coop::dispatch {
// Room interposition is opt-in, including an explicit per-candidate input
// read in805AAF8C. Zero keeps the body; UINT64_MAX suppresses only a scoped
// call already handled/deferred by the adapter. No register clobber leaks from
// adapter helpers into the original native prologue.
inline void invoke_room(uint8_t* r,recomp_context* c,uint32_t function,recomp_func_t* original){
 const auto saved=*c;const auto ticket=workshop_coop_rooms_begin(r,c,function,uint32_t(saved.r4),uint32_t(saved.r5),uint32_t(saved.r6));*c=saved;
 if(ticket==UINT64_MAX){c->r2=0;return;}
 try{original(r,c);workshop_coop_rooms_finish(r,c,ticket,1);}catch(...){workshop_coop_rooms_finish(r,c,ticket,0);throw;}
}
// All addresses are canonical symbols, never a dynamically relocated overlay
// address. These selected callbacks belong to the permanently loaded main code.
enum class Function : uint32_t {
    player_collision = 0x8001E094,
    player_collision_extra = 0x8001E20C,
    tongue_collision = 0x8003C510,
    food_ingestion = 0x80032A8C,
    legacy_actor_kind_handler = 0x80022448,
    actor_draw = 0x8005E884,
    egg_factory = 0x8003B8F0,
    egg_private_init = 0x8003AE00,
    item_detach = 0x80048060,
    collision_pair_6009C = 0x8006009C,
    collision_pair_60274 = 0x80060274,
    collision_pair_603E8 = 0x800603E8,
    collision_pair_60798 = 0x80060798,
    collision_pair_60960 = 0x80060960,
    player_screen_bounds = 0x800233B4,
    player_screen_bounds_extra = 0x8002387C,
    item_attach = 0x80039F48,
    item_attach_state = 0x8003FFD8,
    player_update = 0x80043450,
    player_update_extra = 0x800434EC,
    actor_fast_free = 0x800439F4,
    hierarchy_parent_first = 0x80043E10,
    hierarchy_children_first = 0x80043E88,
    actor_destroy = 0x80045B64,
    current_player_lookup = 0x8004700C,
    actor_update_dispatch = 0x80048A30,
    actor_post_update_dispatch = 0x80048AE0,
    // Not an actor callback: A0/A1 are scheduler arguments. The native body
    // drains its graphics messages before the adapter's after hook may spawn.
    graphics_frame_fence = 0x800675F0,
};

class Scope final {
    uint8_t* rdram_;
    recomp_context* ctx_;
    uint32_t function_, actor_, argument_;
    uintptr_t ticket_ = 0;
    bool completed_ = false;
public:
    Scope(uint8_t* rdram, recomp_context* ctx, uint32_t function)
        : rdram_(rdram), ctx_(ctx), function_(function),
          actor_(uint32_t(ctx->r4)), argument_(uint32_t(ctx->r5)) {
        // Adapter hooks may call native helpers internally. Their transient
        // guest registers must not replace the callee's incoming registers.
        const auto registers = *ctx_;
        try {
            ticket_ = workshop_coop_dispatch_enter(
                rdram_, ctx_, function_, actor_, argument_);
        } catch (...) {
            *ctx_ = registers;
            throw;
        }
        *ctx_ = registers;
    }
    ~Scope() noexcept {
        // Attachment completion is useful even when no owner Scope was
        // admitted. Other ticket0 calls retain their original bypass behavior.
        if (!ticket_ && function_ != 0x80039F48u && function_ != 0x8003FFD8u) return;
        // Preserve every native result register, including floating point,
        // stack/link registers and early-return results, across the exit hook.
        const auto registers = *ctx_;
        workshop_coop_dispatch_leave(rdram_, ctx_, function_, actor_, argument_,
                                     ticket_, completed_ ? 1 : 0);
        *ctx_ = registers;
    }
    void completed() noexcept { completed_ = true; }
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
};

inline void invoke(uint8_t* rdram, recomp_context* ctx, uint32_t function,
                   recomp_func_t* original) {
    Scope scope(rdram, ctx, function);
    original(rdram, ctx);
    scope.completed();
}

inline void invoke_collision(uint8_t* rdram, recomp_context* ctx,
                             uint32_t function, recomp_func_t* original) {
    const auto incoming = *ctx;
    int skip = 0;
    try {
        skip = workshop_coop_skip_collision(
            rdram, ctx, function, uint32_t(incoming.r4));
    } catch (...) {
        *ctx = incoming;
        throw;
    }
    *ctx = incoming;
    if (skip) { ctx->r2 = 0; return; }
    invoke(rdram, ctx, function, original);
}

class FoodTransaction final {
    uint8_t* ram_;recomp_context* ctx_;uint64_t token_=0;bool completed_=false;
public:
    FoodTransaction(uint8_t* ram,recomp_context* ctx,uint32_t fn,uint32_t actor,uint32_t argument):ram_(ram),ctx_(ctx){
        const auto before=*ctx_;token_=workshop_coop_food_begin(ram_,ctx_,fn,actor,argument);*ctx_=before;
    }
    ~FoodTransaction()noexcept{const auto after=*ctx_;workshop_coop_food_finish(ram_,ctx_,token_,completed_?1:0);*ctx_=after;}
    bool blocked()const{return token_==UINT64_MAX;}
    void completed()noexcept{completed_=true;}
};
inline void invoke_food(uint8_t* r,recomp_context* c,uint32_t fn,recomp_func_t* original){
    const auto actor=uint32_t(c->r4),argument=uint32_t(c->r5);
    Scope owner(r,c,fn);FoodTransaction transaction(r,c,fn,actor,argument);
    if(transaction.blocked())return;
    original(r,c);transaction.completed();owner.completed();
}

inline void invoke_egg_factory(uint8_t* rdram, recomp_context* ctx,
                               uint32_t function, recomp_func_t* original) {
    const auto incoming = *ctx;
    const uint32_t owner = workshop_coop_capture_egg_owner(rdram, ctx);
    *ctx = incoming;
    Scope scope(rdram, ctx, function);
    original(rdram, ctx);
    scope.completed();
    // Run on every normal native return, including V0=0 and Scope ticket0.
    // Keep the enclosing scope alive through postprocessing; preserve the
    // complete native result context, particularly the signed guest V0.
    const auto result = *ctx;
    workshop_coop_egg_created(rdram, ctx, uint32_t(result.r2), owner);
    *ctx = result;
}

class EggPrivateInit final {
    uint8_t* rdram_;
    recomp_context* ctx_;
    uint64_t token_=0;
    bool completed_=false;
public:
    EggPrivateInit(uint8_t* rdram,recomp_context* ctx)
        :rdram_(rdram),ctx_(ctx) {
        const auto incoming=*ctx_;
        token_=workshop_coop_begin_egg_private_init(rdram_,ctx_,uint32_t(incoming.r4));
        *ctx_=incoming;
    }
    ~EggPrivateInit() noexcept {
        const auto result=*ctx_;
        workshop_coop_finish_egg_private_init(rdram_,ctx_,token_,completed_?1:0);
        *ctx_=result;
    }
    void completed() noexcept {completed_=true;}
    EggPrivateInit(const EggPrivateInit&)=delete;
    EggPrivateInit& operator=(const EggPrivateInit&)=delete;
};
inline void invoke_egg_private_init(uint8_t* rdram,recomp_context* ctx,
                                    uint32_t function,recomp_func_t* original) {
    Scope scope(rdram,ctx,function);
    // Construct after Scope so token finalization precedes owner restoration.
    // If Scope construction throws there is no token to leak or cancel.
    EggPrivateInit initialization(rdram,ctx);
    original(rdram,ctx);
    initialization.completed();
    scope.completed();
}
}
