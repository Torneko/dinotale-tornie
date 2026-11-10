#include "../boss_native.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
namespace n=workshop::boss_native;
static unsigned checks=0,jump_boundaries=0,egg_calls=0,egg_edges=0,queries=0,native_entries=0;
static int ceiling_override=-1;static void need(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
void do_break(uint32_t){std::abort();}
namespace literal {
void func_800364DC(uint8_t*,recomp_context*);void func_80037FB4(uint8_t*,recomp_context*);void func_800415FC(uint8_t*,recomp_context*);void func_80033FB0(uint8_t*,recomp_context*);void func_8002DD44(uint8_t*,recomp_context*);
// Explicit service boundaries: input admission is enabled; unrelated action
// gates are idle. No service allocates an actor or assigns the jump velocity.
void func_80021958(uint8_t*,recomp_context* c){c->r2=1;}
void func_800309B8(uint8_t*,recomp_context* c){c->r2=0;}void func_800415D0(uint8_t*,recomp_context* c){c->r2=0;}void func_8003200C(uint8_t*,recomp_context* c){c->r2=0;}
void func_80035E28(uint8_t*,recomp_context* c){c->r2=1;}void func_800376A4(uint8_t*,recomp_context* c){c->r2=1;}
void func_8003BEC8(uint8_t* rdram,recomp_context* c){++egg_calls;const auto pad=n::word(rdram,0x800F7EA8);const bool edge=(MEM_HU(12,n::base(pad))&0x2000)!=0;egg_edges+=edge;c->r2=edge?1:0;}
// DB78/B9E4 state transition is deliberately NOT executed by this compact
// oracle. The boundary records admission; full jump state/landing is unclaimed.
void func_8002DB78(uint8_t*,recomp_context*){++jump_boundaries;}
void func_80021F54(uint8_t*,recomp_context*){}void func_8001D770(uint8_t*,recomp_context* c){c->r2=0;}void func_80067BF8(uint8_t*,recomp_context* c){c->r2=0;}void func_80067818(uint8_t*,recomp_context*){}
// Read-only tile-query service, decoded from the actual3x2 page table.
// The original53924 traversal itself is not counted as executed native code.
void func_80053924(uint8_t* rdram,recomp_context* c){++queries;if(ceiling_override>=0){c->r2=ceiling_override;return;}auto p=n::word(rdram,0x800F8810);for(unsigned off:{0x10u,4u,0x16Cu,0x1B0u,0x20u})p=n::word(rdram,p+off);const unsigned columns=MEM_HU(0x12,n::base(p)),rows=MEM_HU(0x14,n::base(p)),stride=MEM_HU(0x16,n::base(p));const float x=c->f12.fl,y=c->f14.fl;need(columns==3&&rows==2&&x>=0&&x<768&&y>=0&&y<512,"Actual ceiling query stays inside real Fish map");const unsigned tx=unsigned(x)/16,ty=unsigned(y)/16,page=n::byte(rdram,n::word(rdram,p+0x18)+(ty/16)*columns+tx/16);c->r2=page==255?0:MEM_HU(0,n::base(n::word(rdram,p+0x24)+2*((ty%16)*stride+page*16+tx%16)));}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#include "native-aim-jump.inc"
#pragma clang diagnostic pop
}
static std::vector<uint8_t> load(const char* file){std::ifstream f(file,std::ios::binary);need(bool(f),"Real Fish capture opens");std::vector<uint8_t> out((std::istreambuf_iterator<char>(f)),{});need(out.size()==0x800000,"Real Fish capture is8MiB");return out;}
struct Fixture {
 static constexpr uint32_t held=0x80400000,reticle=0x80400300;
 std::vector<uint8_t> memory;uint8_t* rdram;uint32_t player=0,priv=0,meta=0;
 explicit Fixture(const std::vector<uint8_t>& source):memory(source),rdram(memory.data()){
  player=n::word(rdram,0x800F8828);priv=n::word(rdram,player+0x1F4);meta=n::word(rdram,player+0x174);need(n::actor(rdram,player,0x4001)&&n::range(priv,0x1300),"Actual player identity and private are present");
  // HOST frontier reconstructs existing4005/4003 ownership and aim primary3.
  // It is not a claim that these constructors ran in the captured frame.
  w(held,0x19971110);h(held+12,0x4005);by(held+0x17D,2);w(held+0x180,player);h(held+0x17E,19);w(player+0x188+19*4,held);h(player+0x184,uint16_t(MEM_HU(0x184,n::base(player)))+1);
  w(reticle,0x19971110);h(reticle+12,0x4003);w(reticle+0x180,player);w(priv+0x1F0,held);w(priv+0x1F4,reticle);by(priv+0x1F8,1);by(priv,1);by(player+0x18,3);w(player+0x20,0x8003BF88);w(priv+0x1E0,1);
  by(priv+0xF4,0);w(meta+4,0);w(meta+8,0);by(meta+0x10,0);by(meta+0x12,0);fl(player+0x84,360);fl(player+0x88,273);ceiling_override=-1;jump_boundaries=egg_calls=egg_edges=queries=0;
 }
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=int32_t(v);}void h(uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}void by(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}void fl(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}
 void inputs(bool edge_a,bool edge_z,bool held_a=false){by(0x800F8996,edge_a);by(0x800F8995,held_a);h(n::word(rdram,0x800F7EA8)+12,edge_z?0x2000:0);}
 void without_aim(){w(priv+0x1F0,0);w(priv+0x1F4,0);by(priv+0x1F8,0);w(player+0x188+19*4,0);h(player+0x184,uint16_t(MEM_HU(0x184,n::base(player)))-1);w(held,0);w(reticle,0);w(held+0x180,0);w(reticle+0x180,0);by(player+0x18,1);w(player+0x20,0x80043450);}
 void call(recomp_func_t* fn){const auto before=memory;const std::array<uint32_t,4> graph{{n::word(rdram,priv+0x1F0),n::word(rdram,priv+0x1F4),n::word(rdram,held+0x180),n::word(rdram,player+0x188+19*4)}};recomp_context c{};c.r29=n::base(0x807EF000);c.r4=n::base(player);fn(rdram,&c);++native_entries;need(uint32_t(c.r29)==0x807EF000,"Native admission chain restores stack");need(graph==std::array<uint32_t,4>{{n::word(rdram,priv+0x1F0),n::word(rdram,priv+0x1F4),n::word(rdram,held+0x180),n::word(rdram,player+0x188+19*4)}},"Existing aim graph or its absence is untouched within the executed scope");bool same=true;for(unsigned i=0;i<memory.size();++i){const auto a=0x80000000u+i;const bool allowed=(a>=0x807EEF00&&a<0x807EF100)||(a>=player+0xC8&&a<player+0xCC)||a==((priv+0x1AD)^3)||a==((priv+0xDC)^3);if(!allowed&&memory[i]!=before[i]){same=false;break;}}need(same,"Admission changes only native vertical speed, its two private bytes and scratch stack; HP/pose/inventory/actors unchanged");}
 float vy(){return std::bit_cast<float>(n::word(rdram,player+0xC8));}
};
int main(int argc,char** argv){try{
 need(argc==2,"Pass actualb2 live Fish capture");const auto source=load(argv[1]);
 for(auto route:{literal::func_800364DC,literal::func_80037FB4}){Fixture f(source);f.inputs(true,false);f.call(route);need(jump_boundaries==1&&egg_calls==0&&queries==1&&f.vy()==-7,"Saved native ground callback admits A during primary3 and originalDD44 assigns minus7");}
 {Fixture f(source);f.inputs(true,true);f.call(literal::func_800415FC);need(jump_boundaries==1&&egg_calls==0&&f.vy()==-7,"FreshA takes priority over simultaneous second-pressZ before3BEC8 release service");f.inputs(false,true,true);f.call(literal::func_800415FC);need(jump_boundaries==1&&egg_calls==1&&egg_edges==1,"Next freshZ can reach the native egg service after the A edge is released");}
 {Fixture f(source);f.inputs(false,false,true);const float before=f.vy();f.call(literal::func_80033FB0);need(jump_boundaries==0&&f.vy()==before,"HeldA without a new edge does not retrigger the jump");}
 {Fixture f(source);f.inputs(true,false);f.w(f.meta+8,0xF000);const float before=f.vy();f.call(literal::func_80033FB0);need(jump_boundaries==0&&f.vy()==before,"Native ceiling-contact mask refuses the jump");}
 {Fixture f(source);f.inputs(true,false);ceiling_override=0x701;const float before=f.vy();f.call(literal::func_80033FB0);need(jump_boundaries==0&&f.vy()==before,"Native material high-byte bit0 blocks an overhead0701 ceiling");}
 {Fixture f(source);f.inputs(true,false);ceiling_override=0x601;f.call(literal::func_80033FB0);need(jump_boundaries==1&&f.vy()==-7,"Native0601 material has no overhead-blocking bit; no extra universal ceiling model is inferred");}
 {Fixture f(source);f.without_aim();f.by(f.priv+0xF4,1);f.fl(f.player+0xC8,1.4f);f.inputs(true,false);f.call(literal::func_800415FC);need(jump_boundaries==1&&f.vy()==-7&&n::byte(f.rdram,f.priv+0xF4)==1&&!n::word(f.rdram,f.priv+0x1F0),"Fresh ordinary A reaches realDD44 in an explicit primary1/F4air/no4005 frontier; upstream idle services remain declared");}
 {Fixture f(source);f.without_aim();f.by(f.priv+0xF4,1);f.w(f.meta+8,0xF000);f.inputs(true,false);const float before=f.vy();f.call(literal::func_800415FC);need(jump_boundaries==0&&f.vy()==before&&n::byte(f.rdram,f.priv+0xF4)==1,"Airborne fresh A still respects the native overhead-contact veto without4005");}
 std::cout<<"BossFishAimJump0260Checks: "<<checks<<" PASS; "<<native_entries<<" native entries, five unchanged bodies; HOSTaim graph/DB78 service boundaries, no full jump/landing claim\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
