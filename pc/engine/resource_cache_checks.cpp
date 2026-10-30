#include "resource_cache.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
extern "C" void func_800505F0(uint8_t*,recomp_context*);
extern "C" void func_80050848(uint8_t*,recomp_context*);
extern "C" void func_80050894(uint8_t*,recomp_context*);
extern "C" void func_80050634(uint8_t*,recomp_context*);
extern "C" void func_80050674(uint8_t*,recomp_context*);
static unsigned faults;
extern "C" int workshop_resource_slot(uint8_t* rdram,recomp_context* ctx,int match){return workshop::cache::find_slot(rdram,ctx,match!=0);}
extern "C" recomp_func_t* get_function(int32_t address){
    if(uint32_t(address)==0x80050634)return func_80050634;
    if(uint32_t(address)==0x80050674)return func_80050674;
    std::abort();
}
extern "C" void Fault_HungUp(uint8_t*,recomp_context*){faults++;}
extern "C" void func_80065728(uint8_t* rdram,recomp_context* ctx){
    const auto address=uint32_t(ctx->r4)&0x7FFFFFFF;
    if(address+uint32_t(ctx->r5)>0x800000)std::abort();
    std::memset(rdram+address,int(ctx->r6),size_t(ctx->r5));
}
int main(){
    using namespace workshop::cache;
    const auto manager_address=static_cast<gpr>(static_cast<int32_t>(manager));
    const auto slot_address=static_cast<gpr>(static_cast<int32_t>(slots));
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();recomp_context ctx{};
    ctx.r29=gpr(int32_t(0x80020000));
    MEM_W(0,gpr(int32_t(records-4)))=0x12345678;MEM_W(0,gpr(int32_t(slots+capacity*4)))=0x23456789;
    auto guard=[&]{if(uint32_t(MEM_W(0,gpr(int32_t(records-4))))!=0x12345678 || uint32_t(MEM_W(0,gpr(int32_t(slots+capacity*4))))!=0x23456789)std::abort();};
    func_800505F0(rdram,&ctx);
    if(MEM_HU(8,manager_address)!=capacity || uint32_t(MEM_W(16,manager_address))!=slots)return 1;
    auto add=[&](unsigned i){ctx.r4=0x100000+i*0x100;ctx.r5=gpr(int32_t(0x80110000+i*0x100));func_80050848(rdram,&ctx);return uint32_t(ctx.r2);};
    for(unsigned i=0;i<capacity;i++){
        if(add(i)!=0 || faults || uint32_t(MEM_W(i*4,slot_address))!=records+i*12)return 2;
        guard();
    }
    if(MEM_HU(6,manager_address)!=capacity)return 3;
    for(unsigned i: {0u,149u,150u,151u,511u}){
        if(add(i)!=0x80110000+i*0x100)return 4;
        ctx.r4=gpr(int32_t(0x80110000+i*0x100));func_80050894(rdram,&ctx);if(ctx.r2!=0)return 5;
        ctx.r4=gpr(int32_t(0x80110000+i*0x100));func_80050894(rdram,&ctx);if(ctx.r2!=1)return 6;
        if(add(i)!=0)return 7;
    }
    add(capacity);if(faults!=1 || MEM_HU(6,manager_address)!=capacity)return 8;
    guard();func_800505F0(rdram,&ctx);guard();
    if(MEM_HU(6,manager_address)!=0 || add(0)!=0 || faults!=1)return 9;
    printf("Resource cache verified: 512 live entries, reference counts, slots 149/150/151/511, slot reuse, full-cache boundary and reset; adjacent RAM intact.\n");
}
