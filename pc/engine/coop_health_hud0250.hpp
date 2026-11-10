#pragma once
#include "coop_runtime.hpp"
#include <array>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

namespace workshop::coop_health0250 {
inline constexpr unsigned maximum_health=8;
struct Tint {uint8_t r,g,b,a=255;};
struct Identity {std::string_view french,english;Tint tint;};
// Native order0..7, then the six established hybrid colours8..13.
inline constexpr std::array<Identity,14> identities{{
 {"Vert","Green",{113,202,75}},{"Rouge","Red",{240,80,76}},
 {"Jaune","Yellow",{247,218,73}},{"Bleu","Blue",{81,122,239}},
 {"Bleu pâle","Light blue",{118,208,242}},{"Rose","Pink",{242,133,186}},
 {"Noir","Black",{118,119,136}},{"Blanc","White",{239,241,233}},
 {"Orange","Orange",{244,158,69}},{"Violet","Purple",{169,113,227}},
 {"Turquoise","Turquoise",{75,207,192}},{"Lime-céleri","Celery lime",{188,214,107}},
 {"Brun","Brown",{182,137,98}},{"Vert foncé","Dark green",{76,157,99}}
}};
struct Player {uint32_t actor=0;unsigned color=0,health=0;bool waiting=false;unsigned remaining_vi=0;};
struct Waiting {bool active=false;unsigned color=14,remaining_vi=0;};
struct Snapshot {uint64_t frame=0;bool visible=false;std::array<Player,2> players{};};
inline std::mutex snapshot_mutex;
inline Snapshot latest;
inline void publish(const Snapshot& s){std::lock_guard lock(snapshot_mutex);latest=s;}
inline Snapshot snapshot(){std::lock_guard lock(snapshot_mutex);return latest;}
inline void hide(uint64_t frame=0){publish({frame,false,{}});}
// Called only by the game thread, at its completed owner-scope fence. Read
// both owner values together: the native health array is intentionally swapped
// during P2 updates and cannot provide two independent player health values.
inline std::optional<Snapshot> capture(uint64_t frame,bool two_players,bool world_ready,
 const coop::OwnerContext* owners,uint32_t primary,uint32_t secondary,bool scopes_empty){
 if(!two_players||!world_ready||!owners)return Snapshot{frame,false,{}};
 if(!scopes_empty||owners->depth()!=0)return std::nullopt;
 Snapshot s{frame,true,{}};
 for(unsigned p=0;p<2;p++){
  const auto& owner=owners->player(p);
  if(!owner.actor||owner.actor.address!=(p?secondary:primary)||owner.color>=identities.size()||owner.health>maximum_health)return Snapshot{frame,false,{}};
  s.players[p]={owner.actor.address,owner.color,owner.health};
 }
 if(primary==secondary||!primary||!secondary)return Snapshot{frame,false,{}};
 return s;
}
inline bool publish_owner_frame(uint64_t frame,bool two_players,bool world_ready,
 const coop::OwnerContext* owners,uint32_t primary,uint32_t secondary,bool scopes_empty){
 auto next=capture(frame,two_players,world_ready,owners,primary,secondary,scopes_empty);
 if(!next)return false;publish(*next);return true;
}
// The native lifecycle adapter attests which owner is actually dead. HP0
// alone never enables this path. No dead root is dereferenced or scoped here.
inline std::optional<Snapshot> capture_waiting(uint64_t frame,bool two_players,bool world_ready,
 const coop::OwnerContext* owners,uint32_t primary,uint32_t secondary,bool scopes_empty,
 const std::array<Waiting,2>& waiting){
 if(!waiting[0].active&&!waiting[1].active)return capture(frame,two_players,world_ready,owners,primary,secondary,scopes_empty);
 if(!two_players||!world_ready||!owners||waiting[0].active==waiting[1].active)return Snapshot{frame,false,{}};
 if(!scopes_empty||owners->depth()!=0)return std::nullopt;
 Snapshot s{frame,true,{}};
 for(unsigned p=0;p<2;p++){
  const auto& owner=owners->player(p);
  if(waiting[p].active){
   if(waiting[p].color>=identities.size()||owner.color!=waiting[p].color||waiting[p].remaining_vi>300)return Snapshot{frame,false,{}};
   s.players[p]={0,waiting[p].color,0,true,waiting[p].remaining_vi};
  }else{
   const auto actor=p?secondary:primary;
   if(!actor||!owner.actor||owner.actor.address!=actor||owner.color>=identities.size()||owner.health>maximum_health)return Snapshot{frame,false,{}};
   s.players[p]={actor,owner.color,owner.health};
  }
 }
 return s;
}
inline bool publish_waiting_frame(uint64_t frame,bool two_players,bool world_ready,
 const coop::OwnerContext* owners,uint32_t primary,uint32_t secondary,bool scopes_empty,
 const std::array<Waiting,2>& waiting){
 auto next=capture_waiting(frame,two_players,world_ready,owners,primary,secondary,scopes_empty,waiting);
 if(!next)return false;publish(*next);return true;
}
// The original native flower follows the live singleton while its partner's
// independent flower retires. Put the survivor's label over that left flower.
inline unsigned visual_slot(const Snapshot& s,unsigned p){return s.players[0].waiting&&!s.players[1].waiting?1-p:p;}
inline std::string player_label(const Snapshot& s,unsigned p,bool french){
 if(p>=2||s.players[p].color>=identities.size())return {};
 if(s.players[p].waiting)return std::string(french?"J":"P")+std::to_string(p+1)+(french?"  Retour ":"  Return ")+std::to_string((s.players[p].remaining_vi+59)/60)+"s";
 const auto& id=identities[s.players[p].color];
 return std::string(french?"J":"P")+std::to_string(p+1)+"  "+std::string(french?id.french:id.english);
}
inline std::string health_label(const Snapshot& s,unsigned p){return p<2?std::to_string(s.players[p].health)+"/8":std::string{};}
inline bool filled(const Snapshot& s,unsigned p,unsigned segment){return p<2&&segment<maximum_health&&segment<s.players[p].health;}
}

// The native4010/4011 flowers provide the actual animated health display.
// The UI layer adds player labels only; no substitute health bars or digits.
#ifdef WORKSHOP_COOP_HEALTH_RECOMP_UI
#include "recompui/recompui.h"
namespace workshop::coop_health0250::ui {
using namespace recompui;
inline ContextId context=ContextId::null();
inline std::array<Label*,2> titles{};
inline void create(){
 auto previous=try_close_current_context();context=create_context();context.open();
 context.set_captures_input(false);context.set_captures_mouse(false);
 auto root=context.get_root_element();root->set_pointer_events(PointerEvents::None);
 for(unsigned p=0;p<2;p++){
  titles[p]=context.create_element<Label>(root,"",LabelStyle::Small);
  titles[p]->set_position(Position::Absolute);titles[p]->set_width(196);titles[p]->set_height(28);titles[p]->set_font_size(21);titles[p]->set_font_weight(750);titles[p]->set_pointer_events(PointerEvents::None);
 }
 context.close();if(previous!=ContextId::null())previous.open();
}
inline void frame(bool allow,bool french){
 const auto s=snapshot();if(!allow||!s.visible){if(context!=ContextId::null()&&is_context_shown(context))hide_context(context);return;}
 if(context==ContextId::null())create();auto previous=try_close_current_context();context.open();
 int width=1280,height=720;get_window_size(width,height);const float available=height>0?1080.f*float(width)/float(height):1920.f;
 const float game_width=std::min(available,1440.f),game_height=game_width*.75f,left=(available-game_width)/2,top=(1080.f-game_height)/2;
 for(unsigned p=0;p<2;p++){
  const auto tint=identities[s.players[p].color].tint;
  titles[p]->set_left(left+game_width*(visual_slot(s,p)?260.f:60.f)/320.f-75.f);titles[p]->set_top(top+game_height*87.f/240.f);
  titles[p]->set_text(player_label(s,p,french));titles[p]->set_color(Color{tint.r,tint.g,tint.b,255});
 }
 context.close();if(previous!=ContextId::null())previous.open();if(!is_context_shown(context))show_context(context,"");
}
}
#endif
