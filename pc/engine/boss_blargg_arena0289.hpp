#pragma once
#include "workshop_data.hpp"

namespace workshop::boss_blargg_arena0289 {
// A data-only, opt-in prototype. Gameplay, lava appearance and camera/culling
// still require a native game run; these definitions never imply victory.
inline constexpr unsigned logical_source=19,width=512,height=256;
inline constexpr uint16_t primary_id=0x8085,background_id=0x8087,rock_art_id=0x8069;
inline constexpr float entry_x=256.f,entry_y=152.f,pot_y=152.f;
inline constexpr std::array<float,1> safe_xs{entry_x};
inline constexpr std::array<uint16_t,2> boss_indices{5,6};
inline constexpr std::array<float,2> member_xs{160.f,352.f};
inline constexpr float member_y=232.f;
// One ordinary native hazard in the opt-in prototype; not a boss member.
// The complete swept envelope still requires two native game cycles.
inline constexpr uint16_t flame_id=0x41D0;
inline constexpr float flame_x=176.f,flame_y=232.f;
inline constexpr uint32_t native_init=0x01000000,bank15_start=0x01000000,limit=0x01400000;
inline constexpr std::array<uint16_t,6> dimensions{256,256,16,16,width,height};
inline constexpr std::array<uint8_t,2> pages{0,1};
inline constexpr std::array<unsigned,6> native_primary_dimensions{256,256,16,16,6400,256};
inline constexpr std::array<unsigned,6> native_art_dimensions{256,256,16,16,7168,512};
inline constexpr uint16_t empty_tile=0;
// All of these real8069 tiles are opaque native rock. Every safe-solid0701 cell has
// an opaque visible tile at the same native tile index; no invisible islands.
inline constexpr std::array<uint16_t,8> surface_tiles{15,16,17,18,19,20,21,22};
inline constexpr std::array<uint16_t,8> body_tiles{56,57,58,59,60,61,62,63};
struct Definition {std::vector<uint8_t> descriptor=std::vector<uint8_t>(0x118);std::vector<uint8_t> objects;};
struct Source {
 size_t primary_lookup=0,primary=0,art=0;
 uint32_t primary_pointer=0,pixels_pointer=0,palette_pointer=0;
 std::array<uint8_t,0x64> primary_descriptor{};
};
inline Definition definition_data;
inline Source source_data;
inline bool prepared=false,installed=false;
inline uint32_t installed_descriptor=0;
inline size_t lookup(std::span<const uint8_t> rom,uint16_t kind){
 for(size_t at=0xA1C90;at<0xA1C90+1582*12;at+=12)if(u16(rom,at)==kind)return at;
 throw std::runtime_error("Missing Blargg arena native resource");
}
inline size_t cell(unsigned x,unsigned y){require(x<32&&y<16,"Blargg arena cell outside512x256");return (size_t(y)*32+x)*2;}
inline constexpr bool island(unsigned x,unsigned y){return y>=11&&y<=13&&((x>=2&&x<=5)||(x>=13&&x<=18)||(x>=26&&x<=29));}
inline constexpr bool border(unsigned x,unsigned y){return y==0||x==0||x==31;}
inline uint16_t collision(unsigned x,unsigned y){
 require(x<32&&y<16,"Blargg arena collision outside512x256");
 return border(x,y)||island(x,y)?0x0701:y==14?0x6000:y==15?0x6701:0;
}
inline uint16_t tile(unsigned x,unsigned y){
 require(x<32&&y<16,"Blargg arena visual cell outside512x256");
 if(!border(x,y)&&!island(x,y))return empty_tile;
 return y==11?surface_tiles[x%surface_tiles.size()]:body_tiles[(x+y)%body_tiles.size()];
}
inline std::vector<uint8_t> tiles(){std::vector<uint8_t> out(1024);for(unsigned y=0;y<16;y++)for(unsigned x=0;x<32;x++)terrain::half(out,cell(x,y),tile(x,y));return out;}
inline std::vector<uint8_t> collisions(){std::vector<uint8_t> out(1024);for(unsigned y=0;y<16;y++)for(unsigned x=0;x<32;x++)terrain::half(out,cell(x,y),collision(x,y));return out;}
inline const Definition& definition(){require(prepared,"Blargg arena must be prepared before planning");return definition_data;}
inline void prepare(std::span<const uint8_t> rom){
 prepared=installed=false;installed_descriptor=0;
 require(rom.size()==rom_size,"Blargg preparation needs the native16MiB image");
 const auto primary_lookup=lookup(rom,primary_id),bg_lookup=lookup(rom,background_id),art_lookup=lookup(rom,rock_art_id);
 const auto primary=terrain::bank(rom,u32(rom,primary_lookup+4)),bg=terrain::bank(rom,u32(rom,bg_lookup+4)),art=terrain::bank(rom,u32(rom,art_lookup+4));
 for(auto at:{primary,bg,art})require(at+0x64<=rom.size()&&u16(rom,at)==0&&u32(rom,at+8)==0x8006E548,"Unknown Blargg terrain resource class");
 require(u16(rom,primary+0x18)==primary_id&&u16(rom,bg+0x18)==background_id&&u16(rom,art+0x18)==rock_art_id,"Blargg terrain resource identity changed");
 auto check_dimensions=[&](size_t a,const auto& expected){const auto d=terrain::bank(rom,u32(rom,a+0x20));for(unsigned i=0;i<6;i++)require(u16(rom,d+2*i)==expected[i],"Blargg native terrain dimensions changed");};
 check_dimensions(primary,native_primary_dimensions);check_dimensions(art,native_art_dimensions);
 const auto old_tiles=terrain::block(rom,u32(rom,primary+0x38)),old_collision=terrain::block(rom,u32(rom,primary+0x50));
 require(old_tiles.size()==12800&&old_collision.size()==12800&&std::all_of(old_tiles.begin(),old_tiles.end(),[](uint8_t v){return v==0;}),"Unknown invisible Blargg terrain map");
 const auto old_pages=terrain::block(rom,u32(rom,primary+0x40));require(old_pages.size()==26&&old_pages[25]==0,"Unknown native Blargg page grid");
 for(unsigned x=0;x<25;x++)require(old_pages[x]==x,"Unknown native Blargg page sequence");
 for(unsigned y=0;y<16;y++)for(unsigned x=0;x<400;x++){const bool ground=y>=11&&y<=13&&(x<=8||(x>=133&&x<=138)||x>=385);require(u16(old_collision,(size_t(y)*400+x)*2)==(y==14?0x6000:y==15?0x6701:ground?0x0701:0),"Native Blargg lava/support source changed");}
 require(terrain::block(rom,u32(rom,primary+0x28)).size()==256,"Blargg source no longer has the native blank atlas");
 require(u32(rom,primary+0x24)==0xC&&u32(rom,primary+0x2C)==0xC&&u32(rom,art+0x24)==0xC&&u32(rom,art+0x2C)==0xC,"Unknown native terrain texture block form");
 for(auto at:{primary_lookup,bg_lookup,art_lookup})require(u32(rom,terrain::bank(rom,u32(rom,at+8)))==0x05000100,"Unknown native terrain placement class");
 const auto pixels=terrain::block(rom,u32(rom,art+0x28)),palette=terrain::block(rom,u32(rom,art+0x30));
 require(pixels.size()==468*256&&palette.size()==512,"Native rock atlas or palette changed");
 for(unsigned p=0;p<256;p++)require((u16(palette,2*size_t(pixels[p]))&1)==0,"Native rock empty tile must stay transparent");
 for(const auto& group:{surface_tiles,body_tiles})for(auto id:group)for(unsigned p=0;p<256;p++)require((u16(palette,2*size_t(pixels[size_t(id)*256+p]))&1)!=0,"Blargg support art must remain fully opaque");
 // Source19 is read directly, never a stale copied room descriptor.
 // 41C7 is an alias with a different native path. Keep the exact41D0
 // resource and its original131-effect preload; no HOST timing/physics patch.
 const auto flame_lookup=lookup(rom,flame_id),flame_alias=lookup(rom,0x41C7);
 require(u32(rom,flame_lookup+4)==u32(rom,flame_alias+4)&&u32(rom,flame_lookup+8)==u32(rom,flame_alias+8),"Native Flame Unit alias resources changed");
 const auto flame=terrain::bank(rom,u32(rom,flame_lookup+4)),flame_base=terrain::bank(rom,u32(rom,flame_lookup+8));
 require(flame+0x64<=rom.size()&&flame_base+0x40<=rom.size()&&u16(rom,flame+0x18)==flame_id&&u32(rom,flame+8)==0x00CB42D0&&u32(rom,flame+0xC)==0x00CB54D0&&u32(rom,flame+0x10)==0x8057DB40&&u32(rom,flame+0x14)==0x8057ED40,"Native Flame Unit overlay changed");
 require(u32(rom,flame_base)==0x02000000&&u32(rom,flame_base+4)==std::bit_cast<uint32_t>(500.3f),"Native Flame Unit placement class changed");
 const auto entry=0xA84A0+logical_source*8;
 require(u32(rom,entry)==native_init,"Blargg native room init changed");
 const auto header=terrain::bank(rom,u32(rom,entry+4));require(header+0x118<=rom.size()&&u32(rom,header+0x24)==0x8006CF04,"Unknown Blargg native room camera");
 const auto metadata_pointer=terrain::bank(rom,u32(rom,header+0x5C)),metadata=terrain::bank(rom,u32(rom,metadata_pointer));
 require(u16(rom,metadata)==143,"Blargg native room placements changed");
 const auto native_objects=terrain::bank(rom,u32(rom,metadata+4));require(native_objects+143*12<=rom.size(),"Truncated Blargg native placements");
 Definition next;std::copy_n(rom.begin()+header,next.descriptor.size(),next.descriptor.begin());
 put32(next.descriptor,0x28,std::bit_cast<uint32_t>(96.f));put32(next.descriptor,0x2C,std::bit_cast<uint32_t>(16.f));
 for(uint16_t kind:{uint16_t(0x410C),primary_id,background_id}){
  unsigned matches=0;for(unsigned i=0;i<143;i++){const auto at=native_objects+i*12;if(u16(rom,at)==kind){++matches;next.objects.insert(next.objects.end(),rom.begin()+at,rom.begin()+at+12);}}
  require(matches==1,"Blargg arena needs one attested native layer placement");
 }
 auto add=[&](uint16_t kind,unsigned instance,float x,float y){const size_t at=next.objects.size();next.objects.resize(at+12);put32(next.objects,at,uint32_t(kind)<<16|instance);put32(next.objects,at+4,std::bit_cast<uint32_t>(x));put32(next.objects,at+8,std::bit_cast<uint32_t>(y));};
 add(0x4001,0,entry_x,entry_y);add(0x44CA,0,entry_x,entry_y);for(unsigned i=0;i<2;i++)add(0x41EC,i,member_xs[i],member_y);
 // Native4103 owns the animated lava mesh and its4104/4105 children.
 // Keep the original record and constructor; it is not a bonus enemy.
 unsigned lava_matches=0;for(unsigned i=0;i<143;i++){const auto at=native_objects+i*12;if(u16(rom,at)==0x4103){++lava_matches;require(u32(rom,at+4)==std::bit_cast<uint32_t>(40.f)&&u32(rom,at+8)==std::bit_cast<uint32_t>(200.f),"Native lava mesh placement changed");next.objects.insert(next.objects.end(),rom.begin()+at,rom.begin()+at+12);}}
 require(lava_matches==1,"Blargg arena needs the original native lava mesh");
 std::vector<uint16_t> preload;for(size_t at=0x60;at<0x100&&u16(next.descriptor,at)!=0xFFFF;at+=2)preload.push_back(u16(next.descriptor,at));
 require(std::find(preload.begin(),preload.end(),uint16_t(0x41CE))!=preload.end(),"Native Flame Unit effect preload missing");
 for(uint16_t kind:{uint16_t(0x410C),primary_id,background_id,uint16_t(0x4103),uint16_t(0x4104),uint16_t(0x4105),uint16_t(0x41EC),uint16_t(0x4273),uint16_t(0x4526),uint16_t(0x46DA),flame_id})if(std::find(preload.begin(),preload.end(),kind)==preload.end())preload.push_back(kind);
 require(preload.size()*2+2<=0xA0,"Blargg room preload tail is occupied");std::fill(next.descriptor.begin()+0x60,next.descriptor.begin()+0x100,0);
 for(unsigned i=0;i<preload.size();i++){next.descriptor[0x60+i*2]=uint8_t(preload[i]>>8);next.descriptor[0x61+i*2]=uint8_t(preload[i]);}
 next.descriptor[0x60+preload.size()*2]=next.descriptor[0x61+preload.size()*2]=255;
 Source source;source.primary_lookup=primary_lookup;source.primary=primary;source.art=art;source.primary_pointer=u32(rom,primary_lookup+4);source.pixels_pointer=u32(rom,art+0x28);source.palette_pointer=u32(rom,art+0x30);std::copy_n(rom.begin()+primary,source.primary_descriptor.size(),source.primary_descriptor.begin());
 definition_data=std::move(next);source_data=source;prepared=true;
}
// Call after boss_rooms_data0250::apply(plan), including the Lips aliases,
// and before special_victory::extend. Native code/assets are never overwritten.
inline void install(std::vector<uint8_t>& rom){
 require(prepared&&!installed&&rom.size()==rom_size&&u32(rom,0xA79D4+60)==0,"Blargg geometry bank unavailable or already installed");
 require(u16(rom,source_data.primary_lookup)==primary_id&&u32(rom,source_data.primary_lookup+4)==source_data.primary_pointer,"Blargg primary resource changed after preparation");
 std::vector<uint8_t> next=rom;
 auto pointer=[](size_t at){require(at>=bank15_start&&at<limit,"Blargg bank pointer overflow");return 0x0F000000u|uint32_t(at-bank15_start);};
 auto append=[&](std::span<const uint8_t> bytes){const size_t at=(next.size()+15)&~size_t(15);require(at<=limit&&bytes.size()<=limit-at,"Blargg arena exceeds native bank15 bound");next.resize(at+bytes.size());std::copy(bytes.begin(),bytes.end(),next.begin()+at);return at;};
 auto store=[&](const auto& bytes){const size_t data=append(bytes);std::vector<uint8_t> info(12);put32(info,0,uint32_t(bytes.size()));put32(info,4,uint32_t(bytes.size()));put32(info,8,pointer(data));return append(info);};
 std::vector<uint8_t> dims(12);for(unsigned i=0;i<6;i++)terrain::half(dims,2*i,dimensions[i]);const size_t dims_at=append(dims),tiles_info=store(tiles()),collision_info=store(collisions()),pages_info=store(pages);
 std::vector<uint8_t> descriptor(source_data.primary_descriptor.begin(),source_data.primary_descriptor.end());put32(descriptor,0x20,pointer(dims_at));
 // The opaque8069 rock assets retain their exact native CI8 pixels/palette.
 put32(descriptor,0x24,0xC);put32(descriptor,0x28,source_data.pixels_pointer);put32(descriptor,0x2C,0xC);put32(descriptor,0x30,source_data.palette_pointer);
 put32(descriptor,0x34,0xC);put32(descriptor,0x38,pointer(tiles_info));put32(descriptor,0x3C,0xC);put32(descriptor,0x40,pointer(pages_info));put32(descriptor,0x44,0);put32(descriptor,0x48,0);put32(descriptor,0x4C,0xC);put32(descriptor,0x50,pointer(collision_info));
 const size_t descriptor_at=append(descriptor);require(next.size()<=bank15_start+0x900,"Blargg geometry fixed bound exceeded");
 put32(next,0xA79D4+60,bank15_start);put32(next,source_data.primary_lookup+4,pointer(descriptor_at));rom=std::move(next);installed_descriptor=pointer(descriptor_at);installed=true;
}
}
