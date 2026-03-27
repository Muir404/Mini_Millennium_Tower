# ======================================================
# CMake 最低版本要求
# ======================================================
cmake_minimum_required(VERSION 3.27.0 FATAL_ERROR)

# ======================================================
# 项目基础配置
# ======================================================
project(TDSDL
    VERSION 1.0.0
    DESCRIPTION "A TOWER DEFENSE GAME BASED ON SDL3"
    LANGUAGES C CXX
)

# C++ 标准配置
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 路径与目标名称配置
set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/prebuilt)
set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

# ======================================================
# LLVM 生态优先配置
# ======================================================
if(NOT CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER "clang" CACHE STRING "C compiler" FORCE)
endif()
if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "C++ compiler" FORCE)
endif()

option(ENABLE_LLD_LINKER "Use LLVM lld linker for faster linking" ON)
option(ENABLE_ASAN "Enable AddressSanitizer for memory debugging" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_LIBCXX "Use libc++ instead of libstdc++" OFF)
option(ENABLE_THINLTO "Enable ThinLTO for Release builds" OFF)
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "🚀 Configuring for LLVM/Clang ecosystem...")
    add_compile_options(-fcolor-diagnostics)

    if(ENABLE_LLD_LINKER)
        include(CheckLinkerFlag)
        check_linker_flag(CXX "-fuse-ld=lld" LINKER_SUPPORTS_LLD)
        if(LINKER_SUPPORTS_LLD)
            message(STATUS "  ✓ Using LLD linker")
            add_link_options(-fuse-ld=lld)
        else()
            message(WARNING "  ✗ LLD linker not found, falling back to default")
        endif()
    endif()

    set(SANITIZER_FLAGS "")
    if(ENABLE_ASAN)
        message(STATUS "  ✓ AddressSanitizer (ASAN) enabled")
        list(APPEND SANITIZER_FLAGS -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls)
    endif()
    if(ENABLE_UBSAN)
        message(STATUS "  ✓ UndefinedBehaviorSanitizer (UBSAN) enabled")
        list(APPEND SANITIZER_FLAGS -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=integer)
    endif()

    if(SANITIZER_FLAGS)
        add_compile_options(${SANITIZER_FLAGS})
        add_link_options(${SANITIZER_FLAGS})
    endif()

    if(ENABLE_LIBCXX)
        message(STATUS "  ✓ Using libc++")
        add_compile_options(-stdlib=libc++)
        add_link_options(-stdlib=libc++)
    endif()

    if(ENABLE_THINLTO AND CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "  ✓ ThinLTO enabled")
        add_compile_options(-flto=thin)
        add_link_options(-flto=thin)
    endif()

    if(ENABLE_CLANG_TIDY)
        find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
        if(CLANG_TIDY_EXE)
            message(STATUS "  ✓ clang-tidy found: ${CLANG_TIDY_EXE}")
            set(CMAKE_CXX_CLANG_TIDY
                "${CLANG_TIDY_EXE}"
                "-checks=*,-llvm-header-guard,-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers,-fuchsia-*"
                "-header-filter=${CMAKE_SOURCE_DIR}/src/.*"
                "--use-color"
            )
        else()
            message(WARNING "  ✗ clang-tidy not found")
        endif()
    endif()
endif()

# ======================================================
# RPATH 运行时库路径配置
# ======================================================
if(APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path;@executable_path/lib;@executable_path/../lib")
    set(CMAKE_BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib;${CMAKE_SOURCE_DIR}/prebuilt/lib")
elseif(UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/lib:$ORIGIN/../lib")
    set(CMAKE_BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib:${CMAKE_SOURCE_DIR}/prebuilt/lib")
endif()

set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()

# ======================================================
# 全局构建选项
# ======================================================
option(BUILD_SHARED_LIBS "依赖库默认编译为动态库" OFF)

# ======================================================
# FetchContent 依赖获取模块配置
# ======================================================
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# ======================================================
# 核心依赖管理宏
# ======================================================
macro(find_or_fetch_dependency DEP_NAME PACKAGE_NAME GIT_REPO GIT_TAG LOCAL_PATH LINK_TYPE)
    message(STATUS "正在处理依赖: ${DEP_NAME}")

    if("${LINK_TYPE}" STREQUAL "STATIC")
        set(_LIB_IS_SHARED OFF)
        set(_LINK_TYPE_STR "静态")
    elseif("${LINK_TYPE}" STREQUAL "SHARED")
        set(_LIB_IS_SHARED ON)
        set(_LINK_TYPE_STR "动态")
    else()
        set(_LIB_IS_SHARED ${BUILD_SHARED_LIBS})
        if(BUILD_SHARED_LIBS)
            set(_LINK_TYPE_STR "动态(全局)")
        else()
            set(_LINK_TYPE_STR "静态(全局)")
        endif()
    endif()

    find_package(${PACKAGE_NAME} QUIET)
    if(${PACKAGE_NAME}_FOUND OR ${DEP_NAME}_FOUND)
        message(STATUS "  ✓ 找到本地安装的 ${PACKAGE_NAME}")
        if(DEFINED ${PACKAGE_NAME}_DIR)
            message(STATUS "     路径: ${${PACKAGE_NAME}_DIR}")
        elseif(DEFINED ${DEP_NAME}_DIR)
            message(STATUS "     路径: ${${DEP_NAME}_DIR}")
        endif()
    else()
        message(STATUS "  ✗ 未找到本地安装，准备从源码构建 [${_LINK_TYPE_STR}]")

        if("${DEP_NAME}" STREQUAL "lua")
            set(LOCAL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/${LOCAL_PATH})
            if(EXISTS ${LOCAL_SOURCE_DIR})
                message(STATUS "  → 正在为 Lua 生成自动构建配置...")
                file(GLOB LUA_SOURCES "${LOCAL_SOURCE_DIR}/*.c")
                list(REMOVE_ITEM LUA_SOURCES "${LOCAL_SOURCE_DIR}/lua.c" "${LOCAL_SOURCE_DIR}/luac.c")
                add_library(lua STATIC ${LUA_SOURCES})
                target_include_directories(lua PUBLIC ${LOCAL_SOURCE_DIR})
                if(EMSCRIPTEN)
                    target_compile_definitions(lua PRIVATE LUA_USE_POSIX)
                elseif(UNIX AND NOT APPLE)
                    target_compile_definitions(lua PRIVATE LUA_USE_LINUX)
                    target_link_libraries(lua PRIVATE dl readline)
                elseif(APPLE)
                    target_compile_definitions(lua PRIVATE LUA_USE_MACOSX)
                elseif(WIN32 AND _LIB_IS_SHARED)
                    target_compile_definitions(lua PRIVATE LUA_BUILD_AS_DLL)
                endif()
            endif()

        elseif("${DEP_NAME}" MATCHES "^SDL" OR "${PACKAGE_NAME}" MATCHES "^SDL")
            set(SDL_SHARED ${_LIB_IS_SHARED} CACHE BOOL "" FORCE)
            set(SDL_STATIC NOT ${_LIB_IS_SHARED} CACHE BOOL "" FORCE)
            set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
            set(SDL_TESTS OFF CACHE BOOL "" FORCE)
            set(SDL_INSTALL_TESTS OFF CACHE BOOL "" FORCE)

            if("${DEP_NAME}" STREQUAL "SDL3_image")
                message(STATUS "     → 为 SDL3_image 禁用 AVIF 格式")
                set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
                set(SDLIMAGE_AVIF_SHARED OFF CACHE BOOL "" FORCE)
                set(SDLIMAGE_AVIF_SAVE OFF CACHE BOOL "" FORCE)
                set(SDLIMAGE_AVIF_VENDORED OFF CACHE BOOL "" FORCE)
                set(SDLIMAGE_DAV1D OFF CACHE BOOL "" FORCE)
                set(SDLIMAGE_AOM OFF CACHE BOOL "" FORCE)
            endif()
        endif()

        if(NOT TARGET ${DEP_NAME} AND NOT TARGET ${PACKAGE_NAME})
            set(LOCAL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/${LOCAL_PATH})
            if(EXISTS ${LOCAL_SOURCE_DIR})
                message(STATUS "  → 使用本地源码: ${LOCAL_SOURCE_DIR}")

                set(_SAVED_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
                set(BUILD_SHARED_LIBS ${_LIB_IS_SHARED} CACHE BOOL "" FORCE)

                add_subdirectory(${LOCAL_SOURCE_DIR} ${CMAKE_BINARY_DIR}/_deps/${DEP_NAME}-build)

                set(BUILD_SHARED_LIBS ${_SAVED_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)
            else()
                message(STATUS "  → 本地源码不存在，在线获取: ${GIT_REPO}")

                string(LENGTH "${GIT_TAG}" TAG_LENGTH)
                string(REGEX MATCH "^[0-9a-f]+$" IS_HEX "${GIT_TAG}")
                if(TAG_LENGTH EQUAL 40 AND IS_HEX)
                    message(STATUS "     版本: commit hash [${GIT_TAG}]，完整克隆")
                    set(USE_SHALLOW FALSE)
                else()
                    message(STATUS "     版本: ${GIT_TAG}，浅克隆")
                    set(USE_SHALLOW TRUE)
                endif()

                set(_SAVED_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
                set(BUILD_SHARED_LIBS ${_LIB_IS_SHARED} CACHE BOOL "" FORCE)

                FetchContent_Declare(${DEP_NAME}
                    GIT_REPOSITORY ${GIT_REPO}
                    GIT_TAG ${GIT_TAG}
                    GIT_SHALLOW ${USE_SHALLOW}
                    GIT_PROGRESS TRUE
                )
                FetchContent_MakeAvailable(${DEP_NAME})

                set(BUILD_SHARED_LIBS ${_SAVED_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)
            endif()
        endif()

        if(TARGET ${DEP_NAME} AND NOT TARGET ${PACKAGE_NAME}::${PACKAGE_NAME})
            message(STATUS "     创建别名: ${PACKAGE_NAME}::${PACKAGE_NAME} -> ${DEP_NAME}")
            add_library(${PACKAGE_NAME}::${PACKAGE_NAME} ALIAS ${DEP_NAME})
        elseif(TARGET ${PACKAGE_NAME} AND NOT TARGET ${PACKAGE_NAME}::${PACKAGE_NAME})
            message(STATUS "     创建别名: ${PACKAGE_NAME}::${PACKAGE_NAME} -> ${PACKAGE_NAME}")
            add_library(${PACKAGE_NAME}::${PACKAGE_NAME} ALIAS ${PACKAGE_NAME})
        endif()
    endif()
endmacro()

# ======================================================
# 项目依赖库声明 (已更新路径为 库名-版本号 格式)
# ======================================================
find_or_fetch_dependency(SDL3 SDL3 "https://github.com/libsdl-org/SDL.git" "release-3.4.0" "external/SDL-release-3.4.0" AUTO)
find_or_fetch_dependency(SDL3_image SDL3_image "https://github.com/libsdl-org/SDL_image.git" "release-3.2.6" "external/SDL_image-release-3.2.6" AUTO)
find_or_fetch_dependency(SDL3_mixer SDL3_mixer "https://github.com/libsdl-org/SDL_mixer.git" "release-3.2.0" "external/SDL_mixer-release-3.2.0" AUTO)
find_or_fetch_dependency(SDL3_ttf SDL3_ttf "https://github.com/libsdl-org/SDL_ttf.git" "release-3.2.2" "external/SDL_ttf-release-3.2.2" AUTO)

find_or_fetch_dependency(glm glm "https://github.com/g-truc/glm.git" "1.0.3" "external/glm-1.0.3" STATIC)
find_or_fetch_dependency(json nlohmann_json "https://github.com/nlohmann/json.git" "v3.12.0" "external/json-v3.12.0" STATIC)
find_or_fetch_dependency(spdlog spdlog "https://github.com/gabime/spdlog.git" "v1.17.0" "external/spdlog-v1.17.0" STATIC)
find_or_fetch_dependency(EnTT EnTT "https://github.com/skypjack/entt.git" "v3.16.0" "external/entt-v3.16.0" STATIC)

find_or_fetch_dependency(lua lua "https://github.com/lua/lua.git" "v5.4.6" "external/lua-v5.4.6" STATIC)
find_or_fetch_dependency(sol2 sol2 "https://github.com/ThePhD/sol2.git" "v3.3.0" "external/sol2-v3.5.0" STATIC)



# ======================================================
# ImGui 配置 (已更新目录为 imgui-v1.92.5-docking)
# ======================================================
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui-v1.92.5-docking)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)

# ======================================================
# 项目源文件列表
# ======================================================
set(SOURCES
    src/main.cpp
    src/engine/audio/audio_player.cpp
    src/engine/core/game_app.cpp
    src/engine/core/time.cpp
    src/engine/core/config.cpp
    src/engine/core/context.cpp
    src/engine/core/game_state.cpp
    src/engine/resource/resource_manager.cpp
    src/engine/resource/texture_manager.cpp
    src/engine/resource/audio_manager.cpp
    src/engine/resource/font_manager.cpp
    src/engine/render/renderer.cpp
    src/engine/render/camera.cpp
    src/engine/render/text_renderer.cpp
    src/engine/input/input_manager.cpp
    src/engine/scene/scene.cpp
    src/engine/scene/scene_manager.cpp
    src/engine/loader/level_loader.cpp
    src/engine/loader/basic_entity_builder.cpp
    src/engine/system/animation_system.cpp
    src/engine/system/render_system.cpp
    src/engine/system/movement_system.cpp
    src/engine/system/ysort_system.cpp
    src/engine/system/audio_system.cpp
    src/engine/ui/ui_manager.cpp
    src/engine/ui/ui_element.cpp
    src/engine/ui/ui_interactive.cpp
    src/engine/ui/ui_panel.cpp
    src/engine/ui/ui_image.cpp
    src/engine/ui/ui_label.cpp
    src/engine/ui/ui_button.cpp
    src/engine/ui/state/ui_normal_state.cpp
    src/engine/ui/state/ui_pressed_state.cpp
    src/engine/ui/state/ui_hover_state.cpp
    src/game/scene/game_scene.cpp
    src/game/scene/title_scene.cpp
    src/game/scene/level_clear_scene.cpp
    src/game/scene/end_scene.cpp
    src/game/system/remove_dead_system.cpp
    src/game/system/followpath_system.cpp
    src/game/system/block_system.cpp
    src/game/system/attack_starter_system.cpp
    src/game/system/animation_state_system.cpp
    src/game/system/orientation_system.cpp
    src/game/system/timer_system.cpp
    src/game/system/set_target_system.cpp
    src/game/system/combat_resolve_system.cpp
    src/game/system/animation_event_system.cpp
    src/game/system/projectile_system.cpp
    src/game/system/effect_system.cpp
    src/game/system/health_bar_system.cpp
    src/game/system/game_rule_system.cpp
    src/game/system/place_unit_system.cpp
    src/game/system/render_range_system.cpp
    src/game/system/debug_ui_system.cpp
    src/game/system/selection_system.cpp
    src/game/system/skill_system.cpp
    src/game/loader/entity_builder_TD.cpp
    src/game/factory/entity_factory.cpp
    src/game/factory/blueprint_manager.cpp
    src/game/data/session_data.cpp
    src/game/data/ui_config.cpp
    src/game/data/level_config.cpp
    src/game/data/lua_combat_calculator.cpp
    src/game/ui/units_portrait_ui.cpp
    src/game/spawner/enemy_spawner.cpp
)

if(WIN32)
    list(APPEND SOURCES resources.rc)
endif()

# ======================================================
# 生成可执行文件
# ======================================================
add_executable(${TARGET} ${SOURCES} ${IMGUI_SOURCES})

# ======================================================
# 链接所有依赖库
# ======================================================
target_link_libraries(${TARGET}
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
    glm::glm
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    EnTT::EnTT
    lua
    sol2::sol2
)

# ======================================================
# 编译器警告与选项配置
# ======================================================
if(MSVC)
    target_compile_options(${TARGET} PRIVATE /W4 /utf-8)
    target_link_options(${TARGET} PRIVATE "/SUBSYSTEM:WINDOWS")
elseif(WIN32 AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic -finput-charset=utf-8 -fexec-charset=utf-8)
else()
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ======================================================
# 资源文件自动复制
# ======================================================
file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/copy_assets_$<CONFIG>.cmake CONTENT "
set(SOURCE_DIR \"${CMAKE_CURRENT_LIST_DIR}/assets\")
set(TARGET_DIR \"$<TARGET_FILE_DIR:${TARGET}>/assets\")

file(GLOB_RECURSE SOURCE_FILES \"\${SOURCE_DIR}/*\")
set(SOURCE_SIZE 0)
foreach(FILE IN LISTS SOURCE_FILES)
    file(SIZE \"\${FILE}\" FILE_SIZE)
    math(EXPR SOURCE_SIZE \"\${SOURCE_SIZE} + \${FILE_SIZE}\")
endforeach()

set(TARGET_SIZE 0)
if(EXISTS \"\${TARGET_DIR}\")
    file(GLOB_RECURSE TARGET_FILES \"\${TARGET_DIR}/*\")
    foreach(FILE IN LISTS TARGET_FILES)
        file(SIZE \"\${FILE}\" FILE_SIZE)
        math(EXPR TARGET_SIZE \"\${TARGET_SIZE} + \${FILE_SIZE}\")
    endforeach()
endif()

if(NOT SOURCE_SIZE EQUAL TARGET_SIZE)
    file(COPY \"\${SOURCE_DIR}/\" DESTINATION \"\${TARGET_DIR}\")
    message(STATUS \"更新资源文件夹 (大小: \${SOURCE_SIZE} 字节)\")
endif()
")

add_custom_command(
    TARGET ${TARGET}
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/copy_assets_$<CONFIG>.cmake
    COMMENT "正在检查并复制游戏资源文件..."
)

# ======================================================
# Windows 平台：DLL 自动复制
# ======================================================
if(WIN32)
    message(STATUS "配置Windows DLL自动检测和复制...")

    if(EXISTS ${CMAKE_SOURCE_DIR}/prebuilt/bin)
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/prebuilt/bin $<TARGET_FILE_DIR:${TARGET}>
            COMMENT "复制预编译DLL到可执行文件目录"
        )
    endif()

    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
        file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/copy_dlls_$<CONFIG>.cmake CONTENT "
set(DLL_LIST \"$<TARGET_RUNTIME_DLLS:${TARGET}>\")
set(TARGET_DIR \"$<TARGET_FILE_DIR:${TARGET}>\")

if(DLL_LIST)
    set(COPIED_COUNT 0)
    set(COPIED_FILES \"\")
    
    foreach(DLL IN LISTS DLL_LIST)
        get_filename_component(DLL_NAME \"\${DLL}\" NAME)
        set(TARGET_DLL \"\${TARGET_DIR}/\${DLL_NAME}\")
        
        set(NEEDS_COPY FALSE)
        if(EXISTS \"\${TARGET_DLL}\")
            file(MD5 \"\${DLL}\" SRC_MD5)
            file(MD5 \"\${TARGET_DLL}\" DST_MD5)
            if(NOT \"\${SRC_MD5}\" STREQUAL \"\${DST_MD5}\")
                set(NEEDS_COPY TRUE)
            endif()
        else()
            set(NEEDS_COPY TRUE)
        endif()
        
        if(NEEDS_COPY)
            execute_process(
                COMMAND \${CMAKE_COMMAND} -E copy_if_different \"\${DLL}\" \"\${TARGET_DIR}\"
                RESULT_VARIABLE COPY_RESULT
                OUTPUT_QUIET
                ERROR_QUIET
            )
            
            if(COPY_RESULT EQUAL 0)
                math(EXPR COPIED_COUNT \"\${COPIED_COUNT} + 1\")
                list(APPEND COPIED_FILES \"  - \${DLL_NAME}\")
            endif()
            
            string(REGEX REPLACE \"\\\\.dll$\" \".pdb\" PDB_FILE \"\${DLL}\")
            if(EXISTS \"\${PDB_FILE}\")
                execute_process(
                    COMMAND \${CMAKE_COMMAND} -E copy_if_different \"\${PDB_FILE}\" \"\${TARGET_DIR}\"
                    OUTPUT_QUIET
                    ERROR_QUIET
                )
            endif()
        endif()
    endforeach()
    
    if(COPIED_COUNT GREATER 0)
        message(STATUS \"更新了 \${COPIED_COUNT} 个运行时DLL:\")
        foreach(FILE IN LISTS COPIED_FILES)
            message(STATUS \"\${FILE}\")
        endforeach()
    endif()
else()
    if(NOT EXISTS \"\${TARGET_DIR}/.dll_check_done\")
        message(STATUS \"未检测到运行时DLL依赖（全静态链接）\")
        file(WRITE \"\${TARGET_DIR}/.dll_check_done\" \"checked\")
    endif()
endif()
")

        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/copy_dlls_$<CONFIG>.cmake
            COMMENT "自动检测和复制运行时DLL"
        )
    else()
        message(WARNING "CMake版本 < 3.21，无法自动复制运行时DLL。")
        message(WARNING "如果使用了动态库，请手动复制DLL到exe目录。")
    endif()
endif()

# ============================================
# Emscripten 特定配置 (强制隔离版)
# ============================================
if(EMSCRIPTEN)
    set_target_properties(${TARGET} PROPERTIES SUFFIX ".html")

    # 链接选项
    target_link_options(${TARGET} PRIVATE
        "-sUSE_ZLIB=1"
        "-sASSERTIONS=1"
        "-SEXIT_RUNTIME=1"
        "-sALLOW_MEMORY_GROWTH=1"
        "--preload-file" "${CMAKE_SOURCE_DIR}/assets@/assets"
        "--shell-file" "${CMAKE_SOURCE_DIR}/cmake/shell_minimal.html"
    )
endif()

# ======================================================
# 配置完成提示
# ======================================================
message(STATUS "")
message(STATUS "==============================================")
message(STATUS "项目配置完成！")
message(STATUS "  目标名称: ${TARGET}")
message(STATUS "  C++标准: C++${CMAKE_CXX_STANDARD}")
if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    message(STATUS "  可执行文件输出: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
else()
    if(CMAKE_CONFIGURATION_TYPES)
        message(STATUS "  可执行文件输出: ${CMAKE_BINARY_DIR}/<配置名称> (如Debug/Release)")
    else()
        message(STATUS "  可执行文件输出: ${CMAKE_BINARY_DIR}")
    endif()
endif()
message(STATUS "  智能依赖获取: 预编译(prebuilt目录) > 系统库 > 本地源码(external目录) > 在线获取")
if(BUILD_SHARED_LIBS)
    message(STATUS "  依赖库默认链接: 动态链接 (Shared)")
else()
    message(STATUS "  依赖库默认链接: 静态链接 (Static)")
endif()
if(WIN32)
    message(STATUS "  运行时库处理: 自动检测并复制所有DLL到exe目录")
    if(MSVC)
        message(STATUS "  字符编码: UTF-8 (/utf-8) - 避免中文乱码")
    endif()
elseif(APPLE)
    message(STATUS "  运行时库处理: 使用RPATH (@executable_path)")
elseif(UNIX)
    message(STATUS "  运行时库处理: 使用RPATH (\$ORIGIN)")
endif()
message(STATUS "==============================================")
message(STATUS "")
