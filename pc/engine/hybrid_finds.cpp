#include "hybrid_finds.hpp"
#include "metal_yoshis0300.hpp"
#include "json/json.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <unordered_map>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace workshop::finds {
namespace {
std::mutex mutex;View ledger;std::filesystem::path filename;
unsigned level=24,room=177,selected=8,carried=0;uint32_t carried_actor=0;bool enabled=false,taken=false;
unsigned pickup_sounded=0,recoverable_metals=0,carried_recovery=0;
std::optional<std::array<unsigned,4>> place;
struct Egg {unsigned color,room;bool scene,recovery;};
std::unordered_map<uint32_t,Egg> actors;
bool persist(){
    try{
        auto temp=filename;temp+=".tmp";
        {std::ofstream f(temp,std::ios::binary|std::ios::trunc);
            f<<nlohmann::json{{"version",1},{"foundMask",ledger.found},{"legacyMigrated",true}}.dump(2)<<'\n';
            f.flush();if(!f)throw std::runtime_error("write");}
        if(std::filesystem::exists(filename)){auto backup=filename;backup+=".bak";std::filesystem::copy_file(filename,backup,std::filesystem::copy_options::overwrite_existing);}
#ifdef _WIN32
        if(!MoveFileExW(temp.c_str(),filename.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("replace");
#else
        std::filesystem::rename(temp,filename);
#endif
        return true;
    }catch(const std::exception& e){fprintf(stderr,"[HybridFind] Save failed: %s\n",e.what());return false;}
}
void reset(){level=24;room=177;selected=8;carried=carried_actor=pickup_sounded=recoverable_metals=carried_recovery=0;enabled=taken=false;place.reset();actors.clear();}
// Called only with mutex held. Already migrated ownership is authoritative;
// absent ownership may be seeded only by an explicitly validated source.
void migrate_locked(uint8_t* rdram,const progress::State& authoritative,unsigned story,bool sourceValid){
    if(!ledger.initialized || !ledger.readable)return;
    if(ledger.migrated){MEM_W(0,int32_t(0x807FC144))=0xF17D0000|ledger.found;return;}
    if(!sourceValid || !progress::valid(authoritative,story))return;
    ledger.found=progress::unlocked(authoritative,story);ledger.saved=persist();
    if(ledger.saved){ledger.migrated=true;MEM_W(0,int32_t(0x807FC144))=0xF17D0000|ledger.found;fprintf(stderr,"[HybridFind] Existing Yoshis retained: %02X.\n",ledger.found);}
}
}
void initialize(const std::filesystem::path& path){
    std::lock_guard lock(mutex);ledger={};ledger.initialized=true;filename=path;reset();
    try{
        if(!std::filesystem::exists(path))return;
        if(std::filesystem::file_size(path)>4096)throw std::runtime_error("size");
        nlohmann::json j;std::ifstream(path)>>j;
        if(j.at("version")!=1 || !j.at("legacyMigrated").get<bool>())throw std::runtime_error("version");
        ledger.found=j.at("foundMask").get<unsigned>();if(ledger.found>63)throw std::runtime_error("mask");
        ledger.migrated=true;
    }catch(...){ledger.readable=ledger.saved=false;fprintf(stderr,"[HybridFind] Invalid find ledger preserved; new finds disabled.\n");}
}
void migrate(uint8_t* rdram,unsigned story){
    std::lock_guard lock(mutex);
    // Preserve the original API, including no payload read before its guards.
    if(!ledger.initialized || !ledger.readable)return;
    if(ledger.migrated){MEM_W(0,int32_t(0x807FC144))=0xF17D0000|ledger.found;return;}
    migrate_locked(rdram,progress::read(rdram,gpr(int32_t(0x807F2000))),story,true);
}
void migrate(uint8_t* rdram,const progress::State& authoritative,unsigned story,bool sourceValid){
    std::lock_guard lock(mutex);migrate_locked(rdram,authoritative,story,sourceValid);
}
View view(){std::lock_guard lock(mutex);return ledger;}
unsigned available(unsigned eligible){
    std::lock_guard lock(mutex);
    // Uninitialized standalone native checks retain their legacy behavior.
    if(!ledger.initialized || !ledger.readable)return eligible&63;
    return eligible&(ledger.migrated?ledger.found:0)&63;
}
void reset_attempt(){std::lock_guard lock(mutex);reset();}
void enter(unsigned next,unsigned destination,bool allowed,unsigned recoverableMetals){
    std::lock_guard lock(mutex);if(level!=next)reset();level=next;room=destination;
    enabled=allowed && next<24 && destination<177 && ledger.initialized && ledger.migrated && ledger.readable && ledger.saved;
    const auto metals=metal_yoshis0300::view();
    recoverable_metals=enabled && !(recoverableMetals&~0xC0u) && metals.initialized && metals.readable && metals.saved
        ? recoverableMetals&(metals.found<<6):0;
    fprintf(stderr,"[HybridFind] Enter level=%u room=%u enabled=%u found=%02X.\n",next,destination,unsigned(enabled),ledger.found);
}
std::optional<unsigned> reserve(unsigned eligible,unsigned source,unsigned x,unsigned y,bool whitePresent){
    std::lock_guard lock(mutex);
    if(!enabled || taken || whitePresent)return {};
    const std::array<unsigned,4> key{room,source,x,y};
    if(place && *place!=key)return {};
    // Preserve the established six-color queue, then append eligible metals.
    const unsigned waiting=(eligible&~ledger.found&63)|(metal_yoshis0300::search_mask()<<6)|recoverable_metals;
    if(!waiting)return {};
    if(!place){place=key;selected=std::countr_zero(waiting);
        fprintf(stderr,"[HybridFind] Reserved color=%u room=%u source=%04X xy=%08X,%08X.\n",selected+8,room,source,x,y);}
    return selected;
}
void created(uint32_t actor){std::lock_guard lock(mutex);actors.erase(actor);}
void spawned(uint32_t actor,unsigned color,bool scene){std::lock_guard lock(mutex);if(actor && color<8)actors[actor]={color,room,scene,((recoverable_metals|carried_recovery)&(1u<<color))!=0};}
bool scene_actor(uint32_t actor){std::lock_guard lock(mutex);auto it=actors.find(actor);return it!=actors.end() && it->second.scene;}
bool egg_state(uint32_t actor,unsigned state){
    std::lock_guard lock(mutex);auto it=actors.find(actor);if(it==actors.end())return false;
    if(state==4 || state==5){carried=1u<<it->second.color;carried_recovery=it->second.recovery?carried:0;carried_actor=actor;taken=true;fprintf(stderr,"[HybridFind] Carried color=%u, finish required.\n",it->second.color+8);}
    else if(actor==carried_actor)carried=carried_actor=carried_recovery=0;
    return true;
}
bool take_pickup_sound(uint32_t actor,unsigned state){
    std::lock_guard lock(mutex);
    auto it=actors.find(actor);
    if(!enabled || it==actors.end() || (state!=4 && state!=5) || actor!=carried_actor)return false;
    const auto bit=1u<<it->second.color;
    const auto known=ledger.found|(metal_yoshis0300::view().found<<6);
    if(((known&~(recoverable_metals|carried_recovery))|pickup_sounded)&bit)return false;
    pickup_sounded|=bit;return true;
}
uint32_t touching(uint8_t* rdram,uint32_t player){
    std::lock_guard lock(mutex);
    if(!enabled || carried || player<0x80000400 || player>0x807FFC00)return 0;
    const auto p=gpr(int32_t(player));
    if(MEM_W(0,p)!=0x19971110 || MEM_HU(12,p)!=0x4001 || MEM_HU(0x184,p)>=20)return 0;
    const auto value=[&](gpr a,int offset){return std::bit_cast<float>(uint32_t(MEM_W(offset,a)));};
    const float scale=value(p,0xB4);
    if(!std::isfinite(scale) || scale<=0 || scale>8)return 0;
    for(auto [address,entry]:actors){
        if(address<0x80000400 || address>0x807FFC00)continue;
        const auto a=gpr(int32_t(address));
        if(!entry.scene || entry.room!=room || (((ledger.found|(metal_yoshis0300::view().found<<6))&~recoverable_metals)&(1u<<entry.color)) || MEM_W(0,a)!=0x19971110 || MEM_HU(12,a)!=0x401D
            || MEM_BU(0x10,a)!=1 || MEM_BU(0x30,a)>=4 || MEM_W(0x180,a))continue;
        // These scene eggs are outside the native carried-egg pool. Claim on
        // body contact, then let the native parent/state machinery carry them.
        if(std::abs(value(a,0x3C)-value(p,0x3C))<=18*scale
            && std::abs(value(a,0x40)-(value(p,0x40)+14*scale))<=20*scale
            && std::abs(value(a,0x44)-value(p,0x44))<=scale)return address;
    }
    return 0;
}
unsigned pending(){std::lock_guard lock(mutex);return carried;}
unsigned pending_hybrids(){std::lock_guard lock(mutex);return carried&63;}
unsigned pending_metals(){std::lock_guard lock(mutex);return (carried>>6)&3;}
bool finish(uint8_t* rdram){
    std::lock_guard lock(mutex);if(!enabled || !carried || !ledger.readable || !ledger.migrated)return false;
    if(carried&0xC0){
        if((carried&~0xC0u)||!std::has_single_bit(carried))return false;
        const unsigned color=8+unsigned(std::countr_zero(carried));
        // A recovered owned egg must only confirm ownership, never acquire it
        // again after another writer replaced or cleared the metal journal.
        const bool recovery=(carried&carried_recovery)!=0;
        if(!(recovery?metal_yoshis0300::confirm_owned(color):metal_yoshis0300::acquire(color)))return false;
        // Only the transient extra-life projection grows to eight bits. Never
        // put Gold/Silver in the legacy six-life payload or native egg flags.
        MEM_W(0,int32_t(0x807F1000))|=carried;
        recoverable_metals&=~carried;
        carried=carried_actor=carried_recovery=0;
        fprintf(stderr,"[MetalYoshi] carried-native-egg-finished color=%u found=%u legacySixBitLedgerUnchanged=1\n",color,metal_yoshis0300::view().found);
        return true;
    }
    const unsigned added=carried&~ledger.found;if(!added)return false;
    const unsigned before=ledger.found;ledger.found|=added;ledger.saved=persist();
    if(!ledger.saved){ledger.found=before;return false;}
    MEM_W(0,int32_t(0x807FC144))=0xF17D0000|ledger.found;
    MEM_W(0,int32_t(0x807F1000))|=added;
    auto p=progress::read(rdram,gpr(int32_t(0x807F2000)));p.lives|=added;progress::write(rdram,gpr(int32_t(0x807F2000)),p);
    MEM_W(0,int32_t(0x807F2024))=1;carried=carried_actor=0;
    fprintf(stderr,"[HybridFind] Found and finished: added=%02X total=%02X.\n",added,ledger.found);return true;
}
void clear_save(){std::lock_guard lock(mutex);if(!ledger.initialized || !ledger.readable)return;ledger.found=0;ledger.migrated=true;ledger.saved=persist();reset();}
}
