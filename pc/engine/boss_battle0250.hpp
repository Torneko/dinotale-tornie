#pragma once
#include <array>
#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <cmath>
#include <limits>
#include "boss_checkpoint0260.hpp"
namespace workshop::boss_battle0250 {
enum class Family : uint8_t { Candy,Invisible,Genie,Lips,Fish,Monsters,RedCandy,Blargg };
inline constexpr std::array<Family,7> ordinary_families{Family::Candy,Family::Invisible,Family::Genie,Family::Lips,Family::Fish,Family::Monsters,Family::Blargg};
inline constexpr unsigned fish_hits=2; // Default two-fish variant; each member owns its configured hit count.
struct Member { uint16_t kind=0;unsigned hits=0;float scale=1;bool red=false,purple=false,green=false; };
struct Encounter {
    unsigned number=0;Family family=Family::Candy;unsigned variant=0,terrain=0,count=0;
    std::array<Member,4> members{};bool genie_companion=false;uint8_t purple_mask=0;
    unsigned birds=0,bird_respawn_ms=8000;bool no_flowers=false,no_egg_blocks=false;
};
// Initial balancing values are explicit and adjustable for playtesting. The
// confirmed hit counts, probabilities, family coverage and ten-fight order
// below remain fixed regardless of these presentation/supply settings.
struct Config {
    float two_candy_scale=.75f,three_candy_scale=.60f;
    unsigned yellow_window_ms=3000,reference_birds=2,more_birds=3,fewer_birds=1,bird_respawn_ms=8000;
    unsigned fish_birds=3,fish_respawn_ms=3000;
    unsigned yellow_healing=1;bool terrain_variants=true;
    // Explicit opt-in while the new native family awaits isolated game proof.
    // False retains every legacy seed and the existing ten-fight roster.
    bool blargg=false;
    bool valid()const{return std::isfinite(two_candy_scale)&&std::isfinite(three_candy_scale)&&two_candy_scale>0&&two_candy_scale<1&&three_candy_scale<two_candy_scale&&yellow_window_ms>0&&yellow_window_ms<=30000&&reference_birds>0&&more_birds>reference_birds&&fewer_birds>0&&fewer_birds<reference_birds&&more_birds<=8&&bird_respawn_ms>=1000&&fish_birds>0&&fish_birds<=3&&fish_respawn_ms>=1000&&fish_respawn_ms<=60000&&yellow_healing>0;}
};
struct Random {
    uint64_t state;
    uint64_t next(){uint64_t z=(state+=0x9E3779B97F4A7C15ull);z=(z^(z>>30))*0xBF58476D1CE4E5B9ull;z=(z^(z>>27))*0x94D049BB133111EBull;return z^(z>>31);}
    unsigned pick(unsigned n){if(!n)return 0;const uint64_t limit=(uint64_t(0)-n)%n;uint64_t v;do{v=next();}while(v<limit);return unsigned(v%n);}
};
inline constexpr uint16_t room(Family f){switch(f){case Family::Candy:case Family::RedCandy:return 162;case Family::Invisible:return 164;case Family::Genie:return 163;case Family::Lips:return 165;case Family::Fish:return 97;case Family::Monsters:return 151;case Family::Blargg:return 19;}return 177;}
inline constexpr std::string_view name(Family f){switch(f){case Family::Candy:return "Bonbon jaune";case Family::Invisible:return "Invisible";case Family::Genie:return "Génie";case Family::Lips:return "Lèvres géantes";case Family::Fish:return "Poissons géants";case Family::Monsters:return "Monstres roses et blancs";case Family::RedCandy:return "Bonbon rouge final";case Family::Blargg:return "Double Blargg";}return "";}
inline bool fish_member(const Member& m){return m.kind==0x4511&&!m.red&&!(m.green&&m.purple);}
inline bool valid_fish(const Encounter& e){
    if(e.family!=Family::Fish||e.variant>2)return false;
    const unsigned fish_count=e.variant==0?3:e.variant==1?2:1;
    if(e.count!=(e.variant==0?3:2))return false;
    uint8_t purple=0;
    for(unsigned i=0;i<fish_count;++i){const auto& m=e.members[i];if(!fish_member(m)||m.scale!=1.f||m.hits!=(e.variant==0?1:e.variant==1?2:m.hits)||!m.hits||m.hits>3)return false;if(m.purple)purple|=uint8_t(1u<<i);}
    if(e.variant==2){const auto& c=e.members[1];if(e.members[0].hits<2||(c.kind!=0x4072&&c.kind!=0x4599)||c.hits!=2||c.scale!=1.f||c.red||c.green||c.purple||e.genie_companion!=(c.kind==0x4072))return false;}
    else if(e.genie_companion)return false;
    return e.purple_mask==purple;
}
inline bool valid_blargg(const Encounter& e,const Config& c){
    if(!c.blargg||e.family!=Family::Blargg||e.variant!=0||e.terrain>2||e.count!=2||e.genie_companion||e.purple_mask||e.birds!=c.reference_birds||e.bird_respawn_ms!=c.bird_respawn_ms||!e.no_flowers||!e.no_egg_blocks)return false;
    for(unsigned i=0;i<e.count;i++){const auto& m=e.members[i];if(m.kind!=0x41EC||m.hits!=3||m.scale!=1.f||m.red||m.purple||m.green)return false;}
    return true;
}
inline Encounter encounter(unsigned number,Family f,unsigned variant,Random& random,const Config& c){
    Encounter e;e.number=number;e.family=f;e.variant=variant;e.terrain=number==1||!c.terrain_variants?0:random.pick(3);e.bird_respawn_ms=c.bird_respawn_ms;
    auto add=[&](uint16_t kind,unsigned hits,float scale=1,bool red=false,bool purple=false){e.members[e.count++]={kind,hits,scale,red,purple};};
    switch(f){
    case Family::Candy:
        if(variant==0){add(0x455A,12);if(number!=1&&random.pick(100)<10){e.genie_companion=true;add(0x4072,2);e.birds=c.reference_birds;}}
        else {unsigned n=variant==1?2:3;for(unsigned i=0;i<n;i++)add(0x455A,12,variant==1?c.two_candy_scale:c.three_candy_scale);}break;
    case Family::Invisible:case Family::Genie:{e.birds=c.reference_birds;unsigned n=variant==0?1:variant==1?2:4,h=variant==0?3:variant==1?2:1;for(unsigned i=0;i<n;i++)add(f==Family::Invisible?0x4599:0x4072,h);break;}
    case Family::Lips:e.birds=c.reference_birds;add(0x40A3,3);if(variant==1)add(0x455A,12,c.two_candy_scale);else if(variant==2)add(0x4599,3);break;
    case Family::Fish:{
        // Consume the former fish draw so a changed fish roster does not
        // reshuffle the later families/terrains of an existing campaign seed.
        const unsigned legacy_roll=random.pick(100);if(legacy_roll>=5&&legacy_roll<15)(void)random.pick(2);
        Random colors{random.state^(uint64_t(number)*0xD6E8FEB86659FD93ull)^(uint64_t(variant)*0xA0761D6478BD642Full)};
        const unsigned count=variant==0?3:variant==1?2:1;
        for(unsigned i=0;i<count;++i){unsigned color=variant==0?i:variant==1?i:colors.pick(3);
            // Five percent total per fish, equally split between the other
            // two colors. All colors use the jumping red native actor.
            if(colors.pick(100)<5)color=(color+1+colors.pick(2))%3;
            const unsigned hits=variant==0?1:variant==1?2:2+colors.pick(2);
            e.members[e.count++]={0x4511,hits,1,false,color==2,color==1};
            if(color==2)e.purple_mask|=uint8_t(1u<<i);
        }
        if(variant==2){e.genie_companion=colors.pick(2)==0;add(e.genie_companion?0x4072:0x4599,2);}
        e.birds=c.fish_birds;e.bird_respawn_ms=c.fish_respawn_ms;break;}
    case Family::Monsters:add(0x41C6,variant==0?4:variant==1?3:5);add(0x41C6,variant==0?4:variant==1?3:5);e.no_flowers=e.no_egg_blocks=true;e.birds=variant==0?c.reference_birds:variant==1?c.more_birds:c.fewer_birds;break;
    case Family::Blargg:e.variant=0;add(0x41EC,3);add(0x41EC,3);e.birds=c.reference_birds;e.no_flowers=e.no_egg_blocks=true;break;
    case Family::RedCandy:add(0x455A,12,1,true);e.birds=c.reference_birds;break;
    }
    return e;
}
struct Plan {uint64_t seed=0;Config config;std::array<Encounter,10> fights{};};
inline Plan make_plan(uint64_t seed,const Config& c={}){
    Plan p;p.seed=seed;p.config=c;if(!c.valid())return p;Random random{seed};p.fights[0]=encounter(1,Family::Candy,0,random,c);
    std::array<Family,5> guaranteed{Family::Invisible,Family::Genie,Family::Lips,Family::Fish,c.blargg?Family::Blargg:Family::Monsters};for(unsigned i=4;i>0;i--)std::swap(guaranteed[i],guaranteed[random.pick(i+1)]);
    for(unsigned i=0;i<5;i++){unsigned v=random.pick(3);p.fights[i+1]=encounter(i+2,guaranteed[i],v,random,c);}
    for(unsigned i=6;i<9;i++){auto f=ordinary_families[random.pick(c.blargg?7:6)];unsigned v=random.pick(3);p.fights[i]=encounter(i+1,f,v,random,c);}p.fights[9]=encounter(10,Family::RedCandy,0,random,c);return p;
}
inline bool valid(const Plan& p){
    if(!p.config.valid())return false;unsigned seen=0;
    for(unsigned i=0;i<10;i++){const auto& e=p.fights[i];if(e.number!=i+1||e.count<1||e.count>4||e.terrain>2||e.variant>2||unsigned(e.family)>7||(!p.config.blargg&&e.family==Family::Blargg))return false;if(i<6)seen|=1u<<unsigned(e.family);if(i<9&&e.family==Family::RedCandy)return false;for(unsigned n=0;n<e.count;n++){const auto& m=e.members[n];if(!m.kind||!m.hits||!std::isfinite(m.scale)||m.scale<=0||m.scale>1||(m.green&&(e.family!=Family::Fish||m.kind!=0x4511||m.purple)))return false;}}
    for(const auto& e:p.fights)if(e.family==Family::Fish){if(e.birds!=p.config.fish_birds||e.bird_respawn_ms!=p.config.fish_respawn_ms||!valid_fish(e))return false;}
    for(const auto& e:p.fights)if(e.family==Family::Invisible||e.family==Family::Genie)if(e.birds!=p.config.reference_birds||e.bird_respawn_ms!=p.config.bird_respawn_ms)return false;
    for(const auto& e:p.fights)if(e.family==Family::Blargg&&!valid_blargg(e,p.config))return false;
    return seen==(p.config.blargg?0x9Fu:0x3Fu)&&p.fights[0].family==Family::Candy&&p.fights[0].variant==0&&p.fights[0].count==1&&!p.fights[0].genie_companion&&p.fights[9].family==Family::RedCandy&&p.fights[9].count==1&&p.fights[9].members[0].red&&p.fights[9].members[0].scale==1;
}
enum class Phase { Idle,Armed,Fighting,Victory,ToRest,Rest,ToFight,Complete,Failed,RetryToRest,RetryToFight };
struct NativeReceipt {unsigned encounter=0,member=0,vi=0;uint32_t actor=0;bool initialized=false,active=false,terminal=false,causal_hit=false;unsigned retry_epoch=0;};
struct Snapshot {Phase phase=Phase::Idle;unsigned index=0,completed=0;uint32_t player=0;std::array<uint32_t,4> actors{};uint8_t defeated=0;bool diagnostic=false;std::string token,failure;unsigned checkpoint_completed=0,max_completed=0,retry_epoch=0,checkpoint_awake_mask=0;};
class Session {
    Plan plan_{};Snapshot state_{};bool skipped_entry_=false;
    bool fail(std::string s){state_.phase=Phase::Failed;state_.failure=std::move(s);return false;}
public:
    bool begin(std::string token,Plan plan,bool diagnostic){state_={};skipped_entry_=false;if(!valid(plan)||token.empty()||token.size()>128||!std::all_of(token.begin(),token.end(),[](unsigned char c){return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_';}))return fail("invalid_battle_configuration");plan_=plan;state_.phase=Phase::Armed;state_.diagnostic=diagnostic;state_.token=std::move(token);return true;}
    const Snapshot& snapshot()const{return state_;}const Plan& plan()const{return plan_;}const Encounter& current()const{return plan_.fights[state_.index];}
    // Diagnostic cold entry skips no actual victories: completed stays zero.
    // It is unavailable to normal/manual sessions and cannot certify ten fights.
    bool diagnostic_entry(unsigned index){if(!state_.diagnostic||state_.phase!=Phase::Armed||index>=10)return false;state_.index=index;skipped_entry_|=index!=0;return true;}
    // Explicit cold-Rest QA starting fixture: six UNPLAYED HOST fights.
    // No native victory, actor, wake, checkpoint, transport or award is made.
    bool diagnostic_cp6_rest6_fixture(){
        if(!state_.diagnostic||state_.phase!=Phase::Armed||!skipped_entry_||state_.index!=5||state_.completed||state_.max_completed||state_.player||state_.retry_epoch||state_.checkpoint_completed||state_.checkpoint_awake_mask||state_.defeated||plan_.seed!=37784098137700ull||!valid(plan_))return false;
        const auto& f=plan_.fights[6];if(f.family!=Family::Fish||f.variant!=2||f.terrain!=1||f.count!=2||f.members[0].kind!=0x4511||f.members[0].hits!=3||f.members[1].kind!=0x4072||f.members[1].hits!=2)return false;
        state_.completed=state_.max_completed=6;skipped_entry_=false;return true;
    }
    bool diagnostic_bind_initial_rest6(uint32_t real_player){
        if(!state_.diagnostic||state_.phase!=Phase::Armed||skipped_entry_||state_.index!=5||state_.completed!=6||state_.max_completed!=6||state_.player||!real_player||state_.retry_epoch||state_.checkpoint_completed||state_.checkpoint_awake_mask||state_.defeated||plan_.seed!=37784098137700ull)return false;
        state_.player=real_player;state_.phase=Phase::Rest;return true;
    }
    void cancel(std::string s){fail(std::move(s));}
    bool enter_fight(unsigned source,uint32_t player){if((state_.phase!=Phase::Armed&&state_.phase!=Phase::ToFight)||source!=room(current().family)||!player||(state_.player&&state_.player!=player))return fail("fight_entry_identity_mismatch");state_.player=player;state_.phase=Phase::Fighting;state_.actors={};state_.defeated=0;return true;}
    bool bind(unsigned member,uint32_t actor){if(state_.phase!=Phase::Fighting||member>=current().count||!actor||actor==state_.player||state_.actors[member])return false;for(auto a:state_.actors)if(a==actor)return false;state_.actors[member]=actor;return true;}
    bool defeat(const NativeReceipt& r){if(state_.phase!=Phase::Fighting||r.encounter!=state_.index||r.member>=current().count||!r.actor||state_.actors[r.member]!=r.actor||(state_.defeated&(1u<<r.member))||!r.vi||!r.initialized||!r.active||!r.terminal||!r.causal_hit||r.retry_epoch!=state_.retry_epoch)return false;state_.defeated|=uint8_t(1u<<r.member);if(state_.defeated==((1u<<current().count)-1)){state_.completed++;state_.max_completed=std::max(state_.max_completed,state_.completed);state_.phase=Phase::Victory;}return true;}
    bool use_victory_pot(){if(state_.phase!=Phase::Victory)return false;if(state_.completed==10){state_.phase=Phase::Complete;return true;}state_.phase=Phase::ToRest;return true;}
    bool arrive_rest(uint32_t player){if(state_.phase!=Phase::ToRest||player!=state_.player)return false;state_.phase=Phase::Rest;return true;}
    bool use_rest_pot(){if(state_.phase!=Phase::Rest||state_.completed>=10||state_.index>=9)return false;++state_.index;state_.phase=Phase::ToFight;state_.actors={};state_.defeated=0;return true;}
    // The native adapter supplies the milestone only after a real wake of the
    // matching numbered character. Waking it never moves or advances Yoshi.
    bool awaken_checkpoint(unsigned milestone){
        const auto bit=boss_checkpoint0260::awake_bit(milestone);
        if(skipped_entry_||!state_.player||!bit||(state_.checkpoint_awake_mask&bit))return false;
        if(!milestone){
            const bool first=(state_.phase==Phase::Fighting&&state_.completed==0)||(state_.phase==Phase::Victory&&state_.completed==1);
            if(!first||state_.index||state_.checkpoint_completed)return false;
        }else if(state_.phase!=Phase::Rest||state_.index+1!=state_.completed||
                 !boss_checkpoint0260::can_awaken(state_.completed,state_.checkpoint_completed,milestone))return false;
        state_.checkpoint_completed=milestone;state_.checkpoint_awake_mask|=bit;return true;
    }
    // No destination argument exists: death may return only to the last wake.
    // This host transition creates no native receipts, health, eggs or awards.
    bool retry_checkpoint(){
        if(skipped_entry_||state_.phase!=Phase::Failed||state_.failure!="yoshi_defeated"||!state_.player||
           !boss_checkpoint0260::valid(state_.checkpoint_completed)||state_.checkpoint_completed>state_.completed||
           state_.max_completed<state_.completed||(state_.checkpoint_awake_mask&~15u)||
           (state_.checkpoint_completed&&!(state_.checkpoint_awake_mask&boss_checkpoint0260::awake_bit(state_.checkpoint_completed)))||
           state_.retry_epoch==std::numeric_limits<unsigned>::max())return false;
        state_.completed=state_.checkpoint_completed;state_.index=boss_checkpoint0260::resume_index(state_.completed);
        ++state_.retry_epoch;state_.actors={};state_.defeated=0;state_.failure.clear();
        state_.phase=state_.completed?Phase::RetryToRest:Phase::RetryToFight;return true;
    }
    bool arrive_retry(unsigned source,uint32_t player){
        if(!player||player!=state_.player)return false;
        if(state_.phase==Phase::RetryToRest&&source==boss_checkpoint0260::rest_room){state_.phase=Phase::Rest;return true;}
        if(state_.phase==Phase::RetryToFight&&source==room(current().family)){state_.phase=Phase::Fighting;return true;}
        return false;
    }
};
} // namespace workshop::boss_battle0250
