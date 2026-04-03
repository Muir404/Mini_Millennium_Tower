#!/usr/bin/env bash
# run_relwithdbg.sh

# ====================== 先删除旧 build 文件夹 ======================
echo "♻️  正在清理旧构建文件：rm -rf build"
rm -rf build
# ======================================================================

# -------------------------- 配置 --------------------------
PRESET_NAME="relwithdbg"
LOG_FILE="build_${PRESET_NAME}.log"
export NO_COLOR=1
export CLICOLOR=0

# -------------------------- 欢迎信息 --------------------------
echo "=============================================="
echo "  正在启动 [性能分析模式] (RelWithDebInfo)"
echo "  日志文件: ${LOG_FILE}"
echo "  提示: 此模式包含调试符号，可配合 perf 或 gdb 使用"
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
echo "[2/3] 正在构建..."
if ! cmake --build --preset "$PRESET_NAME" 2>&1 | tee -a "$LOG_FILE"; then
    echo "❌ 编译失败"
    exit 1
fi

# -------------------------- 3. 运行 --------------------------
echo ""
echo "[3/3] 正在启动游戏..."
echo "=============================================="
./build/${PRESET_NAME}/Mini_Millennium_TowerDefense-Linux 2>&1 | tee -a "$LOG_FILE"

EXIT_CODE=${PIPESTATUS[0]}

echo ""
echo "=============================================="
echo "  游戏已退出，退出码: ${EXIT_CODE}"
echo "=============================================="
exit $EXIT_CODE
