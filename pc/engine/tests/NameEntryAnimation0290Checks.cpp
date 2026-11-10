#include "recomp.h"
#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

static unsigned checks=0,frames=0,texture_commands=0,changed_texture_commands=0;
static void need(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
template<class T> static T& access(uint8_t* ram,uint64_t a,uint64_t b,unsigned flip){
    const uint32_t address=uint32_t(a+b);
    if(address<0x80000000u||address>0x80800000u-sizeof(T))throw std::runtime_error("Native RAM access out of range");
    return *reinterpret_cast<T*>(ram+((address-0x80000000u)^flip));
}
#undef MEM_W
#undef MEM_H
#undef MEM_HU
#undef MEM_B
#undef MEM_BU
#define MEM_W(a,b) access<int32_t>(rdram,uint64_t(a),uint64_t(b),0)
#define MEM_H(a,b) access<int16_t>(rdram,uint64_t(a),uint64_t(b),2)
#define MEM_HU(a,b) access<uint16_t>(rdram,uint64_t(a),uint64_t(b),2)
#define MEM_B(a,b) access<int8_t>(rdram,uint64_t(a),uint64_t(b),3)
#define MEM_BU(a,b) access<uint8_t>(rdram,uint64_t(a),uint64_t(b),3)
static int32_t section_storage[300]{};
int32_t* section_addresses=section_storage;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include NAME_ENTRY_NATIVE_CURRENT
#include NAME_ENTRY_NATIVE_FROZEN
#pragma clang diagnostic pop

static uint32_t word(const std::vector<uint8_t>& ram,uint32_t a){uint32_t v;std::memcpy(&v,ram.data()+(a&0x7FFFFFu),4);return v;}
static uint16_t half(const std::vector<uint8_t>& ram,uint32_t a){uint16_t v;std::memcpy(&v,ram.data()+((a&0x7FFFFFu)^2),2);return v;}
static void put_word(std::vector<uint8_t>& ram,uint32_t a,uint32_t v){std::memcpy(ram.data()+(a&0x7FFFFFu),&v,4);}
static void put_half(std::vector<uint8_t>& ram,uint32_t a,uint16_t v){std::memcpy(ram.data()+((a&0x7FFFFFu)^2),&v,2);}
static uint64_t reg(uint32_t value){return uint64_t(int64_t(int32_t(value)));}
static constexpr uint32_t actor=0x800C88E0u,writer=0x80760000u,commands=0x80760100u,command_end=0x80764000u,stack=0x80770000u,stride=0x8700u;
static recomp_context context(){
    recomp_context out{};auto* bytes=reinterpret_cast<uint8_t*>(&out);
    for(size_t i=0;i<sizeof(out);++i)bytes[i]=uint8_t(i*13+19);
    out.r16=reg(0x80001110u);out.r17=reg(0x80002220u);out.r18=reg(0x80003330u);out.r19=reg(0x80004440u);
    out.r20=reg(0x80005550u);out.r21=reg(0x80006660u);out.r22=reg(0x80007770u);out.r23=reg(0x80008880u);out.r30=reg(0x80009990u);
    out.r4=reg(actor);out.r5=reg(writer);out.r29=reg(stack);out.r31=reg(0x80012340u);return out;
}
static bool same_abi(const recomp_context& a,const recomp_context& b){
    return a.r16==b.r16&&a.r17==b.r17&&a.r18==b.r18&&a.r19==b.r19&&a.r20==b.r20&&a.r21==b.r21&&a.r22==b.r22&&a.r23==b.r23&&a.r29==b.r29&&a.r30==b.r30&&a.r31==b.r31;
}
static void prepare_writer(std::vector<uint8_t>& ram){
    std::fill(ram.begin()+(writer&0x7FFFFFu),ram.begin()+(command_end&0x7FFFFFu),0xA5);
    put_word(ram,writer,commands);
}
static bool scratch(size_t offset){
    return (offset>=(writer&0x7FFFFFu)&&offset<(command_end&0x7FFFFFu))||
           (offset>=((stack-0x60u)&0x7FFFFFu)&&offset<((stack+8u)&0x7FFFFFu));
}
static void compare_guest(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b){
    for(size_t i=0;i<a.size();++i)if(!scratch(i))need(a[i]==b[i],"Freeze removal changed guest state outside declared writer/stack scratch");
}
int main(int argc,char** argv){try{
    need(argc==2,"Expected historical read-only RAM fixture");
    std::ifstream input(argv[1],std::ios::binary);std::vector<uint8_t> seed((std::istreambuf_iterator<char>(input)),{});
    need(seed.size()==0x800000u,"Expected actual eight MiB RAM snapshot");
    need(word(seed,actor)==0x19971110u&&half(seed,actor+12)==0x41E0u,"Expected actual native Name Entry actor");
    const auto priv=word(seed,actor+0x1F4),resource=word(seed,actor+0x168),texture=word(seed,resource+0x80);
    need(priv==0x803FF340u&&resource==0x803FFF50u&&texture==0x80367A70u,"Historical native identity changed");
    need(half(seed,resource+0x74)==216&&half(seed,resource+0x76)==160&&half(seed,resource+0x7A)==1440,"Original nine-frame atlas dimensions");
    section_addresses[245]=int32_t(word(seed,resource+0x44)-0x10u);
    need(uint32_t(section_addresses[245])==0x803FA474u,"Expected actual loaded overlay base");
    unsigned pairs=0;
    for(unsigned primary:std::array<unsigned,3>{0,3,6})for(unsigned secondary:std::array<unsigned,3>{0,3,6}){
        ++pairs;auto current=seed,frozen=seed;
        for(auto* ram:std::array<std::vector<uint8_t>*,2>{&current,&frozen}){
            put_half(*ram,actor+0x144,uint16_t(primary));put_half(*ram,actor+0x14A,uint16_t(secondary));
            put_word(*ram,actor+0x1E0,0);put_word(*ram,actor+0x1E4,0);put_word(*ram,priv+8,0);
        }
        std::array<unsigned,3> seen{};
        for(unsigned frame=1;frame<=55;++frame){
            ++frames;prepare_writer(current);prepare_writer(frozen);auto c=context(),b=c;
            name_entry_current(current.data(),&c);name_entry_frozen(frozen.data(),&b);
            need(same_abi(c,context())&&same_abi(b,context()),"Actual native callee-saved ABI/stack/link regression");
            const unsigned index=word(current,priv+8),expected=frame==55?0u:(frame/9u)%3u;
            need(index==expected&&word(frozen,priv+8)==index,"Original nine-frame selection/reset changed");++seen[index];
            need(word(current,actor+0x1E0)==word(frozen,actor+0x1E0)&&word(current,actor+0x1E4)==word(frozen,actor+0x1E4),"Native timer changed");
            need(half(current,actor+0x144)==primary&&half(current,actor+0x14A)==secondary,"Keyboard choice/base frame changed");
            const auto end=word(current,writer);
            need(end==word(frozen,writer)&&end>commands&&end<command_end&&(end-commands)%8==0,"Display-list count/bounds changed");
            unsigned found=0;
            for(uint32_t address=commands;address<end;address+=8){
                const auto op=word(current,address),value=word(current,address+4),old=word(frozen,address+4);
                need(op==word(frozen,address),"Non-texture native display-list opcode changed");
                if((op>>24)==0xFDu){
                    ++found;++texture_commands;need(op==0xFD8800D7u,"Unexpected texture format/width command");
                    need(value==old+index*stride,"Actual native texture load did not select its original chalk variant");
                    need(value>=texture&&value<texture+9*stride,"Texture selection escaped original nine images");
                    if(value!=old)++changed_texture_commands;
                }else need(value==old,"Non-texture native command payload changed");
            }
            need(found==40,"Expected original forty texture-strip image commands");
            compare_guest(current,frozen);
        }
        need(seen[0]&&seen[1]&&seen[2],"Each keyboard pair must exercise all three native variants");
    }
    need(changed_texture_commands>0,"Frozen negative control never differs");
    std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"nativeCalls\":"<<frames*2<<",\"framesPerVariantPair\":55,\"keyboardPairs\":"<<pairs<<",\"textureCommands\":"<<texture_commands<<",\"changedTextureCommands\":"<<changed_texture_commands<<",\"nativeExternalCalls\":0,\"rendererStubs\":0,\"fullGuestStateCompared\":true,\"gameLaunched\":false,\"gpuVisualsVerified\":false}\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
