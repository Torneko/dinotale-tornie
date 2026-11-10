// Native death dispatch/global ending/registration/destructors execute from
// the lifecycle corpus. Scene scheduling and actor factory remain HOST.
#define main lifecycle_regression_main
#include "CoopLifecycle0280Checks.cpp"
#undef main
namespace at=workshop::coop_attempt0281;
static constexpr uint32_t replacement=0x80110000,replacement2=0x80114000,newfruit=0x80118000;
static void observe(Fixture& f,unsigned tick,unsigned stage,unsigned mask=0){b::attempt_observation(f.r,&f.x,tick,stage,5,43,mask);}
static void end_team(Fixture& f){
 need(f.kill(1)&&f.kill(0),"Both actual native dispatcher calls finish exactly once");
 need(ends==1&&l::lifecycle.snapshot().ending&&at::attempts.phase()==at::Phase::awaiting_native_retry,"True native global ending arms only the retry handshake");
 need(at::attempts.lost_mask()==((1u<<0)|(1u<<5)),"Only two positively ended colors become unavailable");
 const auto before=f.ram;need(b::available_mask(0x3FFF)==(0x3FFF&~((1u<<0)|(1u<<5)))&&f.ram==before,"Color callback filters both ended colors without guest life writes");
}
static void scene_teardown(Fixture& f){
 // HOST native-scene scheduling selects the real existing destruction path.
 for(auto root:{first,second})if(n::actor(f.r,root,0x4001)){f.x.r4=int32_t(root);func_800445A4(f.r,&f.x);}
 need(n::owned.empty()&&n::cleanup_leases.empty(),"Actual free hooks clear the previous owned graph and cleanup leases");
 observe(f,2600,22);need(!b::context&&at::attempts.phase()==at::Phase::awaiting_native_retry,"Outside-gameplay fence closes only old views after real actor removal");
}
static void new_primary(Fixture& f,unsigned color=1){
 birth(f.r,replacement,0x4001,0);init(f.r,replacement);
 for(unsigned field:{0x84,0x88,0x8C})w(f.r,replacement+field,std::bit_cast<uint32_t>(field==0x84?56.f:field==0x88?449.f:500.34f));
 f.x.r4=int32_t(replacement);func_800440E4(f.r,&f.x);
 w(f.r,workshop::coop::native_player,replacement);w(f.r,workshop::coop::custom_color,color);w(f.r,workshop::coop::native_color,workshop::coop::native_alias(color));
 need(u(f.r,table+20)==replacement&&n::primary_ready(f.r,replacement),"True native4001 registration establishes the new canonical slot5");
 birth(f.r,newfruit,0x4030,0); // Actual creation hook, no address inference.
}
int main(){try{
 {Fixture f;end_team(f);const auto before=f.ram;observe(f,2550,7,0x3FFF);need(at::attempts.phase()==at::Phase::awaiting_native_retry&&f.ram==before,"An ending while old roots still exist cannot rearm or change payload");scene_teardown(f);new_primary(f);const auto root=*c::observed_identity(f.r,replacement),fruit=*c::observed_identity(f.r,newfruit);const auto before_retry=f.ram;const auto provider=b::input;const bool interactions=c::registry.interactions();observe(f,2700,7,0x3FFF);
  need(at::attempts.phase()==at::Phase::rearmed&&!b::invalidated&&!n::attempted&&n::enabled,"A genuine fresh native attempt selectively rearms factory locks");
  need(c::observed_identity(f.r,replacement)==root&&c::observed_identity(f.r,newfruit)==fruit&&f.ram==before_retry,"Every8MiBguest byte and new P1/fruit birth are preserved by rearm");
  need(b::input==provider&&c::registry.interactions()==interactions&&n::secondary_color()!=0&&n::secondary_color()!=5&&n::secondary_color()!=1,"Input affinity/settings survive and the new partner uses a distinct available color");
  observe(f,2701,7,0x3FFF);need(at::attempts.phase()==at::Phase::rearmed,"Asynchronous second construction does not discard the grant");
  // HOST factory boundary supplies a genuinely created second incarnation;
  // generic registration and the completed Bridge fence remain native/real.
  birth(f.r,replacement2,0x4001,0);init(f.r,replacement2);n::primary=replacement;need(n::register_secondary(f.r,&f.x,replacement2),"Second fresh root registers generically while preserving canonical5");n::secondary=replacement2;n::remember(f.r,replacement2);n::attempted=true;f.fence(2702);
  need(!b::invalidated&&b::context&&b::bound_primary==replacement&&b::bound_secondary==replacement2&&at::attempts.phase()==at::Phase::active,"Real Bridge fence binds the fresh pair after the selective retry");
  need(l::lifecycle.snapshot().epoch>1&&at::attempts.lost_mask()==((1u<<0)|(1u<<5)),"New attempt epoch does not revive either ended color");
 }
 for(unsigned bad=0;bad<14;bad++){
  Fixture f;end_team(f);scene_teardown(f);new_primary(f);
  switch(bad){case 0:b::owner_stack.push_back(1);break;case 1:workshop::coop_native_input::depth=1;break;case 2:workshop::coop_egg_pools::state.depth=1;break;case 3:n::constructing=true;break;case 4:n::cleanup_leases[first]={0x4001,0};break;case 5:n::owned[first]={0x4001,0};break;case 6:workshop::coop_rooms_native0280::admission.emplace();break;case 7:by(f.r,replacement+0x17D,4);break;case 8:w(f.r,table+20,replacement2);break;case 9:w(f.r,workshop::coop::custom_color,0);break;case 10:workshop::coop_egg_pools::state.allocation=0x803D0000;break;case 11:h(f.r,replacement+0x17E,6);break;case 12:w(f.r,table+24,replacement);break;case 13:by(f.r,first+6,0);break;}
  const auto before=f.ram;observe(f,2700,7,0x3FFF);need(at::attempts.phase()==at::Phase::awaiting_native_retry&&f.ram==before,"Scope/lease/owned/carry/pool/canonical/color corruption cannot rearm or mutate guest state");
  b::owner_stack.clear();workshop::coop_native_input::depth=workshop::coop_egg_pools::state.depth=0;workshop::coop_egg_pools::state.allocation=0;n::constructing=false;
 }
 {Fixture f;need(b::sync_attempt_binding(),"Actual pair is known before an invariant failure");b::invalidated=true;observe(f,2501,22);need(!at::attempts.lost_mask(),"Invariant failure is never disguised as a positive two-player death");scene_teardown(f);new_primary(f);const auto before=f.ram;observe(f,2700,7,0x3FFF);need(at::attempts.phase()==at::Phase::rearmed&&!at::attempts.lost_mask()&&f.ram==before,"Actual scene exit/new canonical birth can retry a failed attempt using native available colors");}
 {Fixture f;f.hp(0,0);f.hp(1,0);need(b::sync_attempt_binding(),"Zero-health but nonfatal pair stays tracked");const auto before=f.ram;observe(f,2501,7,0x3FFF);need(at::attempts.phase()==at::Phase::active&&f.ram==before,"HP0 alone is neither a team ending nor a retry");}
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"nativeEndingAndRegistration\":true,\"actorFactorySceneScheduling\":\"HOST\",\"gameLaunched\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
