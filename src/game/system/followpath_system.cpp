#include "followpath_system.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/velocity_component.h"
#include "../../engine/utils/math.h"
#include "../defs/events.h"
#include "../defs/tags.h"
#include "../component/enemy_component.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>

namespace game::system
{
    void FollowPathSystem::update(entt::registry &registry,
                                  entt::dispatcher &dispatcher,
                                  std::unordered_map<int, game::data::WaypointNode> &waypoint_nodes)
    {
        spdlog::trace("FollowPathSystem::update");

        // 筛选依据：速度组件、变换组件、敌人组件
        auto view = registry.view<engine::component::VelocityComponent,
                                  engine::component::TransformComponent,
                                  game::component::EnemyComponent>();

        for (auto entity : view)
        {
            auto &velocity = registry.get<engine::component::VelocityComponent>(entity);
            auto &transform = registry.get<engine::component::TransformComponent>(entity);
            auto &enemy = registry.get<game::component::EnemyComponent>(entity);

            // 获取目标节点
            auto target_node = waypoint_nodes.at(enemy.target_waypoint_id_);

            // 计算方向向量
            glm::vec2 direction = target_node.position_ - transform.position_;

            // 如果小于阈值，移动到下一个节点，注意阈值不要设置太小，否则会震荡（速度过快）
            if (glm::length(direction) < 3.0f)
            {
                // 如果下一个节点ID列表为空，说明到达了终点，则发送信号并添加删除标记
                auto size = target_node.next_node_ids_.size();
                if (size == 0)
                {
                    spdlog::info("敌人到达了终点");
                    dispatcher.enqueue<game::defs::EnemyArriveHomeEvent>();
                    registry.emplace<game::defs::DeadTag>(entity);
                    continue;
                }
                // 随机选择下一个节点
                auto target_index = engine::utils::randomInt(0, size - 1);
                enemy.target_waypoint_id_ = target_node.next_node_ids_.at(target_index);
                // 更新目标节点与方向向量
                target_node = waypoint_nodes.at(enemy.target_waypoint_id_);
                direction = target_node.position_ - transform.position_;
            }

            // 更新速度组件：归一化方向向量 * 移动速度
            velocity.velocity_ = glm::normalize(direction) * enemy.speed_;
        }
    }
}