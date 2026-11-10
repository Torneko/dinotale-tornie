#include "coop_native_dispatch.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
struct Event { uint32_t function, actor, argument; int complete; };
std::vector<Event> events;
std::vector<uint32_t> owners;
uint32_t owner = 7;
unsigned originals = 0, mode = 0;
bool enabled = true;
uint32_t selected_player = 0;
unsigned lookup_calls = 0, scoped_lookups = 0;
void need(bool ok, const char* why) { if (!ok) throw std::runtime_error(why); }
}
extern "C" uint32_t workshop_coop_scoped_player(uint8_t*) {
    ++scoped_lookups; return selected_player;
}
extern "C" void func_8004700C(uint8_t*, recomp_context*);
extern "C" void coop_original_func_8004700C(uint8_t*, recomp_context* ctx) {
    ++lookup_calls;
    ctx->r2 = gpr(int32_t(0x800CA000)); ctx->r3 = 0x12345678;
    ctx->r4 = 0x4001; ctx->r29 = 0x803FFF00; ctx->r31 = 0x80047048;
    ctx->f0.u64 = 0x1020304050607080ULL;
    // The wrapper must select its enclosing owner before calling the native
    // lookup, while still preserving all native side effects and clobbers.
    selected_player = 0;
}

extern "C" uintptr_t workshop_coop_dispatch_enter(
    uint8_t*, recomp_context* ctx, uint32_t function, uint32_t actor, uint32_t argument) {
    if (!enabled) return 0;
    owners.push_back(owner); owner = actor;
    events.push_back({function, actor, argument, -1});
    // Deliberate helper register clobbers must not leak into the callee.
    ctx->r4 = 123; ctx->r5 = 456; ctx->f0.u64 = 0;
    return owners.size();
}
extern "C" void workshop_coop_dispatch_leave(
    uint8_t*, recomp_context* ctx, uint32_t function, uint32_t actor, uint32_t argument,
    uintptr_t ticket, int completed) noexcept {
    if (owners.empty() || ticket != owners.size()) std::terminate();
    owner = owners.back(); owners.pop_back();
    events.push_back({function, actor, argument, completed});
    ctx->r2 = 999; ctx->r29 = 0; ctx->r31 = 0; ctx->f0.u64 = 0;
}

extern "C" void func_80022448(uint8_t*, recomp_context*);
static void original(uint8_t* rdram, recomp_context* ctx, uint32_t address) {
    ++originals;
    const auto actor = uint32_t(ctx->r4), argument = uint32_t(ctx->r5);
    need(actor != 123 && argument != 456, "Enter hook corrupted guest arguments");
    if (enabled) need(owner == actor, "Original ran outside its owner scope");
    if (mode == 1 && address == 0x80043450) {
        ctx->r4 = 0x800CB000; ctx->r5 = 0xABC;
        func_80022448(rdram, ctx);
        need(owner == actor, "Nested callback did not restore outer owner");
    }
    ctx->r4 = 0; ctx->r5 = 0; // Exit must use captured arguments, not these.
    ctx->r2 = address; ctx->r29 = 0x803FFF00; ctx->r31 = 0x80012345;
    ctx->f0.u64 = 0x1122334455667788ULL;
    if (mode == 2) throw std::runtime_error("native-test-exception");
    // This return is inside the original, not a rewritten return-site hook.
    return;
}
#define STUB(address) \
    extern "C" void func_##address(uint8_t*, recomp_context*); \
    extern "C" void coop_original_func_##address(uint8_t* rdram, recomp_context* ctx) { \
        original(rdram, ctx, 0x##address##u); \
    }
STUB(8001E094) STUB(8001E20C) STUB(80022448) STUB(800233B4) STUB(8002387C)
STUB(80039F48) STUB(8003FFD8) STUB(80043450) STUB(800434EC) STUB(800439F4)
STUB(80043E10) STUB(80043E88) STUB(80045B64) STUB(80048A30) STUB(80048AE0)
STUB(800675F0)
#undef STUB

int main() {
    try {
        std::array<recomp_func_t*, 16> functions = {func_8001E094,func_8001E20C,
            func_80022448,func_800233B4,func_8002387C,func_80039F48,func_8003FFD8,
            func_80043450,func_800434EC,func_800439F4,func_80043E10,func_80043E88,
            func_80045B64,func_80048A30,func_80048AE0,func_800675F0};
        uint8_t memory[4]{}; recomp_context ctx{};
        for (auto function : functions) {
            ctx.r4 = 0x800CA000; ctx.r5 = 0x800C9000;
            function(memory, &ctx);
            const auto& begin = events[events.size()-2]; const auto& end = events.back();
            need(begin.complete == -1 && end.complete == 1, "Before/after imbalance");
            need(end.actor == 0x800CA000 && end.argument == 0x800C9000 && end.function == begin.function,
                 "Captured identity changed before exit");
            need(uint32_t(ctx.r2) == end.function && ctx.r29 == 0x803FFF00 && ctx.r31 == 0x80012345 &&
                 ctx.f0.u64 == 0x1122334455667788ULL, "After hook corrupted native results");
            need(owner == 7 && owners.empty(), "Owner was not restored after ordinary return");
        }
        need(originals == 16 && events.size() == 32, "A public wrapper skipped or repeated its original");
        mode = 1; ctx.r4 = 0x800CA000; ctx.r5 = 55;
        func_80043450(memory, &ctx);
        need(originals == 18 && events.size() == 36 && owner == 7 && owners.empty(), "Nested dispatch imbalance");
        need(events[33].actor == 0x800CB000 && events[34].actor == 0x800CB000 &&
             events[35].actor == 0x800CA000, "Nested exit did not use its own captured actor");
        mode = 2; ctx.r4 = 0x800CA000; ctx.r5 = 88;
        bool caught = false;
        try { func_80043450(memory, &ctx); } catch (const std::runtime_error&) { caught = true; }
        need(caught && events.back().complete == 0 && owner == 7 && owners.empty(), "Exception bypassed RAII restoration");
        need(ctx.f0.u64 == 0x1122334455667788ULL && ctx.r31 == 0x80012345, "Exception exit lost native register state");
        mode = 0; enabled = false; const auto count = events.size();
        ctx.r4 = 0x800CA000; ctx.r5 = 88; func_80043450(memory, &ctx);
        need(events.size() == count && owner == 7 && uint32_t(ctx.r2) == 0x80043450,
             "Inactive adapter changed the native call");
        selected_player = 0; func_8004700C(memory, &ctx);
        need(ctx.r2 == gpr(int32_t(0x800CA000)), "Unscoped native player lookup was changed");
        selected_player = 0x800CB000; func_8004700C(memory, &ctx);
        need(ctx.r2 == gpr(int32_t(0x800CB000)), "Scoped registry lookup ignored its selected owner or lost sign extension");
        need(ctx.r3 == 0x12345678 && ctx.r4 == 0x4001 && ctx.r29 == 0x803FFF00 &&
             ctx.r31 == 0x80047048 && ctx.f0.u64 == 0x1020304050607080ULL,
             "Scoped player lookup discarded native register clobbers");
        need(lookup_calls == 2 && scoped_lookups == 2 && events.size() == count,
             "Special lookup skipped/repeated its original or entered an actor scope");
        std::cout << "Dispatch wrappers PASS: 17 real public symbols, scoped registry lookup, direct/function-pointer calls, "
                     "captured arguments, nested scopes, early return, native result registers, "
                     "exception cleanup and inactive bypass. Native gameplay not exercised.\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
