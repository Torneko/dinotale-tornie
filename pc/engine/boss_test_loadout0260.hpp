#pragma once
#include "boss_checkpoint_inventory0260.hpp"
#include <cstdio>
#include <string>

namespace workshop::boss_test_loadout0260 {
namespace inventory=boss_checkpoint_inventory0260;namespace n=boss_native;
// Explicit visible manual/automatic QA request only. This is not checkpoint restoration and
// supplies no death receipt, victory, progression, award or save operation.
inline constexpr bool mixed_fish_tuple(uint64_t seed,unsigned index){return (seed==7&&index==3)||(seed==17&&index==2);}
inline constexpr bool fish_tuple(uint64_t seed,unsigned index){return (seed==1&&index==4)||mixed_fish_tuple(seed,index);}
struct ReadyWitness {unsigned vi=0,index=10,room=177,source=177,epoch=0;uint32_t player=0,private_data=0;};
struct Frame {
 bool requested=false,manual=false,diagnostic=false,visible=false;
 unsigned case_index=10,index=10,room=177,source_room=177,vi=0;
 bool fighting=false;uint32_t player=0;bool automatic=false;
 bool lips_target=false,proved_zero_controls=false;bool genie_target=false;unsigned target_eggs=6;bool monster_target=false;bool final_target=false;
 bool fish_target=false;uint64_t seed=0;ReadyWitness ready{};unsigned epoch=0;bool native_request_recent=false;unsigned native_request_vi=0;std::array<uint32_t,2> fish_roots{};bool cp6_focused_target=false,rest_ready=false;
};
struct State {
 bool attempted=false,applied=false,failed=false;unsigned vi=0,added=0;uint32_t player=0;
 std::string failure;
};
class Controller final {
 State state_;
 bool fail(const char* why){state_.failed=true;state_.failure=why;return false;}
public:
 void reset(){state_={};}
 const State& snapshot()const{return state_;}
 // True means inert/waiting or successfully applied; snapshot.applied is the
 // sole readiness signal. A consumed partial native failure cannot be retried.
 bool frame(uint8_t* ram,recomp_context* context,const Frame& f){
  if(!f.requested)return true;
  if(state_.failed)return false;
  if(state_.attempted)return state_.applied;
  const bool focused=f.cp6_focused_target,lips=f.lips_target,genie=f.genie_target,genie_pot=genie&&f.seed==249908710788600ull&&f.case_index==7,monster=f.monster_target,final=f.final_target,fish=f.fish_target,mixed=fish&&mixed_fish_tuple(f.seed,f.case_index),targeted=lips||genie||monster||final||fish||focused;
  const unsigned target_count=unsigned(lips)+unsigned(genie)+unsigned(monster)+unsigned(final)+unsigned(fish)+unsigned(focused),target_case=focused?5u:fish?f.case_index:final?9u:monster?3u:(genie?(genie_pot?7u:2u):1u);
  if(focused&&(f.seed!=37784098137700ull||f.case_index!=5||f.epoch))return fail("test_loadout_focused_tuple_rejected");
  if(fish&&!fish_tuple(f.seed,f.case_index))return fail("test_loadout_fish_tuple_rejected");
  if(target_count>1||f.manual==f.automatic||!f.diagnostic||!f.visible||(targeted?(!f.automatic||f.manual||f.case_index!=target_case):f.case_index!=4))return fail("test_loadout_scope_rejected");
  if(f.target_eggs!=6&&(!genie||f.target_eggs!=0))return fail("test_loadout_target_count_rejected");
  if((focused?!f.rest_ready:!f.fighting)||(!mixed&&!focused&&f.vi<2408))return true;
  if(targeted?(focused?(f.index!=5||f.room!=83||f.source_room!=83):fish?(f.index!=f.case_index||f.room!=167+f.index||f.source_room!=97):final?(f.index!=9||f.room!=176||f.source_room!=162):monster?(f.index!=3||f.room!=170||f.source_room!=151):genie?(f.index!=(genie_pot?7u:2u)||f.room!=(genie_pot?174u:169u)||f.source_room!=163):(f.index!=1||f.room!=168||f.source_room!=165)):(f.index!=4||f.room!=171||f.source_room!=97))return fail("test_loadout_scene_rejected");
  if(!ram||!context||!n::actor(ram,f.player,0x4001)||n::word(ram,0x800F8828)!=f.player)return fail("test_loadout_player_rejected");
  const auto primary=n::byte(ram,f.player+0x18);
  if(!primary||primary==0xA||primary==0xFF||n::byte(ram,f.player+0x17D)!=2||n::byte(ram,f.player+0x165)||n::word(ram,f.player+0x180))return true;
  const auto data=n::word(ram,f.player+0x1F4);
  if(!inventory::pointer(data,0x12F0))return fail("test_loadout_private_rejected");
  if(focused){const auto& r=f.ready;if(!r.vi||f.vi<=r.vi)return true;if(r.index!=5||r.room!=83||r.source!=83||r.epoch||r.player!=f.player||r.private_data!=data)return fail("test_loadout_focused_ready_rejected");if(primary!=1||n::byte(ram,data+0x1F8))return true;}
  if(mixed){
   const auto& r=f.ready;if(!r.vi||f.vi<=r.vi)return true;
   if(r.index!=f.index||r.room!=f.room||r.source!=97||r.epoch!=f.epoch||f.epoch||r.player!=f.player||r.private_data!=data)return fail("test_loadout_ready_identity_rejected");
   if(!f.native_request_recent||f.native_request_vi<=r.vi||f.native_request_vi>f.vi||f.vi-f.native_request_vi>2)return true;
   if(primary!=1||n::byte(ram,f.player+0x30)==0x14||n::byte(ram,data+0x12C9)||n::byte(ram,data+0x1F8))return true;
   const std::array<uint16_t,2> kinds{0x4511,uint16_t(f.seed==7?0x4599:0x4072)};
   if(f.fish_roots[0]==f.fish_roots[1]||f.fish_roots[0]==f.player||f.fish_roots[1]==f.player)return fail("test_loadout_member_identity_rejected");
   for(unsigned member=0;member<2;++member){const auto a=f.fish_roots[member];if(!a)return true;if(!n::actor(ram,a,kinds[member]))return fail("test_loadout_member_kind_rejected");if(n::byte(ram,a+0x17D)!=2||!inventory::pointer(n::word(ram,a+0x1F4),4))return true;}
  }
  const auto controls=n::byte(ram,0x800F7EA6);
  if(n::word(ram,data+0x12EC)||n::byte(ram,0x800F7EA5)||(controls!=1&&!((lips||genie)&&f.proved_zero_controls&&controls==0)))return true;
  inventory::Payload target;if(!inventory::capture(ram,f.player,f.vi,target))return fail("test_loadout_inventory_rejected");
  inventory::Inventory original;if(!inventory::read(ram,f.player,original))return fail("test_loadout_capacity_rejected");
  if(!f.target_eggs&&original.count)return fail("test_loadout_nonempty_reserves_rejected");
  if(inventory::half(ram,f.player+0x184)+(f.target_eggs-original.count)>20)return fail("test_loadout_capacity_rejected");
  const auto stack=uint32_t(context->r29);if(stack<0x80000800||!inventory::pointer(stack-0x400,0x480))return fail("test_loadout_stack_rejected");
  const auto initial_hp=target.hp;target.hp=8;target.count=f.target_eggs;
  inventory::Scale scale{};if((original.count<f.target_eggs||final||fish)&&!inventory::normal_scale(ram,f.player,scale))return fail("test_loadout_native_scale_rejected");
  for(unsigned i=original.count;i<f.target_eggs;++i)target.eggs[i]={0x4004,target.pc_color,scale};
  if(!inventory::valid(target))return fail("test_loadout_target_rejected");
  struct Registers {recomp_context* c;recomp_context saved;~Registers(){*c=saved;}} registers{context,*context};
  context->r29=ADD32(context->r29,-0x80);
  state_.attempted=true;state_.vi=f.vi;state_.player=f.player;state_.added=f.target_eggs-original.count;
  for(unsigned i=original.count;i<f.target_eggs;++i){
   inventory::Inventory before;if(!inventory::same_player(ram,target)||!inventory::read(ram,f.player,before)||before.count!=i)return fail("test_loadout_birth_scope_rejected");
   const auto scale_address=uint32_t(context->r29)+0x40;
   if(!inventory::stage_scale(ram,scale_address,target.eggs[i].saved_scale))return fail("test_loadout_native_scale_stage_rejected");
   context->r4=n::base(f.player);context->r5=n::base(f.player+0x84);context->r6=n::base(scale_address);func_80040860(ram,context);
   inventory::Inventory after;if(!inventory::read(ram,f.player,after)||after.count!=i+1)return fail("test_loadout_native_birth_failed");
   for(unsigned j=0;j<i;++j)if(after.actors[j]!=before.actors[j]||after.eggs[j]!=target.eggs[j])return fail("test_loadout_prefix_changed");
   if(after.eggs[i].kind!=0x4004)return fail("test_loadout_native_egg_rejected");
   context->r4=n::base(after.actors[i]);context->r5=target.eggs[i].palette;func_8003B8B0(ram,context);
   if(!inventory::read(ram,f.player,after)||after.count!=i+1||after.eggs[i]!=target.eggs[i])return fail("test_loadout_native_palette_failed");
  }
  if(!inventory::same_player(ram,target))return fail("test_loadout_identity_changed");
  if(initial_hp!=8){context->r4=8;func_800422CC(ram,context);}
  if(!inventory::matches(ram,target))return fail("test_loadout_native_readback_failed");
  state_.applied=true;
  if(mixed)std::fprintf(stderr,"[BossTestLoadoutAdmission] case=%u seed=%llu index=%u room=%u source=%u player=%08X private=%08X readyVI=%u requestVI=%u vi=%u fish=%08X companion=%08X companionKind=%04X epoch=%u nativeControls=1 typedPlayer=1 typedMembers=1 nativeRequest=1 nativeObservation=1\n",f.case_index,static_cast<unsigned long long>(f.seed),f.index,f.room,f.source_room,f.player,data,f.ready.vi,f.native_request_vi,f.vi,f.fish_roots[0],f.fish_roots[1],unsigned(f.seed==7?0x4599:0x4072),f.epoch);
  std::fprintf(stderr,focused?"[BossFishCP6FixtureQA] phase=initial-native-loadout case=%u index=%u room=%u player=%08X hp=8 eggs=%u added=%u vi=%u mode=%s forcedLoadout=1 nativeReadback=1 once=1\n":"[BossTestLoadout] case=%u index=%u room=%u player=%08X hp=8 eggs=%u added=%u vi=%u mode=%s forcedLoadout=1 nativeReadback=1\n",f.case_index,f.index,f.room,f.player,f.target_eggs,state_.added,f.vi,f.automatic?"automatic":"manual");
  if(final||fish)std::fprintf(stderr,"[BossTestLoadoutScale] case=%u player=%08X added=%u scaleBits=%08X,%08X,%08X source=800AFFE8 normalReadback=1 vi=%u\n",f.case_index,f.player,state_.added,scale[0],scale[1],scale[2],f.vi);
  return true;
 }
};
}
