#pragma once
#include "boss_battle0250.hpp"
#include "boss_checkpoint_inventory0260.hpp"
#include <array>
#include <cstring>

namespace workshop::boss_pot_aim0282 {
namespace n=boss_native;
namespace inventory=boss_checkpoint_inventory0260;
// Only the exact native entry-pot request may cancel a cursor. The caller
// supplies the campaign's current player/pot, never an inferred replacement.
struct Frame {
 bool enabled=false;
 boss_battle0250::Phase phase=boss_battle0250::Phase::Idle;
 uint32_t player=0,pot=0,pot_overlay=0;
 unsigned room=177,index=10;
};
struct Result {bool applicable=false,ok=true,cleaned=false;uint32_t reticle=0;const char* reason="";};
inline bool pointer(uint32_t a,unsigned bytes){return !(a&3)&&n::range(a,bytes);}
inline unsigned half(uint8_t* rdram,uint32_t a){return MEM_HU(0,n::base(a));}
inline bool same_inventory(const inventory::Inventory& a,const inventory::Inventory& b){return a.count==b.count&&a.actors==b.actors&&a.eggs==b.eggs;}
inline bool native_reticle(uint8_t* r,uint32_t player,uint32_t a){
 if(!n::actor(r,a,0x4003)||n::byte(r,a+4)==1||n::byte(r,a+0x18)==0xFF||n::byte(r,a+0x17D)!=0||n::word(r,a+0x180)!=player||half(r,a+0x184))return false;
 for(unsigned i=0;i<20;++i)if(n::word(r,a+0x188+4*i))return false;
 const auto slot=half(r,a+0x17E);if(slot>=20||n::word(r,player+0x188+4*slot)!=a)return false;
 const auto data=n::word(r,a+0x1F4),resource=n::word(r,a+0x168),draw=n::word(r,a+0x16C),pool=n::word(r,0x800C3D50);
 if(!pointer(data,0x14)||!pointer(resource,0x48)||half(r,resource+0x3C)!=0x4003||!pointer(draw,4)||n::word(r,draw)!=0x8009EBE8||n::word(r,a+0x174)||!pointer(pool,12)||n::word(r,a+8)!=pool)return false;
 const auto capacity=n::word(r,pool+4),storage=n::word(r,pool+8);
 if(!capacity||capacity>32||!pointer(storage,capacity*0x280)||a<storage+4||(a-storage-4)%0x280)return false;
 const auto index=(a-storage-4)/0x280;
 if(index>=capacity||n::byte(r,a-4)!=1||n::byte(r,a+5)!=index+1)return false;
 const unsigned state=n::byte(r,a+0x18);constexpr std::array<uint32_t,3> callbacks{0x80039970,0x800398CC,0x80039924};
 return state<callbacks.size()&&n::word(r,a+0x20)==callbacks[state];
}
// Validate the exact read path used by4700C before making its real call.
// A missing canonical slot may legitimately fall back through the native
// active/pending trees. This model admits that path; it never supplies V0.
#if defined(_MSC_VER)
__declspec(noinline)
#else
__attribute__((noinline))
#endif
inline bool scan_player(uint8_t* r,uint32_t table,unsigned used,unsigned capacity,unsigned depth,unsigned& budget,uint32_t& found){
 if(!used)return true;
 if(used>capacity||capacity>150||depth>20||!pointer(table,capacity*4))return false;
 for(unsigned i=0;i<capacity&&used&&!found;++i){const auto a=n::word(r,table+4*i);if(!a)continue;--used;if(a==1)continue;
  if(!budget--||!pointer(a,0x1FC)||n::word(r,a)!=0x19971110)return false;
  if(half(r,a+12)==0x4001){found=a;return true;}
  const auto children=half(r,a+0x184);if(children>20||!scan_player(r,a+0x188,children,20,depth+1,budget,found))return false;
 }return found||!used;
}
inline bool lookup_safe(uint8_t* r,uint32_t player){
 const auto manager=n::word(r,0x800F8810);if(!pointer(manager,0x14))return false;
 const auto capacity=half(r,manager+8),table=n::word(r,manager+0x10);
 if(capacity<6||capacity>150||!pointer(table,capacity*4))return false;
 const auto canonical=n::word(r,table+20);if(canonical>1)return canonical==player;
 unsigned budget=1024;uint32_t found=0;
 if(!scan_player(r,table,half(r,manager+6),capacity,0,budget,found))return false;
 if(!found){const uint32_t pending=0x800F8840;if(!scan_player(r,n::word(r,pending+0x10),half(r,pending+6),half(r,pending+8),0,budget,found))return false;}
 return found==player;
}
inline bool retirement_ready(uint8_t* r,uint32_t reticle){
 const uint32_t queue=0x800F8854;const auto cap=half(r,queue+8),used=half(r,queue+6),next=half(r,queue+4),table=n::word(r,queue+0x10);
 if(!cap||cap>150||used>=cap||next>=cap||!pointer(table,cap*4)||n::word(r,table+next*4))return false;
 for(unsigned i=0;i<cap;i++)if(n::word(r,table+i*4)==reticle)return false;
 return true;
}
// Dispatch executes the original resident4700C/38F98 bodies. No guest graph,
// HP, FIFO, pose or cursor flag is edited by this HOST adapter.
template<class Dispatch> inline Result cancel(uint8_t* r,recomp_context* c,const Frame& f,Dispatch&& dispatch){
 Result out;
 const bool victory=f.phase==boss_battle0250::Phase::Victory,rest=f.phase==boss_battle0250::Phase::Rest;
 if(!f.enabled||(!victory&&!rest)||!r||!c||f.index>=10||!pointer(f.pot_overlay,0x130)||uint32_t(c->r31)!=f.pot_overlay+0x12C||uint32_t(c->r4)!=f.player||uint32_t(c->r5)!=f.pot||uint32_t(c->r7))return out;
 out.applicable=true;auto reject=[&](const char* reason){out.ok=false;out.reason=reason;return out;};
 if(!n::actor(r,f.player,0x4001)||n::word(r,0x800F8828)!=f.player||!n::actor(r,f.pot,rest?0x4274:0x4273)||(rest?f.room!=83:f.room==83))return reject("pot_identity");
 const auto p=n::word(r,f.player+0x1F4),pd=n::word(r,f.pot+0x1F4),pr=n::word(r,f.pot+0x168);
 if(!pointer(p,0x12F0))return reject("player_private");
 const auto reticle=n::word(r,p+0x1F4);out.reticle=reticle;
 // Native38F38 may leave an independent fading cursor after clearing this
 // backlink. A pot with no current cursor needs no new restrictions or calls.
 if(!reticle)return out;
 if(!pointer(p,0x12F0)||!pointer(pd,0x264)||!pointer(pr,0x48)||half(r,pr+0x3C)!=(rest?0x4274u:0x4273u)||n::word(r,pr+0x40)!=f.pot_overlay||!pointer(n::word(r,pd+0x260),24)||uint32_t(c->r6)!=n::word(r,f.pot+0x88)||n::byte(r,f.player+0x18)!=1||n::word(r,p+0x12EC)||n::byte(r,p+0x154)||n::byte(r,p+0x155)||n::byte(r,p+0x1F8))return reject("native_entry_request");
 inventory::Inventory before;if(!inventory::read(r,f.player,before))return reject("inventory_before");
 unsigned cursor_children=0;for(unsigned i=0;i<20;++i){const auto a=n::word(r,f.player+0x188+4*i);if(a&&n::actor(r,a,0x4003)){++cursor_children;if(a!=reticle)return reject("orphan_cursor");}}
 if(cursor_children!=1||!native_reticle(r,f.player,reticle))return reject("cursor_identity");
 if(!lookup_safe(r,f.player))return reject("native_player_lookup");
 if(!retirement_ready(r,reticle))return reject("native_retirement_queue");
 const auto color=n::word(r,0x800F8DB0);if(color>=8)return reject("native_color");
 const auto hp=n::word(r,0x800F8DB4+color*4),count=n::word(r,0x800F89D0),score=n::word(r,0x800F8954);
 const auto child_count=half(r,f.player+0x184),slot=half(r,reticle+0x17E),data=n::word(r,reticle+0x1F4),resource=n::word(r,reticle+0x168);
 const recomp_context saved=*c;const auto stack=uint32_t(saved.r29);
 if(stack<0x80000800||!pointer(stack-0x180,0x180))return reject("native_stack");
 try{
  c->r29=n::base(stack-0x100);dispatch(0x8004700C,r,c);
  if(uint32_t(c->r2)!=f.player){*c=saved;return reject("native_player_lookup");}
  *c=saved;c->r29=n::base(stack-0x100);dispatch(0x80038F98,r,c);*c=saved;
 }catch(...){*c=saved;throw;}
 inventory::Inventory after;
 if(n::word(r,p+0x1F4)||n::word(r,reticle+0x180)||n::byte(r,reticle+0x18)!=0xFF||n::byte(r,reticle+0x28)!=3||n::byte(r,reticle+0x17D)!=3||n::word(r,reticle+0x1F4)!=data||n::word(r,reticle+0x168)!=resource||n::word(r,f.player+0x188+4*slot)||half(r,f.player+0x184)+1!=child_count||!inventory::read(r,f.player,after)||!same_inventory(before,after))return reject("native_cursor_retirement");
 if(n::word(r,0x800F8DB0)!=color||n::word(r,0x800F8DB4+color*4)!=hp||n::word(r,0x800F89D0)!=count||n::word(r,0x800F8954)!=score)return reject("payload_changed");
 out.cleaned=true;return out;
}
}
