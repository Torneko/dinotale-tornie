#include "coop_native_probe.hpp"
#include <vector>
#include <iostream>
#include <cassert>
namespace {
unsigned checks=0;
void check(bool ok){checks++;if(!ok)throw std::runtime_error("probe-memory-check-failed");}
void h(uint8_t* rdram,uint32_t p,uint16_t v){const gpr base=workshop::coop_native::base(p);MEM_H(0,base)=v;}
void make(uint8_t* r,uint32_t p,uint16_t kind,uint32_t data=0){using namespace workshop::coop_native;word(r,p,0x19971110);h(r,p+12,kind);word(r,p+0x1F4,data);}
}
int main(){
 using namespace workshop;
 std::vector<uint8_t> data(0x800000);auto* r=data.data();
 constexpr uint32_t root=0x80100000,priv=0x80200000,seed=0x80101000,reserve=0x80102000,aim=0x80103000;
 make(r,root,0x4001,priv);make(r,seed,0x4002,0x80210000);make(r,reserve,0x4004);make(r,aim,0x4005);
 coop_probe::configure(true);auto& player=coop_probe::state.players[0];player.root=root;player.root_private=priv;player.collectible=seed;player.created=true;player.born=1;
 coop_probe::Inventory inv{};check(coop_probe::inventory(r,root,inv));check(inv.count==0);player.before=inv;
 check(!coop_probe::collectible_retired(r,player));check(!coop_probe::observe_reserve(r,2441,0));
 coop_native::byte(r,seed+0x18,0xFF);coop_native::byte(r,seed+0x17D,3);check(coop_probe::collectible_retired(r,player));
 coop_native::byte(r,priv+0x12C8,1);h(r,priv+0x12BE,7);coop_native::word(r,priv+0x11FC+7*24,reserve);coop_native::word(r,reserve+0x180,root);
 const auto before=data;check(coop_probe::observe_reserve(r,2441,0));check(player.picked);check(player.reserve==reserve);check(!player.pickup_callback_invoked);check(data==before);
 player.picked=false;coop_native::word(r,reserve+0x180,0);
 bool rejected=false;try{coop_probe::observe_reserve(r,2442,0);}catch(const std::runtime_error&){rejected=true;}check(rejected);
 coop_native::word(r,reserve+0x180,root);coop_native::byte(r,seed+0x18,1);coop_native::byte(r,seed+0x17D,2);
 rejected=false;try{coop_probe::observe_reserve(r,2442,0);}catch(const std::runtime_error&){rejected=true;}check(rejected);
 coop_native::byte(r,priv+0x12C8,9);check(!coop_probe::inventory(r,root,inv));coop_native::byte(r,priv+0x12C8,1);h(r,priv+0x12BE,8);check(!coop_probe::inventory(r,root,inv));h(r,priv+0x12BE,7);
 h(r,root+0x184,1);coop_native::word(r,root+0x188,aim);coop_native::word(r,aim+0x180,root);check(coop_probe::aiming_child(r,root)==aim);coop_native::word(r,aim+0x180,0);check(coop_probe::aiming_child(r,root)==0);h(r,root+0x184,30);check(coop_probe::aiming_child(r,root)==0);
 coop_native::word(r,seed+0x174,0x80220000);coop_native::byte(r,seed+0x18,1);check(coop_probe::ready_collectible(r,player));coop_native::byte(r,0x80210000,1);check(!coop_probe::ready_collectible(r,player));coop_native::byte(r,0x80210000,0);coop_native::word(r,seed+0x1F4,0);check(!coop_probe::ready_collectible(r,player));
 coop_probe::configure(false);check(!coop_probe::snapshot().enabled);check(!coop_probe::snapshot().roots_bound);check(coop_probe::snapshot().created_count==0);
 std::cout<<"CoopProbe memory checks PASS "<<checks<<" assertions; no native callbacks/game/GPU.\n";
}
