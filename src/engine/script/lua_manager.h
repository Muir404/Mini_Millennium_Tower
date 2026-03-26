#pragma once
#include <sol/sol.hpp>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <functional>

namespace engine::resource
{
    class LuaManager
    {
    public:
        LuaManager();
        ~LuaManager();

        // 禁止拷贝
        LuaManager(const LuaManager &) = delete;
        LuaManager &operator=(const LuaManager &) = delete;
        LuaManager(LuaManager &&) = delete;
        LuaManager &operator=(LuaManager &&) = delete;

        /// @brief 关闭Lua状态
        void shutdown();

        /// @brief 加载脚本
        void loadScript(const std::string &path);

        /// @brief 每帧更新 (热重载检测)
        /// @param delta_time 与上一帧的时间差（单位：秒）
        void update(double delta_time);

        /// @brief 获取Lua状态 (用于绑定)
        sol::state &getState() { return luastate_; }

    private:
        /// @brief 脚本信息结构体
        /// @details 用于存储脚本的路径和最后写入时间
        struct ScriptInfo
        {
            std::string path_;                                /// @brief 脚本路径
            std::filesystem::file_time_type last_write_time_; /// @brief 最后写入时间
        };
        sol::state luastate_;                                      /// @brief Lua状态对象
        std::unordered_map<std::string, ScriptInfo> script_infos_; /// @brief 脚本信息映射表
        double hot_reload_accumulator_ = 0.0;                      ///< 时间累加器
        static constexpr double HOT_RELOAD_CHECK_INTERVAL = 3.0;   ///< 检查间隔（秒）
    };
}
