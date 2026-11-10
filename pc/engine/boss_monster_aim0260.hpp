#pragma once
#include "boss_native.hpp"
#include "boss_driver0260.hpp"
#include <array>
#include <bit>
#include <optional>

namespace workshop::boss_monster_aim0260 {
namespace n=boss_native;namespace d=boss_driver0260;
// Diagnostic observations only. Never invoke a native collision function: some
// similarly named bimap functions (notably6B1C8) MODIFY the terrain.
inline bool pointer(uint32_t a,unsigned bytes){return a%4==0&&n::range(a,bytes);}
inline float scalar(uint8_t* rdram,uint32_t a){return std::bit_cast<float>(n::word(rdram,a));}
struct Map {
 uint32_t descriptor=0,pages=0,data=0;unsigned columns=0,rows=0,stride=0;
 static std::optional<Map> read(uint8_t* rdram){
  if(!rdram)return {};uint32_t p=n::word(rdram,0x800F8810);
  for(unsigned offset:{0x10u,4u,0x16Cu,0x1B0u,0x20u}){if(!pointer(p,offset+4))return {};p=n::word(rdram,p+offset);}
  if(!pointer(p,0x28))return {};
  Map out;out.descriptor=p;out.columns=MEM_HU(0x12,n::base(p));out.rows=MEM_HU(0x14,n::base(p));out.stride=MEM_HU(0x16,n::base(p));out.pages=n::word(rdram,p+0x18);out.data=n::word(rdram,p+0x24);
  if(!out.columns||!out.rows||out.columns>8||out.rows>8||out.columns*out.rows>64||!out.stride||out.stride>1024||out.stride%16||!n::range(out.pages,out.columns*out.rows)||!pointer(out.data,32*out.stride))return {};
  return out;
 }
 std::optional<uint16_t> tile(uint8_t* rdram,float x,float y)const{
  if(!rdram||!d::point(x,y)||x<0||y<0||x>=columns*256.f||y>=rows*256.f)return {};
  const unsigned tx=unsigned(x)/16,ty=unsigned(y)/16,page_index=(ty/16)*columns+tx/16;
  if(page_index>=columns*rows||!n::range(pages+page_index,1))return {};
  const unsigned page=n::byte(rdram,pages+page_index);if(page==0xFF)return uint16_t(0);
  const unsigned column=page*16+tx%16;if(column>=stride)return {};
  const uint32_t at=data+2*((ty%16)*stride+column);if(!n::range(at,2))return {};return uint16_t(MEM_HU(0,n::base(at)));
 }
 std::optional<bool> occupied(uint8_t* rdram,float x,float y)const{
  const auto value=tile(rdram,x,y);if(!value)return {};
  // Native4B4D0..510: lowbyte selects one of12816-row masks, and the
  // actual pixel is profile[y&15] & (8000>>(x&15)). Material/one-way
  // handling can further relax a native collision; rejecting occupied
  // pixels here is deliberately conservative, not a physics replacement.
  if(*value==0||*value==0xFFFF)return false;
  const unsigned material=(*value>>8)&0xF8;if(material==0x38||material==0x40)return false;
  const unsigned shape=*value&0xFF;if(shape>=0x80)return {};const auto profile=n::word(rdram,0x800A0A24+4*shape);
  if(profile<0x800A0604||profile>0x800A0A04||(profile-0x800A0604)%32||!pointer(profile,32))return {};
  const auto mask=uint16_t(MEM_HU(2*(unsigned(y)&15),n::base(profile)));
  return (mask&(0x8000u>>(unsigned(x)&15)))!=0;
 }
 bool clear(uint8_t* rdram,float x,float y,float tx,float ty,float margin=3)const{
  if(!d::point(x,y)||!d::point(tx,ty)||!std::isfinite(margin)||margin<0||margin>4)return false;
  const float length=std::hypot(tx-x,ty-y);if(length>1024)return false;
  const unsigned steps=std::max(1u,unsigned(std::ceil(length/2)));const float px=length>0?-(ty-y)*margin/length:0,py=length>0?(tx-x)*margin/length:0;
  for(unsigned i=0;i<=steps;++i){const float t=float(i)/steps,at_x=x+(tx-x)*t,at_y=y+(ty-y)*t;
   for(float side:{-1.f,0.f,1.f}){const auto solid=occupied(rdram,at_x+side*px,at_y+side*py);if(!solid||*solid)return false;}
  }return true;
 }
};
struct Stand {float x=0,y=0,surface_y=336;};
inline std::optional<Stand> supply_stand(uint8_t* rdram,unsigned room,unsigned encounter){
 if(encounter>=10||room!=(encounter?167+encounter:162))return {};
 const auto map=Map::read(rdram);if(!map||map->columns!=3||map->rows!=2)return {};
 // Original151 placement464F392/360 writes this native static support.
 // Verify its actual current pixel masks before diagnostic navigation.
 for(float x:{380.f,392.f,404.f}){const auto above=map->occupied(rdram,x,335),top=map->occupied(rdram,x,336),below=map->occupied(rdram,x,338);if(!above||*above||!top||!*top||!below||!*below)return {};}
 return Stand{392,326};
}
// CP6 QA terrain1 only. Its configured middle464F was intentionally removed.
// The two retained ORIGINAL464F records, not an elapsed route or a host pose,
// identify the candidate. Native5C20 stamps3x3 at tile((actorXY-16)/16).
// The real84a51 final bitmap independently proves X224..271/Y320..367.
inline std::optional<Stand> configured_side_stand(uint8_t* rdram,unsigned room,unsigned encounter,std::span<const uint8_t> objects){
 if(!rdram||room!=168||encounter!=1||objects.empty()||objects.size()%12||objects.size()/12>149)return {};
 auto u16=[&](size_t at){return unsigned(objects[at])<<8|unsigned(objects[at+1]);};
 auto bits=[&](size_t at){return uint32_t(objects[at])<<24|uint32_t(objects[at+1])<<16|uint32_t(objects[at+2])<<8|uint32_t(objects[at+3]);};
 unsigned left=0,right=0,elevator=0,opposite=0;
 for(size_t at=0;at<objects.size();at+=12){const auto kind=u16(at),instance=u16(at+2);const float x=std::bit_cast<float>(bits(at+4)),y=std::bit_cast<float>(bits(at+8));
  if(kind==0x464F){if(instance==0&&x==248&&y==344)++left;else if(instance==2&&x==536&&y==344)++right;else return {};}
  if(kind==0x415F){if(instance||x!=312||y!=376)return {};++elevator;}
  if(kind==0x415E){if(instance||x!=472||y!=376)return {};++opposite;}
 }
 if(left!=1||right!=1||elevator!=1||opposite!=1)return {};
 const auto map=Map::read(rdram);if(!map||map->columns!=3||map->rows!=2)return {};
 // Check the current full native platform, including head clearance. These
 // observations never stamp/restore a broken brick or accept absent geometry.
 for(unsigned x=224;x<272;++x){const auto above=map->occupied(rdram,float(x),319),top=map->occupied(rdram,float(x),320),inside=map->occupied(rdram,float(x),322);if(!above||*above||!top||!*top||!inside||!*inside)return {};}
 return Stand{248,310,320};
}
struct Eye {uint32_t actor=0,metadata=0,definition=0,resource=0;float x=0,y=0,vx=0,vy=0;};
inline bool observe_eye(uint8_t* rdram,uint32_t root,uint32_t private_data,uint64_t birth,uint32_t child,Eye& output){
 if(!rdram||!birth||!n::actor(rdram,root,0x41C6)||n::word(rdram,root+0x1F4)!=private_data||!pointer(private_data,0x1000)||n::byte(rdram,root+0x17D)!=2||n::byte(rdram,root+0x18)!=1||!n::actor(rdram,child,0x46F9)||n::word(rdram,child+0x180)!=root||n::byte(rdram,child+0x18)==0xFF)return false;
 const auto root_resource=n::word(rdram,root+0x168),resource=n::word(rdram,child+0x168),metadata=n::word(rdram,child+0x174);
 if(!pointer(root_resource,0x48)||!pointer(resource,0x48)||!pointer(metadata,4))return false;
 const auto definition=n::word(rdram,metadata),factory=n::word(rdram,root_resource+0x40);
 // The resource factory80550B20 and this collider's80554928 belong to the
 // same native overlay. Verify the relocated callback, not a guessed eye.
 if(!pointer(definition,0x2C)||!pointer(factory,0x3E0C)||n::word(rdram,resource+0x40)!=factory||n::word(rdram,definition+0x28)!=factory+0x3E08||n::word(rdram,definition+0xC)!=0x00080014)return false;
 Eye e{child,metadata,definition,resource,scalar(rdram,child+0x84),scalar(rdram,child+0x88),0,0};if(!d::point(e.x,e.y))return false;output=e;return true;
}
struct Sample {uint32_t root=0,private_data=0;uint64_t birth=0;unsigned room=177,encounter=0,vi=0;Eye eye{};};
class History {
 std::array<Sample,8> samples_{};unsigned cursor_=0;
 struct NativeSample {Sample sample{};std::array<float,5> x{},y{};unsigned count=0;};
 std::array<NativeSample,8> native_{};unsigned native_cursor_=0;
public:
 void reset(){samples_={};cursor_=0;native_={};native_cursor_=0;}
 Eye sample(uint32_t root,uint32_t private_data,uint64_t birth,unsigned room,unsigned encounter,unsigned vi,Eye eye){
  Sample* found=nullptr;
  for(auto& s:samples_)if(s.root==root&&s.eye.actor==eye.actor){found=&s;break;}
  if(!found)found=&samples_[cursor_++%samples_.size()];
  const auto& old=*found;
  const bool identity=old.root==root&&old.private_data==private_data&&old.birth==birth&&old.room==room&&old.encounter==encounter&&old.eye.metadata==eye.metadata&&old.eye.definition==eye.definition&&old.eye.resource==eye.resource;
  if(identity&&vi==old.vi){eye.vx=old.eye.vx;eye.vy=old.eye.vy;return eye;}
  if(identity&&vi>old.vi&&vi-old.vi<=4){const float dt=float(vi-old.vi),vx=(eye.x-old.eye.x)/dt,vy=(eye.y-old.eye.y)/dt;if(std::isfinite(vx)&&std::isfinite(vy)&&std::abs(vx)<=4&&std::abs(vy)<=4){eye.vx=vx;eye.vy=vy;}}
  *found={root,private_data,birth,room,encounter,vi,eye};return eye;
 }
 // Called only at this owned41C6's real483B8 entry. A5EC advances4002 once
 // per native update too. Repeated controller VI observations must neither
 // zero this velocity nor divide it by the number of intervening VI.
 void native_frame(uint8_t* rdram,uint32_t root,uint32_t private_data,uint64_t birth,unsigned room,unsigned encounter,unsigned vi){
  if(!rdram||encounter>=10||room!=(encounter?167+encounter:162)||!n::actor(rdram,root,0x41C6)||n::word(rdram,root+0x130)!=0x800483B8)return;
  for(unsigned child=0;child<20;++child){Eye eye;if(!observe_eye(rdram,root,private_data,birth,n::word(rdram,root+0x188+4*child),eye))continue;
   NativeSample* found=nullptr;for(auto& s:native_)if(s.sample.root==root&&s.sample.eye.actor==eye.actor){found=&s;break;}if(!found)found=&native_[native_cursor_++%native_.size()];
   auto& s=*found;const auto& old=s.sample;
   const bool same=s.count&&old.root==root&&old.private_data==private_data&&old.birth==birth&&old.room==room&&old.encounter==encounter&&old.eye.metadata==eye.metadata&&old.eye.definition==eye.definition&&old.eye.resource==eye.resource;
   if(same&&vi==old.vi)continue;
   if(!same||vi<old.vi||vi-old.vi>12||std::abs(eye.x-old.eye.x)>8||std::abs(eye.y-old.eye.y)>8){s={};}
   if(s.count==s.x.size()){for(unsigned i=1;i<s.count;++i){s.x[i-1]=s.x[i];s.y[i-1]=s.y[i];}--s.count;}
   s.x[s.count]=eye.x;s.y[s.count]=eye.y;++s.count;
   if(s.count>=3){const float intervals=float(s.count-1);eye.vx=(eye.x-s.x[0])/intervals;eye.vy=(eye.y-s.y[0])/intervals;
    const float latest_x=s.x[s.count-1]-s.x[s.count-2],latest_y=s.y[s.count-1]-s.y[s.count-2];
    if(latest_x*eye.vx<0)eye.vx=latest_x;if(latest_y*eye.vy<0)eye.vy=latest_y;
   }
   s.sample={root,private_data,birth,room,encounter,vi,eye};
  }
 }
 std::optional<Eye> observe(uint32_t root,uint32_t private_data,uint64_t birth,unsigned room,unsigned encounter,unsigned vi,Eye eye)const{
  for(const auto& s:native_){const auto& old=s.sample;
   if(s.count>=3&&old.root==root&&old.private_data==private_data&&old.birth==birth&&old.room==room&&old.encounter==encounter&&old.eye.actor==eye.actor&&old.eye.metadata==eye.metadata&&old.eye.definition==eye.definition&&old.eye.resource==eye.resource&&vi>=old.vi&&vi-old.vi<=8&&std::abs(eye.x-old.eye.x)<=8&&std::abs(eye.y-old.eye.y)<=8&&std::isfinite(old.eye.vx)&&std::isfinite(old.eye.vy)&&std::abs(old.eye.vx)<=4&&std::abs(old.eye.vy)<=4){eye.vx=old.eye.vx;eye.vy=old.eye.vy;return eye;}
  }
  return {};
 }
};
inline std::optional<d::Target> choose(uint8_t* rdram,History& history,uint32_t root,uint32_t private_data,uint64_t birth,unsigned room,unsigned encounter,unsigned vi,const d::Input& input,bool vulnerable){
 if(!rdram||!birth||encounter>=10||room!=(encounter?167+encounter:162)||!n::actor(rdram,root,0x41C6)||n::word(rdram,root+0x1F4)!=private_data||!pointer(private_data,0x1000))return {};
 const auto map=Map::read(rdram);std::optional<d::Target> best;float cost=1e30f;
 for(unsigned i=0;i<20;++i){Eye e;if(!observe_eye(rdram,root,private_data,birth,n::word(rdram,root+0x188+4*i),e))continue;const auto motion=history.observe(root,private_data,birth,room,encounter,vi,e);if(motion)e=*motion;
  d::Target t{root,0x41C6,e.x,e.y,e.vx,e.vy,true,vulnerable&&bool(motion),false,false,false,6.6f,2.f};float tx=0,ty=0;d::predicted_target(input,t,tx,ty);
  t.clear_path=map&&map->clear(rdram,input.x,input.y,e.x,e.y)&&map->clear(rdram,input.x,input.y,tx,ty);
  const float distance=std::hypot(e.x-input.x,e.y-input.y)+(t.clear_path?0:100000.f);if(distance<cost){best=t;cost=distance;}
 }
 return best;
}
}
