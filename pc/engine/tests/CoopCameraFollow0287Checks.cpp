// Existing actual lifecycle/registration/destructor fixture; allocator and
// initialization remain explicit HOST service boundaries, never GPU evidence.
#define main lifecycle_regression_main
#define func_8002AEE4 host_legacy_func_8002AEE4
#include "CoopLifecycle0280Checks.cpp"
#undef func_8002AEE4
#undef main
#include <fstream>
extern "C" void func_80049018(uint8_t*,recomp_context*){throw std::runtime_error("Unadmitted attached-body drawing branch49018");}
extern "C" void func_80047648(uint8_t*,recomp_context*){throw std::runtime_error("Unadmitted attached-body drawing branch47648");}
extern "C" void func_80066E88(uint8_t*,recomp_context*){throw std::runtime_error("Unadmitted attached-body drawing branch66E88");}
extern "C" void func_80066DE4(uint8_t*,recomp_context*){throw std::runtime_error("Unadmitted attached-body drawing branch66DE4");}
extern "C" void func_8005A938(uint8_t*,recomp_context*);
extern "C" void func_80047AB8(uint8_t*,recomp_context*);
extern "C" void func_80065490(uint8_t*,recomp_context*);
extern "C" void func_8006CA74(uint8_t*,recomp_context*);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include "D:/Yoshi-Story/pc/build/coop-camera-follow0287/native-drawing-follow.inc"
#pragma clang diagnostic pop
namespace follow=workshop::coop_camera_follow0287;
static constexpr uint32_t camera=0x80368000,drawing=0x80368100;
static void bind_drawing(Fixture& f){f.pause();recomp_context x=f.x;x.r4=int32_t(drawing);x.r5=int32_t(first);x.r6=int32_t(0x8002ABD0);func_8005A994(f.r,&x);need(u(f.r,drawing+0xE8)==first&&u(f.r,drawing+0xEC)==0x8002ABD0,"Actual normal5A994 binds canonicalP1, with actual5A938/47AB8/65490");func_8002AEE4(f.r,&x);need(!u(f.r,drawing+0xE8)&&u(f.r,drawing+0xEC)==0x80065284,"Actual native2AEE4 reproduces detached target0/noop65284");}
static follow::Receipt waiting(Fixture& f,unsigned owner){bind_drawing(f);need(f.kill(owner)&&b::drawing_returns[owner].has_value(),"Actual fatal dispatcher captures the exact drawing-scene/identity death receipt");const auto receipt=*b::drawing_returns[owner];b::drawing_returns[owner].reset();f.fence(2501);need(l::lifecycle.snapshot().players[owner].phase==l::Phase::waiting,"Actual retirement reaches native waiting without a fake death");return receipt;}
static void reproduce_gpu_dump(){
 std::ifstream in("D:/Yoshi-Story/pc/profiles/validation-coop-health-0260-84a80d5776b7/controller-test-final.rdram",std::ios::binary);std::vector<uint8_t> ram(0x800000);in.read(reinterpret_cast<char*>(ram.data()),std::streamsize(ram.size()));need(in.gcount()==std::streamsize(ram.size()),"Frozen actual failedGPU dump required");auto* r=ram.data();const auto draw=u(r,0x800F8318),cam=u(r,0x800F8294);need(draw==0x800F831C&&!u(r,draw+0xE8)&&u(r,draw+0xEC)==0x80065284,"Actual failedGPU retained native detached drawing state");need(c::half(r,0x800C954C+12)==0x400B&&u(r,0x800C954C+0x180)==0x800C8B5C,"Actual old root address has become a body400B of newP1");need(std::bit_cast<float>(u(r,0x800C8B5C+0x84))==std::bit_cast<float>(u(r,cam+0x9C))-15,"Actual blockingX is the unchanged viewport right edge minus15, not a collision wall");
 const auto actor0=std::vector<uint8_t>(r+0xC8B5C,r+0xC8B5C+0x1FC),actor1=std::vector<uint8_t>(r+0xCDFD4,r+0xCDFD4+0x1FC);const auto viewport=std::vector<uint8_t>(r+(cam&0x7FFFFF)+0x84,r+(cam&0x7FFFFF)+0xB0);recomp_context x{};x.r29=int32_t(0x807A0000);x.r4=int32_t(draw);x.r5=int32_t(0x800C8B5C);x.r6=int32_t(0x8002ABD0);func_8005A994(r,&x);need(u(r,draw+0xE8)==0x800C8B5C&&u(r,draw+0xEC)==0x8002ABD0,"Exact native setter on copied real dump rebinds the true freshP1, never the reused400B");need(std::memcmp(actor0.data(),r+0xC8B5C,actor0.size())==0&&std::memcmp(actor1.data(),r+0xCDFD4,actor1.size())==0&&std::memcmp(viewport.data(),r+(cam&0x7FFFFF)+0x84,viewport.size())==0,"Actual copiedGPU actor poses/headers and viewport bounds remain byte-identical through native setter");
}
int main(){try{
 reproduce_gpu_dump();
 for(unsigned owner=0;owner<2;owner++)for(bool same_address:{false,true}){
  Fixture f;bind_drawing(f);const auto old=l::lifecycle.snapshot().players[owner].root;need(f.kill(owner)&&b::drawing_returns[owner].has_value(),"Fatal native dispatch captures follow repair for either owner");const auto shared=b::progress_hash(f.r);f.fence(2501);f.fence(2799);need(!u(f.r,drawing+0xE8)&&u(f.r,drawing+0xEC)==0x80065284&&created==0,"No follow rebind or new actor before native300VI delay");if(same_address)next_root=owner?second:first;f.fence(2800);
  const auto& s=l::lifecycle.snapshot();need(!s.failed&&s.players[owner].phase==l::Phase::alive&&s.players[owner].root.generation!=old.generation,"True native-return bridge path accepts new address or reused address with fresh birth");need(u(f.r,drawing+0xE8)==b::bound_primary&&u(f.r,drawing+0xEC)==0x8002ABD0&&!b::drawing_returns[owner],"Native follow setter readback precedes one-shot receipt consumption");need(owner==0||u(f.r,drawing+0xE8)==first,"P2 return always follows the unchanged currentP1, never P2");need(b::context->player(owner).health==2&&b::context->player(1-owner).health==8&&b::empty_respawn_inventory(f.r,s.players[owner].root.address)&&b::progress_hash(f.r)==shared,"Follow repair cannot alter actual returnHP/FIFO/survivor/shared progress");f.fence(2801);need(u(f.r,drawing+0xE8)==b::bound_primary&&created==1,"Follow binding remains one-shot across the next normal fence");
 }
 {Fixture f;const auto receipt=waiting(f,0);f.fence(2800);const auto fresh=b::bound_primary;birth(f.r,first,0x400B,first+0x24C);w(f.r,first+0x180,fresh);need(follow::rebind(f.r,&f.x,receipt,l::lifecycle.snapshot(),fresh,b::bound_secondary,2800,43,true),"A genuine old-address400B reuse never overrides authenticated freshP1 in the setter");need(u(f.r,drawing+0xE8)==fresh&&fresh!=first,"Fresh address selected despite oldroot memory now being a valid different actor");}
 for(unsigned bad=0;bad<19;bad++){
  Fixture f;auto receipt=waiting(f,0);f.fence(2800);auto state=l::lifecycle.snapshot();auto primary=b::bound_primary,secondary=b::bound_secondary;bool quiet=true;recomp_context* regs=&f.x;unsigned at=2800,room=43;
  switch(bad){case 0:receipt.epoch++;break;case 1:room=81;break;case 2:state.room=81;break;case 3:quiet=false;break;case 4:regs=nullptr;break;case 5:w(f.r,0x800F8318,drawing+4);break;case 6:w(f.r,0x800F8294,camera+4);break;case 7:w(f.r,camera+0x80,0);break;case 8:w(f.r,drawing+0xE8,second);break;case 9:w(f.r,drawing+0xEC,0x8002AD14);break;case 10:w(f.r,drawing+0xF4,0);break;case 11:w(f.r,table+20,second);break;case 12:by(f.r,0x800F7EA5,1);break;case 13:state.players[0].death_vi++;break;case 14:state.players[0].respawn_vi--;break;case 15:primary=first;break;case 16:receipt.owner=2;break;case 17:at++;break;case 18:need(c::created(primary),"Changed incarnation negative has an explicit HOST creation event");break;}
  const auto before=f.ram;const auto saved=f.x;need(!follow::rebind(f.r,regs,receipt,state,primary,secondary,at,room,quiet),"Foreign scene/epoch/root/target/callback/registration/timing refuses native follow repair");need(f.ram==before&&std::memcmp(&saved,&f.x,sizeof(saved))==0,"Rejected follow repair changes neither guest bytes nor CPU registers");
 }
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"newAddressAndSameAddress\":true,\"p2ReturnFollowsP1\":true,\"actualGpuDumpReproduced\":true,\"nativeFollowBodiesExecuted\":6,\"nativeAllocatorAndInitAreHostBoundaries\":true,\"gpuVerified\":false,\"gameLaunched\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
