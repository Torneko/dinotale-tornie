#pragma once
namespace workshop::boss_manual_handoff0260 {
inline constexpr unsigned handoff_vi=2408;
inline constexpr bool live(bool manual,unsigned vi){return manual&&vi>=handoff_vi;}
inline constexpr bool valid(bool manual,bool campaign,bool driver,bool hidden,int encounter,unsigned deadline,unsigned last_script_vi){
 return !manual||(campaign&&!driver&&!hidden&&encounter>=0&&encounter<10&&deadline>=3600&&last_script_vi<=handoff_vi);
}
}
