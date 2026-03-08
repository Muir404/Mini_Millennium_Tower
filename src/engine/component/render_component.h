#pragma once

namespace engine::component
{
    struct RenderComponent
    {
        int layer_{};   ///< 渲染层，越大越后渲染
        float depth_{}; ///< 渲染深度，越小越先渲染

        RenderComponent(int layer = 0, float depth = 0.0f) : layer_(layer), depth_(depth) {}

        // 重载比较运算符，用于排序
        // auto operator<=>(const RenderComponent &other) const = default; ///< 渲染组件的比较运算符，先按层排序，再按深度排序 C++20 特性
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
