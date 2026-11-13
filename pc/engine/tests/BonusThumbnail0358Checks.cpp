#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#pragma clang diagnostic pop
#include "../bonus_thumbnail0358.hpp"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <limits>
#include <set>
#include <stdexcept>

namespace n=workshop::bonus_thumbnail0358;
namespace fs=std::filesystem;
// The full engine includes this global alias from librecomp/rsp_vu.hpp.
using u32=uint32_t;
static unsigned checks=0;static unsigned queue_calls=0;static std::set<std::string> queued_keys;static std::vector<char> queued_rgba;
static void need(bool value,const char* reason){++checks;if(!value)throw std::runtime_error(reason);}
namespace recompui {inline void queue_image_from_bytes_rgba32(const std::string& key,const std::vector<char>& data,uint32_t w,uint32_t h){need(n::dimensions(w,h)&&data.size()==size_t(w)*h*4,"Actual UI callback queued unbounded pixels");++queue_calls;queued_keys.insert(key);queued_rgba=data;}}
static fs::path pc_root;
static n::Cache bonus_thumbnail_cache0358;
struct MockImage {bool visible=true;std::string source="old";void display_hide(){visible=false;}void set_src(std::string_view text){source=text;}};
static MockImage* bonus_thumbnail_image0358=nullptr;
static unsigned png_decode_calls=0;
static stbi_uc* tracked_stbi_load(const stbi_uc* b,int size,int* x,int* y,int* c,int desired){++png_decode_calls;return stbi_load_from_memory(b,size,x,y,c,desired);}
#define stbi_load_from_memory tracked_stbi_load
#include BONUS_THUMBNAIL_ACTUAL_BODY
#undef stbi_load_from_memory

static std::vector<char> bytes(const fs::path& path){std::ifstream f(path,std::ios::binary);return{std::istreambuf_iterator<char>(f),{}};}
static void put(const fs::path& path,const std::vector<char>& data){fs::create_directories(path.parent_path());std::ofstream f(path,std::ios::binary|std::ios::trunc);f.write(data.data(),std::streamsize(data.size()));need(bool(f),"Fixture write failed");}
static void rewrite_crc(std::vector<char>& data,size_t offset){const auto len=n::u32(data,offset);uint32_t v=n::crc(std::span<const char>(data).subspan(offset+4,size_t(len)+4));for(unsigned i=0;i<4;++i)data[offset+8+len+i]=char(v>>(24-i*8));}
static void set_u32(std::vector<char>& data,size_t p,uint32_t v){for(unsigned i=0;i<4;++i)data[p+i]=char(v>>(24-i*8));}
static fs::path slot(const fs::path& root,unsigned id){return root/"content/bonus-thumbnails0358"/("7-"+std::to_string(id)+".png");}
int main(int argc,char** argv){
 try{
  need(argc==2,"Pass an isolated fixture directory");const auto root=fs::absolute(fs::path(argv[1]));pc_root=root/"pc";const auto fixtures=root/"fixtures";
  const auto rgba=bytes(fixtures/"rgba320.png"),rgb=bytes(fixtures/"rgb640.png"),changed=bytes(fixtures/"changed320.png"),bad_zlib=bytes(fixtures/"bad-zlib.png");
  need(!rgba.empty()&&!rgb.empty()&&!changed.empty(),"Independent PNG fixtures missing");
  const std::string crc_text="123456789";need(n::crc({crc_text.data(),crc_text.size()})==0xCBF43926u,"CRC standard control failed");
  unsigned w=0,h=0;need(n::png_shape(rgba,w,h)&&w==320&&h==180,"RGBA320 fixture rejected");need(n::png_shape(rgb,w,h)&&w==640&&h==360,"RGB640 fixture rejected");
  need(n::read({},1,false).status==n::Status::Locked,"Locked early return touched invalid root");
  need(n::read(pc_root,0,true).status==n::Status::Invalid&&n::read(pc_root,7,true).status==n::Status::Invalid,"Slot range escaped fixed names");
  need(n::read({},1,true).status==n::Status::Invalid,"Empty root accepted");
  need(n::read(pc_root,1,true).status==n::Status::Missing,"Missing library not reported");
  const auto target=slot(pc_root,1);put(target,rgba);
  need(n::read(pc_root,1,false).status==n::Status::Locked,"Locked course image was exposed");
  const auto loaded=n::read(pc_root,1,true);need(loaded.status==n::Status::Ready&&loaded.png==rgba,"Read-only PNG bytes changed");need(bytes(target)==rgba,"Reader modified PNG");
  auto view=read_bonus_thumbnail0358(1,true);need(view.status==n::Status::Ready&&view.key==n::Cache::key(1),"Actual UI reader did not expose valid image");need(queue_calls==1&&queued_rgba.size()==320*180*4,"Actual stb decode/queue missing");need(uint8_t(queued_rgba[0])==7&&uint8_t(queued_rgba[1])==31&&uint8_t(queued_rgba[2])==53&&uint8_t(queued_rgba[3])==255,"Actual queue pixels differ from independent source fixture");
  const auto count=queue_calls;view=read_bonus_thumbnail0358(1,false);need(view.status==n::Status::Locked&&queue_calls==count,"Locked visit queued prior image");
  for(unsigned i=0;i<100;++i){view=bonus_thumbnail_cache0358.consider(1,loaded,[](const std::string&,const n::ReadResult&){need(false,"Unchanged image queued again");return false;});need(view.status==n::Status::Ready&&queue_calls==count,"Unchanged image was queued repeatedly");}
  for(unsigned id=2;id<=6;++id){put(slot(pc_root,id),id%2?rgba:rgb);view=read_bonus_thumbnail0358(id,true);need(view.status==n::Status::Ready,"Valid slot failed");}
  need(queue_calls==6&&queued_keys.size()==6&&bonus_thumbnail_cache0358.remembered_slots()==6,"Cache did not keep exactly six stable assets");
  const auto held=bonus_thumbnail_cache0358.remembered_bytes();
  std::array<n::ReadResult,6> replacements{};for(unsigned id=1;id<=6;++id){put(slot(pc_root,id),changed);view=read_bonus_thumbnail0358(id,true);need(view.status==n::Status::RestartRequired&&view.key.empty(),"Actual menu showed replaced image");replacements[id-1]=n::read(pc_root,id,true);}
  for(unsigned i=0;i<1000;++i){const unsigned id=i%6+1;view=bonus_thumbnail_cache0358.consider(id,replacements[id-1],[](const std::string&,const n::ReadResult&){need(false,"Replacement queued a new asset");return false;});need(view.status==n::Status::RestartRequired&&view.key.empty(),"Changed slot displayed a stale photo");need(queue_calls==6&&queued_keys.size()==6&&bonus_thumbnail_cache0358.remembered_bytes()==held,"Repeated replacements grew cache/key allocations");}
  for(unsigned id=1;id<=6;++id)put(slot(pc_root,id),id==1||id%2?rgba:rgb);
  bonus_thumbnail_cache0358.new_context();for(unsigned id=1;id<=6;++id){view=read_bonus_thumbnail0358(id,true);need(view.status==n::Status::Ready,"New context could not rebind same immutable image");}
  need(queue_calls==12&&queued_keys.size()==6&&bonus_thumbnail_cache0358.remembered_bytes()==held,"Context recreation generated new asset keys");
  MockImage image;bonus_thumbnail_image0358=&image;hide_bonus_thumbnail0358();need(!image.visible&&image.source.empty(),"Actual panel reset left hidden image source exposed");bonus_thumbnail_image0358=nullptr;hide_bonus_thumbnail0358();
  // Corrupt, truncated, oversized and unsupported images stay intact and never queue.
  const auto root_bad=root/"bad";pc_root=root_bad;const auto bad_target=slot(pc_root,1);const unsigned queued_before=queue_calls;
  for(size_t end=0;end<rgba.size();++end){std::vector<char> partial(rgba.begin(),rgba.begin()+end);need(!n::png_shape(partial,w,h),"Truncated PNG parser accepted bytes");}
  for(size_t pos=0;pos<rgba.size();++pos){auto invalid=rgba;invalid[pos]^=1;need(!n::png_shape(invalid,w,h),"Corrupt PNG parser accepted bytes");}
  auto corrupt=rgba;corrupt[corrupt.size()/2]^=1;put(bad_target,corrupt);need(read_bonus_thumbnail0358(1,true).status==n::Status::Invalid&&bytes(bad_target)==corrupt,"Actual corrupt-file read changed bytes or queued image");
  for(const auto dims:std::array<std::array<uint32_t,2>,6>{{{0,180},{320,0},{640,180},{320,360},{0x7FFFFFFF,180},{320,0x7FFFFFFF}}}){auto invalid=rgba;set_u32(invalid,16,dims[0]);set_u32(invalid,20,dims[1]);rewrite_crc(invalid,8);put(bad_target,invalid);need(read_bonus_thumbnail0358(1,true).status==n::Status::Invalid,"Unsupported/oversized dimensions reached decoder");}
  for(unsigned pos:{24u,25u,26u,27u,28u}){auto invalid=rgba;invalid[pos]=char(pos==24?16:pos==25?3:1);rewrite_crc(invalid,8);put(bad_target,invalid);need(read_bonus_thumbnail0358(1,true).status==n::Status::Invalid,"Unsupported PNG format reached UI");}
  auto trailing=rgba;trailing.push_back('x');put(bad_target,trailing);need(read_bonus_thumbnail0358(1,true).status==n::Status::Invalid,"Trailing PNG bytes accepted");
  auto large=rgba;large.resize(n::max_bytes+1,'x');put(bad_target,large);need(read_bonus_thumbnail0358(1,true).status==n::Status::Invalid,"Oversized file accepted");
  bonus_thumbnail_cache0358=n::Cache{};need(n::png_shape(bad_zlib,w,h),"Bad-compression control lacks correct PNG structure");put(bad_target,bad_zlib);need(read_bonus_thumbnail0358(1,true).status==n::Status::Invalid,"Actual stb decoder accepted broken compression");need(queue_calls==queued_before,"Invalid image was queued");
  // A bad first import never poisons the immutable slot.
  bonus_thumbnail_cache0358=n::Cache{};put(bad_target,bad_zlib);need(read_bonus_thumbnail0358(1,true).status==n::Status::Invalid&&bonus_thumbnail_cache0358.remembered_slots()==0,"Decode failure poisoned slot");put(bad_target,changed);need(read_bonus_thumbnail0358(1,true).status==n::Status::Ready,"Valid replacement after failed first decode rejected");
  // The old PNG malloc path accepts extra expanded row bytes. The fixed-output
  // precheck must refuse those and a larger expansion before that loader runs.
  const auto extra=bytes(fixtures/"extra-expanded.png");int old_w=0,old_h=0,old_c=0;
  auto* legacy=stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(extra.data()),int(extra.size()),&old_w,&old_h,&old_c,4);need(legacy!=nullptr,"Legacy extra-row control not reproduced");stbi_image_free(legacy);
  for(const char* name:{"extra-expanded.png","expanded-bomb.png","filter-invalid.png","adler-invalid.png"}){
   bonus_thumbnail_cache0358=n::Cache{};const auto invalid=bytes(fixtures/name);need(n::png_shape(invalid,w,h),"Bounded-inflate control has invalid structural PNG");put(bad_target,invalid);const unsigned decodes=png_decode_calls,queues=queue_calls;
   need(read_bonus_thumbnail0358(1,true).status==n::Status::Invalid,"Unbounded expansion/filter/adler image accepted");need(png_decode_calls==decodes&&queue_calls==queues&&bonus_thumbnail_cache0358.remembered_slots()==0,"Invalid stream reached malloc PNG loader or poisoned slot");need(bytes(bad_target)==invalid,"Invalid stream file modified");
  }
  // Driver-created directory junction points to a valid fixture. It must not be followed.
  const auto linked=root/"junction-pc";need(n::read(linked,1,true).status==n::Status::Invalid,"Directory junction was followed");need(n::read(linked,1,false).status==n::Status::Locked,"Locked visit inspected a junction");
  pc_root=root/fs::path(u8"captures é espaces");put(slot(pc_root,6),rgba);need(n::read(pc_root,6,true).status==n::Status::Ready,"Unicode and spaced library root failed");
  pc_root=root/"non-directory";put(pc_root/"content",std::vector<char>{'x'});need(n::read(pc_root,1,true).status==n::Status::Invalid,"Non-directory content accepted");
  std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"fixedSlots\":6,\"reimportCycles\":1000,\"maxDistinctQueuedAssetKeys\":6,\"actualMenuReadAndHideBodies\":true,\"actualStbPngDecode\":true,\"fixedOutputInflateBeforePngMalloc\":true,\"legacyExtraExpandedRowReproduced\":true,\"windowsJunctionRefused\":true,\"lockedImagesNeverExposed\":true,\"personalOrGameUsed\":false}";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
