#pragma once
#include <glm/vec2.hpp>

namespace engine::component
{

    /**
     * @brief 组件-速度组件
     */
    struct VelocityComponent
    {
        glm::vec2 velocity_{}; ///< 速度
    };

} // namespace engine::component