#pragma once
#include "../../engine/scene/scene.h"
#include "../../engine/system/fwd.h"
#include "../../game/data/waypoint_node.h"
#include "../defs/tags.h"
#include "../defs/events.h"
#include "../system/fwd.h"
#include <memory>

namespace game::factory
{
    class EntityFactory;
    class BlueprintManager;
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

        std::unique_ptr<game::system::FollowPathSystem> followpath_system_;  ///< 跟随系统
        std::unique_ptr<game::system::RemoveDeadSystem> remove_dead_system_; ///< 删除死亡系统
        std::unique_ptr<game::system::BlockSystem> block_system_;            ///< 阻挡系统

        std::unordered_map<int, game::data::WaypointNode> waypoint_nodes_; // 路径节点ID到节点数据的映射
        std::vector<int> start_points_;                                    // 起始点的ID列表

        std::unique_ptr<game::factory::EntityFactory> entity_factory_;       ///< 实体工厂
        std::shared_ptr<game::factory::BlueprintManager> blueprint_manager_; ///< 蓝图管理器

    public:
        GameScene(engine::core::Context &context);
        ~GameScene();

        void init() override;
        void update(float delta_time) override;
        void render() override;
        void clean() override;

    private:
        [[nodiscard]] bool loadLevel();
        [[nodiscard]] bool initEventConnections();
        [[nodiscard]] bool initInputConnections();
        [[nodiscard]] bool initEntityFactory();

        void onEnemyArriveHome(const game::defs::EnemyArriveHomeEvent &event);

        void createTestEnemy();
        bool onCreateTestPlayerMelee();
        bool onCreateTestPlayerRanged();
        bool onClearAllPlayers();
    };

} // namespace game::scene