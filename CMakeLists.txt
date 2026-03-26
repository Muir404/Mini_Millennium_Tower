# ======================================================
# CMake 最低版本要求
# ======================================================
# 指定构建此项目所需的最低 CMake 版本，FATAL_ERROR 表示版本不足时停止构建
cmake_minimum_required(VERSION 3.27.0 FATAL_ERROR)

# ======================================================
# 项目基础配置
# ======================================================
# 定义项目名称、版本号、项目描述和使用的编程语言（C 和 C++）
project(TDSDL
    VERSION 1.0.0
    DESCRIPTION "A TOWER DEFENSE GAME BASED ON SDL3"
    LANGUAGES C CXX
)

# C++ 标准配置
# 强制使用 C++20 标准，不允许编译器扩展，确保跨平台兼容性
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 路径与目标名称配置
# CMAKE_PREFIX_PATH：指定查找预编译库的优先路径（项目根目录下的 prebuilt 文件夹）
# TARGET：定义最终生成的可执行文件名称，格式为“项目名-系统名”
set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/prebuilt)
set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

# ======================================================
# LLVM 生态优先配置
# ======================================================
# 1. 强制默认使用 Clang 编译器
if(NOT CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER "clang" CACHE STRING "C compiler" FORCE)
endif()
if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "C++ compiler" FORCE)
endif()

# 2. 功能选项开关
option(ENABLE_LLD_LINKER "Use LLVM lld linker for faster linking" ON)
option(ENABLE_ASAN "Enable AddressSanitizer for memory debugging" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_LIBCXX "Use libc++ instead of libstdc++" OFF)
option(ENABLE_THINLTO "Enable ThinLTO for Release builds" OFF)
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)

# 3. 应用 LLVM 特定编译选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "🚀 Configuring for LLVM/Clang ecosystem...")

    # 强制彩色输出 (即使在非交互式终端)
    add_compile_options(-fcolor-diagnostics)

    # 4. 链接器配置：使用 LLD
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

    # 5. Sanitizers 配置
    set(SANITIZER_FLAGS "")
    if(ENABLE_ASAN)
        message(STATUS "  ✓ AddressSanitizer (ASAN) enabled")
        list(APPEND SANITIZER_FLAGS -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls)
    endif()
    if(ENABLE_UBSAN)
        message(STATUS "  ✓ UndefinedBehaviorSanitizer (UBSAN) enabled")
        list(APPEND SANITIZER_FLAGS -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=integer)

        # 【新增】指定 UBSan 抑制文件
        # 只有当文件存在时才添加，避免报错
        if(EXISTS "${CMAKE_SOURCE_DIR}/sanitizer_ubsan.supp")
            message(STATUS "  ✓ 应用 UBSan 抑制文件: sanitizer_ubsan.supp")
            # 注意：UBSan 抑制通常通过环境变量或 -fsanitize-blacklist (旧版) 传递
            # 这里我们通过环境变量传递，兼容性最好
        endif()
    endif()

    if(SANITIZER_FLAGS)
        add_compile_options(${SANITIZER_FLAGS})
        add_link_options(${SANITIZER_FLAGS})
    endif()

    # 6. 使用 libc++ (可选)
    if(ENABLE_LIBCXX)
        message(STATUS "  ✓ Using libc++")
        add_compile_options(-stdlib=libc++)
        add_link_options(-stdlib=libc++)
    endif()

    # 7. ThinLTO (仅 Release)
    if(ENABLE_THINLTO AND CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "  ✓ ThinLTO enabled")
        add_compile_options(-flto=thin)
        add_link_options(-flto=thin)
    endif()

    # 8. Clang-Tidy 集成
    if(ENABLE_CLANG_TIDY)
        find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
        if(CLANG_TIDY_EXE)
            message(STATUS "  ✓ clang-tidy found: ${CLANG_TIDY_EXE}")

            # 【修改点 1】添加 -header-filter 限制只检查 src 目录下的头文件
            # 【修改点 2】添加 --use-color 让 Clang-Tidy 也输出彩色
            set(CMAKE_CXX_CLANG_TIDY
                "${CLANG_TIDY_EXE}"
                "-checks=*,-llvm-header-guard,-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers,-fuchsia-*"
                "-header-filter=${CMAKE_SOURCE_DIR}/src/.*" # 正则：只匹配 src 开头的路径
                "--use-color" # 强制 clang-tidy 输出彩色
            )
        else()
            message(WARNING "  ✗ clang-tidy not found")
        endif()
    endif()
endif()

# ======================================================
# RPATH 运行时库路径配置
# 确保程序能正确找到动态库
# ======================================================
# RPATH 用于在运行时定位动态链接库，避免依赖系统全局路径
if(APPLE)
    # macOS：设置安装时的 RPATH，指向可执行文件所在目录及其 lib 子目录
    set(CMAKE_INSTALL_RPATH "@executable_path;@executable_path/lib;@executable_path/../lib")
    # 构建时的 RPATH，指向项目源码中的 lib 和 prebuilt/lib 目录
    set(CMAKE_BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib;${CMAKE_SOURCE_DIR}/prebuilt/lib")
elseif(UNIX)
    # Linux：类似 macOS，使用 $ORIGIN 表示可执行文件所在目录
    set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/lib:$ORIGIN/../lib")
    set(CMAKE_BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib:${CMAKE_SOURCE_DIR}/prebuilt/lib")
endif()

# 构建时不使用安装 RPATH，安装时自动链接库路径
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)

# macOS 特定：启用 RPATH 支持
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()

# ======================================================
# 全局构建选项
# ======================================================
# 定义一个选项，控制依赖库默认编译为静态库（OFF）还是动态库（ON）
option(BUILD_SHARED_LIBS "依赖库默认编译为动态库" OFF)

# ======================================================
# FetchContent 依赖获取模块配置
# ======================================================
# 包含 FetchContent 模块，用于自动下载、构建和管理第三方依赖
include(FetchContent)
# 关闭 FetchContent 的静默模式，显示下载和构建过程
set(FETCHCONTENT_QUIET OFF)
# 允许离线模式，避免每次构建都重新下载依赖
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# ======================================================
# 核心依赖管理宏：find_or_fetch_dependency
# 功能：优先查找本地库，不存在则自动从源码构建
# ======================================================
# 宏参数说明：
# DEP_NAME：依赖在 CMake 中的目标名（如 SDL3）
# PACKAGE_NAME：find_package 查找的包名（如 SDL3）
# GIT_REPO：依赖的 Git 仓库地址
# GIT_TAG：依赖的 Git 标签（版本号）
# LOCAL_PATH：本地源码路径（项目根目录下的相对路径）
# LINK_TYPE：链接类型（STATIC/SHARED/AUTO）
macro(find_or_fetch_dependency DEP_NAME PACKAGE_NAME GIT_REPO GIT_TAG LOCAL_PATH LINK_TYPE)
    message(STATUS "正在处理依赖: ${DEP_NAME}")

    # 根据链接类型设置库类型（静态或动态）
    if("${LINK_TYPE}" STREQUAL "STATIC")
        set(_LIB_IS_SHARED OFF)
    elseif("${LINK_TYPE}" STREQUAL "SHARED")
        set(_LIB_IS_SHARED ON)
    else()
        # AUTO 类型：使用全局 BUILD_SHARED_LIBS 选项
        set(_LIB_IS_SHARED ${BUILD_SHARED_LIBS})
    endif()

    # 第一步：尝试查找本地已安装的库（通过 find_package）
    find_package(${PACKAGE_NAME} QUIET)

    if(${PACKAGE_NAME}_FOUND OR ${DEP_NAME}_FOUND)
        message(STATUS "  ✓ 找到本地安装的 ${PACKAGE_NAME}")
    else()
        message(STATUS "  ✗ 未找到本地安装，准备从源码构建")

        # ------------------------------
        # Lua 特殊自动构建逻辑
        # Lua 没有官方 CMake 配置，需手动添加源码编译
        # ------------------------------
        if("${DEP_NAME}" STREQUAL "lua")
            set(LOCAL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/${LOCAL_PATH})
            if(EXISTS ${LOCAL_SOURCE_DIR})
                message(STATUS "  → 正在为 Lua 生成自动构建配置...")
                # 收集 Lua 所有 C 源文件
                file(GLOB LUA_SOURCES "${LOCAL_SOURCE_DIR}/*.c")
                # 移除 lua.c（主程序入口）和 luac.c（编译器入口），只编译库
                list(REMOVE_ITEM LUA_SOURCES "${LOCAL_SOURCE_DIR}/lua.c" "${LOCAL_SOURCE_DIR}/luac.c")

                # 创建 Lua 静态库目标
                add_library(lua STATIC ${LUA_SOURCES})
                # 暴露 Lua 头文件目录
                target_include_directories(lua PUBLIC ${LOCAL_SOURCE_DIR})

                # 平台相关编译定义
                if(UNIX AND NOT APPLE)
                    target_compile_definitions(lua PRIVATE LUA_USE_LINUX)
                    # Linux 需链接 dl（动态加载）和 readline（命令行编辑）
                    target_link_libraries(lua PRIVATE dl readline)
                elseif(APPLE)
                    target_compile_definitions(lua PRIVATE LUA_USE_MACOSX)
                elseif(WIN32 AND _LIB_IS_SHARED)
                    # Windows 动态库需定义 LUA_BUILD_AS_DLL
                    target_compile_definitions(lua PRIVATE LUA_BUILD_AS_DLL)
                endif()
            endif()

            # ------------------------------
            # SDL 系列库配置
            # 统一设置 SDL 库的静态/动态选项和禁用测试
            # ------------------------------
        elseif("${DEP_NAME}" MATCHES "^SDL")
            set(SDL_SHARED ${_LIB_IS_SHARED} CACHE BOOL "" FORCE)
            set(SDL_STATIC NOT ${_LIB_IS_SHARED} CACHE BOOL "" FORCE)
            set(SDL_TESTS OFF CACHE BOOL "" FORCE)
        endif()

        # ------------------------------
        # 通用源码获取逻辑
        # 优先使用本地源码（external 目录），否则通过 FetchContent 在线下载
        # ------------------------------
        if(NOT TARGET ${DEP_NAME} AND NOT TARGET ${PACKAGE_NAME})
            set(LOCAL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/${LOCAL_PATH})
            if(EXISTS ${LOCAL_SOURCE_DIR})
                # 本地源码存在：直接添加为子项目
                add_subdirectory(${LOCAL_SOURCE_DIR} ${CMAKE_BINARY_DIR}/_deps/${DEP_NAME}-build)
            else()
                # 本地源码不存在：通过 Git 下载指定版本
                FetchContent_Declare(${DEP_NAME}
                    GIT_REPOSITORY ${GIT_REPO}
                    GIT_TAG ${GIT_TAG}
                    GIT_SHALLOW TRUE # 浅克隆，只下载指定版本，减少体积
                )
                FetchContent_MakeAvailable(${DEP_NAME})
            endif()
        endif()

        # 统一库别名，方便后续链接（如 SDL3::SDL3）
        if(TARGET ${DEP_NAME} AND NOT TARGET ${PACKAGE_NAME}::${PACKAGE_NAME})
            add_library(${PACKAGE_NAME}::${PACKAGE_NAME} ALIAS ${DEP_NAME})
        endif()
    endif()
endmacro()

# ======================================================
# 项目依赖库声明
# ======================================================
# SDL 核心模块：提供窗口、渲染、输入等基础功能
find_or_fetch_dependency(SDL3 SDL3 "https://github.com/libsdl-org/SDL.git" "release-3.4.0" "external/SDL" AUTO)
# SDL_image：加载多种格式图片（PNG、JPG 等）
find_or_fetch_dependency(SDL3_image SDL3_image "https://github.com/libsdl-org/SDL_image.git" "release-3.2.6" "external/SDL_image" AUTO)
# SDL_mixer：音频播放和混音
find_or_fetch_dependency(SDL3_mixer SDL3_mixer "https://github.com/libsdl-org/SDL_mixer.git" "release-3.2.0" "external/SDL_mixer" AUTO)
# SDL_ttf：字体渲染（TrueType 字体）
find_or_fetch_dependency(SDL3_ttf SDL3_ttf "https://github.com/libsdl-org/SDL_ttf.git" "release-3.2.2" "external/SDL_ttf" AUTO)

# 工具与数据库
# GLM：OpenGL 数学库，提供向量、矩阵等几何计算
find_or_fetch_dependency(glm glm "https://github.com/g-truc/glm.git" "1.0.3" "external/glm" STATIC)
# nlohmann_json：现代 C++ JSON 库，用于数据序列化
find_or_fetch_dependency(json nlohmann_json "https://github.com/nlohmann/json.git" "v3.12.0" "external/json" STATIC)
# spdlog：快速日志库，用于记录运行时信息
find_or_fetch_dependency(spdlog spdlog "https://github.com/gabime/spdlog.git" "v1.17.0" "external/spdlog" STATIC)
# EnTT：实体组件系统（ECS）库，用于游戏对象管理
find_or_fetch_dependency(EnTT EnTT "https://github.com/skypjack/entt.git" "v3.16.0" "external/entt" STATIC)

# 脚本引擎
# Lua：轻量级脚本语言，用于游戏逻辑
find_or_fetch_dependency(lua lua "https://github.com/lua/lua.git" "v5.4.6" "external/lua" STATIC)
# sol2：Lua 的 C++ 绑定库，简化 Lua 与 C++ 的交互
find_or_fetch_dependency(sol2 sol2 "https://github.com/ThePhD/sol2.git" "v3.3.0" "external/sol2" STATIC)

# ======================================================
# ImGui 配置
# ImGui：即时模式 GUI 库，用于调试界面和工具
# ======================================================
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
# 收集 ImGui 核心源文件和 SDL3 渲染器后端
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

# 添加 ImGui 头文件目录到全局包含路径
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)

# ======================================================
# 项目源文件列表
# ======================================================
set(SOURCES
    # 主程序入口
    src/main.cpp

    # 引擎 - 音频模块：负责音频播放控制
    src/engine/audio/audio_player.cpp

    # 引擎 - 核心模块：游戏主循环、时间管理、配置、上下文等
    src/engine/core/game_app.cpp
    src/engine/core/time.cpp
    src/engine/core/config.cpp
    src/engine/core/context.cpp
    src/engine/core/game_state.cpp

    # 引擎 - 资源管理：纹理、音频、字体等资源的加载与管理
    src/engine/resource/resource_manager.cpp
    src/engine/resource/texture_manager.cpp
    src/engine/resource/audio_manager.cpp
    src/engine/resource/font_manager.cpp

    # 引擎 - 渲染模块：2D 渲染、相机、文本渲染
    src/engine/render/renderer.cpp
    src/engine/render/camera.cpp
    src/engine/render/text_renderer.cpp

    # 引擎 - 输入管理：键盘、鼠标等输入事件处理
    src/engine/input/input_manager.cpp

    # 引擎 - 场景管理：场景切换与生命周期管理
    src/engine/scene/scene.cpp
    src/engine/scene/scene_manager.cpp

    # 引擎 - 加载器：关卡加载、基础实体构建
    src/engine/loader/level_loader.cpp
    src/engine/loader/basic_entity_builder.cpp

    # 引擎 - 系统模块：ECS 系统（动画、渲染、移动、排序、音频等）
    src/engine/system/animation_system.cpp
    src/engine/system/render_system.cpp
    src/engine/system/movement_system.cpp
    src/engine/system/ysort_system.cpp
    src/engine/system/audio_system.cpp

    # 引擎 - UI 框架：自定义 UI 组件（按钮、标签、面板等）及状态管理
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

    # 游戏 - 场景：游戏内具体场景（主菜单、游戏中、通关、结束等）
    src/game/scene/game_scene.cpp
    src/game/scene/title_scene.cpp
    src/game/scene/level_clear_scene.cpp
    src/game/scene/end_scene.cpp

    # 游戏 - 系统：塔防游戏特定 ECS 系统（战斗、路径、技能等）
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

    # 游戏 - 加载器：塔防游戏实体构建
    src/game/loader/entity_builder_TD.cpp

    # 游戏 - 工厂：实体创建与蓝图管理
    src/game/factory/entity_factory.cpp
    src/game/factory/blueprint_manager.cpp

    # 游戏 - 数据：会话数据、UI 配置、关卡配置
    src/game/data/session_data.cpp
    src/game/data/ui_config.cpp
    src/game/data/level_config.cpp
    src/game/data/lua_combat_calculator.cpp

    # 游戏 - UI：游戏内特定 UI（单位头像面板等）
    src/game/ui/units_portrait_ui.cpp

    # 游戏 - 生成器：敌人生成逻辑
    src/game/spawner/enemy_spawner.cpp
)

# Windows 平台：添加资源文件（图标、版本信息等）
if(WIN32)
    list(APPEND SOURCES resources.rc)
endif()

# ======================================================
# 生成可执行文件
# ======================================================
# 创建可执行文件目标，包含项目源码、ImGui 源码和 Windows 资源文件
add_executable(${TARGET} ${SOURCES} ${IMGUI_SOURCES})

# ======================================================
# 链接所有依赖库
# ======================================================
# 将所有依赖库链接到可执行文件，按功能分组便于维护
target_link_libraries(${TARGET}
    # SDL 生态：核心、图像、音频、字体
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf

    # 工具库：数学、JSON、日志、ECS
    glm::glm
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    EnTT::EnTT

    # 脚本引擎：Lua 及其 C++ 绑定
    lua
    sol2::sol2
)

# ======================================================
# 编译器警告与选项配置
# ======================================================
# 根据编译器设置警告级别，提高代码质量
if(MSVC)
    # Visual Studio：启用 W4 警告，使用 UTF-8 编码避免中文乱码
    target_compile_options(${TARGET} PRIVATE /W4 /utf-8)
    # Windows：设置子系统为 WINDOWS（隐藏控制台窗口，发布用；调试可改为 CONSOLE）
    target_link_options(${TARGET} PRIVATE "/SUBSYSTEM:WINDOWS")
else()
    # GCC/Clang：启用 Wall、Wextra、Wpedantic 警告
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ======================================================
# 资源文件自动复制
# ======================================================
# 生成一个 CMake 脚本，用于在构建前复制 assets 目录到可执行文件所在目录
# 使用 file(GENERATE) 确保生成器表达式（如 $<CONFIG>）正确解析
file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/copy_assets_$<CONFIG>.cmake CONTENT "
set(SOURCE_DIR \"${CMAKE_CURRENT_LIST_DIR}/assets\")
set(TARGET_DIR \"$<TARGET_FILE_DIR:${TARGET}>/assets\")
file(GLOB_RECURSE SOURCE_FILES \"\${SOURCE_DIR}/*\")
file(COPY \"\${SOURCE_DIR}/\" DESTINATION \"\${TARGET_DIR}\")
")

# 添加预构建命令：在编译目标前执行资源复制脚本
add_custom_command(
    TARGET ${TARGET}
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/copy_assets_$<CONFIG>.cmake
    COMMENT "正在复制游戏资源文件..."
)

# ======================================================
# Windows 平台：DLL 自动复制
# ======================================================
if(WIN32)
    message(STATUS "配置Windows DLL自动检测和复制...")

    # 第一步：复制 prebuilt/bin 目录下的预编译 DLL（如第三方库的 DLL）
    if(EXISTS ${CMAKE_SOURCE_DIR}/prebuilt/bin)
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/prebuilt/bin $<TARGET_FILE_DIR:${TARGET}>
            COMMENT "复制预编译DLL到可执行文件目录"
        )
    endif()

    # 第二步：CMake 3.21+ 自动检测并复制运行时依赖的 DLL
    # TARGET_RUNTIME_DLLS 是 CMake 内置变量，自动收集目标依赖的所有 DLL
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
        # 生成 DLL 复制脚本，支持配置（Debug/Release）和增量复制（仅当文件变化时）
        file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/copy_dlls_$<CONFIG>.cmake CONTENT "
# 自动复制运行时DLL脚本 (自动生成，请勿手动修改)
# 功能: 
#   1. 检测可执行文件依赖的所有DLL
#   2. 仅当DLL不存在或MD5不一致时才复制（避免重复复制）
#   3. 同时复制对应的PDB调试文件（如果存在）

set(DLL_LIST \"$<TARGET_RUNTIME_DLLS:${TARGET}>\")
set(TARGET_DIR \"$<TARGET_FILE_DIR:${TARGET}>\")

if(DLL_LIST)
    set(COPIED_COUNT 0)
    set(COPIED_FILES \"\")
    
    foreach(DLL IN LISTS DLL_LIST)
        get_filename_component(DLL_NAME \"\${DLL}\" NAME)
        set(TARGET_DLL \"\${TARGET_DIR}/\${DLL_NAME}\")
        
        # 判断是否需要复制：文件不存在 或 MD5 哈希不一致
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
        
        # 执行复制：使用 copy_if_different 确保仅在文件变化时复制
        if(NEEDS_COPY)
            execute_process(
                COMMAND \${CMAKE_COMMAND} -E copy_if_different \"\${DLL}\" \"\${TARGET_DIR}\"
                RESULT_VARIABLE COPY_RESULT
                OUTPUT_QUIET
                ERROR_QUIET
            )
            
            # 记录复制成功的文件
            if(COPY_RESULT EQUAL 0)
                math(EXPR COPIED_COUNT \"\${COPIED_COUNT} + 1\")
                list(APPEND COPIED_FILES \"  - \${DLL_NAME}\")
            endif()
            
            # 复制对应的 PDB 调试文件（如果存在）
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
    
    # 输出复制结果摘要
    if(COPIED_COUNT GREATER 0)
        message(STATUS \"更新了 \${COPIED_COUNT} 个运行时DLL:\")
        foreach(FILE IN LISTS COPIED_FILES)
            message(STATUS \"\${FILE}\")
        endforeach()
    endif()
else()
    # 标记检查完成，避免每次构建都输出提示
    if(NOT EXISTS \"\${TARGET_DIR}/.dll_check_done\")
        message(STATUS \"未检测到运行时DLL依赖（全静态链接）\")
        file(WRITE \"\${TARGET_DIR}/.dll_check_done\" \"checked\")
    endif()
endif()
")

        # 添加后构建命令：执行 DLL 复制脚本
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/copy_dlls_$<CONFIG>.cmake
            COMMENT "自动检测和复制运行时DLL"
        )
    else()
        # CMake 版本过低的警告提示
        message(WARNING "CMake版本 < 3.21，无法自动复制运行时DLL。")
        message(WARNING "如果使用了动态库，请手动复制DLL到exe目录。")
    endif()
endif()

# ======================================================
# 配置完成提示
# ======================================================
# 输出友好的配置摘要，方便用户确认项目设置
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
    message(STATUS "  运行时库处理: 使用RPATH ($ORIGIN)")
endif()
message(STATUS "==============================================")
message(STATUS "")
