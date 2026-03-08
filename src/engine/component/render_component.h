#pragma once

namespace engine::component
{
    struct RenderComponent
    {
        int layer_{};   ///< 图层ID，越小越先渲染
        float depth_{}; ///< 同一图层内的深度，越小越先渲染

        RenderComponent(int layer = 0, float depth = 0.0f) : layer_(layer), depth_(depth) {}

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
