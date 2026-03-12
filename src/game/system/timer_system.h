#pragma once
#include <entt/entity/fwd.hpp>

namespace game::system
{
    /**
     * @brief 定时器系统，用于更新实体的定时器
     */
    class TimerSystem
    {
    public:
        void update(entt::registry &registry, float delta_time);

    private:
        void updateAttackTimer(entt::registry &registry, float delta_time); ///< 更新攻击定时器
    };
} // namespace game::system