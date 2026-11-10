#include "coop_native_dispatch.hpp"

#if defined(WORKSHOP_NATIVE_COOP)
// Keep this list identical to prepare-coop-wrappers0233.py. In the ordinary
// build this translation unit defines no func_* symbol and changes no call.
// Do not use RECOMP_FUNC here: its clang C `extern inline weak` definition
// has different C++ emission rules and can leave no public wrapper symbol.
#if defined(_MSC_VER)
#define COOP_NOINLINE __declspec(noinline)
#else
#define COOP_NOINLINE __attribute__((noinline))
#endif
#define COOP_NATIVE_WRAPPER(address)                                      \
    extern "C" void coop_original_func_##address(uint8_t*, recomp_context*); \
    extern "C" COOP_NOINLINE void func_##address(uint8_t* rdram, recomp_context* ctx) { \
        workshop::coop::dispatch::invoke(                                 \
            rdram, ctx, 0x##address##u, coop_original_func_##address);       \
    }

#define COOP_ROOM_WRAPPER(address) \
 extern "C" void coop_original_func_##address(uint8_t*,recomp_context*); \
 extern "C" COOP_NOINLINE void func_##address(uint8_t* r,recomp_context* c){workshop::coop::dispatch::invoke_room(r,c,0x##address##u,coop_original_func_##address);}
COOP_ROOM_WRAPPER(805AB308)
COOP_ROOM_WRAPPER(805AB4D8)
COOP_ROOM_WRAPPER(80061170)
COOP_ROOM_WRAPPER(805AAF8C)
COOP_ROOM_WRAPPER(8002F7AC)
COOP_ROOM_WRAPPER(80030B7C)
COOP_ROOM_WRAPPER(8005D1C8)
COOP_ROOM_WRAPPER(8002BEB0)
COOP_ROOM_WRAPPER(800443EC)
COOP_ROOM_WRAPPER(800440E4)
COOP_ROOM_WRAPPER(805ABBE8)
#undef COOP_ROOM_WRAPPER

COOP_NATIVE_WRAPPER(8001E094)
COOP_NATIVE_WRAPPER(8001E20C)
// Native tongue collider calls4700C outside the actor-update dispatcher.
// ItsA0 is400C, whose parent chain resolves the receiving Yoshi.
extern "C" void coop_original_func_8003C510(uint8_t*,recomp_context*);
extern "C" COOP_NOINLINE void func_8003C510(uint8_t* r,recomp_context* c){
    workshop::coop::dispatch::invoke_food(r,c,0x8003C510u,coop_original_func_8003C510);
}
extern "C" void coop_original_func_80032A8C(uint8_t*,recomp_context*);
extern "C" COOP_NOINLINE void func_80032A8C(uint8_t* r,recomp_context* c){
    workshop::coop::dispatch::invoke_food(r,c,0x80032A8Cu,coop_original_func_80032A8C);
}
COOP_NATIVE_WRAPPER(80022448)
COOP_NATIVE_WRAPPER(800233B4)
COOP_NATIVE_WRAPPER(8002387C)
COOP_NATIVE_WRAPPER(80039F48)
COOP_NATIVE_WRAPPER(8003FFD8)
COOP_NATIVE_WRAPPER(80043450)
COOP_NATIVE_WRAPPER(800434EC)
COOP_NATIVE_WRAPPER(800439F4)
COOP_NATIVE_WRAPPER(80043E10)
COOP_NATIVE_WRAPPER(80043E88)
COOP_NATIVE_WRAPPER(80045B64)
COOP_NATIVE_WRAPPER(80048A30)
COOP_NATIVE_WRAPPER(80048AE0)
COOP_NATIVE_WRAPPER(80048060)
COOP_NATIVE_WRAPPER(8005E884)
COOP_NATIVE_WRAPPER(800675F0)

//4700C does not read8828: it consults the actor registry then falls back to
//46E88(4001). Always run its original to retain native register clobbers and
// side effects; override onlyV0 when the adapter has an enclosing owner scope.
extern "C" void coop_original_func_8004700C(uint8_t*, recomp_context*);
extern "C" COOP_NOINLINE void func_8004700C(uint8_t* rdram, recomp_context* ctx) {
    const uint32_t selected = workshop_coop_scoped_player(rdram);
    coop_original_func_8004700C(rdram, ctx);
    if (selected) ctx->r2 = gpr(int32_t(selected));
}

// These five native pair-work functions may be suppressed before their
// original body. Unrelated native pairs still run exactly once under Scope.
#define COOP_PAIR_WRAPPER(address)                                        \
    extern "C" void coop_original_func_##address(uint8_t*, recomp_context*); \
    extern "C" COOP_NOINLINE void func_##address(uint8_t* r, recomp_context* c) { \
        workshop::coop::dispatch::invoke_collision(                       \
            r, c, 0x##address##u, coop_original_func_##address);           \
    }
COOP_PAIR_WRAPPER(8006009C)
COOP_PAIR_WRAPPER(80060274)
COOP_PAIR_WRAPPER(800603E8)
COOP_PAIR_WRAPPER(80060798)
COOP_PAIR_WRAPPER(80060960)
#undef COOP_PAIR_WRAPPER

extern "C" void coop_original_func_8003B8F0(uint8_t*, recomp_context*);
extern "C" COOP_NOINLINE void func_8003B8F0(uint8_t* r, recomp_context* c) {
    workshop::coop::dispatch::invoke_egg_factory(
        r, c, 0x8003B8F0u, coop_original_func_8003B8F0);
}

extern "C" void coop_original_func_8003AE00(uint8_t*, recomp_context*);
extern "C" COOP_NOINLINE void func_8003AE00(uint8_t* r, recomp_context* c) {
    workshop::coop::dispatch::invoke_egg_private_init(
        r, c, 0x8003AE00u, coop_original_func_8003AE00);
}

#undef COOP_NATIVE_WRAPPER
#undef COOP_NOINLINE
#endif
