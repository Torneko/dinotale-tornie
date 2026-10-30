#include "workshop_data.hpp"
#include <fstream>
#include <iostream>
#include <iterator>

std::vector<uint8_t> read(const char* path) {
    std::ifstream in(path,std::ios::binary);workshop::require(bool(in),"Cannot read test file");
    return {std::istreambuf_iterator<char>(in),{}};
}
int main(int argc,char** argv) {
    try {
        workshop::require(argc==4 || argc==5,"Usage: WorkshopDataChecks original.z64 level.yslevel output.z64");
        auto original=read(argv[1]),file=read(argv[2]);
        const bool special=argc==5 && std::string(argv[4])=="--special";
        auto output=workshop::apply(original,file,workshop::allocation_start,special);
        workshop::require(std::equal(original.begin()+0x1000,original.begin()+0xA1C90,output.begin()+0x1000),"Main code changed");
        workshop::require(std::equal(original.begin()+0xB6BCC0,original.begin()+0xEBDA70,output.begin()+0xB6BCC0),"Overlay code changed");
        auto reject=[&](std::vector<uint8_t> invalid) {
            bool rejected=false;try{workshop::apply(original,invalid,workshop::allocation_start,special);}catch(const std::runtime_error&){rejected=true;}
            workshop::require(rejected,"Malformed project was accepted");
        };
        auto invalid=file;invalid[0]=0;reject(invalid);
        invalid=file;invalid.pop_back();reject(invalid);
        invalid=file;workshop::put32(invalid,8,42);reject(invalid);
        invalid=file;workshop::put32(invalid,20,0x7FC00000);reject(invalid);
        invalid=file;invalid[16]=0x7F;invalid[17]=0xFF;reject(invalid);
        invalid=file;const auto root=workshop::payload_rooms(file).front();for(size_t p=root.offset;p<root.offset+root.count*root.stride;p+=root.stride)if(workshop::u16(invalid,p)==0x4001)invalid[p+1]=0x30;reject(invalid);
        // A valid structure with a moved spawn is rejected by both loaders.
        for(const auto& room:workshop::payload_rooms(file)){invalid=file;for(unsigned i=0;i<room.count;i++){size_t p=room.offset+i*room.stride;if(workshop::u16(file,p)==0x4001){workshop::put32(invalid,p+4,workshop::u32(file,p+4)^0x00800000);break;}}reject(invalid);}
        // A structurally complete 150-object file must be rejected before the
        // native loader's fixed 149-object buffers can be reached.
        std::vector<uint8_t> overflow(16+150*12);std::copy(workshop::magic.begin(),workshop::magic.end(),overflow.begin());workshop::put32(overflow,8,workshop::u32(file,8));workshop::put32(overflow,12,150);
        size_t spawn=root.offset,other=root.offset;for(unsigned i=0;i<root.count;i++){size_t p=root.offset+i*root.stride;if(workshop::u16(file,p)==0x4001)spawn=p;else other=p;}
        std::copy_n(file.begin()+spawn,12,overflow.begin()+16);for(unsigned i=1;i<150;i++)std::copy_n(file.begin()+other,12,overflow.begin()+16+i*12);reject(overflow);
        std::ofstream out(argv[3],std::ios::binary);out.write(reinterpret_cast<const char*>(output.data()),output.size());
        workshop::require(bool(out),"Cannot write test output");
        std::cout<<"Native loader: valid project accepted; malformed, over-capacity and moved-entry projects rejected; all code preserved.\n";
        return 0;
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
