#include "timer_system.h"
#include "../component/stats_component.h"
#include "../defs/tags.h"
#include <entt/entity/registry.hpp>

namespace game::system
{
    void TimerSystem::update(entt::registry &registry, float delta_time)
    {
        updateAttackTimer(registry, delta_time);
    }

    void TimerSystem::updateAttackTimer(entt::registry &registry, float delta_time)
    {
        // 过滤出所有有StatsComponent但没有AttackTag的实体-攻击在冷却中
        auto view_unit = registry.view<component::StatsComponent>(entt::exclude<game::defs::AttackReadyTag>);
        for (auto entity : view_unit)
        {
            auto &stats = view_unit.get<component::StatsComponent>(entity);
            stats.atk_timer_ += delta_time;              // 推进计时器
            if (stats.atk_timer_ >= stats.atk_interval_) // 检查是否到了攻击时间，到了就添加攻击准备就绪标签
            {
                registry.emplace_or_replace<game::defs::AttackReadyTag>(entity); // 标记为攻击准备就绪
                stats.atk_timer_ = 0.0f;                                         // 重置计时器
            }
        }
    }
} // namespace game::system
