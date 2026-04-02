#include "engine/core/game_app.h"
#include "engine/core/context.h"
#include "game/scene/title_scene.h"
#include "engine/utils/events.h"
#include <spdlog/spdlog.h>
#include <SDL3/SDL_main.h>
#include <entt/entt.hpp>

/// @brief 注册初始场景设置函数
/// @param context 游戏上下文对象
/// @note 游戏在调用run方法之前，会自动调用此函数设置初始场景
void setupInitialScene(engine::core::Context &context)
{
    // GameApp在调用run方法之前，先创建并设置初始场景
    auto title_scene = std::make_unique<game::scene::TitleScene>(context);
    context.getDispatcher().trigger<engine::utils::PushSceneEvent>(engine::utils::PushSceneEvent{std::move(title_scene)});
}

/// @brief 主函数
/// @param argc 命令行参数数量
/// @param argv 命令行参数数组
/// @return int 程序退出状态码
int main(int, char *[])
{
    spdlog::set_level(spdlog::level::info); // 设置日志级别为info，只打印info及以上的日志

    spdlog::info("游戏开始");
    engine::core::GameApp app;
    app.registerSceneSetup(setupInitialScene); // 注册初始场景设置函数
    app.run();
    return 0;
}