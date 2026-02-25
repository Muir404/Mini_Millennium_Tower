#pragma once
#include <entt/entt.hpp>
// 前置声明核心系统
namespace engine::input
{
    class InputManager;
}

namespace engine::render
{
    class Renderer;
    class Camera;
    class TextRenderer;
}

namespace engine::resource
{
    class ResourceManager;
}

namespace engine::audio
{
    class AudioPlayer;
}

namespace engine::core
{
    class GameState;

    /**
     * @brief 持有对核心引擎模块引用的上下文对象。
     *
     * 用于简化依赖注入，传递Context即可获取引擎的各个模块。
     */
    class Context final
    {
    private:
        // 使用引用，确保每个模块都有效，使用时不需要检查指针是否为空。
        entt::dispatcher &dispatcher_;                        ///< 事件分发器
        engine::input::InputManager &input_manager_;          ///< 输入管理器
        engine::render::Renderer &renderer_;                  ///< 渲染器
        engine::render::Camera &camera_;                      ///< 相机
        engine::render::TextRenderer &texture_renderer_;      ///< 字体引擎
        engine::resource::ResourceManager &resource_manager_; ///< 资源管理器
        engine::audio::AudioPlayer &audio_player_;            ///< 音频播放器
        engine::core::GameState &game_state_;                 ///< 游戏状态

    public:
        /**
         * @brief 构造函数，初始化上下文对象。
         * @param dispatcher 事件分发器
         * @param input_manager 输入管理器
         * @param renderer 渲染器
         * @param camera 相机
         * @param texture_renderer 字体引擎
         * @param resource_manager 资源管理器
         * @param audio_player 音频播放器
         * @param game_state 游戏状态
         * @note 所有参数都必须有效，否则行为未定义。
         */
        Context(
            entt::dispatcher &dispatcher,
            engine::input::InputManager &input_manager,
            engine::render::Renderer &renderer,
            engine::render::Camera &camera,
            engine::render::TextRenderer &texture_renderer,
            engine::resource::ResourceManager &resource_manager,
            engine::audio::AudioPlayer &audio_player_,
            engine::core::GameState &game_state_);

        // 禁止拷贝和移动，Context 对象通常是唯一的或按需创建/传递
        Context(const Context &) = delete;
        Context &operator=(const Context &) = delete;
        Context(Context &&) = delete;
        Context &operator=(Context &&) = delete;

        // --- Getters ---
        entt::dispatcher &getDispatcher() const
        {
            return dispatcher_; // 获取事件分发器
        }
        engine::input::InputManager &getInputManager() const
        {
            return input_manager_; // 获取输入管理器
        }
        engine::render::Renderer &getRenderer() const
        {
            return renderer_; // 获取渲染器
        }
        engine::render::Camera &getCamera() const
        {
            return camera_; // 获取相机
        }
        engine::render::TextRenderer &getTextRenderer() const
        {
            return texture_renderer_; // 获取字体引擎
        }
        engine::resource::ResourceManager &getResourceManager() const
        {
            return resource_manager_; // 获取资源管理器
        }
        engine::audio::AudioPlayer &getAudioPlayer() const
        {
            return audio_player_; // 获取播放器
        }
        engine::core::GameState &getGameState() const
        {
            return game_state_; // 获取游戏状态
        }
    };

} // namespace engine::core