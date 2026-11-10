#include "../custom_editor_objects.hpp"
#include "../gold_silver_resources0300.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <cstring>
namespace eo=workshop::editor_objects;
namespace wt=workshop::terrain;
namespace {
unsigned checks=0,sound_banks=0,sounds=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
std::vector<uint8_t> read(const char* name){std::ifstream f(name,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
uint32_t bits(float x){return std::bit_cast<uint32_t>(x);}
size_t ro(const std::vector<uint8_t>& image,unsigned id){for(unsigned n=0;n<1582;n++){auto row=0xA1C90+n*12;if(wt::be16(image,row)==id)return wt::bank(image,wt::be32(image,row+4));}throw std::runtime_error("Missing source resource");}
uint32_t word(uint8_t* rdram,uint32_t address){return uint32_t(MEM_W(0,int32_t(address)));}
void word(uint8_t* rdram,uint32_t address,uint32_t v){MEM_W(0,int32_t(address))=v;}
void half(uint8_t* rdram,uint32_t address,uint16_t v){MEM_H(0,int32_t(address))=v;}
void byte(uint8_t* rdram,uint32_t address,uint8_t v){MEM_B(0,int32_t(address))=v;}
uint16_t half(uint8_t* rdram,uint32_t address){return MEM_HU(0,int32_t(address));}
std::vector<uint8_t> payload(){
 std::vector<uint8_t> p(16);std::copy(workshop::magic.begin(),workshop::magic.end(),p.begin());p[7]='6';wt::word(p,8,46);wt::word(p,12,2);
 auto room=[&](unsigned source,unsigned count){auto at=p.size();p.resize(at+32);wt::word(p,at,source);wt::word(p,at+4,count);wt::word(p,at+12,0xffffffff);};
 auto item=[&](unsigned id,unsigned layer,float x,float y){auto at=p.size();p.resize(at+16);wt::half(p,at,uint16_t(id));wt::half(p,at+2,uint16_t(layer));wt::word(p,at+4,bits(x));wt::word(p,at+8,bits(y));p[at+12]=255;};
 room(46,1);item(0x4001,0,16,16);room(120,11);item(0x4001,0,16,16);
 for(unsigned c=6;c<16;c++)item(0x4f30+c-6,c&1,float(48+(c-6)*64),176);
 workshop::validate_payload(p);return p;
}
constexpr uint32_t table=0x80100000,resource=0x80102000,texture=0x80103000,wrapper_resource=0x80104000,wrapper_palette=0x80105000,wrapper_texture=0x80106000,drawing=0x80107000,palette=0x80200000;
void setup(uint8_t* ram,const std::vector<uint8_t>& palettes){
 word(ram,0x800f84d4,table);byte(ram,table,3);word(ram,table+4,0x1234);word(ram,table+8,resource);word(ram,table+12,wrapper_resource);
 half(ram,resource+0x3c,0x401d);word(ram,resource+0x30,0);word(ram,resource+0x84,texture);word(ram,resource+0x88,palette);
 half(ram,wrapper_resource+0x3c,0x41ce);word(ram,wrapper_resource+0x30,0);word(ram,wrapper_resource+0x84,wrapper_texture);word(ram,wrapper_resource+0x88,wrapper_palette);
 for(unsigned i=0;i<palettes.size();i++)byte(ram,palette+i,palettes[i]);
 word(ram,0x800f8df4,0x12345678);
}
void actor(uint8_t* ram,uint32_t at,unsigned id,bool wrapper){
 word(ram,at,0x19971110);half(ram,at+12,uint16_t(id));byte(ram,at+0x164,0);
 word(ram,at+0x168,wrapper?wrapper_resource:resource);word(ram,at+0x16c,drawing);
 word(ram,drawing+8,wrapper?wrapper_palette:palette);word(ram,drawing+0x30,wrapper?wrapper_texture:texture);word(ram,drawing+0x40,(wrapper?wrapper_resource:resource)+0x74);word(ram,drawing+0x38,0);word(ram,drawing+0x3c,0);
}
template<class F>void rejects(F action){bool rejected=false;try{action();}catch(const std::runtime_error&){rejected=true;}check(rejected,"Invalid placement accepted");}
}
extern "C" void func_80067E68(uint8_t*,recomp_context* ctx){check(ctx->r4==6,"Wrong pickup sound bank");++sound_banks;ctx->r2=0x10203040;ctx->r8=0xcccc;}
extern "C" void func_80067818(uint8_t*,recomp_context* ctx){check(ctx->r4==0x251&&ctx->r5==0x10203040,"Wrong pickup sound cue");++sounds;ctx->r2=0xdddd;ctx->r9=0xeeee;}
int main(int argc,char** argv){try{
 check(argc==3,"Expected original ROM and content ROM inputs");auto original=read(argv[1]),content=read(argv[2]);check(original.size()==0x1000000&&content.size()==0x1000000,"Input ROM length");
 const auto packed=workshop::gold_silver_resources0300::apply(content);const auto egg=ro(packed,0x4002);const auto palettes=wt::block(packed,wt::be32(packed,egg+0x30));check(palettes.size()==16*512,"Real packed egg palettes must contain16 colors");
 std::set<std::vector<uint8_t>> distinct;for(unsigned c=0;c<16;c++)distinct.emplace(palettes.begin()+c*512,palettes.begin()+(c+1)*512);check(distinct.size()==16,"Real egg colors not distinct");
 for(unsigned id=0;id<65536;id++)check(eo::special(id)==(id>=0x4f30&&id<=0x4f39),"Alias bounds changed");
 check(eo::native(0x4692)&&eo::native(0x4693)&&!eo::native(0x4691)&&!eo::native(0x4694),"Native bubble alias changed");
 for(unsigned c=6;c<16;c++)check(eo::color(0x4f30+c-6)==c,"Old/metal alias mapping changed");
 const auto file=payload();eo::configure(original,file);check(eo::placements.size()==10,"Ten editor egg bindings required");const auto alias=eo::aliases.at(120);check(eo::native(alias),"Source-compatible alias not native");
 for(unsigned c=6;c<16;c++){auto input=(0x4f30+c-6)<<16|(c&1);check(eo::packed(120,input)==((uint32_t(alias)<<16)|(c&1)),"Packed native bubble kind/layer changed");}
 check(eo::packed(120,0x401d1234)==0x401d1234,"Unrelated packed kind mutated");rejects([&]{(void)eo::packed(43,0x4f380001);});
 auto overlap=file;auto rooms=workshop::payload_rooms(file);auto at=rooms[1].offset;std::copy_n(overlap.begin()+at+16,12,overlap.begin()+at+32);rejects([&]{workshop::validate_payload(overlap);});

 // Exercise both compatible native aliases with real room metadata, including
 // Gold and Silver in secondary rooms of their actual vanilla level.
 unsigned native_aliases=0;
 for(unsigned room=0;room<177;room++){
  unsigned parent=177;for(auto root:workshop::vanilla_sources)if(workshop::vanilla_room(root,room))parent=root;
  if(parent==177||room==parent)continue;
  unsigned found=65535;
  try{auto h=wt::bank(original,workshop::u32(original,0xa84a0+room*8+4));auto m=wt::bank(original,workshop::u32(original,wt::bank(original,workshop::u32(original,h+0x5c))));auto objects=wt::bank(original,workshop::u32(original,m+4));for(unsigned n=0;n<workshop::u16(original,m);n++)if(eo::native(workshop::u16(original,objects+n*12)))found=std::min(found,unsigned(workshop::u16(original,objects+n*12)));}catch(const std::runtime_error&){continue;}
  if(found==65535)continue;
  auto two=file;wt::word(two,8,parent);wt::word(two,16,parent);wt::word(two,64,room);workshop::validate_payload(two);eo::configure(original,two);check(eo::aliases.at(room)==found,"Native alias resolution differs from real source room");
  check(eo::packed(room,0x4f380001)==((found<<16)|1)&&eo::packed(room,0x4f390000)==(found<<16),"Metal alias not mapped through source-native bubble");native_aliases|=found==0x4692?1u:2u;
 }
 check(native_aliases==3,"Both native4692/4693 sources must be exercised");
 // Full wrapper-to-carried lifecycle, actual methods on private native-layout RAM.
 for(unsigned c=6;c<16;c++){
  eo::configure(original,file);eo::enter_room(120);std::vector<uint8_t> memory(0x800000);auto ram=memory.data();setup(ram,palettes);check(eo::native_egg_resource(ram)==resource,"Native resource table lookup failed");
  recomp_context ctx{};ctx.r29=S32(0x80700000);uint32_t source=0x80500000,child=0x80510000,carried=0x80520000;
  ctx.r4=S32(source);ctx.r5=(uint32_t(alias)<<16)|(c&1);ctx.r6=bits(float(48+(c-6)*64));ctx.r7=bits(176);eo::created(ram,&ctx,120);check(eo::source_color(source)==c,"Source registry lost egg color");
  ctx.r4=S32(source);ctx.r5=0x41ce;eo::begin_child(&ctx);check(eo::creating_parent==source,"Exact wrapper parent missing");
  ctx.r4=S32(child);ctx.r5=0x41ce0000;eo::created(ram,&ctx,120);check(eo::children.at(child).color==c&&eo::creating_parent==0,"Child did not inherit source");
  actor(ram,child,0x41ce,true);word(ram,child+0x1f4,0x80530000);half(ram,0x80530000,0x401c);
  ctx.r7=S32(child);check(eo::release_color(ram,&ctx)==c,"Exact release binding missing");ctx.r7=S32(child+0x100);check(!eo::release_color(ram,&ctx),"Neighbor inferred as custom egg");
  eo::prepare_release(ram,child);check(half(ram,0x80530000)==0x401d,"Release must use401D native resource");
  const auto before=memory;check(!eo::paint(ram,child),"Wrapper must not become a carried egg");check(word(ram,drawing+8)==palette+c*512&&word(ram,drawing+0x30)==texture&&word(ram,drawing+0x40)==resource+0x74,"Palette/texture/geometry binding wrong");
  check(word(ram,child+0x168)==wrapper_resource&&half(ram,child+12)==0x41ce,"Wrapper native resource or actor identity changed");
  for(unsigned i=0;i<512;i++)check(ram[((word(ram,drawing+8)+i)&0x7fffff)^3]==palettes[c*512+i],"Painted palette differs from real content");
  for(size_t i=0;i<memory.size();i++)if(memory[i]!=before[i]){auto logical=uint32_t(i)^3;check((logical>=((drawing+8)&0x7fffff)&&logical<((drawing+12)&0x7fffff))||logical==((drawing+4)&0x7fffff)||(logical>=((drawing+0x30)&0x7fffff)&&logical<((drawing+0x34)&0x7fffff))||(logical>=((drawing+0x40)&0x7fffff)&&logical<((drawing+0x44)&0x7fffff)),"Paint changed bytes beyond borrowed drawing references");}
  eo::restore(ram,child);check(word(ram,drawing+8)==wrapper_palette&&word(ram,drawing+0x30)==wrapper_texture&&word(ram,drawing+0x40)==wrapper_resource+0x74,"Borrowed descriptor not restored");check(eo::children.contains(child)&&!eo::painted.contains(child),"Render-only restore destroyed live wrapper binding");
  (void)eo::paint(ram,child);eo::dispose(ram,child);check(!eo::children.contains(child)&&word(ram,drawing+8)==wrapper_palette,"Full disposal did not restore/forget wrapper");
  eo::spawning_color=int(c);ctx.r4=S32(carried);ctx.r5=0x401d0000;eo::created(ram,&ctx,120);eo::spawning_color=-1;actor(ram,carried,0x401d,false);check(eo::egg_state(carried,0)&&eo::native_egg_actors.contains(carried),"Spawned carried egg not tracked");check(eo::paint(ram,carried)&&word(ram,drawing+8)==palette+c*512,"Carried egg palette wrong");
  auto saved=ctx;const auto ram_before_sound=memory;check(!eo::pickup_sound(ram,&ctx,carried,3),"Early pickup state sounded");check(eo::pickup_sound(ram,&ctx,carried,4),"Pickup sound missing");check(!eo::pickup_sound(ram,&ctx,carried,5),"Pickup sound repeated");check(std::memcmp(&ctx,&saved,sizeof ctx)==0&&memory==ram_before_sound,"Sound boundary modified caller or canonical RAM");check(word(ram,0x800f8df4)==0x12345678,"Canonical unlock flags touched");
  eo::creating_parent=carried;eo::forget(carried);check(!eo::eggs.contains(carried)&&!eo::native_egg_actors.contains(carried)&&eo::creating_parent==0,"Forget left stale carried state");
  eo::spawned(carried,c);ctx.r4=S32(carried);ctx.r5=0x40010000;eo::created(ram,&ctx,120);check(!eo::egg_state(carried,4),"Reused actor inherited previous egg");
  eo::enter_room(121);check(!eo::source_color(source),"Other room leaked source binding");
 }
 check(sound_banks==10&&sounds==10,"Pickup cue count differs across ten colors");
 for(unsigned c=0;c<18;c++){
  eo::eggs.clear();eo::spawned(0x80500000,c);check(eo::eggs.contains(0x80500000)==(c>=6&&c<16),"Direct spawn color bounds changed");eo::eggs.clear();eo::spawned(0,c);check(eo::eggs.empty(),"Null actor registered");
  std::vector<uint8_t> memory(0x800000);recomp_context ctx{};ctx.r4=S32(0x80500000);ctx.r5=0x401d0000;eo::spawning_color=int(c);eo::created(memory.data(),&ctx,120);check(eo::eggs.contains(0x80500000)==(c>=6&&c<16),"Native-created color bounds changed");
 }
 // Native cache lookup must reject old/truncated palette arrays for metal use.
 std::vector<uint8_t> memory(0x800000);auto ram=memory.data();setup(ram,palettes);word(ram,resource+0x88,0x80800000-14*512);check(eo::native_egg_resource(ram)==0,"Truncated14 palette block accepted");word(ram,resource+0x88,0x80800000-16*512);check(eo::native_egg_resource(ram)==resource,"Exactly16 palettes at RAM end rejected");word(ram,resource+0x84,0x807ffff0);check(eo::native_egg_resource(ram)==0,"Truncated texture descriptor accepted");word(ram,resource+0x84,texture);half(ram,resource+0x3c,0x401c);check(eo::native_egg_resource(ram)==0,"Wrong native resource kind accepted");half(ram,resource+0x3c,0x401d);word(ram,resource+0x30,1);check(eo::native_egg_resource(ram)==0,"Unsupported resource format accepted");word(ram,0x800f84d4,0x807ffffe);check(eo::native_egg_resource(ram)==0,"Truncated cache table accepted");

 for(unsigned c:{14u,15u}){
  std::vector<uint8_t> guard(0x800000);auto data=guard.data();setup(data,palettes);eo::eggs.clear();
  for(uint32_t bad:{0x800003ffu,0x807fffffu}){eo::spawned(bad,c);auto before=guard;check(!eo::paint(data,bad)&&guard==before,"Invalid actor RAM range mutated drawing");eo::forget(bad);}
  const uint32_t test=0x80500000;eo::spawned(test,c);actor(data,test,0x401d,false);
  word(data,test,0);auto before=guard;check(!eo::paint(data,test)&&guard==before,"Invalid actor magic accepted");word(data,test,0x19971110);
  byte(data,test+0x164,2);before=guard;check(!eo::paint(data,test)&&guard==before,"Unsupported render type accepted");byte(data,test+0x164,0);
  word(data,drawing+0x38,1);before=guard;check(!eo::paint(data,test)&&guard==before,"Privately owned drawing references overwritten");word(data,drawing+0x38,0);
 }
 eo::configure({},{});check(eo::placements.empty()&&eo::aliases.empty()&&eo::sources.empty()&&eo::children.empty()&&eo::eggs.empty()&&eo::current_room==177&&eo::spawning_color==-1,"Configuration reset not complete");
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"actualMethods\":true,\"colorsCovered\":16,\"specialEggLifecycles\":10,\"pickupSounds\":"<<sounds<<",\"realPackedEggPalettes\":16,\"nativeBodiesExecuted\":0,\"gameStarted\":false,\"gpuUsed\":false,\"canonicalUnlockGranted\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
