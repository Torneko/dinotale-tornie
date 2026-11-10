#pragma once
#include "menu_music_host0310.hpp"
#include <chrono>
#include <functional>
#include <memory>
#include <span>
#include <stop_token>

namespace workshop::menu_music_session0310 {
namespace pcm=menu_music_host0310;
struct Config {std::filesystem::path exe_directory,cache_directory,rom_path;};
struct JobResult {std::shared_ptr<const pcm::Clip> clip;std::string rom_sha256,renderer_sha256,reason;bool from_cache=false;};
struct Snapshot {
 bool configured=false,running=false,pending=false,installed=false,playing=false,closed=false;
 uint64_t generation=0,jobs_started=0;
 std::string status="idle",reason,rom_sha256,renderer_sha256;
};
// Hooks let CPU tests exercise asynchronous ownership without spawning a game
// or opening SDL. Only frame() may install/pump; the worker never calls audio.
struct AudioApi {
 std::function<bool(std::shared_ptr<const pcm::Clip>)> install;
 std::function<bool(bool,bool,double)> pump;
 std::function<void()> stop,shutdown;
};
using Job=std::function<JobResult(const Config&,std::stop_token)>;
class Session final {
 struct Impl;std::unique_ptr<Impl> impl_;
public:
 Session();Session(Job,AudioApi);~Session();
 Session(const Session&)=delete;Session& operator=(const Session&)=delete;
 // Path assignment only: no ROM/cache/process I/O and no waiting for a job.
 void configure(const std::filesystem::path& exe_directory,const std::filesystem::path& cache_directory,const std::filesystem::path& rom_path) noexcept;
 // Caller owns/gates native audio outside this module before passing false.
 // book_owned includes modals opened from the book. native_audio_live always wins.
 void frame(bool book_owned,bool native_audio_live,double volume_percent) noexcept;
 // Synchronous audio stop; cancellation is signalled, never joined on UI.
 void suspend() noexcept;
 // Cancel and join the bounded process worker before permanent host shutdown.
 void shutdown() noexcept;
 Snapshot snapshot()const noexcept;
};
void configure(const std::filesystem::path& exe_directory,const std::filesystem::path& cache_directory,const std::filesystem::path& rom_path) noexcept;
void frame(bool book_owned,bool native_audio_live,double volume_percent) noexcept;
void suspend() noexcept;
void shutdown() noexcept;
Snapshot snapshot() noexcept;
namespace detail {
// Production primitives exposed for isolated boundary tests. No game globals.
std::string sha256(std::span<const uint8_t>);
std::string normalized_rom_sha256(const std::filesystem::path&,std::stop_token);
struct ProcessResult {bool success=false,cancelled=false,timed_out=false;int exit_code=-1;std::string reason;};
ProcessResult run_process(const std::filesystem::path& executable,const std::vector<std::filesystem::path>& arguments,const std::filesystem::path& output_log,std::stop_token,std::chrono::milliseconds timeout) noexcept;
JobResult render_job(const Config&,std::stop_token) noexcept;
}
} // namespace workshop::menu_music_session0310
