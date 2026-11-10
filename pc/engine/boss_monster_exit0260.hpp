#pragma once
#include "boss_monster_aim0260.hpp"
#include <array>
#include <bit>
#include <optional>

namespace workshop::boss_monster_exit0260 {
namespace n=boss_native;namespace m=boss_monster_aim0260;
enum class Stage {Descend,Approach,Rim};
struct Support {
 uint32_t player=0,pot=0,actor=0,resource=0,metadata=0,definition=0,private_data=0,record=0;
 unsigned room=177,encounter=10;float escape_x=0;
};
struct Route {Stage stage=Stage::Descend;Support support{};float target_x=0,pot_x=0,pot_y=0;bool clear_below=false,support_attested=false;};
inline bool pointer(uint32_t p,unsigned bytes){return !(p&3)&&n::range(p,bytes);}
inline float scalar(uint8_t* ram,uint32_t a){return std::bit_cast<float>(n::word(ram,a));}
inline bool same(const Support& a,const Support& b){return a.player==b.player&&a.pot==b.pot&&a.actor==b.actor&&a.resource==b.resource&&a.metadata==b.metadata&&a.definition==b.definition&&a.private_data==b.private_data&&a.record==b.record&&a.room==b.room&&a.encounter==b.encounter;}
// Only observations are returned. The consumer holds the source identity after
// native detachment and emits ordinary input; no pose/parent/velocity is set.
inline std::optional<Route> observe(uint8_t* ram,uint32_t player,uint32_t pot,unsigned room,unsigned encounter,const Support* previous=nullptr){
 auto* rdram=ram;
 if(!ram||encounter>=9||room!=167+encounter||!n::actor(ram,player,0x4001)||n::word(ram,0x800F8828)!=player||!n::actor(ram,pot,0x4273)||n::byte(ram,pot+0x17D)!=2||n::byte(ram,pot+0x18)!=1||n::byte(ram,player+0x18)!=1||n::byte(ram,player+0x165))return {};
 const auto pp=n::word(ram,player+0x1F4),pm=n::word(ram,player+0x174),potm=n::word(ram,pot+0x174);
 if(!pointer(pp,0x1F8)||!pointer(pm,0x18)||!pointer(potm,4))return {};
 const auto pd=n::word(ram,pm),potd=n::word(ram,potm);if(!pointer(pd,0x80)||!pointer(potd,0x80)||!n::byte(ram,potd+0x74))return {};
 const auto xs=n::word(ram,pd+0x30),ys=n::word(ram,pd+0x34),scale=n::word(ram,pd+0x70);
 if(!pointer(xs,48)||!pointer(ys,48)||!pointer(scale,12))return {};
 std::array<std::array<float,2>,11> probes{};float foot=0,foot_width=0;
 for(unsigned i=1;i<12;++i){const float x=scalar(ram,xs+4*i),y=scalar(ram,ys+4*i);if(!std::isfinite(x)||!std::isfinite(y)||std::abs(x)>32||std::abs(y)>32)return {};probes[i-1]={x,y};if(i>=3&&i<=5){if(y!=16)return {};foot=std::max(foot,y);foot_width=std::max(foot_width,std::abs(x));}}
 if(scalar(ram,xs+12)!=-4||scalar(ram,xs+16)!=0||scalar(ram,xs+20)!=4||foot_width!=4)return {};
 const float x=scalar(ram,player+0x84),y=scalar(ram,player+0x88),px=scalar(ram,pot+0x84),py=scalar(ram,pot+0x88);
 if(!boss_driver0260::point(x,y)||!boss_driver0260::point(px,py))return {};
 const auto map=m::Map::read(ram);if(!map||map->columns!=3||map->rows!=2)return {};
 const auto parent=n::word(ram,player+0x180);
 const auto contact_owner=n::word(ram,pm+4);if(contact_owner&&contact_owner!=parent)return {};
 auto reciprocal=[&](uint32_t owner){const auto slot=uint16_t(MEM_HU(0x17E,n::base(player))),count=uint16_t(MEM_HU(0x184,n::base(owner)));unsigned matches=0;for(unsigned i=0;i<20;++i)matches+=n::word(ram,owner+0x188+4*i)==player;return slot<20&&count>0&&count<=20&&matches==1&&n::word(ram,owner+0x188+4*slot)==player;};
 if(parent==pot){if(!reciprocal(pot))return {};Route out;out.stage=Stage::Rim;out.target_x=out.pot_x=px;out.pot_y=py;out.clear_below=true;return out;}
 // Original151 has exactly these two typed elevator sources. They share
 // their native factory/profile, but neither kind grants the other source X.
 const uint32_t actor=previous?previous->actor:parent;
 const bool left=n::actor(ram,actor,0x415F),right=n::actor(ram,actor,0x415E);const float anchor=left?312.f:472.f;const uint32_t packed=left?0x415F0000u:0x415E0000u;
 if((!left&&!right)||n::byte(ram,actor+0x17D)!=2||n::byte(ram,actor+0x18)!=6||n::byte(ram,actor+0x30)!=0||n::word(ram,actor+0x130)!=0x800483B8)return {};
 if(parent&&parent!=actor)return {};if(parent==actor&&!reciprocal(actor))return {};
 Support witness{player,pot,actor,n::word(ram,actor+0x168),n::word(ram,actor+0x174),0,n::word(ram,actor+0x1F4),n::word(ram,actor+0x1D8),room,encounter,0};
 if(!pointer(witness.resource,0x48)||!pointer(witness.metadata,0x18)||!pointer(witness.private_data,0x18)||!pointer(witness.record,12))return {};
 witness.definition=n::word(ram,witness.metadata);if(!pointer(witness.definition,0x80)||n::word(ram,witness.metadata+4))return {};
 const auto factory=n::word(ram,witness.resource+0x40),ss=n::word(ram,witness.definition+0x70);
 if(!pointer(factory,0xC04)||!pointer(ss,12)||n::word(ram,actor+0x20)!=factory+0xC00||n::word(ram,witness.definition+0x64)!=0x800615D0||n::word(ram,witness.definition+0x68)!=0x800611C0||!n::byte(ram,witness.definition+0x75))return {};
 if(n::word(ram,actor+0x1DC)!=packed||(n::word(ram,witness.record)!=packed&&n::word(ram,witness.record)!=0xFFFFFFFF)||n::word(ram,witness.record+4)!=std::bit_cast<uint32_t>(anchor)||n::word(ram,witness.record+8)!=std::bit_cast<uint32_t>(376.f)||scalar(ram,witness.private_data+4)!=376.f)return {};
 if(previous&&!same(*previous,witness))return {};if(!previous&&(parent!=actor||n::byte(ram,pp+0xF4)))return {};
 if(MEM_H(0x50,n::base(witness.definition))!=0||MEM_H(0x52,n::base(witness.definition))!=4||MEM_H(0x56,n::base(witness.definition))!=24||MEM_H(0x58,n::base(witness.definition))!=12)return {};
 const float ax=scalar(ram,actor+0x84),ay=scalar(ram,actor+0x88),sx=scalar(ram,ss),sy=scalar(ram,ss+4),ps=scalar(ram,scale+4);
 const float half=float(MEM_H(0x56,n::base(witness.definition)))*sx,bottom=ay+float(MEM_H(0x52,n::base(witness.definition)))*sy+float(MEM_H(0x58,n::base(witness.definition)))*sy;
 const float player_top=y+float(MEM_H(6,n::base(pd)))*ps-float(MEM_H(0xC,n::base(pd)))*ps;
 if(!std::isfinite(ax)||!std::isfinite(ay)||!std::isfinite(sx)||!std::isfinite(sy)||!std::isfinite(ps)||sx<=0||sy<=0||ps<=0||half<16||half>64||ax!=anchor||ay<320||ay>432||!std::isfinite(bottom)||!std::isfinite(player_top)||py<=bottom+8)return {};
 auto empty=[&](float qx,int qy)->std::optional<bool>{if(qx<0||qx>=768||qy<0||qy>=512)return {};const auto s=map->occupied(ram,qx,float(qy));if(!s)return {};return !*s;};
 // Find the actual floor under the current pot, never an assumed spawnY.
 int floor=-1;for(int row=int(std::floor(py));row<=int(std::floor(py))+32&&row<511;++row){bool all=true;for(float dx:{-4.f,0.f,4.f}){const auto a=empty(px+dx,row-1),b=empty(px+dx,row),c=empty(px+dx,row+1);if(!a||!b||!c)return {};all=all&&*a&&!*b&&!*c;}if(all){floor=row;break;}}
 if(floor<0)return {};
 auto column=[&](float side)->bool{
  if(side<16||side>752)return false;
  for(float dx:{-4.f,0.f,4.f}){const auto a=empty(side+dx,floor-1),b=empty(side+dx,floor),c=empty(side+dx,floor+1);if(!a||!b||!c||!*a||*b||*c)return false;}
  const float landing=float(floor)-foot+1;
  for(const auto& probe:probes){const int start=int(std::floor(std::min(y,landing)+probe[1])),last=std::min(floor-1,int(std::floor(std::max(y,landing)+probe[1])));for(int yy=start;yy<=last;++yy){const auto clear=empty(side+probe[0],yy);if(!clear||!*clear)return false;}}
  return true;
 };
 // Native615D0/60274 admits a rider by its centreX inside physical halfX.
 // Add the native foot half-width and four pixels of clearance. The full
 // terrain probes above independently reject a side obstructed by a wall.
 const float distance=std::max(44.f,half+foot_width+4);float escape=previous?previous->escape_x:0;
 if(previous){if((escape!=ax+distance&&escape!=ax-distance)||!column(escape))return {};}
 else {for(float direction:{1.f,-1.f}){const float candidate=ax+direction*distance;if(column(candidate)){escape=candidate;break;}}if(!escape)return {};}
 witness.escape_x=escape;Route out;out.support=witness;out.support_attested=true;out.pot_x=px;out.pot_y=py;
 out.clear_below=player_top>bottom+4;out.stage=out.clear_below?Stage::Approach:Stage::Descend;out.target_x=out.clear_below?px:escape;
 return out;
}
}
