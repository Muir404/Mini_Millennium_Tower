#!/usr/bin/env bash

# ====================== 配置区域 ======================
EMSDK_PATH="./external/emsdk"
PORT=8000
# 颜色定义
G='\033[0;32m' # 绿
B='\033[0;34m' # 蓝
Y='\033[1;33m' # 黄
R='\033[0;31m' # 红
NC='\033[0m'    # 无色

# 环境变量：确保日志纯净
export NO_COLOR=1
export CLICOLOR=0
REMOVE_COLORS='sed -e "s/\x1B\[[0-9;]*[mGK]//g"'

# ====================== 核心功能 ======================

print_banner() {
    clear
    echo -e "${B}=============================================="
    echo -e "    🎮 Mini Millennium 塔防项目管理工具"
    echo -e "==============================================${NC}"
}

show_menu() {
    print_banner
    echo -e "请选择运行模式:"
    echo -e "  ${G}1)${NC} 开发模式 (Dev)         - 快速编译，无音频"
    echo -e "  ${G}2)${NC} 发布模式 (Release)     - 最高性能优化"
    echo -e "  ${G}3)${NC} 性能分析 (RelWithDbg)  - 包含调试符号"
    echo -e "  ${G}4)${NC} 网页模式 (Web)         - 编译为 HTML/Wasm"
    echo -e "  ${R}q)${NC} 退出"
    echo ""
    read -p "输入数字 [1-4]: " CHOICE
}

execute_task() {
    local MODE_NAME=$1
    local PRESET=$2
    local IS_WEB=$3
    
    # 路径配置
    local BUILD_ROOT="build"
    [ "$IS_WEB" = true ] && BUILD_ROOT="build_web"
    
    local LOG_FILE="build_${PRESET}.log"
    local JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

    echo -e "\n${Y}🚀 正在启动 $MODE_NAME 模式...${NC}"
    
    # 1. 清理
    echo -e "${B}[1/4]${NC} 正在清理旧目录: $BUILD_ROOT"
    rm -rf "$BUILD_ROOT"
    > "$LOG_FILE"

    # 2. 环境准备 (仅 Web)
    if [ "$IS_WEB" = true ]; then
        if [ -f "$EMSDK_PATH/emsdk_env.sh" ]; then
            echo -e "${B}[2/4]${NC} 加载 Emscripten..."
            source "$EMSDK_PATH/emsdk_env.sh" > /dev/null 2>&1
        else
            echo -e "${R}❌ 错误: 未找到 Emscripten SDK ($EMSDK_PATH)${NC}"
            read -p "按回车键继续..."
            return
        fi
    fi

    # 3. 配置与构建
    echo -e "${B}[3/4]${NC} 正在配置与编译 (并行任务: $JOBS)..."
    
    local SUCCESS=0
    if [ "$IS_WEB" = true ]; then
        # Web 编译逻辑
        (emcmake cmake -B "$BUILD_ROOT" && cmake --build "$BUILD_ROOT" -j"$JOBS") 2>&1 | eval $REMOVE_COLORS | tee -a "$LOG_FILE"
        [ ${PIPESTATUS[0]} -eq 0 ] && SUCCESS=1
    else
        # 原生编译逻辑
        (cmake --preset "$PRESET" && cmake --build --preset "$PRESET" -j"$JOBS") 2>&1 | eval $REMOVE_COLORS | tee -a "$LOG_FILE"
        [ ${PIPESTATUS[0]} -eq 0 ] && SUCCESS=1
    fi

    if [ $SUCCESS -ne 1 ]; then
        echo -e "${R}❌ 编译失败！详情请查看: $LOG_FILE${NC}"
        read -p "按回车键返回菜单..."
        return
    fi

    # 4. 运行
    echo -e "${B}[4/4]${NC} 编译成功，准备启动..."
    echo -e "${G}----------------------------------------------${NC}"

    if [ "$IS_WEB" = true ]; then
        echo -e "${Y}🌍 服务器已在构建目录启动: http://localhost:$PORT${NC}"
        echo -e "💡 提示: 请在浏览器中点击生成的 .html 文件"
        cd "$BUILD_ROOT" && python3 -m http.server "$PORT"
    else
        # 严格使用你原来的路径格式
        local BIN="./build/${PRESET}/Mini_Millennium_TowerDefense-Linux"
        
        # 自动处理开发模式的 Dummy 音频
        [ "$PRESET" == "dev" ] && export SDL_AUDIO_DRIVER=dummy
        
        if [ -f "$BIN" ]; then
            chmod +x "$BIN"
            # 运行并同时在控制台和日志输出
            "$BIN" 2>&1 | eval $REMOVE_COLORS | tee -a "$LOG_FILE"
            echo -e "${G}----------------------------------------------${NC}"
            echo -e "退出码: ${PIPESTATUS[0]}"
        else
            echo -e "${R}❌ 找不到可执行文件！${NC}"
            echo -e "预期位置: $BIN"
        fi
    fi
    
    read -p "按回车键返回菜单..."
}

# ====================== 主循环 ======================
while true; do
    show_menu
    case $CHOICE in
        1) execute_task "开发 (Dev)" "dev" false ;;
        2) execute_task "发布 (Release)" "release" false ;;
        3) execute_task "性能分析 (RelWithDbg)" "relwithdbg" false ;;
        4) execute_task "网页 (Web)" "web" true ;;
        q|Q) echo "再见！"; exit 0 ;;
        *) echo -e "${R}无效选项，请重新选择${NC}"; sleep 1 ;;
    esac
done