#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

// This module never starts the game, reads a save or renders a sequence.
// The caller supplies measured native PCM and stops us before native audio.
namespace workshop::menu_music_host0310 {
inline constexpr unsigned channels=2,logical_bgm=22,physical_sequence=11;
inline constexpr uint64_t maximum_pcm_bytes=256ull*1024*1024;
struct Identity {
    std::string rom_sha256,renderer_sha256;
    unsigned logical=logical_bgm,sequence=physical_sequence;
    bool operator==(const Identity&)const=default;
};
struct Clip {
    uint32_t sample_rate=0;
    uint64_t loop_begin=0,loop_end=0; // Stereo frames, measured by native renderer.
    std::vector<int16_t> samples; // Host-order L/R pairs, before user volume.
};
bool valid_identity(const Identity&) noexcept;
bool valid_clip(const Clip&) noexcept;
struct ClipLoad {std::shared_ptr<const Clip> clip;std::string reason;};
// Import the worker's canonical 44-byte PCM16 stereo WAVE header. Identity and
// provenance are verified by the controller before this call; we validate only
// the bounded payload, exact rate and measured frame markers supplied here.
ClipLoad load_wave(const std::filesystem::path&,uint32_t expected_rate,uint64_t loop_begin,uint64_t loop_end) noexcept;
std::optional<std::filesystem::path> cache_path(const std::filesystem::path&,const Identity&);
enum class CacheStatus {Missing,Ready,Invalid,IoError};
struct CacheLoad {CacheStatus status=CacheStatus::Missing;std::shared_ptr<const Clip> clip;std::string reason;};
struct CacheSave {bool saved=false,already_present=false;std::string reason;};
CacheLoad load_cache(const std::filesystem::path&,const Identity&) noexcept;
// Never replaces an existing cache. A bad cache is preserved and fresh PCM may
// still be played without persistence. Concurrent successful writers converge.
CacheSave store_cache(const std::filesystem::path&,const Identity&,const Clip&) noexcept;
inline double volume_gain(double percent) noexcept {return std::isfinite(percent)?std::clamp(percent,0.,100.)/100.:0.;}
struct Snapshot {
    bool installed=false,opened=false,active=false,closed=false;
    uint32_t sample_rate=0;
    uint64_t generated_frames=0;
    std::string reason;
};
// Backend owns ONE separate queued-audio device. No native-game device, queue
// or mutex is exposed. Calls are thread-safe while this object is alive;
// shutdown may overlap pump/install. Join callers before object destruction.
// Required Backend methods: open(rate,channels), queued_bytes(), queue(span),
// start(), clear(), close(). open/queue return bool; other methods return void.
template<class Backend> class Player final {
    Backend backend_;mutable std::mutex mutex_;
    std::shared_ptr<const Clip> clip_;
    std::vector<float> block_;
    uint64_t generated_=0;
    double gain_=-1;
    bool opened_=false,attempted_=false,active_=false,closed_=false;
    std::string reason_;
    void close_device() noexcept {
        const bool had_device=opened_;
        opened_=active_=attempted_=false;generated_=0;gain_=-1;
        if(had_device){try{backend_.clear();}catch(...){}try{backend_.close();}catch(...){}}
    }
    uint64_t source_frame(uint64_t logical)const noexcept {
        if(logical<clip_->loop_end)return logical;
        return clip_->loop_begin+(logical-clip_->loop_end)%(clip_->loop_end-clip_->loop_begin);
    }
    bool failed(const char* reason) noexcept {
        close_device();attempted_=true;reason_=reason;return false;
    }
public:
    explicit Player(Backend backend=Backend{}):backend_(std::move(backend)){}
    Player(const Player&)=delete;Player& operator=(const Player&)=delete;
    ~Player(){shutdown();}
    bool install(std::shared_ptr<const Clip> clip) noexcept {
        try {
            std::lock_guard lock(mutex_);if(closed_)return false;
            close_device();clip_.reset();reason_.clear();
            if(!clip||!valid_clip(*clip)){reason_="invalid-clip";return false;}
            clip_=std::move(clip);return true;
        }catch(...){return false;}
    }
    // Call from UI frames. False is silent/unavailable, never a game failure.
    // Any native-audio-live transition synchronously clears and closes our
    // device before returning, even if book_visible is simultaneously true.
    bool pump(bool book_visible,bool native_audio_live,double main_volume_percent) noexcept {
        try {
            std::lock_guard lock(mutex_);
            const double gain=volume_gain(main_volume_percent);
            if(closed_)return false;
            if(!book_visible||native_audio_live||gain<=0||!clip_){close_device();return false;}
            if(!opened_){
                if(attempted_)return false;attempted_=true;
                if(!backend_.open(clip_->sample_rate,channels)){reason_="audio-device-unavailable";return false;}
                opened_=true;generated_=0;
            }
            uint64_t queued=backend_.queued_bytes()/(channels*sizeof(float));
            // Keep queued old-volume samples from sounding after volume/mute.
            // Rewind only unplayed samples, never jump over the queue we clear.
            if(gain_>=0&&gain_!=gain){
                generated_-=std::min(generated_,queued);backend_.clear();queued=0;
            }
            gain_=gain;
            const uint64_t target=std::max<uint64_t>(1,uint64_t(clip_->sample_rate)*80/1000);
            const uint64_t block_frames=std::max<uint64_t>(1,uint64_t(clip_->sample_rate)*20/1000);
            if(queued>target*4)return failed("audio-queue-inconsistent");
            const float scale=float(gain/32768.);
            while(queued<target){
                const auto count=std::min(block_frames,target-queued);
                if(generated_>UINT64_MAX-count)return failed("audio-cursor-overflow");
                block_.resize(size_t(count)*channels);
                for(uint64_t i=0;i<count;++i){
                    const auto at=source_frame(generated_+i)*channels;
                    block_[size_t(i)*channels]=float(clip_->samples[size_t(at)])*scale;
                    block_[size_t(i)*channels+1]=float(clip_->samples[size_t(at)+1])*scale;
                }
                if(!backend_.queue(std::span<const float>(block_)))return failed("audio-queue-failed");
                generated_+=count;queued+=count;
            }
            if(!active_)backend_.start();active_=true;reason_.clear();return true;
        }catch(...){
            try{std::lock_guard lock(mutex_);return failed("audio-backend-exception");}catch(...){return false;}
        }
    }
    void stop() noexcept {try{std::lock_guard lock(mutex_);close_device();}catch(...){}}
    void shutdown() noexcept {try{std::lock_guard lock(mutex_);if(closed_)return;close_device();clip_.reset();closed_=true;}catch(...){}}
    Snapshot snapshot()const noexcept {
        try{std::lock_guard lock(mutex_);return {bool(clip_),opened_,active_,closed_,clip_?clip_->sample_rate:0,generated_,reason_};}catch(...){return {};}
    }
};
// Production SDL endpoint, implemented in.cpp; cache/Player tests need no SDL.
bool install(std::shared_ptr<const Clip>) noexcept;
bool pump(bool book_visible,bool native_audio_live,double main_volume_percent) noexcept;
void stop() noexcept;
void shutdown() noexcept;
Snapshot snapshot() noexcept;
} // namespace workshop::menu_music_host0310
