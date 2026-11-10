#include "../boss_battle0250.hpp"
#include <iostream>
#include <stdexcept>
using namespace workshop::boss_battle0250;
int main(){unsigned checks=0;auto need=[&](bool x){++checks;if(!x)throw std::runtime_error("diagnostic entry isolation failed");};
for(unsigned index=0;index<10;++index){Session normal;need(normal.begin("normal",make_plan(20261031),false));need(!normal.diagnostic_entry(index));need(normal.snapshot().index==0&&normal.snapshot().completed==0);
Session qa;need(qa.begin("qa",make_plan(20261031),true));need(!qa.diagnostic_entry(10));need(qa.diagnostic_entry(index));need(qa.snapshot().index==index&&qa.snapshot().completed==0);need(qa.enter_fight(room(qa.current().family),0x80010000));need(!qa.diagnostic_entry(0));
for(unsigned n=0;n<qa.current().count;++n){unsigned a=0x80020000+n*0x300;need(qa.bind(n,a));need(qa.defeat({index,n,100+n,a,true,true,true,true}));}
need(qa.snapshot().completed==1&&qa.snapshot().phase==Phase::Victory);need(qa.use_victory_pot()&&qa.snapshot().phase==Phase::ToRest);need(qa.arrive_rest(0x80010000));need(qa.use_rest_pot()==(index<9));need(qa.snapshot().completed==1&&qa.snapshot().phase!=Phase::Complete);}
std::cout<<checks<<" diagnostic boundary assertions passed; synthetic session evidence only.\n";}
