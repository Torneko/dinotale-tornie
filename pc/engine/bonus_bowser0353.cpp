#include "bonus_bowser0353.hpp"
#include <bit>
namespace workshop::bonus_bowser0353 {
namespace {
constexpr gpr at(uint32_t p){return gpr(int32_t(p));}
bool heap(uint32_t p,unsigned n){return !(p&3)&&p>=0x80000400&&p<0x80400000&&n<=0x80400000-p;}
uint32_t word(uint8_t* rdram,uint32_t p){return uint32_t(MEM_W(0,at(p)));}
bool permitted(const Frame& f){return f.pc_mode==7&&f.bonus==6&&f.room>=167&&f.room<=169&&f.epoch&&f.independent_bonus_save&&!f.canonical&&!f.custom_active&&!f.deferred;}
}
void Cadence::reset(){configured_={};controller_.reset();pending_.reset();last_midpoint_.reset();}
bool Cadence::configure(const Frame& f){reset();if(!permitted(f))return false;configured_=f;return true;}
bool Cadence::scope(const Frame& f)const{return permitted(f)&&permitted(configured_)&&f.room==configured_.room&&f.epoch==configured_.epoch&&f.vi>=configured_.vi;}
bool Cadence::live(uint8_t* rdram,const Frame& f)const {
 if(!rdram||!scope(f)||!controller_)return false;const auto& c=*controller_;
 if(!heap(c.actor,0x1FC)||!heap(c.resource,0x48)||!heap(c.private_data,0x24)||c.factory<0x80400000||c.factory>0x80800000-0xB20||!c.birth)return false;
 if(word(rdram,0x800F8B88)!=1||word(rdram,c.actor)!=0x19971110||MEM_HU(12,at(c.actor))!=0x46B1||MEM_BU(0x17D,at(c.actor))==3||word(rdram,c.actor+0x168)!=c.resource||word(rdram,c.actor+0x1F4)!=c.private_data||MEM_HU(0x3C,at(c.resource))!=0x46B1||word(rdram,c.resource+0x40)!=c.factory||word(rdram,c.resource+0x44)!=c.factory+0x938||word(rdram,c.actor+0x20)!=c.factory+0x614||MEM_BU(0x18,at(c.actor))!=1||MEM_BU(0x30,at(c.actor))!=1||word(rdram,c.actor+0x180))return false;
 // The one native controller placement shared by original167/168/169.
 return word(rdram,c.actor+0x3C)==std::bit_cast<uint32_t>(392.0f)&&word(rdram,c.actor+0x40)==std::bit_cast<uint32_t>(344.0f);
}
bool Cadence::bind(uint8_t* rdram,const Controller& c,const Frame& f){if(controller_||!scope(f))return false;controller_=c;if(!live(rdram,f)){controller_.reset();return false;}return true;}
bool Cadence::midpoint_due(uint8_t* rdram,const recomp_context* ctx,const Frame& f){
 pending_.reset();if(!ctx||!live(rdram,f)||uint32_t(ctx->r4)!=controller_->actor||uint32_t(ctx->r16)!=controller_->actor||ctx->r24!=248||word(rdram,controller_->actor+0x1E0)!=247||!heap(uint32_t(ctx->r29),0x68)||(last_midpoint_&&f.vi<=*last_midpoint_))return false;
 pending_=Pending{uint32_t(ctx->r29),f.vi,controller_->birth};return true;
}
bool Cadence::preserve_midpoint(uint8_t* rdram,const recomp_context* ctx,const Frame& f){
 const auto pending=pending_;pending_.reset();if(!pending||!ctx||!live(rdram,f)||pending->birth!=controller_->birth||pending->vi!=f.vi||pending->stack!=uint32_t(ctx->r29)||uint32_t(ctx->r4)!=controller_->actor||uint32_t(ctx->r16)!=controller_->actor||ctx->r24!=248||word(rdram,controller_->actor+0x1E0)!=248)return false;
 last_midpoint_=f.vi;return true;
}
void Cadence::forget(uint32_t actor,uint64_t birth){if(controller_&&controller_->actor==actor&&controller_->birth==birth)reset();}
}
