#include "coop_native_factory.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace workshop::coop_native;
static constexpr uint32_t p0=0x800C0000,p1=0x800C2000;
static unsigned constructors=0,initializers=0,activations=0,disposals=0,cleanup_requests=0;
static bool fail_constructor=false,fail_initializer=false;
static void need(bool ok,const char* error){if(!ok)throw std::runtime_error(error);}
static void live(uint8_t* rdram,uint32_t a,uint16_t type,uint32_t data){
    word(rdram,a,0x19971110);byte(rdram,a+6,0);const gpr b=base(a);MEM_H(12,b)=type;
    word(rdram,a+0x1F4,data);word(rdram,a+0x16C,data+0x2000);word(rdram,a+0x174,data+0x2100);
    MEM_H(0x184,b)=0;byte(rdram,a+0x165,0);byte(rdram,a+0x17D,0);word(rdram,a+0x20,0x80043450);
    word(rdram,a+0x130,0x800233B4);if(type==0x4001)byte(rdram,a+0x164,3);
}
static void group(uint8_t* rdram,uint32_t a,uint32_t data){
    live(rdram,a,0x4001,data);live(rdram,a+0x200,0x400B,data+0x4000);
    live(rdram,a+0x400,0x400C,data+0x8000);live(rdram,a+0x600,0x4007,data+0xC000);
    word(rdram,data+0x160,a+0x200);word(rdram,data+0x15C,a+0x400);
    const gpr b=base(a),child=base(a+0x200);
    MEM_H(0x184,b)=2;word(rdram,a+0x188,a+0x200);word(rdram,a+0x18C,a+0x600);
    MEM_H(0x184,child)=1;word(rdram,a+0x200+0x188,a+0x400);
    word(rdram,a+0x200+0x180,a);word(rdram,a+0x400+0x180,a+0x200);word(rdram,a+0x600+0x180,a);
}
extern "C" void func_8004613C(uint8_t* rdram,recomp_context* c){
    constructors++;need(uint32_t(c->r4)==word(rdram,p0+12),"Factory changed packed source4001 parameters");
    need(word(rdram,color_global)==5 && word(rdram,custom_color_global)==5,"Constructor was not in secondary colour context");
    need(std::bit_cast<float>(uint32_t(c->r5))==240.0f && std::bit_cast<float>(uint32_t(c->r6))==440.0f,"Native constructor argument mapping is wrong");
    if(fail_constructor){word(rdram,player_global,0);c->r2=0;return;}
    live(rdram,p1,0x4001,0);word(rdram,p1+0x16C,0x80202000);word(rdram,p1+0x174,0x80202100);
    word(rdram,p1+0x20,0x800436E0);word(rdram,p1+0x130,0);
    c->r4=base(p1);created(c);
    word(rdram,p1+0x84,uint32_t(c->r5));word(rdram,p1+0x88,uint32_t(c->r6));word(rdram,p1+0x8C,uint32_t(c->r7));
    word(rdram,player_global,p1);c->r2=base(p1);
}
extern "C" void func_80048A30(uint8_t* rdram,recomp_context* c){
    initializers++;need(constructing && uint32_t(c->r4)==p1 && byte(rdram,p1+0x17D)==2,"Genuine init dispatcher ran before registration or outside construction scope");
    need(activations==1 && word(rdram,p1+0x20)==0x800436E0 && word(rdram,p1+0x1F4)==0,"Basic allocator incorrectly preinitialized the player");
    if(fail_initializer)return;
    const auto phase=byte(rdram,p1+0x17D);group(rdram,p1,0x80200000);byte(rdram,p1+0x17D,phase);
    word(rdram,p1+0x600+0x1F4,0); //Shadow4007 keeps its own deferred ctor.
    word(rdram,health_global,1);word(rdram,health_global+5*4,1);
    word(rdram,0x800F897C,1);byte(rdram,0x800F89B4,0);word(rdram,0x800F89BC,0);word(rdram,0x800992D0,0);
    word(rdram,0x801700F4,0x8006920C);byte(rdram,0x80101404,0);c->r2=base(p1);
}
extern "C" void func_8005FDC4(uint8_t*,recomp_context*){}
extern "C" void func_80043A88(uint8_t* rdram,recomp_context* c){
    need(uint32_t(c->r4)==0x801A0000 && uint32_t(c->r5)==0,"Secondary registration did not query a generic free slot");
    c->r2=free_main_slot(rdram,p0);
}
extern "C" void func_80043C0C(uint8_t* rdram,recomp_context* c){
    activations++;need(uint32_t(c->r4)==0x801A0000 && uint32_t(c->r5)==6 && uint32_t(c->r6)==p1,"Wrong secondary registration arguments");
    need(word(rdram,0x801B0014)==p0 && word(rdram,0x801B0018)==0,"Secondary registration would overwrite the primary slot5");
    word(rdram,0x801B0018,p1);const gpr a=base(p1);MEM_H(0x17E,a)=6;MEM_B(0x17C,a)=1;c->r2=1;
}
extern "C" void func_800445A4(uint8_t* rdram,recomp_context* c){
    cleanup_requests++;need(uint32_t(c->r4)==p1,"Cleanup tried to destroy the primary");word(rdram,0x801B0018,1);const gpr a=base(p1);MEM_H(0x17E,a)=0x96;byte(rdram,p1+0x17D,3);
}
extern "C" void func_80045BD8(uint8_t* rdram,recomp_context* c){
    const uint32_t a=uint32_t(c->r4);disposals++;
    if(kind(rdram,a)==0x4001)word(rdram,player_global,0);
    word(rdram,a+0x1F4,0);word(rdram,a+0x16C,0);word(rdram,a+0x174,0);byte(rdram,a+6,3);forget(a);
}
static void seed(uint8_t* rdram){
    configure(false);constructors=initializers=activations=disposals=cleanup_requests=0;fail_constructor=fail_initializer=false;
    group(rdram,p0,0x80100000);byte(rdram,p0+0x17D,2);
    word(rdram,p0+0x84,std::bit_cast<uint32_t>(192.0f));word(rdram,p0+0x88,std::bit_cast<uint32_t>(440.0f));word(rdram,p0+0x8C,std::bit_cast<uint32_t>(500.34f));
    word(rdram,player_global,p0);word(rdram,color_global,0);word(rdram,custom_color_global,8);
    for(unsigned i=0;i<8;i++)word(rdram,health_global+i*4,i+1);
    word(rdram,0x800F897C,0x0A0B0C0D);byte(rdram,0x800F89B4,2);word(rdram,0x800F89BC,0x11223344);word(rdram,0x800992D0,3);
    word(rdram,0x800F8318,0x80170000);word(rdram,0x801700F4,0x80060000);byte(rdram,0x80101404,0xAB);
    const gpr pool=base(pool_header),active=base(0x800F8840);
    MEM_H(8,pool)=8;word(rdram,pool_header+0x10,0x80180000);
    for(unsigned n=0;n<8;n++){const uint32_t a=0x800D0000+n*0x200;word(rdram,0x80180000+n*4,a);byte(rdram,a+6,3);}
    MEM_H(8,active)=150;MEM_H(6,active)=10;word(rdram,0x800F8850,0x80190000);
    word(rdram,0x800F8810,0x801A0000);const gpr manager=base(0x801A0000);MEM_H(8,manager)=150;MEM_H(6,manager)=1;word(rdram,0x801A0010,0x801B0000);
    for(unsigned n=0;n<150;n++)word(rdram,0x801B0000+n*4,0);
    word(rdram,0x801B0014,p0);MEM_H(0x17E,base(p0))=5;
    word(rdram,0x800A6380,2);word(rdram,0x800A6384,0x801C0000);byte(rdram,0x801C0000,1);byte(rdram,0x801C0001,0);
}
int main(){
    try{
        std::vector<uint8_t> memory(8*1024*1024);auto* rdram=memory.data();seed(rdram);
        recomp_context c{};c.r29=base(0x800F7000);c.r4=0x123;c.r5=0x456;auto original_context=c;
        const auto baseline=memory;
        frame(rdram,&c,2500,43);need(memory==baseline && constructors==0,"Disabled prototype wrote memory");
        configure(true);frame(rdram,&c,2399,43);frame(rdram,&c,2500,81);
        need(memory==baseline && constructors==0,"Prototype spawned before ready frame or in another room");
        const auto globals=Globals::capture(rdram);
        frame(rdram,&c,2500,43);
        need(constructors==1 && initializers==1 && activations==1 && primary_actor()==p0 && owned_actor()==p1,"Real factory/activation sequence incomplete");
        need(byte(rdram,p1+0x17D)==2 && word(rdram,0x801B0014)==p0 && word(rdram,0x801B0018)==p1,"Two4001 actors were not registered into distinct manager slots");
        need(std::memcmp(&c,&original_context,sizeof(c))==0,"Factory leaked native registers or stack pointer");
        need(word(rdram,player_global)==p0 && word(rdram,color_global)==0 && word(rdram,custom_color_global)==8,"Factory replaced the primary owner globals");
        for(unsigned i=0;i<8;i++)need(word(rdram,health_global+i*4)==globals.health[i],"Constructor changed primary or another colour health");
        need(word(rdram,0x800F897C)==globals.button_word && byte(rdram,0x800F89B4)==globals.scale_byte && word(rdram,0x800F89BC)==globals.mode_word,"Constructor changed proven singleton control/form fields");
        need(word(rdram,0x801700F4)==globals.camera_callback && byte(rdram,0x80101404)==globals.primary_tail && word(rdram,0x800992D0)==globals.audio_mode,"Constructor changed primary camera/private-tail/audio mode");
        need(independent_graph(rdram,p0,p1) && owned.size()==4 && owns(rdram,p1+0x400),"Native child graph was not independently bound");
        frame(rdram,&c,2510,43);need(constructors==1 && initializers==1,"Factory spawned or initialized the player twice on the next frame");
        const uint32_t saved_private=word(rdram,p1+0x400+0x1F4);word(rdram,p1+0x400+0x1F4,saved_private+4);
        need(!owns(rdram,p1+0x400),"Recycled actor/private identity was still owned");word(rdram,p1+0x400+0x1F4,saved_private);
        need(cleanup(rdram,&c) && cleanup_requests==1 && word(rdram,player_global)==p0 && word(rdram,0x801B0014)==p0 && word(rdram,0x801B0018)==1,"Deferred native unregistration failed or erased primary slot5");
        for(uint32_t a:{p1+0x400,p1+0x200,p1+0x600,p1}){c.r4=base(a);native_cleanup(rdram,&c);}
        need(disposals==4 && !secondary && owned.empty() && word(rdram,player_global)==p0 && actor(rdram,p0,0x4001),"Secondary cleanup freed or unregistered the primary");
        seed(rdram);configure(true);const gpr pool=base(pool_header);MEM_H(8,pool)=3;auto low_capacity=memory;frame(rdram,&c,2500,43);
        need(memory==low_capacity && constructors==0 && attempted,"Insufficient native actor capacity was not rejected without writes");
        seed(rdram);configure(true);for(unsigned n=6;n<124;n++)word(rdram,0x801B0000+n*4,1);auto full_active=memory;frame(rdram,&c,2500,43);
        need(memory==full_active && constructors==0,"Insufficient main-manager capacity was not rejected");
        
        seed(rdram);configure(true);byte(rdram,p0+0x17D,4);auto queued=memory;frame(rdram,&c,2500,43);
        need(memory==queued && constructors==0 && !attempted,"Pending17D4 primary was mistaken for normal registered17D2");
        const auto logged_wait=wait_mask;frame(rdram,&c,2501,43);need(wait_mask==logged_wait && logged_wait!=0,"A repeated WAIT guard was not logged exactly once");
        for(unsigned offset:{0x130u,0x20u}){seed(rdram);configure(true);word(rdram,p0+offset,0);auto bad_callback=memory;frame(rdram,&c,2500,43);need(memory==bad_callback && constructors==0 && !attempted,"Noncanonical player callback was accepted");}
        seed(rdram);configure(true);word(rdram,0x801B0014,1);auto lost_slot=memory;frame(rdram,&c,2500,43);
        need(memory==lost_slot && constructors==0,"Factory accepted a missing primary manager slot5");
        
        seed(rdram);configure(true);word(rdram,0x800A6380,1);auto stock_graphics_full=memory;frame(rdram,&c,2500,43);
        need(memory==stock_graphics_full && constructors==0 && attempted,"Exhausted stock type3 graphics pool reached the native constructor");
        seed(rdram);configure(true);word(rdram,0x800A6384,0);auto invalid_graphics=memory;frame(rdram,&c,2500,43);
        need(memory==invalid_graphics && constructors==0,"Invalid graphics pool flags were accepted");
        seed(rdram);configure(true);fail_constructor=true;auto fail_globals=Globals::capture(rdram);frame(rdram,&c,2500,43);
        need(constructors==1 && !secondary && word(rdram,player_global)==p0,"Failed native initialization lost the original player");
        seed(rdram);configure(true);fail_initializer=true;frame(rdram,&c,2500,43);
        need(constructors==1 && initializers==1 && activations==1 && !secondary && cleanup_requests==1 && word(rdram,player_global)==p0,"Failed genuine initializer exposed an unready secondary or lost primary");
        need(!constructing,"A completed or failed initialization leaked construction mode");
        seed(rdram);configure(true);group(rdram,p1,0x80200000);word(rdram,p1+0x16C,word(rdram,p0+0x16C));
        need(!independent_graph(rdram,p0,p1),"Shared primary graphics were accepted as a second native skeleton");
        std::cout<<"Coop native factory mocks PASS: guards, capacity, constructor/activation route, independent graph, targeted globals/register restoration, incarnation identity and native deferred disposal. Actual native two-player proof is separate.\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}
}
