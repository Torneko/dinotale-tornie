#include "bonus_runtime0352.hpp"
#include "story_rights0320.hpp"
#include "json/json.hpp"
#include <fstream>
#include <set>
#include <system_error>
namespace workshop::bonus_runtime0352 {
namespace {
namespace fs=std::filesystem;
struct Hybrid {uint32_t mask=0;bool present=false,readable=true;std::string reason;};
Hybrid hybrid_read(const fs::path& path){
 Hybrid result;
 try{
  std::error_code error;const auto state=fs::symlink_status(path,error);
  if(error==std::errc::no_such_file_or_directory||(!error&&!fs::exists(state)))return result;
  if(error||fs::is_symlink(state)||!fs::is_regular_file(state))throw std::runtime_error("hybrid-source-not-regular");
  result.present=true;std::ifstream file(path,std::ios::binary);if(!file)throw std::runtime_error("hybrid-source-open-failed");
  std::string bytes(4097,'\0');file.read(bytes.data(),std::streamsize(bytes.size()));bytes.resize(size_t(file.gcount()));
  if(bytes.size()>4096||(!file.eof()&&!file))throw std::runtime_error("hybrid-source-read-failed");
  bool duplicate=false;std::vector<std::set<std::string>> keys;
  auto callback=[&](int,nlohmann::json::parse_event_t event,nlohmann::json& value){
   if(event==nlohmann::json::parse_event_t::object_start)keys.emplace_back();
   else if(event==nlohmann::json::parse_event_t::key){if(keys.empty()||!keys.back().insert(value.get<std::string>()).second)duplicate=true;}
   else if(event==nlohmann::json::parse_event_t::object_end&&!keys.empty())keys.pop_back();return true;
  };
  const auto j=nlohmann::json::parse(bytes,callback);
  auto natural=[](const nlohmann::json& v,uint64_t maximum){return v.is_number_unsigned()&&v.get<uint64_t>()<=maximum;};
  if(duplicate||!j.is_object()||j.size()!=5||!j.contains("schema")||!natural(j["schema"],1)||j["schema"]!=1u||
   !j.contains("observedMask")||!natural(j["observedMask"],0xFFFFFF)||!j.contains("legacyMask")||!natural(j["legacyMask"],0xFFFFFF)||
   !j.contains("legacyChecked")||!j["legacyChecked"].is_boolean()||!j.contains("legacyChecksum")||
   (!j["legacyChecksum"].is_null()&&!natural(j["legacyChecksum"],UINT32_MAX)))throw std::runtime_error("hybrid-source-format-invalid");
  const auto observed=j["observedMask"].get<uint32_t>(),legacy=j["legacyMask"].get<uint32_t>();
  if((!j["legacyChecked"].get<bool>()&&(legacy||!j["legacyChecksum"].is_null()))||(legacy&&j["legacyChecksum"].is_null()))throw std::runtime_error("hybrid-source-provenance-invalid");
  result.mask=observed|legacy;
 }catch(const std::exception& error){result.mask=0;result.readable=false;result.reason=error.what();}
 return result;
}
Status mapped(bonus_session0351::Status status){
 using S=bonus_session0351::Status;
 switch(status){
  case S::Committed:case S::Unchanged:return Status::Accepted;
  case S::Locked:return Status::Locked;
  case S::InvalidArgument:return Status::InvalidArgument;
  case S::WrongRoom:return Status::WrongRoom;
  case S::Unavailable:return Status::PersistenceUnavailable;
  case S::PersistenceFailed:return Status::PersistenceFailed;
  case S::Conflict:return Status::Conflict;
  default:return Status::InvalidState;
 }
}
}
void Service::invalidate_locked(){++epoch_;ticket_.reset();ready_=false;}
Admission Service::reply_locked(Status value,bool with_ticket){status_=value;return {value,with_ticket?ticket_:std::nullopt};}
void Service::access_locked(){
 rights_={};if(!initialized_){rights_.reason="bonus-profile-unavailable";return;}
 const auto canonical=story_rights0320::read(profile_/"saves"/"yoshisstory.n64.us.bin");
 const auto hybrid=hybrid_read(profile_/"pc-hybrid-completion.json");
 rights_.classic_present=canonical.present;rights_.classic_readable=canonical.readable;
 rights_.hybrid_present=hybrid.present;rights_.hybrid_readable=hybrid.readable;
 rights_.classic_mask=canonical.present&&canonical.readable?canonical.trial:0;
 rights_.hybrid_mask=hybrid.readable?hybrid.mask:0;
 rights_.unlocked_mask=bonus_routes0351::unlocked_mask(rights_.classic_mask,rights_.hybrid_mask);
 if(!canonical.readable)rights_.reason="canonical:"+canonical.reason;
 if(!hybrid.readable){if(!rights_.reason.empty())rights_.reason+=';';rights_.reason+="hybrid:"+hybrid.reason;}
}
bool Service::load(const fs::path& profile){
 std::lock_guard guard(mutex_);invalidate_locked();selected_=0;bridge_enabled_=false;authored_enabled_=false;authored_bonus_=0;authored_seed_=0;finale_entry_.reset();finale_attempt_epoch_=0;initialized_=false;profile_.clear();rights_={};status_=Status::PersistenceUnavailable;
 (void)store_.load({});
 try{
  if(profile.empty())throw std::runtime_error("bonus-profile-missing");const auto absolute=fs::absolute(profile);
  if(!fs::is_directory(absolute)||fs::is_symlink(fs::symlink_status(absolute)))throw std::runtime_error("bonus-profile-not-directory");
  profile_=fs::canonical(absolute);initialized_=store_.load(profile_);access_locked();return initialized_;
 }catch(const std::exception& error){rights_.reason=error.what();return false;}
}
void Service::refresh_access(){std::lock_guard guard(mutex_);access_locked();}
Access Service::access()const{std::lock_guard guard(mutex_);return rights_;}
bonus_session0351::Snapshot Service::session()const{std::lock_guard guard(mutex_);return store_.snapshot();}
std::optional<Ticket> Service::current_ticket()const{std::lock_guard guard(mutex_);return ticket_;}
Status Service::status()const{std::lock_guard guard(mutex_);return status_;}
void Service::enable_private_bridge(bool allowed){std::lock_guard guard(mutex_);bridge_enabled_=allowed;if(!allowed)invalidate_locked();}
bool Service::enable_private_authored(const bonus_routes0351::Route& route,bool allowed){
 std::lock_guard guard(mutex_);authored_enabled_=false;authored_bonus_=0;authored_seed_=0;finale_entry_.reset();finale_attempt_epoch_=0;
 if(!allowed||!initialized_||!bonus_routes0351::validate(route)){invalidate_locked();return false;}
 authored_enabled_=true;authored_bonus_=route.bonus;authored_seed_=route.seed;return true;
}
bool Service::enable_private_finale(std::span<const uint8_t> original,const bonus_routes0351::Route& route,const bonus_finale_entries0353::EntrySpec& entry,uint64_t native_epoch,bool allowed){
 std::lock_guard guard(mutex_);finale_entry_.reset();finale_attempt_epoch_=0;
 if(!allowed||!initialized_||!authored_enabled_||!native_epoch||route.bonus!=authored_bonus_||route.seed!=authored_seed_||!bonus_routes0351::validate(route)||!bonus_finale_entries0353::valid(original,route,entry)){invalidate_locked();return false;}
 finale_entry_=entry;finale_attempt_epoch_=native_epoch;return true;
}
bool Service::finale_locked(unsigned bonus,uint64_t seed)const{
 return finale_entry_&&finale_attempt_epoch_&&finale_entry_->bonus==bonus&&finale_entry_->seed==seed;
}
bool Service::policy_locked(LaunchPolicy policy,unsigned bonus,uint64_t seed,unsigned ordinal)const{
 if(policy==LaunchPolicy::PrivateTwoRoomBridge)return bridge_enabled_&&ordinal<2;
 if((policy!=LaunchPolicy::PrivateAuthoredOrdinary&&policy!=LaunchPolicy::PrivateAuthoredComplete)||!authored_enabled_||bonus!=authored_bonus_||seed!=authored_seed_)return false;
 const auto route=bonus_routes0351::make_route(bonus,seed);if(ordinal>=route.steps.size())return false;
 if(policy==LaunchPolicy::PrivateAuthoredOrdinary)return route.steps[ordinal].role==bonus_routes0351::StepRole::Ordinary;
 if(!finale_locked(bonus,seed))return false;
 return route.steps[ordinal].role==bonus_routes0351::StepRole::Ordinary||(ordinal+1==route.steps.size()&&route.steps[ordinal].key.room==finale_entry_->room);
}
Admission Service::new_attempt(unsigned bonus,uint64_t seed){
 std::lock_guard guard(mutex_);if(!initialized_)return reply_locked(Status::PersistenceUnavailable);
 access_locked();if(bonus<1||bonus>6)return reply_locked(Status::InvalidArgument);
 if(!(rights_.unlocked_mask&(1u<<(bonus-1))))return reply_locked(Status::Locked);
 if(!store_.refresh())return reply_locked(Status::PersistenceUnavailable);
 const auto result=store_.new_attempt(store_.snapshot().revision,bonus,seed,rights_.classic_mask,rights_.hybrid_mask);
 if(!result.accepted())return reply_locked(mapped(result.status));selected_=bonus;invalidate_locked();return reply_locked(Status::Accepted);
}
Admission Service::resume(unsigned bonus){
 std::lock_guard guard(mutex_);if(!initialized_)return reply_locked(Status::PersistenceUnavailable);
 access_locked();if(bonus<1||bonus>6)return reply_locked(Status::InvalidArgument);
 if(!(rights_.unlocked_mask&(1u<<(bonus-1))))return reply_locked(Status::Locked);
 if(!store_.refresh())return reply_locked(Status::PersistenceUnavailable);const auto saved=store_.snapshot();const auto& slot=saved.attempts[bonus-1];
 if(!slot)return reply_locked(Status::NoAttempt);if(slot->phase==bonus_session0351::Phase::Completed)return reply_locked(Status::Completed);
 selected_=bonus;invalidate_locked();return reply_locked(Status::Accepted);
}
Admission Service::prepare_locked(LaunchPolicy policy){
 if(policy==LaunchPolicy::FullRoute)return reply_locked(Status::NeedsAuthoredRoute);
 if(policy!=LaunchPolicy::PrivateAuthoredOrdinary&&policy!=LaunchPolicy::PrivateAuthoredComplete&&!bridge_enabled_)return reply_locked(Status::NeedsAuthoredRoute);
 if(!initialized_||selected_<1||selected_>6)return reply_locked(Status::NoAttempt);
 access_locked();if(!(rights_.unlocked_mask&(1u<<(selected_-1))))return reply_locked(Status::Locked);
 if(!store_.refresh())return reply_locked(Status::PersistenceUnavailable);const auto saved=store_.snapshot();const auto& slot=saved.attempts[selected_-1];
 if(!slot)return reply_locked(Status::NoAttempt);if(!bonus_session0351::valid(*slot))return reply_locked(Status::InvalidState);
 if(slot->phase==bonus_session0351::Phase::Completed)return reply_locked(Status::Completed);
 const auto route=bonus_routes0351::make_route(selected_,slot->seed);const auto& step=route.steps[slot->cursor];
 if(step.role==bonus_routes0351::StepRole::Finale&&(policy!=LaunchPolicy::PrivateAuthoredComplete||!finale_locked(selected_,slot->seed)))return reply_locked(Status::NeedsFinaleSetup);
 if(!policy_locked(policy,selected_,slot->seed,slot->cursor))return reply_locked(Status::NeedsAuthoredRoute);
 const bool finale=step.role==bonus_routes0351::StepRole::Finale;
 if(!finale&&(!step.key.course||!step.key.root))return reply_locked(Status::InvalidState);
 const unsigned course=finale?finale_entry_->course:*step.key.course,root=finale?finale_entry_->root:*step.key.root;
 invalidate_locked();ticket_=Ticket{epoch_,saved.revision,slot->seed,selected_,slot->cursor,step.key.room,course,root,step.entry,policy,bonus_save};
 return reply_locked(Status::Ready,true);
}
Admission Service::prepare(LaunchPolicy policy){std::lock_guard guard(mutex_);return prepare_locked(policy);}
bool Service::ticket_locked(const Ticket& ticket)const{
 if(!initialized_||!ticket_||ticket!=*ticket_||!policy_locked(ticket.policy,ticket.bonus,ticket.seed,ticket.ordinal))return false;
 const auto saved=store_.snapshot();if(!saved.readable||saved.revision!=ticket.revision||ticket.bonus<1||ticket.bonus>6)return false;
 const auto& slot=saved.attempts[ticket.bonus-1];return slot&&slot->seed==ticket.seed&&slot->cursor==ticket.ordinal&&slot->order[ticket.ordinal]==ticket.room;
}
bool Service::observed_locked(const Ticket& ticket,const Observation& observation)const{return observation.room==ticket.room&&observation.course==ticket.course&&observation.root==ticket.root;}
Admission Service::room_ready(const Ticket& ticket,const Observation& observation){
 std::lock_guard guard(mutex_);if(!ticket_locked(ticket))return reply_locked(Status::StaleTicket);
 if(!observed_locked(ticket,observation))return reply_locked(Status::WrongRoom);
 if(!observation.player_ready)return reply_locked(Status::PlayerNotReady);
 if(ready_)return reply_locked(Status::Ready,true);
 const auto result=store_.enter(ticket.revision,ticket.bonus,ticket.room);if(!result.accepted())return reply_locked(mapped(result.status));
 ticket_->revision=result.revision;ready_=true;return reply_locked(Status::Ready,true);
}
Admission Service::depart(const Ticket& ticket,const Observation& observation,bool qualified,LaunchPolicy policy){
 std::lock_guard guard(mutex_);if(!ticket_locked(ticket)||ticket.policy!=policy)return reply_locked(Status::StaleTicket);
 if(!ready_)return reply_locked(Status::InvalidState);if(!observed_locked(ticket,observation))return reply_locked(Status::WrongRoom);
 if(!observation.player_ready)return reply_locked(Status::PlayerNotReady);if(!qualified)return reply_locked(Status::InvalidState);
 const auto saved=store_.snapshot();auto next=*saved.attempts[ticket.bonus-1];
 if(next.phase==bonus_session0351::Phase::Finale)return reply_locked(Status::NeedsFinaleSetup);
 if(bonus_session0351::advance(next,ticket.room,true)!=bonus_session0351::Transition::Changed)return reply_locked(Status::InvalidState);
 const auto route=bonus_routes0351::make_route(ticket.bonus,next.seed);
 if(next.phase==bonus_session0351::Phase::Completed)return reply_locked(Status::NeedsFinaleSetup);
 if(route.steps[next.cursor].role==bonus_routes0351::StepRole::Finale&&(policy!=LaunchPolicy::PrivateAuthoredComplete||!finale_locked(ticket.bonus,next.seed)))return reply_locked(Status::NeedsFinaleSetup);
 if(!policy_locked(policy,ticket.bonus,next.seed,next.cursor))return reply_locked(Status::NeedsAuthoredRoute);
 const auto result=store_.advance(ticket.revision,ticket.bonus,ticket.room,true);if(!result.accepted())return reply_locked(mapped(result.status));
 return prepare_locked(policy);
}
Admission Service::complete_finale(const Ticket& ticket,const FinaleReceipt& receipt){
 std::lock_guard guard(mutex_);if(!ticket_locked(ticket)||ticket.policy!=LaunchPolicy::PrivateAuthoredComplete)return reply_locked(Status::StaleTicket);
 if(!ready_)return reply_locked(Status::PlayerNotReady);
 const auto saved=store_.snapshot();const auto& slot=saved.attempts[ticket.bonus-1];const auto route=bonus_routes0351::make_route(ticket.bonus,ticket.seed);
 if(!slot||slot->phase!=bonus_session0351::Phase::Finale||ticket.ordinal+1!=route.steps.size()||route.steps[ticket.ordinal].role!=bonus_routes0351::StepRole::Finale||!finale_locked(ticket.bonus,ticket.seed))return reply_locked(Status::InvalidState);
 if(receipt.bonus!=ticket.bonus||receipt.seed!=ticket.seed||receipt.attempt_epoch!=finale_attempt_epoch_||receipt.ticket_epoch!=ticket.epoch||receipt.room!=ticket.room||receipt.course!=ticket.course||!receipt.arena_generation||receipt.defeated_count!=route.finale.members)return reply_locked(Status::InvalidState);
 const auto result=store_.advance(ticket.revision,ticket.bonus,ticket.room,true);if(!result.accepted())return reply_locked(mapped(result.status));
 if(store_.snapshot().attempts[ticket.bonus-1]->phase!=bonus_session0351::Phase::Completed)return reply_locked(Status::InvalidState);
 invalidate_locked();return reply_locked(Status::Completed);
}
Admission Service::move_locked(const Ticket& ticket,unsigned index,LaunchPolicy policy,bool backward){
 if(!ticket_locked(ticket)||ticket.policy!=policy)return reply_locked(Status::StaleTicket);
 if(!policy_locked(policy,ticket.bonus,ticket.seed,index))return reply_locked(Status::NeedsAuthoredRoute);
 const auto result=backward?store_.backtrack(ticket.revision,ticket.bonus,index):store_.retry(ticket.revision,ticket.bonus,index);
 if(!result.accepted())return reply_locked(mapped(result.status));return prepare_locked(policy);
}
Admission Service::backtrack(const Ticket& ticket,const Observation& observation,unsigned index,LaunchPolicy policy){
 std::lock_guard guard(mutex_);if(!ticket_locked(ticket))return reply_locked(Status::StaleTicket);
 if(!ready_)return reply_locked(Status::InvalidState);if(!observed_locked(ticket,observation))return reply_locked(Status::WrongRoom);
 if(!observation.player_ready)return reply_locked(Status::PlayerNotReady);return move_locked(ticket,index,policy,true);
}
Admission Service::retry(const Ticket& ticket,unsigned index,LaunchPolicy policy){std::lock_guard guard(mutex_);return move_locked(ticket,index,policy,false);}
void Service::cancel(){std::lock_guard guard(mutex_);selected_=0;invalidate_locked();status_=Status::Idle;}
} // namespace workshop::bonus_runtime0352
