#include "story_rights0320.hpp"
#include <fstream>
#include <system_error>

namespace workshop::story_rights0320 {
namespace {
constexpr uint32_t current_magic=0x81317539,version8_magic=0x81317538,vanilla_magic=0x81317531;
constexpr uint64_t compressed_limit=0x7FC3A4111000ull; //228^6 legal names.
unsigned be16(std::span<const uint8_t> p,size_t offset){return unsigned(p[offset])<<8|p[offset+1];}
uint32_t be24(std::span<const uint8_t> p,size_t offset){return uint32_t(p[offset])<<16|uint32_t(p[offset+1])<<8|p[offset+2];}
uint32_t be32(std::span<const uint8_t> p,size_t offset){return uint32_t(p[offset])<<24|uint32_t(p[offset+1])<<16|uint32_t(p[offset+2])<<8|p[offset+3];}
uint16_t checksum(std::span<const uint8_t> p){
    uint16_t value=0;
    for(size_t i=0;i<1018;i++){
        value=uint16_t(value^p[i]);
        value=uint16_t((unsigned(value)<<1)|(value>>15));
    }
    return value;
}
bool decode_copy(std::span<const uint8_t> p,unsigned copy,View& result,std::string& reason){
    const auto magic=be32(p,1020);
    if(magic!=current_magic&&magic!=version8_magic&&magic!=vanilla_magic){reason="magic-invalid";return false;}
    if(be16(p,1018)!=checksum(p)){reason="checksum-invalid";return false;}
    const auto story=be24(p,1);
    progress::State state{0,255,{255,255,255,255},0,63};
    if(magic!=vanilla_magic){
        std::array<uint8_t,6> meta{};
        for(unsigned record=0;record<125;record++){
            const size_t offset=6+size_t(record)*8;
            uint64_t name=p[offset]&0x7Fu;
            for(unsigned i=1;i<6;i++)name=(name<<8)|p[offset+i];
            if(name>=compressed_limit){reason="compressed-name-invalid";return false;}
            if(record<48)meta[record/8]|=uint8_t((p[offset]>>7)<<(7-(record%8)));
        }
        if(!(meta[3]&0x80)){reason="extension-marker-invalid";return false;}
        if(magic==version8_magic)meta[5]=0xFC;
        state.trial=uint32_t(meta[0])<<16|uint32_t(meta[1])<<8|meta[2];
        const auto bits=unsigned(meta[3])<<8|meta[4];
        auto color=[](unsigned value){return uint8_t(value==7?255:value);};
        state.first=color(bits&7);
        for(unsigned i=0;i<4;i++)state.columns[i]=color((bits>>(3+3*i))&7);
        state.eggs=meta[5]&3;state.lives=meta[5]>>2;
        if(!progress::valid(state,story)){reason="extension-state-invalid";return false;}
        if(magic==version8_magic)state.lives=uint8_t(((p[0]&3)<<3)|(p[1015]&7)|32);
    }
    result.present=true;result.readable=true;result.copy=copy;result.magic=magic;
    result.story=story;result.trial=state.trial;result.progression=state;
    result.legacy_owned=progress::unlocked(state,story);result.bw_earned=state.eggs;
    result.raw_flags=p[0];
    for(size_t i=0;i<result.continuation.size();i++)result.continuation[i]=p[1004+i];
    result.continuing=(be16(p,1016)&7)!=0&&(p[1015]&8)!=0;
    result.reason.clear();return true;
}
View invalid(const char* why){View result;result.present=true;result.readable=false;result.reason=why;return result;}
}
View decode(std::span<const uint8_t> eeprom){
    if(eeprom.size()!=2048)return invalid("eeprom-size-invalid");
    View result;std::string reasons;
    for(unsigned copy=0;copy<2;copy++){
        std::string reason;
        if(decode_copy(eeprom.subspan(copy*1024,1024),copy,result,reason))return result;
        if(!reasons.empty())reasons+=';';reasons+="copy"+std::to_string(copy)+":"+reason;
    }
    result=invalid("no-valid-eeprom-copy");result.reason+=';'+reasons;return result;
}
View read(const std::filesystem::path& path){
    try{
        std::error_code error;const auto status=std::filesystem::symlink_status(path,error);
        if(error==std::errc::no_such_file_or_directory||(!error&&!std::filesystem::exists(status)))return {};
        if(error)return invalid("eeprom-status-failed");
        if(!std::filesystem::is_regular_file(status)||std::filesystem::is_symlink(status))return invalid("eeprom-not-regular-file");
        if(std::filesystem::file_size(path)!=2048)return invalid("eeprom-size-invalid");
        std::ifstream file(path,std::ios::binary);if(!file)return invalid("eeprom-open-failed");
        std::array<uint8_t,2049> bytes{};file.read(reinterpret_cast<char*>(bytes.data()),std::streamsize(bytes.size()));
        if(file.gcount()!=2048||(!file.eof()&&!file))return invalid("eeprom-read-failed");
        return decode(std::span<const uint8_t>(bytes.data(),2048));
    }catch(...){return invalid("eeprom-read-failed");}
}
}
