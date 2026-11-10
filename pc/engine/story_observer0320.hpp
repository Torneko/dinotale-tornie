#pragma once
#include "recomp.h"
#include <cstdint>
#include <optional>

namespace workshop::story_observer0320 {
struct Witness {
    uint64_t run_nonce=0,attempt=0;
    unsigned world=6,column=4;
    uint32_t player=0,private_data=0;
    bool armed=false,finished=false,consumed=false;
};
// Read-only native proof adapter. Root owns persistence and the ordinary-solo
// scope gate: exclude custom payloads, diagnostics, cooperative and Boss modes.
// Call frame only at the native manager fence(r4=800F7EA0). It must see a real
// 4001 with0 fruits/ending0 before accepting30 fruits/ending1 in that level.
// Final player/private identity is retained after native result-screen cleanup.
class NativeObserver {
    Witness proof_{};
    uint64_t nonce_=0,serial_=0;
    bool active_=false;
    enum class Kind {None,World,Bowser};
    struct Pending {Kind kind=Kind::None;uint64_t attempt=0;uint32_t stack=0,actor=0;unsigned world=6,column=4;};
    Pending pending_{};
    static bool range(uint32_t address,unsigned bytes){return !(address&3)&&address>=0x80000400&&address<0x80800000&&bytes<=0x80800000-address;}
    static gpr at(uint32_t address){return gpr(int32_t(address));}
    static uint32_t word(uint8_t* rdram,uint32_t address){return uint32_t(MEM_W(0,at(address)));}
    static unsigned byte(uint8_t* rdram,uint32_t address){return MEM_BU(0,at(address));}
    static bool native_story(uint8_t* rdram){return rdram&&word(rdram,0x800F8B88)==0;}
    static bool transition(uint8_t* rdram,unsigned scene){return byte(rdram,0x800F7EA1)==0&&byte(rdram,0x800F841D)==scene;}
    bool attempt_ready(uint8_t* rdram)const{return active_&&native_story(rdram)&&proof_.armed&&!proof_.consumed&&proof_.run_nonce==nonce_;}
    bool ready(uint8_t* rdram)const{return attempt_ready(rdram)&&proof_.finished;}
    bool same_level(uint8_t* rdram)const{return byte(rdram,0x800F8DFC)==proof_.world&&byte(rdram,0x800F8DFD)==proof_.column;}
    static bool bowser(uint8_t* rdram,uint32_t actor){return range(actor,0x200)&&word(rdram,actor)==0x19971110&&MEM_HU(12,at(actor))==0x41A4&&word(rdram,actor+0x1E4)==55;}
    void clear_attempt(){proof_={};pending_={};}
public:
    void start(unsigned mode,bool ordinary_scope,uint64_t run_nonce){reset();active_=mode==5&&ordinary_scope&&run_nonce!=0;nonce_=active_?run_nonce:0;}
    void reset(){active_=false;nonce_=serial_=0;clear_attempt();}
    Witness witness()const{return proof_;}
    void frame(uint8_t* rdram,const recomp_context* context){
        if(!active_||!rdram||!context||uint32_t(context->r4)!=0x800F7EA0)return;
        if(!native_story(rdram)){clear_attempt();return;}
        const unsigned world=byte(rdram,0x800F8DFC),column=byte(rdram,0x800F8DFD);
        if(world>=6||column>=4){if(!proof_.finished)clear_attempt();return;}
        const auto fruits=word(rdram,0x800F89D0),ending=word(rdram,0x800F8AC8);
        const auto player=word(rdram,0x800F8828);
        if(!range(player,0x200)||word(rdram,player)!=0x19971110||MEM_HU(12,at(player))!=0x4001)return;
        const auto data=word(rdram,player+0x1F4);if(!range(data,0x1300))return;
        if(fruits==0&&ending==0){
            if(!proof_.armed||proof_.finished||proof_.world!=world||proof_.column!=column){clear_attempt();proof_={nonce_,++serial_,world,column,player,data,true,false,false};}
            else {proof_.player=player;proof_.private_data=data;}
            return;
        }
        if(!proof_.armed||proof_.consumed)return;
        if(proof_.world!=world||proof_.column!=column){if(!proof_.finished)clear_attempt();return;}
        // Room changes can legitimately replace4001 after the initial zero.
        // Bind only the final winning identity; never require it after cleanup.
        if(!proof_.finished){proof_.player=player;proof_.private_data=data;if(fruits==30&&ending==1)proof_.finished=true;}
    }
    // Exact func_80710B24 fence BEFORE80710C2C (before its delay-slot store).
    // Native scene0/4 and packed continuation page+1 prove the world transition.
    // SP+3B/+3A are the native caller's saved page/column, not guessed globals.
    void arm_world_save(uint8_t* rdram,const recomp_context* context){
        pending_={};if(!context||!ready(rdram)||proof_.world>=5||!same_level(rdram)||!transition(rdram,4))return;
        const auto stack=uint32_t(context->r29);if(!range(stack,0x40))return;
        const auto world=byte(rdram,stack+0x3B),column=byte(rdram,stack+0x3A);
        if(world!=proof_.world||column!=proof_.column||(byte(rdram,0x800F9255)&7)!=world+1||word(rdram,0x800F8E00)!=1)return;
        pending_={Kind::World,proof_.attempt,stack,0,world,column};
    }
    // Exact fence BEFORE80710C34, which can also be reached WITHOUT a save.
    // Only a matching arm and native EEPROM return0 may emit one world event.
    std::optional<unsigned> finish_world_save(uint8_t* rdram,const recomp_context* context){
        const auto pending=pending_;pending_={};
        if(pending.kind!=Kind::World)return {};
        const bool accepted=context&&ready(rdram)&&pending.attempt==proof_.attempt&&uint32_t(context->r29)==pending.stack&&context->r2==0&&same_level(rdram)&&transition(rdram,4)&&byte(rdram,pending.stack+0x3B)==pending.world&&byte(rdram,pending.stack+0x3A)==pending.column&&(byte(rdram,0x800F9255)&7)==pending.world+1&&(word(rdram,0x800F8E5C)&(1u<<(pending.world*4+pending.column)));
        proof_.consumed=true;if(!accepted)return {};return pending.world;
    }
    // Exact Bowser41A4 defeat path BEFORE80792064, counter1E4==55. Page5 is
    // intentionally ineligible for ordinary world-save or score-table events.
    // Bowser victory has its own native proof, independent from30-fruit
    // collection/ending: its body calls the tally only AFTER this fence.
    void arm_bowser(uint8_t* rdram,const recomp_context* context){
        pending_={};if(!context||!attempt_ready(rdram)||proof_.world!=5||!same_level(rdram))return;
        const auto actor=uint32_t(context->r16),stack=uint32_t(context->r29);
        if(!range(stack,0x48)||!bowser(rdram,actor))return;
        pending_={Kind::Bowser,proof_.attempt,stack,actor,5,proof_.column};
    }
    // BEFORE8079207C: the real8006E0FC(0,0,4C) has returned1; the same actor's
    // defeat counter is still55. The following instruction increments it.
    std::optional<unsigned> finish_bowser(uint8_t* rdram,const recomp_context* context){
        const auto pending=pending_;pending_={};
        if(pending.kind!=Kind::Bowser)return {};
        const bool accepted=context&&attempt_ready(rdram)&&pending.attempt==proof_.attempt&&uint32_t(context->r29)==pending.stack&&uint32_t(context->r16)==pending.actor&&context->r2==1&&same_level(rdram)&&transition(rdram,0x4C)&&bowser(rdram,pending.actor);
        proof_.consumed=true;if(!accepted)return {};return 5;
    }
};
}
