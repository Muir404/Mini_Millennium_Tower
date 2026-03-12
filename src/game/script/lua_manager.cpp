// game/script/lua_manager.cpp
#include "lua_manager.h"
#include <filesystem>

namespace game::script
{
    LuaManager::LuaManager()
    {
        init_lua_env();
    }

    void LuaManager::init_lua_env()
    {
        // 初始化 Lua 状态
        lua_.open_libraries(sol::lib::base, sol::lib::math);
        
        // 注册 C++ 函数到 Lua（可选，比如日志）
        lua_.set_function("log_info", [](const std::string& msg) {
            spdlog::info("[Lua] {}", msg);
        });
    }

    bool LuaManager::load_damage_script(const std::string& path)
    {
        damage_script_path_ = path;
        if (!std::filesystem::exists(path))
        {
            spdlog::error("Lua script not found: {}", path);
            return false;
        }

        try
        {
            // 加载并执行脚本
            lua_.script_file(path);
            spdlog::info("Loaded damage script: {}", path);
            return true;
        }
        catch (const sol::error& e)
        {
            spdlog::error("Lua script error: {}", e.what());
            return false;
        }
    }

    float LuaManager::calculate_damage(float attacker_atk, float target_def)
    {
        try
        {
            // 调用 Lua 中的 calculate_effective_damage 函数
            sol::function damage_func = lua_["calculate_effective_damage"];
            if (!damage_func.valid())
            {
                spdlog::warn("Lua damage function not found, use default formula");
                // 降级使用原 C++ 公式（兜底）
                float damage = attacker_atk - target_def;
                return std::max(damage, 0.1f * attacker_atk);
            }
            return damage_func.call<float>(attacker_atk, target_def);
        }
        catch (const sol::error& e)
        {
            spdlog::error("Call Lua damage function failed: {}", e.what());
            // 兜底逻辑
            float damage = attacker_atk - target_def;
            return std::max(damage, 0.1f * attacker_atk);
        }
    }

    bool LuaManager::reload_damage_script()
    {
        spdlog::info("Reloading damage script: {}", damage_script_path_);
        return load_damage_script(damage_script_path_);
    }
}

