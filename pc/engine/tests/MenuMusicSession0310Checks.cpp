#include "../menu_music_session0310.hpp"
#include <atomic>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>
namespace mm=workshop::menu_music_session0310;
namespace pcm=workshop::menu_music_host0310;
namespace fs=std::filesystem;
using namespace std::chrono_literals;
static unsigned checks=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
template<class F>static void wait_for(F ready){const auto until=std::chrono::steady_clock::now()+3s;while(!ready()){if(std::chrono::steady_clock::now()>until)throw std::runtime_error("Async fixture timed out");std::this_thread::sleep_for(1ms);}}
static void write(const fs::path& path,std::span<const uint8_t> bytes){fs::create_directories(path.parent_path());std::ofstream file(path,std::ios::binary);file.write(reinterpret_cast<const char*>(bytes.data()),std::streamsize(bytes.size()));need(bool(file),"Fixture write failed");}
static std::string text(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
static std::shared_ptr<const pcm::Clip> clip(){auto c=std::make_shared<pcm::Clip>();c->sample_rate=32000;c->loop_begin=2;c->loop_end=7;c->samples={-32768,32767,1,-1,2,-2,3,-3,4,-4,5,-5,6,-6,7,-7};return c;}
struct Audio {
 const std::thread::id ui=std::this_thread::get_id();unsigned installs=0,invalidations=0,pumps=0,stops=0,shutdowns=0;bool installed=false,playing=false,wrong_thread=false;double volume=0;
 mm::AudioApi api(){return {[this](auto c){wrong_thread|=std::this_thread::get_id()!=ui;playing=false;installed=bool(c);if(c)++installs;else ++invalidations;return installed;},[this](bool book,bool native,double v){wrong_thread|=std::this_thread::get_id()!=ui;++pumps;volume=v;playing=book&&!native&&installed&&pcm::volume_gain(v)>0;return playing;},[this]{wrong_thread|=std::this_thread::get_id()!=ui;++stops;playing=false;},[this]{wrong_thread|=std::this_thread::get_id()!=ui;++shutdowns;installed=playing=false;}};}
};
struct Gate {
 std::mutex mutex;std::condition_variable cv;bool entered=false,release=false;
 void block(){std::unique_lock lock(mutex);entered=true;cv.notify_all();cv.wait(lock,[&]{return release;});}
 void await(){std::unique_lock lock(mutex);if(!cv.wait_for(lock,3s,[&]{return entered;}))throw std::runtime_error("Job did not enter");}
 void finish(){std::lock_guard lock(mutex);release=true;cv.notify_all();}
};
int main(int argc,char**argv){
 if(argc>=2&&std::string(argv[1])=="--child"){
  if(argc>=3&&std::string(argv[2])=="sleep"){std::cout<<"entered\n"<<std::flush;std::this_thread::sleep_for(5s);return 0;}
  if(argc>=3&&std::string(argv[2])=="fail")return 7;
  for(int i=2;i<argc;++i)std::cout<<argv[i]<<'\n';return 0;
 }
 try{
 need(argc==2,"Pass a fresh output directory");const fs::path out=fs::absolute(argv[1]);need(!fs::exists(out),"Refusing old fixtures");fs::create_directories(out);
 const std::string abc="abc",empty,million(1000000,'a');auto hash=[](const std::string& s){return mm::detail::sha256(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(s.data()),s.size()));};
 need(hash(empty)=="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855","SHA256 empty NIST mismatch");
 need(hash(abc)=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad","SHA256 abc NIST mismatch");
 need(hash(million)=="cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0","SHA256 million-a NIST mismatch");
 std::vector<uint8_t> rom(0x1000000);for(size_t i=0;i<rom.size();++i)rom[i]=uint8_t(i*83+17);rom[0]=0x80;rom[1]=0x37;rom[2]=0x12;rom[3]=0x40;const auto expected=mm::detail::sha256(rom);const auto be=out/"synthetic.z64";write(be,rom);
 need(mm::detail::normalized_rom_sha256(be,{})==expected,"BE normalized identity mismatch");for(size_t i=0;i<rom.size();i+=2)std::swap(rom[i],rom[i+1]);const auto v64=out/"synthetic.v64";write(v64,rom);need(mm::detail::normalized_rom_sha256(v64,{})==expected,"V64 normalized identity differs");
 for(size_t i=0;i<rom.size();i+=2)std::swap(rom[i],rom[i+1]);for(size_t i=0;i<rom.size();i+=4){std::swap(rom[i],rom[i+3]);std::swap(rom[i+1],rom[i+2]);}const auto n64=out/"synthetic.n64";write(n64,rom);need(mm::detail::normalized_rom_sha256(n64,{})==expected,"N64 normalized identity differs");
 std::stop_source stopped;stopped.request_stop();need(mm::detail::normalized_rom_sha256(be,stopped.get_token()).empty(),"Cancelled ROM read continued");
 rom.resize(4);write(out/"short.z64",rom);need(mm::detail::normalized_rom_sha256(out/"short.z64",{}).empty(),"Short ROM accepted");
 mm::Config config{out,out/"cache",be};const auto rejected=mm::detail::render_job(config,{});need(!rejected.clip&&rejected.reason=="original-rom-unavailable"&&!fs::exists(config.cache_directory),"Non-original ROM started worker or created cache");
 const auto self=fs::absolute(argv[0]);const auto spaced=out/fs::path(u8"Dossier été 🥚")/"CPU Fixture.exe";fs::create_directories(spaced.parent_path());fs::copy_file(self,spaced);
 const std::vector<fs::path> arguments{"--child","with spaces",R"(trailing slash\)",R"(quote"inside)","$(literal); & unchanged"};const auto log=out/"arguments.log";
 auto process=mm::detail::run_process(spaced,arguments,log,{},2s);need(process.success&&process.exit_code==0,"Hidden process with Unicode executable/spaces failed");
 auto argument_text=text(log);argument_text.erase(std::remove(argument_text.begin(),argument_text.end(),'\r'),argument_text.end());need(argument_text=="with spaces\ntrailing slash\\\nquote\"inside\n$(literal); & unchanged\n","Arguments were interpreted or incorrectly quoted");
 const auto before=text(log);process=mm::detail::run_process(spaced,arguments,log,{},2s);need(!process.success&&text(log)==before,"Existing process log overwritten");
 process=mm::detail::run_process(spaced,{"--child","fail"},out/"fail.log",{},2s);need(!process.success&&process.exit_code==7,"Child failure status lost");
 auto started=std::chrono::steady_clock::now();process=mm::detail::run_process(spaced,{"--child","sleep"},out/"timeout.log",{},60ms);need(process.timed_out&&!process.success&&std::chrono::steady_clock::now()-started<1500ms,"Worker timeout did not terminate promptly");
 std::stop_source cancel;std::thread cancel_thread([&]{std::this_thread::sleep_for(40ms);cancel.request_stop();});started=std::chrono::steady_clock::now();process=mm::detail::run_process(spaced,{"--child","sleep"},out/"cancel.log",cancel.get_token(),2s);cancel_thread.join();need(process.cancelled&&!process.success&&std::chrono::steady_clock::now()-started<1500ms,"Worker cancellation did not terminate promptly");
 process=mm::detail::run_process(spaced,arguments,out/"pre-cancel.log",stopped.get_token(),2s);need(process.cancelled&&!fs::exists(out/"pre-cancel.log"),"Pre-cancelled process created output");
 need(!mm::detail::run_process("relative.exe",{},out/"invalid.log",{},2s).success&&!fs::exists(out/"invalid.log"),"Relative executable allowed");
 const auto asset=clip();
 {Audio audio;std::atomic_uint calls=0;mm::Session session([&](const auto&,auto){++calls;return mm::JobResult{asset,"rom","renderer","test-ready",false};},audio.api());
  session.configure(out,out/"cache",be);need(session.snapshot().configured&&calls==0,"Configure performed background IO before a frame");session.frame(false,false,3);session.frame(true,true,3);session.frame(true,false,0);need(calls==0&&!audio.playing,"Hidden/native/muted state started work");
  session.frame(true,false,3);wait_for([&]{return session.snapshot().pending;});need(audio.installs==0&&!audio.playing,"Worker installed or played PCM");session.frame(false,false,3);need(audio.installs==0,"Hidden frame accepted late PCM");session.frame(true,false,3);need(audio.installs==1&&audio.playing&&audio.volume==3&&session.snapshot().installed,"Current ready clip did not install on UI");
  for(unsigned i=0;i<20;++i)session.frame(true,false,3);need(calls==1&&session.snapshot().jobs_started==1,"Frames duplicated a successful render");session.frame(true,true,3);need(!audio.playing,"Native live did not win ownership");session.frame(true,false,0);need(!audio.playing,"Menu mute ignored");session.frame(true,false,3);need(audio.playing,"Unmute did not restart installed music");session.suspend();need(!audio.playing&&!session.snapshot().playing,"Suspend was not synchronous");session.frame(true,false,3);need(audio.playing&&calls==1,"Return book rerendered a retained clip");session.shutdown();need(session.snapshot().closed&&!audio.playing&&audio.shutdowns==1&&!audio.wrong_thread,"Shutdown/UI hook ownership failed");session.frame(true,false,3);session.shutdown();need(audio.shutdowns==1,"Shutdown allowed another audio shutdown");
 }
 {Audio audio;Gate gate;std::atomic_uint calls=0;mm::Session session([&](const auto&,auto){if(++calls==1)gate.block();return mm::JobResult{asset,"rom","renderer","late",false};},audio.api());session.configure(out,out/"cache",be);session.frame(true,false,3);gate.await();const auto generation=session.snapshot().generation;
  started=std::chrono::steady_clock::now();session.suspend();need(std::chrono::steady_clock::now()-started<100ms&&session.snapshot().generation>generation,"Suspend waited for slow job");gate.finish();wait_for([&]{return !session.snapshot().running;});session.frame(false,true,3);need(audio.installs==0&&!audio.playing&&!session.snapshot().pending,"Cancelled generation installed late music during game");session.frame(true,false,3);wait_for([&]{return session.snapshot().pending;});session.frame(true,false,3);need(audio.installs==1&&calls==2&&audio.playing,"Return book did not retry cancelled job");session.shutdown();
 }
 {Audio audio;Gate gate;std::atomic_uint calls=0;mm::Session session([&](const auto&,auto){if(++calls==1)gate.block();return mm::JobResult{asset,"rom","renderer","configured",false};},audio.api());session.configure(out,out/"cache",be);session.frame(true,false,3);gate.await();session.configure(out,out/"different-cache",n64);gate.finish();wait_for([&]{return !session.snapshot().running;});need(!session.snapshot().pending&&audio.installs==0,"Old config result survived reconfigure");session.frame(true,false,3);wait_for([&]{return session.snapshot().pending;});session.frame(true,false,3);need(audio.installs==1&&calls==2,"Reconfigure did not execute exactly one new job");session.shutdown();}
 {Audio audio;std::atomic_uint calls=0;mm::Session session([&](const auto&,auto){++calls;return mm::JobResult{{},{},{},"missing-worker",false};},audio.api());session.configure(out,out/"cache",be);session.frame(true,false,3);wait_for([&]{return session.snapshot().status=="unavailable";});for(unsigned i=0;i<100;++i)session.frame(true,false,3);need(calls==1&&!audio.playing,"Optional failure retried per-frame or played invalid PCM");session.configure("relative",out,be);session.frame(true,false,3);need(!session.snapshot().configured&&calls==1,"Invalid configure started a worker");session.shutdown();}
 {Audio audio;std::atomic_bool entered=false;mm::Session session([&](const auto&,std::stop_token token){entered=true;while(!token.stop_requested())std::this_thread::sleep_for(1ms);return mm::JobResult{{},{},{},"cancelled",false};},audio.api());session.configure(out,out/"cache",be);session.frame(true,false,3);wait_for([&]{return entered.load();});started=std::chrono::steady_clock::now();session.shutdown();need(std::chrono::steady_clock::now()-started<1s&&session.snapshot().closed&&!audio.playing,"Shutdown failed to cancel/join cooperative job");}
 std::ofstream report(out/"execution.json");report<<"{\"passed\":true,\"checks\":"<<checks<<",\"sha256NistVectors\":3,\"normalizedSyntheticFormats\":3,\"realWorkerProcessFixture\":true,\"workerTimeoutAndCancellation\":true,\"lateGenerationRejected\":true,\"uiInstallOnly\":true,\"personalFilesAccessed\":false,\"originalRomAccessed\":false,\"gameLaunched\":false,\"physicalAudioOpened\":false}\n";std::cout<<"MenuMusicSession0310 "<<checks<<" checks PASS\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
