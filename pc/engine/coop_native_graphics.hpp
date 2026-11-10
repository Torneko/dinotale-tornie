#pragma once
#include "recomp.h"
#include <array>
#include <cstdint>
#include <cstdio>

extern "C" void func_80064DD0(uint8_t*,recomp_context*);
extern "C" void func_80064E84(uint8_t*,recomp_context*);
extern "C" void func_8007BE70(uint8_t*,recomp_context*);
extern "C" void func_8007FCF0(uint8_t*,recomp_context*);
extern "C" void func_8005DAE4(uint8_t*,recomp_context*);

namespace workshop::coop_graphics {
// Bounded room43 prototype only. Call ensure BEFORE factory.frame, after the
// completed675F0 graphics fence. No native graphics initialization is rerun.
inline constexpr uint32_t descriptor=0x800A6380,frame_globals=0x800F7EA0,
    frame_index=0x800F8920,current_records=0x800F8930,block_size=0x130;
inline constexpr uint32_t record_stride=0x28,records_a=0x10,records_b=0x60,
    matrix_a=0xB0,matrix_b=0xF0;
inline uint32_t allocation=0,primary_actor=0,primary_drawing=0,primary_texture=0;
inline std::array<uint32_t,5> original{};
inline bool installed=false;
inline bool range(uint32_t p,unsigned bytes){return p>=0x80000400 && p<0x80800000 && bytes<=0x80800000-p;}
inline gpr address(uint32_t p){return gpr(int32_t(p));}
inline uint32_t word(uint8_t* rdram,uint32_t p){const gpr a=address(p);return uint32_t(MEM_W(0,a));}
inline void word(uint8_t* rdram,uint32_t p,uint32_t v){const gpr a=address(p);MEM_W(0,a)=v;}
inline unsigned byte(uint8_t* rdram,uint32_t p){const gpr a=address(p);return MEM_BU(0,a);}
inline void byte(uint8_t* rdram,uint32_t p,unsigned v){const gpr a=address(p);MEM_B(0,a)=uint8_t(v);}
inline unsigned half(uint8_t* rdram,uint32_t p){const gpr a=address(p);return MEM_HU(0,a);}
inline bool disjoint(uint32_t a,unsigned an,uint32_t b,unsigned bn){return uint64_t(a)+an<=b || uint64_t(b)+bn<=a;}
inline void copy(uint8_t* r,uint32_t dst,uint32_t src,unsigned size){for(unsigned n=0;n<size;n++)byte(r,dst+n,byte(r,src+n));}
inline uint32_t fingerprint(uint8_t* r,uint32_t p,unsigned size){uint32_t h=2166136261u;for(unsigned n=0;n<size;n++)h=(h^byte(r,p+n))*16777619u;return h;}
inline bool heap_allocation(uint8_t* r,uint32_t p,unsigned size){
    return range(p-0x10,size+0x10) && !(p&15) && half(r,p-0x10)==0x913 &&
        half(r,p-0xE)!=0 && word(r,p-4)>=size+0x10;
}
struct Registers {
    recomp_context* context;recomp_context saved;
    explicit Registers(recomp_context* c):context(c),saved(*c){context->r29=ADD32(saved.r29,-0x80);}
    ~Registers(){*context=saved;}
};
inline std::array<uint32_t,5> read_descriptor(uint8_t* r){
    std::array<uint32_t,5> d{};for(unsigned n=0;n<5;n++)d[n]=word(r,descriptor+n*4);return d;
}
inline bool matches(uint8_t* r){
    return installed && heap_allocation(r,allocation,block_size) &&
        read_descriptor(r)==std::array<uint32_t,5>{2,allocation,record_stride,allocation+records_a,allocation+records_b};
}
inline bool cache_matches(uint8_t* r){return byte(r,frame_index)<2 && word(r,current_records)==allocation+(byte(r,frame_index)?records_b:records_a);}
inline bool valid_record(uint8_t* r,uint32_t p){
    return range(p,record_stride) && byte(r,p)==8 && word(r,p+4)==0x8007BEE8 &&
        range(word(r,p+0x18),0x40);
}
inline void refresh(uint8_t* r,recomp_context* c){c->r4=address(frame_globals);func_8005DAE4(r,c);}
inline bool ensure(uint8_t* r,recomp_context* c,uint32_t primary){
    if(!c)return false;
    if(installed)return matches(r) && cache_matches(r);
    const auto d=read_descriptor(r);
    if(d[0]!=1 || d[2]!=record_stride || !range(d[1],1) || byte(r,d[1])!=1 ||
       !valid_record(r,d[3]) || !valid_record(r,d[4]) || !disjoint(d[3],record_stride,d[4],record_stride))return false;
    const uint32_t old_matrix_a=word(r,d[3]+0x18),old_matrix_b=word(r,d[4]+0x18);
    if(!disjoint(old_matrix_a,0x40,old_matrix_b,0x40))return false;
    if(!range(primary,0x1FC) || word(r,primary)!=0x19971110 || half(r,primary+12)!=0x4001 ||
       byte(r,primary+6)==3 || byte(r,primary+0x164)!=3 || byte(r,primary+0x17D)!=2)return false;
    const uint32_t drawing=word(r,primary+0x16C);
    if(!range(drawing,0x414) || word(r,drawing+0x2C)!=0)return false;
    const uint32_t texture=word(r,drawing+0x40);
    if(!range(texture,0x1EC00) || byte(r,frame_index)>1)return false;
    const std::array<uint32_t,5> old_hashes={fingerprint(r,d[3],record_stride),fingerprint(r,d[4],record_stride),
        fingerprint(r,old_matrix_a,0x40),fingerprint(r,old_matrix_b,0x40),fingerprint(r,texture,0x1EC00)};
    std::array<std::array<uint8_t,record_stride>,2> record_snapshots{};
    for(unsigned f=0;f<2;f++)for(unsigned n=0;n<record_stride;n++)record_snapshots[f][n]=byte(r,d[3+f]+n);
    Registers regs(c);
    c->r4=block_size;func_80064DD0(r,c);const uint32_t block=uint32_t(c->r2);
    if(!block)return false;
    auto free_block=[&](){c->r4=address(block);func_80064E84(r,c);};
    if(!heap_allocation(r,block,block_size) || !disjoint(block,block_size,drawing,0x414) ||
       !disjoint(block,block_size,texture,0x1EC00) || !disjoint(block,block_size,d[1],1) ||
       !disjoint(block,block_size,d[3],record_stride) || !disjoint(block,block_size,d[4],record_stride) ||
       !disjoint(block,block_size,word(r,d[3]+0x18),0x40) || !disjoint(block,block_size,word(r,d[4]+0x18),0x40)){
        // A valid native allocator result alone may be returned to its allocator.
        if(heap_allocation(r,block,block_size))free_block();return false;
    }
    for(unsigned n=0;n<block_size;n++)byte(r,block+n,0);
    byte(r,block,byte(r,d[1])); // Existing slot0 remains occupied; slot1 is free.
    for(unsigned f=0;f<2;f++){
        const uint32_t records=block+(f?records_b:records_a),matrix=block+(f?matrix_b:matrix_a);
        copy(r,records,d[3+f],record_stride);
        c->r4=address(records+record_stride);c->r5=address(matrix);c->r6=0;func_8007BE70(r,c);
        c->r4=address(matrix);func_8007FCF0(r,c);
        if(!valid_record(r,records+record_stride) || word(r,records+record_stride+0x18)!=matrix){free_block();return false;}
    }
    // Check again after allocation/initialization, before exposing any pointer.
    if(read_descriptor(r)!=d || byte(r,d[1])!=1){free_block();return false;}
    for(unsigned f=0;f<2;f++)for(unsigned n=0;n<record_stride;n++)if(byte(r,d[3+f]+n)!=record_snapshots[f][n]){free_block();return false;}
    const std::array<uint32_t,5> after_hashes={fingerprint(r,d[3],record_stride),fingerprint(r,d[4],record_stride),
        fingerprint(r,old_matrix_a,0x40),fingerprint(r,old_matrix_b,0x40),fingerprint(r,texture,0x1EC00)};
    if(old_hashes!=after_hashes){free_block();return false;}
    original=d;allocation=block;primary_actor=primary;primary_drawing=drawing;primary_texture=texture;
    // Graphics are idle at the caller's native fence. Publish storage pointers
    // before capacity; existing P1 indices and matrix pointers remain unchanged.
    word(r,descriptor+4,block);word(r,descriptor+0xC,block+records_a);word(r,descriptor+0x10,block+records_b);word(r,descriptor,2);
    installed=true;refresh(r,c);
    const uint32_t expected=block+(byte(r,frame_index)?records_b:records_a);
    if(word(r,current_records)!=expected)return false; // Keep published storage alive.
    fprintf(stderr,"[CoopGraphics] extended pool3 primary=%08X block=%08X bytes=%u flags=%08X records=%08X,%08X matrices=%08X,%08X priorRecords=%08X,%08X primaryDraw=%08X primaryTexture=%08X cache=%08X matrixArenaCount=%u\n",
        primary,block,block_size,block,block+records_a,block+records_b,block+matrix_a,block+matrix_b,d[3],d[4],drawing,texture,word(r,current_records),word(r,0x800A6340));
    fprintf(stderr,"[CoopGraphics] preserved primary=%08X priorFlags=%08X priorCount=%u stride=%u priorMatrices=%08X,%08X heapBytes=%u recordHashes=%08X,%08X matrixHashes=%08X,%08X textureHash=%08X beforeAfterEqual=1 slot0ByteExact=1\n",
        primary,d[1],d[0],d[2],old_matrix_a,old_matrix_b,word(r,block-4),old_hashes[0],old_hashes[1],old_hashes[2],old_hashes[3],old_hashes[4]);
    return true;
}
// Call only after the secondary is completely unlinked/freed, at the same safe
// graphics fence and BEFORE any room/heap reset. Refuse while slot1 is owned.
inline bool release(uint8_t* r,recomp_context* c){
    if(!installed)return true;
    if(!c || !matches(r) || byte(r,allocation+1)!=0)return false;
    Registers regs(c);
    for(unsigned f=0;f<2;f++)copy(r,original[3+f],allocation+(f?records_b:records_a),record_stride);
    byte(r,original[1],byte(r,allocation));
    for(unsigned n=1;n<5;n++)word(r,descriptor+n*4,original[n]);word(r,descriptor,original[0]);
    refresh(r,c);const uint32_t expected=original[byte(r,frame_index)?4:3];
    if(byte(r,frame_index)>1 || word(r,current_records)!=expected)return false; // Do not free a referenced block.
    const uint32_t block=allocation;
    c->r4=address(block);func_80064E84(r,c);
    installed=false;allocation=primary_actor=primary_drawing=primary_texture=0;original={};
    fprintf(stderr,"[CoopGraphics] restored pool3 and released block=%08X\n",block);return true;
}
}
