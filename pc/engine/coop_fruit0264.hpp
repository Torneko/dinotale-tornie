#pragma once
#include "coop_native_collision.hpp"
#include "coop_lifecycle0280.hpp"
#include <array>
#include <map>
#include <optional>
#include <cstdio>
#include <limits>

namespace workshop::coop_fruit0264 {
namespace n=coop_collision;
inline constexpr uint32_t contact=0x8003C510,ingestion=0x80032A8C;
inline constexpr uint64_t blocked=UINT64_MAX;
constexpr bool fruit(uint16_t kind){return (kind>=0x4030&&kind<=0x4034)||(kind>=0x4F00&&kind<=0x4F05);}
enum class Phase {reserved,attached,consuming,consumed};
struct Claim {n::Identity object{},root{};unsigned owner=0;Phase phase=Phase::reserved;uint64_t serial=0;};
struct Pending {Claim claim{};uint32_t function=0,private_data=0;};
struct Statistics {uint64_t reserved=0,attached=0,consumed=0,released=0,rejected=0;};
class Registry final {
 bool enabled_=false,active_=false,failed_=false,single_=false;
 uint64_t next_=0;
 unsigned vi_=0,room_=177;
 std::array<n::Identity,2> roots_{};
 std::map<uint32_t,Claim> claims_;
 std::map<uint64_t,Pending> pending_;
 Statistics stats_{};
 static constexpr size_t capacity=1024;
 static bool valid_identity(uint8_t* r,const n::Identity& id){const auto now=n::observed_identity(r,id.address);return now&&*now==id;}
 bool root_live(uint8_t* r,unsigned owner)const {
  if(owner>=2||!valid_identity(r,roots_[owner])||roots_[owner].kind!=0x4001||!n::range(roots_[owner].private_data,0x1550))return false;
  const auto p=roots_[owner].address;return n::byte(r,p+0x17D)==2&&n::byte(r,p+0x18)!=0xA;
 }
 std::optional<unsigned> owner(uint8_t* r,uint32_t player)const {
  for(unsigned i=0;i<2;i++)if(roots_[i].address==player&&root_live(r,i))return i;
  return {};
 }
 static bool reciprocal(uint8_t* r,uint32_t object,uint32_t parent){
  if(!n::range(parent,0x1FC))return false;
  const unsigned count=n::half(r,parent+0x184),slot=n::half(r,object+0x17E);if(!count||count>20||slot>=20||n::word(r,parent+0x188+slot*4)!=object)return false;
  unsigned occupied=0,matches=0;for(unsigned i=0;i<20;i++){const auto child=n::word(r,parent+0x188+i*4);occupied+=child!=0;matches+=child==object;}
  return occupied==count&&matches==1;
 }
 bool foreign_parent(uint8_t* r,uint32_t object)const {
  const auto parent=n::word(r,object+0x180);const auto id=n::observed_identity(r,parent);
  return id&&id->kind!=0x4001&&id->kind!=0x400B&&id->kind!=0x400C&&!n::owner_of_body(r,parent)&&n::byte(r,parent+0x17D)!=3&&n::byte(r,parent+0x18)!=0xFF&&reciprocal(r,object,parent);
 }
 bool held(uint8_t* r,const Claim& c)const {
  if(!root_live(r,c.owner)||roots_[c.owner]!=c.root||!valid_identity(r,c.object))return false;
  if(n::word(r,c.root.private_data+0x164)!=c.object.address)return false;
  const auto parent=n::word(r,c.object.address+0x180);const auto actual=n::owner_of_body(r,parent);
  return actual&&*actual==c.owner&&reciprocal(r,c.object.address,parent);
 }
 bool linked(uint8_t* r,const Claim& c)const {
  if(!root_live(r,c.owner)||roots_[c.owner]!=c.root||!valid_identity(r,c.object))return false;
  if(n::word(r,c.root.private_data+0x164)==c.object.address)return true;
  const auto p=n::owner_of_body(r,n::word(r,c.object.address+0x180));return p&&*p==c.owner;
 }
 void erase(std::map<uint32_t,Claim>::iterator it){++stats_.released;claims_.erase(it);}
 uint64_t reject(){++stats_.rejected;return blocked;}
 void fail(const char* reason){if(!failed_)std::fprintf(stderr,"[CoopFruit] failed reason=%s vi=%u nativeCountersUnchangedByAdapter=1\n",reason,vi_);failed_=true;}
public:
 void configure(bool enabled){enabled_=enabled;active_=failed_=single_=false;roots_={};claims_.clear();pending_.clear();stats_={};vi_=0;room_=177;/* Never recycle ticket identities. */}
 bool active()const{return active_;}
 bool failed()const{return failed_;}
 void host_error(){fail("host_registry_exception");}
 const Statistics& statistics()const{return stats_;}
 size_t claims()const{return claims_.size();}
 size_t pending()const{return pending_.size();}
 void forget(uint32_t actor,const coop_lifecycle0280::State* life=nullptr){
  // A native cleanup may occur inside32A8C before164 is cleared. Its opaque
  // Pending retains the already-proved identity until the native return.
  const auto it=claims_.find(actor);if(it!=claims_.end()&&it->second.phase!=Phase::consuming)erase(it);
  for(unsigned p=0;p<2;p++)if(roots_[p].address==actor){
   using P=coop_lifecycle0280::Phase;
   const bool expected=life&&enabled_&&life->enabled&&life->bound&&!life->failed&&!life->ending&&life->epoch&&(room_==43||room_==81)&&life->room==room_&&life->players[p].root==roots_[p]&&life->players[p].death_vi&&(life->players[p].phase==P::death_pending||life->players[p].phase==P::retiring||life->players[p].phase==P::waiting)&&life->players[1-p].phase==P::alive&&life->players[1-p].root==roots_[1-p]&&pending_.empty();
   active_=false;single_=false;
   if(!expected)claims_.clear();
   else for(auto claim=claims_.begin();claim!=claims_.end();)if(claim->second.owner==p){auto old=claim++;erase(old);}else ++claim;
  }
 }
 void frame(uint8_t* r,unsigned vi,unsigned room,bool ready,uint32_t first,uint32_t second,const coop_lifecycle0280::State* life=nullptr){
  vi_=vi;room_=room;
  const auto a=r?n::observed_identity(r,first):std::nullopt,b=r?n::observed_identity(r,second):std::nullopt;
  const bool scoped_room=room==43||(room==81&&life&&life->enabled&&life->bound&&!life->failed&&!life->ending&&life->room==81&&a&&b&&life->players[0].root==*a&&life->players[1].root==*b);
  const bool admissible=enabled_&&r&&ready&&scoped_room&&a&&b&&a->kind==0x4001&&b->kind==0x4001&&first!=second&&pending_.empty();
  if(!admissible){active_=false;if(pending_.empty())claims_.clear();return;}
  const std::array<n::Identity,2> roots{*a,*b};
  if(roots!=roots_){
   for(auto it=claims_.begin();it!=claims_.end();)if(it->second.owner>=2||it->second.root!=roots[it->second.owner]){auto old=it++;erase(old);}else ++it;
   roots_=roots;
  }
  single_=false;
  active_=root_live(r,0)&&root_live(r,1);
  if(!active_){claims_.clear();return;}
  for(auto it=claims_.begin();it!=claims_.end();){
   const auto& c=it->second;
   if(!valid_identity(r,c.object)||(c.phase!=Phase::consumed&&c.phase!=Phase::consuming&&c.phase!=Phase::reserved&&!linked(r,c))){auto old=it++;erase(old);}else ++it;
  }
 }
 // Only the typed lifecycle death receipt may enable an absent partner.
 // Preserve the survivor's in-flight reservation and native mouth; no adoption
 // from a mere address or from an unknown/invalidation/room transition.
 void single_frame(uint8_t* r,unsigned vi,unsigned room,const coop_lifecycle0280::State& life){
  vi_=vi;room_=room;using P=coop_lifecycle0280::Phase;
  unsigned owner=2,dead=2;for(unsigned i=0;i<2;i++){if(life.players[i].phase==P::alive)owner=i;else if(life.players[i].death_vi&&(life.players[i].phase==P::death_pending||life.players[i].phase==P::retiring||life.players[i].phase==P::waiting))dead=i;}
  const bool proof=enabled_&&r&&(room==43||room==81)&&life.enabled&&life.bound&&!life.failed&&!life.ending&&life.room==room&&life.epoch&&owner<2&&dead==1-owner&&vi>=life.players[dead].death_vi&&pending_.empty()&&roots_[owner]==life.players[owner].root;
  if(!proof){active_=false;return;}
  single_=true;active_=root_live(r,owner);
  for(auto it=claims_.begin();it!=claims_.end();)if(it->second.owner!=owner||it->second.root!=roots_[owner]||!valid_identity(r,it->second.object)||(it->second.phase==Phase::attached&&!linked(r,it->second))){auto old=it++;erase(old);}else ++it;
 }
 uint64_t begin(uint8_t* r,uint32_t fn,uint32_t a,uint32_t arg,uint32_t scoped_player){
  if(!active_||!r||(fn!=contact&&fn!=ingestion))return 0;
  const auto who=owner(r,scoped_player);
  const bool known_player=scoped_player&&(scoped_player==roots_[0].address||scoped_player==roots_[1].address);
  if(!known_player){
   // A silently changed/unborn tongue under one of OUR roots is not a
   // third actor: reject its fruit contact without trusting that chain.
   if(fn==contact&&n::range(a,0x1FC)&&n::half(r,a+12)==0x400C&&n::range(arg,0x1FC)&&fruit(n::half(r,arg+12))){
    const auto parent=n::word(r,a+0x180);if(parent==roots_[0].address||parent==roots_[1].address)return reject();
   }
   return 0;
  }
  if(!who||n::word(r,0x800F8828)!=scoped_player){
   const uint32_t object=fn==contact?arg:n::range(arg,0x168)?n::word(r,arg+0x164):0;
   return n::range(object,0x1FC)&&fruit(n::half(r,object+12))?reject():0;
  }
  uint32_t object=arg,priv=roots_[*who].private_data;
  if(fn==ingestion){if(a!=scoped_player||arg!=priv)return reject();object=n::word(r,priv+0x164);}
  // Unknown/unrelated actor kinds retain their native path. Hearts4035,
  // eggs and edible enemies are deliberately outside this fruit-only lot.
  if(!n::range(object,0x1FC)||!fruit(n::half(r,object+12)))return 0;
  if(failed_||(!single_&&!root_live(r,1-*who))||pending_.size()>=32||next_>=UINT64_MAX-1)return reject();
  const auto id=n::observed_identity(r,object);
  if(!id||!fruit(id->kind)||n::byte(r,object+0x18)==0xFF)return reject();
  const unsigned queue=n::byte(r,object+0x17D);
  const auto object_parent=n::word(r,object+0x180);
  if(fn==contact ? !((queue==2&&!object_parent)||((queue==0||queue==2)&&foreign_parent(r,object))) : queue!=0)return reject();
  auto found=claims_.find(object);
  if(found!=claims_.end()&&found->second.object!=*id){erase(found);found=claims_.end();}
  if(fn==contact){
   const auto tongue_owner=n::owner_of_body(r,a);
   if(!tongue_owner||*tongue_owner!=*who||n::half(r,a+12)!=0x400C||n::word(r,priv+0x164))return reject();
   if(found!=claims_.end()){
    if(found->second.phase==Phase::consuming||found->second.phase==Phase::consumed||linked(r,found->second)||found->second.phase==Phase::reserved)return reject();
    erase(found);
   }
   for(unsigned p=0;p<2;p++)if(root_live(r,p)&&n::word(r,roots_[p].private_data+0x164)==object)return reject();
   const auto parent=n::owner_of_body(r,n::word(r,object+0x180));if(parent)return reject();
   if(claims_.size()>=capacity)return reject();
   Claim c{*id,roots_[*who],*who,Phase::reserved,++next_};claims_[object]=c;pending_[c.serial]={c,fn,priv};++stats_.reserved;return c.serial;
  }
  if(found==claims_.end()||found->second.owner!=*who||found->second.root!=roots_[*who]||found->second.phase!=Phase::attached||!held(r,found->second))return reject();
  found->second.phase=Phase::consuming;found->second.serial=++next_;pending_[next_]={found->second,fn,priv};return next_;
 }
 void finish(uint8_t* r,uint64_t token,bool completed){
  if(!token||token==blocked)return;
  const auto p=pending_.find(token);if(p==pending_.end())return;
  const auto pending=p->second;pending_.erase(p);
  auto found=claims_.find(pending.claim.object.address);
  if(found==claims_.end()||found->second.serial!=token||found->second.object!=pending.claim.object){fail("transaction_identity_changed");return;}
  if(pending.function==contact){
   if(completed&&held(r,found->second)){found->second.phase=Phase::attached;++stats_.attached;}
   else erase(found);
   return;
  }
  //32984 retires the object BEFORE clearing164/155. Do not demand a live
  //fruit or birth here: the native free hook may already have forgotten it.
  found->second.phase=Phase::consumed;
  if(!completed||!root_live(r,pending.claim.owner)||roots_[pending.claim.owner]!=pending.claim.root||n::word(r,pending.private_data+0x164)||n::byte(r,pending.private_data+0x155)){
   fail("native_ingestion_did_not_complete_cleanup");return;
  }
  ++stats_.consumed;
  std::fprintf(stderr,"[CoopFruit] consumed actor=%08X birth=%llu owner=%u player=%08X kind=%04X total=%llu vi=%u nativeTransaction=1 nativeCountersUnchangedByAdapter=1\n",pending.claim.object.address,(unsigned long long)pending.claim.object.generation,pending.claim.owner,pending.claim.root.address,pending.claim.object.kind,(unsigned long long)stats_.consumed,vi_);
 }
};
inline Registry registry;
}
