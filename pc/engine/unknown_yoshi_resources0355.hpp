#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
namespace workshop::unknown_yoshi_resources0355 {
inline constexpr uint16_t selection_id = 0x4EFB;
inline constexpr size_t slab_begin = 0xFDA000, slab_end = 0xFDAB00;
inline constexpr size_t object_at = 0xFDA000, dimensions_at = 0xFDA070;
inline constexpr size_t texture_info = 0xFDA080, palette_info = 0xFDA090;
inline constexpr size_t texture_at = 0xFDA100, palette_at = 0xFDA900;
inline constexpr size_t resident_row = 0xF100B8, preload_at = 0xB47D52;
inline constexpr unsigned width = 48, height = 42;
// One static native CI8 question-mark sprite. Root's selection builder chooses
// this resource only for permanently unknown colors; no progress is altered.
// Its selector behavior requires the dedicated neutral init/update admission.
// Apply after elemental_resources0355 and before appended terrain.
std::vector<uint8_t> apply(std::vector<uint8_t> private_rom);
}
