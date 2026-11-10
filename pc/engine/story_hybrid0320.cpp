#include "story_hybrid0320.hpp"
#include "json/json.hpp"
#include <algorithm>
#include <atomic>
#include <bit>
#include <chrono>
#include <fstream>
#include <mutex>
#include <numeric>
#include <random>
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
namespace workshop::story_hybrid0320 {
namespace {
namespace fs=std::filesystem;
struct Data {unsigned worlds=0,acquired=0,metal_owned=0,metal_alive=0;Order order{};bool operator==(const Data&)const=default;};
std::atomic_uint64_t serial=0;
bool valid_order(const Order& order){unsigned seen=0;for(auto color:order){if(color>=6||(seen&(1u<<color)))return false;seen|=1u<<color;}return seen==63;}
unsigned prefix(const Data& d){unsigned result=d.acquired;for(unsigned i=0;i<unsigned(std::popcount(d.worlds));++i)result|=1u<<d.order[i];return result;}
bool exists_regular(const fs::path& path){
 std::error_code error;const auto status=fs::symlink_status(path,error);
 if(error&&error!=std::errc::no_such_file_or_directory)throw std::runtime_error("story-hybrid-status-failed");
 if(!fs::exists(status))return false;
 if(!fs::is_regular_file(status)||fs::is_symlink(status))throw std::runtime_error("story-hybrid-not-regular");return true;
}
Data read_disk(const fs::path& path,bool allow_missing,bool& found){
 found=exists_regular(path);if(!found){if(!allow_missing)throw std::runtime_error("story-hybrid-disappeared");return {};}
 if(fs::file_size(path)>4096)throw std::runtime_error("story-hybrid-too-large");
 std::ifstream input(path,std::ios::binary);if(!input)throw std::runtime_error("story-hybrid-open-failed");
 std::string bytes(4097,'\0');input.read(bytes.data(),std::streamsize(bytes.size()));bytes.resize(size_t(input.gcount()));if(bytes.size()>4096||(!input.eof()&&!input))throw std::runtime_error("story-hybrid-read-failed");
 bool duplicate=false;std::vector<std::set<std::string>> keys;
 auto callback=[&](int,nlohmann::json::parse_event_t event,nlohmann::json& parsed){if(event==nlohmann::json::parse_event_t::object_start)keys.emplace_back();else if(event==nlohmann::json::parse_event_t::key){if(keys.empty()||!keys.back().insert(parsed.get<std::string>()).second)duplicate=true;}else if(event==nlohmann::json::parse_event_t::object_end&&!keys.empty())keys.pop_back();return true;};
 const auto json=nlohmann::json::parse(bytes,callback);
 auto natural=[&](const char* key,unsigned maximum){return json.contains(key)&&json[key].is_number_unsigned()&&json[key].get<uint64_t>()<=maximum;};
 if(duplicate||!json.is_object()||json.size()!=7||!natural("schema",1)||json["schema"]!=1u||!natural("mode",mode)||json["mode"]!=mode||!natural("completedWorldMask",63)||!natural("acquiredMask",63)||!natural("metalOwnedMask",3)||!natural("metalAliveMask",3)||!json.contains("hybridOrder")||!json["hybridOrder"].is_array()||json["hybridOrder"].size()!=6)throw std::runtime_error("story-hybrid-format-invalid");
 Data result{json["completedWorldMask"].get<unsigned>(),json["acquiredMask"].get<unsigned>(),json["metalOwnedMask"].get<unsigned>(),json["metalAliveMask"].get<unsigned>(),{}};
 for(unsigned i=0;i<6;++i){const auto& c=json["hybridOrder"][i];if(!c.is_number_unsigned()||c.get<uint64_t>()>=6)throw std::runtime_error("story-hybrid-order-invalid");result.order[i]=c.get<unsigned>();}
 if(!valid_order(result.order)||(result.metal_alive&~result.metal_owned))throw std::runtime_error("story-hybrid-state-invalid");return result;
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
  handle=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error("story-hybrid-lock-unavailable");
#else
  handle=::open(path.c_str(),O_RDWR|O_CREAT|O_CLOEXEC|O_NOFOLLOW,0600);if(handle<0)throw std::runtime_error("story-hybrid-lock-unavailable");if(::flock(handle,LOCK_EX|LOCK_NB)!=0){::close(handle);handle=-1;throw std::runtime_error("story-hybrid-lock-unavailable");}
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
 const auto bytes=nlohmann::json{{"schema",1u},{"mode",mode},{"completedWorldMask",d.worlds},{"hybridOrder",d.order},{"acquiredMask",d.acquired},{"metalOwnedMask",d.metal_owned},{"metalAliveMask",d.metal_alive}}.dump(2)+'\n';
 auto temp=path;temp+=".tmp-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(serial.fetch_add(1));bool created=false;
 try{
#ifdef _WIN32
  HANDLE file=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(file==INVALID_HANDLE_VALUE)throw std::runtime_error("story-hybrid-temp-create-failed");created=true;
  DWORD count=0;const bool flushed=WriteFile(file,bytes.data(),DWORD(bytes.size()),&count,nullptr)&&count==bytes.size()&&FlushFileBuffers(file);const bool closed=CloseHandle(file)!=0;if(!flushed||!closed)throw std::runtime_error("story-hybrid-temp-flush-failed");
  if(!MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("story-hybrid-publish-failed");
#else
  const int file=::open(temp.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(file<0)throw std::runtime_error("story-hybrid-temp-create-failed");created=true;
  size_t at=0;bool good=true;while(at<bytes.size()){const auto count=::write(file,bytes.data()+at,bytes.size()-at);if(count<0&&errno==EINTR)continue;if(count<=0){good=false;break;}at+=size_t(count);}const bool flushed=good&&::fsync(file)==0,closed=::close(file)==0;if(!flushed||!closed)throw std::runtime_error("story-hybrid-temp-flush-failed");fs::rename(temp,path);
#endif
 }catch(...){if(created){std::error_code ignored;fs::remove(temp,ignored);}throw;}
}
Order shuffled(){Order order{0,1,2,3,4,5};std::mt19937 random(std::random_device{}());std::shuffle(order.begin(),order.end(),random);return order;}
}
struct Journal::Impl {
 mutable std::mutex mutex;fs::path file;Data data;View current;std::function<Order()> choose;
 explicit Impl(std::function<Order()> generator):choose(generator?std::move(generator):shuffled){}
 void publish(){current.worlds=data.worlds;current.count=unsigned(std::popcount(data.worlds));current.order=data.order;current.acquired=data.acquired;current.metal_owned=data.metal_owned;current.metal_alive=data.metal_alive;current.eligible=current.initialized&&current.present&&current.readable&&current.saved?prefix(data):0;}
 enum class Action {Prepare,World,Acquired,Lives,NewRun};
 bool update(Action action,unsigned a=0,unsigned b=0){
  if(!current.initialized||!current.readable)return false;
  try{
   if(action==Action::Prepare)fs::create_directories(file.parent_path());
   FileLock guard(file);bool found=false;Data before;
   try{before=read_disk(file,!current.present,found);}catch(...){current.readable=false;throw;}
   if(!found&&action!=Action::Prepare)return action==Action::Acquired;
   data=before;current.present=found;publish();auto next=before;
   if(!found){next.order=choose();if(!valid_order(next.order))throw std::runtime_error("story-hybrid-random-order-invalid");}
   if(action==Action::Prepare){next.acquired|=a;const unsigned newly_owned=b&~next.metal_owned;next.metal_owned|=b;next.metal_alive|=newly_owned;}
   else if(action==Action::World)next.worlds|=1u<<a;
   else if(action==Action::Acquired)next.acquired|=a;
   else if(action==Action::Lives){if(a&~before.metal_owned)return false;next.metal_alive=a;}
   else {next.metal_owned|=a;next.metal_alive=next.metal_owned;}
   if(!found||next!=before)persist(file,next);
   data=next;current.present=true;current.saved=true;current.reason.clear();publish();return true;
  }catch(const std::exception& error){current.saved=false;current.reason=error.what();publish();return false;}
 }
};
Journal::Journal(std::function<Order()> factory):impl_(std::make_unique<Impl>(std::move(factory))){}
Journal::~Journal()=default;
void Journal::initialize(const fs::path& path){std::lock_guard lock(impl_->mutex);impl_->data={};impl_->current={};impl_->current.initialized=true;impl_->file.clear();try{
 if(path.empty()||path.filename()!=filename)throw std::runtime_error("story-hybrid-path-invalid");impl_->file=fs::absolute(path);bool found=false;impl_->data=read_disk(impl_->file,true,found);impl_->current.present=found;impl_->publish();
}catch(const std::exception& error){impl_->current.readable=impl_->current.saved=false;impl_->current.reason=error.what();impl_->publish();}}
View Journal::view()const{std::lock_guard lock(impl_->mutex);return impl_->current;}
bool Journal::prepare(unsigned selected,unsigned acquired,unsigned metals){std::lock_guard lock(impl_->mutex);impl_->current.active=false;if(selected!=mode||acquired>63||metals>3)return false;const bool ok=impl_->update(Impl::Action::Prepare,acquired,metals);impl_->current.active=ok;return ok;}
bool Journal::successful_world(unsigned page,bool trusted){std::lock_guard lock(impl_->mutex);return page<6&&trusted&&impl_->current.active&&impl_->update(Impl::Action::World,page);}
bool Journal::note_acquired(unsigned mask){std::lock_guard lock(impl_->mutex);if(mask>63||!impl_->current.initialized||!impl_->current.readable)return false;if(!impl_->current.present){try{if(!exists_regular(impl_->file))return true;}catch(const std::exception& error){impl_->current.readable=impl_->current.saved=false;impl_->current.reason=error.what();impl_->publish();return false;}}return impl_->update(Impl::Action::Acquired,mask);}
bool Journal::save_lives(unsigned bits){std::lock_guard lock(impl_->mutex);return bits<=3&&impl_->current.active&&impl_->update(Impl::Action::Lives,bits);}
bool Journal::new_run(unsigned metals){std::lock_guard lock(impl_->mutex);return metals<=3&&impl_->current.active&&impl_->update(Impl::Action::NewRun,metals);}
void Journal::deactivate(){std::lock_guard lock(impl_->mutex);impl_->current.active=false;}
namespace {Journal& journal(){static Journal instance;return instance;}}
void initialize(const fs::path& path){journal().initialize(path);}View view(){return journal().view();}
bool prepare(unsigned selected,unsigned acquired,unsigned metals){return journal().prepare(selected,acquired,metals);}bool successful_world(unsigned page,bool trusted){return journal().successful_world(page,trusted);}
bool note_acquired(unsigned mask){return journal().note_acquired(mask);}bool save_lives(unsigned bits){return journal().save_lives(bits);}bool new_run(unsigned metals){return journal().new_run(metals);}bool reset_run(unsigned metals){return new_run(metals);}void deactivate(){journal().deactivate();}
} // namespace workshop::story_hybrid0320
