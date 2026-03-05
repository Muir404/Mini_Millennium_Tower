#pragma once
#include "../utils/math.h"
#include <SDL3/SDL_rect.h>
#include <entt/core/hashed_string.hpp>
#include <entt/entity/entity.hpp>
#include <glm/vec2.hpp>
#include <glm/common.hpp>
#include <utility>
#include <string>

namespace engine::component
{
    /**
     * @brief 精灵数据结构
     * @note 包含纹理名称、源矩形和是否翻转。
     */
    struct Sprite
    {
        entt::id_type texture_id_{entt::null}; ///< 纹理ID
        std::string texture_path_;             ///< 纹理路径
        engine::utils::Rect src_rect_{};       ///< 源矩形
        bool is_flipped_{false};               ///< 是否翻转

        Sprite() = default; ///< 空的构造函数

        Sprite(std::string texture_path,
               engine::utils::Rect source_rect,
               bool is_flipped = false) : texture_path_(std::move(texture_path)),
                                          src_rect_(std::move(source_rect)),
                                          is_flipped_(is_flipped)
        {
            texture_id_ = entt::hashed_string(texture_path_.c_str());
        }

        Sprite(entt::id_type texture_id, engine::utils::Rect source_rect, bool is_flipped = false)
            : texture_id_(texture_id), src_rect_(std::move(source_rect)), is_flipped_(is_flipped) {}
    };

    /**
     * @brief 精灵组件
     * @note 包含精灵数据、大小、偏移和可见性。
     */
    struct SpriteComponent
    {
        Sprite sprite_;          ///< 精灵
        glm::vec2 size_{0.0f};   ///< 大小
        glm::vec2 offset_{0.0f}; ///< 偏移
        bool is_visible_{true};  ///< 是否可见

        SpriteComponent(Sprite sprite,
                        glm::vec2 size = glm::vec2(0.0f, 0.0f),
                        glm::vec2 offset = glm::vec2(0.0f, 0.0f),
                        bool is_visible = true) : sprite_(std::move(sprite)),
                                                  size_(std::move(size)),
                                                  offset_(std::move(offset)),
                                                  is_visible_(is_visible)
        {
            // 如果size为0（未提供），则使用精灵的源矩形大小
            if (glm::all(glm::equal(size, glm::vec2(0.0f))))
            {
                size_ = glm::vec2(sprite_.src_rect_.size.x, sprite_.src_rect_.size.y);
            }
        }
    };
} // namespace engine::component
