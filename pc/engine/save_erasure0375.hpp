#pragma once
#include <array>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cstdint>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif
namespace workshop::save_erasure0375 {
namespace fs=std::filesystem;
inline constexpr const char* marker="save-erased0375.txt";
inline constexpr const char* signature="DinoTale confirmed active-profile erasure 0375";
struct Request {unsigned parent=0;std::string token;bool pending=false;};
inline void require(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
inline bool token_valid(const std::string& s){return s.size()==32&&s.find_first_not_of("0123456789abcdef")==std::string::npos;}
inline bool linked(const fs::path& p){
 const auto s=fs::symlink_status(p);if(fs::is_symlink(s))return true;
#ifdef _WIN32
 const auto a=GetFileAttributesW(p.c_str());if(a!=INVALID_FILE_ATTRIBUTES&&(a&FILE_ATTRIBUTE_REPARSE_POINT))return true;
#endif
 return false;
}
inline void safe_root(const fs::path& root){
 require(root.is_absolute()&&fs::is_directory(root),"erase-profile-root");
 for(auto p=root;!p.empty();p=p.parent_path()){require(!linked(p),"erase-linked-profile");if(p==p.parent_path())break;}
 require(fs::is_regular_file(root/"portable.txt")&&!linked(root/"portable.txt"),"erase-portable-profile-required");
}
inline std::string read(const fs::path& p){require(fs::is_regular_file(p)&&!linked(p)&&fs::file_size(p)<=4096,"erase-marker-invalid");std::ifstream f(p,std::ios::binary);require(bool(f),"erase-marker-read");return{std::istreambuf_iterator<char>(f),{}};}
inline Request request(const fs::path& root){
 if(!fs::exists(fs::symlink_status(root/marker)))return{};
 const auto text=read(root/marker);const std::string prefix=std::string(signature)+"\n";
 require(text.starts_with(prefix),"erase-marker-signature");const auto state=text.substr(prefix.size());
 if(state=="done\n")return{};
 const auto a=state.find('\n'),b=state.find('\n',a+1),c=state.find('\n',b+1);
 require(a!=std::string::npos&&state.substr(0,a)=="pending"&&b!=std::string::npos&&c==state.size()-1,"erase-request-shape");
 const auto pid=state.substr(a+1,b-a-1);const auto token=state.substr(b+1,c-b-1);
 require(!pid.empty()&&pid.size()<=10&&pid.find_first_not_of("0123456789")==std::string::npos&&token_valid(token),"erase-request-fields");
 const auto n=std::stoull(pid);require(n>0&&n<=0xFFFFFFFFu,"erase-request-parent");return{unsigned(n),token,true};
}
inline void output_safe(const fs::path& p){
 require(!linked(p)&&(!fs::exists(p)||fs::is_regular_file(p)),"erase-invalid-output");auto tmp=p;tmp+=".tmp";require(!linked(tmp)&&(!fs::exists(tmp)||fs::is_regular_file(tmp)),"erase-invalid-temp");
}
inline std::string leaf(const fs::path& p){const auto s=p.filename().u8string();return {s.begin(),s.end()};}
inline void replace(const fs::path& p,const std::string& bytes){
 output_safe(p);
 auto tmp=p;tmp+=".tmp";require(!linked(tmp)&&!fs::is_directory(tmp),"erase-linked-temp");
 {std::ofstream f(tmp,std::ios::binary|std::ios::trunc);f.write(bytes.data(),std::streamsize(bytes.size()));f.flush();require(bool(f),"erase-write");}
#ifdef _WIN32
 require(MoveFileExW(tmp.c_str(),p.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0,"erase-replace");
#else
 fs::rename(tmp,p);
#endif
}
inline void prepare(const fs::path& root,unsigned parent,const std::string& token){
 safe_root(root);require(parent&&token_valid(token),"erase-confirmation-token");require(!linked(root/marker),"erase-linked-marker");
 replace(root/marker,std::string(signature)+"\npending\n"+std::to_string(parent)+"\n"+token+"\n");
}
inline void cancel(const fs::path& root,unsigned parent){const auto q=request(root);if(q.pending&&q.parent==parent)fs::remove(root/marker);}
inline bool journal(const std::string& name){
 constexpr std::array names={"pc-achievements.json","pc-collection.json","pc-wallet.json","pc-heart-wallet.json","pc-heart-wallet.lock","pc-hybrid-records.json","pc-hybrid-completion.json","pc-hybrid-finds.json","pc-metal-yoshis.json","pc-elemental-yoshis.json","pc-boss-challenges.json","pc-story-hybrid.json","pc-story-hard.json","pc-story-hard-preview.json","pc-trial-special-lives0350.json","pc-bonus-session0353.json","pc-bonus-session0352.json","menu-progress.json","menu-progress-story-hybrid.json","menu-progress-story-hard.json","menu-progress-story-hard-preview.json","menu-progress-bonus0352.json","book-session.json","custom-run-result.json","custom-session.json","manual-preview.txt","restauration.json"};
 for(const auto* base:names){const std::string s=base;if(name==s||name.starts_with(s+"."))return true;}return false;
}
inline bool yoshi_save(const std::string& name){return name.starts_with("yoshisstory.n64.us.")&&(name.ends_with(".bin")||name.find(".bin.")!=std::string::npos);}
inline std::string factory(){
 std::array<unsigned char,2048> b{};for(unsigned half:{0u,1024u}){for(unsigned n=198;n<=366;n+=8)if(n!=326)b[half+n]=128;b[half+1019]=8;b[half+1020]=0x81;b[half+1021]=0x31;b[half+1022]=0x75;b[half+1023]=0x39;}return std::string(reinterpret_cast<const char*>(b.data()),b.size());
}
// Called only after the request's previous process has terminated, before any
// journal singleton, native RAM or asynchronous EEPROM writer is initialized.
inline void erase(const fs::path& root,const Request& q){
 safe_root(root);const auto actual=request(root);require(q.pending&&actual.pending&&q.parent==actual.parent&&q.token==actual.token,"erase-unconfirmed-request");
 std::vector<fs::path> files;
 for(const auto& e:fs::directory_iterator(root))if(journal(leaf(e.path()))){require(e.is_regular_file()&&!linked(e.path()),"erase-invalid-journal");files.push_back(e.path());}
 const auto saves=root/"saves";require(!linked(saves)&&(!fs::exists(saves)||fs::is_directory(saves)),"erase-invalid-saves");
 if(fs::exists(saves))for(const auto& e:fs::directory_iterator(saves))if(yoshi_save(leaf(e.path()))){require(e.is_regular_file()&&!linked(e.path()),"erase-invalid-eeprom");files.push_back(e.path());}
 for(const auto& p:{saves/"yoshisstory.n64.us.bin",root/"pc-metal-yoshis.json",root/"pc-elemental-yoshis.json",root/"pc-hybrid-completion.json",root/marker})output_safe(p);
 for(const auto& f:files)require(fs::remove(f),"erase-remove");
 fs::create_directories(saves);replace(saves/"yoshisstory.n64.us.bin",factory());
 // These valid empty journals also stop older Workshop executables from
 // additively reimporting ownership into an explicitly erased profile.
 replace(root/"pc-metal-yoshis.json","{\"schema\":1,\"foundMask\":0,\"storyAliveMask\":0}\n");
 replace(root/"pc-elemental-yoshis.json","{\"schema\":2,\"foundMask\":0,\"storyAliveMask\":0,\"hybridAliveMask\":0}\n");
 replace(root/"pc-hybrid-completion.json","{\"schema\":1,\"observedMask\":0,\"legacyMask\":0,\"legacyChecked\":true,\"legacyChecksum\":null}\n");
 replace(root/marker,std::string(signature)+"\ndone\n");
}
template<class Wait> bool finish_after_parent(const fs::path& root,Wait wait){
 const auto q=request(root);if(!q.pending)return false;require(wait(q.parent),"erase-parent-not-closed");erase(root,q);return true;
}
}
