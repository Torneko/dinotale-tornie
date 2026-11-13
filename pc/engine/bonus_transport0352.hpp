#pragma once
#include "bonus_routes0351.hpp"
#include "bonus_rooms0351.hpp"
#include "bonus_authoring0353.hpp"
#include "recomp.h"
#include <memory>
#include <string>
namespace workshop::bonus_finale_entries0353 {struct EntrySpec;}
namespace workshop::bonus_transport0352 {
inline constexpr uint32_t slab=0x807E0000,slab_end=0x807EF000,slot_bytes=40;
inline constexpr unsigned max_links=(slab_end-slab)/slot_bytes;
struct Environment {
    int pc_mode=-1;unsigned bonus=0,native_course=24;uint64_t seed=0,attempt_epoch=0;
    bool canonical=true,custom_active=true,deferred=true;
    bool prefer_backward=false; // Actual heldL input, used only by a shared native site.
};
struct Link {unsigned from_ordinal=0,to_ordinal=0,source_pot_index=0;};
struct PreparedLink {
    Link ordinal;unsigned source_room=177,destination_room=177,source_course=24,destination_course=24,destination_root=177;
    bonus_rooms0351::Placement source;bonus_rooms0351::Anchor arrival;
    std::array<uint8_t,24> native_payload{};uint32_t record_address=0;
    std::optional<bonus_authoring0353::Site> authored_arrival;bool requires_factory=false;
};
// Supply only a successful native Factory0353 receipt. The adapter additionally
// verifies its own observed constructor birth and original payload in RAM.
struct FactoryWitness {unsigned room=177;uint32_t actor=0,packed=0;bonus_rooms0351::Point point{};uint32_t private_data=0,native_record=0;};
struct Ticket {
    uint64_t attempt_epoch=0,transition_epoch=0;unsigned link=0,from_ordinal=0,to_ordinal=0;
    unsigned source_room=177,destination_room=177,source_course=24,destination_course=24,destination_root=177;
    uint32_t player=0;uint64_t player_birth=0;
};
struct Receipt {Ticket ticket;unsigned health=0,fruits=0,children=0;};
enum class Phase {Disabled,Idle,Departed,Entered,Ready,Arrived,Rejected};
struct View {bool configured=false,proof_only=true;Phase phase=Phase::Disabled;unsigned current_room=177,links=0;uint64_t completed_epoch=0;std::string reason;};
// Single native game thread only. This adapter does not admit/complete a Bonus
// session or make the entire Route playable. Root owns all mode/course fields.
class Transport {
public:
    Transport();~Transport();Transport(const Transport&)=delete;Transport&operator=(const Transport&)=delete;
    // Requires the custom transport to be inactive. Root must reset before
    // handing shared807E RAM to custom content, replacing attempts or ROMs.
    bool configure(std::span<const uint8_t> original,const bonus_routes0351::Route&,std::span<const Link>,const Environment&);
    // All adjacent ordinary links from original/collision-authored sites. This
    // does not certify actor factory, unselected exits, finales or FullRoute.
    bool configure_authored(std::span<const uint8_t> original,const bonus_routes0351::Route&,const Environment&);
    bool configure_authored_finale(std::span<const uint8_t> original,const bonus_routes0351::Route&,const bonus_finale_entries0353::EntrySpec&,const Environment&);
    void reset(uint8_t* rdram);
    bool retry_reset(uint8_t* rdram,const Environment&);
    View view()const;
    std::span<const PreparedLink> links()const;
    unsigned shared_sources()const;
    // Hook80045CAC ENTRY ABI: r4=actor,r5=packed,r6/r7=XYbits, before magic init.
    void created(recomp_context*,unsigned room,const Environment&);
    void destroy(uint8_t*,uint32_t actor,const Environment&);
    bool bind_factory(uint8_t*,const FactoryWitness&,const Environment&);
    void prepare(uint8_t*,uint32_t pot,const Environment&,bool select_direction=true); //805AB914; latch direction only at native1/3
    void departing(uint8_t*,recomp_context*,const Environment&); //8005D320
    std::optional<Ticket> pending_ticket(const Environment&)const;
    void enter_room(unsigned,const Environment&); //after Root set native_course
    void room_ready(uint8_t*,recomp_context*,const Environment&); //8006DD34
    void frame(uint8_t*,recomp_context*,const Environment&); //800675F0
    std::optional<Receipt> take_arrival(const Environment&);
private:
    struct Impl;std::unique_ptr<Impl> p;
};
}
