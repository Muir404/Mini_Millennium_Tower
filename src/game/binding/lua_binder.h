#pragma once
#include <sol/sol.hpp>
#include <string>
#include "../../engine/core/context.h"
// 注意：这里不再直接包含 sol 状态的所有权，而是依赖 LuaManager

namespace game::binding
{
    class LuaBinder
    {
    public:
        /** @brief 初始化 LuaBinder
         * @param context 上下文引用，用于获取 LuaManager
         * @note 该类负责将战斗公式绑定到 Lua 环境中，使 Lua 脚本能够调用。
         */
        explicit LuaBinder(engine::core::Context &context);

        // === 业务接口组 ======
        /** @brief 计算有效伤害
         * @param attacker_atk 攻击者攻击力
         * @param target_def 目标防御力
         * @return 最终造成的伤害
         */
        float calculateEffectiveDamage(float attacker_atk, float target_def);

        // TODO 未来可能会添加其他内容

        // === 静态：注册绑定函数（给 LuaManager 调用） ======
        /** @brief 注册所有绑定函数
         * @param lua Lua 状态引用
         */
        static void registerBindings(sol::state &lua);

        void update();

    private:
        engine::core::Context &context_; ///< 上下文引用，用于获取 LuaManager
    };
}
