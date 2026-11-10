#include "../boss_battle_runtime0250.hpp"
#include "../boss_driver_bridge0260.hpp"
#include <fstream>
#include <iterator>
#include <vector>
#include <cstdio>
#include <cstdlib>
namespace n=workshop::boss_native;namespace m=workshop::boss_monster_aim0260;namespace b=workshop::boss_monster_burst0260;namespace bridge=workshop::boss_driver_bridge0260;namespace d=workshop::boss_driver0260;
static unsigned checks=0;static void need(bool value,const char* why){++checks;if(!value){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
int main(int argc,char** argv){
 need(argc==2,"Supply the unmodified3371/3000 fighting snapshot");std::ifstream file(argv[1],std::ios::binary);std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),{});need(bytes.size()==0x800000,"The actual capture has exactly8MiB");const auto saved=bytes;auto* rdram=bytes.data();
 const auto map=m::Map::read(rdram);need(map&&map->columns==3&&map->rows==2&&map->stride==96,"Actual native151 map provenance is3x2 with its real96-column stride");
 const auto player=n::word(rdram,0x800F8828),data=n::word(rdram,player+0x1F4);need(n::actor(rdram,player,0x4001)&&n::range(data,0x12D0),"Actual same-player graph is typed and bounded");
 b::Scene scene;scene.map=*map;scene.x=m::scalar(rdram,player+0x84);scene.y=m::scalar(rdram,player+0x88);scene.z=m::scalar(rdram,player+0x8C)+m::scalar(rdram,0x800B02E0);need(scene.x==312&&scene.y>349&&scene.y<351,"Snapshot is the actual grounded fighting origin, not post-retry RAM");
 const unsigned head=MEM_HU(0x12BE,n::base(data));need(head<8&&n::byte(rdram,data+0x12C8)==6,"Capture contains the real full native FIFO");const auto reserve=n::word(rdram,data+0x11FC+24*head);need(n::actor(rdram,reserve,0x4004)&&n::word(rdram,reserve+0x180)==player,"Launch scale comes from the actual selected reserve");
 // The8MiB capture does not include the extended profile catalogue. These
 // extents are the separately source-proved native4002 profile, combined
 // with the REAL captured FIFO scale. No profile is injected into the game.
 scene.egg.half_x=14*m::scalar(rdram,reserve+0x78);scene.egg.half_y=16*m::scalar(rdram,reserve+0x7C);scene.egg.half_z=20*m::scalar(rdram,reserve+0x80);
 need(std::abs(scene.egg.half_x-11.2f)<.001f&&std::abs(scene.egg.half_y-12.8f)<.001f,"Captured reserve scale gives the actual11.2/12.8 projectile extents");
 const auto callback=n::word(rdram,0x800F8D18);need(n::range(callback-0x7C0,0x914),"Actual native3C10 callback locates the loaded burst parameter table");const float scale=m::scalar(rdram,callback-0x7C0+0x90C);need(scale==1.5f,"Real native parameter1 collision scale is loaded before first puff");scene.burst_x=scene.burst_y=24*scale;scene.burst_z=20*scale;scene.my=8;
 for(uint32_t root:{0x800C9F3Cu,0x800CA1B8u}){const auto p=n::word(rdram,root+0x1F4);unsigned count=0;for(unsigned slot=0;slot<20;++slot){m::Eye e;if(!m::observe_eye(rdram,root,p,1,n::word(rdram,root+0x188+4*slot),e))continue;scene.eyes[scene.count++]={e.actor,root,e.x,e.y,m::scalar(rdram,e.actor+0x8C),8,8,20,0,0};++count;}need(count==2,"Both actual native41C6 roots retain their two exact46F9 eye graphs");}
 need(scene.count==4,"All four actual eye colliders participate in the composed shot gate");
 // HOST input boundary: use the actual geometry, with explicit stationary
 // motion fixtures and a prospective native4003 point. This does not claim
 // that the captured game was aiming, nor that any CPU fixture fired an egg.
 const auto& eye=scene.eyes[0];d::Target target{eye.root,0x41C6,eye.x,eye.y,0,0,true,true,false,false,true,6.6f,2};d::Input input;input.x=scene.x;input.y=scene.y;input.aiming_actor=1;input.reticle_actor=2;
 const float dx=eye.x-input.x,dy=eye.y-input.y,length=std::hypot(dx,dy),gap=(12.8f+8+36+8)*.5f;input.reticle_x=input.x+dx*(length-gap)/length;input.reticle_y=input.y+dy*(length-gap)/length;
 b::Result result;bridge::plan_monster_burst(rdram,scene,input,target,result);
 need(target.clear_path,"The REAL Bridge ideal-corridor call admits the true3x2 arena");need(result.observed&&result.ready&&target.monster_burst_ready,"The REAL Bridge current-reticle corridor call also admits the true3x2 arena");need(!result.passed,"Composed hypothetical short reticle does not directly contact any captured eye");
 need(!workshop::boss_lips_corridor0260::observe(rdram,*map,input.x,input.y,input.reticle_x,input.reticle_y,11.2f,12.8f),"The unchanged Lips-only helper still rejects this foreign3x2 map, reproducing the original integration failure");
 for(unsigned fault=0;fault<5;++fault){auto bad=scene;if(fault==0)bad.map.columns=2;if(fault==1)bad.map.rows=1;if(fault==2)bad.map.columns=4;if(fault==3)bad.map.pages=0;if(fault==4)bad.map.data=0;
  const auto math=b::evaluate(bad,target.actor,input.reticle_x,input.reticle_y);need(math.ready,"The explicit burst geometry itself remains valid in a bad-map fixture");bridge::plan_monster_burst(rdram,bad,input,target,result);need(!target.clear_path&&!target.monster_burst_ready&&!result.ready,"Both actual Bridge corridor calls fail closed on wrong-domain or untrusted map inputs");}
 for(auto end:std::array<std::array<float,2>,4>{{{-20,100},{760,200},{100,-20},{100,510}}})need(!b::corridor(rdram,*map,scene.x,scene.y,end[0],end[1],11.2f,12.8f),"Complete egg volume must stay within the native768x512 domain");
 need(saved==bytes,"Both actual Bridge calls and all refusals preserve every byte of the captured8MiB");
 std::printf("BossMonsterCorridor0260Checks: %u PASS (actual3371/3000 map and Bridge adapter; explicit HOST motion/reticle fixtures, no native shot/victory claim)\n",checks);
}
