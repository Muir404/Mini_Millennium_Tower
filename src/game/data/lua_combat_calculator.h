#pragma once
#include <sol/sol.hpp>
#include <string>
#include <filesystem>
// 引入你现成的Context
#include "../../engine/core/context.h"

namespace game::data
{

    /**
     * @brief Lua战斗公式计算器
     * 无单例、无全局对象，通过构造函数注入Context，生命周期由EnTT ctx管理
     * 负责：Lua脚本加载、自动热更检查、伤害计算接口
     */
    class LuaCombatCalculator
    {
    public:
        // 构造函数：必须注入Context，不允许无参构造
        explicit LuaCombatCalculator(engine::core::Context &ctx);
        ~LuaCombatCalculator() = default;

        // 禁止拷贝/移动（保证Lua状态唯一，避免野指针）
        LuaCombatCalculator(const LuaCombatCalculator &) = delete;
        LuaCombatCalculator &operator=(const LuaCombatCalculator &) = delete;
        LuaCombatCalculator(LuaCombatCalculator &&) = delete;
        LuaCombatCalculator &operator=(LuaCombatCalculator &&) = delete;

        // ------------------------------
        // 核心接口
        // ------------------------------
        /**
         * @brief 热更新检查（每帧调用，内部会控制3秒检查一次）
         * 检查Lua文件是否修改，修改则自动重载脚本
         */
        void checkHotReload();

        /**
         * @brief 计算有效伤害（调用Lua里的公式）
         * @param attacker_atk 攻击者攻击力
         * @param target_def 目标防御力
         * @return 最终伤害值
         */
        float calculateEffectiveDamage(float attacker_atk, float target_def);

    private:
        // ------------------------------
        // 私有辅助函数
        // ------------------------------
        // 加载/重载Lua脚本到内存
        void loadScript();
        // 检查文件是否被修改（对比最后修改时间）
        bool isFileModified();

        // ------------------------------
        // 成员变量
        // ------------------------------
        engine::core::Context &ctx_; // 引擎上下文，用你现成的
        sol::state lua_;             // Lua虚拟机状态，每个计算器独占一个，不污染全局

        // 热更新相关配置
        const std::string script_path_ = "assets/data/scripts/combat.lua"; // Lua脚本路径
        std::filesystem::file_time_type last_write_time_;                  // 上次文件修改时间
        float check_timer_ = 0.0f;                                         // 检查计时器
        const float CHECK_INTERVAL = 3.0f;                                 // 检查间隔：3秒
    };

} // namespace game::data
