#pragma once
#include <glm/glm.hpp>

namespace engine::utils
{

    /**
     * @brief 矩形结构体，用于表示二维空间中的矩形区域。
     * @param position 矩形的位置（左上角坐标）。
     * @param size 矩形的大小（宽度和高度）。
     */
    struct Rect
    {
        glm::vec2 position;
        glm::vec2 size;
    };

    /**
     * @brief 颜色结构体，用于表示 RGBA 颜色值。
     * @param r 红色通道值（0.0 到 1.0）。
     * @param g 绿色通道值（0.0 到 1.0）。
     * @param b 蓝色通道值（0.0 到 1.0）。
     * @param a 透明度通道值（0.0 到 1.0）。
     */
    struct FColor
    {
        float r;
        float g;
        float b;
        float a;
    };

} // namespace engine::utils