#include "menu_music_host0310.hpp"
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <fstream>
#include <limits>
#include <system_error>
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
#include <unistd.h>
#endif
#ifndef MENU_MUSIC_HOST0310_NO_SDL
#include "SDL.h"
#endif

namespace workshop::menu_music_host0310 {
namespace {
using Header=std::array<uint8_t,128>;
constexpr std::array<uint8_t,8> magic{'D','T','M','M','0','3','1','0'};
constexpr uint64_t hash_initial=14695981039346656037ull,hash_prime=1099511628211ull;
std::atomic_uint64_t serial{0};
void put32(Header& h,size_t at,uint32_t value){for(unsigned i=0;i<4;++i)h[at+i]=uint8_t(value>>(i*8));}
void put64(Header& h,size_t at,uint64_t value){for(unsigned i=0;i<8;++i)h[at+i]=uint8_t(value>>(i*8));}
uint32_t get32(const Header& h,size_t at){uint32_t result=0;for(unsigned i=0;i<4;++i)result|=uint32_t(h[at+i])<<(i*8);return result;}
uint64_t get64(const Header& h,size_t at){uint64_t result=0;for(unsigned i=0;i<8;++i)result|=uint64_t(h[at+i])<<(i*8);return result;}
unsigned digit(char c){return c<='9'?unsigned(c-'0'):unsigned(c-'a'+10);}
bool digest(const std::string& value){return value.size()==64&&std::all_of(value.begin(),value.end(),[](char c){return(c>='0'&&c<='9')||(c>='a'&&c<='f');});}
void put_digest(Header& h,size_t at,const std::string& value){for(unsigned i=0;i<32;++i)h[at+i]=uint8_t(digit(value[i*2])*16+digit(value[i*2+1]));}
std::string get_digest(const Header& h,size_t at){constexpr char chars[]="0123456789abcdef";std::string result(64,'0');for(unsigned i=0;i<32;++i){result[i*2]=chars[h[at+i]>>4];result[i*2+1]=chars[h[at+i]&15];}return result;}
uint64_t hash_bytes(uint64_t state,std::span<const uint8_t> bytes){for(auto b:bytes)state=(state^b)*hash_prime;return state;}
Header header(const Identity& id,const Clip& clip){
 Header h{};std::copy(magic.begin(),magic.end(),h.begin());put32(h,8,1);put32(h,12,channels);put32(h,16,clip.sample_rate);put32(h,20,id.logical);put32(h,24,id.sequence);
 put_digest(h,32,id.rom_sha256);put_digest(h,64,id.renderer_sha256);put64(h,96,clip.samples.size()/channels);put64(h,104,clip.loop_begin);put64(h,112,clip.loop_end);
 uint64_t hash=hash_bytes(hash_initial,std::span<const uint8_t>(h).first(120));
 for(int16_t sample:clip.samples){const auto bits=std::bit_cast<uint16_t>(sample);hash=(hash^uint8_t(bits))*hash_prime;hash=(hash^uint8_t(bits>>8))*hash_prime;}
 put64(h,120,hash);return h;
}
bool key_matches_path(const std::filesystem::path& path,const Identity& id){const auto expected=cache_path(path.parent_path(),id);return expected&&path.filename()==expected->filename();}
bool same_clip(const Clip& a,const Clip& b){return a.sample_rate==b.sample_rate&&a.loop_begin==b.loop_begin&&a.loop_end==b.loop_end&&a.samples==b.samples;}
// The temporary name is exclusively created and never reuses a previous file.
// Final publication is no-replace: competing writers cannot clobber one another
// or overwrite a bad cache that a user may want to inspect.
bool publish(const std::filesystem::path& path,const Header& h,const Clip& clip,std::string& reason){
 auto temp=path;temp+=".tmp-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(serial.fetch_add(1));
#ifdef _WIN32
 HANDLE file=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(file==INVALID_HANDLE_VALUE){reason="cache-temp-create-failed";return false;}
 auto append=[&](std::span<const uint8_t> bytes){DWORD written=0;return bytes.size()<=MAXDWORD&&WriteFile(file,bytes.data(),DWORD(bytes.size()),&written,nullptr)&&written==bytes.size();};
#else
 int file=::open(temp.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);
 if(file<0){reason="cache-temp-create-failed";return false;}
 auto append=[&](std::span<const uint8_t> bytes){size_t at=0;while(at<bytes.size()){const auto n=::write(file,bytes.data()+at,bytes.size()-at);if(n<0&&errno==EINTR)continue;if(n<=0)return false;at+=size_t(n);}return true;};
#endif
 bool ok=append(h);std::array<uint8_t,65536> block{};
 for(size_t at=0;ok&&at<clip.samples.size();){
  const size_t count=std::min(clip.samples.size()-at,block.size()/2);
  for(size_t i=0;i<count;++i){const auto bits=std::bit_cast<uint16_t>(clip.samples[at+i]);block[i*2]=uint8_t(bits);block[i*2+1]=uint8_t(bits>>8);}
  ok=append(std::span<const uint8_t>(block).first(count*2));at+=count;
 }
#ifdef _WIN32
 if(ok)ok=FlushFileBuffers(file)!=0;
 if(!CloseHandle(file))ok=false;
 if(ok)ok=MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_WRITE_THROUGH)!=0;
#else
 if(ok)ok=::fsync(file)==0;
 if(::close(file)!=0)ok=false;
 if(ok)ok=::link(temp.c_str(),path.c_str())==0;
#endif
 // This path is our exclusively-created temporary, never a caller's file.
 std::error_code ignored;std::filesystem::remove(temp,ignored);
 if(!ok)reason="cache-publish-failed";return ok;
}
} // namespace
bool valid_identity(const Identity& id) noexcept {return id.logical==logical_bgm&&id.sequence==physical_sequence&&digest(id.rom_sha256)&&digest(id.renderer_sha256);}
bool valid_clip(const Clip& clip) noexcept {
 return clip.sample_rate>=8000&&clip.sample_rate<=192000&&!clip.samples.empty()&&clip.samples.size()%channels==0&&clip.samples.size()<=maximum_pcm_bytes/sizeof(int16_t)&&clip.loop_begin<clip.loop_end&&clip.loop_end<=clip.samples.size()/channels;
}
ClipLoad load_wave(const std::filesystem::path& path,uint32_t expected_rate,uint64_t loop_begin,uint64_t loop_end) noexcept {
 try{
  if(expected_rate<8000||expected_rate>192000||loop_begin>=loop_end||loop_end>maximum_pcm_bytes/(channels*sizeof(int16_t)))return {{},"wave-parameters-invalid"};
  const auto status=std::filesystem::symlink_status(path);
  if(!std::filesystem::is_regular_file(status)||std::filesystem::is_symlink(status))return {{},"wave-not-regular"};
  const uint64_t bytes=std::filesystem::file_size(path);
  if(bytes<44||bytes>44+maximum_pcm_bytes)return {{},"wave-size-invalid"};
  std::ifstream input(path,std::ios::binary);Header h{};input.read(reinterpret_cast<char*>(h.data()),44);
  constexpr std::array<uint8_t,4> riff{'R','I','F','F'},wave{'W','A','V','E'},fmt{'f','m','t',' '},data{'d','a','t','a'};
  if(!input||!std::equal(riff.begin(),riff.end(),h.begin())||!std::equal(wave.begin(),wave.end(),h.begin()+8)||!std::equal(fmt.begin(),fmt.end(),h.begin()+12)||!std::equal(data.begin(),data.end(),h.begin()+36))return {{},"wave-header-invalid"};
  const uint32_t payload=get32(h,40);
  // Canonical uncompressed PCM, exactly one fmt and one data chunk. Reject
  // ancillary or compressed chunks instead of guessing what the worker meant.
  if(get32(h,4)!=bytes-8||get32(h,16)!=16||get32(h,20)!=0x00020001||get32(h,24)!=expected_rate||get32(h,28)!=expected_rate*4||get32(h,32)!=0x00100004||!payload||payload%4||uint64_t(payload)+44!=bytes)return {{},"wave-format-or-length-invalid"};
  auto clip=std::make_shared<Clip>();clip->sample_rate=expected_rate;clip->loop_begin=loop_begin;clip->loop_end=loop_end;clip->samples.resize(payload/sizeof(int16_t));
  if(!valid_clip(*clip))return {{},"wave-loop-invalid"};
  std::array<uint8_t,65536> block{};
  for(size_t at=0;at<clip->samples.size();){
   const size_t count=std::min(clip->samples.size()-at,block.size()/2);input.read(reinterpret_cast<char*>(block.data()),std::streamsize(count*2));
   if(!input)return {{},"wave-pcm-truncated"};
   for(size_t i=0;i<count;++i)clip->samples[at+i]=std::bit_cast<int16_t>(uint16_t(uint16_t(block[i*2])|uint16_t(uint16_t(block[i*2+1])<<8)));
   at+=count;
  }
  if(input.peek()!=std::char_traits<char>::eof())return {{},"wave-trailing-data"};
  return {std::move(clip),{}};
 }catch(...){return {{},"wave-read-failed"};}
}
std::optional<std::filesystem::path> cache_path(const std::filesystem::path& directory,const Identity& id){
 if(directory.empty()||!valid_identity(id))return {};
 return directory/(id.rom_sha256+"-"+id.renderer_sha256+"-0016-000b.pcm31");
}
CacheLoad load_cache(const std::filesystem::path& path,const Identity& id) noexcept {
 try{
  if(!valid_identity(id)||!key_matches_path(path,id))return {CacheStatus::Invalid,{},"cache-key-invalid"};
  std::error_code error;const auto status=std::filesystem::symlink_status(path,error);
  if(error&&error!=std::errc::no_such_file_or_directory)return {CacheStatus::IoError,{},"cache-status-failed"};
  if(!std::filesystem::exists(status))return {};
  if(!std::filesystem::is_regular_file(status)||std::filesystem::is_symlink(status))return {CacheStatus::Invalid,{},"cache-not-regular"};
  const auto bytes=std::filesystem::file_size(path);
  if(bytes<128||bytes>128+maximum_pcm_bytes)return {CacheStatus::Invalid,{},"cache-size-invalid"};
  std::ifstream input(path,std::ios::binary);Header h{};input.read(reinterpret_cast<char*>(h.data()),h.size());
  if(!input||!std::equal(magic.begin(),magic.end(),h.begin())||get32(h,8)!=1||get32(h,12)!=channels||get32(h,28)!=0)return {CacheStatus::Invalid,{},"cache-header-invalid"};
  const Identity actual{get_digest(h,32),get_digest(h,64),get32(h,20),get32(h,24)};
  if(actual!=id)return {CacheStatus::Invalid,{},"cache-identity-mismatch"};
  const uint64_t frames=get64(h,96);
  if(!frames||frames>maximum_pcm_bytes/(channels*sizeof(int16_t))||bytes!=128+frames*channels*sizeof(int16_t))return {CacheStatus::Invalid,{},"cache-length-invalid"};
  auto clip=std::make_shared<Clip>();clip->sample_rate=get32(h,16);clip->loop_begin=get64(h,104);clip->loop_end=get64(h,112);
  clip->samples.resize(size_t(frames)*channels);
  if(!valid_clip(*clip))return {CacheStatus::Invalid,{},"cache-loop-or-rate-invalid"};
  std::array<uint8_t,65536> block{};uint64_t hash=hash_bytes(hash_initial,std::span<const uint8_t>(h).first(120));
  for(size_t at=0;at<clip->samples.size();){
   const size_t count=std::min(clip->samples.size()-at,block.size()/2);input.read(reinterpret_cast<char*>(block.data()),std::streamsize(count*2));
   if(!input)return {CacheStatus::Invalid,{},"cache-pcm-truncated"};
   hash=hash_bytes(hash,std::span<const uint8_t>(block).first(count*2));
   for(size_t i=0;i<count;++i)clip->samples[at+i]=std::bit_cast<int16_t>(uint16_t(uint16_t(block[i*2])|uint16_t(uint16_t(block[i*2+1])<<8)));
   at+=count;
  }
  if(hash!=get64(h,120))return {CacheStatus::Invalid,{},"cache-checksum-invalid"};
  if(input.peek()!=std::char_traits<char>::eof())return {CacheStatus::Invalid,{},"cache-trailing-data"};
  return {CacheStatus::Ready,std::move(clip),{}};
 }catch(...){return {CacheStatus::IoError,{},"cache-read-failed"};}
}
CacheSave store_cache(const std::filesystem::path& path,const Identity& id,const Clip& clip) noexcept {
 try{
  if(!valid_identity(id)||!valid_clip(clip)||!key_matches_path(path,id))return {false,false,"cache-input-invalid"};
  auto prior=load_cache(path,id);
  if(prior.status==CacheStatus::Ready)return same_clip(*prior.clip,clip)?CacheSave{true,true,{}}:CacheSave{false,true,"cache-conflict-preserved"};
  if(prior.status!=CacheStatus::Missing)return {false,true,"cache-invalid-preserved"};
  std::error_code error;std::filesystem::create_directories(path.parent_path(),error);if(error)return {false,false,"cache-directory-failed"};
  std::string reason;if(publish(path,header(id,clip),clip,reason))return {true,false,{}};
  // Another process may have published this exact cache while we rendered.
  prior=load_cache(path,id);
  if(prior.status==CacheStatus::Ready&&same_clip(*prior.clip,clip))return {true,true,{}};
  return {false,prior.status!=CacheStatus::Missing,reason};
 }catch(...){return {false,false,"cache-write-failed"};}
}
#ifndef MENU_MUSIC_HOST0310_NO_SDL
namespace {
struct SdlDevice {
 SDL_AudioDeviceID id=0;
 bool open(unsigned rate,unsigned count) noexcept {
  if(!(SDL_WasInit(SDL_INIT_AUDIO)&SDL_INIT_AUDIO))return false;
  SDL_AudioSpec desired{};desired.freq=int(rate);desired.format=AUDIO_F32SYS;desired.channels=Uint8(count);desired.samples=512;
  id=SDL_OpenAudioDevice(nullptr,0,&desired,nullptr,0);return id!=0;
 }
 size_t queued_bytes()const noexcept{return id?SDL_GetQueuedAudioSize(id):0;}
 bool queue(std::span<const float> data) noexcept {return id&&data.size_bytes()<=UINT32_MAX&&SDL_QueueAudio(id,data.data(),Uint32(data.size_bytes()))==0;}
 void start()const noexcept {if(id)SDL_PauseAudioDevice(id,0);}
 void clear()const noexcept {if(id)SDL_ClearQueuedAudio(id);}
 void close() noexcept {if(id){SDL_CloseAudioDevice(id);id=0;}}
};
Player<SdlDevice> player;
}
bool install(std::shared_ptr<const Clip> clip) noexcept {return player.install(std::move(clip));}
bool pump(bool visible,bool native_live,double volume) noexcept {return player.pump(visible,native_live,volume);}
void stop() noexcept {player.stop();}
void shutdown() noexcept {player.shutdown();}
Snapshot snapshot() noexcept {return player.snapshot();}
#else
bool install(std::shared_ptr<const Clip>) noexcept {return false;}
bool pump(bool,bool,double) noexcept {return false;}
void stop() noexcept {}
void shutdown() noexcept {}
Snapshot snapshot() noexcept {return {};}
#endif
} // namespace workshop::menu_music_host0310
