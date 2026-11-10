#include "../gold_silver_resources0300.hpp"
#include "../terrain_data.hpp"
#include <bit>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <string>

namespace {
using namespace workshop::terrain;
using namespace workshop::gold_silver_resources0300;
unsigned checks = 0;
void check(bool value, const char* why) { ++checks; if (!value) throw std::runtime_error(why); }
size_t object(const Bytes& rom, unsigned id) {
    for (unsigned i = 0; i < 1582; ++i) if (be16(rom, 0xA1C90 + i * 12) == id) return bank(rom, be32(rom, 0xA1C94 + i * 12));
    throw std::runtime_error("Missing test resource");
}
void rejects(Bytes rom) { bool rejected = false; try { (void)apply(std::move(rom)); } catch (const std::runtime_error&) { rejected = true; } check(rejected, "Malformed/repeated pack accepted"); }
}
int main(int argc, char** argv) {
    try {
        if (argc != 3) throw std::runtime_error("Expected input and output content paths");
        std::ifstream in(argv[1], std::ios::binary); const Bytes source{std::istreambuf_iterator<char>(in), {}};
        check(source.size() == 0x1000000, "Missing input content"); const auto packed = apply(source);
        check(packed.size() == source.size(), "Content length changed");
        std::vector<bool> allowed(source.size(), false);
        auto permit = [&](size_t at, size_t size) { for (size_t i = 0; i < size; ++i) allowed.at(at + i) = true; };
        permit(slab_begin, slab_end - slab_begin); permit(0xF10088, 24); permit(0xB47D4A, 6);
        const size_t player = object(source, 0x4001); permit(player + 0x28, 4);
        const size_t old_at = bank(source, be32(source, player + 0x28));
        const size_t new_at = bank(packed, be32(packed, player + 0x28));
        check(new_at == 0xFC0100, "Player palette location changed");
        check(std::equal(source.begin() + old_at, source.begin() + old_at + 14 * 512, packed.begin() + new_at), "Old player palettes changed");
        for (unsigned id : {0x4002, 0x4524, 0x4656, 0x4727, 0x4006}) {
            size_t a = object(source, id);
            if (id == 0x4006) for (unsigned i = 0; i < 10; ++i) if (be16(source, 0xFB0000 + i * 12) == id) a = bank(source, be32(source, 0xFB0004 + i * 12));
            const uint32_t p = be32(source, a + 0x30); permit(bank(source, p), 12);
            const auto old_palette = block(source, p), next = block(packed, be32(packed, a + 0x30));
            check(next.size() == old_palette.size() + 1024, "Palette count did not grow by two");
            check(std::equal(old_palette.begin(), old_palette.end(), next.begin()), "Old resource palette prefix changed");
            const size_t base = id == 0x4524 ? 512 : 0;
            for (unsigned n = 0; n < 2; ++n) {
                unsigned changed = 0;
                for (unsigned i = 0; i < 256; ++i) {
                    const auto before = be16(old_palette, base + i * 2), after = be16(next, old_palette.size() + n * 512 + i * 2);
                    check((before & 1) == (after & 1), "Palette alpha changed"); changed += before != after;
                }
                check(changed > 10, "Metallic resource has no visible recolor");
            }
        }
        unsigned changed_bytes = 0;
        for (size_t i = 0; i < source.size(); ++i) if (source[i] != packed[i]) { ++changed_bytes; check(allowed[i], "ROM changed outside additive whitelist"); }
        check(changed_bytes > 30000, "Missing pack content");
        const size_t green = object(source, 0x41F1); const auto native_green = block(source, be32(source, green + 0x30));
        std::set<Bytes> distinct;
        for (unsigned c = 0; c < 16; ++c) distinct.emplace(packed.begin() + new_at + c * 512, packed.begin() + new_at + (c + 1) * 512);
        check(distinct.size() == 16, "A metallic color duplicates an existing palette");
        for (unsigned n = 0; n < 2; ++n) {
            const size_t row = 0xF10088 + n * 12; const uint16_t id = uint16_t(0x4EF7 + n);
            check(be16(packed, row) == id && be16(packed, row + 2) == 0, "Bad metal lookup ID");
            const size_t a = bank(packed, be32(packed, row + 4));
            check(be32(packed, row + 8) == be32(source, 0xA3A14), "Selection behavior is not the original green behavior");
            for (unsigned i = 0; i < 0x64; ++i) if (!(i >= 0x18 && i < 0x1A) && !(i >= 0x30 && i < 0x34)) check(packed[a + i] == source[green + i], "Selection template detail changed");
            const auto palette = block(packed, be32(packed, a + 0x30)); check(palette.size() == 512, "Selection palette size changed");
            check(std::equal(palette.begin(), palette.end(), packed.begin() + new_at + (14 + n) * 512), "Selection/player colors disagree");
            for (unsigned i = 0; i < 256; ++i) {
                const uint16_t before = be16(native_green, i * 2), after = be16(palette, i * 2);
                check((before & 1) == (after & 1), "Player alpha changed");
                const unsigned r = (before >> 11) & 31, g = (before >> 6) & 31, b = (before >> 1) & 31;
                const bool body = (before & 1) && g > r && g > b && g - std::min(r, b) >= 4;
                const bool extra = (i >= 5 && i <= 9) || i == 102 || i == 103;
                if (!(i >= 37 && i < 100) && !body && !extra) check(before == after, "Eyes/belly/mouth/soles changed");
            }
        }
        check(be16(packed, 0xB47D4E) == 0xFFFF, "Preload terminator missing");
        constexpr std::array<float, 14> xs{70,115,160,205,250,70,115,160,205,250,92,137,182,227};
        std::set<std::pair<float, float>> positions;
        for (unsigned i = 0; i < 16; ++i) {
            const auto id = be16(packed, metadata_rom + i * 2);
            const float x = std::bit_cast<float>(be32(packed, metadata_rom + 0x20 + i * 4)), y = std::bit_cast<float>(be32(packed, metadata_rom + 0x60 + i * 4));
            check(id == (i < 8 ? 0x41F1 + i : 0x4EF1 + i - 8), "Selection ID order changed");
            check(x >= 20 && x <= 300 && y >= 20 && y <= 220, "Selection exceeds 320x240 screen");
            check(positions.emplace(x, y).second, "Selection positions overlap");
            if (i < 14) { check(x == xs[i], "Old selection X changed"); check(y == (i < 5 ? 110 : i < 10 ? 145 : 180), "Old selection Y changed"); }
            check(be32(packed, metadata_rom + 0xA0 + i * 4) == i * 256, "Rescue offset changed");
        }
        check(be32(packed, metadata_rom + 0xE0) == 0, "Rescue terminal zero changed");
        rejects(packed); auto broken = source; broken.pop_back(); rejects(std::move(broken));
        broken = source; broken[slab_begin + 100] = 0; rejects(std::move(broken));
        broken = source; word(broken, 0xA79D4 + 12, 0); rejects(std::move(broken));
        broken = source; broken[0xF10088] = 1; rejects(std::move(broken));
        broken = source; half(broken, 0xB47D4A, 1); rejects(std::move(broken));
        std::ofstream output(argv[2], std::ios::binary); output.write(reinterpret_cast<const char*>(packed.data()), std::streamsize(packed.size())); check(bool(output), "Failed to write isolated fixture");
        std::cout << "{\"passed\":true,\"checks\":" << checks << ",\"changedBytes\":" << changed_bytes << ",\"gpuValidated\":false,\"sourceWritten\":false}\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
