#pragma once
#include <memory>
#include <functional>
#include <entt/signal/fwd.hpp>

// 前向声明, 减少头文件的依赖，增加编译速度
struct SDL_Window;
struct SDL_Renderer;

namespace engine::resource
{
    class ResourceManager;
}

namespace engine::render
{
    class Renderer;
    class Camera;
    class TextRenderer;
}

namespace engine::input
{
    class InputManager;
}

namespace engine::scene
{
    class SceneManager;
}

namespace engine::audio
{
    class AudioPlayer;
}

namespace engine::core
{ // 命名空间的最佳实践：与文件路径一致
    class Time;
    class Config;
    class Context;
    class GameState;

    /**
     * @brief 主游戏应用程序类，初始化SDL，管理游戏循环。
     */
    class GameApp final
    {
    private:
        SDL_Window *window_ = nullptr;
        SDL_Renderer *sdl_renderer_ = nullptr;
        bool is_running_ = false;

        std::function<void(engine::core::Context &)> scene_setup_func_;

        // 引擎组件
        std::unique_ptr<entt::dispatcher> dispatcher_; // 事件分发器
        std::unique_ptr<engine::core::Time> time_;
        std::unique_ptr<engine::resource::ResourceManager> resource_manager_;
        std::unique_ptr<engine::render::Renderer> renderer_;
        std::unique_ptr<engine::render::Camera> camera_;
        std::unique_ptr<engine::render::TextRenderer> text_renderer_;
        std::unique_ptr<engine::core::Config> config_;
        std::unique_ptr<engine::input::InputManager> input_manager_;
        std::unique_ptr<engine::core::Context> context_;
        std::unique_ptr<engine::scene::SceneManager> scene_manager_;
        std::unique_ptr<engine::audio::AudioPlayer> audio_player_;
        std::unique_ptr<engine::core::GameState> game_state_;

    public:
        GameApp();
        ~GameApp();

        void run();

        void registerSceneSetup(std::function<void(engine::core::Context &)> func);

        // 禁止拷贝和移动
        GameApp(const GameApp &) = delete;
        GameApp &operator=(const GameApp &) = delete;
        GameApp(GameApp &&) = delete;
        GameApp &operator=(GameApp &&) = delete;

    private:
        [[nodiscard]] bool init(); // nodiscard 表示该函数返回值不应该被忽略
        void handleEvents();
        void update(float delta_time);
        void render();
        void close();

        // 各模块的初始化/创建函数，在init()中调用
        [[nodiscard]] bool initDispatcher();
        [[nodiscard]] bool initConfig();
        [[nodiscard]] bool initSDL();
        [[nodiscard]] bool initGameState();
        [[nodiscard]] bool initTime();
        [[nodiscard]] bool initResourceManager();
        [[nodiscard]] bool initAudioPlayer();
        [[nodiscard]] bool initRenderer();
        [[nodiscard]] bool initTextRenderer();
        [[nodiscard]] bool initCamera();
        [[nodiscard]] bool initInputManager();
        [[nodiscard]] bool initContext();
        [[nodiscard]] bool initSceneManager();

        // 事件处理函数
        void onQuitEvent();
    };

} // namespace engine::core
