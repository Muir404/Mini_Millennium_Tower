#include "game_scene.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/core/context.h"
#include <entt/signal/sigh.hpp>
#include "spdlog/spdlog.h"

namespace game::scene
{
    GameScene::GameScene(engine::core::Context &context, engine::scene::SceneManager &scene_manager)
        : engine::scene::Scene("GameScene", context, scene_manager)
    {
    }
    GameScene::~GameScene()
    {
    }
    void GameScene::init()
    {
        // 注册输入回调时间 j k
        auto &input_manager = context_.getInputManager();
        input_manager.onAction("attack").connect<&GameScene::onAttack>(this);
        input_manager.onAction("jump", engine::input::ActionState::RELEASED).connect<&GameScene::onJump>(this);
    }
    void GameScene::clean()
    {
        // 断开输入回调-谁连接谁断开
        auto &input_manager = context_.getInputManager();
        input_manager.onAction("attack").disconnect<&GameScene::onAttack>(this);
        input_manager.onAction("jump", engine::input::ActionState::RELEASED).disconnect<&GameScene::onJump>(this);
    }
    void GameScene::onAttack()
    {
        spdlog::info("攻击!");
    }
    void GameScene::onJump()
    {
        spdlog::info("跳跃!");
    }
}