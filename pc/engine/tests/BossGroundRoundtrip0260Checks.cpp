#include "../boss_ground_roundtrip0260.hpp"
#include <cstdio>
#include <cstdlib>
#include <limits>
using namespace workshop::boss_ground_roundtrip0260;
static unsigned checks=0;static void check(bool b,unsigned line){++checks;if(!b){std::fprintf(stderr,"Check failed at line%u\n",line);std::abort();}}
#define need(b) check((b),__LINE__)
int main(){
 Walk w;Frame f{true,true,true,true,0,100,200,449};uint16_t b=0xffff;float x=1,y=1;
 for(unsigned gate=0;gate<7;++gate){auto bad=f;if(gate==0)bad.enabled=false;if(gate==1)bad.diagnostic=false;if(gate==2)bad.driver=false;if(gate==3)bad.victory=false;if(gate==4)bad.index=1;if(gate==5)bad.x=std::numeric_limits<float>::quiet_NaN();if(gate==6)bad.y=600;need(!w.apply(bad,&b,&x,&y));need(b==0xffff&&x==1&&y==1&&w.stage()==0);}
 need(w.apply(f,&b,&x,&y));need(!b&&!x&&!y);f.vi=130;need(w.apply(f,&b,&x,&y));need(x>0&&!b&&!y);
 f.x=690;f.vi=131;need(w.apply(f,&b,&x,&y));need(x>=.5f&&!b&&!y);
 f.x=700;f.vi=132;need(w.apply(f,&b,&x,&y));f.vi=192;need(w.apply(f,&b,&x,&y));need(w.stage()==1);
 f.vi=193;need(w.apply(f,&b,&x,&y));need(x<0&&!b&&!y);f.x=160;f.vi=400;need(w.apply(f,&b,&x,&y));f.vi=460;need(w.apply(f,&b,&x,&y));need(w.stage()==2&&!w.done());
 f.vi=519;need(w.apply(f,&b,&x,&y));need(!b&&!x&&!y&&!w.done());f.vi=520;need(w.apply(f,&b,&x,&y));need(w.done());b=4;x=y=1;need(!w.apply(f,&b,&x,&y));need(b==4&&x==1&&y==1);
 Walk timeout;f.vi=10;f.x=200;need(timeout.apply(f,&b,&x,&y));f.vi=3611;need(timeout.apply(f,&b,&x,&y));need(timeout.failed()&&!b&&!x&&!y);
 std::printf("%u ground roundtrip controller checks PASS\n",checks);
}
