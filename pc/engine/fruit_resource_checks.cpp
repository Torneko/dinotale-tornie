#include "fruit_resources.hpp"
#include <vector>
#include <bit>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" void func_80045CAC(uint8_t*,recomp_context*);
extern "C" void func_80045CAC_before(uint8_t*,recomp_context*);
extern "C" void func_80045CAC_resource_only(uint8_t*,recomp_context*);
static constexpr uint16_t ids[]={0x4030,0x4031,0x4032,0x4033,0x4034,0x4035,
                                0x4F00,0x4F01,0x4F02,0x4F03,0x4F04,0x4F05};
static constexpr uint32_t actorAddress=0x80010000,drawingAddress=0x80020000,stackAddress=0x80700000;
static unsigned loads=0;
static void require(bool ok,const char* message){if(!ok){fprintf(stderr,"%s\n",message);std::exit(1);}}
static uint32_t resource(uint16_t id){
    for(unsigned i=0;i<std::size(ids);i++)if(ids[i]==id)return 0x80100000+i*0x100;
    std::abort();
}
extern "C" void workshop_mix_parent_created(uint32_t){}
extern "C" void workshop_prepare_fruit_resource(uint8_t* rdram,recomp_context* ctx){
    workshop::fruit_resources::prepare(rdram,uint32_t(ctx->r4),uint16_t(uint32_t(ctx->r5)>>16));
}
extern "C" void workshop_finish_fruit_resource(uint8_t* rdram,recomp_context* ctx){
    workshop::fruit_resources::finish(rdram,uint32_t(MEM_W(0x28,ctx->r29)));
}
extern "C" void func_80051194(uint8_t*,recomp_context* ctx){loads++;ctx->r2=gpr(int32_t(resource(uint16_t(ctx->r4))));}
extern "C" void Fault_HungUp(uint8_t*,recomp_context*){require(false,"Native initializer asserted");}
extern "C" void func_8004921C(uint8_t*,recomp_context* ctx){ctx->r2=0;}
extern "C" void func_80047488(uint8_t*,recomp_context*){}
extern "C" void func_80047E70(uint8_t*,recomp_context*){}

int main(){
    std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
    const auto actor=gpr(int32_t(actorAddress));
    const auto drawing=gpr(int32_t(drawingAddress));
    MEM_W(0,gpr(int32_t(0x800F843C)))=0x80030000;
    for(unsigned offset:{0x3Cu,0x40u,0x44u})MEM_W(offset,gpr(int32_t(0x80030000)))=0x3F800000;
    for(auto id:ids){
        const auto r=gpr(int32_t(resource(id)));
        MEM_B(0,r)=0;MEM_H(0x3C,r)=id;MEM_W(0x44,r)=0x805741B8;
        for(unsigned offset:{8u,12u,16u,20u,24u,28u})MEM_W(offset,r)=0x3F800000;
        MEM_W(0x84,r)=uint32_t(r)+0xA0;MEM_W(0x88,r)=uint32_t(r)+0xC0;
    }
    auto setup=[&](uint16_t old){
        const auto r=gpr(int32_t(resource(old)));
        std::memset(rdram+(drawingAddress-0x80000000),0,0x44);
        MEM_W(0x168,actor)=uint32_t(r);MEM_W(0x16C,actor)=drawingAddress;
        MEM_W(8,drawing)=MEM_W(0x88,r);MEM_W(0x30,drawing)=MEM_W(0x84,r);MEM_W(0x40,drawing)=uint32_t(r)+0x74;
        MEM_W(0x2C,drawing)=44;
    };
    auto initialize=[&](uint16_t requested,unsigned version){
        recomp_context ctx{};ctx.r29=gpr(int32_t(stackAddress));ctx.r31=gpr(int32_t(0x80001234));
        ctx.r4=actor;ctx.r5=gpr(int32_t(uint32_t(requested)<<16|0x1800));
        ctx.r6=std::bit_cast<uint32_t>(136.f);ctx.r7=std::bit_cast<uint32_t>(456.f);
        MEM_W(0x10,ctx.r29)=std::bit_cast<uint32_t>(0.f);
        (version==2?func_80045CAC:version==1?func_80045CAC_resource_only:func_80045CAC_before)(rdram,&ctx);
        require(uint32_t(ctx.r2)==1,"Native initialization failed");
        require(uint32_t(ctx.r29)==stackAddress && uint32_t(ctx.r31)==0x80001234,"Initializer ABI changed");
        require(MEM_HU(12,actor)==requested,"Food identity changed during rebinding");
        require(uint32_t(MEM_W(0x20,actor))==0x805741B8,"Native update callback changed");
    };
    unsigned baseline=0;
    for(auto old:{uint16_t(0x4034),uint16_t(0x4F00),uint16_t(0x4F04)}){
        const uint16_t target=old==0x4F04?0x4032:0x4F04;
        setup(old);const auto beforeLoads=loads;
        initialize(target,0);
        require(uint32_t(MEM_W(0x168,actor))==resource(old) && loads==beforeLoads,"Pre-fix control did not reproduce stale atlas");
        baseline++;
    }
    setup(0x4034);initialize(0x4F05,1);
    require(uint32_t(MEM_W(0x168,actor))==resource(0x4F05),"Resource-only control did not bind kiwi");
    const auto kiwiResource=gpr(int32_t(resource(0x4F05)));
    require(uint32_t(MEM_W(8,drawing))!=uint32_t(MEM_W(0x88,kiwiResource)),"Resource-only control did not reproduce stale rendered palette");
    unsigned transitions=0;
    for(auto old:ids)for(auto target:ids){
        setup(old);const auto beforeLoads=loads;
        initialize(target,2);
        const bool switched=old!=target && (workshop::fruit_resources::hybrid(old)||workshop::fruit_resources::hybrid(target));
        require(uint32_t(MEM_W(0x168,actor))==resource(switched?target:old),"Pooled fruit retained the wrong atlas");
        require(loads==beforeLoads+unsigned(switched),"Unnecessary or missing resource lookup");
        const auto bound=gpr(int32_t(resource(switched?target:old)));
        require(uint32_t(MEM_W(8,drawing))==uint32_t(MEM_W(0x88,bound)),"Rendered palette retained the previous fruit colors");
        require(uint32_t(MEM_W(0x30,drawing))==uint32_t(MEM_W(0x84,bound)),"Rendered texture descriptor retained the previous fruit");
        require(uint32_t(MEM_W(0x40,drawing))==uint32_t(bound)+0x74 && MEM_W(0x2C,drawing)==44,"Render geometry or slot changed incorrectly");
        transitions++;
    }
    for(auto target:ids){
        MEM_W(0x168,actor)=0;MEM_W(0x16C,actor)=0;const auto beforeLoads=loads;initialize(target,2);
        require(uint32_t(MEM_W(0x168,actor))==resource(target) && loads==beforeLoads+1,"Fresh fruit loading regressed");
    }
    MEM_W(0x168,actor)=resource(0x4F04);
    MEM_W(0x16C,actor)=drawingAddress;
    for(auto other:{0x4001,0x4013,0x402C,0x41D3}){
        require(!workshop::fruit_resources::prepare(rdram,actorAddress,uint16_t(other)),"Non-fruit resource was cleared");
        require(uint32_t(MEM_W(0x168,actor))==resource(0x4F04),"Non-fruit actor changed");
        MEM_H(12,actor)=other;
        require(!workshop::fruit_resources::finish(rdram,actorAddress),"Non-fruit render state was changed");
    }
    MEM_H(12,actor)=0x4F04;MEM_B(0x164,actor)=0;
    MEM_W(0x38,drawing)=0x80300000;
    require(!workshop::fruit_resources::finish(rdram,actorAddress),"Owned texture buffers must keep their native lifecycle");
    printf("Fruit resources verified: %u pre-fix atlas mismatches, stale rendered palette reproduced in resource-only control, %u native reuse transitions with rendered palette/descriptor/geometry, 12 fresh loads and non-fruit preservation.\n",baseline,transitions);
}
