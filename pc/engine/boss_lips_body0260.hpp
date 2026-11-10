#pragma once
#include "boss_lips_aim0260.hpp"
#include "boss_driver0260.hpp"
#include "boss_egg_shape0260.hpp"
#include <algorithm>
namespace workshop::boss_lips_body0260 {
namespace n=boss_native;
struct Box {float x=0,y=0,z=0,hx=0,hy=0,hz=0;};
struct Scene {uint32_t root=0,weak=0;Box body{},eye{};};
enum class Reason:unsigned { Unobserved,NoLateralContact,BodyFirst,WeakFirst };
struct Result {bool clear=false;Reason reason=Reason::Unobserved;unsigned weak_step=0,body_step=0,side=0;};
inline std::optional<Box> read_box(uint8_t* rdram,uint32_t actor,uint32_t definition){
 if(!rdram||!n::range(actor,0x1FC)||!n::range(definition,0x74))return {};
 const auto scale=n::word(rdram,definition+0x70);if(!n::range(scale,12))return {};Box b;float* centers[]={&b.x,&b.y,&b.z};float* halves[]={&b.hx,&b.hy,&b.hz};for(unsigned i=0;i<3;++i){const float s=std::bit_cast<float>(n::word(rdram,scale+4*i)),world=std::bit_cast<float>(n::word(rdram,actor+0x84+4*i));const int e=MEM_H(10+2*i,n::base(definition)),o=MEM_H(4+2*i,n::base(definition));if(!std::isfinite(s)||s<=0||s>4||!std::isfinite(world)||std::abs(world)>100000||e<=0||e>256||std::abs(o)>256)return {};*centers[i]=world+float(o)*s;*halves[i]=float(e)*s;}return b;
}
// Read the same offsets, signed extents and scale pointer as6009C/60D90.
// The weak collider is a legitimate phase0 child; the bound root is phase2.
inline std::optional<Scene> observe(uint8_t* ram,uint32_t root,uint32_t data,uint64_t birth){
 auto* rdram=ram;
 const auto weak=boss_lips_aim0260::observe(ram,root,data,birth);if(!weak||n::word(ram,root+0x180))return {};
 const auto resource=n::word(ram,root+0x168),meta=n::word(ram,root+0x174);if(!n::range(resource,0x44)||!n::range(meta,4))return {};
 const auto factory=n::word(ram,resource+0x40),def=n::word(ram,meta);if(!n::range(def,0x74)||n::word(ram,def+0x28)!=factory+0x50||n::word(ram,def+4)||MEM_H(8,n::base(def))||MEM_H(10,n::base(def))!=25||MEM_H(12,n::base(def))!=45||MEM_H(14,n::base(def))!=10)return {};
 const auto body=read_box(ram,root,def),eye=read_box(ram,weak->actor,weak->definition);if(!body||!eye)return {};return Scene{root,weak->actor,*body,*eye};
}
// Object collision is appropriate for avoiding a horizontal walk INTO Bongo.
// Jump ceiling clearance uses the independent native terrain probes instead.
inline bool walk_clear(uint8_t* ram,const Scene& s,uint32_t player,float target_x){
 if(!ram||!n::actor(ram,player,0x4001)||n::word(ram,0x800F8828)!=player||!std::isfinite(target_x))return false;
 const auto meta=n::word(ram,player+0x174);if(!n::range(meta,4))return false;const auto p=read_box(ram,player,n::word(ram,meta));if(!p)return false;
 const float x=std::bit_cast<float>(n::word(ram,player+0x84)),end=p->x+target_x-x;
 if(target_x==x||std::abs(p->y-s.body.y)>p->hy+s.body.hy||std::abs(p->z-s.body.z)>p->hz+s.body.hz)return true;
 const float distance=p->x-s.body.x,radius=p->hx+s.body.hx;
 if(std::abs(distance)<=radius)return (end-s.body.x)*distance>0&&std::abs(end-s.body.x)>std::abs(distance);
 return std::max(p->x,end)<s.body.x-radius||std::min(p->x,end)>s.body.x+radius;
}
// Input planning only: one iteration is one nativeA5EC movement update.
// Conservatively reject simultaneous body/eye overlaps. The actual callback
// order also depends on native queues, contact budgets and metadata cooldowns.
// D44 requires62AEC1/2: horizontal penetration must be STRICTLY smaller.
inline Result ray(const Scene& s,const boss_egg_shape0260::Shape& egg,float x,float y,float z,float vx,float vy,float weak_vx,float body_vx,float body_vy,unsigned limit=96){
 Result out;auto valid=[](const Box& b){return boss_driver0260::point(b.x,b.y)&&std::isfinite(b.z)&&std::isfinite(b.hx+b.hy+b.hz)&&b.hx>0&&b.hy>0&&b.hz>0&&b.hx<=1024&&b.hy<=1024&&b.hz<=1024;};if(!s.root||!s.weak||!valid(s.body)||!valid(s.eye)||!boss_driver0260::point(x,y)||!std::isfinite(z)||!std::isfinite(vx)||!std::isfinite(vy)||!std::isfinite(weak_vx)||!std::isfinite(body_vx)||!std::isfinite(body_vy)||std::abs(weak_vx)>4||std::abs(body_vx)>4||std::abs(body_vy)>4||egg.half_x<=0||egg.half_y<=0||egg.half_z<=0||!std::isfinite(egg.half_x+egg.half_y+egg.half_z)||limit>96)return out;
 out.reason=Reason::NoLateralContact;
 for(unsigned k=0;k<=limit;++k){const float t=float(k),px=x+vx*t,py=y+vy*t;
  const float bx=s.body.x+body_vx*t,by=s.body.y+body_vy*t;
  if(std::abs(px-bx)<=s.body.hx+egg.half_x&&std::abs(py-by)<=s.body.hy+egg.half_y&&std::abs(z-s.body.z)<=s.body.hz+egg.half_z){out.reason=Reason::BodyFirst;out.body_step=k;return out;}
  const float dx=px-(s.eye.x+weak_vx*t),dy=py-(s.eye.y+body_vy*t),ox=s.eye.hx+egg.half_x-std::abs(dx),oy=s.eye.hy+egg.half_y-std::abs(dy);
  if(ox>=0&&oy>=0&&std::abs(z-s.eye.z)<=s.eye.hz+egg.half_z&&ox<oy){out.clear=true;out.reason=Reason::WeakFirst;out.weak_step=k;out.side=dx>=0?1:2;return out;}
 }
 return out;
}
inline Result toward(const Scene& s,const boss_egg_shape0260::Shape& egg,float x,float y,float z,float tx,float ty,float weak_vx,float body_vx,float body_vy){
 const float distance=std::hypot(tx-x,ty-y);if(!std::isfinite(distance)||distance<1||distance>600)return {};
 return ray(s,egg,x,y,z,6.6f*(tx-x)/distance,6.6f*(ty-y)/distance,weak_vx,body_vx,body_vy,std::min(96u,unsigned(std::ceil(distance/6.6f))+2));
}
}
