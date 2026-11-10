#include "hybrid_mode.hpp"
#include "hybrid_maskass_data.hpp"
#include "hybrid_rewards.hpp"
#include "workshop_data.hpp"
#include "json/json.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <fstream>
#include <map>
#include <random>
#include <unordered_map>
#include <cstdio>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
namespace workshop::hybrid_mode {
namespace {
std::atomic_bool enabled=false,trial_records_enabled=false,can_read=true,last_saved=true;
std::filesystem::path filename;
using Source=std::array<uint32_t,5>;
std::map<Source,unsigned> colors;
std::map<std::array<uint32_t,4>,unsigned> custom_colors;
std::set<unsigned> custom_rooms;
std::unordered_map<uint32_t,Source> actors;
std::unordered_map<uint32_t,std::array<uint32_t,2>> painted;
std::map<unsigned,unsigned> lucky_levels;
unsigned lucky=0;bool room_ready=false,normal_captured=false;
std::array<uint8_t,1000> normal_records{},hybrid_records{};
bool has_records=false;
bool maskass(unsigned id){return std::binary_search(maskass_ids.begin(),maskass_ids.end(),id);}
unsigned hash(const Source& key){unsigned h=2166136261;for(auto n:key){h=(h^n)*16777619;h^=h>>13;}return h;}
unsigned checksum(const std::array<uint8_t,1000>& bytes){unsigned h=2166136261;for(auto b:bytes)h=(h^b)*16777619;return h;}
bool persist(){
    try{
        auto temp=filename;temp+=".tmp";
        {std::ofstream f(temp,std::ios::binary|std::ios::trunc);f<<nlohmann::json{{"version",1},{"records",hybrid_records},{"checksum",checksum(hybrid_records)}}.dump()<<'\n';f.flush();if(!f)throw std::runtime_error("write");}
        if(std::filesystem::exists(filename)){auto backup=filename;backup+=".bak";std::filesystem::copy_file(filename,backup,std::filesystem::copy_options::overwrite_existing);}
#ifdef _WIN32
        if(!MoveFileExW(temp.c_str(),filename.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("replace");
#else
        std::filesystem::rename(temp,filename);
#endif
        return true;
    }catch(const std::exception& e){fprintf(stderr,"[HybridMode] Records save failed: %s\n",e.what());return false;}
}
}
void initialize(const std::filesystem::path& path){
    filename=path;enabled=false;trial_records_enabled=false;can_read=last_saved=true;normal_captured=has_records=false;normal_records.fill(0);hybrid_records.fill(0);
    colors.clear();actors.clear();lucky_levels.clear();room_ready=false;custom_colors.clear();custom_rooms.clear();
    lucky=std::uniform_int_distribution<unsigned>(0,5)(*[](){static std::mt19937 random(std::random_device{}());return &random;}());
    try{
        if(!std::filesystem::exists(filename))return;
        if(std::filesystem::file_size(filename)>32768)throw std::runtime_error("size");
        nlohmann::json j;std::ifstream(filename)>>j;
        if(j.at("version")!=1 || !j.at("records").is_array() || j["records"].size()!=1000)throw std::runtime_error("format");
        for(unsigned i=0;i<1000;i++){if(!j["records"][i].is_number_integer())throw std::runtime_error("byte");auto n=j["records"][i].get<int>();if(n<0 || n>255)throw std::runtime_error("byte");hybrid_records[i]=uint8_t(n);}
        if(j.at("checksum").get<unsigned>()!=checksum(hybrid_records))throw std::runtime_error("checksum");has_records=true;
    }catch(...){can_read=last_saved=false;fprintf(stderr,"[HybridMode] Unreadable records preserved; mode unavailable.\n");}
}
bool readable(){return can_read;}
bool saved(){return last_saved;}
void activate(bool value,bool use_hybrid_trial_records){
    trial_records_enabled=false;normal_captured=false;
    enabled=value;actors.clear();colors.clear();lucky_levels.clear();room_ready=false;
    trial_records_enabled=value && use_hybrid_trial_records;
    rewards::custom_enemy_variant=(value || !custom_colors.empty())?enemy_variant:nullptr;
    rewards::lucky_fruit_type=value?lucky_type:nullptr;
}
void custom_placements(std::span<const uint8_t> payload){
    custom_colors.clear();custom_rooms.clear();
    if(!payload.empty())for(const auto& room:workshop::payload_rooms(payload)){
        custom_rooms.insert(room.source);
        if(room.stride==16)for(unsigned i=0;i<room.count;i++){size_t p=room.offset+size_t(i)*room.stride;if(payload[p+12]<6){
            std::array<uint32_t,4> key{room.source,workshop::u32(payload,p),workshop::u32(payload,p+4),workshop::u32(payload,p+8)};
            auto [it,added]=custom_colors.emplace(key,payload[p+12]);workshop::require(added || it->second==payload[p+12],"Overlapping Maskass cannot have different colors");
        }}
    }
    rewards::custom_enemy_variant=(active() || !custom_colors.empty())?enemy_variant:nullptr;
}
bool active(){return enabled;}
void initialize_ram(uint8_t* rdram){
    if(!active() && custom_colors.empty())return;
    for(unsigned c=0;c<6;c++)for(unsigned i=0;i<256;i++)MEM_H(int(c*512+i*2),int32_t(palette_base))=maskass_palettes[c][i];
    if(active())MEM_W(0,int32_t(0x807FC140))=6+lucky;
}
void enter(uint8_t* rdram,uint32_t metadata,uint32_t room){
    if(!fruit_mix::playable(rdram,metadata) && !custom_rooms.contains(room&255))return;
    actors.clear();painted.clear();room_ready=true;
    if(!active())return;
    auto level=unsigned(MEM_BU(0,int32_t(0x800F8DFC)))*4+unsigned(MEM_BU(0,int32_t(0x800F8DFD)));
    auto [it,added]=lucky_levels.try_emplace(level,lucky_levels.empty()?lucky:(hash({room,level,lucky,0,0})%6));lucky=it->second;
    MEM_W(0,int32_t(0x807FC140))=6+lucky;
    fprintf(stderr,"[HybridMode] level=%u-%u lucky=%u fruitChance=75\n",level/4+1,level%4+1,lucky_type());
}
void created(uint8_t* rdram,recomp_context* ctx,uint32_t room){
    const auto actor=uint32_t(ctx->r4);actors.erase(actor);painted.erase(actor);
    if((!active() && custom_colors.empty()) || !room_ready || !maskass(uint32_t(ctx->r5)>>16) || !fruit_mix::range(uint32_t(ctx->r29),0x14))return;
    Source key{room,uint32_t(ctx->r5),uint32_t(ctx->r6),uint32_t(ctx->r7),uint32_t(MEM_W(0x10,ctx->r29))};
    auto chosen=custom_colors.find({room&255,uint32_t(ctx->r5),uint32_t(ctx->r6),uint32_t(ctx->r7)});
    if(!active() && (!custom_rooms.contains(room&255) || chosen==custom_colors.end()))return;
    colors.try_emplace(key,chosen!=custom_colors.end() && custom_rooms.contains(room&255)?chosen->second:hash(key)%6);actors.emplace(actor,key);
    fprintf(stderr,"[HybridMode] Maskass actor=%08X id=%04X color=%u room=%u\n",actor,uint32_t(ctx->r5)>>16,colors.at(key),room&255);
}
std::optional<unsigned> enemy_variant(uint32_t actor){
    if(!active() && custom_colors.empty())return {};auto it=actors.find(actor);if(it==actors.end())return {};return colors.at(it->second);
}
unsigned lucky_type(){return 6+lucky;}
uint16_t lucky_object(){return uint16_t(0x4F00+lucky);}
void paint(uint8_t* rdram,uint32_t actor){
    if((!active() && custom_colors.empty()) || !fruit_mix::range(actor,0x174))return;const auto a=gpr(int32_t(actor));
    if(active() && MEM_HU(12,a)==0x402C && (MEM_HU(14,a)&0x3FF)==0x200){MEM_H(0x144,a)=9+lucky;return;}
    auto color=enemy_variant(actor);if(!color || !maskass(MEM_HU(12,a)))return;
    const auto drawing=uint32_t(MEM_W(0x16C,a));if(!fruit_mix::range(drawing,12))return;
    const uint32_t palette=palette_base+*color*512;MEM_W(8,int32_t(drawing))=palette;
    auto [it,added]=painted.try_emplace(actor,std::array<uint32_t,2>{drawing,palette});
    if(added || it->second!=std::array<uint32_t,2>{drawing,palette}){it->second={drawing,palette};fprintf(stderr,"[HybridMode] painted actor=%08X color=%u room=%u palette=%08X\n",actor,*color,actors.at(actor)[0]&255,palette);}
}
void recolored(uint8_t* rdram,uint32_t actor){
    auto it=actors.find(actor);if((!active() && custom_colors.empty()) || it==actors.end())return;
    auto& color=colors.at(it->second);color=(color+1)%6;paint(rdram,actor);
    fprintf(stderr,"[HybridMode] recolor actor=%08X color=%u\n",actor,color);
}
void records_loaded(uint8_t* rdram,uint32_t payload){
    if(!trial_records_enabled || !fruit_mix::range(payload,1018))return;
    for(unsigned i=0;i<1000;i++)normal_records[i]=MEM_BU(int(i+4),int32_t(payload));
    normal_captured=true;
    // The original codec's forward glyph table protects the same native
    // Name Entry paths when a sidecar supplies its separate high scores.
    for(unsigned row=0;row<125;row++)for(unsigned n=0;n<6;n++)if(MEM_BU(hybrid_records[row*8+2+n],int32_t(0x807F2100))>=228){can_read=last_saved=false;fprintf(stderr,"[HybridMode] Invalid record glyph preserved.\n");return;}
    for(unsigned i=0;i<1000;i++)MEM_B(int(i+4),int32_t(payload))=hybrid_records[i];
    fprintf(stderr,"[HybridMode] Separate records loaded; ordinary scores preserved.\n");
}
bool records_saving(uint8_t* rdram,uint32_t payload,uint32_t buffer){
    if(!trial_records_enabled)return true;
    if(!can_read || !normal_captured || !fruit_mix::range(payload,1018) || !fruit_mix::range(buffer,1018))return false;
    for(unsigned i=0;i<1000;i++)hybrid_records[i]=MEM_BU(int(i+4),int32_t(payload));
    last_saved=persist();if(!last_saved)return false;has_records=true;
    for(unsigned i=0;i<1000;i++)MEM_B(int(i+4),int32_t(buffer))=normal_records[i];
    fprintf(stderr,"[HybridMode] Hybrid records saved; ordinary EEPROM score table restored before encoding.\n");return true;
}
}
