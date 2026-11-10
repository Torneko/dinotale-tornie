#pragma once
#include <cstdint>
namespace workshop::editor_objects {
constexpr bool special(unsigned id){return id>=0x4F30 && id<=0x4F39;}
constexpr bool native(unsigned id){return id==0x4692 || id==0x4693;}
constexpr unsigned color(unsigned id){return 6+id-0x4F30;}
}
