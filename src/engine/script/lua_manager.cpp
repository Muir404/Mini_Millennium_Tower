#include "lua_manager.h"
#include "../core/time.h"

#include <spdlog/spdlog.h>

namespace engine::resource
{
    LuaManager::LuaManager()
    {
        luastate_.open_libraries(sol::lib::base,
                                 sol::lib::math,
                                 sol::lib::table,
                                 sol::lib::string);
        spdlog::info("[LuaManager] 初始化完成 ");
    }

    LuaManager::~LuaManager()
    {
        shutdown();
    }

    void LuaManager::loadScript(const std::string &path)
    {
        try
        {
            luastate_.script_file(path);
            script_infos_[path] = {path, std::filesystem::last_write_time(path)};
            spdlog::info("[LuaManager] 加载脚本: {}", path);
        }
        catch (const sol::error &e)
        {
            spdlog::error("[LuaManager] 加载失败: {}", e.what());
        }
    }

    void LuaManager::update(double delta_time)
    {
        namespace fs = std::filesystem;

        // 1. 累加时间
        hot_reload_accumulator_ += delta_time;

        // 2. 如果还没到检查时间，直接返回
        if (hot_reload_accumulator_ < HOT_RELOAD_CHECK_INTERVAL)
        {
            return;
        }

        // 3. 时间到了，重置计时器（减去间隔而不是设为0，防止时间误差累积）
        hot_reload_accumulator_ -= HOT_RELOAD_CHECK_INTERVAL;

        // 4. 执行原本的检查逻辑
        for (auto &[path, info] : script_infos_)
        {
            try
            {
                auto currentTime = fs::last_write_time(path);
                if (currentTime != info.last_write_time_)
                {
                    spdlog::warn("[LuaManager] 热重载中: {}", path);
                    luastate_.script_file(path);
                    info.last_write_time_ = currentTime;
                }
            }
            catch (...)
            {
                spdlog::error("[LuaManager] 更新失败: {}", path);
            }
        }
    }

    void LuaManager::shutdown()
    {
        script_infos_.clear();       // 清空脚本信息
        luastate_.collect_garbage(); // 收集Lua状态中的垃圾
    }
}
