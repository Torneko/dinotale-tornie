#pragma once
#include "boss_native.hpp"
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace workshop::boss_solo_fight {
// Host session state for the first playable milestone only. The caller must
// obtain the existing strict native DefeatObservation after807AEA78 returns,
// retire its46EE cinematic actor natively, then use the warm transport route.
// This module never mutates RDRAM, launches a room, issues a result, or awards
// a full ten-fight completion. Native room-ready/carry validation stays in the
// bridge; arrive_rest is called only after the carried Yoshi is restored.
enum class Phase { idle, armed, fighting, victory_ready, departing, rest, milestone_finished, failed };
struct Snapshot {
    Phase phase=Phase::idle;
    std::string run_token,failure;
    unsigned rest_room=43,completion_vi=0;
    bool diagnostic=false;
    uint32_t player=0,candy=0,transition=0;
};
class Session {
    Snapshot state_;
    bool fail(std::string reason){state_.phase=Phase::failed;state_.failure=std::move(reason);return false;}
    static bool token_valid(std::string_view token){
        if(token.empty() || token.size()>128)return false;
        for(unsigned char c:token)if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'))return false;
        return true;
    }
public:
    bool begin(std::string token,bool diagnostic=false,unsigned rest_room=43){
        state_={};
        if(!token_valid(token) || rest_room>255 || rest_room==boss_native::first_room)return fail("invalid_session_configuration");
        state_.phase=Phase::armed;state_.run_token=std::move(token);state_.diagnostic=diagnostic;state_.rest_room=rest_room;return true;
    }
    void cancel(std::string reason){if(state_.phase!=Phase::idle)fail(std::move(reason));}
    void reset(){state_={};}
    const Snapshot& snapshot()const{return state_;}
    bool active()const{return state_.phase!=Phase::idle && state_.phase!=Phase::failed && state_.phase!=Phase::milestone_finished;}
    bool enter_room(unsigned source_room){
        if(state_.phase==Phase::armed && source_room==boss_native::first_room){state_.phase=Phase::fighting;return true;}
        if(state_.phase==Phase::fighting && source_room==boss_native::first_room)return true;
        if(state_.phase==Phase::departing && source_room==state_.rest_room)return true;
        if(state_.phase==Phase::rest && source_room==state_.rest_room)return true;
        return active()?fail("unexpected_room_or_phase"):false;
    }
    bool accept_defeat(const boss_native::DefeatObservation& proof){
        if(state_.phase!=Phase::fighting)return false;
        if(proof.run_token!=state_.run_token || proof.diagnostic!=state_.diagnostic || proof.stage!=boss_native::first_stage || proof.source_room!=boss_native::first_room || proof.initial_segments!=int(boss_native::original_segments) || proof.remaining_segments<0 || proof.remaining_segments>int(boss_native::original_segments) || !proof.vi || !boss_native::range(proof.actor,0x1FC) || !boss_native::range(proof.player,0x1FC) || !boss_native::range(proof.transition_actor,0x1FC) || proof.actor==proof.player || proof.actor==proof.transition_actor || proof.player==proof.transition_actor)return fail("native_defeat_scope_mismatch");
        state_.candy=proof.actor;state_.player=proof.player;state_.transition=proof.transition_actor;state_.completion_vi=proof.vi;state_.phase=Phase::victory_ready;return true;
    }
    bool begin_departure(unsigned destination){
        if(state_.phase!=Phase::victory_ready)return false;
        if(destination!=state_.rest_room)return fail("wrong_rest_destination");
        state_.phase=Phase::departing;return true;
    }
    bool arrive_rest(uint8_t* rdram,unsigned source_room,uint32_t restored_player){
        if(state_.phase!=Phase::departing)return false;
        if(source_room!=state_.rest_room || restored_player!=state_.player || !boss_native::alive_player(rdram,restored_player) || boss_native::word(rdram,restored_player+0x180)!=0)return fail("rest_carry_identity_mismatch");
        state_.phase=Phase::rest;return true;
    }
    // The first milestone ends at the onward pot. No second boss is silently
    // substituted and no ten-fight challenge can be granted by this state.
    bool request_next(){if(state_.phase!=Phase::rest)return false;state_.phase=Phase::milestone_finished;return true;}
};
} // namespace workshop::boss_solo_fight
