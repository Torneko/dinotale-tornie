#pragma once
#include "recomp.h"
#include <cstdint>
#include <optional>
namespace workshop::bonus_bowser0353 {
struct Frame {
 int pc_mode=-1;unsigned bonus=0,room=177,vi=0;uint64_t epoch=0;
 bool independent_bonus_save=false,canonical=true,custom_active=true,deferred=true;
};
struct Controller {uint32_t actor=0,resource=0,private_data=0,factory=0;uint64_t birth=0;auto operator<=>(const Controller&)const=default;};
// Game-thread only, no files and NO RAM writes. The two guarded native C204
// sites still perform every allocation, RNG draw, parent link and bomb effect.
class Cadence {
public:
 bool configure(const Frame&);
 bool bind(uint8_t*,const Controller&,const Frame&); // after authentic46B1 init
 bool midpoint_due(uint8_t*,const recomp_context*,const Frame&); //8079BDAC before native delay-slot counter store
 bool preserve_midpoint(uint8_t*,const recomp_context*,const Frame&); //8079BDB8 after native counter store, skip ONLY its zero reset
 void forget(uint32_t actor,uint64_t birth);
 void reset();
 bool bound()const{return controller_.has_value();}
private:
 struct Pending{uint32_t stack=0;unsigned vi=0;uint64_t birth=0;};
 Frame configured_{};std::optional<Controller> controller_;std::optional<Pending> pending_;std::optional<unsigned> last_midpoint_;
 bool scope(const Frame&)const;bool live(uint8_t*,const Frame&)const;
};
}
