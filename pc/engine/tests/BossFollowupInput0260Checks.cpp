#include "../boss_followup_input0260.hpp"
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
namespace q=workshop::boss_followup_input0260;
static unsigned checks=0;static void need(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
int main(){try{
 const std::array<q::Event,4> events{{{0,2,0x2002,.5f,.25f},{3,4,0x8000,0,0},{4,7,0,-1,1},{420,429,0x2000,.75f,-.5f}}};
 need(q::valid_events(events),"finite ordered ordinary controller vector valid");
 q::Anchor a;uint16_t b=0xFFFF;float x=.875f,y=-.875f;q::Frame f{true,true,true,0,true,false,3000};
 auto untouched=[&](const q::Frame& input){b=0xFFFF;x=.875f;y=-.875f;const bool applied=a.apply(input,events,&b,&x,&y);need(!applied&&b==0xFFFF&&x==.875f&&y==-.875f,"foreign/prelude/unauthorized input cannot override existing driver");};
 untouched(f);f.fighting=false;untouched(f);f.index=1;untouched(f);f.victory=true;untouched(f);need(!a.anchored(),"victory without native second-fight entry never establishes anchor");
 f={true,true,true,1,true,false,5126};
 for(unsigned guard=0;guard<4;++guard){auto bad=f;switch(guard){case 0:bad.enabled=false;break;case 1:bad.diagnostic=false;break;case 2:bad.driver=false;break;case 3:bad.index=2;break;}untouched(bad);}
 auto both=f;both.victory=true;untouched(both);need(!a.anchored(),"invalid authorization/ambiguous phase cannot anchor");
 need(!a.apply(f,events,nullptr,&x,&y)&&!a.apply(f,events,&b,nullptr,&y)&&!a.apply(f,events,&b,&x,nullptr)&&!a.anchored(),"null output refusals are mutation-free");
 need(a.apply(f,events,&b,&x,&y)&&a.anchor_vi()==5126&&b==0x2002&&x==.5f&&y==.25f,"first actual Fighting/index1 anchors and overrides after prior driver output");
 need(a.apply(f,events,&b,&x,&y)&&a.anchor_vi()==5126&&b==0x2002,"duplicate same-VI input read is deterministic and never restarts anchor");
 f.vi=5127;need(a.apply(f,events,&b,&x,&y)&&b==0x2002,"event remains active through last-1");
 f.vi=5128;need(a.apply(f,events,&b,&x,&y)&&!b&&x==0&&y==0,"half-open event end and gaps emit neutral, no fallback auto input");
 f.vi=5129;need(a.apply(f,events,&b,&x,&y)&&b==0x8000,"one-frame ordinary button edge preserved");
 f.vi=5130;need(a.apply(f,events,&b,&x,&y)&&b==0&&x==-1&&y==1,"adjacent event and signed native axis range preserved");
 auto old=f;old.vi=5129;untouched(old);need(a.anchor_vi()==5126,"out-of-order sample cannot reanchor");
 f.vi=5554;need(a.apply(f,events,&b,&x,&y)&&b==0x2000&&x==.75f,"last included original relative428 action retained");
 f.vi=5555;b=0x1234;x=.625f;y=-.375f;need(!a.apply(f,events,&b,&x,&y)&&b==0x1234&&x==.625f&&y==-.375f,"after final event Fighting returns control to existing driver without altering its inputs");
 f.vi=5556;need(!a.apply(f,events,&b,&x,&y)&&b==0x1234&&x==.625f&&y==-.375f,"continued Fighting never steals control back after script expiration");f.fighting=false;f.victory=true;need(a.apply(f,events,&b,&x,&y)&&b==0&&x==0&&y==0,"second victory still holds neutral after returning combat control");
 f.vi=5557;f.fighting=true;f.victory=false;need(a.apply(f,events,&b,&x,&y)&&!b&&x==0&&y==0,"victory latch cannot resume script on a later fighting sample");
 f.index=2;untouched(f);a.reset();need(!a.anchored()&&a.anchor_vi()==0,"explicit new-campaign reset removes only host anchor state");
 for(unsigned fault=0;fault<10;++fault){std::vector<q::Event> bad(events.begin(),events.end());switch(fault){case 0:bad.clear();break;case 1:bad[0].last=0;break;case 2:bad[0].first=3;break;case 3:bad[1].first=1;break;case 4:bad[3].last=430;break;case 5:bad[0].x=std::numeric_limits<float>::quiet_NaN();break;case 6:bad[0].y=std::numeric_limits<float>::infinity();break;case 7:bad[0].x=-1.01f;break;case 8:bad[0].y=1.01f;break;case 9:bad.resize(513);break;}need(!q::valid_events(bad),"malformed/out-of-order/overlapping/unbounded/nonfinite vector refused");b=0xFFFF;x=.875f;y=-.875f;auto test=q::Frame{true,true,true,1,true,false,1};need(!a.apply(test,bad,&b,&x,&y)&&!a.anchored()&&b==0xFFFF&&x==.875f&&y==-.875f,"invalid vector never anchors or changes existing driver outputs");}
 f={true,true,true,1,true,false,9000};need(a.apply(f,events,&b,&x,&y)&&a.anchor_vi()==9000&&b==0x2002,"reset permits a later independent second encounter to establish its own origin");
 const std::array<q::Event,1> short_events{{{0,2,0x8000,0,0}}};q::Anchor short_anchor;q::Frame short_frame{true,true,true,1,true,false,10};need(short_anchor.apply(short_frame,short_events,&b,&x,&y)&&b==0x8000,"short followup anchors its ordinary action");short_frame.vi=12;b=0x4321;x=-.25f;y=.125f;need(!short_anchor.apply(short_frame,short_events,&b,&x,&y)&&b==0x4321&&x==-.25f&&y==.125f,"handoff uses actual last event end rather than fixed maximum429");
 std::cout<<"BossFollowupInput0260Checks: "<<checks<<" PASS; host-only input/phase handoff, no RAM/native/GPU actions.\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
