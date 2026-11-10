#pragma once
#include <array>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
namespace workshop::story_hybrid0320 {
inline constexpr unsigned mode=5,all_worlds=63,all_hybrids=63,all_metals=3;
inline constexpr const char* filename="pc-story-hybrid.json";
using Order=std::array<unsigned,6>;
struct View {
 unsigned worlds=0,count=0,eligible=0,acquired=0,metal_owned=0,metal_alive=0;
 Order order{};
 bool initialized=false,present=false,active=false,readable=true,saved=true;
 std::string reason;
};
// Pure journal service: no native RAM, EEPROM, menus, collection, wallet or
// canonical ownership services are called. acquired is an observed projection,
// never a grant of personal ownership. Caller proves genuine successful worlds.
class Journal final {
 struct Impl;std::unique_ptr<Impl> impl_;
public:
 // Factory injection is for isolated deterministic tests. Production shuffles
 // all six colors once, persists the order, and never redraws on reload/reset.
 explicit Journal(std::function<Order()> permutation_factory={});~Journal();
 Journal(const Journal&)=delete;Journal&operator=(const Journal&)=delete;
 void initialize(const std::filesystem::path& path=filename);
 View view()const;
 // This is the ONLY operation that may create a missing journal, only for5.
 bool prepare(unsigned selected_mode,unsigned acquired_hybrids=0,unsigned owned_metals=0);
 bool successful_world(unsigned page,bool trusted_receipt);
 // Cross-mode observation may update an existing journal, never create one.
 bool note_acquired(unsigned hybrid_mask);
 bool save_lives(unsigned two_bits);
 bool new_run(unsigned owned_metals=0);
 bool reset_run(unsigned owned_metals=0){return new_run(owned_metals);}
 void deactivate();
};
void initialize(const std::filesystem::path& path=filename);
View view();
bool prepare(unsigned selected_mode,unsigned acquired_hybrids=0,unsigned owned_metals=0);
bool successful_world(unsigned page,bool trusted_receipt);
bool note_acquired(unsigned hybrid_mask);
bool save_lives(unsigned two_bits);
bool new_run(unsigned owned_metals=0);
bool reset_run(unsigned owned_metals=0);
void deactivate();
} // namespace workshop::story_hybrid0320
