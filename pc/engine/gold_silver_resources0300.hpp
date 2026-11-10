#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

// Additive PC resources. The six-colour EEPROM extension is unrelated to these
// sixteen display slots and must keep its original six-byte codec.
namespace workshop::gold_silver_resources0300 {
inline constexpr unsigned display_count = 16;
inline constexpr uint16_t gold_selection_id = 0x4EF7;
inline constexpr uint16_t silver_selection_id = 0x4EF8;
inline constexpr size_t slab_begin = 0xFC0000;
inline constexpr size_t slab_end = 0xFD0000;
inline constexpr size_t metadata_rom = 0xFC8400;
inline constexpr uint32_t metadata_ram = 0x807FD200;
inline constexpr size_t metadata_size = 0xF0;
inline constexpr uint32_t selection_ids_ram = metadata_ram;
inline constexpr uint32_t selection_x_ram = metadata_ram + 0x20;
inline constexpr uint32_t selection_y_ram = metadata_ram + 0x60;
inline constexpr uint32_t rescue_offsets_ram = metadata_ram + 0xA0;

// Call after validating the input content and before appending payload/terrain.
// The 16 MiB source on disk is never written. An occupied reserved slab, an
// already applied pack or an incompatible resource layout is rejected.
std::vector<uint8_t> apply(std::vector<uint8_t> rom);
}
