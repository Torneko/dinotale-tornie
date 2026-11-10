// CPU oracle: translated factory/ingestion bodies, explicit allocation/attachment ABI doubles.
// No renderer, native boss physics, victory or permanent campaign award runs.
#include "../boss_tree_hybrid0260.hpp"
#include "../boss_tree0240.hpp"
#include <array>
#include <bit>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace h=workshop::boss_tree_hybrid0260;
namespace n=workshop::boss_native;
namespace normal=workshop::boss_tree0240;
static unsigned checks=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
static void w(uint8_t* rdram,uint32_t a,uint32_t v){MEM_W(0,n::base(a))=int32_t(v);}
static void b(uint8_t* rdram,uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}
static void sh(uint8_t* rdram,uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}
static uint16_t hu(uint8_t* rdram,uint32_t a){return MEM_HU(0,n::base(a));}
static void actor(uint8_t* r,uint32_t a,uint16_t id){w(r,a,0x19971110);sh(r,a+12,id);b(r,a+0x18,1);b(r,a+0x17D,2);}
extern "C" {
void func_80574A88(uint8_t*,recomp_context*);void func_80574B44(uint8_t*,recomp_context*);
void func_80573F94(uint8_t*,recomp_context*);void func_80574060(uint8_t*,recomp_context*);
void func_80044194(uint8_t*,recomp_context*);void func_80046804(uint8_t*,recomp_context*);
void func_80047090(uint8_t*,recomp_context*);void func_80047F9C(uint8_t*,recomp_context*);
void func_8004A73C(uint8_t*,recomp_context*);void func_80052BA4(uint8_t*,recomp_context*);
void func_8005FDC4(uint8_t*,recomp_context*);void func_80067818(uint8_t*,recomp_context*);
void func_80573F08(uint8_t*,recomp_context*);void func_8002F7AC(uint8_t*,recomp_context*);
void func_8004700C(uint8_t*,recomp_context*);void func_800495AC(uint8_t*,recomp_context*);
}
static uint16_t workshop_mix_spawn(uint8_t*,recomp_context*);
#ifndef BOSS_TREE_HYBRID_NATIVE_SLICE
#error Supply the translated native fruit slice with provenance.
#endif
#include BOSS_TREE_HYBRID_NATIVE_SLICE
static std::array<int32_t,300> section_storage{};
int32_t* section_addresses=section_storage.data();
static h::Controller* ctl=nullptr;static const h::Frame* current_frame=nullptr;
static bool normal_ledger=false;
static uint32_t allocation=0x80180000;static bool fail_allocation=false;
static unsigned allocations=0,attachments=0,mix_calls=0,mix_scoped=0,collector_calls=0,collected=0;
extern "C" void switch_error(const char*,uint32_t,uint32_t){throw std::runtime_error("Unexpected native collider jump");}
static uint16_t workshop_mix_spawn(uint8_t* r,recomp_context* c){
 ++mix_calls;const auto id=uint16_t(c->r5);if(ctl)if(auto own=ctl->mix_override(r,c,id)){++mix_scoped;return *own;}
 // Hostile global replacement double proves owned normal slots bypass random mixing.
 return id>=0x4030&&id<=0x4033?0x4F05:id;
}
extern "C" void func_80046804(uint8_t* r,recomp_context* c){
 ++allocations;if(fail_allocation){c->r2=0;return;}
 const auto kind=uint16_t(uint32_t(c->r5)>>16);const auto a=allocation;allocation+=0x1000;
 actor(r,a,kind);w(r,a+0x168,a+0x200);w(r,a+0x174,a+0x300);sh(r,a+0x23C,kind);w(r,a+0x300,a+0x400);c->r2=n::base(a);
}
extern "C" void func_80047F9C(uint8_t* r,recomp_context* c){
 ++attachments;const auto p=uint32_t(c->r4),a=uint32_t(c->r5);need(n::actor(r,p,0x4708)&&n::range(a,0x1FC),"Explicit attachment boundary receives tree and native factory fruit");
 unsigned i=0;while(i<20&&n::word(r,p+0x188+4*i))++i;need(i<20,"Explicit child-list double has room");w(r,p+0x188+4*i,a);sh(r,p+0x184,hu(r,p+0x184)+1);w(r,a+0x180,p);if(normal_ledger)normal::trace(r,c,"func_80047F9C");c->r2=1;
}
extern "C" void func_80047090(uint8_t*,recomp_context* c){c->r2=7;}
extern "C" void func_80044194(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected unparented factory mode");}
extern "C" void func_8004A73C(uint8_t*,recomp_context*){}
extern "C" void func_80052BA4(uint8_t*,recomp_context*){}
extern "C" void func_8005FDC4(uint8_t*,recomp_context*){}
extern "C" void func_80067818(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected 30-fruit award sound");}
extern "C" void func_80573F08(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected 30-fruit terminal branch");}
extern "C" void func_8002F7AC(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected scene exit");}
extern "C" void func_8004700C(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected 30-fruit actor lookup");}
extern "C" void func_800495AC(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected unrelated scene HUD update");}
extern "C" void func_80573F94(uint8_t* r,recomp_context* c){
 ++collector_calls;if(normal_ledger)normal::trace(r,c,"func_80573F94");need(ctl&&current_frame,"Native collector has campaign context");if(ctl->collect(r,c,*current_frame))++collected;native_original_func_80573F94(r,c);
}
struct Fixture {
 static constexpr uint32_t tree=0x80100000,priv=0x80110000,to=0x80200000,fo=0x80210000,sp=0x80780000,record=sp+0x34;
 std::vector<uint8_t> mem=std::vector<uint8_t>(0x800000);uint8_t* r=mem.data();recomp_context c{};h::Controller control;h::Frame f{true,true,83,1,tree,priv,to,fo};
 void context(unsigned slot=0){c={};c.r29=n::base(sp);c.r31=n::base(to+0x15C);c.r4=n::base(record);w(r,priv+0x18,slot);w(r,record,tree);sh(r,record+4,h::native_kinds[slot]);w(r,record+8,std::bit_cast<uint32_t>(12.f));w(r,record+12,std::bit_cast<uint32_t>(24.f));w(r,record+16,std::bit_cast<uint32_t>(36.f));w(r,record+20,0);w(r,record+24,0);w(r,record+28,2);}
 Fixture(uint64_t seed=7){control.configure(true,seed);actor(r,tree,0x4708);w(r,tree+0x20,to+0x30C);w(r,tree+0x1F4,priv);for(unsigned i=0;i<6;++i)sh(r,priv+0x1C+2*i,i?3:0);for(unsigned i=0;i<6;++i){sh(r,fo+0xE7C+i*4,16+i);sh(r,fo+0xE7E+i*4,24+i);}section_addresses[126]=int32_t(fo);context();ctl=&control;current_frame=&f;allocation=0x80180000;normal_ledger=false;fail_allocation=false;allocations=attachments=mix_calls=mix_scoped=collector_calls=collected=0;w(r,0x800F828C,0);b(r,0x800F8438,0x4E);}
 std::optional<h::Birth> birth(unsigned slot=0){context(slot);const auto before=std::vector<uint8_t>(mem.begin()+(record&0x7FFFFF),mem.begin()+(record&0x7FFFFF)+32);const auto ticket=control.spawn_enter(r,&c,f);need(ticket&&ticket!=h::suppress_spawn,"Owned factory entry gets scope");const auto selected=hu(r,record+4);func_80574B44(r,&c);need(uint32_t(c.r29)==sp&&uint32_t(c.r31)==to+0x15C,"Actual factory restores stack and Tree RA");auto out=control.spawn_leave(r,&c,ticket);need(std::equal(before.begin(),before.end(),mem.begin()+(record&0x7FFFFF)),"Complete local record restored exactly");if(uint32_t(c.r2)){need(n::actor(r,uint32_t(c.r2),selected),"Actual factory allocates selected kind");w(r,priv+4*slot,uint32_t(c.r2));}return out;}
 void detach(uint32_t a){const auto p=n::word(r,a+0x180);if(n::range(p,0x1FC))for(unsigned i=0;i<20;++i)if(n::word(r,p+0x188+4*i)==a){w(r,p+0x188+4*i,0);sh(r,p+0x184,hu(r,p+0x184)-1);}w(r,a+0x180,0);}
 void eat(uint32_t a,unsigned state=0x66){c={};c.r29=n::base(sp);c.r31=0x80012340;c.r4=n::base(a);c.r5=state;func_80574060(r,&c);need(uint32_t(c.r29)==sp&&uint32_t(c.r31)==0x80012340,"Actual ingestion restores stack and caller");}
};
int main(){try{
 for(uint64_t seed=0;seed<4096;++seed){h::Controller a,z;a.configure(true,seed);z.configure(true,seed);need(a.ledger_valid()&&a.snapshot().order==z.snapshot().order,"Shuffle is deterministic six-kind permutation");need(a.next()==a.snapshot().order[0]&&a.snapshot().consumed==0&&a.snapshot().consumed_mask==0,"New session begins unconsumed");}
 {h::Controller a;a.configure(false,1);need(!a.next()&&a.ledger_valid(),"Disabled bag offers no fruit");}
 // Frame writes only slot0 supply. All failed guards are fully read only.
 for(unsigned fault=0;fault<15;++fault){Fixture x;switch(fault){case 1:x.f.active=false;break;case 2:x.f.in_rest=false;break;case 3:++x.f.room;break;case 4:++x.f.tree;break;case 5:x.f.tree_private+=4;break;case 6:++x.f.tree_overlay;break;case 7:w(x.r,Fixture::tree,0);break;case 8:sh(x.r,Fixture::tree+12,0x4001);break;case 9:b(x.r,Fixture::tree+0x17D,3);break;case 10:b(x.r,Fixture::tree+0x18,2);break;case 11:w(x.r,Fixture::tree+0x20,Fixture::to+0x310);break;case 12:w(x.r,Fixture::tree+0x1F4,Fixture::priv+4);break;case 13:x.f.tree_overlay=0x807FFFFC;break;case 14:x.control.configure(false,7);break;}auto before=x.mem;x.control.frame(x.r,x.f);if(!fault){need(hu(x.r,Fixture::priv+0x1C)==1,"Available type enables slot0");sh(x.r,Fixture::priv+0x1C,0);}need(x.mem==before,"Frame changes no memory except proven slot0 supply");}
 for(unsigned fault=0;fault<19;++fault){Fixture x;switch(fault){case 0:x.f.active=false;break;case 1:x.f.in_rest=false;break;case 2:++x.f.room;break;case 3:++x.c.r29;break;case 4:x.c.r4+=4;break;case 5:x.c.r31+=4;break;case 6:w(x.r,Fixture::record,Fixture::tree+4);break;case 7:w(x.r,Fixture::record+28,1);break;case 8:w(x.r,Fixture::priv+0x18,6);break;case 9:sh(x.r,Fixture::record+4,0x4030);break;case 10:x.f.tree_private=0x807FFFFC;break;case 11:x.f.tree_overlay=0;break;case 12:sh(x.r,Fixture::tree+12,0x4001);break;case 13:b(x.r,Fixture::tree+6,3);break;case 14:b(x.r,Fixture::tree+0x17D,3);break;case 15:b(x.r,Fixture::tree+0x18,0);break;case 16:w(x.r,Fixture::tree+0x20,0);break;case 17:x.control.configure(false,7);break;case 18:x.c.r29=n::base(0x807FFFFC);x.c.r4=n::base(0x80800030);break;}const auto before=x.mem;const auto regs=x.c;need(!x.control.spawn_enter(x.r,&x.c,x.f),"Foreign request is not scoped");need(x.mem==before&&std::memcmp(&regs,&x.c,sizeof(regs))==0,"Rejected entry preserves all data and registers");}
 {Fixture x;need(!x.control.spawn_enter(nullptr,&x.c,x.f)&&!x.control.spawn_enter(x.r,nullptr,x.f),"Null entries reject");need(!x.control.mix_override(x.r,&x.c,0x4030),"Mix needs native scope");}
 {Fixture x;const auto next=x.control.next();fail_allocation=true;need(!x.birth(),"Failed native allocation binds no fruit");need(x.control.next()==next&&!x.control.snapshot().token.actor&&!x.control.snapshot().consumed,"Allocation failure consumes nothing");fail_allocation=false;need(x.birth().has_value(),"Same type retries after allocation failure");}
 for(unsigned slot=1;slot<6;++slot){Fixture x;need(!x.birth(slot),"Normal native fruit binds no hybrid token");need(allocations==1&&attachments==1&&mix_calls==1&&mix_scoped==1&&!x.control.snapshot().invalidated,"All five normal kinds survive random mix and native mode2 attachment");need(!x.control.snapshot().token.actor&&x.control.snapshot().consumed==0,"Normal fruit leaves hybrid ledger alone");}
 {Fixture x;const auto ticket=x.control.spawn_enter(x.r,&x.c,x.f);need(ticket&&ticket!=h::suppress_spawn,"First request allowed");recomp_context nested=x.c;sh(x.r,Fixture::record+4,0x4035);need(x.control.spawn_enter(x.r,&nested,x.f)==h::suppress_spawn,"Pending slot0 suppresses nested birth");sh(x.r,Fixture::record+4,*x.control.next());x.c.r2=0;x.control.spawn_leave(x.r,&x.c,ticket);need(hu(x.r,Fixture::record+4)==0x4035&&!x.control.snapshot().invalidated,"Pending original request restores on failure");}
 // Actual factory then actual state66 ingestion, all six entries.
 {Fixture x;unsigned seen=0;for(unsigned pos=0;pos<6;++pos){x.control.frame(x.r,x.f);need(hu(x.r,Fixture::priv+0x1C)==1,"Next fruit enables slot0");auto fruit=x.birth();need(fruit&&fruit->position==pos&&h::hybrid(fruit->kind),"Native birth binds current hybrid");need(!(seen&(1u<<(fruit->kind-0x4F00))),"No duplicate bag type");seen|=1u<<(fruit->kind-0x4F00);need(mix_calls==mix_scoped&&attachments==allocations,"Every native birth is scoped and attached");x.context();need(x.control.spawn_enter(x.r,&x.c,x.f)==h::suppress_spawn,"Live fruit suppresses duplicate");x.control.frame(x.r,x.f);need(!hu(x.r,Fixture::priv+0x1C),"Live fruit blocks slot0 supply");x.detach(fruit->actor);x.eat(fruit->actor,0x65);need(x.control.snapshot().consumed==pos&&n::word(x.r,0x800F89D0)==pos,"Non-ingestion native state consumes nothing");x.eat(fruit->actor);need(x.control.snapshot().consumed==pos+1&&n::word(x.r,0x800F89D0)==pos+1,"Actual state66 advances bag and native inventory together");need(n::word(x.r,0x800F89D4+4*pos)==6+fruit->kind-0x4F00,"Native inventory stores matching custom fruit type");need(x.control.ledger_valid()&&!x.control.snapshot().invalidated,"Ingestion preserves prefix ledger");b(x.r,fruit->actor+0x17D,3);x.control.forget(fruit->actor);w(x.r,Fixture::priv,0);}
 need(seen==63&&x.control.snapshot().consumed_mask==63&&!x.control.next(),"Six unique fruits exhaust once");x.control.frame(x.r,x.f);need(!hu(x.r,Fixture::priv+0x1C),"Exhausted slot0 never regenerates");x.context();need(x.control.spawn_enter(x.r,&x.c,x.f)==h::suppress_spawn,"Exhausted direct native request suppressed");for(unsigned i=1;i<6;++i)need(hu(x.r,Fixture::priv+0x1C+2*i)==3,"Five normal supplies untouched");need(collected==6&&collector_calls==6,"Exactly six real collector calls consume six fruits");}
 // Cross-ledger oracle: existing untouched tree rules keep exactly15 normal
 // consumptions, while independent slot0 supplies exactly6 hybrids.
 {Fixture x;normal_ledger=true;normal::configure(true);normal::enter_room(83);x.c={};x.c.r4=n::base(Fixture::tree);x.c.r5=1;normal::trace(x.r,&x.c,"func_807B810C");need(normal::snapshot().initialized,"Existing normal ledger binds actual Tree constructor setter");
 unsigned custom=0,total=0;for(unsigned batch=0;batch<3;++batch)for(unsigned slot=1;slot<6;++slot){normal::frame(x.r);x.control.frame(x.r,x.f);if(custom<6){auto fruit=x.birth();need(bool(fruit),"Independent slot0 available alongside normal ledger");x.detach(fruit->actor);x.eat(fruit->actor);++custom;b(x.r,fruit->actor+0x17D,3);x.control.forget(fruit->actor);normal::forget(fruit->actor);w(x.r,Fixture::priv,0);}
 need(hu(x.r,Fixture::priv+0x1C+2*slot)==1,"Existing normal slot allowance remains one when available");need(!x.birth(slot),"Normal tree birth remains outside hybrid bag");const auto a=uint32_t(x.c.r2);need(normal::snapshot().fruitActors[slot-1]==a,"Original ledger binds actual normal factory child");x.detach(a);x.eat(a);++total;need(normal::snapshot().totalConsumed==total&&normal::ledger_valid(),"Existing normal collection ledger advances causally and stays valid");b(x.r,a+0x17D,3);normal::forget(a);w(x.r,Fixture::priv+4*slot,0);}
 normal::frame(x.r);x.control.frame(x.r,x.f);need(normal::snapshot().totalConsumed==15&&normal::snapshot().regenerations==2&&normal::snapshot().consumedmask==31&&!normal::snapshot().invalidated,"Original five slots and two regenerations total exactly15 normal fruit");need(custom==6&&x.control.snapshot().consumed==6&&!x.control.snapshot().invalidated&&n::word(x.r,0x800F89D0)==21,"Independent six-custom bag plus15 normal fruit have21 real native ingestions");for(unsigned slot=0;slot<6;++slot)need(hu(x.r,Fixture::priv+0x1C+2*slot)==0,"All six supplies exhausted without invincibility heart or renewal");normal_ledger=false;}
 {Fixture x;auto fruit=x.birth();need(bool(fruit),"Carried fruit born natively");x.detach(fruit->actor);x.f.in_rest=false;x.f.room=162;x.control.forget(Fixture::tree);w(x.r,Fixture::tree,0);x.control.frame(x.r,x.f);need(x.control.snapshot().token.actor==fruit->actor,"Room change/tree teardown preserves detached live fruit");x.f.tree=0x80120000;x.f.tree_private=0x80130000;actor(x.r,x.f.tree,0x4708);w(x.r,x.f.tree+0x20,Fixture::to+0x30C);w(x.r,x.f.tree+0x1F4,x.f.tree_private);x.f.in_rest=true;x.f.room=83;sh(x.r,x.f.tree_private+0x1C,7);x.control.frame(x.r,x.f);need(!hu(x.r,x.f.tree_private+0x1C)&&x.control.snapshot().token.actor==fruit->actor,"New rest tree cannot duplicate carried fruit");x.f.in_rest=false;x.f.room=165;x.eat(fruit->actor);need(x.control.snapshot().consumed==1&&n::word(x.r,0x800F89D0)==1,"Real ingestion outside rest consumes same bound fruit");}
 for(unsigned cause=0;cause<8;++cause){Fixture x;auto fruit=x.birth();need(bool(fruit),"Disposal fixture born");const auto current=x.control.next();x.detach(fruit->actor);switch(cause){case 0:x.control.forget(fruit->actor);break;case 1:w(x.r,fruit->actor,0);break;case 2:sh(x.r,fruit->actor+12,0x4030);break;case 3:b(x.r,fruit->actor+0x17D,3);break;case 4:w(x.r,fruit->actor+0x168,fruit->actor+0x204);break;case 5:w(x.r,fruit->actor+0x174,fruit->actor+0x304);break;case 6:sh(x.r,fruit->actor+0x23C,0x4F00+(fruit->kind-0x4F00+1)%6);break;case 7:w(x.r,fruit->actor+0x300,fruit->actor+0x404);break;}x.control.frame(x.r,x.f);need(x.control.snapshot().consumed==0&&x.control.next()==current&&!x.control.snapshot().token.actor,"Disposal/identity change does not consume bag entry");}
 for(unsigned fault=0;fault<13;++fault){Fixture x;auto fruit=x.birth();need(bool(fruit),"Collect guard fixture born");x.c={};x.c.r31=n::base(Fixture::fo+0x204);x.c.r4=n::base(fruit->actor);x.c.r5=0x66;switch(fault){case 0:x.f.active=false;break;case 1:x.f.fruit_overlay=0;break;case 2:x.f.fruit_overlay=0x807FFFFC;break;case 3:x.c.r31+=4;break;case 4:x.c.r5=0x65;break;case 5:x.c.r4+=4;break;case 6:w(x.r,fruit->actor,0);break;case 7:sh(x.r,fruit->actor+12,0x4035);break;case 8:b(x.r,fruit->actor+0x17D,3);break;case 9:w(x.r,fruit->actor+0x168,0);break;case 10:w(x.r,fruit->actor+0x174,0);break;case 11:sh(x.r,fruit->actor+0x23C,0x4035);break;case 12:w(x.r,fruit->actor+0x300,0);break;}const auto before=x.mem;const auto regs=x.c;need(!x.control.collect(x.r,&x.c,x.f)&&x.control.snapshot().consumed==0,"Foreign/non-causal collect rejected");need(x.mem==before&&std::memcmp(&regs,&x.c,sizeof(regs))==0,"Collect observer preserves all native bytes and registers");}
 for(unsigned fault=0;fault<7;++fault){Fixture x;const auto ticket=x.control.spawn_enter(x.r,&x.c,x.f);recomp_context inner{};inner.r17=n::base(Fixture::record);inner.r29=n::base(Fixture::sp-0x38);inner.r31=n::base(Fixture::to+0x15C);auto requested=*x.control.next();switch(fault){case 0:inner.r17+=4;break;case 1:inner.r29+=4;break;case 2:inner.r31+=4;break;case 3:++requested;break;case 4:sh(x.r,Fixture::record+4,0x4035);break;case 5:w(x.r,Fixture::record,Fixture::tree+4);break;}const auto before=x.mem;need(bool(x.control.mix_override(x.r,&inner,requested))==(fault==6),"Only exact scoped native request overrides global mix");need(x.mem==before,"Mix observer is read-only");sh(x.r,Fixture::record+4,*x.control.next());w(x.r,Fixture::record,Fixture::tree);x.c.r2=0;x.control.spawn_leave(x.r,&x.c,ticket);need(hu(x.r,Fixture::record+4)==0x4035,"Scope restoration exact after mix query");}
 {Fixture x;const auto ticket=x.control.spawn_enter(x.r,&x.c,x.f);x.c.r29+=4;x.c.r2=0;need(!x.control.spawn_leave(x.r,&x.c,ticket)&&x.control.snapshot().invalidated&&hu(x.r,Fixture::record+4)==0x4035,"Corrupted return stack still restores scoped local kind before rejection");}
 {Fixture x;const auto ticket=x.control.spawn_enter(x.r,&x.c,x.f);sh(x.r,Fixture::record+4,*x.control.next()+1);x.c.r2=0;x.control.spawn_leave(x.r,&x.c,ticket);need(hu(x.r,Fixture::record+4)==0x4035&&x.control.snapshot().invalidated,"Changed return record restored then rejected");}
 std::cout<<"BossTreeHybrid0260 "<<checks<<" CPU checks passed; native factory/ingestion with explicit ABI doubles, not rendering or full gameplay proof\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
