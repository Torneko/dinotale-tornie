#include "rt64_gbi_s2dex_slice.h"
#include <cstdio>
#include <cstdlib>
using RT64::GBI_S2DEX::backgroundSlice;
static void require(bool b,const char* text){if(!b){fprintf(stderr,"%s\n",text);std::exit(1);}}
int main(){
    unsigned cases=0;
    for(int capacity:{2,3,4,7,15})for(int scale=1025;scale<=(capacity-1)*1024;scale+=37)
    for(int origin:{-1567,0,31,511,1023,383*1024+1000}){
        const int height=384,frame=240;int screenY=0;
        while(screenY<frame){
            const auto global=int64_t(origin)+int64_t(screenY)*scale;
            const auto s=backgroundSlice(global,height,frame-screenY,capacity,scale);
            require(s.screenRows>=1 && s.sourceRows+1<=capacity+1,"Slice overruns TMEM or fails to progress");
            for(int y=0;y<s.screenRows;y++){
                const auto reference=((global+int64_t(y)*scale)%(height*1024)+height*1024)%(height*1024);
                const auto rendered=(int64_t(s.sourceRow)*1024+(s.phase>>5)*32+int64_t(y)*scale)%(height*1024);
                const auto error=(reference-rendered+height*1024)%(height*1024);
                require(error<32,"Source phase jumps at a slice boundary");
                require(s.phase+y*scale<(s.sourceRows<<10),"Sample is outside loaded source rows");
            }
            screenY+=s.screenRows;cases++;
        }
        require(screenY==frame,"Screen rows are missing or drawn twice");
    }
    // The exact 496x384 room starts with scale 1430. The previous fixed
    // three-source-row partition loses over one texel of phase here.
    int legacyRows=0,fixed=0;bool legacyJump=false;
    const int span=(3<<20)/1430;
    for(int i=0;i<40;i++){
        fixed+=span;const int drawn=fixed>>10;fixed&=1023;
        const int error=i*3*1024-legacyRows*1430;
        legacyJump|=error>1024;legacyRows+=drawn;
    }
    require(legacyJump,"The reported downscale no longer exercises the legacy defect");
    printf("Background slicing: %u strips preserve continuous sampling, screen coverage, wrap and bilinear load limits; legacy room reproduces a texel jump.\n",cases);
}
