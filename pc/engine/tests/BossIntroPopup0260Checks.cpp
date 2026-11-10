// Isolated CPU guards plus two unmodified translated native request functions.
// No game, renderer, native boss physics, victory or permanent award is run.
#include "../boss_intro0260.hpp"
#include <array>
#include <bit>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace intro=workshop::boss_intro0260;
namespace n=workshop::boss_native;
static unsigned checks=0;
static void need(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
extern "C" void func_800492C0(uint8_t*,recomp_context*);
extern "C" void func_80515DF8(uint8_t*,recomp_context*);
extern "C" void func_80046804(uint8_t*,recomp_context*);
extern "C" void func_80044194(uint8_t*,recomp_context*);
static recomp_func_t* oracle_lookup(uint32_t);
#undef LOOKUP_FUNC
#define LOOKUP_FUNC(val) oracle_lookup(uint32_t(val))
#ifndef BOSS_INTRO_NATIVE_SLICE
#error Supply the unmodified translated native dialog slice.
#endif
#include BOSS_INTRO_NATIVE_SLICE
static std::array<int32_t,300> section_storage{};
int32_t* section_addresses=section_storage.data();
static const intro::Frame* current_frame=nullptr;
static unsigned native_lookup=0,suppressed=0,vanilla_allocation=0,registration=0;
static bool allow_suppression=false;
static constexpr uint32_t popup=0x80170000;
static recomp_func_t* oracle_lookup(uint32_t at){
 need(at==uint32_t(section_addresses[91])+0x958,"Native task table selects actual80515DF8");
 ++native_lookup;return func_80515DF8;
}
extern "C" void func_80046804(uint8_t* r,recomp_context* c){
 need(current_frame!=nullptr,"Native allocation has an explicit campaign frame");
 const auto proof=intro::observe_dialog_creation(r,c,*current_frame);
 if(allow_suppression&&proof){++suppressed;c->r2=0;return;}
 ++vanilla_allocation;c->r2=n::base(popup); // ABI double; not a real popup constructor.
}
extern "C" void func_80044194(uint8_t*,recomp_context* c){
 need(uint32_t(c->r4)==popup,"Only the unsuppressed native popup is registered");++registration;c->r2=1;
}
struct Fixture {
 static constexpr uint32_t root=0x80100000,priv=0x80110000,dialog=0x80200000,family=0x80210000,caller_sp=0x80780000;
 std::vector<uint8_t> mem=std::vector<uint8_t>(0x800000);uint8_t* rdram=mem.data();uint8_t* r=rdram;recomp_context c{};
 std::array<intro::Member,5> members{};intro::Frame f;intro::Spec spec;
 void w(uint32_t at,uint32_t v){MEM_W(0,n::base(at))=int32_t(v);}
 void by(uint32_t at,unsigned v){MEM_B(0,n::base(at))=uint8_t(v);}
 void actor(uint32_t at,uint16_t kind){w(at,0x19971110);MEM_H(12,n::base(at))=kind;by(at+0x17D,2);by(at+0x18,1);}
 Fixture(unsigned which,unsigned encounter=1,unsigned logical=0):spec(intro::specs[which]){
  f={true,true,encounter,encounter?167+encounter:162,logical?logical:spec.room,dialog,{}};
  members[0]={root,priv,spec.kind,family};f.members={members.data(),1};
  actor(root,spec.kind);w(root+0x1F4,priv);w(root+0x20,family+spec.intro-spec.rom_base);
  c.r29=n::base(caller_sp-0x58);c.r31=n::base(dialog+0x98C);c.r4=n::base(dialog+0x1130);c.r5=0x40882200;c.r16=n::base(root);
  const auto sp=uint32_t(c.r29);w(sp+0x1C,0x80049308);w(sp+0x28,sp+0x48);w(sp+0x3C,family+spec.caller_ra-spec.rom_base);w(sp+0x54,uint32_t(spec.text_id));w(sp+0x78,root);
 }
 auto query(){return intro::observe_dialog_creation(r,&c,f);}
};
int main(){try{
 constexpr std::array<std::pair<unsigned,unsigned>,6> owned_sources{{{0,162},{1,164},{2,163},{3,165},{1,97},{2,97}}};
 need(intro::source_allows(97,0x4599)&&intro::source_allows(97,0x4072)&&!intro::source_allows(97,0x455A)&&!intro::source_allows(97,0x40A3)&&!intro::source_allows(97,0x4511),"Fish source admits only its two configured companion kinds, not every boss kind");
 for(unsigned s=0;s<intro::specs.size();++s)for(unsigned encounter=0;encounter<10;++encounter){
  Fixture x(s,encounter);const auto before=x.mem;const auto context=x.c;auto p=x.query();
  need(p&&p->member==0&&p->actor==Fixture::root&&p->text_id==x.spec.text_id,"Four original boss requests recognized at every encounter clone");
  need(x.mem==before&&std::memcmp(&x.c,&context,sizeof(context))==0,"Popup observer preserves every byte of8MiB and all registers");
 }
 for(auto [s,logical]:std::array<std::pair<unsigned,unsigned>,5>{{{0,165},{1,165},{2,162},{1,97},{2,97}}}){
  Fixture x(s,4,logical);const auto before=x.mem;const auto context=x.c;need(x.query().has_value(),"Native Candy/Inviso/Genie companions use the owned mixed arena");need(x.mem==before&&std::memcmp(&x.c,&context,sizeof(context))==0,"Mixed-source admission changes neither guest memory nor registers");
 }
 for(auto [s,logical]:owned_sources)for(unsigned fault=0;fault<34;++fault){
  Fixture x(s,1,logical);const auto sp=uint32_t(x.c.r29);
  switch(fault){
   case 0:x.f.active=false;break;case 1:x.f.fighting=false;break;case 2:x.f.encounter=10;break;
   case 3:++x.f.physical_room;break;case 4:x.f.logical_room=151;break;case 5:x.f.dialog_overlay=0x807FFFFC;break;
   case 6:x.f.members={};break;case 7:x.f.members={x.members.data(),5};break;case 8:x.c.r29+=1;break;
   case 9:x.c.r31+=4;break;case 10:x.c.r4+=4;break;case 11:x.c.r5+=1;break;
   case 12:x.w(sp+0x1C,0x80049304);break;case 13:x.w(sp+0x28,sp+0x4C);break;
   case 14:x.w(sp+0x3C,Fixture::family+x.spec.caller_ra-x.spec.rom_base+4);break;case 15:x.w(sp+0x54,uint32_t(x.spec.text_id-1));break;
   case 16:x.w(sp+0x78,Fixture::root+0x200);x.c.r16=n::base(Fixture::root+0x200);break;
   case 17:x.members[0].actor+=0x200;break;case 18:x.w(Fixture::root,0);break;case 19:x.by(Fixture::root+6,3);break;
   case 20:x.actor(Fixture::root,0x4001);break;case 21:x.by(Fixture::root+0x17D,3);break;
   case 22:x.by(Fixture::root+0x18,2);break;case 23:x.members[0].private_data=0x807FFFFC;break;
   case 24:x.w(Fixture::root+0x1F4,Fixture::priv+0x100);break;case 25:x.w(Fixture::root+0x20,Fixture::family+x.spec.intro-x.spec.rom_base+4);break;
   case 26:x.members[0].family_overlay+=4;break;case 27:x.members[0].kind=0x41C6;break;
   case 28:x.members[1]=x.members[0];x.f.members={x.members.data(),2};break;case 29:x.f.logical_room=s==1?163:s==2?164:97;break;
   case 30:x.f.dialog_overlay+=4;break;case 31:x.c.r29=n::base(0x807FFFFC);break;
   case 32:x.members[0].private_data=0;break;case 33:x.members[0].family_overlay=0;break;
  }
  const auto before=x.mem;const auto context=x.c;need(!x.query(),"Foreign, malformed, reused or non-intro dialog is not suppressed");
  need(x.mem==before&&std::memcmp(&x.c,&context,sizeof(context))==0,"Failed guards change no native data or registers");
 }
 {Fixture x(0);need(!intro::observe_dialog_creation(nullptr,&x.c,x.f)&&!intro::observe_dialog_creation(x.r,nullptr,x.f),"Null inputs fail closed");}
 // Run the actual800492C0 +80515DF8 bodies, not a synthetic frame calculation.
 // Eight modes distinguish enabled owned requests from normal/foreign paths.
 for(auto [s,logical]:owned_sources)for(unsigned mode=0;mode<8;++mode){
  Fixture x(s,1,logical);const auto sp=Fixture::caller_sp;current_frame=&x.f;allow_suppression=mode!=1;
  x.c.r29=n::base(sp);x.c.r31=n::base(Fixture::family+x.spec.caller_ra-x.spec.rom_base);x.c.r4=x.spec.text_id;
  x.w(sp+0x20,Fixture::root); // Real Candy caller's saved A3; other callers use S0.
  x.w(0x800F8318,0x80140000);x.w(0x80140030,std::bit_cast<uint32_t>(104.f));x.w(0x80140034,std::bit_cast<uint32_t>(168.f));x.w(0x80140038,std::bit_cast<uint32_t>(500.f));
  x.w(0x800F8908,0x80150000);x.w(0x80150010,0x80151000);x.w(0x80151020,Fixture::dialog+0x958);
  x.w(0x800F8E58,0);x.by(0x800F7EA5,0);x.by(0x800F7EA6,0);section_addresses[91]=int32_t(Fixture::dialog);
  if(mode==2)x.f.active=false;if(mode==3)x.f.fighting=false;if(mode==4)x.c.r31+=4;
  if(mode==5)x.c.r4=x.spec.text_id-1;if(mode==6)x.members[0].actor+=4;if(mode==7)x.f.logical_room=151;
  native_lookup=suppressed=vanilla_allocation=registration=0;const auto root_bytes=std::vector<uint8_t>(x.mem.begin()+(Fixture::root&0x7FFFFF),x.mem.begin()+(Fixture::priv&0x7FFFFF)+0x100);
  func_800492C0(x.r,&x.c);
  need(native_lookup==1&&uint32_t(x.c.r29)==sp,"Actual native request dispatch runs and restores its stack");
  if(mode==0){need(suppressed==1&&vanilla_allocation==0&&registration==0&&x.c.r2==0,"Owned Boss Battle request takes actual null-allocation branch with no popup or registration");}
  else{need(suppressed==0&&vanilla_allocation==1&&registration==1&&uint32_t(x.c.r2)==popup,"Unowned/disabled/foreign request preserves the original native allocation and registration path");}
  need(n::byte(x.r,0x800F7EA5)==0&&n::byte(x.r,0x800F7EA6)==0,"Request suppression does not set scene or player control flags");
  need(std::equal(root_bytes.begin(),root_bytes.end(),x.mem.begin()+(Fixture::root&0x7FFFFF)),"Request functions change no boss/player state, health, timer or callback");
 }
 std::cout<<"BossIntroPopup0260 "<<checks<<" CPU checks passed; native request/allocation branch, not rendering or boss gameplay proof\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
