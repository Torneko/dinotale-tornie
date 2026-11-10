#pragma once
#include "boss_native.hpp"
#include <array>
#include <cstdio>
#include <optional>
#include <string_view>

namespace workshop::boss_variant_action_probe0281 {
// One read-only failure record per explicitly configured encounter. No native
// function, input source, allocator or guest state is touched by this probe.
struct Actor {
 uint32_t address=0,private_data=0,resource=0,update=0,parent=0,x=0,y=0;
 unsigned kind=0,life=0,primary=0,sub=0,queue=0,children=0;
};
struct Pool {uint32_t header=0,storage=0;unsigned capacity=0,busy=0;bool valid=false;};
struct Failure {
 std::string_view reason;unsigned operation=0,encounter=0,member=0,vi=0;
 uint32_t function=0,stack=0,result=0;uint64_t birth=0,current_birth=0;
 Actor actor{},target{},boss{};std::array<Pool,2> pools{};
 bool enabled=false,invalidated=false;
};
inline std::optional<Failure> last;
inline void reset(){last.reset();}
inline Actor actor(uint8_t* rdram,uint32_t a){
 Actor out;out.address=a;if(!rdram||a%4||!boss_native::range(a,0x1FC)||boss_native::word(rdram,a)!=0x19971110)return out;
 const auto base=boss_native::base(a);out.kind=MEM_HU(12,base);out.life=MEM_BU(6,base);out.primary=MEM_BU(0x18,base);out.sub=MEM_BU(0x30,base);out.queue=MEM_BU(0x17D,base);out.children=MEM_HU(0x184,base);
 out.private_data=boss_native::word(rdram,a+0x1F4);out.resource=boss_native::word(rdram,a+0x168);out.update=boss_native::word(rdram,a+0x20);out.parent=boss_native::word(rdram,a+0x180);out.x=boss_native::word(rdram,a+0x84);out.y=boss_native::word(rdram,a+0x88);return out;
}
inline Pool pool(uint8_t* rdram,uint32_t header){
 Pool out;out.header=header;if(!rdram||header%4||!boss_native::range(header,12))return out;
 out.capacity=boss_native::word(rdram,header+4);out.storage=boss_native::word(rdram,header+8);
 if(!out.capacity||out.capacity>150||out.storage%4||!boss_native::range(out.storage,out.capacity*0x280))return out;
 out.valid=true;for(unsigned i=0;i<out.capacity;i++)out.busy+=MEM_BU(0,boss_native::base(out.storage+i*0x280))!=0;return out;
}
inline const char* operation(unsigned op){constexpr std::array<const char*,8> names{"Water","Jump","Retire","GenieFire","BirdSpawn","FireStart","FireStop","BirdInjury"};return op<names.size()?names[op]:"Unknown";}
inline bool record(uint8_t* r,const recomp_context* c,std::string_view reason,unsigned op,unsigned encounter,unsigned member,unsigned vi,uint32_t fn,uint32_t subject,uint32_t target,uint64_t birth,uint64_t current_birth,uint32_t factory,uint32_t boss,bool enabled,bool invalidated){
 if(last)return false;
 Failure f;f.reason=reason;f.operation=op;f.encounter=encounter;f.member=member;f.vi=vi;f.function=fn;f.stack=c?uint32_t(c->r29):0;f.result=c?uint32_t(c->r2):0;f.birth=birth;f.current_birth=current_birth;f.actor=actor(r,subject);f.target=actor(r,target);f.boss=actor(r,boss);f.enabled=enabled;f.invalidated=invalidated;
 if(r&&factory%4==0&&boss_native::range(factory,0x2A2C)){f.pools[0]=pool(r,factory+0x2A10);f.pools[1]=pool(r,factory+0x2A20);}last=f;
 std::fprintf(stderr,"[BossVariantAction] rejected reason=%.*s op=%s fn=%08X encounter=%u member=%u vi=%u birth=%llu currentBirth=%llu sp=%08X result=%08X enabled=%u invalidated=%u actor=%08X kind=%04X phase=%u/%u queue=%u private=%08X resource=%08X update=%08X parent=%08X children=%u xy=%08X,%08X target=%08X targetKind=%04X targetPhase=%u/%u targetQueue=%u factory=%08X poolYellow=%08X:%u/%u:%08X poolWounded=%08X:%u/%u:%08X boss=%08X bossPhase=%u/%u bossXY=%08X,%08X readonly=1\n",int(reason.size()),reason.data(),operation(op),fn,encounter,member,vi,(unsigned long long)birth,(unsigned long long)current_birth,f.stack,f.result,unsigned(enabled),unsigned(invalidated),f.actor.address,f.actor.kind,f.actor.primary,f.actor.sub,f.actor.queue,f.actor.private_data,f.actor.resource,f.actor.update,f.actor.parent,f.actor.children,f.actor.x,f.actor.y,f.target.address,f.target.kind,f.target.primary,f.target.sub,f.target.queue,factory,f.pools[0].header,f.pools[0].busy,f.pools[0].capacity,f.pools[0].storage,f.pools[1].header,f.pools[1].busy,f.pools[1].capacity,f.pools[1].storage,f.boss.address,f.boss.primary,f.boss.sub,f.boss.x,f.boss.y);
 return false;
}
}
