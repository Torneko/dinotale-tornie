#include "special_victory_draw.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
using namespace workshop::terrain;
int main(int argc,char** argv){try{
 need(argc==3,"Expected content and output atlas paths");std::ifstream in(argv[1],std::ios::binary);need(bool(in),"Missing content");Bytes original{std::istreambuf_iterator<char>(in),{}},extended=workshop::special_victory::extend(original);
 size_t entry=0;for(unsigned i=0;i<1582;i++)if(be16(original,0xA1C90+i*12)==0x471A)entry=0xA1C90+i*12;
 auto a=bank(extended,be32(extended,entry+4)),old=bank(original,be32(original,entry+4));auto pixels=block(extended,be32(extended,a+0x28)),before=block(original,be32(original,old+0x28)),map=block(extended,be32(extended,a+0x38));
 need(pixels.size()==12*4096 && std::equal(before.begin(),before.end(),pixels.begin()),"Original endings changed");
 std::set<Bytes> unique;for(unsigned i=0;i<12;i++)need(be16(map,i*2)==i,"Invalid victory frame map");
 for(unsigned i=6;i<12;i++){Bytes frame(pixels.begin()+i*4096,pixels.begin()+(i+1)*4096);need(std::count_if(frame.begin(),frame.end(),[](auto b){return (b&15)!=0;})>300,"Missing fruit silhouette");need(unique.insert(frame).second,"Two fruits share their background");}
 std::array<unsigned,30> ring{};for(unsigned type=0;type<12;type++){ring.fill(type);auto result=workshop::special_victory::special_type(30,ring);need(bool(result)==(type>=6),"Wrong identical fruit background selection");if(result)need(*result==type-6,"Wrong special fruit association");need(!workshop::special_victory::special_type(29,ring),"Premature background");ring[29]=100;need(!workshop::special_victory::special_type(30,ring),"Mixed fruit triggered special background");}
 need(std::equal(original.begin()+0x1000,original.begin()+0xA1C90,extended.begin()+0x1000),"Main code changed");need(std::equal(original.begin()+0xB6BCC0,original.begin()+0xEBDA70,extended.begin()+0xB6BCC0),"Overlay code changed");
 // Existing terrain appended to the same bank must survive this extension.
 auto terrain=original;terrain.resize(0x1000100,0xA7);word(terrain,0xA79D4+60,0x1000000);auto combined=workshop::special_victory::extend(terrain);need(std::equal(terrain.begin()+0x1000000,terrain.end(),combined.begin()+0x1000000),"Terrain extension overwritten");
 // Exercise actual native memory addressing and both constructor aliases.
 std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();const auto actor=gpr(int32_t(0x80010000)),drawing=gpr(int32_t(0x80011000)),ringBase=gpr(int32_t(0x800F89D4));
 for(unsigned id:{0x471Au,0x46E6u})for(unsigned fruit=0;fruit<6;fruit++){
    MEM_H(12,actor)=id;MEM_W(0x16C,actor)=uint32_t(drawing);MEM_H(0x144,actor)=0;
    for(unsigned i=0;i<30;i++)MEM_W(i*4,ringBase)=6+fruit;
    auto selected=workshop::special_victory::paint(rdram,uint32_t(actor));need(selected && *selected==fruit && MEM_HU(0x144,actor)==6+fruit,"Native frame/alias mismatch");
    for(unsigned shade=0;shade<2;shade++)for(unsigned c=0;c<3;c++)need(MEM_BU(13+shade*4+c,drawing)==workshop::special_victory::colors[fruit][c],"Native tint mismatch");
    MEM_W(116,ringBase)=0;need(!workshop::special_victory::paint(rdram,uint32_t(actor)),"Mixed native ring changed the background");
 }
 std::ofstream out(argv[2],std::ios::binary);out.write(reinterpret_cast<const char*>(pixels.data()),pixels.size());need(bool(out),"Atlas write failed");std::cout<<"Six unique native fruit backgrounds; original six frames unchanged; mixed/unfinished rejected; native code and appended terrain intact.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
