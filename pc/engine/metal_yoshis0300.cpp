#include "metal_yoshis0300.hpp"
#include "game_modes0320.hpp"
#include "hybrid_completion0290.hpp"
#include "json/json.hpp"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <set>
#include <stdexcept>
#include <system_error>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif
namespace workshop::metal_yoshis0300 {
namespace {
namespace fs=std::filesystem;
struct Data {unsigned found=0,story_alive=0;bool operator==(const Data&)const=default;};
std::mutex mutex;std::atomic_uint64_t serial{0};
Data data;View current;fs::path filename;bool present=false,scope=false;unsigned session=4;
enum class Action {Acquire,StoryLives,Restore,Clear};
void publish(){current.found=data.found;current.story_alive=data.story_alive;}
bool playable_scope(){return scope&&game_modes0320::solo_playable(int(session));}
bool search_scope(){return playable_scope()&&session!=2;}
bool completion_ready(){const auto complete=hybrid_completion0290::view();return complete.readable&&complete.saved&&complete.mask==hybrid_completion0290::all_levels;}
bool exists_regular(const fs::path& path){
    std::error_code error;const auto status=fs::symlink_status(path,error);
    if(error&&error!=std::errc::no_such_file_or_directory)throw std::runtime_error("metal-status-failed");
    if(!fs::exists(status))return false;
    if(!fs::is_regular_file(status)||fs::is_symlink(status))throw std::runtime_error("metal-not-regular-file");
    return true;
}
Data read_disk(bool allow_missing,bool& found){
    found=exists_regular(filename);if(!found){if(!allow_missing)throw std::runtime_error("metal-journal-disappeared");return {};}
    if(fs::file_size(filename)>4096)throw std::runtime_error("metal-journal-too-large");
    std::ifstream file(filename,std::ios::binary);if(!file)throw std::runtime_error("metal-open-failed");
    std::string bytes(4097,'\0');file.read(bytes.data(),std::streamsize(bytes.size()));bytes.resize(size_t(file.gcount()));
    if(bytes.size()>4096||(!file.eof()&&!file))throw std::runtime_error("metal-read-failed");
    bool duplicate=false;std::vector<std::set<std::string>> keys;
    auto callback=[&](int,nlohmann::json::parse_event_t event,nlohmann::json& parsed){
        if(event==nlohmann::json::parse_event_t::object_start)keys.emplace_back();
        else if(event==nlohmann::json::parse_event_t::key){if(keys.empty()||!keys.back().insert(parsed.get<std::string>()).second)duplicate=true;}
        else if(event==nlohmann::json::parse_event_t::object_end){if(!keys.empty())keys.pop_back();}
        return true;
    };
    const auto json=nlohmann::json::parse(bytes,callback);
    auto bits=[&](const char* key){return json.contains(key)&&json[key].is_number_unsigned()&&json[key].get<uint64_t>()<=3;};
    if(duplicate||!json.is_object()||json.size()!=3||!json.contains("schema")||!json["schema"].is_number_unsigned()||json["schema"]!=1u||!bits("foundMask")||!bits("storyAliveMask"))throw std::runtime_error("metal-format-invalid");
    const Data result{json["foundMask"].get<unsigned>(),json["storyAliveMask"].get<unsigned>()};
    if(result.story_alive&~result.found)throw std::runtime_error("metal-unearned-story-life");return result;
}
class FileLock {
#ifdef _WIN32
    HANDLE handle=INVALID_HANDLE_VALUE;
#else
    int handle=-1;
#endif
public:
    explicit FileLock(const fs::path& file){auto path=file;path+=".lock";(void)exists_regular(path);
#ifdef _WIN32
        handle=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error("metal-lock-unavailable");
#else
        handle=::open(path.c_str(),O_RDWR|O_CREAT|O_CLOEXEC|O_NOFOLLOW,0600);if(handle<0)throw std::runtime_error("metal-lock-unavailable");if(::flock(handle,LOCK_EX|LOCK_NB)!=0){::close(handle);handle=-1;throw std::runtime_error("metal-lock-unavailable");}
#endif
    }
    ~FileLock(){
#ifdef _WIN32
        if(handle!=INVALID_HANDLE_VALUE)CloseHandle(handle);
#else
        if(handle>=0){::flock(handle,LOCK_UN);::close(handle);}
#endif
    }
    FileLock(const FileLock&)=delete;FileLock& operator=(const FileLock&)=delete;
};
void atomic_replace(const Data& next){
    const auto bytes=nlohmann::json{{"schema",1u},{"foundMask",next.found},{"storyAliveMask",next.story_alive}}.dump(2)+'\n';
    auto temp=filename;temp+=".tmp-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(serial.fetch_add(1));
    try{
#ifdef _WIN32
        HANDLE handle=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error("metal-temp-create-failed");
        DWORD written=0;const bool flushed=WriteFile(handle,bytes.data(),DWORD(bytes.size()),&written,nullptr)&&written==bytes.size()&&FlushFileBuffers(handle);
        const bool closed=CloseHandle(handle)!=0;if(!flushed||!closed)throw std::runtime_error("metal-temp-flush-failed");
        if(!MoveFileExW(temp.c_str(),filename.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("metal-replace-failed");
#else
        const int handle=::open(temp.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(handle<0)throw std::runtime_error("metal-temp-create-failed");
        size_t offset=0;bool good=true;while(offset<bytes.size()){const auto count=::write(handle,bytes.data()+offset,bytes.size()-offset);if(count<0&&errno==EINTR)continue;if(count<=0){good=false;break;}offset+=size_t(count);}
        const bool flushed=good&&::fsync(handle)==0;const bool closed=::close(handle)==0;if(!flushed||!closed)throw std::runtime_error("metal-temp-flush-failed");fs::rename(temp,filename);
#endif
    }catch(...){std::error_code ignored;fs::remove(temp,ignored);throw;}
}
bool update(Action action,unsigned value){
    if(!current.initialized||!current.readable)return false;
    try{
        FileLock lock(filename);bool found=false;Data before;
        try{before=read_disk(!present,found);}catch(...){current.readable=false;throw;}
        data=before;present=found;publish();auto next=before;
        if(action==Action::Acquire){next.found|=value;if(session!=game_modes0320::StoryHybrid)next.story_alive|=value&~before.found;}
        else if(action==Action::StoryLives){if(value&~before.found)return false;next.story_alive=value;}
        else if(action==Action::Restore){if(!(value&before.found))return false;next.story_alive|=value;}
        else next={};
        if(next!=before){atomic_replace(next);present=true;}
        data=next;current.saved=true;current.reason.clear();publish();return true;
    }catch(const std::exception& error){current.saved=false;current.reason=error.what();fprintf(stderr,"[MetalYoshi] preserved reason=%s\n",error.what());return false;}
}
}
void initialize(const fs::path& path){
    std::lock_guard guard(mutex);filename=path;data={};current={};scope=present=false;session=4;
    try{if(filename.empty())throw std::runtime_error("metal-path-invalid");data=read_disk(true,present);current.initialized=true;publish();}
    catch(const std::exception& error){current.readable=current.saved=false;current.reason=error.what();fprintf(stderr,"[MetalYoshi] preserved reason=%s\n",error.what());}
}
void start_session(unsigned mode,bool ordinary_solo){std::lock_guard guard(mutex);session=mode;scope=ordinary_solo;}
void set_scope(bool ordinary_solo){std::lock_guard guard(mutex);scope=ordinary_solo;}
View view(){std::lock_guard guard(mutex);return current;}
unsigned search_mask(){std::lock_guard guard(mutex);return search_scope()&&current.initialized&&current.readable&&current.saved&&completion_ready()?(~data.found)&3u:0;}
unsigned unlocked_extra(){std::lock_guard guard(mutex);return playable_scope()&&current.initialized&&current.readable?yoshi_colors0300::extra_from_metals(data.found):0;}
unsigned story_extra(bool continuing){std::lock_guard guard(mutex);return playable_scope()&&current.initialized&&current.readable?yoshi_colors0300::extra_from_metals(continuing?data.story_alive:data.found):0;}
bool acquire(unsigned pc_color){
    std::lock_guard guard(mutex);const auto bit=yoshi_colors0300::metal_bit(pc_color);
    if(!bit||!search_scope()||!current.initialized||!current.readable||(!(data.found&bit)&&!completion_ready()))return false;
    return update(Action::Acquire,bit);
}
bool save_story_alive(unsigned bits){std::lock_guard guard(mutex);return bits<=3&&scope&&session==0&&update(Action::StoryLives,bits);}
bool restore_life(unsigned pc_color){
    std::lock_guard guard(mutex);const auto bit=yoshi_colors0300::metal_bit(pc_color);
    if(!bit||!playable_scope()||!current.initialized||!current.readable||!(data.found&bit))return false;
    return session!=0||update(Action::Restore,bit);
}
bool clear_save(){std::lock_guard guard(mutex);return update(Action::Clear,0);}
}
