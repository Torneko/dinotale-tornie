#pragma once
#include <cstdint>
namespace workshop::boss_diagnostic_watch0260 {
// Disposable input-only QA watchdog. It owns no RDRAM, gameplay phase,
// inventory, victory receipt, profile or native dispatcher.
struct Sample {
 bool enabled=false,diagnostic=false,driver=false,full=false,scripted=false,bounded=false,fighting=false;
 bool live_player=false,inventory_valid=false,aiming=false;
 unsigned vi=0,encounter=0,reserves=0;uint32_t player=0;
};
class AmmoWatch {
 bool armed_=false;unsigned since_=0,last_vi_=0,encounter_=0;uint32_t player_=0;
public:
 static constexpr unsigned budget_vi=3600;
 void reset(){armed_=false;since_=last_vi_=encounter_=0;player_=0;}
 bool step(const Sample& in){
  const bool watch=in.enabled&&in.diagnostic&&in.driver&&in.full&&in.scripted&&in.bounded&&in.fighting&&in.live_player&&in.player&&in.inventory_valid&&in.reserves==0&&!in.aiming&&in.encounter<10;
  if(!watch){reset();return false;}
  if(!armed_||player_!=in.player||encounter_!=in.encounter||in.vi<last_vi_){armed_=true;since_=last_vi_=in.vi;player_=in.player;encounter_=in.encounter;return false;}
  last_vi_=in.vi;return in.vi-since_>=budget_vi;
 }
};
// A second QA-only bound keeps a missed-target pilot from using the whole
// twenty-minute deadline without one genuine native monster hit.
struct ProgressSample {Sample gate{};bool monsters=false,initialized=false;unsigned causal_hits=0,defeated_mask=0;};
class MonsterProgressWatch {
 bool armed_=false;unsigned since_=0,last_vi_=0,encounter_=0,hits_=0,mask_=0;uint32_t player_=0;
public:
 static constexpr unsigned budget_vi=18000;
 void reset(){armed_=false;since_=last_vi_=encounter_=hits_=mask_=0;player_=0;}
 bool step(const ProgressSample& in){
  const auto& q=in.gate;
  const bool watch=q.enabled&&q.diagnostic&&q.driver&&q.full&&q.scripted&&q.bounded&&q.fighting&&q.live_player&&q.player&&q.inventory_valid&&q.encounter<10&&in.monsters&&in.initialized;
  if(!watch){reset();return false;}
  if(!armed_||player_!=q.player||encounter_!=q.encounter||q.vi<last_vi_||hits_!=in.causal_hits||mask_!=in.defeated_mask){armed_=true;since_=last_vi_=q.vi;player_=q.player;encounter_=q.encounter;hits_=in.causal_hits;mask_=in.defeated_mask;return false;}
  last_vi_=q.vi;return q.vi-since_>=budget_vi;
 }
};
}
