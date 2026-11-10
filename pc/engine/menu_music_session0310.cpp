#include "menu_music_session0310.hpp"
#include "menu_music_native0310.hpp"
#include "json/json.hpp"
#include <array>
#include <atomic>
#include <condition_variable>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
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
#include <csignal>
#include <fcntl.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
extern char** environ;
#endif
namespace workshop::menu_music_session0310 {
namespace fs=std::filesystem;
namespace {
constexpr auto original_rom=dinotale::menu_music_native0310::original_usa_rom_sha256;
std::atomic_uint64_t directory_serial=0;
bool digest(const std::string& value){return value.size()==64&&std::all_of(value.begin(),value.end(),[](char c){return(c>='0'&&c<='9')||(c>='a'&&c<='f');});}
std::string read_text(const fs::path& path,uint64_t limit){if(!fs::is_regular_file(fs::symlink_status(path))||fs::file_size(path)>limit)return {};std::ifstream f(path,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
struct Directory {
 fs::path path;
 explicit Directory(const fs::path& root){
  std::error_code error;fs::create_directories(root,error);if(error)return;
  for(unsigned i=0;i<8;++i){auto candidate=root/("job-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(directory_serial.fetch_add(1)));if(fs::create_directory(candidate,error)){path=std::move(candidate);break;}if(error&&error!=std::errc::file_exists)break;}
 }
 ~Directory(){if(path.empty())return;std::error_code ignored;for(const char* name:{"identity.log","render.log","music.wav","music.wav.json"})fs::remove(path/name,ignored);fs::remove(path,ignored);}
};
#ifdef _WIN32
struct Handle {HANDLE value=nullptr;explicit Handle(HANDLE h=nullptr):value(h){}~Handle(){if(value&&value!=INVALID_HANDLE_VALUE)CloseHandle(value);}Handle(const Handle&)=delete;Handle&operator=(const Handle&)=delete;bool valid()const{return value&&value!=INVALID_HANDLE_VALUE;}};
std::wstring quote(const fs::path& value){
 const auto text=value.wstring();std::wstring result=L"\"";size_t slashes=0;
 for(wchar_t c:text){if(c==L'\\'){++slashes;continue;}if(c==L'\"'){result.append(slashes*2+1,L'\\');result+=c;slashes=0;continue;}result.append(slashes,L'\\');slashes=0;result+=c;}
 result.append(slashes*2,L'\\');result+=L'\"';return result;
}
#endif
}
namespace detail {
std::string sha256(std::span<const uint8_t> input) {
 const uint32_t k[64]={0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
 auto r=[](uint32_t x,unsigned n){return (x>>n)|(x<<(32-n));};
 uint32_t h[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
 std::vector<uint8_t> data(input.begin(),input.end());uint64_t bits=uint64_t(data.size())*8;data.push_back(0x80);while(data.size()%64!=56)data.push_back(0);for(int i=7;i>=0;i--)data.push_back(uint8_t(bits>>(8*i)));
 for(size_t p=0;p<data.size();p+=64){uint32_t w[64];for(unsigned i=0;i<16;i++)w[i]=(uint32_t(data[p+i*4])<<24)|(uint32_t(data[p+i*4+1])<<16)|(uint32_t(data[p+i*4+2])<<8)|data[p+i*4+3];for(unsigned i=16;i<64;i++){auto s0=r(w[i-15],7)^r(w[i-15],18)^(w[i-15]>>3);auto s1=r(w[i-2],17)^r(w[i-2],19)^(w[i-2]>>10);w[i]=w[i-16]+s0+w[i-7]+s1;}uint32_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],v=h[7];for(unsigned i=0;i<64;i++){auto s1=r(e,6)^r(e,11)^r(e,25),ch=(e&f)^(~e&g),t1=v+s1+ch+k[i]+w[i],s0=r(a,2)^r(a,13)^r(a,22),maj=(a&b)^(a&c)^(b&c),t2=s0+maj;v=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;}h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=v;}
 std::ostringstream out;out<<std::hex<<std::setfill('0');for(auto x:h)out<<std::setw(8)<<x;return out.str();
}
std::string normalized_rom_sha256(const fs::path& path,std::stop_token cancel){
 if(cancel.stop_requested())return {};const auto status=fs::symlink_status(path);if(!fs::is_regular_file(status)||fs::is_symlink(status)||fs::file_size(path)!=0x1000000)return {};
 std::ifstream file(path,std::ios::binary);std::vector<uint8_t> data(0x1000000);
 for(size_t at=0;at<data.size();at+=65536){if(cancel.stop_requested())return {};file.read(reinterpret_cast<char*>(data.data()+at),65536);if(!file)return {};}
 if(data[0]==0x37&&data[1]==0x80&&data[2]==0x40&&data[3]==0x12){for(size_t i=0;i<data.size();i+=2)std::swap(data[i],data[i+1]);}
 else if(data[0]==0x40&&data[1]==0x12&&data[2]==0x37&&data[3]==0x80){for(size_t i=0;i<data.size();i+=4){std::swap(data[i],data[i+3]);std::swap(data[i+1],data[i+2]);}}
 if(data[0]!=0x80||data[1]!=0x37||data[2]!=0x12||data[3]!=0x40||cancel.stop_requested())return {};
 return sha256(data);
}
ProcessResult run_process(const fs::path& executable,const std::vector<fs::path>& arguments,const fs::path& output_log,std::stop_token cancel,std::chrono::milliseconds timeout) noexcept {
 try{
  if(cancel.stop_requested())return {false,true,false,-1,"cancelled"};
  if(!executable.is_absolute()||!output_log.is_absolute()||timeout.count()<=0||timeout>std::chrono::minutes(10)||arguments.size()>8||!fs::is_regular_file(executable))return {false,false,false,-1,"process-input-invalid"};
  for(const auto& argument:arguments)if(argument.native().size()>32768||argument.native().find(typename fs::path::value_type(0))!=fs::path::string_type::npos)return {false,false,false,-1,"process-argument-invalid"};
  const auto deadline=std::chrono::steady_clock::now()+timeout;
#ifdef _WIN32
  SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES),nullptr,TRUE};
  Handle log(CreateFileW(output_log.c_str(),GENERIC_WRITE,FILE_SHARE_READ,&security,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr));
  Handle input(CreateFileW(L"NUL",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,&security,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr));
  Handle job(CreateJobObjectW(nullptr,nullptr));if(!log.valid()||!input.valid()||!job.valid())return {false,false,false,-1,"process-handles-failed"};
  JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
  if(!SetInformationJobObject(job.value,JobObjectExtendedLimitInformation,&limits,sizeof(limits)))return {false,false,false,-1,"process-job-failed"};
  SIZE_T attribute_bytes=0;InitializeProcThreadAttributeList(nullptr,1,0,&attribute_bytes);std::vector<uint8_t> attributes(attribute_bytes);
  auto* attribute_list=reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attributes.data());
  if(!InitializeProcThreadAttributeList(attribute_list,1,0,&attribute_bytes))return {false,false,false,-1,"process-attributes-failed"};
  struct AttributeCleanup {LPPROC_THREAD_ATTRIBUTE_LIST value;~AttributeCleanup(){DeleteProcThreadAttributeList(value);}} cleanup{attribute_list};
  std::array<HANDLE,2> handles{log.value,input.value};
  if(!UpdateProcThreadAttribute(attribute_list,0,PROC_THREAD_ATTRIBUTE_HANDLE_LIST,handles.data(),sizeof(handles),nullptr,nullptr))return {false,false,false,-1,"process-inheritance-failed"};
  STARTUPINFOEXW startup{};startup.StartupInfo.cb=sizeof(startup);startup.StartupInfo.dwFlags=STARTF_USESTDHANDLES;startup.StartupInfo.hStdOutput=log.value;startup.StartupInfo.hStdError=log.value;startup.StartupInfo.hStdInput=input.value;startup.lpAttributeList=attribute_list;
  std::wstring command=quote(executable);for(const auto& a:arguments){command+=L' ';command+=quote(a);}if(command.size()>32760)return {false,false,false,-1,"process-command-too-long"};
  PROCESS_INFORMATION info{};const auto working=executable.parent_path();
  if(!CreateProcessW(executable.c_str(),command.data(),nullptr,nullptr,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED|EXTENDED_STARTUPINFO_PRESENT,nullptr,working.c_str(),&startup.StartupInfo,&info))return {false,false,false,-1,"process-start-failed"};
  Handle process(info.hProcess),thread(info.hThread);
  if(!AssignProcessToJobObject(job.value,process.value)){TerminateProcess(process.value,2);WaitForSingleObject(process.value,1000);return {false,false,false,-1,"process-isolation-failed"};}
  if(ResumeThread(thread.value)==DWORD(-1)){TerminateJobObject(job.value,2);WaitForSingleObject(process.value,1000);return {false,false,false,-1,"process-resume-failed"};}
  for(;;){const DWORD wait=WaitForSingleObject(process.value,20);if(wait==WAIT_OBJECT_0)break;if(wait==WAIT_FAILED){TerminateJobObject(job.value,2);WaitForSingleObject(process.value,1000);return {false,false,false,-1,"process-wait-failed"};}
   const bool stopped=cancel.stop_requested(),expired=std::chrono::steady_clock::now()>=deadline;
   if(stopped||expired){TerminateJobObject(job.value,2);WaitForSingleObject(process.value,1000);return {false,stopped,expired,-1,stopped?"cancelled":"process-timeout"};}
  }
  DWORD code=0;if(!GetExitCodeProcess(process.value,&code))return {false,false,false,-1,"process-exit-failed"};
  return {code==0,false,false,int(code),code==0?std::string{}:"process-failed"};
#else
  const int log=::open(output_log.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(log<0)return {false,false,false,-1,"process-log-failed"};
  struct Fd {int fd;~Fd(){::close(fd);}} log_close{log};
  std::vector<std::string> texts;texts.push_back(executable.string());for(const auto& a:arguments)texts.push_back(a.string());std::vector<char*> argv;for(auto& a:texts)argv.push_back(a.data());argv.push_back(nullptr);
  posix_spawn_file_actions_t actions;posix_spawnattr_t attributes;
  if(posix_spawn_file_actions_init(&actions)!=0)return {false,false,false,-1,"process-actions-failed"};
  struct ActionsCleanup{posix_spawn_file_actions_t* value;~ActionsCleanup(){posix_spawn_file_actions_destroy(value);}} action_cleanup{&actions};
  if(posix_spawnattr_init(&attributes)!=0)return {false,false,false,-1,"process-attributes-failed"};
  struct SpawnCleanup{posix_spawnattr_t* value;~SpawnCleanup(){posix_spawnattr_destroy(value);}} spawn_cleanup{&attributes};
  if(posix_spawn_file_actions_adddup2(&actions,log,STDOUT_FILENO)||posix_spawn_file_actions_adddup2(&actions,log,STDERR_FILENO)||posix_spawn_file_actions_addopen(&actions,STDIN_FILENO,"/dev/null",O_RDONLY,0)||posix_spawnattr_setflags(&attributes,POSIX_SPAWN_SETPGROUP)||posix_spawnattr_setpgroup(&attributes,0))return {false,false,false,-1,"process-setup-failed"};
  pid_t pid=0;if(posix_spawn(&pid,executable.c_str(),&actions,&attributes,argv.data(),environ)!=0)return {false,false,false,-1,"process-start-failed"};
  for(;;){int status=0;const auto waited=waitpid(pid,&status,WNOHANG);if(waited==pid){const int code=WIFEXITED(status)?WEXITSTATUS(status):-1;return {code==0,false,false,code,code==0?std::string{}:"process-failed"};}if(waited<0&&errno!=EINTR)return {false,false,false,-1,"process-wait-failed"};
   const bool stopped=cancel.stop_requested(),expired=std::chrono::steady_clock::now()>=deadline;
   if(stopped||expired){::kill(-pid,SIGKILL);for(unsigned i=0;i<100;++i){const auto done=waitpid(pid,&status,WNOHANG);if(done==pid||(done<0&&errno==ECHILD))break;std::this_thread::sleep_for(std::chrono::milliseconds(5));}return {false,stopped,expired,-1,stopped?"cancelled":"process-timeout"};}
   std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
#endif
 }catch(...){return {false,false,false,-1,"process-exception"};}
}
JobResult render_job(const Config& config,std::stop_token cancel) noexcept {
 try{
  JobResult result;if(cancel.stop_requested()){result.reason="cancelled";return result;}
  result.rom_sha256=normalized_rom_sha256(config.rom_path,cancel);
  if(result.rom_sha256!=original_rom){result.reason=cancel.stop_requested()?"cancelled":"original-rom-unavailable";return result;}
#ifdef _WIN32
  const auto executable=config.exe_directory/"MusicRender0310.exe";
#else
  const auto executable=config.exe_directory/"MusicRender0310";
#endif
  if(!fs::is_regular_file(executable)){result.reason="renderer-unavailable";return result;}
  Directory temporary(config.cache_directory/"jobs");if(temporary.path.empty()){result.reason="job-directory-unavailable";return result;}
  auto process=run_process(executable,{"--identity"},temporary.path/"identity.log",cancel,std::chrono::seconds(5));
  if(!process.success){result.reason=process.reason;return result;}
  result.renderer_sha256=read_text(temporary.path/"identity.log",1024);
  while(!result.renderer_sha256.empty()&&(result.renderer_sha256.back()=='\r'||result.renderer_sha256.back()=='\n'))result.renderer_sha256.pop_back();
  if(!digest(result.renderer_sha256)){result.reason="renderer-identity-invalid";return result;}
  const pcm::Identity identity{result.rom_sha256,result.renderer_sha256};const auto path=pcm::cache_path(config.cache_directory,identity);
  if(!path){result.reason="cache-key-invalid";return result;}if(cancel.stop_requested()){result.reason="cancelled";return result;}
  const auto cached=pcm::load_cache(*path,identity);
  if(cached.status==pcm::CacheStatus::Ready){result.clip=cached.clip;result.from_cache=true;result.reason="cache-ready";return result;}
  const auto wave=temporary.path/"music.wav";
  process=run_process(executable,{config.rom_path,wave},temporary.path/"render.log",cancel,std::chrono::minutes(5));
  if(!process.success){result.reason=process.reason;return result;}
  if(cancel.stop_requested()){result.reason="cancelled";return result;}
  const auto text=read_text(fs::path(wave.native()+fs::path(".json").native()),65536);
  const auto json=nlohmann::json::parse(text,nullptr,false);
  if(!json.is_object()){result.reason="renderer-metadata-invalid";return result;}
  auto number=[&](const char* key)->std::optional<uint64_t>{auto it=json.find(key);return it!=json.end()&&it->is_number_unsigned()?std::optional<uint64_t>(it->get<uint64_t>()):std::nullopt;};
  auto exact=[&](const char* key,uint64_t expected){return number(key)==expected;};
  auto string=[&](const char* key,const std::string& expected){auto it=json.find(key);return it!=json.end()&&it->is_string()&&it->get_ref<const std::string&>()==expected;};
  const auto rate=number("sample_rate"),frames=number("frames"),begin=number("loop_begin"),end=number("loop_end"),seam=number("seam_step_max"),follow=number("seam_follow_256_frames_max_error");
  if(!exact("schema",1)||!string("normalized_rom_sha256",identity.rom_sha256)||!string("renderer_sha256",identity.renderer_sha256)||!exact("logical_track",22)||!exact("physical_sequence",11)||!exact("channels",2)||!exact("bits_per_sample",16)||!rate||*rate!=32000||!frames||!*frames||*frames>pcm::maximum_pcm_bytes/4||!begin||!end||*begin>=*end||*end>*frames||!seam||*seam>=128||!follow||*follow>2||!number("rsp_tasks").value_or(0)||!number("rom_dmas").value_or(0)) {result.reason="renderer-provenance-mismatch";return result;}
  bool begin_measured=false,end_measured=false;auto markers=json.find("loop_markers");
  if(markers!=json.end()&&markers->is_array()&&markers->size()<=100){for(const auto& marker:*markers){auto at=marker.find("frame");if(at!=marker.end()&&at->is_number_unsigned()){const auto value=at->get<uint64_t>();begin_measured|=value==*begin;end_measured|=value==*end;}}}
  if(!begin_measured||!end_measured){result.reason="renderer-loop-unmeasured";return result;}
  const auto imported=pcm::load_wave(wave,uint32_t(*rate),*begin,*end);
  if(!imported.clip||imported.clip->samples.size()/2!=*frames){result.reason="renderer-wave-invalid";return result;}
  if(cancel.stop_requested()){result.reason="cancelled";return result;}
  result.clip=imported.clip;const auto saved=pcm::store_cache(*path,identity,*result.clip);result.reason=saved.saved?"render-ready":"render-ready-cache-unavailable";return result;
 }catch(...){return {{},{},{},"renderer-exception",false};}
}
} // namespace detail
struct Session::Impl {
 mutable std::mutex mutex;std::condition_variable cv;Config config;Job job;AudioApi audio;Snapshot state;
 std::stop_source cancel;bool exit=false,requested=false,attempted=false;std::optional<JobResult> result;
 bool book_owned=false,native_audio_live=false;std::string last_log;std::thread worker;
 void log(const char* event){
  const auto host=pcm::snapshot();std::ostringstream line;line<<"event="<<event<<" status="<<state.status<<" installed="<<state.installed<<" playing="<<state.playing<<" jobs="<<state.jobs_started<<" generation="<<state.generation<<" book_owned="<<book_owned<<" native_audio_live="<<native_audio_live<<" sample_rate="<<host.sample_rate<<" generated_frames="<<host.generated_frames<<" reason="<<state.reason;
  // generated_frames advances each frame; compare only meaningful state edges.
  std::ostringstream edge;edge<<event<<' '<<state.status<<' '<<state.installed<<' '<<state.playing<<' '<<state.jobs_started<<' '<<state.generation<<' '<<book_owned<<' '<<native_audio_live<<' '<<state.reason;
  if(edge.str()!=last_log){last_log=edge.str();std::cout<<"[BookMusicSession0310] "<<line.str()<<'\n'<<std::flush;}
 }
 explicit Impl(Job render,AudioApi api):job(std::move(render)),audio(std::move(api)),worker([this]{work();}){}
 void work()noexcept{
  std::unique_lock lock(mutex);
  for(;;){cv.wait(lock,[&]{return exit||requested;});if(exit)return;
   const auto current=config;const auto generation=state.generation;auto token=cancel.get_token();requested=false;state.running=true;++state.jobs_started;state.status="working";
   lock.unlock();JobResult ready;try{ready=job(current,token);}catch(...){ready.reason="job-exception";}lock.lock();state.running=false;
   if(exit)return;if(generation!=state.generation||token.stop_requested())continue;
   state.rom_sha256=ready.rom_sha256;state.renderer_sha256=ready.renderer_sha256;state.reason=ready.reason;
   if(ready.clip&&pcm::valid_clip(*ready.clip)){result=std::move(ready);state.pending=true;state.status="ready";}else{state.status="unavailable";state.pending=false;}
  }
 }
};
Session::Session():Session(detail::render_job,{pcm::install,pcm::pump,pcm::stop,pcm::shutdown}){}
Session::Session(Job job,AudioApi audio):impl_(std::make_unique<Impl>(std::move(job),std::move(audio))){}
Session::~Session(){shutdown();}
void Session::configure(const fs::path& exe,const fs::path& cache,const fs::path& rom)noexcept{try{
 std::lock_guard lock(impl_->mutex);if(impl_->state.closed)return;impl_->cancel.request_stop();impl_->cancel=std::stop_source{};impl_->requested=false;impl_->attempted=false;impl_->result.reset();
 impl_->config={exe,cache,rom};++impl_->state.generation;impl_->state.configured=exe.is_absolute()&&cache.is_absolute()&&rom.is_absolute();impl_->state.pending=impl_->state.installed=impl_->state.playing=false;impl_->state.status=impl_->state.configured?"configured":"unavailable";impl_->state.reason=impl_->state.configured?"":"configuration-invalid";impl_->state.rom_sha256.clear();impl_->state.renderer_sha256.clear();
 impl_->audio.install({});impl_->log("configure");
}catch(...) {}}
void Session::frame(bool owned,bool native_live,double volume)noexcept{try{
 std::lock_guard lock(impl_->mutex);auto& state=impl_->state;if(state.closed)return;impl_->book_owned=owned;impl_->native_audio_live=native_live;
 if(!owned||native_live){impl_->audio.stop();state.playing=false;impl_->log("frame");return;}
 if(!state.configured){impl_->audio.stop();state.playing=false;impl_->log("frame");return;}
 if(impl_->result){auto ready=std::move(*impl_->result);impl_->result.reset();state.pending=false;state.installed=impl_->audio.install(std::move(ready.clip));state.status=state.installed?"installed":"unavailable";if(!state.installed)state.reason="host-install-failed";}
 if(!state.installed&&!impl_->attempted&&!impl_->requested&&!state.running&&pcm::volume_gain(volume)>0){impl_->attempted=true;impl_->requested=true;impl_->cancel=std::stop_source{};state.status="queued";impl_->cv.notify_one();}
 state.playing=state.installed&&impl_->audio.pump(true,false,volume);impl_->log("frame");
}catch(...) {}}
void Session::suspend()noexcept{try{
 std::lock_guard lock(impl_->mutex);if(impl_->state.closed)return;impl_->audio.stop();impl_->state.playing=false;impl_->cancel.request_stop();++impl_->state.generation;impl_->requested=false;impl_->attempted=false;impl_->result.reset();impl_->state.pending=false;impl_->state.status="suspended";impl_->book_owned=false;impl_->log("suspend");
}catch(...) {}}
void Session::shutdown()noexcept{try{
 {std::lock_guard lock(impl_->mutex);if(impl_->state.closed)return;impl_->state.closed=true;impl_->state.playing=false;impl_->state.pending=false;impl_->result.reset();++impl_->state.generation;impl_->cancel.request_stop();impl_->exit=true;impl_->requested=false;impl_->audio.stop();impl_->cv.notify_one();}
 if(impl_->worker.joinable())impl_->worker.join();
 {std::lock_guard lock(impl_->mutex);impl_->audio.shutdown();impl_->state.installed=false;impl_->state.status="closed";impl_->log("shutdown");}
}catch(...) {}}
Snapshot Session::snapshot()const noexcept{try{std::lock_guard lock(impl_->mutex);return impl_->state;}catch(...){return {};}}
namespace {Session& session(){static Session instance;return instance;}}
void configure(const fs::path& exe,const fs::path& cache,const fs::path& rom)noexcept{try{session().configure(exe,cache,rom);}catch(...) {}}
void frame(bool owned,bool native_live,double volume)noexcept{try{session().frame(owned,native_live,volume);}catch(...) {}}
void suspend()noexcept{try{session().suspend();}catch(...) {}}
void shutdown()noexcept{try{session().shutdown();}catch(...) {}}
Snapshot snapshot()noexcept{try{return session().snapshot();}catch(...){return {};}}
} // namespace workshop::menu_music_session0310
