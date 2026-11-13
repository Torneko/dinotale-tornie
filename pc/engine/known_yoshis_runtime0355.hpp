#pragma once
#include "known_yoshis0355.hpp"
#include <atomic>
namespace workshop::known_yoshis_runtime0355 {
// The canonical White/Black receipt is cached at menu/selection entry, never
// reread from a draw callback or inferred from a Story run's surviving team.
inline std::atomic_uint32_t canonical_bw{0};
inline void canonical(const story_rights0320::View& rights){
 canonical_bw.store(known_yoshis0355::project(rights,{},{},{}).knownMask18&0xC0u,std::memory_order_relaxed);
}
inline known_yoshis0355::Snapshot snapshot(){
 auto state=known_yoshis0355::project({},finds::view(),metal_yoshis0300::view(),elemental_yoshis0355::view());
 state.knownMask18|=canonical_bw.load(std::memory_order_relaxed);
 return state;
}
} // namespace workshop::known_yoshis_runtime0355
