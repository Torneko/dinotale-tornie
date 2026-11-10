#include "heart_wallet0283.hpp"
#include "json/json.hpp"
#include <algorithm>
#include <array>
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

namespace workshop::heart_wallet0283 {
namespace {
namespace fs=std::filesystem;
std::mutex transaction_mutex;
std::atomic_uint64_t serial{0};
constexpr size_t max_file_bytes=4096;
bool valid_attempt(std::string_view token){return token.size()==32&&std::all_of(token.begin(),token.end(),[](char c){return (c>='0'&&c<='9')||(c>='a'&&c<='f');});}
uint64_t next_random(uint64_t& state){
    uint64_t z=(state+=0x9E3779B97F4A7C15ull);
    z=(z^(z>>30))*0xBF58476D1CE4E5B9ull;
    z=(z^(z>>27))*0x94D049BB133111EBull;
    return z^(z>>31);
}
unsigned bounded(uint64_t& state,unsigned bound){
    const uint64_t n=bound,threshold=(uint64_t(0)-n)%n;
    for(;;){const auto value=next_random(state);if(value>=threshold)return unsigned(value%n);}
}
bool exists_regular(const fs::path& path){
    std::error_code error;const auto status=fs::symlink_status(path,error);
    if(error&&error!=std::errc::no_such_file_or_directory)throw std::runtime_error("wallet-status-failed");
    if(!fs::exists(status))return false;
    if(!fs::is_regular_file(status)||fs::is_symlink(status))throw std::runtime_error("wallet-not-regular-file");
    return true;
}
Data read_disk(const fs::path& file,bool allow_missing,bool& present){
    present=exists_regular(file);
    if(!present){if(!allow_missing)throw std::runtime_error("wallet-disappeared");return {};}
    std::ifstream input(file,std::ios::binary);if(!input)throw std::runtime_error("wallet-open-failed");
    std::string bytes(max_file_bytes+1,'\0');input.read(bytes.data(),std::streamsize(bytes.size()));bytes.resize(size_t(input.gcount()));
    if(bytes.size()>max_file_bytes)throw std::runtime_error("wallet-too-large");
    if(!input.eof()&&!input)throw std::runtime_error("wallet-read-failed");
    bool duplicate=false;std::vector<std::set<std::string>> keys;
    auto callback=[&](int,nlohmann::json::parse_event_t event,nlohmann::json& parsed){
        if(event==nlohmann::json::parse_event_t::object_start)keys.emplace_back();
        else if(event==nlohmann::json::parse_event_t::key){if(keys.empty()||!keys.back().insert(parsed.get<std::string>()).second)duplicate=true;}
        else if(event==nlohmann::json::parse_event_t::object_end){if(!keys.empty())keys.pop_back();}
        return true;
    };
    const auto json=nlohmann::json::parse(bytes,callback);
    const bool modern=json.is_object()&&json.contains("lastAttempt");
    if(duplicate||!json.is_object()||json.size()!=(modern?4u:3u)||!json.contains("schema")||!json["schema"].is_number_unsigned()||json["schema"]!=1u||
       !json.contains("held")||!json["held"].is_string()||!json.contains("pending")||!json["pending"].is_object())throw std::runtime_error("wallet-format-invalid");
    const auto bits=json["held"].get<std::string>();const auto& pending=json["pending"];
    if(bits.size()!=slot_count||pending.size()!=2||!pending.contains("life")||!pending["life"].is_boolean()||!pending.contains("eggs")||!pending["eggs"].is_boolean())throw std::runtime_error("wallet-shape-invalid");
    Data result;
    for(unsigned i=0;i<slot_count;++i){if(bits[i]!='0'&&bits[i]!='1')throw std::runtime_error("wallet-slot-invalid");result.held.set(i,bits[i]=='1');}
    result.pending={pending["life"].get<bool>(),pending["eggs"].get<bool>()};
    if(modern){if(!json["lastAttempt"].is_string())throw std::runtime_error("wallet-attempt-invalid");result.last_attempt=json["lastAttempt"].get<std::string>();if(!result.last_attempt.empty()&&!valid_attempt(result.last_attempt))throw std::runtime_error("wallet-attempt-invalid");}
    return result;
}
std::string encode(const Data& data){
    std::string bits(slot_count,'0');for(unsigned i=0;i<slot_count;++i)if(data.held.test(i))bits[i]='1';
    return nlohmann::json{{"schema",1u},{"held",bits},{"pending",{{"life",data.pending.life},{"eggs",data.pending.eggs}}},{"lastAttempt",data.last_attempt}}.dump(2)+'\n';
}
class FileLock {
#ifdef _WIN32
    HANDLE handle_=INVALID_HANDLE_VALUE;
#else
    int handle_=-1;
#endif
public:
    explicit FileLock(const fs::path& profile){
        const auto path=profile/"pc-heart-wallet.lock";
        // A real OS handle lock also serializes independent Wallet instances
        // and processes. The lock contains no progression or credit.
        std::error_code error;const auto status=fs::symlink_status(path,error);
        if((error&&error!=std::errc::no_such_file_or_directory)||fs::is_symlink(status))throw std::runtime_error("wallet-lock-status-failed");
#ifdef _WIN32
        handle_=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(handle_==INVALID_HANDLE_VALUE)throw std::runtime_error("wallet-lock-unavailable");
#else
        handle_=::open(path.c_str(),O_RDWR|O_CREAT|O_CLOEXEC|O_NOFOLLOW,0600);
        if(handle_<0)throw std::runtime_error("wallet-lock-unavailable");
        if(::flock(handle_,LOCK_EX|LOCK_NB)!=0){::close(handle_);handle_=-1;throw std::runtime_error("wallet-lock-unavailable");}
#endif
    }
    ~FileLock(){
#ifdef _WIN32
        if(handle_!=INVALID_HANDLE_VALUE)CloseHandle(handle_);
#else
        if(handle_>=0){::flock(handle_,LOCK_UN);::close(handle_);}
#endif
    }
    FileLock(const FileLock&)=delete;
    FileLock& operator=(const FileLock&)=delete;
};
void atomic_replace(const fs::path& file,const std::string& bytes){
    auto temp=file;temp+=".tmp-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(serial.fetch_add(1));
    try{
#ifdef _WIN32
        HANDLE handle=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error("wallet-temp-create-failed");
        DWORD written=0;const bool flushed=WriteFile(handle,bytes.data(),DWORD(bytes.size()),&written,nullptr)&&written==bytes.size()&&FlushFileBuffers(handle);
        const bool closed=CloseHandle(handle)!=0;if(!flushed||!closed)throw std::runtime_error("wallet-temp-flush-failed");
        if(!MoveFileExW(temp.c_str(),file.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("wallet-replace-failed");
#else
        const int handle=::open(temp.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(handle<0)throw std::runtime_error("wallet-temp-create-failed");
        size_t offset=0;bool good=true;
        while(offset<bytes.size()){const auto count=::write(handle,bytes.data()+offset,bytes.size()-offset);if(count<0&&errno==EINTR)continue;if(count<=0){good=false;break;}offset+=size_t(count);}
        const bool flushed=good&&::fsync(handle)==0;const bool closed=::close(handle)==0;if(!flushed||!closed)throw std::runtime_error("wallet-temp-flush-failed");
        fs::rename(temp,file);
#endif
    }catch(...){std::error_code ignored;fs::remove(temp,ignored);throw;}
}
} // namespace
std::optional<SpendPlan> plan_spend(const Held& held,unsigned cost,uint64_t seed){
    const auto count=unsigned(held.count());if(!cost||cost>count)return {};
    std::array<unsigned,slot_count> candidates{};unsigned size=0;
    for(unsigned i=0;i<slot_count;++i)if(held.test(i))candidates[size++]=i;
    SpendPlan result{held,{}};
    for(unsigned i=0;i<cost;++i){const auto chosen=i+bounded(seed,size-i);std::swap(candidates[i],candidates[chosen]);result.remaining.reset(candidates[i]);result.removed.set(candidates[i]);}
    return result;
}
bool Wallet::load(const fs::path& profile){
    std::lock_guard guard(transaction_mutex);profile_=profile;file_=profile/filename;view_={};initialized_=present_=false;
    try{
        if(profile.empty()||!fs::is_directory(profile))throw std::runtime_error("wallet-profile-directory-missing");
        view_.value=read_disk(file_,true,present_);view_.readable=view_.last_write_succeeded=initialized_=true;return true;
    }catch(const std::exception& error){view_.reason=error.what();return false;}
}
Snapshot Wallet::snapshot()const{std::lock_guard guard(transaction_mutex);return view_;}
bool Wallet::refresh(){
    std::lock_guard guard(transaction_mutex);if(!initialized_||!view_.readable)return false;
    try{
        FileLock lock(profile_);bool exists=false;
        try{view_.value=read_disk(file_,!present_,exists);}
        catch(const std::exception& error){initialized_=false;view_.readable=view_.last_write_succeeded=false;view_.reason=error.what();return false;}
        present_=exists;view_.last_write_succeeded=true;view_.reason.clear();return true;
    }catch(const std::exception& error){view_.last_write_succeeded=false;view_.reason=error.what();return false;}
}

Reply Wallet::claim(unsigned level,unsigned slot){return update(Action::Claim,level,slot,0);}
Reply Wallet::spend(unsigned cost,uint64_t seed){return update(Action::Spend,cost,0,seed);}
Reply Wallet::purchase(Bonus bonus,uint64_t seed){
    if(bonus==Bonus::Life)return update(Action::Life,life_cost,0,seed);
    if(bonus==Bonus::Eggs)return update(Action::Eggs,eggs_cost,0,seed);
    std::lock_guard guard(transaction_mutex);return {Status::InvalidBonus,{},{},view_.value.balance()};
}
Reply Wallet::consume_pending(){return update(Action::Consume,0,0,0);}
Reply Wallet::consume_pending(std::string_view attempt,Pending applied){return update(Action::ConsumeScoped,0,0,0,attempt,applied);}
Reply Wallet::update(Action action,unsigned a,unsigned b,uint64_t seed,std::string_view attempt,Pending applied){
    std::lock_guard guard(transaction_mutex);
    auto reply=[&](Status status){return Reply{status,{},{},view_.value.balance()};};
    if(!initialized_||!view_.readable)return reply(Status::Unavailable);
    if(action==Action::ConsumeScoped&&(!valid_attempt(attempt)||applied.empty()))return reply(Status::InvalidAttempt);
    if(action==Action::Claim&&!index(a,b))return reply(Status::InvalidSlot);
    if((action==Action::Spend||action==Action::Life||action==Action::Eggs)&&(!a||a>slot_count))return reply(Status::InvalidCost);
    try{
        FileLock lock(profile_);
        Data current;bool exists=false;
        try{current=read_disk(file_,!present_,exists);}
        catch(const std::exception& error){initialized_=false;view_.readable=view_.last_write_succeeded=false;view_.reason=error.what();return reply(Status::Unavailable);}
        view_.value=current;present_=exists;view_.last_write_succeeded=true;view_.reason.clear();
        auto next=current;Held removed;Pending bonuses;
        if(action==Action::Claim){
            const auto bit=*index(a,b);if(next.held.test(bit))return reply(Status::AlreadyHeld);next.held.set(bit);
        }else if(action==Action::Consume||action==Action::ConsumeScoped){
            if(action==Action::ConsumeScoped){
                if(next.last_attempt==attempt)return reply(Status::AlreadyConsumed);
                if((applied.life&&!next.pending.life)||(applied.eggs&&!next.pending.eggs))return reply(Status::PendingChanged);
                bonuses=applied;next.pending.life=next.pending.life&&!applied.life;next.pending.eggs=next.pending.eggs&&!applied.eggs;next.last_attempt=std::string(attempt);
            }else{if(next.pending.empty())return reply(Status::NoPending);bonuses=next.pending;next.pending={};}
        }else{
            if((action==Action::Life&&next.pending.life)||(action==Action::Eggs&&next.pending.eggs))return reply(Status::AlreadyPending);
            const auto plan=plan_spend(next.held,a,seed);if(!plan)return reply(Status::InsufficientFunds);
            next.held=plan->remaining;removed=plan->removed;
            if(action==Action::Life)next.pending.life=true;
            if(action==Action::Eggs)next.pending.eggs=true;
        }
        atomic_replace(file_,encode(next));view_.value=next;view_.last_write_succeeded=true;present_=true;
        return {Status::Committed,removed,bonuses,next.balance()};
    }catch(const std::exception& error){view_.last_write_succeeded=false;view_.reason=error.what();return reply(Status::PersistenceFailed);}
}
} // namespace workshop::heart_wallet0283
