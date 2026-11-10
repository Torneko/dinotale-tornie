#include "../boss_rest_heart0260.hpp"
#include <array>
#include <bit>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#ifndef BOSS_RED_HEART_NATIVE_SLICE
#error Generate the fingerprinted original40C5 slice first.
#endif
namespace h=workshop::boss_rest_heart0260;namespace n=workshop::boss_native;
static unsigned checks=0,native_contacts=0,native_collections=0,commits=0,walkers=0,particles=0,sounds=0;
static int32_t sections[300]{};int32_t* section_addresses=sections;
static void need(bool yes,const char* why){++checks;if(!yes)throw std::runtime_error(why);}
struct Fixture;static Fixture* current=nullptr;
static void observe_hook(uint8_t*,recomp_context*,const char*);
#undef LOOKUP_FUNC
#define LOOKUP_FUNC(v) lookup(int32_t(v))
#define WORKSHOP_SPECIAL_CONTENT
namespace native_red_heart {
#include BOSS_RED_HEART_NATIVE_SLICE
}
static uint32_t be32(const std::vector<uint8_t>& b,size_t p){need(p+4<=b.size(),"ROM bounded word");return(uint32_t(b[p])<<24)|(uint32_t(b[p+1])<<16)|(uint32_t(b[p+2])<<8)|b[p+3];}
struct Fixture {
 static constexpr uint32_t player=0x80100000,heart=0x80101000,egg=0x80102000,resource=0x80103000,metadata=0x80104000,definition=0x80105000,vecA=0x80106000,vecB=0x80106100,scene=0x80107000,api=0x80108000,factory=0x80200000,stack=0x807B0000;
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* rdram=memory.data();h::Bonus bonus;h::Frame frame{83,5,10,player,true};recomp_context c{};std::optional<h::SpawnRequest> request;unsigned color=0;
 Fixture(const std::vector<uint8_t>& rom,unsigned pc=0,unsigned hp=3,bool spawn=true,unsigned phase=1):color(pc){
  current=this;sections[220]=int32_t(factory);actor(player,0x4001);w(0x800F8828,player);w(0x800F8DB0,pc<8?pc:0);w(0x807F0008,pc);for(unsigned i=0;i<8;++i)w(0x800F8DB4+i*4,4);w(0x800F8DB4+(pc<8?pc:0)*4,hp);
  w(0x800F8908,scene);w(scene+0x10,api);w(api+0x18,0x80700000);actor(egg,0x4002);b(egg+0x18,0x68);fl(vecA,2);fl(vecA+4,0);fl(vecB,0);fl(vecB+4,0);
  for(unsigned i=0;i<0xF10;i+=4)w(factory+i,be32(rom,0xDFC180+i));
  // Exact native callback table relocations in otherwise copied overlay data.
  for(unsigned i=0;i<10;++i){const auto raw=be32(rom,0xDFC180+0xEBC+i*4);w(factory+0xEBC+i*4,factory+raw-h::original_factory);}
  c.r29=n::base(stack);c.r31=n::base(0x80000300);c.r16=0x12345678;bonus.configure(true);bonus.enter_room(83);
  if(spawn){request=bonus.request_spawn(rdram,frame);need(bool(request),"Five real-completion input observations allow a logical spawn request");build_actor(rom,phase);need(bonus.bind_spawn(rdram,frame,*request,heart),"Native state/collider setters establish a bound40C5");}
 }
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=int32_t(v);}void sh(uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}void b(uint32_t a,uint8_t v){MEM_B(0,n::base(a))=v;}void fl(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}uint32_t word(uint32_t a){return n::word(rdram,a);}
 void actor(uint32_t a,uint16_t kind){w(a,0x19971110);sh(a+12,kind);b(a+0x18,1);b(a+0x17D,2);}
 void call(recomp_func_t* fn,uint32_t a,uint32_t a1=0,uint32_t a2=0,uint32_t a3=0){c.r4=n::base(a);c.r5=n::base(a1);c.r6=n::base(a2);c.r7=n::base(a3);const auto oldsp=c.r29,olds0=c.r16;fn(rdram,&c);need(c.r29==oldsp&&c.r16==olds0,"Actual native body preserves SP/S0");}
 void build_actor(const std::vector<uint8_t>& rom,unsigned phase=1){
  actor(heart,h::kind);w(heart+0x168,resource);w(resource+0x40,factory);w(heart+0x174,metadata);w(metadata,definition);fl(heart+0x3C,h::spawn_x);fl(heart+0x40,h::spawn_y);fl(heart+0x44,h::spawn_z);sh(heart+0x1F0,0xA);
  // Native26byte record becomes the expanded native definition after its4byte header.
  for(unsigned i=0;i<12;++i)b(definition+4+i,rom[0x52E710+i]);
  call(native_red_heart::func_806E72A0,heart);call(native_red_heart::func_806E7E4C,heart,phase);
 }
 void contact(uint32_t other=player){call(native_red_heart::func_806E7574,heart,other,vecA,vecB);++native_contacts;}
 void collect(){++frame.vi;call(native_red_heart::func_806E7728,heart);++native_collections;}
 void event(const char* name,uint32_t a,uint32_t a1=0,uint32_t ra=0){const auto saved=c;c.r4=n::base(a);c.r5=n::base(a1);c.r31=n::base(ra);observe_hook(rdram,&c,name);c=saved;}
};
static void observe_hook(uint8_t* ram,recomp_context* c,const char* name){need(current&&ram==current->rdram,"Native callbacks retain fixture owner");const auto before=current->memory;const auto regs=*c;if(current->bonus.trace(ram,c,name,current->frame))++commits;need(current->memory==before&&!std::memcmp(c,&regs,sizeof(*c)),"Bonus observer never writes native RAM/registers");}
namespace native_red_heart {
void workshop_trace_entry(uint8_t* ram,recomp_context* c,const char* name){observe_hook(ram,c,name);}
void func_80067BBC(uint8_t*,recomp_context* c){c->r2=0;}
void func_80067818(uint8_t*,recomp_context*){++sounds;}
void func_80067930(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected heart sound shutdown branch");}
void func_80067878(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected heart sound stop branch");}
void func_800679F0(uint8_t*,recomp_context*){}
void func_80049624(uint8_t* ram,recomp_context* c){observe_hook(ram,c,"func_80049624");}
void func_80057F74(uint8_t* ram,recomp_context* c){
 ++walkers;need(uint32_t(c->r5)==1&&uint32_t(c->r6)==0x80043E88&&uint32_t(c->r7)==0x80044658,"Actual447C8 selects permanent retirement callbacks");const auto actor=n::word(ram,uint32_t(c->r4));need(actor==Fixture::heart,"Finite queue-walker fixture retires exact bound heart");
 // Generic queue/allocator mechanics are outside this oracle. Model disposal
 // here to test that the host pending proof survives the real caller's order.
 current->bonus.forget(actor);current->b(actor+6,3);
 need(!current->bonus.snapshot().consumed&&!current->bonus.snapshot().binding.actor,"Retirement before counter/health cannot itself consume bonus");
}
void particle(uint8_t*,recomp_context* c){++particles;c->r2=0;}
recomp_func_t* lookup(int32_t a){if(uint32_t(a)==0x80700000)return particle;throw std::runtime_error("Unknown native red-heart dynamic call");}
}
int main(int argc,char**argv){try{
 need(argc==2,"Original fingerprinted USA ROM required");std::ifstream input(argv[1],std::ios::binary);std::vector<uint8_t> rom((std::istreambuf_iterator<char>(input)),{});need(rom.size()==0x1000000,"Original USA ROM size");need(be32(rom,0x52E738+0x10)==h::original_factory,"40C5 native resource factory confirmed");need(be32(rom,0xDFC180+0xEBC+3*4)==0x806E7728,"Real state3 callback is red-heart collection");
 const auto original=rom;
 for(unsigned color=0;color<14;++color)for(unsigned hp=0;hp<=8;++hp)for(bool egg:{false,true}){
  Fixture f(rom,color,hp);const auto aliases=f.word(0x800F8DB0);std::array<unsigned,8> old{};for(unsigned k=0;k<8;++k)old[k]=f.word(0x800F8DB4+k*4);
  const auto before_commit=commits;f.contact(egg?Fixture::egg:Fixture::player);need(n::byte(f.rdram,Fixture::heart+0x18)==3&&!f.bonus.snapshot().consumed,"Actual P1/egg contact selects3 without consuming bonus early");f.collect();
  need(f.bonus.snapshot().consumed&&!f.bonus.snapshot().invalidated&&commits==before_commit+1,"Exact native collect chain consumes once");need(f.word(h::heart_count_address)==1,"Actual native code increments heart counter exactlyonce");need(f.word(0x800F8DB4+aliases*4)==8,"Actual native max-health428 heals every0..8 color/alias to8");need(f.word(0x800F8944)==0,"Red heart creates no SuperHappy timer");
  for(unsigned k=0;k<8;++k)need(f.word(0x800F8DB4+k*4)==(k==aliases?8:old[k]),"Only native current-color health changes");
  f.bonus.enter_room(162);f.bonus.enter_room(83);f.frame.completed=9;need(!f.bonus.request_spawn(f.rdram,f.frame),"Consumed heart never respawns in later rest visits");
  f.event("func_806E7728",Fixture::heart);need(commits==before_commit+1,"Duplicate/stale callback cannot consume another bonus");
 }
 // Native7574 has no phase restriction: both bobbing2 and native spawn5
 // may select collection3. Exercise their exact native callbacks, not synthetic
 // observer events, for every standard/custom alias and both accepted contacts.
 for(unsigned color=0;color<14;++color)for(unsigned phase:{2u,5u})for(bool egg:{false,true}){
  Fixture f(rom,color,4,true,phase);const auto before_commit=commits;f.contact(egg?Fixture::egg:Fixture::player);need(n::byte(f.rdram,Fixture::heart+0x18)==3,"Real phase2/5 contact selects3");f.collect();
  need(f.bonus.snapshot().consumed&&!f.bonus.snapshot().invalidated&&commits==before_commit+1,"Real phase2/5 native collection retains strict causality");need(f.word(0x800F8DB4+(color<8?color:0)*4)==8&&f.word(0x800F8944)==0,"Phase2/5 native heal remains8 without SuperHappy");
 }
 for(unsigned fault=0;fault<7;++fault){Fixture f(rom,0,3,false);if(fault==0)f.frame.completed=4;if(fault==1)f.frame.completed=10;if(fault==2)f.frame.rest_ready=false;if(fault==3)f.frame.room=162;if(fault==4)f.w(0x800F8828,0);if(fault==5)f.frame.vi=0;if(fault==6)f.bonus.configure(false);const auto before=f.memory;need(!f.bonus.request_spawn(f.rdram,f.frame)&&before==f.memory,"Invalid/unearned/contextless request is read-only and cannot spawn");}
 {Fixture f(rom);need(!f.bonus.request_spawn(f.rdram,f.frame),"One live heart per visit");f.bonus.forget(Fixture::heart);need(!f.bonus.snapshot().consumed&&!f.bonus.request_spawn(f.rdram,f.frame),"Forget/culling is not collection nor same-visit respawn");f.bonus.enter_room(162);f.bonus.enter_room(83);f.frame.completed=6;need(f.bonus.request_spawn(f.rdram,f.frame).has_value(),"Uncollected logical bonus remains available next rest visit");}
 {Fixture f(rom);f.w(Fixture::heart+0x180,Fixture::player);f.bonus.enter_room(162);f.bonus.enter_room(83);need(!f.bonus.request_spawn(f.rdram,f.frame),"Real still-bound token across a room transition prevents duplicate");}
 {Fixture f(rom);f.w(Fixture::heart,0);f.bonus.frame(f.rdram,f.frame);need(!f.bonus.snapshot().consumed&&!f.bonus.request_spawn(f.rdram,f.frame),"Disappearance never grants a reward or new same-visit token");}
 {Fixture f(rom);f.sh(Fixture::egg+12,0x4004);f.contact(Fixture::egg);need(n::byte(f.rdram,Fixture::heart+0x18)==1&&!f.bonus.snapshot().consumed,"Native non-projectile4004 contact is rejected");}
 {Fixture f(rom);f.call(native_red_heart::func_806E7E4C,Fixture::heart,3);f.event("func_806E7728",Fixture::heart);need(f.bonus.snapshot().invalidated&&!f.bonus.snapshot().consumed,"Synthetic state3 without real contact proof cannot consume");}
 {Fixture f(rom);f.contact();f.frame.vi++;f.event("func_806E7728",Fixture::heart);f.bonus.forget(Fixture::heart);f.frame.vi++;f.bonus.frame(f.rdram,f.frame);need(!f.bonus.snapshot().consumed&&!f.bonus.request_spawn(f.rdram,f.frame)&&f.word(h::heart_count_address)==0,"Teardown with no native counter/health effect cannot count as collection or grant same-visit duplicate");}
 for(unsigned fault=0;fault<6;++fault){Fixture f(rom,0,3,false);auto req=f.bonus.request_spawn(f.rdram,f.frame);need(bool(req),"Negative bind request");f.build_actor(rom);if(fault==0)f.sh(Fixture::heart+12,0x4035);if(fault==1)f.w(Fixture::heart+0x180,Fixture::player);if(fault==2)f.w(Fixture::definition+0x28,Fixture::factory+0x330);if(fault==3)f.sh(Fixture::definition+0xA,9);if(fault==4)f.fl(Fixture::heart+0x40,400);if(fault==5)req->birth++;need(!f.bonus.bind_spawn(f.rdram,f.frame,*req,Fixture::heart)&&!f.bonus.snapshot().consumed,"Foreign kind/parent/collider/position/birth cannot bind bonus");}
 {Fixture f(rom);f.contact();f.collect();f.bonus.configure(true);f.bonus.enter_room(83);need(!f.bonus.snapshot().consumed&&f.bonus.request_spawn(f.rdram,f.frame).has_value(),"New campaign explicitly resets one-use logical ledger");}
 need(rom==original,"Original ROM caller buffer unchanged");std::cout<<"BossRestHeart0260Checks: "<<checks<<" PASS; nativeContacts="<<native_contacts<<" nativeCollections="<<native_collections<<" strictCommits="<<commits<<" retireWalkers="<<walkers<<" particles="<<particles<<" sounds="<<sounds<<"; 14colors*9health*2contacts plus56phase2/5, no GPU/game/save claim.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
