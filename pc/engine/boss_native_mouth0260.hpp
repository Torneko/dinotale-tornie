#pragma once
#include "boss_native.hpp"
#include "boss_stick_aim0260.hpp"
#include "boss_tongue_geometry0260.hpp"
#include <array>
#include <bit>
namespace workshop::boss_native_mouth0260 {
// Extracted unchanged from the existing bridge; read-only native observation.
inline bool observe_native_mouth(uint8_t* rdram,uint32_t player,std::array<float,80>& xs,std::array<float,80>& ys,boss_stick_aim0260::Mouth& output){
 namespace n=boss_native;if(!rdram||!n::actor(rdram,player,0x4001)||n::word(rdram,0x800F8828)!=player)return false;
 const auto p=n::word(rdram,player+0x1F4);if(!n::range(p,0x1200)||n::word(rdram,player+0x138)>1)return false;
 auto fl=[&](uint32_t a){return std::bit_cast<float>(n::word(rdram,a));};boss_stick_aim0260::Mouth m;m.observed=true;m.right=n::word(rdram,player+0x138)!=0;m.scale_x=fl(player+0x6C);m.scale_y=fl(player+0x70);
 const float vertical=fl(player+0xC8);if(!std::isfinite(vertical))return false;
 m.posture=MEM_BU(0x111,n::base(p))?30:MEM_BU(0xF4,n::base(p))==0?0:MEM_BU(0x30,n::base(player))==0x22?10:vertical<0?20:10;
 for(unsigned i=0;i<80;++i){xs[i]=fl(0x8009ECD8+4*i);ys[i]=fl(0x8009EE18+4*i);}m.offsets_x=xs;m.offsets_y=ys;if(!m.valid())return false;output=m;return true;
}
// Raw159 survives native damage cancellation:3262C/3271C clear154/156
// and3D76C installsC9B0 on both actors, without clearing159. Only a
// proven native actor graph can classify a live extension or cancelled idle.
struct TongueRequest {bool observed=false,ready=false;unsigned phase=0;bool callback_pending=false,live=false,cancelled_idle=false,heading_active=false;float max_reach=0,heading_degrees=std::numeric_limits<float>::quiet_NaN();};
inline TongueRequest observe_ordinary_tongue(uint8_t* rdram,uint32_t player){
 namespace n=boss_native;TongueRequest out;
 if(!rdram||!n::actor(rdram,player,0x4001)||n::word(rdram,0x800F8828)!=player)return out;
 const auto p=n::word(rdram,player+0x1F4);if(!n::range(p,0x12F0))return out;
 out.phase=MEM_BU(0x159,n::base(p));out.callback_pending=n::word(rdram,p+0xC0)!=0;
 const auto body=n::word(rdram,p+0x160),tip=n::word(rdram,p+0x15C);
 auto child=[&](uint32_t parent,uint32_t wanted){for(unsigned i=0;i<20;++i)if(n::word(rdram,parent+0x188+4*i)==wanted)return true;return false;};
 auto part=[&](uint32_t a,uint16_t kind){
  if(!n::actor(rdram,a,kind)||MEM_BU(0x18,n::base(a))==0xFF||!n::range(n::word(rdram,a+0x1F4),0x14)||!n::range(n::word(rdram,a+0x168),0x50))return false;
  const auto metadata=n::word(rdram,a+0x174);
  //400B has no collision metadata in the original game;400C has a real one.
  if(!metadata)return kind==0x400B;
  return n::range(metadata,4)&&n::range(n::word(rdram,metadata),0x50);
 };
 if(!part(body,0x400B)||!part(tip,0x400C)||n::word(rdram,body+0x180)!=player||n::word(rdram,tip+0x180)!=body||!child(player,body)||!child(body,tip))return out;
 const auto reach=boss_tongue_geometry0260::max_centre_reach(std::bit_cast<float>(n::word(rdram,player+0x6C)),std::bit_cast<float>(n::word(rdram,body+0x78)),n::word(rdram,0x800F8944)!=0);
 const float heading=std::bit_cast<float>(n::word(rdram,p+0x174));if(!reach||!std::isfinite(heading)||heading< -90||heading>90)return out;
 out.max_reach=*reach;out.heading_degrees=heading;out.heading_active=n::word(rdram,player+0x160)==0x80025EA4;
 const auto body_update=n::word(rdram,body+0x20),tip_update=n::word(rdram,tip+0x20);
 const auto requested=MEM_BU(0x154,n::base(p)),started=MEM_BU(0x156,n::base(p));
 const bool live_callbacks=(body_update==0x8003D54C||body_update==0x8003D0B0)&&(tip_update==0x8003D61C||tip_update==0x8003D444);
 out.live=requested==1&&started==1&&live_callbacks&&out.phase>=1&&out.phase<=3;
 const bool idle=requested==0&&started==0&&body_update==0x8003C9B0&&tip_update==0x8003C9B0&&n::word(rdram,p+0x164)==0&&!out.callback_pending;
 out.cancelled_idle=idle&&(out.phase==1||out.phase==2);
 const bool initial_idle=idle&&out.phase==0;
 out.observed=out.live||out.cancelled_idle||initial_idle;
 const auto state=MEM_BU(0x18,n::base(player));
 // Native320C4 itself does not reject stale159. Known cancelled idle may
 // request a fresh edge after the controller releases its previous B.
 out.ready=(initial_idle||out.cancelled_idle)&&(state==1||state==2)&&MEM_BU(0x1F8,n::base(p))==0&&MEM_H(0x150,n::base(p))==0&&n::word(rdram,p+0x12EC)==0&&MEM_BU(0x111,n::base(p))==0;
 return out;
}
inline bool observe_native_tongue_ready(uint8_t* rdram,uint32_t player){const auto q=observe_ordinary_tongue(rdram,player);return q.observed&&q.ready;}

}
