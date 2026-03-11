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
}