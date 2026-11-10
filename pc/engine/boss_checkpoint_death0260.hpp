#pragma once
#include "boss_checkpoint_inventory0260.hpp"
#include "boss_carry0260.hpp"
#include <array>

extern "C" void func_800447C8(uint8_t*,recomp_context*);
namespace workshop::boss_checkpoint_death0260 {
namespace n=boss_native;namespace inventory=boss_checkpoint_inventory0260;
inline constexpr unsigned overlay_section=128,overlay_extent=0x1220;
inline constexpr uint16_t escort_kind=0x4015;
inline constexpr uint32_t update_offset=0xA80,destructor_offset=0xAC0,initializer_offset=0xB48;
inline bool pointer(uint32_t a,unsigned bytes){return !(a&3)&&n::range(a,bytes);}
inline unsigned half(uint8_t* rdram,uint32_t a){return MEM_HU(0,n::base(a));}
inline bool fatal(uint8_t* ram,uint32_t player){return inventory::fatal_death_state(ram,player)&&n::word(ram,0x800F8828)==player;}
struct Child {
 uint32_t actor=0;unsigned slot=0;bool escort=false;
 std::array<uint32_t,0x1FC/4> original{};
};
inline bool escort_identity(uint8_t* ram,uint32_t a,uint32_t player,unsigned slot,uint32_t overlay){
 if(!pointer(overlay,overlay_extent)||!n::actor(ram,a,escort_kind)||n::byte(ram,a+4)==1||n::byte(ram,a+6)!=2||n::byte(ram,a+0x17D)!=0||
    n::byte(ram,a+0x18)!=0||n::word(ram,a+0x180)!=player||half(ram,a+0x17E)!=slot||half(ram,a+0x184)||
    n::word(ram,a+0x1F4)||n::word(ram,a+0x1D8)||n::word(ram,a+0x1DC)||
    n::word(ram,a+0x20)!=overlay+update_offset||n::word(ram,a+0x2C)!=overlay+destructor_offset)return false;
 const auto resource=n::word(ram,a+0x168);
 if(!pointer(resource,0x48)||half(ram,resource+0x3C)!=escort_kind||n::word(ram,resource+0x30)||
    n::word(ram,resource+0x40)!=overlay||n::word(ram,resource+0x44)!=overlay+initializer_offset)return false;
 const auto metadata=n::word(ram,a+0x174);
 if(metadata){
  if(!pointer(metadata,4))return false;const auto definition=n::word(ram,metadata);
  if(!pointer(definition,0x7C)||n::byte(ram,definition+0x74)||n::byte(ram,definition+0x78))return false;
 }
 for(unsigned i=0;i<20;++i)if(n::word(ram,a+0x188+i*4))return false;
 return true;
}
// NativeC8D0 creates four cinematic4015 children during death. Unlike enemy
// Shy Guys, theirC590 update never retires them. Ordinary death discards P1;
// a same-4001 checkpoint must retire only these children before native30B7C
// carries the graph.447C8 performs the real recursive detach/collider/queue
// retirement. No actor graph, collision flag, HP or controller flag is patched.
inline bool cleanup(uint8_t* ram,recomp_context* context,uint32_t player,uint32_t overlay){
 if(!ram||!context||!pointer(player,0x1FC)||!fatal(ram,player))return false;
 const unsigned count=half(ram,player+0x184);if(count>20)return false;
 std::array<Child,20> children{};std::array<unsigned,4> escorts{};unsigned occupied=0,escort_count=0,ordinal_mask=0;
 for(unsigned slot=0;slot<20;++slot){
  const auto a=n::word(ram,player+0x188+slot*4);if(!a)continue;
  if(!pointer(a,0x1FC)||n::word(ram,a)!=0x19971110||n::word(ram,a+0x180)!=player)return false;
  for(unsigned i=0;i<occupied;++i)if(children[i].actor==a)return false;
  auto& child=children[occupied];child.actor=a;child.slot=slot;child.escort=half(ram,a+12)==escort_kind;
  if(child.escort){
   if(escort_count==escorts.size()||!escort_identity(ram,a,player,slot,overlay))return false;
   const unsigned ordinal=n::word(ram,a+0x1E0);if(ordinal>=4||(ordinal_mask&(1u<<ordinal)))return false;
   ordinal_mask|=1u<<ordinal;escorts[escort_count++]=occupied;
  }else for(unsigned i=0;i<child.original.size();++i)child.original[i]=n::word(ram,a+i*4);
  ++occupied;
 }
 if(occupied!=count)return false;
 inventory::Inventory reserves;boss_carry0260::Payload stats;
 if(!inventory::read(ram,player,reserves)||!boss_carry0260::capture(ram,stats))return false;
 if(!escort_count)return true;
 const auto stack=uint32_t(context->r29);if(stack<0x80000800||!pointer(stack-0x400,0x480))return false;
 const auto native_color=n::word(ram,0x800F8DB0),pc_color=n::word(ram,0x807F0008);
 if(native_color>=8||pc_color>=14)return false;
 const auto hp=n::word(ram,0x800F8DB4+native_color*4);
 struct Registers {recomp_context* c;recomp_context saved;~Registers(){*c=saved;}} registers{context,*context};
 // All candidates and the entire sparse graph were validated before the first
 // native mutation. Siblings keep their original slots and actor bytes.
 for(unsigned i=0;i<escort_count;++i){
  const auto a=children[escorts[i]].actor;context->r29=ADD32(registers.saved.r29,-0xC0);context->r4=n::base(a);
  func_800447C8(ram,context);
  if(n::word(ram,a+0x180)||n::byte(ram,a+0x18)!=0xFF||n::byte(ram,a+0x17D)!=3)return false;
 }
 if(!fatal(ram,player)||half(ram,player+0x184)!=count-escort_count||n::word(ram,0x800F8DB0)!=native_color||
    n::word(ram,0x807F0008)!=pc_color||n::word(ram,0x800F8DB4+native_color*4)!=hp||!boss_carry0260::matches(ram,stats))return false;
 for(unsigned i=0;i<occupied;++i){const auto& child=children[i];
  if(n::word(ram,player+0x188+child.slot*4)!=(child.escort?0:child.actor))return false;
  if(!child.escort)for(unsigned j=0;j<child.original.size();++j)if(n::word(ram,child.actor+j*4)!=child.original[j])return false;
 }
 inventory::Inventory after;return inventory::read(ram,player,after)&&after.count==reserves.count&&after.actors==reserves.actors&&after.eggs==reserves.eggs;
}
} // namespace workshop::boss_checkpoint_death0260
