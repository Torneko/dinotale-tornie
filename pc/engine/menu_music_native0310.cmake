# Include from the engine CMake configuration. This creates a separate CPU-only
# process; none of its game symbols are linked into the gameplay executable.
function(dinotale_add_menu_music_native0310 target_name project_root)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(mm_dir "${CMAKE_CURRENT_BINARY_DIR}/menu-music-native0310")
    set(mm_script "${project_root}/pc/scripts/verify-menu-music-native0310.py")
    if(ARGC GREATER 2)
        set(mm_native_dir "${ARGV2}")
    elseif(DEFINED DINO_RECOMP_DIR AND NOT DINO_RECOMP_DIR STREQUAL "")
        set(mm_native_dir "${DINO_RECOMP_DIR}")
    else()
        set(mm_native_dir "${project_root}/pc/build/coop-0233-crash-fix/RecompiledFuncs")
    endif()
    file(GLOB mm_native_inputs CONFIGURE_DEPENDS "${mm_native_dir}/funcs_*.c")
    set(mm_runtime "${project_root}/pc/dinotale/lib/N64ModernRuntime")
    add_custom_command(OUTPUT "${mm_dir}/audio_closure.c"
        "${mm_dir}/funcs.h" "${mm_dir}/function_map.inc" "${mm_dir}/native_identity.inc"
        COMMAND "${Python3_EXECUTABLE}" "${mm_script}" --output-dir "${mm_dir}" --native-dir "${mm_native_dir}"
        DEPENDS "${mm_script}" ${mm_native_inputs}
            "${project_root}/pc/engine/menu_music_native0310.cpp"
            "${project_root}/pc/engine/menu_music_native0310.hpp"
            "${project_root}/pc/dinotale/rsp/aspMain.cpp"
            "${mm_runtime}/librecomp/src/rsp.cpp"
            "${mm_runtime}/librecomp/include/librecomp/rsp.hpp"
            "${mm_runtime}/librecomp/include/librecomp/rsp_vu.hpp"
            "${mm_runtime}/librecomp/include/librecomp/rsp_vu_impl.hpp"
            "${mm_runtime}/N64Recomp/include/recomp.h"
        VERBATIM COMMENT "Extracting the CPU-only original title music renderer")
    add_executable(${target_name}
        "${mm_dir}/audio_closure.c" "${mm_dir}/funcs.h"
        "${mm_dir}/function_map.inc" "${mm_dir}/native_identity.inc"
        "${project_root}/pc/engine/menu_music_native0310.cpp"
        "${project_root}/pc/dinotale/rsp/aspMain.cpp"
        "${mm_runtime}/librecomp/src/rsp.cpp")
    target_include_directories(${target_name} PRIVATE "${mm_dir}"
        "${mm_runtime}/N64Recomp/include"
        "${mm_runtime}/librecomp/include" "${mm_runtime}/librecomp/include/librecomp"
        "${mm_runtime}/ultramodern/include")
    target_compile_definitions(${target_name} PRIVATE MENU_MUSIC_NATIVE0310_MAIN)
    target_compile_features(${target_name} PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /O2 /W0 /UWORKSHOP_SPECIAL_CONTENT)
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${target_name} PRIVATE /clang:-msse4.1)
        endif()
    else()
        target_compile_options(${target_name} PRIVATE -O2 -w -msse4.1 -UWORKSHOP_SPECIAL_CONTENT)
    endif()
    set_target_properties(${target_name} PROPERTIES OUTPUT_NAME MusicRender0310)
endfunction()
