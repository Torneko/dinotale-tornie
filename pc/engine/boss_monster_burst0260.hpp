#pragma once
#include "boss_monster_aim0260.hpp"
#include "boss_egg_shape0260.hpp"
#include "boss_lips_corridor0260.hpp"
#include <span>

namespace workshop::boss_monster_burst0260 {
namespace n=boss_native;namespace m=boss_monster_aim0260;namespace e=boss_egg_shape0260;namespace d=boss_driver0260;
struct Root {uint32_t actor=0,private_data=0;uint64_t birth=0;};
struct Box {uint32_t actor=0,root=0;float x=0,y=0,z=0,hx=0,hy=0,hz=0,vx=0,vy=0;};
// Native3C10 passes these exact Z bits to3BB8 for the primary (mode0)4018.
struct Scene {std::array<Box,8> eyes{};unsigned count=0,param=0;float x=0,y=0,z=0,mx=0,my=0,burst_x=0,burst_y=0,burst_z=0;float burst_depth=std::bit_cast<float>(0x43FA07CFu);e::Shape egg{};m::Map map{};};
enum class Reason:unsigned {Unobserved,TooShort,DirectEggContact,OutsideBurst,Ready};
struct Result {bool observed=false,ready=false,passed=false;Reason reason=Reason::Unobserved;unsigned expiry=0,contact=0;float x=0,y=0;};
// Native151 is a3x2 paged map. Keep this domain local: the unrelated Lips
// corridor deliberately admits only its own2x1 arena.
inline std::optional<boss_lips_corridor0260::Result> corridor(uint8_t* ram,const m::Map& map,float x,float y,float tx,float ty,float hx,float hy){
 if(!ram||map.columns!=3||map.rows!=2||!d::point(x,y)||!d::point(tx,ty)||!std::isfinite(hx)||!std::isfinite(hy)||hx<=0||hy<=0||hx>64||hy>64||std::hypot(tx-x,ty-y)>1024)return {};
 const float lx=std::min(x,tx)-hx,rx=std::max(x,tx)+hx,ly=std::min(y,ty)-hy,ry=std::max(y,ty)+hy;
 if(lx<0||ly<0||rx>=768||ry>=512)return {};
 const int left=std::max(0,int(std::floor(lx))-1),right=int(std::floor(rx)),top=std::max(0,int(std::floor(ly))-1),bottom=int(std::floor(ry));
 const unsigned cells=unsigned(right-left+1)*unsigned(bottom-top+1);if(!cells||cells>768u*512u)return {};
 boss_lips_corridor0260::Result out;out.clear=true;
 for(int py=top;py<=bottom;++py)for(int px=left;px<=right;++px){
  if(!boss_lips_corridor0260::intersects(x,y,tx,ty,float(px)-hx,float(py)-hy,float(px+1)+hx,float(py+1)+hy))continue;
  const auto solid=map.occupied(ram,float(px),float(py));if(!solid)return {};++out.pixels_tested;
  if(*solid){out.clear=false;out.first_solid_x=float(px);out.first_solid_y=float(py);return out;}
 }
 return out;
}
inline std::optional<Scene> observe(std::span<const uint8_t> memory,uint32_t player,std::span<const Root> roots,const m::History& history,unsigned room,unsigned encounter,unsigned vi){
 if(memory.size()<e::required_capacity||roots.empty()||roots.size()>4)return {};auto* ram=const_cast<uint8_t*>(memory.data());auto* rdram=ram;
 const auto shape=e::observe_aimed(memory,player);const auto map=m::Map::read(ram);if(!shape||!map||!n::actor(ram,player,0x4001))return {};
 const auto data=n::word(ram,player+0x1F4);if(!n::range(data,0x12D0)||n::byte(ram,data+0xF4))return {};
 auto child=[&](uint32_t a,uint16_t kind){
  if(!n::actor(ram,a,kind)||n::word(ram,a+0x180)!=player)return false;
  const auto slot=MEM_HU(0x17E,n::base(a)),count=MEM_HU(0x184,n::base(player));if(slot>=20||!count||count>20||n::word(ram,player+0x188+4*slot)!=a)return false;
  unsigned matches=0;for(unsigned k=0;k<20;++k)matches+=n::word(ram,player+0x188+4*k)==a;return matches==1;
 };
 const auto primary=n::byte(ram,player+0x18);const auto reticle=n::word(ram,data+0x1F4);
 // BE44 creates4005/4003 then EF78 saves primary1 and enters3/BF88.
 // Missing4003 after a native allocation failure is only a spatial waiting
 // scene: the existing Bridge cannot evaluate or release without4003.
 if(primary!=1&&(primary!=3||n::word(ram,player+0x20)!=0x8003BF88||n::byte(ram,data)!=1||!child(n::word(ram,data+0x1F0),0x4005)))return {};
 if(reticle&&!child(reticle,0x4003))return {};
 const auto callback=n::word(ram,0x800F8D18);if(callback<0x80000BC0)return {};const auto overlay=callback-0x7C0;
 // Native3C10 is the actual egg-expiry callback;3A1C reads these three
 // parameter scales. AE4C selects the parameter from native colour and HP.
 if(!n::range(overlay,0xB10)||m::scalar(ram,overlay+0x908)!=.75f||m::scalar(ram,overlay+0x90C)!=1.5f||m::scalar(ram,overlay+0x910)!=2.25f)return {};
 const auto colour=n::word(ram,0x800F8DB0);if(colour>=8)return {};const auto hp=n::word(ram,0x800F8DB4+4*colour);if(hp>8)return {};
 Scene out;out.param=(colour==6||colour==7)?(hp>=8?2u:1u):(hp>=8?1u:0u);const float scale=m::scalar(ram,overlay+0x908+4*out.param);
 constexpr uint32_t row=e::table+(0x4018-0x4001)*24;const auto count=e::word(memory,row),profile=e::word(memory,row+4);
 if(count!=1||profile%4||!e::native_range(profile,0x1A))return {};
 for(unsigned axis=0;axis<3;++axis)if(e::half(memory,profile+2*axis)!=0||e::half(memory,profile+6+2*axis)!=(axis==2?20:24))return {};
 out.egg=*shape;out.map=*map;out.x=m::scalar(ram,player+0x84);out.y=m::scalar(ram,player+0x88);out.z=m::scalar(ram,player+0x8C)+m::scalar(ram,0x800B02E0);
 out.burst_x=24*scale;out.burst_y=24*scale;out.burst_z=20*scale;
 if(!d::point(out.x,out.y)||!std::isfinite(out.z)||m::scalar(ram,0x800B02E0)!=-.001f)return {};
 // The observed deferred405C8 path can move the parent and its relative4003
 // during eight VI. Permit only the real grounded415F rider or static ground;
 // test a conservative envelope of that current native motion, never a pose.
 auto speed=[&](uint32_t a,unsigned velocity,unsigned factor,unsigned extra){return m::scalar(ram,a+velocity)*m::scalar(ram,a+factor)+m::scalar(ram,a+extra);};
 out.mx=8*std::abs(speed(player,0xC4,0x100,0x10C));out.my=0;
 const auto parent=n::word(ram,player+0x180);if(parent){
  if(!n::actor(ram,parent,0x415F)||n::byte(ram,parent+0x17D)!=2||n::word(ram,parent+0x130)!=0x800483B8)return {};
  const auto slot=MEM_HU(0x17E,n::base(player)),children=MEM_HU(0x184,n::base(parent));if(slot>=20||!children||children>20||n::word(ram,parent+0x188+4*slot)!=player)return {};
  const float px=speed(parent,0xC4,0x100,0x10C),py=speed(parent,0xC8,0x104,0x110);if(!std::isfinite(px)||!std::isfinite(py)||std::abs(px)>1||std::abs(py)>1)return {};out.mx+=8*std::abs(px);out.my=8*std::abs(py);
 }
 if(!std::isfinite(out.mx)||out.mx>8)return {};
 for(const auto& root:roots){unsigned found=0;for(unsigned slot=0;slot<20;++slot){m::Eye eye;if(!m::observe_eye(ram,root.actor,root.private_data,root.birth,n::word(ram,root.actor+0x188+4*slot),eye))continue;
   const auto motion=history.observe(root.actor,root.private_data,root.birth,room,encounter,vi,eye);if(!motion||out.count==out.eyes.size()||!n::range(eye.definition,0x74))return {};
   const auto scale_ptr=n::word(ram,eye.definition+0x70);if(!n::range(scale_ptr,12))return {};Box b;b.actor=eye.actor;b.root=root.actor;b.x=eye.x;b.y=eye.y;b.z=m::scalar(ram,eye.actor+0x8C);b.vx=motion->vx;b.vy=motion->vy;
   for(unsigned axis=0;axis<3;++axis){if(MEM_H(4+2*axis,n::base(eye.definition))||MEM_H(10+2*axis,n::base(eye.definition))!=(axis==2?20:8)||m::scalar(ram,scale_ptr+4*axis)!=1.f)return {};}
   if(!std::isfinite(b.z))return {};b.hx=b.hy=8;b.hz=20;out.eyes[out.count++]=b;++found;
  }if(found!=2)return {};
 }
 return out.count?std::optional<Scene>(out):std::nullopt;
}
// Discrete AAB8 expiry is checked BEFORE the next A5EC movement. Reject any
// possible direct4002 contact through that point, including other roots' eyes.
// Require the fixed native4018 collider to overlap at BOTH first subsequent
// update possibilities (+1/+2); no scheduler latency or actual hit is claimed.
inline Result evaluate(const Scene& s,uint32_t target,float tx,float ty){
 Result out;if(!target||!s.count||s.count>s.eyes.size()||!d::point(tx,ty))return out;
 const float dx=tx-s.x,dy=ty-s.y,length=std::hypot(dx,dy);if(!std::isfinite(length)||length<8||length>600)return out;
 const float vx=6.6f*dx/length,vy=6.6f*dy/length;out.observed=true;out.reason=Reason::TooShort;
 unsigned expiry=0;for(;expiry<=96;++expiry)if(std::abs(s.x+vx*expiry-tx)<=std::max(std::abs(vx),1.f)&&std::abs(s.y+vy*expiry-ty)<=std::max(std::abs(vy),1.f))break;
 if(expiry>96)return out;out.expiry=expiry;out.x=s.x+vx*expiry;out.y=s.y+vy*expiry;
 for(unsigned delay=0;delay<=8;++delay)for(unsigned tick=0;tick<=expiry;++tick)for(unsigned i=0;i<s.count;++i){const auto& b=s.eyes[i];const float flight=float(tick),t=float(delay+tick);
  if(std::abs(s.x+vx*flight-(b.x+b.vx*t))<=s.egg.half_x+b.hx+s.mx&&std::abs(s.y+vy*flight-(b.y+b.vy*t))<=s.egg.half_y+b.hy+s.my&&std::abs(s.z-b.z)<=s.egg.half_z+b.hz){out.reason=Reason::DirectEggContact;out.passed=true;out.contact=tick;return out;}
 }
 out.reason=Reason::OutsideBurst;for(unsigned i=0;i<s.count;++i){const auto& b=s.eyes[i];if(b.root!=target)continue;bool both=true;
  for(unsigned launch_delay=0;launch_delay<=8;++launch_delay)for(unsigned delay:{1u,2u}){const float t=float(launch_delay+expiry+delay);both=both&&std::abs(out.x-(b.x+b.vx*t))+s.mx<s.burst_x+b.hx&&std::abs(out.y-(b.y+b.vy*t))+s.my<s.burst_y+b.hy&&std::abs(s.burst_depth-b.z)<s.burst_z+b.hz;}
  if(both){out.ready=true;out.reason=Reason::Ready;break;}
 }
 return out;
}
}
