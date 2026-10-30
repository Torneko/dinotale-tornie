#pragma once
#include "fruit_mix.hpp"
#include <filesystem>
#include <optional>
#include <span>
namespace workshop::hybrid_mode {
inline constexpr unsigned mode=3,threshold=45,palette_base=0x807FC400;
constexpr bool entitled(unsigned trial,bool purchased){return trial==0xFFFFFF && purchased;}
void initialize(const std::filesystem::path& path="pc-hybrid-records.json");
bool readable();
bool saved();
void activate(bool value);
void custom_placements(std::span<const uint8_t> payload);
bool active();
void initialize_ram(uint8_t* rdram);
void enter(uint8_t* rdram,uint32_t metadata,uint32_t room);
void created(uint8_t* rdram,recomp_context* ctx,uint32_t room);
void paint(uint8_t* rdram,uint32_t actor);
void recolored(uint8_t* rdram,uint32_t actor);
std::optional<unsigned> enemy_variant(uint32_t actor);
unsigned lucky_type();
uint16_t lucky_object();
void records_loaded(uint8_t* rdram,uint32_t payload);
bool records_saving(uint8_t* rdram,uint32_t payload,uint32_t buffer);
}
