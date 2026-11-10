#pragma once
#include "recomp.h"
#include <cstdint>
#include <filesystem>
#include <string>

namespace workshop::hybrid_completion0290 {
inline constexpr uint32_t all_levels=0xFFFFFF;
struct View {
    uint32_t mask=0;
    unsigned count=0;
    bool readable=true,saved=true;
    bool legacy_checked=false,legacy_readable=true;
    uint32_t legacy_mask=0;
    std::string reason;
};
// A separate PC journal. Only the explicitly supplied Hybrid record sidecar
// may supply legacy credit; native EEPROM and merged achievements are unused.
void initialize(const std::filesystem::path& path="pc-hybrid-completion.json",
                const std::filesystem::path& legacy_records={});
void start_session(unsigned mode);
// Call on the normal manager frame fence. Reads native RAM/registers only.
// allowed must exclude custom content and synthetic result diagnostics.
void frame(uint8_t* rdram,recomp_context* context,bool allowed);
View view();
}
