#pragma once
#include "recomp.h"
#include <cstdint>
#include <array>
namespace workshop::locked_selection_native0355 {
inline bool range(uint32_t a,unsigned n){return !(a&3)&&a>=0x80000400&&a<=0x80800000-n;}
inline bool raw_node(uint8_t* rdram,uint32_t actor){return rdram&&range(actor,0x200)&&uint32_t(MEM_W(0,int32_t(actor)))==0x19971110&&MEM_HU(12,int32_t(actor))==0x4EFB;}
inline bool node(uint8_t* rdram,uint32_t actor){
 if(!raw_node(rdram,actor))return false;const auto parent=uint32_t(MEM_W(0x180,int32_t(actor)));
 return range(parent,0x200)&&uint32_t(MEM_W(0,int32_t(parent)))==0x19971110&&MEM_HU(12,int32_t(parent))==0x41FC;
}
inline bool confirm(uint8_t* rdram,const recomp_context* ctx){
 if(!rdram||!ctx)return false;const auto cursor=uint32_t(ctx->r4);
 return range(cursor,0x200)&&uint32_t(MEM_W(0,int32_t(cursor)))==0x19971110&&MEM_HU(12,int32_t(cursor))==0x41FA&&node(rdram,uint32_t(MEM_W(0x180,int32_t(cursor))));
}
enum class Init {not_question,ready,refused};
// Called only from the real804A0548 constructor, before its prologue. Questions
// receive the original sprite display-list record, never the Yoshi state/name
// machine. The wrapper invokes actual1398 for the native menu scale/Z then
// restores its ABI. A malformed question is deactivated, never retried asYoshi.
inline Init initialize(uint8_t* rdram,recomp_context* ctx,uint32_t overlay57){
 if(!ctx||!raw_node(rdram,uint32_t(ctx->r4)))return Init::not_question;
 const auto actor=int32_t(ctx->r4);const auto draw=uint32_t(MEM_W(0x16C,actor));
 MEM_W(0x1C,actor)=int32_t(0x80065284);MEM_W(0x20,actor)=int32_t(0x80065284);MEM_W(0x2C,actor)=int32_t(0x80065284);MEM_W(0x1F4,actor)=0;
 MEM_B(0x165,actor)=0;MEM_B(0x30,actor)=0;MEM_B(0x31,actor)=0;MEM_H(0x144,actor)=0;MEM_W(0x14C,actor)=0;MEM_W(0x124,actor)=0;MEM_W(0x130,actor)=0;
 if(!range(draw,4)||!range(overlay57,0x31C0)){MEM_B(0x18,actor)=0;return Init::refused;}
 MEM_W(0,int32_t(draw))=int32_t(overlay57+0x2530);MEM_B(0x18,actor)=1;return Init::ready;
}
// The real builder calls this immediately after loading its first child into
// t2, before any White icon/cursor creation. Preserve all slots and positions;
// only stable-order liveYoshis before questions and repair native childindices.
inline bool prepare_children(uint8_t* rdram,recomp_context* ctx){
 if(!rdram||!ctx)return false;ctx->r10=0;const auto root=uint32_t(ctx->r19);
 if(!range(root,0x200)||uint32_t(MEM_W(0,int32_t(root)))!=0x19971110||MEM_HU(12,int32_t(root))!=0x41FC)return false;
 const int count=MEM_H(0x184,int32_t(root));if(count<0||count>20)return false;
 std::array<uint32_t,20> live{},questions{};unsigned nl=0,nq=0;
 for(unsigned i=0;i<20;++i){const auto actor=uint32_t(MEM_W(0,int32_t(root+0x188+i*4)));if(!actor)continue;
  if(!range(actor,0x200)||uint32_t(MEM_W(0,int32_t(actor)))!=0x19971110||uint32_t(MEM_W(0x180,int32_t(actor)))!=root||MEM_HU(0x17E,int32_t(actor))!=i)return false;
  const unsigned kind=MEM_HU(12,int32_t(actor));
  if((kind>=0x41F1&&kind<=0x41F8)||(kind>=0x4EF1&&kind<=0x4EFA)){if(nl>=live.size())return false;live[nl++]=actor;}
  else if(kind==0x4EFB){if(nq>=questions.size())return false;questions[nq++]=actor;}
  else return false;
 }
 if(nl+nq!=unsigned(count))return false;
 unsigned slot=0;for(unsigned j=0;j<nl;++j,++slot){MEM_W(0,int32_t(root+0x188+slot*4))=int32_t(live[j]);MEM_H(0x17E,int32_t(live[j]))=uint16_t(slot);}
 for(unsigned j=0;j<nq;++j,++slot){MEM_W(0,int32_t(root+0x188+slot*4))=int32_t(questions[j]);MEM_H(0x17E,int32_t(questions[j]))=uint16_t(slot);}
 for(;slot<20;++slot)MEM_W(0,int32_t(root+0x188+slot*4))=0;
 if(nl)ctx->r10=gpr(int32_t(live[0]));return true;
}
} // namespace workshop::locked_selection_native0355
