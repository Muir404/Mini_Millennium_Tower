#pragma once
#include <entt/entity/registry.hpp>

namespace game::system
{
    class SetTargetSystem
    {
    public:
        void update(entt::registry &registry);

    private:
        void updateHasTarget(entt::registry &registry);      ///< 处理有目标的角色
        void updateNoTargetPlayer(entt::registry &registry); ///< 处理无目标的玩家角色
        void updateNoTargetEnemy(entt::registry &registry);  ///< 处理无目标的敌人角色
        void updateHealer(entt::registry &registry);         ///< 处理治疗者角色
    };
} // namespace game::system