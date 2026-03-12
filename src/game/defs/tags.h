#pragma once

// entt中空的结构提可作为标签使用
namespace game::defs
{
    /**
     * @brief 死亡标签
     *
     */
    struct DeadTag
    {
    };

    /**
     * @brief 面向左侧标签
     *
     */
    struct FaceLeftTag
    {
    };

    /**
     * @brief 近战单位标签
     *
     */
    struct MeleeUnitTag
    {
    };

    /**
     * @brief 远程单位标签
     *
     */
    struct RangedUnitTag
    {
    };

    /**
     * @brief 治疗单位标签
     *
     */
    struct HealerTag
    {
    };

    /**
     * @brief 攻击准备就绪标签
     *
     */
    struct AttackReadyTag
    {
    };

    /**
     * @brief 受伤标签，存在HP损失
     */
    struct InjuredTag
    {
    };

    /**
     * @brief 行动锁定标签，单位行动被锁定，播完动画后才能继续行动
     */
    struct ActionLockTag
    {
    };
}