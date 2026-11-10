#pragma once
#include <cstdint>
namespace workshop::special_bubbles {
constexpr bool special(unsigned id){return id>=0x4F20 && id<0x4F26;}
constexpr bool native(unsigned id){return (id>=0x41D1 && id<=0x41D5) || (id>=0x452A && id<=0x452E) || (id>=0x468D && id<=0x4691) || id==0x46A8 || id==0x46A9;}
}
