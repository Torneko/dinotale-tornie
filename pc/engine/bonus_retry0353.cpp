#include "bonus_retry0353.hpp"
#include "workshop_data.hpp"
#include <bit>
#include <cmath>
#include <map>
#include <set>
namespace workshop::bonus_retry0353 {
namespace {
constexpr uint32_t flags=0x800F845C,count=0x800F8458,records=0x800F846C,selected_record=0x800F84CC;
constexpr gpr at(uint32_t p){return gpr(int32_t(p));}
bool range(uint32_t p,size_t bytes){return !(p&3)&&p>=0x80000400&&p<0x80800000&&bytes<=0x80800000-p;}
bool heap(uint32_t p,size_t bytes){return range(p,bytes)&&p<0x80400000&&bytes<=0x80400000-p;}
uint32_t word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,at(p)));}
bool actor(uint8_t* rdram,uint32_t p,unsigned kind){return rdram&&heap(p,0x1FC)&&word(rdram,p)==0x19971110&&MEM_HU(12,at(p))==kind&&MEM_BU(0x17D,at(p))!=3;}
unsigned team(uint8_t* rdram){return (word(rdram,0x800F8DF4)&255)|((word(rdram,0x807F1000)&1023)<<8);}
bool native_scope(uint8_t* rdram){return rdram&&word(rdram,0x800F8B88)==1;}
unsigned course(uint8_t* rdram){const unsigned page=MEM_BU(0,at(0x800F8DFC)),col=MEM_BU(0,at(0x800F8DFD));return page<6&&col<4?page*4+col:24;}
bool permitted(const Environment& e){return e.pc_mode==7&&e.bonus>=1&&e.bonus<=6&&e.attempt_epoch&&e.owned_mask<=0x3FFFF&&e.independent_bonus_save&&!e.canonical&&!e.custom_active&&!e.deferred;}
std::optional<unsigned> awake(uint8_t* rdram){unsigned mask=0;for(unsigned i=0;i<4;++i){const auto value=word(rdram,flags+i*4);if(value>1)return {};mask|=value<<i;}if(word(rdram,count)!=unsigned(std::popcount(mask)))return {};return mask;}
bool scale(uint8_t* rdram,uint32_t a){for(unsigned offset:{0xB4u,0xB8u,0xBCu}){const auto f=std::bit_cast<float>(word(rdram,a+offset));if(!std::isfinite(f)||f<=0||f>1.0f)return false;}return true;}
}
struct Adapter::Impl {
 struct Birth {uint64_t serial=0;unsigned room=177;uint32_t packed=0;bonus_rooms0351::Point point{};};
 struct Wake {uint32_t actor=0,data=0;uint64_t birth=0;unsigned index=4,before_mask=0;bonus_runtime0352::Ticket ticket;};
 struct Death {bonus_runtime0352::Ticket ticket;uint32_t player=0,data=0,before_flags=0,before_extras=0,counter=0;uint64_t birth=0;unsigned color=18,before_team=0;};
 Environment env;bonus_routes0351::Route route;bonus_rooms0351::Catalog catalog;View state;
 std::optional<bonus_finale_entries0353::EntrySpec> finale;
 std::array<std::array<uint8_t,24>,4> checkpoint_templates{};
 std::optional<bonus_runtime0352::Ticket> room;unsigned visited=0;uint64_t serial=0,death_serial=0;
 uint32_t previous_player_record=0;bool owns_player_record=false;
 std::map<uint32_t,Birth> births;std::optional<Wake> wake;std::optional<Death> death;std::optional<bonus_runtime0352::Ticket> admission;
 bool same(const Environment& e)const{return permitted(e)&&permitted(env)&&e.bonus==env.bonus&&e.seed==env.seed&&e.attempt_epoch==env.attempt_epoch&&e.owned_mask==env.owned_mask;}
 void fail(const char* why){state.phase=Phase::Rejected;state.reason=why;}
};
Adapter::Adapter():p(std::make_unique<Impl>()){}Adapter::~Adapter()=default;
void Adapter::reset(uint8_t* rdram){
 if(p->owns_player_record&&!rdram){p->fail("Live retry pointer requires RAM to reset");return;}
 if(p->owns_player_record&&rdram&&word(rdram,0x800D9A80)==saved_player_record_ram)MEM_W(0,at(0x800D9A80))=int32_t(p->previous_player_record);
 p=std::make_unique<Impl>();
}
View Adapter::view()const{return p->state;}
bool Adapter::configure(std::span<const uint8_t> original,const bonus_routes0351::Route& route,const Environment& env){
 if(p->owns_player_record){p->fail("Retry pointer reset required before reconfigure");return false;}
 reset();if(!permitted(env))return false;
 try{workshop::require(bonus_routes0351::validate(route)&&route.bonus==env.bonus&&route.seed==env.seed,"Retry route/attempt mismatch");p->catalog=bonus_rooms0351::read(original);for(unsigned i=0;i<4;++i)std::copy_n(original.begin()+0xA8370+i*24,24,p->checkpoint_templates[i].begin());p->route=route;p->env=env;p->state.phase=Phase::Live;return true;}
 catch(const std::exception& e){p->state.phase=Phase::Rejected;p->state.reason=e.what();return false;}
}
bool Adapter::attest_finale(std::span<const uint8_t> original,const bonus_finale_entries0353::EntrySpec& entry,const Environment& e){
 if(!p->same(e)||p->state.phase==Phase::DeathObserved||p->state.phase==Phase::SelectionPending||p->state.phase==Phase::GameOver)return false;
 p->finale.reset();if(!bonus_finale_entries0353::valid(original,p->route,entry))return false;p->finale=entry;return true;
}
bool Adapter::bind_room(const bonus_runtime0352::Ticket& t,const bonus_session0351::Snapshot& saved,const Environment& e){
 if(!p->same(e)||!saved.readable||!saved.last_write_succeeded||t.bonus!=e.bonus||t.seed!=e.seed||t.save_stem!=bonus_runtime0352::bonus_save||t.revision!=saved.revision||t.ordinal>=p->route.steps.size())return false;
 const auto& slot=saved.attempts[t.bonus-1];if(!slot||!bonus_session0351::valid(*slot)||slot->seed!=t.seed||slot->cursor!=t.ordinal||slot->order[t.ordinal]!=t.room)return false;
 const auto& step=p->route.steps[t.ordinal];
 if(step.role==bonus_routes0351::StepRole::Finale){
  if(!p->finale||slot->phase!=bonus_session0351::Phase::Finale||t.ordinal!=p->finale->from_ordinal+1||t.room!=p->finale->room||t.course!=p->finale->course||t.root!=p->finale->root)return false;
 }else if(step.role!=bonus_routes0351::StepRole::Ordinary||step.key.course!=t.course||step.key.root!=t.root)return false;
 p->room=t;p->visited=slot->visited_mask;if(p->state.phase==Phase::Disabled)p->state.phase=Phase::Live;return true;
}
void Adapter::created(const recomp_context* c,const Environment& e){
 if(!p->same(e)||!p->room||!c||!heap(uint32_t(c->r4),0x1FC))return;
 p->births[uint32_t(c->r4)]={++p->serial,p->room->room,uint32_t(c->r5),{uint32_t(c->r6),uint32_t(c->r7)}};
}
void Adapter::forget(uint32_t actor,const Environment& e){if(p->same(e))p->births.erase(actor);}
void Adapter::wake_begin(uint8_t* rdram,const recomp_context* c,const Environment& e){
 p->wake.reset();if(!p->same(e)||!p->room||p->route.steps[p->room->ordinal].role!=bonus_routes0351::StepRole::Ordinary||!native_scope(rdram)||!c||course(rdram)!=p->room->course)return;
 const auto a=uint32_t(c->r4);const auto birth=p->births.find(a);if(birth==p->births.end()||birth->second.room!=p->room->room)return;
 const auto kind=birth->second.packed>>16;if(kind<0x4507||kind>0x450A||!actor(rdram,a,kind))return;
 const auto data=word(rdram,a+0x1F4);if(!heap(data,0x280))return;const auto index=word(rdram,data+0x27C);const auto mask=awake(rdram);if(index!=kind-0x4507||index>=4||!mask||(*mask&(1u<<index)))return;
 const auto* room=p->catalog.find(p->room->room);if(!room)return;
 const auto placed=std::find_if(room->checkpoints.begin(),room->checkpoints.end(),[&](const auto& checkpoint){return checkpoint.number==index+1&&checkpoint.placement.packed==birth->second.packed&&checkpoint.placement.point==birth->second.point;});if(placed==room->checkpoints.end())return;
 p->wake=Impl::Wake{a,data,birth->second.serial,index,*mask,*p->room};
}
bool Adapter::wake_finish(uint8_t* rdram,const Environment& e){
 const auto pending=p->wake;p->wake.reset();if(!pending||!p->same(e)||!p->room||*p->room!=pending->ticket||!native_scope(rdram)||course(rdram)!=pending->ticket.course)return false;
 const auto birth=p->births.find(pending->actor);const auto mask=awake(rdram);const auto address=records+pending->index*24;
 if(birth==p->births.end()||birth->second.serial!=pending->birth||!actor(rdram,pending->actor,0x4507+pending->index)||word(rdram,pending->actor+0x1F4)!=pending->data||!mask||*mask!=(pending->before_mask|(1u<<pending->index))||word(rdram,selected_record)!=address)return false;
 std::array<uint8_t,24> record{};for(unsigned i=0;i<24;++i)record[i]=MEM_BU(i,at(address));
 auto expected=p->checkpoint_templates[pending->index];expected[4]=record[4];if(record!=expected)return false;
 const unsigned destination=record[4];const auto* source=p->catalog.find(pending->ticket.room);if(!source||p->catalog.courses[source->course].warp_destinations[pending->index]!=destination)return false;
 const auto dest=std::find_if(p->route.steps.begin(),p->route.steps.end(),[&](const auto& step){return step.role==bonus_routes0351::StepRole::Ordinary&&step.key.room==destination;});
 if(dest==p->route.steps.end())return false;const auto ordinal=unsigned(dest-p->route.steps.begin());if(!(p->visited&(1u<<ordinal))||!dest->key.course||!dest->key.root||*dest->key.course!=source->course)return false;
 const auto* target=p->catalog.find(destination);if(!target)return false;const uint32_t packed=workshop::u32(record,0);std::optional<bonus_rooms0351::Point> point;
 for(const auto& item:target->placements)if((item.packed&0xFFFF03FFu)==(packed&0xFFFF03FFu)){if(point)return false;point=item.point;}
 if(!point||(packed>>16)!=0x4507+pending->index)return false;
 const auto player_record=word(rdram,0x800D9A80);if((!heap(player_record,12)&&!(p->owns_player_record&&player_record==saved_player_record_ram))||(word(rdram,player_record)>>16)!=0x4001)return false;
 std::array<uint8_t,12> saved{};for(unsigned i=0;i<12;++i)saved[i]=MEM_BU(i,at(player_record));
 const bonus_rooms0351::Point native_point{word(rdram,player_record+4),word(rdram,player_record+8)};
 if(!navigation::coordinate(native_point[0])||!navigation::coordinate(native_point[1])||(native_point[0]==0&&native_point[1]==0))return false;
 p->state.checkpoint=Checkpoint{ordinal,destination,*dest->key.course,*dest->key.root,pending->index,*mask,record,saved,native_point};return true;
}
bool Adapter::death_begin(uint8_t* rdram,const recomp_context* c,const Environment& e){
 if(!p->same(e)||!p->room||!native_scope(rdram)||!c||course(rdram)!=p->room->course||p->state.phase==Phase::DeathObserved||p->state.phase==Phase::SelectionPending)return false;
 const auto a=uint32_t(c->r4);const auto birth=p->births.find(a);if(birth==p->births.end()||!actor(rdram,a,0x4001)||word(rdram,0x800F8828)!=a||MEM_BU(0x18,at(a))!=0xA)return false;
 const auto sub=MEM_BU(0x30,at(a));if(sub!=0xA&&sub!=0xB&&sub!=0xC&&sub!=0xD&&sub!=0x10&&sub!=0x11)return false;
 const auto data=word(rdram,a+0x1F4);const auto color=word(rdram,0x807F0008),before=team(rdram);if(!heap(data,0x1300)||color>=18||!(e.owned_mask&(1u<<color))||!(before&(1u<<color))||(before&~e.owned_mask)||MEM_HU(0,at(0x800F897E))>8)return false;
 p->death=Impl::Death{*p->room,a,data,word(rdram,0x800F8DF4),word(rdram,0x807F1000),word(rdram,0x807F1004),birth->second.serial,color,before};p->state.phase=Phase::DeathObserved;p->state.retry.reset();p->admission.reset();return true;
}
std::optional<RetryPlan> Adapter::death_scene(uint8_t* rdram,const recomp_context* c,const Environment& e){
 if(!p->same(e)||!p->death||p->state.phase!=Phase::DeathObserved||!native_scope(rdram)||!c||uint32_t(c->r31)!=0x807F1198u||c->r5!=22)return {};
 const auto& death=*p->death;const unsigned after=death.before_team&~(1u<<death.color);const unsigned scene=after?8:29;
 if(c->r6!=scene||team(rdram)!=after||word(rdram,0x807F1008)!=death.color||word(rdram,0x807F1004)!=death.counter+1u){p->fail("Native Bonus loss/team/scene receipt mismatch");return {};}
 if((word(rdram,0x800F8DF4)&~255u)!=(death.before_flags&~255u)||(word(rdram,0x807F1000)&~1023u)!=(death.before_extras&~1023u)){p->fail("Native Bonus loss changed unrelated flags");return {};}
 RetryPlan result;result.death_epoch=++p->death_serial;result.previous=death.ticket;result.post_death_team=after;result.dead_color=death.color;result.dead_player=death.player;result.dead_birth=death.birth;
 if(p->state.checkpoint&&(p->visited&(1u<<p->state.checkpoint->ordinal))){const auto& checkpoint=*p->state.checkpoint;result.target_ordinal=checkpoint.ordinal;result.room=checkpoint.room;result.course=checkpoint.course;result.root=checkpoint.root;result.checkpoint=checkpoint;}
 else{const auto& root=p->route.steps[0].key;result.target_ordinal=0;result.room=root.room;result.course=root.course.value_or(24);result.root=root.root.value_or(177);}
 p->state.retry=result;p->state.phase=after?Phase::SelectionPending:Phase::GameOver;return result;
}
bool Adapter::bind_admission(const bonus_runtime0352::Admission& a,const Environment& e){
 if(!p->same(e)||p->state.phase!=Phase::SelectionPending||!p->state.retry||a.status!=bonus_runtime0352::Status::Ready||!a.ticket)return false;
 const auto& t=*a.ticket;const auto& retry=*p->state.retry;if(t.bonus!=e.bonus||t.seed!=e.seed||t.ordinal!=retry.target_ordinal||t.room!=retry.room||t.course!=retry.course||t.root!=retry.root||t.epoch==retry.previous.epoch||t.revision<retry.previous.revision||t.save_stem!=bonus_runtime0352::bonus_save)return false;
 p->admission=t;p->room=t;p->state.phase=Phase::AdmissionPending;return true;
}
bool Adapter::before_room_selection(uint8_t* rdram,const Environment& e){
 if(!p->same(e)||!native_scope(rdram)||!p->state.retry||!p->admission||(p->state.phase!=Phase::AdmissionPending&&p->state.phase!=Phase::Installed)||course(rdram)!=p->admission->course||team(rdram)!=p->state.retry->post_death_team)return false;
 const auto& checkpoint=p->state.retry->checkpoint;
 if(checkpoint){const auto current=word(rdram,0x800D9A80);
  if(current!=saved_player_record_ram){if(!heap(current,12)||(word(rdram,current)>>16)!=0x4001)return false;p->previous_player_record=current;}
  else if(!p->owns_player_record)return false;
  for(unsigned i=0;i<12;++i)MEM_B(i,at(saved_player_record_ram))=checkpoint->player_record[i];
  MEM_W(0,at(0x800D9A80))=int32_t(saved_player_record_ram);p->owns_player_record=true;
  const auto address=records+checkpoint->index*24;for(unsigned i=0;i<24;++i)MEM_B(i,at(address))=checkpoint->native_record[i];for(unsigned i=0;i<4;++i)MEM_W(0,at(flags+i*4))=(checkpoint->awake_mask>>i)&1;MEM_W(0,at(count))=std::popcount(checkpoint->awake_mask);MEM_W(0,at(selected_record))=int32_t(address);}
 else{if(p->owns_player_record&&word(rdram,0x800D9A80)==saved_player_record_ram)MEM_W(0,at(0x800D9A80))=int32_t(p->previous_player_record);p->owns_player_record=false;
  for(unsigned i=0;i<4;++i)MEM_W(0,at(flags+i*4))=0;MEM_W(0,at(count))=0;MEM_W(0,at(selected_record))=0;}
 p->state.phase=Phase::Installed;return true;
}
std::optional<Receipt> Adapter::arrival(uint8_t* rdram,unsigned observed_room,const Environment& e){
 if(!p->same(e)||p->state.phase!=Phase::Installed||!p->state.retry||!p->admission||!native_scope(rdram)||observed_room!=p->admission->room||course(rdram)!=p->admission->course||team(rdram)!=p->state.retry->post_death_team)return {};
 const auto a=word(rdram,0x800F8828);const auto birth=p->births.find(a);const auto color=word(rdram,0x807F0008);if(birth==p->births.end()||birth->second.serial<=p->state.retry->dead_birth||!actor(rdram,a,0x4001)||MEM_BU(0x18,at(a))==0xA||color>=18||!(team(rdram)&(1u<<color))||!scale(rdram,a)||MEM_BU(0x165,at(a))!=0||word(rdram,a+0x130)!=0x800233B4)return {};
 const auto data=word(rdram,a+0x1F4);if(!heap(data,0x1300)||word(rdram,data+0x14)!=1)return {};const unsigned hp=MEM_HU(0,at(0x800F897E)),eggs=MEM_HU(0x184,at(a));if(hp>8||eggs>6)return {};
 std::set<uint32_t> unique;for(unsigned i=0;i<eggs;++i){const auto child=word(rdram,a+0x188+i*4);const auto item=p->births.find(child);if(!unique.insert(child).second||item==p->births.end()||item->second.serial<=p->state.retry->dead_birth||!heap(child,0x1FC)||word(rdram,child)!=0x19971110||word(rdram,child+0x180)!=a||!scale(rdram,child)||(MEM_HU(12,at(child))!=0x4004&&MEM_HU(12,at(child))!=0x4002))return {};}
 const auto* root=p->catalog.find(p->state.retry->room);if(!root)return {};
 const auto expected=p->state.retry->checkpoint?p->state.retry->checkpoint->point:root->spawn.point;
 if(word(rdram,a+0x3C)!=expected[0]||word(rdram,a+0x40)!=expected[1])return {};
 const Receipt result{*p->state.retry,*p->admission,a,birth->second.serial,hp,eggs};p->state.phase=Phase::Arrived;return result;
}
}
