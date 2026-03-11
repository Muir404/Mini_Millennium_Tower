#pragma once
#include <memory>
#include <entt/entity/entity.hpp>

namespace engine::scene
{
    class Scene;
}

namespace engine::utils
{
    /**
     * @struct QuitEvent
     * @brief 表示退出事件的结构体。
     *
     * 当触发退出事件时，通常会导致应用程序关闭或退出。
     */
    struct QuitEvent
    {
    };

    /**
     * @struct PopSceneEvent
     * @brief 表示弹出场景事件的结构体。
     *
     * 当触发弹出场景事件时，通常会导致当前场景从场景栈中弹出。
     */
    struct PopSceneEvent
    {
    };

    /**
     * @struct PushSceneEvent
     * @brief 表示推送场景事件的结构体。
     *
     * 当触发推送场景事件时，通常会导致新场景被压入场景栈中。
     */
    struct PushSceneEvent
    {
        std::unique_ptr<engine::scene::Scene> scene;
    };

    /**
     * @struct ReplaceSceneEvent
     * @brief 表示替换场景事件的结构体。
     *
     * 当触发替换场景事件时，通常会导致当前场景被新场景替换。
     */
    struct ReplaceSceneEvent
    {
        std::unique_ptr<engine::scene::Scene> scene;
    };

    /**
     * @struct PlayAnimationEvent
     * @brief 表示播放动画事件的结构体。
     *
     * 当触发播放动画事件时，通常会导致指定实体播放指定动画。
     */
    struct PlayAnimationEvent
    {
        entt::entity entity_{entt::null};
        entt::id_type animation_id_{entt::null};
        bool loop_{false};
    };
} // namespace engine::utils
