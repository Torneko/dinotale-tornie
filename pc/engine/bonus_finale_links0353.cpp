#include "bonus_finale_links0353.hpp"
#include "workshop_data.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
namespace workshop::bonus_finale_links0353 {
namespace {
void require(bool value,const char* reason){if(!value)throw std::invalid_argument(reason);}
bool permitted(const bonus_transport0352::Environment& e){return e.pc_mode==7&&e.bonus>=1&&e.bonus<=6&&e.attempt_epoch&&!e.canonical&&!e.custom_active&&!e.deferred;}
}
bonus_transport0352::PreparedLink make_link(std::span<const uint8_t> original,
 const bonus_routes0351::Route& route,const bonus_transport0352::Environment& env,
 const bonus_finale_entries0353::EntrySpec& entry,unsigned slot){
 namespace tx=bonus_transport0352;namespace ar=bonus_authoring0353;namespace rooms=bonus_rooms0351;
 require(permitted(env)&&bonus_routes0351::validate(route)&&route.bonus==env.bonus&&route.seed==env.seed,"Finale link environment/canonical route mismatch");
 require(slot<tx::max_links&&tx::max_links<=1536&&uint64_t(tx::slab)+uint64_t(slot+1)*tx::slot_bytes<=tx::slab_end,"Finale link owned slab slot invalid");
 require(bonus_finale_entries0353::valid(original,route,entry),"Finale entry is not freshly validated against original/route");
 const auto from=entry.from_ordinal,to=from+1;require(uint64_t(slot)==uint64_t(from)*2,"Finale link must append exactly after all ordinary two-way slots");
 require(to+1==route.steps.size()&&route.steps[from].role==bonus_routes0351::StepRole::Ordinary&&route.steps[to].role==bonus_routes0351::StepRole::Finale&&route.steps[from].key==entry.from_key&&route.steps[to].key.room==entry.room,"Finale link must be the single canonical last-ordinary boundary");
 const auto plan=ar::build(original,route);const auto catalog=rooms::read(original);
 const auto room=std::find_if(plan.rooms.begin(),plan.rooms.end(),[&](const auto& r){return r.ordinal==from&&r.role==bonus_routes0351::StepRole::Ordinary&&r.key==entry.from_key;});
 require(room!=plan.rooms.end()&&room->forward&&room->key.course&&room->key.root,"Finale source has no actual authored forward site");
 const auto& site=*room->forward;require(site.room==room->key.room&&site.ordinal==from&&site.direction==ar::Direction::Forward,"Finale source site identity/direction mismatch");
 const auto* source=catalog.find(room->key.room);require(source&&source->course==*room->key.course&&source->root==*room->key.root&&source->world==env.bonus,"Finale source native provenance mismatch");
 tx::PreparedLink link;link.ordinal={from,to,site.source_placement_index.value_or(UINT32_MAX)};link.source_room=source->id;link.source_course=source->course;link.destination_room=entry.room;link.destination_course=entry.course;link.destination_root=entry.root;link.record_address=tx::slab+slot*tx::slot_bytes;link.requires_factory=site.needs_factory;
 if(site.reuse_native_pot){const auto native=std::find_if(source->passages.begin(),source->passages.end(),[&](const auto& p){return p.native.editable_pot&&p.source.index==site.source_placement_index&&p.source.packed==site.source_packed&&p.source.point==site.point;});require(native!=source->passages.end()&&!site.needs_factory,"Finale source native pot alias/payload mismatch");link.source=native->source;link.native_payload=native->native.native_payload;
 }else{require(site.needs_factory&&site.template_kind==0x4208,"Finale dynamic source needs authentic4208 factory template");const auto* calibration=catalog.find(43);require(calibration,"Finale4208 calibration source missing");const auto prototype=std::find_if(calibration->passages.begin(),calibration->passages.end(),[](const auto& p){return p.source.index==119&&p.native.kind==0x4208&&p.native.editable_pot;});require(prototype!=calibration->passages.end(),"Finale authentic4208 source payload missing");link.source={UINT32_MAX,0,0x42082000u,site.point};link.native_payload=prototype->native.native_payload;}
 // This is a real prepared44CA marker, not an original catalogue entry for
 // external arenas. Explicit EntrySpec course policy is retained above.
 link.arrival={rooms::AnchorKind::UnplacedNativeTarget,entry.room,{entry.marker_index,0,0x44CA0000u,entry.point},{},{entry.prototype_kind}};
 ar::Site contact;contact.room=entry.room;contact.ordinal=to;contact.direction=ar::Direction::Backward;contact.point=entry.point;contact.provenance=ar::Provenance::NativeArrival;contact.source_placement_index=entry.marker_index;contact.source_packed=0x44CA0000u;contact.template_kind=0x44CA;contact.needs_factory=false;contact.needs_dynamic_admission=true;link.authored_arrival=contact;
 // Payload MUST stay the native source pot's exact24 bytes for bind_factory/
 // prepare ownership. Transport later overrides target=-1, room and camera
 // in its private clone while carrying the SAME native4001. The destination
 // EntrySpec's4268/44CA payload is provenance, never a forged source payload.
 require(navigation::coordinate(site.point[0])&&navigation::coordinate(site.point[1])&&(site.point[0]||site.point[1]),"Finale source site point invalid");
 return link;
}
}
