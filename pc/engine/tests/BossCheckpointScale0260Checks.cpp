// Reuse the existing exact-native inventory oracle; do not run its broad matrix.
#define main checkpoint_inventory_matrix_not_executed
#include "BossCheckpointInventory0260Checks.cpp"
#undef main
#include <fstream>
#include <iterator>
#include <limits>
static void source_scale(uint8_t* rdram,uint32_t egg,const cp::Scale& scale){for(unsigned axis=0;axis<3;++axis)word(rdram,egg+0x78+4*axis,scale[axis]);}
static cp::Scale native_scale(float x,float y,float z){return {std::bit_cast<uint32_t>(x),std::bit_cast<uint32_t>(y),std::bit_cast<uint32_t>(z)};}
int main(int argc,char** argv){
 std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();
 for(unsigned color=0;color<14;++color)for(unsigned count:{0u,1u,6u})for(unsigned milestone:{0u,3u,6u,9u}){
  setup(rdram,color,7,count,7,8);cp::Payload value;const auto before_capture=memory;
  check(cp::capture(rdram,player,100,value)&&memory==before_capture,"Capturing exact stable78 scales is read-only");
  for(unsigned i=0;i<count;++i)check(value.eggs[i].saved_scale==native_scale(.50f+.015625f*float(i),.625f+.015625f*float(i),.75f+.015625f*float(i)),"Each FIFO egg retains its own anisotropic saved scale bits");
  native_death_fixture(rdram);cp::Restore restore;auto c=context();const auto registers=c;
  check(restore.apply(rdram,&c,value,{player,100,200,300,1,milestone?83u:162u,milestone}),"Actual40860 recreates captured scale rather than the larger player78");
  check(!std::memcmp(&c,&registers,sizeof(c))&&cp::matches(rdram,value),"Exact scale, palette, order, HP and caller registers survive retry");
  cp::Inventory inventory;check(cp::read(rdram,player,inventory),"Recreated FIFO has exact native graph");
  for(unsigned i=0;i<count;++i)for(unsigned axis=0;axis<3;++axis){const auto egg=inventory.actors[i];check(cp::n::word(rdram,egg+0x6C+axis*4)==value.eggs[i].saved_scale[axis]&&cp::n::word(rdram,egg+0x78+axis*4)==value.eggs[i].saved_scale[axis],"Native40860 initially copies the same captured scale to6C and78");}
  if(count){const auto egg=inventory.actors[0];word(rdram,egg+0x6C,std::bit_cast<uint32_t>(.3f));check(cp::matches(rdram,value),"Animated current6C is not confused with stable saved78");word(rdram,egg+0x78,std::bit_cast<uint32_t>(.8f));check(!cp::matches(rdram,value),"A native readback with enlarged saved78 is rejected");}
  const auto once=memory;check(!restore.apply(rdram,&c,value,{player,100,200,300,1,milestone?83u:162u,milestone})&&memory==once,"Same retry cannot enlarge or recreate eggs again");
 }
 for(uint32_t bad:{0u,0x80000000u,0xBF800000u,0x7FC00000u,0x7F800000u,0x40A00000u})for(unsigned axis=0;axis<3;++axis){
  setup(rdram,10,5,1,7,8);cp::Payload value;check(cp::capture(rdram,player,100,value),"Malformed-scale fixture captures valid FIFO first");
  word(rdram,0x80100000+0x78+axis*4,bad);const auto invalid=memory;cp::Inventory inventory;check(!cp::read(rdram,player,inventory)&&!cp::capture(rdram,player,101,value)&&memory==invalid,"Zero/negative/nonfinite/oversized native saved scale refuses capture without writes");
  setup(rdram,10,5,1,7,8);check(cp::capture(rdram,player,100,value),"Payload scale rejection fixture capture");value.eggs[0].saved_scale[axis]=bad;native_death_fixture(rdram);const auto before=memory;cp::Restore restore;auto c=context();const auto registers=c;check(!restore.apply(rdram,&c,value,{player,100,200,300,1,83,3})&&memory==before&&!allocations&&!std::memcmp(&c,&registers,sizeof(c)),"Invalid stored scale is refused before any native mutation");
 }
 // Data-only source provenance from real8MiB captures. These reads do not
 // turn an unfenced snapshot into a death/transport or gameplay proof.
 check(argc>=3,"Supply normal natural-refill and legacy oversized diagnostic captures");
 for(int file=1;file<argc;++file){std::ifstream input(argv[file],std::ios::binary);std::vector<uint8_t> actual((std::istreambuf_iterator<char>(input)),{});check(actual.size()==0x800000,"Actual native capture is8MiB");auto* raw=actual.data();const auto owner=cp::n::word(raw,0x800F8828);cp::Payload captured;const auto untouched=actual;check(cp::capture(raw,owner,100,captured)&&actual==untouched,"Actual native FIFO scale/palette observation is read-only");cp::Scale expected;check(cp::normal_scale(raw,owner,expected)&&expected==native_scale(.8f*.7f,.8f*.7f,.8f*.7f),"Actual normal reserve birth is player0.8 times nativeRAM0.7");check(captured.count>0,"Actual comparison capture has reserves");
  for(unsigned i=0;i<captured.count;++i){check(captured.eggs[i].saved_scale==(file==1?expected:native_scale(.8f,.8f,.8f)),"Actual source capture distinguishes normal0.56 from legacy oversized0.8");}
  setup(rdram,captured.pc_color,captured.hp,captured.count,7,8);
  for(unsigned i=0;i<captured.count;++i)source_scale(rdram,0x80100000+i*0x400,captured.eggs[i].saved_scale);
  cp::Payload value;check(cp::capture(rdram,player,100,value),"Explicit host fixture carries the actual observed scale triples");native_death_fixture(rdram);cp::Restore restore;auto c=context();check(restore.apply(rdram,&c,value,{player,100,200,300,1,83,3})&&cp::matches(rdram,value),"Actual captured scale values survive the real native constructor/restore/readback");
 }
 std::printf("BossCheckpointScale0260Checks: %u checks PASS; real native40860 bodies, explicit allocator/death fixtures; actual captures are read-only provenance, noGPU proof\n",checks);return 0;
}
