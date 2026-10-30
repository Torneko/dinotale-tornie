#include "collections.hpp"
#include "fruit_mix.hpp"
#include "story_progress.hpp"
#include "json/json.hpp"
#include <fstream>
#include <mutex>
#include <unordered_map>
#include <cstdio>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
namespace workshop::collections {
bool State::valid()const{
    if(purchases>15 || legacyEggs>3)return false;
    for(auto& l:levels){
        if(l.places.size()>20 || (l.claimed>>l.places.size()))return false;
        for(unsigned i=0;i<l.places.size();i++){
            const auto& p=l.places[i];if(p.room>=177 || p.index>=2000)return false;
            for(unsigned j=0;j<i;j++)if(l.places[j].room==p.room && l.places[j].index==p.index)return false;
        }
    }
    return spent()<=earned() && (!owns(Item::Editor) || ((purchases&7)==7 && earned()==480))
        && (!remainder || (earned()==480 && (purchases&7)==7));
}
bool State::claim(unsigned level,unsigned slot){
    if(level>=24 || slot>=levels[level].places.size() || (levels[level].claimed&(1u<<slot)))return false;
    levels[level].claimed|=1u<<slot;
    remainder|=earned()==480 && (purchases&7)==7;return true;
}
bool State::buy(Item item,bool hybridReady){
    const auto i=unsigned(item);if(i>=4 || owns(item) || balance()<prices[i])return false;
    if(item==Item::Hybrid && !hybridReady)return false;
    if(item==Item::Editor && (!hybridReady || (purchases&7)!=7 || earned()!=480))return false;
    purchases|=1u<<i;remainder|=earned()==480 && (purchases&7)==7;return true;
}
namespace {
std::mutex mutex;View ledger;std::filesystem::path filename;bool initialized=false,session=false,room_active=false;
unsigned current_level=24,current_room=177;
struct Actor {unsigned level,slot;bool scored=false;};
std::unordered_map<uint32_t,Actor> actors;
constexpr auto root=gpr(int32_t(0x800F7EA0));
constexpr uint32_t palette=0x807FC200;
// The six Hybrid-mode palettes occupy C400..CFFF.
constexpr uint32_t white_palette=0x807FD000;
bool persist(){
    try{
        nlohmann::json levels=nlohmann::json::object();
        for(unsigned i=0;i<24;i++){
            auto& l=ledger.state.levels[i];if(l.places.empty())continue;
            auto places=nlohmann::json::array();
            for(auto& p:l.places)places.push_back({p.room,p.index,p.x,p.y});
            levels[std::to_string(i/4+1)+"-"+std::to_string(i%4+1)]={{"places",places},{"claimedMask",l.claimed}};
        }
        auto temp=filename;temp+=".tmp";
        {std::ofstream f(temp,std::ios::binary|std::ios::trunc);
            f<<nlohmann::json{{"version",1},{"levels",levels},{"purchases",ledger.state.purchases},
                {"legacyEggs",ledger.state.legacyEggs},{"remainderAchievement",ledger.state.remainder}}.dump(2)<<'\n';
            f.flush();if(!f)throw std::runtime_error("write");}
        if(std::filesystem::exists(filename)){auto backup=filename;backup+=".bak";std::filesystem::copy_file(filename,backup,std::filesystem::copy_options::overwrite_existing);}
#ifdef _WIN32
        if(!MoveFileExW(temp.c_str(),filename.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("replace");
#else
        std::filesystem::rename(temp,filename);
#endif
        return true;
    }catch(const std::exception& e){fprintf(stderr,"[Collection] Save failed: %s\n",e.what());return false;}
}
void save(){ledger.saved=persist();}
bool ready(){return initialized && ledger.readable;}
}
void initialize(const std::filesystem::path& path){
    std::lock_guard lock(mutex);filename=path;ledger={};actors.clear();initialized=true;session=room_active=false;
    try{
        if(!std::filesystem::exists(path))return;
        if(std::filesystem::file_size(path)>262144)throw std::runtime_error("size");
        nlohmann::json j;std::ifstream(path)>>j;if(j.at("version")!=1)throw std::runtime_error("version");
        ledger.state.purchases=j.at("purchases").get<unsigned>();ledger.state.legacyEggs=j.value("legacyEggs",0u);
        ledger.state.remainder=j.at("remainderAchievement").get<bool>();
        for(auto& [id,data]:j.at("levels").items()){
            if(id.size()!=3 || id[0]<'1' || id[0]>'6' || id[1]!='-' || id[2]<'1' || id[2]>'4')throw std::runtime_error("level");
            auto& l=ledger.state.levels[(id[0]-'1')*4+id[2]-'1'];
            for(auto& p:data.at("places")){if(!p.is_array() || p.size()!=4)throw std::runtime_error("place");l.places.push_back({p[0].get<unsigned>(),p[1].get<unsigned>(),p[2].get<unsigned>(),p[3].get<unsigned>()});}
            l.claimed=data.at("claimedMask").get<unsigned>();
        }
        if(!ledger.state.valid())throw std::runtime_error("state");
    }catch(...){ledger={};ledger.readable=ledger.saved=false;fprintf(stderr,"[Collection] Invalid ledger preserved; currency and purchases disabled.\n");}
}
void start_session(unsigned mode){std::lock_guard lock(mutex);session=mode<2;room_active=false;actors.clear();}
View view(){std::lock_guard lock(mutex);return ledger;}
bool purchase(Item item){
    std::lock_guard lock(mutex);if(!ready())return false;auto before=ledger.state;
    if(!ledger.state.buy(item,true))return false;
    if(!persist()){ledger.state=before;ledger.saved=false;return false;}
    ledger.saved=true;return true;
}
bool editor_unlocked(){auto v=view();return v.readable && v.state.owns(Item::Editor);}
bool egg_allowed(uint8_t* rdram,unsigned color){
    std::lock_guard lock(mutex);if(!ready() || color<6 || color>7)return false;
    auto p=progress::read(rdram,gpr(int32_t(0x807F2000)));
    const auto story=uint32_t(MEM_W(0,gpr(int32_t(0x800F8E5C))));
    if(!progress::valid(p,story) || p.trial!=0xFFFFFF)return false;
    // Existing earned eggs retain their availability after migrating saves.
    if(p.eggs&(1u<<(color-6)))return true;
    return ledger.state.owns(color==6?Item::White:Item::Black);
}
std::optional<bool> egg_bubble_hidden(uint8_t* rdram,uint32_t actor){
    if(!fruit_mix::range(actor,0x10))return {};
    const auto id=MEM_HU(12,int32_t(actor));
    if(id!=0x4692 && id!=0x4693)return {};
    const unsigned color=id==0x4692?6:7,mask=color==6?0x1440:0x2880;
    // The original predicate unconditionally hides these two sources in
    // Trial. Keep its already-carried/already-earned flags, with the PC gate.
    return !egg_allowed(rdram,color) || (uint32_t(MEM_W(0xF54,root))&mask)!=0;
}
void enter(uint8_t* rdram,unsigned room,uint32_t metadata,bool allowed){
    std::lock_guard lock(mutex);
    // This loader also preloads non-level resources after gameplay actors
    // have been born. Those calls must not erase the current room bindings.
    if(!fruit_mix::playable(rdram,metadata))return;
    actors.clear();room_active=false;
    if(!ready() || !session || !allowed || room>=177 || MEM_W(0xCE8,root)!=1)return;
    const unsigned page=MEM_BU(0xF5C,root),column=MEM_BU(0xF5D,root);
    if(page>=6 || column>=4)return;current_level=page*4+column;current_room=room;room_active=true;
    auto& places=ledger.state.levels[current_level].places;bool changed=false;
    const auto meta=gpr(int32_t(metadata)),objects=gpr(int32_t(MEM_W(4,meta)));
    for(unsigned i=0;i<MEM_HU(0,meta) && places.size()<20;i++){
        if(MEM_HU(i*16,objects)!=0x406F)continue;
        bool exists=false;for(auto& p:places)exists|=p.room==room && p.index==i;
        if(!exists){places.push_back({room,i,unsigned(MEM_W(i*16+4,objects)),unsigned(MEM_W(i*16+8,objects))});changed=true;}
    }
    if(changed)save();
    fprintf(stderr,"[Collection] level=%u-%u room=%u placements=%zu claimed=%u balance=%u\n",page+1,column+1,room,places.size(),std::popcount(ledger.state.levels[current_level].claimed),ledger.state.balance());
}
void created(uint8_t*,recomp_context* ctx){
    std::lock_guard lock(mutex);const auto actor=uint32_t(ctx->r4);actors.erase(actor);
    if(!room_active || uint32_t(ctx->r5)>>16!=0x406F)return;
    auto& places=ledger.state.levels[current_level].places;
    for(unsigned i=0;i<places.size();i++)if(places[i].room==current_room && places[i].x==uint32_t(ctx->r6) && places[i].y==uint32_t(ctx->r7)){
        actors[actor]={current_level,i};break;
    }
}
void draw(uint8_t* rdram,uint32_t actor){
    std::lock_guard lock(mutex);
    if(!fruit_mix::range(actor,0x174))return;const auto a=gpr(int32_t(actor));
    if(MEM_HU(12,a)!=0x406F)return;
    const uint32_t resource=MEM_W(0x168,a),drawing=MEM_W(0x16C,a);
    if(!fruit_mix::range(resource,0x8C) || !fruit_mix::range(drawing,12))return;
    const auto resource_at=static_cast<gpr>(static_cast<int32_t>(resource));
    const auto drawing_at=static_cast<gpr>(static_cast<int32_t>(drawing));
    const uint32_t original=MEM_W(0x88,resource_at);
    if(!fruit_mix::range(original,512))return;
    // Transform only the borrowed palette. Native frames, geometry and sound
    // stay untouched, and a recycled ordinary coin gets its original palette.
    const bool selected=room_active && actors.contains(actor);
    const bool claimed=selected && (ledger.state.levels[actors.at(actor).level].claimed&(1u<<actors.at(actor).slot));
    const uint32_t target=claimed?white_palette:palette;
    const auto palette_at=static_cast<gpr>(static_cast<int32_t>(target));
    const auto original_at=static_cast<gpr>(static_cast<int32_t>(original));
    if(selected)for(unsigned i=0;i<256;i++){
        const auto c=MEM_HU(i*2,original_at);MEM_H(i*2,palette_at)=claimed?collected_white(c):turquoise(c);
    }
    MEM_W(8,drawing_at)=selected?target:original;
}
void collect(uint8_t* rdram,uint32_t actor){
    std::lock_guard lock(mutex);auto found=actors.find(actor);
    if(!room_active || !ready() || found==actors.end() || found->second.scored)return;
    auto& a=found->second;a.scored=true;
    const bool fresh=ledger.state.claim(a.level,a.slot);if(fresh)save();
    // The native coin counter keeps its original one-point tally. Add the
    // remaining two points once, without replacing its pickup animation/sound.
    auto score=uint32_t(MEM_W(0xAB4,root));MEM_W(0xAB4,root)=score>99997?99999:score+2;
    fprintf(stderr,"[Collection] pickup level=%u-%u slot=%u currency=%u balance=%u extra-score=2\n",a.level/4+1,a.level%4+1,a.slot,unsigned(fresh),ledger.state.balance());
}
}
