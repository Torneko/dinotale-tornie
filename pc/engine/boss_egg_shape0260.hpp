#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <optional>
#include <span>

namespace workshop::boss_egg_shape0260 {
// PC-specific object metadata is intentionally separate from the 8MiB actor
// range. extend-object-storage.py relocates4096 independent24-byte rows here.
// Callers MUST supply actual buffer capacity. An ordinary8MiB dump fails before
// this table is read. Never increase the generic native actor range for it.
inline constexpr uint32_t ram_base=0x80000000,actor_end=0x80800000;
inline constexpr uint32_t table=0x80810000,table_end=0x80828000;
inline constexpr uint32_t egg_row=table+(0x4002-0x4001)*24,launch_constant=0x800B02D0;
inline constexpr size_t required_capacity=table_end-ram_base;
struct Shape {
 uint32_t row=egg_row,profile=0,count=0,player=0;
 std::array<int16_t,3> offsets{},extents{};
 std::array<float,3> player_scale{},projectile_scale{};
 float scale_constant=0,half_x=0,half_y=0,half_z=0;
};
inline bool native_range(uint32_t p,size_t bytes){return p>=0x80000400&&p<actor_end&&bytes<=actor_end-p;}
inline uint32_t word(std::span<const uint8_t> mem,uint32_t p){uint32_t v;std::memcpy(&v,mem.data()+size_t(p-ram_base),4);return v;}
inline int16_t half(std::span<const uint8_t> mem,uint32_t p){int16_t v;std::memcpy(&v,mem.data()+(size_t(p-ram_base)^2),2);return v;}
inline unsigned byte(std::span<const uint8_t> mem,uint32_t p){return mem[size_t(p-ram_base)^3];}
inline float real(std::span<const uint8_t> mem,uint32_t p){float v;std::memcpy(&v,mem.data()+size_t(p-ram_base),4);return v;}
// 5BA30 maps kind->kind-4001;5BB78 reads row+4 without loading.62518 copies
// profile[0..1A) to collision-definition+4. Thus profile+6/8/A are exactly
// definition+A/C/E used by600CC and60D90, and profile+0/2/4 are centre offsets.
//40370..3A8 scales the projectile byP1scale*RAM[800B02D0];52A74 binds def70
// to projectile+6C. No translated function or mutable native lookup is called.
inline std::optional<Shape> observe(std::span<const uint8_t> mem,uint32_t player){
 if(mem.size()<required_capacity||!mem.data()||player%4||!native_range(player,0x1FC))return {};
 if(word(mem,0x800F8828)!=player||word(mem,player)!=0x19971110||half(mem,player+12)!=0x4001||byte(mem,player+6)==3||byte(mem,player+0x17D)==3||byte(mem,player+0x18)==0xFF)return {};
 Shape out;out.player=player;out.count=word(mem,egg_row);out.profile=word(mem,egg_row+4);
 if(!out.count||out.count>4096||out.profile%4||!native_range(out.profile,size_t(out.count)*0x1A))return {};
 out.scale_constant=real(mem,launch_constant);
 if(!std::isfinite(out.scale_constant)||out.scale_constant<=0||out.scale_constant>4)return {};
 for(unsigned i=0;i<3;++i){
  out.offsets[i]=half(mem,out.profile+2*i);out.extents[i]=half(mem,out.profile+6+2*i);
  // The original4002 profile has zero native centre offsets. An altered centre
  // needs explicit trajectory support rather than an unproved centred sweep.
  if(out.offsets[i]!=0||out.extents[i]<=0||out.extents[i]>256)return {};
  out.player_scale[i]=real(mem,player+0x6C+4*i);
  if(!std::isfinite(out.player_scale[i])||out.player_scale[i]<=0||out.player_scale[i]>4)return {};
  out.projectile_scale[i]=out.player_scale[i]*out.scale_constant;
  const float extent=float(out.extents[i])*out.projectile_scale[i];
  if(!std::isfinite(extent)||extent<=0||extent>128)return {};
  (i==0?out.half_x:i==1?out.half_y:out.half_z)=extent;
 }
 return out;
}
// Ordinary aiming follows405C8, not4031C:40630/40644 copy4005+78
// into the newly created4002 scales. Before4005 exists,401D8/40230 copies
// the selected native FIFO4004 saved scale to4005. This is that path only.
inline std::optional<Shape> observe_aimed(std::span<const uint8_t> mem,uint32_t player){
 auto out=observe(mem,player);if(!out)return {};
 auto actor=[&](uint32_t a,uint16_t kind){return a%4==0&&native_range(a,0x1FC)&&word(mem,a)==0x19971110&&uint16_t(half(mem,a+12))==kind&&byte(mem,a+6)!=3&&byte(mem,a+0x17D)!=3;};
 const auto data=word(mem,player+0x1F4);if(!native_range(data,0x12D0))return {};const auto held=word(mem,data+0x1F0);uint32_t scale=0;
 if(held){
  if(!actor(held,0x4005)||word(mem,held+0x180)!=player)return {};
  const auto slot=uint16_t(half(mem,held+0x17E)),count=uint16_t(half(mem,player+0x184));if(slot>=20||count==0||count>20||word(mem,player+0x188+4*slot)!=held)return {};
  unsigned matches=0;for(unsigned k=0;k<20;++k)matches+=word(mem,player+0x188+4*k)==held;if(matches!=1)return {};scale=held+0x78;
 }else{
  const unsigned count=byte(mem,data+0x12C8),head=uint16_t(half(mem,data+0x12BE));if(count==0||count>6||head>=8)return {};
  const auto reserve=word(mem,data+0x11FC+head*24);if(!actor(reserve,0x4004)||word(mem,reserve+0x180)!=player)return {};scale=reserve+0x78;
 }
 for(unsigned i=0;i<3;++i){const float s=real(mem,scale+4*i);if(!std::isfinite(s)||s<=0||s>4)return {};out->projectile_scale[i]=s;const float extent=float(out->extents[i])*s;if(!std::isfinite(extent)||extent<=0||extent>128)return {};(i==0?out->half_x:i==1?out->half_y:out->half_z)=extent;}
 return out;
}
} // namespace workshop::boss_egg_shape0260
