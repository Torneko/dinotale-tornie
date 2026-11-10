#include "../boss_diagnostic_watch0260.hpp"
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
namespace q=workshop::boss_diagnostic_watch0260;
static unsigned checks=0;
static void need(bool value,const char* why){++checks;if(!value){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
static q::Sample valid(unsigned vi=0){q::Sample s;s.enabled=s.diagnostic=s.driver=s.full=s.scripted=s.bounded=s.fighting=s.live_player=s.inventory_valid=true;s.vi=vi;s.player=0x800CA6B0;s.encounter=2;return s;}
int main(){
 for(unsigned start:{0u,17u,72000u}){q::AmmoWatch w;auto s=valid(start);need(!w.step(s),"new empty ring arms without ending QA");for(unsigned dt=0;dt<3600;++dt){s.vi=start+dt;need(!w.step(s),"no timeout before3600 actual VI");need(!w.step(s),"repeated same-VI observation never advances clock");}s.vi=start+3600;need(w.step(s),"bounded QA expires at3600 empty VI");need(w.step(s),"deadline is stable across same-VI calls");}
 for(unsigned fault=0;fault<13;++fault){q::AmmoWatch w;auto s=valid(100);need(!w.step(s),"pre-arm reset scenario");s.vi=3700;if(fault==0)s.enabled=false;if(fault==1)s.diagnostic=false;if(fault==2)s.driver=false;if(fault==3)s.full=false;if(fault==4)s.scripted=false;if(fault==5)s.bounded=false;if(fault==6)s.fighting=false;if(fault==7)s.live_player=false;if(fault==8)s.player=0;if(fault==9)s.inventory_valid=false;if(fault==10)s.reserves=1;if(fault==11)s.aiming=true;if(fault==12)s.encounter=10;need(!w.step(s),"disabled/production/other phase/refill/real4005/invalid evidence cannot stop QA");s=valid(3701);need(!w.step(s),"re-entry starts a fresh empty window");s.vi=7300;need(!w.step(s),"old deadline is discarded");s.vi=7301;need(w.step(s),"fresh complete empty window expires");}
 for(unsigned reset=0;reset<3;++reset){q::AmmoWatch w;auto s=valid(4000);need(!w.step(s),"arm provenance scenario");s.vi=7600;if(reset==0)s.player+=0x280;if(reset==1)s.encounter++;if(reset==2)s.vi=100;need(!w.step(s),"player/encounter/rewound clock changes reset provenance");const unsigned start=s.vi;s.vi=start+3599;need(!w.step(s),"new provenance gets whole budget");s.vi=start+3600;need(w.step(s),"new provenance times out only at its deadline");w.reset();s.vi++;need(!w.step(s),"explicit reset discards previous deadline");}
 {q::AmmoWatch w;auto s=valid(0);need(!w.step(s),"initial zero VI is valid");s.vi=4000;s.reserves=6;need(!w.step(s),"normal refill resets watchdog");s.vi=4001;s.reserves=0;need(!w.step(s),"later exhaustion starts a new watch");s.vi=7600;need(!w.step(s),"refill prevents old timeout");s.vi=7601;need(w.step(s),"later exhaustion remains bounded");}
 {q::MonsterProgressWatch w;q::ProgressSample p;p.gate=valid();p.monsters=p.initialized=true;need(!w.step(p),"native monster progress arms without gameplay writes");
  for(unsigned vi=1;vi<18000;++vi){p.gate.vi=vi;p.gate.reserves=vi%7;p.gate.aiming=vi%2;need(!w.step(p),"aim/refill alone does not fabricate a native hit or expire early");}
  p.gate.vi=18000;need(w.step(p),"five minutes without genuine hit ends only isolated QA");p.causal_hits=1;need(!w.step(p),"actual native hit rearms progress budget");p.gate.vi=35999;need(!w.step(p),"native hit receives full new budget");p.gate.vi=36000;need(w.step(p),"new hit deadline remains bounded");p.defeated_mask=1;need(!w.step(p),"native member defeat also rearms");p.gate.vi=53999;need(!w.step(p),"member transition receives full budget");p.gate.vi=54000;need(w.step(p),"member transition deadline exact");w.reset();need(!w.step(p),"explicit monster watchdog reset");
 }
 for(unsigned fault=0;fault<12;++fault){q::MonsterProgressWatch w;q::ProgressSample p;p.gate=valid(10);p.monsters=p.initialized=true;need(!w.step(p),"arm monster eligibility scenario");p.gate.vi=18010;
  if(fault==0)p.gate.enabled=false;if(fault==1)p.gate.diagnostic=false;if(fault==2)p.gate.driver=false;if(fault==3)p.gate.full=false;if(fault==4)p.gate.scripted=false;if(fault==5)p.gate.bounded=false;if(fault==6)p.gate.fighting=false;if(fault==7)p.gate.live_player=false;if(fault==8)p.gate.inventory_valid=false;if(fault==9)p.monsters=false;if(fault==10)p.initialized=false;if(fault==11)p.gate.player=0;
  need(!w.step(p),"production/disabled/nonmonster/unbound state cannot time out");p.gate=valid(18011);p.monsters=p.initialized=true;need(!w.step(p),"re-entering eligible state starts fresh");p.gate.vi=36010;need(!w.step(p),"no stale monster timeout");p.gate.vi=36011;need(w.step(p),"fresh monster deadline exact");
 }
 std::printf("BossDiagnosticWatch0260Checks: %u PASS (pure QA timeout provenance; no game/RAM/native victory)\n",checks);
}
