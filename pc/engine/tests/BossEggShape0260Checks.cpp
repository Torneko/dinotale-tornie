// Pure read-only shape guards. Synthetic9MiB catalogue row points to a copy of
// native4002 profile captured in85c finalRAM at8034A580 (14/16/20;zerooffsets).
#include "../boss_egg_shape0260.hpp"
#include <bit>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
namespace s=workshop::boss_egg_shape0260;
static unsigned checks=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
struct Fixture {
 static constexpr uint32_t player=0x800CA6B0,profile=0x8034A580;
 std::vector<uint8_t> mem=std::vector<uint8_t>(0x900000);
 void w(uint32_t p,uint32_t v){std::memcpy(mem.data()+p-s::ram_base,&v,4);}
 void h(uint32_t p,int16_t v){std::memcpy(mem.data()+((p-s::ram_base)^2),&v,2);}
 void b(uint32_t p,uint8_t v){mem[(p-s::ram_base)^3]=v;}
 void f(uint32_t p,float v){w(p,std::bit_cast<uint32_t>(v));}
 Fixture(){w(0x800F8828,player);w(player,0x19971110);h(player+12,0x4001);b(player+0x17D,2);b(player+0x18,1);w(s::egg_row,1);w(s::egg_row+4,profile);const int16_t native_profile[]={0,0,0,14,16,20,1,1,0,3847,0,0,64};for(unsigned i=0;i<13;++i)h(profile+2*i,native_profile[i]);for(unsigned i=0;i<3;++i)f(player+0x6C+4*i,.8f);f(s::launch_constant,.7f);}
 auto observe(){return s::observe(std::span<const uint8_t>(mem),player);}
};
int main(){try{
 {Fixture x;const auto before=x.mem;const auto got=x.observe();need(got&&got->row==0x80810018&&got->profile==Fixture::profile&&got->count==1&&got->player==Fixture::player,"Source-derived catalogue provenance retained");need(got->offsets==std::array<int16_t,3>{0,0,0}&&got->extents==std::array<int16_t,3>{14,16,20},"Captured native4002 centre/extents read directly");need(got->half_x==14.f*(.8f*.7f)&&got->half_y==16.f*(.8f*.7f)&&got->half_z==20.f*(.8f*.7f),"Shape matches actual native7.84/8.96/11.2 half-extents");need(x.mem==before,"All9MiB remain byte-identical");}
 {Fixture x;for(size_t size:{size_t(0),size_t(0x800000),s::required_capacity-1}){const auto before=x.mem;need(!s::observe(std::span<const uint8_t>(x.mem.data(),size),Fixture::player),"8MiB and truncated spans fail beforePCtable read");need(x.mem==before,"Truncated span rejection is read-only");}need(bool(s::observe(std::span<const uint8_t>(x.mem.data(),s::required_capacity),Fixture::player)),"Exact documentedPCtable capacity is sufficient");}
 for(unsigned fault=0;fault<25;++fault){Fixture x;uint32_t player=Fixture::player;switch(fault){case 0:player=0;break;case 1:++player;break;case 2:player=0x807FFFFC;break;case 3:x.w(0x800F8828,0);break;case 4:x.w(player,0);break;case 5:x.h(player+12,0x4005);break;case 6:x.b(player+6,3);break;case 7:x.b(player+0x17D,3);break;case 8:x.b(player+0x18,0xFF);break;case 9:x.w(s::egg_row,0);break;case 10:x.w(s::egg_row,4097);break;case 11:x.w(s::egg_row+4,0);break;case 12:x.w(s::egg_row+4,Fixture::profile+1);break;case 13:x.w(s::egg_row+4,0x807FFFFC);break;case 14:x.w(s::egg_row+4,0x80810000);break;case 15:x.h(Fixture::profile,1);break;case 16:x.h(Fixture::profile+2,-1);break;case 17:x.h(Fixture::profile+4,1);break;case 18:x.h(Fixture::profile+6,0);break;case 19:x.h(Fixture::profile+8,-16);break;case 20:x.h(Fixture::profile+10,257);break;case 21:x.f(s::launch_constant,0);break;case 22:x.f(s::launch_constant,std::numeric_limits<float>::quiet_NaN());break;case 23:x.f(s::launch_constant,std::numeric_limits<float>::infinity());break;case 24:x.f(s::launch_constant,4.1f);break;}const auto before=x.mem;need(!s::observe(std::span<const uint8_t>(x.mem),player),"Malformed identity/table/profile/constant fails closed");need(x.mem==before,"Rejected observation never mutates memory");}
 for(unsigned axis=0;axis<3;++axis)for(float bad:{0.f,-1.f,4.1f,std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity()}){Fixture x;x.f(Fixture::player+0x6C+axis*4,bad);const auto before=x.mem;need(!x.observe()&&x.mem==before,"Invalid per-axis player scale rejected read-only");}
 for(unsigned axis=0;axis<3;++axis){Fixture x;x.h(Fixture::profile+6+2*axis,256);x.f(Fixture::player+0x6C+4*axis,4.f);need(!x.observe(),"Overlarge final sweep dimensions rejected");}
 for(float scale:{.2f,.5f,.8f,1.f,1.5f,2.f}){Fixture x;for(unsigned i=0;i<3;++i)x.f(Fixture::player+0x6C+i*4,scale);auto v=x.observe();need(v&&v->half_x==14.f*(scale*.7f)&&v->half_y==16.f*(scale*.7f),"Finite variableP1 scale is observed instead of hardcoded14/16 or.56");}
 std::cout<<"BossEggShape0260 "<<checks<<" CPU checks passed; read-only bounded catalogue lookup, syntheticPCrow/native captured profile\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
