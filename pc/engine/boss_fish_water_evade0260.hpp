#pragma once
#include "boss_fish_evade0260.hpp"

namespace workshop::boss_fish_water_evade0260 {
namespace n=boss_native;namespace m=boss_monster_aim0260;namespace boxes=boss_fish_explosion0260;
struct Identity {uint32_t actor=0,resource=0,private_data=0;};
struct Threat {
 bool observed=false,imminent=false,head_clear=false;uint32_t actor=0;unsigned step=0;
 Identity identity{};bool previous_valid=false;
};
struct Water {Identity identity{};boxes::Box box{};std::array<float,3> velocity{};};
inline bool same(const Identity& a,const Identity& b){return a.actor==b.actor&&a.resource==b.resource&&a.private_data==b.private_data;}
// Native8073C160 returns its public initializer8073C7E4. The4632 collision
// callback8073C5D4 calls2F96C with table[-3,+3],Y=-3, then latches private8.
inline std::optional<Water> water(uint8_t* rdram,uint32_t actor){
 if(!rdram||!n::actor(rdram,actor,0x4632)||n::byte(rdram,actor+0x17D)!=2||n::byte(rdram,actor+0x18)!=1||n::byte(rdram,actor+0x30)||n::byte(rdram,actor+0x165)||n::word(rdram,actor+0x180)||n::word(rdram,actor+0x130)!=0x800483B8)return {};
 const auto resource=n::word(rdram,actor+0x168),data=n::word(rdram,actor+0x1F4),metadata=n::word(rdram,actor+0x174);
 if(!m::pointer(resource,0x48)||!m::pointer(data,0xE0)||!m::pointer(metadata,4)||n::word(rdram,data+8))return {};
 const auto factory=n::word(rdram,resource+0x40),definition=n::word(rdram,metadata);
 if(!m::pointer(factory,0xF90)||!m::pointer(definition,0x7C)||MEM_HU(0x3C,n::base(resource))!=0x4632||n::word(rdram,resource+0x44)!=factory+0x684||n::word(rdram,actor+0x20)!=factory+0x58C)return {};
 // Attest the actual relocated factory getter, not another overlay which
 // happens to contain the same callback offset. All reads remain in8MiB.
 const auto hi=n::word(rdram,factory),low=n::word(rdram,factory+4);
 if((hi&0xFFFF0000)!=0x3C020000||(low&0xFFFF0000)!=0x24420000||n::word(rdram,factory+8)!=0x03E00008||n::word(rdram,factory+12)!=0xAFA40000)return {};
 const auto initializer=uint32_t((hi&0xFFFF)<<16)+uint32_t(int32_t(int16_t(low&0xFFFF)));
 if(initializer!=factory+0x684||m::scalar(rdram,factory+0xF70)!=-3||m::scalar(rdram,factory+0xF74)!=3)return {};
 if(!n::byte(rdram,definition+0x74)||n::word(rdram,definition+0x70)!=actor+0x6C||n::word(rdram,definition+0x24)!=factory+0x474||n::word(rdram,definition+0x28)!=factory+0x474||MEM_HU(0x16,n::base(definition))!=0x8707||MEM_HU(0x1C,n::base(definition))!=4)return {};
 Water out{{actor,resource,data},{actor,metadata,definition,actor+0x6C},{}};
 for(unsigned i=0;i<3;++i){
  if(MEM_H(4+2*i,n::base(definition))||MEM_H(10+2*i,n::base(definition))!=(i==2?10:12))return {};
  const float scale=m::scalar(rdram,actor+0x6C+4*i),position=m::scalar(rdram,actor+0x84+4*i);
  const float speed=m::scalar(rdram,actor+0xC4+4*i),multiplier=m::scalar(rdram,actor+0x100+4*i),external=m::scalar(rdram,actor+0x10C+4*i);
  if(!std::isfinite(scale)||scale<=0||scale>4||!std::isfinite(position)||!std::isfinite(speed)||!std::isfinite(multiplier)||multiplier<=0||multiplier>4||!std::isfinite(external))return {};
  out.box.center[i]=position;out.box.half[i]=float(i==2?10:12)*scale;out.velocity[i]=speed*multiplier+external;
  if(!std::isfinite(out.velocity[i])||std::abs(out.velocity[i])>16)return {};
 }
 return out;
}
// A bounded constant-motion sweep is only a risk estimate for an ordinary A
// gesture. It does not simulate future callbacks, gravity, landing or damage.
inline std::optional<unsigned> risk(const boxes::Box& person,const std::array<float,3>& player_velocity,const Water& projectile){
 float entry=0,leave=16;
 for(unsigned i=0;i<3;++i){
  const float distance=projectile.box.center[i]-person.center[i],extent=projectile.box.half[i]+person.half[i],relative=projectile.velocity[i]-player_velocity[i];
  if(!std::isfinite(distance)||!std::isfinite(extent)||!std::isfinite(relative)||extent<=0)return {};
  if(relative==0){if(std::abs(distance)>extent)return {};continue;}
  const float a=(-extent-distance)/relative,b=(extent-distance)/relative;
  entry=std::max(entry,std::min(a,b));leave=std::min(leave,std::max(a,b));if(entry>leave)return {};
 }
 if(!std::isfinite(entry)||entry<0||entry>16)return {};return unsigned(std::ceil(entry));
}
inline Threat observe(uint8_t* ram,uint32_t player,unsigned room,unsigned encounter,const Identity* previous=nullptr){
 auto* rdram=ram;Threat out;
 if(!ram||encounter>=9||room!=167+encounter||!n::actor(ram,player,0x4001)||n::word(ram,0x800F8828)!=player||n::byte(ram,player+0x17D)!=2||n::byte(ram,player+0x165)||n::word(ram,player+0x180)||n::word(ram,player+0x130)!=0x800233B4)return out;
 const auto primary=n::byte(ram,player+0x18);if(primary!=1&&primary!=3&&primary!=4)return out;
 const auto data=n::word(ram,player+0x1F4),metadata=n::word(ram,player+0x174);if(!m::pointer(data,0x12D0)||!m::pointer(metadata,0x18)||n::word(ram,metadata+4))return out;
 const auto definition=n::word(ram,metadata);if(!m::pointer(definition,0x80)||!n::byte(ram,definition+0x74)||n::word(ram,definition+0x70)!=data+8)return out;
 const auto xs=n::word(ram,definition+0x30),ys=n::word(ram,definition+0x34);if(!m::pointer(xs,48)||!m::pointer(ys,48))return out;
 boxes::Box person;std::array<float,3> velocity{};
 for(unsigned i=0;i<3;++i){
  if(MEM_H(4+2*i,n::base(definition))||MEM_H(10+2*i,n::base(definition))!=(i==0?14:i==1?19:20))return out;
  const float scale=m::scalar(ram,data+8+4*i),position=m::scalar(ram,player+0x84+4*i),old=m::scalar(ram,player+0xA8+4*i);
  if(!std::isfinite(scale)||scale<=0||scale>4||!std::isfinite(position)||!std::isfinite(old))return out;
  person.center[i]=position;person.half[i]=float(i==0?14:i==1?19:20)*scale;
  // Native47C38 preserves the previous WORLD pose in A8/AC/B0. Use that
  // last resolved displacement, not a guessed .7/1.0 raw-velocity multiplier.
  velocity[i]=position-old;if(!std::isfinite(velocity[i])||std::abs(velocity[i])>16)return out;
 }
 const auto map=m::Map::read(ram);if(!map||map->columns!=3||map->rows!=2)return out;
 // Same native head probes1/2 and48px clearance as the frozen A3D9 reader.
 out.head_clear=(n::word(ram,metadata+8)&0xF000)==0;
 for(unsigned i:{1u,2u}){const float dx=m::scalar(ram,xs+4*i),dy=m::scalar(ram,ys+4*i);if(!std::isfinite(dx)||!std::isfinite(dy)||std::abs(dx)>16||dy>=0||dy< -32)return Threat{};
  for(unsigned step=0;step<=48&&out.head_clear;++step){const auto solid=map->occupied(ram,person.center[0]+dx,person.center[1]+dy-float(step));if(!solid||*solid)out.head_clear=false;}
 }
 constexpr uint32_t queue=0x800F8814;const unsigned capacity=MEM_HU(8,n::base(queue));const auto table=n::word(ram,queue+0x10);
 if(!capacity||capacity>150||!m::pointer(table,capacity*4))return Threat{};
 unsigned earliest=17;
 for(unsigned i=0;i<capacity;++i){const auto actor=n::word(ram,table+4*i);if(!n::range(actor,0x1FC)||n::word(ram,actor)!=0x19971110||MEM_HU(12,n::base(actor))!=0x4632||n::byte(ram,actor+0x17D)!=2||n::byte(ram,actor+0x18)==0xFF)continue;
  const auto p=n::word(ram,actor+0x1F4);if(!m::pointer(p,0xE0))return Threat{};
  if(n::word(ram,p+8))continue; // This original projectile already pushed P1.
  const auto projectile=water(ram,actor);if(!projectile)return Threat{};
  if(previous&&same(*previous,projectile->identity))out.previous_valid=true;
  if(const auto step=risk(person,velocity,*projectile);step&&*step<earliest){earliest=*step;out.imminent=true;out.actor=actor;out.step=*step;out.identity=projectile->identity;}
 }
 out.observed=true;return out;
}
}
