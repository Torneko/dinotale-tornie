#pragma once
#include "bonus_finales0353.hpp"
#include "boss_fish_green0260.hpp"
#include "boss_fish_purple0260.hpp"
#include "boss_fish_flash0260.hpp"
#include <algorithm>
#include <array>
#include <map>
#include <vector>
namespace workshop::bonus_fish0354 {
namespace n=boss_native;
using Color=bonus_finales0353::FishColor;
// A private draw stream; room ordering and later arena RNG are unaffected.
// The two-fish base is red/green, as in the current native Boss variant.
// Each member has five percent total probability of one of the other colors.
inline std::array<Color,2> colors(uint64_t seed){
 auto draw=[&](unsigned bound){seed+=0x9E3779B97F4A7C15ull;auto z=seed;z=(z^(z>>30))*0xBF58476D1CE4E5B9ull;z=(z^(z>>27))*0x94D049BB133111EBull;return unsigned((z^(z>>31))%bound);};
 seed^=0x7B0354F15A6D924Cull;std::array<Color,2> result{Color::Red,Color::Green};
 for(auto& c:result)if(draw(100)<5)c=Color((unsigned(c)+1+draw(2))%3);return result;
}
struct Frame{uint64_t epoch=0;unsigned room=177,vi=0;bool combat=false;};
class Adapter {
 struct Binding{uint32_t actor=0,private_data=0;uint64_t birth=0;unsigned hits=0,last_hit_vi=0;bool initialized=false,defeated=false,vertical=false,jump_started=false;};
 struct Paint{uint32_t actor=0,resource=0,source=0,replacement=0,drawing=0,drawing_source=0;uint64_t birth=0;boss_fish_green0260::DrawTicket rotation{};};
 std::array<Color,2> colors_{};std::array<Binding,2> members_{};uint64_t epoch_=0;bool enabled_=false;
 std::vector<Paint> paints_;std::map<uint64_t,uint32_t> palettes_;unsigned cursor_=0;
 static constexpr uint32_t palette_begin_=0x807D8000,palette_end_=0x807DC000;
 static bool scope(uint8_t* r,const Frame& f,uint64_t epoch){return r&&epoch&&f.epoch==epoch&&f.room==97&&f.combat&&!n::byte(r,0x800F7EA5)&&n::byte(r,0x800F7EA6)==1;}
 bool live(uint8_t* r,unsigned m,const Frame& f)const{auto* rdram=r;
  if(!enabled_||m>=members_.size()||!scope(r,f,epoch_))return false;const auto& b=members_[m];
  if(!b.birth||!b.initialized||b.defeated||!n::actor(r,b.actor,0x4511)||n::word(r,b.actor+0x1F4)!=b.private_data||!n::range(b.private_data,0x188)||n::byte(r,b.actor+0x17D)!=2||n::byte(r,b.actor+0x18)!=1||n::word(r,b.private_data+0x40))return false;
  const auto resource=n::word(r,b.actor+0x168);if(!n::range(resource,0x48)||resource%4||MEM_HU(0x3C,n::base(resource))!=0x4511)return false;
  const auto factory=n::word(r,resource+0x40);return factory%4==0&&n::range(factory,0x3FF0)&&n::word(r,resource+0x44)==factory+0x35A8;
 }
 std::optional<unsigned> owner(uint8_t* r,uint32_t actor,const Frame& f)const{for(unsigned m=0;m<2;++m)if(members_[m].actor==actor&&live(r,m,f))return m;return {};}
 bool vertical(unsigned m)const{return colors_[m]==Color::Green||(colors_[m]==Color::Purple&&members_[m].vertical);}
 boss_fish_green0260::Frame green(unsigned m,const Frame& f)const{const auto& b=members_[m];return {true,true,f.combat,f.room,b.actor,b.private_data,b.birth};}
 boss_fish_purple0260::Frame purple(unsigned m,const Frame& f)const{const auto& b=members_[m];return {true,true,f.combat,f.room,b.actor,b.private_data,b.birth};}
 static uint16_t tint(uint16_t v,Color color){
  const unsigned r=(v>>11)&31,g=(v>>6)&31,b=(v>>1)&31,hi=std::max({r,g,b}),lo=std::min({r,g,b});if(hi<5||hi-lo<5||color==Color::Red)return v;
  return color==Color::Green?uint16_t((g<<11)|(hi<<6)|(b<<1)|(v&1)):uint16_t((hi<<11)|(((lo+hi)/3)<<6)|(hi<<1)|(v&1));
 }
public:
 void configure(std::array<Color,2> colors,uint64_t epoch){reset();colors_=colors;epoch_=epoch;enabled_=epoch!=0&&std::all_of(colors.begin(),colors.end(),[](Color c){return unsigned(c)<=unsigned(Color::Purple);});}
 void reset(){colors_={};members_={};epoch_=0;enabled_=false;paints_.clear();palettes_.clear();cursor_=0;}
 void synchronize(const bonus_finales0353::Snapshot& s,unsigned vi){
  if(!enabled_||s.epoch!=epoch_)return;if(!s.enabled||s.failed||s.bonus!=4||s.room!=97||s.expected!=2){enabled_=false;return;}
  for(unsigned m=0;m<2;++m){const auto& source=s.members[m];auto& b=members_[m];
   if(source.actor!=b.actor||source.birth!=b.birth){b={};b.actor=source.actor;b.birth=source.birth;}
   if(source.hits>b.hits)b.last_hit_vi=vi;b.hits=source.hits;b.private_data=source.private_data;b.initialized=source.initialized;b.defeated=source.defeated;
  }
 }
 void forget(uint32_t actor,uint64_t birth){for(auto& b:members_)if(b.actor==actor&&b.birth==birth)b={};}
 void trace(uint8_t* r,const recomp_context* c,std::string_view name,const Frame& f){auto* rdram=r;
  if(!c||!enabled_||!scope(r,f,epoch_))return;uint32_t actor=uint32_t(c->r4);if(name=="func_80048F84"){if(actor<0xC4)return;actor-=0xC4;}
  const auto m=owner(r,actor,f);if(!m)return;auto& b=members_[*m];
  if(name=="func_80048E30"){
   // Preserve the native always-visible bounds for this exact live birth;
   // no position, collision extents, resource or AI state is changed.
   MEM_H(0xC0,n::base(actor))=0x7FFF;MEM_H(0xC2,n::base(actor))=0x7FFF;return;
  }
  if(name=="func_805C9D10"){
   if(colors_[*m]==Color::Purple&&b.jump_started&&boss_fish_purple0260::completes_jump(r,c,purple(*m,f))){b.vertical=!b.vertical;b.jump_started=false;}return;
  }
  if(name=="func_800483B8"){
   if(vertical(*m)){if(boss_fish_green0260::admitted(r,green(*m,f)))b.jump_started=true;}
   else if(colors_[*m]==Color::Purple){if(boss_fish_purple0260::admitted(r,purple(*m,f)))b.jump_started=true;boss_fish_purple0260::before_integrate(r,c,purple(*m,f));}return;
  }
  if(name=="func_80048F84"){
   if(vertical(*m))boss_fish_green0260::before_axis(r,c,green(*m,f));else if(colors_[*m]==Color::Purple)boss_fish_purple0260::before_axis(r,c,purple(*m,f));
  }
 }
 void leave(uint8_t* r,const recomp_context* c,std::string_view name,const Frame& f){if(!c||name!="func_805C74C8")return;const auto m=owner(r,uint32_t(c->r4),f);if(m&&vertical(*m))boss_fish_green0260::after_target(r,c,green(*m,f));}
 bool paint_begin(uint8_t* r,uint32_t actor,bool outer,const Frame& f){auto* rdram=r;
  if(paints_.size()>=16)return false;const auto m=owner(r,actor,f);if(!m)return false;const auto& b=members_[*m];
  const auto flash=b.hits&&boss_fish_flash0260::alternate(f.vi-b.last_hit_vi);const auto color=colors_[*m];if(color==Color::Red&&!flash)return false;
  auto rotation=outer&&vertical(*m)?boss_fish_green0260::draw_begin(r,green(*m,f)):boss_fish_green0260::DrawTicket{};
  auto angle_only=[&](){if(!rotation.active)return false;paints_.push_back({actor,0,0,0,0,0,b.birth,rotation});return true;};
  const auto resource=n::word(r,actor+0x168),drawing=n::word(r,actor+0x16C);if(!n::range(resource,0x90)||!n::range(drawing,12)||resource%4||drawing%4)return angle_only();
  const auto source=n::word(r,resource+0x88);if(!n::range(source,512)||source%4||(source>=palette_begin_&&source<palette_end_))return angle_only();
  const uint64_t key=(uint64_t(source)<<3)|(unsigned(color)<<1)|unsigned(flash);auto it=palettes_.find(key);uint32_t replacement=0;
  if(it!=palettes_.end())replacement=it->second;else{if(cursor_>=32)return angle_only();replacement=palette_begin_+512*cursor_++;palettes_.emplace(key,replacement);}if(replacement+512>palette_end_)return angle_only();
  for(unsigned i=0;i<256;++i){const auto original=uint16_t(MEM_HU(int(i*2),n::base(source)));MEM_H(int(i*2),n::base(replacement))=boss_fish_flash0260::color(tint(original,color),flash);}
  paints_.push_back({actor,resource,source,replacement,drawing,n::word(r,drawing+8),b.birth,rotation});MEM_W(0x88,n::base(resource))=replacement;MEM_W(8,n::base(drawing))=replacement;return true;
 }
 void paint_finish(uint8_t* r){auto* rdram=r;
  if(!r||paints_.empty())return;const auto p=paints_.back();paints_.pop_back();
  const auto current=std::find_if(members_.begin(),members_.end(),[&](const auto& b){return b.actor==p.actor&&b.birth==p.birth;});if(current==members_.end()||!n::actor(r,p.actor,0x4511))return;
  if(p.resource&&(n::word(r,p.actor+0x168)!=p.resource||n::word(r,p.actor+0x16C)!=p.drawing))return;
  if(p.resource&&n::range(p.resource,0x90)&&n::word(r,p.resource+0x88)==p.replacement)MEM_W(0x88,n::base(p.resource))=p.source;
  if(p.drawing&&n::range(p.drawing,12)&&n::word(r,p.drawing+8)==p.replacement)MEM_W(8,n::base(p.drawing))=p.drawing_source;
  if(p.rotation.active)for(const auto& b:members_)if(b.actor==p.actor&&b.birth==p.birth)boss_fish_green0260::draw_end(r,p.rotation);
 }
 bool vertical_phase(unsigned member)const{return member<2&&members_[member].vertical;}
};
} // namespace workshop::bonus_fish0354
