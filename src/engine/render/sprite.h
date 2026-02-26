#pragma once
#include <SDL3/SDL_rect.h> // 用于 SDL——FRect
#include <optional>        // 用于 std::optional 表示可选的源矩形
#include <string>
#include <entt/entt.hpp>
#include "../utils/math.h"

namespace engine::utils
{
    struct Rect;
}

namespace engine::render
{
    // 包含纹理标识符和要绘制的纹理部分以及翻转状态
    class Sprite final
    {
    private:
        std::string texture_path_;                       ///< 纹理资源路径
        entt::id_type texture_id_{entt::null};           ///< 纹理资源标识符
        std::optional<engine::utils::Rect> source_rect_; ///< 可选：要绘制的纹理部分
        bool is_flipped_ = false;                        ///< 是否水平翻转

    public:
        Sprite() = default;

        Sprite(std::string_view texture_path,
               const std::optional<engine::utils::Rect> source_rect = std::nullopt,
               bool is_flipped = false)
            : texture_path_(texture_path.data()),
              texture_id_(entt::hashed_string(texture_path_.data())),
              source_rect_(std::move(source_rect)),
              is_flipped_((is_flipped))
        {
        }

        Sprite(entt::id_type texture_id,
               std::optional<engine::utils::Rect> source_rect = std::nullopt,
               bool is_flipped = false)
            : texture_id_(texture_id),
              source_rect_(std::move(source_rect)),
              is_flipped_(is_flipped)
        {
        }

        // getters and setters
        std::string_view getTexturePath() const { return texture_path_; }                        ///< 获取纹理路径
        entt::id_type getTextureId() const { return texture_id_; }                               ///< 获取纹理 ID
        const std::optional<engine::utils::Rect> &getSourceRect() const { return source_rect_; } ///< 获取源矩形 (如果使用整个纹理则为 std::nullopt)
        bool isFlipped() const { return is_flipped_; }                                           ///< 获取是否水平翻转

        void setTexture(std::string_view texture_path) ///< 设置纹理路径 (需确保已载入)
        {
            texture_path_ = texture_path.data();
            texture_id_ = entt::hashed_string(texture_path.data());
        }

        void setTextureId(entt::id_type texture_id) ///< 设置纹理ID (需确保已载入)
        {
            texture_id_ = texture_id;
        }

        void setSourceRect(std::optional<engine::utils::Rect> source_rect) ///< 设置源矩形 (如果使用整个纹理则为 std::nullopt)
        {
            source_rect_ = std::move(source_rect);
        }

        void setFlipped(bool flipped) ///< 设置是否水平翻转
        {
            is_flipped_ = flipped;
        }
    };
}