#include "../boss_battle_runtime0250.hpp"
#include "../boss_driver_bridge0260.hpp"
#include <bit>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
namespace c=workshop::boss_campaign0250;namespace b=workshop::boss_battle0250;namespace v=workshop::boss_variants0250;namespace m=workshop::boss_monster_aim0260;namespace n=workshop::boss_native;
static unsigned checks=0;
static void need(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
static std::vector<uint8_t> load(const char* path){std::ifstream f(path,std::ios::binary);std::vector<uint8_t> r((std::istreambuf_iterator<char>(f)),{});need(r.size()==0x800000,"Actual immutable8MiB source capture");return r;}
static void placement(unsigned index,b::Family family,unsigned source,float left,float right,float y,std::vector<uint8_t>& captured){
 const auto original=captured;auto* rdram=captured.data();const auto map=m::Map::read(rdram);need(map&&map->columns==3&&map->rows==(source==163?3u:2u),"Actual native bitmap belongs to the intended source arena");
 need(c::configure("BirdDeliveryChecks",1,true,false,int(index)),"Existing diagnostic encounter setup");c::rooms::configure(c::session.plan());c::physical_room=c::source_for(index);c::prepare_encounter();
 need(!c::failed&&c::session.current().family==family&&c::logical_room==source,"Actual Runtime selects the intended native family/source");need(c::session.current().birds==2&&v::snapshot().enabled&&!v::snapshot().invalidated,"Reference quota reaches the real Runtime placement path");
 for(unsigned slot=0;slot<2;++slot){const auto& bird=v::birds[slot];const float x=std::bit_cast<float>(bird.position[0]),at_y=std::bit_cast<float>(bird.position[1]),z=std::bit_cast<float>(bird.position[2]);need(bird.positioned&&bird.option_count==1&&!bird.actor&&!bird.pending,"Setpoints prepare native births without fabricating an actor or inventory");need(x==(slot?right:left)&&at_y==y&&z==500.34f,"Runtime publishes the declared reachable source positions");bool clear=true;for(int xx=int(x)-12;xx<=int(x)+12;++xx)for(int yy=int(y)-32;yy<=int(y)+32;++yy){const auto solid=map->occupied(rdram,float(xx),float(yy));if(!solid||*solid)clear=false;}need(clear,"Actual bitmap leaves the chosen point and12x32 margin clear; not a flight simulation");if(source==163)need(x>=256&&x<=415&&673.f-y>=0&&673.f-y<=80,"Genie sources are over the proved continuous support within the existing forage height window");}
 need(captured==original,"Source terrain and all native memory stay immutable");
}
int main(int argc,char** argv){try{
 need(argc==4,"Supply actual Genie f706, Invisible58A8 and finalCandy F60D captures");auto genie=load(argv[1]),invisible=load(argv[2]),food=load(argv[3]);
 placement(2,b::Family::Genie,163,304,368,624,genie);placement(5,b::Family::Invisible,164,300,348,280,invisible);
 const auto original=food;auto* rdram=food.data();constexpr uint32_t bird=0x800C8B5C;need(n::actor(rdram,bird,0x4526)&&n::byte(rdram,bird+0x17D)==2,"Actual F60D contains the ordinary native yellow bird");const auto flags=workshop::boss_driver_bridge0260::egg_food_flags(rdram,bird);need(flags==0xC1,"Actual native bird advertises the ordinary edible/egg flag");workshop::boss_driver0260::Supply source{bird,0x4526,304,624,true,false,flags};need(workshop::boss_driver0260::edible_supply(source)&&workshop::boss_driver0260::supply_valid(source),"Existing pilot accepts these native food flags without a Driver change");need(food==original,"Food observation makes no guest-memory write");
 std::cout<<"BossBirdDelivery0260Checks: "<<checks<<" PASS; actual Runtime placement/bitmap margins and native4526 food flags. No allocation, ingestion, inventory, motion or victory is simulated.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
