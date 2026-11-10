#include "../starting_bonus0283.hpp"
#include "json/json.hpp"
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif
namespace sb=workshop::starting_bonus0283;
namespace iv=workshop::boss_checkpoint_inventory0260;
namespace n=workshop::boss_native;
namespace w=workshop::heart_wallet0283;
namespace fs=std::filesystem;
static unsigned checks=0,allocations=0,allocation_limit=6,health_calls=0;
static int32_t sections[300]{};int32_t* section_addresses=sections;
static constexpr uint32_t player=0x80010000,data=0x80020000,stack=0x80080000,palette=0x80500000,owner=0x80091000,info=0x80092000;
static void need(bool yes,const char* why){++checks;if(!yes)throw std::runtime_error(why);}
static void word(uint8_t* rdram,uint32_t a,uint32_t v){MEM_W(0,n::base(a))=v;}
static void half(uint8_t* rdram,uint32_t a,unsigned v){MEM_H(0,n::base(a))=uint16_t(v);}
static void byte(uint8_t* rdram,uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}
extern "C" void func_8003E5EC(uint8_t*,recomp_context*);
extern "C" void func_80068C00(uint8_t*,recomp_context*);
extern "C" void func_8006E0FC(uint8_t*,recomp_context*);
extern "C" void func_804A2714(uint8_t*,recomp_context*);
extern "C" void func_80068CF8(uint8_t*,recomp_context*){} // scheduler is outside this CPU oracle
extern "C" void switch_error(const char*,uint32_t,uint32_t){std::abort();}
extern "C" recomp_func_t* get_function(int32_t address){if(uint32_t(address)==0x8003E5EC)return func_8003E5EC;std::abort();}
extern "C" void Fault_HungUp(uint8_t*,recomp_context*){std::abort();}
extern "C" void func_80047090(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_80046804(uint8_t* r,recomp_context* c){
    if(allocations>=allocation_limit){c->r2=0;return;}
    const uint32_t a=0x80200000+allocations++*0x400,resource=a+0x200,drawing=a+0x300;
    word(r,a,0x19971110);half(r,a+12,unsigned(c->r5>>16));word(r,a+0x168,resource);word(r,a+0x16C,drawing);word(r,resource+0x88,palette);word(r,drawing,1);c->r2=n::base(a);
}
extern "C" void func_8004590C(uint8_t* r,recomp_context* c){word(r,n::word(r,uint32_t(c->r4)+0x16C)+8,uint32_t(c->r5));}
extern "C" void func_8003B890(uint8_t*,recomp_context*){std::abort();}
extern "C" void func_80065490(uint8_t* r,recomp_context* c){for(unsigned i=0;i<3;++i)word(r,uint32_t(c->r4)+i*4,n::word(r,uint32_t(c->r5)+i*4));}
extern "C" void func_8001D770(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_80067E98(uint8_t*,recomp_context* c){c->r2=0;}
extern "C" void func_80067818(uint8_t*,recomp_context*){}
extern "C" void func_800679F0(uint8_t*,recomp_context*){++health_calls;}
// Explicit fixture boundary for a completed native manager preparation.
// The REAL68C00 body, not the helper, derives flag6 from the actual stage.
extern "C" void func_80068B2C(uint8_t*,recomp_context* c){c->r2=1;}
static std::string bytes(const fs::path& p){std::ifstream input(p,std::ios::binary);return {std::istreambuf_iterator<char>(input),{}};}
static void put(const fs::path& p,const std::string& s){std::ofstream output(p,std::ios::binary);output<<s;need(bool(output),"QA fixture write failed");}
struct Fixture {
    std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);
    uint8_t* ram=memory.data();recomp_context context{};
    Fixture(unsigned color=0,unsigned hp=3,unsigned eggs=0,unsigned stage=6){
        allocations=0;allocation_limit=6;health_calls=0;
        word(ram,player,0x19971110);half(ram,player+12,0x4001);word(ram,player+0x1F4,data);byte(ram,player+0x18,1);byte(ram,player+0x17D,2);
        word(ram,0x800F8828,player);word(ram,0x807F0008,color);word(ram,0x800F8DB0,color<8?color:0);word(ram,0x800F8DB4+(color<8?color:0)*4,hp);
        for(unsigned axis=0;axis<3;++axis){word(ram,player+0x6C+axis*4,std::bit_cast<uint32_t>(.8f));word(ram,player+0x78+axis*4,std::bit_cast<uint32_t>(.8f));}
        word(ram,0x800AFFE8,std::bit_cast<uint32_t>(.7f));word(ram,data+0x64,0x3F800000);
        word(ram,player+0x84,0x43000000);word(ram,player+0x88,0x43800000);word(ram,player+0x8C,0x43FA0000);
        const unsigned head=(color+hp+eggs)%8;half(ram,data+0x12BE,head);byte(ram,data+0x12C8,eggs);half(ram,player+0x184,eggs);
        for(unsigned i=0;i<eggs;++i){
            const uint32_t a=0x80100000+i*0x400;word(ram,a,0x19971110);half(ram,a+12,0x4004);word(ram,a+0x180,player);half(ram,a+0x17E,i);word(ram,a+0x1EC,(color+i)%14);
            for(unsigned axis=0;axis<3;++axis)word(ram,a+0x78+axis*4,std::bit_cast<uint32_t>(.5f+.015625f*float(i)+.125f*float(axis)));
            word(ram,player+0x188+i*4,a);word(ram,data+0x11FC+((head+i)&7)*24,a);
        }
        word(ram,0x800F8958,72);word(ram,0x800F89D0,10);word(ram,0x800F8944,77);
        context.r29=n::base(stack);context.r31=0x12345678;context.r4=11;context.r5=22;context.r16=0xAABBCCDD;
        set_stage(stage);
    }
    void request(unsigned stage,unsigned scene){
        auto call=context;call.r4=0;call.r5=stage;call.r6=scene;func_8006E0FC(ram,&call);
        need(n::byte(ram,0x800F7EA1)==stage,"Actual native transition did not set pending stage");
        // The real687B8 promotion is an explicit lifecycle input boundary here.
        set_stage(n::byte(ram,0x800F7EA1));
    }
    void select_level(){
        constexpr uint32_t menu=0x80300000,private_data=0x80301000,child=0x80302000,drawing=0x80303000,overlay=0x80240000;
        sections[57]=int32_t(overlay);
        for(unsigned level=0;level<24;++level)byte(ram,overlay+0x30A0+level,workshop::heart_sources0283::sources[level*3].root);
        word(ram,menu+0x1F4,private_data);word(ram,private_data+0x28,child);word(ram,child+0x16C,drawing);byte(ram,drawing+0x10,250);
        auto call=context;call.r4=n::base(menu);func_804A2714(ram,&call);
        need(n::byte(ram,0x800F7EA1)==7&&n::byte(ram,0x800F7EA6)==1,"Actual Yoshi selector did not request stage7/release");
        set_stage(n::byte(ram,0x800F7EA1));
    }
    void set_stage(unsigned stage){
        byte(ram,0x800F7EA2,stage);auto call=context;func_80068C00(ram,&call);
    }
    auto scratch()const{std::array<uint8_t,0x480> result{};std::memcpy(result.data(),ram+((stack-0x400)&0x7FFFFF),result.size());return result;}
};
static void fund(w::Wallet& wallet,const fs::path& profile,w::Pending pending){
    fs::create_directory(profile);need(wallet.load(profile),"Wallet fixture did not load");
    for(unsigned bit=0;bit<8;++bit)need(wallet.claim(bit/3,bit%3).committed(),"Synthetic fixture credit failed");
    if(pending.life)need(wallet.purchase(w::Bonus::Life,1).committed(),"Synthetic life purchase failed");
    if(pending.eggs)need(wallet.purchase(w::Bonus::Eggs,2).committed(),"Synthetic eggs purchase failed");
}
static sb::Environment environment(sb::Mode mode){return {mode,10,false,mode==sb::Mode::BossBattle,0,0};}
static void evidence(Fixture& f,sb::Controller& controller,sb::Environment& env,bool new_birth=true,bool birth_after_ready=false,unsigned requested=43,bool metadata_ok=true){
    f.set_stage(0);word(f.ram,0x800F8B88,env.mode==sb::Mode::Story?0:1);byte(f.ram,0x800F8DFC,env.mode==sb::Mode::BossBattle?2:0);byte(f.ram,0x800F8DFD,0);
    controller.observe_scene(f.ram,env);f.request(6,2);auto call=f.context;
    controller.trace(f.ram,&call,"func_80052298",env);
    call.r4=n::base(owner);call.r5=2;controller.trace(f.ram,&call,"func_8006DA0C",env);
    word(f.ram,owner+0x20,info);byte(f.ram,info+1,2);call.r4=n::base(info);controller.trace(f.ram,&call,"func_8006DD34",env);
    if(sb::allowed(env.mode)&&!env.custom&&(env.mode!=sb::Mode::BossBattle||env.boss_armed)){
        need(controller.snapshot().phase==sb::Phase::WaitingGameStage&&controller.snapshot().attempt.empty(),"Selection raw2 cancelled the candidate or prematurely armed a game token");
    }
    f.select_level();env.vi=15;controller.trace(f.ram,&call,"func_80052298",env);
    if(new_birth&&!birth_after_ready){call.r4=n::base(player);controller.trace(f.ram,&call,"func_80045CAC",env);}
    call.r4=n::base(owner);call.r5=requested;controller.trace(f.ram,&call,"func_8006DA0C",env);
    word(f.ram,owner+0x20,metadata_ok?info:info+4);byte(f.ram,info+1,requested);
    call.r4=n::base(info);controller.trace(f.ram,&call,"func_8006DD34",env);
    if(new_birth&&birth_after_ready){call.r4=n::base(player);controller.trace(f.ram,&call,"func_80045CAC",env);}
    env.vi=20;
}
int main(int argc,char** argv){try{
    if(argc!=2)throw std::runtime_error("Explicit NEW isolated CPU output required");
    const auto out=fs::absolute(argv[1]);if(fs::exists(out))throw std::runtime_error("Refusing to replace previous test artifacts");fs::create_directories(out);
    {Fixture six(0,3,0,6),seven(0,3,0,7);need(n::byte(six.ram,0x800F7EA6)==0&&n::byte(seven.ram,0x800F7EA6)==1,"Actual68C00 oracle did not establish stage6-zero/stage7-one controls");}
    for(unsigned level=0;level<24;++level){
        Fixture f;byte(f.ram,0x800F8DFC,level/4);byte(f.ram,0x800F8DFD,level%4);f.request(6,2);
        need(n::byte(f.ram,0x800F8433)==2,"Actual6DFC4 did not store scene6 at F8433");
        f.select_level();need(n::byte(f.ram,0x800F8424)==workshop::heart_sources0283::sources[level*3].root,"Actual804A2714/6DFC4 did not request the correct stage7 scene at F8424");
    }
    {Fixture f;w::Wallet wallet;const auto dir=out/"selection-only";fund(wallet,dir,{true,true});sb::Controller controller(90);auto env=environment(sb::Mode::Trial);
     f.set_stage(0);word(f.ram,0x800F8B88,1);controller.observe_scene(f.ram,env);f.request(6,2);auto call=f.context;controller.trace(f.ram,&call,"func_80052298",env);
     call.r4=n::base(owner);call.r5=2;controller.trace(f.ram,&call,"func_8006DA0C",env);word(f.ram,owner+0x20,info);byte(f.ram,info+1,2);call.r4=n::base(info);controller.trace(f.ram,&call,"func_8006DD34",env);
     call.r4=n::base(player);controller.trace(f.ram,&call,"func_80045CAC",env);const auto saved=bytes(dir/w::filename);controller.frame(f.ram,&f.context,env,wallet);
     need(controller.snapshot().phase==sb::Phase::WaitingGameStage&&controller.snapshot().attempt.empty()&&!allocations&&!health_calls&&bytes(dir/w::filename)==saved,"Yoshi selection stage6 consumed a voucher or invoked game effects");
     f.select_level();controller.trace(f.ram,&call,"func_80052298",env);call.r4=n::base(owner);call.r5=43;controller.trace(f.ram,&call,"func_8006DA0C",env);word(f.ram,owner+0x20,info);byte(f.ram,info+1,43);call.r4=n::base(info);controller.trace(f.ram,&call,"func_8006DD34",env);controller.frame(f.ram,&f.context,env,wallet);
     need(controller.snapshot().phase==sb::Phase::Idle&&!allocations&&!health_calls&&bytes(dir/w::filename)==saved,"Selection-stage birth incorrectly qualified as a fresh stage7 player");}
    unsigned matrix=0;
    for(unsigned color=0;color<14;++color)for(unsigned hp=0;hp<=8;++hp)for(unsigned count=0;count<=6;++count)for(w::Pending pending:{w::Pending{true,false},w::Pending{false,true},w::Pending{true,true}}){
        Fixture f(color,hp,count);sb::NativePlan plan;
        need(sb::prepare(f.ram,&f.context,player,100,pending,plan),"Read-only preflight rejected actual stage6-zero control oracle");
        const auto registers=f.context;const auto scratch=f.scratch();const auto original=plan.original;
        need(sb::apply_native(f.ram,&f.context,plan),"Exact native health/topup application failed");
        need(!std::memcmp(&registers,&f.context,sizeof registers)&&f.scratch()==scratch,"Native application changed caller context or reserved scratch");
        iv::Inventory after;need(iv::read(f.ram,player,after),"Native graph/FIFO readback failed");
        need(after.count==(pending.eggs?6u:count)&&n::word(f.ram,0x800F8DB4+(color<8?color:0)*4)==(pending.life?8u:hp),"Bonus changed wrong quantity or HP");
        for(unsigned i=0;i<count;++i)need(after.actors[i]==original.actors[i]&&after.eggs[i]==original.eggs[i],"Native topup changed existing egg order/palette/scale");
        for(unsigned i=count;i<after.count;++i)need(after.eggs[i].palette==color&&after.eggs[i].saved_scale==plan.target.eggs[i].saved_scale&&after.eggs[i].saved_scale[0]==std::bit_cast<uint32_t>(.8f*.7f),"Added reserve is enlarged or lacks selected native palette");
        need(n::word(f.ram,0x800F8958)==72&&n::word(f.ram,0x800F89D0)==10&&n::word(f.ram,0x800F8944)==77,"Starting bonus changed native heart statistic/fruits/SuperHappy");
        need(allocations==(pending.eggs?6-count:0)&&health_calls==unsigned(pending.life&&hp!=8),"Unexpected native allocation or health setter count");
        ++matrix;
    }
    for(unsigned fault=0;fault<9;++fault){
        Fixture f;const auto pristine=f.memory;auto call=f.context;
        if(fault==0)byte(f.ram,player+0x17D,1);if(fault==1)byte(f.ram,player+0x165,1);if(fault==2)word(f.ram,data+0x12EC,1);
        if(fault==3)byte(f.ram,0x800F7EA5,1);if(fault==4)byte(f.ram,0x800F7EA6,2);if(fault==5)word(f.ram,0x800AFFE8,0x7FC00000);
        if(fault==6)call.r29=0;if(fault==7)byte(f.ram,data+0x12C8,7);if(fault==8)word(f.ram,0x800F8828,player+4);
        const auto before=f.memory;sb::NativePlan plan;need(!sb::prepare(f.ram,&call,player,100,{true,true},plan)&&f.memory==before,"Invalid preflight mutated native memory or accepted malformed buffers");(void)pristine;
    }
    for(unsigned order=0;order<2;++order)for(auto mode:{sb::Mode::Story,sb::Mode::Trial,sb::Mode::Hybrid,sb::Mode::BossBattle}){
        Fixture f;w::Wallet wallet;const auto dir=out/("positive-"+std::to_string(order)+"-"+std::to_string(unsigned(mode)));fund(wallet,dir,{true,true});
        sb::Controller controller(0xAABBCCDD);auto env=environment(mode);evidence(f,controller,env,true,order!=0,mode==sb::Mode::BossBattle?52u:43u);
        const auto registers=f.context;controller.frame(f.ram,&f.context,env,wallet);
        need(controller.snapshot().phase==sb::Phase::Complete&&!controller.unsettled()&&wallet.snapshot().value.pending.empty(),"Admitted initial attempt did not apply and consume once");
        need(!std::memcmp(&registers,&f.context,sizeof registers)&&allocations==6&&health_calls==1,"Controller damaged context or called unexpected native effects");
        const auto calls=allocations+health_calls;const auto saved=bytes(dir/w::filename);controller.frame(f.ram,&f.context,env,wallet);
        need(allocations+health_calls==calls&&bytes(dir/w::filename)==saved,"Same frame/attempt replayed native effects or consumed twice");
        need(wallet.load(dir)&&wallet.consume_pending(controller.snapshot().attempt,{true,true}).status==w::Status::AlreadyConsumed,"Current-attempt idempotency was lost after reload");
    }
    for(unsigned fault=0;fault<8;++fault){
        Fixture f;w::Wallet wallet;const auto dir=out/("admission-"+std::to_string(fault));fund(wallet,dir,{true,true});sb::Controller controller(100+fault);auto env=environment(sb::Mode::Trial);
        if(fault==0)env.mode=sb::Mode::Practice;if(fault==1)env.mode=sb::Mode::Custom;if(fault==2)env.custom=true;if(fault==3){env.mode=sb::Mode::BossBattle;env.boss_armed=false;env.retry_epoch=1;}
        const auto saved=bytes(dir/w::filename);evidence(f,controller,env,fault!=4,false,fault==5?81u:43u,fault!=6);
        if(fault==7){auto call=f.context;call.r4=n::base(owner);call.r5=81;controller.trace(f.ram,&call,"func_8006DA0C",env);}
        const auto before=f.memory;controller.frame(f.ram,&f.context,env,wallet);
        need(f.memory==before&&allocations==0&&health_calls==0&&bytes(dir/w::filename)==saved,"Practice/custom/retry/old-birth/room/metadata guard allowed a bonus");
    }
    {Fixture f;w::Wallet wallet;const auto dir=out/"release-wait";fund(wallet,dir,{true,true});sb::Controller controller(300);auto env=environment(sb::Mode::Trial);evidence(f,controller,env);byte(f.ram,player+0x17D,1);const auto saved=bytes(dir/w::filename);
     controller.frame(f.ram,&f.context,env,wallet);need(!allocations&&!health_calls&&bytes(dir/w::filename)==saved,"Queued P1 consumed bonuses before release");byte(f.ram,player+0x17D,2);++env.vi;controller.frame(f.ram,&f.context,env,wallet);need(controller.snapshot().phase==sb::Phase::Complete,"Released P1 could not use still-pending bonuses");}
    {Fixture f;w::Wallet wallet;const auto dir=out/"native-partial";fund(wallet,dir,{true,true});sb::Controller controller(400);auto env=environment(sb::Mode::Trial);evidence(f,controller,env);allocation_limit=1;const auto saved=bytes(dir/w::filename);const auto registers=f.context;const auto scratch=f.scratch();
     controller.frame(f.ram,&f.context,env,wallet);need(controller.snapshot().phase==sb::Phase::Failed&&allocations==1&&!health_calls&&wallet.snapshot().value.pending==w::Pending{true,true}&&bytes(dir/w::filename)==saved,"Partial native failure lost voucher or healed");need(!std::memcmp(&registers,&f.context,sizeof registers)&&f.scratch()==scratch,"Partial native failure damaged context/scratch");allocation_limit=6;++env.vi;controller.frame(f.ram,&f.context,env,wallet);need(allocations==1&&!health_calls,"Failed native attempt retried and manufactured more eggs");}
    for(unsigned tracked=0;tracked<2;++tracked){Fixture f;w::Wallet wallet;const auto dir=out/("carried-then-late-birth-"+std::to_string(tracked));fund(wallet,dir,{true,true});sb::Controller controller(450);auto env=environment(sb::Mode::Trial);
     if(tracked)controller.created(player);evidence(f,controller,env,false);const auto saved=bytes(dir/w::filename);const auto before=f.memory;
     controller.frame(f.ram,&f.context,env,wallet);need(controller.snapshot().phase==sb::Phase::Idle&&f.memory==before&&bytes(dir/w::filename)==saved,"Old or unknown released P1 did not close the initial candidate");
     auto call=f.context;call.r4=n::base(player);controller.trace(f.ram,&call,"func_80045CAC",env);++env.vi;controller.frame(f.ram,&f.context,env,wallet);
     need(controller.snapshot().phase==sb::Phase::Idle&&!allocations&&!health_calls&&bytes(dir/w::filename)==saved&&wallet.snapshot().value.pending==w::Pending{true,true},"Late P1 birth without a new edge/entry reopened a carried-player candidate");}
    {Fixture f;w::Wallet wallet;const auto dir=out/"external-invalid";fund(wallet,dir,{true,true});sb::Controller controller(500);auto env=environment(sb::Mode::Trial);evidence(f,controller,env);put(dir/w::filename,"external-corruption");const auto before=f.memory;controller.frame(f.ram,&f.context,env,wallet);need(f.memory==before&&!wallet.snapshot().purchases_enabled()&&bytes(dir/w::filename)=="external-corruption","Native effect ran before detecting invalid wallet on disk");
     const nlohmann::json restored{{"schema",1u},{"held",std::string(72,'0')},{"pending",{{"life",true},{"eggs",true}}}};put(dir/w::filename,restored.dump());need(wallet.load(dir),"Restored isolated wallet did not reload");++env.vi;controller.frame(f.ram,&f.context,env,wallet);
     need(controller.snapshot().phase==sb::Phase::Idle&&!allocations&&!health_calls&&wallet.snapshot().value.pending==w::Pending{true,true},"Recovered wallet applied a bonus after the initial fence closed");}
    {Fixture f;w::Wallet wallet;const auto dir=out/"buffers-then-recovery";fund(wallet,dir,{true,true});sb::Controller controller(550);auto env=environment(sb::Mode::Trial);evidence(f,controller,env);const auto saved=bytes(dir/w::filename);
     word(f.ram,0x800AFFE8,0x7FC00000);controller.frame(f.ram,&f.context,env,wallet);need(controller.snapshot().phase==sb::Phase::Idle&&!allocations&&!health_calls&&bytes(dir/w::filename)==saved,"Invalid initial native buffers did not preserve the voucher and close the candidate");
     word(f.ram,0x800AFFE8,std::bit_cast<uint32_t>(.7f));word(f.ram,0x800F8DB4,1);++env.vi;controller.frame(f.ram,&f.context,env,wallet);
     need(!allocations&&!health_calls&&n::word(f.ram,0x800F8DB4)==1&&bytes(dir/w::filename)==saved,"Native buffer recovery applied a delayed in-game heal/topup");}
#ifdef _WIN32
    {Fixture f;w::Wallet wallet;const auto dir=out/"preflight-lock-recovery";fund(wallet,dir,{true,true});sb::Controller controller(575);auto env=environment(sb::Mode::Trial);evidence(f,controller,env);const auto saved=bytes(dir/w::filename);
     HANDLE held=CreateFileW((dir/"pc-heart-wallet.lock").c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);need(held!=INVALID_HANDLE_VALUE,"QA preflight OS lock failed");
     controller.frame(f.ram,&f.context,env,wallet);CloseHandle(held);need(controller.snapshot().phase==sb::Phase::Idle&&!allocations&&!health_calls&&bytes(dir/w::filename)==saved,"Preflight I/O failure mutated native state or lost a voucher");
     need(wallet.refresh(),"Wallet lock recovery did not refresh");++env.vi;controller.frame(f.ram,&f.context,env,wallet);
     need(!allocations&&!health_calls&&bytes(dir/w::filename)==saved&&wallet.snapshot().value.pending==w::Pending{true,true},"Recovered I/O reopened native application in the same attempt");}
    {Fixture f;w::Wallet wallet;const auto dir=out/"io-commit";fund(wallet,dir,{true,true});sb::Controller controller(600);auto env=environment(sb::Mode::Trial);evidence(f,controller,env);
     HANDLE held=CreateFileW((dir/w::filename).c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);need(held!=INVALID_HANDLE_VALUE,"QA I/O failure handle failed");
     controller.frame(f.ram,&f.context,env,wallet);need(controller.snapshot().phase==sb::Phase::AppliedAwaitingCommit&&controller.unsettled()&&allocations==6&&health_calls==1&&wallet.snapshot().value.pending==w::Pending{true,true},"I/O failure repeated effects or lost pending voucher");
     env.vi=21;controller.frame(f.ram,&f.context,env,wallet);need(allocations==6&&health_calls==1,"Throttled I/O retry replayed native effects");
     CloseHandle(held);word(f.ram,0x800F8DB4,4);env.vi=80;f.set_stage(7);controller.frame(f.ram,&f.context,env,wallet);
     need(controller.snapshot().phase==sb::Phase::Complete&&!controller.unsettled()&&wallet.snapshot().value.pending.empty()&&n::word(f.ram,0x800F8DB4)==4&&allocations==6&&health_calls==1,"HOST-only commit recovery blocked gameplay or replayed healing");
    }
#endif
    nlohmann::json report{{"passed",true},{"checks",checks},{"nativeMatrixCases",matrix},{"actualNativeControlsOracle","68C00: stage6->0,stage7->1"},{"actualNativeSelectionOracle","804A2714->6E0FC->6E06C->6DFC4: selection6/2 then game7/root, F8424 verified for24roots"},{"selectionStageNeverConsumes",true},{"nativeStartingEffects","40860/3BB20/3BA90/47F9C/3F14C/3E658/3E5EC/3B8B0/422CC/2B3A0"},{"allEntryAndPlayerInputsSynthetic",true},{"scratchAndContextRestored",true},{"lateBirthAndPreflightRecoveryNeverReopenAttempt",true},{"existingEggPalettesScalesAndOrderPreserved",true},{"nativeNormalScaleRecipe",true},{"currentAttemptConsumeIdempotent",true},{"ioFailureDoesNotBlockGameOrReplayEffects",true},{"partialNativeFailureRetainsVoucherWithoutRetry",true},{"practiceCustomCheckpointAndRoomChangesExcluded",true},{"gameLaunched",false},{"personalProfileUsed",false},{"gpuGameplayVerified",false}};
    put(out/"verification.json",report.dump(2)+'\n');std::cout<<report.dump(2)<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
