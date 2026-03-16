# ==============================================================================
# 基础配置 - 项目初始化与编译环境设置
# ==============================================================================
# 指定CMake最低版本要求 (3.27.0+)，确保使用现代CMake特性
cmake_minimum_required(VERSION 3.27.0 FATAL_ERROR)

# 项目基本信息配置
#   - 项目名称: TD
#   - 版本号: 1.0.0
#   - 描述: A TOWER DEFENSE GAME BASED ON SDL3
#   - 支持语言: C/C++
project(TD 
        VERSION 1.0.0 
        DESCRIPTION "A TOWER DEFENSE GAME BASED ON SDL3"
        LANGUAGES C CXX)

# C++标准配置 (强制使用C++20，禁用编译器扩展)
#   - CMAKE_CXX_STANDARD_REQUIRED: 强制要求指定的C++标准，不存在则报错
#   - CMAKE_CXX_EXTENSIONS: 禁用编译器特定扩展（如GNU扩展），保证跨平台兼容性
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# 预编译库搜索路径配置
#   将项目根目录下的prebuilt目录加入CMake搜索路径，优先查找本地预编译依赖
set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/prebuilt)

# 目标名称配置 (自动适配不同系统，格式: 项目名-系统名)
#   例如: TD-Windows / TD-Linux / TD-Darwin(macOS)
set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

# ==============================================================================
# 运行时库路径(RPATH)配置 - 解决动态库加载问题
# ==============================================================================
# RPATH作用: 告诉可执行文件在哪里查找运行时依赖的动态库
# 分为构建时RPATH(CMAKE_BUILD_RPATH)和安装后RPATH(CMAKE_INSTALL_RPATH)

# macOS系统RPATH配置
if(APPLE)
    # 安装后RPATH: 优先查找可执行文件同目录、lib子目录、上级目录的lib子目录
    set(CMAKE_INSTALL_RPATH "@executable_path;@executable_path/lib;@executable_path/../lib")
    # 构建时RPATH: 查找项目根目录的lib和prebuilt/lib
    set(CMAKE_BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib;${CMAKE_SOURCE_DIR}/prebuilt/lib")
# Linux/Unix系统RPATH配置
elseif(UNIX)
    # $ORIGIN: 可执行文件所在目录（Linux特有）
    set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/lib:$ORIGIN/../lib")
    set(CMAKE_BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib:${CMAKE_SOURCE_DIR}/prebuilt/lib")
endif()

# RPATH高级配置
#   - CMAKE_BUILD_WITH_INSTALL_RPATH: 构建时不使用安装后的RPATH
#   - CMAKE_INSTALL_RPATH_USE_LINK_PATH: 自动将链接时的库路径加入安装后RPATH
#   - CMAKE_MACOSX_RPATH: 启用macOS的RPATH功能（macOS特有）
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
if(APPLE)
    set(CMAKE_MACOSX_RPATH TRUE)
endif()

# ==============================================================================
# 项目构建选项
# ==============================================================================
# 全局动态/静态库编译开关 (默认关闭，即默认编译静态库)
#   可通过 cmake -DBUILD_SHARED_LIBS=ON 手动开启
option(BUILD_SHARED_LIBS "依赖库默认编译为动态库" OFF)

# ==============================================================================
# 依赖管理模块 - FetchContent配置
# ==============================================================================
# 引入FetchContent模块，用于从Git仓库自动下载并构建依赖
include(FetchContent)
#   - FETCHCONTENT_QUIET: 关闭静默模式，显示下载和构建进度
#   - FETCHCONTENT_UPDATES_DISCONNECTED: 禁用自动更新，使用指定版本
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# ==============================================================================
# 核心宏定义 - find_or_fetch_dependency
# 功能: 智能查找/获取依赖库，优先级: 系统安装 > 本地源码 > 在线下载
# 参数说明:
#   DEP_NAME: 依赖库名称 (如SDL3)
#   PACKAGE_NAME: CMake查找包名称 (如SDL3)
#   GIT_REPO: Git仓库地址
#   GIT_TAG: Git版本标签/Commit Hash
#   LOCAL_PATH: 本地源码路径 (项目external目录下)
#   LINK_TYPE: 链接类型 (STATIC/SHARED/AUTO)
# ==============================================================================
macro(find_or_fetch_dependency DEP_NAME PACKAGE_NAME GIT_REPO GIT_TAG LOCAL_PATH LINK_TYPE)
    message(STATUS "正在处理依赖: ${DEP_NAME}")
    
    # 解析链接类型参数
    if("${LINK_TYPE}" STREQUAL "STATIC")
        set(_LIB_IS_SHARED OFF)
        set(_LINK_TYPE_STR "静态")
    elseif("${LINK_TYPE}" STREQUAL "SHARED")
        set(_LIB_IS_SHARED ON)
        set(_LINK_TYPE_STR "动态")
    else()
        # AUTO模式: 使用全局BUILD_SHARED_LIBS配置
        set(_LIB_IS_SHARED ${BUILD_SHARED_LIBS})
        if(BUILD_SHARED_LIBS)
            set(_LINK_TYPE_STR "动态(全局)")
        else()
            set(_LINK_TYPE_STR "静态(全局)")
        endif()
    endif()
    
    # 第一步: 尝试查找系统中已安装的依赖库 (QUIET模式不输出错误信息)
    find_package(${PACKAGE_NAME} QUIET)
    
    # 如果找到系统安装的库
    if(${PACKAGE_NAME}_FOUND OR ${DEP_NAME}_FOUND)
        message(STATUS "  ✓ 找到本地安装的 ${PACKAGE_NAME}")
        # 输出库的路径信息 (按优先级依次查找)
        if(DEFINED ${PACKAGE_NAME}_DIR)
            message(STATUS "     路径: ${${PACKAGE_NAME}_DIR}")
        elseif(DEFINED ${DEP_NAME}_DIR)
            message(STATUS "     路径: ${${DEP_NAME}_DIR}")
        elseif(DEFINED ${PACKAGE_NAME}_ROOT)
            message(STATUS "     根目录: ${${PACKAGE_NAME}_ROOT}")
        elseif(DEFINED ${DEP_NAME}_ROOT)
            message(STATUS "     根目录: ${${DEP_NAME}_ROOT}")
        elseif(DEFINED ${PACKAGE_NAME}_INCLUDE_DIRS)
            message(STATUS "     头文件: ${${PACKAGE_NAME}_INCLUDE_DIRS}")
        elseif(DEFINED ${DEP_NAME}_INCLUDE_DIRS)
            message(STATUS "     头文件: ${${DEP_NAME}_INCLUDE_DIRS}")
        endif()
    # 如果未找到系统安装的库，准备从源码构建
    else()
        message(STATUS "  ✗ 未找到本地安装，准备从源码构建 [${_LINK_TYPE_STR}]")
        
        # SDL系列库特殊配置 (强制设置静态/动态编译选项)
        if("${DEP_NAME}" MATCHES "^SDL" OR "${PACKAGE_NAME}" MATCHES "^SDL")
            if(_LIB_IS_SHARED)
                set(SDL_SHARED ON CACHE BOOL "" FORCE)
                set(SDL_STATIC OFF CACHE BOOL "" FORCE)
            else()
                set(SDL_SHARED OFF CACHE BOOL "" FORCE)
                set(SDL_STATIC ON CACHE BOOL "" FORCE)
            endif()
            # 禁用SDL测试相关组件，加快编译速度
            set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
            set(SDL_TESTS OFF CACHE BOOL "" FORCE)
            set(SDL_INSTALL_TESTS OFF CACHE BOOL "" FORCE)
        endif()
        
        # 全局禁用所有依赖库的测试/示例/文档构建
        set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(BUILD_DOCS OFF CACHE BOOL "" FORCE)
        
        # SDL3_image特殊配置: 禁用AVIF相关编解码器 (减少依赖)
        if("${DEP_NAME}" STREQUAL "SDL3_image")
            set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_AVIF_SHARED OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_AVIF_SAVE OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_AVIF_VENDORED OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_DAV1D OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_AOM OFF CACHE BOOL "" FORCE)
        endif()
        
        # 第二步: 检查本地external目录是否有源码
        set(LOCAL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/${LOCAL_PATH})
        if(EXISTS ${LOCAL_SOURCE_DIR})
            message(STATUS "  → 使用本地源码: ${LOCAL_SOURCE_DIR}")
            # 临时保存全局BUILD_SHARED_LIBS配置，避免影响其他依赖
            set(_SAVED_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
            # 强制设置当前依赖的编译类型
            set(BUILD_SHARED_LIBS ${_LIB_IS_SHARED} CACHE BOOL "" FORCE)
            # 添加本地源码目录到构建系统
            add_subdirectory(${LOCAL_SOURCE_DIR} ${CMAKE_BINARY_DIR}/_deps/${DEP_NAME}-build)
            # 恢复全局BUILD_SHARED_LIBS配置
            set(BUILD_SHARED_LIBS ${_SAVED_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)
            
            # 创建标准别名目标 (统一链接方式: PACKAGE_NAME::PACKAGE_NAME)
            if(TARGET ${DEP_NAME} AND NOT TARGET ${PACKAGE_NAME}::${PACKAGE_NAME})
                message(STATUS "     创建别名: ${PACKAGE_NAME}::${PACKAGE_NAME} -> ${DEP_NAME}")
                add_library(${PACKAGE_NAME}::${PACKAGE_NAME} ALIAS ${DEP_NAME})
            elseif(TARGET ${PACKAGE_NAME} AND NOT TARGET ${PACKAGE_NAME}::${PACKAGE_NAME})
                message(STATUS "     创建别名: ${PACKAGE_NAME}::${PACKAGE_NAME} -> ${PACKAGE_NAME}")
                add_library(${PACKAGE_NAME}::${PACKAGE_NAME} ALIAS ${PACKAGE_NAME})
            endif()
        # 第三步: 本地源码不存在，从Git仓库在线下载
        else()
            message(STATUS "  → 本地源码不存在，在线获取: ${GIT_REPO}")
            # 判断Git标签类型 (Commit Hash/普通标签)
            string(LENGTH "${GIT_TAG}" TAG_LENGTH)
            string(REGEX MATCH "^[0-9a-f]+$" IS_HEX "${GIT_TAG}")
            
            # Commit Hash (40位十六进制): 完整克隆 (避免浅克隆失败)
            if(TAG_LENGTH EQUAL 40 AND IS_HEX)
                message(STATUS "     版本: commit hash [${GIT_TAG}]，完整克隆")
                set(USE_SHALLOW FALSE)
            # 普通标签/分支: 浅克隆 (只克隆最新提交，加快下载速度)
            else()
                message(STATUS "     版本: ${GIT_TAG}，浅克隆")
                set(USE_SHALLOW TRUE)
            endif()
            
            # 临时修改全局BUILD_SHARED_LIBS配置
            set(_SAVED_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
            set(BUILD_SHARED_LIBS ${_LIB_IS_SHARED} CACHE BOOL "" FORCE)
            
            # 声明要下载的依赖
            FetchContent_Declare(
                ${DEP_NAME}
                GIT_REPOSITORY ${GIT_REPO}    # Git仓库地址
                GIT_TAG ${GIT_TAG}            # 版本标签/Commit Hash
                GIT_SHALLOW ${USE_SHALLOW}    # 是否浅克隆
                GIT_PROGRESS TRUE             # 显示下载进度
            )
            # 下载并构建依赖
            FetchContent_MakeAvailable(${DEP_NAME})
            
            # 恢复全局BUILD_SHARED_LIBS配置
            set(BUILD_SHARED_LIBS ${_SAVED_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)
        endif()
    endif()
endmacro()

# ==============================================================================
# 依赖库配置列表
# 使用find_or_fetch_dependency宏管理所有第三方依赖
# 格式: find_or_fetch_dependency(内部名 CMake包名 Git仓库 版本 本地路径 链接类型)
# ==============================================================================

# SDL3 (多媒体基础库) - AUTO模式: 跟随全局BUILD_SHARED_LIBS配置
find_or_fetch_dependency(
    SDL3
    SDL3
    "https://github.com/libsdl-org/SDL.git"
    "release-3.4.0"
    "external/SDL-release-3.4.0"
    AUTO
)

# SDL3_image (图片加载库)
find_or_fetch_dependency(
    SDL3_image
    SDL3_image
    "https://github.com/libsdl-org/SDL_image.git"
    "release-3.2.6"
    "external/SDL_image-release-3.2.6"
    AUTO
)

# SDL3_mixer (音频播放库)
find_or_fetch_dependency(
    SDL3_mixer
    SDL3_mixer
    "https://github.com/libsdl-org/SDL_mixer.git"
    "release-3.2.0"
    "external/SDL_mixer-release-3.2.0"
    AUTO
)

# SDL3_ttf (字体渲染库)
find_or_fetch_dependency(
    SDL3_ttf
    SDL3_ttf
    "https://github.com/libsdl-org/SDL_ttf.git"
    "release-3.2.2"
    "external/SDL_ttf-release-3.2.2"
    AUTO
)

# glm (数学库) - 强制静态链接
find_or_fetch_dependency(
    glm
    glm
    "https://github.com/g-truc/glm.git"
    "1.0.3"
    "external/glm-1.0.3"
    STATIC
)

# nlohmann/json (JSON解析库) - 强制静态链接
find_or_fetch_dependency(
    json
    nlohmann_json
    "https://github.com/nlohmann/json.git"
    "v3.12.0"
    "external/json-v3.12.0"
    STATIC
)

# spdlog (日志库) - 强制静态链接
find_or_fetch_dependency(
    spdlog
    spdlog
    "https://github.com/gabime/spdlog.git"
    "v1.17.0"
    "external/spdlog-v1.17.0"
    STATIC
)

# EnTT (ECS实体组件系统) - 强制静态链接
find_or_fetch_dependency(
    EnTT
    EnTT
    "https://github.com/skypjack/entt.git"
    "v3.16.0"
    "external/entt-v3.16.0"
    STATIC
)

# googletest (单元测试框架) - 注释掉，按需启用
# find_or_fetch_dependency(
#     googletest
#     GTest
#     "https://github.com/google/googletest.git"
#     "v1.17.0"
#     "external/googletest-v1.17.0"
#     STATIC
# )

# ==============================================================================
# ImGui配置 (独立处理，非通过find_or_fetch_dependency)
# ==============================================================================
# 指定ImGui源码目录
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui-v1.92.5-docking)
# ImGui核心源码文件列表
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp          # Demo代码，可根据需要移除
    # SDL3+SDLRenderer3后端实现
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

# 添加ImGui头文件搜索路径
include_directories(
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# ==============================================================================
# 源文件配置
# ==============================================================================
# 项目核心源文件列表
set(SOURCES
    # main
    src/main.cpp

    # engine-audio
    src/engine/audio/audio_player.cpp

    # engine-core 
    src/engine/core/game_app.cpp
    src/engine/core/time.cpp
    src/engine/core/config.cpp
    src/engine/core/context.cpp
    src/engine/core/game_state.cpp

    # engine-resource
    src/engine/resource/resource_manager.cpp
    src/engine/resource/texture_manager.cpp
    src/engine/resource/audio_manager.cpp
    src/engine/resource/font_manager.cpp

    # engine-render
    src/engine/render/renderer.cpp
    src/engine/render/camera.cpp
    src/engine/render/text_renderer.cpp

    # engine-input
    src/engine/input/input_manager.cpp

    # engine-scene
    src/engine/scene/scene.cpp
    src/engine/scene/scene_manager.cpp

    # engine-loader
    src/engine/loader/level_loader.cpp
    src/engine/loader/basic_entity_builder.cpp

    # engine-system
    src/engine/system/animation_system.cpp
    src/engine/system/render_system.cpp
    src/engine/system/movement_system.cpp
    src/engine/system/ysort_system.cpp
    src/engine/system/audio_system.cpp

    # engine-ui
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

    # game-scene
    src/game/scene/game_scene.cpp

    # game-system
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

    # game-loader
    src/game/loader/entity_builder_TD.cpp

    # game-factory
    src/game/factory/entity_factory.cpp
    src/game/factory/blueprint_manager.cpp

    # game-data
    src/game/data/session_data.cpp
    src/game/data/ui_config.cpp

    # game-ui
    src/game/ui/units_portrait_ui.cpp
)

# Windows系统特殊配置: 添加资源文件 (图标/版本信息等)
if(WIN32)
    list(APPEND SOURCES resources.rc)
endif()

# ==============================================================================
# 可执行文件构建配置
# ==============================================================================
# 创建可执行文件目标
add_executable(${TARGET} ${SOURCES} ${IMGUI_SOURCES})

# 链接依赖库 (使用别名目标，保证跨平台一致性)
target_link_libraries(${TARGET}
    SDL3::SDL3                  # SDL3核心库
    SDL3_image::SDL3_image      # 图片加载库
    SDL3_mixer::SDL3_mixer      # 音频播放库
    SDL3_ttf::SDL3_ttf          # 字体渲染库
    glm::glm                    # 数学库
    nlohmann_json::nlohmann_json # JSON库
    spdlog::spdlog              # 日志库
    EnTT::EnTT                  # ECS系统
    # GTest::gtest               # 测试库（按需启用）
    # GTest::gtest_main
)

# ==============================================================================
# 编译器选项配置
# ==============================================================================
# MSVC (Visual Studio) 编译器
if(MSVC)
    #   - /W4: 最高级别警告
    #   - /utf-8: 强制使用UTF-8编码，解决中文乱码
    target_compile_options(${TARGET} PRIVATE /W4 /utf-8)
    # /SUBSYSTEM:WINDOWS: 构建Windows窗口程序（无控制台窗口）
    # 如需调试控制台，可改为 /SUBSYSTEM:CONSOLE
    target_link_options(${TARGET} PRIVATE "/SUBSYSTEM:WINDOWS")
# Windows下的GCC/Clang编译器 (MinGW/Cygwin)
elseif(WIN32 AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
    # 启用所有警告，并强制UTF-8编码
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic -finput-charset=utf-8 -fexec-charset=utf-8)
# Linux/macOS等Unix系统
else()
    # 启用所有警告
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# # ==============================================================================
# # 构建前操作 - 资源文件复制
# # ==============================================================================
# # 生成动态资源复制脚本 (按配置生成，支持多配置构建)
# file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/copy_assets_$<CONFIG>.cmake CONTENT "
# # 生成的Assets复制脚本 (自动生成，请勿手动修改)
# # 功能: 仅当资源文件大小变化时才复制，避免每次构建都全量复制

# # 资源源目录和目标目录
# set(SOURCE_DIR \"${CMAKE_CURRENT_LIST_DIR}/assets\")
# set(TARGET_DIR \"$<TARGET_FILE_DIR:${TARGET}>/assets\")

# # 计算源目录所有文件总大小
# file(GLOB_RECURSE SOURCE_FILES \"\${SOURCE_DIR}/*\")
# set(SOURCE_SIZE 0)
# foreach(FILE IN LISTS SOURCE_FILES)
#     file(SIZE \"\${FILE}\" FILE_SIZE)
#     math(EXPR SOURCE_SIZE \"\${SOURCE_SIZE} + \${FILE_SIZE}\")
# endforeach()

# # 计算目标目录所有文件总大小
# set(TARGET_SIZE 0)
# if(EXISTS \"\${TARGET_DIR}\")
#     file(GLOB_RECURSE TARGET_FILES \"\${TARGET_DIR}/*\")
#     foreach(FILE IN LISTS TARGET_FILES)
#         file(SIZE \"\${FILE}\" FILE_SIZE)
#         math(EXPR TARGET_SIZE \"\${TARGET_SIZE} + \${FILE_SIZE}\")
#     endforeach()
# endif()

# # 仅当大小不一致时才复制 (增量更新)
# if(NOT SOURCE_SIZE EQUAL TARGET_SIZE)
#     file(COPY \"\${SOURCE_DIR}/\" DESTINATION \"\${TARGET_DIR}\")
#     message(STATUS \"更新资源文件夹 (大小: \${SOURCE_SIZE} 字节)\")
# endif()
# ")

# # 添加预构建步骤: 执行资源复制脚本
# add_custom_command(TARGET ${TARGET} PRE_BUILD
#     COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/copy_assets_$<CONFIG>.cmake
#     COMMENT "检查并复制资源文件"
# )

# ==============================================================================
# 构建后操作 - Windows DLL自动复制
# ==============================================================================
if(WIN32)
    message(STATUS "配置Windows DLL自动检测和复制...")
    
    # 第一步: 复制prebuilt/bin目录下的预编译DLL
    if(EXISTS ${CMAKE_SOURCE_DIR}/prebuilt/bin)
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/prebuilt/bin $<TARGET_FILE_DIR:${TARGET}>
            COMMENT "复制预编译DLL到可执行文件目录"
        )
    endif()
    
    # 第二步: CMake 3.21+ 自动检测并复制运行时依赖的DLL
    # TARGET_RUNTIME_DLLS: CMake内置的DLL依赖检测功能
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
        file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/copy_dlls_$<CONFIG>.cmake CONTENT "
# 自动复制运行时DLL脚本 (自动生成，请勿手动修改)
# 功能: 
#   1. 检测可执行文件依赖的所有DLL
#   2. 仅当DLL不存在或MD5不一致时才复制
#   3. 同时复制对应的PDB调试文件

set(DLL_LIST \"$<TARGET_RUNTIME_DLLS:${TARGET}>\")
set(TARGET_DIR \"$<TARGET_FILE_DIR:${TARGET}>\")

if(DLL_LIST)
    set(COPIED_COUNT 0)
    set(COPIED_FILES \"\")
    
    foreach(DLL IN LISTS DLL_LIST)
        get_filename_component(DLL_NAME \"\${DLL}\" NAME)
        set(TARGET_DLL \"\${TARGET_DIR}/\${DLL_NAME}\")
        
        # 判断是否需要复制: 文件不存在 或 MD5不一致
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
        
        # 执行复制操作 (copy_if_different: 仅当文件不同时复制)
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
            
            # 复制对应的PDB调试文件 (如果存在)
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
    
    # 输出复制结果
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
        
        # 添加后构建步骤: 执行DLL复制脚本
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/copy_dlls_$<CONFIG>.cmake
            COMMENT "自动检测和复制运行时DLL"
        )
    else()
        # CMake版本过低的警告提示
        message(WARNING "CMake版本 < 3.21，无法自动复制运行时DLL。")
        message(WARNING "如果使用了动态库，请手动复制DLL到exe目录。")
    endif()
endif()

# ==============================================================================
# 项目配置完成提示
# ==============================================================================
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
# message(STATUS "  智能依赖获取: 预编译(prebuilt目录) > 系统库 > 本地源码(external目录) > 在线获取")
# if(BUILD_SHARED_LIBS)
#     message(STATUS "  依赖库默认链接: 动态链接 (Shared)")
# else()
#     message(STATUS "  依赖库默认链接: 静态链接 (Static)")
# endif()
# if(WIN32)
#     message(STATUS "  运行时库处理: 自动检测并复制所有DLL到exe目录")
#     if(MSVC)
#         message(STATUS "  字符编码: UTF-8 (/utf-8) - 避免中文乱码")
#     endif()
# elseif(APPLE)
#     message(STATUS "  运行时库处理: 使用RPATH (@executable_path)")
# elseif(UNIX)
#     message(STATUS "  运行时库处理: 使用RPATH ($ORIGIN)")
# endif()
message(STATUS "==============================================")
message(STATUS "")
