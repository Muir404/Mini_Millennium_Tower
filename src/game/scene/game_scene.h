#pragma once

#include "../../engine/scene/scene.h"
#include "../../engine/system/fwd.h"

#include "../data/waypoint_node.h"
#include "../data/game_stats.h"
#include "../data/level_data.h"

#include "../system/fwd.h"

#include <memory>

namespace engine::ui
{
    class UIElement;
}

namespace game::ui
{
    class UnitsPortraitUI;
}

namespace game::factory
{
    class EntityFactory;
    class BlueprintManager;
}

namespace game::data
{
    class SessionData;
    class UIConfig;
    class LevelConfig;
}

namespace game::spawner
{
    class EnemySpawner;
}

namespace game::scene
{

    class GameScene final : public engine::scene::Scene
    {
    private:
        std::unique_ptr<engine::system::RenderSystem> render_system_;       ///< 渲染系统
        std::unique_ptr<engine::system::MovementSystem> movement_system_;   ///< 移动系统
        std::unique_ptr<engine::system::AnimationSystem> animation_system_; ///< 动画系统
        std::unique_ptr<engine::system::YSortSystem> ysort_system_;         ///< Y轴排序系统
        std::unique_ptr<engine::system::AudioSystem> audio_system_;         ///< 音频系统

        std::unique_ptr<game::system::FollowPathSystem> followpath_system_;          ///< 跟随系统
        std::unique_ptr<game::system::RemoveDeadSystem> remove_dead_system_;         ///< 删除死亡系统
        std::unique_ptr<game::system::BlockSystem> block_system_;                    ///< 阻挡系统
        std::unique_ptr<game::system::AttackStarterSystem> attack_starter_system_;   ///< 攻击启动系统
        std::unique_ptr<game::system::OrientationSystem> orientation_system_;        ///< 方向系统
        std::unique_ptr<game::system::TimerSystem> timer_system_;                    ///< 定时器系统
        std::unique_ptr<game::system::SetTargetSystem> set_target_system_;           ///< 设置目标系统
        std::unique_ptr<game::system::AnimationStateSystem> animation_state_system_; ///< 动画状态系统
        std::unique_ptr<game::system::AnimationEventSystem> animation_event_system_; ///< 动画事件系统
        std::unique_ptr<game::system::CombatResolveSystem> combat_resolve_system_;   ///< 战斗解析系统
        std::unique_ptr<game::system::ProjectileSystem> projectile_system_;          ///< 投射物系统
        std::unique_ptr<game::system::EffectSystem> effect_system_;                  ///< 特效系统
        std::unique_ptr<game::system::HealthBarSystem> health_bar_system_;           ///< 血量条系统
        std::unique_ptr<game::system::GameRuleSystem> game_rule_system_;             ///< 游戏规则系统
        std::unique_ptr<game::system::PlaceUnitSystem> place_unit_system_;           ///< 放置单位系统
        std::unique_ptr<game::system::RenderRangeSystem> render_range_system_;       ///< 渲染范围系统
        std::unique_ptr<game::system::DebugUISystem> debug_ui_system_;               ///< 调试UI系统

        std::unique_ptr<game::spawner::EnemySpawner> enemy_spawner_;   ///< 敌人生成系统
        std::unique_ptr<game::ui::UnitsPortraitUI> units_portrait_ui_; // 封装的单位肖像UI，负责管理单位肖像UI的创建、更新和排列

        std::unique_ptr<game::factory::EntityFactory> entity_factory_; ///< 实体工厂

        std::shared_ptr<game::factory::BlueprintManager> blueprint_manager_; ///< 蓝图管理器
        std::shared_ptr<game::data::SessionData> session_data_;              ///< 会话数据
        std::shared_ptr<game::data::UIConfig> ui_config_;                    ///< UI配置
        std::shared_ptr<game::data::LevelConfig> level_config_;              ///< 关卡数据

        std::unordered_map<int, game::data::WaypointNode> waypoint_nodes_; // 路径节点ID到节点数据的映射
        std::vector<int> start_points_;                                    // 起始点的ID列表
        game::data::GameStats game_stats_;                                 ///< 游戏内统计数据
        game::data::Waves waves_;                                          ///< 敌人波次数据

        int level_number_{1};

    public:
        GameScene(engine::core::Context &context);
        ~GameScene();

        void init() override;
        void update(float delta_time) override;
        void render() override;
        void clean() override;

    private:
        [[nodiscard]] bool initSessionData();
        [[nodiscard]] bool initLevelConfig();
        [[nodiscard]] bool initUIConfig();
        [[nodiscard]] bool loadLevel();
        [[nodiscard]] bool initEventConnections();
        [[nodiscard]] bool initInputConnections();
        [[nodiscard]] bool initEntityFactory();
        [[nodiscard]] bool initRegistryContext();
        [[nodiscard]] bool initSystems();
        [[nodiscard]] bool initEnemySpawner();
        [[nodiscard]] bool initUnitsPortraitUI();

        bool onClearAllPlayers();
    };

} // namespace game::scene