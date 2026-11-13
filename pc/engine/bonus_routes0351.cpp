#include "bonus_routes0351.hpp"
#include "vanilla_sources.hpp"
#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>

namespace workshop::bonus_routes0351 {
namespace {
void require_bonus(unsigned bonus){
 if(bonus<1||bonus>bonus_count)throw std::invalid_argument("Bonus number must be1..6");
}
void require_masks(uint32_t classic,uint32_t hybrid){
 if((classic|hybrid)&~course_mask)throw std::invalid_argument("Completion masks contain unknown courses");
}
// SplitMix64 and rejection sampling use specified unsigned arithmetic only;
// no implementation-defined standard-library distributions or global RNG.
class Random {
 uint64_t state;
public:
 explicit Random(uint64_t seed):state(seed){}
 uint64_t next(){
  uint64_t z=(state+=UINT64_C(0x9E3779B97F4A7C15));
  z=(z^(z>>30))*UINT64_C(0xBF58476D1CE4E5B9);
  z=(z^(z>>27))*UINT64_C(0x94D049BB133111EB);
  return z^(z>>31);
 }
 unsigned index(unsigned bound){
  const uint64_t width=bound,threshold=(uint64_t(0)-width)%width;
  uint64_t value;do{value=next();}while(value<threshold);
  return unsigned(value%width);
 }
};
FinaleSpec finale_for(unsigned bonus){
 switch(bonus){
  case 1:return {FinaleFamily::Marmots,{151},2,{},false};
  case 2:return {FinaleFamily::Lips,{165},1,{},false};
  case 3:return {FinaleFamily::YellowCandy,{162},2,{},false};
  case 4:return {FinaleFamily::Fish,{97},2,3,false};
  case 5:return {FinaleFamily::Genie,{163},2,3,false};
  // Bowser's native phases stay inside one arena. Rooms168/169 are not a
  // witnessed sequential route and must not add two fictitious battles.
  case 6:return {FinaleFamily::Bowser,{167},1,{},false};
 }
 throw std::invalid_argument("Bonus number must be1..6");
}
RoomKey arena_key(unsigned room){
 if(auto known=source_key(room))return *known;
 return {room,{},{}};
}
}
uint8_t unlocked_mask(uint32_t classic,uint32_t hybrid){
 require_masks(classic,hybrid);const uint32_t combined=classic|hybrid;uint8_t result=0;
 for(unsigned world=0;world<bonus_count;++world){
  const uint32_t page=uint32_t(0xFu)<<(world*4);
  if((combined&page)==page)result|=uint8_t(1u<<world);
 }
 return result;
}
bool unlocked(unsigned bonus,uint32_t classic,uint32_t hybrid){
 require_bonus(bonus);return (unlocked_mask(classic,hybrid)&(1u<<(bonus-1)))!=0;
}
std::vector<RoomKey> world_pool(unsigned bonus){
 require_bonus(bonus);std::vector<RoomKey> result;
 const unsigned first=(bonus-1)*4;
 for(unsigned course=first;course<first+4;++course)
  for(unsigned room:vanilla_rooms[course])if(room<177)
   result.push_back({room,course,vanilla_sources[course]});
 return result;
}
std::optional<RoomKey> source_key(unsigned room) noexcept{
 if(room>=177)return {};
 for(unsigned course=0;course<course_count;++course)
  for(unsigned candidate:vanilla_rooms[course])if(candidate<177&&candidate==room)
   return RoomKey{room,course,vanilla_sources[course]};
 return {};
}
Route make_route(unsigned bonus,uint64_t seed){
 auto pool=world_pool(bonus);const auto finale=finale_for(bonus);
 std::array<RoomKey,4> roots{};unsigned root_count=0;
 for(const auto& key:pool)if(key.root&&key.room==*key.root)roots.at(root_count++)=key;
 if(root_count!=4)throw std::logic_error("Vanilla catalogue does not have four source roots");
 Random random(seed^(UINT64_C(0xD1B54A32D192ED03)*uint64_t(bonus)));
 const auto first=roots[random.index(root_count)];
 std::erase_if(pool,[&](const RoomKey& key){
  return key==first || (bonus==4&&key.room==97);
 });
 for(std::size_t count=pool.size();count>1;--count)
  std::swap(pool[count-1],pool[random.index(unsigned(count))]);
 Route result;result.bonus=bonus;result.seed=seed;result.finale=finale;
 result.steps.push_back({StepRole::Ordinary,first,EntryRequirement::NativeRootSpawn});
 for(const auto& key:pool)result.steps.push_back({StepRole::Ordinary,key,EntryRequirement::NativeAnchorPending});
 for(unsigned room:finale.arena_sequence)
  result.steps.push_back({StepRole::Finale,arena_key(room),EntryRequirement::FinaleSetupPending});
 return result;
}
bool validate(const Route& route) noexcept{
 if(route.schema!=route_schema||route.bonus<1||route.bonus>bonus_count)return false;
 try{return route==make_route(route.bonus,route.seed);}catch(...){return false;}
}
} // namespace workshop::bonus_routes0351
