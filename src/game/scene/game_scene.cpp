#include "game_scene.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/core/context.h"
#include "../../engine/utils/events.h"
#include <entt/signal/sigh.hpp>
#include "spdlog/spdlog.h"

namespace game::scene
{
    GameScene::GameScene(engine::core::Context &context)
        : engine::scene::Scene("GameScene", context)
    {
    }
    GameScene::~GameScene()
    {
    }
    void GameScene::init()
    {
        static int count = 0; // 场景计数器
        scene_num_ = count++; // 为当前场景分配编号
        spdlog::info("[GameScene] 初始化场景: {}", scene_num_);

        // 注册输入回调时间
        auto &input_manager = context_.getInputManager();
        input_manager.onAction("jump").connect<&GameScene::onReplace>(this);
        input_manager.onAction("mouse_left").connect<&GameScene::onPush>(this);
        input_manager.onAction("mouse_right").connect<&GameScene::onPop>(this);
        input_manager.onAction("pause").disconnect<&GameScene::onQuit>(this);

        Scene::init();
    }
    void GameScene::clean()
    {
        // 断开输入回调-谁连接谁断开
        auto &input_manager = context_.getInputManager();
        input_manager.onAction("jump").disconnect<&GameScene::onReplace>(this);
        input_manager.onAction("mouse_left").disconnect<&GameScene::onPush>(this);
        input_manager.onAction("mouse_right").disconnect<&GameScene::onPop>(this);
        input_manager.onAction("pause").connect<&GameScene::onQuit>(this);

        Scene::clean();
    }
    bool GameScene::onReplace()
    {
        spdlog::info("[GameScene] 替换场景");
        requestReplaceScene(std::make_unique<game::scene::GameScene>(context_));
        return true;
    }
    bool GameScene::onPush()
    {
        spdlog::info("[GameScene] 压栈场景");
        requestPushScene(std::make_unique<game::scene::GameScene>(context_));
        return true;
    }
    bool GameScene::onPop()
    {
        spdlog::info("[GameScene] 出栈场景{}", scene_num_);
        requestPopScene();
        return true;
    }
    bool GameScene::onQuit()
    {
        spdlog::info("[GameScene] 退出游戏");
        quit();
        return true;
    }
}