#include "../boss_pot_aim0282.hpp"
#include <bit>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace h=workshop::boss_pot_aim0282;namespace n=workshop::boss_native;
static unsigned checks=0,calls=0;static void need(bool x,const char* why){++checks;if(!x)throw std::runtime_error(why);}
struct GuestFault {uint32_t address;};
template<class T> static T& checked(uint8_t* r,uint64_t a,uint64_t b,unsigned flip){const auto address=uint32_t(a+b);if(address<0x80000000||address>0x80800000-sizeof(T))throw GuestFault{address};return *reinterpret_cast<T*>(r+((address-0x80000000)^flip));}
#undef MEM_W
#undef MEM_H
#undef MEM_HU
#undef MEM_B
#undef MEM_BU
#define MEM_W(a,b) checked<int32_t>(rdram,uint64_t(a),uint64_t(b),0)
#define MEM_H(a,b) checked<int16_t>(rdram,uint64_t(a),uint64_t(b),2)
#define MEM_HU(a,b) checked<uint16_t>(rdram,uint64_t(a),uint64_t(b),2)
#define MEM_B(a,b) checked<int8_t>(rdram,uint64_t(a),uint64_t(b),3)
#define MEM_BU(a,b) checked<uint8_t>(rdram,uint64_t(a),uint64_t(b),3)
#undef LOOKUP_FUNC
#define LOOKUP_FUNC(v) lookup(int32_t(v))
namespace actual {
#include POT_AIM_NATIVE_DECLS
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include POT_AIM_NATIVE_SLICE
#pragma clang diagnostic pop
#include POT_AIM_NATIVE_STUBS
recomp_func_t* lookup(int32_t a){if(uint32_t(a)==0x80046E64)return func_80046E64;throw std::runtime_error("Unexpected indirect native target");}
}
struct Fixture {
 static constexpr uint32_t player=0x80010000,p=0x80011000,pool=0x80013000,storage=0x80014000,cursor=storage+4,data=0x80015000,res=0x80016000,draw=0x80017000,pot=0x80018000,potdata=0x80019000,potres=0x8001A000,route=0x8001B000,manager=0x8001C000,table=0x8001D000,retiretable=0x8001E000,overlay=0x805AAF40,stack=0x807B0000;
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* rdram=memory.data();recomp_context c{};h::Frame f{true,workshop::boss_battle0250::Phase::Victory,player,pot,overlay,174,7};
 void w(uint32_t a,uint32_t v){checked<int32_t>(rdram,a,0,0)=int32_t(v);}void half(uint32_t a,unsigned v){checked<uint16_t>(rdram,a,0,2)=uint16_t(v);}void b(uint32_t a,unsigned v){checked<uint8_t>(rdram,a,0,3)=uint8_t(v);}void fl(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}
 void actor(uint32_t a,unsigned kind){w(a,0x19971110);half(a+12,kind);}
 Fixture(unsigned count=2,unsigned head=0){
  actor(player,0x4001);w(player+0x1F4,p);b(player+0x18,1);b(player+0x17D,4);w(0x800F8828,player);half(player+0x184,count+1);w(player+0x188,cursor);w(p+0x1F4,cursor);
  w(0x800F8810,manager);half(manager+6,1);half(manager+8,16);w(manager+0x10,table);w(table+20,player);w(0x800F8854+0x10,retiretable);half(0x800F8854+8,8);
  actor(cursor,0x4003);b(cursor-4,1);b(cursor+5,1);w(cursor+8,pool);b(cursor+0x18,1);w(cursor+0x20,0x800398CC);w(cursor+0x1F4,data);w(cursor+0x168,res);half(res+0x3C,0x4003);w(cursor+0x16C,draw);w(draw,0x8009EBE8);w(cursor+0x180,player);half(cursor+0x17E,0);w(0x800C3D50,pool);w(pool+4,1);w(pool+8,storage);
  actor(pot,0x4273);w(pot+0x1F4,potdata);w(pot+0x168,potres);w(potres+0x40,overlay);half(potres+0x3C,0x4273);w(potdata+0x260,route);fl(pot+0x88,648);
  w(0x800F8DB4,8);w(0x800F8954,1234);w(0x800F89D0,7);b(p+0x12C8,count);half(p+0x12BE,head);
  for(unsigned i=0;i<count;i++){const auto egg=0x80020000+i*0x400;actor(egg,0x4004);w(egg+0x180,player);half(egg+0x17E,i+1);w(egg+0x1EC,8+i%6);for(unsigned axis=0;axis<3;++axis)fl(egg+0x78+axis*4,1.f);w(player+0x188+(i+1)*4,egg);w(p+0x11FC+((head+i)&7)*24,egg);}
  c.r29=n::base(stack);c.r31=n::base(overlay+0x12C);c.r4=n::base(player);c.r5=n::base(pot);c.r6=std::bit_cast<uint32_t>(648.f);c.r16=0x12345678;c.r17=0x23456789;
 }
 auto dispatch(){return [&](uint32_t fn,uint8_t* r,recomp_context* state){++calls;if(fn==0x8004700C)actual::func_8004700C(r,state);else if(fn==0x80038F98)actual::func_80038F98(r,state);else throw std::runtime_error("Unexpected adapter native operation");};}
 h::Result run(){return h::cancel(rdram,&c,f,dispatch());}
};
int main(){try{
 {Fixture x;auto c=x.c;c.r4=n::base(Fixture::cursor);x.w(Fixture::table+20,0);x.half(Fixture::manager+6,0);bool fault=false;try{actual::func_800398CC(x.rdram,&c);}catch(const GuestFault& e){fault=e.address==0x1F4;}need(fault&&uint32_t(c.r2)==0,"Actual398CC->39828->4700C/46E88 reproduces precisely the NULL+1F4 read after registry withdrawal");}
 for(unsigned count:{0u,2u,6u})for(unsigned head:{0u,3u,7u}){
  Fixture x(count,head);const auto before=x.c;const auto memory_before=x.memory;const auto result=x.run();need(result.applicable&&result.ok&&result.cleaned&&result.reticle==Fixture::cursor,"Exact native cursor retirement succeeds");need(!std::memcmp(&x.c,&before,sizeof(before)),"All caller registers including SP and RA restored");need(n::word(x.rdram,Fixture::p+0x1F4)==0&&n::word(x.rdram,Fixture::player+0x188)==0&&n::byte(x.rdram,Fixture::cursor+0x17D)==3&&n::word(x.rdram,Fixture::retiretable)==Fixture::cursor,"Actual38F98/44658/48060/43D34 retirement queue and reciprocal graph effects observed");
  bool isolated=true;for(unsigned offset=0;offset<x.memory.size();++offset){if(x.memory[offset]==memory_before[offset])continue;const auto a=0x80000000u+offset;const bool allowed=(a>=Fixture::cursor&&a<Fixture::cursor+0x1FC)||(a>=Fixture::player+0x184&&a<Fixture::player+0x1D8)||(a>=Fixture::p+0x1F4&&a<Fixture::p+0x1F8)||(a>=0x800F8854&&a<0x800F8868)||(a>=Fixture::retiretable&&a<Fixture::retiretable+32)||(a>=0x800D8150&&a<0x800D8154)||(a>=Fixture::stack-0x200&&a<Fixture::stack);if(!allowed){isolated=false;break;}}need(isolated,"8MiB comparison permits only native cursor/player-link/retirement/UI/scratch effects");
  const auto after=x.memory;const auto previous_calls=calls;const auto again=x.run();need(again.ok&&!again.cleaned&&calls==previous_calls&&x.memory==after,"No cursor is an idempotent no-op without native calls or guest writes");
 }
 {Fixture x;x.w(Fixture::table+20,0);x.half(Fixture::manager+6,0);x.half(0x800F8840+6,1);x.half(0x800F8840+8,8);x.w(0x800F8840+0x10,0x80030000);x.w(0x80030000,Fixture::player);const auto result=x.run();need(result.ok&&result.cleaned,"Actual native fallback through pending tree resolves the genuine player without fabricated V0");}
 {Fixture x;x.f.phase=workshop::boss_battle0250::Phase::Rest;x.f.room=83;x.half(Fixture::pot+12,0x4274);x.half(Fixture::potres+0x3C,0x4274);x.w(Fixture::player+0x180,Fixture::pot);x.b(Fixture::player+0x165,1);const auto result=x.run();need(result.ok&&result.cleaned&&n::word(x.rdram,Fixture::player+0x180)==Fixture::pot&&n::byte(x.rdram,Fixture::player+0x165)==1,"4274 uses the same native144 entry; unrelated player parent/hide flags preserved");}
 {Fixture x;const auto result=h::cancel(x.rdram,&x.c,x.f,[&](uint32_t fn,uint8_t* r,recomp_context* c){if(fn==0x8004700C)actual::func_8004700C(r,c);});need(result.applicable&&!result.ok&&!result.cleaned&&std::string_view(result.reason)=="native_cursor_retirement","Unknown native no-op is rejected rather than acknowledged as cleanup");}
 const std::vector<std::function<void(Fixture&)>> bad{
  [](auto& x){x.w(Fixture::cursor+0x180,0);},[](auto& x){x.half(Fixture::cursor+12,0x4005);},[](auto& x){x.b(Fixture::cursor+5,2);},[](auto& x){x.w(Fixture::cursor+8,0);},[](auto& x){x.w(Fixture::pool+4,33);},[](auto& x){x.w(Fixture::pool+8,Fixture::storage+4);},[](auto& x){x.w(Fixture::draw,0);},[](auto& x){x.half(Fixture::res+0x3C,0x4002);},[](auto& x){x.w(Fixture::cursor+0x20,0x80038FEC);},[](auto& x){x.b(Fixture::cursor+0x18,3);},[](auto& x){x.b(Fixture::cursor+0x17D,3);},[](auto& x){x.half(Fixture::cursor+0x184,1);},[](auto& x){x.w(Fixture::cursor+0x174,0x80030000);},[](auto& x){x.w(Fixture::p+0x12EC,1);},[](auto& x){x.b(Fixture::p+0x155,1);},[](auto& x){x.w(Fixture::potres+0x40,0);},[](auto& x){x.w(Fixture::potdata+0x260,0);},[](auto& x){x.w(0x800F8828,0);},[](auto& x){x.b(Fixture::p+0x12C8,7);},[](auto& x){x.c.r29=0x400;},[](auto& x){x.w(0x800F8854+0x10,0);},[](auto& x){x.half(0x800F8854+8,151);},[](auto& x){x.half(0x800F8854+4,9);},[](auto& x){x.w(Fixture::retiretable,1);},[](auto& x){x.half(0x800F8854+6,8);},[](auto& x){x.w(0x800F8810,0);},[](auto& x){x.w(Fixture::p+0x1F4,Fixture::cursor);x.w(Fixture::player+0x1F4,0x807FED30);}
 };
 for(const auto& mutate:bad){Fixture x;mutate(x);const auto memory=x.memory;const auto c=x.c;const auto previous=calls;const auto result=x.run();need(result.applicable&&!result.ok&&!result.cleaned,"Malformed cursor/pot/player/FIFO identity rejected");need(x.memory==memory&&!std::memcmp(&x.c,&c,sizeof(c))&&calls==previous,"Preflight rejects without native calls, context changes or guest mutation");}
 {Fixture x;x.w(Fixture::table+20,0);x.half(Fixture::manager+6,0);const auto c=x.c;const auto result=x.run();need(!result.ok&&!result.cleaned&&std::string_view(result.reason)=="native_player_lookup"&&!std::memcmp(&x.c,&c,sizeof(c))&&n::word(x.rdram,Fixture::p+0x1F4)==Fixture::cursor,"No fabricated player fallback and no cursor retirement when native lookup is proved absent");}
 for(unsigned mode=0;mode<5;++mode){Fixture x;if(mode==0)x.f.enabled=false;if(mode==1)x.f.phase=workshop::boss_battle0250::Phase::Fighting;if(mode==2)x.c.r31=0;if(mode==3)x.c.r5=0;if(mode==4)x.c.r7=1;const auto memory=x.memory;const auto previous=calls;const auto result=x.run();need(!result.applicable&&result.ok&&calls==previous&&x.memory==memory,"Unowned mode/phase/caller/pot request untouched");}
 {Fixture x;x.w(Fixture::p+0x1F4,0);x.b(Fixture::player+0x18,0xA);x.b(Fixture::p+0x12C8,7);const auto memory=x.memory;const auto previous=calls;const auto result=x.run();need(result.applicable&&result.ok&&!result.cleaned&&calls==previous&&x.memory==memory,"A pot with no current cursor does not acquire new state/FIFO restrictions");}
 std::cout<<"BossPotAim0282Checks: "<<checks<<" PASS; actual native NULL cursor reproduction and38F98 retirement; nativeCalls="<<calls<<"; no GPU or natural-route claim\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}catch(const GuestFault& e){std::cerr<<"Unexpected guest fault: "<<std::hex<<e.address<<'\n';return 1;}}
