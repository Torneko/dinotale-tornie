#pragma once
#include <cstdint>
#include <filesystem>
#include <string>

// This journal records only Trial life availability. It never changes ownership,
// EEPROM, currencies, native RAM or mode selection; trusted callers supply scope
// and successful-level evidence for recover().
namespace workshop::trial_special_lives0350 {
inline constexpr unsigned special_mask=0xC0C0u;
inline constexpr const char* filename="pc-trial-special-lives0350.json";
constexpr bool special(unsigned color){return color==6||color==7||color==14||color==15;}
struct Snapshot {
    unsigned lost_mask=0;
    bool readable=false,last_write_succeeded=false;
    std::string reason;
    // An unavailable journal blocks these four colors, never ordinary colors
    // or unrelated native flags. No helper grants an ownership bit.
    unsigned blocked_mask()const{return readable&&last_write_succeeded?lost_mask:special_mask;}
    unsigned filter(unsigned owned_mask)const{return owned_mask&~blocked_mask();}
    bool selectable(unsigned color)const{return color<16&&!(blocked_mask()&(1u<<color));}
};
enum class Status {Committed,Unchanged,InvalidColor,Unavailable,PersistenceFailed};
struct Reply {
    Status status=Status::Unavailable;
    unsigned lost_mask=0;
    bool committed()const{return status==Status::Committed;}
    bool accepted()const{return committed()||status==Status::Unchanged;}
};
class Store {
public:
    // Missing journal means no losses, without creating a file or lock.
    bool load(const std::filesystem::path& profile);
    Snapshot snapshot()const;
    // Read-only refresh. A failed write remains fail-closed until a successful
    // mutation retry, rather than being silently cleared by a picker read.
    bool refresh();
    Reply lose(unsigned pc_color);
    Reply recover(unsigned pc_color);
private:
    Reply update(unsigned pc_color,bool lost);
    std::filesystem::path profile_,file_;
    Snapshot view_{};
    bool initialized_=false,present_=false;
    unsigned failed_mask_=0;
};
} // namespace workshop::trial_special_lives0350
