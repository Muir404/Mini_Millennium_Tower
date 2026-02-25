#pragma once
#include <memory>
#include <functional>
#include <entt/signal/fwd.hpp>

// 前向声明，减少头文件依赖，加速编译
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

namespace engine::core // 命名空间与路径一致，实践推荐
{
    class Time;
    class Config;
    class Context;
    class GameState;

    class GameApp final
    {
    private:
        SDL_Window *window_ = nullptr;         ///< SDL窗口指针，用于创建和管理游戏窗口。
        SDL_Renderer *sdl_renderer_ = nullptr; ///< SDL渲染器指针，用于渲染游戏场景。
        bool is_running_ = false;              ///< 游戏运行状态标志，用于控制游戏主循环。

        std::function<void(engine::core::Context &)> scene_setup_func_; ///< 场景设置函数，用于在游戏前设置初始场景，不负责决定初始场景。

        // 引擎组件
        std::unique_ptr<entt::dispatcher> dispatcher_;                        ///< 事件调度器，用于处理游戏事件。
        std::unique_ptr<engine::core::Time> time_;                            ///< 时间管理器，用于管理游戏时间。
        std::unique_ptr<engine::resource::ResourceManager> resource_manager_; ///< 资源管理器，用于管理游戏资源。
        std::unique_ptr<engine::render::Renderer> renderer_;                  ///< 渲染器，用于渲染游戏场景。
        std::unique_ptr<engine::render::Camera> camera_;                      ///< 相机，用于渲染游戏场景。
        std::unique_ptr<engine::render::TextRenderer> text_renderer_;         ///< 文本渲染器，用于渲染文本。
        std::unique_ptr<engine::core::Config> config_;                        ///< 配置管理器，用于管理游戏配置。
        std::unique_ptr<engine::input::InputManager> input_manager_;          ///< 输入管理器，用于管理游戏输入。
        std::unique_ptr<engine::core::Context> context_;                      ///< 上下文管理器，用于管理游戏上下文。
        std::unique_ptr<engine::scene::SceneManager> scene_manager_;          ///< 场景管理器，用于管理游戏场景。
        std::unique_ptr<engine::audio::AudioPlayer> audio_player_;            ///< 音频播放器，用于播放游戏音频。
        std::unique_ptr<engine::core::GameState> game_state_;                 ///< 游戏状态管理器，用于管理游戏状态。

    public:
        GameApp();
        ~GameApp();

        void run(); ///<  运行游戏主循环，处理事件、更新状态、渲染场景。

        /**
         * @brief 注册场景设置函数，用于在游戏前设置初始场景。
         *
         * @param func 场景设置函数，用于在游戏前设置初始场景。
         * @note 该函数必须在run方法之前调用，否则会导致未定义行为。
         */
        void registerSceneSetup(std::function<void(engine::core::Context &)> func);

        // 由于不需要移动构造和复制构造等内容，此处做delete处理
        GameApp(const GameApp &) = delete;
        GameApp &operator=(const GameApp &) = delete;
        GameApp(const GameApp &&) = delete;
        GameApp &operator=(const GameApp &&) = delete;

    private:
        // discard表示不可忽略，必须处理
        [[nodiscard]] bool init();     ///<  初始化游戏应用，设置游戏环境和组件。
        void handleEvents();           ///<  处理游戏事件，如键盘输入、鼠标事件等。
        void update(float delta_time); ///<  更新游戏状态，如处理输入、更新游戏逻辑等。
        void render();                 ///<  渲染游戏场景，将游戏状态绘制到屏幕上。
        void close();                  ///<  关闭游戏，清理资源。

        [[nodiscard]] bool initDispatcher();      ///<  初始化事件调度器，用于处理游戏事件。
        [[nodiscard]] bool initConfig();          ///<  初始化配置管理器，用于管理游戏配置。
        [[nodiscard]] bool initSDL();             ///<  初始化SDL，用于创建和管理游戏窗口。
        [[nodiscard]] bool initTime();            ///<  初始化时间管理器，用于管理游戏时间。
        [[nodiscard]] bool initResourceManager(); ///<  初始化资源管理器，用于管理游戏资源。
        [[nodiscard]] bool initAudioPlayer();     ///<  初始化音频播放器，用于播放游戏音频。
        [[nodiscard]] bool initRenderer();        ///<  初始化渲染器，用于渲染游戏场景。
        [[nodiscard]] bool initCamera();          ///<  初始化相机，用于渲染游戏场景。
        [[nodiscard]] bool initTextRenderer();    ///<  初始化文本渲染器，用于渲染文本。
        [[nodiscard]] bool initInputManager();    ///<  初始化输入管理器，用于管理游戏输入。
        [[nodiscard]] bool initGameState();       ///<  初始化游戏状态管理器，用于管理游戏状态。
        [[nodiscard]] bool initContext();         ///<  初始化上下文管理器，用于管理游戏上下文。
        [[nodiscard]] bool initSceneManager();    ///< 初始化场景管理器，用于管理游戏场景。

        /**
         * @brief 处理退出事件的回调函数。
         *
         * 当触发退出事件时，会调用此函数来处理退出逻辑。
         */
        void onQuitEvent();
    };
} // namespace engine::core