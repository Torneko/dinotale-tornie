#include "../story_rights0320.hpp"
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
namespace rights=workshop::story_rights0320;
namespace {
unsigned checks=0;
void check(bool condition,const char* why){++checks;if(!condition)throw std::runtime_error(why);}
using Image=std::array<uint8_t,2048>;
using Meta=std::array<uint8_t,6>;
std::vector<uint8_t> file_bytes(const std::filesystem::path& file){std::ifstream input(file,std::ios::binary);return {std::istreambuf_iterator<char>(input),{}};}
// Native checksum expressed as16-bit bitset operations rather than shifts of
// a host uint16_t. Known real raw EEPROM checksums also anchor this reference.
uint16_t reference_checksum(const uint8_t* p){std::array<bool,16> bits{};for(unsigned i=0;i<1018;i++){for(unsigned b=0;b<8;b++)bits[b]=bits[b]!=bool((p[i]>>b)&1);const auto high=bits.back();for(unsigned b=15;b>0;b--)bits[b]=bits[b-1];bits[0]=high;}unsigned sum=0;for(unsigned b=0;b<16;b++)if(bits[b])sum|=1u<<b;return uint16_t(sum);}
void seal(Image& image,unsigned copy){auto* p=image.data()+copy*1024;const auto sum=reference_checksum(p);p[1018]=uint8_t(sum>>8);p[1019]=uint8_t(sum);}
void magic(Image& image,unsigned copy,uint32_t value){auto* p=image.data()+copy*1024;for(unsigned b=0;b<4;b++)p[1020+b]=uint8_t(value>>(24-b*8));}
void meta(Image& image,unsigned copy,const Meta& value){auto* p=image.data()+copy*1024;for(unsigned r=0;r<48;r++)p[6+8*r]=uint8_t((p[6+8*r]&127)|(((value[r/8]>>(7-r%8))&1)<<7));seal(image,copy);}
Meta metadata(uint32_t trial,unsigned first,std::array<unsigned,4> columns,unsigned eggs,unsigned lives){unsigned draws=0x8000|first;for(unsigned c=0;c<4;c++)draws|=columns[c]<<(3+3*c);return {uint8_t(trial>>16),uint8_t(trial>>8),uint8_t(trial),uint8_t(draws>>8),uint8_t(draws),uint8_t(eggs|(lives<<2))};}
Image fixture(){Image result{};for(unsigned copy=0;copy<2;copy++){magic(result,copy,0x81317539);meta(result,copy,{0,0,0,255,255,252});}return result;}
void both_meta(Image& image,const Meta& value){meta(image,0,value);meta(image,1,value);}
void set_story(Image& image,uint32_t value){for(unsigned copy=0;copy<2;copy++){auto* p=image.data()+copy*1024;p[1]=uint8_t(value>>16);p[2]=uint8_t(value>>8);p[3]=uint8_t(value);seal(image,copy);}}
void name(Image& image,unsigned copy,unsigned record,uint64_t value){auto* p=image.data()+copy*1024+6+8*record;const auto high=p[0]&128;for(unsigned b=0;b<6;b++)p[b]=uint8_t(value>>(40-b*8));p[0]|=uint8_t(high);seal(image,copy);}
void write(const std::filesystem::path& file,std::span<const uint8_t> bytes){std::ofstream output(file,std::ios::binary);output.write(reinterpret_cast<const char*>(bytes.data()),std::streamsize(bytes.size()));output.close();check(bool(output),"Fixture write failed");}
void rejected(const Image& input,const char* why){const auto old=input;auto view=rights::decode(input);check(view.present&&!view.readable,why);check(!view.trial_complete()&&view.legacy_owned==0&&view.bw_earned==0,"Malformed source leaked ownership");check(input==old,"Decoder changed malformed EEPROM");}
}
int main(int argc,char** argv){try{
 check(argc==3,"Expected real raw EEPROM fixture and isolated directory");
 const auto bytes=file_bytes(argv[1]);check(bytes.size()==2048,"Real raw EEPROM size");const auto original=bytes;
 const auto real=rights::decode(bytes);check(real.present&&real.readable&&real.copy==0,"Real raw canonical EEPROM not accepted");
 check(real.magic==0x81317539&&real.story==0xFFFFFF&&real.trial==0xFFFFFF&&real.trial_complete(),"Real native Story/Trial mask mapping");
 check(real.progression.first==0&&real.progression.columns==std::array<uint8_t,4>{1,2,3,4}&&real.progression.eggs==3&&real.progression.lives==63,"Real compressed metadata mapping");
 check(real.legacy_owned==63&&real.bw_earned==3&&!real.continuing,"Real native rights projection");check(bytes==original,"Real EEPROM mutated");
 check(reference_checksum(bytes.data())==0x7E81&&reference_checksum(bytes.data()+1024)==0x7E81,"Native real checksum reference mismatch");
 auto base=fixture();auto empty=rights::decode(base);check(empty.readable&&empty.legacy_owned==0&&empty.trial==0&&empty.story==0&&!empty.continuing,"Minimal native metadata not valid");
 for(size_t length:std::array<size_t,10>{0,1,6,48,1023,1024,1025,2047,2049,4096}){std::vector<uint8_t> p(length);auto view=rights::decode(p);check(view.present&&!view.readable&&view.reason=="eeprom-size-invalid","Wrong raw EEPROM bounds admitted");}
 // Every payload bit is protected by the real native checksum, with fallback
 // to the second copy. Both bad copies must fail without yielding rights.
 for(unsigned offset=0;offset<1018;offset++)for(unsigned bit=0;bit<8;bit++){
  auto bad=base;bad[offset]^=uint8_t(1u<<bit);auto good=rights::decode(bad);check(good.readable&&good.copy==1,"Checksum corruption did not fall back to copy1");
  bad[1024+offset]^=uint8_t(1u<<bit);rejected(bad,"Both corrupted payload copies accepted");
 }
 for(unsigned copy=0;copy<2;copy++)for(unsigned offset=1018;offset<1024;offset++)for(unsigned bit=0;bit<8;bit++){
  auto bad=base;bad[copy*1024+offset]^=uint8_t(1u<<bit);auto view=rights::decode(bad);const uint32_t altered_magic=uint32_t(bad[1020])<<24|uint32_t(bad[1021])<<16|uint32_t(bad[1022])<<8|bad[1023];const bool first_accepted=copy!=0||((offset>=1020)&&(altered_magic==0x81317531||altered_magic==0x81317538||altered_magic==0x81317539));check(view.readable&&view.copy==(first_accepted?0u:1u),"Header/checksum byte corruption priority mismatch");
 }
 // Valid checksum must not allow invalid compressed names, even in unused
 // records or in a late copy. The maximal representable legal name passes.
 for(unsigned record=0;record<125;record++){
  auto good=base;name(good,0,record,228ull*228*228*228*228*228-1);check(rights::decode(good).copy==0,"Maximal legal compressed name rejected");
  for(uint64_t value:std::array<uint64_t,3>{0x7FC3A4111000ull,0x7FC3FFFFFFFFull,0x7FFFFFFFFFFFull}){
   auto bad=base;name(bad,0,record,value);check(rights::decode(bad).copy==1,"Invalid compressed name accepted as first copy");name(bad,1,record,value);rejected(bad,"Invalid compressed names accepted with valid checksums");
  }
 }
 // Proven native compatibility intentionally ignores the metadata bit for
 // records48..124, and vanilla magic does not use the compressed codec.
 for(unsigned record=48;record<125;record++){auto good=base;good[6+8*record]|=128;seal(good,0);check(rights::decode(good).copy==0,"Native unused high bit rejected");}
 auto vanilla=base;for(unsigned copy=0;copy<2;copy++){magic(vanilla,copy,0x81317531);name(vanilla,copy,124,0x7FFFFFFFFFFFull);}set_story(vanilla,0xA13579);auto v=rights::decode(vanilla);check(v.readable&&v.story==0xA13579&&v.trial==0&&v.legacy_owned==0&&v.bw_earned==0,"Vanilla raw compatibility failed");
 auto no_marker=base;both_meta(no_marker,{0,0,0,127,255,252});rejected(no_marker,"Missing extension marker accepted");
 auto bad_color=base;both_meta(bad_color,metadata(0,6,{7,7,7,7},0,63));rejected(bad_color,"Reserved first color6 accepted");
 for(unsigned c=0;c<4;c++){auto bad=base;std::array<unsigned,4> cols{7,7,7,7};cols[c]=6;both_meta(bad,metadata(0,7,cols,0,63));rejected(bad,"Reserved column color6 accepted");}
 for(unsigned eggs=1;eggs<4;eggs++){auto bad=base;both_meta(bad,metadata(0,7,{7,7,7,7},eggs,63));rejected(bad,"Egg rights without24 Trial clears accepted");}
 for(unsigned color=0;color<6;color++){
  auto good=base;both_meta(good,metadata(0,color,{7,7,7,7},0,color*10));auto view=rights::decode(good);check(view.readable&&view.legacy_owned==(1u<<color)&&view.progression.lives==color*10,"First owned color incorrectly derived from alive lives");
  for(unsigned c=0;c<4;c++){
   std::array<unsigned,4> cols{7,7,7,7};cols[c]=(color+1)%6;
   auto story=good;set_story(story,workshop::progress::column(c));both_meta(story,metadata(0,color,cols,0,0));view=rights::decode(story);check(view.readable&&view.legacy_owned==((1u<<color)|(1u<<((color+1)%6))),"Modern Story-only reward rejected or lost");
   auto duplicate=story;cols[c]=color;both_meta(duplicate,metadata(0,color,cols,0,63));rejected(duplicate,"Duplicated drawn color accepted");
   auto unearned=good;cols[c]=(color+1)%6;both_meta(unearned,metadata(0,color,cols,0,63));rejected(unearned,"Unearned stored column reward accepted");
   auto trial=good;both_meta(trial,metadata(workshop::progress::column(c),color,{7,7,7,7},0,63));rejected(trial,"Missing Trial column reward accepted");
   auto reconciliable=good;set_story(reconciliable,workshop::progress::column(c));check(rights::decode(reconciliable).readable,"Historical unreconciled Story column rejected");
  }
 }
 for(unsigned eggs=0;eggs<4;eggs++)for(unsigned lives=0;lives<64;lives++){
  auto full=base;both_meta(full,metadata(0xFFFFFF,0,{1,2,3,4},eggs,lives));auto view=rights::decode(full);check(view.readable&&view.legacy_owned==63&&view.bw_earned==eggs&&view.progression.lives==lives&&view.trial_complete(),"AllTrial rights must survive independent lives");
 }
 auto story_full=base;set_story(story_full,0xFFFFFF);both_meta(story_full,metadata(0,5,{0,1,2,3},0,0));check(rights::decode(story_full).legacy_owned==63,"Full Story sixth legacy owned color missing");
 // V8 life migration uses the raw native5+1 bits, never the old sixth byte.
 for(unsigned low=0;low<4;low++)for(unsigned last=0;last<8;last++){
  auto old=base;for(unsigned copy=0;copy<2;copy++){magic(old,copy,0x81317538);old[copy*1024]=uint8_t(low);old[copy*1024+1015]=uint8_t(last);seal(old,copy);}both_meta(old,metadata(0,2,{7,7,7,7},3,0));auto view=rights::decode(old);check(view.readable&&view.bw_earned==0&&view.progression.lives==((low<<3)|last|32)&&view.legacy_owned==4,"V8 native migration mismatch");
 }
 for(unsigned flags=0;flags<16;flags++)for(unsigned continued=0;continued<16;continued++){
  auto image=base;for(unsigned copy=0;copy<2;copy++){auto* p=image.data()+copy*1024;for(unsigned i=0;i<14;i++)p[1004+i]=uint8_t(33+i);p[0]=0xAF;p[1015]=uint8_t(flags);p[1016]=0xA0;p[1017]=uint8_t(continued);seal(image,copy);}auto view=rights::decode(image);check(view.readable&&view.raw_flags==0xAF&&view.continuing==bool((flags&8)&&(continued&7))&&view.continuation[0]==33&&view.continuation[11]==flags&&view.continuation[12]==0xA0&&view.continuation[13]==continued,"Raw continuation packing mismatch");
 }
 const std::filesystem::path work=argv[2];const auto path=work/std::filesystem::path(u8"é-canonique-雪.eep");write(path,bytes);auto disk=rights::read(path);check(disk.readable&&disk.trial_complete()&&disk.bw_earned==3,"Unicode canonical source not decoded");check(file_bytes(path)==bytes,"Source file modified during read");
 auto missing=rights::read(work/"absent.eep");check(!missing.present&&missing.readable&&!missing.trial_complete()&&missing.legacy_owned==0,"Absent source treated as corrupt or owned");
 check(!rights::read(work).readable,"Directory admitted as EEPROM");auto oversized=bytes;oversized.push_back(1);write(path,oversized);check(!rights::read(path).readable,"Oversized file admitted");write(path,std::span<const uint8_t>(bytes.data(),2047));check(!rights::read(path).readable,"Truncated file admitted");
 write(path,bytes);std::error_code error;const auto link=work/"link.eep";std::filesystem::create_symlink(path,link,error);if(!error)check(!rights::read(link).readable,"Symlink source admitted");
 check(bytes==original,"Original raw bytes changed at end");
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"realChecksum\":\"7E81\",\"realFixture\":true,\"gpu\":false,\"gameplay\":false}"<<std::endl;return 0;
 }catch(const std::exception& error){std::cerr<<"FAIL after "<<checks<<" checks: "<<error.what()<<std::endl;return 1;}}
