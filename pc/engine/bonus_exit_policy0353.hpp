#pragma once
#include "bonus_authoring0353.hpp"
#include "bonus_transport0352.hpp"
#include <memory>
namespace workshop::bonus_exit_policy0353 {
using Environment=bonus_transport0352::Environment;
enum class RetireReason { UnselectedNativeExit, OriginalDynamicPot };
struct RetireAction {
 unsigned ordinal=0,room=177,placement_index=0;uint32_t actor=0,packed=0;
 bonus_rooms0351::Point point{};uint64_t birth=0,attempt_epoch=0,room_epoch=0;
 RetireReason reason=RetireReason::UnselectedNativeExit;std::array<uint8_t,24> expected_native_payload{};
 bool operator==(const RetireAction&)const=default;
};
struct View {bool configured=false,ready=false;unsigned expected=0,observed=0,queued=0,blocked_children=0;};
// Single game thread. This module never removes ROM placements or calls native
// destructors. Root validates an action immediately before800447C8, then
// acknowledges its real deferred-retirement state. General slots free later.
class Policy {
public:
 Policy();~Policy();Policy(const Policy&)=delete;Policy&operator=(const Policy&)=delete;
 bool configure(std::span<const uint8_t> original,const bonus_routes0351::Route&,const Environment&);
 void reset();
 bool enter_room(unsigned ordinal,uint64_t room_epoch,const Environment&);
 bool room_ready(uint8_t*,uint32_t level_info,const Environment&);
 void created(recomp_context*,unsigned room,const Environment&);
 void destroyed(uint32_t actor,const Environment&);
 std::vector<RetireAction> pending(uint8_t*,const Environment&)const;
 bool validate(const RetireAction&,uint8_t*,const Environment&)const;
 bool acknowledge(const RetireAction&,uint8_t*,const Environment&);
 View view(uint8_t*,const Environment&)const;
private:struct Impl;std::unique_ptr<Impl> p;
};
}
