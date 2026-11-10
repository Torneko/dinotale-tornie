#pragma once
#include "recomp.h"
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>

extern "C" void func_80022E0C(uint8_t*,recomp_context*);
namespace workshop::coop_native_input {
// Backend axes are normalized [-1,1]. Native pad bytes are signed [-80,80].
// Health is NOT stored here:897E is the processed horizontal stick halfword.
struct Pad{uint16_t buttons=0;float x=0,y=0;};
inline constexpr uint32_t pad_pointer=0x800F7EA8,current=0x800F897C,previous=0x800F899C;
inline Pad latest{};
inline uint16_t pressed=0;
inline uint64_t generation=0,processed_generation=0;
inline unsigned depth=0;
inline int active_owner=-1;
inline bool secondary_initialized=false;
inline gpr b(uint32_t address){return gpr(int32_t(address));}
inline uint32_t word(uint8_t* rdram,uint32_t address){const gpr p=b(address);return uint32_t(MEM_W(0,p));}
inline bool range(uint32_t address,size_t bytes){return address>=0x80000400u && address<0x80800000u && bytes<=0x80800000u-address;}
inline bool update(Pad pad){
    if(!std::isfinite(pad.x) || !std::isfinite(pad.y))return false;
    pad.x=std::clamp(pad.x,-1.0f,1.0f);pad.y=std::clamp(pad.y,-1.0f,1.0f);
    pressed=uint16_t(pad.buttons & ~latest.buttons);latest=pad;
    if(++generation==0){generation=1;processed_generation=0;}
    return true;
}
struct State{
    //22E0C explicitly copies the24-byte current structure to previous.
    std::array<uint8_t,24> now{},old{};
    //22D6C: A held, A pressed, B held, B pressed.
    std::array<uint8_t,4> buttons{};
    static State capture(uint8_t* rdram){
        State s;const gpr cur=b(current),prev=b(previous),flags=b(0x800F8995);
        for(int i=0;i<24;i++){s.now[i]=MEM_BU(i,cur);s.old[i]=MEM_BU(i,prev);}
        for(int i=0;i<4;i++)s.buttons[i]=MEM_BU(i,flags);
        return s;
    }
    void install(uint8_t* rdram)const{
        const gpr cur=b(current),prev=b(previous),flags=b(0x800F8995);
        for(int i=0;i<24;i++){MEM_B(i,cur)=now[i];MEM_B(i,prev)=old[i];}
        for(int i=0;i<4;i++)MEM_B(i,flags)=buttons[i];
    }
};
struct Raw{
    uint16_t buttons=0,pressed=0;
    int8_t x=0,y=0;
    static Raw capture(uint8_t* rdram,uint32_t pointer){
        const gpr p=b(pointer);return {uint16_t(MEM_HU(0,p)),uint16_t(MEM_HU(12,p)),int8_t(MEM_B(2,p)),int8_t(MEM_B(3,p))};
    }
    void install(uint8_t* rdram,uint32_t pointer)const{
        const gpr p=b(pointer);MEM_H(0,p)=buttons;MEM_H(12,p)=pressed;MEM_B(2,p)=x;MEM_B(3,p)=y;
    }
};
inline std::array<State,2> states{};
inline Raw primary_raw{};
inline Raw secondary_raw(){return {latest.buttons,pressed,int8_t(std::lround(latest.x*80.0f)),int8_t(std::lround(latest.y*80.0f))};}
inline void reset(){
    if(depth)throw std::logic_error("Cannot reset an active native pad scope");
    latest={};pressed=0;generation=processed_generation=0;active_owner=-1;secondary_initialized=false;states={};primary_raw={};
}
inline uint8_t owner_facing(uint8_t* rdram,uint8_t fallback){
    const uint32_t actor=word(rdram,0x800F8828);
    if(!range(actor,0x1FC) || word(rdram,actor)!=0x19971110)return fallback;
    //22AC0's zero-axis branch reads actor138 and stores the low byte at897C.
    return uint8_t(word(rdram,actor+0x138));
}
inline void install_owner(uint8_t* rdram,uint32_t pointer,int owner){
    states[owner].install(rdram);
    (owner?secondary_raw():primary_raw).install(rdram,pointer);
}
class Scope {
    uint8_t* rdram_=nullptr;
    uint32_t pointer_=0;
    int owner_=0,prior_=-1;
    bool installed_=false,same_=false;
public:
    // Enter the owner/colour scope FIRST, so zero-axis facing uses that actor.
    // Pass secondary=false when a P1 callback nests within a P2 callback.
    Scope(uint8_t* rdram,recomp_context* context,bool secondary=true,bool neutral=false):rdram_(rdram),owner_(secondary?1:0),prior_(active_owner){
        pointer_=word(rdram,pad_pointer);
        if(!range(pointer_,16) || !context || depth>=32)throw std::invalid_argument("Invalid native controller context");
        if(!depth){states[0]=State::capture(rdram);primary_raw=Raw::capture(rdram,pointer_);}
        else{
            states[active_owner]=State::capture(rdram);
            if(active_owner==0)primary_raw=Raw::capture(rdram,pointer_);
        }
        if(!secondary_initialized && owner_==1){states[1]={};states[1].now[0]=owner_facing(rdram,1);secondary_initialized=true;}
        same_=prior_==owner_;active_owner=owner_;depth++;installed_=true;
        if(!same_)install_owner(rdram,pointer_,owner_);
        if(neutral || (owner_==1 && processed_generation!=generation)){
            const auto saved=*context;
            try{
                (neutral?Raw{}:secondary_raw()).install(rdram,pointer_);
                // Reuse the native deadzone, two-stage smoothing, clamp,
                // magnitude/angle conversion, facing and A/B edge processing.
                context->r29=ADD32(saved.r29,-0x40);func_80022E0C(rdram,context);*context=saved;
                //4700C is a singleton native lookup. Correct only its zero-X
                // facing fallback to the explicitly scoped owner actor.
                const gpr cur=b(current);
                if(MEM_H(2,cur)==0)MEM_B(0,cur)=owner_facing(rdram,MEM_BU(0,cur));
                states[owner_]=State::capture(rdram);if(owner_==1)processed_generation=generation;
            }catch(...){
                *context=saved;states[owner_]=State::capture(rdram);depth--;active_owner=prior_;installed_=false;
                install_owner(rdram,pointer_,prior_<0?0:prior_);throw;
            }
        }
    }
    ~Scope(){
        if(!installed_)return;
        states[owner_]=State::capture(rdram_);
        if(owner_==0)primary_raw=Raw::capture(rdram_,pointer_);
        depth--;active_owner=prior_;
        if(!same_)install_owner(rdram_,pointer_,prior_<0?0:prior_);
    }
    Scope(const Scope&)=delete;Scope& operator=(const Scope&)=delete;
};
}
