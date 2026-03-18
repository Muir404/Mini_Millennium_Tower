#pragma once
#include <glm/vec2.hpp>
#include "../../engine/utils/math.h"

// entt中空的结构提可作为标签使用
namespace game::defs
{
    constexpr float BLOCK_RADIUS{50.0f};                    ///< 块半径
    constexpr float UNIT_RADIUS{25.0f};                     ///< 单位半径
    constexpr float PLACE_RADIUS{50.0f};                    ///< 放置半径
    constexpr float HOVER_RADIUS{35.0f};                    ///< 悬停半径
    constexpr glm::vec2 SKILL_DISPLAY_OFFSET{0.0f, -80.0f}; ///< 技能显示偏移量

    constexpr engine::utils::FColor RANGE_COLOR{0.0f, 1.0f, 0.0f, 0.2f}; ///< 攻击范围显示颜色

    constexpr glm::vec2 HEALTH_BAR_SIZE{50.0f, 10.0f}; ///< 健康条大小
    constexpr float HEALTH_BAR_OFFSET_Y{10.0f};        ///< 健康条偏移量

    /**
     * @brief 玩家职业类型
     */
    enum class PlayerType
    {
        UNKNOWN,
        MELEE,  ///< 近战玩家
        RANGED, ///< 远程玩家
        MIXED   ///< 混合玩家
    };
}