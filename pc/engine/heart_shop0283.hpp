#pragma once
#include "heart_wallet0283.hpp"

namespace workshop::heart_shop0283 {
#if defined(WORKSHOP_NATIVE_COOP) && defined(WORKSHOP_SPECIAL_CONTENT) && defined(WORKSHOP_CONTENT_BUILD)
inline constexpr bool runtime_available=true;
#else
inline constexpr bool runtime_available=false;
#endif
// The launcher fixes the working directory to the active profile before UI or
// emulation starts. Both threads share this transaction-protected wallet.
inline heart_wallet0283::Wallet& wallet(){
    static heart_wallet0283::Wallet value;
    static const bool loaded=value.load(std::filesystem::current_path());
    (void)loaded;
    return value;
}
} // namespace workshop::heart_shop0283
