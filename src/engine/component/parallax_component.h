#pragma once
#include <glm/vec2.hpp>
#include <utility>

namespace engine::component
{
    /**
     * @brief 视差组件
     * @note 包含视差滚动速度因子、是否重复和可见性。
     */
    struct ParallaxComponent
    {
        glm::vec2 scroll_factor_{}; ///< 滚动速度因子 (0=静止, 1=随相机移动, <1=比相机慢)
        glm::bvec2 repeat_{true};   ///< 是否重复
        bool is_visible_{true};     ///< 是否可见

        ParallaxComponent(glm::vec2 scroll_factor,
                          glm::bvec2 repeat = glm::bvec2(true, true),
                          bool is_visible = true) : scroll_factor_(std::move(scroll_factor)),
                                                    repeat_(std::move(repeat)),
                                                    is_visible_(is_visible) {}
    };
} // namespace engine::component