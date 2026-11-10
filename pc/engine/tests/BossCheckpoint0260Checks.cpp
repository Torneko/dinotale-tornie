#include "../boss_battle0250.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace b=workshop::boss_battle0250;
namespace p=workshop::boss_checkpoint0260;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static_assert(unsigned(b::Phase::Idle)==0&&unsigned(b::Phase::Failed)==8);
static_assert(unsigned(b::Phase::RetryToRest)==9&&unsigned(b::Phase::RetryToFight)==10);

// Every victory below is an explicit synthetic host-model receipt. This test
// executes no native gameplay, uses no profile and opens no challenge ledger.
struct Fixture {
    b::Session session;unsigned vi=100;
    static constexpr uint32_t player=0x80010000;
    std::array<unsigned char,sizeof(b::Plan)> plan_bytes{};
    explicit Fixture(uint64_t seed=1,bool diagnostic=false){
        need(session.begin("checkpoint-synthetic-0260",b::make_plan(seed),diagnostic),"Begin explicit synthetic session");
        std::memcpy(plan_bytes.data(),&session.plan(),plan_bytes.size());
    }
    uint32_t actor(unsigned member)const{return 0x80100000+session.snapshot().index*0x1000+member*0x200;}
    b::NativeReceipt receipt(unsigned member){return {session.snapshot().index,member,++vi,actor(member),true,true,true,true,session.snapshot().retry_epoch};}
    void enter(){
        if(session.snapshot().phase==b::Phase::Rest)need(session.use_rest_pot(),"Only ordinary rest pot advances encounter");
        need(session.enter_fight(b::room(session.current().family),player),"Same player enters the next planned fight");
    }
    void win(){
        if(session.snapshot().phase!=b::Phase::Fighting)enter();
        const unsigned before=session.snapshot().completed;
        for(unsigned n=0;n<session.current().count;++n){
            need(session.bind(n,actor(n)),"Fresh exact synthetic member bound");
            auto r=receipt(n),bad=r;bad.causal_hit=false;
            need(!session.defeat(bad),"Checkpoint paths retain causal hit requirement");
            need(session.defeat(r),"All configured members require explicit receipts");
        }
        need(session.snapshot().phase==b::Phase::Victory&&session.snapshot().completed==before+1,"One actual model victory increments count once");
    }
    void rest(){need(session.use_victory_pot()&&session.arrive_rest(player),"Normal post-victory native-shaped rest transition");}
    void reach_rest(unsigned completed){while(session.snapshot().completed<completed){win();rest();}}
    void preserved(unsigned checkpoint,unsigned maximum,unsigned epoch){
        const auto& s=session.snapshot();
        need(s.player==player&&s.token=="checkpoint-synthetic-0260","Retry preserves exact player and campaign token");
        need(std::memcmp(plan_bytes.data(),&session.plan(),plan_bytes.size())==0,"Entire plan, future variants, config and seed unchanged");
        need(s.checkpoint_completed==checkpoint&&s.max_completed==maximum&&s.retry_epoch==epoch,"Latest wake, maximum genuine count and epoch retained");
        need(!s.defeated&&s.actors==std::array<uint32_t,4>{},"Retry discards current member progress and actor bindings");
    }
    void die(){session.cancel("yoshi_defeated");need(session.retry_checkpoint(),"Explicit death may retry only the last wake");}
    void return_rest(unsigned completed,unsigned maximum,unsigned epoch){
        need(session.snapshot().phase==b::Phase::RetryToRest&&session.snapshot().completed==completed&&session.snapshot().index==completed-1,"Checkpoint returns to its completed rest, not the following fight");
        preserved(completed,maximum,epoch);
        need(!session.use_rest_pot()&&!session.use_victory_pot()&&!session.awaken_checkpoint(9),"Pending return cannot advance, complete or wake");
        need(!session.arrive_retry(83,player+4)&&!session.arrive_retry(162,player),"Wrong player and wrong retry room rejected");
        need(session.arrive_retry(83,player)&&session.snapshot().phase==b::Phase::Rest,"Same native actor must actually return to rest83");
        need(!session.arrive_retry(83,player)&&!session.retry_checkpoint(),"Arrival and retry cannot replay while alive");
    }
};

int main(){try{
    for(unsigned n=0;n<=12;++n){
        need(p::valid(n)==(n==0||n==3||n==6||n==9),"Only four checkpoint values exist");
        need(p::number(n)==(p::valid(n)?n/3+1:0),"Checkpoint numbering is1 through4");
        need(p::awake_bit(n)==(p::valid(n)?1u<<(n/3):0),"Each real checkpoint has one distinct wake bit");
        need(p::eligible(n)==(n<=10?std::min(9u,n/3*3):0),"Eligibility floors completed groups without out-of-range promotion");
    }
    need(!p::can_awaken(3,0,6)&&!p::can_awaken(5,3,0)&&!p::can_awaken(10,6,9),"Future, backward and completed-run wake requests rejected");
    for(uint64_t seed=0;seed<12;++seed){
        Fixture f(seed);need(!f.session.awaken_checkpoint(3)&&!f.session.retry_checkpoint(),"Fresh armed session has no free checkpoint or retry");
        f.reach_rest(2);f.win();
        need(!f.session.awaken_checkpoint(3),"Victory alone cannot wake a checkpoint before rest");f.rest();
        need(!f.session.awaken_checkpoint(0)&&!f.session.awaken_checkpoint(6),"Base marker and future marker cannot advance checkpoint");
        // Leave the third-rest character asleep and wake it on the fifth rest.
        f.reach_rest(5);need(f.session.snapshot().checkpoint_completed==0,"Passing milestone does not automatically activate it");
        need(f.session.awaken_checkpoint(3)&&!f.session.awaken_checkpoint(3),"Late wake after5 activates3 exactly once without moving");
        need(f.session.snapshot().checkpoint_awake_mask==2,"Waking character2 does not fabricate a wake of character1");
        need(f.session.snapshot().completed==5&&f.session.snapshot().index==4,"Wake preserves current encounter and actual victories");
        f.die();f.return_rest(3,5,1);f.enter();
        need(f.session.snapshot().checkpoint_awake_mask==2,"Retry retains exact wake mask");
        const auto stale=b::NativeReceipt{3,0,50,f.actor(0),true,true,true,true,0};
        need(f.session.bind(0,f.actor(0)),"Recycled actor address explicitly rebound in new retry");
        need(!f.session.defeat(stale),"Old epoch receipt rejected even with same encounter and recycled address");
        auto wrong=stale;wrong.retry_epoch=2;need(!f.session.defeat(wrong),"Future epoch receipt rejected");
        wrong.retry_epoch=1;wrong.actor+=4;need(!f.session.defeat(wrong),"New epoch does not relax exact actor identity");
        // Finish this partially bound encounter with new, epoch-scoped receipts.
        need(f.session.defeat(f.receipt(0)),"Only a current epoch receipt can credit the new member");
        for(unsigned n=1;n<f.session.current().count;++n)need(f.session.bind(n,f.actor(n))&&f.session.defeat(f.receipt(n)),"Every replayed companion requires fresh proof");
        f.rest();f.reach_rest(7);
        need(f.session.awaken_checkpoint(6)&&!f.session.awaken_checkpoint(3),"Late wake after7 activates6; old marker cannot move backward");
        need(f.session.snapshot().checkpoint_awake_mask==6,"New wake adds only its own bit");
        f.reach_rest(8);f.die();f.return_rest(6,8,2);
        f.reach_rest(9);need(f.session.awaken_checkpoint(9),"Ninth victory permits the fourth character");
        need(f.session.snapshot().checkpoint_awake_mask==14,"All three later wakes leave skipped first character asleep");
        f.enter();need(f.session.snapshot().index==9&&f.session.snapshot().completed==9,"Ordinary pot reaches only final encounter");
        f.die();f.return_rest(9,9,3);
        need(f.session.snapshot().phase!=b::Phase::Complete,"Fourth checkpoint is not a completion");
        f.win();need(f.session.snapshot().completed==10&&f.session.snapshot().max_completed==10,"Final replay requires new causal victory");
        need(f.session.use_victory_pot()&&f.session.snapshot().phase==b::Phase::Complete,"Only final victory pot completes ten fights");
        need(!f.session.retry_checkpoint()&&!f.session.awaken_checkpoint(9),"Completed campaign cannot travel or reawaken");
    }
    for(unsigned passed:{1u,2u,3u,5u,6u,8u,9u}){
        Fixture f;f.reach_rest(passed);f.die();f.preserved(0,passed,1);
        need(f.session.snapshot().phase==b::Phase::RetryToFight&&f.session.snapshot().completed==0&&f.session.snapshot().index==0,"Unwoken milestones return only to beginning");
        need(!f.session.enter_fight(162,Fixture::player)&&f.session.snapshot().phase==b::Phase::Failed,"Ordinary entry cannot bypass pending retry arrival");
        // A separate fixture checks successful native-shaped return without
        // changing the fail-closed result of the attempted wrong entry above.
        Fixture g;g.reach_rest(passed);g.die();
        need(!g.session.arrive_retry(83,Fixture::player)&&!g.session.arrive_retry(162,Fixture::player+4),"Base retry also requires correct room and same actor");
        need(g.session.arrive_retry(162,Fixture::player)&&g.session.snapshot().phase==b::Phase::Fighting,"Base retry resumes first fight without selecting another Yoshi");
        need(g.session.snapshot().completed==0&&g.session.snapshot().max_completed==passed,"Base arrival creates no victory and retains maximum progress");
    }
    for(unsigned late:{4u,5u,7u,8u}){Fixture f;f.reach_rest(late);const auto count=f.session.snapshot().completed;need(f.session.awaken_checkpoint(late<6?3:6),"Every later rest within a group permits its missed wake");need(f.session.snapshot().completed==count&&f.session.snapshot().phase==b::Phase::Rest,"Late wake never teleports or changes victories");}
    {Fixture f;need(!f.session.awaken_checkpoint(0),"First character cannot wake before player entry");f.enter();need(f.session.awaken_checkpoint(0)&&!f.session.awaken_checkpoint(0),"First character can really wake once during first fight");need(f.session.snapshot().checkpoint_awake_mask==1&&!f.session.snapshot().completed&&!f.session.snapshot().checkpoint_completed&&f.session.snapshot().phase==b::Phase::Fighting,"First wake records its bit without moving or creating a victory");f.die();f.preserved(0,0,1);need(f.session.snapshot().checkpoint_awake_mask==1,"First wake survives defeat");need(f.session.arrive_retry(162,Fixture::player),"First wake returns same player to first encounter");f.reach_rest(6);need(f.session.awaken_checkpoint(6)&&f.session.snapshot().checkpoint_awake_mask==5,"Skipping second character activates only first and third bits");}
    for(unsigned departure:{0u,1u,2u}){Fixture f;f.win();const auto before=f.session.snapshot();need(f.session.awaken_checkpoint(0)&&!f.session.awaken_checkpoint(0),"First genuine wake can activate No1 once after first victory");const auto& awake=f.session.snapshot();need(awake.phase==b::Phase::Victory&&awake.index==0&&awake.completed==1&&awake.max_completed==1&&!awake.checkpoint_completed&&awake.checkpoint_awake_mask==1&&awake.retry_epoch==before.retry_epoch&&awake.player==before.player&&awake.actors==before.actors&&awake.defeated==before.defeated,"Victory wake records only bit0 without replaying victory, moving or clearing its real receipt");need(!f.session.retry_checkpoint()&&!f.session.awaken_checkpoint(3),"Victory wake alone never retries or unlocks a later checkpoint");if(departure){f.rest();need(!f.session.awaken_checkpoint(0),"First wake cannot replay from rest");if(departure==2)f.enter();}f.die();f.preserved(0,1,1);need(f.session.snapshot().phase==b::Phase::RetryToFight&&f.session.snapshot().index==0&&f.session.snapshot().completed==0&&f.session.snapshot().checkpoint_awake_mask==1,"Death after a victory wake returns only to checkpoint0 with exact sparse mask");need(f.session.arrive_retry(162,Fixture::player)&&!f.session.awaken_checkpoint(0),"Retry keeps exact player and forbids another No1 activation");need(f.session.bind(0,f.actor(0)),"New retry member explicitly bound after victory wake");auto stale=f.receipt(0);stale.retry_epoch=0;need(!f.session.defeat(stale),"Victory-wake retry rejects the previous epoch even at the same actor address");need(f.session.defeat(f.receipt(0)),"Replayed first victory requires fresh current-epoch native proof");need(f.session.snapshot().completed==1&&f.session.snapshot().max_completed==1&&f.session.snapshot().phase==b::Phase::Victory,"Retry victory neither skips the first fight nor duplicates maximum progress");}
    {Fixture f;f.win();f.rest();need(!f.session.awaken_checkpoint(0),"Unawakened No1 cannot activate after leaving its victory phase");f.enter();need(!f.session.awaken_checkpoint(0),"Later encounter cannot activate the first checkpoint backward");f.win();need(!f.session.awaken_checkpoint(0),"Second victory cannot impersonate the first No1 activation");}
    {Fixture f;f.reach_rest(6);need(f.session.awaken_checkpoint(6)&&f.session.snapshot().checkpoint_awake_mask==4,"A late first activation at6 does not awaken either lower checkpoint");need(!f.session.awaken_checkpoint(0)&&!f.session.awaken_checkpoint(3),"Lower asleep characters cannot switch a later checkpoint backward");f.die();f.return_rest(6,6,1);need(f.session.snapshot().checkpoint_awake_mask==4,"Sparse wake mask survives retry unchanged");}
    {Fixture f;f.reach_rest(3);need(f.session.awaken_checkpoint(3),"Setup wake before integrity failure");f.session.cancel("native_receipt_rejected");need(!f.session.retry_checkpoint()&&f.session.snapshot().phase==b::Phase::Failed,"Integrity failure cannot be reclassified as player defeat");}
    {Fixture f;f.session.cancel("yoshi_defeated");need(!f.session.retry_checkpoint(),"No player identity means no authorized retry");}
    {Fixture f(1,true);need(f.session.diagnostic_entry(3),"Legacy diagnostic cold entry remains available");f.reach_rest(3);need(!f.session.awaken_checkpoint(3),"Three cold-entry victories are not a proven prefix from first boss");f.session.cancel("yoshi_defeated");need(!f.session.retry_checkpoint(),"Cold entry cannot become a checkpoint-qualified campaign");}
    {Fixture f;f.reach_rest(3);need(f.session.awaken_checkpoint(3),"Reset setup checkpoint");f.die();need(f.session.begin("new-session",b::make_plan(9),false),"Explicit new campaign still starts fresh");const auto& s=f.session.snapshot();need(s.phase==b::Phase::Armed&&!s.completed&&!s.player&&!s.checkpoint_completed&&!s.max_completed&&!s.retry_epoch&&!s.checkpoint_awake_mask,"New campaign clears all checkpoint metadata");}
    std::cout<<"BossCheckpoint0260Checks: "<<checks<<" PASS; synthetic host model only, no native gameplay/profile/ledger.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
