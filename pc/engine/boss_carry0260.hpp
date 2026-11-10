#pragma once
#include "boss_native.hpp"
#include <array>
#include <cstdint>
#include <string_view>

namespace workshop::boss_carry0260 {
// Native scene setup687B8 calls6D1CC, which resets level counters through
// CFC0 before calling519C0. The generated trace preserves both MIPS links.
// The campaign owns the player/destination/phase and a one-shot pending-reset
// gate; these helpers do not authorize a route or synthesize a collection.
inline constexpr uint32_t reset_caller=0x8006D228,hud_caller=0x8006884C;
inline constexpr uint32_t score_base=0x800F8950,count_address=0x800F89D0;
inline constexpr uint32_t fruits_base=0x800F89D4,backup_count_address=0x800F8A4C;
inline constexpr uint32_t backup_fruits_base=0x800F8A50,ending_address=0x800F8AC8,half_fruits_base=0x800F8AD4;
inline constexpr unsigned slots=30,score_words=10;
struct Payload {
 bool captured=false;
 std::array<uint32_t,score_words> scores{};
 uint32_t count=0,backup_count=0,ending=0;
 std::array<uint32_t,slots> fruits{},backup_fruits{};
 std::array<uint16_t,slots> half_fruits{};
 bool operator==(const Payload&)const=default;
};
inline bool reset_fence(std::string_view name,uint32_t caller){return name=="func_8006CFC0"&&caller==reset_caller;}
inline bool hud_fence(std::string_view name,uint32_t caller){return name=="func_800519C0"&&caller==hud_caller;}
// Vanilla4030..4034 maps to0..4. The six hybrid fruits map to6..11;
// 5 is the invincibility heart, which never enters the eaten-fruit ring.
inline bool fruit_type(uint32_t type){return type<=4||(type>=6&&type<=11);}
inline bool valid(const Payload& p){
 if(!p.captured||p.count>slots||p.backup_count>p.count||p.ending)return false;
 for(unsigned i=0;i<p.count;++i)if(!fruit_type(p.fruits[i])||p.half_fruits[i]!=p.fruits[i])return false;
 for(unsigned i=0;i<p.backup_count;++i)if(!fruit_type(p.backup_fruits[i]))return false;
 return true;
}
inline bool capture(uint8_t* rdram,Payload& out){
 if(!rdram)return false;Payload p;p.captured=true;
 for(unsigned i=0;i<score_words;++i)p.scores[i]=boss_native::word(rdram,score_base+4*i);
 p.count=boss_native::word(rdram,count_address);p.backup_count=boss_native::word(rdram,backup_count_address);p.ending=boss_native::word(rdram,ending_address);
 for(unsigned i=0;i<slots;++i){p.fruits[i]=boss_native::word(rdram,fruits_base+4*i);p.backup_fruits[i]=boss_native::word(rdram,backup_fruits_base+4*i);p.half_fruits[i]=uint16_t(MEM_HU(2*i,boss_native::base(half_fruits_base)));}
 if(!valid(p))return false;out=p;return true;
}
// Call only at the authorized one-shot HUD fence after the original CFC0 has
// returned. Validate the WHOLE payload before any write. Actor/HP/pose/queues,
// identity114, save indices and all other original reset effects stay native.
inline bool restore(uint8_t* rdram,const Payload& p){
 if(!rdram||!valid(p))return false;
 for(unsigned i=0;i<score_words;++i)MEM_W(4*i,boss_native::base(score_base))=p.scores[i];
 MEM_W(0,boss_native::base(count_address))=p.count;MEM_W(0,boss_native::base(backup_count_address))=p.backup_count;MEM_W(0,boss_native::base(ending_address))=p.ending;
 for(unsigned i=0;i<slots;++i){MEM_W(4*i,boss_native::base(fruits_base))=p.fruits[i];MEM_W(4*i,boss_native::base(backup_fruits_base))=p.backup_fruits[i];MEM_H(2*i,boss_native::base(half_fruits_base))=p.half_fruits[i];}
 return true;
}
inline bool matches(uint8_t* rdram,const Payload& expected){Payload actual;return valid(expected)&&capture(rdram,actual)&&actual==expected;}
} // namespace workshop::boss_carry0260
