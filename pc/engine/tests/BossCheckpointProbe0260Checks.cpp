#include "../boss_checkpoint_probe0260.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
namespace p=workshop::boss_checkpoint_probe0260;namespace n=workshop::boss_native;
static unsigned checks=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
struct Fixture {
 static constexpr uint32_t actor=0x80100000,record=0x80101000,resource=0x80102000,metadata=0x80103000,definition=0x80104000,xs=0x80105000,ys=0x80106000,query=0x80107000,overlay=0x80200000;
 std::vector<uint8_t> bytes=std::vector<uint8_t>(0x900000);uint8_t* rdram=bytes.data();recomp_context c{};
 void w(uint32_t a,uint32_t v){MEM_W(0,n::base(a))=int32_t(v);}void b(uint32_t a,unsigned v){MEM_B(0,n::base(a))=uint8_t(v);}void h(uint32_t a,unsigned v){MEM_H(0,n::base(a))=uint16_t(v);}void f(uint32_t a,float v){w(a,std::bit_cast<uint32_t>(v));}
 Fixture(unsigned kind=0x4508){
  p::reset();w(actor,0x19971110);h(actor+12,kind);b(actor+0x17D,2);w(actor+0x1D8,record);w(actor+0x1DC,0x45070000);w(record,0xFFFFFFFF);f(record+4,160);f(record+8,448);
  w(actor+0x168,resource);w(resource+0x40,overlay);w(resource+0x44,overlay+0x1340);w(actor+0x20,overlay+0x1340);w(actor+0x174,metadata);w(metadata,definition);
  f(actor+0x3C,160);f(actor+0x40,448);f(actor+0x44,500.3f);f(actor+0x84,160);f(actor+0x88,448);f(actor+0x8C,500.3f);
  w(definition+0x2C,1);w(definition+0x30,xs);w(definition+0x34,ys);w(definition+0x40,query);w(definition+0x44,0x80049FA4);w(definition+0x48,0x80049FA4);h(definition+0x4C,1);b(definition+0x74,1);b(definition+0x75,1);f(xs,0);f(ys,0);
  const uint32_t row=0x80810000+(kind-0x4001)*24;for(unsigned i=0;i<6;++i)w(row+i*4,std::array<uint32_t,6>{1,0x80300000,1,0x80300100,2,0x80300200}[i]);c.r4=n::base(actor);
 }
 void entry(std::string_view name="func_800483B8",bool enabled=true,size_t capacity=0x900000){p::trace_entry(rdram,&c,name,123,capacity,enabled);}
 void leave(std::string_view name="func_805A6600",bool enabled=true,size_t capacity=0x900000){p::trace_leave(rdram,&c,name,123,capacity,enabled);}
};
int main(int argc,char** argv){try{
 for(unsigned kind:{0x4508u,0x4509u,0x450Au}){
  Fixture x(kind);const auto before=x.bytes;const auto regs=x.c;const auto o=p::observe(x.rdram,Fixture::actor,x.bytes.size());need(bool(o),"Exact numbered point can be observed");need(o->catalogue_known&&o->catalogue[0]==1&&o->catalogue[2]==1&&o->catalogue[4]==2,"PC catalogue WORD counts1/1/2 read beyond8MiB");need(o->probe_known&&o->probe_x==0&&o->probe_y==0&&o->terrain==0x80049FA4,"Native terrain probe/callback values retained");
  const auto short_read=p::observe(x.rdram,Fixture::actor,0x800000);need(short_read&&!short_read->catalogue_known,"8MiB capacity never licenses PC catalogue access");
  x.entry("func_805A6600");x.leave();for(unsigned i=0;i<100;++i)x.entry();need(p::state.updates==100&&p::state.lines==34,"32 first physics observations plus two initializer fences are bounded");
  x.f(Fixture::actor+0x40,464);for(unsigned i=0;i<100;++i)x.entry();need(p::state.lines==46&&p::state.near_updates==100,"Twelve extra surface-window observations cover late crossing");
  need(!std::memcmp(&regs,&x.c,sizeof(regs)),"Probe preserves native context");x.f(Fixture::actor+0x40,448);need(x.bytes==before,"Probe changes no byte of9MiB RAM");
 }
 for(unsigned fault=0;fault<13;++fault){Fixture x;switch(fault){case 0:x.w(Fixture::actor,0);break;case 1:x.h(Fixture::actor+12,0x4507);break;case 2:x.w(Fixture::actor+0x1DC,0x45080000);break;case 3:x.w(Fixture::record,0x45070000);break;case 4:x.f(Fixture::record+4,161);break;case 5:x.f(Fixture::record+8,449);break;case 6:x.w(Fixture::resource+0x44,Fixture::overlay);break;case 7:x.w(Fixture::actor+0x20,Fixture::overlay);break;case 8:x.c.r4=0x807FFFFC;break;case 9:x.b(Fixture::actor+6,3);break;case 10:x.f(Fixture::actor+0x3C,159);break;case 11:x.w(Fixture::actor+0x1D8,0xFFFFFFFF);break;case 12:x.w(Fixture::actor+0x168,0xFFFFFFFF);break;}
  const auto before=x.bytes;x.entry("func_805A6600");x.leave();need(!p::state.lines&&!p::state.actor,"Foreign or malformed initializer cannot register a probe");need(x.bytes==before,"Rejected probes preserve RAM");}
 {Fixture x;const auto before=x.bytes;x.entry("func_805A6600",false);x.leave("func_805A6600",false);need(!p::state.lines&&x.bytes==before,"Disabled diagnostics are inert");}
 {Fixture x;for(uint32_t bad:{0u,0xFFFFFFFFu,0x807FFFFEu}){x.w(Fixture::definition+0x30,bad);x.w(Fixture::definition+0x34,bad);const auto before=x.bytes;const auto o=p::observe(x.rdram,Fixture::actor,0x800000);need(o&&!o->probe_known&&x.bytes==before,"Malformed probe pointers remain unknown without writes");}need(!p::range(0xFFFFFFFF,4,0x900000)&&!p::range(0x808178A8,24,0x800000)&&p::range(0x808178A8,24,0x900000),"Capacity bound cannot wrap into PC catalogue");}
 {Fixture x;uint32_t at=0x80108000;x.w(0x800F8810,at);for(unsigned offset:{0x10u,4u,0x16Cu,0x1B0u,0x20u}){x.w(at+offset,at+0x200);at+=0x200;}const uint32_t pages=0x80110000,data=0x80120000;x.h(at+0x12,3);x.h(at+0x14,2);x.h(at+0x16,96);x.w(at+0x18,pages);x.w(at+0x24,data);x.b(pages+3,3);x.h(data+2*(13*96+58),0x601);x.h(data+2*(14*96+58),0x401);const auto before=x.bytes;const auto o=p::observe(x.rdram,Fixture::actor,x.bytes.size());need(o&&o->tiles[0]==0x601&&o->tiles[1]==0x401&&o->tiles[2]==0,"Current native map top/under/current-probe tiles observed without material reinterpretation");need(x.bytes==before,"Map observation preserves all RAM");x.w(at+0x24,0xFFFFFF00);const auto bad=p::observe(x.rdram,Fixture::actor,x.bytes.size());need(bad&&bad->tiles[0]==0x10000,"Overflowing tile-data address stays unknown");}
 {Fixture x;const auto before=x.bytes;for(unsigned i=0;i<100;++i)x.entry("func_805A6600");need(p::state.lines==64&&p::state.total_lines==64,"Diagnostic has a hard64-line point cap");for(unsigned kind:{0x4509u,0x450Au}){x.h(Fixture::actor+12,kind);for(unsigned i=0;i<100;++i)x.entry("func_805A6600");}need(p::state.points==3&&p::state.total_lines==192,"Diagnostic has a hard192-line overall cap");x.h(Fixture::actor+12,0x4508);x.entry("func_805A6600");need(p::state.total_lines==192,"Extra recreation cannot evade total cap");need(x.bytes==before,"Cap accounting changes no native byte");}
 if(argc==2){std::ifstream in(argv[1],std::ios::binary);std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)),{});need(bytes.size()==0x800000,"Actual aa4d final snapshot is8MiB");const auto before=bytes;const auto o=p::observe(bytes.data(),0x800CB598,bytes.size());need(o&&o->kind==0x4508&&o->queue==0&&o->primary==255&&o->metadata==0,"Actual fenced No2 is retired with no collision metadata");need(!o->catalogue_known&&bytes==before,"Captured8MiB neither invents PC catalogue nor changes captured data");}
 std::cout<<"BossCheckpointProbe0260Checks: "<<checks<<" PASS; read-only diagnostic scope/capacity only, no physics or GPU claim.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
