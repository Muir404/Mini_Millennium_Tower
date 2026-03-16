#pragma once
#include "../utils/math.h"

namespace engine::component
{
    /**
     * @brief 渲染组件
     *
     * 用于渲染实体的组件，包含图层ID和深度。
     * 颜色调整参数（调整后 = 原始颜色 * 颜色调整参数）
     */
    struct RenderComponent
    {
        static constexpr int MAIN_LAYER{10};

        int layer_{};   ///< 图层ID，越小越先渲染
        float depth_{}; ///< 同一图层内的深度，越小越先渲染
        engine::utils::FColor color_{engine::utils::FColor::WHITE()};

        RenderComponent(int layer = MAIN_LAYER,
                        float depth = 0.0f,
                        engine::utils::FColor color = engine::utils::FColor::WHITE()) : layer_(layer),
                                                                                        depth_(depth),
                                                                                        color_(color)
        {
        }

        // 重载比较运算符，用于排序
        bool operator<(const RenderComponent &other) const
        {
            if (layer_ == other.layer_)
            { // 如果图层相同，则比较深度
                return depth_ < other.depth_;
            }
            return layer_ < other.layer_; // 如果图层不同，则比较图层ID
        }
    };
}
