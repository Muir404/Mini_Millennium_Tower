#pragma once
#include <sol/sol.hpp>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <functional>

namespace engine::core
{
    class Context;
}

namespace engine::script
{
    // 定义一个绑定函数的类型别名
    using LuaBindFunc = std::function<void(sol::state &)>;

    class LuaManager
    {
    public:
        explicit LuaManager();
        ~LuaManager() = default;

        // 禁止拷贝移动
        LuaManager(const LuaManager &) = delete;
        LuaManager &operator=(const LuaManager &) = delete;

        // --- 核心功能 ---

        /**
         * @brief 获取 Lua 状态（给需要直接操作的地方）
         * @return Lua 状态引用
         */
        sol::state &getState() { return lua_; }

        /**
         * @brief 注册绑定模块（游戏逻辑调用此函数来注册）
         * @param name 绑定模块的名称（用于唯一标识）
         * @param func 绑定函数，用于在Lua中调用
         */
        void addBinding(const std::string &name, LuaBindFunc func);

        /**
         * @brief 执行所有注册的绑定（通常在加载脚本前调用一次）
         */
        void performBindings();

        /**
         * @brief 加载脚本（带热更监控）
         * @param identifier 脚本的唯一标识符（用于热更监控）
         * @param filepath 脚本文件路径
         * @return 是否成功加载脚本
         */
        bool loadScript(const std::string &identifier, const std::string &filepath);

        /**
         * @brief 每帧调用的热更检查（内部会控制3秒检查一次）
         * 检查Lua文件是否修改，修改则自动重载脚本
         */
        void update(float dt);

    private:
        /**
         * @brief 脚本信息结构体，用于存储已加载脚本的路径和最后修改时间
         * @param path 脚本文件路径
         * @param last_write 最后修改时间
         */
        struct ScriptInfo
        {
            std::string path;
            std::filesystem::file_time_type last_write;
        };

        sol::state lua_;                                                ///< Lua虚拟机状态
        std::unordered_map<std::string, LuaBindFunc> pending_bindings_; ///< 待注册绑定模块
        std::unordered_map<std::string, ScriptInfo> loaded_scripts_;    ///< 已加载脚本信息
        bool bindings_done_ = false;                                    ///< 是否已执行所有绑定模块
        float reload_cooldown_ = 0.0f;                                  ///< 重载冷却时间
    };
}
