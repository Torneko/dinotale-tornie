#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>
#include "FocusedAdmissionExpression.hpp"
// The expression is extracted byte-for-byte from workshop_loader.cpp's real
// focused require. This fixture tests argument admission, not native gameplay.
struct Input {
 bool boss_test_fish_cp6_focused=true,relative_play=true,boss_solo_driver=true;
 bool trace_startup=false,test_tally=false,test_hybrid=false,test_book_resume=false,test_book_shop=false,test_find_reveal=false,test_editor_eggs=false,test_release_spawn=false,test_coop_interactions=false,test_coop_egg_probe=false,test_coop_egg_stress=false,test_coop_shared_egg_pools=false,test_coop_room_fixture0280=false,test_coop_room_route_driver0280=false,test_coop_room_death0280=false,test_spawn=false,test_warp=false,boss_manual_handoff=false,background_test=false,boss_test_menu=false,test_coop_native=false,test_coop_manual=false,boss_test_fish_cp6=false,boss_test_route_checkpoints=false,boss_test_swallow_checkpoint=false,boss_test_full_loadout=false,boss_test_special_fish=false,boss_test_automatic=false,boss_test_lips_loadout=false,boss_test_genie_loadout=false,boss_test_genie_empty_reserves=false,boss_test_monsters_loadout=false,boss_test_checkpoint=false,boss_test_retry_replay=false,boss_test_fish_loadout=false,boss_test_final_loadout=false;
 int test_color=0,test_special_victory=-1,test_stage=-1,test_zone=-1,boss_test_case=5;
 unsigned fruit_seed=0,test_native_history=0,end_test_vi=30000;uint64_t boss_battle_seed=37784098137700ull;
 std::string boss_battle_token="focused-token",boss_test_goal="fish-cp6-focused-replay",custom_token,boss_solo_token,test_first_boss;
 std::vector<int> test_inputs{1},test_inputs_p2,boss_followup_inputs;std::vector<uint8_t> payload;
 bool admits()const{return FOCUSED_ADMISSION_EXPRESSION;}
};
static unsigned checks=0;static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
int main(){try{
 Input valid;need(valid.admits(),"Exact focused CLI scope admits");
 {auto core=valid;core.boss_test_goal="fish-cp6-rebirth";need(core.admits(),"Distinct rebirth goal admits only within the same isolated focused fixture scope");core.boss_test_case=0;need(!core.admits(),"Rebirth cannot reuse natural Full case0 admission");}
 for(auto member:{&Input::trace_startup,&Input::test_tally,&Input::test_hybrid,&Input::test_book_resume,&Input::test_book_shop,&Input::test_find_reveal,&Input::test_editor_eggs,&Input::test_release_spawn,&Input::test_coop_interactions,&Input::test_coop_egg_probe,&Input::test_coop_egg_stress,&Input::test_coop_shared_egg_pools,&Input::test_coop_room_fixture0280,&Input::test_coop_room_route_driver0280,&Input::test_coop_room_death0280,&Input::test_spawn,&Input::test_warp,&Input::boss_manual_handoff,&Input::background_test,&Input::boss_test_menu,&Input::test_coop_native,&Input::test_coop_manual,&Input::boss_test_fish_cp6,&Input::boss_test_route_checkpoints,&Input::boss_test_swallow_checkpoint,&Input::boss_test_full_loadout,&Input::boss_test_special_fish,&Input::boss_test_automatic,&Input::boss_test_lips_loadout,&Input::boss_test_genie_loadout,&Input::boss_test_genie_empty_reserves,&Input::boss_test_monsters_loadout,&Input::boss_test_checkpoint,&Input::boss_test_retry_replay,&Input::boss_test_fish_loadout,&Input::boss_test_final_loadout}){auto bad=valid;bad.*member=true;need(!bad.admits(),"Every foreign global write fixture or alternate QA mode fails closed");}
 for(auto member:{&Input::relative_play,&Input::boss_solo_driver}){auto bad=valid;bad.*member=false;need(!bad.admits(),"Focused input pilot ownership is required");}
 for(unsigned which=0;which<17;++which){auto bad=valid;switch(which){case 0:bad.boss_battle_token.clear();break;case 1:bad.boss_test_case=0;break;case 2:bad.boss_battle_seed=1;break;case 3:bad.boss_test_goal="fish-cp6-replay";break;case 4:bad.test_inputs.clear();break;case 5:bad.end_test_vi=11999;break;case 6:bad.end_test_vi=36001;break;case 7:bad.test_color=1;break;case 8:bad.test_special_victory=0;break;case 9:bad.fruit_seed=1;break;case 10:bad.test_native_history=64;break;case 11:bad.custom_token="custom";break;case 12:bad.payload={1};break;case 13:bad.boss_solo_token="solo";break;case 14:bad.test_first_boss="boss";break;case 15:bad.test_stage=0;break;case 16:bad.test_zone=0;break;}need(!bad.admits(),"Wrong CLI identity, seed, mode, boundary or payload fails before game launch");}
 {auto bad=valid;bad.test_inputs_p2={1};need(!bad.admits(),"Secondary inputs cannot join a solo fixture");bad=valid;bad.boss_followup_inputs={1};need(!bad.admits(),"Foreign followup script cannot join focused pilot");}
 for(unsigned boundary:{12000u,30000u,36000u}){auto in=valid;in.end_test_vi=boundary;need(in.admits(),"Exact bounded focused deadlines admitted");}
 {auto full=valid;full.boss_test_fish_cp6_focused=false;full.boss_test_fish_cp6=true;full.boss_test_case=0;full.boss_test_goal="fish-cp6-replay";need(full.admits(),"New focused guard remains inert for existing Full case0 admission");}
 std::printf("BossFishFocusedAdmission0284 PASS %u extracted-source guard checks; no engine/game/native execution\n",checks);return 0;
 }catch(const std::exception& e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
