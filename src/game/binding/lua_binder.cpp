#include "lua_binder.h"
#include "../../engine/script/lua_manager.h" // 包含我们新的管理器

namespace game::binding
{
    LuaBinder::LuaBinder(engine::core::Context &context) : context_(context)
    {
        // 1. 向 LuaManager 注册这个模块的绑定
        auto &lua_mgr = context_.getLuaManager();
        lua_mgr.addBinding("Combat", &LuaBinder::registerBindings);

        // 2. 告诉 LuaManager 加载脚本
        lua_mgr.loadScript("combat_script", "src/game/scripts/combat/combat.lua");
    }

    // 这里是真正的 C++ 函数暴露给 Lua 的地方
    void LuaBinder::registerBindings(sol::state &lua)
    {
        lua["calculateEffectiveDamage"] = &LuaBinder::calculateEffectiveDamage;
    }

    void LuaBinder::update()
    {
        context_.getLuaManager().update(0.0f);
    }

    float LuaBinder::calculateEffectiveDamage(float attacker_atk, float target_def)
    {
        auto &lua = context_.getLuaManager().getState();

        // 安全地获取 Lua 函数
        sol::protected_function func = lua["calculateEffectiveDamage"];
        if (!func.valid())
        {
            return 0.0f; // 默认值或错误处理
        }

        // 安全调用
        auto result = func(attacker_atk, target_def);
        if (result.valid())
        {
            return result;
        }

        // 错误处理
        sol::error err = result;
        // 打印 err.what()
        return 0.0f;
    }
}
