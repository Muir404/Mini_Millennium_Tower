#include "engine/core/game_app.h"
#include "engine/core/context.h"
#include "game/scene/title_scene.h"
#include "engine/utils/events.h"
#include <spdlog/spdlog.h>
#include <SDL3/SDL_main.h>
#include <entt/entt.hpp>

// 注册初始场景
void setupInitialScene(engine::core::Context &context)
{
    // GameApp在调用run方法之前，先创建并设置初始场景
    auto title_scene = std::make_unique<game::scene::TitleScene>(context);
    context.getDispatcher().trigger<engine::utils::PushSceneEvent>(engine::utils::PushSceneEvent{std::move(title_scene)});
}

// 主函数
int main(int, char *[])
{
    spdlog::set_level(spdlog::level::info);

    spdlog::info("游戏开始");

    engine::core::GameApp app;
    app.registerSceneSetup(setupInitialScene);
    app.run();
    return 0;
}