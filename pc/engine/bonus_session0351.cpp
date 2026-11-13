#include "bonus_session0351.hpp"
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

namespace workshop::bonus_session0351 {
namespace {
namespace fs=std::filesystem;
std::mutex transaction_mutex;
std::atomic_uint64_t serial{0};
constexpr size_t max_file_bytes=32768;
unsigned low_bits(unsigned count){return count==32?0xFFFFFFFFu:(1u<<count)-1;}
Phase role_phase(const bonus_routes0351::Route& route,unsigned cursor){
 return route.steps[cursor].role==bonus_routes0351::StepRole::Finale?Phase::Finale:Phase::Active;
}
Status status(Transition t){
 switch(t){
  case Transition::Changed:return Status::Committed;
  case Transition::Unchanged:return Status::Unchanged;
  case Transition::WrongRoom:return Status::WrongRoom;
  case Transition::Unqualified:return Status::Unqualified;
  default:return Status::InvalidTransition;
 }
}
bool regular(const fs::path& file){
    std::error_code error;const auto status=fs::symlink_status(file,error);
    if(error&&error!=std::errc::no_such_file_or_directory)throw std::runtime_error("bonus-session-status-failed");
    if(!fs::exists(status))return false;
    if(fs::is_symlink(status)||!fs::is_regular_file(status))throw std::runtime_error("bonus-session-not-regular");
    return true;
}

uint64_t number(const nlohmann::json& j,const char* name,uint64_t limit){
 if(!j.contains(name)||!j[name].is_number_unsigned())throw std::runtime_error("bonus-session-number-invalid");
 const auto n=j[name].get<uint64_t>();if(n>limit)throw std::runtime_error("bonus-session-number-range");return n;
}
Snapshot read_disk(const fs::path& file,bool allow_missing,bool& present){
 present=regular(file);Snapshot result;
 if(!present){if(!allow_missing)throw std::runtime_error("bonus-session-disappeared");result.readable=result.last_write_succeeded=true;return result;}
 std::ifstream stream(file,std::ios::binary);if(!stream)throw std::runtime_error("bonus-session-open-failed");
 std::string bytes(max_file_bytes+1,'\0');stream.read(bytes.data(),std::streamsize(bytes.size()));bytes.resize(size_t(stream.gcount()));
 if(bytes.size()>max_file_bytes)throw std::runtime_error("bonus-session-too-large");
 if(!stream.eof()&&!stream)throw std::runtime_error("bonus-session-read-failed");
 bool duplicate=false;std::vector<std::set<std::string>> keys;
 auto callback=[&](int,nlohmann::json::parse_event_t event,nlohmann::json& value){
  if(event==nlohmann::json::parse_event_t::object_start)keys.emplace_back();
  else if(event==nlohmann::json::parse_event_t::key){if(keys.empty()||!keys.back().insert(value.get<std::string>()).second)duplicate=true;}
  else if(event==nlohmann::json::parse_event_t::object_end&&!keys.empty())keys.pop_back();return true;
 };
 const auto j=nlohmann::json::parse(bytes,callback);
 if(duplicate||!j.is_object()||j.size()!=4||number(j,"schema",1)!=1||!j.contains("attempts")||!j["attempts"].is_array()||j["attempts"].size()!=6)throw std::runtime_error("bonus-session-format-invalid");
 result.revision=number(j,"revision",UINT64_MAX);result.completed_mask=unsigned(number(j,"completedMask",63));
 for(unsigned slot=0;slot<6;++slot){
  const auto& row=j["attempts"][slot];if(row.is_null()){if(result.completed_mask&(1u<<slot))throw std::runtime_error("bonus-session-completion-without-attempt");continue;}
  if(!row.is_object()||row.size()!=8||!row.contains("order")||!row["order"].is_array()||row["order"].empty()||row["order"].size()>32)throw std::runtime_error("bonus-session-attempt-invalid");
  Attempt a;a.bonus=unsigned(number(row,"bonus",6));a.seed=number(row,"seed",UINT64_MAX);a.phase=Phase(number(row,"phase",3));
  a.cursor=unsigned(number(row,"cursor",31));a.visited_mask=unsigned(number(row,"visitedMask",UINT32_MAX));a.cleared_prefix=unsigned(number(row,"clearedPrefix",32));
  for(const auto& room:row["order"]){if(!room.is_number_unsigned()||room.get<uint64_t>()>176)throw std::runtime_error("bonus-session-room-invalid");a.order.push_back(room.get<unsigned>());}
  if(number(row,"routeSchema",bonus_routes0351::route_schema)!=bonus_routes0351::route_schema||a.bonus!=slot+1||!valid(a)||(a.phase==Phase::Completed&&!(result.completed_mask&(1u<<slot))))throw std::runtime_error("bonus-session-attempt-invalid");
  result.attempts[slot]=std::move(a);
 }
 result.readable=result.last_write_succeeded=true;return result;
}
std::string encode(const Snapshot& state){
 nlohmann::json rows=nlohmann::json::array();
 for(const auto& slot:state.attempts){
  if(!slot){rows.push_back(nullptr);continue;}const auto& a=*slot;
  rows.push_back({{"bonus",a.bonus},{"seed",a.seed},{"phase",unsigned(a.phase)},{"cursor",a.cursor},
   {"visitedMask",a.visited_mask},{"clearedPrefix",a.cleared_prefix},{"order",a.order},{"routeSchema",bonus_routes0351::route_schema}});
 }
 return nlohmann::json{{"schema",1u},{"revision",state.revision},{"completedMask",state.completed_mask},{"attempts",rows}}.dump(2)+'\n';
}
class FileLock {
#ifdef _WIN32
    HANDLE handle_=INVALID_HANDLE_VALUE;
#else
    int handle_=-1;
#endif
public:
    explicit FileLock(const fs::path& profile){
        const auto file=profile/"pc-bonus-session0353.lock";
        (void)regular(file);
#ifdef _WIN32
        handle_=CreateFileW(file.c_str(),GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(handle_==INVALID_HANDLE_VALUE)throw std::runtime_error("bonus-session-lock-unavailable");
#else
        handle_=::open(file.c_str(),O_RDWR|O_CREAT|O_CLOEXEC|O_NOFOLLOW,0600);
        if(handle_<0)throw std::runtime_error("bonus-session-lock-unavailable");
        if(::flock(handle_,LOCK_EX|LOCK_NB)!=0){::close(handle_);handle_=-1;throw std::runtime_error("bonus-session-lock-unavailable");}
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
        if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error("bonus-session-temp-create-failed");
        DWORD written=0;const bool good=WriteFile(handle,bytes.data(),DWORD(bytes.size()),&written,nullptr)&&written==bytes.size()&&FlushFileBuffers(handle);
        const bool closed=CloseHandle(handle)!=0;if(!good||!closed)throw std::runtime_error("bonus-session-temp-flush-failed");
        if(!MoveFileExW(temp.c_str(),file.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("bonus-session-replace-failed");
#else
        const int handle=::open(temp.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);
        if(handle<0)throw std::runtime_error("bonus-session-temp-create-failed");
        size_t offset=0;bool good=true;
        while(offset<bytes.size()){
            const auto n=::write(handle,bytes.data()+offset,bytes.size()-offset);
            if(n<0&&errno==EINTR)continue;if(n<=0){good=false;break;}offset+=size_t(n);
        }
        const bool flushed=good&&::fsync(handle)==0;const bool closed=::close(handle)==0;
        if(!flushed||!closed)throw std::runtime_error("bonus-session-temp-flush-failed");
        fs::rename(temp,file);
#endif
    }catch(...){std::error_code ignored;fs::remove(temp,ignored);throw;}
}

void failed_read(Snapshot& view,const std::exception& error){view.readable=view.last_write_succeeded=false;view.reason=error.what();}
} // namespace
Attempt make_ready(unsigned bonus,uint64_t seed){
 const auto route=bonus_routes0351::make_route(bonus,seed);
 if(!bonus_routes0351::validate(route)||route.steps.empty()||route.steps.size()>32)throw std::invalid_argument("Unsupported bonus route");
 Attempt a;a.bonus=bonus;a.seed=seed;for(const auto& step:route.steps)a.order.push_back(step.key.room);return a;
}
bool valid(const Attempt& a) noexcept {try{
 const auto ready=make_ready(a.bonus,a.seed);if(a.order!=ready.order||a.cursor>=a.order.size()||a.cleared_prefix>a.order.size())return false;
 const auto count=unsigned(a.order.size());
 if(a.phase==Phase::Ready)return a.cursor==0&&a.visited_mask==0&&a.cleared_prefix==0;
 if(a.phase==Phase::Completed)return a.cursor==count-1&&a.visited_mask==low_bits(count)&&a.cleared_prefix==count;
 if(a.phase!=Phase::Active&&a.phase!=Phase::Finale)return false;
 if(a.cleared_prefix>=count||a.cursor>a.cleared_prefix||a.visited_mask!=low_bits(a.cleared_prefix+1))return false;
 return a.phase==role_phase(bonus_routes0351::make_route(a.bonus,a.seed),a.cursor);
 }catch(...){return false;}}
Transition enter(Attempt& a,unsigned room){
 if(!valid(a)||a.phase==Phase::Completed)return Transition::InvalidState;
 if(a.order[a.cursor]!=room)return Transition::WrongRoom;
 if(a.phase!=Phase::Ready)return Transition::Unchanged;
 a.visited_mask=1;a.phase=role_phase(bonus_routes0351::make_route(a.bonus,a.seed),a.cursor);return Transition::Changed;
}
Transition advance(Attempt& a,unsigned room,bool qualified){
 if(!valid(a)||(a.phase!=Phase::Active&&a.phase!=Phase::Finale))return Transition::InvalidState;
 if(a.order[a.cursor]!=room)return Transition::WrongRoom;if(!qualified)return Transition::Unqualified;
 if(a.cursor==a.cleared_prefix)++a.cleared_prefix;
 const auto count=unsigned(a.order.size());
 if(a.cleared_prefix==count){a.phase=Phase::Completed;return Transition::Changed;}
 ++a.cursor;a.visited_mask|=1u<<a.cursor;a.phase=role_phase(bonus_routes0351::make_route(a.bonus,a.seed),a.cursor);return Transition::Changed;
}
Transition retry(Attempt& a,unsigned index){
 if(!valid(a)||a.phase==Phase::Ready||a.phase==Phase::Completed||index>=a.order.size()||!(a.visited_mask&(1u<<index)))return Transition::InvalidState;
 if(index==a.cursor)return Transition::Unchanged;
 a.cursor=index;a.phase=role_phase(bonus_routes0351::make_route(a.bonus,a.seed),index);return Transition::Changed;
}
Transition backtrack(Attempt& a,unsigned index){if(index>a.cursor)return Transition::InvalidState;return retry(a,index);}
bool Store::load(const fs::path& profile){
 std::lock_guard guard(transaction_mutex);profile_.clear();file_.clear();view_={};initialized_=present_=false;
 try{
  if(profile.empty())throw std::runtime_error("bonus-session-profile-missing");
  const auto absolute=fs::absolute(profile);
  if(!fs::is_directory(absolute)||fs::is_symlink(fs::symlink_status(absolute)))throw std::runtime_error("bonus-session-profile-missing");
  profile_=fs::canonical(absolute);file_=profile_/filename;
  view_=read_disk(file_,true,present_);initialized_=true;return true;
 }catch(const std::exception& error){failed_read(view_,error);return false;}
}
Snapshot Store::snapshot()const{std::lock_guard guard(transaction_mutex);return view_;}
bool Store::refresh(){
 std::lock_guard guard(transaction_mutex);if(!initialized_||!view_.readable)return false;
 try{bool present=false;view_=read_disk(file_,!present_,present);present_=present;return true;}
 catch(const std::exception& error){initialized_=false;failed_read(view_,error);return false;}
}
Reply Store::apply(uint64_t revision,const std::function<Status(Snapshot&)>& operation){
 std::lock_guard guard(transaction_mutex);auto reply=[&](Status s){return Reply{s,view_.revision};};
 if(!initialized_||!view_.readable)return reply(Status::Unavailable);
 try{
  FileLock lock(profile_);bool present=false;
  try{view_=read_disk(file_,!present_,present);present_=present;}
  catch(const std::exception& error){initialized_=false;failed_read(view_,error);return reply(Status::Unavailable);}
  if(view_.revision!=revision)return reply(Status::Conflict);
  Snapshot next=view_;const auto result=operation(next);if(result!=Status::Committed)return reply(result);
  if(next.revision==UINT64_MAX)return reply(Status::RevisionExhausted);
  ++next.revision;atomic_replace(file_,encode(next));present_=true;view_=std::move(next);return reply(Status::Committed);
 }catch(const std::exception& error){view_.last_write_succeeded=false;view_.reason=error.what();return reply(Status::PersistenceFailed);}
}
Reply Store::new_attempt(uint64_t revision,unsigned bonus,uint64_t seed,uint32_t classic,uint32_t hybrid){
 if(bonus<1||bonus>6||((classic|hybrid)&~bonus_routes0351::course_mask))return {Status::InvalidArgument,snapshot().revision};
 if(!bonus_routes0351::unlocked(bonus,classic,hybrid))return {Status::Locked,snapshot().revision};
 const auto a=make_ready(bonus,seed);return apply(revision,[&](Snapshot& next){next.attempts[bonus-1]=a;return Status::Committed;});
}
Reply Store::step(uint64_t revision,unsigned bonus,const std::function<Transition(Attempt&)>& action){
 if(bonus<1||bonus>6)return {Status::InvalidArgument,snapshot().revision};
 return apply(revision,[&](Snapshot& next){auto& slot=next.attempts[bonus-1];if(!slot)return Status::InvalidTransition;
  const auto result=action(*slot);if(result==Transition::Changed&&slot->phase==Phase::Completed)next.completed_mask|=1u<<(bonus-1);return status(result);});
}
Reply Store::enter(uint64_t r,unsigned b,unsigned room){return step(r,b,[&](Attempt& a){return bonus_session0351::enter(a,room);});}
Reply Store::advance(uint64_t r,unsigned b,unsigned room,bool qualified){return step(r,b,[&](Attempt& a){return bonus_session0351::advance(a,room,qualified);});}
Reply Store::backtrack(uint64_t r,unsigned b,unsigned index){return step(r,b,[&](Attempt& a){return bonus_session0351::backtrack(a,index);});}
Reply Store::retry(uint64_t r,unsigned b,unsigned index){return step(r,b,[&](Attempt& a){return bonus_session0351::retry(a,index);});}
} // namespace workshop::bonus_session0351
