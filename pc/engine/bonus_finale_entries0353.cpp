#include "bonus_finale_entries0353.hpp"
#include "bonus_finales0353.hpp"
#include "boss_rooms_data0250.hpp"
#include "workshop_data.hpp"
#include <algorithm>
#include <bit>
#include <stdexcept>
namespace workshop::bonus_finale_entries0353 {
namespace {
void require(bool value,const char* reason){if(!value)throw std::invalid_argument(reason);}
std::vector<std::array<uint32_t,3>> geometry(const std::vector<bonus_finales0353::Placement>& objects){
 std::vector<std::array<uint32_t,3>> result;for(const auto& p:objects){const auto k=p.packed>>16;if(k>=0x8000||k==0x415E||k==0x415F||k==0x464F||k==0x4592||k==0x4593||k==0x4671||k==0x4515)result.push_back({p.packed,p.x,p.y});}return result;
}
}
EntrySpec prepare(std::span<const uint8_t> original,const bonus_routes0351::Route& route,unsigned previous_ordinal,const bonus_routes0351::RoomKey& previous){
 require(bonus_routes0351::validate(route),"Finale entry route is not canonical");
 require(previous_ordinal<route.steps.size()&&previous_ordinal+2==route.steps.size()&&route.steps[previous_ordinal].role==bonus_routes0351::StepRole::Ordinary&&route.steps.back().role==bonus_routes0351::StepRole::Finale&&route.steps[previous_ordinal].key==previous,"Finale entry requires exact last ordinary predecessor");
 const auto catalog=bonus_rooms0351::read(original);const auto* from=catalog.find(previous.room);
 require(from&&previous.course&&previous.root&&from->course==*previous.course&&from->root==*previous.root&&catalog.courses[*previous.course].root==*previous.root,"Finale predecessor origin differs from native catalogue");
 const auto plan=bonus_finales0353::prepare(original,route.bonus);require(bonus_finales0353::valid(plan)&&plan.arenas.size()==1&&plan.arenas[0].room==route.steps.back().key.room,"Prepared finale and canonical route differ");
 const auto& arena=plan.arenas[0];EntrySpec result;result.bonus=route.bonus;result.seed=route.seed;result.from_ordinal=previous_ordinal;result.from_key=previous;result.room=arena.room;
 constexpr std::array<unsigned,6> courses{6,11,8,13,9,23},roots{16,14,52,22,40,24};
 result.course=courses[route.bonus-1];result.root=roots[route.bonus-1];require(catalog.courses[result.course].root==result.root,"Declared finale course policy is not a real native course");
 std::optional<unsigned> warm;for(unsigned i=0;i<arena.placements.size();++i)if(arena.placements[i].packed==0x44CA0000){require(!warm,"Finale warm marker is ambiguous");warm=i;}
 require(warm.has_value(),"Prepared finale has no real warm marker");result.marker_index=*warm;const auto& marker=arena.placements[*warm];result.point={marker.x,marker.y};require(navigation::coordinate(marker.x)&&navigation::coordinate(marker.y)&&(marker.x||marker.y),"Finale warm marker is invalid/zero");
 navigation::Source source(original);const auto prototype=source.passage(result.prototype_kind);require(prototype&&prototype->destination==97&&prototype->target==result.target_packed,"Authentic44CA prototype4268 differs");result.prototype_record_rom=prototype->record;result.native_payload=source.native_payload(*prototype);result.native_payload[4]=uint8_t(result.room);
 for(const auto& room:catalog.rooms)for(const auto& p:room.passages)if(p.native.destination==result.room&&p.native_target)result.original_incoming.push_back({room.id,room.course,room.root,p.source.index,p.native.kind,p.source.packed,p.native.target_packed,p.native_target->point,p.native.resource_a,p.native.record_rom,p.native.native_payload});
 if(route.bonus==1){result.origin=Origin::NativeIncomingContactAuthoredWarmMarker;require(std::any_of(result.original_incoming.begin(),result.original_incoming.end(),[&](const auto& p){return p.source_kind==0x426D&&p.target_packed==0x426E2000&&p.target==result.point;}),"Marmot warm marker lost native incoming contact");}
 else if(route.bonus==4){result.origin=Origin::PriorCompactWarmMarker;require(arena.compact_fish_geometry&&marker.origin==bonus_finales0353::Origin::PriorAuthoredArena,"Compact fish marker has no prior authored provenance");}
 else{result.origin=Origin::OriginalWarmMarker;const auto originals=source.objects(result.room);unsigned same=0;for(const auto& p:originals)same+=p.packed==0x44CA0000&&p.x==marker.x&&p.y==marker.y;require(same==1,"Original warm marker contact changed");}
 if(route.bonus!=6){const auto& prior=boss_rooms_data0250::base_arena(result.room);result.prior_contact=Point{std::bit_cast<uint32_t>(prior.entry_x),std::bit_cast<uint32_t>(prior.entry_y)};std::vector<bonus_finales0353::Placement> old;for(unsigned i=0;i<prior.objects.size()/12;++i)old.push_back({u32(prior.objects,i*12),u32(prior.objects,i*12+4),u32(prior.objects,i*12+8),bonus_finales0353::Origin::PriorAuthoredArena,i});result.prior_geometry_data_match=geometry(old)==geometry(arena.placements);if(route.bonus==4)require(result.prior_geometry_data_match,"Compact fish geometry changed from prior arena data");}
 if(route.bonus==4)require(result.prior_contact==result.point,"Compact fish warm marker differs from prior contact");
 return result;
}
bool valid(std::span<const uint8_t> original,const bonus_routes0351::Route& route,const EntrySpec& spec)noexcept{try{return spec==prepare(original,route,spec.from_ordinal,spec.from_key);}catch(...){return false;}}
}
