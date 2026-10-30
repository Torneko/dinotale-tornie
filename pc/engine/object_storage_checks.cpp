#include "recomp.h"
#include <cstdio>
#include <cstring>
#include <vector>
extern "C" {
void func_8005B940(uint8_t*,recomp_context*);
void func_8005B9E8(uint8_t*,recomp_context*);
void func_8005BB28(uint8_t*,recomp_context*);
void func_8005BB78(uint8_t*,recomp_context*);
void func_8005BC68(uint8_t*,recomp_context*);
void func_8005BCB8(uint8_t*,recomp_context*);
void func_8005BDA8(uint8_t*,recomp_context*);
void func_8005BDF8(uint8_t*,recomp_context*);
}
static unsigned freed;
extern "C" void func_80064E84(uint8_t*,recomp_context*){freed++;}
extern "C" void func_800657CC(uint8_t* rdram,recomp_context* ctx){std::memset(rdram+(uint32_t(ctx->r4)&0x7FFFFFFF),int(ctx->r6),size_t(ctx->r5));}
int main(){
    std::vector<uint8_t> memory(0x900000);auto* rdram=memory.data();recomp_context ctx{};ctx.r29=gpr(int32_t(0x80020000));
    constexpr auto table=gpr(int32_t(0x80810000));
    std::memset(rdram+0xDB450,0x5A,0xCC30);
    MEM_W(-4,table)=0x12345678;MEM_W(0x18000,table)=0x23456789;
    func_8005B940(rdram,&ctx);
    recomp_func_t* getters[]={func_8005BB28,func_8005BB78,func_8005BC68,func_8005BCB8,func_8005BDA8,func_8005BDF8};
    for(unsigned row=0;row<4096;row++)for(unsigned field=0;field<6;field++)MEM_W(row*24+field*4,table)=row*6+field+1;
    for(unsigned row=0;row<4096;row++)for(unsigned field=0;field<6;field++){
        ctx.r4=0x4001+row;getters[field](rdram,&ctx);
        if(ctx.r2!=row*6+field+1){fprintf(stderr,"Storage mismatch id=%04X field=%u actual=%u\n",0x4001+row,field,unsigned(ctx.r2));return 1;}
    }
    func_8005B9E8(rdram,&ctx);
    if(freed!=4096*3)return 2;
    for(unsigned offset=0;offset<0x18000;offset+=4)if(MEM_W(offset,table)!=0)return 3;
    for(unsigned offset=0;offset<0xCC30;offset++)if(rdram[0xDB450+offset]!=0x5A)return 4;
    if(uint32_t(MEM_W(-4,table))!=0x12345678 || uint32_t(MEM_W(0x18000,table))!=0x23456789)return 5;
    puts("Object storage verified: 24576 getter cases, 4096 independent ID rows, 12288 cleanup calls; original table and adjacent RAM untouched.");
}
