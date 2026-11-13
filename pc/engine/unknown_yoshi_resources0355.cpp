#include "unknown_yoshi_resources0355.hpp"
#include "elemental_resources0355.hpp"
#include "terrain_data.hpp"
#include <array>
namespace workshop::unknown_yoshi_resources0355 {
namespace {
using namespace workshop::terrain;
constexpr size_t bank3_base = 0x526EC0;
constexpr uint32_t pointer(size_t address) { return 0x03000000u | uint32_t(address - bank3_base); }
struct Object { size_t a; uint32_t behavior; };
Object object(const Bytes& rom, unsigned id) {
 for (unsigned i = 0; i < 1582; ++i) {
  const size_t row = 0xA1C90 + i * 12;
  if (be16(rom, row) == id) return {bank(rom, be32(rom, row + 4)), be32(rom, row + 8)};
 }
 throw std::runtime_error("Unknown Yoshi native template missing");
}
void descriptor(Bytes& rom, size_t info, size_t data, unsigned length) {
 word(rom, info, length); word(rom, info + 4, length); word(rom, info + 8, pointer(data));
}
}
std::vector<uint8_t> apply(std::vector<uint8_t> rom) {
 using namespace workshop::terrain;
 need(rom.size() == 0x1000000, "Unknown Yoshi resources must precede terrain");
 constexpr std::array<uint8_t,8> magic{'Y','S','E','L','0','3','5','5'};
 need(std::equal(magic.begin(), magic.end(), rom.begin() + elemental_resources0355::slab_begin), "Apply elemental resources before unknown Yoshi marker");
 need(be32(rom, 0xA79D4 + 12) == bank3_base, "Unexpected unknown Yoshi resource bank");
 need(std::all_of(rom.begin() + slab_begin, rom.begin() + slab_end, [](uint8_t c) { return c == 255; }), "Unknown Yoshi resource slab occupied");
 need(std::all_of(rom.begin() + resident_row, rom.begin() + resident_row + 12, [](uint8_t c) { return c == 0; }), "Unknown Yoshi resident row occupied");
 need(be16(rom, preload_at) == 0xFFFF && be32(rom, preload_at + 2) == 0, "Unknown Yoshi preload terminator changed");
 for (unsigned i = 0; i < 1582; ++i) need(be16(rom, 0xA1C90 + i * 12) != selection_id, "Unknown Yoshi selection ID occupied");
 const auto green = object(rom, 0x41F1), question = object(rom, 0x4684);
 const size_t dimensions = bank(rom, be32(rom, question.a + 0x20));
 need(be16(rom, dimensions + 4) == 32 && be16(rom, dimensions + 6) == 32, "Unexpected native question-block dimensions");
 const auto pixels = block(rom, be32(rom, question.a + 0x28)), colors = block(rom, be32(rom, question.a + 0x30));
 need(pixels.size() >= 32 * 32 && colors.size() >= 512, "Native question mark sprite missing");
 Bytes glyph(width * height, 0); unsigned opaque = 0;
 // Reuse only the original neutral question glyph inside the brown block.
 // The central crop excludes the four rivets; chromatic wood pixels are left
 // transparent. No Yoshi texture, outline, color or name enters this asset.
 for (unsigned y = 5; y <= 28; ++y) for (unsigned x = 6; x <= 25; ++x) {
  const auto color = be16(colors, unsigned(pixels[y * 32 + x]) * 2);
  const unsigned r = color >> 11, g = (color >> 6) & 31, b = (color >> 1) & 31;
  const auto lo = std::min({r,g,b}), hi = std::max({r,g,b});
  if ((color & 1) && lo >= 15 && hi - lo <= 5) {
   glyph[(y + 5) * width + x + 8] = uint8_t(1 + (r + g + b) / 3); ++opaque;
  }
 }
 need(opaque >= 100 && opaque <= 500, "Native question glyph extraction changed");
 const Bytes interior = glyph;
 for (unsigned y = 1; y + 1 < height; ++y) for (unsigned x = 1; x + 1 < width; ++x) {
  if (interior[y * width + x]) continue;
  for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx)
   if (interior[(unsigned(int(y) + dy)) * width + unsigned(int(x) + dx)]) glyph[y * width + x] = 4;
 }
 std::copy(rom.begin() + green.a, rom.begin() + green.a + 0x64, rom.begin() + object_at);
 half(rom, object_at + 0x18, selection_id);
 word(rom, object_at + 0x20, pointer(dimensions_at));
 word(rom, object_at + 0x28, pointer(texture_info));
 word(rom, object_at + 0x30, pointer(palette_info));
 for (unsigned i = 0; i < 6; ++i) half(rom, dimensions_at + i * 2, uint16_t(i % 2 ? height : width));
 descriptor(rom, texture_info, texture_at, width * height); descriptor(rom, palette_info, palette_at, 512);
 std::copy(glyph.begin(), glyph.end(), rom.begin() + texture_at);
 std::fill(rom.begin() + palette_at, rom.begin() + palette_at + 512, 0);
 for (unsigned v = 0; v < 32; ++v) half(rom, palette_at + (v + 1) * 2, uint16_t(v << 11 | v << 6 | v << 1 | 1));
 half(rom, resident_row, selection_id); half(rom, resident_row + 2, 0);
 word(rom, resident_row + 4, pointer(object_at)); word(rom, resident_row + 8, green.behavior);
 half(rom, preload_at, selection_id); half(rom, preload_at + 2, 0xFFFF);
 return rom;
}
}
