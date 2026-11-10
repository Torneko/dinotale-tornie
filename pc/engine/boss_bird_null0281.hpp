#pragma once
#include "boss_native.hpp"
#include <array>
#include <optional>

namespace workshop::boss_bird_null0281 {
// Native1EDC may return NULL when its five-slot46DA provider is full.
//3444 then retires the original4526 through447C8. This receipt recognizes
//only that exact normal path; it never retires, respawns or rewards an actor.
struct Ticket {
 uint32_t actor=0,private_data=0,resource=0,drawing=0,collision=0,update=0,factory=0,stack=0;
 uint32_t manager=0,table=0,pool_manager=0,pool_table=0;
 std::array<uint8_t,12> pool_header{};
 std::array<uint8_t,5*0x280> records{};
 unsigned capacity=0,pool_capacity=0,life=0,sub=0;
 uint64_t birth=0;
};
inline uint32_t word(uint8_t* r,uint32_t a){return boss_native::word(r,a);}
inline bool range(uint32_t a,unsigned size){return !(a&3)&&boss_native::range(a,size);}
inline unsigned byte(uint8_t* rdram,uint32_t a){return MEM_BU(0,boss_native::base(a));}
inline unsigned half(uint8_t* rdram,uint32_t a){return MEM_HU(0,boss_native::base(a));}
inline bool full_provider(uint8_t* r,uint32_t factory){
 if(!range(factory,0x4350))return false;const auto pool=factory+0x2A20,storage=factory+0x36D0;
 if(word(r,pool+4)!=5||word(r,pool+8)!=storage)return false;
 for(unsigned i=0;i<5;i++){const auto a=storage+i*0x280+4,res=word(r,a+0x168);
  if(byte(r,a-4)!=1||!boss_native::actor(r,a,0x46DA)||word(r,a+8)!=pool||byte(r,a+5)!=i+1||!range(res,0x48)||half(r,res+0x3C)!=0x46DA||word(r,res+0x40)!=factory||word(r,res+0x44)!=factory+0x1B84)return false;
 }return true;
}
inline std::optional<Ticket> capture(uint8_t* r,uint32_t actor,uint64_t birth,uint32_t factory,uint32_t stack){
 if(!r||!birth||!boss_native::actor(r,actor,0x4526)||!range(stack-4,4)||!full_provider(r,factory))return {};
 Ticket t;t.actor=actor;t.birth=birth;t.factory=factory;t.stack=stack;t.private_data=word(r,actor+0x1F4);t.resource=word(r,actor+0x168);t.drawing=word(r,actor+0x16C);t.collision=word(r,actor+0x174);t.update=word(r,actor+0x20);t.life=byte(r,actor+6);t.sub=byte(r,actor+0x30);
 if(!range(t.private_data,12)||!range(t.resource,0x48)||half(r,t.resource+0x3C)!=0x4526||word(r,t.resource+0x40)!=factory||word(r,t.resource+0x44)!=factory+0x1B1C||byte(r,actor+0x17D)!=2||byte(r,actor+0x18)!=1||byte(r,actor+0x165)||word(r,actor+0x180)||half(r,actor+0x184)||(t.sub!=5&&t.sub!=6)||t.update!=factory+(t.sub==5?0x9BC:0x884))return {};
 for(unsigned i=0;i<20;i++)if(word(r,actor+0x188+i*4))return {};
 t.manager=word(r,0x800F8810);t.pool_manager=word(r,0x800F887C);if(!range(t.manager,0x14)||!range(t.pool_manager,0x14))return {};
 t.table=word(r,t.manager+0x10);t.capacity=half(r,t.manager+8);t.pool_table=word(r,t.pool_manager+0x10);t.pool_capacity=half(r,t.pool_manager+8);
 if(!t.capacity||t.capacity>150||!range(t.table,t.capacity*4)||!t.pool_capacity||t.pool_capacity>150||!range(t.pool_table,t.pool_capacity*4))return {};
 unsigned actor_refs=0,pool_refs=0;for(unsigned i=0;i<t.capacity;i++)actor_refs+=word(r,t.table+i*4)==actor;for(unsigned i=0;i<t.pool_capacity;i++)pool_refs+=word(r,t.pool_table+i*4)==factory+0x2A20;
 if(actor_refs!=1||pool_refs!=1)return {};
 for(unsigned i=0;i<t.pool_header.size();i++)t.pool_header[i]=uint8_t(byte(r,factory+0x2A20+i));
 for(unsigned i=0;i<t.records.size();i++)t.records[i]=uint8_t(byte(r,factory+0x36D0+i));return t;
}
inline bool completed(uint8_t* r,const recomp_context* c,const Ticket& t){
 const auto a=t.actor;if(!r||!c||uint32_t(c->r29)!=t.stack||word(r,t.stack-4)!=0||!boss_native::actor(r,a,0x4526))return false;
 if(word(r,a+0x1F4)!=t.private_data||word(r,a+0x168)!=t.resource||word(r,a+0x16C)!=t.drawing||word(r,a+0x174)!=t.collision||word(r,a+0x20)!=t.update||byte(r,a+6)!=t.life||byte(r,a+0x30)!=t.sub||byte(r,a+0x18)!=0xFF||byte(r,a+0x28)!=3||byte(r,a+0x17D)!=3||word(r,a+0x180)||half(r,a+0x184))return false;
 if(half(r,t.resource+0x3C)!=0x4526||word(r,t.resource+0x40)!=t.factory||word(r,t.resource+0x44)!=t.factory+0x1B1C||word(r,0x800F8810)!=t.manager||word(r,0x800F887C)!=t.pool_manager||word(r,t.manager+0x10)!=t.table||half(r,t.manager+8)!=t.capacity||word(r,t.pool_manager+0x10)!=t.pool_table||half(r,t.pool_manager+8)!=t.pool_capacity||!full_provider(r,t.factory))return false;
 unsigned refs=0;for(unsigned i=0;i<t.pool_capacity;i++)refs+=word(r,t.pool_table+i*4)==t.factory+0x2A20;if(refs!=1)return false;
 for(unsigned i=0;i<t.capacity;i++)if(word(r,t.table+i*4)==a)return false;
 for(unsigned i=0;i<20;i++)if(word(r,a+0x188+i*4))return false;
 for(unsigned i=0;i<t.pool_header.size();i++)if(byte(r,t.factory+0x2A20+i)!=t.pool_header[i])return false;
 for(unsigned i=0;i<t.records.size();i++)if(byte(r,t.factory+0x36D0+i)!=t.records[i])return false;return true;
}
}
