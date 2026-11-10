#include "../menu_music_host0310.hpp"
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>

namespace mm=workshop::menu_music_host0310;
namespace fs=std::filesystem;
static unsigned checks=0,corrupt_cases=0,wave_corrupt_cases=0;
static void need(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
static std::vector<uint8_t> bytes(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
static void write(const fs::path& p,std::span<const uint8_t> value){fs::create_directories(p.parent_path());std::ofstream f(p,std::ios::binary);f.write(reinterpret_cast<const char*>(value.data()),std::streamsize(value.size()));need(bool(f),"Fixture write failed");}
static void put32(std::vector<uint8_t>& a,size_t at,uint32_t v){for(unsigned i=0;i<4;++i)a[at+i]=uint8_t(v>>(i*8));}
static void put64(std::vector<uint8_t>& a,size_t at,uint64_t v){for(unsigned i=0;i<8;++i)a[at+i]=uint8_t(v>>(i*8));}
static mm::Identity identity(){return {std::string(64,'a'),std::string(64,'b')};}
static mm::Clip clip(unsigned rate=8000,unsigned frames=11,unsigned begin=3,unsigned end=8){
 mm::Clip value;value.sample_rate=rate;value.loop_begin=begin;value.loop_end=end;value.samples.resize(size_t(frames)*2);
 for(unsigned i=0;i<frames;++i){value.samples[i*2]=int16_t(int(i)*731-32768);value.samples[i*2+1]=int16_t(32767-int(i)*389);}return value;
}
struct DeviceState {
 std::mutex mutex;std::vector<float> pending,history;
 std::condition_variable cv;bool block_queue=false,queue_entered=false,release_queue=false;
 bool opened=false,fail_open=false,fail_queue=false,throw_queue=false,oversize=false;
 unsigned opens=0,closes=0,clears=0,starts=0,after_close=0,rate=0,channels=0;
};
struct Device {
 std::shared_ptr<DeviceState> state;
 bool open(unsigned rate,unsigned channels){std::lock_guard lock(state->mutex);++state->opens;if(state->fail_open)return false;state->opened=true;state->rate=rate;state->channels=channels;return true;}
 size_t queued_bytes()const{std::lock_guard lock(state->mutex);return state->oversize?std::numeric_limits<size_t>::max():state->pending.size()*sizeof(float);}
 bool queue(std::span<const float> data){std::unique_lock lock(state->mutex);if(state->block_queue){state->queue_entered=true;state->cv.notify_all();state->cv.wait(lock,[&]{return state->release_queue;});}if(!state->opened){++state->after_close;return false;}if(state->throw_queue)throw std::runtime_error("fixture queue");if(state->fail_queue)return false;state->pending.insert(state->pending.end(),data.begin(),data.end());state->history.insert(state->history.end(),data.begin(),data.end());return true;}
 void start(){std::lock_guard lock(state->mutex);++state->starts;}
 void clear(){std::lock_guard lock(state->mutex);++state->clears;state->pending.clear();}
 void close(){std::lock_guard lock(state->mutex);++state->closes;state->opened=false;}
};
static uint64_t frame(unsigned logical){return logical<8?logical:3+(logical-8)%5;}
static void expect_samples(const DeviceState& state,const mm::Clip& c,unsigned logical,double gain){
 for(size_t i=0;i<state.pending.size()/2;++i)for(unsigned side=0;side<2;++side){const float expected=float(c.samples[size_t(frame(logical+unsigned(i)))*2+side])*float(gain/32768.);need(std::abs(state.pending[i*2+side]-expected)<.00000001f,"Intro/loop/channel/volume sample differs from independent reference");}
}
int main(int argc,char** argv){try{
 need(argc==2,"Pass a new isolated fixture directory");const fs::path out=fs::absolute(argv[1]);need(!fs::exists(out),"Refusing old test output");fs::create_directories(out);
 auto id=identity();auto c=clip();
 need(mm::valid_identity(id)&&mm::valid_clip(c),"Valid source key/clip refused");
 for(unsigned fault=0;fault<7;++fault){auto bad=id;if(fault==0)bad.rom_sha256="../../other";if(fault==1)bad.renderer_sha256=std::string(64,'G');if(fault==2)bad.logical=11;if(fault==3)bad.sequence=22;if(fault==4)bad.rom_sha256=std::string(63,'a');if(fault==5)bad.renderer_sha256=std::string(65,'b');if(fault==6)bad.rom_sha256[5]='\\';need(!mm::valid_identity(bad)&&!mm::cache_path(out,bad),"Invalid cache key/path accepted");}
 for(unsigned fault=0;fault<8;++fault){auto bad=c;if(fault==0)bad.sample_rate=0;if(fault==1)bad.sample_rate=192001;if(fault==2)bad.samples.clear();if(fault==3)bad.samples.pop_back();if(fault==4)bad.loop_begin=bad.loop_end;if(fault==5)bad.loop_end=UINT64_MAX;if(fault==6)bad.loop_begin=UINT64_MAX;if(fault==7)bad.sample_rate=7999;need(!mm::valid_clip(bad),"Malformed clip bounds accepted");}
 need(!mm::cache_path({},id),"Empty cache root accepted");
 c=clip(44100,70001,101,70000);for(size_t i=0;i<c.samples.size();++i)c.samples[i]=std::bit_cast<int16_t>(uint16_t(i*977));
 // The renderer emits canonical PCM16LE stereo WAVE. Validate at the
 // trust boundary before installing or persisting any worker output.
 {std::vector<uint8_t> wav(44+c.samples.size()*2);const char* labels="RIFFWAVEfmt data";
  for(unsigned i=0;i<4;++i){wav[i]=uint8_t(labels[i]);wav[8+i]=uint8_t(labels[4+i]);wav[12+i]=uint8_t(labels[8+i]);wav[36+i]=uint8_t(labels[12+i]);}
  put32(wav,4,uint32_t(wav.size()-8));put32(wav,16,16);put32(wav,20,0x00020001);put32(wav,24,c.sample_rate);put32(wav,28,c.sample_rate*4);put32(wav,32,0x00100004);put32(wav,40,uint32_t(c.samples.size()*2));
  for(size_t i=0;i<c.samples.size();++i){const auto value=std::bit_cast<uint16_t>(c.samples[i]);wav[44+i*2]=uint8_t(value);wav[45+i*2]=uint8_t(value>>8);}
  const auto p=out/"canonical.wav";write(p,wav);auto imported=mm::load_wave(p,c.sample_rate,c.loop_begin,c.loop_end);
  need(imported.clip&&imported.clip->samples==c.samples&&imported.clip->sample_rate==c.sample_rate&&imported.clip->loop_begin==c.loop_begin&&imported.clip->loop_end==c.loop_end,"WAVE import changed signed PCM, L/R order or measured markers");
  need(!mm::load_wave(p,32000,c.loop_begin,c.loop_end).clip&&!mm::load_wave(p,c.sample_rate,0,UINT64_MAX).clip&&!mm::load_wave(p,0,0,1).clip&&!mm::load_wave(p,c.sample_rate,c.loop_end,c.loop_begin).clip,"Unverified WAVE rate or overflowing markers accepted");
  need(!mm::load_wave(out/"missing.wav",c.sample_rate,0,1).clip&&!mm::load_wave(out,c.sample_rate,0,1).clip,"Missing/directory WAVE accepted");
  for(unsigned fault=0;fault<15;++fault){auto bad=wav;if(fault==0)bad[0]^=1;if(fault==1)bad[8]^=1;if(fault==2)bad[12]^=1;if(fault==3)bad[36]^=1;if(fault==4)put32(bad,16,18);if(fault==5)bad[20]=3;if(fault==6)bad[22]=1;if(fault==7)put32(bad,24,0);if(fault==8)put32(bad,28,UINT32_MAX);if(fault==9)bad[32]=2;if(fault==10)bad[34]=8;if(fault==11)put32(bad,40,UINT32_MAX);if(fault==12)bad.pop_back();if(fault==13)bad.push_back(0);if(fault==14)bad.resize(43);
   const auto corrupt=out/("wave-corrupt-"+std::to_string(fault)+".wav");write(corrupt,bad);need(!mm::load_wave(corrupt,c.sample_rate,c.loop_begin,c.loop_end).clip&&bytes(corrupt)==bad,"Malformed WAVE imported or modified");++wave_corrupt_cases;
  }
 }
 const auto path=*mm::cache_path(out/"valid",id);
 need(mm::load_cache(path,id).status==mm::CacheStatus::Missing,"Missing cache not identified");
 auto saved=mm::store_cache(path,id,c);need(saved.saved&&!saved.already_present,"Fresh cache was not atomically stored");
 const auto original=bytes(path);need(original.size()==128+c.samples.size()*2,"Serialized size is not exact stereo int16LE");
 auto loaded=mm::load_cache(path,id);need(loaded.status==mm::CacheStatus::Ready&&loaded.clip&&loaded.clip->samples==c.samples&&loaded.clip->sample_rate==c.sample_rate&&loaded.clip->loop_begin==c.loop_begin&&loaded.clip->loop_end==c.loop_end,"Roundtrip changed signed PCM or loop bounds");
 saved=mm::store_cache(path,id,c);need(saved.saved&&saved.already_present&&bytes(path)==original,"Repeated store overwrote data");
 auto other=c;other.samples[100]^=1;saved=mm::store_cache(path,id,other);need(!saved.saved&&bytes(path)==original,"Conflicting render overwrote existing valid cache");
 for(unsigned fault=0;fault<17;++fault){
  auto data=original;
  if(fault==0)data[0]^=1;if(fault==1)data[8]=2;if(fault==2)data[12]=1;if(fault==3)data[28]=1;if(fault==4)data[32]^=1;if(fault==5)data[64]^=1;
  if(fault==6)put64(data,96,UINT64_MAX);if(fault==7)put64(data,96,0);if(fault==8)put64(data,104,UINT64_MAX);if(fault==9)put64(data,112,UINT64_MAX);
  if(fault==10){data[16]=0;data[17]=0;data[18]=0;data[19]=0;}if(fault==11)data[128+31]^=1;if(fault==12)data[120]^=1;if(fault==13)data.push_back(0);if(fault==14)data.pop_back();if(fault==15)data.resize(127);if(fault==16){data[24]=22;}
  const auto badpath=*mm::cache_path(out/("corrupt-"+std::to_string(fault)),id);write(badpath,data);
  const auto before=bytes(badpath);const auto read=mm::load_cache(badpath,id);
  need(read.status==mm::CacheStatus::Invalid&&!read.clip,"Corrupt cache accepted");
  need(!mm::store_cache(badpath,id,c).saved&&bytes(badpath)==before,"Corrupt cache overwritten");++corrupt_cases;
 }
 {auto key=id;key.renderer_sha256=std::string(64,'c');const auto p=*mm::cache_path(out/"stale",key);write(p,original);need(mm::load_cache(p,key).status==mm::CacheStatus::Invalid,"Stale renderer data accepted");}
 {const auto p=*mm::cache_path(out/"directory",id);fs::create_directories(p);need(mm::load_cache(p,id).status==mm::CacheStatus::Invalid&&!mm::store_cache(p,id,c).saved&&fs::is_directory(p),"A cache directory was followed/replaced");}
 {need(mm::load_cache(out/"save.json",id).status==mm::CacheStatus::Invalid&&!mm::store_cache(out/"save.json",id,c).saved&&!fs::exists(out/"save.json"),"Non-cache filename was created/read");}
 {const auto p=*mm::cache_path(out/"concurrent-cache",id);std::atomic_uint accepted=0;std::array<std::thread,8> threads;for(auto& t:threads)t=std::thread([&]{if(mm::store_cache(p,id,c).saved)++accepted;});for(auto& t:threads)t.join();need(accepted==threads.size()&&mm::load_cache(p,id).status==mm::CacheStatus::Ready&&bytes(p)==original,"Concurrent publication clobbered/rejected an identical winner");for(auto& entry:fs::directory_iterator(p.parent_path()))need(entry.path()==p,"Writer left a temporary file behind");}
 c=clip();auto asset=std::make_shared<const mm::Clip>(c);auto state=std::make_shared<DeviceState>();
 {mm::Player<Device> player(Device{state});
  need(player.install(asset),"Clip install failed");need(!player.pump(false,false,3)&&!state->opens,"Hidden book opened an audio device");
  need(!player.pump(true,true,3)&&!state->opens,"Live native audio overlapped menu audio");need(!player.pump(true,false,0)&&!state->opens,"Muted menu opened a device");
  need(player.pump(true,false,3)&&state->rate==8000&&state->channels==2&&state->starts==1&&state->pending.size()==1280,"Bounded queue/device format is incorrect");
  expect_samples(*state,c,0,.03);const auto history=state->history.size();need(player.pump(true,false,3)&&state->history.size()==history&&state->starts==1,"Full queue accumulated latency or restarted");
  state->pending.erase(state->pending.begin(),state->pending.begin()+34);need(player.pump(true,false,3)&&state->history.size()==history+34,"Drain refill lost or manufactured frames");
  expect_samples(*state,c,17,.03);
  const auto clears=state->clears;need(player.pump(true,false,10)&&state->clears==clears+1&&state->pending.size()==1280,"Volume change did not replace queued gain");expect_samples(*state,c,17,.10);
  need(!player.pump(true,true,10)&&!state->opened&&state->pending.empty()&&!player.snapshot().active,"Native-live edge left audible menu data");
  need(player.pump(true,false,3),"Allowed restart failed");expect_samples(*state,c,0,.03);
  need(!player.pump(true,false,std::numeric_limits<double>::quiet_NaN())&&!state->opened&&state->pending.empty(),"NaN volume was audible");
  need(player.pump(true,false,1000),"Clamped volume restart failed");expect_samples(*state,c,0,1.0);
  player.stop();player.stop();need(!state->opened&&state->pending.empty()&&!player.snapshot().active,"Repeated stop was not silent");
  need(player.pump(true,false,3),"Stop permanently disabled music");
  const auto opened_before=state->opens;auto malformed=std::make_shared<mm::Clip>(c);malformed->loop_end=UINT64_MAX;need(!player.install(malformed)&&!state->opened&&!player.snapshot().installed,"Invalid replacement retained old playback");
  need(!player.pump(true,false,3)&&state->opens==opened_before,"Invalid asset opened a device");
  need(player.install(asset)&&player.pump(true,false,3),"Replacement recovery failed");player.shutdown();
  const auto calls=state->opens;need(player.snapshot().closed&&!player.install(asset)&&!player.pump(true,false,3)&&state->opens==calls&&state->pending.empty(),"Shutdown allowed subsequent audio activity");
 }
 need(!state->opened&&!state->after_close,"Destruction left an endpoint open or queued after close");
 for(unsigned fault=0;fault<4;++fault){auto s=std::make_shared<DeviceState>();mm::Player<Device> player(Device{s});need(player.install(asset),"Fault asset failed");s->fail_open=fault==0;s->fail_queue=fault==1;s->throw_queue=fault==2;s->oversize=fault==3;
  need(!player.pump(true,false,3),"Bad backend was accepted");need(!player.snapshot().active,"Bad backend remained active");const auto opens=s->opens;need(!player.pump(true,false,3)&&s->opens==opens,"Failed endpoint was retried unboundedly");
  s->fail_open=s->fail_queue=s->throw_queue=s->oversize=false;player.stop();need(player.pump(true,false,3),"Explicit retry after device recovery failed");player.shutdown();need(!s->opened&&s->pending.empty()&&!s->after_close,"Failed endpoint was not closed cleanly");}
 {auto s=std::make_shared<DeviceState>();{mm::Player<Device> player(Device{s});need(player.install(asset)&&player.pump(true,false,3),"Destructor fixture did not play");}need(!s->opened&&s->closes==1&&s->pending.empty(),"Destructor without explicit shutdown did not synchronously stop audio");}
 {auto s=std::make_shared<DeviceState>();mm::Player<Device> player(Device{s});need(player.install(asset),"Concurrency asset refused");std::barrier start(3);
  std::thread pump([&]{start.arrive_and_wait();for(unsigned i=0;i<400;++i)player.pump(true,false,i%2?3:4);});
  std::thread replace([&]{start.arrive_and_wait();for(unsigned i=0;i<200;++i)player.install(asset);});
  start.arrive_and_wait();player.shutdown();pump.join();replace.join();need(player.snapshot().closed&&!s->opened&&s->pending.empty()&&!s->after_close,"Concurrent pump/install/shutdown queued after close");}
 // Deterministically enter a backend queue before initiating shutdown.
 // The pump owns the Player lock; shutdown must wait and then close all data.
 {auto s=std::make_shared<DeviceState>();s->block_queue=true;mm::Player<Device> player(Device{s});need(player.install(asset),"Blocked pump fixture refused");
  std::thread pump([&]{player.pump(true,false,3);});{std::unique_lock lock(s->mutex);s->cv.wait(lock,[&]{return s->queue_entered;});}
  std::atomic_bool shutdown_called=false;std::thread finish([&]{shutdown_called=true;player.shutdown();});while(!shutdown_called.load())std::this_thread::yield();
  {std::lock_guard lock(s->mutex);s->release_queue=true;s->cv.notify_all();}pump.join();finish.join();need(player.snapshot().closed&&!s->opened&&s->pending.empty()&&!s->after_close&&s->closes==1,"Shutdown overlapping an entered queue left samples/device alive");
 }
 std::ofstream report(out/"execution.json");report<<"{\"passed\":true,\"checks\":"<<checks<<",\"corruptCaches\":"<<corrupt_cases<<",\"corruptWaves\":"<<wave_corrupt_cases<<",\"identicalConcurrentWriters\":8,\"syntheticPcm\":true,\"physicalSdlDeviceOpened\":false,\"nativeMusicRendered\":false,\"gameLaunched\":false,\"personalFilesAccessed\":false,\"shutdownOverlapTestedWhileObjectAlive\":true,\"callingAfterObjectDestructionSupported\":false}\n";
 std::cout<<"MenuMusicHost0310 "<<checks<<" checks PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
