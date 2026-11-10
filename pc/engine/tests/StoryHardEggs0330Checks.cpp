#include "recomp.h"
#include <array>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
namespace {unsigned checks=0,contacts=0,promotions=0,state_inits=0,metadata_stubs=0,fifo_stubs=0,scene_stubs=0,sounds=0,untracked_promotion_boundaries=0;void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}template<class T>T& access(uint8_t* ram,uint64_t a,uint64_t b,unsigned flip){uint32_t p=uint32_t(a+b);if(p<0x80000000||p>0x80800000-sizeof(T))throw std::runtime_error("Hard egg RAM out of bounds");return *reinterpret_cast<T*>(ram+((p-0x80000000)^flip));}}
#undef MEM_W
#undef MEM_H
#undef MEM_HU
#undef MEM_B
#undef MEM_BU
#define MEM_W(a,b) access<int32_t>(rdram,uint64_t(a),uint64_t(b),0)
#define MEM_H(a,b) access<int16_t>(rdram,uint64_t(a),uint64_t(b),2)
#define MEM_HU(a,b) access<uint16_t>(rdram,uint64_t(a),uint64_t(b),2)
#define MEM_B(a,b) access<int8_t>(rdram,uint64_t(a),uint64_t(b),3)
#define MEM_BU(a,b) access<uint8_t>(rdram,uint64_t(a),uint64_t(b),3)
#include "../story_hard_eggs0330.hpp"
#include "../gold_silver_resources0300.hpp"
namespace he=workshop::story_hard_eggs0330;
namespace wt=workshop::terrain;
namespace {he::Registry registry;bool route_hard=true;constexpr uint32_t p1=0x80100000,p1data=0x80110000,source=0x80130000,wrapper=0x80140000,egg=0x80150000,egg_private=0x80160000,metadata=0x80170000,definition=0x80180000,item=0x80190000,resource=0x80200000,egg_resource=0x80201000,drawing=0x80202000,texture=0x80203000,palette=0x80210000,wrapper_texture=0x80204000,wrapper_palette=0x80205000,table=0x80220000,stack=0x807D0000;}
void func_80039F48(uint8_t*,recomp_context*);void func_8004012C(uint8_t*,recomp_context*);void func_8003FFD8(uint8_t*,recomp_context*);void func_8003AD58(uint8_t*,recomp_context*);void func_8003B540(uint8_t*,recomp_context*);void func_80047F9C(uint8_t*,recomp_context*);void func_80048114(uint8_t*,recomp_context*);
void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected native actor ownership failure");}
void func_80039F00(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected bounce callback path");}
void func_8002F96C(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected bounced player path");}
void func_8003FC48(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected ordinary egg path");}
void func_8003FDC4(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected portable non-egg path");}
void func_8003F894(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected full inventory fallback");}
void func_80048060(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected already-parented egg");}
void func_80065490(uint8_t* rdram,recomp_context* c){++metadata_stubs;for(unsigned i=0;i<12;i++)MEM_BU(i,c->r4)=MEM_BU(i,c->r5);}
void func_8003E658(uint8_t* rdram,recomp_context* c){++fifo_stubs;MEM_BU(0x12C8,c->r4)++;}
void func_8003AF20(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected native non-contact event path");}
void func_8003F14C(uint8_t*,recomp_context*){++metadata_stubs;}
void func_8005FE48(uint8_t*,recomp_context*){++metadata_stubs;}
void func_80039EAC(uint8_t*,recomp_context*){++metadata_stubs;}
void func_8003A060(uint8_t*,recomp_context*){++metadata_stubs;}
void func_800444D0(uint8_t*,recomp_context*){++scene_stubs;}
void state_init(uint8_t*,recomp_context*){++state_inits;}
extern "C" recomp_func_t* get_function(int32_t address){if(uint32_t(address)==0x8003AD58)return func_8003AD58;if(uint32_t(address)==0x80001000)return state_init;std::abort();}
// Exact native B5A0 boundary, with the Hard registry decision supplied byRoot.
// Old shared find services are deliberately unavailable in this isolated unit.
void workshop_find_egg_state(uint8_t* rdram,recomp_context* c){++promotions;const auto state=MEM_BU(0x1F,c->r29);if(route_hard&&registry.egg_state(uint32_t(c->r4),state)){if(registry.pickup_sound(uint32_t(c->r4),state))++sounds;c->r2=0x401E;return;}++untracked_promotion_boundaries;}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmicrosoft-goto"
#pragma clang diagnostic ignored "-Wunused-label"
#include HARD_EGGS_NATIVE_BODY
#pragma clang diagnostic pop
namespace {
void word(uint8_t* rdram,uint32_t p,uint32_t value){MEM_W(0,int32_t(p))=value;}uint32_t word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,int32_t(p)));}
void byte(uint8_t* rdram,uint32_t p,unsigned v){MEM_BU(0,int32_t(p))=uint8_t(v);}void half(uint8_t* rdram,uint32_t p,unsigned v){MEM_HU(0,int32_t(p))=uint16_t(v);}
std::vector<uint8_t> read(const char* name){std::ifstream f(name,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
size_t resource_offset(const std::vector<uint8_t>& rom,unsigned kind){for(unsigned n=0;n<1582;n++){const auto p=0xA1C90+n*12;if(wt::be16(rom,p)==kind)return wt::bank(rom,wt::be32(rom,p+4));}throw std::runtime_error("Real sprite source not found");}
void actor(uint8_t* rdram,uint32_t p,unsigned kind){word(rdram,p,0x19971110);half(rdram,p+12,kind);byte(rdram,p+0x18,1);}
he::Placement placement(unsigned level=8,unsigned room=82){return {level,room,0x46920000,0x44200000,0x43200000};}
void setup(uint8_t* rdram,unsigned color,const std::vector<uint8_t>& palettes){
 std::memset(rdram,0,0x800000);route_hard=true;registry.configure(std::array{placement()});registry.enter(8,82,color);
 actor(rdram,p1,0x4001);word(rdram,p1+0x1F4,p1data);word(rdram,0x800F8828,p1);word(rdram,0x800F8DF4,0xA5A5003F);word(rdram,0x807FC144,0xF17D002D);word(rdram,0x807F2000,0x123456);
 actor(rdram,source,0x4692);auto place=placement();recomp_context c{};c.r4=int32_t(source);c.r5=place.packed;c.r6=place.x;c.r7=place.y;registry.created(&c);need(registry.hidden(source)==false,"Added Hard source did not bind");
 c={};c.r4=int32_t(source);c.r5=0x41CE;registry.begin_child(&c);actor(rdram,wrapper,0x41CE);c={};c.r4=int32_t(wrapper);c.r5=0x41CE0000;registry.created(&c);need(registry.owns_wrapper(wrapper)&&registry.release_color(wrapper)==color,"Native begin_child/created ordering lost binding");
 word(rdram,wrapper+0x1F4,item);half(rdram,item,0x401C);registry.prepare_release(rdram,wrapper);need(MEM_HU(0,int32_t(item))==0x401D,"Native wrapper item kind not normalized safely");
 actor(rdram,egg,0x401D);word(rdram,egg+0x1F4,egg_private);word(rdram,egg+0x174,metadata);word(rdram,metadata,definition);word(rdram,egg+0x1C,0x8003AD58);word(rdram,egg+0x1EC,color);registry.spawned(egg,color);
 word(rdram,0x8009EC98,0x80001004);word(rdram,0x8009ECB0,0x80001000);
 word(rdram,0x800F84D4,table);byte(rdram,table,1);word(rdram,table+4,egg_resource);half(rdram,egg_resource+0x3C,0x401D);word(rdram,egg_resource+0x84,texture);word(rdram,egg_resource+0x88,palette);for(unsigned i=0;i<palettes.size();i++)byte(rdram,palette+i,palettes[i]);
 half(rdram,resource+0x3C,0x41CE);word(rdram,resource+0x84,wrapper_texture);word(rdram,resource+0x88,wrapper_palette);word(rdram,wrapper+0x168,resource);word(rdram,wrapper+0x16C,drawing);word(rdram,drawing+8,wrapper_palette);word(rdram,drawing+0x30,wrapper_texture);word(rdram,drawing+0x40,resource+0x74);
}
void contact(uint8_t* rdram){recomp_context c{};c.r4=int32_t(egg);c.r5=int32_t(p1);c.r29=int32_t(stack);++contacts;func_80039F48(rdram,&c);}
template<class F>void rejects(F action){bool rejected=false;try{action();}catch(const std::runtime_error&){rejected=true;}need(rejected,"Invalid registry placement admitted");}
}
int main(int argc,char** argv){try{
 need(argc==2,"Expected content ROM input");const auto input=read(argv[1]);need(input.size()==0x1000000,"Real content size");const auto packed=workshop::gold_silver_resources0300::apply(input);const auto source_palette=resource_offset(packed,0x4002);const auto palettes=wt::block(packed,wt::be32(packed,source_palette+0x30));need(palettes.size()==16*512,"Real sixteen-color palette pack missing");
 std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
 for(unsigned color=0;color<16;color++){
  setup(rdram,color,palettes);need(!registry.release_color(wrapper)&&registry.owns_wrapper(wrapper)&&registry.hidden(source)==true,"Released wrapper could fall back to native White egg");const auto flags=word(rdram,0x800F8DF4),finds=word(rdram,0x807FC144),progress=word(rdram,0x807F2000);registry.frame(rdram);need(!registry.carried(),"State0 not attached but already counted");contact(rdram);need(MEM_BU(0x30,int32_t(egg))==4&&word(rdram,egg+0x180)==p1,"Real native contact did not attach Hard401D");need(MEM_BU(0x17C,int32_t(egg))==1&&MEM_HU(0x184,int32_t(p1))==1&&word(rdram,p1+0x188)==egg,"Actual native mode1/child list not retained");registry.frame(rdram);auto carried=registry.carried();need(carried&&carried->color==color&&carried->actor==egg&&carried->player==p1,"Actual native carried egg not observed");need(word(rdram,0x800F8DF4)==flags&&word(rdram,0x807FC144)==finds&&word(rdram,0x807F2000)==progress,"Hard promotion contaminated native BW/shared finds/progress");
  // mode0 from47F9C and mode1 from3FFD8 are both genuine parent attachment
  // states; other modes and non-current/fake parents must never qualify.
  byte(rdram,egg+0x17C,0);registry.frame(rdram);need(registry.carried().has_value(),"Real native attachment mode0 rejected");for(unsigned mode=2;mode<5;mode++){byte(rdram,egg+0x17C,mode);registry.frame(rdram);need(!registry.carried(),"Invalid attachment mode admitted");}byte(rdram,egg+0x17C,1);word(rdram,egg+0x180,p1+4);registry.frame(rdram);need(!registry.carried(),"Wrong parent admitted");word(rdram,egg+0x180,p1);word(rdram,0x800F8828,0);registry.frame(rdram);need(!registry.carried(),"Non-current player parent admitted");word(rdram,0x800F8828,p1);registry.frame(rdram);need(registry.carried().has_value(),"Valid carry did not recover after guard tests");
  need(!registry.pickup_sound(egg,4),"Pickup sound repeated after native transition");registry.committed();registry.frame(rdram);need(!registry.carried(),"Committed egg recredited on next frame");registry.attempt_reset();registry.frame(rdram);need(!registry.carried(),"Attempt reset recredited old pooled egg");registry.spawned(egg,color);registry.frame(rdram);need(registry.carried().has_value(),"New generation of recycled Hard pool egg rejected");
  // Wrapper paint borrows the real401D palette/texture but returnsfalse so
  // the native bubble still renders; exact pool actors returntrue.
  const auto static_palette=palettes;need(!registry.paint(rdram,wrapper),"Wrapper paint wrongly replaced native wrapper drawing");need(word(rdram,drawing+8)==palette+color*512&&word(rdram,drawing+0x30)==texture&&word(rdram,drawing+0x40)==egg_resource+0x74,"Borrowed16 palette/texture/geometry mismatch");for(unsigned i=0;i<512;i++)need(MEM_BU(i,int32_t(word(rdram,drawing+8)))==static_palette[color*512+i],"Real palette bytes not exact");registry.restore(rdram,wrapper);need(word(rdram,drawing+8)==wrapper_palette&&word(rdram,drawing+0x30)==wrapper_texture&&word(rdram,drawing+0x40)==resource+0x74,"Borrowed wrapper descriptor not restored before cleanup");
  word(rdram,egg+0x168,egg_resource);word(rdram,egg+0x16C,drawing);need(registry.paint(rdram,egg),"Tracked401D paint not handled");need(word(rdram,drawing+8)==palette+color*512,"Tracked401D display color mismatch");registry.dispose(rdram,wrapper);need(!registry.owns_wrapper(wrapper)&&!registry.release_color(wrapper),"Disposed wrapper binding remained");
 }
 need(sounds==16&&untracked_promotion_boundaries==0,"Canonical/shared promotion boundary reached for tracked Hard eggs");
 // Untracked/native eggs keep their original White promotion behavior.
 setup(rdram,6,palettes);registry.forget(egg);const auto flags=word(rdram,0x800F8DF4);contact(rdram);need(word(rdram,0x800F8DF4)==(flags|0x400)&&untracked_promotion_boundaries==1,"Unrelated native White egg behavior changed");
 setup(rdram,2,palettes);registry.enter(9,83,3);registry.frame(rdram);need(!registry.carried()&&!registry.release_color(wrapper)&&registry.owns_wrapper(wrapper),"Old level wrapper released in new level");registry.enter(8,82,2);need(registry.release_color(wrapper)==2,"New level reset did not permit source revisit");
 // Owned source gone during nested construction must not bind another parent.
 registry.configure(std::array{placement()});registry.enter(8,82,2);recomp_context c{};c.r4=int32_t(source);c.r5=placement().packed;c.r6=placement().x;c.r7=placement().y;registry.created(&c);c.r5=0x41CE;registry.begin_child(&c);registry.forget(source);c.r4=int32_t(wrapper);c.r5=0x41CE0000;registry.created(&c);need(!registry.owns_wrapper(wrapper),"Destroyed parent bound a new wrapper");
 rejects([&]{auto bad=placement();bad.level=24;registry.configure(std::array{bad});});rejects([&]{auto bad=placement();bad.room=177;registry.configure(std::array{bad});});rejects([&]{auto bad=placement();bad.packed=0x46930000;registry.configure(std::array{bad});});rejects([&]{registry.configure(std::array{placement(),placement()});});
 need(workshop::editor_objects::sources.empty()&&workshop::editor_objects::children.empty()&&workshop::editor_objects::eggs.empty(),"Hard registry contaminated editor actor ownership");
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"realNativeContactCalls\":"<<contacts<<",\"nativePromotionCalls\":"<<promotions<<",\"stateInitializerStubs\":"<<state_inits<<",\"fifoStubs\":"<<fifo_stubs<<",\"metadataStubs\":"<<metadata_stubs<<",\"sceneStubs\":"<<scene_stubs<<",\"soundDecisionCount\":"<<sounds<<",\"untrackedNativePromotionChecks\":"<<untracked_promotion_boundaries<<",\"gpu\":false,\"game\":false}"<<std::endl;return 0;
 }catch(const std::exception& error){std::cerr<<"FAIL after "<<checks<<" checks: "<<error.what()<<std::endl;return 1;}}
