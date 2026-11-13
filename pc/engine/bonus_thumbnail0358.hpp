#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <system_error>
#include <vector>
#include <utility>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

namespace workshop::bonus_thumbnail0358 {
inline constexpr size_t max_bytes=1024*1024;
enum class Status { Locked,Missing,Invalid,Ready,RestartRequired };
struct ReadResult {Status status=Status::Missing;unsigned width=0,height=0;std::vector<char> png;};
inline ReadResult outcome(Status status){ReadResult result;result.status=status;return result;}
inline uint32_t u32(std::span<const char> b,size_t p){return(uint32_t(uint8_t(b[p]))<<24)|(uint32_t(uint8_t(b[p+1]))<<16)|(uint32_t(uint8_t(b[p+2]))<<8)|uint8_t(b[p+3]);}
inline uint32_t crc(std::span<const char> b){uint32_t c=0xFFFFFFFFu;for(unsigned char v:b){c^=v;for(unsigned k=0;k<8;++k)c=(c>>1)^(0xEDB88320u&uint32_t(0-int(c&1)));}return c^0xFFFFFFFFu;}
inline bool dimensions(unsigned w,unsigned h){return(w==320&&h==180)||(w==640&&h==360);}
// This structural check bounds the later stb decode. Imported pixels are not
// proof that the screenshot or a Bonus course was validated in natural play.
inline bool png_shape(std::span<const char> b,unsigned& width,unsigned& height){
 if(b.size()<57||b.size()>max_bytes)return false;
 constexpr std::array<uint8_t,8> signature{137,80,78,71,13,10,26,10};
 for(size_t i=0;i<signature.size();++i)if(uint8_t(b[i])!=signature[i])return false;
 bool header=false,data=false;unsigned metadata=0,chunks=0;size_t p=8;
 while(p<b.size()){
  if(++chunks>1024||b.size()-p<12)return false;
  const uint32_t size=u32(b,p);if(size>b.size()-p-12)return false;
  const std::string name(b.data()+p+4,4);const size_t payload=p+8;
  if(crc(b.subspan(p+4,size_t(size)+4))!=u32(b,p+8+size))return false;
  if(!header){
   if(name!="IHDR"||size!=13)return false;
   width=u32(b,payload);height=u32(b,payload+4);
   if(!dimensions(width,height)||uint8_t(b[payload+8])!=8||(uint8_t(b[payload+9])!=2&&uint8_t(b[payload+9])!=6)||b[payload+10]!=0||b[payload+11]!=0||b[payload+12]!=0)return false;
   header=true;
  }else if(name=="IDAT"){
   if(size==0)return false;data=true;
  }else if(name=="IEND")return data&&size==0&&p+12==b.size();
  else{
   if(name=="IHDR"||data)return false;
   unsigned flag=0;uint32_t expected=0;
   if(name=="gAMA"){flag=1;expected=4;}
   else if(name=="cHRM"){flag=2;expected=32;}
   else if(name=="sRGB"){flag=4;expected=1;}
   else if(name=="pHYs"){flag=8;expected=9;}
   else return false;
   if((metadata&flag)||size!=expected)return false;metadata|=flag;
  }
  p+=size_t(size)+12;
 }
 return false;
}
inline bool link_or_error(const std::filesystem::path& path){
 std::error_code ec;const auto state=std::filesystem::symlink_status(path,ec);
 if(ec||std::filesystem::is_symlink(state))return true;
#ifdef _WIN32
 const DWORD attrs=GetFileAttributesW(path.c_str());if(attrs==INVALID_FILE_ATTRIBUTES)return true;
 if(attrs&FILE_ATTRIBUTE_REPARSE_POINT){
  const HANDLE h=CreateFileW(path.c_str(),FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS,nullptr);
  if(h==INVALID_HANDLE_VALUE)return true;
  FILE_ATTRIBUTE_TAG_INFO tag{};const bool ok=GetFileInformationByHandleEx(h,FileAttributeTagInfo,&tag,sizeof(tag))!=0;CloseHandle(h);
  if(!ok||tag.ReparseTag==IO_REPARSE_TAG_SYMLINK||tag.ReparseTag==IO_REPARSE_TAG_MOUNT_POINT)return true;
 }
#endif
 return false;
}
inline ReadResult read(const std::filesystem::path& pc_root,unsigned bonus,bool entitled){
 // Locked courses never resolve a filename, stat, open or decode an image.
 if(!entitled)return outcome(Status::Locked);
 if(bonus<1||bonus>6||pc_root.empty())return outcome(Status::Invalid);
 try{
  const auto root=std::filesystem::absolute(pc_root).lexically_normal();
  const auto content=root/"content",folder=content/"bonus-thumbnails0358",file=folder/("7-"+std::to_string(bonus)+".png");
  for(const auto& p:{content,folder,file}){
   std::error_code ec;const auto state=std::filesystem::symlink_status(p,ec);
   if(ec==std::errc::no_such_file_or_directory||(!ec&&!std::filesystem::exists(state)))return outcome(Status::Missing);
   if(ec||link_or_error(p))return outcome(Status::Invalid);
   if(p!=file&&!std::filesystem::is_directory(state))return outcome(Status::Invalid);
   if(p==file&&!std::filesystem::is_regular_file(state))return outcome(Status::Invalid);
  }
  std::error_code ec;const auto size=std::filesystem::file_size(file,ec);
  if(ec||size<57||size>max_bytes)return outcome(Status::Invalid);
  std::ifstream stream(file,std::ios::binary);if(!stream)return outcome(Status::Invalid);
  ReadResult result;result.png.resize(size_t(size));stream.read(result.png.data(),std::streamsize(size));
  if(stream.gcount()!=std::streamsize(size)||stream.bad())return outcome(Status::Invalid);
  char extra=0;if(stream.get(extra)||stream.bad())return outcome(Status::Invalid);
  if(!png_shape(result.png,result.width,result.height))return outcome(Status::Invalid);
  result.status=Status::Ready;return result;
 }catch(const std::exception&){return outcome(Status::Invalid);}
}
struct View {Status status=Status::Missing;std::string key;};
// The frontend's file-image map uses emplace and has no CPU-byte eviction API.
// Retain at most six immutable, bounded payloads for this process. A changed
// slot is hidden until restart instead of displaying stale pixels or creating
// unbounded content-addressed texture names. No frontend/submodule change.
class Cache {
 struct Entry {std::vector<char> png;bool queued=false;};
 std::array<Entry,6> entries{};
public:
 static std::string key(unsigned bonus){return "?/workshop/bonus-thumbnail0358-"+std::to_string(bonus);}
 void new_context(){for(auto& entry:entries)entry.queued=false;}
 size_t remembered_slots()const{size_t n=0;for(const auto& e:entries)n+=!e.png.empty();return n;}
 size_t remembered_bytes()const{size_t n=0;for(const auto& e:entries)n+=e.png.size();return n;}
 template<class Queue> View consider(unsigned bonus,const ReadResult& image,Queue&& queue){
  if(image.status!=Status::Ready)return{image.status,{}};
  if(bonus<1||bonus>6||!dimensions(image.width,image.height)||image.png.empty()||image.png.size()>max_bytes)return{Status::Invalid,{}};
  auto& entry=entries[bonus-1];
  if(!entry.png.empty()&&entry.png!=image.png)return{Status::RestartRequired,{}};
  if(!entry.queued){
   std::vector<char> first=entry.png.empty()?image.png:std::vector<char>{};
   if(!queue(key(bonus),image))return{Status::Invalid,{}};
   if(entry.png.empty())entry.png=std::move(first);
   entry.queued=true;
  }
  return{Status::Ready,key(bonus)};
 }
};
} // namespace workshop::bonus_thumbnail0358
