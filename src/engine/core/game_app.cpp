#include "game_app.h"
#include "time.h"
#include "context.h"
#include "config.h"
#include "game_state.h"
#include "../resource/resource_manager.h"
#include "../audio/audio_player.h"
#include "../render/renderer.h"
#include "../render/camera.h"
#include "../render/text_renderer.h"
#include "../input/input_manager.h"
#include "../scene/scene_manager.h"
#include "../utils/events.h"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <entt/signal/dispatcher.hpp>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

namespace engine::core
{

    GameApp::GameApp() = default;

    GameApp::~GameApp()
    {
        if (is_running_)
        {
            spdlog::warn("GameApp 被销毁时没有显式关闭。现在关闭。 ...");
            close();
        }
    }

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

    void GameApp::run()
    {
        if (!init())
        {
            spdlog::error("GameApp 初始化失败，无法运行游戏。");
            return;
        }
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop_arg([](void *arg)
                                     { static_cast<GameApp *>(arg)->oneIter(); }, this, 0, 1);
#else
        while (is_running_)
        {
            oneIter();
        }
        close();
#endif
    }

    void GameApp::oneIter()
    {
        time_->update();
        float delta_time = time_->getDeltaTime();

        handleEvents();
        update(delta_time);
        render();

        // 分发事件
        dispatcher_->update();
#ifdef __EMSCRIPTEN__
        if (!is_running_)
        {
            emscripten_cancel_main_loop();
            close();
        }
#endif
    }

    void GameApp::registerSceneSetup(std::function<void(engine::core::Context &)> func)
    {
        scene_setup_func_ = std::move(func);
        spdlog::trace("已注册场景设置函数。");
    }

    bool GameApp::init()
    {
        spdlog::trace("初始化 GameApp ...");
        if (!scene_setup_func_)
        {
            spdlog::error("未注册场景设置函数，无法初始化 GameApp。");
            return false;
        }
        if (!initDispatcher())
        {
            return false;
        }
        if (!initConfig())
        {
            return false;
        }
        if (!initSDL())
        {
            return false;
        }
        if (!initGameState())
        {
            return false;
        }
        if (!initTime())
        {
            return false;
        }
        if (!initResourceManager())
        {
            return false;
        }
        if (!initAudioPlayer())
        {
            return false;
        }
        if (!initRenderer())
        {
            return false;
        }
        if (!initCamera())
        {
            return false;
        }
        if (!initTextRenderer())
        {
            return false;
        }
        if (!initInputManager())
        {
            return false;
        }
        if (!initContext())
        {
            return false;
        }
        if (!initSceneManager())
        {
            return false;
        }
        if (!initImGui())
        {
            return false;
        }

        // 调用场景设置函数 (创建第一个场景并压入栈)
        scene_setup_func_(*context_);

        // 注册退出事件 (回调函数可以无参数，代表不使用事件结构体中的数据)
        dispatcher_->sink<utils::QuitEvent>().connect<&GameApp::onQuitEvent>(this);

        is_running_ = true;
        spdlog::trace("GameApp 初始化成功。");
        return true;
    }

    void GameApp::handleEvents()
    {
        // 处理并分发输入事件
        input_manager_->update();
    }

    void GameApp::update(float delta_time)
    {
        // 游戏逻辑更新
        scene_manager_->update(delta_time);
    }

    void GameApp::render()
    {
        // 1. 清除屏幕
        renderer_->clearScreen();

        // 2. 具体渲染代码
        scene_manager_->render();

        // 3. 更新屏幕显示
        renderer_->present();
    }

    void GameApp::close()
    {
        spdlog::trace("关闭 GameApp ...");

        // 步骤系列四：清理 ImGui 资源
        // 9. 清理 ImGui 资源，接续initImGui()
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        // 断开事件处理函数
        dispatcher_->sink<utils::QuitEvent>().disconnect<&GameApp::onQuitEvent>(this);

        // 先关闭场景管理器，确保所有场景都被清理
        scene_manager_->close();

        // 为了确保正确的销毁顺序，有些智能指针对象也需要手动管理
        resource_manager_.reset();

        if (sdl_renderer_ != nullptr)
        {
            SDL_DestroyRenderer(sdl_renderer_);
            sdl_renderer_ = nullptr;
        }
        if (window_ != nullptr)
        {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
        SDL_Quit();
        is_running_ = false;
    }

    bool GameApp::initDispatcher()
    {
        try
        {
            dispatcher_ = std::make_unique<entt::dispatcher>();
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化事件分发器失败: {}", e.what());
            return false;
        }
        spdlog::trace("事件分发器初始化成功。");
        return true;
    }

    bool GameApp::initConfig()
    {
        try
        {
            config_ = std::make_unique<engine::core::Config>("assets/config.json");
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化配置失败: {}", e.what());
            return false;
        }
        spdlog::trace("配置初始化成功。");
        return true;
    }

    bool GameApp::initSDL()
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        {
            spdlog::error("SDL 初始化失败! SDL错误: {}", SDL_GetError());
            return false;
        }

        int window_width = static_cast<int>(config_->window_width_ * config_->window_scale_);
        int window_height = static_cast<int>(config_->window_height_ * config_->window_scale_);
        window_ = SDL_CreateWindow(config_->window_title_.c_str(), window_width, window_height, SDL_WINDOW_RESIZABLE);
        if (window_ == nullptr)
        {
            spdlog::error("无法创建窗口! SDL错误: {}", SDL_GetError());
            return false;
        }

        sdl_renderer_ = SDL_CreateRenderer(window_, nullptr);
        if (sdl_renderer_ == nullptr)
        {
            spdlog::error("无法创建渲染器! SDL错误: {}", SDL_GetError());
            return false;
        }

        // 设置渲染器支持透明色
        SDL_SetRenderDrawBlendMode(sdl_renderer_, SDL_BLENDMODE_BLEND);

        // 设置 VSync (注意: VSync 开启时，驱动程序会尝试将帧率限制到显示器刷新率，有可能会覆盖我们手动设置的 target_fps)
        int vsync_mode = config_->vsync_enabled_ ? SDL_RENDERER_VSYNC_ADAPTIVE : SDL_RENDERER_VSYNC_DISABLED;
        SDL_SetRenderVSync(sdl_renderer_, vsync_mode);
        spdlog::trace("VSync 设置为: {}", config_->vsync_enabled_ ? "Enabled" : "Disabled");

        // 设置逻辑分辨率为窗口大小的一半（针对像素游戏）
        int logical_width = static_cast<int>(static_cast<float>(config_->window_width_) * config_->window_logical_scale_);
        int logical_height = static_cast<int>(static_cast<float>(config_->window_height_) * config_->window_logical_scale_);
        SDL_SetRenderLogicalPresentation(sdl_renderer_, logical_width, logical_height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
        spdlog::trace("SDL 初始化成功。");
        return true;
    }

    bool GameApp::initTime()
    {
        try
        {
            time_ = std::make_unique<Time>();
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化时间管理失败: {}", e.what());
            return false;
        }
        time_->setTargetFps(config_->target_fps_);
        spdlog::trace("时间管理初始化成功。");
        return true;
    }

    bool GameApp::initResourceManager()
    {
        try
        {
            resource_manager_ = std::make_unique<engine::resource::ResourceManager>(sdl_renderer_);
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化资源管理器失败: {}", e.what());
            return false;
        }
        spdlog::trace("资源管理器初始化成功。");
        resource_manager_->loadResources("assets/data/resource_mapping.json"); // 载入默认资源映射文件
        return true;
    }

    bool GameApp::initAudioPlayer()
    {
        try
        {
            audio_player_ = std::make_unique<engine::audio::AudioPlayer>(resource_manager_.get());
            audio_player_->setMusicVolume(config_->music_volume_); // 设置背景音乐音量
            audio_player_->setSoundVolume(config_->sound_volume_); // 设置音效音量
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化音频播放器失败: {}", e.what());
            return false;
        }
        spdlog::trace("音频播放器初始化成功。");
        return true;
    }

    bool GameApp::initRenderer()
    {
        try
        {
            renderer_ = std::make_unique<engine::render::Renderer>(sdl_renderer_, resource_manager_.get());
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化渲染器失败: {}", e.what());
            return false;
        }
        spdlog::trace("渲染器初始化成功。");
        return true;
    }

    bool GameApp::initCamera()
    {
        try
        {
            camera_ = std::make_unique<engine::render::Camera>(game_state_->getLogicalSize());
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化相机失败: {}", e.what());
            return false;
        }
        spdlog::trace("相机初始化成功。");
        return true;
    }

    bool GameApp::initTextRenderer()
    {
        try
        {
            text_renderer_ = std::make_unique<engine::render::TextRenderer>(sdl_renderer_, resource_manager_.get());
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化文字渲染引擎失败: {}", e.what());
            return false;
        }
        spdlog::trace("文字渲染引擎初始化成功。");
        return true;
    }

    bool GameApp::initInputManager()
    {
        try
        {
            input_manager_ = std::make_unique<engine::input::InputManager>(sdl_renderer_, config_.get(), dispatcher_.get());
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化输入管理器失败: {}", e.what());
            return false;
        }
        spdlog::trace("输入管理器初始化成功。");
        return true;
    }

    bool GameApp::initGameState()
    {
        try
        {
            game_state_ = std::make_unique<engine::core::GameState>(window_, sdl_renderer_);
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化游戏状态失败: {}", e.what());
            return false;
        }
        return true;
    }

    bool GameApp::initContext()
    {
        try
        {
            context_ = std::make_unique<engine::core::Context>(*dispatcher_,
                                                               *input_manager_,
                                                               *renderer_,
                                                               *camera_,
                                                               *text_renderer_,
                                                               *resource_manager_,
                                                               *audio_player_,
                                                               *game_state_,
                                                               *time_);
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化上下文失败: {}", e.what());
            return false;
        }
        return true;
    }

    bool GameApp::initSceneManager()
    {
        try
        {
            scene_manager_ = std::make_unique<engine::scene::SceneManager>(*context_);
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化场景管理器失败: {}", e.what());
            return false;
        }
        spdlog::trace("场景管理器初始化成功。");
        return true;
    }

    bool GameApp::initImGui()
    {
        // 步骤系列一：初始化ImGui上下文
        // 1- 初始化ImGui上下文
        IMGUI_CHECKVERSION();   // 检查ImGui版本是否匹配
        ImGui::CreateContext(); // 创建ImGui上下文

        // 2- 配置ImGui上下文
        ImGuiIO &io = ImGui::GetIO();                         // 获取ImGui IO 接口
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 启用键盘导航
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // 启用游戏手柄导航
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // 启用Docking功能

        // 3- 初始化ImGui样式
        ImGui::StyleColorsDark(); // 使用暗色主题
        // ImGui::StyleColorsLight(); // 使用亮色主题

        // 4- 初始化缩放因子
        float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay()); // 获取主显示器的缩放因子
        // float main_scale = 1.0f; // 直接设置为1.0f
        ImGuiStyle &style = ImGui::GetStyle(); // 获取ImGui样式接口
        style.ScaleAllSizes(main_scale);       // 应用缩放因子
        style.FontScaleDpi = main_scale;       // 应用缩放因子到字体

        // 5- 初始化透明度
        float window_alpha = 0.3f; // 设置窗口透明度为30%

        // 6- 应用透明度
        style.Colors[ImGuiCol_WindowBg].w = window_alpha; // 应用透明度到窗口背景
        style.Colors[ImGuiCol_PopupBg].w = window_alpha;  // 应用透明度到弹窗背景

        // 7- 初始化字体
        ImFont *font = io.Fonts->AddFontFromFileTTF("assets/fonts/VonwaonBitmap-16px.ttf",              // 字体文件路径
                                                    16.0f,                                              // 字体大小
                                                    nullptr,                                            // 字体配置
                                                    io.Fonts->GetGlyphRangesChineseSimplifiedCommon()); // 添加默认字体
        if (!font)
        {
            io.Fonts->AddFontDefault();
            spdlog::error("初始化字体失败，不支持中文");
        }

        // 8- 初始化后端渲染器
        ImGui_ImplSDL3_InitForSDLRenderer(window_, sdl_renderer_);
        ImGui_ImplSDLRenderer3_Init(sdl_renderer_);

        // 9- 清理ImGui上下文，详细参看clean()
        spdlog::info("ImGui初始化完成");
        return true;
    }

    void GameApp::onQuitEvent()
    {
        spdlog::trace("GameApp 收到来自事件分发器的退出请求。");
        is_running_ = false;
    }

} // namespace engine::core