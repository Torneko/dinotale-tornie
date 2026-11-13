#include "bonus_plan0351.hpp"
#include <set>
#include <stdexcept>
namespace workshop::bonus_plan0351 {
namespace {
void require(bool value,const char* why){if(!value)throw std::invalid_argument(why);}
void validate_snapshot(const bonus_session0351::Snapshot& state,uint64_t revision){
 require(state.readable&&state.last_write_succeeded,"Bonus plan requires readable committed session state");
 require(state.revision==revision,"Bonus plan session revision is stale");
 require((state.completed_mask&~63u)==0,"Bonus plan completion mask is invalid");
 for(unsigned i=0;i<6;++i){const auto& slot=state.attempts[i];
  if(!slot){require(!(state.completed_mask&(1u<<i)),"Bonus completion lacks an attempt");continue;}
  require(slot->bonus==i+1&&bonus_session0351::valid(*slot),"Bonus plan attempt is invalid");
  require(slot->phase!=bonus_session0351::Phase::Completed||(state.completed_mask&(1u<<i)),"Completed attempt lacks completion receipt");
 }
}
void validate_origins(const bonus_rooms0351::Catalog& catalog){
 require(catalog.source_sha256==bonus_rooms0351::original_sha256&&catalog.rooms.size()==113,"Bonus plan catalogue identity is invalid");
 std::set<unsigned> identities;
 for(const auto& room:catalog.rooms){const auto source=bonus_routes0351::source_key(room.id);
  require(source&&source->course&&source->root&&*source->course==room.course&&*source->root==room.root&&room.world==room.course/4+1,"Bonus catalogue origin disagrees with route origin");
  require(identities.insert(room.id).second,"Bonus catalogue contains duplicate room identities");
  const auto& course=catalog.courses.at(room.course);
  require(course.index==room.course&&course.root==room.root&&course.world==room.world,"Bonus course identity is invalid");
 }
 unsigned expected=0;
 for(unsigned b=1;b<=6;++b)for(const auto& source:bonus_routes0351::world_pool(b)){
  require(identities.contains(source.room),"Bonus catalogue is missing a source room");++expected;
 }
 require(expected==113&&identities.size()==113,"Bonus catalogue coverage is incomplete");
}
}
Plan build(std::span<const uint8_t> original,const bonus_routes0351::Route& route,
           const bonus_session0351::Snapshot& state,uint64_t revision){
 require(bonus_routes0351::validate(route),"Bonus plan route is not canonical");
 validate_snapshot(state,revision);const auto& slot=state.attempts[route.bonus-1];
 require(slot&&slot->seed==route.seed&&slot->bonus==route.bonus,"Bonus plan route seed is stale");
 require(slot->order.size()==route.steps.size(),"Bonus plan route order is stale");
 for(unsigned i=0;i<route.steps.size();++i)require(slot->order[i]==route.steps[i].key.room,"Bonus plan route order is stale");
 const auto catalog=bonus_rooms0351::read(original);validate_origins(catalog);
 Plan plan;plan.original_sha256=catalog.source_sha256;plan.bonus=route.bonus;plan.seed=route.seed;
 plan.revision=revision;plan.cursor=slot->cursor;plan.phase=slot->phase;plan.steps.reserve(route.steps.size());
 for(unsigned i=0;i<route.steps.size();++i){
  Step step;step.ordinal=i;step.route_step=route.steps[i];const auto* room=catalog.find(step.route_step.key.room);
  if(room){require(step.route_step.key.course==room->course&&step.route_step.key.root==room->root,"Bonus step origin is inconsistent");step.known_anchor=room->entry;step.unplaced_candidates=room->candidates;}
  else require(!step.route_step.key.course&&!step.route_step.key.root&&step.route_step.role==bonus_routes0351::StepRole::Finale,"External ordinary source room is unsupported");
  if(step.route_step.role==bonus_routes0351::StepRole::Finale)step.entry=EntryState::FinaleNeedsSetup;
  else if(step.known_anchor){
   require(step.known_anchor->room==step.route_step.key.room,"Bonus anchor refers to another room");
   if(step.known_anchor->kind==bonus_rooms0351::AnchorKind::PlayerSpawn)
    step.entry=room&&room->id==room->root?EntryState::NativeRootSpawn:EntryState::NativeSpawnKnown;
   else if(step.known_anchor->kind==bonus_rooms0351::AnchorKind::PlacedIncoming)step.entry=EntryState::PlacedTargetKnown;
   else throw std::invalid_argument("Unplaced native target was promoted to a known entry");
  }else if(!step.unplaced_candidates.empty())step.entry=EntryState::UnplacedNativeTarget;
  for(const auto& candidate:step.unplaced_candidates)require(candidate.target.kind==bonus_rooms0351::AnchorKind::UnplacedNativeTarget&&candidate.target.room==step.route_step.key.room,"Bonus candidate identity is invalid");
  if(i>0)step.previous=Link{i-1};if(i+1<route.steps.size())step.next=Link{i+1};
  plan.steps.push_back(std::move(step));
 }
 return plan;
}
} // namespace workshop::bonus_plan0351
