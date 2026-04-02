#!/bin/bash

# 1. 加载 Emscripten 环境 (请根据你的实际路径调整)
EMSDK_PATH="./external/emsdk"
if [ -f "$EMSDK_PATH/emsdk_env.sh" ]; then
    source "$EMSDK_PATH/emsdk_env.sh"
else
    echo "错误: 未找到 emsdk_env.sh，请检查路径。"
    exit 1
fi

# 2. 配置与编译
BUILD_DIR="build_web"

echo "--- 开始配置项目 ---"
emcmake cmake -B $BUILD_DIR --fresh

echo "--- 开始并行编译 ---"
# 使用 $(nproc) 获取 CPU 核心数进行并行编译
cmake --build $BUILD_DIR/ -j$(nproc)

# 3. 启动预览服务器
if [ $? -eq 0 ]; then
    echo "--- 编译成功！启动本地服务器 ---"
    echo "请在浏览器访问: http://localhost:8000"
    echo "注意：请点击 $BUILD_DIR 目录下的 .html 文件"
    
    # 启动服务器并自动进入构建目录
    cd $BUILD_DIR
    python3 -m http.server 8000
else
    echo "--- 编译失败，请检查错误日志 ---"
    exit 1
fi