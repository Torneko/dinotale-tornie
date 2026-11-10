#include "../boss_checkpoint_pose0260.hpp"
#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#ifndef BOSS_POSE_NATIVE_SLICE
#error Supply the original native warm-reset, integration and pose-copy bodies.
#endif
namespace p=workshop::boss_checkpoint_pose0260;namespace i=workshop::boss_checkpoint_inventory0260;namespace n=workshop::boss_native;
static unsigned checks=0,steps=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
namespace actual_native {
int dinotale_screen_right_test(int32_t);int dinotale_screen_left_test(uint8_t*,uint32_t,int32_t);
void func_800232E0(uint8_t*,recomp_context*);void func_80023310(uint8_t*,recomp_context*);void func_800233B4(uint8_t*,recomp_context*);void func_80066E88(uint8_t*,recomp_context*);void func_80066DE4(uint8_t*,recomp_context*);
void func_80047C38(uint8_t*,recomp_context*);void func_80065490(uint8_t*,recomp_context*);void func_80049018(uint8_t*,recomp_context*);void func_80047648(uint8_t*,recomp_context*);void func_80048320(uint8_t*,recomp_context*);void func_80048EDC(uint8_t*,recomp_context*);void func_8002C4F0(uint8_t*,recomp_context*);
void func_80048060(uint8_t*,recomp_context*);void Fault_HungUp(uint8_t*,recomp_context*);
#include BOSS_POSE_NATIVE_SLICE
// XY trigonometry is outside this depth oracle. Its actors have heading0 and
// zero XY speed; Z integration itself is the untouched native233B4 body.
void func_80066E88(uint8_t*,recomp_context* c){c->f0.fl=1.f;}
void func_80066DE4(uint8_t*,recomp_context* c){c->f0.fl=0.f;}
void func_80023310(uint8_t*,recomp_context*){}
void func_80049018(uint8_t*,recomp_context*){throw std::runtime_error("Unparented player uses no parent rotation");}
void func_80047648(uint8_t*,recomp_context*){throw std::runtime_error("Unparented player uses no parent transformation");}
void Fault_HungUp(uint8_t*,recomp_context*){throw std::runtime_error("Native detach parent invariant failed");}
int dinotale_screen_right_test(int32_t x){return x<0x130;}
int dinotale_screen_left_test(uint8_t*,uint32_t,int32_t x){return x<0x10;}
}
struct Fixture {
 static constexpr uint32_t actor=0x80100000,priv=0x80101000,meta=0x80104000,def=0x80105000,dims=0x80106000,stack=0x807B0000;
 std::vector<uint8_t> mem=std::vector<uint8_t>(0x800000);uint8_t* rdram=mem.data();recomp_context c{};i::Payload inventory{};p::Payload pose{};p::Restore restore;
 Fixture(unsigned color=0){
  w(actor,0x19971110);sh(actor+12,0x4001);b(actor+0x18,1);b(actor+0x17D,2);w(actor+0x1F4,priv);w(actor+0x174,meta);w(meta,def);w(def+0x30,dims);fl(dims+0x28,16);w(0x800F8828,actor);w(0x800F8DB0,color<8?color:0);w(0x807F0008,color);w(0x800F8DB4+(color<8?color:0)*4,1);
  fl(actor+0x3C,100);fl(actor+0x40,449);fl(actor+0x44,500.34f);fl(actor+0x84,100);fl(actor+0x88,449);fl(actor+0x8C,500.34f);w(actor+0x34,100);w(actor+0x38,100);fl(actor+0x6C,.8f);fl(actor+0x70,.8f);fl(actor+0x74,.8f);fl(actor+0x100,1);fl(actor+0x104,1);fl(actor+0x108,1);fl(priv+0xEC,1);
  inventory.activation_vi=100;inventory.player=actor;inventory.native_color=color<8?color:0;inventory.pc_color=color;inventory.hp=1;c.r29=n::base(stack);c.r4=n::base(actor);need(p::capture(rdram,inventory,pose),"Stable native plane captured with exact inventory owner");
 }
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=int32_t(v);}void sh(uint32_t a,uint16_t v){MEM_H(0,n::base(a))=v;}void b(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}void fl(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}float f(uint32_t a){return std::bit_cast<float>(n::word(rdram,a));}
 void fatal(float z=451.7401f,float speed=-6){b(actor+0x18,0xA);b(actor+0x30,0xB);fl(actor+0x44,z);fl(actor+0x8C,z);fl(actor+0xCC,speed);fl(actor+0xD8,speed+.1f);fl(actor+0x114,-.5f);}
 void step(){c.r4=n::base(actor);actual_native::func_80048EDC(rdram,&c);c.r4=n::base(actor);actual_native::func_800233B4(rdram,&c);c.r4=n::base(actor);actual_native::func_80047C38(rdram,&c);++steps;}
};
static bool depth_byte(size_t raw){const auto a=0x80000000u|uint32_t(raw^3);for(unsigned off:{0x44u,0x8Cu,0xCCu,0xD8u,0x114u})if(a>=Fixture::actor+off&&a<Fixture::actor+off+4)return true;return false;}
int main(int argc,char** argv){try{
 for(unsigned color=0;color<14;++color){
  Fixture broken(color);broken.fatal();broken.c.r4=n::base(Fixture::actor);actual_native::func_8002C4F0(broken.rdram,&broken.c);need(broken.f(Fixture::actor+0xCC)==-6&&broken.f(Fixture::actor+0x44)==451.7401f,"Actual warm reset leaves death Z and CC untouched");broken.step();need(broken.f(Fixture::actor+0x44)<451.7401f&&broken.f(Fixture::actor+0xD8)==-6,"True48320 records old CC in D8; true233B4 continues Z drift");
  Fixture fixed(color);fixed.fatal();const auto before=fixed.mem;need(fixed.restore.apply(fixed.rdram,fixed.pose,fixed.inventory,200,1),"Proved fatal same-player retry restores plane once");for(size_t j=0;j<before.size();++j)if(before[j]!=fixed.mem[j])need(depth_byte(j),"Only44/8C/CC/D8/114 change at retry fence");need(!fixed.restore.apply(fixed.rdram,fixed.pose,fixed.inventory,200,1),"Same retry epoch cannot reapply plane");
  fixed.c.r4=n::base(Fixture::actor);actual_native::func_8002C4F0(fixed.rdram,&fixed.c);for(unsigned frame=0;frame<120;++frame)fixed.step();need(fixed.f(Fixture::actor+0x44)==500.34f&&fixed.f(Fixture::actor+0x8C)==500.34f&&fixed.f(Fixture::actor+0xCC)==0&&fixed.f(Fixture::actor+0xD8)==0&&fixed.f(Fixture::actor+0x114)==0,"Actual warm reset and120 native movements retain restored gameplay plane");need(n::word(fixed.rdram,0x807F0008)==color&&n::word(fixed.rdram,0x800F8DB4+(color<8?color:0)*4)==1,"Color and HP remain exact");
 }
 for(unsigned fault=0;fault<14;++fault){Fixture x;auto pose=x.pose;auto inv=x.inventory;unsigned death=200,epoch=1;x.fatal();switch(fault){case 0:pose.player+=4;break;case 1:pose.activation_vi++;break;case 2:pose.native_color=1;break;case 3:pose.pc_color=1;break;case 4:pose.z=0x7FC00000;break;case 5:inv.player+=4;break;case 6:death=100;break;case 7:epoch=0;break;case 8:x.w(0x800F8828,Fixture::actor+4);break;case 9:x.b(Fixture::actor+0x18,1);break;case 10:x.b(Fixture::actor+0x30,2);break;case 11:x.w(0x807F0008,1);break;case 12:inv.activation_vi++;break;case 13:pose.player=0;break;}const auto before=x.mem;need(!x.restore.apply(x.rdram,pose,inv,death,epoch)&&x.mem==before,"Wrong owner/phase/epoch/nonfinite payload refuses every write");}
 for(unsigned field:{0x44u,0xCCu,0xD8u,0x114u}){Fixture x;x.fl(Fixture::actor+field,1);const auto before=x.mem;p::Payload rejected;need(!p::capture(x.rdram,x.inventory,rejected)&&x.mem==before,"Moving or mismatched plane cannot become checkpoint baseline");}
 // Optional before-death + corrupted-after-death RAM. Never modify source files.
 if(argc>=3){std::ifstream pre(argv[1],std::ios::binary),post(argv[2],std::ios::binary);std::vector<uint8_t> a((std::istreambuf_iterator<char>(pre)),{}),b((std::istreambuf_iterator<char>(post)),{});need(a.size()==0x800000&&b.size()==0x800000,"Original RAM snapshot sizes");auto* rdram=a.data();const auto player=n::word(rdram,0x800F8828);i::Payload inv;inv.player=player;inv.activation_vi=100;inv.native_color=n::word(rdram,0x800F8DB0);inv.pc_color=n::word(rdram,0x807F0008);inv.hp=n::word(rdram,0x800F8DB4+inv.native_color*4);p::Payload plane;need(p::capture(rdram,inv,plane),"Actual pre-death67122 Z500.34 is stable");p::Restore restore;need(restore.apply(b.data(),plane,inv,200,1),"Actual later fatal67122 runaway depth restores to original plane");need(n::word(b.data(),player+0x44)==plane.z&&n::word(b.data(),player+0x8C)==plane.z&&!n::word(b.data(),player+0xCC)&&!n::word(b.data(),player+0xD8)&&!n::word(b.data(),player+0x114),"Actual corrupted Z and both current/previous velocities repaired once");
  if(argc>=4){std::ifstream input(argv[3],std::ios::binary);std::vector<uint8_t> wake((std::istreambuf_iterator<char>(input)),{});need(wake.size()==0x800000,"Actual5d684 wake RAM size");rdram=wake.data();need(n::word(rdram,0x800F8828)==player,"Real wake and death snapshots retain exact player");const auto parent=n::word(rdram,player+0x180);need(n::actor(rdram,parent,0x4507)&&n::word(rdram,player+0x44)!=n::word(rdram,player+0x8C),"Actual wake player temporarily has No1 parent and relative local depth");recomp_context c{};c.r4=n::base(player);c.r29=n::base(0x807B0000);const auto world=n::word(rdram,player+0x8C);actual_native::func_80047C38(rdram,&c);need(n::word(rdram,player+0x8C)==world&&world==plane.z,"True47C38 composes No1worldZ plus relative localZ to original gameplay plane");
   auto live=inv;live.activation_vi=300;live.hp=n::word(rdram,0x800F8DB4+live.native_color*4);p::Payload attached;const auto untouched=wake;need(!p::capture(rdram,live,attached),"Default capture never accepts arbitrary parented players");need(p::capture(rdram,live,attached,parent,&plane)&&attached.z==world&&attached.activation_vi==live.activation_vi&&wake==untouched,"Actual wake accepts only explicit ownedNo1 and original stable world baseline without RAM writes");need(!p::capture(rdram,live,attached,parent+4,&plane),"A neighboring unproved parent is rejected");auto wrong=plane;wrong.z=std::bit_cast<uint32_t>(400.f);need(!p::capture(rdram,live,attached,parent,&wrong),"Wrong world baseline cannot become a checkpoint plane");
   MEM_B(0x18,n::base(player))=0xA;MEM_B(0x30,n::base(player))=0xB;p::Restore guarded;const auto fatal_attached=wake;need(!guarded.apply(rdram,attached,live,400,1)&&wake==fatal_attached,"Fatal player still parented to a point cannot receive absolute depth writes");c.r4=n::base(player);actual_native::func_80048060(rdram,&c);need(!n::word(rdram,player+0x180)&&n::word(rdram,player+0x44)==world,"True48060 detach converts world pose back to unparented local pose");need(guarded.apply(rdram,attached,live,400,1),"The same actual wake payload restores once after native detach");
  }
 }
 std::cout<<"BossCheckpointPose0260Checks: "<<checks<<" PASS; actualNativeMovementSteps="<<steps<<"; 14 colors, warm-reset omission, previous-velocity proof,120-frame plane stability; no GPU/save claim.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
