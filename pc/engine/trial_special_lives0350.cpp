#include "trial_special_lives0350.hpp"
#include "json/json.hpp"
#include <atomic>
#include <chrono>
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

namespace workshop::trial_special_lives0350 {
namespace {
namespace fs=std::filesystem;
std::mutex transaction_mutex;
std::atomic_uint64_t serial{0};
constexpr size_t max_file_bytes=1024;
bool regular(const fs::path& file){
    std::error_code error;const auto status=fs::symlink_status(file,error);
    if(error&&error!=std::errc::no_such_file_or_directory)throw std::runtime_error("trial-lives-status-failed");
    if(!fs::exists(status))return false;
    if(fs::is_symlink(status)||!fs::is_regular_file(status))throw std::runtime_error("trial-lives-not-regular");
    return true;
}
unsigned read_disk(const fs::path& file,bool allow_missing,bool& present){
    present=regular(file);
    if(!present){if(!allow_missing)throw std::runtime_error("trial-lives-disappeared");return 0;}
    std::ifstream stream(file,std::ios::binary);if(!stream)throw std::runtime_error("trial-lives-open-failed");
    std::string bytes(max_file_bytes+1,'\0');stream.read(bytes.data(),std::streamsize(bytes.size()));bytes.resize(size_t(stream.gcount()));
    if(bytes.size()>max_file_bytes)throw std::runtime_error("trial-lives-too-large");
    if(!stream.eof()&&!stream)throw std::runtime_error("trial-lives-read-failed");
    bool duplicate=false;std::vector<std::set<std::string>> keys;
    auto callback=[&](int,nlohmann::json::parse_event_t event,nlohmann::json& value){
        if(event==nlohmann::json::parse_event_t::object_start)keys.emplace_back();
        else if(event==nlohmann::json::parse_event_t::key){if(keys.empty()||!keys.back().insert(value.get<std::string>()).second)duplicate=true;}
        else if(event==nlohmann::json::parse_event_t::object_end&&!keys.empty())keys.pop_back();
        return true;
    };
    const auto j=nlohmann::json::parse(bytes,callback);
    if(duplicate||!j.is_object()||j.size()!=2||!j.contains("schema")||!j["schema"].is_number_unsigned()||j["schema"]!=1u||
       !j.contains("lostMask")||!j["lostMask"].is_number_unsigned())throw std::runtime_error("trial-lives-format-invalid");
    const auto value=j["lostMask"].get<uint64_t>();
    if(value>0xFFFFu||(value&~uint64_t(special_mask)))throw std::runtime_error("trial-lives-mask-invalid");
    return unsigned(value);
}
std::string encode(unsigned lost){return nlohmann::json{{"schema",1u},{"lostMask",lost}}.dump(2)+'\n';}
class FileLock {
#ifdef _WIN32
    HANDLE handle_=INVALID_HANDLE_VALUE;
#else
    int handle_=-1;
#endif
public:
    explicit FileLock(const fs::path& profile){
        const auto file=profile/"pc-trial-special-lives0350.lock";
        (void)regular(file);
#ifdef _WIN32
        handle_=CreateFileW(file.c_str(),GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(handle_==INVALID_HANDLE_VALUE)throw std::runtime_error("trial-lives-lock-unavailable");
#else
        handle_=::open(file.c_str(),O_RDWR|O_CREAT|O_CLOEXEC|O_NOFOLLOW,0600);
        if(handle_<0)throw std::runtime_error("trial-lives-lock-unavailable");
        if(::flock(handle_,LOCK_EX|LOCK_NB)!=0){::close(handle_);handle_=-1;throw std::runtime_error("trial-lives-lock-unavailable");}
#endif
    }
    ~FileLock(){
#ifdef _WIN32
        if(handle_!=INVALID_HANDLE_VALUE)CloseHandle(handle_);
#else
        if(handle_>=0){::flock(handle_,LOCK_UN);::close(handle_);}
#endif
    }
    FileLock(const FileLock&)=delete;FileLock& operator=(const FileLock&)=delete;
};
void atomic_replace(const fs::path& file,const std::string& bytes){
    auto temp=file;temp+=".tmp-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(serial.fetch_add(1));
    try{
#ifdef _WIN32
        HANDLE handle=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error("trial-lives-temp-create-failed");
        DWORD written=0;const bool good=WriteFile(handle,bytes.data(),DWORD(bytes.size()),&written,nullptr)&&written==bytes.size()&&FlushFileBuffers(handle);
        const bool closed=CloseHandle(handle)!=0;if(!good||!closed)throw std::runtime_error("trial-lives-temp-flush-failed");
        if(!MoveFileExW(temp.c_str(),file.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("trial-lives-replace-failed");
#else
        const int handle=::open(temp.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);
        if(handle<0)throw std::runtime_error("trial-lives-temp-create-failed");
        size_t offset=0;bool good=true;
        while(offset<bytes.size()){
            const auto n=::write(handle,bytes.data()+offset,bytes.size()-offset);
            if(n<0&&errno==EINTR)continue;if(n<=0){good=false;break;}offset+=size_t(n);
        }
        const bool flushed=good&&::fsync(handle)==0;const bool closed=::close(handle)==0;
        if(!flushed||!closed)throw std::runtime_error("trial-lives-temp-flush-failed");
        fs::rename(temp,file);
#endif
    }catch(...){std::error_code ignored;fs::remove(temp,ignored);throw;}
}
void failed_read(Snapshot& view,const std::exception& error){view.readable=view.last_write_succeeded=false;view.reason=error.what();}
}
bool Store::load(const fs::path& profile){
    std::lock_guard guard(transaction_mutex);profile_=profile;file_=profile/filename;view_={};initialized_=present_=false;failed_mask_=0;
    try{
        if(profile.empty()||!fs::is_directory(profile)||fs::is_symlink(fs::symlink_status(profile)))throw std::runtime_error("trial-lives-profile-missing");
        view_.lost_mask=read_disk(file_,true,present_);view_.readable=view_.last_write_succeeded=initialized_=true;return true;
    }catch(const std::exception& error){failed_read(view_,error);return false;}
}
Snapshot Store::snapshot()const{std::lock_guard guard(transaction_mutex);return view_;}
bool Store::refresh(){
    std::lock_guard guard(transaction_mutex);if(!initialized_||!view_.readable)return false;
    try{
        bool present=false;view_.lost_mask=read_disk(file_,!present_,present);present_=present;
        view_.last_write_succeeded=failed_mask_==0;if(!failed_mask_)view_.reason.clear();return true;
    }catch(const std::exception& error){initialized_=false;failed_read(view_,error);return false;}
}
Reply Store::lose(unsigned color){return update(color,true);}
Reply Store::recover(unsigned color){return update(color,false);}
Reply Store::update(unsigned color,bool lost){
    std::lock_guard guard(transaction_mutex);
    auto result=[&](Status status){return Reply{status,view_.lost_mask};};
    if(!special(color))return result(Status::InvalidColor);
    if(!initialized_||!view_.readable)return result(Status::Unavailable);
    try{
        FileLock lock(profile_);bool present=false;unsigned current=0;
        try{current=read_disk(file_,!present_,present);}
        catch(const std::exception& error){initialized_=false;failed_read(view_,error);return result(Status::Unavailable);}
        present_=present;view_.lost_mask=current;
        const unsigned bit=1u<<color,next=lost?(current|bit):(current&~bit);
        if(next==current){failed_mask_&=~bit;view_.last_write_succeeded=failed_mask_==0;if(!failed_mask_)view_.reason.clear();return result(Status::Unchanged);}
        atomic_replace(file_,encode(next));present_=true;view_.lost_mask=next;failed_mask_&=~bit;view_.last_write_succeeded=failed_mask_==0;if(!failed_mask_)view_.reason.clear();return result(Status::Committed);
    }catch(const std::exception& error){failed_mask_|=1u<<color;view_.last_write_succeeded=false;view_.reason=error.what();return result(Status::PersistenceFailed);}
}
} // namespace workshop::trial_special_lives0350
