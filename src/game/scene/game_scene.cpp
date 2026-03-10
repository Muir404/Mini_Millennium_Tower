#include "game_scene.h"
#include "../loader/entity_builder_TD.h"
#include "../defs/tags.h"
#include "../system/fwd.h"
#include "../system/remove_dead_system.h"
#include "../system/followpath_system.h"
#include "../defs/events.h"
#include "../component/enemy_component.h"

#include "../../engine/core/context.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/velocity_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/render_component.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/audio/audio_player.h"
#include "../../engine/resource/resource_manager.h"
#include "../../engine/render/text_renderer.h"
#include "../../engine/system/render_system.h"
#include "../../engine/system/movement_system.h"
#include "../../engine/system/animation_system.h"
#include "../../engine/system/ysort_system.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_image.h"
#include "../../engine/ui/ui_label.h"
#include "../../engine/loader/level_loader.h"
#include "../../engine/loader/basic_entity_builder.h"

#include <unordered_map>
#include <entt/core/hashed_string.hpp>
#include <entt/signal/sigh.hpp>
#include <spdlog/spdlog.h>

using namespace entt::literals;

namespace game::scene
{

    GameScene::GameScene(engine::core::Context &context)
        : engine::scene::Scene("GameScene", context)
    {
        // 初始化系统
        render_system_ = std::make_unique<engine::system::RenderSystem>();
        movement_system_ = std::make_unique<engine::system::MovementSystem>();
        animation_system_ = std::make_unique<engine::system::AnimationSystem>();
        ysort_system_ = std::make_unique<engine::system::YSortSystem>();

        followpath_system_ = std::make_unique<game::system::FollowPathSystem>();
        remove_dead_system_ = std::make_unique<game::system::RemoveDeadSystem>();

        spdlog::info("GameScene 构造完成");
    }

    GameScene::~GameScene()
    {
    }

    void GameScene::init()
    {
        if (!loadLevel())
        {
            spdlog::error("加载关卡失败");
            return;
        }
        if (!initEventConnections())
        {
            spdlog::error("初始化事件连接失败");
            return;
        }
        createTestEnemy();
        Scene::init();
    }

    void GameScene::update(float delta_time)
    {
        auto &dispatcher = context_.getDispatcher();

        remove_dead_system_->update(registry_);                             // 删除死亡实体，优先处理
        followpath_system_->update(registry_, dispatcher, waypoint_nodes_); // 跟随系统，更新实体跟随目标
        movement_system_->update(registry_, delta_time);                    // 更新移动, 确保在动画之前更新，在跟随系统之后更新
        animation_system_->update(registry_, delta_time);                   // 更新动画
        ysort_system_->update(registry_);                                   // Y轴排序，确保渲染顺序正确，在movement_system_之后更新

        Scene::update(delta_time);
    }

    void GameScene::render()
    {
        render_system_->update(registry_, context_.getRenderer(), context_.getCamera());

        Scene::render();
    }

    void GameScene::clean()
    {
        auto &dispatcher = context_.getDispatcher();
        dispatcher.disconnect(this); // 断开所有与当前场景相关的事件连接
        Scene::clean();
    }

    bool GameScene::loadLevel()
    {
        engine::loader::LevelLoader level_loader;

        level_loader.setEntityBuilder(std::make_unique<game::loader::EntityBuilderTD>(level_loader,
                                                                                      context_,
                                                                                      registry_,
                                                                                      waypoint_nodes_,
                                                                                      start_points_));

        if (!level_loader.loadLevel("assets/maps/level1.tmj", this))
        {
            spdlog::error("加载关卡失败");
            return false;
        }
        return true;
    }
    bool GameScene::initEventConnections()
    {
        auto &dispatcher = context_.getDispatcher();
        dispatcher.sink<game::defs::EnemyArriveHomeEvent>().connect<&GameScene::onEnemyArriveHome>(this);
        return true;
    }
    void GameScene::onEnemyArriveHome(const game::defs::EnemyArriveHomeEvent &event)
    {
    }

    void GameScene::createTestEnemy()
    {
        // 测试函数
        // 每个起点创建一个敌人
        for (auto start_index : start_points_)
        {
            auto position = waypoint_nodes_[start_index].position_;

            auto enemy = registry_.create();
            registry_.emplace<engine::component::TransformComponent>(enemy, position);
            registry_.emplace<engine::component::VelocityComponent>(enemy, glm::vec2(0, 0));
            registry_.emplace<game::component::EnemyComponent>(enemy, start_index, 100.0f);

            auto sprite = engine::component::Sprite("assets/textures/Enemy/wolf.png", engine::utils::Rect(0, 0, 192, 192));
            // 设置精灵组件时，需设置偏移量以对齐中心点位置(否则会默认以左上角为中心点)
            registry_.emplace<engine::component::SpriteComponent>(enemy, std::move(sprite), glm::vec2(192, 192), glm::vec2(-96, -128));
            // 暂定主战斗图层编号为10
            registry_.emplace<engine::component::RenderComponent>(enemy, 10);
        }
    }
} // namespace game::scene