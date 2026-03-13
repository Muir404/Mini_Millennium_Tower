#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <entt/entity/registry.hpp>

namespace engine::core
{
    class Context;
}

namespace engine::ui
{
    class UIManager;
}

namespace engine::scene
{
    class SceneManager;

    class Scene
    {
    protected:
        std::string scene_name_;                            ///< 场景名称
        engine::core::Context &context_;                    ///< 上下文引用（隐式，构造时传入）
        std::unique_ptr<engine::ui::UIManager> ui_manager_; ///< UI管理器(初始化时自动创建)
        entt::registry registry_;                           ///< ECS注册表

        bool is_initialized_ = false; ///< 场景是否已初始化(非当前场景很可能未被删除，因此需要初始化标志避免重复初始化)

    public:
        Scene(std::string_view name, engine::core::Context &context);

        // 1. 基类必须声明虚析构函数才能让派生类析构函数被正确调用。
        // 2. 析构函数定义必须写在cpp中，不然需要引入GameObject头文件
        virtual ~Scene();

        // 禁止拷贝和移动构造
        Scene(const Scene &) = delete;
        Scene &operator=(const Scene &) = delete;
        Scene(Scene &&) = delete;
        Scene &operator=(Scene &&) = delete;

        // 核心循环方法
        virtual void init();                   ///< 初始化场景。
        virtual void update(float delta_time); ///< 更新场景。
        virtual void render();                 ///< 渲染场景。
        virtual void clean();                  ///< 清理场景。

        void requestPopScene();
        void requestPushScene(std::unique_ptr<engine::scene::Scene> &&scene);
        void requestReplaceScene(std::unique_ptr<engine::scene::Scene> &&scene);
        void quit();

        // getters and setters
        void setName(std::string_view name) { scene_name_ = name; }              ///< 设置场景名称
        std::string_view getName() const { return scene_name_; }                 ///< 获取场景名称
        void setInitialized(bool initialized) { is_initialized_ = initialized; } ///< 设置场景是否已初始化
        bool isInitialized() const { return is_initialized_; }                   ///< 获取场景是否已初始化
        entt::registry &getRegistry() { return registry_; }                      ///< 获取注册表引用

        engine::core::Context &getContext() const { return context_; } ///< 获取上下文引用
    };

} // namespace engine::scene