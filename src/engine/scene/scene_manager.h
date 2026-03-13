#pragma once
#include <memory>
#include <string>
#include <vector>
#include "../utils/events.h"

// 前置声明
namespace engine::core
{
    class Context;
}
namespace engine::scene
{
    class Scene;
}

namespace engine::scene
{

    /**
     * @brief 管理游戏中的场景栈，处理场景切换和生命周期。
     */
    class SceneManager final
    {
    private:
        engine::core::Context &context_;                  ///< 引擎上下文引用
        std::vector<std::unique_ptr<Scene>> scene_stack_; ///< 场景栈

        /**
         * @brief 待处理的场景操作类型。
         */
        enum class PendingAction
        {
            None,
            Push,
            Pop,
            Replace
        };
        PendingAction pending_action_ = PendingAction::None; ///< 待处理的动作
        std::unique_ptr<Scene> pending_scene_;               ///< 待处理场景

    public:
        explicit SceneManager(engine::core::Context &context);
        ~SceneManager();

        // 禁止拷贝和移动
        SceneManager(const SceneManager &) = delete;
        SceneManager &operator=(const SceneManager &) = delete;
        SceneManager(SceneManager &&) = delete;
        SceneManager &operator=(SceneManager &&) = delete;

        // getters
        Scene *getCurrentScene() const;                                ///< 获取当前活动场景（栈顶场景）的指针。
        engine::core::Context &getContext() const { return context_; } ///< 获取引擎上下文引用。

        // 核心循环函数
        void update(float delta_time); ///< 更新场景栈中所有场景的状态。
        void render();                 ///< 渲染场景栈中所有场景的内容。
        void close();                  ///< 关闭场景栈中所有场景，清理资源。

    private:
        // 事件回调函数-延时处理
        void onPopScene();                                            ///< 处理弹栈场景事件，移除栈顶场景。
        void onPushScene(engine::utils::PushSceneEvent &event);       ///< 处理压栈场景事件，将新场景压入栈顶。
        void onReplaceScene(engine::utils::ReplaceSceneEvent &event); ///< 处理替换场景事件，将新场景设为栈顶场景。

        void processPendingActions(); ///< 处理挂起的场景操作（每轮更新最后调用）。

        // 直接切换场景
        void pushScene(std::unique_ptr<Scene> &&scene);    ///< 将一个新场景压入栈顶，使其成为活动场景。
        void popScene();                                   ///< 移除栈顶场景。
        void replaceScene(std::unique_ptr<Scene> &&scene); ///< 清理场景栈所有场景，将此场景设为栈顶场景。
    };

} // namespace engine::scene