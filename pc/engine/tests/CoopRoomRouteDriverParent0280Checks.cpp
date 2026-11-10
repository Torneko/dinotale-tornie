// Three QA pad decisions only. No native body, profile or GPU execution.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlogical-op-parentheses" // Existing coop_runtime.hpp127.
#include "../coop_room_route_driver0280.hpp"
#pragma clang diagnostic pop
#include <iostream>
#include <stdexcept>
namespace d=workshop::coop_room_route_driver0280;
int main(){
 d::Frame f;f.vi=9600;f.room=81;f.leader=f.player=1;f.enabled=f.bound=f.complete=f.owner_valid=f.pot_valid=f.parent_is_pot=true;f.grounded=false;f.x=6264;f.y=151;f.pot_x=6264;f.pot_y=181;
 auto out=d::decide(f);if(!out.handled||out.buttons!=0x400||out.x!=0||out.y!=-1)throw std::runtime_error("Observed-parent leader requests Down even when F4 is not settled");
 f.player=0;out=d::decide(f);if(!out.handled||out.buttons||out.x!=0||out.y!=0)throw std::runtime_error("Partner with parent remains neutral");
 f.player=1;f.parent_is_pot=false;out=d::decide(f);if(!out.handled||out.buttons!=0x8000||out.x!=0||out.y!=0)throw std::runtime_error("Without parent the existing complete A pulse remains");
 std::cout<<"PASS3 QA parent-pad checks; no native gameplay execution\n";
}
