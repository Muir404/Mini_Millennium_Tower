#!/usr/bin/env bash
# run_web.sh

# ====================== 配置区域 ======================
EMSDK_PATH="./external/emsdk"
BUILD_DIR="build_web"
PORT=8000
LOG_FILE="build_web.log"
# 获取 CPU 核心数用于加速编译
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# 导出环境变量关闭颜色转义符，确保日志纯净
export NO_COLOR=1
export CLICOLOR=0

# ====================== 1. 清理旧构建 ======================
echo "♻️  正在清理旧构建文件：rm -rf $BUILD_DIR"
rm -rf "$BUILD_DIR"
> "$LOG_FILE"

echo "=============================================="
echo "  🌐 正在启动 [Web 编译模式]"
echo "  日志文件: ${LOG_FILE}"
echo "  并行任务: ${JOBS}"
echo "=============================================="

# ====================== 2. 加载 Emscripten ======================
if [ -f "$EMSDK_PATH/emsdk_env.sh" ]; then
    echo "[1/4] 正在加载 Emscripten 环境..."
    # 屏蔽 source 时的输出，保持界面整洁
    source "$EMSDK_PATH/emsdk_env.sh" > /dev/null 2>&1
else
    echo "❌ 错误: 未找到 $EMSDK_PATH/emsdk_env.sh"
    exit 1
fi

# ====================== 3. 配置 CMake ======================
echo "[2/4] 正在配置 Web 工程 (emcmake)..."
if ! emcmake cmake -B "$BUILD_DIR" 2>&1 | tee -a "$LOG_FILE"; then
    echo "❌ CMake 配置失败"
    exit 1
fi

# ====================== 4. 构建项目 ======================
echo ""
echo "[3/4] 正在并行编译..."
if ! cmake --build "$BUILD_DIR" -j"$JOBS" 2>&1 | tee -a "$LOG_FILE"; then
    echo "❌ 编译失败"
    exit 1
fi

# ====================== 5. 启动预览服务器 ======================
echo ""
echo "[4/4] 编译成功！准备启动本地服务器..."
echo "=============================================="
echo "🌍 访问地址: http://localhost:$PORT"
echo "💡 提示: 请在浏览器中点击生成的 .html 文件查看效果"
echo "按下 Ctrl+C 可停止服务器"
echo "=============================================="

# 进入构建目录并启动服务器
cd "$BUILD_DIR" || exit
python3 -m http.server "$PORT"