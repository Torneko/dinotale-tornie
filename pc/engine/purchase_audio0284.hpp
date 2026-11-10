#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <span>
#include <utility>

namespace workshop::purchase_audio0284 {
inline constexpr unsigned sample_rate=44100,channels=2,duration_ms=120;
inline constexpr std::size_t frames=sample_rate*duration_ms/1000,samples=frames*channels;
inline constexpr float peak_limit=.16f;
using Pcm=std::array<float,samples>;
static_assert(frames==5292 && samples*sizeof(float)==42336);
inline double volume_gain(double percent) noexcept {
 return std::isfinite(percent)?std::clamp(percent,0.,100.)/100.:0.;
}
// A short original rising tone. Both endpoints and their envelopes are zero.
inline const Pcm& unit_pcm() noexcept {
 static const Pcm pcm=[] {
  Pcm out{};constexpr double pi=3.14159265358979323846;
  for(std::size_t i=0;i<frames;++i){
   const double time=double(i)/sample_rate,remaining=double(frames-1-i)/sample_rate;
   const double attack=std::sin(.5*pi*std::min(time/.008,1.));
   const double release=std::sin(.5*pi*std::min(remaining/.024,1.));
   const float value=float(double(peak_limit)*attack*attack*release*release*std::sin(2*pi*(660*time+1100*time*time)));
   out[i*channels]=out[i*channels+1]=value;
  }
  return out;
 }();
 return pcm;
}
inline void render(Pcm& out,double percent) noexcept {
 const float gain=float(volume_gain(percent));const auto& source=unit_pcm();
 for(std::size_t i=0;i<samples;++i)out[i]=source[i]*gain;
}
// Backend owns a separate host device. No game buffer, rate, queue or lock is
// part of this interface. A busy voice is dropped instead of building latency.
template<class Backend> class Player {
 Backend backend_;std::mutex mutex_;Pcm buffer_{};
 bool attempted_=false,opened_=false,closed_=false;
public:
 explicit Player(Backend backend=Backend{}):backend_(std::move(backend)){}
 Player(const Player&)=delete;Player& operator=(const Player&)=delete;
 ~Player(){shutdown();}
 bool play(double percent) noexcept {
  if(volume_gain(percent)<=0)return false;
  try {
   std::unique_lock lock(mutex_,std::try_to_lock);if(!lock.owns_lock()||closed_)return false;
   if(!attempted_){attempted_=true;opened_=backend_.open(sample_rate,channels);}
   if(!opened_||backend_.queued_bytes()!=0)return false;
   render(buffer_,percent);if(!backend_.queue(std::span<const float>(buffer_)))return false;
   backend_.start();return true;
  }catch(...){return false;}
 }
 void shutdown() noexcept {
  try {std::lock_guard lock(mutex_);if(closed_)return;closed_=true;if(opened_){backend_.close();opened_=false;}}
  catch(...){}
 }
};
}
#ifdef WORKSHOP_CONTENT_BUILD
// Called only after a purchase has been committed by the menu's own ledger.
// False means muted, busy, unavailable, or unable to queue; purchases still stand.
bool workshop_purchase_sound0284() noexcept;
#else
inline bool workshop_purchase_sound0284() noexcept {return false;}
#endif
