#include "title_scene.h"
#include "game_scene.h"
#include "../data/ui_config.h"
#include "../data/session_data.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/core/context.h"
#include "../../engine/core/time.h"
#include "../../engine/core/game_state.h"
#include "../../engine/audio/audio_player.h"
#include "../../engine/utils/events.h"
#include "../../engine/system/render_system.h"
#include "../../engine/system/ysort_system.h"
#include "../../engine/system/animation_system.h"
#include "../../engine/system/movement_system.h"
#include "../../engine/loader/level_loader.h"
#include "../../engine/loader/basic_entity_builder.h"
#include "../system/debug_ui_system.h"
#include <spdlog/spdlog.h>
#include <entt/entity/registry.hpp>

using namespace entt::literals;

namespace game::scene
{

    TitleScene::TitleScene(engine::core::Context &context,
                           std::shared_ptr<game::factory::BlueprintManager> blueprint_manager,
                           std::shared_ptr<game::data::SessionData> session_data,
                           std::shared_ptr<game::data::UIConfig> ui_config,
                           std::shared_ptr<game::data::LevelConfig> level_config)
        : engine::scene::Scene("TitleScene", context),
          blueprint_manager_(blueprint_manager),
          session_data_(session_data),
          ui_config_(ui_config),
          level_config_(level_config)
    {
    }

    TitleScene::~TitleScene() = default;

    void TitleScene::init()
    {
        if (!initSessionData())
        {
            spdlog::error("初始化session_data_失败");
            return;
        }
        if (!initLevelConfig())
        {
            spdlog::error("初始化关卡配置失败");
            return;
        }
        if (!initBlueprintManager())
        {
            spdlog::error("初始化蓝图管理器失败");
            return;
        }
        if (!initUIConfig())
        {
            spdlog::error("初始化UI配置失败");
            return;
        }
        if (!loadTitleLevel())
        {
            spdlog::error("加载关卡失败");
            return;
        }
        if (!initSystems())
        {
            spdlog::error("初始化系统失败");
            return;
        }
        if (!initRegistryContext())
        {
            spdlog::error("初始化注册表上下文失败");
            return;
        }
        if (!initUI())
        {
            spdlog::error("初始化UI失败");
            return;
        }

        context_.getGameState().setState(engine::core::State::Title);
        context_.getTime().setTimeScale(1.0f); // 重置游戏速度

        context_.getAudioPlayer().playMusic("title_bgm"_hs); // 设置标题场景背景音乐

        engine::scene::Scene::init();
    }

    void TitleScene::update(float delta_time)
    {
        engine::scene::Scene::update(delta_time);
        animation_system_->update(delta_time);
        movement_system_->update(registry_, delta_time);
        ysort_system_->update(registry_);
    }

    void TitleScene::render()
    {
        auto &renderer = context_.getRenderer();
        auto &camera = context_.getCamera();

        render_system_->update(registry_, renderer, camera);

        engine::scene::Scene::render();
        debug_ui_system_->updateTitle(*this);
    }

    bool TitleScene::initSessionData()
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
        return true;
    }

    bool TitleScene::initLevelConfig()
    {
        if (!level_config_)
        {
            level_config_ = std::make_shared<game::data::LevelConfig>();
            if (!level_config_->loadFromFile("assets/data/level_config.json"))
            {
                spdlog::error("加载关卡配置失败");
                return false;
            }
        }
        return true;
    }

    bool TitleScene::initBlueprintManager()
    {
        if (!blueprint_manager_)
        {
            blueprint_manager_ = std::make_shared<game::factory::BlueprintManager>(context_.getResourceManager());
            if (!blueprint_manager_->loadEnemyClassBlueprints("assets/data/enemy_data.json") ||
                !blueprint_manager_->loadPlayerClassBlueprints("assets/data/player_data.json") ||
                !blueprint_manager_->loadProjectileBlueprints("assets/data/projectile_data.json") ||
                !blueprint_manager_->loadEffectBlueprints("assets/data/effect_data.json") ||
                !blueprint_manager_->loadSkillBlueprints("assets/data/skill_data.json"))
            {
                spdlog::error("加载蓝图失败");
                return false;
            }
        }
        return true;
    }

    bool TitleScene::initUIConfig()
    {
        if (!ui_config_)
        {
            ui_config_ = std::make_shared<game::data::UIConfig>();
            if (!ui_config_->loadFromFile("assets/data/ui_config.json"))
            {
                spdlog::error("加载UI配置失败");
                return false;
            }
        }
        return true;
    }

    bool TitleScene::loadTitleLevel()
    {
        engine::loader::LevelLoader level_loader;
        if (!level_loader.loadLevel("assets/maps/title.tmj", this))
        {
            spdlog::error("加载标题关卡失败");
            return false;
        }
        return true;
    }

    bool TitleScene::initSystems()
    {
        // 初始化系统
        auto &dispatcher = context_.getDispatcher();
        debug_ui_system_ = std::make_unique<game::system::DebugUISystem>(registry_, context_);
        render_system_ = std::make_unique<engine::system::RenderSystem>();
        ysort_system_ = std::make_unique<engine::system::YSortSystem>();
        animation_system_ = std::make_unique<engine::system::AnimationSystem>(registry_, dispatcher);
        movement_system_ = std::make_unique<engine::system::MovementSystem>();
        return true;
    }

    bool TitleScene::initRegistryContext()
    {
        // 让注册表存储一些数据类型实例作为上下文，方便使用
        registry_.ctx().emplace<std::shared_ptr<game::data::SessionData>>(session_data_);
        registry_.ctx().emplace<std::shared_ptr<game::factory::BlueprintManager>>(blueprint_manager_);
        registry_.ctx().emplace<std::shared_ptr<game::data::UIConfig>>(ui_config_);
        return true;
    }

    bool TitleScene::initUI()
    {
        auto window_size = context_.getGameState().getLogicalSize();
        if (!ui_manager_->init(window_size))
            return false;

        // 设置背景音乐
        // context_.getAudioPlayer().playMusic("title_bgm"_hs);

        /* 先用ImGui实现UI，未来再使用游戏内UI */
        return true;
    }

    void TitleScene::onStartGameClick()
    {
        // 如果数据是读档载入的，有可能已经通关，此时需要进入下一关
        if (session_data_->isLevelClear())
        {
            session_data_->setLevelClear(false);
            session_data_->addOneLevel();
        }
        requestReplaceScene(std::make_unique<game::scene::GameScene>(
            context_,
            blueprint_manager_,
            session_data_,
            ui_config_,
            level_config_));
    }

    void TitleScene::onConfirmRoleClick()
    {
        show_unit_info_ = !show_unit_info_;
        /* 用ImGui快速实现逻辑，未来再完善游戏内UI */
    }

    void TitleScene::onLoadGameClick()
    {
        show_load_panel_ = !show_load_panel_;
        /* 用ImGui快速实现逻辑，未来再完善游戏内UI */
    }

    void TitleScene::onQuitClick()
    {
        quit();
    }

} // namespace game::scene