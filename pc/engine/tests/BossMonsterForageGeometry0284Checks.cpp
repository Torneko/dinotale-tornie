#include "../boss_monster_forage_geometry0284.hpp"
#include <cstdio>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>
namespace g=workshop::boss_monster_forage_geometry0284;namespace n=workshop::boss_native;
static unsigned checks=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
static void host_word(std::vector<uint8_t>& host,uint32_t address,uint32_t value){auto* rdram=host.data();MEM_W(0,n::base(address))=value;}
int main(int argc,char** argv){try{
 need(argc==3,"Supply immutable5917 and94 GPU final RAM dumps for geometry only");std::ifstream input(argv[1],std::ios::binary);std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(input)),{});need(bytes.size()==0x800000,"Actual8MiB RAM dump size");auto* ram=bytes.data();const auto player=n::word(ram,0x800F8828);const auto before=bytes;const auto scene=g::observe(ram,player);need(bool(scene),"Actual typed singleton/native12probes/body/queue2/parent0/3x2map observed");
 std::printf("ActualBody player=%08X xy=%.3f,%.3f extents=%.3f,%.3f,%.3f,%.3f\n",player,g::m::scalar(ram,player+0x84),g::m::scalar(ram,player+0x88),scene->body.left,scene->body.right,scene->body.top,scene->body.bottom);
 struct Probe {float x,y,tx,ty;bool contact,expected;const char* name;};
 for(const Probe p:std::array<Probe,8>{{{248,305,290,305,true,true,"departureFloor"},{248,303,290,210,false,true,"rising210"},{248,303,290,190,false,true,"rising190"},{290,190,248,190,false,false,"returnAir190"},{290,210,248,210,false,true,"returnAir210"},{248,210,248,305,true,true,"returnFloor210"},{248,303,312,210,false,true,"riseToLift312"},{312,210,248,210,false,true,"returnFromLift312"}}}){const bool result=g::clear(ram,*scene,p.x,p.y,p.tx,p.ty,p.contact);std::printf("corridor %s clear=%u\n",p.name,unsigned(result));need(result==p.expected,"Actual current terrain admits safe candidates and rejects blocked190px return (not native trajectory proof)");}
 need(!g::clear(ram,*scene,248,305,290,305),"No floor slack is inferred for full airborne feet at supportedY305");
 need(!g::observe(nullptr,player)&&!g::observe(ram,0),"Unknown player source rejected");
 auto invalid=*scene;invalid.map.data=0;need(!g::clear(ram,invalid,248,303,290,210),"Unknown or replaced map rejected");invalid=*scene;invalid.body.left=std::numeric_limits<float>::quiet_NaN();need(!g::clear(ram,invalid,248,303,290,210),"Unknown/nonfinite native-derived body rejected");
 for(float bad:{-1.f,769.f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()})need(!g::clear(ram,*scene,bad,305,290,210),"Out-of-map or nonfinite route rejected");
 need(!g::clear(ram,*scene,248,303,700,210),"Unbounded route rejected");need(before==bytes,"Every actual GPU dump observation preserves all8MiB");
 // Negative probes below are explicit disposable HOST copies. The actual
 // dump, its loaded8MiB vector and all live process memory stay untouched.
 const auto meta=n::word(ram,player+0x174),def=n::word(ram,meta),xs=n::word(ram,def+0x30);
 for(unsigned fault=0;fault<5;++fault){auto host=bytes;if(fault==0)host_word(host,def+0x30,0);if(fault==1)host_word(host,def+0x2C,11);if(fault==2)host_word(host,xs+6*4,0x7FC00000);if(fault==3)host_word(host,player+0x180,0x800C88E0);if(fault==4)host_word(host,0x800F8810,0);const auto hostBefore=host;need(!g::observe(host.data(),player),"Unknown native probe/map/attachment HOST frontier rejected");need(host==hostBefore,"Negative geometry observer remains read-only on HOST copy");}
 {const auto lift=g::elevator(ram);need(lift&&lift->actor==0x800C88E0&&lift->x==312&&std::abs(lift->half_x-36.f)<.001f,"First immutable native snapshot identifies the real312 elevator and its current36px half-width");}
 {std::ifstream second(argv[2],std::ios::binary);std::vector<uint8_t> failure((std::istreambuf_iterator<char>(second)),{});need(failure.size()==0x800000,"Actual94 failure snapshot is8MiB");auto* r=failure.data();const auto unchanged=failure;const auto player94=n::word(r,0x800F8828);const auto map94=g::observe(r,player94);const auto lift94=g::elevator(r);need(map94&&lift94&&lift94->x==312&&std::abs(lift94->half_x-31.2f)<.001f,"Second immutable native snapshot independently proves312 elevator with dynamic31.2px half-width");
  need(g::clear(r,*map94,248,303,312,210)&&g::clear(r,*map94,312,210,248,210),"Actual94 map admits the bounded rising and210px return candidates");
  need(g::clear(r,*map94,312,460,312,360),"Actual94 map admits the candidate elevator-column rise from real lower floor height");
  need(!g::clear(r,*map94,267,460,312,460,true),"Conservative floor rectangle intersects actual slope0649; ordinary native recovery is not falsely labelled a clear corridor");
  need(failure==unchanged,"Actual94 snapshot unchanged; these are geometry candidates, not an admission of its already-dead player");
 }
 std::printf("BossMonsterForageGeometry0284Checks: %u PASS; actual GPU dump read-only; HOST negative fixtures; clearance only, no physics/landing/food/native-movement claim\n",checks);return 0;
 }catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}}
