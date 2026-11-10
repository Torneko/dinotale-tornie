#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <utility>
#include "recompinput/profiles.h"

namespace workshop::coop_manual_input {
// Manual QA only: the base game's SP reader combines keyboard and controllers
// for every port. Read exactly one device profile for each native Yoshi.
// No profile assignment or binding is changed. With the SP backend, all pads
// are aggregated; this first manual test deliberately uses one connected pad.
inline bool permits(recompinput::InputType type,unsigned owner) {
    using Type=recompinput::InputType;
    return owner==0 ? type==Type::Keyboard :
        type==Type::ControllerDigital || type==Type::ControllerAnalog;
}
inline auto bindings(int profile,recompinput::GameInput input,unsigned owner) {
    std::array<recompinput::InputField,recompinput::num_bindings_per_input> result{};
    for(size_t n=0;n<result.size();n++) {
        const auto& field=recompinput::profiles::get_input_binding(profile,input,n);
        if(permits(field.input_type,owner))result[n]=field;
    }
    return result;
}
inline bool read(int player,uint16_t* buttons,float* x,float* y) {
    if(!buttons || !x || !y)return false;
    *buttons=0;*x=0;*y=0;
    if(player<0 || player>1)return false;
    if(recompinput::game_input_disabled())return true;
    using namespace recompinput;
    const unsigned owner=unsigned(player);
    const auto device=owner==0?InputDevice::Keyboard:InputDevice::Controller;
    const int profile=owner==0?profiles::get_sp_keyboard_profile_index():profiles::get_sp_controller_profile_index();
    if(profile<0 || profile>=profiles::get_input_profile_count() || profiles::get_input_profile_device(profile)!=device)return false;
    // Use the frontend's canonical N64 button definitions, including D-pad.
#define DEFINE_INPUT(name,value,readable) std::pair{GameInput::name,uint16_t(value)},
    constexpr auto bits=std::array{DEFINE_N64_BUTTON_INPUTS()};
#undef DEFINE_INPUT
    for(const auto& [input,bit]:bits) {
        const auto fields=bindings(profile,input,owner);
        if(get_input_digital(0,std::span<const InputField>(fields)))*buttons|=bit;
    }
    const auto axis=[&](GameInput input) {
        const auto fields=bindings(profile,input,owner);
        return get_input_analog(0,std::span<const InputField>(fields));
    };
    float ax=axis(GameInput::X_AXIS_POS)-axis(GameInput::X_AXIS_NEG);
    float ay=axis(GameInput::Y_AXIS_POS)-axis(GameInput::Y_AXIS_NEG);
    // The stock reader applies deadzone to controller axes before adding
    // keyboard axes. Each owner here has only that one corresponding device.
    if(owner==1)apply_joystick_deadzone(ax,ay,&ax,&ay);
    *x=std::clamp(ax,-1.0f,1.0f);*y=std::clamp(ay,-1.0f,1.0f);
    return true;
}
}
