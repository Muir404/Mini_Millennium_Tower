# ==============================
# 设置编译器具体信息
# ==============================

# Step1 设置C++标准
set(CMAKE_CXX_STANDARD 20) # C++20
set(CMAKE_CXX_STANDARD_REQUIRED True) # 必须使用
set(CMAKE_CXX_EXTENSIONS OFF) # 关闭扩展

# Step2 编译选项配置函数
# 用法：setup_compiler_options(目标名称)
function(setup_compiler_options TARGET_NAME)
    if(MSVC)
        # Visual Studio: 启用所有警告 + UTF-8编码支持
        target_compile_options(${TARGET_NAME} PRIVATE /W4 /utf-8)
        # 添加并行编译支持
        add_compile_options(/MP)
        # 不要弹出控制台窗口
        target_link_options(${TARGET_NAME} PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
    elseif(WIN32 AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
        # MinGW/Clang on Windows: 设置UTF-8编码
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic -finput-charset=utf-8 -fexec-charset=utf-8)
    else()
        # Linux/macOS: 标准警告选项
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

