#pragma once
#include <array>
#include <cstdint>
#include <compare>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
#include <algorithm>

namespace workshop::coop {
enum class Role { local,host,guest };
struct Config {
    Role role=Role::local;
    bool player_interactions=false;
    unsigned fruit_target=30;
    uint64_t respawn_delay_ticks=300; // Proposed five seconds at 60 Hz; configurable.
};
struct Capabilities {
    bool prepared_team_rules=true;
    bool prepared_owner_context=true;
    bool native_actor_factory_identified=true;
    bool native_second_actor_proven=false;
    bool native_controller_two_routed=false;
    bool native_collision_filter_proven=false;
    bool native_independent_death_proven=false;
    bool native_camera_proven=false;
    bool local_coop_playable=false;
    bool host_join_playable=false;
};
inline constexpr Capabilities capabilities{};
struct ActorHandle {
    uint32_t address=0,birth=0;
    auto operator<=>(const ActorHandle&)const=default;
    explicit operator bool()const{return address>=0x80000400 && address<0x807FFE00 && birth!=0;}
};
struct EntityToken {
    uint64_t epoch=0,id=0;
    auto operator<=>(const EntityToken&)const=default;
};
struct Player {
    ActorHandle actor{};
    unsigned color=0,health=0;
    bool connected=false,alive=false,respawn_requested=false;
    std::optional<uint64_t> respawn_at;
};
struct RespawnRequest {unsigned player,anchor_player;uint64_t due_tick;};
struct InputFrame {uint64_t sequence=0,tick=0;uint16_t buttons=0;int8_t x=0,y=0;};
class Session {
    Config config_{};
    bool active_=false;
    uint64_t epoch_=0;
    std::array<Player,2> players_{};
    std::set<EntityToken> fruits_,enemies_;
    std::map<ActorHandle,unsigned> projectiles_;
    std::array<std::optional<InputFrame>,2> inputs_{};
    bool valid_player(unsigned p)const{return active_ && p<2 && players_[p].connected;}
    bool authority()const{return config_.role!=Role::guest;}
    bool token(EntityToken t)const{return active_ && t.epoch==epoch_ && t.id!=0;}
public:
    void start(uint64_t epoch,Config config={}) {
        if(!epoch || !config.fruit_target || config.fruit_target>999 || config.respawn_delay_ticks>60*60*60)throw std::invalid_argument("Invalid co-op session configuration");
        config_=config;epoch_=epoch;active_=true;players_={};inputs_={};fruits_.clear();enemies_.clear();projectiles_.clear();
    }
    void stop(){active_=false;players_={};inputs_={};projectiles_.clear();fruits_.clear();enemies_.clear();}
    bool active()const{return active_;}
    uint64_t epoch()const{return epoch_;}
    const Player& player(unsigned index)const{return players_.at(index);}
    void bind(unsigned index,ActorHandle actor,unsigned color,unsigned health) {
        if(!active_ || index>=2 || !actor || color>=14 || !health || health>8)throw std::invalid_argument("Invalid co-op player binding");
        for(unsigned p=0;p<2;p++)if(p!=index && players_[p].connected && players_[p].actor.address==actor.address)throw std::invalid_argument("A native actor cannot represent two players");
        players_[index]={actor,color,health,true,true,false,{}};
    }
    std::optional<unsigned> owner(ActorHandle actor)const {
        for(unsigned p=0;p<2;p++)if(players_[p].connected && players_[p].actor==actor)return p;
        auto found=projectiles_.find(actor);return found==projectiles_.end()?std::nullopt:std::optional<unsigned>(found->second);
    }
    bool collect(unsigned p,EntityToken object) {
        return authority() && valid_player(p) && players_[p].alive && token(object) && fruits_.size()<config_.fruit_target && fruits_.insert(object).second;
    }
    bool defeat(unsigned p,EntityToken object) {
        return authority() && valid_player(p) && players_[p].alive && token(object) && enemies_.insert(object).second;
    }
    // Only a transport-validated authoritative host message may call this.
    // Clients never admit their own requests through collect()/defeat().
    bool apply_host_collection(EntityToken object) {
        return config_.role==Role::guest && token(object) && fruits_.size()<config_.fruit_target && fruits_.insert(object).second;
    }
    unsigned fruit_count()const{return unsigned(fruits_.size());}
    unsigned defeated_count()const{return unsigned(enemies_.size());}
    bool fruit_target_reached()const{return active_ && fruits_.size()>=config_.fruit_target;}
    bool players_collide(unsigned a,unsigned b)const{return a!=b && valid_player(a) && valid_player(b) && players_[a].alive && players_[b].alive && config_.player_interactions;}
    void projectile(ActorHandle projectile,unsigned owner_player) {
        if(!projectile || !valid_player(owner_player))throw std::invalid_argument("Invalid projectile owner");
        projectiles_[projectile]=owner_player;
    }
    void forget(ActorHandle actor){projectiles_.erase(actor);}
    bool egg_may_affect_teammate(ActorHandle egg,ActorHandle target)const {
        const auto source=owner(egg),other=owner(target);
        return !(source && other && *source!=*other); // Always pass through teammate; independent from interaction option.
    }
    bool die(unsigned p,uint64_t now) {
        if(!authority() || !valid_player(p) || !players_[p].alive)return false;
        auto& player=players_[p];player.alive=false;player.health=0;player.respawn_requested=false;
        player.respawn_at=now>UINT64_MAX-config_.respawn_delay_ticks?UINT64_MAX:now+config_.respawn_delay_ticks;
        return true;
    }
    std::vector<RespawnRequest> advance(uint64_t now) {
        std::vector<RespawnRequest> result;if(!active_ || !authority())return result;
        for(unsigned p=0;p<2;p++){auto& player=players_[p];unsigned other=1-p;
            if(player.connected && !player.alive && player.respawn_at && !player.respawn_requested && now>=*player.respawn_at && valid_player(other) && players_[other].alive){
                player.respawn_requested=true;result.push_back({p,other,*player.respawn_at});
            }
        }return result;
    }
    bool defer_respawn(unsigned p) {
        if(!valid_player(p) || players_[p].alive || !players_[p].respawn_requested)return false;
        players_[p].respawn_requested=false;return true;
    }
    bool confirm_respawn(unsigned p,ActorHandle actor,unsigned health) {
        if(!valid_player(p) || players_[p].alive || !players_[p].respawn_requested || !valid_player(1-p) || !players_[1-p].alive || !actor || !health || health>8 || actor.address==players_[1-p].actor.address)return false;
        auto& player=players_[p];player.actor=actor;player.health=health;player.alive=true;player.respawn_at.reset();player.respawn_requested=false;return true;
    }
    bool both_down()const{return valid_player(0) && valid_player(1) && !players_[0].alive && !players_[1].alive;}
    bool input(unsigned p,InputFrame frame,uint64_t host_tick) {
        if(!valid_player(p) || frame.x< -80 || frame.x>80 || frame.y< -80 || frame.y>80 || (frame.tick>host_tick && frame.tick-host_tick>8) || (host_tick>frame.tick && host_tick-frame.tick>180) || inputs_[p] && frame.sequence<=inputs_[p]->sequence)return false;
        inputs_[p]=frame;return true;
    }
    std::optional<InputFrame> input(unsigned p)const{return p<2?inputs_[p]:std::nullopt;}
};

// Native identity and colour/health globals are singletons. This prepared
// transaction virtualizes only proven scalar fields. It does not overwrite
// actor lists, allocators, camera structures, controller buffers or save data.
// A live adapter still needs wrappers around update/draw/collision, complete
// controller conversion and lifecycle/room/death integration before play.
class ScalarMemory {
public:
    virtual uint32_t read(uint32_t address)=0;
    virtual void write(uint32_t address,uint32_t value)=0;
    virtual ~ScalarMemory()=default;
};
inline constexpr uint32_t native_player=0x800F8828,native_color=0x800F8DB0,custom_color=0x807F0008,native_health=0x800F8DB4;
inline constexpr unsigned native_alias(unsigned color){return color<8?color:0;}
class OwnerContext {
public:
    struct View {ActorHandle actor{};unsigned color=0,health=1;};
private:
    struct Snapshot {uint32_t actor,color,custom;std::array<uint32_t,8> health;std::optional<unsigned> previous;unsigned entered;};
    ScalarMemory& memory_;
    std::array<View,2> players_{};
    std::optional<unsigned> current_;
    std::vector<Snapshot> stack_;
    void capture_current(){
        if(current_)players_[*current_].health=memory_.read(native_health+native_alias(players_[*current_].color)*4);
        else for(auto& p:players_)if(p.actor && p.actor.address==memory_.read(native_player))p.health=memory_.read(native_health+native_alias(p.color)*4);
    }
public:
    explicit OwnerContext(ScalarMemory& memory):memory_(memory){}
    void bind(unsigned index,View view){
        if(!stack_.empty() || index>=2 || !view.actor || view.color>=14 || !view.health || view.health>8)throw std::invalid_argument("Invalid owner-context binding");
        if(players_[1-index].actor && players_[1-index].actor.address==view.actor.address)throw std::invalid_argument("Owner-context actor is shared");
        players_[index]=view;
    }
    const View& player(unsigned index)const{return players_.at(index);}
    void enter(unsigned index){
        if(index>=2 || !players_[index].actor)throw std::invalid_argument("Unknown owner context");
        capture_current();
        Snapshot saved{memory_.read(native_player),memory_.read(native_color),memory_.read(custom_color),{},current_,index};
        for(unsigned i=0;i<8;i++)saved.health[i]=memory_.read(native_health+i*4);
        stack_.push_back(saved);current_=index;
        const auto& p=players_[index];memory_.write(native_player,p.actor.address);memory_.write(native_color,native_alias(p.color));memory_.write(custom_color,p.color);memory_.write(native_health+native_alias(p.color)*4,p.health);
    }
    void leave(){
        if(stack_.empty())throw std::logic_error("Owner-context stack underflow");
        capture_current();const auto saved=stack_.back();stack_.pop_back();
        for(unsigned i=0;i<8;i++)memory_.write(native_health+i*4,saved.health[i]);
        memory_.write(native_player,saved.actor);memory_.write(native_color,saved.color);memory_.write(custom_color,saved.custom);current_=saved.previous;
        if(current_)memory_.write(native_health+native_alias(players_[*current_].color)*4,players_[*current_].health);
        else for(const auto& p:players_)if(p.actor && p.actor.address==saved.actor)memory_.write(native_health+native_alias(p.color)*4,p.health);
    }
    unsigned depth()const{return unsigned(stack_.size());}
};
class ScopedOwner {
    OwnerContext* owner_;
public:
    ScopedOwner(OwnerContext& owner,unsigned index):owner_(&owner){owner_->enter(index);}
    ~ScopedOwner(){if(owner_)owner_->leave();}
    ScopedOwner(const ScopedOwner&)=delete;
    ScopedOwner& operator=(const ScopedOwner&)=delete;
};
}
