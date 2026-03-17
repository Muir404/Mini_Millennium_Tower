#include "game_scene.h"
#include "../loader/entity_builder_TD.h"

#include "../defs/tags.h"
#include "../defs/events.h"

#include "../component/enemy_component.h"
#include "../component/player_component.h"
#include "../component/stats_component.h"
#include "../component/target_component.h"
#include "../component/projectile_component.h"

#include "../data/session_data.h"
#include "../data/ui_config.h"
#include "../data/game_stats.h"
#include "../data/level_config.h"
#include "../data/level_data.h"

#include "../system/fwd.h"
#include "../system/remove_dead_system.h"
#include "../system/followpath_system.h"
#include "../system/block_system.h"
#include "../system/attack_starter_system.h"
#include "../system/orientation_system.h"
#include "../system/timer_system.h"
#include "../system/set_target_system.h"
#include "../system/animation_state_system.h"
#include "../system/animation_event_system.h"
#include "../system/combat_resolve_system.h"
#include "../system/projectile_system.h"
#include "../system/effect_system.h"
#include "../system/health_bar_system.h"
#include "../system/game_rule_system.h"
#include "../system/place_unit_system.h"
#include "../system/render_range_system.h"
#include "../system/debug_ui_system.h"

#include "../ui/units_portrait_ui.h"

#include "../factory/entity_factory.h"
#include "../factory/blueprint_manager.h"

#include "../spawner/enemy_spawner.h"

#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"

#include "../../engine/input/input_manager.h"

#include "../../engine/audio/audio_player.h"

#include "../../engine/resource/resource_manager.h"

#include "../../engine/render/text_renderer.h"

#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/velocity_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/render_component.h"
#include "../../engine/component/audio_component.h"

#include "../../engine/system/render_system.h"
#include "../../engine/system/movement_system.h"
#include "../../engine/system/animation_system.h"
#include "../../engine/system/ysort_system.h"
#include "../../engine/system/audio_system.h"

#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_interactive.h"
#include "../../engine/ui/ui_element.h"
#include "../../engine/ui/ui_image.h"
#include "../../engine/ui/ui_label.h"
#include "../../engine/ui/ui_panel.h"
#include "../../engine/ui/ui_button.h"

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

        spdlog::info("GameScene 构造完成");
    }

    GameScene::~GameScene()
    {
    }

    void GameScene::init()
    {
        if (!initSessionData())
        {
            spdlog::error("初始化session_data_失败");
            return;
        }
        if (!initLevelConfig())
        {
            spdlog::error("初始化level_config_失败");
            return;
        }
        if (!initUIConfig())
        {
            spdlog::error("初始化ui_config_失败");
            return;
        }
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
        if (!initInputConnections())
        {
            spdlog::error("初始化输入连接失败");
            return;
        }
        if (!initEntityFactory())
        {
            spdlog::error("初始化实体工厂失败");
            return;
        }
        if (!initRegistryContext())
        {
            spdlog::error("初始化注册表上下文失败");
            return;
        }
        if (!initUnitsPortraitUI())
        {
            spdlog::error("初始化单位肖像UI失败");
            return;
        }
        if (!initSystems())
        {
            spdlog::error("初始化系统失败");
            return;
        }
        if (!initEnemySpawner())
        {
            spdlog::error("初始化敌人生成器失败");
            return;
        }
        Scene::init();
    }

    void GameScene::update(float delta_time)
    {
        auto &dispatcher = context_.getDispatcher();

        // 每一帧最先清理死亡实体(要在dispatcher处理完事件后再清理，因此放在下一帧开头)
        remove_dead_system_->update(registry_);

        // 注意系统更新的顺序
        timer_system_->update(registry_, delta_time);
        game_rule_system_->update(delta_time);
        block_system_->update(registry_, dispatcher);
        set_target_system_->update(registry_);
        followpath_system_->update(registry_, dispatcher, waypoint_nodes_);
        orientation_system_->update(registry_); // 调用顺序要在Block、SetTarget、FollowPath之后
        attack_starter_system_->update(registry_, dispatcher);
        projectile_system_->update(delta_time);
        movement_system_->update(registry_, delta_time);
        animation_system_->update(delta_time);
        place_unit_system_->update(delta_time);
        ysort_system_->update(registry_); // 调用顺序要在MovementSystem之后

        enemy_spawner_->update(delta_time);
        units_portrait_ui_->update(delta_time);
        Scene::update(delta_time);
    }

    void GameScene::render()
    {
        auto &renderer = context_.getRenderer();
        auto &camera = context_.getCamera();

        // 渲染顺序
        render_system_->update(registry_, renderer, camera);
        health_bar_system_->update(registry_, renderer, camera);
        render_range_system_->update(registry_, renderer, camera);

        Scene::render();

        debug_ui_system_->update();
    }

    void GameScene::clean()
    {
        auto &dispatcher = context_.getDispatcher();
        auto &input_manager = context_.getInputManager();
        // 断开所有事件连接
        dispatcher.disconnect(this);
        // 断开输入信号连接
        input_manager.onAction("pause"_hs).disconnect<&GameScene::onClearAllPlayers>(this);
        Scene::clean();
    }

    bool GameScene::initSessionData()
    {
        if (!session_data_)
        {
            session_data_ = std::make_shared<game::data::SessionData>();
            if (!session_data_->loadDefaultData())
            {
                spdlog::error("初始化session_data_失败");
                return false;
            }
        }
        level_number_ = session_data_->getLevelNumber();
        return true;
    }

    bool GameScene::initLevelConfig()
    {
        if (!level_config_)
        {
            level_config_ = std::make_shared<game::data::LevelConfig>();
            if (!level_config_->loadFromFile("assets/data/level_config.json"))
            {
                spdlog::error("初始化level_config_失败");
                return false;
            }
        }
        waves_ = level_config_->getWavesData(level_number_);
        game_stats_.enemy_count_ = level_config_->getTotalEnemyCount(level_number_);
        return true;
    }

    bool GameScene::initUIConfig()
    {
        if (!ui_config_)
        {
            ui_config_ = std::make_shared<game::data::UIConfig>();
            if (!ui_config_->loadFromFile("assets/data/ui_config.json"))
            {
                spdlog::error("初始化ui_config_失败");
                return false;
            }
        }
        return true;
    }

    bool GameScene::loadLevel()
    {
        engine::loader::LevelLoader level_loader;

        level_loader.setEntityBuilder(std::make_unique<game::loader::EntityBuilderTD>(level_loader,
                                                                                      context_,
                                                                                      registry_,
                                                                                      waypoint_nodes_,
                                                                                      start_points_));

        auto map_path = level_config_->getMapPath(level_number_);
        if (!level_loader.loadLevel(map_path, this))
        {
            spdlog::error("加载关卡失败");
            return false;
        }
        return true;
    }

    bool GameScene::initEventConnections()
    {
        return true;
    }

    bool GameScene::initEntityFactory()
    {
        if (!blueprint_manager_)
        {
            blueprint_manager_ = std::make_shared<game::factory::BlueprintManager>(context_.getResourceManager());
            if (!blueprint_manager_->loadEnemyClassBlueprints("assets/data/enemy_data.json") ||
                !blueprint_manager_->loadPlayerClassBlueprints("assets/data/player_data.json") ||
                !blueprint_manager_->loadProjectileBlueprints("assets/data/projectile_data.json"))
            {
                spdlog::error("加载蓝图失败");
                return false;
            }
        }
        entity_factory_ = std::make_unique<game::factory::EntityFactory>(registry_, *blueprint_manager_);
        spdlog::info("实体工厂初始化完成");
        return true;
    }

    bool GameScene::initSystems()
    {
        auto &dispatcher = context_.getDispatcher();

        // 初始化系统
        render_system_ = std::make_unique<engine::system::RenderSystem>();
        movement_system_ = std::make_unique<engine::system::MovementSystem>();
        animation_system_ = std::make_unique<engine::system::AnimationSystem>(registry_, dispatcher);
        ysort_system_ = std::make_unique<engine::system::YSortSystem>();
        audio_system_ = std::make_unique<engine::system::AudioSystem>(registry_, context_);

        followpath_system_ = std::make_unique<game::system::FollowPathSystem>();
        remove_dead_system_ = std::make_unique<game::system::RemoveDeadSystem>();
        block_system_ = std::make_unique<game::system::BlockSystem>();
        set_target_system_ = std::make_unique<game::system::SetTargetSystem>();
        attack_starter_system_ = std::make_unique<game::system::AttackStarterSystem>();
        orientation_system_ = std::make_unique<game::system::OrientationSystem>();
        timer_system_ = std::make_unique<game::system::TimerSystem>();
        animation_state_system_ = std::make_unique<game::system::AnimationStateSystem>(registry_, dispatcher);
        animation_event_system_ = std::make_unique<game::system::AnimationEventSystem>(registry_, dispatcher);
        combat_resolve_system_ = std::make_unique<game::system::CombatResolveSystem>(registry_, dispatcher);
        projectile_system_ = std::make_unique<game::system::ProjectileSystem>(registry_, dispatcher, *entity_factory_);
        effect_system_ = std::make_unique<game::system::EffectSystem>(registry_, dispatcher, *entity_factory_);
        health_bar_system_ = std::make_unique<game::system::HealthBarSystem>();
        game_rule_system_ = std::make_unique<game::system::GameRuleSystem>(registry_, dispatcher);
        place_unit_system_ = std::make_unique<game::system::PlaceUnitSystem>(registry_, *entity_factory_, context_);
        render_range_system_ = std::make_unique<game::system::RenderRangeSystem>();
        debug_ui_system_ = std::make_unique<game::system::DebugUISystem>(registry_, context_);

        spdlog::info("系统初始化完成");
        return true;
    }

    bool GameScene::initEnemySpawner()
    {
        enemy_spawner_ = std::make_unique<game::spawner::EnemySpawner>(registry_, *entity_factory_);
        spdlog::info("敌人生成器初始化完成");
        return true;
    }

    bool GameScene::initInputConnections()
    {
        auto &input_manager = context_.getInputManager();
        input_manager.onAction("pause"_hs).connect<&GameScene::onClearAllPlayers>(this);
        return true;
    }

    bool GameScene::initRegistryContext()
    {
        // 1. 核心系统管理器 (其他系统依赖的基础设施)
        registry_.ctx().emplace<std::shared_ptr<game::factory::BlueprintManager>>(blueprint_manager_);

        // 2. 游戏核心配置 (依赖蓝图管理器来解析/创建)
        registry_.ctx().emplace<std::shared_ptr<game::data::LevelConfig>>(level_config_);
        registry_.ctx().emplace<int>(level_number_);
        registry_.ctx().emplace<game::data::Waves>(waves_);

        // 3. 游戏运行状态
        registry_.ctx().emplace<game::data::GameStats>(game_stats_);
        registry_.ctx().emplace<std::shared_ptr<game::data::SessionData>>(session_data_);

        // 4. 场景/地图数据
        registry_.ctx().emplace<std::unordered_map<int, game::data::WaypointNode> &>(waypoint_nodes_);
        registry_.ctx().emplace<std::vector<int> &>(start_points_);

        // 5. UI/其他配置 (可能最后使用)
        registry_.ctx().emplace<std::shared_ptr<game::data::UIConfig>>(ui_config_);

        spdlog::info("初始化游戏上下文");
        return true;
    }

    bool GameScene::initUnitsPortraitUI()
    {
        try
        {
            units_portrait_ui_ = std::make_unique<game::ui::UnitsPortraitUI>(registry_, *ui_manager_, context_);
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化单位肖像UI失败: {}", e.what());
            return false;
        }
        return true;
    }

    bool GameScene::onClearAllPlayers()
    {
        auto view = registry_.view<game::component::PlayerComponent>();
        for (auto entity : view)
        {
            context_.getDispatcher().enqueue<game::defs::RemovePlayerUnitEvent>(entity);
        }
        return true;
    }

} // namespace game::scene