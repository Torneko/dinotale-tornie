#include "../boss_solo_runtime0240.hpp"
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
namespace b=workshop::boss_native;namespace r=workshop::boss_solo0240;namespace s=workshop::boss_solo_fight;namespace d=workshop::boss_rest_data0240;
static unsigned checks=0;static void require(bool v,const char* m){++checks;if(!v)throw std::runtime_error(m);}
struct Call{std::string fn;uint32_t a0,a1,a2,a3,sp;};static std::vector<Call> calls;static std::vector<uint32_t> carry;static size_t carry_index=0;static bool registration_success=true,pot_activation_success=true,pot_allocation_success=true;static uint32_t pot_resource=0x200;static unsigned expected_pending_survivors=1;static float expected_pot_x=120.0f;
static constexpr uint32_t pot=0x80025000,pot_data=0x80026000,pot_route=0x80027000;
static void record(const char* n,recomp_context* c){calls.push_back({n,uint32_t(c->r4),uint32_t(c->r5),uint32_t(c->r6),uint32_t(c->r7),uint32_t(c->r29)});}
static void clobber(recomp_context* c,uint32_t result=0){c->r4=0x1111;c->r5=0x2222;c->r29=0x3333;c->r31=0x4444;c->f2.u32l=0x5555;c->hi=0x6666;c->r2=result;}
// Native57C9C consumes exactly the head slot. It deliberately cannot skip a hole.
// Native57D1C removes exactly the tail-1 slot. These stubs model both cursors.
static uint32_t fifo_pop(uint8_t* rdram,bool tail){
 const auto q=b::base(0x800F882C);const auto cap=MEM_HU(8,q);const auto count=MEM_HU(6,q);if(!count)return 0;
 const unsigned index=tail?(MEM_HU(4,q)+cap-1)%cap:MEM_HU(2,q);const auto table=b::base(uint32_t(MEM_W(0x10,q)));const auto a=uint32_t(MEM_W(index*4,table));if(!a||a==1)return 0;
 MEM_W(index*4,table)=0;MEM_H(tail?4:2,q)=tail?index:(index+1)%cap;MEM_H(6,q)=count-1;return a;
}
static void clear_binding(uint8_t* rdram,uint32_t a){MEM_H(0x17E,b::base(a))=0x96;MEM_B(0x17D,b::base(a))=0;}
static bool contiguous_pending(uint8_t* rdram){
 const auto q=b::base(0x800F882C);const unsigned cap=MEM_HU(8,q),count=MEM_HU(6,q),head=MEM_HU(2,q),tail=MEM_HU(4,q);if(!cap||head>=cap||tail>=cap||count>cap||tail!=(head+count)%cap)return false;
 const auto table=b::base(uint32_t(MEM_W(0x10,q)));for(unsigned n=0;n<count;++n)if(uint32_t(MEM_W(((head+n)%cap)*4,table))<=1)return false;return true;
}
extern "C" void func_800447C8(uint8_t* rdram,recomp_context* c){require(MEM_BU(0x17D,c->r4)==2,"447C8 refuses pending actor");record("retire-active",c);MEM_B(0x18,c->r4)=0xFF;MEM_B(0x17D,c->r4)=3;clobber(c);}
extern "C" void func_80043DD4(uint8_t* rdram,recomp_context* c){
 record("pop-pending-head",c);require(uint32_t(c->r4)==0x800F882C,"head-pop queue ABI");const auto a=fifo_pop(rdram,false);require(a==0x80016000&&MEM_BU(0x17D,b::base(a))==1,"head-pop exact pending cinematic");clear_binding(rdram,a);clobber(c,a);
}
extern "C" void func_80057D1C(uint8_t* rdram,recomp_context* c){
 record("pop-pending-tail",c);require(uint32_t(c->r4)==0x800F882C,"tail-pop queue ABI");const auto a=fifo_pop(rdram,true);require(a==0x80016000&&MEM_BU(0x17D,b::base(a))==1,"tail-pop exact pending cinematic");clobber(c,a);
}
extern "C" void func_80043C94(uint8_t* rdram,recomp_context* c){
 record("clear-pending-binding",c);require(uint32_t(c->r4)==0x80016000&&MEM_BU(0x17D,c->r4)==1,"tail binding cleared only for detached cinematic");clear_binding(rdram,uint32_t(c->r4));clobber(c,1);
}
extern "C" void func_800445A4(uint8_t* rdram,recomp_context* c){
 record("retire-pending",c);const auto q=b::base(0x800F882C);require(MEM_BU(0x17D,c->r4)==0&&MEM_HU(0x17E,c->r4)==0x96,"pending unregistered before disposal");require(MEM_HU(6,q)==expected_pending_survivors&&contiguous_pending(rdram),"native pop decremented once and preserved a contiguous FIFO");if(expected_pending_survivors)require(uint32_t(MEM_W(MEM_HU(2,q)*4,b::base(uint32_t(MEM_W(0x10,q)))))==0x80021000,"unrelated pending head preserved");MEM_B(0x18,c->r4)=0xFF;MEM_B(0x17D,c->r4)=3;clobber(c,1);
}
extern "C" void func_80030B7C(uint8_t*,recomp_context* c){record("unexpected-auto-warp",c);clobber(c);}
extern "C" void func_80047090(uint8_t*,recomp_context* c){record("pot-resource",c);require(c->r4==d::arena_pot_kind,"request native4273 resource");clobber(c,pot_resource);}
extern "C" void func_8004613C(uint8_t* rdram,recomp_context* c){record("pot-allocate",c);require(uint32_t(c->r4)==((uint32_t(d::arena_pot_kind)<<16)|0x2000|pot_resource),"native pot packed/resource ABI");require(uint32_t(c->r5)==std::bit_cast<uint32_t>(expected_pot_x)&&uint32_t(c->r6)==std::bit_cast<uint32_t>(d::arena_pot_y)&&uint32_t(c->r7)==d::arena_pot_z_bits,"native arena pot position ABI");if(!pot_allocation_success){clobber(c);return;}MEM_W(0,b::base(pot))=0x19971110;MEM_H(12,b::base(pot))=d::arena_pot_kind;MEM_W(0x1F4,b::base(pot))=pot_data;MEM_W(0x260,b::base(pot_data))=pot_route;MEM_H(0xC0,b::base(pot))=8;MEM_H(0xC2,b::base(pot))=16;clobber(c,pot);}
extern "C" void func_80044194(uint8_t* rdram,recomp_context* c){
 record("pot-activate",c);require(uint32_t(c->r4)==pot,"activate allocated native pot");require(MEM_HU(0xC0,c->r4)==0x7FFF&&MEM_HU(0xC2,c->r4)==0x7FFF,"owned exit pot gets native uncullable bounds before registration");if(pot_activation_success){const auto q=b::base(0x800F882C);const unsigned tail=MEM_HU(4,q),cap=MEM_HU(8,q),count=MEM_HU(6,q);const auto table=b::base(uint32_t(MEM_W(0x10,q)));require(count<cap&&!MEM_W(tail*4,table),"native FIFO enqueue requires free tail slot");MEM_W(tail*4,table)=pot;MEM_H(4,q)=(tail+1)%cap;MEM_H(6,q)=count+1;MEM_H(0x17E,c->r4)=tail;MEM_B(0x17D,c->r4)=1;}clobber(c,pot_activation_success?1:0);
}
extern "C" void func_8005D29C(uint8_t*,recomp_context* c){record("dequeue",c);auto a=carry_index<carry.size()?carry[carry_index++]:0;clobber(c,a);}
extern "C" void func_8002BEB0(uint8_t*,recomp_context* c){record("restore-native",c);clobber(c);}
extern "C" void func_800443EC(uint8_t*,recomp_context* c){record("register",c);clobber(c,registration_success?1:0);}
extern "C" void func_80040C0C(uint8_t*,recomp_context* c){record("controls",c);clobber(c);}
extern "C" void func_80047AB8(uint8_t* rdram,recomp_context* c){record("matrix",c);for(unsigned off:{0x3Cu,0x40u,0x44u})MEM_W(off+0x48,c->r5)=MEM_W(off,c->r5);clobber(c);}
struct Fixture{
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* rdram=memory.data();
 static constexpr uint32_t root=0x80010000,priv=0x80011000,head=0x80012000,hp=0x80013000,player=0x80014000,transition=0x80016000,tp=0x80017000,queue=0x80018000,info=0x80019000,other_route=0x8001A000,egg1=0x8001B000,egg2=0x8001C000,tongue=0x8001D000,popup=0x80022000,player_data=0x80030000;
 recomp_context c{};b::Frame f{162,100,0x80400000,0x80500000};
 void w(uint32_t p,uint32_t v){MEM_W(0,b::base(p))=int32_t(v);}void by(uint32_t p,unsigned v){MEM_B(0,b::base(p))=uint8_t(v);}void half(uint32_t p,unsigned v){MEM_H(0,b::base(p))=uint16_t(v);}void actor(uint32_t a,uint16_t kind){w(a,0x19971110);half(a+12,kind);}bool context(const recomp_context& v){return std::memcmp(&c,&v,sizeof(c))==0;}
 Fixture(bool pending=true){
  calls.clear();carry.clear();carry_index=0;registration_success=pot_activation_success=pot_allocation_success=true;pot_resource=0x200;expected_pending_survivors=1;expected_pot_x=120.0f;
  actor(root,0x455A);w(root+0x1F4,priv);w(priv,head);actor(head,0x454F);w(head+0x1F4,hp);w(head+0x180,root);w(hp+0x84,12);
  actor(player,0x4001);w(player+0x1F4,player_data);w(player+0x130,0x800233B4);w(player+0x3C,std::bit_cast<uint32_t>(40.0f));w(player+0x84,std::bit_cast<uint32_t>(40.0f));half(player+0xC0,0x1111);half(player+0xC2,0x2222);actor(transition,0x46EE);w(transition+0x1F4,tp);w(0x800F8D78,f.scene_overlay+0x2418);w(0x800F8828,player);w(0x800F8770,queue);
  by(transition+0x17D,pending?1:2);half(transition+0x17E,pending?46:15);half(0x800F882C+2,pending?46:47);half(0x800F882C+4,48);half(0x800F882C+6,pending?2:1);half(0x800F882C+8,150);w(0x800F882C+0x10,0x80020000);actor(0x80021000,0x4030);half(0x80021000+0xC0,0x3333);half(0x80021000+0xC2,0x4444);by(0x80021000+0x17D,1);half(0x80021000+0x17E,47);w(0x80020000+47*4,0x80021000);if(pending)w(0x80020000+46*4,transition);
  w(0x800F8DB0,2);w(0x800F8DB4+2*4,7);half(0x800F897E,0xBEEF);w(0x800F89D0,4);by(info+1,d::source_room);by(0x800F7EA6,1);
  actor(egg1,0x4004);actor(egg2,0x4004);actor(tongue,0x4007);actor(popup,0x4016);for(uint32_t a:{egg1,egg2,tongue,popup})w(a+0x180,player);
  w(player+0x188,egg1);w(player+0x188+7*4,egg2);w(player+0x188+12*4,popup);w(player+0x188+19*4,tongue);half(player+0x184,4);
  half(player_data+0x12BE,7);by(player_data+0x12C8,2);w(player_data+0x11FC+7*24,egg1);w(player_data+0x11FC,egg2);
  for(unsigned n=0;n<24;++n){by(pot_route+n,d::native_arena_pot_route[n]);by(other_route+n,d::native_pot_route[n]);}
  require(r::configure("solo-runtime-0240",false,false),"runtime config");require(b::configure(true,"solo-runtime-0240",false),"observer config");r::enter_room(52);
  c.r4=b::base(0x80023400);c.r5=0x9876;c.r6=0xABCD;c.r29=b::base(0x800FD000);c.r31=b::base(0x80123456);c.f2.u32l=0x77889900;c.hi=0x11223344;
 }
 void entry(const char* n,uint32_t a,uint32_t ra=0,uint32_t a1=0,uint32_t a2=0){const auto old=c;c.r4=b::base(a);c.r5=a1;c.r6=a2;c.r31=b::base(ra);b::observe_entry(rdram,&c,n,f);c=old;}
 void victory(bool capture=true){entry("func_80664E18",root);b::frame(rdram,f);by(root+0x18,1);by(root+0x30,1);entry("func_806657A0",root);w(hp+0x84,0);w(priv+0x20,1);by(root+0x30,8);w(root+0x1E0,0);++f.vi;entry("func_80665C80",root);by(root+0x18,0xFF);by(root+0x17D,3);if(capture)r::defeat_begin(rdram);entry("func_807AEA78",player,f.candy_overlay+0x109C);entry("func_807AEA24",transition,f.scene_overlay+0x23AC,1,0);by(0x800F7EA6,0);const auto old=c;r::defeat_complete(rdram,&c);require(context(old),"victory restores full caller context");}
 void spawn(){const auto old=c;r::frame(rdram,&c,200);require(context(old),"pot creation preserves full caller context");}
 void transport(){by(player+0x18,0xA);by(player+0x30,2);w(player+0x20,0x80031230);w(player_data+0xD0,pot);w(player_data+0xC0,0);by(player+0x165,1);by(player+0x17D,0);w(player+0x180,0);}
 bool use_pot(bool prepare_native=true){if(prepare_native)transport();const auto old=c;c.r4=b::base(pot_route);const bool handled=r::pot_complete(rdram,&c);auto expected=old;expected.r4=b::base(r::route_address);if(r::session.snapshot().phase==s::Phase::departing)require(context(expected),"arena pot changes only nativeA0 route");else {expected.r4=b::base(pot_route);require(context(expected),"rejected arena pot preserves all caller registers");}c=old;return handled;}
 void remove_popup_and_reorder(){for(unsigned n=0;n<20;++n)w(player+0x188+n*4,0);w(player+0x188+2*4,tongue);w(player+0x188+5*4,egg2);w(player+0x188+19*4,egg1);half(player+0x184,3);w(popup+0x180,0);}
 // Simulate native44CA arrival, then native31230/30D14/2CA68 release.
 // Runtime is an observer here: it must never dequeue/register/set player state.
 void arrive(bool other_player=false){r::enter_room(d::source_room);const auto old=c;c.r4=b::base(info);r::room_ready(rdram,&c);c=old;const auto count=calls.size();const auto waiting=memory;r::frame(rdram,&c,210);require(context(old)&&memory==waiting&&calls.size()==count&&r::session.snapshot().phase==s::Phase::departing,"native transport A/2 waits without runtime RAM/context/native calls");
  by(player+0x18,1);by(player+0x30,2);w(player+0x20,0x80043450);by(player+0x17D,2);by(player+0x165,0);w(player+0x180,0);w(player+0x3C,std::bit_cast<uint32_t>(d::entry_x));w(player+0x84,std::bit_cast<uint32_t>(d::entry_x));if(other_player){actor(0x80023000,0x4001);w(0x800F8828,0x80023000);}const auto released=memory;r::frame(rdram,&c,211);require(context(old)&&memory==released&&calls.size()==count,"runtime observes native release without state/callback/position writes or calls");if(other_player)r::frame(rdram,&c,2011);
 }
 void ready(){victory();spawn();require(!use_pot()&&r::session.snapshot().phase==s::Phase::departing,"real arena pot requests original native transport");}
};
int main(){try{
 {Fixture x;const auto before=x.memory;require(r::configure("startup-0240",false,false),"startup config");for(unsigned room:{0u,4u,41u,5u}){r::enter_room(room);require(r::session.snapshot().phase==s::Phase::armed&&r::room==177,"selection staysarmed");}r::enter_room(52);require(r::session.snapshot().phase==s::Phase::fighting&&r::room==162&&x.memory==before&&calls.empty(),"native root52 begins arena162 without mutating RAM");}
 {Fixture x;const auto before=x.memory;const auto old=x.c;r::enabled=false;r::defeat_begin(x.rdram);r::defeat_complete(x.rdram,&x.c);r::frame(x.rdram,&x.c,200);require(x.memory==before&&calls.empty()&&x.context(old),"disabled runtime inert");}
 {Fixture x;const auto before=x.memory;r::defeat_complete(x.rdram,&x.c);x.spawn();require(x.memory==before&&calls.empty()&&r::session.snapshot().phase==s::Phase::fighting,"no pot or cleanup before strict native proof");}
 {
  Fixture x;x.victory();require(calls.size()==3&&calls[0].fn=="pop-pending-head"&&calls[1].fn=="retire-pending"&&calls[2].fn=="controls","pending cinematic cancelled then player controls restored");require(b::byte(x.rdram,0x800F7EA6)==1,"pre-cinematic running flag restored");require(r::session.snapshot().phase==s::Phase::victory_ready,"victory waitsfor voluntary pot");r::defeat_complete(x.rdram,&x.c);require(calls.size()==3,"same receipt cannot trigger second cleanup");
  x.spawn();require(calls.size()==6&&calls[3].fn=="pot-resource"&&calls[4].fn=="pot-allocate"&&calls[5].fn=="pot-activate"&&r::victory_pot==pot,"one real native arena pot created");require(contiguous_pending(x.rdram)&&fifo_pop(x.rdram,false)==0x80021000&&fifo_pop(x.rdram,false)==pot&&!fifo_pop(x.rdram,false)&&contiguous_pending(x.rdram),"native pending activation reaches unrelated actor then victory pot without holes");auto old=x.c;r::frame(x.rdram,&x.c,500);require(calls.size()==6&&r::room==162&&r::session.snapshot().phase==s::Phase::victory_ready&&x.context(old),"waiting after victory neverauto-warps or respawns pot");for(const auto& call:calls)require(call.fn!="unexpected-auto-warp","no auto30B7C departure");
  x.c.r4=b::base(Fixture::other_route);require(!r::pot_complete(x.rdram,&x.c)&&r::session.snapshot().phase==s::Phase::victory_ready,"unbound pot cannot start departure");x.c=old;
  require(!x.use_pot()&&r::session.snapshot().phase==s::Phase::departing,"native arena pot starts voluntary passage");require(r::carried_health==7&&r::carried_color==2&&r::carried_fruits==4,"true healthword chosen by native color, not padF897E");require(r::carried_reserves[0]==Fixture::egg1&&r::carried_reserves[1]==Fixture::egg2,"wrapped native egg ring preserves order");require(b::word(x.rdram,r::route_address)==0x44CA2000&&b::byte(x.rdram,r::route_address+4)==d::source_room,"owned warmroute preserves exact native44CA2000 identity and names rest83");
  x.remove_popup_and_reorder();x.half(0x800F897E,0x1234);x.arrive();require(r::session.snapshot().phase==s::Phase::rest&&!r::finished,"same living Yoshi reaches rest after natural popup cleanup/reordered slots");require(b::word(x.rdram,Fixture::player+0x130)==0x800233B4&&std::bit_cast<float>(b::word(x.rdram,Fixture::player+0x84))==d::entry_x,"native arrival callback and position remain unchanged by observer");require(b::word(x.rdram,0x800F8DB4+2*4)==7&&b::word(x.rdram,0x800F89D0)==4,"health and fruit count unchanged");
  const std::vector<std::string> expected{"pop-pending-head","retire-pending","controls","pot-resource","pot-allocate","pot-activate"};std::vector<std::string> names;for(const auto& call:calls)names.push_back(call.fn);require(names==expected,"native cleanup/spawn sequence exact; rest observer never calls warmcarry restoration");
  x.c.r4=b::base(Fixture::other_route);x.by(Fixture::other_route+4,0);require(!r::pot_complete(x.rdram,&x.c)&&!r::finished,"other rest route untouched");x.by(Fixture::other_route+4,d::native_pot_route[4]);const auto before=x.memory;const auto context=x.c;require(r::pot_complete(x.rdram,&x.c)&&r::finished&&r::session.snapshot().phase==s::Phase::milestone_finished,"onward rest pot endsfirst slice only");require(x.memory==before&&x.context(context)&&!r::pot_complete(x.rdram,&x.c),"onward pot inert after one completion");
 }
 // FIFO endpoint cases use the same native operations as actual pending actor activation.
 for(bool tail:{false,true})for(bool wrap:{false,true}){
  Fixture x;const unsigned h=wrap?149:45,index=tail?(h+1)%150:h,other=tail?h:(h+1)%150;
  for(unsigned n=0;n<150;++n)x.w(0x80020000+n*4,0);x.half(0x800F882C+2,h);x.half(0x800F882C+4,(h+2)%150);x.half(0x800F882C+6,2);x.half(Fixture::transition+0x17E,index);x.w(0x80020000+index*4,Fixture::transition);x.w(0x80020000+other*4,0x80021000);x.half(0x80021000+0x17E,other);
  x.victory();require(r::session.snapshot().phase==s::Phase::victory_ready&&calls[0].fn==(tail?"pop-pending-tail":"pop-pending-head"),"pending cinematic uses only its FIFO endpoint");require(calls.size()==(tail?4u:3u)&&contiguous_pending(x.rdram),"endpoint pop preserves count and both cursors across wrap");if(tail)require(calls[1].fn=="clear-pending-binding","tail-pop explicitly clears native binding");x.spawn();require(fifo_pop(x.rdram,false)==0x80021000&&fifo_pop(x.rdram,false)==pot&&!fifo_pop(x.rdram,false)&&contiguous_pending(x.rdram),"endpoint cancellation allows later native pot activation");
 }
 {Fixture x;expected_pending_survivors=0;x.w(0x80020000+47*4,0);x.half(0x800F882C+4,47);x.half(0x800F882C+6,1);x.victory();require(calls.size()==3&&calls[0].fn=="pop-pending-head"&&contiguous_pending(x.rdram),"single queued cinematic uses head-pop with coincident endpoints");x.spawn();require(fifo_pop(x.rdram,false)==pot&&contiguous_pending(x.rdram),"empty FIFO after cinematic accepts next pot without stale head");}
 {Fixture x;x.victory();x.half(0x800F882C+2,46);x.spawn();const auto before=x.memory;require(!contiguous_pending(x.rdram)&&!fifo_pop(x.rdram,false)&&x.memory==before&&(b::word(x.rdram,0x800F8830)&0xFFFF)==2,"regression: old43D0C hole stalls exact native head pop despite queued actors");}
 {Fixture x(false);x.victory();require(calls.size()==2&&calls[0].fn=="retire-active"&&calls[1].fn=="controls","active cinematic uses447C8 then controls");}
 for(unsigned fault=0;fault<8;++fault){Fixture x;if(fault==0)x.w(0x80020000+46*4,0);if(fault==1)x.half(0x800F882C+6,0);if(fault==2)x.half(Fixture::transition+0x17E,150);if(fault==3)x.w(Fixture::transition+0x188+19*4,Fixture::egg1);if(fault==4)x.by(Fixture::transition+0x17D,3);if(fault==5)x.half(0x800F882C+2,150);if(fault==6)x.half(0x800F882C+4,150);if(fault==7){x.half(0x800F882C+2,45);x.half(0x800F882C+4,48);x.half(0x800F882C+6,3);x.w(0x80020000+45*4,0x80021100);}x.victory();require(calls.empty()&&r::session.snapshot().phase==s::Phase::failed,"bad pending slot/count/index/child/phase/cursors/internal endpoint rejected");}
 {Fixture x;x.victory(false);require(r::session.snapshot().phase==s::Phase::failed,"missing pre-native running capture rejected");}
 {Fixture x;std::array<uint32_t,20> children{};x.w(Fixture::player+0x188+19*4,Fixture::egg1);require(!r::check_children(x.rdram,Fixture::player,children),"duplicate child rejected across20 sparse slots");}
 {Fixture x;std::array<uint32_t,6> eggs{};require(r::read_reserves(x.rdram,Fixture::player,eggs)&&eggs[0]==Fixture::egg1&&eggs[1]==Fixture::egg2,"real private reserve ring accepted");x.w(Fixture::player_data+0x11FC,Fixture::egg1);require(!r::read_reserves(x.rdram,Fixture::player,eggs),"same reserve cannot occur twice");}
 for(unsigned fault=0;fault<4;++fault){Fixture x;x.victory();x.spawn();if(fault==0)x.w(Fixture::egg2+0x180,Fixture::root);if(fault==1)x.by(Fixture::player_data+0x12C8,7);if(fault==2)x.half(Fixture::player_data+0x12BE,8);if(fault==3)x.w(0x800F8DB0,8);require(x.use_pot()&&r::session.snapshot().phase==s::Phase::failed,"invalid carry is blocked before original5D320");}
 {Fixture x;x.victory();pot_resource=0x3FF;x.spawn();require(r::session.snapshot().phase==s::Phase::failed&&r::victory_pot==0,"missing native pot resource cannot be ready");}
 {Fixture x;x.victory();pot_allocation_success=false;x.spawn();require(r::session.snapshot().phase==s::Phase::failed&&r::victory_pot==0,"failed native pot allocation cannot be ready");}
 {Fixture x;x.victory();pot_activation_success=false;x.spawn();require(r::session.snapshot().phase==s::Phase::failed,"failed native pot registration cannot be ready");}
 {Fixture x;x.ready();r::enter_room(d::source_room);x.c.r4=b::base(Fixture::info);r::room_ready(x.rdram,&x.c);const auto before=x.memory;const auto old_context=x.c;const auto count=calls.size();r::frame(x.rdram,&x.c,210);r::frame(x.rdram,&x.c,2010);require(r::session.snapshot().phase==s::Phase::departing,"native release allowed exactly1800VI");r::frame(x.rdram,&x.c,2011);require(r::session.snapshot().phase==s::Phase::failed&&r::session.snapshot().failure=="native-rest-arrival-timeout"&&x.memory==before&&x.context(old_context)&&calls.size()==count,"unreleased transport times out without native calls or player rewrites");}
 {Fixture x;x.ready();x.arrive(true);require(r::session.snapshot().phase==s::Phase::failed&&!r::finished,"different globalYoshi cannot impersonate native carried arrival");}
 for(unsigned fault=0;fault<3;++fault){Fixture x;x.ready();if(fault==0)x.w(0x800F8DB4+2*4,6);if(fault==1)x.w(Fixture::player_data+0x11FC,0);if(fault==2)x.w(Fixture::tongue+0x180,0);x.arrive();require(r::session.snapshot().phase==s::Phase::failed,"changed health/reserve/functionalchild cannot pass carryproof");}
 // Native pot entry is stateA/2, not a death or a normal living actor.
 {Fixture x;x.victory();x.spawn();require(x.use_pot(false)&&r::session.snapshot().phase==s::Phase::failed,"alive player without actual native transport cannot forge a pot departure");}
 for(unsigned fault=0;fault<11;++fault){Fixture x;x.victory();x.spawn();x.transport();require(!b::alive_player(x.rdram,Fixture::player)&&r::pot_passenger(x.rdram,Fixture::player),"transport exception is local; original alive_player remains strict");
  if(fault==0)x.actor(Fixture::player,0x4004);if(fault==1)x.w(0x800F8828,Fixture::root);if(fault==2)x.w(0x800F8DB4+2*4,0);if(fault==3)x.by(Fixture::player+0x18,1);if(fault==4)x.by(Fixture::player+0x30,0x11);if(fault==5)x.w(Fixture::player+0x20,0x80043450);if(fault==6)x.w(Fixture::player_data+0xD0,Fixture::root);if(fault==7)x.by(Fixture::player+0x165,0);if(fault==8)x.by(Fixture::player+0x17D,2);if(fault==9)x.w(Fixture::player+0x180,pot);if(fault==10)x.w(Fixture::player_data+0xC0,0x8002C3B4);
  require(!r::pot_passenger(x.rdram,Fixture::player)&&x.use_pot(false)&&r::session.snapshot().phase==s::Phase::failed,"pot transport requires exact identity, state, callback, pot owner, HP and native detachment");
 }
 {Fixture x;x.ready();r::enter_room(d::source_room);const auto old=x.c;x.c.r4=b::base(Fixture::info);r::room_ready(x.rdram,&x.c);x.c=old;const auto count=calls.size();for(unsigned stage=0;stage<4;++stage){if(stage==1)x.by(Fixture::player+0x17D,2);if(stage==2)x.by(Fixture::player+0x165,0);if(stage==3){x.by(Fixture::player+0x18,1);x.by(Fixture::player+0x30,0x11);}const auto before=x.memory;r::frame(x.rdram,&x.c,210+stage);require(r::session.snapshot().phase==s::Phase::departing&&x.memory==before&&x.context(old)&&calls.size()==count,"partial release or death-state11 waits; observer never forces playable arrival");}}
 // Dynamic exit placement and native persistence apply only to this owned pot.
 for(const auto [player_x,want]:std::array<std::pair<float,float>,9>{{{40,120},{500,580},{700,724},{-100,40},{-40,40},{644,724},{0,80},{std::numeric_limits<float>::max(),724},{std::numeric_limits<float>::lowest(),40}}}){
  Fixture x;x.w(Fixture::player+0x84,std::bit_cast<uint32_t>(player_x));expected_pot_x=want;const auto memory_before=x.memory;const auto helper=r::nearby_pot_x(x.rdram,Fixture::player);require(helper&&*helper==want&&x.memory==memory_before&&calls.empty(),"nearby pot helper is readonly and clamps finite playerX+80 to40..724");
  x.victory();const auto player_bounds=b::word(x.rdram,Fixture::player+0xC0),other_bounds=b::word(x.rdram,0x80021000+0xC0);x.spawn();require(r::victory_pot==pot&&r::victory_pot_x==want,"spawn stores actual nearby pot position");require(b::word(x.rdram,pot+0xC0)==0x7FFF7FFF&&b::word(x.rdram,Fixture::player+0xC0)==player_bounds&&b::word(x.rdram,0x80021000+0xC0)==other_bounds,"native persistence sentinel changes only owned pot bounds");require(calls[4].a1==std::bit_cast<uint32_t>(want),"native allocator receives clamped position not old640");
 }
 for(const float invalid:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()}){
  Fixture x;x.victory();calls.clear();x.w(Fixture::player+0x84,std::bit_cast<uint32_t>(invalid));const auto memory_before=x.memory;require(!r::nearby_pot_x(x.rdram,Fixture::player),"nonfinite player position has no exit candidate");x.spawn();require(r::session.snapshot().phase==s::Phase::failed&&r::session.snapshot().failure=="invalid-victory-pot-position"&&!r::victory_pot&&r::victory_pot_x==0&&calls.empty()&&x.memory==memory_before,"nonfinite spawn refuses native calls and all RAM mutations");
 }
 for(unsigned invalid=0;invalid<4;++invalid){
  Fixture x;uint32_t candidate=Fixture::player;if(invalid==0)candidate=0;if(invalid==1)x.actor(candidate,0x4004);if(invalid==2)x.by(candidate+0x18,0xA);if(invalid==3)x.by(candidate+0x30,0x11);const auto memory_before=x.memory;require(!r::nearby_pot_x(x.rdram,candidate)&&x.memory==memory_before&&calls.empty(),"nearby exit requires exact living player");
 }
 {Fixture x;x.victory();x.by(Fixture::player+0x18,0xA);calls.clear();const auto memory_before=x.memory;x.spawn();require(r::session.snapshot().phase==s::Phase::failed&&!r::victory_pot&&calls.empty()&&x.memory==memory_before,"player dying after victory cannot spawn an exit using stale position");}
 // QA must hop onto a native pot rim, then press down only for the bound parent.
 {Fixture x;x.victory();x.spawn();r::driver=true;
  const auto drive=[&](unsigned vi,uint16_t want,float wx,float wy,const char* message){const auto memory_before=x.memory;const auto context_before=x.c;const auto call_count=calls.size();uint16_t buttons=0xFFFF;float sx=1,sy=1;r::drive_input(x.rdram,vi,&buttons,&sx,&sy);require(buttons==want&&sx==wx&&sy==wy&&r::victory_pot_x==120,message);require(x.memory==memory_before&&x.context(context_before)&&calls.size()==call_count,"QA pot navigation only changes controller outputs");};
  for(const auto [player_x,want_x]:std::array<std::pair<float,float>,8>{{{40,0.65f},{180,-0.65f},{120,0},{114,0},{126,0},{113,0.65f},{127,-0.65f},{101,0.65f}}}){x.w(Fixture::player+0x84,std::bit_cast<uint32_t>(player_x));drive(300,0,want_x,0,"QA aligns within6 but does not crouch on ground");}
  for(const auto [player_x,want]:std::array<std::pair<float,uint16_t>,5>{{{95,0},{96,0x8000},{120,0x8000},{144,0x8000},{145,0}}}){x.w(Fixture::player+0x84,std::bit_cast<uint32_t>(player_x));drive(240,want,player_x<114?0.65f:player_x>126?-0.65f:0,0,"QA jumps only inside24-pixel approach window");}
  x.w(Fixture::player+0x84,std::bit_cast<uint32_t>(120.0f));for(unsigned vi:{269u,270u,359u,360u})drive(vi,vi%120<30?0x8000:0,0,0,"QA holdsA30VI then releases90VI before another edge");
  x.w(Fixture::player+0x180,pot);drive(240,0,0,-0.9f,"exact native parent4273 enters throughDown instead of another jump");
  x.actor(0x80024000,0x4273);x.w(Fixture::player+0x180,0x80024000);drive(240,0x8000,0,0,"another4273 cannot impersonate bound victory exit");
  x.w(Fixture::player+0x180,pot);x.actor(pot,0x4274);drive(240,0x8000,0,0,"wrong native parent kind cannot triggerDown");x.actor(pot,0x4273);
  for(unsigned dead=0;dead<2;++dead){x.by(Fixture::player+(dead?0x30:0x18),dead?0x11:0xA);uint16_t buttons=0x5555;float sx=0.25f,sy=0.5f;const auto before=x.memory;r::drive_input(x.rdram,240,&buttons,&sx,&sy);require(buttons==0x5555&&sx==0.25f&&sy==0.5f&&x.memory==before,"dead/dyingYoshi is never driven");x.by(Fixture::player+(dead?0x30:0x18),0);}
  r::driver=false;uint16_t buttons=0x1234;float sx=0.2f,sy=0.3f;const auto before=x.memory;r::drive_input(x.rdram,240,&buttons,&sx,&sy);require(buttons==0x1234&&sx==0.2f&&sy==0.3f&&x.memory==before,"manual version receives no QA pot controls");
  require(r::configure("new-nearby-run",false,false)&&!r::victory_pot&&r::victory_pot_x==0,"new run clears old exit identity and stored coordinate");
 }
 {Fixture x;x.ready();x.arrive();r::driver=true;x.w(Fixture::player+0x84,std::bit_cast<uint32_t>(d::pot_x));const auto drive=[&](unsigned vi,uint16_t want,float wy,const char* message){const auto before=x.memory;uint16_t buttons=0xFFFF;float sx=1,sy=1;r::drive_input(x.rdram,vi,&buttons,&sx,&sy);require(buttons==want&&!sx&&sy==wy&&x.memory==before,message);};
  drive(509,0,0,"rest driver waits300VI without moving or jumping");drive(600,0x8000,0,"rest onwardpot also needs a normal jump beforeDown");drive(630,0,0,"rest onwardpot releasesA after30VI");
  x.actor(0x80024000,0x4274);x.w(Fixture::player+0x180,0x80024000);drive(600,0,-0.9f,"native rest parent4274 permitsDown without fixed actor identity");
  x.actor(0x80024000,0x4273);drive(600,0x8000,0,"arena-kind parent is not accepted for rest onwardpot");
 }
 std::cout<<"Boss solo runtime: "<<checks<<" assertions passed; voluntary pot flow and native-arrival observation, ABI stubs and synthetic RAM only."<<std::endl;return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 1;}}
