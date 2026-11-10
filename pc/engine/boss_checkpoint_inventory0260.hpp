#pragma once
#include "boss_native.hpp"
#include <array>
#include <cstdint>
#include <bit>
#include <cmath>

extern "C" void func_80040860(uint8_t*,recomp_context*);
extern "C" void func_8003B8B0(uint8_t*,recomp_context*);
extern "C" void func_800422CC(uint8_t*,recomp_context*);
extern "C" void func_8003ECCC(uint8_t*,recomp_context*);

namespace workshop::boss_checkpoint_inventory0260 {
namespace n=boss_native;
// A checkpoint stores values, never reserve actor pointers. Shy Guy egg colours
// 8..13 are ordinary native4004 reserves with an explicit fourteen-way palette.
using Scale=std::array<uint32_t,3>;
// Saved native78/7C/80 are the stable egg size. The current6C/70/74 may
// animate, while4005 and a subsequently thrown4002 inherit saved78.
struct Egg {uint16_t kind=0;unsigned palette=0;Scale saved_scale{};bool operator==(const Egg&)const=default;};
struct Payload {
 unsigned activation_vi=0;uint32_t player=0;unsigned native_color=8,pc_color=14,hp=0,count=0;
 std::array<Egg,6> eggs{};
 bool operator==(const Payload&)const=default;
};
struct Inventory {unsigned count=0;std::array<uint32_t,6> actors{};std::array<Egg,6> eggs{};};
inline unsigned half(uint8_t* rdram,uint32_t a){return MEM_HU(0,n::base(a));}
inline bool pointer(uint32_t a,unsigned bytes){return !(a&3)&&n::range(a,bytes);}
inline bool valid_scale(const Scale& value){
 for(auto bits:value){const float scale=std::bit_cast<float>(bits);if(!std::isfinite(scale)||scale<=0||scale>4)return false;}return true;
}
inline bool read_scale(uint8_t* ram,uint32_t egg,Scale& out){
 if(!ram||!n::actor(ram,egg,0x4004))return false;
 Scale value{};for(unsigned axis=0;axis<3;++axis)value[axis]=n::word(ram,egg+0x78+4*axis);
 if(!valid_scale(value))return false;out=value;return true;
}
// Native309F8..30A2C prepares A2 for40860 as player6C/70/74 multiplied by
// RAM[AFFE8].40860 itself only copies that input to both egg6C and saved78.
inline bool normal_scale(uint8_t* ram,uint32_t player,Scale& out){
 if(!ram||!n::actor(ram,player,0x4001)||n::word(ram,0x800F8828)!=player)return false;
 const float factor=std::bit_cast<float>(n::word(ram,0x800AFFE8));if(!std::isfinite(factor)||factor<=0||factor>4)return false;
 Scale value{};for(unsigned axis=0;axis<3;++axis){const float parent=std::bit_cast<float>(n::word(ram,player+0x6C+4*axis));if(!std::isfinite(parent)||parent<=0||parent>4)return false;value[axis]=std::bit_cast<uint32_t>(parent*factor);}
 if(!valid_scale(value))return false;out=value;return true;
}
inline bool stage_scale(uint8_t* rdram,uint32_t address,const Scale& scale){
 if(!rdram||!pointer(address,12)||!valid_scale(scale))return false;
 for(unsigned axis=0;axis<3;++axis)MEM_W(int(4*axis),n::base(address))=scale[axis];return true;
}
inline bool same_player(uint8_t* ram,const Payload& p){
 return ram&&n::actor(ram,p.player,0x4001)&&n::word(ram,0x800F8828)==p.player&&
        p.pc_color<14&&p.native_color==(p.pc_color<8?p.pc_color:0)&&
        n::word(ram,0x800F8DB0)==p.native_color&&n::word(ram,0x807F0008)==p.pc_color;
}
// Includes portable native objects recognised by3F894, to avoid silently treating
// one as a normal egg. These objects need their own constructor/data recipe;
// this first restoration path deliberately refuses them before any mutation.
inline bool read(uint8_t* ram,uint32_t player,Inventory& out){
 if(!ram||!n::actor(ram,player,0x4001))return false;
 const auto data=n::word(ram,player+0x1F4);if(!pointer(data,0x12D0))return false;
 unsigned occupied=0;std::array<uint32_t,20> children{};
 for(unsigned i=0;i<children.size();i++){
  const auto a=n::word(ram,player+0x188+i*4);children[i]=a;if(!a)continue;
  if(!pointer(a,0x1FC)||n::word(ram,a)!=0x19971110||n::word(ram,a+0x180)!=player)return false;
  for(unsigned j=0;j<i;j++)if(children[j]==a)return false;
  ++occupied;
 }
 if(occupied!=half(ram,player+0x184))return false;
 Inventory result;result.count=n::byte(ram,data+0x12C8);const unsigned head=half(ram,data+0x12BE);
 if(result.count>6||head>=8)return false;
 for(unsigned i=0;i<result.count;i++){
  const auto a=n::word(ram,data+0x11FC+((head+i)&7)*24);
  if(!pointer(a,0x1FC)||n::word(ram,a)!=0x19971110||n::byte(ram,a+6)==3||n::word(ram,a+0x180)!=player)return false;
  const auto kind=half(ram,a+12);if(kind!=0x4004&&kind!=0x406C&&kind!=0x4144)return false;
  for(unsigned j=0;j<i;j++)if(result.actors[j]==a)return false;
  unsigned references=0;
  for(unsigned j=0;j<20;j++)if(n::word(ram,player+0x188+j*4)==a)++references;
  if(references!=1)return false;
  const auto palette=kind==0x4004?n::word(ram,a+0x1EC):0;
  if(kind==0x4004&&palette>=14)return false;
  Scale saved{};if(kind==0x4004&&!read_scale(ram,a,saved))return false;
  result.actors[i]=a;result.eggs[i]={uint16_t(kind),palette,saved};
 }
 out=result;return true;
}
inline bool valid(const Payload& p){
 if(!p.activation_vi||!pointer(p.player,0x1FC)||p.pc_color>=14||p.native_color!=(p.pc_color<8?p.pc_color:0)||p.hp>8||p.count>6)return false;
 for(unsigned i=0;i<p.count;i++)if(p.eggs[i].kind!=0x4004||p.eggs[i].palette>=14||!valid_scale(p.eggs[i].saved_scale))return false;
 for(unsigned i=p.count;i<p.eggs.size();i++)if(p.eggs[i]!=Egg{})return false;
 return true;
}
inline bool capture(uint8_t* ram,uint32_t player,unsigned vi,Payload& out){
 if(!ram||!vi)return false;
 Payload result;result.activation_vi=vi;result.player=player;
 result.native_color=n::word(ram,0x800F8DB0);result.pc_color=n::word(ram,0x807F0008);
 // In0.26 primary1/sub11 is a living hungry action. The frozen0.24 solo
 // predicate rejects that substate globally, so keep its use out of captures.
 if(!same_player(ram,result)||n::byte(ram,player+0x18)==0xA)return false;
 result.hp=n::word(ram,0x800F8DB4+result.native_color*4);
 Inventory inventory;if(!read(ram,player,inventory))return false;
 result.count=inventory.count;result.eggs=inventory.eggs;
 if(!valid(result))return false;
 out=result;return true;
}
// Runtime supplies this receipt only after its strict native death observer and
// same-4001 native30B7C transport have completed. death_hp is the observed
// value at that native fence, not proof of death: HP0 alone is still playable.
// Current HP must match that exact value before the activation HP is restored.
// This is not a save/load API.
struct RetryProof {uint32_t player=0;unsigned activation_vi=0,death_vi=0,retry_vi=0,retry_epoch=0,room=177,checkpoint_completed=0,death_hp=0;};
inline bool eligible(uint8_t* ram,const Payload& p,const RetryProof& proof){
 if(!valid(p)||!same_player(ram,p)||proof.player!=p.player||proof.activation_vi!=p.activation_vi||
    proof.death_vi<=p.activation_vi||proof.retry_vi<=proof.death_vi||!proof.retry_epoch||proof.checkpoint_completed>9||proof.checkpoint_completed%3||proof.room!=(proof.checkpoint_completed?83u:162u)||proof.death_hp>8||
    n::word(ram,0x800F8DB4+p.native_color*4)!=proof.death_hp)return false;
 Inventory inventory;return read(ram,p.player,inventory)&&inventory.count==0&&half(ram,p.player+0x184)+p.count<=20;
}
inline bool matches(uint8_t* ram,const Payload& p){
 if(!valid(p)||!same_player(ram,p)||n::word(ram,0x800F8DB4+p.native_color*4)!=p.hp)return false;
 Inventory inventory;return read(ram,p.player,inventory)&&inventory.count==p.count&&inventory.eggs==p.eggs;
}
// Native2D53C already calls3ECCC during death. If a completed native death
// still exposes reserves, discard them in the OLD arena before30B7C. Native
//3ECCC drops4002 pickups there and retires their4004 reserves; old-scene teardown
// removes the drops, including a death/reload of rest83 itself. The fatal-state
// guard refuses potA/2 and normal arrival, which would leave undeserved pickups.
inline bool fatal_death_state(uint8_t* ram,uint32_t player){
 if(!ram||!n::actor(ram,player,0x4001)||n::byte(ram,player+0x18)!=0xA)return false;
 const auto sub=n::byte(ram,player+0x30);return sub==0xA||sub==0xB||sub==0xC||sub==0xD||sub==0x10||sub==0x11;
}
inline bool discard_after_death(uint8_t* ram,recomp_context* context,const Payload& p,unsigned death_vi,unsigned source_room,unsigned death_hp=0){
 if(!context||!valid(p)||!same_player(ram,p)||death_vi<=p.activation_vi||source_room>255||!fatal_death_state(ram,p.player)||death_hp>8||
    n::word(ram,0x800F8DB4+p.native_color*4)!=death_hp)return false;
 Inventory before;if(!read(ram,p.player,before))return false;
 for(unsigned i=0;i<before.count;i++)if(before.eggs[i].kind!=0x4004)return false;
 if(!before.count)return true;
 const auto stack=uint32_t(context->r29);if(stack<0x80000800||!pointer(stack-0x400,0x480))return false;
 struct Registers {recomp_context* c;recomp_context saved;~Registers(){*c=saved;}} registers{context,*context};
 context->r29=ADD32(context->r29,-0x80);context->r4=n::base(p.player);func_8003ECCC(ram,context);
 Inventory after;if(!read(ram,p.player,after)||after.count)return false;
 for(unsigned i=0;i<before.count;i++)if(n::word(ram,before.actors[i]+0x180)==p.player)return false;
 return same_player(ram,p)&&n::word(ram,0x800F8DB4+p.native_color*4)==death_hp;
}
class Restore final {
 unsigned last_epoch_=0;
public:
 void reset(){last_epoch_=0;}
 unsigned last_epoch()const{return last_epoch_;}
 bool apply(uint8_t* ram,recomp_context* context,const Payload& p,const RetryProof& proof){
  if(!context||proof.retry_epoch<=last_epoch_||!eligible(ram,p,proof))return false;
  const auto stack=uint32_t(context->r29);
  if(stack<0x80000800||!pointer(stack-0x400,0x480))return false;
  const auto saved=*context;
  struct Registers {recomp_context* c;recomp_context value;~Registers(){*c=value;}} registers{context,saved};
  context->r29=ADD32(saved.r29,-0x80);
  // Consume the retry before the first native call. A partial native allocation
  // failure is fatal to this attempt; replaying it cannot manufacture extra eggs.
  last_epoch_=proof.retry_epoch;
  for(unsigned i=0;i<p.count;i++){
   Inventory before;if(!read(ram,p.player,before)||before.count!=i)return false;
   // The caller owns this scratch inside its reserved0x80-byte frame.
   // Passing player78 here would enlarge a normal0.56 egg to player0.8.
   const auto scale=uint32_t(context->r29)+0x40;
   if(!stage_scale(ram,scale,p.eggs[i].saved_scale))return false;
   context->r4=n::base(p.player);context->r5=n::base(p.player+0x84);context->r6=n::base(scale);
   func_80040860(ram,context);
   Inventory after;if(!read(ram,p.player,after)||after.count!=i+1)return false;
   for(unsigned j=0;j<i;j++)if(after.actors[j]!=before.actors[j]||after.eggs[j]!=p.eggs[j])return false;
   const auto egg=after.actors[i];if(after.eggs[i].kind!=0x4004)return false;
   context->r4=n::base(egg);context->r5=p.eggs[i].palette;func_8003B8B0(ram,context);
   if(!read(ram,p.player,after)||after.eggs[i]!=p.eggs[i])return false;
  }
  if(!same_player(ram,p))return false;
  context->r4=p.hp;func_800422CC(ram,context);
  return matches(ram,p);
 }
};
} // namespace workshop::boss_checkpoint_inventory0260
