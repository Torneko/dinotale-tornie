#include "../boss_battle_runtime0250.hpp"
#include <iostream>
#include <vector>
#include <stdexcept>
namespace r=workshop::boss_campaign0250;namespace c=workshop::boss_battle0250;
int main(){unsigned n=0;auto need=[&](bool x){++n;if(!x)throw std::runtime_error("active control baseline changed incorrectly");};std::vector<uint8_t> memory(0x800000);uint8_t* rdram=memory.data();
r::flags_ready=false;r::restore_pending=false;MEM_B(0,int32_t(0x800F7EA5))=2;MEM_B(0,int32_t(0x800F7EA6))=1;auto before=memory;r::remember_controls(rdram,c::Phase::Fighting);need(r::flags_ready&&r::flags5==2&&memory==before);
MEM_B(0,int32_t(0x800F7EA6))=0;MEM_B(0,int32_t(0x800F7EA5))=9;before=memory;r::remember_controls(rdram,c::Phase::Fighting);need(r::flags5==2&&memory==before);
MEM_B(0,int32_t(0x800F7EA6))=1;MEM_B(0,int32_t(0x800F7EA5))=1;before=memory;r::remember_controls(rdram,c::Phase::Fighting);need(r::flags5==1&&r::flags6==1&&memory==before);
r::restore_pending=true;MEM_B(0,int32_t(0x800F7EA5))=2;before=memory;r::remember_controls(rdram,c::Phase::Fighting);need(r::flags5==1&&memory==before);
r::restore_pending=false;for(auto phase:{c::Phase::Idle,c::Phase::Armed,c::Phase::Victory,c::Phase::ToRest,c::Phase::Rest,c::Phase::ToFight,c::Phase::Complete,c::Phase::Failed}){r::remember_controls(rdram,phase);need(r::flags5==1&&memory==before);}std::cout<<n<<" readonly control baseline assertions passed; no native gameplay claim.\n";}
