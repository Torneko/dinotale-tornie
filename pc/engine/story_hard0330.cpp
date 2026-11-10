#include "story_hard0330.hpp"
#include "json/json.hpp"
#include <algorithm>
#include <atomic>
#include <bit>
#include <chrono>
#include <fstream>
#include <mutex>
#include <random>
#include <set>
#include <stdexcept>
#include <system_error>
#include <type_traits>
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
namespace workshop::story_hard0330 {
namespace {
namespace fs=std::filesystem;using Json=nlohmann::json;
inline constexpr uint64_t max_bytes=32768;
struct Data {
 bool preview=false;Run run;uint32_t starts=0,wins=0;unsigned completed=0;
 CompletionPolicy policy=CompletionPolicy::Undecided;
 std::array<uint32_t,16> color_wins{};
 bool operator==(const Data&)const=default;
};
std::atomic_uint64_t serial=0;
bool token_valid(const std::string& s){return s.size()==32&&std::all_of(s.begin(),s.end(),[](char c){return(c>='0'&&c<='9')||(c>='a'&&c<='f');});}
bool prefix(unsigned worlds){return worlds<=63&&(worlds&(worlds+1))==0;}
bool valid(const Data& d){
 const auto& r=d.run;
 if(d.starts>maximum_runs||d.wins>d.starts||d.completed>all_yoshis||r.serial>maximum_runs)return false;
 unsigned completed=0;for(unsigned c=0;c<16;++c){if(d.color_wins[c]>d.wins)return false;if(d.color_wins[c])completed|=1u<<c;}
 if(completed!=d.completed||(d.wins==0)!=(d.policy==CompletionPolicy::Undecided)||d.policy==CompletionPolicy::AllSixWorlds)return false;
 if(d.preview&&(d.starts||d.wins||d.completed||d.policy!=CompletionPolicy::Undecided))return false;
 if(!d.preview&&d.starts!=r.serial)return false;
 if(r.phase==Phase::Empty)return r==Run{}&&d.starts==0&&d.wins==0;
 if(!d.preview&&((r.phase==Phase::Won&&d.wins==0)||(r.phase!=Phase::Won&&d.wins>=d.starts)))return false;
 if(!token_valid(r.token)||!r.serial||r.phase>Phase::Abandoned||r.owned>all_yoshis||!(r.owned&1)||!(r.found&1)||(r.found&~r.owned)||(r.alive&~r.found)||(r.ever_lost&~r.found)||((r.found&~r.alive)&~r.ever_lost)||!prefix(r.worlds)||r.claimed_levels>0xFFFFFF)return false;
 if((r.phase==Phase::Active&&!r.alive)||(r.phase==Phase::Lost&&r.alive)||(r.phase==Phase::Won&&(r.worlds!=63||!r.alive)))return false;
 unsigned claimed_colors=1,claimed_worlds=0;
 for(unsigned level=0;level<24;++level){const auto color=r.egg_choice[level];if(color!=unassigned&&(color>=16||!(r.owned&(1u<<color))||color==0||level/4>unsigned(std::popcount(r.worlds))))return false;
  if(r.claimed_levels&(1u<<level)){if(color>=16||(claimed_colors&(1u<<color))||(claimed_worlds&(1u<<(level/4))))return false;claimed_colors|=1u<<color;claimed_worlds|=1u<<(level/4);}
 }
 if(claimed_colors!=r.found)return false;
 for(unsigned c=0;c<16;++c)if(r.yoshi_worlds[c]>63||(r.yoshi_worlds[c]&~r.worlds)||(!(r.found&(1u<<c))&&r.yoshi_worlds[c]))return false;
 for(unsigned w=0;w<6;++w){const auto level=r.world_levels[w];if(r.worlds&(1u<<w)){if(level>=24||level/4!=w)return false;}else if(level!=unassigned)return false;}
 if(!r.checkpoint_bound&&r.checkpoint!=Checkpoint{})return false;return true;
}
Json encode(const Data& d){const auto& r=d.run;return {{"schema",1u},{"mode",mode},{"preview",d.preview},{"runsStarted",d.starts},{"runsWon",d.wins},{"completedMask",d.completed},{"completionPolicy",unsigned(d.policy)},{"yoshiWins",d.color_wins},{"run",{{"token",r.token},{"serial",r.serial},{"phase",unsigned(r.phase)},{"owned",r.owned},{"found",r.found},{"alive",r.alive},{"everLost",r.ever_lost},{"worldMask",r.worlds},{"claimedLevels",r.claimed_levels},{"yoshiWorldMasks",r.yoshi_worlds},{"eggChoice",r.egg_choice},{"worldLevels",r.world_levels},{"checkpointBound",r.checkpoint_bound},{"checkpoint",r.checkpoint}}}};}
bool exists_regular(const fs::path& path){
 std::error_code error;const auto status=fs::symlink_status(path,error);
 if(error&&error!=std::errc::no_such_file_or_directory)throw std::runtime_error("story-hard-status-failed");
 if(!fs::exists(status))return false;
 if(!fs::is_regular_file(status)||fs::is_symlink(status))throw std::runtime_error("story-hard-not-regular");return true;
}
Data read_disk(const fs::path& file,Scope scope,bool allow_missing,bool& present){
 present=exists_regular(file);if(!present){if(!allow_missing)throw std::runtime_error("story-hard-disappeared");Data empty;empty.preview=scope==Scope::Preview;return empty;}
 if(fs::file_size(file)>max_bytes)throw std::runtime_error("story-hard-too-large");
 std::ifstream input(file,std::ios::binary);if(!input)throw std::runtime_error("story-hard-open-failed");std::string bytes(size_t(max_bytes)+1,'\0');input.read(bytes.data(),std::streamsize(bytes.size()));bytes.resize(size_t(input.gcount()));if(bytes.size()>max_bytes||(!input.eof()&&!input))throw std::runtime_error("story-hard-read-failed");
 bool duplicate=false;std::vector<std::set<std::string>> keys;
 auto callback=[&](int,Json::parse_event_t event,Json& parsed){if(event==Json::parse_event_t::object_start)keys.emplace_back();else if(event==Json::parse_event_t::key){if(keys.empty()||!keys.back().insert(parsed.get<std::string>()).second)duplicate=true;}else if(event==Json::parse_event_t::object_end&&!keys.empty())keys.pop_back();return true;};
 const auto j=Json::parse(bytes,callback);
 auto number=[](const Json& object,const char* name,uint64_t maximum)->uint64_t{if(!object.contains(name)||!object[name].is_number_unsigned()||object[name].get<uint64_t>()>maximum)throw std::runtime_error("story-hard-number-invalid");return object[name].get<uint64_t>();};
 auto array=[](const Json& object,const char* name,auto& target,unsigned maximum){if(!object.contains(name)||!object[name].is_array()||object[name].size()!=target.size())throw std::runtime_error("story-hard-array-invalid");for(size_t i=0;i<target.size();++i){const auto& n=object[name][i];if(!n.is_number_unsigned()||n.get<uint64_t>()>maximum)throw std::runtime_error("story-hard-array-value-invalid");target[i]=typename std::remove_reference_t<decltype(target)>::value_type(n.get<unsigned>());}};
 if(duplicate||!j.is_object()||j.size()!=9||number(j,"schema",1)!=1||number(j,"mode",mode)!=mode||!j.contains("preview")||!j["preview"].is_boolean()||j["preview"].get<bool>()!=(scope==Scope::Preview)||!j.contains("run")||!j["run"].is_object()||j["run"].size()!=14)throw std::runtime_error("story-hard-format-invalid");
 Data d;d.preview=j["preview"].get<bool>();d.starts=uint32_t(number(j,"runsStarted",maximum_runs));d.wins=uint32_t(number(j,"runsWon",maximum_runs));d.completed=unsigned(number(j,"completedMask",all_yoshis));d.policy=CompletionPolicy(number(j,"completionPolicy",2));array(j,"yoshiWins",d.color_wins,maximum_runs);
 const auto& x=j["run"];auto& r=d.run;if(!x.contains("token")||!x["token"].is_string()||!x.contains("checkpointBound")||!x["checkpointBound"].is_boolean())throw std::runtime_error("story-hard-run-format-invalid");r.token=x["token"].get<std::string>();r.serial=number(x,"serial",maximum_runs);r.phase=Phase(number(x,"phase",unsigned(Phase::Abandoned)));r.owned=unsigned(number(x,"owned",all_yoshis));r.found=unsigned(number(x,"found",all_yoshis));r.alive=unsigned(number(x,"alive",all_yoshis));r.ever_lost=unsigned(number(x,"everLost",all_yoshis));r.worlds=unsigned(number(x,"worldMask",63));r.claimed_levels=unsigned(number(x,"claimedLevels",0xFFFFFF));array(x,"yoshiWorldMasks",r.yoshi_worlds,63);array(x,"eggChoice",r.egg_choice,unassigned);array(x,"worldLevels",r.world_levels,unassigned);r.checkpoint_bound=x["checkpointBound"].get<bool>();array(x,"checkpoint",r.checkpoint,255);
 if(!valid(d))throw std::runtime_error("story-hard-state-invalid");return d;
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
  handle=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error("story-hard-lock-unavailable");
#else
  handle=::open(path.c_str(),O_RDWR|O_CREAT|O_CLOEXEC|O_NOFOLLOW,0600);if(handle<0)throw std::runtime_error("story-hard-lock-unavailable");if(::flock(handle,LOCK_EX|LOCK_NB)!=0){::close(handle);handle=-1;throw std::runtime_error("story-hard-lock-unavailable");}
#endif
 }
 ~FileLock(){
#ifdef _WIN32
  if(handle!=INVALID_HANDLE_VALUE)CloseHandle(handle);
#else
  if(handle>=0){::flock(handle,LOCK_UN);::close(handle);}
#endif
 }
 FileLock(const FileLock&)=delete;FileLock&operator=(const FileLock&)=delete;
};
void persist(const fs::path& path,const Data& d){
 const auto bytes=encode(d).dump(2)+'\n';
 auto temp=path;temp+=".tmp-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(serial.fetch_add(1));bool created=false;
 try{
#ifdef _WIN32
  HANDLE file=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(file==INVALID_HANDLE_VALUE)throw std::runtime_error("story-hard-temp-create-failed");created=true;
  DWORD count=0;const bool flushed=WriteFile(file,bytes.data(),DWORD(bytes.size()),&count,nullptr)&&count==bytes.size()&&FlushFileBuffers(file);const bool closed=CloseHandle(file)!=0;if(!flushed||!closed)throw std::runtime_error("story-hard-temp-flush-failed");
  if(!MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("story-hard-publish-failed");
#else
  const int file=::open(temp.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(file<0)throw std::runtime_error("story-hard-temp-create-failed");created=true;
  size_t at=0;bool good=true;while(at<bytes.size()){const auto count=::write(file,bytes.data()+at,bytes.size()-at);if(count<0&&errno==EINTR)continue;if(count<=0){good=false;break;}at+=size_t(count);}const bool flushed=good&&::fsync(file)==0,closed=::close(file)==0;if(!flushed||!closed)throw std::runtime_error("story-hard-temp-flush-failed");fs::rename(temp,path);
#endif
 }catch(...){if(created){std::error_code ignored;fs::remove(temp,ignored);}throw;}
}
unsigned random_index(unsigned count){std::random_device random;std::mt19937 engine(random());return std::uniform_int_distribution<unsigned>(0,count-1)(engine);}
}
struct Journal::Impl {
 mutable std::mutex mutex;fs::path file;Scope scope=Scope::Production;Data data;View current;unsigned owned=0;std::string bound_token;std::function<unsigned(unsigned)> choose;
 explicit Impl(std::function<unsigned(unsigned)> choice):choose(choice?std::move(choice):random_index){}
 void publish(){current.scope=scope;current.run=data.run;current.runs_started=data.starts;current.runs_won=data.wins;current.completed=data.completed;current.completion_policy=data.policy;current.yoshi_wins=data.color_wins;current.solo_boss_route_entitled=current.readable&&current.saved&&!data.preview&&data.wins>0;current.bound=current.active&&current.readable&&current.saved&&!bound_token.empty()&&bound_token==data.run.token&&data.run.phase==Phase::Active;}
 bool matches(const Data& d,const std::string& token,bool active_run=true)const{return current.active&&current.readable&&token_valid(token)&&token==bound_token&&token==d.run.token&&(!active_run||d.run.phase==Phase::Active);}
 template<class Change>bool update(Change&& change,bool create=false){
  if(!current.initialized||!current.readable)return false;
  try{
   if(create)fs::create_directories(file.parent_path());FileLock lock(file);bool present=false;Data before;
   try{before=read_disk(file,scope,!current.present,present);}catch(...){current.readable=false;throw;}
   data=before;current.present=present;publish();if(!present&&!create)return false;
   auto next=before;if(!change(next))return false;if(!valid(next))throw std::runtime_error("story-hard-update-invalid");
   if(!present||next!=before)persist(file,next);data=std::move(next);current.present=true;current.saved=true;current.reason.clear();publish();return true;
  }catch(const std::exception& error){current.saved=false;current.reason=error.what();publish();return false;}
 }
};
Journal::Journal(std::function<unsigned(unsigned)> choose):impl_(std::make_unique<Impl>(std::move(choose))){}Journal::~Journal()=default;
void Journal::initialize(const fs::path& path,Scope scope){std::lock_guard lock(impl_->mutex);impl_->current={};impl_->current.initialized=true;impl_->scope=scope;impl_->data={};impl_->data.preview=scope==Scope::Preview;impl_->bound_token.clear();impl_->owned=0;impl_->file.clear();try{
 const auto expected=scope==Scope::Preview?preview_filename:production_filename;if(path.empty()||path.filename()!=expected)throw std::runtime_error("story-hard-path-scope-invalid");impl_->file=fs::absolute(path);bool found=false;impl_->data=read_disk(impl_->file,scope,true,found);impl_->current.present=found;impl_->publish();
}catch(const std::exception& error){impl_->current.readable=impl_->current.saved=false;impl_->current.reason=error.what();impl_->publish();}}
View Journal::view()const{std::lock_guard lock(impl_->mutex);return impl_->current;}
bool Journal::prepare(unsigned selected,const story_hard_access0330::Evidence& access,bool verified){std::lock_guard lock(impl_->mutex);impl_->current.active=false;impl_->bound_token.clear();impl_->publish();if(selected!=mode||access.owned16>all_yoshis||!(access.owned16&1))return false;
 const bool allowed=impl_->scope==Scope::Preview?(verified&&access.readable&&access.saved):story_hard_access0330::entitled(access);if(!allowed)return false;impl_->owned=access.owned16;const bool ok=impl_->update([](Data&){return true;},true);impl_->current.active=ok;impl_->publish();return ok;}
bool Journal::new_run(const std::string& token,bool trusted){std::lock_guard lock(impl_->mutex);if(!trusted||!impl_->current.active||!token_valid(token))return false;
 const bool ok=impl_->update([&](Data& d){if(d.run.serial>=maximum_runs||d.run.token==token)return false;Run fresh;fresh.serial=d.run.serial+1;fresh.token=token;fresh.phase=Phase::Active;fresh.owned=impl_->owned;fresh.found=fresh.alive=1;d.run=std::move(fresh);if(!d.preview)++d.starts;return true;});if(ok)impl_->bound_token=token;impl_->publish();return ok;}
bool Journal::bind_checkpoint(const std::string& token,std::span<const uint8_t> bytes,bool trusted){std::lock_guard lock(impl_->mutex);if(!trusted||bytes.size()!=14)return false;return impl_->update([&](Data& d){if(!impl_->matches(d,token,false)||d.run.phase==Phase::Empty||d.run.phase==Phase::Abandoned)return false;std::copy(bytes.begin(),bytes.end(),d.run.checkpoint.begin());d.run.checkpoint_bound=true;return true;});}
bool Journal::resume_run(const std::string& token,std::span<const uint8_t> bytes,bool continuing){std::lock_guard lock(impl_->mutex);impl_->bound_token.clear();impl_->publish();if(!continuing||!impl_->current.active||!token_valid(token)||bytes.size()!=14)return false;
 const bool ok=impl_->update([&](Data& d){return d.run.phase==Phase::Active&&d.run.token==token&&d.run.checkpoint_bound&&std::equal(bytes.begin(),bytes.end(),d.run.checkpoint.begin())&&!(d.run.owned&~impl_->owned);});if(ok)impl_->bound_token=token;impl_->publish();return ok;}
std::optional<unsigned> Journal::egg_for(unsigned level,const std::string& token,bool trusted){std::lock_guard lock(impl_->mutex);if(!trusted||level>=24)return {};std::optional<unsigned> chosen;
 if(!impl_->update([&](Data& d){if(!impl_->matches(d,token)||level/4!=unsigned(std::popcount(d.run.worlds)))return false;for(unsigned i=(level/4)*4;i<(level/4)*4+4;++i)if(d.run.claimed_levels&(1u<<i))return false;
  auto& color=d.run.egg_choice[level];if(color==unassigned){const unsigned pool=d.run.owned&~d.run.found;std::array<unsigned,16> colors{};unsigned count=0;for(unsigned c=0;c<16;++c)if(pool&(1u<<c))colors[count++]=c;if(!count)return false;const auto index=impl_->choose(count);if(index>=count)throw std::runtime_error("story-hard-random-index-invalid");color=colors[index];}
  if(d.run.found&(1u<<color))return false;chosen=color;return true;})){return {};}
 return chosen;}
bool Journal::found_egg(unsigned level,unsigned color,const std::string& token,bool trusted){std::lock_guard lock(impl_->mutex);if(!trusted||level>=24||color>=16)return false;return impl_->update([&](Data& d){if(!impl_->matches(d,token,false)||d.run.egg_choice[level]!=color)return false;if(d.run.claimed_levels&(1u<<level))return true;if(d.run.phase!=Phase::Active||level/4!=unsigned(std::popcount(d.run.worlds)))return false;for(unsigned i=(level/4)*4;i<(level/4)*4+4;++i)if(d.run.claimed_levels&(1u<<i))return false;if(d.run.found&(1u<<color))return false;d.run.claimed_levels|=1u<<level;d.run.found|=1u<<color;d.run.alive|=1u<<color;return true;});}
bool Journal::lose_yoshi(unsigned color,const std::string& token,bool trusted){std::lock_guard lock(impl_->mutex);if(!trusted||color>=16)return false;return impl_->update([&](Data& d){if(!impl_->matches(d,token,false)||!(d.run.found&(1u<<color)))return false;if(!(d.run.alive&(1u<<color)))return (d.run.ever_lost&(1u<<color))&&(d.run.phase==Phase::Active||d.run.phase==Phase::Lost);if(d.run.phase!=Phase::Active)return false;d.run.alive&=~(1u<<color);d.run.ever_lost|=1u<<color;if(!d.run.alive)d.run.phase=Phase::Lost;return true;});}
bool Journal::restore_yoshi(unsigned color,const std::string& token,bool trusted){std::lock_guard lock(impl_->mutex);if(!trusted||color>=16)return false;return impl_->update([&](Data& d){if(!impl_->matches(d,token)||!(d.run.found&(1u<<color)))return false;d.run.alive|=1u<<color;return true;});}
bool Journal::world_finished(unsigned level,const std::string& token,bool trusted){std::lock_guard lock(impl_->mutex);if(!trusted||level>=24)return false;return impl_->update([&](Data& d){if(!impl_->matches(d,token,false))return false;const unsigned w=level/4,bit=1u<<w;if(d.run.worlds&bit)return d.run.world_levels[w]==level;if(d.run.phase!=Phase::Active||w!=unsigned(std::popcount(d.run.worlds)))return false;d.run.worlds|=bit;d.run.world_levels[w]=level;for(unsigned c=0;c<16;++c)if(d.run.alive&(1u<<c))d.run.yoshi_worlds[c]|=bit;return true;});}
bool Journal::complete_run(const std::string& token,CompletionPolicy policy,bool trusted){std::lock_guard lock(impl_->mutex);if(!trusted||policy!=CompletionPolicy::SurvivingRecruits)return false;return impl_->update([&](Data& d){if(!impl_->matches(d,token,false))return false;if(d.run.phase==Phase::Won)return true;if(d.run.phase!=Phase::Active||d.run.worlds!=63)return false;d.run.phase=Phase::Won;if(!d.preview){if(d.wins>=maximum_runs)return false;++d.wins;d.policy=CompletionPolicy::SurvivingRecruits;const unsigned qualified=d.run.alive&~d.run.ever_lost;for(unsigned c=0;c<16;++c)if(qualified&(1u<<c)){++d.color_wins[c];d.completed|=1u<<c;}}return true;});}
bool Journal::abandon_run(const std::string& token,bool trusted){std::lock_guard lock(impl_->mutex);if(!trusted)return false;return impl_->update([&](Data& d){if(!impl_->matches(d,token,false)||d.run.phase==Phase::Empty||d.run.phase==Phase::Won)return false;d.run.phase=Phase::Abandoned;return true;});}
void Journal::deactivate(){std::lock_guard lock(impl_->mutex);impl_->current.active=false;impl_->bound_token.clear();impl_->publish();}
namespace {Journal& journal(){static Journal instance;return instance;}}
void initialize(const fs::path& path,Scope s){journal().initialize(path,s);}View view(){return journal().view();}bool prepare(unsigned m,const story_hard_access0330::Evidence& e,bool v){return journal().prepare(m,e,v);}bool new_run(const std::string& t,bool v){return journal().new_run(t,v);}
bool bind_checkpoint(const std::string& t,std::span<const uint8_t> b,bool v){return journal().bind_checkpoint(t,b,v);}bool resume_run(const std::string& t,std::span<const uint8_t> b,bool v){return journal().resume_run(t,b,v);}std::optional<unsigned> egg_for(unsigned l,const std::string& t,bool v){return journal().egg_for(l,t,v);}bool found_egg(unsigned l,unsigned c,const std::string& t,bool v){return journal().found_egg(l,c,t,v);}bool lose_yoshi(unsigned c,const std::string& t,bool v){return journal().lose_yoshi(c,t,v);}bool restore_yoshi(unsigned c,const std::string& t,bool v){return journal().restore_yoshi(c,t,v);}bool world_finished(unsigned l,const std::string& t,bool v){return journal().world_finished(l,t,v);}bool complete_run(const std::string& t,CompletionPolicy p,bool v){return journal().complete_run(t,p,v);}bool abandon_run(const std::string& t,bool v){return journal().abandon_run(t,v);}void deactivate(){journal().deactivate();}
} // namespace workshop::story_hard0330
