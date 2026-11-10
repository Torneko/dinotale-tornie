#include "recomp.h"
#include <array>
#include <atomic>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
namespace {
unsigned checks=0,snapshot_calls=0,metal_calls=0;
int session_mode=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
template<class T>T& access(uint8_t* ram,uint64_t a,uint64_t b,unsigned flip){const uint32_t address=uint32_t(a+b);if(address<0x80000000||address>0x80800000-sizeof(T))throw std::runtime_error("Snapshot RDRAM out of bounds");return *reinterpret_cast<T*>(ram+((address-0x80000000)^flip));}
}
#undef MEM_W
#undef MEM_H
#undef MEM_HU
#undef MEM_B
#undef MEM_BU
#define MEM_W(a,b) access<int32_t>(rdram,uint64_t(a),uint64_t(b),0)
#define MEM_H(a,b) access<int16_t>(rdram,uint64_t(a),uint64_t(b),2)
#define MEM_HU(a,b) access<uint16_t>(rdram,uint64_t(a),uint64_t(b),2)
#define MEM_B(a,b) access<int8_t>(rdram,uint64_t(a),uint64_t(b),3)
#define MEM_BU(a,b) access<uint8_t>(rdram,uint64_t(a),uint64_t(b),3)
#include "../workshop_menu_game.hpp"
#include "../workshop_menu.hpp"
#include "../story_progress.hpp"
#include "../game_modes0320.hpp"
#include "../hybrid_finds.hpp"
#include "../metal_yoshis0300.hpp"
int workshop_menu_session_mode(){return session_mode;}
namespace workshop::metal_yoshis0300 {
View view(){++metal_calls;return {};}
unsigned search_mask(){++metal_calls;return 0;}
bool acquire(unsigned){++metal_calls;return false;}
}
namespace workshop::book {namespace {
constexpr gpr manager=gpr(int32_t(0x800F7EA0));
std::atomic_bool paused{false};
#include MENU_GAME_SNAPSHOT_BODY
}}
namespace {
namespace book=workshop::book;
constexpr uint32_t root=0x800F7EA0,player=0x80100000;
void word(uint8_t* rdram,uint32_t at,uint32_t value){MEM_W(0,int32_t(at))=value;}
void byte(uint8_t* rdram,uint32_t at,unsigned value){MEM_B(0,int32_t(at))=uint8_t(value);}
void setup(uint8_t* rdram,const workshop::progress::State& p,unsigned story,unsigned marker=0xF17D0003){
 std::memset(rdram,0,0x800000);workshop::progress::write(rdram,gpr(int32_t(0x807F2000)),p);word(rdram,0x800F8E5C,story);word(rdram,0x807FC144,marker);byte(rdram,root+2,7);byte(rdram,root+0x57D,46);word(rdram,0x800F8B88,0);word(rdram,0x800F8828,player);word(rdram,0x800F89D0,17);word(rdram,player+0x3C,0x42A00000);word(rdram,player+0x40,0x43480000);
}
std::string bytes(const std::filesystem::path& file){std::ifstream input(file,std::ios::binary);return {std::istreambuf_iterator<char>(input),{}};}
void ledger(const std::filesystem::path& file,const std::string& text){std::ofstream out(file,std::ios::binary);out<<text;out.close();check(bool(out),"Cannot write isolated ledger fixture");workshop::finds::initialize(file);}
book::Snapshot snapshot(uint8_t* rdram,int mode,unsigned expected,unsigned vi=4321){session_mode=mode;++snapshot_calls;const auto s=book::read_snapshot(rdram,vi);check(s.extras==expected,"Snapshot extras incorrect");check(s.ready&&s.frame==vi&&s.paused==book::paused.load(),"Snapshot lifecycle fields changed");check(s.stage==7&&s.scene==46&&!s.mainMenu&&s.mode==0,"Snapshot native mode/stage/scene fields changed");check(s.player==player&&s.fruits==17&&s.x==0x42A00000&&s.y==0x43480000&&s.health==0,"Snapshot player/fruit/coordinate fields changed");return s;}
}
int main(int argc,char** argv){try{
 check(argc==2,"Expected isolated fixture directory");const auto dir=std::filesystem::path(argv[1]);const auto file=dir/std::filesystem::path(u8"é-owned-find-snapshot.json");
 std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
 const workshop::progress::State empty{0,255,{255,255,255,255},0,63};
 ledger(file,"{\"version\":1,\"foundMask\":3,\"legacyMigrated\":true}\n");const auto healthy_bytes=bytes(file);const auto healthy=workshop::finds::view();check(healthy.initialized&&healthy.readable&&healthy.migrated&&healthy.saved&&healthy.found==3,"Real find ledger did not initialize");
 setup(rdram,empty,0);auto before=memory;auto s=snapshot(rdram,5,3);check(s.valid&&s.story==0&&s.trial==0,"Legacy child payload changed interpretation");check(memory==before,"StoryHybrid snapshot wrote native RAM");
 for(int mode=0;mode<5;mode++){s=snapshot(rdram,mode,0);check(s.valid&&s.story==0&&s.trial==0,"Old mode child state changed");}
 check(bytes(file)==healthy_bytes,"Read-only snapshot modified ownership ledger");
 // A native child may have a smaller eligible mask than global ownership.
 // OnlyPC5 changes projection; the original payload+RAM marker intersection
 // remains exact for every existing mode0..4, including the delayed Boss slot.
 const workshop::progress::State child{0,0,{255,255,255,255},0,1};setup(rdram,child,0);before=memory;s=snapshot(rdram,5,3);check(s.valid&&s.story==0&&s.trial==0,"Valid child payload lost");for(int mode=0;mode<5;mode++)snapshot(rdram,mode,1);check(memory==before,"Snapshot changed valid child RAM");
 setup(rdram,child,0,0xF17D0002);for(int mode=0;mode<5;mode++)snapshot(rdram,mode,0);snapshot(rdram,5,3);
 const workshop::progress::State story_child{0,5,{0,255,255,255},0,0};setup(rdram,story_child,workshop::progress::column(0),0xF17D0003);s=snapshot(rdram,5,3);check(s.valid&&s.story==0x111111&&s.trial==0,"Modern Story column state lost");for(int mode=0;mode<5;mode++)snapshot(rdram,mode,1);
 // Old modes still ignore an unrecognized transient marker; PC5 continues
 // to read the authoritative journal instead of trusting arbitrary RAM.
 setup(rdram,story_child,workshop::progress::column(0),0xDEADBEEF);for(int mode=0;mode<5;mode++)snapshot(rdram,mode,33);snapshot(rdram,5,3);
 const workshop::progress::State trial_child{0xFFFFFF,0,{1,2,3,4},3,63};setup(rdram,trial_child,0,0xF17D0003);s=snapshot(rdram,5,3);check(s.valid&&s.trial==0xFFFFFF&&s.story==0,"Valid24Trial child fields changed");for(int mode=0;mode<5;mode++){s=snapshot(rdram,mode,3);check(s.valid&&s.trial==0xFFFFFF,"Old24Trial projection changed");}
 for(const auto& corrupt:std::array<std::string,4>{"{","{\"version\":1,\"foundMask\":64,\"legacyMigrated\":true}","{\"version\":1,\"foundMask\":3,\"legacyMigrated\":false}","{\"version\":2,\"foundMask\":3,\"legacyMigrated\":true}"}){
  ledger(file,corrupt);const auto broken=workshop::finds::view();check(broken.initialized&&!broken.readable&&!broken.saved,"Actual corrupt ledger must fail closed");setup(rdram,child,0);before=memory;s=snapshot(rdram,5,0);check(s.valid,"Corrupt ledger changed valid child native state");for(int mode=0;mode<5;mode++)snapshot(rdram,mode,1);check(bytes(file)==corrupt&&memory==before,"Corrupt ledger or native RAM overwritten");
 }
 const auto missing=dir/"missing-find-snapshot.json";workshop::finds::initialize(missing);setup(rdram,child,0);snapshot(rdram,5,0);for(int mode=0;mode<5;mode++)snapshot(rdram,mode,1);check(!std::filesystem::exists(missing),"Snapshot created missing find ledger");
 ledger(file,healthy_bytes);setup(rdram,empty,0);book::paused=true;s=snapshot(rdram,5,3,99);check(s.paused,"Paused state not preserved");book::paused=false;byte(rdram,root+2,0);byte(rdram,root+0x57D,4);session_mode=5;s=book::read_snapshot(rdram,100);++snapshot_calls;check(s.mainMenu&&s.ready&&s.extras==3&&s.frame==100,"Native menu snapshot mapping changed");
 check(metal_calls==0,"Snapshot unexpectedly queried or mutated metal ownership");check(bytes(file)==healthy_bytes,"Final healthy find ledger changed");
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"exactSnapshotCalls\":"<<snapshot_calls<<",\"realFindsInitializeView\":true,\"metalStubCalls\":"<<metal_calls<<",\"game\":false,\"gpu\":false}"<<std::endl;return 0;
 }catch(const std::exception& error){std::cerr<<"FAIL after "<<checks<<" checks: "<<error.what()<<std::endl;return 1;}}
