#include "../boss_rest_floor0260.hpp"
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
namespace rf=workshop::boss_rest_floor0260;namespace n=workshop::boss_native;namespace data=workshop::boss_rest_data0240;namespace fp=workshop::boss_floor_persistence0260;
static unsigned checks=0,retirements=0,calls=0;static uint32_t allocator_result=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
extern "C" void do_break(uint32_t){std::abort();}
static int32_t sections[300]{};int32_t* section_addresses=sections;
static int32_t dinotale_actor_screen_x(int32_t x){return x;}
#undef LOOKUP_FUNC
#define LOOKUP_FUNC(v) lookup(int32_t(v))
namespace native_floor {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include "native-floor.inc"
#pragma clang diagnostic pop
void func_80045A34(uint8_t*,recomp_context* c){need(c->r5==12,"Real floor initializer allocates12 private bytes");c->r2=n::base(allocator_result);}
void func_80045BD8(uint8_t*,recomp_context*){++retirements;}
void func_8004700C(uint8_t*,recomp_context* c){c->r2=0;}
void func_8006F324(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected special terrain property");}
void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected native floor assertion");}
recomp_func_t* lookup(int32_t a){if(uint32_t(a)==uint32_t(sections[189])+fp::dtor_offset)return func_8069D12C;throw std::runtime_error("Unexpected floor dynamic callback");}
}
struct Fixture {
 static constexpr uint32_t table=0x80710000,actors=0x80700000,draws=0x80708000,privates=0x8070C000,stack=0x807B0000;
 std::vector<uint8_t> bytes;uint8_t* rdram;rf::Guard guard;recomp_context c{};uint32_t prototype=0,overlay=0;unsigned index;
 Fixture(const std::vector<uint8_t>& input,unsigned encounter=2):bytes(input),rdram(bytes.data()),index(encounter){
  for(uint32_t p=0x80000400;p<0x80700000;p+=4)if(n::actor(rdram,p,0x4592)&&word(p+0x1F4)&&word(p+0x1DC)==0x45920000){prototype=p;break;}
  need(prototype!=0,"Actual RAM supplies native4592 resource/drawing graph");overlay=word(word(prototype+0x168)+0x40);sections[189]=int32_t(overlay);guard.reset(83,index);c.r29=n::base(stack);c.r31=n::base(0x80000300);c.r16=0x12345678;c.r17=n::base(0x87654321);
  for(unsigned i=0;i<data::object_count;++i){for(unsigned j=0;j<3;++j)w(record(i)+j*4,workshop::u32(data::rest_objects,i*12+j*4));w(record(i)+12,0);
   if(!floor(i))continue;const auto a=actor(i),d=draws+i*0x100;std::memcpy(rdram+(a&0x7FFFFF),rdram+(prototype&0x7FFFFF),0x1FC);std::memcpy(rdram+(d&0x7FFFFF),rdram+(word(prototype+0x16C)&0x7FFFFF),0x80);w(a+0x16C,d);w(a+0x1F4,0);w(a+0x1D8,0);w(a+0x1DC,0);w(a+0x20,overlay+fp::init_offset);for(unsigned f:{0x3Cu,0x84u})w(a+f,workshop::u32(data::rest_objects,i*12+4));for(unsigned f:{0x40u,0x88u})w(a+f,workshop::u32(data::rest_objects,i*12+8));sh(a+0xC0,96);sh(a+0xC2,96);b(a+0x1E8,1);
  }
 }
 bool floor(unsigned i)const{return workshop::u16(data::rest_objects,i*12)==0x4592;}
 uint32_t record(unsigned i)const{return table+i*16;}uint32_t actor(unsigned i)const{return actors+i*0x300;}
 uint32_t word(uint32_t p)const{return n::word(rdram,p);}unsigned half(uint32_t p)const{return MEM_HU(0,n::base(p));}
 void w(uint32_t p,uint32_t v){MEM_W(0,n::base(p))=int32_t(v);}void sh(uint32_t p,unsigned v){MEM_H(0,n::base(p))=uint16_t(v);}void b(uint32_t p,unsigned v){MEM_B(0,n::base(p))=uint8_t(v);}
 unsigned checkpoint_support()const{for(unsigned i=0;i<data::object_count;++i)if(floor(i)&&workshop::u32(data::rest_objects,i*12+4)==std::bit_cast<uint32_t>(168.f)&&workshop::u32(data::rest_objects,i*12+8)==std::bit_cast<uint32_t>(472.f))return i;throw std::runtime_error("Missing actual checkpoint support source");}
 unsigned player_slot()const{for(unsigned i=0;i<data::object_count;++i)if(workshop::u32(data::rest_objects,i*12)==0x40010000)return i;throw std::runtime_error("Missing actual4001 source");}
 void call(recomp_func_t* fn,uint32_t a,uint32_t a1=0){c.r4=n::base(a);c.r5=n::base(a1);c.r6=c.r7=0;const auto sp=c.r29,s0=c.r16,s1=c.r17;fn(rdram,&c);++calls;need(c.r29==sp&&c.r16==s0&&c.r17==s1,"Native SP/S0/S1 restored");}
 void construct(unsigned i,bool pin=true,std::optional<rf::RetrySource> retry={}){const auto a=actor(i),r=record(i);c.r4=n::base(a);c.r5=n::base(r);c.r31=n::base(0x8005A050);const auto before=bytes;need(guard.capture(rdram,&c,83,index,overlay,retry)&&bytes==before,"Exact Rest source captured without writes");call(native_floor::func_80048E9C,a,r);allocator_result=privates+i*0x20;call(native_floor::func_8069D294,a);need(half(a+0xC0)==96&&half(a+0xC2)==96,"Real native init retains ordinary margins");auto entry=c;entry.r4=n::base(a);call(native_floor::func_8069D05C,a);
  if(pin){auto expected=bytes;MEM_H(0xC0,n::base(a))=32767;MEM_H(0xC2,n::base(a))=32767;expected=bytes;sh(a+0xC0,96);sh(a+0xC2,96);const auto regs=c;need(guard.pin(rdram,&entry,83,index,overlay)&&bytes==expected&&!std::memcmp(&regs,&c,sizeof(c)),"Pin after native stamp changes only two margins");}
 }
 void all(bool pin=true){for(unsigned i=0;i<data::object_count;++i)if(floor(i))construct(i,pin);}
 unsigned tile(int x,int y){call(native_floor::func_8006ABF0,unsigned(x),unsigned(y));return unsigned(c.r2)&65535;}
 void bounds(unsigned i,int camera){const auto a=actor(i);w(a+0x34,uint32_t(int(std::bit_cast<float>(workshop::u32(data::rest_objects,i*12+4)))-camera));w(a+0x38,uint32_t(int(std::bit_cast<float>(workshop::u32(data::rest_objects,i*12+8)))-256));call(native_floor::func_80048D74,a,overlay+fp::dtor_offset);}
};
int main(int argc,char** argv){try{
 need(argc==2,"Immutable actual RAM snapshot argument required");std::ifstream in(argv[1],std::ios::binary);std::vector<uint8_t> input((std::istreambuf_iterator<char>(in)),{});need(input.size()==0x800000,"Actual RAM is8MiB");const auto original=input;
 {Fixture f(input);f.all(false);const auto support=f.checkpoint_support();need((f.tile(10,29)&0x7FF)==0x601,"Real source at168/472 supplies checkpoint160 top464");const auto old=retirements;f.bounds(support,700);need(retirements==old+1&&f.tile(10,29)==0,"Actual off-screen D12C/CD84 erases checkpoint floor despite other source floors");}
 {Fixture f(input);const auto before=f.bytes;need(!f.guard.checkpoint_support_ready(f.rdram,83,f.index,f.overlay)&&f.bytes==before,"Existing bitmap pixels alone cannot attest an owned native support stamp");f.construct(f.checkpoint_support(),false);const auto stamped=f.bytes;need(!f.guard.checkpoint_support_ready(f.rdram,83,f.index,f.overlay)&&f.bytes==stamped,"Completed native stamp without its exact source pin does not release the constructor");}
 for(unsigned index:{2u,5u,8u}){Fixture f(input,index);f.all();const auto before=f.bytes;need(f.guard.checkpoint_support_ready(f.rdram,83,index,f.overlay)&&f.bytes==before,"True source-proven pinned D05C plus601/401 releases checkpoint2/3/4 read-only");need(!f.guard.checkpoint_support_ready(f.rdram,162,index,f.overlay)&&!f.guard.checkpoint_support_ready(f.rdram,83,index+1,f.overlay)&&!f.guard.checkpoint_support_ready(f.rdram,83,index,f.overlay+4),"Foreign room/visit/overlay cannot release a deferred constructor");}
 for(unsigned fault=0;fault<10;++fault){Fixture f(input);f.all();const auto a=f.actor(f.checkpoint_support());if(fault==0)f.guard.forget(a);if(fault==1)f.sh(a+0xC0,96);if(fault==2)f.w(a+0x20,f.overlay+fp::stamp_offset);if(fault==3)f.w(a+0x1D8,0);if(fault==4)f.w(a+0x1F4,0);if(fault==5)f.b(a+0x17D,0);if(fault==6)f.w(a+0x180,0x80010000);if(fault==7)f.w(0x800F8810,0);if(fault==8)f.call(native_floor::func_8069CD84,a,0);if(fault==9)f.w(a+0x168,0);const auto before=f.bytes;need(!f.guard.checkpoint_support_ready(f.rdram,83,f.index,f.overlay)&&f.bytes==before,"Missing lifetime/source/pin/map/native support tiles cannot release a constructor");}
 for(unsigned index:{0u,2u,5u,8u}){Fixture f(input,index);f.all();unsigned floors=0;for(const auto& b:f.guard.snapshot())floors+=b.pinned;need(floors==20,"All twenty exact Rest4592 sources pinned independently despite repeated ordinals");const auto top=f.tile(10,29),under=f.tile(10,30),old=retirements;need((top&0x7FF)==0x601&&(under&0x7FF)==0x401,"Native stamp supplies both checkpoint support rows");for(int camera:{0,700,0})for(unsigned i=0;i<data::object_count;++i)if(f.floor(i))f.bounds(i,camera);need(retirements==old&&f.tile(10,29)==top&&f.tile(10,30)==under,"True native far-and-back culling keeps checkpoint2/3/4 floor stable");const auto support=f.checkpoint_support();need(f.guard.owned(f.actor(support)),"Support identity remains owned");f.guard.forget(f.actor(support));need(!f.guard.owned(f.actor(support)),"Actual visit teardown releases only its exact binding");f.guard.reset(177,index);need(!f.guard.owned(f.actor(2)),"Other room ends Rest floor ownership");}
 for(unsigned fault=0;fault<13;++fault){Fixture f(input);const unsigned s=f.checkpoint_support();const auto a=f.actor(s),r=f.record(s);f.c.r4=n::base(a);f.c.r5=n::base(r);f.c.r31=n::base(0x8005A050);unsigned room=83,index=2;uint32_t overlay=f.overlay;
  if(fault==0)room=162;if(fault==1)index=3;if(fault==2)f.c.r31+=4;if(fault==3)f.w(r,0x45930000);if(fault==4)f.w(r+4,std::bit_cast<uint32_t>(160.f));if(fault==5)f.w(r+8,std::bit_cast<uint32_t>(448.f));if(fault==6)f.w(f.record(0),0);if(fault==7)overlay+=4;if(fault==8)f.w(a+0x168,0);if(fault==9)f.w(f.word(a+0x168)+0x44,overlay+4);if(fault==10)f.c.r5+=4;if(fault==11)f.c.r4+=4;if(fault==12)f.guard.reset();const auto before=f.bytes;need(!f.guard.capture(f.rdram,&f.c,room,index,overlay)&&before==f.bytes,"Wrong room/visit/source/table/caller/resource/identity rejects without writes");}
 for(unsigned fault=0;fault<8;++fault){Fixture f(input);const auto s=f.checkpoint_support();f.construct(s,false);const auto a=f.actor(s);f.c.r4=n::base(a);if(fault==0)f.w(a+0x20,f.overlay+fp::stamp_offset);if(fault==1)f.w(a+0x1D8,0);if(fault==2)f.w(a+0x1DC,0);if(fault==3)f.w(a+0x180,0x80010000);if(fault==4)f.w(a+0x174,0x80010000);if(fault==5)f.w(f.word(a+0x1F4)+8,0);if(fault==6)f.sh(f.overlay+fp::table_offset+12,0);if(fault==7)f.guard.forget(a);const auto before=f.bytes;need(!f.guard.pin(f.rdram,&f.c,83,f.index,f.overlay)&&f.bytes==before,"Only exact source-proven completed native stamp may pin Rest floor");}
 for(unsigned cp:{3u,6u,9u})for(bool consumed:{false,true}){
  Fixture f(input,cp-1);const auto player=f.word(0x800F8828),source=f.record(f.player_slot()),slot=f.checkpoint_support();
  f.w(source+4,std::bit_cast<uint32_t>(data::checkpoint_x));f.w(source+8,std::bit_cast<uint32_t>(data::checkpoint_y-16.f));
  if(consumed)f.w(source,0xFFFFFFFF);
  f.c.r4=n::base(f.actor(slot));f.c.r5=n::base(f.record(slot));f.c.r31=n::base(0x8005A050);const auto before=f.bytes;
  need(!f.guard.capture(f.rdram,&f.c,83,f.index,f.overlay)&&f.bytes==before,"A normal first-entry proof never accepts the wake-rewritten4001 source without authenticated retry context");
  const rf::SourceProof retry{player,cp,cp/3,cp,rf::SourceMode::Retry};need(rf::checkpoint_player_record(f.rdram,0x40010000,f.word(source+4),f.word(source+8),83,f.index,retry)&&f.bytes==before,"Shared retry matcher permits only the immutable4001 source identity and actual native48C2C coordinate for the exact player/checkpoint visit");
  need(!rf::retry_player_record(f.rdram,0xFFFFFFFF,f.word(source+4),f.word(source+8),83,f.index,retry),"The shared matcher never treats genericFFFFFFFF as proof of a4001 source");
  f.construct(slot,true,retry);const auto ready=f.bytes;
  need(f.guard.checkpoint_support_ready(f.rdram,83,f.index,f.overlay)&&f.bytes==ready,"Narrow retry source admission still requires the real native floor constructor/stamp/pin and actual601/401 support");
  f.guard.reset(83,f.index);f.c.r4=n::base(f.actor(slot));f.c.r5=n::base(f.record(slot));f.c.r31=n::base(0x8005A050);const auto reset=f.bytes;need(!f.guard.capture(f.rdram,&f.c,83,f.index,f.overlay)&&f.bytes==reset,"A new visit cannot inherit retry coordinate permission from a previously proved table");
 }
 for(unsigned fault=0;fault<19;++fault){
  Fixture f(input);const auto player=f.word(0x800F8828),source=f.record(f.player_slot()),slot=f.checkpoint_support();
  f.w(source+4,std::bit_cast<uint32_t>(data::checkpoint_x));f.w(source+8,std::bit_cast<uint32_t>(data::checkpoint_y-16.f));
  rf::SourceProof retry{player,3,1,3,rf::SourceMode::Retry};unsigned room=83,index=2;auto overlay=f.overlay;f.c.r4=n::base(f.actor(slot));f.c.r5=n::base(f.record(slot));f.c.r31=n::base(0x8005A050);
  if(fault==0)retry.player=0;if(fault==1)retry.player+=4;if(fault==2)retry.epoch=0;if(fault==3)retry.checkpoint=0;if(fault==4)retry.checkpoint=4;if(fault==5)retry.checkpoint=6;
  if(fault==6)room=162;if(fault==7)index=3;if(fault==8)f.w(source+4,std::bit_cast<uint32_t>(161.f));if(fault==9)f.w(source+8,std::bit_cast<uint32_t>(433.f));
  if(fault==10)f.w(source,0x40012000);if(fault==11)f.w(source,0x45070000);if(fault==12)f.w(f.record((f.player_slot()+1)%data::object_count)+4,0);
  if(fault==13)f.w(0x800F8828,0);if(fault==14)f.w(player,0);if(fault==15)f.sh(player+12,0x4004);if(fault==16)f.c.r31+=4;if(fault==17)overlay+=4;if(fault==18)f.w(f.record(slot),0x45930001);
  const auto before=f.bytes;need(!f.guard.capture(f.rdram,&f.c,room,index,overlay,retry)&&f.bytes==before,"Retry permission never admits foreign player/source/XY/packed/checkpoint/visit/caller/resource or a changed nonplayer row");
 }
 for(const auto visit:std::array<std::array<unsigned,2>,5>{{{3,4},{3,5},{6,7},{6,8},{9,9}}})for(unsigned epoch:{0u,1u})for(bool consumed:{false,true}){
  Fixture f(input,visit[1]-1);const auto player=f.word(0x800F8828),source=f.record(f.player_slot()),slot=f.checkpoint_support();
  f.w(source+4,std::bit_cast<uint32_t>(data::checkpoint_x));f.w(source+8,std::bit_cast<uint32_t>(data::checkpoint_y-16.f));if(consumed)f.w(source,0xFFFFFFFF);
  const rf::SourceProof normal{player,visit[0],epoch,visit[1],rf::SourceMode::Normal};const auto before=f.bytes;
  need(rf::checkpoint_player_record(f.rdram,0x40010000,f.word(source+4),f.word(source+8),83,f.index,normal)&&f.bytes==before,"Authenticated native wake coordinates persist on later normal Rest visits, including epoch0 with no previous defeat");
  f.construct(slot,true,normal);const auto ready=f.bytes;need(f.guard.checkpoint_support_ready(f.rdram,83,f.index,f.overlay)&&f.bytes==ready,"Normal cached-room source admission preserves actual native floor stamping and support readiness");
 }
 for(unsigned fault=0;fault<8;++fault){
  Fixture f(input,3);const auto player=f.word(0x800F8828),source=f.record(f.player_slot()),slot=f.checkpoint_support();
  f.w(source+4,std::bit_cast<uint32_t>(data::checkpoint_x));f.w(source+8,std::bit_cast<uint32_t>(data::checkpoint_y-16.f));
  rf::SourceProof proof{player,3,0,4,rf::SourceMode::Normal};unsigned index=3;f.c.r4=n::base(f.actor(slot));f.c.r5=n::base(f.record(slot));f.c.r31=n::base(0x8005A050);
  if(fault==0)proof.mode=rf::SourceMode::None;if(fault==1)proof.mode=static_cast<rf::SourceMode>(99);if(fault==2)proof.checkpoint=6;if(fault==3)proof.completed=5;
  if(fault==4){proof.completed=10;index=9;f.guard.reset(83,index);}if(fault==5)proof.checkpoint=0;if(fault==6){proof.mode=rf::SourceMode::Retry;proof.epoch=1;}if(fault==7)proof=rf::RetrySource{player,3,1};
  const auto before=f.bytes;need(!f.guard.capture(f.rdram,&f.c,83,index,f.overlay,proof)&&f.bytes==before,"None/unknown mode, future/mismatched checkpoint/visit, finalRest10 or legacy untyped retry aggregate cannot authorize cached source coordinates");
 }
 need(input==original,"Actual captured input snapshot unchanged");std::cout<<"BossRestFloor0260Checks: "<<checks<<" PASS; nativeCalls="<<calls<<" nativeRetirements="<<retirements<<"; native floor CPU oracle only, no GPU or actual checkpoint gameplay claim.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
