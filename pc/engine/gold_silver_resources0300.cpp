#include "gold_silver_resources0300.hpp"
#include "terrain_data.hpp"
#include <bit>

namespace workshop::gold_silver_resources0300 {
namespace {
using namespace workshop::terrain;
constexpr size_t bank3_base = 0x526EC0;
constexpr size_t player_at = 0xFC0100, egg_at = 0xFC2100;
constexpr size_t egg_extra_at = 0xFC4100, transition_at = 0xFC6300;
constexpr size_t gold_info = 0xFC8300, silver_info = 0xFC830C;
constexpr size_t gold_object = 0xFC8320, silver_object = 0xFC8390;
constexpr uint32_t ptr(size_t at) { return 0x03000000u | uint32_t(at - bank3_base); }
struct Object { size_t a, row; uint32_t b; };
Object object(const Bytes& rom, unsigned id) {
    for (unsigned i = 0; i < 1582; ++i) {
        const size_t row = 0xA1C90 + i * 12;
        if (be16(rom, row) == id) return {bank(rom, be32(rom, row + 4)), row, be32(rom, row + 8)};
    }
    throw std::runtime_error("Missing metallic resource template");
}
void put(Bytes& rom, size_t at, const Bytes& bytes) {
    need(at >= slab_begin && at + bytes.size() <= slab_end, "Metallic slab overflow");
    std::copy(bytes.begin(), bytes.end(), rom.begin() + at);
}
void descriptor(Bytes& rom, size_t info, size_t at, size_t size) {
    word(rom, info, uint32_t(size)); word(rom, info + 4, uint32_t(size)); word(rom, info + 8, ptr(at));
}
bool body(uint16_t c) {
    const unsigned r = (c >> 11) & 31, g = (c >> 6) & 31, b = (c >> 1) & 31;
    return (c & 1) && g > r && g > b && g - std::min(r, b) >= 4;
}
Bytes recolor(const Bytes& original, bool silver, bool player) {
    need(original.size() == 512, "Invalid metallic template palette");
    Bytes result = original;
    for (unsigned i = 0; i < 256; ++i) {
        const uint16_t c = be16(original, i * 2);
        const unsigned r = (c >> 11) & 31, g = (c >> 6) & 31, b = (c >> 1) & 31;
        const unsigned v = std::max({r, g, b}), low = std::min({r, g, b});
        const bool boots = player && i >= 37 && i < 100 && (c & 1);
        const bool extra = player && ((i >= 5 && i <= 9) || i == 102 || i == 103);
        if (!boots && !extra && !body(c)) continue;
        unsigned nr, ng, nb;
        if (boots && silver) { nr = (v * 9 + 15) / 31; ng = (v * 13 + 15) / 31; nb = (v * 19 + 15) / 31; }
        else if (boots) { nr = (v * 19 + 15) / 31; ng = (low * 10 + 15) / 31; nb = (v * 8 + 15) / 31; }
        else if (silver) {
            nr = std::min(31u, (v * 23 + low * 5 + 15) / 31);
            ng = std::min(31u, (v * 25 + low * 5 + 15) / 31);
            nb = std::min(31u, (v * 27 + low * 4 + 15) / 31);
        } else { nr = v; ng = (v * 25 + 15) / 31; nb = std::min(v, (low * 2 + (v * 3 + 15) / 31 + 1) / 3); }
        half(result, i * 2, uint16_t(nr << 11 | ng << 6 | nb << 1 | (c & 1)));
    }
    return result;
}
Bytes append_metals(const Bytes& original, unsigned old_count, unsigned green, bool player) {
    need(original.size() == old_count * 512 && green < old_count, "Unexpected metallic palette count");
    Bytes result = original;
    const Bytes base(original.begin() + green * 512, original.begin() + (green + 1) * 512);
    for (bool silver : {false, true}) { const auto p = recolor(base, silver, player); result.insert(result.end(), p.begin(), p.end()); }
    return result;
}
}
std::vector<uint8_t> apply(std::vector<uint8_t> rom) {
    need(rom.size() == 0x1000000, "Metallic resources require the original 16 MiB content");
    need(be32(rom, 0xA79D4 + 12) == bank3_base, "Unexpected metallic resource bank");
    need(std::all_of(rom.begin() + slab_begin, rom.begin() + slab_end, [](uint8_t c) { return c == 0xFF; }), "Metallic resource slab is occupied");
    for (unsigned id : {gold_selection_id, silver_selection_id})
        for (unsigned i = 0; i < 1582; ++i) need(be16(rom, 0xA1C90 + i * 12) != id, "Metallic selection ID is occupied");
    need(std::all_of(rom.begin() + 0xF10088, rom.begin() + 0xF100A0, [](uint8_t c) { return c == 0; }), "Metallic resident lookup is occupied");
    need(be16(rom, 0xB47D4A) == 0xFFFF && be32(rom, 0xB47D4C) == 0, "Unexpected selection preload terminator");
    const auto player = object(rom, 0x4001), egg = object(rom, 0x4002), extra = object(rom, 0x4524);
    const auto castle = object(rom, 0x4656), rescue = object(rom, 0x4727), green = object(rom, 0x41F1);
    const size_t player_source = bank(rom, be32(rom, player.a + 0x28));
    need(player_source + 14 * 512 <= rom.size(), "Invalid player palette array");
    const Bytes player_old(rom.begin() + player_source, rom.begin() + player_source + 14 * 512);
    const auto player_new = append_metals(player_old, 14, 0, true);
    const auto egg_new = append_metals(block(rom, be32(rom, egg.a + 0x30)), 14, 0, false);
    const auto extra_new = append_metals(block(rom, be32(rom, extra.a + 0x30)), 15, 1, false);
    // The special-content lookup overrides 4006. Its clone, not the cartridge
    // object table's eight-colour original, is the active transition resource.
    size_t transition = 0;
    for (unsigned i = 0; i < 10; ++i) { const size_t row = 0xFB0000 + i * 12; if (be16(rom, row) == 0x4006) transition = bank(rom, be32(rom, row + 4)); }
    need(transition != 0, "Missing extended transition resource");
    const auto transition_new = append_metals(block(rom, be32(rom, transition + 0x30)), 14, 0, true);
    need(block(rom, be32(rom, castle.a + 0x30)) == player_old && block(rom, be32(rom, rescue.a + 0x30)) == player_old, "Unexpected castle/rescue palette prefix");
    need(block(rom, be32(rom, green.a + 0x30)) == Bytes(player_old.begin(), player_old.begin() + 512), "Unexpected selection palette template");
    put(rom, player_at, player_new); put(rom, egg_at, egg_new); put(rom, egg_extra_at, extra_new); put(rom, transition_at, transition_new);
    word(rom, player.a + 0x28, ptr(player_at));
    for (const auto& [a, at, size] : std::array<std::array<size_t, 3>, 5>{{{egg.a, egg_at, 0x2000}, {extra.a, egg_extra_at, 0x2200}, {castle.a, player_at, 0x2000}, {rescue.a, player_at, 0x2000}, {transition, transition_at, 0x2000}}})
        descriptor(rom, bank(rom, be32(rom, a + 0x30)), at, size);
    for (unsigned n = 0; n < 2; ++n) {
        const size_t a = n ? silver_object : gold_object, info = n ? silver_info : gold_info;
        const uint16_t id = n ? silver_selection_id : gold_selection_id;
        const Bytes copied(rom.begin() + green.a, rom.begin() + green.a + 0x64); put(rom, a, copied);
        half(rom, a + 0x18, id); word(rom, a + 0x30, ptr(info)); descriptor(rom, info, player_at + (14 + n) * 512, 512);
        const size_t row = 0xF10088 + n * 12; half(rom, row, id); half(rom, row + 2, 0); word(rom, row + 4, ptr(a)); word(rom, row + 8, green.b);
        half(rom, 0xB47D4A + n * 2, id);
    }
    half(rom, 0xB47D4E, 0xFFFF);
    constexpr std::array<float, 16> xs{70,115,160,205,250,70,115,160,205,250,92,137,182,227,47,272};
    constexpr std::array<float, 16> ys{110,110,110,110,110,145,145,145,145,145,180,180,180,180,180,180};
    for (unsigned i = 0; i < display_count; ++i) {
        half(rom, metadata_rom + i * 2, uint16_t(i < 8 ? 0x41F1 + i : 0x4EF1 + i - 8));
        word(rom, metadata_rom + 0x20 + i * 4, std::bit_cast<uint32_t>(xs[i]));
        word(rom, metadata_rom + 0x60 + i * 4, std::bit_cast<uint32_t>(ys[i]));
        word(rom, metadata_rom + 0xA0 + i * 4, i * 256);
    }
    word(rom, metadata_rom + 0xE0, 0); // Preserve the original table's terminal zero.
    std::fill(rom.begin() + metadata_rom + 0xE4, rom.begin() + metadata_rom + metadata_size, 0);
    constexpr std::array<uint8_t, 8> magic{'Y','S','G','S','0','3','0','0'};
    std::copy(magic.begin(), magic.end(), rom.begin() + slab_begin);
    word(rom, slab_begin + 8, display_count); word(rom, slab_begin + 12, uint32_t(metadata_rom)); word(rom, slab_begin + 16, metadata_ram); word(rom, slab_begin + 20, uint32_t(metadata_size));
    return rom;
}
}
