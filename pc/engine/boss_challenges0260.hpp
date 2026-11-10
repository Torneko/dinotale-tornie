#pragma once
#include "json/json.hpp"
#include <array>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
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

namespace workshop::boss_challenges0260 {
// Only the native production-menu adapter may build qualifying evidence.
// CPU validation of these fields is not proof that native gameplay occurred.
enum class Provenance { Unknown,ProductionMenu,ScriptedQa,ManualQa,Simulation };
struct EncounterEvidence {
    unsigned number=0,expectedMembers=0,defeatedMask=0,defeatVi=0,color=14;
    uint32_t player=0;
    bool initialized=false,active=false,terminal=false,causal=false;
};
struct CompletionEvidence {
    std::string token;
    Provenance provenance=Provenance::Unknown;
    bool phaseComplete=false,diagnostic=false,qaMarker=false,skippedEntry=false;
    unsigned completed=0,index=0,color=14,initialColor=14,completionVi=0;
    uint32_t player=0,initialPlayer=0;
    uint16_t playerKind=0;
    std::array<EncounterEvidence,10> encounters{};
};
struct Snapshot {
    uint16_t mask14=0;
    std::array<unsigned,14> counts14{};
    bool readable=false,saved=false;
    std::string reason;
};
namespace detail {
inline constexpr unsigned max_runs=100000;
inline constexpr uintmax_t max_file_bytes=16*1024*1024;
struct State {Snapshot view;std::map<std::string,unsigned> runs;};
inline std::mutex mutex;
inline State state;
inline std::filesystem::path directory,filename;
inline bool initialized=false;
inline std::atomic_uint64_t serial{0};
inline bool hex_token(std::string_view s){return s.size()==32&&std::all_of(s.begin(),s.end(),[](unsigned char c){return (c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F');});}
inline std::string normalized(std::string s){for(auto& c:s)if(c>='A'&&c<='F')c=char(c-'A'+'a');return s;}
inline bool pointer(uint32_t a){return a>=0x80000400&&a<=0x807FFE00&&(a&3)==0;}
inline std::string evidence_error(const CompletionEvidence& e){
    if(!hex_token(e.token))return "invalid-run-token";
    if(e.provenance!=Provenance::ProductionMenu)return "not-production-menu";
    if(e.diagnostic||e.qaMarker||e.skippedEntry)return "qa-or-skipped-entry";
    if(!e.phaseComplete||e.completed!=10||e.index!=9)return "ten-complete-encounters-required";
    if(!pointer(e.player)||e.player!=e.initialPlayer||e.playerKind!=0x4001)return "player-identity-changed";
    if(e.color>=14||e.color!=e.initialColor)return "yoshi-color-changed";
    unsigned previous=0;
    for(unsigned n=0;n<10;++n){const auto& r=e.encounters[n];
        if(r.number!=n+1||r.expectedMembers<1||r.expectedMembers>4||r.defeatedMask!=((1u<<r.expectedMembers)-1))return "incomplete-native-member-set";
        if(!r.initialized||!r.active||!r.terminal||!r.causal)return "missing-native-causal-proof";
        if(r.player!=e.player||r.color!=e.color)return "encounter-identity-changed";
        if(!r.defeatVi||r.defeatVi<=previous)return "encounter-order-invalid";
        previous=r.defeatVi;
    }
    if(!e.completionVi||e.completionVi<previous)return "completion-before-final-defeat";
    return {};
}
inline bool qa_directory(){
    for(const char* marker:{"boss-battle-qa.txt","solo-boss-qa.txt","manual-coop-qa.txt","native-coop-qa.txt","coop-qa.txt","manual-preview.txt"}){
        std::error_code error;const auto status=std::filesystem::symlink_status(directory/marker,error);
        if((error&&error!=std::errc::no_such_file_or_directory)||std::filesystem::exists(status))return true;
    }
    return false;
}
inline State read_disk(){
    State result;result.view.readable=result.view.saved=true;
    std::error_code error;const auto status=std::filesystem::symlink_status(filename,error);
    if(error&&error!=std::errc::no_such_file_or_directory)throw std::runtime_error("ledger-status-failed");
    if(!std::filesystem::exists(status))return result;
    if(std::filesystem::is_symlink(status)||!std::filesystem::is_regular_file(status))throw std::runtime_error("ledger-not-regular-file");
    if(std::filesystem::file_size(filename)>max_file_bytes)throw std::runtime_error("ledger-too-large");
    std::ifstream file(filename,std::ios::binary);if(!file)throw std::runtime_error("ledger-open-failed");
    const auto data=nlohmann::json::parse(file);
    if(!data.is_object()||data.size()!=4||!data.contains("schema")||!data["schema"].is_number_unsigned()||data["schema"]!=1||!data.contains("mask14")||!data["mask14"].is_number_unsigned()||!data.contains("counts14")||!data["counts14"].is_array()||data["counts14"].size()!=14||!data.contains("runs")||!data["runs"].is_array()||data["runs"].size()>max_runs)throw std::runtime_error("ledger-format-invalid");
    const auto mask=data["mask14"].get<uint64_t>();if(mask>0x3FFF)throw std::runtime_error("ledger-mask-invalid");
    for(const auto& row:data["runs"]){
        if(!row.is_object()||row.size()!=2||!row.contains("token")||!row["token"].is_string()||!row.contains("color")||!row["color"].is_number_unsigned())throw std::runtime_error("ledger-run-invalid");
        const auto token=row["token"].get<std::string>();const auto color=row["color"].get<uint64_t>();
        if(!hex_token(token)||token!=normalized(token)||color>=14||!result.runs.emplace(token,unsigned(color)).second)throw std::runtime_error("ledger-run-identity-invalid");
        result.view.counts14[color]++;result.view.mask14|=uint16_t(1u<<color);
    }
    if(result.view.mask14!=mask)throw std::runtime_error("ledger-mask-count-mismatch");
    for(unsigned n=0;n<14;++n)if(!data["counts14"][n].is_number_unsigned()||data["counts14"][n].get<uint64_t>()!=result.view.counts14[n])throw std::runtime_error("ledger-count-mismatch");
    return result;
}
inline std::string encode(const State& value){
    auto runs=nlohmann::json::array();for(const auto& [token,color]:value.runs)runs.push_back({{"token",token},{"color",color}});
    return nlohmann::json{{"schema",1u},{"mask14",unsigned(value.view.mask14)},{"counts14",value.view.counts14},{"runs",runs}}.dump(2)+'\n';
}
inline void atomic_replace(const std::string& bytes){
    auto temp=filename;temp+=std::string(".tmp-")+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(serial.fetch_add(1));
    try{
#ifdef _WIN32
        HANDLE file=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(file==INVALID_HANDLE_VALUE)throw std::runtime_error("ledger-temp-create-failed");
        DWORD written=0;const bool flushed=WriteFile(file,bytes.data(),DWORD(bytes.size()),&written,nullptr)&&written==bytes.size()&&FlushFileBuffers(file);
        const bool closed=CloseHandle(file)!=0;if(!flushed||!closed)throw std::runtime_error("ledger-temp-flush-failed");
        if(!MoveFileExW(temp.c_str(),filename.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("ledger-replace-failed");
#else
        const int file=::open(temp.c_str(),O_WRONLY|O_CREAT|O_EXCL,0600);if(file<0)throw std::runtime_error("ledger-temp-create-failed");
        size_t offset=0;bool good=true;while(offset<bytes.size()){const auto count=::write(file,bytes.data()+offset,bytes.size()-offset);if(count<0&&errno==EINTR)continue;if(count<=0){good=false;break;}offset+=size_t(count);}
        const bool flushed=good&&::fsync(file)==0;const bool closed=::close(file)==0;if(!flushed||!closed)throw std::runtime_error("ledger-temp-flush-failed");
        std::filesystem::rename(temp,filename);
#endif
    }catch(...){std::error_code ignored;std::filesystem::remove(temp,ignored);throw;}
}
}
// Explicit profile destination only. No personal/default profile is inferred,
// and an unreadable existing file is retained rather than reset or overwritten.
inline bool load(const std::filesystem::path& profile){
    std::lock_guard lock(detail::mutex);detail::state={};detail::initialized=false;
    detail::directory=profile;detail::filename=profile/"pc-boss-challenges.json";
    try{if(profile.empty()||!std::filesystem::is_directory(profile))throw std::runtime_error("profile-directory-missing");detail::state=detail::read_disk();detail::initialized=true;return true;}
    catch(const std::exception& e){detail::state.view.reason=e.what();return false;}
}
inline Snapshot snapshot(){std::lock_guard lock(detail::mutex);return detail::state.view;}
inline bool record(const CompletionEvidence& evidence){
    std::lock_guard lock(detail::mutex);
    auto reject=[](std::string reason){detail::state.view.reason=std::move(reason);return false;};
    if(!detail::initialized||!detail::state.view.readable)return reject("ledger-not-readable");
    if(auto reason=detail::evidence_error(evidence);!reason.empty())return reject(std::move(reason));
    if(detail::qa_directory())return reject("qa-profile-marker");
    // Reload before commit so an externally corrupted ledger is never replaced,
    // and already recorded tokens remain once-only after a caller reload/retry.
    detail::State current;
    try{current=detail::read_disk();}catch(const std::exception& e){detail::state.view.readable=detail::state.view.saved=false;return reject(e.what());}
    detail::state=std::move(current);
    const auto token=detail::normalized(evidence.token);
    if(detail::state.runs.contains(token))return reject("duplicate-run-token");
    if(detail::state.runs.size()>=detail::max_runs)return reject("ledger-capacity-reached");
    auto next=detail::state;next.runs.emplace(token,evidence.color);next.view.counts14[evidence.color]++;next.view.mask14|=uint16_t(1u<<evidence.color);next.view.reason.clear();
    try{detail::atomic_replace(detail::encode(next));detail::state=std::move(next);return true;}
    catch(const std::exception& e){detail::state.view.saved=false;return reject(e.what());}
}
}
