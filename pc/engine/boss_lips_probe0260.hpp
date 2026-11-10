#pragma once
#include "boss_driver_bridge0260.hpp"
#include <cstdio>
namespace workshop::boss_lips_probe0260 {
namespace c=boss_campaign0250;namespace n=boss_native;
inline unsigned contact_lines=0;
// Bounded read-only collision journal for isolated input diagnostics.
inline void trace(uint8_t* ram,const recomp_context* context,std::string_view name,unsigned vi,bool diagnostic){
 if(!diagnostic||!ram||!context||contact_lines>=64||!c::active()||!c::driver||!c::session.snapshot().diagnostic||c::session.snapshot().phase!=boss_battle0250::Phase::Fighting||c::session.current().family!=boss_battle0250::Family::Lips||(name!="func_80557BA0"&&name!="func_80557D44"))return;
 const auto a=uint32_t(context->r4),egg=uint32_t(context->r5);if(!n::actor(ram,egg,0x4002)||!n::range(a,0x1FC))return;
 const auto root=name=="func_80557BA0"?a:n::word(ram,a+0x180);if(!n::actor(ram,root,0x40A3))return;
 bool bound=false;for(unsigned member=0;member<c::session.current().count;++member)bound|=c::bound[member]==root;if(!bound)return;
 const auto data=n::word(ram,root+0x1F4),ep=n::word(ram,egg+0x1F4);if(!n::range(data,0x34)||!n::range(ep,0x14))return;
 unsigned serial=0;for(const auto& shot:boss_driver_bridge0260::native_launch_traces)if(shot.egg==egg&&shot.private_data==ep&&shot.resource==n::word(ram,egg+0x168))serial=std::max(serial,shot.serial);
 const auto weak=n::word(ram,data+0x10);auto value=[&](uint32_t actor,unsigned offset){return std::bit_cast<float>(n::word(ram,actor+offset));};
 ++contact_lines;std::fprintf(stderr,"[BossLipsContact] vi=%u event=%.*s serial=%u root=%08X collider=%08X egg=%08X eggXY=%.5f,%.5f velocity=%.5f,%.5f goal=%.5f,%.5f marked=%u weak=%08X weakXY=%.5f,%.5f counter=%u lock=%u state=%u/%u arg2=%08X arg3=%08X nativeObservation=1\n",vi,int(name.size()),name.data(),serial,root,a,egg,value(egg,0x84),value(egg,0x88),value(egg,0xC4),value(egg,0xC8),value(ep,8),value(ep,12),n::byte(ram,ep+3),weak,n::actor(ram,weak,0x40A7)?value(weak,0x84):0.f,n::actor(ram,weak,0x40A7)?value(weak,0x88):0.f,n::word(ram,data+0x24),n::word(ram,data+0x30),n::byte(ram,root+0x18),n::byte(ram,root+0x30),uint32_t(context->r6),uint32_t(context->r7));
}
}
