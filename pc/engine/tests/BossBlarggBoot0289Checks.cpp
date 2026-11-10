#include "../boss_blargg_boot0289.hpp"
#include "../boss_blargg_arena0289.hpp"
#include "../boss_rooms_data0250.hpp"
#include "../boss_rest_data0240.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <stdexcept>
#include <vector>
#ifndef BLARGG_BOOT_NATIVE_SLICE
#error Supply the literal cold-load parent/loader/getter corpus.
#endif
namespace boot=workshop::boss_blargg_boot0289;namespace core=workshop::boss_battle0250;namespace n=workshop::boss_native;
static unsigned checks=0,body_calls=0,allocation_boundaries=0,parser_boundaries=0,readonly_hooks=0;
static std::map<std::string,unsigned> body_counts;
static void need(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
extern "C" void do_break(uint32_t){throw std::runtime_error("Unexpected native break");}
int32_t section_array[512]{};int32_t* section_addresses=section_array;
static void boot_entry(uint8_t*,recomp_context*);
struct Prepared {
 core::Config config;core::Plan plan;unsigned index=10,target=177;std::array<uint32_t,2> words{};std::vector<uint8_t> rom,descriptor,objects;
 Prepared(){
  std::ifstream stream("D:/Yoshi-Story/work/yoshis-story-decomp/baseroms/us/baserom.z64",std::ios::binary);need(bool(stream),"Original US ROM available");rom.assign(std::istreambuf_iterator<char>(stream),{});
  config.blargg=true;plan=core::make_plan(1,config);for(unsigned i=1;i<9;++i)if(plan.fights[i].family==core::Family::Blargg){index=i;break;}need(index==3,"Exact failed diagnostic source is first guaranteed Blargg at index3");target=167+index;
  workshop::boss_blargg_arena0289::prepare(rom);rom=workshop::boss_rest_data0240::apply(std::move(rom));rom=workshop::boss_rooms_data0250::apply(std::move(rom),plan);workshop::boss_blargg_arena0289::install(rom);
  const auto at=0xA84A0+target*8;std::copy_n(rom.begin()+at,8,rom.begin()+0xA84A0+52*8);for(unsigned j=0;j<2;++j)words[j]=workshop::u32(rom,at+j*4);
  need(words==std::array<uint32_t,2>{0x01000000,0x044A1660},"Prepared original-ROM words match failed GPU RAM descriptor exactly");
  const auto header=workshop::terrain::bank(rom,words[1]);descriptor.assign(rom.begin()+header,rom.begin()+header+0x118);
  const auto image=workshop::boss_rooms_data0250::arena(target).objects;objects.assign(image.begin(),image.end());need(objects.size()==8*12,"Same eight prepared records as failed native scene");
 }
};
static const Prepared* prepared=nullptr;
static constexpr uint32_t cache=0x801011B8,registry=0x80101188,table=0x800A78A0,stack=0x807A0000;
namespace actual {
recomp_func_t* lookup(int32_t);
void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected native assertion");}
void func_8006DC38(uint8_t*,recomp_context*){throw std::runtime_error("Warm camera branch excluded from cold fixture");}
void func_8006D59C(uint8_t*,recomp_context* c){++allocation_boundaries;need(c->r4==1,"Literal loader passes actual native-init high byte to allocator boundary");c->r2=n::base(cache);}
void func_8006D85C(uint8_t* rdram,recomp_context* c){
 ++parser_boundaries;need(uint32_t(c->r4)==cache+4&&uint32_t(c->r5)==prepared->words[1],"Literal loader selects the exact captured native descriptor");
 const auto wrapper=n::word(rdram,cache+0x60);
 for(unsigned p=0;p<0x118;++p)MEM_B(p,c->r4)=prepared->descriptor[p];
 MEM_W(0x5C,c->r4)=wrapper; // Explicit parser boundary mirrors native6D85C's preserved wrapper pointer.
 c->r2=1;
}
#undef LOOKUP_FUNC
#define LOOKUP_FUNC(v) lookup(int32_t(v))
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#include BLARGG_BOOT_NATIVE_SLICE
#pragma clang diagnostic pop
recomp_func_t* lookup(int32_t a){if(uint32_t(a)==0x8006D6E4)return func_8006D6E4;throw std::runtime_error("Unknown original native cache callback");}
}
struct Fixture {
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* r=memory.data();uint8_t* rdram=r;recomp_context c{};boot::Router router;boot::Frame frame;
 Fixture(){
  for(unsigned i=0;i<11;++i){const auto entry=cache+i*0xA94;w(registry+i*4,entry);b(entry+1,255);}
  h(0x800F8448,11);w(0x800F8450,registry);
  w(boot::cold_controller+0x18,table);b(boot::cold_controller+0x1C,52);
  for(unsigned j=0;j<2;++j){w(table+52*8+j*4,prepared->words[j]);w(table+prepared->target*8+j*4,prepared->words[j]);}
  b(0x800F7EA2,7);w(0x800F8770,0x800F8774);
  w(cache+0x60,cache+0x128);w(cache+0x128,cache+0x12C);h(cache+0x12C,8);w(cache+0x130,cache+0x134);
  for(unsigned i=0;i<8;++i)for(unsigned j=0;j<3;++j)w(cache+0x134+i*16+j*4,workshop::u32(prepared->objects,i*12+j*4));
  c.r4=n::base(boot::cold_controller);c.r5=52;c.r29=n::base(stack);c.r31=n::base(0x8006DDFC);c.r16=0x12345678;c.f2.u32l=0x13579BDF;
  frame={true,true,core::Phase::Armed,52,prepared->index,prepared->target,19,0,0,1940,0};
  need(router.arm(true,prepared->plan,prepared->index,prepared->target,prepared->words),"Arm unique first-Blargg QA cold-source bridge");
 }
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}void h(uint32_t a,unsigned v){MEM_H(0,n::base(a))=uint16_t(v);}void b(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}
 bool records()const{for(unsigned i=0;i<8;++i)for(unsigned j=0;j<3;++j)if(n::word(r,cache+0x134+i*16+j*4)!=workshop::u32(prepared->objects,i*12+j*4))return false;return true;}
};
static Fixture* current=nullptr;
static bool use_router=true;
static void boot_entry(uint8_t* r,recomp_context* c){
 if(!current||!use_router)return;
 const auto memory=current->memory;const auto before=*c;
 const auto result=current->router.redirect(r,c,current->frame);
 need(!result.rejected,"Actual parent native scope admits QA redirect");
 auto expected=before;if(result.redirected)expected.r5=prepared->target;
 need(current->memory==memory&&!std::memcmp(&expected,c,sizeof(expected)),"Redirect preserves complete8MiB RAM and every register except owned A1");++readonly_hooks;
}
static uint32_t get(Fixture& f,unsigned source){auto ctx=f.c;ctx.r4=source;ctx.r29=n::base(stack-0x200);actual::func_8006D6F8(f.r,&ctx);return uint32_t(ctx.r2);}
int main(){try{
 Prepared source;prepared=&source;
 {Fixture f;const auto memory=f.memory;const auto ctx=f.c;const auto rom=source.rom;const auto result=f.router.redirect(f.r,&f.c,f.frame);auto expected=ctx;expected.r5=source.target;need(result.redirected&&!result.rejected&&f.router.snapshot().used&&f.memory==memory&&!std::memcmp(&expected,&f.c,sizeof(expected))&&source.rom==rom,"Single successful bridge changes only A1, no ROM/RAM payload");const auto after=f.c;need(!f.router.redirect(f.r,&f.c,f.frame).redirected&&f.memory==memory&&!std::memcmp(&after,&f.c,sizeof(after)),"Consumed bridge cannot activate twice");}
 for(unsigned fault=0;fault<30;++fault){Fixture f;
  if(fault==0)f.frame.marker=false;if(fault==1)f.frame.diagnostic=false;if(fault==2)f.frame.phase=core::Phase::ToFight;if(fault==3)f.frame.phase=core::Phase::RetryToFight;if(fault==4)f.frame.cold_scene=170;if(fault==5)f.frame.index++;if(fault==6)f.frame.physical_room++;if(fault==7)f.frame.logical_room=162;if(fault==8)f.frame.retry_epoch=1;if(fault==9)f.frame.completed=1;if(fault==10)f.frame.player=0x800C9054;if(fault==11)f.frame.vi=0;
  if(fault==12)f.c.r4+=4;if(fault==13)f.c.r5=170;if(fault==14)f.c.r31+=4;if(fault==15)f.c.r29=0;
  if(fault==16)f.b(0x800F7EA2,8);if(fault==17)f.w(0x800F828C,4);if(fault==18)f.w(0x800F8290,4);if(fault==19)f.b(boot::cold_controller+0x1C,170);if(fault==20)f.w(0x800F84CC,0x80102000);
  if(fault==21)f.w(0x800F8770,0);if(fault==22)f.h(0x800F8774+0x7C,1);if(fault==23)f.w(boot::scene_table_slot,0);if(fault==24)f.w(table+52*8+4,source.words[1]+4);if(fault==25)f.w(table+source.target*8,0);if(fault==26)f.h(0x800F8448,12);if(fault==27)f.w(0x800F8450,0);if(fault==28)f.b(cache+1,52);if(fault==29)f.b(cache+1,source.target);
  const auto memory=f.memory;const auto ctx=f.c;const auto result=f.router.redirect(f.r,&f.c,f.frame);need(!result.redirected&&!f.router.snapshot().used&&memory==f.memory&&!std::memcmp(&ctx,&f.c,sizeof(ctx)),"Wrong session/native parent/mirror/cache rejects before any write");
 }
 for(unsigned fault=0;fault<7;++fault){boot::Router router;auto plan=source.plan;auto words=source.words;auto index=source.index,target=source.target;bool marker=true;if(fault==0)marker=false;if(fault==1)plan.config.blargg=false;if(fault==2)index=0;if(fault==3)target=52;if(fault==4)words[0]=0;if(fault==5)words[1]=0x0F001000;if(fault==6){index=9;target=176;}need(!router.arm(marker,plan,index,target,words)&&!router.snapshot().armed,"Invalid arm never enables dormant QA bridge");}
 {Fixture f;current=&f;use_router=false;auto ctx=f.c;ctx.r31=n::base(0x80068830);actual::func_8006DDCC(f.r,&ctx);need(n::byte(f.r,cache+1)==52&&get(f,52)==cache&&!get(f,source.target),"Literal unmodified parent reproduces raw52 cache miss for planned170");need(f.records(),"Failed baseline source preserves all eight real prepared records");current=nullptr;}
 {Fixture f;current=&f;use_router=true;auto ctx=f.c;ctx.r31=n::base(0x80068830);const auto rom=source.rom;actual::func_8006DDCC(f.r,&ctx);
  need(f.router.snapshot().used&&n::byte(f.r,cache+1)==source.target&&n::word(f.r,boot::cold_controller+0x20)==cache,"Literal parent6DDCC and original6DA0C establish source170 and current cache natively");
  need(n::byte(f.r,boot::cold_controller+0x1C)==52&&n::word(f.r,0x800F828C)==0&&n::word(f.r,0x800F8290)==0,"Native raw-scene global intentionally remains52 after parent return; no global patch");
  need(get(f,source.target)==cache&&!get(f,52),"Original getter after parent return finds only170 as required by unchanged Guard");
  need(f.records()&&source.rom==rom,"Eight record image and all prepared ROM bytes survive cold redirect/native loader");
  const auto parses=parser_boundaries,allocations=allocation_boundaries;auto warm=f.c;warm.r5=source.target;actual::func_8006DA0C(f.r,&warm);
  need(parser_boundaries==parses&&allocation_boundaries==allocations&&n::word(f.r,boot::cold_controller+0x20)==cache&&get(f,source.target)==cache&&f.records(),"Warm170 original loader reuses exact cache without parsing, allocation or payload mutation");
  need(n::byte(f.r,boot::cold_controller+0x1C)==52,"Warm direct cache reuse still does not patch native raw-scene global");
  current=nullptr;
 }
 std::cout<<"BossBlarggBoot0289Checks: "<<checks<<" PASS; "<<body_calls<<" literal native calls; "<<readonly_hooks<<" readonly redirect hooks; allocator/parser HOST boundaries, no game or retry proof.\n";
 std::cout<<"{\"checks\":"<<checks<<",\"nativeBodyCalls\":"<<body_calls<<",\"readonlyRedirectHooks\":"<<readonly_hooks<<",\"allocatorBoundaries\":"<<allocation_boundaries<<",\"parserBoundaries\":"<<parser_boundaries<<",\"bodyCallsByName\":{";bool comma=false;for(const auto&[name,count]:body_counts){if(comma)std::cout<<",";comma=true;std::cout<<"\""<<name<<"\":"<<count;}std::cout<<"}}\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
