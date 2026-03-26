#include "lua_combat_calculator.h"
#include "../../engine/core/time.h"
#include <spdlog/spdlog.h> // 你项目已经在用的日志库

namespace game::data
{

    // 构造函数：初始化Lua环境，加载脚本，记录初始文件修改时间
    LuaCombatCalculator::LuaCombatCalculator(engine::core::Context &ctx)
        : ctx_(ctx)
    {
        // 1. 打开Lua标准库（只开需要的，减少内存占用）
        lua_.open_libraries(
            sol::lib::base, // 基础库
            sol::lib::math  // 数学库（伤害计算需要）
        );

        // 2. 首次加载Lua脚本
        loadScript();

        // 3. 记录脚本文件的初始修改时间
        try
        {
            last_write_time_ = std::filesystem::last_write_time(script_path_);
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            spdlog::error("❌ 无法获取Lua文件修改时间：{}", e.what());
        }
    }

    // 加载/重载Lua脚本
    void LuaCombatCalculator::loadScript()
    {
        try
        {
            // 执行Lua脚本文件，把里面的函数加载到Lua虚拟机内存中
            lua_.script_file(script_path_);
            spdlog::info("✅ 战斗Lua脚本加载成功：{}", script_path_);
        }
        catch (const sol::error &e)
        {
            // 脚本写错了也不会崩溃，打错误日志，用默认公式兜底
            spdlog::error("❌ Lua脚本加载失败：{}", e.what());
        }
    }

    // 检查文件是否被修改
    bool LuaCombatCalculator::isFileModified()
    {
        try
        {
            // 获取当前文件的最后修改时间
            auto current_write_time = std::filesystem::last_write_time(script_path_);
            // 和上次记录的时间对比，不一样说明文件被修改了
            if (current_write_time != last_write_time_)
            {
                last_write_time_ = current_write_time; // 更新时间
                return true;
            }
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            spdlog::error("❌ 检查Lua文件失败：{}", e.what());
        }
        return false;
    }

    void LuaCombatCalculator::checkHotReload()
    {
        // ==============================================
        // 核心修正：用你Time类的 getUnscaledDeltaTime()
        // 为什么用Unscaled？因为游戏暂停/时间缩放时，热更新检查还是要继续
        // ==============================================
        check_timer_ += ctx_.getTime().getUnscaledDeltaTime();

        // 到了3秒检查间隔，才执行检查
        if (check_timer_ >= CHECK_INTERVAL)
        {
            check_timer_ = 0.0f; // 重置计时器
            // 文件被修改了，就重载脚本
            if (isFileModified())
            {
                loadScript();
                spdlog::warn("🔄 战斗公式已自动热更新！新公式立即生效");
            }
        }
    }

    // 核心：调用Lua里的伤害计算函数
    float LuaCombatCalculator::calculateEffectiveDamage(float attacker_atk, float target_def)
    {
        try
        {
            // 直接调用Lua里的calculateEffectiveDamage函数
            return lua_["calculateEffectiveDamage"](attacker_atk, target_def);
        }
        catch (const sol::error &e)
        {
            // Lua函数调用失败（比如函数名写错、脚本语法错），用默认公式兜底，保证游戏不崩溃
            spdlog::error("❌ Lua伤害计算失败，使用默认公式：{}", e.what());
            float default_damage = attacker_atk - target_def;
            return std::max(default_damage, 0.1f * attacker_atk);
        }
    }

} // namespace game::data
