#include "custom_editor_objects.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
using namespace workshop;
static unsigned banks=0,sounds=0;
extern "C" void func_80067E68(uint8_t*,recomp_context* c){if(c->r4!=6)throw std::runtime_error("wrong sound bank");banks++;c->r2=0x1234;}
extern "C" void func_80067818(uint8_t*,recomp_context* c){if(c->r4!=0x251 || c->r5!=0x1234)throw std::runtime_error("wrong pickup sound");sounds++;}
static std::vector<uint8_t> read(const char* p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("Missing fixture");return {std::istreambuf_iterator<char>(f),{}};}
static void check(bool yes,const char* message){if(!yes)throw std::runtime_error(message);}
#include "black_flyguy_memory_checks.hpp"
int main(int argc,char** argv){try{
    check(argc==3,"Expected original ROM and eight-egg payload");check_black_flyguy(argv[2]);auto original=read(argv[1]),payload=read(argv[2]);validate_payload(payload);editor_objects::configure(original,payload);
    std::vector<uint8_t> memory(0x800000,0);auto* rdram=memory.data();auto untouched=memory;
    unsigned tested=0;
    for(auto room:payload_rooms(payload))for(unsigned i=0;i<room.count;i++){
        const auto p=room.offset+i*room.stride;const auto id=u16(payload,p);if(!editor_objects::special(id))continue;
        const unsigned color=editor_objects::color(id);editor_objects::enter_room(room.source);
        const uint32_t parent=0x80010000+tested*0x1000,child=parent+0x400,egg=parent+0x800;
        recomp_context c{};c.r4=int32_t(parent);c.r5=editor_objects::packed(room.source,u32(payload,p));c.r6=u32(payload,p+4);c.r7=u32(payload,p+8);
        editor_objects::created(rdram,&c,room.source);check(editor_objects::source_color(parent)==color,"Custom egg binding lost color");
        c.r4=int32_t(parent);c.r5=0x41CE;editor_objects::begin_child(&c);
        c.r4=int32_t(child);c.r5=0x41CE2000;c.r6=0;c.r7=0;editor_objects::created(rdram,&c,room.source);
        c.r7=int32_t(child);check(editor_objects::release_color(rdram,&c)==color,"Wrapper released wrong custom egg");
        editor_objects::spawned(egg,color);check(editor_objects::egg_state(egg,4),"Custom pool egg untracked");
        c.r4=int32_t(egg);c.r5=0x778899;c.r29=int32_t(0x80040000);c.r31=0x12345678;const auto saved=c;
        check(!editor_objects::pickup_sound(rdram,&c,egg,3),"Pickup audio before carried state");
        check(editor_objects::pickup_sound(rdram,&c,egg,4),"Missing custom egg pickup cue");
        check(std::memcmp(&saved,&c,sizeof(c))==0,"Pickup changed native context");
        check(!editor_objects::pickup_sound(rdram,&c,egg,5),"Duplicate pickup audio");
        editor_objects::enter_room(room.source+1);check(!editor_objects::source_color(parent),"Stale room source remained active");
        check(editor_objects::egg_state(egg,5),"Carried egg lost on room transition");
        tested++;
    }
    check(tested==8 && banks==8 && sounds==8,"Expected eight egg colors and audio cues");
    check(memory==untouched,"Registry/state/audio changed native progress memory");
    // Rendering borrows all three references together, retaining the wrapper's
    // native resource and ownership. Disposal must restore the defaults.
    constexpr uint32_t table=0x80057000,native=0x80058000,wrapper=0x80058100,palette=0x80060000,texture=0x80062000;
    MEM_W(0,int32_t(0x800F84D4))=table;MEM_B(0,int32_t(table))=1;MEM_W(4,int32_t(table))=native;
    MEM_H(0x3C,int32_t(native))=0x401D;MEM_W(0x88,int32_t(native))=palette;MEM_W(0x84,int32_t(native))=texture;
    MEM_H(0x3C,int32_t(wrapper))=0x41CE;MEM_W(0x88,int32_t(wrapper))=0x80064000;MEM_W(0x84,int32_t(wrapper))=0x80065000;
    for(unsigned i=0;i<8;i++){
        const auto child=0x80010400+i*0x1000,drawing=0x80070000+i*0x80,item=0x80071000+i*4;
        MEM_W(0,int32_t(child))=0x19971110;MEM_H(12,int32_t(child))=0x41CE;MEM_W(0x168,int32_t(child))=wrapper;MEM_W(0x16C,int32_t(child))=drawing;
        MEM_W(0x1F4,int32_t(child))=item;MEM_H(0,int32_t(item))=0x401C;
        editor_objects::enter_room(editor_objects::children.at(child).room);
        const auto before=memory;
        editor_objects::paint(rdram,child);
        check(uint32_t(MEM_W(8,int32_t(drawing)))==palette+(6+i)*512,"Wrong palette bank");
        check(uint32_t(MEM_W(0x30,int32_t(drawing)))==texture && uint32_t(MEM_W(0x40,int32_t(drawing)))==native+0x74,"Texture/geometry did not match native egg resource");
        check(uint32_t(MEM_W(0x168,int32_t(child)))==wrapper && MEM_HU(0x144,int32_t(child))==0,"Wrapper logic/frame changed");
        auto expected=before;auto* drawmem=rdram;rdram=expected.data();MEM_W(8,int32_t(drawing))=palette+(6+i)*512;MEM_B(4,int32_t(drawing))=0;MEM_W(0x30,int32_t(drawing))=texture;MEM_W(0x40,int32_t(drawing))=native+0x74;rdram=drawmem;
        check(memory==expected,"Render changed memory outside borrowed draw references");
        editor_objects::prepare_release(rdram,child);check(MEM_HU(0,int32_t(item))==0x401D,"Black alias did not route through custom color pool");
        editor_objects::restore(rdram,child);check(editor_objects::children.contains(child),"Graphics cleanup lost logical wrapper binding");
        recomp_context release{};release.r7=int32_t(child);check(editor_objects::release_color(rdram,&release)==6+i,"Graphics cleanup lost release color");
        check(uint32_t(MEM_W(8,int32_t(drawing)))==0x80064000 && uint32_t(MEM_W(0x30,int32_t(drawing)))==0x80065000 && uint32_t(MEM_W(0x40,int32_t(drawing)))==wrapper+0x74,"Disposal left shared egg references attached");
        editor_objects::paint(rdram,child);check(uint32_t(MEM_W(8,int32_t(drawing)))==palette+(6+i)*512,"Reconstructed drawing lost custom color");
        editor_objects::dispose(rdram,child);check(!editor_objects::children.contains(child) && !editor_objects::release_color(rdram,&release),"Full actor disposal retained wrapper binding");
        check(uint32_t(MEM_W(0x30,int32_t(drawing)))==0x80065000,"Full actor disposal did not restore borrowed texture");
    }
    recomp_context reused{};reused.r4=int32_t(0x80010000);reused.r5=0x41CE;editor_objects::begin_child(&reused);reused.r4=int32_t(0x80010400);reused.r5=0x41CE2000;editor_objects::created(rdram,&reused,editor_objects::current_room);check(editor_objects::children.contains(0x80010400),"Reuse fixture wrapper not bound");
    const auto beforeReuse=memory;reused.r5=0x403A2000;editor_objects::created(rdram,&reused,editor_objects::current_room);check(memory==beforeReuse && !editor_objects::children.contains(0x80010400),"Actor reset retained old wrapper binding");
    editor_objects::spawned(0x80074000,8);const auto beforeForget=memory;editor_objects::forget(0x80074000);check(memory==beforeForget && !editor_objects::egg_state(0x80074000,4),"Fast free touched RAM or retained egg");
    const auto ordinary=0x80073000,item=0x80073400;MEM_W(0x1F4,int32_t(ordinary))=item;MEM_H(0,int32_t(item))=0x401C;const auto guarded=memory;
    editor_objects::prepare_release(rdram,ordinary);editor_objects::restore(rdram,ordinary);editor_objects::paint(rdram,ordinary);check(memory==guarded,"Ordinary actor entered custom rendering/release branch");
    editor_objects::configure(original,{});check(editor_objects::placements.empty() && editor_objects::eggs.empty(),"No-payload session retained custom bindings");
    check(!editor_objects::source_color(0x80010000) && !editor_objects::egg_state(0x80010800,4),"Ordinary actor entered custom branch");
    std::cout<<"Editor object registry passed: eight colors, exact source binding, wrapper/pool transfer, carried room transition, one native cue with register preservation, zero progress RAM changes, borrowed native texture/palette/geometry and disposal ownership, plus no-payload isolation. Visual palettes still require native runtime proof.\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
