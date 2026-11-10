#include "achievements.hpp"
#include "json/json.hpp"
#include <fstream>
#include <mutex>
#include <cstdio>
#include <algorithm>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace workshop::achievements {
namespace {
std::mutex mutex;
View ledger;Tracker tracker;std::filesystem::path filename;
bool initialized=false,session_allowed=false;
bool persist(){
    try{
        nlohmann::json levels=nlohmann::json::object();
        for(unsigned i=0;i<24;i++){
            const auto& e=ledger.levels[i];if(!e.completions)continue;
            levels[std::to_string(i/4+1)+"-"+std::to_string(i%4+1)]={
                {"completions",e.completions},{"modes",e.modes},{"bestHealth",e.bestHealth},
                {"fullHealth",e.fullHealth},{"sameFruitMask",e.sameFruit},{"bestHearts",e.bestHearts},{"allHearts",e.allHearts}};
        }
        auto temp=filename;temp+=".tmp";
        {std::ofstream file(temp,std::ios::binary|std::ios::trunc);
            file<<nlohmann::json{{"version",2},{"levels",levels}}.dump(2)<<'\n';
            file.flush();if(!file)throw std::runtime_error("write");}
        if(std::filesystem::exists(filename)){
            auto backup=filename;backup+=".bak";
            std::filesystem::copy_file(filename,backup,std::filesystem::copy_options::overwrite_existing);
        }
#ifdef _WIN32
        if(!MoveFileExW(temp.c_str(),filename.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("replace");
#else
        std::filesystem::rename(temp,filename);
#endif
        return true;
    }catch(const std::exception& e){fprintf(stderr,"[Achievements] Save failed: %s\n",e.what());return false;}
}
}
void initialize(const std::filesystem::path& path){
    std::lock_guard lock(mutex);filename=path;ledger={};tracker.reset();initialized=true;session_allowed=false;
    try{
        if(!std::filesystem::exists(filename))return;
        if(std::filesystem::file_size(filename)>262144)throw std::runtime_error("size");
        std::ifstream file(filename);nlohmann::json data;file>>data;
        if((data.at("version")!=1 && data.at("version")!=2) || !data.at("levels").is_object())throw std::runtime_error("format");
        for(auto& [id,value]:data.at("levels").items()){
            if(id.size()!=3 || id[0]<'1' || id[0]>'6' || id[1]!='-' || id[2]<'1' || id[2]>'4')throw std::runtime_error("level");
            Entry e{value.at("completions").get<unsigned>(),value.at("modes").get<unsigned>(),
                value.at("bestHealth").get<unsigned>(),value.at("sameFruitMask").get<unsigned>(),value.at("fullHealth").get<bool>()};
            if(data.at("version")==2){e.bestHearts=value.at("bestHearts").get<unsigned>();e.allHearts=value.at("allHearts").get<bool>();}
            if(!e.completions || e.completions>1000000 || !e.modes || (e.modes&~3) || e.bestHealth>8
                || (e.sameFruit&~fruit_mask) || e.fullHealth!=(e.bestHealth==8) || e.bestHearts>3 || e.allHearts!=(e.bestHearts==3))throw std::runtime_error("entry");
            ledger.levels[(id[0]-'1')*4+id[2]-'1']=e;
        }
    }catch(...){ledger={};ledger.readable=false;ledger.saved=false;fprintf(stderr,"[Achievements] Existing file preserved; invalid or unsupported format.\n");}
}
void start_session(unsigned mode){std::lock_guard lock(mutex);session_allowed=mode<2 || mode==3 || mode==5;tracker.reset();}
View view(){std::lock_guard lock(mutex);return ledger;}
void frame(uint8_t* rdram,recomp_context* ctx,bool allowed){
    constexpr auto root=gpr(int32_t(0x800F7EA0));
    if(uint32_t(ctx->r4)!=uint32_t(root))return;
    std::lock_guard lock(mutex);
    if(!initialized || !session_allowed || !allowed || !ledger.readable)return;
    Sample s;s.mode=MEM_W(0,gpr(int32_t(0x800F8B88)));
    unsigned page=MEM_BU(0,gpr(int32_t(0x800F8DFC))),column=MEM_BU(0,gpr(int32_t(0x800F8DFD)));
    if(page>=6 || column>=4)return;
    s.level=page*4+column;s.fruits=MEM_W(0,gpr(int32_t(0x800F89D0)));s.ending=MEM_W(0,gpr(int32_t(0x800F8AC8)));
    const uint32_t player=MEM_W(0,gpr(int32_t(0x800F8828)));
    const auto actor=static_cast<gpr>(static_cast<int32_t>(player));
    s.playable=player>=0x80000400 && player<0x807FFE00 && MEM_HU(12,actor)==0x4001;
    unsigned nativeColor=MEM_W(0xF10,root);if(nativeColor>=8)return;
    s.health=MEM_W(0xF14+nativeColor*4,root);
    s.hearts=MEM_W(0xAB8,root);
    if(s.fruits==30)for(unsigned i=0;i<30;i++)s.ring[i]=MEM_W(0xB34+i*4,root);
    auto finish=tracker.observe(s);if(!finish)return;
    auto& e=ledger.levels[finish->level];if(e.completions<1000000)e.completions++;
    e.modes|=1u<<finish->mode;e.bestHealth=(std::max)(e.bestHealth,finish->health);
    e.fullHealth|=finish->health==8;e.sameFruit|=finish->sameFruit;
    e.bestHearts=(std::max)(e.bestHearts,finish->hearts);e.allHearts|=finish->hearts==3;
    ledger.saved=persist();
    fprintf(stderr,"[Achievements] level=%u-%u mode=%u health=%u same-fruit=%03X saved=%u\n",page+1,column+1,finish->mode,finish->health,finish->sameFruit,unsigned(ledger.saved));
}
}
