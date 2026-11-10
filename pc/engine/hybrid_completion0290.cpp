#include "hybrid_completion0290.hpp"
#include "json/json.hpp"
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <optional>
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

namespace workshop::hybrid_completion0290 {
namespace {
namespace fs=std::filesystem;
struct Data {uint32_t observed=0,legacy=0;bool checked=false;std::optional<uint32_t> checksum;};
std::mutex mutex;
std::atomic_uint64_t serial{0};
Data data;
View current;
fs::path filename,legacy_filename;
bool initialized=false,present=false;
unsigned session=0,attempt_level=24;
uint32_t attempt_player=0,attempt_private=0,pending=0;
bool armed=false,finished=false;
void reset_attempt(){attempt_level=24;attempt_player=attempt_private=0;armed=finished=false;}
void publish(){current.mask=data.observed|data.legacy;current.count=unsigned(std::popcount(current.mask));current.legacy_mask=data.legacy;current.legacy_checked=data.checked;}
bool exists_regular(const fs::path& path){
    std::error_code error;const auto status=fs::symlink_status(path,error);
    if(error&&error!=std::errc::no_such_file_or_directory)throw std::runtime_error("file-status-failed");
    if(!fs::exists(status))return false;
    if(!fs::is_regular_file(status)||fs::is_symlink(status))throw std::runtime_error("not-regular-file");
    return true;
}
nlohmann::json read_json(const fs::path& path,size_t maximum){
    if(fs::file_size(path)>maximum)throw std::runtime_error("file-too-large");
    std::ifstream input(path,std::ios::binary);if(!input)throw std::runtime_error("file-open-failed");
    std::string bytes(maximum+1,'\0');input.read(bytes.data(),std::streamsize(bytes.size()));bytes.resize(size_t(input.gcount()));
    if(bytes.size()>maximum||(!input.eof()&&!input))throw std::runtime_error("file-read-failed");
    bool duplicate=false;std::vector<std::set<std::string>> keys;
    auto callback=[&](int,nlohmann::json::parse_event_t event,nlohmann::json& parsed){
        if(event==nlohmann::json::parse_event_t::object_start)keys.emplace_back();
        else if(event==nlohmann::json::parse_event_t::key){if(keys.empty()||!keys.back().insert(parsed.get<std::string>()).second)duplicate=true;}
        else if(event==nlohmann::json::parse_event_t::object_end){if(!keys.empty())keys.pop_back();}
        return true;
    };
    auto json=nlohmann::json::parse(bytes,callback);if(duplicate)throw std::runtime_error("duplicate-json-key");return json;
}
bool natural(const nlohmann::json& value,uint32_t maximum){return value.is_number_unsigned()&&value.get<uint64_t>()<=maximum;}
Data read_journal(bool allow_missing,bool& found){
    found=exists_regular(filename);if(!found){if(!allow_missing)throw std::runtime_error("journal-disappeared");return {};}
    const auto json=read_json(filename,4096);
    if(!json.is_object()||json.size()!=5||!json.contains("schema")||!natural(json["schema"],1)||json["schema"]!=1u||
       !json.contains("observedMask")||!natural(json["observedMask"],all_levels)||!json.contains("legacyMask")||!natural(json["legacyMask"],all_levels)||
       !json.contains("legacyChecked")||!json["legacyChecked"].is_boolean()||!json.contains("legacyChecksum")||
       (!json["legacyChecksum"].is_null()&&!natural(json["legacyChecksum"],UINT32_MAX)))throw std::runtime_error("journal-format-invalid");
    Data result{json["observedMask"].get<uint32_t>(),json["legacyMask"].get<uint32_t>(),json["legacyChecked"].get<bool>(),{}};
    if(!json["legacyChecksum"].is_null())result.checksum=json["legacyChecksum"].get<uint32_t>();
    if((!result.checked&&(result.legacy||result.checksum))||(result.legacy&&!result.checksum))throw std::runtime_error("journal-provenance-invalid");
    return result;
}
void import_legacy(Data& next){
    if(next.checked)return;
    if(!exists_regular(legacy_filename)){next.checked=true;return;}
    const auto json=read_json(legacy_filename,32768);
    if(!json.is_object()||json.size()!=3||!json.contains("version")||!natural(json["version"],1)||json["version"]!=1u||
       !json.contains("records")||!json["records"].is_array()||json["records"].size()!=1000||
       !json.contains("checksum")||!natural(json["checksum"],UINT32_MAX))throw std::runtime_error("legacy-record-format-invalid");
    std::array<uint8_t,1000> records{};uint32_t checksum=2166136261u;
    for(unsigned i=0;i<records.size();++i){if(!natural(json["records"][i],255))throw std::runtime_error("legacy-record-byte-invalid");records[i]=json["records"][i].get<uint8_t>();checksum=(checksum^records[i])*16777619u;}
    if(checksum!=json["checksum"].get<uint32_t>())throw std::runtime_error("legacy-record-checksum-invalid");
    // Original Name Entry 41E0's five glyph lists, also used by the lossless
    // codec at807F2100. Source/proof checked by verify-hybrid-completion0290.py.
    constexpr std::array<uint64_t,4> glyph_bits{0x380FFFFFFFFF0001ull,0xFFFFFFFFFFFFFFF8ull,0xFFFFFFFFFFFFFFFFull,0x7FFFFFFFFFFFFFFFull};
    for(unsigned row=0;row<125;++row)for(unsigned n=0;n<6;++n){const auto glyph=records[row*8+2+n];if(!(glyph_bits[glyph/64]&(uint64_t(1)<<(glyph%64))))throw std::runtime_error("legacy-record-glyph-invalid");}
    uint32_t mask=0;
    // Native packing:40 Story bytes (5x8), then960 Trial bytes (24x5x8).
    // Each name getter69684/696D4 uses rootFEA+level*40+rank*8;
    // the two-byte big-endian score immediately precedes that name.
    for(unsigned level=0;level<24;++level)for(unsigned rank=0;rank<5;++rank){const unsigned at=(5+level*5+rank)*8;if(records[at]||records[at+1])mask|=1u<<level;}
    next.legacy=mask;next.checked=true;next.checksum=checksum;
}
class FileLock {
#ifdef _WIN32
    HANDLE handle=INVALID_HANDLE_VALUE;
#else
    int handle=-1;
#endif
public:
    explicit FileLock(const fs::path& file){
        auto lock=file;lock+=".lock";const bool exists=exists_regular(lock);(void)exists;
#ifdef _WIN32
        handle=CreateFileW(lock.c_str(),GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error("journal-lock-unavailable");
#else
        handle=::open(lock.c_str(),O_RDWR|O_CREAT|O_CLOEXEC|O_NOFOLLOW,0600);if(handle<0)throw std::runtime_error("journal-lock-unavailable");
        if(::flock(handle,LOCK_EX|LOCK_NB)!=0){::close(handle);handle=-1;throw std::runtime_error("journal-lock-unavailable");}
#endif
    }
    ~FileLock(){
#ifdef _WIN32
        if(handle!=INVALID_HANDLE_VALUE)CloseHandle(handle);
#else
        if(handle>=0){::flock(handle,LOCK_UN);::close(handle);}
#endif
    }
    FileLock(const FileLock&)=delete;FileLock& operator=(const FileLock&)=delete;
};
void atomic_replace(const Data& next){
    const nlohmann::json checksum=next.checksum?nlohmann::json(*next.checksum):nlohmann::json(nullptr);
    const auto bytes=nlohmann::json{{"schema",1u},{"observedMask",next.observed},{"legacyMask",next.legacy},{"legacyChecked",next.checked},{"legacyChecksum",checksum}}.dump(2)+'\n';
    auto temp=filename;temp+=".tmp-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(serial.fetch_add(1));
    try{
#ifdef _WIN32
        HANDLE handle=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error("journal-temp-create-failed");
        DWORD written=0;const bool flushed=WriteFile(handle,bytes.data(),DWORD(bytes.size()),&written,nullptr)&&written==bytes.size()&&FlushFileBuffers(handle);
        const bool closed=CloseHandle(handle)!=0;if(!flushed||!closed)throw std::runtime_error("journal-temp-flush-failed");
        if(!MoveFileExW(temp.c_str(),filename.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("journal-replace-failed");
#else
        const int handle=::open(temp.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(handle<0)throw std::runtime_error("journal-temp-create-failed");
        size_t offset=0;bool good=true;
        while(offset<bytes.size()){const auto count=::write(handle,bytes.data()+offset,bytes.size()-offset);if(count<0&&errno==EINTR)continue;if(count<=0){good=false;break;}offset+=size_t(count);}
        const bool flushed=good&&::fsync(handle)==0;const bool closed=::close(handle)==0;if(!flushed||!closed)throw std::runtime_error("journal-temp-flush-failed");fs::rename(temp,filename);
#endif
    }catch(...){std::error_code ignored;fs::remove(temp,ignored);throw;}
}
bool commit(bool initialize_only){
    try{
        FileLock lock(filename);bool found=false;Data next;
        try{next=read_journal(!present,found);}catch(...){current.readable=false;throw;}
        try{import_legacy(next);}catch(...){current.readable=false;current.legacy_readable=false;throw;}
        const auto added=pending&~(next.observed|next.legacy);next.observed|=added;
        // Missing source+no observation requires no new file. A present source
        // writes its one-time, validated migration even if every score is0.
        const bool needs_write=!found?(!initialize_only||next.checksum.has_value()):
            added!=0||next.checked!=data.checked||next.legacy!=data.legacy||next.checksum!=data.checksum;
        if(needs_write)atomic_replace(next);
        data=next;present=found||needs_write;pending=0;current.saved=true;current.reason.clear();publish();return true;
    }catch(const std::exception& error){current.saved=false;current.reason=error.what();fprintf(stderr,"[HybridCompletion] preserved reason=%s\n",error.what());return false;}
}
}
void initialize(const fs::path& path,const fs::path& legacy_records){
    std::lock_guard guard(mutex);filename=path;legacy_filename=legacy_records.empty()?path.parent_path()/"pc-hybrid-records.json":legacy_records;
    data={};current={};pending=0;initialized=present=false;session=0;reset_attempt();
    try{
        if(filename.empty()||filename.lexically_normal()==legacy_filename.lexically_normal())throw std::runtime_error("journal-path-invalid");
        bool found=false;data=read_journal(true,found);present=found;initialized=true;publish();
        (void)commit(true);
    }catch(const std::exception& error){current.readable=current.saved=false;current.reason=error.what();fprintf(stderr,"[HybridCompletion] preserved reason=%s\n",error.what());}
}
void start_session(unsigned mode){std::lock_guard guard(mutex);session=mode;reset_attempt();}
View view(){std::lock_guard guard(mutex);return current;}
void frame(uint8_t* rdram,recomp_context* context,bool allowed){
    constexpr auto root=gpr(int32_t(0x800F7EA0));
    if(!rdram||!context||uint32_t(context->r4)!=uint32_t(root))return;
    std::lock_guard guard(mutex);
    if(!initialized||!current.readable||!allowed||session!=3){reset_attempt();return;}
    if(pending)(void)commit(false);
    if(!current.readable)return;
    const auto mode=uint32_t(MEM_W(0,gpr(int32_t(0x800F8B88))));
    const auto page=MEM_BU(0,gpr(int32_t(0x800F8DFC))),column=MEM_BU(0,gpr(int32_t(0x800F8DFD)));
    const auto player=uint32_t(MEM_W(0,gpr(int32_t(0x800F8828))));
    const auto actor=static_cast<gpr>(static_cast<int32_t>(player));
    if(mode!=1||page>=6||column>=4||player<0x80000400||player>0x807FFE00||(player&3)||MEM_W(0,actor)!=0x19971110||MEM_HU(12,actor)!=0x4001){reset_attempt();return;}
    const auto private_data=uint32_t(MEM_W(0x1F4,actor));
    if(private_data<0x80000400||private_data>0x807FED00||(private_data&3)){reset_attempt();return;}
    const unsigned level=unsigned(page)*4+unsigned(column);
    if(attempt_level!=level||attempt_player!=player||attempt_private!=private_data){reset_attempt();attempt_level=level;attempt_player=player;attempt_private=private_data;}
    const auto fruits=uint32_t(MEM_W(0,gpr(int32_t(0x800F89D0)))),ending=uint32_t(MEM_W(0,gpr(int32_t(0x800F8AC8))));
    if(fruits==0&&ending==0){armed=true;finished=false;return;}
    if(!armed||finished||fruits!=30||ending!=1)return;
    finished=true;pending|=1u<<level;
    if(commit(false))fprintf(stderr,"[HybridCompletion] level=%u-%u observed=1 mask=%06X count=%u saved=1\n",unsigned(page)+1,unsigned(column)+1,current.mask,current.count);
}
}
