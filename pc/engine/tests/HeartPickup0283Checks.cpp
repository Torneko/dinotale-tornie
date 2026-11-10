#include "heart_pickup0283.hpp"
#include <bit>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
using namespace workshop;
namespace h=heart_pickup0283;namespace s=heart_sources0283;
static unsigned checks=0,observations=0,native_collections=0,native_factories=0;
static int32_t sections[300]{};int32_t* section_addresses=sections;
static void need(bool yes,const char* why){++checks;if(!yes)throw std::runtime_error(why);}
struct Fixture;static Fixture* current=nullptr;
static void hook(uint8_t*,recomp_context*,const char*);
#undef LOOKUP_FUNC
#define LOOKUP_FUNC(v) lookup(int32_t(v))
#define WORKSHOP_SPECIAL_CONTENT
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wmicrosoft-goto"
namespace native {
#include "native.inc"
}
#pragma clang diagnostic pop
static std::vector<uint8_t> read(const std::filesystem::path& p){std::ifstream in(p,std::ios::binary);need(bool(in),"ROM CPU fixture exists");return {(std::istreambuf_iterator<char>(in)),{}};}
static uint32_t be(const std::vector<uint8_t>& b,size_t at){need(at+4<=b.size(),"Fixture bounded word");return uint32_t(b[at])<<24|uint32_t(b[at+1])<<16|uint32_t(b[at+2])<<8|b[at+3];}
struct Fixture {
 static inline std::filesystem::path overlays;
 static constexpr uint32_t metadata=0x80010000,table=0x80011000,player=0x80100000,source=0x80102000,stack=0x807B0000,scene=0x80500000,api=0x80501000;
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* rdram=memory.data();h::Observer observer;h::Frame frame{0x80200000,0x80300000,0x80400000,player};
 recomp_context c{};uint32_t next=0x80120000,last_wrapper=0,last_heart=0;const s::Source* selected=nullptr;std::filesystem::path directory;bool enabled=true,observe=true;unsigned fault=0;
 Fixture(const std::filesystem::path& dir):directory(dir){
  current=this;sections[134]=int32_t(frame.bubble_overlay);sections[220]=int32_t(frame.heart_overlay);sections[227]=int32_t(frame.hidden_overlay);
  sections[2]=int32_t(0x80600000);
  for(unsigned sec:{2u,134u,220u,227u}){auto bytes=read(overlays/("overlay-"+std::to_string(sec)+".bin"));for(unsigned i=0;i<bytes.size();i+=4)w(uint32_t(sections[sec])+i,be(bytes,i));}
  init_actor(player,0x4001);w(0x800F8828,player);w(0x800F8DB0,0);w(0x800F8DB4,4);w(0x800F8B88,1);w(0x800F8D38,frame.heart_overlay+0xC30);
  w(0x800F8908,scene);w(scene+0x10,api);w(api+0x18,0x80700000);w(0x800F8808,0x80503000);sh(0x800F8802,0);sh(0x80503000,0);b(0x800A05B4,1);
  c.r29=h::base(stack);c.r31=h::base(0x80000300);
 }
 uint32_t word(uint32_t a)const{return h::word(const_cast<uint8_t*>(rdram),a);}void w(uint32_t a,uint32_t value){MEM_W(0,h::base(a))=int32_t(value);}void sh(uint32_t a,unsigned value){MEM_H(0,h::base(a))=uint16_t(value);}void b(uint32_t a,unsigned value){MEM_B(0,h::base(a))=uint8_t(value);}
 void init_actor(uint32_t a,uint16_t type){for(unsigned i=0;i<0x1000;i+=4)w(a+i,0);w(a,0x19971110);sh(a+12,type);b(a+0x17D,2);sh(a+0x17E,150);w(a+0x168,a+0x500);w(a+0x16C,a+0x600);w(a+0x174,a+0x700);w(a+0x700,a+0x800);}
 void call(recomp_func_t* fn,uint32_t a,uint32_t a1=0,uint32_t a2=0,uint32_t a3=0){c.r4=h::base(a);c.r5=h::base(a1);c.r6=h::base(a2);c.r7=h::base(a3);const auto sp=c.r29;fn(rdram,&c);need(c.r29==sp,"Actual native SP restored");}
 bool enter(const s::Source& item,bool allowed=true){selected=&item;auto bytes=read(directory/("room-"+std::to_string(item.room)+".bin"));const auto count=be(bytes,0);sh(metadata,count);w(metadata+4,table);for(unsigned i=0;i<count;++i)for(unsigned j=0;j<3;++j)w(table+i*16+j*4,be(bytes,4+i*12+j*4));b(0x800F8DFC,item.level/4);b(0x800F8DFD,item.level%4);return observer.enter(rdram,item.level,item.room,metadata,allowed);}
 void place(const s::Source& item){need(enter(item,enabled)==enabled,"Actual ROM table admitted only in allowed Trial");observer.created(source);init_actor(source,item.kind);w(source+0x3C,item.x);w(source+0x84,item.x);w(source+0x40,item.y);w(source+0x88,item.y);w(source+0x44,0x43FA30A4);w(source+0x8C,0x43FA30A4);call(native::func_80048BD8,source,table+item.index*16);call(native::func_8006D340,table+item.index*16);}
 uint32_t build(const s::Source& item){
  place(item);
  if(s::direct(item.kind)){last_heart=source;call(native::func_806E7468,source);}
  else if(s::bubble(item.kind)){
   call(native::func_804001DC,source);need(last_wrapper!=0,"Real native source constructor creates41CE");
   need(h::byte(rdram,last_wrapper+0x30)==7&&word(last_wrapper+0x180)==source,"Real native container selects heart7 and attaches wrapper");
   b(last_wrapper+0x17D,2);call(native::func_8059EA78,last_wrapper);
   need(MEM_HU(0,h::base(word(last_wrapper+0x1F4)))==0x40C5,"Real wrapper constructor resolves native table7 to40C5");
   call(native::func_80048060,last_wrapper);need(word(last_wrapper+0x180)==0,"Real native detach precedes release");
   call(native::func_8059EC8C,last_wrapper);
  }else{need(item.kind==0x464C,"Known hidden source");call(native::func_806F55E8,source);}
  need(last_heart!=0,"Actual source/factory produced real red heart");b(last_heart+0x17D,2);return last_heart;
 }
 void contact(uint32_t heart){call(native::func_806E7574,heart,player,heart+0xA00,heart+0xA10);need(h::byte(rdram,heart+0x18)==3,"Actual native contact selects collect state3");}
 void collect(uint32_t heart){++native_collections;call(native::func_806E7728,heart);need(word(0x800F8958)==1&&word(0x800F8DB4)==8,"Actual native counter increment and heal remain normal");need(h::byte(rdram,heart+0x17D)==3,"Actual permanent retirement completed");need(word(0x800F8944)==0,"Native red heart never changes Lucky Tree SuperHappy timer");}
 void raw(const char* name,uint32_t a,uint32_t a1=0,uint32_t ra=0){const auto old=c;c.r4=h::base(a);c.r5=h::base(a1);c.r31=h::base(ra);hook(rdram,&c,name);c=old;}
};
static void hook(uint8_t* r,recomp_context* c,const char* name){
 need(current&&r==current->rdram,"Native trace fixture owner");if(!current->observe)return;++observations;
 const auto ram=current->memory;const auto registers=*c;
 auto copy=*c;auto frame=current->frame;std::string_view n=name;
 if(current->fault==1&&n=="func_806E7E70")copy.r31+=4;
 if(current->fault==2&&n=="func_80049624")copy.r31+=4;
 if(current->fault==3&&n=="func_8002B3A0")frame.player+=4;
 if(current->fault==4&&n=="func_80044658")return;
 if(current->fault==5&&n=="func_8002B428")return;
 const auto witnessed=copy;current->observer.trace(r,&copy,name,frame);
 need(current->memory==ram&&!std::memcmp(c,&registers,sizeof(*c))&&!std::memcmp(&copy,&witnessed,sizeof(copy)),"Observer preserves all8MiB RAM and every native register");
}
namespace native {
void workshop_trace_entry(uint8_t* r,recomp_context* c,const char* name){hook(r,c,name);}
void func_80047090(uint8_t*,recomp_context* c){c->r2=1;}
void func_8004613C(uint8_t* r,recomp_context* c){
 const auto packed=uint32_t(c->r4);const auto type=uint16_t(packed>>16);const auto a=current->next;current->next+=0x1000;current->observer.created(a);current->init_actor(a,type);current->w(a+0xC,packed);current->w(a+0x3C,uint32_t(c->r5));current->w(a+0x40,uint32_t(c->r6));current->w(a+0x44,uint32_t(c->r7));
 if(type==0x41CE)current->last_wrapper=a;if(type==0x40C5){current->last_heart=a;++native_factories;}
 c->r2=h::base(a);(void)r;
}
void func_80044194(uint8_t*,recomp_context* c){current->b(uint32_t(c->r4)+0x17D,1);c->r2=1;}
void func_80045A34(uint8_t*,recomp_context* c){c->r2=h::base(uint32_t(c->r4)+0x900);}
void func_80057F74(uint8_t* r,recomp_context* c){
 need(uint32_t(c->r5)==1&&uint32_t(c->r6)==0x80043E88&&uint32_t(c->r7)==0x80044658,"Real permanent retirement walker selects44658");
 const auto a=h::word(r,uint32_t(c->r4));const auto saved=*c;c->r4=h::base(a);func_80044658(r,c);*c=saved;c->r2=1;
}
void func_80043D34(uint8_t*,recomp_context* c){c->r2=1;}
void func_80067930(uint8_t*,recomp_context* c){c->r2=0;}
void func_80067E68(uint8_t*,recomp_context* c){c->r2=0;}
void func_80049318(uint8_t*,recomp_context* c){c->r2=0;}
void func_800492C0(uint8_t*,recomp_context* c){c->r2=0;}
int workshop_find_hidden(uint8_t*,recomp_context*,uint32_t){return 0;}
int workshop_egg_bubble_gate(uint8_t*,recomp_context*){return 0;}
int workshop_find_release(uint8_t*,recomp_context*){return 0;}
int workshop_hybrid_active(){return 0;}
uint32_t workshop_hybrid_lucky_object(){return 0x4035;}
void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Native fault reached");}
void particle(uint8_t*,recomp_context* c){c->r2=0;}
recomp_func_t* lookup(int32_t address){const auto a=uint32_t(address);if(a==current->frame.heart_overlay+0xC30)return func_806E7E70;if(a==0x80700000)return particle;return particle;}
#include "providers.inc"
}
int main(int argc,char** argv){try{
 need(argc==3,"Disposable fixture and overlay directories required");const std::filesystem::path directory=argv[1];Fixture::overlays=argv[2];std::set<std::pair<unsigned,unsigned>> received;
 for(const auto& item:s::sources){Fixture f(directory);const auto heart=f.build(item);need(f.observer.drain().empty(),"Creation/reveal/release cannot credit wallet");f.contact(heart);need(f.observer.drain().empty(),"Contact alone cannot credit wallet");f.collect(heart);auto receipts=f.observer.drain();need(receipts.size()==1&&receipts[0].level==item.level&&receipts[0].slot==item.slot&&receipts[0].room==item.room,"Native collection identifies exact physical source among72");received.emplace(item.level,item.slot);f.raw("func_806E7728",heart);need(f.observer.drain().empty(),"Same-birth duplicate cannot credit twice");}
 need(received.size()==72&&native_factories==17,"All72 identities and all17 dynamic hearts proved");
 // Reverse static enumeration proves that slot assignment is independent of
 // the order in which the player finds physical hearts.
 for(auto i=s::sources.rbegin();i!=s::sources.rend();++i){Fixture f(directory);const auto heart=f.build(*i);f.contact(heart);f.collect(heart);const auto receipts=f.observer.drain();need(receipts.size()==1&&receipts[0].slot==i->slot&&received.contains({receipts[0].level,receipts[0].slot}),"Replay/reverse order retains exact identity");}
 for(unsigned fault=1;fault<=5;++fault){Fixture f(directory);f.fault=fault;const auto& item=s::sources[0];const auto heart=f.build(item);f.contact(heart);f.collect(heart);need(f.observer.drain().empty(),"Wrong factory/counter RA, missing retirement/heal or foreign P1 cannot credit");}
 {Fixture f(directory);f.enabled=false;const auto heart=f.build(s::sources[1]);f.contact(heart);f.collect(heart);need(f.observer.drain().empty(),"Disallowed Story/editor/Boss scope retains native gameplay without currency");}
 {Fixture f(directory);const auto heart=f.build(s::sources[1]);f.observer.created(heart);f.contact(heart);f.collect(heart);need(f.observer.drain().empty(),"Address recycling invalidates old physical binding");}
 {Fixture f(directory);const auto heart=f.build(s::sources[1]);f.observer.forget(heart);f.contact(heart);f.collect(heart);need(f.observer.drain().empty(),"Cull/forget never grants receipt");}
 {Fixture f(directory);const auto heart=f.build(s::sources[1]);f.call(native::func_806E7E4C,heart,3);f.collect(heart);need(f.observer.drain().empty(),"Injected state3 without actual contact cannot credit");}
 {Fixture f(directory);const auto heart=f.build(s::sources[1]);f.b(heart+0x17D,1);f.contact(heart);f.b(heart+0x17D,2);f.collect(heart);need(f.observer.drain().empty(),"Queued contact cannot claim before native promotion");}
 {Fixture f(directory);need(f.enter(s::sources[1]),"Allowed Trial context for unknown dynamic source");f.call(native::func_806E7E70,Fixture::player+0x3C);need(f.last_heart!=0,"Actual unknown factory still spawns healing heart");f.b(f.last_heart+0x17D,2);f.contact(f.last_heart);f.collect(f.last_heart);need(f.observer.drain().empty(),"Unknown/BossRest-like dynamic heart never mints physical Trial currency");}
 for(unsigned corrupt=0;corrupt<5;++corrupt){Fixture f(directory);const auto& item=s::sources[1];need(f.enter(item),"Valid native table before negative");if(corrupt==0)f.w(Fixture::table+item.index*16,item.packed^1);if(corrupt==1)f.w(Fixture::table+item.index*16+4,item.x+1);if(corrupt==2)f.w(Fixture::metadata+4,0x807FFFF0);if(corrupt==3)f.sh(Fixture::metadata,s::count(item.room)-1);const auto ram=f.memory;need(!f.observer.enter(f.rdram,item.level,corrupt==4?14:item.room,Fixture::metadata,true)&&ram==f.memory,"Foreign signature/count/room/table rejected with no RAM changes");}
 std::cout<<"HeartPickup0283Checks: "<<checks<<" PASS; sources72; dynamics17+17replay; aliases55+55replay; nativeCollections="<<native_collections<<"; readOnlyObservations="<<observations<<"; no game/save/GPU.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
