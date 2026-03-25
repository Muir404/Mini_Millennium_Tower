#!/usr/bin/env bash
# run_dev.sh

# ====================== 先删除旧 build 文件夹 ======================
echo "♻️  正在清理旧构建文件：rm -rf build"
rm -rf build
# ======================================================================

# -------------------------- 配置 --------------------------
PRESET_NAME="dev"
LOG_FILE="build_${PRESET_NAME}.log"
# 强制禁用重定向时的颜色，防止 log.txt 乱码
export NO_COLOR=1
export CLICOLOR=0

# -------------------------- 欢迎信息 --------------------------
echo "=============================================="
echo "  正在启动 [开发模式] (Dev Mode)"
echo "  日志文件: ${LOG_FILE}"
echo "=============================================="

# 清空旧日志
> "$LOG_FILE"

# -------------------------- 1. 配置 CMake --------------------------
echo "[1/3] 正在配置 CMake (--preset ${PRESET_NAME})..."
if ! cmake --preset "$PRESET_NAME" 2>&1 | tee -a "$LOG_FILE"; then
    echo "❌ CMake 配置失败，请查看 ${LOG_FILE}"
    exit 1
fi

# -------------------------- 2. 构建 --------------------------
echo ""
echo "[2/3] 正在构建 (--preset ${PRESET_NAME})..."
if ! cmake --build --preset "$PRESET_NAME" 2>&1 | tee -a "$LOG_FILE"; then
    echo "❌ 编译失败，请查看 ${LOG_FILE}"
    exit 1
fi

# -------------------------- 3. 运行 --------------------------
echo ""
echo "[3/3] 正在启动游戏 (使用 Dummy 音频驱动)..."
echo "=============================================="
# 注意：这里使用 SDL_AUDIO_DRIVER=dummy
SDL_AUDIO_DRIVER=dummy ./build/${PRESET_NAME}/TDSDL-Linux 2>&1 | tee -a "$LOG_FILE"

# 记录退出码
EXIT_CODE=${PIPESTATUS[0]}

echo ""
echo "=============================================="
echo "  游戏已退出，退出码: ${EXIT_CODE}"
echo "  完整日志已保存至: ${LOG_FILE}"
echo "=============================================="

exit $EXIT_CODE
