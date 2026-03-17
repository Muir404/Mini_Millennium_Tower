#pragma once
#include <glm/glm.hpp>
#include <string_view>
#include <random>

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
        float r{};
        float g{};
        float b{};
        float a{};
        // 预设颜色
        static constexpr FColor WHITE() noexcept
        {
            return {1.0f, 1.0f, 1.0f, 1.0f};
        }
        static constexpr FColor BLACK() noexcept
        {
            return {0.0f, 0.0f, 0.0f, 1.0f};
        }
        static constexpr FColor RED() noexcept
        {
            return {1.0f, 0.0f, 0.0f, 1.0f};
        }
        static constexpr FColor GREEN() noexcept
        {
            return {0.0f, 1.0f, 0.0f, 1.0f};
        }
        static constexpr FColor BLUE() noexcept
        {
            return {0.0f, 0.0f, 1.0f, 1.0f};
        }
        static constexpr FColor YELLOW() noexcept
        {
            return {1.0f, 1.0f, 0.0f, 1.0f};
        }
        static constexpr FColor CYAN() noexcept
        {
            return {0.0f, 1.0f, 1.0f, 1.0f};
        }
    };

    constexpr FColor
    parseHexColor(std::string_view hex_color)
    {
        auto hexToInt = [](char c) -> int // 将十六进制字符转换为整数
        {
            if (c >= '0' && c <= '9')
            {
                return c - '0'; // 0-9：0-9 的 ASCII 值减去 '0' 的 ASCII 值
            }
            if (c >= 'a' && c <= 'f')
            {
                return c - 'a' + 10; // a-f：a-f 的 ASCII 值减去 'a' 的 ASCII 值再加上 10
            }
            if (c >= 'A' && c <= 'F')
            {
                return c - 'A' + 10; // A-F：A-F 的 ASCII 值减去 'A' 的 ASCII 值再加上 10
            }
            return -1;
        };

        size_t len = hex_color.length(); // 获取十六进制颜色字符串的长度

        // 检查十六进制颜色字符串是否有效
        if (hex_color.empty() || hex_color[0] != '#' || (len != 7 && len != 9))
        {
            return {0, 0, 0, 0};
        }

        // 解析 RGB 通道
        int r = hexToInt(hex_color[1]) * 16 + hexToInt(hex_color[2]);
        int g = hexToInt(hex_color[3]) * 16 + hexToInt(hex_color[4]);
        int b = hexToInt(hex_color[5]) * 16 + hexToInt(hex_color[6]);

        // 解析透明度
        int a = 255;
        if (len == 9)
        {
            a = hexToInt(hex_color[7]) * 16 + hexToInt(hex_color[8]);
        }

        return {
            static_cast<float>(r) / 255.0f,
            static_cast<float>(g) / 255.0f,
            static_cast<float>(b) / 255.0f,
            static_cast<float>(a) / 255.0f}; // 归一化处理
    }

    inline int randomInt(int min, int max)
    {
        static thread_local std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> dist{min, max};
        return dist(rng);
    }

    inline float statModify(float base, int level = 1, int rarity = 1)
    {
        return base * (1 + 0.1f * level) * (1 + 0.1f * rarity);
    }

    inline float distanceSquared(const glm::vec2 &a, const glm::vec2 &b)
    {
        return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
    }

    template <typename RandomIt>
    inline void shuffle(RandomIt first, RandomIt last)
    {
        static thread_local std::mt19937 rng{std::random_device{}()};
        std::shuffle(first, last, rng);
    }

} // namespace engine::utils