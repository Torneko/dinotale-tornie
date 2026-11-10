#pragma once
#include "custom_black_flyguy.hpp"
#include <filesystem>
static void check_black_flyguy(const char* egg_fixture){
 using namespace workshop;auto file=std::filesystem::path(egg_fixture).parent_path()/"black-flyguy-runtime.yslevel";auto payload=read(file.string().c_str());black_flyguy::configure(payload);black_flyguy::enter_room(35);
 auto rooms=payload_rooms(payload);auto room=*std::find_if(rooms.begin(),rooms.end(),[](auto r){return r.source==35;});size_t source=0;for(unsigned i=0;i<room.count;i++){auto p=room.offset+i*room.stride;if(u16(payload,p)==0x45E3){source=p;break;}}check(source!=0,"Missing standalone fixture");
 std::vector<uint8_t> memory(0x800000);auto* rdram=memory.data();constexpr uint32_t custom=0x80010000,ordinary=0x80020000,child=0x80030000,collision=0x80011000,ordinary_collision=0x80021000;
 for(auto a:{custom,ordinary}){MEM_W(0,int32_t(a))=0x19971110;MEM_H(12,int32_t(a))=0x45E5;MEM_B(0x164,int32_t(a))=0;MEM_B(0x165,int32_t(a))=0;}
 MEM_W(0x174,int32_t(custom))=collision;MEM_W(0x174,int32_t(ordinary))=ordinary_collision;MEM_W(0x14,int32_t(collision))=3;MEM_W(0x14,int32_t(ordinary_collision))=4;
 recomp_context c{};c.r4=int32_t(custom);c.r5=black_flyguy::packed(u32(payload,source));c.r6=u32(payload,source+4);c.r7=u32(payload,source+8);black_flyguy::created(&c,35);
 c.r4=int32_t(ordinary);c.r6=u32(payload,source+4)^1;black_flyguy::created(&c,35);auto expected=memory;black_flyguy::prepare(rdram,ordinary);check(memory==expected,"Ordinary45E5 modified");
 auto* expected_ram=expected.data();{auto* rdram=expected_ram;MEM_B(0x165,int32_t(custom))=1;MEM_W(0x14,int32_t(collision))=0x7FFFFFFF;}
 black_flyguy::prepare(rdram,custom);check(memory==expected,"Custom controller modified fields beyond165/private inhibition");check(black_flyguy::skip_collision(rdram,custom) && !black_flyguy::skip_collision(rdram,ordinary),"Collision guard is not exact");
 MEM_W(0,int32_t(child))=0x19971110;MEM_H(12,int32_t(child))=0x45E3;MEM_W(0x180,int32_t(child))=custom;c.r4=int32_t(custom);c.r5=int32_t(child);black_flyguy::attached(rdram,&c);check(black_flyguy::children.at(child)==custom && uint32_t(MEM_W(0x180,int32_t(child)))==custom,"Native parent link lost");
 // The graphics-only cleanup hook must preserve the live native controller.
 editor_objects::restore(rdram,custom);check(black_flyguy::controllers.contains(custom) && black_flyguy::children.at(child)==custom && black_flyguy::skip_collision(rdram,custom),"Graphics cleanup discarded standalone controller");
 black_flyguy::destroy(rdram,custom);check(!black_flyguy::controllers.contains(custom) && !black_flyguy::children.contains(child) && MEM_BU(0x165,int32_t(custom))==0 && MEM_W(0x14,int32_t(collision))==3,"Custom disposal failed to restore owned fields");
 c.r4=int32_t(custom);c.r5=black_flyguy::packed(u32(payload,source));c.r6=u32(payload,source+4);c.r7=u32(payload,source+8);black_flyguy::created(&c,35);c.r6^=1;black_flyguy::created(&c,35);expected=memory;black_flyguy::prepare(rdram,custom);check(memory==expected && !black_flyguy::controllers.contains(custom),"Reused address retained custom binding");
 c.r4=int32_t(custom);c.r5=black_flyguy::packed(u32(payload,source));c.r6=u32(payload,source+4);black_flyguy::created(&c,35);black_flyguy::prepare(rdram,custom);expected=memory;black_flyguy::forget(custom);check(memory==expected && !black_flyguy::controllers.contains(custom),"Fast-free standalone cleanup touched RAM or retained binding");
 black_flyguy::configure({});check(black_flyguy::placements.empty() && black_flyguy::controllers.empty() && black_flyguy::children.empty(),"No-payload standalone isolation failed");
}
