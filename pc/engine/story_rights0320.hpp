#pragma once
#include "story_progress.hpp"
#include <array>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>

namespace workshop::story_rights0320 {
// A read-only projection of the canonical native EEPROM. This never changes
// names, checksums, records, continuation bytes, ownership journals or files.
struct View {
    bool present=false,readable=true;
    unsigned copy=2;
    uint32_t magic=0,story=0,trial=0;
    progress::State progression{0,255,{255,255,255,255},0,63};
    unsigned legacy_owned=0,bw_earned=0;
    bool continuing=false;
    uint8_t raw_flags=0;
    std::array<uint8_t,14> continuation{};
    std::string reason;
    bool trial_complete()const{return present&&readable&&trial==0xFFFFFF;}
};
// The native format contains exactly two1024-byte copies. A present malformed
// source fails closed; the first valid copy wins, matching the cartridge read.
View decode(std::span<const uint8_t> eeprom);
// Reads at most2049 bytes. Missing sources are distinguishable from malformed
// sources. Symlinks and non-regular files are rejected without opening them.
View read(const std::filesystem::path& path);
}
