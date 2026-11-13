#include "custom_session.hpp"
#include "workshop_menu_game.hpp"
#include "workshop_data.hpp"
#include "json/json.hpp"
#include <atomic>
#include <fstream>
#include <filesystem>
#include <cstdio>
#include <mutex>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
namespace workshop::custom_session {
namespace {
std::string token;bool automatic=false;int preferred=-1,stage=-1;
std::atomic_bool completed=false,stored=true;
nlohmann::json result;std::mutex mutex;
bool persist(){
    const auto path=std::filesystem::path("custom-run-result.json"),temp=std::filesystem::path("custom-run-result.json.tmp");
    try{
        {std::ofstream f(temp,std::ios::binary|std::ios::trunc);f<<result.dump(2)<<'\n';f.flush();if(!f)throw std::runtime_error("write");}
#ifdef _WIN32
        if(!MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("replace");
#else
        std::filesystem::rename(temp,path);
#endif
        return true;
    }catch(const std::exception& e){fprintf(stderr,"[CustomSession] Result not saved: %s\n",e.what());return false;}
}
}
void configure(const std::string& value,bool next,int color,unsigned source){
    workshop::require(value.size()==32 && value.find_first_not_of("0123456789abcdefABCDEF")==std::string::npos,"Invalid custom run token");
    workshop::require(color>=-1 && color<18,"Invalid preferred Yoshi");
    workshop::require(vanilla_stage(source)>=0,"Invalid custom level source");
    stage=vanilla_stage(source);token=value;automatic=next;preferred=color;completed=false;stored=true;
}
bool active(){return !token.empty();}
bool ready(){return completed;}
bool saved(){return stored;}
int preferred_color(){return preferred;}
void release_preferred_color(){
    if(preferred<0)return;
    fprintf(stderr,"[CustomSession] Preferred Yoshi %d unavailable; returning selection to the player.\n",preferred);
    preferred=-1;automatic=false;
}
int source_stage(){return active()?stage:-1;}
void navigation(unsigned frame,uint16_t* buttons,float* x,float* y){
    if(!active() || ready())return;const auto s=book::snapshot();
    if(s.paused || s.stage!=0)return;
    // Choose the embedded custom level, using native controller input. The
    // first Yoshi remains a player choice. Subsequent launches retain it.
    if((s.scene==41 || (automatic && (s.scene==4 || s.scene==5))) && frame%120<8){*buttons|=s.scene==4?0x1000:0x8000;*x=*y=0;}
}
void finish(uint8_t* rdram){
    if(!active() || ready() || uint32_t(MEM_W(0,int32_t(0x800F89D0)))!=30 || uint32_t(MEM_W(0,int32_t(0x800F8AC8)))!=1)return;
    std::lock_guard lock(mutex);if(completed)return;
    const auto color=uint32_t(MEM_W(0,int32_t(0x807F0008)));
    result={{"schema",1},{"token",token},{"completed",true},{"fruits",30},{"selectedColor",color<18?color:0},{"advance",true}};
    stored=persist();completed=true;
    fprintf(stderr,"[CustomSession] finished token=%s fruits=30 color=%u saved=%d\n",token.c_str(),color,int(stored.load()));fflush(stderr);
}
bool close(bool advance){std::lock_guard lock(mutex);if(!ready() || !saved())return false;result["advance"]=advance;stored=persist();return stored;}
}
