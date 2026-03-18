#pragma once
#include <entt/entity/fwd.hpp>
#include <entt/signal/dispatcher.hpp>
namespace game::system
{
    /**
     * @brief 定时器系统，用于更新实体的定时器
     */
    class TimerSystem
    {
        entt::registry &registry_;     ///< @brief 实体注册表
        entt::dispatcher &dispatcher_; ///< @brief 事件调度器

    public:
        TimerSystem(entt::registry &registry, entt::dispatcher &dispatcher);

        void update(float delta_time);

    private:
        void updateAttackTimer(float delta_time);        ///< 更新攻击定时器
        void updateSkillCooldownTimer(float delta_time); ///< 更新技能冷却定时器
        void updateSkillDurationTimer(float delta_time); ///< 更新技能持续时间定时器
    };
} // namespace game::system