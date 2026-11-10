#define main bird_injury_regression_main
#include "BossBirdInjury0260Checks.cpp"
#undef main
#include <string_view>
static int32_t sections[512]{};int32_t* section_addresses=sections;
static unsigned lookup_calls=0,retire_calls=0,unexpected_allocations=0;
extern "C" void func_805D3444(uint8_t*,recomp_context*);
extern "C" void func_805D1EDC(uint8_t*,recomp_context*);
extern "C" void func_80046804(uint8_t*,recomp_context*);
extern "C" void func_800464BC(uint8_t*,recomp_context*);
extern "C" void func_80046448(uint8_t*,recomp_context*);
extern "C" void func_800463B4(uint8_t*,recomp_context*);
extern "C" void func_800447C8(uint8_t*,recomp_context*);
extern "C" void func_80044658(uint8_t*,recomp_context*);
extern "C" void func_80057F74(uint8_t*,recomp_context*);
extern "C" void func_80043E88(uint8_t*,recomp_context*);
extern "C" recomp_func_t* get_function(int32_t);
extern "C" void func_80046118(uint8_t*,recomp_context* c){need(uint32_t(c->r4)==0x46DA,"HOST resource lookup is restricted to the actual wounded species");++lookup_calls;c->r2=n::base(Fixture::newres);}
extern "C" void func_80067BBC(uint8_t*,recomp_context* c){c->r2=0;} // Audio only.
extern "C" void func_80067818(uint8_t*,recomp_context*){} // Audio only.
extern "C" void host_particle(uint8_t*,recomp_context*){} // Native3444 visual endpoint.
#define UNREACHED(fn) extern "C" void fn(uint8_t*,recomp_context*){++unexpected_allocations;throw std::runtime_error(#fn " must not execute with full native provider");}
UNREACHED(func_80046014) UNREACHED(func_80045CAC) UNREACHED(func_80045BD8)
UNREACHED(func_80047090) UNREACHED(func_80044194) UNREACHED(func_80052BA4) UNREACHED(func_80048060)
UNREACHED(func_800444D0) UNREACHED(func_80047F9C) UNREACHED(Fault_HungUp)
extern "C" void func_80071380(uint8_t*,recomp_context*){throw std::runtime_error("Unexpected native scheduler fault");}
extern "C" void func_8005FE48(uint8_t*,recomp_context*){} // Collision-mask endpoint, not a lifecycle writer.
// Explicit HOST scheduler endpoints: actual447C8/44658 choose these calls.
static constexpr uint32_t main_manager=0x800F8814,main_table=0x803E0000,pool_manager=0x800F8880,pool_table=0x803E1000;
extern "C" void func_8004457C(uint8_t* rdram,recomp_context* c){const auto a=uint32_t(c->r4);need(a==Fixture::bird,"Native44658 unregisters only the original bird");for(unsigned i=0;i<150;i++)if(v::word(rdram,main_table+i*4)==a)MEM_W(0,n::base(main_table+i*4))=0;MEM_H(0,n::base(a+0x17E))=150;}
extern "C" void func_80043D34(uint8_t*,recomp_context* c){need(uint32_t(c->r4)==0x800F8854&&uint32_t(c->r5)==Fixture::bird,"Native44658 requests its real retirement queue");++retire_calls;c->r2=1;}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#include "D:/Yoshi-Story/pc/build/boss-0281/native-bird-null.inc"
#pragma clang diagnostic pop
extern "C" recomp_func_t* get_function(int32_t a){switch(uint32_t(a)){case 0x80043E88:return func_80043E88;case 0x80044658:return func_80044658;case 0x80001000:return host_particle;default:throw std::runtime_error("Unexpected native indirect call");}}
struct Full:Fixture{
 Full():Fixture(b::Family::Lips,1){
  lookup_calls=retire_calls=unexpected_allocations=0;sections[156]=int32_t(factory);
  w(0x800F8810,main_manager);h(main_manager+8,150);w(main_manager+0x10,main_table);w(main_table+7*4,bird);h(bird+0x17E,7);
  w(0x800F887C,pool_manager);h(pool_manager+8,150);w(pool_manager+0x10,pool_table);w(pool_table+4,factory+0x2A20);
  const auto header=factory+0x2A20,storage=factory+0x36D0;w(header,0);w(header+4,5);w(header+8,storage);
  h(newres+0x3C,0x46DA);w(newres+0x40,factory);w(newres+0x44,factory+0x1B84);
  for(unsigned i=0;i<5;i++){const auto a=storage+i*0x280+4;actor(a,0x46DA);by(a-4,1);by(a+5,i+1);w(a+8,header);w(a+0x168,newres);}
  w(0x800F8908,0x80074000);w(0x80074010,0x80074100);w(0x80074128,0x80001000);
  w(0x800F8950,1234);w(0x800F89D0,7);w(0x800F89D4,4);
 }
 v::Action action(){frame.vi=2;v::frame(rdram,frame);auto a=v::take_actions();need(a.size()==1&&a[0].operation==v::Operation::BirdInjury,"Exactly the owned native injury is requested");return a[0];}
};
int main(){try{
 {Full x;const auto a=x.action();const auto saved=x.c;const auto records=std::vector<uint8_t>(x.mem.begin()+((Fixture::factory+0x36D0)&0x7FFFFF),x.mem.begin()+((Fixture::factory+0x4350)&0x7FFFFF));
  need(v::execute(x.rdram,&x.c,a,[](uint32_t fn,uint8_t* r,recomp_context* c){need(fn==0x805D3444,"The actual original injury executes");func_805D3444(r,c);}),"Native full46DA->NULL->447C8 is deferred rather than ending the campaign");
  need(lookup_calls==1&&retire_calls==1&&!unexpected_allocations&&std::memcmp(&saved,&x.c,sizeof(saved))==0,"Real46804/464BC scans the full provider without any constructor, restoring CPU context");
  need(x.word(Fixture::bird+0x180)==0&&x.word(main_table+28)==0&&v::byte(x.rdram,Fixture::bird+0x18)==0xFF&&v::byte(x.rdram,Fixture::bird+0x17D)==3,"Actual447C8/44658 performs native terminal state writes");
  need(x.word(0x800F8950)==1234&&x.word(0x800F89D0)==7&&x.word(0x800F89D4)==4&&v::take_receipts().empty()&&!workshop::boss_variant_action_probe0281::last,"No score, ring, defeat receipt or fatal probe is invented");
  need(std::equal(records.begin(),records.end(),x.mem.begin()+((Fixture::factory+0x36D0)&0x7FFFFF)),"The five busy wounded records are byte-identical");
  v::action_completed(x.rdram,a,true,2);x.step();need(!v::birds[0].actor&&!v::birds[0].pending&&v::take_actions().empty(),"Existing native-retirement maintenance schedules ordinary replenishment");
 }
 for(unsigned bad=0;bad<12;bad++){Full x;const auto a=x.action();need(v::bird_ready(x.rdram,0),"Exact source identity is captured before negative");const auto entry=uint32_t(x.c.r29)-0xC0;const auto ticket=workshop::boss_bird_null0281::capture(x.rdram,Fixture::bird,a.birth,Fixture::factory,entry);need(ticket.has_value(),"Full known native provider yields an exact pre-call ticket");x.c.r29=n::base(entry);x.c.r4=n::base(Fixture::bird);x.c.r5=n::base(Fixture::pulse);func_805D3444(x.rdram,&x.c);
  switch(bad){case 0:x.w(entry-4,Fixture::wounded);break;case 1:x.by(Fixture::bird+0x18,1);break;case 2:x.by(Fixture::bird+0x17D,2);break;case 3:x.w(main_table+28,Fixture::bird);break;case 4:x.w(Fixture::bird+0x180,Fixture::wounded);break;case 5:x.w(Fixture::bird+0x1F4,Fixture::priv+4);break;case 6:x.w(Fixture::res+0x44,Fixture::factory+0x1B20);break;case 7:x.by(Fixture::factory+0x36D0,0);break;case 8:x.w(pool_table+8,Fixture::factory+0x2A20);break;case 9:x.c.r29+=4;break;case 10:x.w(Fixture::bird+0x188+19*4,Fixture::wounded);break;case 11:x.w(Fixture::factory+0x36D0+4+0x84,1);break;}
  const auto before=x.mem;need(!workshop::boss_bird_null0281::completed(x.rdram,&x.c,*ticket)&&x.mem==before,"NonNULL/stale/resource/manager/pool/slot/graph/stack changes cannot be deferred");
 }
 {Full x;const auto a=x.action();need(!v::execute(x.rdram,&x.c,a,[](uint32_t,uint8_t*,recomp_context*){}),"Unknown native no-op remains a fatal action failure");need(workshop::boss_variant_action_probe0281::last&&workshop::boss_variant_action_probe0281::last->reason=="wounded-bird-successor","The first exact rejected action is recorded");const auto before=x.mem;const auto retained=*workshop::boss_variant_action_probe0281::last;v::execute(x.rdram,&x.c,a,[](uint32_t,uint8_t*,recomp_context*){});need(x.mem==before&&workshop::boss_variant_action_probe0281::last->reason==retained.reason,"First rejection is bounded and read-only");}
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"actualNative\":\"3444/1EDC/46804/464BC/447C8/44658\",\"schedulerAudioResources\":\"HOST endpoints\",\"gameLaunched\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
