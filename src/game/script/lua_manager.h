// game/script/lua_manager.h
#pragma once
#include <sol/sol.hpp>
#include <string>
#include <spdlog/spdlog.h>

namespace game::script
{
    class LuaManager
    {
    private:
        sol::state lua_;          // Lua 虚拟机
        std::string damage_script_path_; // 伤害计算脚本路径

    public:
        LuaManager();
        ~LuaManager() = default;

        // 加载/重载伤害计算脚本
        bool load_damage_script(const std::string& path);
        
        // 调用 Lua 中的伤害计算函数
        float calculate_damage(float attacker_atk, float target_def);

        // 热重载脚本（外部触发）
        bool reload_damage_script();

    private:
        // 初始化 Lua 环境（注册基础函数/变量）
        void init_lua_env();
    };
}

