#include "lua_manager.h"
#include "spdlog/spdlog.h" // 引入 spdlog 头文件

namespace engine::script
{
    LuaManager::LuaManager()
    {
        lua_.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string);
    }

    void LuaManager::addBinding(const std::string &name, LuaBindFunc func)
    {
        pending_bindings_[name] = std::move(func);
    }

    void LuaManager::performBindings()
    {
        if (bindings_done_)
            return;
        for (auto &[name, func] : pending_bindings_)
        {
            func(lua_); // 执行具体的绑定逻辑
        }
        bindings_done_ = true;
        pending_bindings_.clear(); // 清理临时存储
    }

    bool LuaManager::loadScript(const std::string &identifier, const std::string &filepath)
    {
        // 确保绑定先执行
        if (!bindings_done_)
            performBindings();

        try
        {
            lua_.safe_script_file(filepath);
            // 记录文件时间用于热更
            loaded_scripts_[identifier] = {
                filepath,
                std::filesystem::last_write_time(filepath)};
            return true;
        }
        catch (const sol::error &e)
        {
            spdlog::error("Lua Error: {}", e.what());
            return false;
        }
    }

    void LuaManager::update(float dt)
    {
        reload_cooldown_ -= dt;
        if (reload_cooldown_ > 0)
        {
            return;
        }
        reload_cooldown_ = 3.0f; // 3秒检查一次

        for (auto &[id, info] : loaded_scripts_)
        {
            auto current_time = std::filesystem::last_write_time(info.path);
            if (current_time > info.last_write)
            {
                spdlog::info("Hot-reloading: {}", info.path);
                // 重新加载
                if (loadScript(id, info.path))
                {
                    info.last_write = current_time;
                }
            }
        }
    }
}
