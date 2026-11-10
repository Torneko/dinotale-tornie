// Reuse the complete historical oracle and its explicit native boundary stubs.
#define main legacy_starting_bonus_main
#include "StartingBonus0283Checks.cpp"
#undef main
namespace solo=workshop::solo_inventory0300;
int main(int argc,char** argv){
 const int legacy=legacy_starting_bonus_main(argc,argv);if(legacy)return legacy;
 try{
  const auto before_checks=checks;unsigned matrix=0;
  for(unsigned color:{14u,15u})for(unsigned hp=0;hp<=8;++hp)for(unsigned count=0;count<=6;++count){
   Fixture f(color,hp,count);
   const auto alias=solo::native_alias(color);word(f.ram,0x800F8DB0,alias);word(f.ram,0x800F8DB4+alias*4,hp);
   // Existing special reserves must retain their individual metallic palettes.
   for(unsigned i=0;i<count;++i)word(f.ram,0x80100000+i*0x400+0x1EC,14+(i&1));
   const auto before=f.memory;sb::NativePlan rejected;
   need(!sb::prepare(f.ram,&f.context,player,100,{true,true},rejected)&&before==f.memory,"Legacy/Boss adapter accepted a metallic color or mutated RAM");
   sb::NativePlan plan;need(sb::prepare(f.ram,&f.context,player,100,{true,true},plan,true),"Solo metallic preflight failed");
   need(plan.before.native_color==alias&&plan.before.pc_color==color&&plan.target.hp==8&&plan.target.count==6,"Solo plan changed alias or requested effects");
   const auto registers=f.context;const auto scratch=f.scratch();
   need(sb::apply_native(f.ram,&f.context,plan),"Native metallic starting effects failed");
   need(solo::matches(f.ram,plan.target)&&n::word(f.ram,0x800F8DB0)==alias&&n::word(f.ram,0x807F0008)==color,"Metallic identity lost through native effects");
   need(!std::memcmp(&registers,&f.context,sizeof registers)&&scratch==f.scratch(),"Metallic native effects damaged registers or scratch");
   need(allocations==6-count&&health_calls==unsigned(hp!=8),"Metallic native effects allocated/healed incorrect amounts");
   for(unsigned i=0;i<count;++i)need(plan.target.eggs[i]==plan.before.eggs[i],"Existing metallic egg changed palette or scale");
   ++matrix;
  }
  for(unsigned color:{14u,15u})for(auto mode:{sb::Mode::Story,sb::Mode::Trial,sb::Mode::Hybrid,sb::Mode::BossBattle}){
   Fixture f(color);const auto alias=solo::native_alias(color);word(f.ram,0x800F8DB0,alias);word(f.ram,0x800F8DB4+alias*4,3);
   w::Wallet wallet;const auto dir=fs::absolute(argv[1])/("metal-mode-"+std::to_string(color)+"-"+std::to_string(unsigned(mode)));fund(wallet,dir,{true,true});
   sb::Controller controller(1000+color*10+unsigned(mode));auto env=environment(mode);evidence(f,controller,env,true,false,mode==sb::Mode::BossBattle?52u:43u);
   const auto saved=bytes(dir/w::filename);controller.frame(f.ram,&f.context,env,wallet);
   if(mode==sb::Mode::BossBattle)need(!allocations&&!health_calls&&saved==bytes(dir/w::filename)&&wallet.snapshot().value.pending==w::Pending{true,true},"Deferred Boss mode spent metallic voucher");
   else need(controller.snapshot().phase==sb::Phase::Complete&&allocations==6&&health_calls==1&&wallet.snapshot().value.pending.empty(),"Solo mode did not apply/consume metallic voucher once");
  }
  nlohmann::json report{{"passed",true},{"metallicChecks",checks-before_checks},{"metallicMatrixCases",matrix},{"old14RegressionExecuted",true},{"nativeEffectsBodiesReused",true},{"inputsSynthetic",true},{"graphicsAllocatorAndSoundStubs",true},{"gameLaunched",false},{"personalProfileUsed",false},{"bossMetallicRejected",true},{"scratchAndContextPreserved",true}};
  put(fs::absolute(argv[1])/"metal-verification.json",report.dump(2)+'\n');std::cout<<report.dump(2)<<'\n';return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL0300: "<<e.what()<<'\n';return 1;}
}
