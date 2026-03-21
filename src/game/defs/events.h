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

    /**
     * @brief 特效事件
     */
    struct EffectEvent
    {
        entt::id_type effect_id_{entt::null}; ///< @brief 效果ID
        glm::vec2 position_{};                ///< @brief 位置
        bool is_flipped_{false};              ///< @brief 是否面向左侧
    };

    /**
     * @brief (创建)准备单位事件
     */
    struct PrepUnitEvent
    {
        entt::id_type name_id_{entt::null};  ///< @brief 单位名称ID
        entt::id_type class_id_{entt::null}; ///< @brief 职业ID
        int cost_{0};                        ///< @brief 费用
    };

    /**
     * @brief 移除角色肖像事件
     */
    struct RemoveUIPortraitEvent
    {
        entt::id_type name_id_{entt::null}; ///< @brief 角色名称ID
    };

    /**
     * @brief 移除玩家单位事件
     */
    struct RemovePlayerUnitEvent
    {
        entt::entity entity_{entt::null}; ///< @brief 单位实体
    };

    /**
     * @brief 技能准备就绪事件
     */
    struct SkillReadyEvent
    {
        entt::entity entity_{entt::null}; ///< @brief 单位实体
    };

    /**
     * @brief 技能激活事件
     */
    struct SkillActiveEvent
    {
        entt::entity entity_{entt::null}; ///< @brief 单位实体
    };

    /**
     * @brief 技能持续结束事件
     */
    struct SkillDurationEndEvent
    {
        entt::entity entity_{entt::null}; ///< @brief 单位实体
    };

    /**
     * @brief 角色肖像悬浮进入事件
     */
    struct UIPortraitHoverEnterEvent
    {
        entt::id_type name_id_{entt::null}; ///< @brief 角色名称ID
    };

    /**
     * @brief 角色肖像悬浮退出事件
     */
    struct UIPortraitHoverLeaveEvent
    {
    };

    /**
     * @brief 升级单位事件
     */
    struct UpgradeUnitEvent
    {
        entt::entity entity_{entt::null}; ///< @brief 单位实体
        int cost_{0};                     ///< @brief 消耗费用
    };

    /**
     * @brief 撤退单位事件
     */
    struct RetreatEvent
    {
        entt::entity entity_{entt::null}; ///< @brief 单位实体
        int cost_{0};                     ///< @brief 返还费用
    };

    /**
     * @brief 重新开始事件
     */
    struct RestartEvent
    {
    };

    /**
     * @brief 返回主菜单事件
     */
    struct BackToTitleEvent
    {
    };

    /**
     * @brief 保存游戏事件
     */
    struct SaveEvent
    {
    };

    /**
     * @brief 通关事件
     */
    struct LevelClearEvent
    {
    };

    /**
     * @brief 通关延迟事件
     */
    struct LevelClearDelayedEvent
    {
        float delay_time_{3.0f}; ///< @brief 延迟时间
    };

    /**
     * @brief 游戏结束事件
     */
    struct GameEndEvent
    {
        bool is_win_{false}; ///< @brief 是否胜利
    };

} // namespace game::defs
