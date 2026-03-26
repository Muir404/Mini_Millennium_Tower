#include "combat_script_loader.h"
#include "engine/resource/lua_manager.h"
#include <spdlog/spdlog.h>

namespace game::script
{
    CombatScriptLoader &CombatScriptLoader::GetInstance()
    {
        static CombatScriptLoader instance;
        return instance;
    }

    void CombatScriptLoader::LoadScript(const std::string &path)
    {
        auto &luaMgr = engine::resource::LuaManager::GetInstance();
        luaMgr.LoadScript(path);

        // 缓存Lua函数指针
        auto &lua = luaMgr.GetState();
        m_calcDamageFunc = lua["CalculateDamage"];
        m_isFuncValid = m_calcDamageFunc.valid();

        if (m_isFuncValid)
        {
            spdlog::info("[CombatScript] 战斗规则加载成功");
        }
        else
        {
            spdlog::warn("[CombatScript] 未找到CalculateDamage函数，将使用默认公式");
        }
    }

    float CombatScriptLoader::CalculateDamage(float attackerAtk, float targetDef,
                                              const std::string &attackerClass, const std::string &targetClass)
    {
        // 1. 如果Lua函数无效，直接回退
        if (!m_isFuncValid)
        {
            return fallbackDamage(attackerAtk, targetDef);
        }

        // 2. 安全调用Lua函数
        auto result = m_calcDamageFunc(attackerAtk, targetDef, attackerClass, targetClass);

        if (result.valid())
        {
            return static_cast<float>(result);
        }
        else
        {
            // Lua运行时报错（比如除以零、索引越界）
            sol::error err = result;
            spdlog::error("[CombatScript] 计算错误: {}", err.what());
            return fallbackDamage(attackerAtk, targetDef);
        }
    }

    float CombatScriptLoader::fallbackDamage(float atk, float def) const
    {
        float damage = atk - def;
        return std::max(damage, 0.1f * atk); // 最小10%伤害
    }
}
