#pragma once
#include "boss_variants0250.hpp"
#include "boss_fish_explosion0260.hpp"
#include <cstdio>
#include <unordered_map>
#include <unordered_set>
namespace workshop::boss_fish_probe0260 {
namespace v=boss_variants0250;namespace n=boss_native;namespace volume=boss_fish_explosion0260;
// Read-only, bounded diagnostics for the explicit fully provisioned Fish test.
// No collision, actor, health, input, progression or timing is changed.
inline unsigned purple_lines=0,purple_state=0xFFFF;inline bool green_seen=false;inline std::array<uint64_t,8> bird_birth{};inline unsigned bird_lines=0;
inline unsigned lines=0,generation=0;inline std::unordered_map<uint64_t,unsigned> observed;inline std::unordered_set<uint32_t> flights;inline std::unordered_map<uint32_t,unsigned> generations;inline std::unordered_map<uint64_t,unsigned> contacts;
inline void reset(){purple_lines=0;purple_state=0xFFFF;green_seen=false;bird_birth={};bird_lines=0;lines=generation=0;observed.clear();flights.clear();generations.clear();contacts.clear();}
inline bool scope(){const auto& s=v::snapshot();return s.enabled&&!s.invalidated&&s.encounter==4&&s.room==97;}
inline unsigned kind(uint8_t* ram,uint32_t a){auto* rdram=ram;return n::range(a,0x1FC)&&n::word(ram,a)==0x19971110?MEM_HU(12,n::base(a)):0;}
inline unsigned enabled(uint8_t* ram,uint32_t a){if(!n::range(a,0x1FC))return 99;const auto m=n::word(ram,a+0x174);if(!v::range(m,4))return 99;const auto d=n::word(ram,m);return v::range(d,0x75)?n::byte(ram,d+0x74):99;}
inline void trace(uint8_t* ram,const recomp_context* c,std::string_view name,unsigned vi,const v::Frame& frame){
 if(!ram||!c||!scope())return;
 if(purple_lines<48&&(name=="func_800483B8"||name=="func_805C9D10")){
  const auto actor=uint32_t(c->r4);
  for(unsigned member=0;member<v::encounter.count;++member){const auto& b=v::snapshot().members[member];
   if(!v::encounter.members[member].purple||b.actor!=actor||!b.birth||!b.initialized||!v::live(ram,member))continue;
   const unsigned current=(n::byte(ram,actor+0x18)<<8)|n::byte(ram,actor+0x30);
   const bool transition=name=="func_805C9D10"&&uint32_t(c->r5)==1&&uint32_t(c->r6)==1;
   if(current==purple_state&&!transition)continue;
   purple_state=current;++purple_lines;
   auto scalar=[&](unsigned off){return std::bit_cast<float>(n::word(ram,actor+off));};
   std::fprintf(stderr,"[BossPurpleState] vi=%u actor=%08X state=%u/%u xy=%.3f,%.3f local=%.3f,%.3f velocity=%.3f,%.3f xScale=%.3f xExtra=%.3f nativeWaterTransition=%u ra=%08X nativeObservation=1\n",vi,actor,current>>8,current&255,scalar(0x84),scalar(0x88),scalar(0x3C),scalar(0x40),scalar(0xC4),scalar(0xC8),scalar(0x100),scalar(0x10C),unsigned(transition),uint32_t(c->r31));
  }
 }
 if(lines>=128)return;
 if(name=="func_80048F84"){
  const auto axis=uint32_t(c->r4);
  if(!green_seen&&axis>=0xC4&&uint32_t(c->r5)==0){if(const auto g=v::green_frame(ram,axis-0xC4,frame);g&&boss_fish_green0260::admitted(ram,*g)&&!n::word(ram,g->actor+0xC4)&&!n::word(ram,g->actor+0xDC)&&!n::word(ram,g->actor+0xE8)){
   green_seen=true;std::fprintf(stderr,"[BossGreenMotion] vi=%u actor=%08X vx=0 targetX=0 accelerationX=0 vy=%.3f nativeAxis=1\n",vi,g->actor,std::bit_cast<float>(n::word(ram,g->actor+0xC8)));
  }}
  return;
 }
 if(name!="func_806F3A1C"&&name!="func_8003AAB8"&&name!="func_805C6900"&&name!="func_806F3498")return;
 const auto a=uint32_t(c->r4),other=uint32_t(c->r5);const auto k=kind(ram,a),ok=kind(ram,other);const auto it=v::pulses.find(a);
 if(name=="func_806F3A1C")generations[a]=++generation;
 if(name=="func_8003AAB8"&&!flights.insert(a).second)return;
 if(name=="func_806F3498"&&ok!=0x4511&&ok!=0x4631)return;
 if(name=="func_805C6900"&&ok!=0x4001&&ok!=0x4002&&ok!=0x4018)return;
 if(name=="func_805C6900"||name=="func_806F3498"){
  const uint64_t key=(uint64_t(a)<<32)|other;const unsigned birth=generations[name=="func_806F3498"?a:other];const auto seen=contacts.find(key);
  if(seen!=contacts.end()&&seen->second==birth)return;contacts[key]=birth;
 }
 ++lines;std::fprintf(stderr,"[BossFishProbe] event=%.*s vi=%u actor=%08X kind=%04X state=%u other=%08X otherKind=%04X otherState=%u a2=%08X a3=%08X primaryTracked=%u private=%08X colliderEnabled=%u\n",int(name.size()),name.data(),vi,a,k,k?n::byte(ram,a+0x18):255,other,ok,ok?n::byte(ram,other+0x18):255,uint32_t(c->r6),uint32_t(c->r7),unsigned(it!=v::pulses.end()&&it->second.primary_egg),k?n::word(ram,a+0x1F4):0,k?enabled(ram,a):99);
}
inline void frame(uint8_t* ram,unsigned vi){
 if(!ram||!scope())return;
 for(unsigned slot=0;slot<v::encounter.birds&&slot<bird_birth.size()&&bird_lines<12;++slot){const auto& bird=v::birds[slot];const auto k=kind(ram,bird.actor);if(!bird.actor||!bird.birth||bird_birth[slot]==bird.birth||(k!=0x4526&&k!=0x46DA))continue;
  bird_birth[slot]=bird.birth;++bird_lines;std::fprintf(stderr,"[BossBirdSpawn] vi=%u slot=%u actor=%08X x=%.1f y=%.1f z=%.2f respawnMs=%u maxAlive=%u nativeActor=1\n",vi,slot,bird.actor,std::bit_cast<float>(bird.position[0]),std::bit_cast<float>(bird.position[1]),std::bit_cast<float>(bird.position[2]),v::encounter.bird_respawn_ms,v::encounter.birds);
 }
 if(lines>=128)return;
 for(const auto& [pulse,p]:v::pulses){if(!p.primary_egg||!p.birth)continue;
  const auto a=volume::observe(ram,pulse,0x4018);const bool causal=v::pulse(ram,pulse);
  for(unsigned member=0;member<v::encounter.count&&lines<128;++member){const auto& b=v::snapshot().members[member];const auto fk=uint16_t(kind(ram,b.actor));const auto fish=volume::observe(ram,b.actor,fk);const auto overlap=volume::overlaps(ram,pulse,b.actor);
   const unsigned status=unsigned(causal)|(unsigned(a.has_value())<<1)|(unsigned(fish.has_value())<<2)|(unsigned(b.active)<<3)|(unsigned(b.initialized)<<4)|(unsigned(overlap.value_or(false))<<5);
   const auto key=p.birth*4+member;auto old=observed.find(key);if(old!=observed.end()&&old->second==status)continue;observed[key]=status;
   float gx=0,gy=0,gz=0;if(a&&fish){gx=std::abs(a->center[0]-fish->center[0])-a->half[0]-fish->half[0];gy=std::abs(a->center[1]-fish->center[1])-a->half[1]-fish->half[1];gz=std::abs(a->center[2]-fish->center[2])-a->half[2]-fish->half[2];}
   ++lines;std::fprintf(stderr,"[BossFishProbe] event=volume vi=%u burst=%08X birth=%llu causal=%u burstBox=%u enabled=%u member=%u fish=%08X kind=%04X initialized=%u active=%u fishBox=%u fishEnabled=%u phase=%u state=%u/%u remaining=%u passenger=%u overlap=%d gap=%.3f,%.3f,%.3f\n",vi,pulse,static_cast<unsigned long long>(p.birth),unsigned(causal),unsigned(a.has_value()),enabled(ram,pulse),member,b.actor,unsigned(fk),unsigned(b.initialized),unsigned(b.active),unsigned(fish.has_value()),enabled(ram,b.actor),fk?n::byte(ram,b.actor+0x17D):255,fk?n::byte(ram,b.actor+0x18):255,fk?n::byte(ram,b.actor+0x30):255,b.remaining,v::range(b.private_data,v::fish_private_size)?n::word(ram,b.private_data+0x40):0xFFFFFFFFu,overlap?(*overlap?1:0):-1,gx,gy,gz);
  }
 }
}
}
