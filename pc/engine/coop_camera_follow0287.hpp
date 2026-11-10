#pragma once
#include "coop_native_factory.hpp"
#include "coop_lifecycle0280.hpp"
#include <optional>
extern "C" void func_8005A994(uint8_t*,recomp_context*);
namespace workshop::coop_camera_follow0287 {
namespace n=coop_native;namespace ids=coop_collision;namespace life=coop_lifecycle0280;
// A local native death calls2AEE4: the ordinary scene reset would later call
// the5A994 follow setter again. Cooperative local return skips that scene reset.
struct Receipt {ids::Identity dead{},primary{};uint64_t epoch=0;unsigned owner=2,room=0,death_vi=0;uint32_t camera=0,drawing=0;};
inline bool observed_root(uint8_t* r,const ids::Identity& id){const auto now=ids::observed_identity(r,id.address);return id.kind==0x4001&&n::range(id.private_data,0x1550)&&now&&*now==id;}
inline bool scene(uint8_t* r,const Receipt& receipt){
 return n::range(receipt.camera,0xB0)&&n::range(receipt.drawing,0xF8)&&n::word(r,0x800F8294)==receipt.camera&&n::word(r,0x800F8318)==receipt.drawing&&n::word(r,receipt.camera+0x80)==receipt.drawing&&n::word(r,0x800F7EB8)==0x80022E4C&&n::word(r,receipt.camera+0x78)==0x8006CF04&&n::word(r,receipt.drawing+0xF4)==0x8006920C;
}
inline std::optional<Receipt> capture(uint8_t* r,const life::State& state,unsigned owner,unsigned vi,unsigned room){
 if(!r||owner>1||!state.enabled||!state.bound||state.failed||state.ending||!state.epoch||state.room!=room||(room!=43&&room!=81)||state.players[owner].phase!=life::Phase::death_pending||state.players[owner].death_vi!=vi||!observed_root(r,state.players[owner].root)||!observed_root(r,state.players[0].root))return std::nullopt;
 Receipt receipt{state.players[owner].root,state.players[0].root,state.epoch,owner,room,vi,n::word(r,0x800F8294),n::word(r,0x800F8318)};
 if(!scene(r,receipt)||n::word(r,receipt.drawing+0xE8)||n::word(r,receipt.drawing+0xEC)!=0x80065284)return std::nullopt;
 return receipt;
}
inline bool rebind(uint8_t* r,recomp_context* c,const Receipt& receipt,const life::State& state,uint32_t primary,uint32_t secondary,unsigned vi,unsigned room,bool quiescent){
 if(!r||!c||!quiescent||receipt.owner>1||!state.enabled||!state.bound||state.failed||state.ending||state.epoch!=receipt.epoch||state.room!=receipt.room||room!=receipt.room||(room!=43&&room!=81)||vi!=state.vi||!scene(r,receipt)||n::byte(r,0x800F7EA5)||n::byte(r,0x800F7EB4))return false;
 const auto& returned=state.players[receipt.owner];
 if(returned.phase!=life::Phase::alive||returned.death_vi!=receipt.death_vi||returned.respawn_vi!=vi||returned.respawn_vi<receipt.death_vi||returned.respawn_vi-receipt.death_vi<life::respawn_delay||returned.root.generation==receipt.dead.generation||returned.root.kind!=0x4001)return false;
 if(state.players[0].root.address!=primary||state.players[1].root.address!=secondary||primary==secondary||!life::lifecycle.alive(r,0)||!life::lifecycle.alive(r,1)||!observed_root(r,state.players[0].root)||!observed_root(r,state.players[1].root)||!n::primary_ready(r,primary))return false;
 // A returning P2 reattaches the current unchanged P1, never its own body.
 if((receipt.owner==0&&receipt.primary!=receipt.dead)||(receipt.owner==1&&state.players[0].root!=receipt.primary))return false;
 const auto manager=n::word(r,0x800F8810);if(!n::range(manager,0x14))return false;
 const auto capacity=ids::half(r,manager+8);const auto table=n::word(r,manager+0x10);
 if(capacity<6||capacity>150||!n::range(table,size_t(capacity)*4)||n::word(r,table+20)!=primary)return false;
 if(n::word(r,receipt.drawing+0xE8)||n::word(r,receipt.drawing+0xEC)!=0x80065284)return false;
 // Only the existing native setter changes the drawing follow target and its
 // native anchor copy. No actor position, viewport bounds or gameplay data is set.
 {n::ScopedContext registers(c);c->r4=n::base(receipt.drawing);c->r5=n::base(primary);c->r6=n::base(0x8002ABD0);func_8005A994(r,c);}
 return scene(r,receipt)&&n::word(r,receipt.drawing+0xE8)==primary&&n::word(r,receipt.drawing+0xEC)==0x8002ABD0&&observed_root(r,state.players[0].root)&&observed_root(r,state.players[1].root);
}
}
