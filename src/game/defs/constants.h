#pragma once
#include <glm/vec2.hpp>
#include "../../engine/utils/math.h"

// entt中空的结构提可作为标签使用
namespace game::defs
{
    constexpr float BLOCK_RADIUS{50.0f};
    constexpr float UNIT_RADIUS{25.0f};
    constexpr float PLACE_RADIUS{50.0f};
    constexpr float HOVER_RADIUS{35.0f};

    constexpr engine::utils::FColor RANGE_COLOR{0.0f, 1.0f, 0.0f, 0.2f}; ///< 攻击范围显示颜色

    constexpr glm::vec2 HEALTH_BAR_SIZE{50.0f, 10.0f};
    constexpr float HEALTH_BAR_OFFSET_Y{10.0f};

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