#pragma once
#include <entt/entity/entity.hpp>
#include <glm/vec2.hpp>

namespace game::defs
{
    /**
     * @brief 敌人到达了终点
     */
    struct EnemyArriveHomeEvent
    {
    };

    /**
     * @brief 攻击（命中）事件
     *
     */
    struct AttackEvent
    {
        entt::entity attacker_{entt::null}; ///< @brief 攻击者
        entt::entity target_{entt::null};   ///< @brief 目标
        float damage_{};                    ///< @brief 原始伤害
    };

    /**
     * @brief 治疗（命中）事件
     *
     */
    struct HealEvent
    {
        entt::entity healer_{entt::null}; ///< @brief 治疗者
        entt::entity target_{entt::null}; ///< @brief 目标
        float amount_{};                  ///< @brief 治疗量
    };

    /**
     * @brief 发射项目事件
     */
    struct EmitProjectileEvent
    {
        entt::id_type id_{entt::null};    ///< @brief 投射物id
        entt::entity target_{entt::null}; ///< @brief 目标
        glm::vec2 start_position_{};      ///< @brief 发射位置
        glm::vec2 target_position_{};     ///< @brief 目标位置
        float damage_{};                  ///< @brief 伤害量
    };

    /**
     * @brief 敌人死亡特效事件
     */
    struct EnemyDeadEffectEvent
    {
        entt::id_type class_id_{entt::null}; ///< @brief 敌人ID
        glm::vec2 position_{};               ///< @brief 敌人位置
        bool is_flipped_{false};             ///< @brief 是否面向左侧
    };

} // namespace game::defs
