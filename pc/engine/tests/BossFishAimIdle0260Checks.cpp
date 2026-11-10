#include "../boss_fish_aim0260.hpp"
#include <fstream>
#include <iterator>
#include <iostream>
#include <vector>
#include <stdexcept>
namespace a=workshop::boss_fish_aim0260;namespace n=workshop::boss_native;
unsigned checks=0;void need(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
std::vector<uint8_t> read(const char* path){std::ifstream f(path,std::ios::binary);std::vector<uint8_t> out((std::istreambuf_iterator<char>(f)),{});need(out.size()==0x800000,"Untouched real8MiB capture required");return out;}
int main(int argc,char**argv){try{
 need(argc==3,"Supply Full5e52 and prior positive Directb23 real captures");auto full=read(argv[1]);const auto original=full;
 need(!workshop::boss_fish_green0260::updates[1]&&!workshop::boss_fish_green0260::updates[2],"Green physical override still excludes swimming/braking");need(a::updates[1]==0x805C7C2C&&a::updates[2]==0x805C8140,"Only exact native1/1 and1/2 observations added");
 for(unsigned s=0;s<a::updates.size();++s)if(s!=1&&s!=2)need(a::updates[s]==workshop::boss_fish_green0260::updates[s],"All original jump/passenger/cinematic callback slots preserved");
 for(uint32_t root:{0x800C9A44u,0x800C9CC0u,0x800C9F3Cu}){auto* rdram=full.data();need(n::actor(rdram,root,0x4511)&&n::byte(rdram,root+0x30)==1,"Actual Full source holds swimming4511");auto m=a::observe(rdram,root,0x4511,1);need(m&&m->actor==root&&m->speed==6.6f&&m->y==362.f,"Actual previously refused swimming fish now yields original box/motion/speed");need(full==original,"All8MiB stay byte-identical during actual observation");}
 const auto root=0x800C9A44u;auto* rdram=full.data();const auto factory=n::word(rdram,n::word(rdram,root+0x168)+0x40),p=n::word(rdram,root+0x1F4);MEM_B(0x30,n::base(root))=2;MEM_W(0x20,n::base(root))=factory+(0x805C8140-0x805C64F0);const auto braking=full;need(bool(a::observe(rdram,root,0x4511,1))&&full==braking,"Explicit HOST state2/currentcallback frontier reads braking without native changes");
 for(unsigned fault=0;fault<6;++fault){full=original;rdram=full.data();if(fault==0)MEM_W(0x20,n::base(root))=factory+0x1740;if(fault==1)MEM_W(0x40,n::base(p))=1;if(fault==2)MEM_B(0x30,n::base(root))=0;if(fault==3)MEM_W(0x180,n::base(root))=0x800CA6B0;if(fault==4)MEM_B(0x18,n::base(root))=0xA;if(fault==5)MEM_W(0x130,n::base(root))=0;const auto before=full;need(!a::observe(rdram,root,0x4511,1)&&full==before,"Wrong native callback/passenger/intro/parent/physics fails closed unchanged");}
 need(!a::observe(original.empty()?nullptr:const_cast<uint8_t*>(original.data()),root,0x4511,0),"No owned generation means no observation");
 auto direct=read(argv[2]);const auto before=direct;rdram=direct.data();need(n::actor(rdram,0x800C9A44,0x4511)&&n::byte(rdram,0x800C9A44+0x30)==15,"Prior actual Directpositive is free-jumpred15");const auto red=a::observe(rdram,0x800C9A44,0x4511,1),blue=a::observe(rdram,0x800C9CC0,0x4631,2);need(red&&blue&&red->vx==-2.f&&std::abs(red->vy+2.900002f)<.001f&&red->speed==6.6f,"Previously positive real Directmobile observation stays exact");need(direct==before,"Directpositive all8MiB also remains untouched");
 std::cout<<"BossFishAimIdle0260Checks: "<<checks<<" PASS; actual Full swimming and Direct mobile captures, HOST state2/fault frontiers only; no game/physics/victory claim.\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
