#pragma once
#include <utility>
#include <glm/vec2.hpp>

namespace engine::component
{
    /**
     * @brief 组件-变换组件
     * @note 用于变换实体的位置、缩放比例和旋转角度
     */
    struct TransformComponent
    {
        glm::vec2 position_{};  ///< 位置
        glm::vec2 scale_{1.0f}; ///< 缩放比例
        float rotation_{};      ///< 旋转角度

        /**
         * @brief 变换组件
         *
         * @param position 位置
         * @param scale 缩放比例
         * @param rotation 旋转角度
         */
        explicit TransformComponent(glm::vec2 position,
                                    glm::vec2 scale = glm::vec2(1.0f, 1.0f),
                                    float rotation = 0.0f) : position_(std::move(position)),
                                                             scale_(std::move(scale)),
                                                             rotation_(rotation)
        {
            // 构造函数
        }
    };
} // namespace engine::component