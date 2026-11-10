// Private CPU-only renderer: no game/UI/save/runtime initialization is linked.
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vector>
#include "funcs.h"
#include "menu_music_native0310.hpp"
#include "native_identity.inc"
#include <array>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include "librecomp/rsp.hpp"

namespace {
std::vector<uint8_t> rom;
std::string normalized_rom_hash;
using namespace dinotale::menu_music_native0310;
std::string sha256(const std::vector<uint8_t>& input) {
 const uint32_t k[64]={0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
 auto r=[](uint32_t x,unsigned n){return (x>>n)|(x<<(32-n));};
 uint32_t h[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
 auto data=input;uint64_t bits=uint64_t(data.size())*8;data.push_back(0x80);while(data.size()%64!=56)data.push_back(0);for(int i=7;i>=0;i--)data.push_back(uint8_t(bits>>(8*i)));
 for(size_t p=0;p<data.size();p+=64){uint32_t w[64];for(unsigned i=0;i<16;i++)w[i]=(uint32_t(data[p+i*4])<<24)|(uint32_t(data[p+i*4+1])<<16)|(uint32_t(data[p+i*4+2])<<8)|data[p+i*4+3];for(unsigned i=16;i<64;i++){auto s0=r(w[i-15],7)^r(w[i-15],18)^(w[i-15]>>3);auto s1=r(w[i-2],17)^r(w[i-2],19)^(w[i-2]>>10);w[i]=w[i-16]+s0+w[i-7]+s1;}uint32_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],v=h[7];for(unsigned i=0;i<64;i++){auto s1=r(e,6)^r(e,11)^r(e,25),ch=(e&f)^(~e&g),t1=v+s1+ch+k[i]+w[i],s0=r(a,2)^r(a,13)^r(a,22),maj=(a&b)^(a&c)^(b&c),t2=s0+maj;v=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;}h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=v;}
 std::ostringstream out;out<<std::hex<<std::setfill('0');for(auto x:h)out<<std::setw(8)<<x;return out.str();
}

std::vector<int16_t> pcm;
std::map<uint32_t,uint64_t> calls;
uint32_t tick=0,rate=0,ai_remaining=0;
uint64_t dma_calls=0,rsp_tasks=0,ai_calls=0;
uint32_t last_function=0;
struct Mark {uint32_t tick,buffer,subindex,offset=0;uint64_t frame=~0ull;uint32_t script,pc,opcode,arg;};
std::vector<Mark> marks;
uint32_t current_buffer=0,current_subindex=0,current_offset=0;

[[noreturn]] void fail(const char* what,uint32_t value=0) {
 fprintf(stderr,"FAIL %s value=%08x tick=%u native=%08x calls=%llu pcmFrames=%llu\n",what,value,tick,last_function,(unsigned long long)calls[last_function],(unsigned long long)(pcm.size()/2));fflush(stderr);std::exit(2);
}
uint32_t off(uint32_t x,uint32_t n=4) { uint32_t a=x&0x1fffffff;if(a>0x800000 || n>0x800000-a) fail("RAM range",x);return a; }
uint32_t rd(uint8_t* ram,uint32_t a) {uint32_t v;std::memcpy(&v,ram+off(a),4);return v;}
void wr(uint8_t* ram,uint32_t a,uint32_t v){std::memcpy(ram+off(a),&v,4);}
void send(uint8_t* ram,uint32_t q,uint32_t message){
 auto valid=rd(ram,q+8),first=rd(ram,q+12),max=rd(ram,q+16),buf=rd(ram,q+20);
 if(!max || valid>=max)fail("synchronous completion queue full",q);
 wr(ram,buf+4*((first+valid)%max),message);wr(ram,q+8,valid+1);
}
void pi_dma(uint8_t* ram,recomp_context* ctx){
 ++dma_calls;auto msg=uint32_t(ctx->r5),dir=uint32_t(ctx->r6);
 uint32_t dst=rd(ram,msg+8),src=rd(ram,msg+12),len=rd(ram,msg+16),q=rd(ram,msg+4);
 src&=0x0fffffff;
 if(dir!=0)fail("PI DMA write forbidden",dir);
 if(src>rom.size() || len>rom.size()-src)fail("ROM DMA range",src);
 auto pos=off(dst,len);
 for(uint32_t i=0;i<len;i++)ram[(pos+i)^3]=rom[src+i];
 send(ram,q,msg);ctx->r2=0;
}
void write_u32(std::ofstream& f,uint32_t x){f.write((const char*)&x,4);}
void write_u16(std::ofstream& f,uint16_t x){f.write((const char*)&x,2);}
void wave(const std::filesystem::path& name){std::ofstream f(name,std::ios::binary);uint32_t bytes=uint32_t(pcm.size()*2);f.write("RIFF",4);write_u32(f,bytes+36);f.write("WAVEfmt ",8);write_u32(f,16);write_u16(f,1);write_u16(f,2);write_u32(f,rate);write_u32(f,rate*4);write_u16(f,4);write_u16(f,16);f.write("data",4);write_u32(f,bytes);f.write((const char*)pcm.data(),bytes);if(!f)fail("PCM output write");}
}
extern RspExitReason aspMain(uint8_t*,uint32_t);
extern "C" {
int32_t* section_addresses=nullptr;
void music_native_call(uint32_t address,uint8_t* ram,recomp_context*ctx){
last_function=address;++calls[address];if(calls[address]>1000000000ull)fail("native call bound",address);
if(address==0x800007b0){current_buffer=uint32_t(ctx->r6);current_offset=0;}
if(address==0x8001012c){current_subindex=uint16_t(rd(ram,0x800b6a98)>>16)-1-uint32_t(ctx->r4);}
if(address==0x8000d398 && uint32_t(ctx->r6)==0xfb && (ctx->r7&0xffff)==0x2c && uint32_t(ctx->r5)==uint32_t(ctx->r4)+0x74 && rd(ram,uint32_t(ctx->r5))==rd(ram,uint32_t(ctx->r4)+0x18)+0x4d){
 Mark m{tick,current_buffer,current_subindex,0,~0ull,uint32_t(ctx->r4),rd(ram,uint32_t(ctx->r5)),uint32_t(ctx->r6),uint32_t(ctx->r7)};marks.push_back(m);
 fprintf(stderr,"LOOP tick=%u sub=%u base=%08x pc=%08x script=%08x queuedFrames=%llu\n",tick,current_subindex,rd(ram,m.script+0x18),m.pc,m.script,(unsigned long long)(pcm.size()/2));
}
if(address==0x800016a8){for(auto&m:marks)if(m.tick==tick&&m.subindex==uint32_t(ctx->r7))m.offset=current_offset;current_offset+=uint32_t(ctx->r5);}
}
recomp_func_t* get_function(int32_t address) {switch(uint32_t(address)){
#include "function_map.inc"
case 0x8008AB60u:return pi_dma;
default:fail("unknown indirect native call",uint32_t(address));}}
void switch_error(const char*,uint32_t address,uint32_t){fail("native switch",address);}
void do_break(uint32_t address){fail("native break",address);}
void cop0_status_write(recomp_context*ctx,gpr value){ctx->status_reg=uint32_t(value);}
gpr cop0_status_read(recomp_context*ctx){return ctx->status_reg;}
void osCreateMesgQueue_recomp(uint8_t*ram,recomp_context*ctx){auto q=uint32_t(ctx->r4);wr(ram,q,0);wr(ram,q+4,0);wr(ram,q+8,0);wr(ram,q+12,0);wr(ram,q+16,uint32_t(ctx->r6));wr(ram,q+20,uint32_t(ctx->r5));ctx->r2=0;}
void osSendMesg_recomp(uint8_t*ram,recomp_context*ctx){auto q=uint32_t(ctx->r4);if(rd(ram,q+8)>=rd(ram,q+16)){if(ctx->r6)fail("blocking send would wait",q);ctx->r2=S32(-1);return;}send(ram,q,uint32_t(ctx->r5));ctx->r2=0;}
void osRecvMesg_recomp(uint8_t*ram,recomp_context*ctx){auto q=uint32_t(ctx->r4),count=rd(ram,q+8),first=rd(ram,q+12),max=rd(ram,q+16);if(!count){if(ctx->r6)fail("blocking recv has no completion",q);ctx->r2=S32(-1);return;}if(ctx->r5)wr(ram,uint32_t(ctx->r5),rd(ram,rd(ram,q+20)+first*4));wr(ram,q+8,count-1);wr(ram,q+12,(first+1)%max);ctx->r2=0;}
void osCartRomInit_recomp(uint8_t*ram,recomp_context*ctx){wr(ram,0x8000032c,0xb0000000);ctx->r2=S32(0x80000320);}
void __osDisableInt_recomp(uint8_t*,recomp_context*ctx){ctx->r2=0;}
void __osRestoreInt_recomp(uint8_t*,recomp_context*ctx){ctx->r2=0;}
void osSetIntMask_recomp(uint8_t*,recomp_context*ctx){ctx->r2=0;}
void osGetCount_recomp(uint8_t*,recomp_context*ctx){ctx->r2=uint32_t(uint64_t(tick)*46875000/60);}
void osInvalDCache_recomp(uint8_t*,recomp_context*ctx){ctx->r2=0;}
void osWritebackDCache_recomp(uint8_t*,recomp_context*ctx){ctx->r2=0;}
void osWritebackDCacheAll_recomp(uint8_t*,recomp_context*ctx){ctx->r2=0;}
void osAiSetFrequency_recomp(uint8_t*,recomp_context*ctx){auto r=uint32_t(ctx->r4);if(rate&&rate!=r)fail("AI rate changed",r);rate=r;ctx->r2=r;fprintf(stderr,"AI frequency %u\n",rate);}
void osAiGetLength_recomp(uint8_t*,recomp_context*ctx){ctx->r2=ai_remaining;}
void osAiSetNextBuffer_recomp(uint8_t*ram,recomp_context*ctx){auto size=uint32_t(ctx->r5),addr=off(uint32_t(ctx->r4),size);if(size%4)fail("AI alignment",size);if(pcm.size()/2+size/4>max_pcm_frames)fail("PCM frame bound");for(auto&m:marks)if(m.buffer==uint32_t(ctx->r4)&&m.frame==~0ull)m.frame=pcm.size()/2+m.offset;auto p=reinterpret_cast<const int16_t*>(ram+addr);for(uint32_t i=0;i<size/2;i+=2){pcm.push_back(p[i+1]);pcm.push_back(p[i]);}ai_remaining+=size;++ai_calls;ctx->r2=0;}
}
#ifdef MENU_MUSIC_NATIVE0310_MAIN
int render_entry(int argc,char**argv){
 if(argc==2 && std::string(argv[1])=="--identity"){puts(MENU_MUSIC_NATIVE0310_ID);return 0;}
 if(argc<3){fprintf(stderr,"usage: music-render0310 ROM.z64 OUTPUT.wav [ticks]\n");return 1;}
 const auto input_path=std::filesystem::u8path(argv[1]),output_path=std::filesystem::u8path(argv[2]);
 auto metadata_path=output_path;metadata_path+=".json";
 for(const auto& output : {output_path,metadata_path}) {
  if(std::filesystem::weakly_canonical(input_path)==std::filesystem::weakly_canonical(output) || (std::filesystem::exists(output) && std::filesystem::equivalent(input_path,output)))fail("output must differ from ROM");
 }
 std::ifstream f(input_path,std::ios::binary);if(!f)fail("ROM open");f.seekg(0,std::ios::end);auto length=f.tellg();if(length!=0x1000000)fail("ROM size");f.seekg(0);rom.resize(size_t(length));if(!f.read((char*)rom.data(),rom.size()))fail("ROM read");
 if(rom[0]==0x37 && rom[1]==0x80 && rom[2]==0x40 && rom[3]==0x12){for(size_t i=0;i<rom.size();i+=2)std::swap(rom[i],rom[i+1]);}
 else if(rom[0]==0x40 && rom[1]==0x12 && rom[2]==0x37 && rom[3]==0x80){for(size_t i=0;i<rom.size();i+=4){std::swap(rom[i],rom[i+3]);std::swap(rom[i+1],rom[i+2]);}}
 normalized_rom_hash=sha256(rom);if(normalized_rom_hash!=original_usa_rom_sha256)fail("original USA ROM SHA256 mismatch");
 if(rom.size()!=0x1000000 || rom[0]!=0x80 || rom[1]!=0x37 || rom[2]!=0x12 || rom[3]!=0x40)fail("original BE16MiB ROM required");
 std::vector<uint8_t> memory(0x800000);auto ram=memory.data();
 for(uint32_t a=0x400;a<0xb4cb0;a++)ram[a^3]=rom[a+0xc00];
 recomp_context ctx{};ctx.r29=S32(0x803f0000);ctx.f_odd=&ctx.f1.u32l;
 recomp::rsp::constants_init();recomp::rsp::set_callbacks({[](const OSTask*t)->RspUcodeFunc*{return t->t.type==M_AUDTASK?aspMain:nullptr;}});
 fprintf(stderr,"init native\n");func_80010520(ram,&ctx);fprintf(stderr,"init done divider=%u\n",rd(ram,0x800b6a8c)>>16);
 ctx.r4=22;func_800679B0(ram,&ctx);fprintf(stderr,"BGM queued\n");
 uint32_t ticks=argc>3?uint32_t(std::strtoul(argv[3],nullptr,10)):render_vi_count;
 if(ticks>max_render_vi_count || ticks<1)fail("render VI bound",ticks);
 for(tick=0;tick<ticks;tick++){
  auto consumed=(uint64_t(tick+1)*rate/60-uint64_t(tick)*rate/60)*4;
  ai_remaining=uint32_t(consumed>=ai_remaining?0:ai_remaining-consumed);
  func_80010558(ram,&ctx);func_80009930(ram,&ctx);
  if(ctx.r2){auto task=reinterpret_cast<const OSTask*>(ram+off(uint32_t(ctx.r2),sizeof(OSTask)));if(!recomp::rsp::run_task(ram,task))fail("aspMain task failed");++rsp_tasks;}
  if(tick%60==0){fprintf(stderr,"tick %u pcm=%llu DMA=%llu RSP=%llu AI=%llu pending=%u\n",tick,(unsigned long long)(pcm.size()/2),(unsigned long long)dma_calls,(unsigned long long)rsp_tasks,(unsigned long long)ai_calls,ai_remaining);fflush(stderr);}
 }
 // Select a boundary only from observed main-script FB002C branches. Do not
 // infer frame counts from tempo/ticks; keep native quantization and DSP tail.
 uint64_t loop_begin=0,loop_end=0;int best_step=0x7fffffff,best_follow=0x7fffffff;
 for(size_t i=2;i+1<marks.size();i++){
  uint64_t b=marks[i].frame,e=marks[i+1].frame;if(b==~0ull||e==~0ull||e+256>=pcm.size()/2||b<1)continue;
  int step=0,follow=0;for(size_t c=0;c<2;c++)step=std::max(step,std::abs(int(pcm[b*2+c])-int(pcm[(e-1)*2+c])));
  for(size_t n=0;n<512;n++)follow=std::max(follow,std::abs(int(pcm[b*2+n])-int(pcm[e*2+n])));
  if(step<128 && follow<=2 && (step<best_step || (step==best_step&&follow<best_follow))){loop_begin=b;loop_end=e;best_step=step;best_follow=follow;}
 }
 if(argc==3){if(!loop_end)fail("no verified native PCM seam");pcm.resize(loop_end*2);}
 wave(output_path);int peak=0;uint64_t nonzero=0;for(auto x:pcm){peak=std::max(peak,std::abs(int(x)));if(x)++nonzero;}
 fprintf(stderr,"DONE rate=%u frames=%llu peak=%d nonzero=%llu tasks=%llu DMAs=%llu\n",rate,(unsigned long long)(pcm.size()/2),peak,(unsigned long long)nonzero,(unsigned long long)rsp_tasks,(unsigned long long)dma_calls);
 std::ofstream meta(metadata_path);meta<<"{\"schema\":1,\"normalized_rom_sha256\":\""<<normalized_rom_hash<<"\",\"renderer_sha256\":\""<<MENU_MUSIC_NATIVE0310_ID<<"\",\"logical_track\":22,\"physical_sequence\":11,\"channels\":2,\"bits_per_sample\":16,\"loop_begin\":"<<loop_begin<<",\"loop_end\":"<<loop_end<<",\"seam_step_max\":"<<best_step<<",\"seam_follow_256_frames_max_error\":"<<best_follow<<",\"sample_rate\":"<<rate<<",\"frames\":"<<pcm.size()/2<<",\"peak\":"<<peak<<",\"nonzero\":"<<nonzero<<",\"rsp_tasks\":"<<rsp_tasks<<",\"rom_dmas\":"<<dma_calls<<",\"native_calls\":{";bool first=true;for(auto [addr,n]:calls){if(!first)meta<<",";first=false;char b[20];sprintf(b,"0x%08X",addr);meta<<"\""<<b<<"\":"<<n;}meta<<"},\"loop_markers\":[";first=true;for(auto&m:marks){if(!first)meta<<",";first=false;meta<<"{\"tick\":"<<m.tick<<",\"frame\":"<<m.frame<<",\"buffer\":"<<m.buffer<<",\"offset\":"<<m.offset<<",\"subindex\":"<<m.subindex<<",\"script\":"<<m.script<<",\"pc\":"<<m.pc<<"}";}meta<<"]}";if(!meta)fail("provenance output write");
 return nonzero?0:3;
}
#if defined(_WIN32)
// Windows CRT narrow argv can lose characters outside the active code page.
// Keep the process boundary UTF-16, then use filesystem::u8path internally.
int wmain(int argc,wchar_t** wide_argv) {
 std::vector<std::string> encoded;encoded.reserve(argc);
 for(int i=0;i<argc;i++) {
  std::string s;
  for(size_t n=0;wide_argv[i][n];n++) {
   uint32_t cp=uint16_t(wide_argv[i][n]);
   if(cp>=0xd800 && cp<=0xdbff) {auto lo=uint16_t(wide_argv[i][++n]);if(lo<0xdc00||lo>0xdfff)fail("invalid UTF16 argument");cp=0x10000+((cp-0xd800)<<10)+(lo-0xdc00);}
   else if(cp>=0xdc00 && cp<=0xdfff)fail("invalid UTF16 argument");
   if(cp<0x80)s.push_back(char(cp));
   else if(cp<0x800){s.push_back(char(0xc0|(cp>>6)));s.push_back(char(0x80|(cp&63)));}
   else if(cp<0x10000){s.push_back(char(0xe0|(cp>>12)));s.push_back(char(0x80|((cp>>6)&63)));s.push_back(char(0x80|(cp&63)));}
   else{s.push_back(char(0xf0|(cp>>18)));s.push_back(char(0x80|((cp>>12)&63)));s.push_back(char(0x80|((cp>>6)&63)));s.push_back(char(0x80|(cp&63)));}
  }
  encoded.push_back(std::move(s));
 }
 std::vector<char*> argv;for(auto& a:encoded)argv.push_back(a.data());
 return render_entry(argc,argv.data());
}
#else
int main(int argc,char**argv) {return render_entry(argc,argv);}
#endif
#endif
