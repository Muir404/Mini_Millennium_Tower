#!/usr/bin/env bash
# run_release.sh

# ====================== 先删除旧 build 文件夹 ======================
echo "♻️  正在清理旧构建文件：rm -rf build"
rm -rf build
# ======================================================================

# -------------------------- 配置 --------------------------
PRESET_NAME="release"
LOG_FILE="build_${PRESET_NAME}.log"
export NO_COLOR=1
export CLICOLOR=0

# -------------------------- 欢迎信息 --------------------------
echo "=============================================="
echo "  正在启动 [发布模式] (Release Mode)"
echo "  日志文件: ${LOG_FILE}"
echo "=============================================="

> "$LOG_FILE"

# -------------------------- 1. 配置 CMake --------------------------
echo "[1/3] 正在配置 CMake..."
if ! cmake --preset "$PRESET_NAME" 2>&1 | tee -a "$LOG_FILE"; then
    echo "❌ CMake 配置失败"
    exit 1
fi

# -------------------------- 2. 构建 --------------------------
echo ""
echo "[2/3] 正在构建 (开启 LTO，可能较慢)..."
if ! cmake --build --preset "$PRESET_NAME" 2>&1 | tee -a "$LOG_FILE"; then
    echo "❌ 编译失败"
    exit 1
fi

# -------------------------- 3. 运行 --------------------------
echo ""
echo "[3/3] 正在启动游戏..."
echo "=============================================="
./build/${PRESET_NAME}/TDSDL-Linux 2>&1 | tee -a "$LOG_FILE"

EXIT_CODE=${PIPESTATUS[0]}

echo ""
echo "=============================================="
echo "  游戏已退出，退出码: ${EXIT_CODE}"
echo "=============================================="
exit $EXIT_CODE
