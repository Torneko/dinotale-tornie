#include "elemental_yoshis0355.hpp"
#include "game_modes0320.hpp"
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
namespace workshop::elemental_yoshis0355 {
namespace {
namespace fs=std::filesystem;
struct Data {unsigned found=0,story_alive=0,hybrid_alive=0;bool operator==(const Data&)const=default;};
std::mutex mutex;std::atomic_uint64_t serial{0};
Data data;View current;fs::path filename;bool present=false,scope=false;unsigned session=4;
enum class Action {Acquire,StoryLives,Restore,Clear};
void publish(){current.found=data.found;current.story_alive=data.story_alive;current.hybrid_alive=data.hybrid_alive;}
bool playable_scope(){return scope&&game_modes0320::valid(int(session));}
bool search_scope(){return playable_scope()&&session==6;}
bool exists_regular(const fs::path& path){
    std::error_code error;const auto status=fs::symlink_status(path,error);
    if(error&&error!=std::errc::no_such_file_or_directory)throw std::runtime_error("elemental-status-failed");
    if(!fs::exists(status))return false;
    if(!fs::is_regular_file(status)||fs::is_symlink(status))throw std::runtime_error("elemental-not-regular-file");
    return true;
}
Data read_disk(bool allow_missing,bool& found){
    found=exists_regular(filename);if(!found){if(!allow_missing)throw std::runtime_error("elemental-journal-disappeared");return {};}
    if(fs::file_size(filename)>4096)throw std::runtime_error("elemental-journal-too-large");
    std::ifstream file(filename,std::ios::binary);if(!file)throw std::runtime_error("elemental-open-failed");
    std::string bytes(4097,'\0');file.read(bytes.data(),std::streamsize(bytes.size()));bytes.resize(size_t(file.gcount()));
    if(bytes.size()>4096||(!file.eof()&&!file))throw std::runtime_error("elemental-read-failed");
    bool duplicate=false;std::vector<std::set<std::string>> keys;
    auto callback=[&](int,nlohmann::json::parse_event_t event,nlohmann::json& parsed){
        if(event==nlohmann::json::parse_event_t::object_start)keys.emplace_back();
        else if(event==nlohmann::json::parse_event_t::key){if(keys.empty()||!keys.back().insert(parsed.get<std::string>()).second)duplicate=true;}
        else if(event==nlohmann::json::parse_event_t::object_end){if(!keys.empty())keys.pop_back();}
        return true;
    };
    const auto json=nlohmann::json::parse(bytes,callback);
    auto bits=[&](const char* key){return json.contains(key)&&json[key].is_number_unsigned()&&json[key].get<uint64_t>()<=3;};
    if(duplicate||!json.is_object()||!json.contains("schema")||!json["schema"].is_number_unsigned()||(json["schema"]!=1u&&json["schema"]!=2u)||json.size()!=(json["schema"]==1u?3u:4u)||!bits("foundMask")||!bits("storyAliveMask")||(json["schema"]==2u&&!bits("hybridAliveMask")))throw std::runtime_error("elemental-format-invalid");
    const Data result{json["foundMask"].get<unsigned>(),json["storyAliveMask"].get<unsigned>(),json["schema"]==2u?json["hybridAliveMask"].get<unsigned>():0u};
    if((result.story_alive|result.hybrid_alive)&~result.found)throw std::runtime_error("elemental-unearned-story-life");return result;
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
        handle=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error("elemental-lock-unavailable");
#else
        handle=::open(path.c_str(),O_RDWR|O_CREAT|O_CLOEXEC|O_NOFOLLOW,0600);if(handle<0)throw std::runtime_error("elemental-lock-unavailable");if(::flock(handle,LOCK_EX|LOCK_NB)!=0){::close(handle);handle=-1;throw std::runtime_error("elemental-lock-unavailable");}
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
    const auto bytes=nlohmann::json{{"schema",2u},{"foundMask",next.found},{"storyAliveMask",next.story_alive},{"hybridAliveMask",next.hybrid_alive}}.dump(2)+'\n';
    auto temp=filename;temp+=".tmp-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(serial.fetch_add(1));
    try{
#ifdef _WIN32
        HANDLE handle=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error("elemental-temp-create-failed");
        DWORD written=0;const bool flushed=WriteFile(handle,bytes.data(),DWORD(bytes.size()),&written,nullptr)&&written==bytes.size()&&FlushFileBuffers(handle);
        const bool closed=CloseHandle(handle)!=0;if(!flushed||!closed)throw std::runtime_error("elemental-temp-flush-failed");
        if(!MoveFileExW(temp.c_str(),filename.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("elemental-replace-failed");
#else
        const int handle=::open(temp.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(handle<0)throw std::runtime_error("elemental-temp-create-failed");
        size_t offset=0;bool good=true;while(offset<bytes.size()){const auto count=::write(handle,bytes.data()+offset,bytes.size()-offset);if(count<0&&errno==EINTR)continue;if(count<=0){good=false;break;}offset+=size_t(count);}
        const bool flushed=good&&::fsync(handle)==0;const bool closed=::close(handle)==0;if(!flushed||!closed)throw std::runtime_error("elemental-temp-flush-failed");fs::rename(temp,filename);
#endif
    }catch(...){std::error_code ignored;fs::remove(temp,ignored);throw;}
}
bool update(Action action,unsigned value){
    if(!current.initialized||!current.readable)return false;
    try{
        fs::create_directories(fs::absolute(filename).parent_path());FileLock lock(filename);bool found=false;Data before;
        try{before=read_disk(!present,found);}catch(...){current.readable=false;throw;}
        data=before;present=found;publish();auto next=before;
        if(action==Action::Acquire){next.found|=value;next.story_alive|=value&~before.found;next.hybrid_alive|=value&~before.found;}
        else if(action==Action::StoryLives){if(value&~before.found)return false;if(session==5)next.hybrid_alive=value;else next.story_alive=value;}
        else if(action==Action::Restore){if(!(value&before.found))return false;if(session==5)next.hybrid_alive|=value;else next.story_alive|=value;}
        else next={};
        if(next!=before){atomic_replace(next);present=true;}
        data=next;current.saved=true;current.reason.clear();publish();return true;
    }catch(const std::exception& error){current.saved=false;current.reason=error.what();fprintf(stderr,"[ElementalYoshi] preserved reason=%s\n",error.what());return false;}
}
}
void initialize(const fs::path& path){
    std::lock_guard guard(mutex);filename=path;data={};current={};scope=present=false;session=4;
    try{if(filename.empty()||filename.filename()!="pc-elemental-yoshis.json")throw std::runtime_error("elemental-path-invalid");data=read_disk(true,present);current.initialized=true;publish();}
    catch(const std::exception& error){current.readable=current.saved=false;current.reason=error.what();fprintf(stderr,"[ElementalYoshi] preserved reason=%s\n",error.what());}
}
void start_session(unsigned mode,bool ordinary_solo){std::lock_guard guard(mutex);session=mode;scope=ordinary_solo;}
void set_scope(bool ordinary_solo){std::lock_guard guard(mutex);scope=ordinary_solo;}
View view(){std::lock_guard guard(mutex);return current;}
std::optional<unsigned> search_color(unsigned level){std::lock_guard guard(mutex);if(!search_scope()||!current.initialized||!current.readable||!current.saved)return {};return level==5?std::optional<unsigned>{16}:level==11?std::optional<unsigned>{17}:std::nullopt;}
unsigned unlocked_extra(){std::lock_guard guard(mutex);return playable_scope()&&current.initialized&&current.readable?data.found<<8:0;}
unsigned story_extra(bool continuing){std::lock_guard guard(mutex);return playable_scope()&&current.initialized&&current.readable?(continuing?(session==5?data.hybrid_alive:data.story_alive):data.found)<<8:0;}
bool acquire(unsigned pc_color,unsigned level,bool trusted_carry_finish){
    std::lock_guard guard(mutex);const auto bit=elemental_yoshis0355::bit(pc_color);
    if(!trusted_carry_finish||!bit||!search_scope()||!current.initialized||!current.readable||!((pc_color==16&&level==5)||(pc_color==17&&level==11)))return false;
    return update(Action::Acquire,bit);
}
bool confirm_owned(unsigned pc_color){
    std::lock_guard guard(mutex);const auto bit=elemental_yoshis0355::bit(pc_color);
    if(!bit||!search_scope()||!current.initialized||!current.readable||!(data.found&bit))return false;
    try{
        FileLock lock(filename);bool found=false;Data before;
        try{before=read_disk(false,found);}catch(...){current.readable=false;throw;}
        data=before;present=found;publish();
        if(!(before.found&bit)){current.reason="elemental-recovery-not-owned";return false;}
        current.saved=true;current.reason.clear();return true;
    }catch(const std::exception& error){current.saved=false;current.reason=error.what();fprintf(stderr,"[ElementalYoshi] recovery preserved reason=%s\n",error.what());return false;}
}
bool save_story_alive(unsigned bits){std::lock_guard guard(mutex);return bits<=3&&scope&&(session==0||session==5)&&update(Action::StoryLives,bits);}
bool restore_life(unsigned pc_color){
    std::lock_guard guard(mutex);const auto bit=elemental_yoshis0355::bit(pc_color);
    if(!bit||!playable_scope()||!current.initialized||!current.readable||!(data.found&bit))return false;
    return (session!=0&&session!=5)||update(Action::Restore,bit);
}
bool clear_save(){std::lock_guard guard(mutex);return update(Action::Clear,0);}
}
