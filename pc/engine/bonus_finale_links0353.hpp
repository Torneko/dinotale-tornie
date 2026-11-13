#pragma once
#include "bonus_transport0352.hpp"
#include "bonus_finale_entries0353.hpp"
namespace workshop::bonus_finale_links0353 {
// Pure preparation of ONE adjacent last-ordinary -> finale link. The caller
// owns the reserved slot and remains responsible for fresh factory receipts,
// any shared-site L direction map, native admission and session ticket checks.
// No backwards pot is introduced inside the arena; no native player is made.
bonus_transport0352::PreparedLink make_link(std::span<const uint8_t> original,
    const bonus_routes0351::Route&,const bonus_transport0352::Environment&,
    const bonus_finale_entries0353::EntrySpec&,unsigned slab_slot);
}
