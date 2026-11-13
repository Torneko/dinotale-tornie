target_sources(DinoTale PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/menu_music_host0310.cpp
    ${CMAKE_CURRENT_LIST_DIR}/menu_music_session0310.cpp)
include(${CMAKE_CURRENT_LIST_DIR}/menu_music_native0310.cmake)
get_filename_component(WORKSHOP_PROJECT_ROOT0310 "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
dinotale_add_menu_music_native0310(MusicRender0310 "${WORKSHOP_PROJECT_ROOT0310}" "${DINO_RECOMP_DIR}")
add_dependencies(DinoTale MusicRender0310)
if(WORKSHOP_RUNTIME_OUTPUT)
    set_target_properties(MusicRender0310 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${WORKSHOP_RUNTIME_OUTPUT}")
endif()

target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/custom_session.cpp)
target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/workshop_loader.cpp)
target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/workshop_menu.cpp)
target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/workshop_menu_game.cpp)
target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/achievements.cpp)
target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/collections.cpp)

target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/heart_wallet0283.cpp)
target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/hybrid_mode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/story_hybrid0320.cpp
    ${CMAKE_CURRENT_LIST_DIR}/story_rights0320.cpp
    ${CMAKE_CURRENT_LIST_DIR}/story_hard0330.cpp
    ${CMAKE_CURRENT_LIST_DIR}/story_hard_access0330.cpp
    ${CMAKE_CURRENT_LIST_DIR}/story_hard_rooms0330.cpp
    ${CMAKE_CURRENT_LIST_DIR}/story_hard_bridge0330.cpp)
target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/hybrid_completion0290.cpp)
target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/metal_yoshis0300.cpp)
target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/gold_silver_resources0300.cpp
    ${CMAKE_CURRENT_LIST_DIR}/elemental_resources0355.cpp
    ${CMAKE_CURRENT_LIST_DIR}/unknown_yoshi_resources0355.cpp
    ${CMAKE_CURRENT_LIST_DIR}/elemental_yoshis0355.cpp)
target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/hybrid_finds.cpp)

target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/trial_special_lives0350.cpp)

target_sources(DinoTale PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/bonus_routes0351.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bonus_session0351.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bonus_rooms0351.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bonus_plan0351.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bonus_runtime0352.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bonus_transport0352.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bonus_game0352.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bonus_retry0353.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bonus_authoring0353.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bonus_finales0353.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bonus_factory0353.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bonus_bowser0353.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bonus_exit_policy0353.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bonus_finale_bridge0353.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bonus_finale_entries0353.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bonus_finale_links0353.cpp)
target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/linux_process.cpp)
add_custom_command(TARGET DinoTale POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory
    $<TARGET_FILE_DIR:DinoTale>/assets/workshop-menu
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_LIST_DIR}/../design/menu-principal-maquette-2026-10-26.png
    $<TARGET_FILE_DIR:DinoTale>/assets/workshop-menu/book-approved.png
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_LIST_DIR}/../design/menu-right-approved-2026-11-08.png
    $<TARGET_FILE_DIR:DinoTale>/assets/workshop-menu/book-right-approved.png)
target_include_directories(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(DinoTale PRIVATE ${CMAKE_SOURCE_DIR}/lib/N64ModernRuntime/thirdparty)
target_compile_definitions(DinoTale PRIVATE YOSHI_WORKSHOP)
option(WORKSHOP_NATIVE_COOP "Build isolated native co-op and first-boss diagnostics" OFF)
if(WORKSHOP_NATIVE_COOP)
    if(NOT WORKSHOP_SPECIAL_CONTENT)
        message(FATAL_ERROR "Native co-op prototype requires special content")
    endif()
    target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/coop_native_dispatch.cpp)
    target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/boss_solo_dispatch0240.cpp)
    target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/boss_battle_dispatch0250.cpp)
    if(MSVC)
        # These C-linkage native callbacks can propagate a C++ host exception.
        # Keep per-draw palette RAII active across that boundary, including
        # clang-cl builds whose global /EHsc otherwise assumes C cannot throw.
        set_property(SOURCE ${CMAKE_CURRENT_LIST_DIR}/boss_battle_dispatch0250.cpp
            APPEND PROPERTY COMPILE_OPTIONS /EHs /EHc-)
    endif()
    target_sources(DinoTale PRIVATE ${CMAKE_CURRENT_LIST_DIR}/boss_blargg_dispatch0289.cpp)
    target_compile_definitions(DinoTale PRIVATE WORKSHOP_NATIVE_COOP)
    target_compile_definitions(RecompiledFuncs PRIVATE WORKSHOP_NATIVE_COOP)
endif()
option(WORKSHOP_HYBRID_FINDS "Enable the experimental hybrid egg quest" ON)
if(NOT WORKSHOP_HYBRID_FINDS)
    target_compile_definitions(DinoTale PRIVATE WORKSHOP_DISABLE_HYBRID_FINDS)
endif()
if(WORKSHOP_RUNTIME_OUTPUT)
    set_target_properties(DinoTale PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${WORKSHOP_RUNTIME_OUTPUT}")
endif()
if(WORKSHOP_CONTENT_ROM)
    target_compile_definitions(DinoTale PRIVATE WORKSHOP_CONTENT_BUILD)
    target_compile_definitions(RecompiledFuncs PRIVATE WORKSHOP_CONTENT_BUILD)
    get_filename_component(WORKSHOP_CONTENT_CONFIG_DIR ${WORKSHOP_CONTENT_ROM} DIRECTORY)
    target_include_directories(DinoTale PRIVATE ${WORKSHOP_CONTENT_CONFIG_DIR})
    add_custom_command(TARGET DinoTale POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${WORKSHOP_CONTENT_ROM} $<TARGET_FILE_DIR:DinoTale>/workshop-content.bin)
endif()
if(WORKSHOP_SPECIAL_CONTENT)
    target_compile_definitions(DinoTale PRIVATE WORKSHOP_SPECIAL_CONTENT)
    target_compile_definitions(RecompiledFuncs PRIVATE WORKSHOP_SPECIAL_CONTENT)
endif()
add_executable(WorkshopDataChecks ${CMAKE_CURRENT_LIST_DIR}/workshop_data_checks.cpp)
target_compile_features(WorkshopDataChecks PRIVATE cxx_std_20)

add_executable(FruitResourceChecks ${CMAKE_CURRENT_LIST_DIR}/special_resource_checks.cpp)
target_compile_features(FruitResourceChecks PRIVATE cxx_std_20)

target_include_directories(FruitResourceChecks PRIVATE ${CMAKE_SOURCE_DIR}/lib/N64ModernRuntime/N64Recomp/include)

add_executable(EditorObjectChecks ${CMAKE_CURRENT_LIST_DIR}/editor_object_checks.cpp)
target_compile_features(EditorObjectChecks PRIVATE cxx_std_20)
target_include_directories(EditorObjectChecks PRIVATE ${CMAKE_SOURCE_DIR}/lib/N64ModernRuntime/N64Recomp/include)

add_executable(NavigationRuntimeChecks ${CMAKE_CURRENT_LIST_DIR}/navigation_checks.cpp)
target_compile_features(NavigationRuntimeChecks PRIVATE cxx_std_20)
target_include_directories(NavigationRuntimeChecks PRIVATE ${CMAKE_SOURCE_DIR}/lib/N64ModernRuntime/N64Recomp/include)
