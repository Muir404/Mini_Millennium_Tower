#pragma once
#include <glm/glm.hpp>

namespace engine::utils
{

    /**
     * @brief 矩形结构体，用于表示二维空间中的矩形区域。
     * @param position 矩形的位置（左上角坐标）。
     * @param size 矩形的大小（宽度和高度）。
     * @param x 矩形的左上角 x 坐标。
     * @param y 矩形的左上角 y 坐标。
     * @param width 矩形的宽度。
     * @param height 矩形的高度。
     * @param right 矩形的右上角 x 坐标（等于 x + width）。
     */
    struct Rect
    {
        glm::vec2 position{};
        glm::vec2 size{};

        Rect() = default;
        Rect(glm::vec2 position, glm::vec2 size) : position(position), size(size) {}
        Rect(float x, float y, float width, float height) : position(x, y), size(width, height) {}
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