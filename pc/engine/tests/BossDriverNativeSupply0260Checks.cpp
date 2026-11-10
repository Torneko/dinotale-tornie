#include "../boss_battle_runtime0250.hpp"
#include "../boss_driver_bridge0260.hpp"
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <vector>
namespace b=workshop::boss_native;
namespace d=workshop::boss_driver0260;
namespace bridge=workshop::boss_driver_bridge0260;
static unsigned checks=0;
static void need(bool yes,const char* why){++checks;if(!yes){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
 std::vector<uint8_t> memory=std::vector<uint8_t>(0x800000);uint8_t* rdram=memory.data();
 static constexpr uint32_t actor=0x80010000,private_data=0x80011000,metadata=0x80012000,definition=0x80013000;
 void word(uint32_t a,uint32_t v){MEM_W(0,b::base(a))=v;}
 void half(uint32_t a,uint16_t v){MEM_H(0,b::base(a))=v;}
 void byte(uint32_t a,uint8_t v){MEM_B(0,b::base(a))=v;}
 void floating(uint32_t a,float v){word(a,std::bit_cast<uint32_t>(v));}
 Fixture(uint16_t kind){word(actor,0x19971110);half(actor+0xC,kind);byte(actor+0x17D,2);byte(actor+0x18,1);byte(actor+0x30,0xB);word(actor+0x1F4,private_data);floating(actor+0x84,88);floating(actor+0x88,376);half(private_data,0x4528);half(private_data+2,1);half(private_data+4,0);word(actor+0x174,metadata);word(metadata,definition);half(definition+0x1C,0xC1);}
};
int main(){
 for(uint16_t kind:{uint16_t(0x4526),uint16_t(0x46DA)}){
  Fixture f(kind);const auto before=f.memory;need(bridge::egg_food_flags(f.rdram,Fixture::actor)==0xC1,"real native edible definition preserves egg-yield flag");need(f.memory==before,"native food observation never writes RAM");
  for(uint16_t flags:{uint16_t(0),uint16_t(0xC0),uint16_t(2),uint16_t(0xFFFE)}){f.half(Fixture::definition+0x1C,flags);const auto immutable=f.memory;d::Supply source{Fixture::actor,kind,150,350,true,false,bridge::egg_food_flags(f.rdram,Fixture::actor)};need(!d::supply_valid(source)&&f.memory==immutable,"native non-egg food cannot become ammunition");}
 }
 for(unsigned fault=0;fault<8;++fault){Fixture f(0x4526);if(fault==0)f.half(Fixture::actor+0xC,0x41DD);if(fault==1)f.byte(Fixture::actor+6,3);if(fault==2)f.byte(Fixture::actor+0x17D,0);if(fault==3)f.byte(Fixture::actor+0x18,0xFF);if(fault==4)f.word(Fixture::actor+0x174,0);if(fault==5)f.word(Fixture::metadata,0x807FFFF0);if(fault==6)f.word(Fixture::actor,0);if(fault==7)f.word(Fixture::metadata,0);const auto before=f.memory;need(!bridge::egg_food_flags(f.rdram,Fixture::actor)&&f.memory==before,"wrong bird identity/disposed/header/definition rejected read-only");}
 for(uint16_t flags:{uint16_t(0),uint16_t(1),uint16_t(2),uint16_t(3)})for(unsigned sub:{0xAu,0xBu}){
  Fixture f(0x456E);f.half(Fixture::private_data+2,flags);f.byte(Fixture::actor+0x30,uint8_t(sub));f.word(Fixture::actor+0x174,0);const auto before=f.memory;d::Supply source;need(bridge::observe_feeder(f.rdram,Fixture::actor,449,source)&&source.feeder_flags==flags&&source.feeder_kind==0x4528&&d::supply_valid(source),"native invisible456E is a typed non-edible feeder in either active state");need(f.memory==before,"feeder observation does not mutate native actor/private data");
 }
 for(unsigned fault=0;fault<14;++fault){Fixture f(0x456E);float player_y=449;d::Supply source{0x80100000,0x4002,1,2,true,true};const auto unchanged=source;
  if(fault==0)f.half(Fixture::actor+0xC,0x4528);if(fault==1)f.byte(Fixture::actor+6,3);if(fault==2)f.byte(Fixture::actor+0x17D,1);if(fault==3)f.byte(Fixture::actor+0x18,0);if(fault==4)f.byte(Fixture::actor+0x30,5);if(fault==5)f.word(Fixture::actor+0x1F4,0);if(fault==6)f.half(Fixture::private_data,0x46DB);if(fault==7)f.half(Fixture::private_data+4,1);if(fault==8)player_y=504;if(fault==9)player_y=std::numeric_limits<float>::quiet_NaN();if(fault==10)f.floating(Fixture::actor+0x84,std::numeric_limits<float>::infinity());if(fault==11)f.floating(Fixture::actor+0x88,std::numeric_limits<float>::quiet_NaN());if(fault==12)f.word(Fixture::actor,0);if(fault==13)f.word(Fixture::actor+0x1F4,0x807FFFFC);
  const auto before=f.memory;need(!bridge::observe_feeder(f.rdram,Fixture::actor,player_y,source)&&f.memory==before,"wrong native generator provenance or perpendicular distance fails read-only");need(source.actor==unchanged.actor&&source.kind==unchanged.kind&&source.x==unchanged.x&&source.y==unchanged.y&&source.collectible==unchanged.collectible,"failed native observation preserves previous output object");
 }
 for(uint16_t kind:{uint16_t(0x410A),uint16_t(0x469E)}){
  Fixture f(0x4001);constexpr uint32_t spring=0x80014000;f.word(0x800F8828,Fixture::actor);f.word(Fixture::actor+0x180,spring);f.word(spring,0x19971110);f.half(spring+0xC,kind);f.byte(spring+0x17D,2);f.byte(Fixture::actor+0x17D,0);f.half(Fixture::actor+0x17E,3);f.half(spring+0x184,1);f.word(spring+0x188+3*4,Fixture::actor);const auto before=f.memory;need(bridge::observe_spring_attachment(f.rdram,Fixture::actor)&&f.memory==before,"only real global P1 attached to live native spring is observed without RAM writes");
 }
 for(unsigned fault=0;fault<13;++fault){
  Fixture f(0x4001);constexpr uint32_t spring=0x80014000;f.word(0x800F8828,Fixture::actor);f.word(Fixture::actor+0x180,spring);f.word(spring,0x19971110);f.half(spring+0xC,0x469E);f.byte(spring+0x17D,2);f.byte(Fixture::actor+0x17D,0);f.half(Fixture::actor+0x17E,3);f.half(spring+0x184,1);f.word(spring+0x188+3*4,Fixture::actor);
  if(fault==0)f.word(0x800F8828,0);if(fault==1)f.half(Fixture::actor+0xC,0x4002);if(fault==2)f.word(Fixture::actor,0);if(fault==3)f.byte(Fixture::actor+6,3);if(fault==4)f.byte(Fixture::actor+0x17D,2);if(fault==5)f.word(Fixture::actor+0x180,0);if(fault==6)f.half(spring+0xC,0x407D);if(fault==7)f.word(spring,0);if(fault==8)f.byte(spring+6,3);if(fault==9)f.byte(spring+0x17D,1);if(fault==10)f.byte(spring+0x17D,3);if(fault==11)f.word(Fixture::actor+0x180,0x807FFFF0);if(fault==12)f.word(0x800F8828,spring);
  const auto before=f.memory;need(!bridge::observe_spring_attachment(f.rdram,Fixture::actor)&&f.memory==before,"wrong player/parent kind, lifetime, queuephase or global identity cannot request spring controls");
 }
 need(!bridge::observe_spring_attachment(nullptr,Fixture::actor),"null spring RAM rejected");
 need(!bridge::egg_food_flags(nullptr,Fixture::actor),"null food RAM rejected");d::Supply source;need(!bridge::observe_feeder(nullptr,Fixture::actor,449,source),"null generator RAM rejected");
 std::printf("BossDriverNativeSupply0260Checks: %u PASS (read-only8MiB native ABI fixtures; no gameplay victory claim)\n",checks);
}
