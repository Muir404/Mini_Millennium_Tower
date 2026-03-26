#pragma once
#include <sol/sol.hpp>
#include <string>

namespace game::script
{
    class CombatScriptLoader
    {
    public:
        static CombatScriptLoader &GetInstance();

        // 禁止拷贝
        CombatScriptLoader(const CombatScriptLoader &) = delete;
        CombatScriptLoader &operator=(const CombatScriptLoader &) = delete;

        // 加载战斗规则脚本
        void LoadScript(const std::string &path);

        // 【核心】给CombatSystem调用的接口
        float CalculateDamage(float attackerAtk, float targetDef,
                              const std::string &attackerClass, const std::string &targetClass);

    private:
        CombatScriptLoader() = default;
        ~CombatScriptLoader() = default;

        // 缓存Lua函数，避免每帧字符串查找
        sol::protected_function m_calcDamageFunc;
        bool m_isFuncValid = false;

        // 回退用的C++默认公式
        float fallbackDamage(float atk, float def) const;
    };
}
