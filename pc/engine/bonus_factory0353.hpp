#pragma once
#include "bonus_authoring0353.hpp"
#include "bonus_transport0352.hpp"
#include <memory>
namespace workshop::bonus_factory0353 {
using Environment=bonus_transport0352::Environment;
using Direction=bonus_authoring0353::Direction;
enum class Status {Denied,Unavailable,Scheduled,Cached,ExistingNativeSource,Created,Failed};
struct Receipt {
    unsigned ordinal=0,room=177;Direction direction=Direction::Forward;
    uint64_t attempt_epoch=0,room_epoch=0,birth=0;
    uint32_t actor=0,packed=0;bonus_authoring0353::Point point{};
    uint32_t private_data=0,native_record=0,allocator_list=0,allocator_table=0;unsigned allocator_capacity=0;
};
struct Reply {Status status=Status::Unavailable;std::optional<Receipt> receipt;std::string reason;};
class Factory {
public:
    Factory();~Factory();Factory(const Factory&)=delete;Factory&operator=(const Factory&)=delete;
    bool configure(std::span<const uint8_t> original,const bonus_routes0351::Route&,const Environment&);
    void reset();
    bool enter_room(unsigned ordinal,uint64_t room_epoch,const Environment&);
    bool room_ready(uint8_t*,uint32_t native_level_info,const Environment&);
    Reply preload(uint8_t*,recomp_context*,const Environment&);
    Reply spawn(Direction,uint8_t*,recomp_context*,const Environment&);
    // Feed every real80045CAC ENTRY, before magic init, including unrelated
    // constructors so address reuse invalidates prior fresh receipts.
    void created(recomp_context*,unsigned room,const Environment&);
    void destroyed(uint32_t actor,const Environment&);
    std::optional<Receipt> receipt(Direction,const Environment&)const;
    const bonus_authoring0353::Site* site(Direction)const;
private:
    struct Impl;std::unique_ptr<Impl> p;
};
}
