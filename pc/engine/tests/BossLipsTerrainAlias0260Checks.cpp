#include "../boss_rooms_data0250.hpp"
#include "../resource_cache.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <cstring>
#include <cstdlib>
namespace a=workshop::boss_lips_terrain_alias0260;
namespace b=workshop::boss_battle0250;
namespace rooms=workshop::boss_rooms_data0250;
static unsigned checks=0,original_fruit_notes=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
template<class F>static void rejected(F f,const char* why){bool caught=false;try{f();}catch(const std::exception&){caught=true;}need(caught,why);}
extern "C" void workshop_special_lookup_note(uint32_t){++original_fruit_notes;}
extern "C" int workshop_use_special_hud(){return 1;}
extern "C" void Fault_HungUp(uint8_t*,recomp_context*){std::abort();}
extern "C" void func_80065728(uint8_t* rdram,recomp_context* c){const auto p=uint32_t(c->r4)&0x7FFFFFFFu;const auto size=uint32_t(c->r5);need(p<=0x800000&&size<=0x800000-p,"Native zero service stays in8MiB");std::memset(rdram+p,int(c->r6),size);}
extern "C" int workshop_resource_slot(uint8_t* rdram,recomp_context* c,int match){return workshop::cache::find_slot(rdram,c,match!=0);}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wunused-label"
extern "C" {
#include NATIVE_ALIAS_SOURCE
}
#pragma clang diagnostic pop
extern "C" recomp_func_t* get_function(int32_t address){if(uint32_t(address)==0x80050634)return func_80050634;if(uint32_t(address)==0x80050674)return func_80050674;std::abort();}
static uint32_t word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,static_cast<gpr>(int32_t(p))));}
static void stage(uint8_t* rdram){for(unsigned i=0;i<a::table_size;++i)MEM_B(i,static_cast<gpr>(int32_t(a::ram_table)))=a::prepared_table[i];}
int main(int argc,char** argv){try{
 need(argc==2,"Original USA ROM required");std::ifstream f(argv[1],std::ios::binary);std::vector<uint8_t> original((std::istreambuf_iterator<char>(f)),{});need(original.size()==workshop::rom_size,"Native ROM size");
 auto plan=b::make_plan(1);b::Random rng{99};for(unsigned index:{1u,6u,7u,8u}){plan.fights[index]=b::encounter(index+1,b::Family::Lips,0,rng,plan.config);plan.fights[index].terrain=index%2?1:2;}
 need(b::valid(plan),"Four Lips encounters retain guaranteed family coverage");const auto base_rom=rooms::apply(original);auto rom=rooms::apply(original,plan);need(rom.size()==original.size(),"No ROM extension for Lips aliases");
 need(std::equal(base_rom.begin()+0xA1C90,base_rom.begin()+0xA1C90+1582*12,rom.begin()+0xA1C90),"All1582 native resource lookup entries stay exact after the existing Fish crop installation");
 const auto fa=workshop::terrain::bank(original,workshop::u32(original,a::lookup(original,0x8110)+4)),ba=workshop::terrain::bank(original,workshop::u32(original,a::lookup(original,0x8111)+4));
 const auto orig_collision=workshop::terrain::block(original,workshop::u32(original,fa+0x50)),orig_background=workshop::terrain::block(original,workshop::u32(original,ba+0x38));
 unsigned active=0;for(unsigned slot=0;slot<a::slots;++slot){const size_t at=a::allocation+slot*a::record_size;const auto id=workshop::u16(rom,at);if(!id){for(unsigned q=0;q<a::record_size;++q)need(rom[at+q]==0,"Unused alias slot is fully cleared");continue;}++active;need(id==a::first_id+slot&&id>>14==2,"Every alias retains native geometry class2 and unique encounter/layer ID");const auto index=slot/2+1,layer=slot%2;need(plan.fights[index].family==b::Family::Lips,"Alias belongs to a real Lips fight only");const auto source=layer?ba:fa,cloned=workshop::terrain::bank(rom,workshop::u32(rom,at+4));need(cloned>=a::allocation+a::table_size&&cloned+0x64<=a::end,"Alias descriptor stays in owned slab");need(workshop::u16(rom,cloned+0x18)==id&&workshop::u32(rom,at+8)==workshop::u32(original,a::lookup(original,uint16_t(0x8110+layer))+8),"Cloned A identity matches requested alias and original B metadata remains exact");for(unsigned q=0;q<0x64;++q)if(!(q>=0x18&&q<0x1A)&&!(q>=(layer?0x38u:0x50u)&&q<(layer?0x3Cu:0x54u)))need(rom[cloned+q]==original[source+q],"Native geometry dimensions/atlas/palette/pages/callback/flags unchanged");
  const auto data=workshop::terrain::block(rom,workshop::u32(rom,cloned+(layer?0x38:0x50)));need(data.size()==1024,"Native raw alias block is exactly two pages");unsigned changes=0;for(unsigned y=0;y<16;++y)for(unsigned x=0;x<32;++x){const bool target=plan.fights[index].terrain==1?(x>=14&&x<18&&y>=7&&y<(layer?9u:8u)):(((x>=8&&x<10)||(x>=22&&x<24))&&y>=9&&y<(layer?11u:10u));const auto before=workshop::u16(layer?orig_background:orig_collision,a::cell(x,y)),after=workshop::u16(data,a::cell(x,y));if(target){++changes;if(layer){const unsigned sx=plan.fights[index].terrain==1?x-4:x<10?x+4:x-12;need(after==workshop::u16(orig_background,a::cell(sx,y)),"Removed platform reveals original native cloth of matching phase");}else need(before==0x0601&&after==0,"Only existing one-way appui collision disappears");}else need(before==after,"Every non-target collision/visual tile including floor/walls stays exact");}need(changes==(layer?8u:4u),"Exact bounded visual/collision delta per terrain");const auto& arena=rooms::arena(rooms::source_for_encounter(index));need(arena.entry_x==104&&arena.entry_y==168&&arena.safe_pot_y==168&&arena.native_width==512&&arena.native_height==256,"Lips entry/pot/dimensions remain native");}
 need(active==8,"Four distinct fights own eight independent geometry resources");
 std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();recomp_context ctx{};ctx.r29=static_cast<gpr>(int32_t(0x800FF000));MEM_W(0,static_cast<gpr>(int32_t(a::ram_table-4)))=0x12345678;MEM_W(0,static_cast<gpr>(int32_t(a::ram_table+a::table_size)))=0x23456789;
 a::runtime::reset(rdram);for(unsigned i=0;i<a::table_size;++i)need(MEM_BU(i,static_cast<gpr>(int32_t(a::ram_table)))==0,"Init clears table even for a non-boss boot");stage(rdram);need(a::runtime::publish(rdram),"Big-endian ROM table staged with native MEM byte representation publishes");
 const auto table_before=a::prepared_table;MEM_W(0,static_cast<gpr>(int32_t(0x800F828C)))=0;ctx.r4=static_cast<gpr>(int32_t(0x800F84D4));func_80051C24(rdram,&ctx);func_800505F0(rdram,&ctx);need(word(rdram,0x800F84D4)==0x800F84D8,"True native51C24 establishes empty current room cache");need(MEM_BU(0,static_cast<gpr>(int32_t(0x800F84D8)))==0&&MEM_HU(8,static_cast<gpr>(int32_t(workshop::cache::manager)))==512,"True native cache resets clear room entries and initialize512 ROM slots");for(unsigned i=0;i<a::table_size;++i)need(MEM_BU(i,static_cast<gpr>(int32_t(a::ram_table)))==table_before[i],"Native reset boundary preserves installed alias records");
 // Original native leaf getter keeps every existing special-fruit branch.
 for(unsigned i=0;i<1582*12;++i)MEM_B(i,static_cast<gpr>(int32_t(0x800A1090)))=original[0xA1C90+i];
 for(unsigned id:{0x8110u,0x8111u,0x4F00u,0x4013u}){recomp_context actual=ctx,expected=ctx;actual.r4=expected.r4=id;battle_original_func_80050C8C(rdram,&expected);a::runtime::dispatch(rdram,&actual,battle_original_func_80050C8C);need(!std::memcmp(&actual,&expected,sizeof(actual)),"No-alias path exactly matches original native getter registers, including fruits/HUD");}
 need(original_fruit_notes>=4,"Existing fruit/HUD interception remains active");
 for(unsigned slot=0;slot<a::slots;++slot){const auto id=a::first_id+slot;ctx.r4=id;const auto saved=ctx;if(workshop::u16(a::prepared_table,slot*12)){a::runtime::dispatch(rdram,&ctx,battle_original_func_80050C8C);need(uint32_t(ctx.r2)==a::ram_table+slot*12&&ctx.r2==static_cast<gpr>(int32_t(a::ram_table+slot*12))&&ctx.r5==id,"Alias result is exact signed native pointer with native positive r5 ID");auto expected=saved;expected.r2=ctx.r2;expected.r5=ctx.r5;need(!std::memcmp(&ctx,&expected,sizeof(ctx)),"Alias getter changes only declared return/id registers");}else rejected([&]{a::runtime::dispatch(rdram,&ctx,battle_original_func_80050C8C);},"Unused alias fails cleanly before any unknown native callback");}
 need(!a::runtime::publish(rdram),"Records cannot be republished after a cache lookup");ctx.r4=a::first_id;const auto immutable=ctx;MEM_B(4,static_cast<gpr>(int32_t(a::ram_table)))^=1;rejected([&]{a::runtime::dispatch(rdram,&ctx,battle_original_func_80050C8C);},"Corrupt record never reaches original unknown-ID path");need(!std::memcmp(&ctx,&immutable,sizeof(ctx)),"Corruption preserves all guest registers");MEM_B(4,static_cast<gpr>(int32_t(a::ram_table)))^=1;
 // Native sorting and binary search preserve high geometry IDs as full keys.
 constexpr uint32_t list=0x80020000;MEM_B(0,static_cast<gpr>(int32_t(list)))=4;const std::array<unsigned,4> ids{0x8F1F,0x8110,0x8F10,0x8111};for(unsigned i=0;i<4;++i){const uint32_t resource=0x80021000+0x100*i;MEM_W(4+4*i,static_cast<gpr>(int32_t(list)))=int32_t(resource);MEM_H(0x3C,static_cast<gpr>(int32_t(resource)))=int16_t(ids[i]);}ctx.r4=static_cast<gpr>(int32_t(list));func_80050FCC(rdram,&ctx);MEM_W(0,static_cast<gpr>(int32_t(0x800F84D4)))=int32_t(list);for(unsigned id:ids){ctx.r4=id;func_80051100(rdram,&ctx);need(ctx.r2&&MEM_HU(0x3C,static_cast<gpr>(int32_t(word(rdram,uint32_t(ctx.r2)))) )==id,"Actual native sort/search distinguishes baseline and both encounter alias keys");}
 a::runtime::reset(rdram);need(!a::runtime::ready&&!a::runtime::used,"New boot clears registry readiness");for(unsigned i=0;i<a::table_size;++i)need(MEM_BU(i,static_cast<gpr>(int32_t(a::ram_table)))==0,"New boot removes every stale record including previously used slots");need(word(rdram,a::ram_table-4)==0x12345678&&word(rdram,a::ram_table+a::table_size)==0x23456789,"Alias init/lookup/cache tests preserve adjacent PC flags");
 // Reinstall baseline-only plan clears unused ROM records; no alias pointers
 // are retained from the prior four-fight preparation.
 auto baseline=b::make_plan(1);for(auto& e:baseline.fights)if(e.family==b::Family::Lips)e.terrain=0;auto clean=rooms::apply(original,baseline);for(unsigned i=0;i<a::table_size;++i)need(clean[a::allocation+i]==0,"Baseline boot has no stale lookup records");
 auto occupied=original;occupied[a::allocation]=0;rejected([&]{rooms::apply(occupied,plan);},"Occupied alias slab is refused before any resource install");rejected([&]{a::pair(0);},"Opening arena cannot own a Lips pair");rejected([&]{a::pair(9);},"Final RedCandy cannot own a Lips pair");
 bool shown1=false,shown2=false;for(uint64_t seed=0;seed<120&&!(shown1&&shown2);++seed){const auto source=b::make_plan(seed);for(unsigned index=1;index<6;++index){const auto& e=source.fights[index];if(e.family==b::Family::Lips&&e.variant==0&&e.terrain&&((e.terrain==1&&!shown1)||(e.terrain==2&&!shown2))){std::cout<<"LipsSoloCandidate seed="<<seed<<" case="<<index<<" terrain="<<e.terrain<<" hp=3\n";if(e.terrain==1)shown1=true;else shown2=true;}}}
 std::cout<<"BossLipsTerrainAlias0260Checks: "<<checks<<" PASS; actual native getter/cache leafs, ROM/RDRAM representation, no GPU/room traversal/combat claim.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
