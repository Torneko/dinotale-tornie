#pragma once
#include "bonus_finales0353.hpp"
#include "bonus_routes0351.hpp"
#include <functional>
namespace workshop::bonus_finale_bridge0353 {
struct Environment {int pc_mode=-1;unsigned bonus=0;uint64_t seed=0,attempt_epoch=0;bool independent_save=false,canonical=true,custom=true,deferred=true;};
struct Frame {unsigned vi=0;std::span<const int32_t> overlays{};};
using NativeInvoke=std::function<void(uint32_t,uint8_t*,recomp_context*)>;
struct Completion {unsigned bonus=0,room=177,course=24,defeated_count=0;uint64_t seed=0,attempt_epoch=0,ticket_epoch=0,arena_generation=0;};
struct View {
 bool configured=false,entered=false,failed=false,arrival_protocol_pending=true,
      supply_or_color_adapter_pending=true,bowser_adapter_pending=false,bowser_cadence_observed=false,completion_available=false;
 unsigned bonus=0,room=177,course=24,registered_bosses=0,observed_constructors=0;
 uint64_t seed=0,attempt_epoch=0,ticket_epoch=0,arena_generation=0;
 bonus_finales0353::Snapshot evidence{};std::string reason;
 static constexpr bool physical_gameplay_verified=false;
};
// Native game-thread only. No journal, reward, save, Story or campaign singleton
// mutation. The Root caller owns admission/transport/factory and may accept a
// one-shot completion only against its own current finale ticket.
class Bridge {
public:
 Bridge();~Bridge();Bridge(const Bridge&)=delete;Bridge&operator=(const Bridge&)=delete;
 bool configure(std::span<const uint8_t> original,const bonus_routes0351::Route&,const Environment&,
                std::array<bonus_finales0353::FishColor,2> colors={bonus_finales0353::FishColor::Red,bonus_finales0353::FishColor::Red});
 std::vector<uint8_t> prepare_content(std::vector<uint8_t> private_rom)const;
 bool enter_room(unsigned native_room,unsigned native_course,uint64_t ticket_epoch);
 // EVERY constructor entry must reach trace(), including nested native calls.
 // Wrapped hit entries are intentionally ignored here to avoid double delivery.
 void trace(uint8_t*,recomp_context*,std::string_view name,const Frame&);
 void wrapped_entry(uint8_t*,const recomp_context*,std::string_view name,const Frame&);
 void wrapped_leave(uint8_t*,const recomp_context*,std::string_view name,const Frame&);
 bool paint_begin(uint8_t*,uint32_t actor,bool outer_fish_draw,const Frame&);
 void paint_finish(uint8_t*);
 bool shared_completion(uint8_t*,const recomp_context*,const Frame&);
 void frame(uint8_t*,recomp_context*,const Frame&,const NativeInvoke&);
 void bowser_arm(uint8_t*,const recomp_context*,const Frame&);
 bool bowser_commit(uint8_t*,const recomp_context*,const Frame&);
 bool bowser_midpoint_due(uint8_t*,const recomp_context*,const Frame&);
 bool bowser_preserve_midpoint(uint8_t*,const recomp_context*,const Frame&);
 std::optional<Completion> take_completion();
 View view()const;
 void reset();
private:struct Impl;std::unique_ptr<Impl> p;
};
} // namespace workshop::bonus_finale_bridge0353
